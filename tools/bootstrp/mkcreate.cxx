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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"

// global includes
#include <stdio.h>

// local includes
#include "bootstrp/mkcreate.hxx"
#include "bootstrp/inimgr.hxx"
#include "bootstrp/appdef.hxx"
#include <tools/geninfo.hxx>
#include <tools/iparser.hxx>
#include "bootstrp/prj.hxx"

char const *NoBuildProject[] = {
    "solenv",
    "EndOf_NoBuildProject"
};

char const *LimitedPath[] = {
    "jurt\\com\\sun\\star",
    "r_tools",
    "ridljar",
    "setup2",
    "connectivity",
    "EndOf_LimitedPath"
};

//
// class SourceDirectory
//

/*****************************************************************************/
SourceDirectory::SourceDirectory( const ByteString &rDirectoryName,
    sal_uInt16 nOperatingSystem, SourceDirectory *pParentDirectory )
/*****************************************************************************/
                : ByteString( rDirectoryName ),
                pParent( pParentDirectory ),
                pSubDirectories( NULL ),
                nOSType( nOperatingSystem ),
                nDepth( 0 ),
                pDependencies( NULL ),
                pCodedDependencies( NULL ),
                pCodedIdentifier( NULL )
{
    if ( pParent ) {
        if ( !pParent->pSubDirectories )
            pParent->pSubDirectories = new SourceDirectoryList();
        pParent->pSubDirectories->InsertSorted( this );
        nDepth = pParent->nDepth + 1;
    }
}

/*****************************************************************************/
SourceDirectory::~SourceDirectory()
/*****************************************************************************/
{
    delete pSubDirectories;
}

/*****************************************************************************/
CodedDependency *SourceDirectory::AddCodedDependency(
    const ByteString &rCodedIdentifier, sal_uInt16 nOperatingSystems )
/*****************************************************************************/
{
    CodedDependency *pReturn = NULL;

    if ( !pCodedDependencies ) {
        pCodedDependencies = new SByteStringList();
        pReturn = new CodedDependency( rCodedIdentifier, nOperatingSystems );
        pCodedDependencies->PutString(( ByteString * ) pReturn );
    }
    else {
        size_t nPos = pCodedDependencies->IsString( (ByteString*)(&rCodedIdentifier) );
        if ( nPos == NOT_THERE ) {
            pReturn =
                new CodedDependency( rCodedIdentifier, nOperatingSystems );
            pCodedDependencies->PutString(( ByteString * ) pReturn );
        }
        else {
            pReturn = ( CodedDependency * ) (*pCodedDependencies)[ nPos ];
            pReturn->TryToMerge( rCodedIdentifier, nOperatingSystems );
        }
    }
    return pReturn;
}

/*****************************************************************************/
CodedDependency *SourceDirectory::AddCodedIdentifier(
    const ByteString &rCodedIdentifier, sal_uInt16 nOperatingSystems )
/*****************************************************************************/
{
    CodedDependency *pReturn = NULL;

    if ( !pCodedIdentifier ) {
        pCodedIdentifier = new SByteStringList();
        pReturn = new CodedDependency( rCodedIdentifier, nOperatingSystems );
        pCodedIdentifier->PutString(( ByteString * ) pReturn );
    }
    else {
        size_t nPos = pCodedIdentifier->IsString( ( ByteString*)(&rCodedIdentifier) );
        if ( nPos == NOT_THERE ) {
            pReturn =
                new CodedDependency( rCodedIdentifier, nOperatingSystems );
            pCodedIdentifier->PutString(( ByteString * ) pReturn );
        }
        else {
            pReturn = ( CodedDependency * ) (*pCodedIdentifier)[ nPos ];
            pReturn->TryToMerge( rCodedIdentifier, nOperatingSystems );
        }
    }
    if ( pParent && pParent->nDepth > 1 )
        pParent->AddCodedIdentifier( rCodedIdentifier, nOperatingSystems );

    return pReturn;
}

/*****************************************************************************/
ByteString SourceDirectory::GetFullPath()
/*****************************************************************************/
{
    ByteString sFullPath;
    if ( pParent ) {
        sFullPath = pParent->GetFullPath();
        sFullPath += ByteString( PATH_SEPARATOR );
    }
    sFullPath += *this;

    return sFullPath;
}

/*****************************************************************************/
SourceDirectory *SourceDirectory::GetRootDirectory()
/*****************************************************************************/
{
    if ( !pParent )
        return this;

    return pParent->GetRootDirectory();
}

/*****************************************************************************/
SourceDirectory *SourceDirectory::GetSubDirectory(
    const ByteString &rDirectoryPath, sal_uInt16 nOperatingSystem )
/*****************************************************************************/
{
    ByteString sSearch;

    sal_Bool bSubs = sal_True;
    sal_uIntPtr nIndex = 0;

    while ( bSubs && ByteString( LimitedPath[ nIndex ]) != "EndOf_LimitedPath" ) {
        SourceDirectory *pActDir = this;
        ByteString sLimitation( LimitedPath[ nIndex ]);

        sal_Bool bBreak = sal_False;
        for ( sal_uIntPtr i = sLimitation.GetTokenCount( '\\' ); i > 0 && !bBreak; i-- ) {
            if (( !pActDir ) || ( *pActDir != sLimitation.GetToken(( sal_uInt16 )( i - 1 ), '\\' )))
                bBreak = sal_True;
            else
                pActDir = pActDir->pParent;
        }
        bSubs = bBreak;
        nIndex++;
    }

    if ( !bSubs )
    {
        sSearch = rDirectoryPath;
    }
    else
        sSearch = rDirectoryPath.GetToken( 0, PATH_SEPARATOR );

    SourceDirectory *pSubDirectory = NULL;

    if ( pSubDirectories )
        pSubDirectory = pSubDirectories->Search( sSearch );

    if ( !pSubDirectory )
        pSubDirectory = new SourceDirectory(
            sSearch, nOperatingSystem, this );

    pSubDirectory->nOSType |= nOperatingSystem;

    if ( sSearch.Len() == rDirectoryPath.Len())
        return pSubDirectory;

    ByteString sPath = rDirectoryPath.Copy( sSearch.Len() + 1 );

    return pSubDirectory->GetSubDirectory( sPath, nOperatingSystem );
}

/*****************************************************************************/
SourceDirectory *SourceDirectory::GetDirectory(
    const ByteString &rDirectoryName, sal_uInt16 nOperatingSystem )
/*****************************************************************************/
{
    ByteString sDirectoryName( rDirectoryName );
#ifdef UNX
    sDirectoryName.SearchAndReplaceAll( "\\", "/" );
#endif

    SourceDirectory *pRoot = GetRootDirectory();

    if ( sDirectoryName.Search( *pRoot ) != 0 )
        return NULL;

    if ( sDirectoryName.Len() == pRoot->Len())
        return pRoot;

    if ( sDirectoryName.GetChar( pRoot->Len()) == PATH_SEPARATOR ) {
        ByteString sSub = sDirectoryName.Copy( pRoot->Len() + 1 );
        return pRoot->GetSubDirectory( sSub, nOperatingSystem );
    }

    return NULL;
}

/*****************************************************************************/
SourceDirectory *SourceDirectory::Insert( const ByteString &rDirectoryName,
        sal_uInt16 nOperatingSystem )
/*****************************************************************************/
{
    SourceDirectory *pSubDirectory = NULL;
    if ( pSubDirectories )
        pSubDirectory = pSubDirectories->Search( rDirectoryName );

    if ( !pSubDirectory )
        pSubDirectory = new SourceDirectory(
            rDirectoryName, nOperatingSystem, this );

    return pSubDirectory;
}

/*****************************************************************************/
Dependency *SourceDirectory::ResolvesDependency(
    CodedDependency *pCodedDependency )
/*****************************************************************************/
{
    if ( !pCodedIdentifier )
        return NULL;

    size_t nPos = pCodedIdentifier->IsString( pCodedDependency );
    if ( nPos != NOT_THERE ) {
        CodedDependency *pIdentifier = ( CodedDependency* )(*pCodedIdentifier)[ nPos ];
        sal_uInt16 nResult =
            pIdentifier->GetOperatingSystem() &
            pCodedDependency->GetOperatingSystem();
        Dependency *pReturn = new Dependency( *this, nResult );
        nResult ^= pCodedDependency->GetOperatingSystem();
        pCodedDependency->SetOperatingSystem( nResult );
        return pReturn;
    }
    return NULL;
}


/*****************************************************************************/
void SourceDirectory::ResolveDependencies()
/*****************************************************************************/
{
    if ( !pSubDirectories )
        return;

    for ( size_t i = 0; i < pSubDirectories->size(); i++ ) {
        SourceDirectory *pActDirectory = (SourceDirectory*)(*pSubDirectories)[ i ];
        if ( pActDirectory->pSubDirectories )
            pActDirectory->ResolveDependencies();

        if ( pActDirectory->pCodedDependencies ) {
            while ( pActDirectory->pCodedDependencies->size() )
            {
                CodedDependency *pCodedDependency = ( CodedDependency * )
                    (*pActDirectory->pCodedDependencies)[ 0 ];

                for ( size_t k = 0;
                         ( k < pSubDirectories->size() )
                      && ( pCodedDependency->GetOperatingSystem() != OS_NONE );
                      k++
                ) {
                    Dependency *pDependency =
                        ((SourceDirectory *)(*pSubDirectories)[ k ])->
                        ResolvesDependency( pCodedDependency );
                    if ( pDependency )
                    {
                        if ( !pActDirectory->pDependencies )
                            pActDirectory->pDependencies = new SByteStringList();
                        pActDirectory->pDependencies->PutString( pDependency );
                    }
                }
                if ( pCodedDependency->GetOperatingSystem()) {
                    if ( !pCodedDependencies )
                        pCodedDependencies = new SByteStringList();
                    pCodedDependencies->PutString( pCodedDependency );
                }
                else
                    delete pCodedDependency;
                pActDirectory->pCodedDependencies->erase( 0 );
            }
        }
    }
}

/*****************************************************************************/
ByteString SourceDirectory::GetTarget()
/*****************************************************************************/
{
    ByteString sReturn;

    if ( !pDependencies )
        return sReturn;

    size_t k = 0;
    while ( k < pDependencies->size()) {
        if ( *this == *(*pDependencies)[ k ] )
            delete pDependencies->erase( k );
        else
            k++;
    }

    if ( !pDependencies->size() ) {
        delete pDependencies;
        pDependencies = NULL;
        return sReturn;
    }

    sal_Bool bDependsOnPlatform = sal_False;
    for ( size_t i = 0; i < pDependencies->size(); i++ ) {
        if ( ((Dependency*)(*pDependencies)[ i ])->GetOperatingSystem() != OS_ALL ) {
            bDependsOnPlatform = sal_True;
            break;
        }
    }

    ByteString sTarget( *this );
    sTarget.SearchAndReplaceAll( "\\", "$/" );
    if ( !bDependsOnPlatform ) {
        sReturn = sTarget;
        sReturn += " :";
        for ( size_t i = 0; i < pDependencies->size(); i++ ) {
            ByteString sDependency( *(*pDependencies)[ i ] );
            sDependency.SearchAndReplaceAll( "\\", "$/" );
            sReturn += " ";
            sReturn += sDependency;
        }
    }
    else {
        ByteString sUNX( ".IF \"$(GUI)\" == \"UNX\"\n" );
        sUNX += sTarget;
        sUNX += " :";
        sal_Bool bUNX = sal_False;

        ByteString sWNT( ".IF \"$(GUI)\" == \"WNT\"\n" );
        sWNT += sTarget;
        sWNT += " :";
        sal_Bool bWNT = sal_False;

        ByteString sOS2( ".IF \"$(GUI)\" == \"OS2\"\n" );
        sOS2 += sTarget;
        sOS2 += " :";
        sal_Bool bOS2 = sal_False;

        for ( size_t i = 0; i < pDependencies->size(); i++ ) {
            Dependency *pDependency = (Dependency*)(*pDependencies)[ i ];
            ByteString sDependency( *pDependency );
            sDependency.SearchAndReplaceAll( "\\", "$/" );

            if ( pDependency->GetOperatingSystem() & OS_UNX ) {
                sUNX += " ";
                sUNX += sDependency;
                bUNX = sal_True;
            }
            if ( pDependency->GetOperatingSystem() & OS_WIN32 ) {
                sWNT += " ";
                sWNT += sDependency;
                bWNT = sal_True;
            }
            if ( pDependency->GetOperatingSystem() & OS_OS2 ) {
                sOS2 += " ";
                sOS2 += sDependency;
                bOS2 = sal_True;
            }
        }

        if ( bUNX ) {
            sReturn += sUNX;
            sReturn += "\n.ENDIF\n";
        }
        if ( bWNT ) {
            sReturn += sWNT;
            sReturn += "\n.ENDIF\n";
        }
        if ( bOS2 ) {
            sReturn += sOS2;
            sReturn += "\n.ENDIF\n";
        }
    }
    sReturn.EraseTrailingChars( '\n' );
    return sReturn;
}

/*****************************************************************************/
ByteString SourceDirectory::GetSubDirsTarget()
/*****************************************************************************/
{
    ByteString sReturn;

    if ( pSubDirectories ) {
        sal_Bool bDependsOnPlatform = sal_False;
        for ( size_t i = 0; i < pSubDirectories->size(); i++ ) {
            if ( ((SourceDirectory*)(*pSubDirectories)[ i ])->GetOperatingSystems() != OS_ALL ) {
                bDependsOnPlatform = sal_True;
            }
        }

        if ( !bDependsOnPlatform ) {
            sReturn = "RC_SUBDIRS = ";

            for ( size_t i = 0; i < pSubDirectories->size(); i++ ) {
                ByteString sSubDirectory( *(*pSubDirectories)[ i ] );
                sSubDirectory.SearchAndReplaceAll( "\\", "$/" );
                sReturn += " \\\n\t";
                sReturn += sSubDirectory;
            }
            sReturn += "\n";
        }
        else {
            ByteString sUNX( ".IF \"$(GUI)\" == \"UNX\"\n" );
            sUNX += "RC_SUBDIRS = ";
            sal_Bool bUNX = sal_False;

            ByteString sWNT( ".IF \"$(GUI)\" == \"WNT\"\n" );
            sWNT += "RC_SUBDIRS = ";
            sal_Bool bWNT = sal_False;

            ByteString sOS2( ".IF \"$(GUI)\" == \"OS2\"\n" );
            sOS2 += "RC_SUBDIRS = ";
            sal_Bool bOS2 = sal_False;

            for ( size_t i = 0; i < pSubDirectories->size(); i++ ) {
                SourceDirectory *pDirectory = (SourceDirectory*)(*pSubDirectories)[ i ];
                ByteString sDirectory( *pDirectory );
                sDirectory.SearchAndReplaceAll( "\\", "$/" );

                if ( pDirectory->GetOperatingSystems() & OS_UNX ) {
                    sUNX += " \\\n\t";
                    sUNX += sDirectory;
                    bUNX = sal_True;
                }
                if ( pDirectory->GetOperatingSystems() & OS_WIN32 ) {
                    sWNT += " \\\n\t";
                    sWNT += sDirectory;
                    bWNT = sal_True;
                }
                if ( pDirectory->GetOperatingSystems() & OS_OS2 ) {
                    sOS2 += " \\\n\t";
                    sOS2 += sDirectory;
                    bOS2 = sal_True;
                }
            }
            if ( bUNX ) {
                sReturn += sUNX;
                sReturn += "\n.ENDIF\n";
            }
            if ( bWNT ) {
                sReturn += sWNT;
                sReturn += "\n.ENDIF\n";
            }
            if ( bOS2 ) {
                sReturn += sOS2;
                sReturn += "\n.ENDIF\n";
            }
        }
    }
    return sReturn;
}

/*****************************************************************************/
sal_uInt16 SourceDirectory::GetOSType( const ByteString &sDependExt )
/*****************************************************************************/
{
    sal_uInt16 nOSType = 0;
    if ( sDependExt == "" )
        nOSType |= OS_ALL;
    else if ( sDependExt == "N" || sDependExt == "W" )
        nOSType |= OS_WIN32;
    else if ( sDependExt == "U" )
        nOSType |= OS_UNX;
    else if ( sDependExt == "P" )
        nOSType |= OS_OS2;
    return nOSType;
}

/*****************************************************************************/
SourceDirectory *SourceDirectory::CreateRootDirectory(
    const ByteString &rRoot, const ByteString &rVersion, sal_Bool bAll )
/*****************************************************************************/
{
    IniManager aIniManager;
    aIniManager.Update();

    ByteString sDefLst( GetDefStandList());
    ByteString sStandLst( aIniManager.ToLocal( sDefLst ));
    String s = String( sStandLst, gsl_getSystemTextEncoding());
    InformationParser aParser;
    GenericInformationList *pVerList = aParser.Execute( s );

    ByteString sVersion( rVersion );
    Star aStar( pVerList, sVersion, sal_True, rRoot.GetBuffer());

    SourceDirectory *pSourceRoot = new SourceDirectory( rRoot, OS_ALL );

    for ( sal_uIntPtr i = 0; i < aStar.Count(); i++ ) {
        Prj *pPrj = aStar.GetObject( i );

        sal_Bool bBuildable = sal_True;
        sal_uIntPtr nIndex = 0;

        while ( bBuildable && ByteString( NoBuildProject[ nIndex ]) != "EndOf_NoBuildProject" ) {
            bBuildable = ( ByteString( NoBuildProject[ nIndex ]) !=  pPrj->GetProjectName());
            nIndex ++;
        }

        if ( bBuildable ) {
            SourceDirectory *pProject = pSourceRoot->Insert( pPrj->GetProjectName(), OS_ALL );

            SByteStringList *pPrjDependencies = pPrj->GetDependencies( sal_False );
            if ( pPrjDependencies )
                for ( size_t x = 0; x < pPrjDependencies->size(); x++ )
                    pProject->AddCodedDependency( *(*pPrjDependencies)[ x ], OS_ALL );

            pProject->AddCodedIdentifier( pPrj->GetProjectName(), OS_ALL );

            for ( sal_uIntPtr j = 0; j < pPrj->Count(); j++ ) {
                CommandData *pData = pPrj->GetObject( j );
                if ( bAll || ( pData->GetCommandType() == COMMAND_NMAKE )) {
                    ByteString sDirPath( rRoot );
                    sDirPath += ByteString( PATH_SEPARATOR );
                    sDirPath += pData->GetPath();
                    SourceDirectory *pDirectory =
                        pSourceRoot->InsertFull( sDirPath, pData->GetOSType());
                    SByteStringList *pDependencies = pData->GetDependencies();
                    if ( pDependencies ) {
                        for ( size_t k = 0; k < pDependencies->size(); k++ ) {
                            ByteString sDependency( *(*pDependencies)[ k ] );
                            ByteString sDependExt(sDependency.GetToken( 1, '.' ));
                            sDependExt.ToUpperAscii();
                            pDirectory->AddCodedDependency(
                                sDependency.GetToken( 0, '.' ), GetOSType( sDependExt ));
                        }
                    }
                    ByteString sIdentifier = pData->GetLogFile();
                    ByteString sIdExt = sIdentifier.GetToken( 1, '.' );
                    sIdExt.ToUpperAscii();
                    pDirectory->AddCodedIdentifier( sIdentifier.GetToken( 0, '.' ), GetOSType( sIdExt ));
                }
            }
        }
    }
    delete pVerList;
    return pSourceRoot;
}

/*****************************************************************************/
sal_Bool SourceDirectory::RemoveDirectoryTreeAndAllDependencies()
/*****************************************************************************/
{
    if ( !pParent )
        return sal_False;

    SourceDirectoryList *pParentContent = pParent->pSubDirectories;
    size_t i = 0;
    while ( i < pParentContent->size() ) {
        SourceDirectory *pCandidate = (SourceDirectory*)(*pParentContent)[ i ];
        if ( pCandidate == this ) {
            pParentContent->erase( i );
        }
        else {
            if ( pCandidate->pDependencies ) {
                size_t nPos = pCandidate->pDependencies->IsString( this );
                if ( nPos != NOT_THERE )
                    delete pCandidate->pDependencies->erase( nPos );
            }
            i++;
        }
    }
    delete this;
    return sal_True;
}

/*****************************************************************************/
sal_Bool SourceDirectory::CreateRecursiveMakefile( sal_Bool bAllChilds )
/*****************************************************************************/
{
    if ( !pSubDirectories )
        return sal_True;

    fprintf( stdout, "%s", GetFullPath().GetBuffer());

    String aTmpStr( GetFullPath(), gsl_getSystemTextEncoding());
    DirEntry aEntry( aTmpStr );
    if ( !aEntry.Exists()) {
        fprintf( stdout, " ... no directory!\n" );
        return sal_False;
    }

    size_t j = 0;
    while( j < pSubDirectories->size()) {
        String sSubDirectory(
            ((SourceDirectory*)(*pSubDirectories)[ j ])->GetFullPath(),
            gsl_getSystemTextEncoding()
        );
        DirEntry aSubDirectory( sSubDirectory );
        if ( !aSubDirectory.Exists() )
            ((SourceDirectory*)(*pSubDirectories)[ j ])->RemoveDirectoryTreeAndAllDependencies();
        else
            j++;
    }

    DirEntry aRCFile( String( "makefile.rc", gsl_getSystemTextEncoding()));
    DirEntry aRCEntry( aEntry );
    aRCEntry += aRCFile;

    DirEntry aMKFile( String( "makefile.mk", gsl_getSystemTextEncoding()));
    DirEntry aMKEntry( aEntry );
    aMKEntry += aMKFile;

    sal_Bool bMakefileMk = sal_False;
    if ( aMKEntry.Exists()) {
        if ( nDepth == 1 && *this == ByteString( "api" ))
            fprintf( stdout, " ... makefile.mk exists, ignoring (hack: prj == api)!" );
        else {
            fprintf( stdout, " ... makefile.mk exists, including!" );
            bMakefileMk = sal_True;
        }
    }

    SvFileStream aMakefile( aRCEntry.GetFull(), STREAM_STD_WRITE | STREAM_TRUNC );
    if ( !aMakefile.IsOpen()) {
        fprintf( stdout, " ... failed!\n" );
        return sal_False;
    }

    ByteString sHeader(
        "#*************************************************************************\n"
        "#\n"
        "# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.\n"
        "#\n"
        "# Copyright 2000, 2010 Oracle and/or its affiliates.\n"
        "#\n"
        "# OpenOffice.org - a multi-platform office productivity suite\n"
        "#\n"
        "# This file is part of OpenOffice.org.\n"
        "#\n"
        "# OpenOffice.org is free software: you can redistribute it and/or modify\n"
        "# it under the terms of the GNU Lesser General Public License version 3\n"
        "# only, as published by the Free Software Foundation.\n"
        "#\n"
        "# OpenOffice.org is distributed in the hope that it will be useful,\n"
        "# but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
        "# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
        "# GNU Lesser General Public License version 3 for more details\n"
        "# (a copy is included in the LICENSE file that accompanied this code).\n"
        "#\n"
        "# You should have received a copy of the GNU Lesser General Public License\n"
        "# version 3 along with OpenOffice.org.  If not, see\n"
        "# <http://www.openoffice.org/license.html>\n"
        "# for a copy of the LGPLv3 License.\n"
        "#\n"
        "#*************************************************************************\n"
        "\n"
    );
    if ( !bMakefileMk ) {
        if ( nDepth == 0 ) {
             sHeader += ByteString(
                "\n"
                "# \n"
                "# mark this makefile as a recursive one\n"
                "# \n"
                "\n"
                "MAKEFILERC=yes\n"
                "\n"
                "# \n"
                "# implementation of cvs checkout\n"
                "# \n"
                "\n"
                ".IF \"$(checkout)\"==\"\"\n"
                "all_target: ALLTAR\n"
                ".ELSE\t# \"$(checkout)\"==\"\"\n"
                ".IF \"$(checkout)\"==\"true\"\n"
                "% : $(NULL)\n"
                "\t_cvs co $@\n"
                ".ELSE\t# \"$(checkout)\"==\"true\"\n"
                "% : $(NULL)\n"
                "\t_cvs co -r$(checkout) $@\n"
                ".ENDIF\t# \"$(checkout)\"==\"true\"\n"
                "all_subdirs : $(RC_SUBDIRS)\n"
                ".ENDIF\t# \"$(checkout)\"==\"\"\n"
            );
        }
        else {
             sHeader += ByteString(
                "\n"
                "# \n"
                "# mark this makefile as a recursive one\n"
                "# \n"
                "\n"
                "MAKEFILERC=yes\n"
            );
            if ( nDepth == 1 )
                sHeader += ByteString(
                    ".IF \"$(build_deliver)\"==\"true\"\n"
                    "all_target:\t\t\\\n"
                    "\tTG_DELIVER\t\\\n"
                    "\tALLTAR\n"
                    ".ELSE # \"$(build_deliver)\"==\"true\"\n"
                    "all_target: ALLTAR\n"
                    ".ENDIF # \"$(build_deliver)\"==\"true\"\n"
                );
            else
                sHeader += ByteString(
                    "all_target: ALLTAR\n"
                );
        }
    }
    else {
        if ( nDepth == 1 )
            sHeader += ByteString(
                ".IF \"$(build_deliver)\"==\"true\"\n"
                "all_target:\t\t\\\n"
                "\tTG_DELIVER\t\\\n"
                "\tALLTAR\n"
                ".ELSE # \"$(build_deliver)\"==\"true\"\n"
                "all_target: ALLTAR\n"
                ".ENDIF # \"$(build_deliver)\"==\"true\"\n"
            );
    }
    sHeader += ByteString(
        "\n"
        "# \n"
        "# macro RC_SUBDIRS handles iteration over\n"
        "# all mandatory sub directories\n"
        "# \n"
    );

    aMakefile.WriteLine( sHeader );
    aMakefile.WriteLine( GetSubDirsTarget());

    if ( nDepth == 0 ) {
        ByteString sBootstrapTarget(
            "# \n"
            "# bootstrap target\n"
            "# \n\n"
            "bootstrap .PHONY :\n"
            "\t@config_office/bootstrap\n\n"
        );
        aMakefile.WriteLine( sBootstrapTarget );
        ByteString sConfigureTarget(
            "# \n"
            "# configure target\n"
            "# \n\n"
            "configure .PHONY SETDIR=config_office :\n"
            "\t@configure\n"
        );
        aMakefile.WriteLine( sConfigureTarget );
    }
    else if ( nDepth == 1 ) {
        ByteString sDeliverTarget(
            "# \n"
            "# deliver target to handle\n"
            "# project dependencies\n"
            "# \n\n"
            "TG_DELIVER : $(RC_SUBDIRS)\n"
            "\t$(DELIVER)\n"
        );
        aMakefile.WriteLine( sDeliverTarget );
    }

    if ( bMakefileMk ) {
        ByteString sInclude(
            "# \n"
            "# local makefile\n"
            "# \n"
            "\n"
            ".INCLUDE : makefile.mk\n"
        );

        if ( nDepth != 1 )
            sInclude += ByteString(
                "\n"
                "all_rc_target: ALLTAR\n"
            );

        aMakefile.WriteLine( sInclude );
    }

    ByteString sComment(
        "# \n"
        "# single directory targets for\n"
        "# dependency handling between directories\n"
        "# \n"
    );
    aMakefile.WriteLine( sComment );

    for ( size_t i = 0; i < pSubDirectories->size(); i++ ) {
        ByteString sTarget( ((SourceDirectory*)(*pSubDirectories)[ i ])->GetTarget() );
        if ( sTarget.Len())
            aMakefile.WriteLine( sTarget );
    }

    ByteString sFooter(
        "\n"
    );
    if ( !bMakefileMk ) {
        sFooter += ByteString(
            "# \n"
            "# central target makefile\n"
            "# \n"
            "\n"
        );
        if ( nDepth != 0 ) {
            sFooter += ByteString(
                ".INCLUDE : target.mk\n"
            );
        }
        else {
            sFooter += ByteString(
                ".IF \"$(checkout)\"==\"\"\n"
                ".INCLUDE : target.mk\n"
                ".ENDIF\t#\"$(checkout)\"==\"\"\n"
            );
        }
    }
    sFooter += ByteString(
        "\n"
        "#*************************************************************************\n"
    );
    aMakefile.WriteLine( sFooter );

    aMakefile.Close();

    fprintf( stdout, "\n" );

    sal_Bool bSuccess = sal_True;
    if ( bAllChilds ) {
        for ( size_t k = 0; k < pSubDirectories->size(); k++ ) {
            if ( !((SourceDirectory*)(*pSubDirectories)[ k ])->CreateRecursiveMakefile( TRUE ) ) {
                bSuccess = sal_False;
                break;
            }
        }
    }

    return bSuccess;
}

//
// class SourceDirectoryList
//

/*****************************************************************************/
SourceDirectoryList::~SourceDirectoryList()
/*****************************************************************************/
{
    for ( size_t i = 0; i < size(); i++ )
        delete at( i );
}

/*****************************************************************************/
SourceDirectory *SourceDirectoryList::Search(
    const ByteString &rDirectoryName )
/*****************************************************************************/
{
    size_t nPos = IsString( ( ByteString * ) (&rDirectoryName) );
    if ( nPos != LIST_ENTRY_NOTFOUND )
        return ( SourceDirectory * ) at( nPos );

    return NULL;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
