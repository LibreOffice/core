/*************************************************************************
 *
 *  $RCSfile: glosdoc.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: os $ $Date: 2000-10-20 14:18:05 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif


#pragma hdrstop

#ifndef _COM_SUN_STAR_UCB_XCOMMANDENVIRONMENT_HPP_
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTACCESS_HPP_
#include <com/sun/star/ucb/XContentAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _UCBHELPER_CONTENTIDENTIFIER_HXX
#include <ucbhelper/contentidentifier.hxx>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDER_HPP_
#include <com/sun/star/ucb/XContentProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_TRANSFERINFO_HPP_
#include <com/sun/star/ucb/TransferInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_NAMECLASH_HPP_
#include <com/sun/star/ucb/NameClash.hpp>
#endif
#ifndef _UCBHELPER_CONTENTBROKER_HXX
#include <ucbhelper/contentbroker.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#define _SVSTDARR_STRINGS
#include <svtools/svstdarr.hxx>
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef __RSC //autogen
#include <tools/errinf.hxx>
#endif
#ifndef _SFXINIMGR_HXX //autogen
#include <svtools/iniman.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif

#ifndef _UNOATXT_HXX
#include <unoatxt.hxx>
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

#ifndef _SWERROR_H
#include <swerror.h>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;
using namespace ::ucb;
using namespace ::rtl;

#define C2S(cChar) UniString::CreateFromAscii(cChar)
// INCLUDEs die nicht bedingungslos im MSC-PCH landen --------------------


// STATIC ---------------------------------------------------------------
static const char __FAR_DATA pDefName[] = "standard";

char __FAR_DATA aExt[] = ".bau";
extern const char* __FAR_DATA pGlosExt = aExt;

SV_IMPL_PTRARR(XAutoTextGroupPtrArr, XAutoTextGroupPtr)
SV_IMPL_PTRARR(XAutoTextEntryPtrArr, XInterfacePtr)

// PUBLIC METHODES -------------------------------------------------------
/* -----------------23.11.98 14:30-------------------
 *
 * --------------------------------------------------*/
sal_Bool lcl_RemoveFileFromList(SvStrings* pGlosArr,
                            XAutoTextEntryPtrArr& rGlosEntryArr,
                            XAutoTextGroupPtrArr& rGlosGroupArr,
                            const String& rName)
{
    if(pGlosArr)
    {
        const sal_uInt16 nCount = pGlosArr->Count();
        for(sal_uInt16 i = 0; i < nCount; ++i)
        {
            String *pTmp = (*pGlosArr)[i];
            if(*pTmp == rName)
            {
                //UNO-Objekt fuer die Gruppe aus dem Array loeschen
                OUString aUName = rName;
                sal_uInt16 nXCount = rGlosGroupArr.Count();
                for(sal_uInt16 j = 0; j < nXCount; ++j)
                {
                    uno::Reference< text::XAutoTextGroup > * pxGroup = rGlosGroupArr.GetObject(j);
                    uno::Reference< container::XNamed >  xNamed(*pxGroup, uno::UNO_QUERY);


                    if(xNamed->getName() == aUName )
                    {
                        text::XAutoTextGroup* pGroup = pxGroup->get();
                        ((SwXAutoTextGroup*)pGroup)->Invalidate();
                        rGlosGroupArr.Remove(j);
                        delete pxGroup;
                        break;
                    }
                }
                // alle UNO-Objekte fuer enthaltene Entries loeschen - rueckwaerts!
                nXCount = rGlosEntryArr.Count();
                for(j = nXCount; j; --j)
                {
                    uno::Reference< uno::XInterface > * pxEntry = rGlosEntryArr.GetObject(j);
                    uno::Reference< lang::XUnoTunnel >  xTunnel(*pxEntry, uno::UNO_QUERY);
                    SwXAutoTextEntry* pEntry = (SwXAutoTextEntry*)
                                xTunnel->getSomething(SwXAutoTextEntry::getUnoTunnelId());
                    if(pEntry->GetGroupName() == rName )
                    {
                        pEntry->Invalidate();
                        rGlosEntryArr.Remove(j);
                        delete pxEntry;
                    }
                }

                pGlosArr->Remove(i);
                delete pTmp;
                break;
            }
        }
    }
    return sal_True;
}
/* -----------------------------08.02.00 15:54--------------------------------

 ---------------------------------------------------------------------------*/
String lcl_CheckFileName(const String& rNewFilePath, const String& rNewGroupName)
{
    String sRet;
    //group name should contain only A-Z and a-z and spaces
    for(sal_uInt16 i = 0; i < rNewGroupName.Len(); i++)
    {
        char cChar = rNewGroupName.GetChar(i);
        if( (cChar >= 'A') && (cChar <= 'Z') ||
                (cChar >= 'a') && (cChar <= 'z') ||
                    (cChar >= '0') && (cChar <= '9') ||
                        (cChar = '_') ||
                    cChar == 0x20 )
        {
            sRet += cChar;
        }
    }
    sRet.EraseLeadingChars();
    sRet.EraseTrailingChars();
    String sTmpDir(rNewFilePath);
    sTmpDir += INET_PATH_TOKEN;
    sTmpDir += sRet;
    sTmpDir += SwGlossaries::GetExtension();

    BOOL bCreated = FALSE;
    try
    {
        ::ucb::Content aTestContent(    sTmpDir ,
                                        uno::Reference< XCommandEnvironment >());
        bCreated = aTestContent.isDocument();
    }
    catch(...)
    {
        bCreated = FALSE;
    }
    if(!sRet.Len() || bCreated)
    {
        //generate generic name
        const String sGroupBaseName(C2S("group"));
        String sTmpDir(rNewFilePath);
        sTmpDir += INET_PATH_TOKEN;
        for(sal_uInt16 i = 0; i < USHRT_MAX; i++)
        {
            String sName(sGroupBaseName);
            sName += String::CreateFromInt32(i);
            sName += SwGlossaries::GetExtension();
            sName.Insert(sTmpDir, 0);
            try
            {
                ::ucb::Content aTestContent(sName ,
                                            uno::Reference< XCommandEnvironment >());
                bCreated = aTestContent.isDocument();
            }
            catch(...)
            {
                bCreated = FALSE;
            }

            if(!bCreated)
            {
                sRet = sGroupBaseName;
                sRet += String::CreateFromInt32(i);
                break;
            }
        }
    }
    return sRet;
}
/*------------------------------------------------------------------------
    Beschreibung: Liefert den Namen der Default-Gruppe
------------------------------------------------------------------------*/


String  SwGlossaries::GetDefName()
{
    return C2S(pDefName);
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
    //man darf zweimal suchen, denn bei mehreren Verzeichnissen koennte der caseinsensitive Name mehrfach auftreten
    const International& rInt = Application::GetAppInternational();
    Reference< lang::XMultiServiceFactory > xMSF = comphelper::getProcessServiceFactory();

    for(i = 0; i < nCount; i++)
    {
        String sTemp(GetGroupName(i));
        String sPath = sTemp.GetToken(1, GLOS_DELIM);
        sal_uInt16 nPath = sPath.ToInt32();

        BOOL bCaseSensitive = FALSE;
        try
        {
            INetURLObject aTempObj(*(*pPathArr)[nPath]);
            aTempObj.SetBase(aTempObj.GetBase().ToLowerAscii());
            Reference<XContentIdentifier> xRef1 = new
                    ::ucb::ContentIdentifier( xMSF, aTempObj.GetMainURL());
            aTempObj.SetBase(aTempObj.GetBase().ToUpperAscii());
            Reference<XContentIdentifier> xRef2 = new
                    ::ucb::ContentIdentifier( xMSF, aTempObj.GetMainURL());

            ContentBroker& rBroker = *ContentBroker::get();

            Reference<XContentProvider > xProv = rBroker.getContentProviderInterface();
            sal_Int32 nCompare = xProv->compareContentIds( xRef1, xRef2 );
            bCaseSensitive = nCompare != 0;
        }
        catch(...)
        {
        }

        if( !bCaseSensitive &&
                rInt.CompareEqual( rGroup, sTemp.GetToken(0, GLOS_DELIM),
                                    INTN_COMPARE_IGNORECASE))
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
    sal_uInt16 nNewPath = rGroupName.GetToken(1, GLOS_DELIM).ToInt32();
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
        rGroupName += GLOS_DELIM;
        rGroupName += String::CreateFromInt32(nNewPath);
        return sal_True;
    }
    return sal_False;
}
/* -----------------23.11.98 13:13-------------------
 *
 * --------------------------------------------------*/
sal_Bool    SwGlossaries::RenameGroupDoc(
    const String& rOldGroup, String& rNewGroup, const String& rNewTitle)
{
    sal_Bool bRet = sal_False;
    sal_uInt16 nOldPath = rOldGroup.GetToken(1, GLOS_DELIM).ToInt32();
    if(nOldPath < pPathArr->Count())
    {
        String sOldFileURL(*(*pPathArr)[nOldPath]);
        sOldFileURL += INET_PATH_TOKEN;
        sOldFileURL += rOldGroup.GetToken(0, GLOS_DELIM);
        sOldFileURL.AppendAscii(pGlosExt);
        BOOL bExist = FALSE;
        try
        {
            ::ucb::Content aTestContent(    sOldFileURL ,
                                            uno::Reference< XCommandEnvironment >());
            bExist = aTestContent.isDocument();
        }
        catch(...)
        {
        }
        DBG_ASSERT(bExist, "Gruppe existiert nicht!")
        if(bExist)
        {
            sal_uInt16 nNewPath = rNewGroup.GetToken(1, GLOS_DELIM).ToInt32();
            if( nNewPath < pPathArr->Count())
            {
                String sNewFilePath(*(*pPathArr)[nNewPath]);
                String sNewFileName = lcl_CheckFileName(
                                    sNewFilePath, rNewGroup.GetToken(0, GLOS_DELIM));
                //String aTmp( rNewGroup.GetToken(0, GLOS_DELIM));
                const sal_uInt16 nFileNameLen = sNewFileName.Len();
                sNewFileName.AppendAscii(pGlosExt);
                String sTempNewFilePath(sNewFilePath);
                sTempNewFilePath += INET_PATH_TOKEN;
                sTempNewFilePath += sNewFileName ;
                BOOL bExist = FALSE;
                try
                {
                    ::ucb::Content aTestContent( sTempNewFilePath,
                                                    uno::Reference< XCommandEnvironment >());
                    bExist = aTestContent.isDocument();
                }
                catch(...)
                {
                }
                DBG_ASSERT(!bExist, "Gruppe existiert bereits!")
                if(!bExist)
                {
                    BOOL bCopyCompleted = TRUE;
                    try
                    {
                        ::ucb::Content aTempContent(sNewFilePath,
                                                    Reference< XCommandEnvironment > ());

                        Any aAny;
                        TransferInfo aInfo;
                        aInfo.NameClash = NameClash::ERROR;
                        aInfo.NewTitle = sNewFileName;
                        aInfo.SourceURL = sOldFileURL;
                        aInfo.MoveData  = TRUE;

                        aAny <<= aInfo;
                        aTempContent.executeCommand( rtl::OUString::createFromAscii( "transfer" ),
                                            aAny);
                    }
                    catch( Exception& rEx )
                    {
                        bCopyCompleted = FALSE;
                    }
                    if(bCopyCompleted)
                    {
                        bRet = sal_True;
                        lcl_RemoveFileFromList(pGlosArr, aGlosEntryArr, aGlosGroupArr, rOldGroup);
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
    sal_uInt16 nPath = rName.GetToken(1, GLOS_DELIM).ToInt32();
    if(nPath >= pPathArr->Count())
        return sal_False;
    String sFileURL(*(*pPathArr)[nPath]);
    String aTmp( rName.GetToken(0, GLOS_DELIM));

    String aName(aTmp);
    aName += GLOS_DELIM;
    aName += String::CreateFromInt32(nPath);

    aTmp.AppendAscii(pGlosExt);
    sFileURL += INET_PATH_TOKEN;
    sFileURL += aTmp;
        // Auch, wenn das File nicht existiert, muss es aus der Liste
        // der Textbausteinbereiche entfernt werden
    // Kein && wegen CFfront
#ifdef DBG_UTIL
    BOOL bRemoved = FALSE;
#endif
    try
    {
        ::ucb::Content aTempContent(sFileURL,
                                    Reference< XCommandEnvironment > ());
        aTempContent.executeCommand( rtl::OUString::createFromAscii( "delete" ),
                            makeAny( sal_Bool( sal_True ) ) );
#ifdef DBG_UTIL
        bRemoved = TRUE;
#endif
    }
    catch( ... )
    {
        DBG_ERRORFILE( "Exception" );
    }
    DBG_ASSERT(bRemoved, "file has not been removed")
    return lcl_RemoveFileFromList(pGlosArr, aGlosEntryArr, aGlosGroupArr, aName);
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
    nCount = aGlosGroupArr.Count();
    for(i = 0; i < nCount; ++i)
    {
        text::XAutoTextGroup* pGroup = aGlosGroupArr.GetObject(i)->get();
        ((SwXAutoTextGroup*)pGroup)->Invalidate();
    }
    nCount = aGlosEntryArr.Count();
    for(i = 0; i < nCount; ++i)
    {
        uno::Reference< uno::XInterface > * pxEntry = aGlosEntryArr.GetObject(i);
        uno::Reference< lang::XUnoTunnel > xTunnel(*pxEntry, uno::UNO_QUERY);
        DBG_ASSERT(xTunnel.is(), "No tunnel for SwXAutoTextEntry?");
        SwXAutoTextEntry* pEntry =
            (SwXAutoTextEntry*)xTunnel->getSomething(SwXAutoTextEntry::getUnoTunnelId());
        pEntry->Invalidate();
    }
}
/*------------------------------------------------------------------------
    Beschreibung: Bausteindokument einlesen
------------------------------------------------------------------------*/


SwTextBlocks* SwGlossaries::GetGlosDoc( const String &rName, sal_Bool bCreate ) const
{
    sal_uInt16 nPath = rName.GetToken(1, GLOS_DELIM).ToInt32();
    SwTextBlocks *pTmp = 0;
    if(nPath < pPathArr->Count())
    {
        String sFileURL(*(*pPathArr)[nPath]);
        String aTmp( rName.GetToken(0, GLOS_DELIM));
        aTmp.AppendAscii(pGlosExt);
        sFileURL += INET_PATH_TOKEN;
        sFileURL += aTmp;

        BOOL bExist = FALSE;
        if(!bCreate)
        {
            try
            {
                ::ucb::Content aTestContent(sFileURL,
                                            uno::Reference< XCommandEnvironment >());
                bExist = aTestContent.isDocument();
            }
            catch(...)
            {
                bExist = FALSE;
            }
        }

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
    if(!pGlosArr)
    {
        pGlosArr = new SvStrings;
        for(sal_uInt16 i = 0; i < pPathArr->Count(); i++)
        {
                  try
                {
                    ::ucb::Content aCnt(*(*pPathArr)[i],
                                        uno::Reference< XCommandEnvironment >());
                       Reference< sdbc::XResultSet > xResultSet;
                      Sequence< OUString > aProps(1);
                    OUString* pProps = aProps.getArray();
                    pProps[ 0 ] = OUString::createFromAscii( "Title" );
                    try
                    {
                           xResultSet = aCnt.createCursor(
                                        aProps, ::ucb::INCLUDE_DOCUMENTS_ONLY );
                    }
                    catch ( Exception )
                    {
                        DBG_ERRORFILE( "create cursor failed!" );
                    }

                    if ( aCnt.isFolder() && xResultSet.is() )
                    {
                          Reference< sdbc::XRow > xRow( xResultSet, UNO_QUERY );
                        Reference< XContentAccess >
                        xContentAccess( xResultSet, UNO_QUERY );
                        try
                        {
                            if ( xResultSet->first() )
                            {
                                do
                                {
//                                  OUString sId( xContentAccess->queryContentIdentfierString() );
                                    // an den Gruppennamen wird der Pfad-Index angehaengt
                                    // damit spaeter richtig zugegriffen werden kann
                                    String sTitle = xRow->getString( 1 );
                                    String sExt;
                                    if(sTitle.Len() > 4)  //length of ".bau"
                                    {
                                        String sExt  = sTitle.Copy( sTitle.Len() - 4, 4);
                                        if(sExt.EqualsIgnoreCaseAscii(pGlosExt))
                                        {
                                            sTitle.Erase( sTitle.Len() - 4, 4);
                                            sTitle += GLOS_DELIM;
                                            sTitle += String::CreateFromInt32(i);
                                            String *pTitle = new String(sTitle);
                                            pGlosArr->Insert(pTitle, pGlosArr->Count());
                                        }
                                    }
                                }
                                while ( xResultSet->next() );
                              }
                        }
                        catch ( ... )
                        {
                            DBG_ERRORFILE( "Exception caught!" );
                        }
                    }
                }
                catch ( ... )
                {
                    DBG_ERRORFILE( "Exception caught!" );
                }
            }

        if(!pGlosArr->Count())
        {
            // Der Standard-Baustein steht im ersten Teil des Pfades
            String *pTmp = new String(C2S(pDefName));
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
    String aNewPath( aPathOpt.GetGlossaryPath() );
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
        sal_uInt16 nTokenCount = aPath.GetTokenCount(SFX_SEARCHPATH_DELIMITER);
        SvStrings aDirArr;
        for( i = 0; i < nTokenCount; i++ )
        {
            String sPth(aPath.GetToken(i, SFX_SEARCHPATH_DELIMITER));
            sPth = URIHelper::SmartRelToAbs(sPth);

            if(i && lcl_FindSameEntry(aDirArr, sPth))
            {
                continue;
            }
            aDirArr.Insert(new String(sPth), aDirArr.Count());
            BOOL bExists = FALSE;
            try
            {
                ::ucb::Content aTestContent(sPth, uno::Reference< XCommandEnvironment >());
                bExists = aTestContent.isFolder();
            }
            catch(...)
            {
                DBG_ERROR("exception <getPropertyValue(IsFolder)>")
            }

            if(!bExists)
            {
                if(sErrPath.Len())
                    sErrPath += SFX_SEARCHPATH_DELIMITER;
                INetURLObject aTemp(sPth);
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
String  SwGlossaries::GetExtension()
{
    return C2S(aExt);
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.2  2000/10/06 13:35:57  jp
    should changes: don't use IniManager

    Revision 1.1.1.1  2000/09/18 17:14:44  hr
    initial import

    Revision 1.115  2000/09/18 16:05:56  willem.vandorp
    OpenOffice header added.

    Revision 1.114  2000/08/08 10:37:27  os
    #77403# rename of AutoText category repaired

    Revision 1.113  2000/08/08 10:14:48  os
    ucb transfer command used

    Revision 1.112  2000/08/07 08:52:02  os
    #77227# '_' allowed in glossary names

    Revision 1.111  2000/07/20 15:24:37  jp
    Bug #77040#: don't create references with stack objects, must always created on the heap

    Revision 1.110  2000/07/13 13:39:40  os
    #76805# ignore case of .bau extension

    Revision 1.109  2000/06/26 13:16:26  os
    INetURLObject::SmartRelToAbs removed

    Revision 1.108  2000/06/20 14:51:24  os
    SUPD removed

    Revision 1.107  2000/06/13 09:57:14  os
    using UCB

    Revision 1.106  2000/06/08 09:47:32  os
    using UCB

    Revision 1.105  2000/06/07 13:27:12  os
    using UCB

    Revision 1.104  2000/05/23 19:22:29  jp
    Bugfixes for Unicode

    Revision 1.103  2000/05/19 12:08:36  os
    appending of indices corrected

    Revision 1.102  2000/04/18 15:08:17  os
    UNICODE

    Revision 1.101  2000/03/23 07:49:13  os
    UNO III

    Revision 1.100  2000/03/06 08:43:31  os
    #70359# renames glossary groups: return corrected group name

    Revision 1.99  2000/02/22 13:26:49  os
    #73271# always append path index

    Revision 1.98  2000/02/14 14:40:48  os
    #70473# Unicode

    Revision 1.97  2000/02/10 10:34:32  os
    #70359# titles added to AutoText groups

    Revision 1.96  1999/10/21 17:48:55  jp
    have to change - SearchFile with SfxIniManager, dont use SwFinder for this

    Revision 1.95  1999/07/21 14:12:06  JP
    Bug #67779#: set any MsgBoxType at the StringErrorInfo


      Rev 1.94   21 Jul 1999 16:12:06   JP
   Bug #67779#: set any MsgBoxType at the StringErrorInfo

      Rev 1.93   09 Feb 1999 10:47:04   OS
   #61205# AutoText-Gruppen koennen beliebige Namen erhalten

      Rev 1.92   25 Jan 1999 13:40:16   HR
   Insert() jetzt richtig

      Rev 1.91   25 Jan 1999 13:26:44   OS
   #61050# C40_INSERT

      Rev 1.90   21 Jan 1999 15:45:40   OS
   #61050# doppelte Pfade abfangen

      Rev 1.89   10 Dec 1998 15:57:06   OS
   #56371# TF_ONE51 Zwischenstand

      Rev 1.88   27 Nov 1998 13:57:38   OS
   #59548# illegales Sonderzeichen entfernt

      Rev 1.87   24 Nov 1998 08:54:42   OS
   #59548# AutoText-Gruppen umbenennbar

      Rev 1.86   19 Jun 1998 16:34:58   OS
   GetGroupName liefert String, nicht UString

      Rev 1.85   19 Jun 1998 14:55:26   MH
   add: cast

      Rev 1.84   18 Jun 1998 18:17:42   OS
   Array fuer Textbausteine

      Rev 1.83   16 Jun 1998 16:49:36   OS
   AutoText-interface verbessert

      Rev 1.82   27 May 1998 17:08:18   OM
   Uno ::com::sun::star::text::AutoTextContainer

      Rev 1.81   17 Feb 1998 14:29:42   RG
   Mac: sysdep raus

      Rev 1.80   16 Dec 1997 18:14:18   JP
   GetSearchDelim gegen SFX_SEARCH_DELIMITER ausgetauscht

      Rev 1.79   28 Nov 1997 19:57:08   MA
   includes

      Rev 1.78   24 Nov 1997 16:47:46   MA
   includes

      Rev 1.77   03 Nov 1997 13:22:40   MA
   precomp entfernt

      Rev 1.76   10 Oct 1997 12:28:38   OS
   vollstaendige Pfadpruefung

      Rev 1.75   07 Oct 1997 07:28:56   OS
   Path-Index ueberpruefen #44360#

      Rev 1.74   26 Aug 1997 16:02:16   TRI
   VCL Anpassungen

      Rev 1.73   30 Jul 1997 18:33:30   HJS
   includes

      Rev 1.72   30 Jul 1997 11:27:32   OM
   #41772# Bereich einfuegen und sofort wieder loeschen

      Rev 1.71   17 Jun 1997 10:16:18   OS
   Leerstring als AutoText-Pfad ueberleben

      Rev 1.70   10 Jun 1997 14:38:52   OS
   AutoText aus mehreren Verzeichnissen

      Rev 1.69   23 Oct 1996 13:49:30   JP
   SVMEM -> SVSTDARR Umstellung

      Rev 1.68   18 Oct 1996 13:49:04   JP
   GetGlosDoc: defaultetes Flag entfernt

      Rev 1.67   26 Sep 1996 20:24:58   HJS
   del: pDBAddress und aDBAddress

      Rev 1.66   26 Sep 1996 16:59:14   OS
   +default-Extension fuer Autotexte in swtypes

      Rev 1.65   30 Aug 1996 12:38:34   OS
   UpdateGlosPath kann Blockliste aktualisieren

      Rev 1.64   28 Aug 1996 14:12:16   OS
   includes

      Rev 1.63   03 Jul 1996 13:11:32   OS
   UpdateGlosPath: Ohne das Array zu loeschen, wird es nichts

      Rev 1.62   19 Jun 1996 12:32:38   OM
   Umstellung auf 323

      Rev 1.61   14 Dec 1995 23:00:38   JP
   Filter Umstellung: ErrorHandling und Read/Write-Parameter

      Rev 1.60   13 Dec 1995 17:19:48   MA
   opt: International

      Rev 1.59   30 Nov 1995 16:26:08   JP
   Umstellung SV304 - ErrorHandling

      Rev 1.58   24 Nov 1995 16:58:48   OM
   PCH->PRECOMPILED

      Rev 1.57   16 Nov 1995 18:45:36   OM
   Methoden zur Fehlerbehandlung

      Rev 1.56   26 Oct 1995 18:08:40   JP
   Umstellung auf SfxErrorHandler

      Rev 1.55   03 Oct 1995 10:16:20   JP
   FAR_DATA auf den String und nicht auf den Pointer

      Rev 1.54   07 Sep 1995 07:21:16   OS
    ',' in PM2-ifdef

------------------------------------------------------------------------*/

