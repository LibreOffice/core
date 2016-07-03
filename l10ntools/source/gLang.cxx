/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#include <iostream>
#include <string>
#include <vector>
using namespace std;

#include "gL10nMem.hxx"
#include "gConv.hxx"



class handler
{
    public:
        handler()  {};
        ~handler() {};

        static void showRunTimeError(string sErr);
        static void showUsage(string        sErr);
        void checkCommandLine(int argc, char *argv[]);
        void run();

    private:
        bool mbForceSave;
        enum {DO_CONVERT, DO_EXTRACT, DO_MERGE} meWorkMode;
        string         msTargetDir;
        string         msPoDir;
        vector<string> mvSourceFiles;
        l10nMem        mcMemory;

        static void showManual();
        void loadL10MEM(bool onlyTemplates);
        void runConvert();
        void runExtract();
        void runMerge();
};



void handler::showRunTimeError(string sErr)
{
    cerr << "runtime error: "
         << (sErr.size() ? sErr : "No description")
         << endl;
    exit(-1);
}



void handler::showUsage(string sErr)
{
    if (sErr.size())
        cerr << "commandline error: " << sErr << endl;

    cout << "syntax oveview, use \"genLang help\" for full description\n"
        "   genLang <cmd> <options>\n"
        "   <cmd> is one of\n"
        "      convert   convert old pot/po files to new format\n"
        "      extract   extract pot templates from sources\n"
        "      help      show manual\n"
        "      merge     merge po files back to sources\n"
        "   <options> is a combination of\n"
        "      -d        show debug information\n"
        "      -s        save unconditionally\n"
        "      -v        show progress information\n"
        "\n"
        "      --files     <files>       input file list\n"
        "      --target    <directory>   target root directory\n"
        "      --base      <directory>   base directory for files\n"
        "      --po        <directory>   po root directory\n";

    if (sErr.size())
        exit(-1);
}



void handler::showManual()
{
    // give the correct usage
    cout <<
        "genLang(c) 2016 by Document Foundation\n"
        "=============================================\n"
        "As part of the L10N framework for LibreOffice,\n"
        "genLang extracts en-US texts sources of the following types:\n"
        "  .xrm, .xhp, .xcu, .xcs, .ulf, .tree, .src, .prop\n"
        "to generate .pot files used for translation\n"
        "genLang extract localized texts from\n"
        "  .po\n"
        "to generate sources containing all translated languages\n"
        "\n"
        "genLang can also convert old .po and .pot files\n"
        "the conversion makes tool changes transparent to translators\n"
        "\n"
        "Syntax:\n\n";

    showUsage("");

    cout <<
        "\n"
        "  genLang extract [-v] [-d] [-s]\n"
        "                  --base <directory> --files <files> --target <directory>\n"
        "    extract text from source (.ui etc) <files>, result is .pot\n"
        "    templates files written to <directory> with a structure\n"
        "    files are expanded with --base, to shorten command line"
        "\n\n";

    cout <<
        "  genLang merge [-v] [-d] [-s]\n"
        "                --base  <directory>"
        "                --files <files>\n"
        "                --target <directory>\n"
        "                --po     <directory>\n"
        "  merges translations (--po) with source files (--files)\n"
        "  and write the result to --target\n"
        "  files are expanded with --base, to shorten command line"
        "\n\n";

    cout <<
        "  genLang convert [-v] [-d] [-s]\n"
        "                  --base  <directory>"
        "                  --files <files>\n"
        "                  --po     <directory>\n"
        "                  --target <directory>\n"
        "  read old po (--po) and new po (--files) files and\n"
        "  write po files (--target), ready to be loaded\n"
        "  files are expanded with --base, to shorten command line"
        "  in Pootle\n"
        "\n\n";

    cout <<
        "  genLang help\n"
        "    this text\n"
        "\n\n";
    exit(0);
}



void handler::checkCommandLine(int argc, char *argv[])
{
    string sWorkText, sBaseDir("");
    int    i;
    bool   bSourceFiles, bTargetDir, bPoDir;


    // Set default
    mbForceSave = false;

    // check for fixed parameter: genLang <cmd>
    if (argc < 2)
      throw "Not enough parameters";

    // check for working mode
    sWorkText = argv[1];
    if      (sWorkText == "convert")  meWorkMode = DO_CONVERT;
    else if (sWorkText == "extract")  meWorkMode = DO_EXTRACT;
    else if (sWorkText == "merge")    meWorkMode = DO_MERGE;
    else if (sWorkText == "help")     showManual();
    else                              throw "<command> is mandatory";

    // loop through all variable arguments
    for (i = 2; i < argc; ++i) {
        sWorkText = argv[i];
        if (sWorkText == "-d") {
            // show debug information
            mcMemory.setDebug(true);
        }
        else if (sWorkText == "-v") {
            // show progress information
            mcMemory.setVerbose(true);
        }
        else if (sWorkText == "-s") {
            // forced save
            mbForceSave = true;
        }
        else {
            // These arguments, all need and extra argument
            if (i == argc - 1)
                throw sWorkText + " missing filename arguments";

            if (sWorkText == "--files") {
                // Loop through filenames
                for (++i; i < argc && argv[i][0] != '-'; ++i)
                    mvSourceFiles.push_back(sBaseDir + argv[i]);
                --i;
            }
            else if (sWorkText == "--base") {
                sBaseDir = argv[++i];
                if (sBaseDir[sBaseDir.length() - 1] != '/')
                    sBaseDir += "/";
            }
            else if (sWorkText == "--target") {
                msTargetDir = argv[++i];
                if (msTargetDir[msTargetDir.length() - 1] != '/')
                    msTargetDir += "/";
            }
            else if (sWorkText == "--po") {
                msPoDir = argv[++i];
                if (msPoDir[msPoDir.length() - 1] != '/')
                    msPoDir += "/";
            }
            else
                throw "unknown argument";
        }
    }

    // Check all the correct parameters are supplied
    bSourceFiles = bTargetDir = bPoDir = false;
    switch (meWorkMode) {
        case DO_CONVERT:
             bSourceFiles = bPoDir = bTargetDir = true;
             break;
        case DO_EXTRACT:
             bSourceFiles = bTargetDir = true;
             break;
        case DO_MERGE:
             bPoDir = bTargetDir = true;
             break;
    }

    if ( (mvSourceFiles.size() > 0) != bSourceFiles)
        throw bSourceFiles ? "--files missing" : "--files used, but not permitted";
    if ( (msPoDir.size() > 0) != bPoDir)
        throw bPoDir ? "--po missing" : "--po used, but not permitted";
}



void handler::run()
{
    // use workMode to start correct control part
    switch (meWorkMode)
    {
        case DO_EXTRACT:     runExtract(); break;
        case DO_MERGE:       runMerge();   break;
        case DO_CONVERT:     runConvert(); break;
    }
}



void handler::loadL10MEM(bool onlyTemplates)
{
    string sLoad = msPoDir + "templates/";
//FIX    vector<string>::iterator siLang;

    // no convert
    mcMemory.setConvert(false, false);

    // load texts from en-US po file (master)
    // tell system
    l10nMem::showDebug("genLang loading master text from file " + sLoad);

    // and load file
    mcMemory.setLanguage("", true);
    convert_gen::createInstance(mcMemory, sLoad, msTargetDir, "").execute(false);

    if (onlyTemplates)
      return;

    // loop through all languages and load text
//FIX    for (siLang = mvLanguages.begin(); siLang != mvLanguages.end(); ++siLang)
//FIX    {
//FIX        sLoad = msPoDir + *siLang + "/";

        // get converter and extract files
//FIX        mcMemory.setLanguage(*siLang, true);

        // tell system
//FIX        l10nMem::showDebug("genLang loading text from language file " + sLoad);

//FIX        convert_gen::createInstance(mcMemory, sLoad, msTargetDir, "").execute(false, false);
//FIX    }
}



void handler::runConvert()
{
    vector<string>::iterator siSource;
//FIX    vector<string>::iterator siLang;


    // convert
    loadL10MEM(true);
    mcMemory.setConvert(true, false);

    // loop through all source files, and extract messages from each file
    for (siSource = mvSourceFiles.begin(); siSource != mvSourceFiles.end(); ++siSource) {
        // tell system
        l10nMem::showDebug("genLang compare template " + *siSource);

        // get converter and extract files
        convert_gen& convertObj = convert_gen::createInstance(mcMemory, "./", msTargetDir, *siSource);
        convertObj.execute(false);

        mcMemory.showNOconvert();

//FIX        for (siLang = mvLanguages.begin(); siLang != mvLanguages.end(); ++siLang) {
//FIX            string sFilePath = *siLang + "/";

            // get converter and extract files
//FIX            mcMemory.setLanguage(*siLang, false);

            // tell system
//FIX            l10nMem::showDebug("genLang convert text from file " +
//FIX                               sFilePath + *siSource + " language " + *siLang);

            // get converter and extract files
//FIX            convert_gen& convertObj = convert_gen::createInstance(mcMemory, sFilePath, msTargetDir, *siSource);
//FIX            convertObj.execute(true, false);
//FIX        }
    }

    // and generate language file
    //mcMemory.saveLanguages(msPoOutDir, mbForceSave);
}



void handler::runExtract()
{
    vector<string>::iterator siSource;
    int newPos;

    // no convert
    mcMemory.setConvert(false, false);

    // loop through all source files, and extract messages from each file
    for (siSource = mvSourceFiles.begin(); siSource != mvSourceFiles.end(); ++siSource) {
        // tell system
        l10nMem::showDebug("genLang extracting text from file " + *siSource);

        // set module name
        newPos = (*siSource).find_last_of("/\\", (*siSource).length());
        mcMemory.setModuleName((*siSource).substr(0, newPos));

        // get converter and extract file
        convert_gen& convertObj = convert_gen::createInstance(mcMemory, "", msTargetDir, *siSource);
        convertObj.execute(false);
    }

    // and generate language file
    mcMemory.saveTemplates(msTargetDir, mbForceSave);
}



void handler::runMerge()
{
    vector<string>::iterator siSource;

    // no convert
    mcMemory.setConvert(false, false);

    // loop through all source files, and extract messages from each file
    for (siSource = mvSourceFiles.begin(); siSource != mvSourceFiles.end(); ++siSource) {
        // tell system
        l10nMem::showDebug("genLang merging translated text to file " + *siSource);

        // get converter and extract file
        convert_gen& convertObj = convert_gen::createInstance(mcMemory, "", msTargetDir, *siSource);
        convertObj.execute(true);
    }
}



int main(int argc, char *argv[])
{
    handler cHandler;

    // check command line (exit if problems)
    try {
        cHandler.checkCommandLine(argc, argv);
    }
    catch(const char *sErr) {
        string myErr(sErr);
        handler::showUsage(myErr);
        exit(-1);
    }
    catch(const string& sErr) {
        handler::showUsage(sErr);
        exit(-1);
    }

    // command line is ok, so execute it
    try {
        cHandler.run();
    }
    catch(const char *sErr) {
        string myErr(sErr);
        handler::showRunTimeError(myErr);
        exit(-1);
    }
    catch(const string& sErr) {
        handler::showRunTimeError(sErr);
        exit(-1);
    }
}
