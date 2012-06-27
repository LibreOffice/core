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

#ifndef X2C_CMDLINE_HXX
#define X2C_CMDLINE_HXX

#include "sistr.hxx"

class CommandLine
{
  public:
                        CommandLine(
                            int                 i_argc,
                            char *              i_argv[] );
                        ~CommandLine();

    bool                IsOk() const            { return bIsOk; }
    const char *        ErrorText() const;

    const char *        XmlSrcFile() const      { return sXmlSourceFile.str(); }
    const char *        FuncFile() const        { return sFuncFile.str(); }
    const char *        HtmlFile() const        { return sHtmlFile.str(); }
    const char *        TypeInfoFile() const    { return sTypeInfoFile.str(); }
    const char *        DepPath() const         { return sDepPath.str(); }

    bool                IsDepCommand() const    { return sDepPath.l() > 0; }
    const char *        XmlSrcDirectory() const { return sXmlSourceDirectory.str(); }
    const char *        OutputDirectory() const { return sOutputDirectory.str(); }

    const char *        IdlRootPath() const     { return sIdlRootPath.str(); }

  private:
    void                ParseSingleFileCommand(
                            int                 argc,
                            char *              argv[] );
    Simstr              sXmlSourceFile;
    Simstr              sFuncFile;
    Simstr              sHtmlFile;
    Simstr              sTypeInfoFile;

    Simstr              sXmlSourceDirectory;
    Simstr              sOutputDirectory;

    Simstr              sIdlRootPath;
    Simstr              sDepPath;

    bool                bIsOk;
};




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
