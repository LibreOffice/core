/*************************************************************************
 *
 *  $RCSfile: adc_cmds.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:26 $
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


#include <precomp.h>
#include "adc_cmds.hxx"


// NOT FULLY DEFINED SERVICES
#include <cosv/template/tpltools.hxx>
#include <cosv/ploc.hxx>
#include "adc_cl.hxx"



namespace autodoc
{
namespace command
{

Parse::InitMap Parse::aOptions_;



inline bool
is( char * * pArgsIterator, const char * pOption )
{
    return strcmp(*pArgsIterator, pOption) == 0;
}

#define CHECK( cond, text ) \
    if ( NOT ( cond ) ) \
        throw X_CommandLine( text )
#define CHECKOPT( cond, miss, opt ) \
    if ( NOT ( cond ) ) \
    { \
        StreamLock slMsg(100); \
        throw X_CommandLine( slMsg() << "Missing " << miss <<" after " << opt << "." << c_str ); \
    }


//**************************     Parsing     ***********************//


S_ProjectData::S_ProjectData( const char *        i_sName,
                              const char *        i_sRootDir )
    :   sName(i_sName),
        aRootDirectory(i_sRootDir,true),
        //  pLanguage,
        bHtmlIsDefaultForDocs(false)
        // aFiles
{
}

S_ProjectData::~S_ProjectData()
{
}

Parse::Parse( CommandLine & io_rCommandLine )
    :   // sRepositoryName,
        // sRepositoryDirectoryForUpdate
        // pGlobal_Language
        bGlobal_HtmlIsDefaultForDocs(false),
        // aProjects,
        pCommandLine(&io_rCommandLine)
{
}

Parse::~Parse()
{
    csv::erase_container_of_heap_ptrs(aProjects);
}

S_ProjectData &
Parse::CreateDefaultProject()
{
    if ( aProjects.size() > 0 )
        throw new X_CommandLine("Unexpected default project in command line.");

    S_ProjectData * ret = new S_ProjectData("", ".");
    aProjects.push_back(ret);
    return *ret;
}

char * *
Parse::do_Init( char * *    i_nCurArgsBegin,
                char * *    i_nEndOfAllArgs )
{
    for ( char * * it = i_nCurArgsBegin;
          it != i_nEndOfAllArgs;
          )
    {
         F_Init fi = FindFI(*it);
        if (fi != 0)
            it = (this->*fi)(it, i_nEndOfAllArgs);
    }

    return it;
}


const Parse::InitMap &
Parse::Options()
{
     if ( aOptions_.size() > 0 )
        return aOptions_;

    aOptions_[udmstri(C_opt_Parse)] = &Parse::FI_Start_ParseOptions;
    aOptions_[udmstri(C_opt_Name)] = &Parse::FI_SetName;
    aOptions_[udmstri(C_opt_Update)] = &Parse::FI_SetUpdate;
    aOptions_[udmstri(C_opt_LangAll)] = &Parse::FI_SetLanguage4All;
    aOptions_[udmstri(C_opt_ExtensionsAll)] = &Parse::FI_SetExtensions4All;
    aOptions_[udmstri(C_opt_DocAll)] = &Parse::FI_SetDocAttrs4All;

    aOptions_[udmstri(C_opt_Project)] = &Parse::FI_Start_ProjectOptions;
    aOptions_[udmstri(C_opt_Lang)] = &Parse::FI_SetLanguage;
    aOptions_[udmstri(C_opt_Extensions)] = &Parse::FI_SetExtensions;
    aOptions_[udmstri(C_opt_Doc)] = &Parse::FI_SetDocAttrs;
    aOptions_[udmstri(C_opt_SourceDir)] = &Parse::FI_SetSourceDirs;
    aOptions_[udmstri(C_opt_SourceTree)] = &Parse::FI_SetSourceTrees;
    aOptions_[udmstri(C_opt_SourceFile)] = &Parse::FI_SetSourceFiles;

    return aOptions_;
}

Parse::F_Init
Parse::FindFI( const char * i_pArg ) const
{
     InitMap::const_iterator itFound = Options().find( udmstri(i_pArg) );
    if ( itFound == Options().end() )
        return 0;

    return (*itFound).second;
}

char * *
Parse::FI_Start_ParseOptions( char * *            i_nCurArgsBegin,
                              char * *            i_nEndOfAllArgs )
{
    return i_nCurArgsBegin + 1;
}

char * *
Parse::FI_SetName( char * *            i_nCurArgsBegin,
                   char * *            i_nEndOfAllArgs )
{
    char * * ret = i_nCurArgsBegin + 1;
    CHECKOPT( **ret != '-' && ret != i_nEndOfAllArgs,
              "name", C_opt_Name );

    sRepositoryName = *ret;
    return ++ret;
}

char * *
Parse::FI_SetUpdate( char * *            i_nCurArgsBegin,
                     char * *            i_nEndOfAllArgs )
{
    char * * ret = i_nCurArgsBegin + 1;
    CHECKOPT( **ret != '-' && ret != i_nEndOfAllArgs,
              "directory",  C_opt_Update );

    sRepositoryDirectoryForUpdate = *ret;
    pCommandLine->SetUpdate(*ret);
    return ++ret;
}

char * *
Parse::FI_SetLanguage4All( char * *            i_nCurArgsBegin,
                           char * *            i_nEndOfAllArgs )
{
    char * * ret = i_nCurArgsBegin + 1;

    if ( is(ret, C_arg_Cplusplus) )
    {
        pGlobal_Language = new S_LanguageInfo(S_LanguageInfo::cpp);
        pGlobal_Language->aExtensions.push_back(udmstri("*.hxx"));
        pGlobal_Language->aExtensions.push_back(udmstri("*.h"));
    }
    else if (is(ret, C_arg_Idl) )
    {
        pGlobal_Language = new S_LanguageInfo(S_LanguageInfo::idl);
        pGlobal_Language->aExtensions.push_back(udmstri("*.idl"));
    }
    else if (is(ret, C_arg_Corba) )
    {
        pGlobal_Language = new S_LanguageInfo(S_LanguageInfo::corba);
        pGlobal_Language->aExtensions.push_back(udmstri("*.idl"));
    }
    else if (is(ret, C_arg_Java) )
    {
        pGlobal_Language = new S_LanguageInfo(S_LanguageInfo::java);
        pGlobal_Language->aExtensions.push_back(udmstri("*.java"));
    }
    else
    {
        StreamLock slMsg(100);
        throw X_CommandLine(
            slMsg() << "Missing language after " << C_opt_LangAll << "." << c_str );
    }

    return ++ret;
}

char * *
Parse::FI_SetExtensions4All( char * *            i_nCurArgsBegin,
                             char * *            i_nEndOfAllArgs )
{
    char * * ret = i_nCurArgsBegin + 1;
    CHECKOPT( ret != i_nEndOfAllArgs && **ret == '.',
              "extension(s)",  C_opt_ExtensionsAll );
    StreamLock slCheck(150);
    slCheck() << C_opt_ExtensionsAll
              << " used without previous "
              << C_opt_LangAll;

    CHECK( pGlobal_Language != 0,
           slCheck().c_str() );

    do {
        pGlobal_Language->aExtensions.push_back(udmstri(*ret));
        ++ret;
    }   while ( ret != i_nEndOfAllArgs && **ret == '.' );

    return ret;
}

char * *
Parse::FI_SetDocAttrs4All( char * *            i_nCurArgsBegin,
                           char * *            i_nEndOfAllArgs )
{
    char * * ret = i_nCurArgsBegin + 1;
    CHECKOPT( ret != i_nEndOfAllArgs && **ret != '-',
              "attribute", C_opt_DocAll );

    if ( is(ret, C_arg_Usehtml) )
    {
        bGlobal_HtmlIsDefaultForDocs = true;
        ++ret;
    }
    else
    {
        throw X_CommandLine(
            StreamLock(100)() << "Unknown attribute after "
                              << C_opt_DocAll
                              << "."
                              << c_str );
    }

    return ret;
}

char * *
Parse::FI_Start_ProjectOptions( char * *            i_nCurArgsBegin,
                                char * *            i_nEndOfAllArgs )
{
    if ( aProjects.size() == 1 )
    {
         if ( aProjects[0]->sName.length() == 0 )
            throw X_CommandLine( "Both, named projects and a default project, cannot be used together." );
    }

    S_ProjectData * pProject = 0;

    char * * ret = i_nCurArgsBegin + 1;
    CHECKOPT( ret != i_nEndOfAllArgs && **ret != '-',
              "name",  C_opt_Project );
    ++ret;
    CHECKOPT( ret != i_nEndOfAllArgs && **ret != '-',
              "root directory",  C_opt_Project );
    ++ret;

    pProject = new S_ProjectData( i_nCurArgsBegin[1], i_nCurArgsBegin[2] );
    pCommandLine->SetCurProject(*pProject);
    aProjects.push_back(pProject);

    return ret;
}

char * *
Parse::FI_SetLanguage( char * *            i_nCurArgsBegin,
                       char * *            i_nEndOfAllArgs )
{
    char * * ret = i_nCurArgsBegin + 1;

    if ( is(ret, C_arg_Cplusplus) )
        pCommandLine->CurProject().pLanguage
            = new S_LanguageInfo(S_LanguageInfo::cpp);
    else if (is(ret, C_arg_Idl) )
        pCommandLine->CurProject().pLanguage
            = new S_LanguageInfo(S_LanguageInfo::idl);
    else if (is(ret, C_arg_Corba) )
        pCommandLine->CurProject().pLanguage
            = new S_LanguageInfo(S_LanguageInfo::corba);
    else if (is(ret, C_arg_Java) )
        pCommandLine->CurProject().pLanguage
            = new S_LanguageInfo(S_LanguageInfo::java);
    else
        throw X_CommandLine(
            StreamLock(100)() << "Missing language after "
                              << C_opt_Lang
                              << "."
                              << c_str );

    return ++ret;
}

char * *
Parse::FI_SetExtensions( char * *            i_nCurArgsBegin,
                         char * *            i_nEndOfAllArgs )
{
    char * * ret = i_nCurArgsBegin + 1;

    CHECKOPT( ret != i_nEndOfAllArgs && **ret == '.',
              "extension(s)",  C_opt_Extensions );
    CHECK( pCommandLine->CurProject().pLanguage != 0,
           StreamLock(100)() << C_opt_Extensions
                             << " used without previous "
                             << C_opt_Lang
                             << c_str );

    do {
        pCommandLine->CurProject().pLanguage
            ->aExtensions.push_back(udmstri(*ret));
        ++ret;
    }   while ( ret != i_nEndOfAllArgs && **ret == '.' );

    return ret;
}

char * *
Parse::FI_SetDocAttrs( char * *            i_nCurArgsBegin,
                       char * *            i_nEndOfAllArgs )
{
    char * * ret = i_nCurArgsBegin + 1;

    CHECKOPT( ret != i_nEndOfAllArgs && **ret != '-',
              "attribute", C_opt_Doc );

    if ( is(ret, C_arg_Usehtml) )
    {
        pCommandLine->CurProject().bHtmlIsDefaultForDocs = true;
        ++ret;
    }
    else
    {
        throw X_CommandLine(
            StreamLock(100)() << "Unknown attribute after "
                              << C_opt_Doc
                              << "."
                              << c_str );
    }

    return ret;
}

char * *
Parse::FI_SetSourceDirs( char * *            i_nCurArgsBegin,
                         char * *            i_nEndOfAllArgs )
{
    char * * ret = i_nCurArgsBegin + 1;
    CHECKOPT( ret != i_nEndOfAllArgs && **ret != '-',
              "directory", C_opt_SourceDir );
    do {
        pCommandLine->CurProject().aFiles.
            aDirectories.push_back( udmstri(*ret) );
        ++ret;
    } while (ret != i_nEndOfAllArgs && **ret != '-');

    return ret;
}

char * *
Parse::FI_SetSourceTrees( char * *            i_nCurArgsBegin,
                          char * *            i_nEndOfAllArgs )
{
    char * * ret = i_nCurArgsBegin + 1;
    CHECKOPT( ret != i_nEndOfAllArgs && **ret != '-',
              "directory", C_opt_SourceTree );
    do {
        pCommandLine->CurProject().aFiles.
            aTrees.push_back( udmstri(*ret) );
        ++ret;
    } while (ret != i_nEndOfAllArgs && **ret != '-');

    return ret;
}

char * *
Parse::FI_SetSourceFiles( char * *            i_nCurArgsBegin,
                          char * *            i_nEndOfAllArgs )
{
    char * * ret = i_nCurArgsBegin + 1;
    CHECKOPT( ret != i_nEndOfAllArgs && **ret != '-',
              "file", C_opt_SourceFile );       // KORR_FUTURE
                                                // What about files, which start with '-'?
    do {
        pCommandLine->CurProject().aFiles.
            aFiles.push_back( udmstri(*ret) );
        ++ret;
    } while (ret != i_nEndOfAllArgs && **ret != '-');   // KORR_FUTURE
                                                        // What about files, which start with '-'?
    return ret;
}

//**************************        Load        ***********************//

Load::Load( const char * i_sRepositoryDirectory )
    :   sRepositoryDirectory(i_sRepositoryDirectory)
{
}

Load::~Load()
{
}

char * *
Load::do_Init( char * *            i_nCurArgsBegin,
               char * *            i_nEndOfAllArgs )
{
    char * * ret = i_nCurArgsBegin + 1;
    CHECKOPT( ret != i_nEndOfAllArgs AND **ret != '-',
              "directory",  C_opt_Load );

    sRepositoryDirectory = *ret;
    return ++ret;
}



//**************************        CreateHTML    ***********************//

CreateHtml::CreateHtml()
//  :   // sOutputRootDirectory,
        // aExtLinks
{
}

CreateHtml::~CreateHtml()
{
}

char * *
CreateHtml::do_Init( char * *            i_nCurArgsBegin,
                     char * *            i_nEndOfAllArgs )
{
    char * * ret = i_nCurArgsBegin + 1;
    CHECKOPT( ret != i_nEndOfAllArgs && **ret != '-',
              "output directory", C_opt_CreateHtml );
    sOutputRootDirectory = *ret;

    if ( is(++ret, C_opt_ExternLinks) )
    {
        CHECKOPT( ret+1 != i_nEndOfAllArgs && **(ret+1) != '-',
                  "namespace", C_opt_ExternLinks );
        CHECKOPT( ret+2 != i_nEndOfAllArgs && **(ret+2) != '-',
                  "linked root directory", C_opt_ExternLinks );
        do {
            aExtLinks.push_back( S_ExternLinkage(*(ret+1), *(ret+2)) );
            ret += 3;
        } while ( is(ret, C_opt_ExternLinks) );
    }

    return ret;
}


//**************************        Out XML     ***********************//

CreateXml::CreateXml()
//  :   // sOutputRootDirectory
{
}

CreateXml::~CreateXml()
{
}

char * *
CreateXml::do_Init( char * *            i_nCurArgsBegin,
                    char * *            i_nEndOfAllArgs )
{
    char * * ret = i_nCurArgsBegin + 1;
    CHECKOPT( ret != i_nEndOfAllArgs && **ret != '-',
              "output directory", C_opt_CreateXml );

    sOutputRootDirectory = *ret;
    return ++ret;
}


//**************************        Save        ***********************//

Save::Save()
//    :   // sRepositoryDirectory,
{
}

Save::~Save()
{
}

char * *
Save::do_Init( char * *            i_nCurArgsBegin,
               char * *            i_nEndOfAllArgs )
{
    char * * ret = i_nCurArgsBegin + 1;
    CHECKOPT( ret != i_nEndOfAllArgs AND **ret != '-',
              "directory",  C_opt_Save );

    sRepositoryDirectory = *ret;
    return ++ret;
}


}   // namespace command
}   // namespace autodoc



