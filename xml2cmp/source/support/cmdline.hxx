/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef X2C_CMDLINE_HXX
#define X2C_CMDLINE_HXX

#include "sistr.hxx"
#include "list.hxx"


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

    bool                IsIndexCommand() const  { return sIndexFile.l() > 0; }
    const char *        XmlSrcDirectory() const { return sXmlSourceDirectory.str(); }
    const char *        IndexOutputFile() const { return sIndexFile.str(); }
    const char *        OutputDirectory() const { return sOutputDirectory.str(); }
    const List<Simstr> &
                        IndexedTags() const     { return aTagsInIndex; }

    const char *        IdlRootPath() const     { return sIdlRootPath.str(); }

  private:
    void                ParseIndexCommand(
                            int                 argc,
                            char *              argv[] );
    void                ParseSingleFileCommand(
                            int                 argc,
                            char *              argv[] );
    Simstr              sXmlSourceFile;
    Simstr              sFuncFile;
    Simstr              sHtmlFile;
    Simstr              sTypeInfoFile;

    Simstr              sXmlSourceDirectory;
    Simstr              sIndexFile;
    Simstr              sOutputDirectory;
    List<Simstr>        aTagsInIndex;

    Simstr              sIdlRootPath;

    bool                bIsOk;
};




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
