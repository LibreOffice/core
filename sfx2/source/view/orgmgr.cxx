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
#include "precompiled_sfx2.hxx"
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/ElementModes.hpp>

#include <vcl/msgbox.hxx>
#include <tools/urlobj.hxx>

#include <comphelper/processfactory.hxx>
#include <unotools/intlwrapper.hxx>

#include <comphelper/storagehelper.hxx>

#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/doctempl.hxx>
#include "docvor.hxx"
#include "orgmgr.hxx"
#include "sfxtypes.hxx"
#include "sfxresid.hxx"
#include "view.hrc"
#include <sfx2/docfilt.hxx>
#include "fltfnc.hxx"

using namespace ::com::sun::star;

//=========================================================================

/*  [Beschreibung]

    Implementierungsklasse; einzelner Eintrag in der Dateiansicht

*/

struct _FileListEntry
{
    String aFileName;           // Dateiname mit komplettem Pfad
    String aBaseName;           // Dateiname
    const CollatorWrapper* pCollator;
    SfxObjectShellLock aDocShell; // ObjectShell als Ref-Klasse

//REMOVE        SvStorageRef aStor;         // Referenz auf Storage, wenn wir diesen geoeffnet haben
    //uno::Reference< embed::XStorage > xStorage;

    BOOL bFile;                 // als Datei auf Platte
                                // (!= unbenannt1, nicht als Dok. geladen;
                                // diese werden nicht gespeichert!)
    BOOL bOwner;                // selbst erzeugt
    BOOL bNoName;
    BOOL bOwnFormat;

    _FileListEntry( const String& rFileName,
                    const CollatorWrapper* pColl, const String* pTitle = NULL );
    ~_FileListEntry();

    int operator==( const _FileListEntry &rCmp) const;
    int operator< ( const _FileListEntry &rCmp) const;
    BOOL DeleteObjectShell();
};

//-------------------------------------------------------------------------

inline int _FileListEntry::operator==(const _FileListEntry &rCmp) const
{
    DBG_ASSERT( pCollator, "invalid CollatorWrapper" );
    return COMPARE_EQUAL == pCollator->compareString(aBaseName, rCmp.aBaseName);
}

//-------------------------------------------------------------------------

inline int _FileListEntry::operator< (const _FileListEntry &rCmp) const
{
    DBG_ASSERT( pCollator, "invalid CollatorWrapper" );
    return COMPARE_LESS == pCollator->compareString(aBaseName, rCmp.aBaseName);
}

//-------------------------------------------------------------------------

_FileListEntry::_FileListEntry( const String& rFileName,
                                const CollatorWrapper* pColl, const String* pTitle ) :

    aFileName   ( rFileName ),
    pCollator   ( pColl ),
    bFile       ( FALSE ),
    bOwner      ( FALSE ),
    bNoName     ( TRUE ),
    bOwnFormat  ( TRUE )
{
    if ( pTitle )
        aBaseName = *pTitle;
    else
    {
        INetURLObject aObj( rFileName, INET_PROT_FILE );
        aBaseName = aObj.getName( INetURLObject::LAST_SEGMENT, true,
                                  INetURLObject::DECODE_WITH_CHARSET );
    }
}

//-------------------------------------------------------------------------

_FileListEntry::~_FileListEntry()
{
    DeleteObjectShell();
}

//-------------------------------------------------------------------------

SV_IMPL_OP_PTRARR_SORT(_SfxObjectList, _FileListEntry*)

//=========================================================================

BOOL _FileListEntry::DeleteObjectShell()

/*  [Beschreibung]

    Freigabe der DokumentShell

    [Returnwert]            TRUE: alles Ok
                            FALSE: es ist ein Fehler aufgetreten (das
                            Dokument konnte nicht gesichert werden)

*/

{
    BOOL bRet = TRUE;
    //Falls wir die Shell angelegt haben und sie veraendert wurde
    if(bOwner && aDocShell.Is() && aDocShell->IsModified())
    {
        //Mussten wir konvertieren?
        if( bOwnFormat )
        {
            if(!aDocShell->Save() )
                bRet = FALSE;
            else
            {
                try {
                    uno::Reference< embed::XTransactedObject > xTransact( aDocShell->GetStorage(), uno::UNO_QUERY );
                    OSL_ENSURE( xTransact.is(), "Storage must implement XTransactedObject!\n" );
                    if ( !xTransact.is() )
                        throw uno::RuntimeException();

                    xTransact->commit();
                }
                catch( uno::Exception& )
                {
                }

//              aDocShell->SfxObjectShell::DoSaveCompleted();
            }
        }
        else
        {
            // Falls konvertiert im eigenen Format speichern
            INetURLObject aObj( aFileName );
            String aTitle = aObj.getName( INetURLObject::LAST_SEGMENT, true,
                                          INetURLObject::DECODE_WITH_CHARSET );
            bRet = aDocShell->PreDoSaveAs_Impl(
                        aTitle, aDocShell->GetFactory().GetFilterContainer()->GetAnyFilter( SFX_FILTER_IMPORT | SFX_FILTER_EXPORT )->GetFilterName(), 0 );
        }
    }

    if( bOwner)
    {
        aDocShell.Clear();
    }

    return bRet;
}

//-------------------------------------------------------------------------

SfxObjectList::SfxObjectList()
{
}

//-------------------------------------------------------------------------

SfxObjectList::~SfxObjectList()
{
    DeleteAndDestroy(0, Count());
}

//-------------------------------------------------------------------------

const String &SfxObjectList::GetBaseName(USHORT i) const
{
    return (*this)[i]->aBaseName;
}

//-------------------------------------------------------------------------

const String& SfxObjectList::GetFileName( USHORT i ) const
{
    return (*this)[i]->aFileName;
}

//-------------------------------------------------------------------------

SfxOrganizeMgr::SfxOrganizeMgr( SfxOrganizeListBox_Impl *pLeft,
                                SfxOrganizeListBox_Impl *pRight,
                                SfxDocumentTemplates *pTempl) :
    pImpl(new SfxOrganizeMgr_Impl),
    pTemplates(pTempl? pTempl: new SfxDocumentTemplates),
    pLeftBox(pLeft),
    pRightBox(pRight),
    bDeleteTemplates(pTempl == 0),
    bModified(0)

/*  [Beschreibung]

    Konstruktor

    Das aktuelle Dokument wird in die Liste der Dokumente
    aufgenommen.

*/
{
    pImpl->pDocList = new SfxObjectList;
    pImpl->pIntlWrapper = new IntlWrapper( ::comphelper::getProcessServiceFactory(), Application::GetSettings().GetLocale() );
    const CollatorWrapper* pCollator = pImpl->pIntlWrapper->getCaseCollator();
    for ( SfxObjectShell* pTmp = SfxObjectShell::GetFirst(); pTmp; pTmp = SfxObjectShell::GetNext(*pTmp) )
    {
        if ( pTmp->GetCreateMode() != SFX_CREATE_MODE_STANDARD ||
             !( pTmp->GetFlags() & SFXOBJECTSHELL_HASOPENDOC ) || !pTmp->GetStyleSheetPool() )
            continue;
        _FileListEntry* pNewEntry = NULL;
        String aTitle = pTmp->GetTitle( SFX_TITLE_TITLE );
        pNewEntry = new _FileListEntry( pTmp->GetMedium()->GetName(), pCollator, &aTitle );
        pNewEntry->aDocShell = pTmp;
        pImpl->pDocList->C40_PTR_INSERT( _FileListEntry, pNewEntry );
    }
}

//-------------------------------------------------------------------------

SfxOrganizeMgr::~SfxOrganizeMgr()
{
    if ( bDeleteTemplates )
        delete pTemplates;
    delete pImpl->pDocList;
    delete pImpl->pIntlWrapper;
    delete pImpl;
    pLeftBox = pRightBox = NULL;
}

//-------------------------------------------------------------------------

SfxObjectShellRef SfxOrganizeMgr::CreateObjectShell( USHORT nIdx )

/*  [Beschreibung]

    Zugriff auf die DokumentShell an der Position nIdx

    [Returnwert]            Referenz auf die DokumentShell

*/

{
    _FileListEntry* pEntry = (*pImpl->pDocList)[nIdx];
    // andernfalls Doc-Shell anlegen
    if ( !pEntry->aDocShell.Is() )
    {
        INetURLObject aFileObj( pEntry->aFileName );
        BOOL bDum = FALSE;
        SfxApplication* pSfxApp = SFX_APP();
        String aFilePath = aFileObj.GetMainURL( INetURLObject::NO_DECODE );
        pEntry->aDocShell = pSfxApp->DocAlreadyLoaded( aFilePath, FALSE, bDum );
        if ( !pEntry->aDocShell.Is() )
        {
            pEntry->bOwner = TRUE;
            SfxMedium* pMed = new SfxMedium(
                aFilePath, ( STREAM_READ | STREAM_SHARE_DENYWRITE ), FALSE, 0 );
            const SfxFilter* pFilter = NULL;
            pMed->UseInteractionHandler(TRUE);
            if (
                pSfxApp->GetFilterMatcher().GuessFilter(*pMed, &pFilter, SFX_FILTER_TEMPLATE, 0) ||
                (pFilter && !pFilter->IsOwnFormat()) ||
                (pFilter && !pFilter->UsesStorage())
               )
            {
                pSfxApp->LoadTemplate( pEntry->aDocShell, aFilePath );
                pEntry->bOwnFormat = FALSE;
                delete pMed;
                if ( pEntry->aDocShell.Is() )
                    return (SfxObjectShellRef)(SfxObjectShell*)(pEntry->aDocShell);
            }
            else
            {
                if ( pFilter )
                {
                    pEntry->bOwnFormat = TRUE;
                    pEntry->aDocShell = SfxObjectShell::CreateObject( pFilter->GetServiceName(), SFX_CREATE_MODE_ORGANIZER );
                    if ( pEntry->aDocShell.Is() )
                    {
                        pEntry->aDocShell->DoInitNew(0);
                        pEntry->aDocShell->LoadFrom( *pMed );
                        // Medium is now owned by DocShell
                        pEntry->aDocShell->DoSaveCompleted( pMed );
                    }
                }
            }
        }
    }
    return ( SfxObjectShellRef )(SfxObjectShell*)(pEntry->aDocShell);
}

//-------------------------------------------------------------------------

BOOL SfxOrganizeMgr::DeleteObjectShell(USHORT nIdx)

/*  [Beschreibung]

    Freigabe der DokumentShell an der Position nIdx

    [Returnwert]            TRUE: alles Ok
                            FALSE: es ist ein Fehler aufgetreten (das
                            Dokument konnte nicht gesichert werden)

*/
{
    return (*pImpl->pDocList)[nIdx]->DeleteObjectShell();
}

//-------------------------------------------------------------------------

SfxObjectShellRef SfxOrganizeMgr::CreateObjectShell(USHORT nRegion,
                                                        USHORT nIdx)
/*  [Beschreibung]

    Zugriff auf die DokumentShell an der Position nIdx im Bereich
    nRegion (Dokumentvorlage)

    [Returnwert]            Referenz auf die DokumentShell

*/
{
    return pTemplates->CreateObjectShell(nRegion, nIdx);
}

//-------------------------------------------------------------------------

BOOL SfxOrganizeMgr::DeleteObjectShell(USHORT nRegion, USHORT nIdx)

/*  [Beschreibung]

    Freigabe der DokumentShell an der Position nIdx im Bereich
    nRegion (Dokumentvorlage)

    [Returnwert]            TRUE: alles Ok
                            FALSE: es ist ein Fehler aufgetreten (das
                            Dokument konnte nicht gesichert werden)

*/

{
    return pTemplates->DeleteObjectShell(nRegion, nIdx);
}

//-------------------------------------------------------------------------

BOOL    SfxOrganizeMgr::Copy(USHORT nTargetRegion,
                            USHORT nTargetIdx,
                            USHORT nSourceRegion,
                            USHORT nSourceIdx)

/*  [Beschreibung]

    Kopieren einer Dokumentvorlage

    [Parameter]

    USHORT nTargetRegion            Index des Zielbereiches
    USHORT nTargetIdx               Index Zielposition
    USHORT nSourceRegion            Index des Quellbereiches
    USHORT nSourceIdx               Index der zu kopierenden / z uverschiebenden
                                    Dokumentvorlage

    [R"uckgabewert]                 Erfolg (TRUE) oder Mi"serfolg (FALSE)


    [Querverweise]

    <SfxDocumentTemplates::Copy(USHORT nTargetRegion,
                                USHORT nTargetIdx,
                                USHORT nSourceRegion,
                                USHORT nSourceIdx)>

*/

{
    if(nSourceIdx == USHRT_MAX) // keine Verzeichnisse kopieren
        return FALSE ;
    const BOOL bOk = pTemplates->Copy(nTargetRegion, nTargetIdx,
                                        nSourceRegion, nSourceIdx);
    if(bOk)
        bModified = 1;
    return bOk;
}

//-------------------------------------------------------------------------

BOOL    SfxOrganizeMgr::Move(USHORT nTargetRegion,
                            USHORT nTargetIdx,
                            USHORT nSourceRegion,
                            USHORT nSourceIdx)

/*  [Beschreibung]

    Verschieben einer Dokumentvorlage

    [Parameter]

    USHORT nTargetRegion            Index des Zielbereiches
    USHORT nTargetIdx               Index Zielposition
    USHORT nSourceRegion            Index des Quellbereiches
    USHORT nSourceIdx               Index der zu kopierenden / z uverschiebenden
                                    Dokumentvorlage

    [R"uckgabewert]                 Erfolg (TRUE) oder Mi"serfolg (FALSE)


    [Querverweise]

    <SfxDocumentTemplates::Move(USHORT nTargetRegion,
                                USHORT nTargetIdx,
                                USHORT nSourceRegion,
                                USHORT nSourceIdx)>

*/

{
    if(nSourceIdx == USHRT_MAX) // keine Verzeichnisse verschieben
        return FALSE ;
    const BOOL bOk = pTemplates->Move(nTargetRegion, nTargetIdx,
                                        nSourceRegion, nSourceIdx);
    if(bOk)
        bModified = 1;
    return bOk;
}

//-------------------------------------------------------------------------

BOOL    SfxOrganizeMgr::Delete(SfxOrganizeListBox_Impl *pCaller,
                                USHORT nRegion, USHORT nIdx)

/*  [Beschreibung]

    "oschen einer Dokumentvorlage

    [Parameter]

    SfxOrganizeListBox *pCaller     rufende ListBox; da dieses
                                    Event durch das Men"u oder
                                    durch das Keyboard angetriggert wird,
                                    mu"s das Model der ListBox anschlie"send
                                    aktualisiert werden.
    USHORT nRegion                  Index des Bereiches
    USHORT nIdx                     Index der Dokumentvorlage

    [R"uckgabewert]                 Erfolg (TRUE) oder Mi"serfolg (FALSE)


    [Querverweise]

    <SfxDocumentTemplates::Delete(USHORT nRegion, USHORT nIdx)>

*/

{
    BOOL bOk = FALSE;

    if ( USHRT_MAX == nIdx )
    {
        // deleting of a group

        SvLBoxEntry *pGroupToDelete = pCaller->SvLBox::GetEntry(nRegion);
        if ( pGroupToDelete )
        {
            USHORT nItemNum = (USHORT)( pCaller->GetModel()->GetChildCount( pGroupToDelete ) );
            USHORT nToDeleteNum = 0;
            SvLBoxEntry **pEntriesToDelete = new SvLBoxEntry*[nItemNum];

            USHORT nInd = 0;
            for ( nInd = 0; nInd < nItemNum; nInd++ )
                pEntriesToDelete[nInd] = NULL;

            for ( nInd = 0; nInd < nItemNum; nInd++ )
            {
                // TODO/LATER: check that nInd is the same index that is used in pTemplates
                if ( pTemplates->Delete( nRegion, nInd ) )
                {
                    bModified = 1;
                    pEntriesToDelete[nToDeleteNum++] = pCaller->SvLBox::GetEntry( pGroupToDelete, nInd );
                }
            }

            for ( nInd = 0; nInd < nToDeleteNum; nInd++ )
                if ( pEntriesToDelete[nInd] )
                    pCaller->GetModel()->Remove( pEntriesToDelete[nInd] );

            if ( !pCaller->GetModel()->GetChildCount( pGroupToDelete ) )
            {
                bOk = pTemplates->Delete( nRegion, nIdx );
                if ( bOk )
                    pCaller->GetModel()->Remove( pGroupToDelete );
            }
        }
    }
    else
    {
        // deleting of a template
        bOk = pTemplates->Delete(nRegion, nIdx);
        if(bOk)
        {
            bModified = 1;
                // zu loeschender Eintrag
            SvLBoxEntry *pEntryToDelete = pCaller->SvLBox::GetEntry(pCaller->SvLBox::GetEntry(nRegion), nIdx);

            pCaller->GetModel()->Remove(pEntryToDelete);
        }
    }

    return bOk;
}

//-------------------------------------------------------------------------

BOOL    SfxOrganizeMgr::InsertDir
(
    SfxOrganizeListBox_Impl*    pCaller,/*  rufende ListBox; da dieses Event
                                            durch das Men"u oder durch das
                                            Keyboard angetriggert wird,
                                            mu\s das Model der ListBox
                                            anschlie\send aktualisiert werden */
    const String&               rText,  //  logischer Name des Bereiches
    USHORT                      nRegion //  Index des Bereiches
)

/*  [Beschreibung]

    Einf"ugen eines Bereiches


    [R"uckgabewert]

    Erfolg (TRUE) oder Mi\serfolg (FALSE)


    [Querverweise]

    <SfxDocumentTemplates::InsertDir(const String &, USHORT nRegion)>
*/

{
    const BOOL bOk = pTemplates->InsertDir(rText, nRegion);
    if(bOk)
    {
        bModified = 1;
        SvLBoxEntry *pEntry = pCaller->InsertEntry(rText,
                                                   pCaller->GetOpenedBmp(0),
                                                   pCaller->GetClosedBmp(0),
                                                   0, TRUE, nRegion);
        pCaller->Update();
        pCaller->EditEntry(pEntry);
    }
    return bOk;
}

//-------------------------------------------------------------------------

BOOL SfxOrganizeMgr::SetName(const String &rName,
                             USHORT nRegion, USHORT nIdx)

/*  [Beschreibung]

    "Andern eines (logischen) Namens

    [Parameter]

    const String &rName             der neue Name
    USHORT nRegion                  Index des Bereiches
    USHORT nIdx                     Index der Dokumentvorlage

    [R"uckgabewert]                 Erfolg (TRUE) oder Mi"serfolg (FALSE)


    [Querverweise]

    <SfxDocumentTemplates::SetName(const String &, USHORT nRegion, USHORT nIdx)>

*/

{
    const BOOL bOk = pTemplates->SetName(rName, nRegion, nIdx);
    if(bOk)
        bModified = 1;
    return bOk;
}

//-------------------------------------------------------------------------

BOOL SfxOrganizeMgr::CopyTo(USHORT nRegion, USHORT nIdx, const String &rName) const

/*  [Beschreibung]

    Export einer Vorlage

    [Parameter]

    USHORT nRegion                  Index des Bereiches
    USHORT nIdx                     Index der Dokumentvorlage
    const String &rName             Dateiname

    [R"uckgabewert]                 Erfolg (TRUE) oder Mi"serfolg (FALSE)


    [Querverweise]

    <SfxDocumentTemplates::CopyTo( USHORT nRegion, USHORT nIdx, const String &)>

*/

{
    return pTemplates->CopyTo(nRegion, nIdx, rName);
}

//-------------------------------------------------------------------------

BOOL SfxOrganizeMgr::CopyFrom(SfxOrganizeListBox_Impl *pCaller,
                              USHORT nRegion, USHORT nIdx, String &rName)

/*  [Beschreibung]

    Import einer Vorlage

    [Parameter]

    SfxOrganizeListBox *pCaller     rufende ListBox; da dieses
                                    Event durch das Men"u angetriggert wird,
                                    mu"s das Model der ListBox anschlie"send
                                    aktualisiert werden.
    USHORT nRegion                  Index des Bereiches
    USHORT nIdx                     Index der Dokumentvorlage
    String &rName                   Dateiname

    [R"uckgabewert]                 Erfolg (TRUE) oder Mi"serfolg (FALSE)


    [Querverweise]

    <SfxDocumentTemplates::CopyFrom( USHORT nRegion, USHORT nIdx, const String &)>

*/

{
    SvLBoxEntry *pParent = pCaller->FirstSelected();
    if( nIdx!=USHRT_MAX )
        pParent = pCaller->GetParent(pParent);
    if( pTemplates->CopyFrom( nRegion, nIdx, rName ) )
    {
        // pCaller aktualisieren
        if( nIdx == USHRT_MAX )
            nIdx = 0;
        else nIdx++;

        pCaller->InsertEntry( rName,
                              pCaller->GetOpenedBmp(1),
                              pCaller->GetClosedBmp(1),
                              pParent,
                              TRUE,
                              nIdx);
        pCaller->Update();
        // pCaller->EditEntry( pEntry );
        pCaller->Expand( pParent );
        bModified = TRUE;
        return TRUE;
    }
    return FALSE;
}

//-------------------------------------------------------------------------

BOOL SfxOrganizeMgr::InsertFile( SfxOrganizeListBox_Impl* pCaller, const String& rFileName )

/*  [Beschreibung]

    Eine Datei in der Dateiansicht hinzuf"ugen

    [Parameter]

    SfxOrganizeListBox *pCaller     rufende ListBox; da dieses
                                    Event durch das Men"u angetriggert wird,
                                    mu"s das Model der ListBox anschlie"send
                                    aktualisiert werden.
    const String &rFileName         Name der hinzuf"ugenden Datei

    [R"uckgabewert]                 Erfolg (TRUE) oder Mi"serfolg (FALSE)

*/

{
    const CollatorWrapper* pCollator = pImpl->pIntlWrapper->getCaseCollator();
    _FileListEntry* pEntry = new _FileListEntry( rFileName, pCollator );
    if ( pImpl->pDocList->C40_PTR_INSERT( _FileListEntry, pEntry ) )
    {
        USHORT nPos = 0;
        pImpl->pDocList->Seek_Entry( pEntry, &nPos );
        pCaller->InsertEntry( pEntry->aBaseName, pCaller->GetOpenedBmp(1),
                              pCaller->GetClosedBmp(1), 0, TRUE, nPos );
        return TRUE;
    }
    return FALSE;
}

//-------------------------------------------------------------------------

BOOL SfxOrganizeMgr::Rescan()

/*  [Beschreibung]

    Aktualisieren der Datenbasis

    [R"uckgabewert]

    TRUE                    es bestanden Unterschiede
    FALSE                   keine "Anderung

    [Querverweise]

    <SfxDocumentTemplates::Rescan()>
*/

{
    if(pTemplates->Rescan())
    {
        bModified = TRUE;
        return TRUE;
    }
    return FALSE;
}

//-------------------------------------------------------------------------

void SfxOrganizeMgr::SaveAll(Window *pParent)

/*  [Beschreibung]

    Schreiben aller ge"anderten Dokumente

    [Parameter]

    Window *pParent         Parent der Boxen f"ur Fehlermeldungen

*/

{
    USHORT nRangeCount = pTemplates->GetRegionCount();
    USHORT i;
    for(i = 0; i < nRangeCount; ++i)
    {
        if( pTemplates->IsRegionLoaded( i ))
        {
            const USHORT nCount = pTemplates->GetCount(i);
            for(USHORT j = 0; j < nCount; ++j)
            {
                if(!pTemplates->DeleteObjectShell(i, j))
                {
                    String aText = String(SfxResId(STR_ERROR_SAVE_TEMPLATE));
                    aText += pTemplates->GetName(i, j);
                    ErrorBox aBox(pParent,
                                  WinBits(WB_OK_CANCEL | WB_DEF_CANCEL),
                                  aText);
                    if(RET_CANCEL == aBox.Execute())
                        break;
                }
            }
        }
    }
    nRangeCount = pImpl->pDocList->Count();
    for(i = 0; i < nRangeCount; ++i)
    {
        _FileListEntry *pEntry = (*pImpl->pDocList)[i];
        if(!pEntry->DeleteObjectShell())
        {
            String aText(SfxResId(STR_ERROR_SAVE_TEMPLATE));
            aText += pEntry->aBaseName;
            ErrorBox aBox(pParent, WinBits(WB_OK_CANCEL | WB_DEF_CANCEL), aText);
            if(RET_CANCEL == aBox.Execute())
                break;
        }
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
