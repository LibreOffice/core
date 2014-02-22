/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <comphelper/string.hxx>
#include <sfx2/app.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/linkmgr.hxx>
#include <svl/stritem.hxx>
#include <vcl/msgbox.hxx>

#include "arealink.hxx"

#include "tablink.hxx"
#include "document.hxx"
#include "docsh.hxx"
#include "rangenam.hxx"
#include "dbdata.hxx"
#include "undoblk.hxx"
#include "globstr.hrc"
#include "markdata.hxx"
#include "hints.hxx"
#include "filter.hxx"

#include "attrib.hxx"
#include "patattr.hxx"
#include "docpool.hxx"

#include "sc.hrc"
#include "scabstdlg.hxx"
#include "clipparam.hxx"

struct AreaLink_Impl
{
    ScDocShell* m_pDocSh;
    AbstractScLinkedAreaDlg* m_pDialog;

    AreaLink_Impl() : m_pDocSh( NULL ), m_pDialog( NULL ) {}
};

TYPEINIT1(ScAreaLink,::sfx2::SvBaseLink);



ScAreaLink::ScAreaLink( SfxObjectShell* pShell, const OUString& rFile,
                        const OUString& rFilter, const OUString& rOpt,
                        const OUString& rArea, const ScRange& rDest,
                        sal_uLong nRefresh ) :
    ::sfx2::SvBaseLink(sfx2::LINKUPDATE_ONCALL,FORMAT_FILE),
    ScRefreshTimer  ( nRefresh ),
    pImpl           ( new AreaLink_Impl() ),
    aFileName       (rFile),
    aFilterName     (rFilter),
    aOptions        (rOpt),
    aSourceArea     (rArea),
    aDestArea       (rDest),
    bAddUndo        (true),
    bInCreate       (false),
    bDoInsert       (true)
{
    OSL_ENSURE(pShell->ISA(ScDocShell), "ScAreaLink mit falscher ObjectShell");
    pImpl->m_pDocSh = static_cast< ScDocShell* >( pShell );
    SetRefreshHandler( LINK( this, ScAreaLink, RefreshHdl ) );
    SetRefreshControl( pImpl->m_pDocSh->GetDocument()->GetRefreshTimerControlAddress() );
}

ScAreaLink::~ScAreaLink()
{
    StopRefreshTimer();
    delete pImpl;
}

void ScAreaLink::Edit(Window* pParent, const Link& /* rEndEditHdl */ )
{
    
    
    

    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

    AbstractScLinkedAreaDlg* pDlg = pFact->CreateScLinkedAreaDlg(pParent);
    OSL_ENSURE(pDlg, "Dialog create fail!");
    pDlg->InitFromOldLink( aFileName, aFilterName, aOptions, aSourceArea, GetRefreshDelay() );
    pImpl->m_pDialog = pDlg;
    pDlg->StartExecuteModal( LINK( this, ScAreaLink, AreaEndEditHdl ) );
}

::sfx2::SvBaseLink::UpdateResult ScAreaLink::DataChanged(
    const OUString&, const ::com::sun::star::uno::Any& )
{
    
    

    if (bInCreate)
        return SUCCESS;

    sfx2::LinkManager* pLinkManager=pImpl->m_pDocSh->GetDocument()->GetLinkManager();
    if (pLinkManager!=NULL)
    {
        OUString aFile, aArea, aFilter;
        pLinkManager->GetDisplayNames(this, NULL, &aFile, &aArea, &aFilter);

        
        
        ScDocumentLoader::RemoveAppPrefix( aFilter );

        
        if (aArea.isEmpty())
        {
            aArea = aSourceArea;

            
            OUString aNewLinkName;
            OUString aTmp = aFilter;
            sfx2::MakeLnkName(aNewLinkName, NULL, aFile, aArea, &aTmp);
            aFilter = aTmp;
            SetName( aNewLinkName );
        }

        sfx2::SvBaseLinkRef const xThis(this); 
        Refresh( aFile, aFilter, aArea, GetRefreshDelay() );
    }

    return SUCCESS;
}

void ScAreaLink::Closed()
{
    

    ScDocument* pDoc = pImpl->m_pDocSh->GetDocument();
    sal_Bool bUndo (pDoc->IsUndoEnabled());
    if (bAddUndo && bUndo)
    {
        pImpl->m_pDocSh->GetUndoManager()->AddUndoAction( new ScUndoRemoveAreaLink( pImpl->m_pDocSh,
                                                        aFileName, aFilterName, aOptions,
                                                        aSourceArea, aDestArea, GetRefreshDelay() ) );

        bAddUndo = false;   
    }

    SCTAB nDestTab = aDestArea.aStart.Tab();
    if (pDoc->IsStreamValid(nDestTab))
        pDoc->SetStreamValid(nDestTab, false);

    SvBaseLink::Closed();
}

void ScAreaLink::SetDestArea(const ScRange& rNew)
{
    aDestArea = rNew;           
}

void ScAreaLink::SetSource(const OUString& rDoc, const OUString& rFlt, const OUString& rOpt,
                                const OUString& rArea)
{
    aFileName   = rDoc;
    aFilterName = rFlt;
    aOptions    = rOpt;
    aSourceArea = rArea;

    
    OUString aNewLinkName;
    sfx2::MakeLnkName( aNewLinkName, NULL, aFileName, aSourceArea, &aFilterName );
    SetName( aNewLinkName );
}

bool ScAreaLink::IsEqual( const OUString& rFile, const OUString& rFilter, const OUString& rOpt,
                            const OUString& rSource, const ScRange& rDest ) const
{
    return aFileName == rFile && aFilterName == rFilter && aOptions == rOpt &&
            aSourceArea == rSource && aDestArea.aStart == rDest.aStart;
}


bool ScAreaLink::FindExtRange( ScRange& rRange, ScDocument* pSrcDoc, const OUString& rAreaName )
{
    bool bFound = false;
    OUString aUpperName = ScGlobal::pCharClass->uppercase(rAreaName);
    ScRangeName* pNames = pSrcDoc->GetRangeName();
    if (pNames)         
    {
        const ScRangeData* p = pNames->findByUpperName(aUpperName);
        if (p && p->IsValidReference(rRange))
            bFound = true;
    }
    if (!bFound)        
    {
        ScDBCollection* pDBColl = pSrcDoc->GetDBCollection();
        if (pDBColl)
        {
            const ScDBData* pDB = pDBColl->getNamedDBs().findByUpperName(aUpperName);
            if (pDB)
            {
                SCTAB nTab;
                SCCOL nCol1, nCol2;
                SCROW nRow1, nRow2;
                pDB->GetArea(nTab,nCol1,nRow1,nCol2,nRow2);
                rRange = ScRange( nCol1,nRow1,nTab, nCol2,nRow2,nTab );
                bFound = true;
            }
        }
    }
    if (!bFound)        
    {
        ScAddress::Details aDetails(pSrcDoc->GetAddressConvention(), 0, 0);
        if ( rRange.ParseAny( rAreaName, pSrcDoc, aDetails ) & SCA_VALID )
            bFound = true;
    }
    return bFound;
}



bool ScAreaLink::Refresh( const OUString& rNewFile, const OUString& rNewFilter,
                            const OUString& rNewArea, sal_uLong nNewRefresh )
{
    

    if (rNewFile.isEmpty() || rNewFilter.isEmpty())
        return false;

    OUString aNewUrl( ScGlobal::GetAbsDocName( rNewFile, pImpl->m_pDocSh ) );
    sal_Bool bNewUrlName = (aNewUrl != aFileName);

    const SfxFilter* pFilter = pImpl->m_pDocSh->GetFactory().GetFilterContainer()->GetFilter4FilterName(rNewFilter);
    if (!pFilter)
        return false;

    ScDocument* pDoc = pImpl->m_pDocSh->GetDocument();

    sal_Bool bUndo (pDoc->IsUndoEnabled());
    pDoc->SetInLinkUpdate( true );

    
    if ( rNewFilter != aFilterName )
        aOptions = "";

    SfxMedium* pMed = ScDocumentLoader::CreateMedium( aNewUrl, pFilter, aOptions);

    
    ScDocShell* pSrcShell = new ScDocShell(SFX_CREATE_MODE_INTERNAL);
    SfxObjectShellLock aRef = pSrcShell;
    pSrcShell->DoLoad(pMed);

    ScDocument* pSrcDoc = pSrcShell->GetDocument();

    
    OUString aNewOpt = ScDocumentLoader::GetOptions(*pMed);
    if (aNewOpt.isEmpty())
        aNewOpt = aOptions;

    
    OUString aTempArea;

    if( rNewFilter == ScDocShell::GetWebQueryFilterName() )
        aTempArea = ScFormatFilter::Get().GetHTMLRangeNameList( pSrcDoc, rNewArea );
    else
        aTempArea = rNewArea;

    
    SCCOL nWidth = 0;
    SCROW nHeight = 0;
    sal_Int32 nTokenCnt = comphelper::string::getTokenCount(aTempArea, ';');
    sal_Int32 nStringIx = 0;
    sal_Int32 nToken;

    for( nToken = 0; nToken < nTokenCnt; nToken++ )
    {
        OUString aToken( aTempArea.getToken( 0, ';', nStringIx ) );
        ScRange aTokenRange;
        if( FindExtRange( aTokenRange, pSrcDoc, aToken ) )
        {
            
            nWidth = std::max( nWidth, (SCCOL)(aTokenRange.aEnd.Col() - aTokenRange.aStart.Col() + 1) );
            
            nHeight += aTokenRange.aEnd.Row() - aTokenRange.aStart.Row() + 2;
        }
    }
    
    if( nHeight > 0 )
        nHeight--;

    

    ScAddress aDestPos = aDestArea.aStart;
    SCTAB nDestTab = aDestPos.Tab();
    ScRange aOldRange = aDestArea;
    ScRange aNewRange = aDestArea;          
    if (nWidth > 0 && nHeight > 0)
    {
        aNewRange.aEnd.SetCol( aNewRange.aStart.Col() + nWidth - 1 );
        aNewRange.aEnd.SetRow( aNewRange.aStart.Row() + nHeight - 1 );
    }

    
    bool bCanDo = ValidColRow( aNewRange.aEnd.Col(), aNewRange.aEnd.Row() ) &&
                  pDoc->CanFitBlock( aOldRange, aNewRange );
    if (bCanDo)
    {
        ScDocShellModificator aModificator( *pImpl->m_pDocSh );

        SCCOL nOldEndX = aOldRange.aEnd.Col();
        SCROW nOldEndY = aOldRange.aEnd.Row();
        SCCOL nNewEndX = aNewRange.aEnd.Col();
        SCROW nNewEndY = aNewRange.aEnd.Row();
        ScRange aMaxRange( aDestPos,
                    ScAddress(std::max(nOldEndX,nNewEndX), std::max(nOldEndY,nNewEndY), nDestTab) );

        

        ScDocument* pUndoDoc = NULL;
        if ( bAddUndo && bUndo )
        {
            pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            if ( bDoInsert )
            {
                if ( nNewEndX != nOldEndX || nNewEndY != nOldEndY )             
                {
                    pUndoDoc->InitUndo( pDoc, 0, pDoc->GetTableCount()-1 );
                    pDoc->CopyToDocument( 0,0,0,MAXCOL,MAXROW,MAXTAB,
                                            IDF_FORMULA, false, pUndoDoc );     
                }
                else
                    pUndoDoc->InitUndo( pDoc, nDestTab, nDestTab );             
                pDoc->CopyToDocument( aOldRange, IDF_ALL & ~IDF_NOTE, false, pUndoDoc );
            }
            else        
            {
                pUndoDoc->InitUndo( pDoc, nDestTab, nDestTab );             
                pDoc->CopyToDocument( aMaxRange, IDF_ALL & ~IDF_NOTE, false, pUndoDoc );
            }
        }

        
        

        if (bDoInsert)
            pDoc->FitBlock( aOldRange, aNewRange );         
        else
            pDoc->DeleteAreaTab( aMaxRange, IDF_ALL & ~IDF_NOTE );

        

        if (nWidth > 0 && nHeight > 0)
        {
            ScDocument aClipDoc( SCDOCMODE_CLIP );
            ScRange aNewTokenRange( aNewRange.aStart );
            nStringIx = 0;
            for( nToken = 0; nToken < nTokenCnt; nToken++ )
            {
                OUString aToken( aTempArea.getToken( 0, ';', nStringIx ) );
                ScRange aTokenRange;
                if( FindExtRange( aTokenRange, pSrcDoc, aToken ) )
                {
                    SCTAB nSrcTab = aTokenRange.aStart.Tab();
                    ScMarkData aSourceMark;
                    aSourceMark.SelectOneTable( nSrcTab );      
                    aSourceMark.SetMarkArea( aTokenRange );

                    ScClipParam aClipParam(aTokenRange, false);
                    pSrcDoc->CopyToClip(aClipParam, &aClipDoc, &aSourceMark);

                    if ( aClipDoc.HasAttrib( 0,0,nSrcTab, MAXCOL,MAXROW,nSrcTab,
                                            HASATTR_MERGED | HASATTR_OVERLAPPED ) )
                    {
                        

                        ScPatternAttr aPattern( pSrcDoc->GetPool() );
                        aPattern.GetItemSet().Put( ScMergeAttr() );             
                        aPattern.GetItemSet().Put( ScMergeFlagAttr() );
                        aClipDoc.ApplyPatternAreaTab( 0,0, MAXCOL,MAXROW, nSrcTab, aPattern );
                    }

                    aNewTokenRange.aEnd.SetCol( aNewTokenRange.aStart.Col() + (aTokenRange.aEnd.Col() - aTokenRange.aStart.Col()) );
                    aNewTokenRange.aEnd.SetRow( aNewTokenRange.aStart.Row() + (aTokenRange.aEnd.Row() - aTokenRange.aStart.Row()) );
                    ScMarkData aDestMark;
                    aDestMark.SelectOneTable( nDestTab );
                    aDestMark.SetMarkArea( aNewTokenRange );
                    pDoc->CopyFromClip( aNewTokenRange, aDestMark, IDF_ALL, NULL, &aClipDoc, false );
                    aNewTokenRange.aStart.SetRow( aNewTokenRange.aEnd.Row() + 2 );
                }
            }
        }
        else
        {
            OUString aErr = ScGlobal::GetRscString(STR_LINKERROR);
            pDoc->SetString( aDestPos.Col(), aDestPos.Row(), aDestPos.Tab(), aErr );
        }

        

        if ( bAddUndo && bUndo)
        {
            ScDocument* pRedoDoc = new ScDocument( SCDOCMODE_UNDO );
            pRedoDoc->InitUndo( pDoc, nDestTab, nDestTab );
            pDoc->CopyToDocument( aNewRange, IDF_ALL & ~IDF_NOTE, false, pRedoDoc );

            pImpl->m_pDocSh->GetUndoManager()->AddUndoAction(
                new ScUndoUpdateAreaLink( pImpl->m_pDocSh,
                                            aFileName, aFilterName, aOptions,
                                            aSourceArea, aOldRange, GetRefreshDelay(),
                                            aNewUrl, rNewFilter, aNewOpt,
                                            rNewArea, aNewRange, nNewRefresh,
                                            pUndoDoc, pRedoDoc, bDoInsert ) );
        }

        

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

        if ( nNewRefresh != GetRefreshDelay() )
            SetRefreshDelay( nNewRefresh );

        SCCOL nPaintEndX = std::max( aOldRange.aEnd.Col(), aNewRange.aEnd.Col() );
        SCROW nPaintEndY = std::max( aOldRange.aEnd.Row(), aNewRange.aEnd.Row() );

        if ( aOldRange.aEnd.Col() != aNewRange.aEnd.Col() )
            nPaintEndX = MAXCOL;
        if ( aOldRange.aEnd.Row() != aNewRange.aEnd.Row() )
            nPaintEndY = MAXROW;

        if ( !pImpl->m_pDocSh->AdjustRowHeight( aDestPos.Row(), nPaintEndY, nDestTab ) )
            pImpl->m_pDocSh->PostPaint(
                ScRange(aDestPos.Col(), aDestPos.Row(), nDestTab, nPaintEndX, nPaintEndY, nDestTab),
                PAINT_GRID);
        aModificator.SetDocumentModified();
    }
    else
    {
        
        
        

        
        
        InfoBox aBox( Application::GetDefDialogParent(),
                        ScGlobal::GetRscString( STR_MSSG_DOSUBTOTALS_2 ) );
        aBox.Execute();
    }

    

    aRef->DoClose();

    pDoc->SetInLinkUpdate( false );

    if (bCanDo)
    {
        
        
        ScLinkRefreshedHint aHint;
        aHint.SetAreaLink( aDestPos );
        pDoc->BroadcastUno( aHint );
    }

    return bCanDo;
}


IMPL_LINK_NOARG(ScAreaLink, RefreshHdl)
{
    return long(
        Refresh( aFileName, aFilterName, aSourceArea, GetRefreshDelay() ));
}

IMPL_LINK_NOARG(ScAreaLink, AreaEndEditHdl)
{
    
    

    if ( pImpl->m_pDialog && pImpl->m_pDialog->GetResult() == RET_OK )
    {
        aOptions = pImpl->m_pDialog->GetOptions();
        Refresh( pImpl->m_pDialog->GetURL(), pImpl->m_pDialog->GetFilter(),
                 pImpl->m_pDialog->GetSource(), pImpl->m_pDialog->GetRefresh() );

        
        OUString aNewLinkName;
        sfx2::MakeLnkName( aNewLinkName, NULL, aFileName, aSourceArea, &aFilterName );
        SetName( aNewLinkName );
    }
    pImpl->m_pDialog = NULL;    

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
