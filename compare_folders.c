#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

typedef struct FilePath
{
    char *path;
    struct FilePath *next;
} FilePath;

void Add(FilePath **fp, const char *path)
{
    FilePath *node = (FilePath *)malloc(sizeof(FilePath));
    if (node == NULL)
    {
        fprintf(stderr, "Memory allocation error\n");
        exit(1);
    }

    node->path = (char *)malloc(strlen(path) + 1); // +1 for the null terminator
    if (node->path == NULL)
    {
        fprintf(stderr, "Memory allocation error\n");
        exit(1);
    }

    strcpy(node->path, path);
    node->next = NULL;

    if (*fp == NULL)
        *fp = node;
    else
    {
        FilePath *p = *fp;
        while (p->next)
        {
            p = p->next;
        }
        p->next = node;
    }
}

void PrintPaths(FilePath *fp)
{
    for (FilePath *p = fp; p; p = p->next)
        printf("%s\n", p->path);
}

char* Relative(const char* root, const char* full_path) {
    size_t root_len = strlen(root);
    size_t full_path_len = strlen(full_path);

    if (root_len > full_path_len || strncmp(root, full_path, root_len) != 0)
        return NULL;

    size_t relative_len = full_path_len - root_len;


    char* relative_path = (char*)malloc(relative_len + 1);
    if (relative_path == NULL) {
        perror("Error allocating memory");
        exit(1);
    }

    strncpy(relative_path, full_path + root_len, relative_len);
    relative_path[relative_len] = '\0';

    return relative_path;
}

void PrintPathsRelative(FilePath* fp,const char* root)
{
    for (FilePath *p = fp; p; p = p->next)
        printf("%s\n",Relative(root,p->path));
}

FilePath *GetFiles(const char *path)
{
    DIR *dir = opendir(path);
    if (dir == NULL)
    {
        perror("Error opening directory");
        return NULL;
    }

    FilePath *fileList = NULL;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        char full_path[1024]; // Adjust the size as needed
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        if (entry->d_type == DT_REG)
        {
            // Create a new FilePath node and add it to the list
            FilePath *node = (FilePath *)malloc(sizeof(FilePath));
            if (node == NULL)
            {
                perror("Error allocating memory");
                exit(1);
            }
            node->path = strdup(full_path);
            node->next = fileList;
            fileList = node;
        }
        else if (entry->d_type == DT_DIR)
        {
            FilePath *subFiles = GetFiles(full_path);

            // Merge the subdirectory's file list with the current file list
            if (subFiles)
            {
                FilePath *temp = subFiles;
                while (temp->next)
                {
                    temp = temp->next;
                }
                temp->next = fileList;
                fileList = subFiles;
            }
        }
    }

    closedir(dir);
    return fileList;
}

void FreeFilePathList(FilePath *list)
{
    while (list)
    {
        FilePath *temp = list;
        list = list->next;
        free(temp->path);
        free(temp);
    }
}

int CompareFileBytes(const char *file1Path, const char *file2Path) {
    FILE *file1 = fopen(file1Path, "rb");
    FILE *file2 = fopen(file2Path, "rb");

    if (file1 == NULL || file2 == NULL) {
        perror("Error opening files");
        return -1; // Return an error code
    }

    int isIdentical = 1;
    int ch1, ch2;

    while (1) {
        ch1 = fgetc(file1);
        ch2 = fgetc(file2);

        if (ch1 != ch2) {
            isIdentical = 0;
            break; // Files are not identical
        }

        if (ch1 == EOF || ch2 == EOF) {
            break; // Reached the end of one or both files
        }
    }

    fclose(file1);
    fclose(file2);
    return isIdentical;
}

int Exists(const char* path,FilePath* fp,const char* root1,const char* root2)
{
    for(FilePath* p=fp;p;p=p=p->next)
        if(!strcmp(Relative(root1,path),Relative(root2,p->path)))
            return 1;
    return 0;
}

void CompareDir(const char* dir1,const char* dir2,char relative)
{
    FilePath* dir1_files=GetFiles(dir1);
    FilePath* dir2_files=GetFiles(dir2);

    FilePath* removed=NULL;
    FilePath* modified=NULL;
    FilePath* added=NULL;


    for(FilePath* p1=dir1_files;p1;p1=p1->next)
        if(!Exists(p1->path,dir2_files,dir1,dir2))
            Add(&removed,p1->path);


    for(FilePath* p1=dir1_files;p1;p1=p1->next)
        for(FilePath* p2=dir2_files;p2;p2=p2->next)
            if(!strcmp(Relative(dir1,p1->path),Relative(dir2,p2->path)))
                if(!CompareFileBytes(p1->path,p2->path))
                    Add(&modified,p1->path);

    for(FilePath* p2=dir2_files;p2;p2=p2->next)
        if(!Exists(p2->path,dir1_files,dir2,dir1))
            Add(&added,p2->path);

    printf("Removed Files:\n");
    relative?PrintPathsRelative(removed,dir1):PrintPaths(removed);
    printf("\n");

    printf("Modified Files:\n");
    relative?PrintPathsRelative(modified,dir1):PrintPaths(modified);
    printf("\n");

    printf("Added Files:\n");
    relative?PrintPathsRelative(added,dir2):PrintPaths(added);
    printf("\n");
}

int main()
{
    //FilePath *path1 = PrintAndGetFiles("in_dir");
    //PrintPaths(GetFiles("C:\\Users\\uig33827\\Desktop\\dir"));
    //FilePath* path1=GetFiles("C:\\Users\\uig33827\\Desktop\\dir");
    //CompareDir("C:\\Users\\uig33827\\Desktop\\dir","C:\\Users\\uig33827\\Desktop\\dir2",1);
    CompareDir("C:\\Users\\uig33827\\Desktop\\MOD_And","C:\\Users\\uig33827\\Desktop\\MOD_And(or)",1);
}
