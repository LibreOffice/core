/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: orgmgr.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:29:10 $
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

#ifndef _COM_SUN_STAR_EMBED_XSTORAGE_HPP_
#include <com/sun/star/embed/XStorage.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XTRANSACTEDOBJECT_HPP_
#include <com/sun/star/embed/XTransactedObject.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif

#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#include <tools/urlobj.hxx>
#ifndef GCC
#pragma hdrstop
#endif

#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif
#ifndef _UNOTOOLS_INTLWRAPPER_HXX
#include <unotools/intlwrapper.hxx>
#endif

#include <comphelper/storagehelper.hxx>

#include "app.hxx"
#include "objsh.hxx"
#include "docfile.hxx"
#include "docfac.hxx"
#include "doctempl.hxx"
#include "docvor.hxx"
#include "orgmgr.hxx"
#include "sfxtypes.hxx"
#include "sfxresid.hxx"
#include "view.hrc"
#include "docfilt.hxx"
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
    bOwnFormat  ( TRUE ),
    bNoName     ( TRUE )
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
//(mba)/task        SfxWaitCursor aWaitCursor;
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
            if( pSfxApp->GetFilterMatcher().GuessFilter( *pMed, &pFilter, SFX_FILTER_TEMPLATE, 0 ) ||
                pFilter && !pFilter->IsOwnFormat() ||
                pFilter && !pFilter->UsesStorage() )
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
//(mba)/task    SfxWaitCursor aWaitCursor;
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
    const BOOL bOk = pTemplates->Delete(nRegion, nIdx);
    if(bOk)
    {
        bModified = 1;
            // zu loeschender Eintrag
        SvLBoxEntry *pEntryToDelete = USHRT_MAX == nIdx?    // Verzeichnis
            pCaller->SvLBox::GetEntry(nRegion):
            pCaller->SvLBox::GetEntry(pCaller->SvLBox::GetEntry(nRegion), nIdx);

        pCaller->GetModel()->Remove(pEntryToDelete);
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

        SvLBoxEntry *pEntry = pCaller->InsertEntry( rName,
                                                    pCaller->GetOpenedBmp(1),
                                                    pCaller->GetClosedBmp(1),
                                                    pParent, TRUE, nIdx);
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


