/*************************************************************************
 *
 *  $RCSfile: arealink.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: dr $ $Date: 2001-04-05 10:48:47 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------

#include <sfx2/app.hxx>
#include <sfx2/docfile.hxx>
#include <svx/linkmgr.hxx>
#include <svtools/stritem.hxx>
#include <vcl/msgbox.hxx>

#include "arealink.hxx"

#include "tablink.hxx"
#include "document.hxx"
#include "docsh.hxx"
#include "rangenam.hxx"
#include "dbcolect.hxx"
#include "undoblk.hxx"
#include "globstr.hrc"
#include "markdata.hxx"
#include "hints.hxx"

#include "attrib.hxx"           // raus, wenn ResetAttrib am Dokument
#include "patattr.hxx"          // raus, wenn ResetAttrib am Dokument
#include "docpool.hxx"          // raus, wenn ResetAttrib am Dokument

TYPEINIT1(ScAreaLink,::so3::SvBaseLink);

//------------------------------------------------------------------------

ScAreaLink::ScAreaLink( SfxObjectShell* pShell, const String& rFile,
                        const String& rFilter, const String& rOpt,
                        const String& rArea, const ScRange& rDest,
                        ULONG nRefresh ) :
    ::so3::SvBaseLink(LINKUPDATE_ONCALL,FORMAT_FILE),
    pDocShell       ((ScDocShell*)pShell),
    aFileName       (rFile),
    aFilterName     (rFilter),
    aOptions        (rOpt),
    aSourceArea     (rArea),
    aDestArea       (rDest),
    nRefreshDelay   (nRefresh),
    bAddUndo        (TRUE),
    bInCreate       (FALSE),
    bDoInsert       (TRUE)
{
    DBG_ASSERT(pShell->ISA(ScDocShell), "ScAreaLink mit falscher ObjectShell");
}

__EXPORT ScAreaLink::~ScAreaLink()
{
    // Verbindung aufheben
}

BOOL __EXPORT ScAreaLink::Edit(Window* pParent)
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

void __EXPORT ScAreaLink::DataChanged( const String&,
                                       const ::com::sun::star::uno::Any& )
{
    //  bei bInCreate nichts tun, damit Update gerufen werden kann, um den Status im
    //  LinkManager zu setzen, ohne die Daten im Dokument zu aendern

    if (bInCreate)
        return;

    SvxLinkManager* pLinkManager=pDocShell->GetDocument()->GetLinkManager();
    if (pLinkManager!=NULL)
    {
        String aFile;
        String aFilter;
        String aArea;
        pLinkManager->GetDisplayNames( this,0,&aFile,&aArea,&aFilter);

        //  the file dialog returns the filter name with the application prefix
        //  -> remove prefix
        ScDocumentLoader::RemoveAppPrefix( aFilter );

        // #81155# dialog doesn't set area, so keep old one
        if ( !aArea.Len() )
        {
            aArea = aSourceArea;

            // adjust in dialog:
            String aLinkName;
            MakeLnkName( aLinkName, NULL, aFile, aArea, &aFilter );
            SetName( aLinkName );
        }

        Refresh( aFile, aFilter, aArea, nRefreshDelay );
    }
}

void __EXPORT ScAreaLink::Closed()
{
    // Verknuepfung loeschen: Undo

    ScDocument* pDoc = pDocShell->GetDocument();
    BOOL bUndo (pDoc->IsUndoEnabled());
    if (bAddUndo && bUndo)
    {
        pDocShell->GetUndoManager()->AddUndoAction( new ScUndoRemoveAreaLink( pDocShell,
                                                        aFileName, aFilterName, aOptions,
                                                        aSourceArea, aDestArea, nRefreshDelay ) );

        bAddUndo = FALSE;   // nur einmal
    }

    SvBaseLink::Closed();
}

void ScAreaLink::SetDestArea(const ScRange& rNew)
{
    aDestArea = rNew;           // fuer Undo
}

void ScAreaLink::SetSource(const String& rDoc, const String& rFlt, const String& rOpt,
                                const String& rArea)
{
    aFileName   = rDoc;
    aFilterName = rFlt;
    aOptions    = rOpt;
    aSourceArea = rArea;
}

BOOL ScAreaLink::IsEqual( const String& rFile, const String& rFilter, const String& rOpt,
                            const String& rSource, const ScRange& rDest ) const
{
    return aFileName == rFile && aFilterName == rFilter && aOptions == rOpt &&
            aSourceArea == rSource && aDestArea == rDest;
}

//  ausfuehren:

BOOL ScAreaLink::Refresh( const String& rNewFile, const String& rNewFilter,
                            const String& rNewArea, ULONG nNewRefresh )
{
    //  Dokument laden - wie TabLink

    if (!rNewFile.Len() || !rNewFilter.Len())
        return FALSE;

    String aNewUrl( ScGlobal::GetAbsDocName( rNewFile, pDocShell ) );
    BOOL bNewUrlName = (aNewUrl != aFileName);

    const SfxFilter* pFilter = SFX_APP()->GetFilter( pDocShell->GetFactory(), rNewFilter );
    if (!pFilter)
        return FALSE;

    ScDocument* pDoc = pDocShell->GetDocument();

    BOOL bUndo (pDoc->IsUndoEnabled());
    pDoc->SetInLinkUpdate( TRUE );

    //  wenn neuer Filter ausgewaehlt wurde, Optionen vergessen
    if ( rNewFilter != aFilterName )
        aOptions.Erase();

    //  ItemSet immer anlegen, damit die DocShell die Optionen setzen kann
    SfxItemSet* pSet = new SfxAllItemSet( SFX_APP()->GetPool() );
    if ( aOptions.Len() )
        pSet->Put( SfxStringItem( SID_FILE_FILTEROPTIONS, aOptions ) );

    SfxMedium* pMed = new SfxMedium(aNewUrl, STREAM_STD_READ, FALSE, pFilter);

    ScDocShell* pSrcShell = new ScDocShell(SFX_CREATE_MODE_INTERNAL);
    SvEmbeddedObjectRef aRef = pSrcShell;
    pSrcShell->DoLoad(pMed);

    // Optionen koennten gesetzt worden sein
    String aNewOpt = ScDocumentLoader::GetOptions(*pMed);
    if (!aNewOpt.Len())
        aNewOpt = aOptions;

    //  Bereich suchen

    BOOL bFound = FALSE;
    ScRange aSourceRange;
    ScDocument* pSrcDoc = pSrcShell->GetDocument();
    ScRangeName* pNames = pSrcDoc->GetRangeName();
    USHORT nPos;
    if (pNames)         // benannte Bereiche
    {
        if (pNames->SearchName( rNewArea, nPos ))
            if ( (*pNames)[nPos]->IsReference( aSourceRange ) )
                bFound = TRUE;
    }
    if (!bFound)        // Datenbankbereiche
    {
        ScDBCollection* pDBColl = pSrcDoc->GetDBCollection();
        if (pDBColl)
            if (pDBColl->SearchName( rNewArea, nPos ))
            {
                USHORT nTab,nCol1,nRow1,nCol2,nRow2;
                (*pDBColl)[nPos]->GetArea(nTab,nCol1,nRow1,nCol2,nRow2);
                aSourceRange = ScRange( nCol1,nRow1,nTab, nCol2,nRow2,nTab );
                bFound = TRUE;
            }
    }
    if (!bFound)        // direct reference (range or cell)
    {
        if ( aSourceRange.ParseAny( rNewArea, pSrcDoc ) & SCA_VALID )
            bFound = TRUE;
    }

    //  alte Daten loeschen / neue kopieren

    ScAddress aDestPos = aDestArea.aStart;
    USHORT nDestTab = aDestPos.Tab();
    ScRange aOldRange = aDestArea;
    ScRange aNewRange = aDestArea;          // alter Bereich, wenn Datei nicht gefunden o.ae.
    if (bFound)
        aNewRange.aEnd = ScAddress(
                aSourceRange.aEnd.Col() - aSourceRange.aStart.Col() + aDestPos.Col(),
                aSourceRange.aEnd.Row() - aSourceRange.aStart.Row() + aDestPos.Row(),
                nDestTab );


    BOOL bCanDo = pDoc->CanFitBlock( aOldRange, aNewRange );    //! nach bDoInsert unterscheiden
    if (bCanDo)
    {
        ScDocShellModificator aModificator( *pDocShell );

        USHORT nStartX = aDestPos.Col();
        USHORT nStartY = aDestPos.Row();
        USHORT nOldEndX = aOldRange.aEnd.Col();
        USHORT nOldEndY = aOldRange.aEnd.Row();
        USHORT nNewEndX = aNewRange.aEnd.Col();
        USHORT nNewEndY = aNewRange.aEnd.Row();
        ScRange aMaxRange( aDestPos,
                    ScAddress(Max(nOldEndX,nNewEndX), Max(nOldEndY,nNewEndY), nDestTab) );

        //  Undo initialisieren

        ScDocument* pUndoDoc = NULL;
        ScDocument* pRedoDoc = NULL;
        if ( bAddUndo && bUndo )
        {
            pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            if ( bDoInsert )
            {
                if ( nNewEndX != nOldEndX || nNewEndY != nOldEndY )             // Bereich veraendert?
                {
                    pUndoDoc->InitUndo( pDoc, 0, pDoc->GetTableCount()-1 );
                    pDoc->CopyToDocument( 0,0,0,MAXCOL,MAXROW,MAXTAB,
                                            IDF_FORMULA, FALSE, pUndoDoc );     // alle Formeln
                }
                else
                    pUndoDoc->InitUndo( pDoc, nDestTab, nDestTab );             // nur Zieltabelle
                pDoc->CopyToDocument( aOldRange, IDF_ALL, FALSE, pUndoDoc );
            }
            else        // ohne Einfuegen
            {
                pUndoDoc->InitUndo( pDoc, nDestTab, nDestTab );             // nur Zieltabelle
                pDoc->CopyToDocument( aMaxRange, IDF_ALL, FALSE, pUndoDoc );
            }
        }

        //  Zellen einfuegen / loeschen
        //  DeleteAreaTab loescht auch MERGE_FLAG Attribute

        if (bDoInsert)
            pDoc->FitBlock( aOldRange, aNewRange );         // incl. loeschen
        else
            pDoc->DeleteAreaTab( aMaxRange, IDF_ALL );

        //  Daten kopieren

        if (bFound)
        {
            USHORT nSrcTab = aSourceRange.aStart.Tab();
            ScMarkData aSourceMark;
            aSourceMark.SelectOneTable( nSrcTab );      // selektieren fuer CopyToClip
            aSourceMark.SetMarkArea( aSourceRange );

            ScDocument* pClipDoc = new ScDocument( SCDOCMODE_CLIP );
            pSrcDoc->CopyToClip( aSourceRange.aStart.Col(),aSourceRange.aStart.Row(),
                                 aSourceRange.aEnd.Col(),aSourceRange.aEnd.Row(),
                                 FALSE, pClipDoc, FALSE, &aSourceMark );

            if ( pClipDoc->HasAttrib( 0,0,nSrcTab, MAXCOL,MAXROW,nSrcTab,
                                        HASATTR_MERGED | HASATTR_OVERLAPPED ) )
            {
                //! ResetAttrib am Dokument !!!

                ScPatternAttr aPattern( pSrcDoc->GetPool() );
                aPattern.GetItemSet().Put( ScMergeAttr() );             // Defaults
                aPattern.GetItemSet().Put( ScMergeFlagAttr() );
                pClipDoc->ApplyPatternAreaTab( 0,0, MAXCOL,MAXROW, nSrcTab, aPattern );
            }

            ScMarkData aDestMark;
            aDestMark.SelectOneTable( nDestTab );
            aDestMark.SetMarkArea( aNewRange );
            pDoc->CopyFromClip( aNewRange, aDestMark, IDF_ALL, NULL, pClipDoc, FALSE );

            delete pClipDoc;
        }
        else
        {
            String aErr = ScGlobal::GetRscString(STR_LINKERROR);
            pDoc->SetString( aDestPos.Col(), aDestPos.Row(), aDestPos.Tab(), aErr );
        }

        //  Undo eintragen

        if ( bAddUndo && bUndo)
        {
            pRedoDoc = new ScDocument( SCDOCMODE_UNDO );
            pRedoDoc->InitUndo( pDoc, nDestTab, nDestTab );
            pDoc->CopyToDocument( aNewRange, IDF_ALL, FALSE, pRedoDoc );

            pDocShell->GetUndoManager()->AddUndoAction(
                new ScUndoUpdateAreaLink( pDocShell,
                                            aFileName, aFilterName, aOptions,
                                            aSourceArea, aOldRange, nRefreshDelay,
                                            aNewUrl, rNewFilter, aNewOpt,
                                            rNewArea, aNewRange, nNewRefresh,
                                            pUndoDoc, pRedoDoc, bDoInsert ) );
        }

        //  neue Einstellungen merken

        if ( bNewUrlName )
            aFileName = aNewUrl;
        if ( rNewFilter != aFilterName )
            aFilterName = rNewFilter;
        if ( rNewArea != aSourceArea )
            aSourceArea = rNewArea;
        if ( aNewOpt != aOptions )
            aOptions = aNewOpt;

        if ( aNewRange != aDestArea )
            aDestArea = aNewRange;

        USHORT nPaintEndX = Max( aOldRange.aEnd.Col(), aNewRange.aEnd.Col() );
        USHORT nPaintEndY = Max( aOldRange.aEnd.Row(), aNewRange.aEnd.Row() );

        if ( aOldRange.aEnd.Col() != aNewRange.aEnd.Col() )
            nPaintEndX = MAXCOL;
        if ( aOldRange.aEnd.Row() != aNewRange.aEnd.Row() )
            nPaintEndY = MAXROW;

        pDocShell->PostPaint( aDestPos.Col(),aDestPos.Row(),nDestTab,
                                nPaintEndX,nPaintEndY,nDestTab, PAINT_GRID );
        aModificator.SetDocumentModified();
    }
    else
    {
        //  CanFitBlock FALSE -> Probleme mit zusammengefassten Zellen
        //                       oder Tabellengrenze erreicht!
        //! Zellschutz ???

        //! Link-Dialog muss Default-Parent setzen
        //  "kann keine Zeilen einfuegen"
        InfoBox aBox( Application::GetDefDialogParent(),
                        ScGlobal::GetRscString( STR_MSSG_DOSUBTOTALS_2 ) );
        aBox.Execute();
    }

    //  aufraeumen

    aRef->DoClose();

    pDoc->SetInLinkUpdate( FALSE );

    if (bCanDo)
    {
        //  notify Uno objects (for XRefreshListener)
        //! also notify Uno objects if file name was changed!
        ScLinkRefreshedHint aHint;
        aHint.SetAreaLink( aDestPos );
        pDoc->BroadcastUno( aHint );
    }

    return bCanDo;
}



