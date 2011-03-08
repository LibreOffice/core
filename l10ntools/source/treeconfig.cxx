/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include <vector>
#include <string>
#include <iostream>
#include "treeconfig.hxx"
#include "export.hxx"
#ifdef WNT
#include <direct.h>
#include <io.h>
#else
#include <dirent.h>
#endif
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

namespace transex3
{

bool Treeconfig::parseConfig(){

    string source_config_file = string( static_cast<ByteString>( Export::GetEnv("SRC_ROOT") ).GetBuffer() );
    if( source_config_file.empty() )
    {
        cerr << "Error: no suitable environment set?!?";
        exit( -1 );
    }
    source_config_file += string("/source_config");
    if( isConfigFilePresent() )
    {
        inireader.read( map , source_config_file );
        return true;
    }
    else return false;
}

// ALWAYS add all repositories from source_config file to the container active_repos
// if a config_file is present ALWAYS return false
// if you are in the root of a repository also add it to the container active_repos
// if you are far inside a repository /my/path/ooo/sw/source then don't add it to the container but return true
// if you are in some misc place like /tmp then return true
// => the application can decide what to do in case the function returns true thus how to handle pwd() path
bool Treeconfig::getActiveRepositories( vector<string>& active_repos ){

    bool isPresent = isConfigFilePresent();
    bool hasPath   = false;
    string pwd;
    string guessedRepo;
    Export::getCurrentDir( pwd );
    string source_root = Export::GetEnv( "SRC_ROOT" );
    string solarsrc    = Export::GetEnv( "SOLARSRC" );
    string partial;

    // if we are inside of a repository root then active it otherwise let the app handle the return!
    string::size_type pos = pwd.find_first_of( source_root );
    if( pos != string::npos && ( pos + source_root.length() +1 ) < pwd.length()){  // I am within SRC_ROOT
        partial = pwd.substr( pos + source_root.length() +1  , pwd.length());
        string::size_type nextPart = partial.find_first_of( "/" );
        if( nextPart != string::npos )
            hasPath = true;
        else
            guessedRepo = partial;
    }
    else                              // I am NOT within SRC_ROOT
        hasPath = true;

    if( isPresent )
    {
        hasPath = false;                // if config_file is present don't care about pwd
        stringmap* repos = static_cast<stringmap*>( map[ string("repositories") ] );
        if( repos != 0 )
        {
            for( stringmap::iterator iter = repos->begin() ; iter != repos->end() ; ++iter )
            {
                if( static_cast<string>( iter->second ) == string( "active" ) )
                {
                    active_repos.push_back( iter->first );
                    if( static_cast<string>( iter->first ) == guessedRepo )
                    {
                        guessedRepo.clear();            // don't add double in case it is present in config_file
                    }
                }
            }
        }
        else
        {
            cerr << "Error: source_config files doesn't contain a 'repositories' section ?!?";
            exit( -1 );
        }
    }
    if( !guessedRepo.empty() ){
        active_repos.push_back( guessedRepo );          // add myrepo
    }
    return hasPath;                                     // are we deep inside of a source tree or outside of SRC_ROOT?
}

void Treeconfig::getCurrentDir( string& dir )
{
    char buffer[64000];
    if( getcwd( buffer , sizeof( buffer ) ) == 0 ){
        cerr << "Error: getcwd failed!\n";
        exit( -1 );
    }
    dir = string( buffer );
}

bool Treeconfig::isConfigFilePresent()
{
    string config_file = Export::GetEnv( "SRC_ROOT" );
    config_file += "/source_config";

    struct stat status;
    if( stat( config_file.c_str() , &status ) < 0 )
    {
        return false;
    }
#ifdef WNT
    return ( status.st_mode & _S_IFREG ) && ( _access( config_file.c_str() , 4 ) >= 0 ) ;
#else
    return ( status.st_mode & S_IFREG ) && ( access( config_file.c_str() , R_OK ) >= 0 ) ;
#endif
}



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
