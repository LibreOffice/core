/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cmdline.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 11:56:13 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

