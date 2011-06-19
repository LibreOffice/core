/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef COLLECTDIRCONTENT_H
#define COLLECTDIRCONTENT_H

#if defined __cplusplus

#include <set>
#include <map>
#include <string>

#if defined( WNT )
#include <windows.h>
#include <algorithm>
#else
#include <dirent.h>
#endif // defined( WNT )

#include <iostream>

using namespace std;

typedef set<string> DirContent;
typedef map<string, DirContent> DirMap;
typedef DirMap::value_type EntriesPair;
typedef pair<string, string> PathFilePair;


struct IncludesCollection {
    private:
    DirMap allIncludes;
    PathFilePair split_path(const string& filePath);
    void add_to_collection(const string& dirPath);

    public:
        bool exists(string filePath);
};

#else

struct IncludesCollection;

#endif

#if defined __cplusplus
extern "C" {
#endif

struct IncludesCollection * create_IncludesCollection(void);
void delete_IncludesCollection(struct IncludesCollection *);

int call_IncludesCollection_exists(struct IncludesCollection* m, const char* filePath);

#if defined __cplusplus
}
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
