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
#include "precompiled_sw.hxx"

#include <memory>

#include <com/sun/star/container/XNamed.hpp>

#define _SVSTDARR_STRINGS
#include <unotools/transliterationwrapper.hxx>

#include <svl/svstdarr.hxx>

#ifndef __RSC //autogen
#include <tools/errinf.hxx>
#endif
#include <tools/debug.hxx>
#include <svl/urihelper.hxx>
#ifndef SVTOOLS_FSTATHELPER_HXX
#include <svl/fstathelper.hxx>
#endif
#include <unotools/pathoptions.hxx>
#include <unotools/tempfile.hxx>
#include <swtypes.hxx>
#include <errhdl.hxx>       // ASSERT
#include <uitool.hxx>
#include <glosdoc.hxx>
#include <shellio.hxx>
#include <swunohelper.hxx>

#include <unoatxt.hxx>
#include <swerror.h>
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


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

    sal_Bool bOk = sal_False;
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

        if( !SWUnoHelper::UCB_IsCaseSensitiveFileName( *(*m_pPathArr)[nPath] )
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
    ASSERT(nGroupId < m_pGlosArr->Count(), Textbausteinarray ueberindiziert);
    return *(*m_pGlosArr)[nGroupId];
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
    if(bCreate && m_pGlosArr)
    {
        const String aName(rName);
        const sal_uInt16 nCount = m_pGlosArr->Count();
        sal_uInt16 i;

        for( i = 0; i < nCount; ++i)
        {
            const String *pName = (*m_pGlosArr)[i];
            if(*pName == aName)
                break;
        }
        if(i == nCount)
        {   // Baustein nicht in der Liste
            String *pTmp = new String(aName);
            m_pGlosArr->Insert(pTmp, m_pGlosArr->Count());
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
    String sNewFilePath(*(*m_pPathArr)[nNewPath]);
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
    if(nOldPath < m_pPathArr->Count())
    {
        String sOldFileURL(*(*m_pPathArr)[nOldPath]);
        sOldFileURL += INET_PATH_TOKEN;
        sOldFileURL += rOldGroup.GetToken(0, GLOS_DELIM);
        sOldFileURL += SwGlossaries::GetExtension();
        sal_Bool bExist = FStatHelper::IsDocument( sOldFileURL );
        DBG_ASSERT(bExist, "Gruppe existiert nicht!");
        if(bExist)
        {
            sal_uInt16 nNewPath = (sal_uInt16)rNewGroup.GetToken(1, GLOS_DELIM).ToInt32();
            if( nNewPath < m_pPathArr->Count())
            {
                String sNewFilePath(*(*m_pPathArr)[nNewPath]);
                String sNewFileName = lcl_CheckFileName(
                                    sNewFilePath, rNewGroup.GetToken(0, GLOS_DELIM));
                //String aTmp( rNewGroup.GetToken(0, GLOS_DELIM));
                const sal_uInt16 nFileNameLen = sNewFileName.Len();
                sNewFileName += SwGlossaries::GetExtension();
                String sTempNewFilePath(sNewFilePath);
                sTempNewFilePath += INET_PATH_TOKEN;
                sTempNewFilePath += sNewFileName ;
                bExist = FStatHelper::IsDocument( sTempNewFilePath );
                DBG_ASSERT(!bExist, "Gruppe existiert bereits!");
                if(!bExist)
                {
                    sal_Bool bCopyCompleted = SWUnoHelper::UCB_CopyFile(
                                        sOldFileURL, sTempNewFilePath, sal_True );
                    if(bCopyCompleted)
                    {
                        bRet = sal_True;
                        RemoveFileFromList( rOldGroup );

                        rNewGroup = sNewFileName.Copy(0, nFileNameLen);
                        rNewGroup += GLOS_DELIM;
                        rNewGroup += String::CreateFromInt32(nNewPath);
                        String *pTmp = new String(rNewGroup);
                        if(!m_pGlosArr)
                            GetNameList();
                        else
                            m_pGlosArr->Insert(pTmp, m_pGlosArr->Count());

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
    if(nPath >= m_pPathArr->Count())
        return sal_False;
    String sFileURL(*(*m_pPathArr)[nPath]);
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
    sal_Bool bRemoved = SWUnoHelper::UCB_DeleteFile( sFileURL );
    DBG_ASSERT(bRemoved, "file has not been removed");
    RemoveFileFromList( aName );
    return bRemoved;
}
/*------------------------------------------------------------------------
    Beschreibung: DTOR
------------------------------------------------------------------------*/


SwGlossaries::~SwGlossaries()
{
    sal_uInt16 nCount = m_pGlosArr? m_pGlosArr->Count() : 0;
    sal_uInt16 i;

    for( i = 0; i < nCount; ++i)
    {
        String *pTmp = (*m_pGlosArr)[i];
        delete pTmp;
    }
    nCount = m_pPathArr? m_pPathArr->Count() : 0;
    for(i = 0; i < nCount; ++i)
    {
        String *pTmp = (*m_pPathArr)[i];
        delete pTmp;
    }
    delete m_pGlosArr;
    delete m_pPathArr;

    InvalidateUNOOjects();
}
/*------------------------------------------------------------------------
    Beschreibung: Bausteindokument einlesen
------------------------------------------------------------------------*/


SwTextBlocks* SwGlossaries::GetGlosDoc( const String &rName, sal_Bool bCreate ) const
{
    sal_uInt16 nPath = (sal_uInt16)rName.GetToken(1, GLOS_DELIM).ToInt32();
    SwTextBlocks *pTmp = 0;
    if(nPath < m_pPathArr->Count())
    {
        String sFileURL(*(*m_pPathArr)[nPath]);
        String aTmp( rName.GetToken(0, GLOS_DELIM));
        aTmp += SwGlossaries::GetExtension();
        sFileURL += INET_PATH_TOKEN;
        sFileURL += aTmp;

        sal_Bool bExist = sal_False;
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
    if( !m_pGlosArr )
    {
        m_pGlosArr = new SvStrings;
        String sExt( SwGlossaries::GetExtension() );
        for( sal_uInt16 i = 0; i < m_pPathArr->Count(); i++ )
        {
            SvStrings aFiles( 16, 16 );

            SWUnoHelper::UCB_GetFileListOfFolder( *(*m_pPathArr)[i], aFiles,
                                                    &sExt );
            for( sal_uInt16 nFiles = 0, nFEnd = aFiles.Count();
                    nFiles < nFEnd; ++nFiles )
            {
                String* pTitle = aFiles[ nFiles ];
                String sName( pTitle->Copy( 0, pTitle->Len() - sExt.Len() ));
                sName += GLOS_DELIM;
                sName += String::CreateFromInt32( i );
                m_pGlosArr->Insert( new String(sName), m_pGlosArr->Count() );

                // don't need any more these pointers
                delete pTitle;
            }
        }
        if(!m_pGlosArr->Count())
        {
            // Der Standard-Baustein steht im ersten Teil des Pfades
            String *pTmp = new String( SwGlossaries::GetDefName() );
            (*pTmp) += GLOS_DELIM;
            (*pTmp) += '0';
            m_pGlosArr->Insert(pTmp, m_pGlosArr->Count());
        }
    }
    return m_pGlosArr;
}

/*------------------------------------------------------------------------
    Beschreibung: CTOR
------------------------------------------------------------------------*/


SwGlossaries::SwGlossaries() :
    m_pPathArr(0),
    m_pGlosArr(0)
{
    m_pPathArr = new SvStrings;
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
    for(sal_uInt16 i = 0; i < rDirArr.Count(); i++)
        if(rEntryURL == (*rDirArr.GetObject(i)))
            return sal_True;
    return sal_False;
}

void SwGlossaries::UpdateGlosPath(sal_Bool bFull)
{
    SvtPathOptions aPathOpt;
    String aNewPath( aPathOpt.GetAutoTextPath() );
    sal_Bool bPathChanged = m_aPath != aNewPath;
    if (bFull || bPathChanged)
    {
        m_aPath = aNewPath;
        sal_uInt16 nCount = m_pPathArr? m_pPathArr->Count() : 0;
        sal_uInt16 i;

        for( i = nCount; i; --i)
        {
            String *pTmp = (*m_pPathArr)[i - 1];
            m_pPathArr->Remove(i - 1);
            delete pTmp;
        }
        sal_uInt16 nTokenCount = m_aPath.GetTokenCount(SVT_SEARCHPATH_DELIMITER);
        SvStrings aDirArr;
        for( i = 0; i < nTokenCount; i++ )
        {
            String sPth(m_aPath.GetToken(i, SVT_SEARCHPATH_DELIMITER));
            sPth = URIHelper::SmartRel2Abs(
                INetURLObject(), sPth, URIHelper::GetMaybeFileHdl());

            if(i && lcl_FindSameEntry(aDirArr, sPth))
            {
                continue;
            }
            aDirArr.Insert(new String(sPth), aDirArr.Count());
            if( !FStatHelper::IsFolder( sPth ) )
            {
                if( m_sErrPath.Len() )
                    m_sErrPath += SVT_SEARCHPATH_DELIMITER;
                INetURLObject aTemp( sPth );
                m_sErrPath += String(aTemp.GetFull());
            }
            else
                m_pPathArr->Insert(new String(sPth), m_pPathArr->Count());
        }
        aDirArr.DeleteAndDestroy(0, aDirArr.Count());

        if(!nTokenCount ||
            (m_sErrPath.Len() && (bPathChanged || m_sOldErrPath != m_sErrPath)) )
        {
            m_sOldErrPath = m_sErrPath;
            // Falscher Pfad, d.h. AutoText-Verzeichnis existiert nicht

            ErrorHandler::HandleError( *new StringErrorInfo(
                                    ERR_AUTOPATH_ERROR, m_sErrPath,
                                    ERRCODE_BUTTON_OK | ERRCODE_MSG_ERROR ));
            m_bError = sal_True;
        }
        else
            m_bError = sal_False;

        if(m_pGlosArr)
        {
            for(i = 0; i < m_pGlosArr->Count(); ++i)
            {
                delete (String *)(*m_pGlosArr)[i];
            }
            DELETEZ(m_pGlosArr);
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
                                            m_sErrPath, ERRCODE_BUTTON_OK );
    ErrorHandler::HandleError( nPathError );
}
/* -----------------------------09.02.00 11:37--------------------------------

 ---------------------------------------------------------------------------*/
String SwGlossaries::GetExtension()
{
    return String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( ".bau" ));
}



void SwGlossaries::RemoveFileFromList( const String& rGroup )
{
    if(m_pGlosArr)
    {
        const sal_uInt16 nCount = m_pGlosArr->Count();
        for(sal_uInt16 i = 0; i < nCount; ++i)
        {
            String *pTmp = (*m_pGlosArr)[i];
            if(*pTmp == rGroup)
            {
                rtl::OUString aUName = rGroup;
                {
                    // tell the UNO AutoTextGroup object that it's not valid anymore
                    for (   UnoAutoTextGroups::iterator aLoop = m_aGlossaryGroups.begin();
                            aLoop != m_aGlossaryGroups.end();
                            ++aLoop
                        )
                    {
                        Reference< container::XNamed > xNamed( aLoop->get(), UNO_QUERY );
                        if ( xNamed.is() && ( xNamed->getName() == aUName ) )
                        {
                            static_cast< SwXAutoTextGroup* >( xNamed.get() )->Invalidate();
                                // note that this static_cast works because we know that the array only
                                // contains SwXAutoTextGroup implementation
                            m_aGlossaryGroups.erase( aLoop );
                            break;
                        }
                    }
                }

                {
                    // tell all our UNO AutoTextEntry objects that they're not valid anymore
                    for (   UnoAutoTextEntries::iterator aLoop = m_aGlossaryEntries.begin();
                            aLoop != m_aGlossaryEntries.end();
                        )
                    {
                        Reference< lang::XUnoTunnel > xEntryTunnel( aLoop->get(), UNO_QUERY );

                        SwXAutoTextEntry* pEntry = NULL;
                        if ( xEntryTunnel.is() )
                            pEntry = reinterpret_cast< SwXAutoTextEntry* >(
                                xEntryTunnel->getSomething( SwXAutoTextEntry::getUnoTunnelId() ) );

                        if ( pEntry && ( pEntry->GetGroupName() == rGroup ) )
                        {
                            pEntry->Invalidate();
                            aLoop = m_aGlossaryEntries.erase( aLoop );
                        }
                        else
                            ++aLoop;
                    }
                }

                m_pGlosArr->Remove(i);
                delete pTmp;
                break;
            }
        }
    }
}


String SwGlossaries::GetCompleteGroupName( const rtl::OUString& GroupName )
{
    sal_uInt16 nCount = GetGroupCnt();
    //wenn der Gruppenname intern erzeugt wurde, dann steht auch hier der Pfad drin
    String sGroup(GroupName);
    String sGroupName(sGroup.GetToken(0, GLOS_DELIM));
    String sPath = sGroup.GetToken(1, GLOS_DELIM);
    sal_Bool bPathLen = sPath.Len() > 0;
    for ( sal_uInt16 i = 0; i < nCount; i++ )
    {
        String sGrpName = GetGroupName(i);
        if(bPathLen ? sGroup == sGrpName : sGroupName == sGrpName.GetToken(0, GLOS_DELIM))
        {
            return sGrpName;
        }
    }
    return aEmptyStr;
}


void SwGlossaries::InvalidateUNOOjects()
{
    // invalidate all the AutoTextGroup-objects
    for (   UnoAutoTextGroups::iterator aGroupLoop = m_aGlossaryGroups.begin();
            aGroupLoop != m_aGlossaryGroups.end();
            ++aGroupLoop
        )
    {
        Reference< text::XAutoTextGroup > xGroup( aGroupLoop->get(), UNO_QUERY );
        if ( xGroup.is() )
            static_cast< SwXAutoTextGroup* >( xGroup.get() )->Invalidate();
    }
    UnoAutoTextGroups aTmpg = UnoAutoTextGroups();
    m_aGlossaryGroups.swap( aTmpg );

    // invalidate all the AutoTextEntry-objects
    for (   UnoAutoTextEntries::const_iterator aEntryLoop = m_aGlossaryEntries.begin();
            aEntryLoop != m_aGlossaryEntries.end();
            ++aEntryLoop
        )
    {
        Reference< lang::XUnoTunnel > xEntryTunnel( aEntryLoop->get(), UNO_QUERY );
        SwXAutoTextEntry* pEntry = NULL;
        if ( xEntryTunnel.is() )
            pEntry = reinterpret_cast< SwXAutoTextEntry* >(
                xEntryTunnel->getSomething( SwXAutoTextEntry::getUnoTunnelId() ) );

        if ( pEntry )
            pEntry->Invalidate();
    }
    UnoAutoTextEntries aTmpe = UnoAutoTextEntries();
    m_aGlossaryEntries.swap( aTmpe );
}

//-----------------------------------------------------------------------
//--- 03.03.2003 14:15:32 -----------------------------------------------

Reference< text::XAutoTextGroup > SwGlossaries::GetAutoTextGroup( const ::rtl::OUString& _rGroupName, bool _bCreate )
{
    // first, find the name with path-extension
    String sCompleteGroupName = GetCompleteGroupName( _rGroupName );

    Reference< text::XAutoTextGroup >  xGroup;

    // look up the group in the cache
    UnoAutoTextGroups::iterator aSearch = m_aGlossaryGroups.begin();
    for ( ; aSearch != m_aGlossaryGroups.end(); )
    {
        Reference< lang::XUnoTunnel > xGroupTunnel( aSearch->get(), UNO_QUERY );

        SwXAutoTextGroup* pSwGroup = 0;
        if ( xGroupTunnel.is() )
            pSwGroup = reinterpret_cast< SwXAutoTextGroup* >( xGroupTunnel->getSomething( SwXAutoTextGroup::getUnoTunnelId() ) );

        if ( !pSwGroup )
        {
            // the object is dead in the meantime -> remove from cache
            aSearch = m_aGlossaryGroups.erase( aSearch );
            continue;
        }

        if ( _rGroupName == pSwGroup->getName() )
        {                               // the group is already cached
            if ( sCompleteGroupName.Len() )
            {   // the group still exists -> return it
                xGroup = pSwGroup;
                break;
            }
            else
            {
                // this group does not exist (anymore) -> release the cached UNO object for it
                aSearch = m_aGlossaryGroups.erase( aSearch );
                // so it won't be created below
                _bCreate = sal_False;
                break;
            }
        }

        ++aSearch;
    }

    if ( !xGroup.is() && _bCreate )
    {
        xGroup = new SwXAutoTextGroup( sCompleteGroupName, this );
        // cache it
        m_aGlossaryGroups.push_back( AutoTextGroupRef( xGroup ) );
    }

    return xGroup;
}

//-----------------------------------------------------------------------
//--- 03.03.2003 13:46:06 -----------------------------------------------

Reference< text::XAutoTextEntry > SwGlossaries::GetAutoTextEntry( const String& _rCompleteGroupName, const ::rtl::OUString& _rGroupName, const ::rtl::OUString& _rEntryName,
    bool _bCreate )
{
    //standard must be created
    sal_Bool bCreate = ( _rCompleteGroupName == GetDefName() );
    ::std::auto_ptr< SwTextBlocks > pGlosGroup( GetGroupDoc( _rCompleteGroupName, bCreate ) );

    if ( pGlosGroup.get() && !pGlosGroup->GetError() )
    {
        sal_uInt16 nIdx = pGlosGroup->GetIndex( _rEntryName );
        if ( USHRT_MAX == nIdx )
            throw container::NoSuchElementException();
    }
    else
        throw lang::WrappedTargetException();

    Reference< text::XAutoTextEntry > xReturn;
    String sGroupName( _rGroupName );
    String sEntryName( _rEntryName );

    UnoAutoTextEntries::iterator aSearch( m_aGlossaryEntries.begin() );
    for ( ; aSearch != m_aGlossaryEntries.end(); )
    {
        Reference< lang::XUnoTunnel > xEntryTunnel( aSearch->get(), UNO_QUERY );

        SwXAutoTextEntry* pEntry = NULL;
        if ( xEntryTunnel.is() )
            pEntry = reinterpret_cast< SwXAutoTextEntry* >( xEntryTunnel->getSomething( SwXAutoTextEntry::getUnoTunnelId() ) );
        else
        {
            // the object is dead in the meantime -> remove from cache
            aSearch = m_aGlossaryEntries.erase( aSearch );
            continue;
        }

        if  (   pEntry
            &&  ( COMPARE_EQUAL == pEntry->GetGroupName().CompareTo( sGroupName ) )
            &&  ( COMPARE_EQUAL == pEntry->GetEntryName().CompareTo( sEntryName ) )
            )
        {
            xReturn = pEntry;
            break;
        }

        ++aSearch;
    }

    if ( !xReturn.is() && _bCreate )
    {
        xReturn = new SwXAutoTextEntry( this, sGroupName, sEntryName );
        // cache it
        m_aGlossaryEntries.push_back( AutoTextEntryRef( xReturn ) );
    }

    return xReturn;
}



