/*************************************************************************
 *
 *  $RCSfile: orgmgr.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: dv $ $Date: 2001-03-28 14:50:57 $
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

#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _INTN_HXX //autogen
#include <tools/intn.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#include <tools/urlobj.hxx>
#pragma hdrstop

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

//=========================================================================

/*  [Beschreibung]

    Implementierungsklasse; einzelner Eintrag in der Dateiansicht

*/

struct _FileListEntry
{
    String aFileName;           // Dateiname mit komplettem Pfad
    String aBaseName;           // Dateiname
    International aInter;
    SfxObjectShellLock aDocShell; // ObjectShell als Ref-Klasse
    SvStorageRef aStor;         // Referenz auf Storage, wenn wir diesen geoeffnet haben
    BOOL bFile;                 // als Datei auf Platte
                                // (!= unbenannt1, nicht als Dok. geladen;
                                // diese werden nicht gespeichert!)
    BOOL bOwner;                // selbst erzeugt
    BOOL bNoName;

    _FileListEntry( const String& rFileName,
                    const International& rInter, const String* pTitle = NULL );
    ~_FileListEntry();

    int operator==( const _FileListEntry &rCmp) const;
    int operator< ( const _FileListEntry &rCmp) const;
    BOOL DeleteObjectShell();
};

//-------------------------------------------------------------------------

inline int _FileListEntry::operator==(const _FileListEntry &rCmp) const
{
    return COMPARE_EQUAL == aInter.Compare(aBaseName, rCmp.aBaseName);
}

//-------------------------------------------------------------------------

inline int _FileListEntry::operator< (const _FileListEntry &rCmp) const
{
    return COMPARE_LESS == aInter.Compare(aBaseName, rCmp.aBaseName);
}

//-------------------------------------------------------------------------

_FileListEntry::_FileListEntry( const String& rFileName,
                                const International& rInter, const String* pTitle ) :

    aFileName   ( rFileName ),
    aInter      ( rInter ),
    bFile       ( FALSE ),
    bOwner      ( FALSE ),
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
        if(aStor.Is())
        {
            if(!aDocShell->Save() )
                bRet = FALSE;
            else
            {
                aStor->Commit();
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
                        aTitle, aDocShell->GetFactory().GetFilter(0)->GetName(), 0 );
        }
    }
    if( bOwner)
    {
        aDocShell.Clear();
        aStor.Clear();
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
    pLeftBox(pLeft),
    pRightBox(pRight),
    pTemplates(pTempl? pTempl: new SfxDocumentTemplates),
    pDocList(new SfxObjectList),
    bModified(0),
    bDeleteTemplates(pTempl == 0)

/*  [Beschreibung]

    Konstruktor

    Das aktuelle Dokument wird in die Liste der Dokumente
    aufgenommen.

*/
{
    const International aInter( GetpApp()->GetAppInternational() );
    for ( SfxObjectShell* pTmp = SfxObjectShell::GetFirst(); pTmp; pTmp = SfxObjectShell::GetNext(*pTmp) )
    {
        if ( pTmp->GetCreateMode() != SFX_CREATE_MODE_STANDARD ||
             !( pTmp->GetFlags() & SFXOBJECTSHELL_HASOPENDOC ) || !pTmp->GetStyleSheetPool() )
            continue;
        _FileListEntry* pNewEntry = NULL;
        BOOL bHasLongName = pTmp->GetMedium()->GetLongName().Len() != 0;
        String aTitle = pTmp->GetTitle( SFX_TITLE_TITLE );
        pNewEntry = new _FileListEntry( pTmp->GetMedium()->GetName(), aInter, &aTitle );
        pNewEntry->aDocShell = pTmp;
#if defined( SOLARIS )
        pDocList->Insert( (_FileListEntry const *)pNewEntry );
#else
#if defined( WTC) || ( defined( IRIX ) && defined( C700 ) ) || defined( ICC ) || defined ( HPUX )
        pDocList->Insert( (_FileListEntry const *&) pNewEntry );
#else
        pDocList->Insert( pNewEntry );
#endif
#endif
    }
}

//-------------------------------------------------------------------------

SfxOrganizeMgr::~SfxOrganizeMgr()
{
    if(bDeleteTemplates)
        delete pTemplates;
    delete pDocList;
    pLeftBox = pRightBox = 0;
}

//-------------------------------------------------------------------------

SfxObjectShellRef SfxOrganizeMgr::CreateObjectShell( USHORT nIdx )

/*  [Beschreibung]

    Zugriff auf die DokumentShell an der Position nIdx

    [Returnwert]            Referenz auf die DokumentShell

*/

{
    _FileListEntry* pEntry = (*pDocList)[nIdx];
    // andernfalls Doc-Shell anlegen
    if ( !pEntry->aDocShell.Is() )
    {
//(mba)/task        SfxWaitCursor aWaitCursor;
        INetURLObject aFileObj( pEntry->aFileName );
        BOOL bDum = FALSE;
        SfxApplication* pSfxApp = SFX_APP();
        String aFilePath = aFileObj.GetMainURL();
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
                pEntry->aStor = 0;
                delete pMed;
                if ( pEntry->aDocShell.Is() )
                    return (SfxObjectShellRef)(SfxObjectShell*)(pEntry->aDocShell);
            }
            else
            {
                delete pMed;
                if ( pFilter )
                {
                    pEntry->aDocShell = (SfxObjectShell *)
                        ((SfxFactoryFilterContainer*)pFilter->GetFilterContainer())
                        ->GetFactory().CreateObject(SFX_CREATE_MODE_ORGANIZER);
                }

#if SUPD<583 //(mba)
                if ( !pEntry->aDocShell.Is() )
                    // Config-Files
                    pEntry->aDocShell = new SfxGenericObjectShell( SFX_CREATE_MODE_ORGANIZER );
#else
                if ( !pEntry->aDocShell.Is() )
                    return NULL;
#endif
                pEntry->aStor = new SvStorage( aFilePath,
                                                STREAM_READWRITE |
                                                STREAM_NOCREATE  |
                                                STREAM_SHARE_DENYALL,
                                                STORAGE_TRANSACTED );
                // kein Storage-File oder andere Fehler beim Oeffnen
                if( SVSTREAM_OK == pEntry->aStor->GetError() )
                {
                    if ( pEntry->aDocShell.Is() )
                    {
                        String aOldBaseURL = INetURLObject::GetBaseURL();
                        pEntry->aDocShell->DoInitNew(0);
                        INetURLObject::SetBaseURL( pEntry->aDocShell->GetMedium()->GetName() );
                        pEntry->aDocShell->LoadFrom(pEntry->aStor);
                        pEntry->aDocShell->DoHandsOff();
                        pEntry->aDocShell->DoSaveCompleted(pEntry->aStor);
                        INetURLObject::SetBaseURL( aOldBaseURL );
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
    return (*pDocList)[nIdx]->DeleteObjectShell();
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
        pCaller->EditEntry( pEntry );
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
    const International aInter( GetpApp()->GetAppInternational() );
    _FileListEntry* pEntry = new _FileListEntry( rFileName, aInter );
#if defined( SOLARIS )
    if ( pDocList->Insert( (_FileListEntry const *)pEntry ) )
#else
#if defined( WTC ) || ( defined( IRIX ) && defined( C700 ) ) || defined( ICC ) || defined ( HPUX )
    if ( pDocList->Insert( (_FileListEntry const *&)pEntry ) )
#else
    if ( pDocList->Insert( pEntry ) )
#endif
#endif
    {
        USHORT nPos = 0;
        pDocList->Seek_Entry( pEntry, &nPos );
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
    nRangeCount = pDocList->Count();
    for(i = 0; i < nRangeCount; ++i)
    {
        _FileListEntry *pEntry = (*pDocList)[i];
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


