/*************************************************************************
 *
 *  $RCSfile: glosdoc.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: hjs $ $Date: 2003-08-19 11:59:42 $
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



#pragma hdrstop

#define _SVSTDARR_STRINGS

#ifndef _UNOTOOLS_TRANSLITERATIONWRAPPER_HXX
#include <unotools/transliterationwrapper.hxx>
#endif

#include <svtools/svstdarr.hxx>

#ifndef __RSC //autogen
#include <tools/errinf.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef SVTOOLS_FSTATHELPER_HXX
#include <svtools/fstathelper.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _UNOTOOLS_TEMPFILE_HXX
#include <unotools/tempfile.hxx>
#endif

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>       // ASSERT
#endif
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _GLOSDOC_HXX
#include <glosdoc.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _SWUNOHELPER_HXX
#include <swunohelper.hxx>
#endif

#ifndef _SWSWERROR_H
#include <swerror.h>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif


// PUBLIC METHODES -------------------------------------------------------
/* -----------------------------08.02.00 15:54--------------------------------

 ---------------------------------------------------------------------------*/
String lcl_CheckFileName( const String& rNewFilePath,
                          const String& rNewGroupName )
{
    String sRet;
    //group name should contain only A-Z and a-z and spaces
    for( xub_StrLen i = 0; i < rNewGroupName.Len(); i++ )
    {
        sal_Unicode cChar = rNewGroupName.GetChar(i);
        if( (cChar >= 'A' && cChar <= 'Z') ||
            (cChar >= 'a' && cChar <= 'z') ||
            (cChar >= '0' && cChar <= '9') ||
            cChar == '_' || cChar == 0x20 )
        {
            sRet += cChar;
        }
    }
    sRet.EraseLeadingChars();
    sRet.EraseTrailingChars();

    BOOL bOk = FALSE;
    if( sRet.Len() )
    {
        String sTmpDir(rNewFilePath);
        sTmpDir += INET_PATH_TOKEN;
        sTmpDir += sRet;
        sTmpDir += SwGlossaries::GetExtension();
        bOk = !FStatHelper::IsDocument( sTmpDir );
    }

    if( !bOk )
    {
        String rSG = SwGlossaries::GetExtension();
        //generate generic name
        utl::TempFile aTemp(
            String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "group" )),
            &rSG, &rNewFilePath );
        aTemp.EnableKillingFile();

        INetURLObject aTempURL( aTemp.GetURL() );
        sRet = aTempURL.GetBase();
    }
    return sRet;
}
/*------------------------------------------------------------------------
    Beschreibung: Liefert den Namen der Default-Gruppe
------------------------------------------------------------------------*/


String  SwGlossaries::GetDefName()
{
    return String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "standard" ));

}
/*------------------------------------------------------------------------
    Beschreibung: Liefert die Anzahl der Textbausteingruppen
------------------------------------------------------------------------*/


sal_uInt16 SwGlossaries::GetGroupCnt()
{
    return  GetNameList()->Count();
}
/*------------------------------------------------------------------------
    Beschreibung: Liefert den Gruppennamen
------------------------------------------------------------------------*/
sal_Bool SwGlossaries::FindGroupName(String & rGroup)
{
    //  enthaelt der Gruppenname keinen Pfad, kann hier ein passender
    // Gruppeneintrag gesucht werden;
    sal_uInt16 nCount = GetGroupCnt();
    sal_uInt16 i;
    for(i= 0; i < nCount; i++)
    {
        String sTemp(GetGroupName(i));
        if(rGroup.Equals( sTemp.GetToken(0, GLOS_DELIM)))
        {
            rGroup = sTemp;
            return sal_True;
        }
    }
    //man darf zweimal suchen, denn bei mehreren Verzeichnissen koennte
    //der caseinsensitive Name mehrfach auftreten
    const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();
    for(i = 0; i < nCount; i++)
    {
        String sTemp( GetGroupName( i ));
        sal_uInt16 nPath = (sal_uInt16)sTemp.GetToken(1, GLOS_DELIM).ToInt32();

        if( !SWUnoHelper::UCB_IsCaseSensitiveFileName( *(*pPathArr)[nPath] )
             && rSCmp.isEqual( rGroup, sTemp.GetToken( 0, GLOS_DELIM) ) )
        {
            rGroup = sTemp;
            return sal_True;
        }
    }
    return sal_False;
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/

String SwGlossaries::GetGroupName(sal_uInt16 nGroupId)
{
    ASSERT(nGroupId < pGlosArr->Count(), Textbausteinarray ueberindiziert);
    return *(*pGlosArr)[nGroupId];
}
/* -----------------------------08.02.00 13:04--------------------------------

 ---------------------------------------------------------------------------*/
String  SwGlossaries::GetGroupTitle( const String& rGroupName )
{
    String  sRet;
    String sGroup(rGroupName);
    if(STRING_NOTFOUND == sGroup.Search(GLOS_DELIM))
        FindGroupName(sGroup);
    SwTextBlocks* pGroup = GetGroupDoc(sGroup, sal_False);
    if(pGroup)
    {
        sRet = pGroup->GetName();
         PutGroupDoc( pGroup );
    }
    return sRet;
}

/*------------------------------------------------------------------------
    Beschreibung: Liefert das Textbaustein-Dokument der Gruppe rName
------------------------------------------------------------------------*/

SwTextBlocks* SwGlossaries::GetGroupDoc(const String &rName,
                                        sal_Bool bCreate) const
{
        // gfs. in die Liste der Textbausteine eintragen
    if(bCreate && pGlosArr)
    {
        const String aName(rName);
        const sal_uInt16 nCount = pGlosArr->Count();
        for(sal_uInt16 i = 0; i < nCount; ++i)
        {
            const String *pName = (*pGlosArr)[i];
            if(*pName == aName)
                break;
        }
        if(i == nCount)
        {   // Baustein nicht in der Liste
            String *pTmp = new String(aName);
            pGlosArr->Insert(pTmp, pGlosArr->Count());
        }
    }
    return GetGlosDoc( rName, bCreate );
}

/*------------------------------------------------------------------------
 Beschreibung:  Loeschen Textblock
------------------------------------------------------------------------*/

void SwGlossaries::PutGroupDoc(SwTextBlocks *pBlock) {
    delete pBlock;
}
/*------------------------------------------------------------------------
    Beschreibung:   Erzeugt ein neues Dokument mit dem Gruppenname
                    Wird temp. auch als File angelegt, damit die
                    Gruppen auch spaeter (ohne Zugriff) vorhanden sind.
------------------------------------------------------------------------*/


sal_Bool SwGlossaries::NewGroupDoc(String& rGroupName, const String& rTitle)
{
    sal_uInt16 nNewPath = (sal_uInt16)rGroupName.GetToken(1, GLOS_DELIM).ToInt32();
    String sNewFilePath(*(*pPathArr)[nNewPath]);
    String sNewGroup = lcl_CheckFileName(sNewFilePath, rGroupName.GetToken(0, GLOS_DELIM));
    sNewGroup += GLOS_DELIM;
    sNewGroup += rGroupName.GetToken(1, GLOS_DELIM);
    SwTextBlocks *pBlock = GetGlosDoc( sNewGroup );
    if(pBlock)
    {
        String *pTmp =
            new String(sNewGroup);
        SvStrings* pList = GetNameList();
        pList->Insert(pTmp, pList->Count());
        pBlock->SetName(rTitle);
        PutGroupDoc(pBlock);
        rGroupName = sNewGroup;
        return sal_True;
    }
    return sal_False;
}
/* -----------------23.11.98 13:13-------------------
 *
 * --------------------------------------------------*/
sal_Bool    SwGlossaries::RenameGroupDoc(
    const String& rOldGroup, String& rNewGroup, const String& rNewTitle )
{
    sal_Bool bRet = sal_False;
    sal_uInt16 nOldPath = (sal_uInt16)rOldGroup.GetToken(1, GLOS_DELIM).ToInt32();
    if(nOldPath < pPathArr->Count())
    {
        String sOldFileURL(*(*pPathArr)[nOldPath]);
        sOldFileURL += INET_PATH_TOKEN;
        sOldFileURL += rOldGroup.GetToken(0, GLOS_DELIM);
        sOldFileURL += SwGlossaries::GetExtension();
        BOOL bExist = FStatHelper::IsDocument( sOldFileURL );
        DBG_ASSERT(bExist, "Gruppe existiert nicht!")
        if(bExist)
        {
            sal_uInt16 nNewPath = (sal_uInt16)rNewGroup.GetToken(1, GLOS_DELIM).ToInt32();
            if( nNewPath < pPathArr->Count())
            {
                String sNewFilePath(*(*pPathArr)[nNewPath]);
                String sNewFileName = lcl_CheckFileName(
                                    sNewFilePath, rNewGroup.GetToken(0, GLOS_DELIM));
                //String aTmp( rNewGroup.GetToken(0, GLOS_DELIM));
                const sal_uInt16 nFileNameLen = sNewFileName.Len();
                sNewFileName += SwGlossaries::GetExtension();
                String sTempNewFilePath(sNewFilePath);
                sTempNewFilePath += INET_PATH_TOKEN;
                sTempNewFilePath += sNewFileName ;
                BOOL bExist = FStatHelper::IsDocument( sTempNewFilePath );
                DBG_ASSERT(!bExist, "Gruppe existiert bereits!")
                if(!bExist)
                {
                    BOOL bCopyCompleted = SWUnoHelper::UCB_CopyFile(
                                        sOldFileURL, sTempNewFilePath, TRUE );
                    if(bCopyCompleted)
                    {
                        bRet = sal_True;
                        RemoveFileFromList( rOldGroup );

                        rNewGroup = sNewFileName.Copy(0, nFileNameLen);
                        rNewGroup += GLOS_DELIM;
                        rNewGroup += String::CreateFromInt32(nNewPath);
                        String *pTmp = new String(rNewGroup);
                        if(!pGlosArr)
                            GetNameList();
                        else
                            pGlosArr->Insert(pTmp, pGlosArr->Count());

                        sNewFilePath += INET_PATH_TOKEN;
                        sNewFilePath += sNewFileName ;
                        SwTextBlocks* pNewBlock = new SwTextBlocks( sNewFilePath );
                        pNewBlock->SetName(rNewTitle);
                        delete pNewBlock;
                    }
                }
            }
        }
    }
    return bRet;
}

/*------------------------------------------------------------------------
    Beschreibung: Loescht eine Textbausteingruppe
------------------------------------------------------------------------*/


sal_Bool SwGlossaries::DelGroupDoc(const String &rName)
{
    sal_uInt16 nPath = (sal_uInt16)rName.GetToken(1, GLOS_DELIM).ToInt32();
    if(nPath >= pPathArr->Count())
        return sal_False;
    String sFileURL(*(*pPathArr)[nPath]);
    String aTmp( rName.GetToken(0, GLOS_DELIM));
    String aName(aTmp);
    aName += GLOS_DELIM;
    aName += String::CreateFromInt32(nPath);

    aTmp += SwGlossaries::GetExtension();
    sFileURL += INET_PATH_TOKEN;
    sFileURL += aTmp;
        // Auch, wenn das File nicht existiert, muss es aus der Liste
        // der Textbausteinbereiche entfernt werden
    // Kein && wegen CFfront
    BOOL bRemoved = SWUnoHelper::UCB_DeleteFile( sFileURL );
    DBG_ASSERT(bRemoved, "file has not been removed");
    RemoveFileFromList( aName );
    return bRemoved;
}
/*------------------------------------------------------------------------
    Beschreibung: DTOR
------------------------------------------------------------------------*/


SwGlossaries::~SwGlossaries()
{
    sal_uInt16 nCount = pGlosArr? pGlosArr->Count() : 0;
    for(sal_uInt16 i = 0; i < nCount; ++i)
    {
        String *pTmp = (*pGlosArr)[i];
        delete pTmp;
    }
    nCount = pPathArr? pPathArr->Count() : 0;
    for(i = 0; i < nCount; ++i)
    {
        String *pTmp = (*pPathArr)[i];
        delete pTmp;
    }
    delete pGlosArr;
    delete pPathArr;

    InvalidateUNOOjects();
}
/*------------------------------------------------------------------------
    Beschreibung: Bausteindokument einlesen
------------------------------------------------------------------------*/


SwTextBlocks* SwGlossaries::GetGlosDoc( const String &rName, sal_Bool bCreate ) const
{
    sal_uInt16 nPath = (sal_uInt16)rName.GetToken(1, GLOS_DELIM).ToInt32();
    SwTextBlocks *pTmp = 0;
    if(nPath < pPathArr->Count())
    {
        String sFileURL(*(*pPathArr)[nPath]);
        String aTmp( rName.GetToken(0, GLOS_DELIM));
        aTmp += SwGlossaries::GetExtension();
        sFileURL += INET_PATH_TOKEN;
        sFileURL += aTmp;

        BOOL bExist = FALSE;
        if(!bCreate)
            bExist = FStatHelper::IsDocument( sFileURL );

        if (bCreate || bExist)
        {
            pTmp = new SwTextBlocks( sFileURL );
            sal_Bool bOk = sal_True;
            if( pTmp->GetError() )
            {
                ErrorHandler::HandleError( pTmp->GetError() );
                bOk = !IsError( pTmp->GetError() );
            }

            if( bOk && !pTmp->GetName().Len() )
                pTmp->SetName( rName );
        }
    }

    return pTmp;
}

/*------------------------------------------------------------------------
    Beschreibung: Zugriff auf die Liste der Name; diese wird gfs. eingelesen
------------------------------------------------------------------------*/

SvStrings* SwGlossaries::GetNameList()
{
    if( !pGlosArr )
    {
        pGlosArr = new SvStrings;
        String sExt( SwGlossaries::GetExtension() );
        for( sal_uInt16 i = 0; i < pPathArr->Count(); i++ )
        {
            SvStrings aFiles( 16, 16 );

            SWUnoHelper::UCB_GetFileListOfFolder( *(*pPathArr)[i], aFiles,
                                                    &sExt );
            for( USHORT nFiles = 0, nFEnd = aFiles.Count();
                    nFiles < nFEnd; ++nFiles )
            {
                String* pTitle = aFiles[ nFiles ];
                String sName( pTitle->Copy( 0, pTitle->Len() - sExt.Len() ));
                sName += GLOS_DELIM;
                sName += String::CreateFromInt32( i );
                pGlosArr->Insert( new String(sName), pGlosArr->Count() );

                // don't need any more these pointers
                delete pTitle;
            }
        }
        if(!pGlosArr->Count())
        {
            // Der Standard-Baustein steht im ersten Teil des Pfades
            String *pTmp = new String( SwGlossaries::GetDefName() );
            (*pTmp) += GLOS_DELIM;
            (*pTmp) += '0';
            pGlosArr->Insert(pTmp, pGlosArr->Count());
        }
    }
    return pGlosArr;
}

/*------------------------------------------------------------------------
    Beschreibung: CTOR
------------------------------------------------------------------------*/


SwGlossaries::SwGlossaries() :
    pGlosArr(0),
    pPathArr(0)
{
    pPathArr = new SvStrings;
    UpdateGlosPath(sal_True);
}

/*------------------------------------------------------------------------
    Beschreibung: Neuen Pfad einstellen und internes Array neu aufbauen
------------------------------------------------------------------------*/

/* -----------------21.01.99 15:36-------------------
*   #61050# Doppelte Pfade fuehren zu Verwirrung - als raus damit
 * --------------------------------------------------*/
sal_Bool lcl_FindSameEntry(const SvStrings& rDirArr, const String& rEntryURL)
{
    sal_uInt16 nSize = rDirArr.Count();
    for(sal_uInt16 i = 0; i < rDirArr.Count(); i++)
        if(rEntryURL == (*rDirArr.GetObject(i)))
            return sal_True;
    return sal_False;
}

void SwGlossaries::UpdateGlosPath(sal_Bool bFull)
{
    SvtPathOptions aPathOpt;
    String aNewPath( aPathOpt.GetAutoTextPath() );
    sal_Bool bPathChanged = aPath != aNewPath;
    if (bFull || bPathChanged)
    {
        aPath = aNewPath;
        sal_uInt16 nCount = pPathArr? pPathArr->Count() : 0;
        for(sal_uInt16 i = nCount; i; --i)
        {
            String *pTmp = (*pPathArr)[i - 1];
            pPathArr->Remove(i - 1);
            delete pTmp;
        }
        String sErrPath;
        sal_uInt16 nTokenCount = aPath.GetTokenCount(SVT_SEARCHPATH_DELIMITER);
        SvStrings aDirArr;
        for( i = 0; i < nTokenCount; i++ )
        {
            String sPth(aPath.GetToken(i, SVT_SEARCHPATH_DELIMITER));
            sPth = URIHelper::SmartRelToAbs(sPth);

            if(i && lcl_FindSameEntry(aDirArr, sPth))
            {
                continue;
            }
            aDirArr.Insert(new String(sPth), aDirArr.Count());
            if( !FStatHelper::IsFolder( sPth ) )
            {
                if( sErrPath.Len() )
                    sErrPath += SVT_SEARCHPATH_DELIMITER;
                INetURLObject aTemp( sPth );
                sErrPath += aTemp.GetFull();
            }
            else
                pPathArr->Insert(new String(sPth), pPathArr->Count());
        }
        aDirArr.DeleteAndDestroy(0, aDirArr.Count());

        if(!nTokenCount ||
            sErrPath.Len() && (bPathChanged || sOldErrPath != sErrPath) )
        {
            sOldErrPath = sErrPath;
            // Falscher Pfad, d.h. AutoText-Verzeichnis existiert nicht

            ErrorHandler::HandleError( *new StringErrorInfo(
                                    ERR_AUTOPATH_ERROR, sErrPath,
                                    ERRCODE_BUTTON_OK | ERRCODE_MSG_ERROR ));
            bError = sal_True;
        }
        else
            bError = sal_False;

        if(pGlosArr)
        {
            const sal_uInt16 nCount = pGlosArr->Count();
            for(sal_uInt16 i = 0; i < nCount; ++i)
            {
                delete (String *)(*pGlosArr)[i];
            }
            DELETEZ(pGlosArr);
            GetNameList();
        }
    }
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/


void SwGlossaries::ShowError()
{
    sal_uInt32 nPathError = *new StringErrorInfo(ERR_AUTOPATH_ERROR,
                                            sErrPath, ERRCODE_BUTTON_OK );
    ErrorHandler::HandleError( nPathError );
}
/* -----------------------------09.02.00 11:37--------------------------------

 ---------------------------------------------------------------------------*/
String SwGlossaries::GetExtension()
{
    return String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( ".bau" ));
}


