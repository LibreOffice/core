/*************************************************************************
 *
 *  $RCSfile: tablink.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: jp $ $Date: 2001-03-08 20:49:42 $
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

#ifdef WNT
#pragma optimize("",off)
#endif

//------------------------------------------------------------------

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------

#include <sfx2/app.hxx>
#include <svtools/itemset.hxx>
#include <svtools/stritem.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <svx/linkmgr.hxx>
#include <tools/urlobj.hxx>

#include "tablink.hxx"

#include "scextopt.hxx"
#include "table.hxx"
#include "document.hxx"
#include "docsh.hxx"
#include "globstr.hrc"
#include "undoblk.hxx"
#include "undotab.hxx"
#include "global.hxx"
#include "hints.hxx"

TYPEINIT1(ScTableLink, ::so3::SvBaseLink);

//------------------------------------------------------------------------

ScTableLink::ScTableLink(ScDocShell* pDocSh, const String& rFile,
                            const String& rFilter, const String& rOpt):
    ::so3::SvBaseLink(LINKUPDATE_ONCALL,FORMAT_FILE),
    pDocShell(pDocSh),
    aFileName(rFile),
    aFilterName(rFilter),
    aOptions(rOpt),
    bInCreate( FALSE ),
    bAddUndo( TRUE ),
    bDoPaint( TRUE )
{
}

ScTableLink::ScTableLink(SfxObjectShell* pShell, const String& rFile,
                            const String& rFilter, const String& rOpt):
    ::so3::SvBaseLink(LINKUPDATE_ONCALL,FORMAT_FILE),
    pDocShell((ScDocShell*)pShell),
    aFileName(rFile),
    aFilterName(rFilter),
    aOptions(rOpt),
    bInCreate( FALSE ),
    bAddUndo( TRUE ),
    bDoPaint( TRUE )
{
}

__EXPORT ScTableLink::~ScTableLink()
{
    // Verbindung aufheben

    String aEmpty;
    ScDocument* pDoc = pDocShell->GetDocument();
    USHORT nCount = pDoc->GetTableCount();
    for (USHORT nTab=0; nTab<nCount; nTab++)
        if (pDoc->IsLinked(nTab) && pDoc->GetLinkDoc(nTab)==aFileName)
            pDoc->SetLink( nTab, SC_LINK_NONE, aEmpty, aEmpty, aEmpty, aEmpty );
}

BOOL __EXPORT ScTableLink::Edit(Window* pParent)
{
    //  DefModalDialogParent setzen, weil evtl. aus der DocShell beim ConvertFrom
    //  ein Optionen-Dialog kommt...

    Window* pOldParent = Application::GetDefDialogParent();
    if (pParent)
        Application::SetDefDialogParent(pParent);

    BOOL bRet = SvBaseLink::Edit(pParent);

    Application::SetDefDialogParent(pOldParent);

    return bRet;
}

void __EXPORT ScTableLink::DataChanged( const String&,
                                        const ::com::sun::star::uno::Any& )
{
    SvxLinkManager* pLinkManager=pDocShell->GetDocument()->GetLinkManager();
    if (pLinkManager!=NULL)
    {
        String aFile;
        String aFilter;
        pLinkManager->GetDisplayNames( this,0,&aFile,NULL,&aFilter);

        //  the file dialog returns the filter name with the application prefix
        //  -> remove prefix
        ScDocumentLoader::RemoveAppPrefix( aFilter );

        if (!bInCreate)
            Refresh(aFile,aFilter);         // nicht doppelt laden!
    }
}

void __EXPORT ScTableLink::Closed()
{
    // Verknuepfung loeschen: Undo
    ScDocument* pDoc = pDocShell->GetDocument();
    BOOL bUndo (pDoc->IsUndoEnabled());

    if (bAddUndo && bUndo)
    {
        pDocShell->GetUndoManager()->AddUndoAction(
                new ScUndoRemoveLink( pDocShell, aFileName ) );

        bAddUndo = FALSE;   // nur einmal
    }

    // Verbindung wird im dtor aufgehoben

    SvBaseLink::Closed();
}

BOOL ScTableLink::IsUsed() const
{
    return pDocShell->GetDocument()->HasLink( aFileName, aFilterName, aOptions );
}

BOOL ScTableLink::Refresh(const String& rNewFile, const String& rNewFilter,
                            const String* pNewOptions)
{
    //  Dokument laden

    if (!rNewFile.Len() || !rNewFilter.Len())
        return FALSE;

    String aNewUrl( ScGlobal::GetAbsDocName( rNewFile, pDocShell ) );
    BOOL bNewUrlName = (aNewUrl != aFileName);

    const SfxFilter* pFilter = SFX_APP()->GetFilter( pDocShell->GetFactory(), rNewFilter );
    if (!pFilter)
        return FALSE;

    ScDocument* pDoc = pDocShell->GetDocument();
    pDoc->SetInLinkUpdate( TRUE );

    BOOL bUndo(pDoc->IsUndoEnabled());

    //  wenn neuer Filter ausgewaehlt wurde, Optionen vergessen
    if ( rNewFilter != aFilterName )
        aOptions.Erase();
    if ( pNewOptions )                  // Optionen hart angegeben?
        aOptions = *pNewOptions;

    //  ItemSet immer anlegen, damit die DocShell die Optionen setzen kann
    SfxItemSet* pSet = new SfxAllItemSet( SFX_APP()->GetPool() );
    if ( aOptions.Len() )
        pSet->Put( SfxStringItem( SID_FILE_FILTEROPTIONS, aOptions ) );

    SfxMedium* pMed = new SfxMedium(aNewUrl, STREAM_STD_READ, FALSE, pFilter, pSet);

    ScDocShell* pSrcShell = new ScDocShell(SFX_CREATE_MODE_INTERNAL);
    SvEmbeddedObjectRef aRef = pSrcShell;
    pSrcShell->DoLoad(pMed);

    // Optionen koennten gesetzt worden sein
    String aNewOpt = ScDocumentLoader::GetOptions(*pMed);
    if (!aNewOpt.Len())
        aNewOpt = aOptions;

    //  Undo...

    ScDocument* pUndoDoc = NULL;
    BOOL bFirst = TRUE;
    if (bAddUndo && bUndo)
        pUndoDoc = new ScDocument( SCDOCMODE_UNDO );

    //  Tabellen kopieren

    ScDocShellModificator aModificator( *pDocShell );

    BOOL bNotFound = FALSE;
    ScDocument* pSrcDoc = pSrcShell->GetDocument();

    //  #74835# from text filters that don't set the table name,
    //  use the one table regardless of link table name
    BOOL bAutoTab = (pSrcDoc->GetTableCount() == 1) &&
                    ScDocShell::HasAutomaticTableName( rNewFilter );

    USHORT nCount = pDoc->GetTableCount();
    for (USHORT nTab=0; nTab<nCount; nTab++)
    {
        BYTE nMode = pDoc->GetLinkMode(nTab);
        if (nMode && pDoc->GetLinkDoc(nTab)==aFileName)
        {
            String aTabName = pDoc->GetLinkTab(nTab);

            //  Undo

            if (bAddUndo && bUndo)
            {
                if (bFirst)
                    pUndoDoc->InitUndo( pDoc, nTab, nTab, TRUE, TRUE );
                else
                    pUndoDoc->AddUndoTab( nTab, nTab, TRUE, TRUE );
                bFirst = FALSE;
                ScRange aRange(0,0,nTab,MAXCOL,MAXROW,nTab);
                pDoc->CopyToDocument(aRange, IDF_ALL, FALSE, pUndoDoc);
                pUndoDoc->TransferDrawPage( pDoc, nTab, nTab );
                pUndoDoc->SetLink( nTab, nMode, aFileName, aFilterName, aOptions, aTabName );
            }

            //  Tabellenname einer ExtDocRef anpassen

            if ( bNewUrlName && nMode == SC_LINK_VALUE )
            {
                String aName;
                pDoc->GetName( nTab, aName );
                if ( ScGlobal::pScInternational->CompareEqual(
                        ScGlobal::GetDocTabName( aFileName, aTabName ), aName,
                        INTN_COMPARE_IGNORECASE ) )
                {
                    pDoc->RenameTab( nTab,
                        ScGlobal::GetDocTabName( aNewUrl, aTabName ),
                        FALSE, TRUE );  // kein RefUpdate, kein ValidTabName
                }
            }

            //  kopieren

            USHORT nSrcTab = 0;
            BOOL bFound = TRUE;         // kein Tab-Name angegeben: immer die erste
            if ( aTabName.Len() && !bAutoTab )
                bFound = pSrcDoc->GetTable( aTabName, nSrcTab );
            if (bFound)
                pDoc->TransferTab( pSrcDoc, nSrcTab, nTab, FALSE,       // nicht neu einfuegen
                                        (nMode == SC_LINK_VALUE) );     // nur Werte?
            else
            {
                pDoc->DeleteAreaTab( 0,0,MAXCOL,MAXROW, nTab, IDF_ALL );
//              pDoc->ClearDrawPage(nTab);
                //  Fehler eintragen
                pDoc->SetString( 0,0,nTab, ScGlobal::GetRscString(STR_LINKERROR) );
                pDoc->SetString( 0,1,nTab, ScGlobal::GetRscString(STR_LINKERRORFILE) );
                pDoc->SetString( 1,1,nTab, aNewUrl );
                pDoc->SetString( 0,2,nTab, ScGlobal::GetRscString(STR_LINKERRORTAB) );
                pDoc->SetString( 1,2,nTab, aTabName );

                bNotFound = TRUE;
            }

            if ( bNewUrlName || rNewFilter != aFilterName || aNewOpt != aOptions || pNewOptions )
                pDoc->SetLink( nTab, nMode, aNewUrl, rNewFilter, aNewOpt, aTabName );
        }
    }

    //  neue Einstellungen merken

    if ( bNewUrlName )
        aFileName = aNewUrl;
    if ( rNewFilter != aFilterName )
        aFilterName = rNewFilter;
    if ( aNewOpt != aOptions )
        aOptions = aNewOpt;

    //  aufraeumen

//  pSrcShell->DoClose();
    aRef->DoClose();

    //  Undo

    if (bAddUndo && bUndo)
        pDocShell->GetUndoManager()->AddUndoAction(
                    new ScUndoRefreshLink( pDocShell, pUndoDoc ) );

    //  Paint (koennen mehrere Tabellen sein)

    if (bDoPaint)
    {
        pDocShell->PostPaint( ScRange(0,0,0,MAXCOL,MAXROW,MAXTAB),
                                PAINT_GRID | PAINT_TOP | PAINT_LEFT );
        aModificator.SetDocumentModified();
    }

    if (bNotFound)
    {
        //! Fehler ausgeben ?
    }

    pDoc->SetInLinkUpdate( FALSE );

    //  notify Uno objects (for XRefreshListener)
    //! also notify Uno objects if file name was changed!
    ScLinkRefreshedHint aHint;
    aHint.SetSheetLink( aFileName );
    pDoc->BroadcastUno( aHint );

    return TRUE;
}

// === ScDocumentLoader ==================================================

String ScDocumentLoader::GetOptions( SfxMedium& rMedium )       // static
{
    SfxItemSet* pSet = rMedium.GetItemSet();
    const SfxPoolItem* pItem;
    if ( pSet && SFX_ITEM_SET == pSet->GetItemState( SID_FILE_FILTEROPTIONS, TRUE, &pItem ) )
        return ((const SfxStringItem*)pItem)->GetValue();

    return EMPTY_STRING;
}

void ScDocumentLoader::GetFilterName( const String& rFileName,
                                    String& rFilter, String& rOptions )         // static
{
    TypeId aScType = TYPE(ScDocShell);
    SfxObjectShell* pDocSh = SfxObjectShell::GetFirst( &aScType );
    while ( pDocSh )
    {
        if ( pDocSh->HasName() )
        {
            SfxMedium* pMed = pDocSh->GetMedium();
            if ( rFileName == pMed->GetName() )
            {
                rFilter = pMed->GetFilter()->GetFilterName();
                rOptions = GetOptions(*pMed);
                return;
            }
        }
        pDocSh = SfxObjectShell::GetNext( *pDocSh, &aScType );
    }

    //  Filter-Detection

    const SfxFilter* pSfxFilter = NULL;
    SfxMedium* pMedium = new SfxMedium( rFileName, STREAM_STD_READ, FALSE );
    if ( pMedium->GetError() == ERRCODE_NONE )
    {
        SfxFilterMatcher aMatcher( ScDocShell::Factory().GetFilterContainer() );
        aMatcher.GuessFilter( *pMedium, &pSfxFilter );
    }

    if ( pSfxFilter )
        rFilter = pSfxFilter->GetFilterName();
    else
        rFilter = ScDocShell::GetOwnFilterName();       //  sonst Calc-Datei

    delete pMedium;
}

void ScDocumentLoader::RemoveAppPrefix( String& rFilterName )       // static
{
    String aAppPrefix = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM( STRING_SCAPP ));
    aAppPrefix.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ": " ));
    xub_StrLen nPreLen = aAppPrefix.Len();
    if ( rFilterName.Copy(0,nPreLen) == aAppPrefix )
        rFilterName.Erase(0,nPreLen);
}

ScDocumentLoader::ScDocumentLoader( const String& rFileName,
                                    String& rFilterName, String& rOptions,
                                    UINT32 nRekCnt ) :
        pDocShell(0),
        pMedium(0)
{
    if ( !rFilterName.Len() )
        GetFilterName( rFileName, rFilterName, rOptions );

    const SfxFilter* pFilter = SFX_APP()->GetFilter(
                                ScDocShell::Factory(), rFilterName );

    //  ItemSet immer anlegen, damit die DocShell die Optionen setzen kann
    SfxItemSet* pSet = new SfxAllItemSet( SFX_APP()->GetPool() );
    if ( rOptions.Len() )
        pSet->Put( SfxStringItem( SID_FILE_FILTEROPTIONS, rOptions ) );

    pMedium = new SfxMedium( rFileName, STREAM_STD_READ, FALSE, pFilter, pSet );
    if ( pMedium->GetError() != ERRCODE_NONE )
        return ;

    pDocShell = new ScDocShell( SFX_CREATE_MODE_INTERNAL );
    aRef = pDocShell;

    ScDocument* pDoc = pDocShell->GetDocument();
    if( pDoc )
    {
        ScExtDocOptions*    pExtDocOpt = pDoc->GetExtDocOptions();
        if( !pExtDocOpt )
        {
            pExtDocOpt = new ScExtDocOptions;
            pDoc->SetExtDocOptions( pExtDocOpt );
        }
        pExtDocOpt->nLinkCnt = nRekCnt;
    }

    pDocShell->DoLoad( pMedium );

    String aNew = GetOptions(*pMedium);         // Optionen werden beim Laden per Dialog gesetzt
    if (aNew.Len() && aNew != rOptions)
        rOptions = aNew;
}

ScDocumentLoader::~ScDocumentLoader()
{
/*  if ( pDocShell )
        pDocShell->DoClose();
*/
    if ( aRef.Is() )
        aRef->DoClose();
    else if ( pMedium )
        delete pMedium;
}

ScDocument* ScDocumentLoader::GetDocument()
{
    return pDocShell ? pDocShell->GetDocument() : 0;
}

BOOL ScDocumentLoader::IsError() const
{
    if ( pDocShell && pMedium )
        return pMedium->GetError() != ERRCODE_NONE;
    else
        return TRUE;
}

String ScDocumentLoader::GetTitle() const
{
    if ( pDocShell )
        return pDocShell->GetTitle();
    else
        return EMPTY_STRING;
}



