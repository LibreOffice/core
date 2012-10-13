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

typedef std::set<std::string> DirContent;
typedef std::map<std::string, DirContent> DirMap;
typedef DirMap::value_type EntriesPair;
typedef std::pair<std::string, std::string> PathFilePair;


struct IncludesCollection {
    private:
    DirMap allIncludes;
    PathFilePair split_path(const std::string& filePath);
    void add_to_collection(const std::string& dirPath);

    public:
    bool exists(std::string filePath);
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
