/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include <vector>
#include <string>

#include "inireader.hxx"

namespace transex3{

class Treeconfig
{

    private:
        INIreader   inireader;
        INImap      map;
        bool        has_config_file;
        void getCurrentDir( string& dir );
        bool isConfigFilePresent();

    public:

        Treeconfig() : has_config_file( false ) { parseConfig(); }
        // read the config file, returns true in case a config file had been found
        bool parseConfig();
        // returns a string vector containing all active  repositories, returns true in case we are deep inside
        // of a source tree. This could affect the behavour of the tool
        bool getActiveRepositories( vector<string>& active_repos);
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
