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
#include "gLang.hxx"
#include <cstdlib>
#include <iostream>
#include <fstream>



/*****************************************************************************
 ************************   G H A N D L E R . C X X   ************************
 *****************************************************************************
 * This is the control module, that interpret the command line and implement
 * the different work types
 *     extract / merge / generate / convert
 *****************************************************************************/



/**********************   I M P L E M E N T A T I O N   **********************/
handler::handler()
                :
                 mbForceSave(false)
{
}



/**********************   I M P L E M E N T A T I O N   **********************/
handler::~handler()
{
}



/**********************   I M P L E M E N T A T I O N   **********************/
void handler::checkCommandLine(int argc, char *argv[])
{
  enum {ARG_NONE, ARG_F, ARG_O, ARG_S, ARG_T} eGotArg = ARG_NONE;
  std::string sWorkText, sLangText;
  int         argNow, nLen;


  // make internal throw test (to avoid if cascades
  try
  {
    // check for fixed parameter: genLang <cmd> <module> <po dir> <languages>
    if (argc <= 5)
      throw "Not enough parameters";

    // check for working mode
    sWorkText = argv[1];
    if      (sWorkText == "convert")  meWorkMode = DO_CONVERT;
    else if (sWorkText == "extract")  meWorkMode = DO_EXTRACT;
    else if (sWorkText == "merge")    meWorkMode = DO_MERGE;
    else if (sWorkText == "help")     showManual();
    else                              throw "<command> is mandatory";

    // and set fixed parameters
    msModuleName = argv[2];
    msPoOutDir   = msPoDir = argv[3];
    sLangText    = argv[4];
    if (sLangText[0] == '\"')
      sLangText.erase(0,1);
    nLen = sLangText.size() -1;
    if (nLen > 0 && sLangText[nLen] == '\"')
      sLangText.erase(nLen);

    // decode parameters and translate to variables
    for (argNow = 5; argNow < argc;)
    {
      std::string sArg(argv[argNow++]);

      // all -x is followed by a directory/file name
      if (sArg[0] == '-')
      {
        // Terminate collection ?
        if (eGotArg == ARG_F)
          eGotArg = ARG_NONE;

        // Are we waiting for a directory
        if (eGotArg != ARG_NONE)
          throw std::string("missing argument to ") + argv[argNow-1];

        // is it a known parameter
        if      (sArg == "-d") {l10nMem::setShowDebug();               }
        else if (sArg == "-f") {eGotArg   = ARG_F;                     }
        else if (sArg == "-k") {if (meWorkMode == DO_EXTRACT)
                                  meWorkMode = DO_EXTRACT_KID;
                               else
                                  throw "-k is not valid";             }
        else if (sArg == "-o") {eGotArg   = ARG_O; mbForceSave = true; }
        else if (sArg == "-p") {if (meWorkMode == DO_CONVERT)
                                  meWorkMode = DO_CONVERT_POT;
                               else
                                  throw "-p is not valid";             }
        else if (sArg == "-s") {eGotArg   = ARG_S;                     }
        else if (sArg == "-t") {eGotArg   = ARG_T;                     }
        else if (sArg == "-v") {l10nMem::setShowVerbose();             }
        else throw std::string("unknown parameter: ") + sArg;
      }
      else
      {
        switch (eGotArg)
        {
          case ARG_NONE:                                       break;
          case ARG_F: mvSourceFiles.push_back(sArg);           break;
          case ARG_O: msPoOutDir   = sArg; eGotArg = ARG_NONE; break;
          case ARG_S: msSourceDir  = sArg; eGotArg = ARG_NONE; break;
          case ARG_T: msTargetDir  = sArg; eGotArg = ARG_NONE; break;
       }
      }
    }

    // Check parameters mandatory for all commands
    if (!msModuleName.size())
      throw "<module name> is mandatory";
    if (!msPoDir.size())
      throw "<po dir> is mandatory";

    // control parameter usage according to selected action;
    {
      int useLangText, useSoption, useToption;
      useLangText = useSoption = useToption = -1;

      if (!mvSourceFiles.size())
        throw "-f <files> is missing or empty";

      switch (meWorkMode)
      {
        case DO_EXTRACT:
        case DO_EXTRACT_KID:
             break;

        case DO_MERGE:
             useSoption  = 0;
             useLangText = 1;
             useToption  = 1;
             break;

        case DO_CONVERT:
             useLangText = 1;
        case DO_CONVERT_POT:
             useToption  = 1;
             break;
      }

      if (sLangText.size() && useLangText == -1)
        throw "<languages> must be empty";
      if (!sLangText.size() && useLangText == 1)
        throw "<languages> is mandatory";
      if (msSourceDir.size() && useSoption == -1)
        throw "-s <source dir> is not valid";
      if (msSourceDir.size() && useSoption == 1)
        throw "-s <source dir> is mandatory";
      if (msTargetDir.size() && useToption == -1)
        throw "-t <target dir> is not valid";
      if (!msTargetDir.size() && useToption == 1)
        throw "-t <target dir> is mandatory";
    }

    if (msTargetDir.size() && !convert_gen::checkAccess(msTargetDir))
      throw "<target dir> does not exist";
    if (msSourceDir.size() && !convert_gen::checkAccess(msSourceDir))
      throw "<source dir> does not exist";
  }
  catch(const char *sErr)
  {
    std::string myErr(sErr);
    showUsage(myErr);
    exit(-1);
  }
  catch(std::string sErr)
  {
    showUsage(sErr);
    exit(-1);
  }

  // update directories to include final /
  nLen = msSourceDir.size();
  if (nLen && msSourceDir.at(nLen-1) != '/')
    msSourceDir.append("/");
  nLen = msTargetDir.size();
  if (nLen && msTargetDir.at(nLen-1) != '/')
    msTargetDir.append("/");
  nLen = msPoDir.size();
  if (nLen && msPoDir.at(nLen-1) != '/')
    msPoDir.append("/");
  nLen = msPoOutDir.size();
  if (nLen && msPoOutDir.at(nLen-1) != '/')
    msPoOutDir.append("/");

  // and convert language to a vector
  if (sLangText.size())
  {
    int current;
    int next = -1;
    do
    {
      current = next + 1;
      next = sLangText.find_first_of( " ", current );
      std::string sNewLang = sLangText.substr(current,next-current);
      if (sNewLang != "en-US")
        mvLanguages.push_back(sLangText.substr(current,next-current));
    }
    while (next != (int)std::string::npos);
  }

  // check if source files list needs to be coverted
  if (mvSourceFiles[0] == "USEFILE:")
    readFileWithSources();

  // tell system
  l10nMem::showVerbose("genLang starting to " + sWorkText + " from module " + msModuleName);
}



/**********************   I M P L E M E N T A T I O N   **********************/
void handler::run()
{
  try
  {
    // prepare translation memory to module type
    mcMemory.setModuleName(msModuleName);

    // build list of languages (to be loaded and later written
    if (msPoDir.size())
      loadL10MEM( (meWorkMode == DO_EXTRACT) );

    // use workMode to start correct control part
    switch (meWorkMode)
    {
      case DO_EXTRACT:     runExtract(false); break;
      case DO_EXTRACT_KID: runExtract(true);  break;
      case DO_MERGE:       runMerge();        break;
      case DO_CONVERT:     runConvert(false); break;
      case DO_CONVERT_POT: runConvert(true);  break;
    }
  }
  catch(int)
  {
    exit(-1);
  }
}



/**********************   I M P L E M E N T A T I O N   **********************/
void handler::runExtract(bool bKid)
{
  // just to satisfy compiler
  if (bKid)
    return;

  // no convert
  mcMemory.setConvert(false, false);

  // loop through all source files, and extract messages from each file
  for (std::vector<std::string>::iterator siSource = mvSourceFiles.begin(); siSource != mvSourceFiles.end(); ++siSource)
  {
    // tell system
    l10nMem::showDebug("genLang extracting text from file " + msSourceDir + *siSource);

    // get converter and extract file
    convert_gen convertObj(mcMemory, msSourceDir, msTargetDir, *siSource);
    convertObj.execute(false);
  }

  // and generate language file
  mcMemory.saveTemplates(msPoOutDir, false, mbForceSave);
}



/**********************   I M P L E M E N T A T I O N   **********************/
void handler::runMerge()
{
  // no convert
  mcMemory.setConvert(false, false);

  // loop through all source files, and extract messages from each file
  for (std::vector<std::string>::iterator siSource = mvSourceFiles.begin(); siSource != mvSourceFiles.end(); ++siSource)
  {
    // tell system
    l10nMem::showDebug("genLang merging translated text to file " + msSourceDir + *siSource);

    // get converter and extract file
    convert_gen convertObj(mcMemory, msSourceDir, msTargetDir, *siSource);
    convertObj.execute(true);
  }
}



/**********************   I M P L E M E N T A T I O N   **********************/
void handler::runConvert(bool bPot)
{
  // convert
  mcMemory.setConvert(true, bPot);

  // loop through all source files, and extract messages from each file
  for (std::vector<std::string>::iterator siSource = mvSourceFiles.begin(); siSource != mvSourceFiles.end(); ++siSource)
  {
    // handle .pot (templates)
    if (bPot)
    {
      // tell system
      l10nMem::showDebug("genLang compare template " + msSourceDir + *siSource);

      // get converter and extract files
      convert_gen convertObj(mcMemory, "./", msTargetDir, *siSource);
      convertObj.execute(false);

      if (bPot)
        mcMemory.showNOconvert();
    }
    else
      for (std::vector<std::string>::iterator siLang = mvLanguages.begin(); siLang != mvLanguages.end(); ++siLang)
      {
        std::string sFilePath = msSourceDir + *siLang + "/";

        // get converter and extract files
        mcMemory.setLanguage(*siLang, false);

        // tell system
        l10nMem::showDebug("genLang convert text from file " + sFilePath + *siSource + " language " + *siLang);

        // get converter and extract files
        convert_gen convertObj(mcMemory, sFilePath, msTargetDir, *siSource);
        convertObj.execute(true);
      }
  }

  // and generate language file
  if (!bPot)
    mcMemory.saveLanguages(msPoOutDir, mbForceSave);
}



/**********************   I M P L E M E N T A T I O N   **********************/
void handler::showUsage(std::string& sErr)
{
  // do we have an error text ?
  if (sErr.size())
    std::cerr << "commandline error:" << sErr << std::endl;

  std::cout <<
    "syntax oveview, use \"genLang help\" for full description\n"
    "genLang <cmd> <module> <po dir> <languages> [-d] [-f <files>] [-k] [-o <dir>] [-p] [-s <dir>] [-t <dir>] [-v]\n"
    "<cmd> is one of \"convert\", \"extract\", \"help\", \"merge\",\n";

  exit(-1);
}



/**********************   I M P L E M E N T A T I O N   **********************/
void handler::showManual()
{
  // give the correct usage
  std::cout <<
    "genLang (c)2013 by Apache Software Foundation\n"
    "=============================================\n"
    "As part of the L10N framework for Apache Open Office (AOO),\n"
    "genLang extracts en-US texts sources of the following types:\n"
    "  .xrm, .xhp, .xcu, .xcs, .ulf, .tree, .src, .prop and .po (.pot)\n"
    "and merges with .po files in different languages.\n"
    "genLang uses .po files and AOO sources to generate language sources.\n"
    "\n"
    "genLang can also convert old .po files (generated from .sdf)\n"
    "\n";

  std::cout <<
    "Syntax:\n"
    "  genLang extract <module> <po dir> \"\" [-v] [-d] \\\n"
    "          -f <files> [-s <source dir>]\n"
    "    if -s is omitted . is used\n"
    "    extract text from <source dir>/<files>, result is merged and\n"
    "    written to <po dir>/<module>.pot\n"
    "\n";
  std::cout <<
    "  genLang merge <module> <po dir> <languages> [-v] [-d] [-k]\\\n"
    "          [-o <po outdir>]  -f <files> [-s <source dir>] \\\n"
    "          -t <target dir>\n"
    "    works as \"extract\" and additionally merges\n"
    "    <source dir>/<files> with all native language text\n"
    "    from <po dir>/*lang/<module>.po\n"
    "    The result is stored in <target dir>/<files>\n"
    "\n";
  std::cout <<
    "  genLang convert <module> <po dir> <languages> [-v] [-d]\\\n"
    "          [-p] [-o <po outdir>] -s <source dir> -f <files>\n"
    "    reads sdf generated .po <files> from\n"
    "    <source dir>/*lang/<module>.po\n"
    "    and merges with\n"
    "    <po dir>/*lang/<module>.po\n"
    "    Result is written to <po outdir>/*lang/<module>.po if\n"
    "    present or <po dir>/*lang/<module>.po is overwritten\n"
    "    - Keys in <source dir>, not in <module>.po\n"
    "      are shown as warnings\n"
    "    - Keys in <source dir>, with changed translation\n"
    "      are marked \"fuzzy\"\n"
    "    - Keys in source files not in .po files (new keys)\n"
    "      are added and marked \"fuzzy\"\n"
    "    if -p is used, .pot files is assumed, and a compare is made\n"
    "\n";
  std::cout <<
    "  genLang help\n"
    "    this text\n"
    "\n";
  std::cout <<
    "Parameters:\n"
    "  <module>\n"
    "     name of module (directory in main)\n"
    "  <po dir>\n"
    "     directory containing a directory for each language which contains\n"
    "     a .po file for each module or a module.dbpo file for fast loading\n"
    "  <languages>\n"
    "     comma separated string with langauge id to be used\n"
    "\n"
    "  -d\n"
    "     extensive verbose mode, tells what genLang is doing in detail\n"
    "  -f <files>\n"
    "     list of files containing messages to be extracted\n"
    "     \"convert\" expect sdf generated po files, to be converted\n"
    "     instead of passing a list of files, it is possible to pass\n"
    "     a file contains the list, by using:\n"
    "     -f USEFILE: <filename>\n"
    "  -k\n"
    "     generate kid id (hex) for all messages in the source code,\n"
    "     solely for QA\n"
    "  -o <po outdir>\n"
    "     directory to write .po files, same structure as -p\n"
    "  -p\n"
    "     used with convert, when comparing .pot files (old system == new system)\n"
    "  -s <source dir>\n"
    "     directory containing all source files (root path for <files>\n"
    "  -t <target dir>\n"
    "     <directory> used to write merged source files (root path for <files>\n"
    "  -v\n"
    "     verbose mode, tells what genLang is doing\n";

  exit(0);
}



/**********************   I M P L E M E N T A T I O N   **********************/
void handler::loadL10MEM(bool onlyTemplates)
{
  std::string sMod  = msModuleName + ".pot";
  std::string sLoad = msPoDir + "templates/";

  // no convert
  mcMemory.setConvert(false, false);

  // load texts from en-US po file (master)
  // tell system
  l10nMem::showDebug("genLang loading master text from file " + sLoad + sMod);

  // and load file
  mcMemory.setLanguage("", true);
  convert_gen (mcMemory, sLoad, msTargetDir, sMod).execute(false);

  if (onlyTemplates)
    return;

  // loop through all languages and load text
  sMod  = msModuleName + ".po";
  for (std::vector<std::string>::iterator siLang = mvLanguages.begin(); siLang != mvLanguages.end(); ++siLang)
  {
    sLoad = msPoDir + *siLang + "/";

    // get converter and extract files
    mcMemory.setLanguage(*siLang, true);

    // tell system
    l10nMem::showDebug("genLang loading text from language file " + sLoad + sMod);

    convert_gen(mcMemory, sLoad, msTargetDir, sMod).execute(false);
  }
}



/**********************   I M P L E M E N T A T I O N   **********************/
void handler::readFileWithSources()
{
  std::ifstream fInput;
  char          buf[256];


  if (mvSourceFiles.size() < 2)
    throw l10nMem::showError("missing file with sources (-f USEFILE: <filename>)");

  fInput.open (mvSourceFiles[1].c_str(), std::ifstream::in);
  if (!fInput.is_open())
    throw l10nMem::showError("Cannot open file with sources (-f), trying to open" + mvSourceFiles[1]);

  mvSourceFiles.clear();

  while (fInput.good())
  {
    fInput.getline(buf, sizeof(buf));
    if (!buf[0])
      continue;
    mvSourceFiles.push_back(buf);
  }

  fInput.close();
}
