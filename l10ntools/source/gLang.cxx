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

#include "gL10nMem.hxx"
#include "gConv.hxx"



class handler
{
    public:
        handler()  {};
        ~handler() {};

        void showRunTimeError(std::string sErr);
        void showUsage(std::string        sErr);
        void checkCommandLine(int argc, char *argv[]);
        void run();

    private:
        bool mbForceSave;
        enum {DO_CONVERT, DO_EXTRACT, DO_MERGE_KID, DO_MERGE} meWorkMode;
        std::string              msTargetDir;
        std::string              msPoDir;
        std::string              msPotDir;
        std::vector<std::string> mvSourceFiles;
        std::vector<std::string> mvLanguages;
        l10nMem                  mcMemory;

        void showManual();
        void loadL10MEM(bool onlyTemplates);
        void runConvert();
        void runExtract();
        void runMerge(bool bKid);
};



void handler::showRunTimeError(std::string sErr)
{
    std::cerr << "runtime error: "
              << (sErr.size() ? sErr : "No description")
              << std::endl;
    exit(-1);
}



void handler::showUsage(std::string sErr)
{
    if (sErr.size())
        std::cerr << "commandline error: " << sErr << std::endl;

    std::cout << "syntax oveview, use \"genLang help\" for full description\n"
                 "   genLang <cmd> <options>\n"
                 "   <cmd> is one of\n"
                 "      convert   convert old pot/po files to new format\n"
                 "      extract   extract pot templates from sources\n"
                 "      help      show manual\n"
                 "      merge     merge po files back to sources\n"
                 "   <options> is a combination of\n"
                 "      -d        show debug information\n"
                 "      -k        generate key identifier version\n"
                 "      -s        save unconditionally\n"
                 "      -v        show progress information\n"
                 "\n"
                 "      --files     <files>       input file list\n"
                 "      --languages <languages>   language list (omitting is all)\n"
                 "      --target    <directory>   target root directory\n"
                 "      --po        <directory>   po root directory\n"
                 "      --pot       <directory>   pot root directory\n";

    if (sErr.size())
        exit(-1);
}



void handler::showManual()
{
    // give the correct usage
    std::cout <<
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

    std::cout <<
        "\n"
        "  genLang extract [-v] [-d] [-s]\n"
        "                  --files <files>   --pot   <directory>\n"
        "    extract text from <files>, result is .pot template\n"
        "    files written to <directory> with a structure\n"
        "\n\n";

    std::cout <<
        "  genLang merge [-v] [-d] [-s] [-k]\n"
        "                --languages <languages>\n"
        "                --target <directory>\n"
        "                --po     <directory>\n"
        "  merges translations (--po) with source files\n"
        "  and write the result to --target\n"
        "\n\n";

    std::cout <<
        "  genLang convert [-v] [-d] [-s]\n"
        "                  --po     <directory>\n"
        "                  --pot    <directory>\n"
        "                  --target <directory>\n"
        "  read old po (--po) and pot (--pot) files and updates\n"
        "  target po and pot files (--target), ready to be loaded\n"
        "  in Pootle\n"
        "\n\n";

    std::cout <<
        "  genLang help\n"
        "    this text\n"
        "\n\n";

    std::cout <<
        "Parameters:\n"
        "      -d        show debug information\n"
        "      -k        generate key identifier version\n"
        "      -v        show progress information\n"
        "      -s        save unconditionally\n"
        "\n"
        "      --files     <files>        input file list\n"
        "      --languages <languages>   language list (omitting is all)\n"
        "      --target    <directory>   target root directory\n"
        "      --po        <directory>   po root directory\n"
        "      --pot       <directory>   pot root directory\n";

    exit(0);
}



void handler::checkCommandLine(int argc, char *argv[])
{
    std::string sWorkText;
    int         i;


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
        else if (sWorkText == "-k") {
            // generate key identifier version
            if (meWorkMode != DO_MERGE)
                showUsage("-k requires \"merge\"");
            meWorkMode = DO_MERGE_KID;
        }
        else if (sWorkText == "-v") {
            // show progress information
            mcMemory.setVerbose(true);
        }
        else if (sWorkText == "-s") {
            // forced save
            mbForceSave = true;
        }
        else if (sWorkText == "--files") {
            // list of input files
            if (meWorkMode != DO_EXTRACT)
                showUsage("--files not valid for command");
            if (i == argc)
                showUsage("--files missing filename arguments");

            // Loop through filenames
            for (; i < argc && argv[i][0] != '-'; ++i)
                mvSourceFiles.push_back(argv[i]);
        }
        else if (sWorkText == "--languages") {
            // list of languages
            if (meWorkMode != DO_MERGE)
                showUsage("--languages not valid for command");
            if (i == argc)
                showUsage("--languages missing arguments");

            // Loop through filenames
            for (; i < argc && argv[i][0] != '-'; ++i)
                mvLanguages.push_back(argv[i]);
        }
        else if (sWorkText == "--target") {
            // target root directory
            if (meWorkMode != DO_MERGE && meWorkMode != DO_CONVERT)
                showUsage("--target not valid for command");
            if (i == argc)
                showUsage("--target missing directory argument");

            msTargetDir = argv[++i];
        }
        else if (sWorkText == "--po") {
            // po file root directory
            if (meWorkMode != DO_MERGE && meWorkMode != DO_CONVERT)
                showUsage("--po not valid for command");
            if (i == argc)
                showUsage("--po missing directory argument");

            msPoDir = argv[++i];
        }
        else if (sWorkText == "--pot") {
            // pot file root directory
            if (meWorkMode != DO_EXTRACT && meWorkMode != DO_CONVERT)
                showUsage("--pot not valid for command");
            if (i == argc)
                showUsage("--pot missing directory argument");

            msPotDir = argv[++i];
        }
        else {
            // collect files
            showUsage("unknown argument");
        }
    }

    // Check all the correct parameters are suplied
    {
        bool bSourceFiles, bLanguages, bTargetDir, bPoDir, bPotDir;

        bSourceFiles = bLanguages = bTargetDir = bPoDir = bPotDir = false;
        switch (meWorkMode)
        {
            case DO_CONVERT:
                 bPoDir = bPotDir = bTargetDir = true;
                 break;
            case DO_EXTRACT:
                 bPotDir = bSourceFiles = true;
                 break;
            case DO_MERGE_KID:
            case DO_MERGE:
                 bPoDir = bLanguages = bTargetDir = true;
                 break;
        }

        if ( (mvSourceFiles.size() > 0) != bSourceFiles)
            throw bSourceFiles ? "--files missing" :
                                 "--files used, but not permitted";
        if ( (mvLanguages.size() > 0) != bLanguages)
            throw bLanguages ?  "--languages missing" :
                                "--languages used, but not permitted";
        if ( (msPoDir.size() > 0) != bPoDir)
            throw bPoDir ? "--po missing" :
                           "--po used, but not permitted";
        if ( (msPotDir.size() > 0) != bPotDir)
            throw bPotDir ? "--pot missing" :
                            "--pot used, but not permitted";
    }
}



void handler::run()
{
    // Start memory module
    loadL10MEM( (meWorkMode == DO_EXTRACT) );

    // use workMode to start correct control part
    switch (meWorkMode)
    {
        case DO_EXTRACT:     runExtract();      break;
        case DO_MERGE:       runMerge(false);   break;
        case DO_MERGE_KID:   runMerge(true);    break;
        case DO_CONVERT:     runConvert();      break;
    }
}



void handler::loadL10MEM(bool onlyTemplates)
{
    std::string sLoad = msPoDir + "templates/";
    std::vector<std::string>::iterator siLang;

    // no convert
    mcMemory.setConvert(false, false);

    // load texts from en-US po file (master)
    // tell system
    l10nMem::showDebug("genLang loading master text from file " + sLoad);

    // and load file
    mcMemory.setLanguage("", true);
    convert_gen (mcMemory, sLoad, msTargetDir, "").execute(false, false);

    if (onlyTemplates)
      return;

    // loop through all languages and load text
    for (siLang = mvLanguages.begin(); siLang != mvLanguages.end(); ++siLang)
    {
        sLoad = msPoDir + *siLang + "/";

        // get converter and extract files
        mcMemory.setLanguage(*siLang, true);

        // tell system
        l10nMem::showDebug("genLang loading text from language file " + sLoad);

        convert_gen(mcMemory, sLoad, msTargetDir, "").execute(false, false);
    }
}



void handler::runConvert()
{
    std::vector<std::string>::iterator siSource;
    std::vector<std::string>::iterator siLang;


    // convert
    mcMemory.setConvert(true, false);

    // loop through all source files, and extract messages from each file
    for (siSource = mvSourceFiles.begin(); siSource != mvSourceFiles.end(); ++siSource)
    {
        // tell system
        l10nMem::showDebug("genLang compare template " + *siSource);

        // get converter and extract files
        convert_gen convertObj(mcMemory, "./", msTargetDir, *siSource);
        convertObj.execute(false, false);

        mcMemory.showNOconvert();

        for (siLang = mvLanguages.begin(); siLang != mvLanguages.end(); ++siLang)
        {
            std::string sFilePath = *siLang + "/";

            // get converter and extract files
            mcMemory.setLanguage(*siLang, false);

            // tell system
            l10nMem::showDebug("genLang convert text from file " +
                               sFilePath + *siSource + " language " + *siLang);

            // get converter and extract files
            //convert_gen convertObj(mcMemory, sFilePath, msTargetDir, *siSource);
            convertObj.execute(true, false);
        }
    }

    // and generate language file
    //mcMemory.saveLanguages(msPoOutDir, mbForceSave);
}



void handler::runExtract()
{
    std::vector<std::string>::iterator siSource;


    // no convert
    mcMemory.setConvert(false, false);

    // loop through all source files, and extract messages from each file
    for (siSource = mvSourceFiles.begin(); siSource != mvSourceFiles.end(); ++siSource)
    {
        // tell system
        l10nMem::showDebug("genLang extracting text from file " + *siSource);

        // get converter and extract file
        convert_gen convertObj(mcMemory, "", msTargetDir, *siSource);
        convertObj.execute(false, false);
    }

    // and generate language file
    mcMemory.saveTemplates(msPoDir, false, mbForceSave);
}



void handler::runMerge(bool bKid)
{
    std::vector<std::string>::iterator siSource;

    // no convert
    mcMemory.setConvert(false, false);

    // loop through all source files, and extract messages from each file
    for (siSource = mvSourceFiles.begin(); siSource != mvSourceFiles.end(); ++siSource)
    {
        // tell system
        l10nMem::showDebug("genLang merging translated text to file " + *siSource);

        // get converter and extract file
        convert_gen convertObj(mcMemory, "", msTargetDir, *siSource);
        convertObj.execute(true, bKid);
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
        std::string myErr(sErr);
        cHandler.showUsage(myErr);
        exit(-1);
    }
    catch(std::string sErr) {
        cHandler.showUsage(sErr);
        exit(-1);
    }

    // command line is ok, so execute it
    try {
        cHandler.run();
    }
    catch(const char *sErr) {
        std::string myErr(sErr);
        cHandler.showRunTimeError(myErr);
        exit(-1);
    }
    catch(std::string sErr) {
        cHandler.showRunTimeError(sErr);
        exit(-1);
    }
}
