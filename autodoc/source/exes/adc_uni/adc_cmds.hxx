/*************************************************************************
 *
 *  $RCSfile: adc_cmds.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:27 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef ADC_ADC_CMDS_HXX
#define ADC_ADC_CMDS_HXX



// USED SERVICES
    // BASE CLASSES
#include <cosv/comdline.hxx>
    // COMPONENTS
#include <cosv/ploc.hxx>
    // PARAMETERS

namespace ary
{
    class Repository;
}

namespace autodoc
{
    class CommandLine;

namespace command
{


class Command
{
  public:
    virtual             ~Command() {}

    /** @return The first not parsed argument.
        If an error occured, 0 is returned.
    */
    char * *            Init(
                            char * *            i_nCurArgsBegin,
                            char * *            i_nEndOfAllArgs );
  private:
    virtual char * *    do_Init(
                            char * *            i_nCurArgsBegin,
                            char * *            i_nEndOfAllArgs ) = 0;
};

inline char * *
Command::Init( char * *            i_nCurArgsBegin,
               char * *            i_nEndOfAllArgs )
    { return do_Init(i_nCurArgsBegin, i_nEndOfAllArgs); }


struct S_ProjectData;
struct S_LanguageInfo;
struct S_ExternLinkage;

class Parse : public Command
{
  public:
    typedef std::vector< DYN S_ProjectData * >  ProjectList;
    typedef ProjectList::const_iterator         ProjectIterator;

                        Parse(
                            CommandLine &       io_rCommandLine );
                        ~Parse();

    // INQUIRY
    const udmstri &     ReposyName() const;
    const S_LanguageInfo *
                        GlobalLanguageInfo() const;
    bool                GlobalIsHtmlDefaultForDocs() const;
    ProjectIterator     ProjectsBegin() const;
    ProjectIterator     ProjectsEnd() const;


    // ACCESS
    S_ProjectData &     CreateDefaultProject();

  private:
    // Interface Command:
    virtual char * *    do_Init(
                            char * *            i_nCurArgsBegin,
                            char * *            i_nEndOfAllArgs );

    typedef char * * (Parse::*F_Init)(char * *, char * *);
    typedef std::map< udmstri , F_Init >    InitMap;

    static const InitMap &
                        Options();
    F_Init              FindFI(
                            const char *        i_pArg ) const;

    char * *            FI_Start_ParseOptions(
                            char * *            i_nCurArgsBegin,
                            char * *            i_nEndOfAllArgs );
    char * *            FI_SetName(
                            char * *            i_nCurArgsBegin,
                            char * *            i_nEndOfAllArgs );
    char * *            FI_SetUpdate(
                            char * *            i_nCurArgsBegin,
                            char * *            i_nEndOfAllArgs );
    char * *            FI_SetLanguage4All(
                            char * *            i_nCurArgsBegin,
                            char * *            i_nEndOfAllArgs );
    char * *            FI_SetExtensions4All(
                            char * *            i_nCurArgsBegin,
                            char * *            i_nEndOfAllArgs );
    char * *            FI_SetDocAttrs4All(
                            char * *            i_nCurArgsBegin,
                            char * *            i_nEndOfAllArgs );

    char * *            FI_Start_ProjectOptions(
                            char * *            i_nCurArgsBegin,
                            char * *            i_nEndOfAllArgs );
    char * *            FI_SetLanguage(
                            char * *            i_nCurArgsBegin,
                            char * *            i_nEndOfAllArgs );
    char * *            FI_SetExtensions(
                            char * *            i_nCurArgsBegin,
                            char * *            i_nEndOfAllArgs );
    char * *            FI_SetDocAttrs(
                            char * *            i_nCurArgsBegin,
                            char * *            i_nEndOfAllArgs );
    char * *            FI_SetSourceDirs(
                            char * *            i_nCurArgsBegin,
                            char * *            i_nEndOfAllArgs );
    char * *            FI_SetSourceTrees(
                            char * *            i_nCurArgsBegin,
                            char * *            i_nEndOfAllArgs );
    char * *            FI_SetSourceFiles(
                            char * *            i_nCurArgsBegin,
                            char * *            i_nEndOfAllArgs );

    // DATA
    udmstri             sRepositoryName;
    udmstri             sRepositoryDirectoryForUpdate;

    Dyn<S_LanguageInfo> pGlobal_Language;
    bool                bGlobal_HtmlIsDefaultForDocs;

    ProjectList         aProjects;

    CommandLine *       pCommandLine;
    static InitMap      aOptions_;
};

inline const udmstri &
Parse::ReposyName() const
    { return sRepositoryName; }
inline const S_LanguageInfo *
Parse::GlobalLanguageInfo() const
    { return pGlobal_Language ? pGlobal_Language.Ptr() : 0; }
inline bool
Parse::GlobalIsHtmlDefaultForDocs() const
    { return bGlobal_HtmlIsDefaultForDocs; }
inline Parse::ProjectIterator
Parse::ProjectsBegin() const
    { return aProjects.begin(); }
inline Parse::ProjectIterator
Parse::ProjectsEnd() const
    { return aProjects.end(); }

class Load : public Command
{
  public:
                        Load(
                            const char *        i_sRepositoryDirectory = "" );
                        ~Load();

    const udmstri &     ReposyDir() const;

  private:
    // Interface Command:
    virtual char * *    do_Init(
                            char * *            i_nCurArgsBegin,
                            char * *            i_nEndOfAllArgs );
    // DATA
    udmstri             sRepositoryDirectory;
};

inline const udmstri &
Load::ReposyDir() const
    { return sRepositoryDirectory; }

struct S_ExternLinkage
{
                        S_ExternLinkage(
                            const char *        i_sNamespace,
                            const char *        i_sLinkedRootDirectory )
                            :   sNamespace(i_sNamespace),
                                sLinkedRootDirectory(i_sLinkedRootDirectory)
                            { }

    udmstri             sNamespace;
    udmstri             sLinkedRootDirectory;

};

class CreateHtml : public Command
{
  public:
                        CreateHtml();
                        ~CreateHtml();

    const udmstri &     OutputDir() const;

  private:
    // Interface Command:
    virtual char * *    do_Init(
                            char * *            i_nCurArgsBegin,
                            char * *            i_nEndOfAllArgs );
    // DATA
    udmstri             sOutputRootDirectory;

    std::vector< S_ExternLinkage >
                        aExtLinks;
};

inline const udmstri &
CreateHtml::OutputDir() const
    { return sOutputRootDirectory; }


class CreateXml : public Command
{
  public:
                        CreateXml();
                        ~CreateXml();

    const udmstri &     OutputDir() const;

  private:
    // Interface Command:
    virtual char * *    do_Init(
                            char * *            i_nCurArgsBegin,
                            char * *            i_nEndOfAllArgs );
    // DATA
    udmstri             sOutputRootDirectory;
};

inline const udmstri &
CreateXml::OutputDir() const
    { return sOutputRootDirectory; }



class Save : public Command
{
  public:
                        Save();
                        ~Save();

    const udmstri &     ReposyDir() const;

  private:
    // Interface Command:
    virtual char * *    do_Init(
                            char * *            i_nCurArgsBegin,
                            char * *            i_nEndOfAllArgs );
    // DATA
    udmstri             sRepositoryDirectory;
};

inline const udmstri &
Save::ReposyDir() const
    { return sRepositoryDirectory; }


class X_CommandLine
{
  public:
                        X_CommandLine(
                            const char *        i_sExplanation )
                            :   sExplanation(i_sExplanation) {}

    void                Report(
                            ostream &           o_rOut )
                            { o_rOut << "Error in command line: "
                                     << sExplanation << Endl(); }

  private:
    udmstri             sExplanation;
};

struct S_LanguageInfo
{
    enum E_ProgrammingLanguage
    {
        none,
        cpp,
        idl,
        corba,
        java
    };
                        S_LanguageInfo(
                            E_ProgrammingLanguage
                                                i_eLang );
    StringVector
                        aExtensions;        // An empty string is possible and means exactly that: files without extension.
    E_ProgrammingLanguage
                        eLanguage;
};

inline
S_LanguageInfo::S_LanguageInfo( E_ProgrammingLanguage i_eLang )
{
    eLanguage = i_eLang;
}

struct S_Sources
{
    StringVector        aDirectories;
    StringVector        aTrees;
    StringVector        aFiles;
};

struct S_ProjectData
{
                        S_ProjectData(
                            const char *        i_sName,
                            const char *        i_sRootDir );
                        ~S_ProjectData();

    udmstri             sName;
    csv::ploc::Path     aRootDirectory;

    Dyn<S_LanguageInfo> pLanguage;
    bool                bHtmlIsDefaultForDocs;

    S_Sources           aFiles;
};




const char * const C_opt_Verbose = "-v";

const char * const C_opt_Parse = "-parse";
const char * const C_opt_Name = "-name";
const char * const C_opt_Update = "-update";
const char * const C_opt_LangAll = "-lg";
const char * const C_opt_ExtensionsAll = "-extg";
const char * const C_opt_DocAll = "-docg";

const char * const C_arg_Usehtml = "usehtml";
const char * const C_arg_Cplusplus = "c++";
const char * const C_arg_Idl = "idl";
const char * const C_arg_Corba = "corba";
const char * const C_arg_Java = "java";

const char * const C_opt_Project = "-p";
const char * const C_opt_Lang = "-l";
const char * const C_opt_Extensions = "-ext";
const char * const C_opt_Doc = "-doc";
const char * const C_opt_SourceDir = "-d";
const char * const C_opt_SourceTree = "-t";
const char * const C_opt_SourceFile = "-f";

const char * const C_opt_Load = "-load";

const char * const C_opt_CreateHtml = "-html";
const char * const C_opt_ExternLinks = "-xlinks";

const char * const C_opt_CreateXml = "-xml";
const char * const C_opt_Save = "-save";

// IMPLEMENTATION



}   // namespace command
}   // namespace autodoc


#endif

