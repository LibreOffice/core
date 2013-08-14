/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <hintids.hxx>
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <svl/urihelper.hxx>
#include <svl/fstathelper.hxx>
#include <unotools/moduleoptions.hxx>
#include <sfx2/docfile.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/paperinf.hxx>
#include <node.hxx>
#include <docary.hxx>
#include <fmtanchr.hxx>
#include <fmtfsize.hxx>
#include <fmtpdsc.hxx>
#include <swtypes.hxx>
#include <shellio.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <pam.hxx>
#include <editsh.hxx>
#include <undobj.hxx>           // fuer Undo Insert-Dokument
#include <swundo.hxx>           // fuer Undo Insert-Dokument
#include <swtable.hxx>
#include <tblsel.hxx>
#include <pagedesc.hxx>
#include <poolfmt.hxx>
#include <fltini.hxx>
#include <docsh.hxx>
#include <redline.hxx>
#include <swerror.h>
#include <paratr.hxx>

// #i73788#
#include <pausethreadstarting.hxx>

using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////////

sal_uLong SwReader::Read( const Reader& rOptions )
{
    // Variable uebertragen
    Reader* po = (Reader*) &rOptions;
    po->pStrm = pStrm;
    po->pStg  = pStg;
    po->xStg  = xStg;
    po->bInsertMode = 0 != pCrsr;

    // ist ein Medium angegeben, dann aus diesem die Streams besorgen
    if( 0 != (po->pMedium = pMedium ) &&
        !po->SetStrmStgPtr() )
    {
        po->SetReadUTF8( sal_False );
        po->SetBlockMode( sal_False );
        po->SetOrganizerMode( sal_False );
        po->SetIgnoreHTMLComments( sal_False );
        return ERR_SWG_FILE_FORMAT_ERROR;
    }

    sal_uLong nError = 0L;

    GetDoc();

    // waehrend des einlesens kein OLE-Modified rufen
    Link aOLELink( pDoc->GetOle2Link() );
    pDoc->SetOle2Link( Link() );

    pDoc->SetInReading( true );
    pDoc->SetInXMLImport( 0 != dynamic_cast< XMLReader* >(po) );

    SwPaM *pPam;
    if( pCrsr )
        pPam = pCrsr;
    else
    {
        // Wenn der Reader nicht mit einem Shell konstruiert wurde,
        // selber einen Pam machen.
        SwNodeIndex nNode( pDoc->GetNodes().GetEndOfContent(), -1 );
        pPam = new SwPaM( nNode );
        // Bei Web-Dokumenten wird die Default-Vorlage schon im InitNew
        // gesetzt und braucht deshalb nicht nochmal gesetzt zu werden.
        // Das gilt natuerlich nicht, wenn der Filter nicht der HTML-Filter
        // ist oder im ConvertFrom zuvor ein SetTemplateName gerufen
        // wurde.
        if( !pDoc->get(IDocumentSettingAccess::HTML_MODE) || ReadHTML != po || !po->pTemplate  )
            po->SetTemplate( *pDoc );
    }

    // Pams sind ringfoermig verkettet. Aufhoeren, wenn man wieder beim
    // ersten ist.
    SwPaM *pEnd = pPam;
    SwUndoInsDoc* pUndo = 0;

    sal_Bool bReadPageDescs = sal_False;
    bool const bDocUndo = pDoc->GetIDocumentUndoRedo().DoesUndo();
    bool bSaveUndo = bDocUndo && pCrsr;
    if( bSaveUndo )
    {
        // das Einlesen von Seitenvorlagen ist nicht Undofaehig!
        if( 0 != ( bReadPageDescs = po->aOpt.IsPageDescs() ) )
        {
            bSaveUndo = false;
            pDoc->GetIDocumentUndoRedo().DelAllUndoObj();
        }
        else
        {
            pDoc->GetIDocumentUndoRedo().ClearRedo();
            pDoc->GetIDocumentUndoRedo().StartUndo( UNDO_INSDOKUMENT, NULL );
        }
    }
    pDoc->GetIDocumentUndoRedo().DoUndo(false);

    SwNodeIndex aSplitIdx( pDoc->GetNodes() );

    RedlineMode_t eOld = pDoc->GetRedlineMode();
    RedlineMode_t ePostReadRedlineMode( nsRedlineMode_t::REDLINE_IGNORE );

    // Array von FlyFormaten
    SwFrmFmts aFlyFrmArr;
    // only read templates? then ignore multi selection!
    sal_Bool bFmtsOnly = po->aOpt.IsFmtsOnly();

    while( true )
    {
        if( bSaveUndo )
            pUndo = new SwUndoInsDoc( *pPam );

        pDoc->SetRedlineMode_intern( nsRedlineMode_t::REDLINE_IGNORE );

        SwPaM* pUndoPam = 0;
        if( bDocUndo || pCrsr )
        {
            // Pam auf den Node davor setzen damit er nicht mit verschoben wird
            const SwNodeIndex& rTmp = pPam->GetPoint()->nNode;
            pUndoPam = new SwPaM( rTmp, rTmp, 0, -1 );
        }

        // Speicher mal alle Fly's
        if( pCrsr )
        {
            std::copy(pDoc->GetSpzFrmFmts()->begin(),
                pDoc->GetSpzFrmFmts()->end(), std::back_inserter(aFlyFrmArr));
        }

        xub_StrLen nSttCntnt = pPam->GetPoint()->nContent.GetIndex();

        // damit fuer alle Reader die Ende-Position immer stimmt, hier
        // pflegen.
        SwCntntNode* pCNd = pPam->GetCntntNode();
        xub_StrLen nEndCntnt = pCNd ? pCNd->Len() - nSttCntnt : 0;
        SwNodeIndex aEndPos( pPam->GetPoint()->nNode, 1 );

        pDoc->SetRedlineMode_intern( eOld );

        nError = po->Read( *pDoc, GetBaseURL(), *pPam, aFileName );

        // an ODF document may contain redline mode in settings.xml; save it!
        ePostReadRedlineMode = pDoc->GetRedlineMode();

        pDoc->SetRedlineMode_intern( nsRedlineMode_t::REDLINE_IGNORE );

        if( !IsError( nError ))     // dann setzen wir das Ende mal richtig
        {
            aEndPos--;
            pCNd = aEndPos.GetNode().GetCntntNode();
            if( !pCNd && 0 == ( pCNd = pDoc->GetNodes().GoPrevious( &aEndPos ) ))
                pCNd = pDoc->GetNodes().GoNext( &aEndPos );

            pPam->GetPoint()->nNode = aEndPos;
            xub_StrLen nLen = pCNd->Len();
            if( nLen < nEndCntnt )
                nEndCntnt = 0;
            else
                nEndCntnt = nLen - nEndCntnt;
            pPam->GetPoint()->nContent.Assign( pCNd, nEndCntnt );

            const SwStartNode* pTblBoxStart = pCNd->FindTableBoxStartNode();
            if ( pTblBoxStart )
            {
                SwTableBox* pBox = pTblBoxStart->GetTblBox();
                if ( pBox )
                {
                    pDoc->ChkBoxNumFmt( *pBox, sal_True );
                }
            }
        }

        if( pCrsr )
        {
            *pUndoPam->GetMark() = *pPam->GetPoint();
            pUndoPam->GetPoint()->nNode++;
            SwNode* pNd = pUndoPam->GetNode();
            if( pNd->IsCntntNode() )
                pUndoPam->GetPoint()->nContent.Assign(
                                    (SwCntntNode*)pNd, nSttCntnt );
            else
                pUndoPam->GetPoint()->nContent.Assign( 0, 0 );

            int bChkHeaderFooter = pNd->FindHeaderStartNode() ||
                                   pNd->FindFooterStartNode();

            // Suche alle neuen Fly's und speicher sie als einzelne Undo
            // Objecte
            for( sal_uInt16 n = 0; n < pDoc->GetSpzFrmFmts()->size(); ++n )
            {
                SwFrmFmt* pFrmFmt = (*pDoc->GetSpzFrmFmts())[ n ];
                const SwFmtAnchor& rAnchor = pFrmFmt->GetAnchor();
                if( USHRT_MAX == aFlyFrmArr.GetPos( pFrmFmt) )
                {
                    SwPosition const*const pFrameAnchor(
                            rAnchor.GetCntntAnchor());
                    if  (   (FLY_AT_PAGE == rAnchor.GetAnchorId())
                        ||  (   pFrameAnchor
                            &&  (   (   (FLY_AT_PARA == rAnchor.GetAnchorId())
                                    &&  (   (pUndoPam->GetPoint()->nNode ==
                                             pFrameAnchor->nNode)
                                        ||  (pUndoPam->GetMark()->nNode ==
                                             pFrameAnchor->nNode)
                                        )
                                    )
                                // #i97570# also check frames anchored AT char
                                ||  (   (FLY_AT_CHAR == rAnchor.GetAnchorId())
                                    &&  !IsDestroyFrameAnchoredAtChar(
                                              *pFrameAnchor,
                                              *pUndoPam->GetPoint(),
                                              *pUndoPam->GetMark())
                                    )
                                )
                            )
                        )
                    {
                        if( bChkHeaderFooter &&
                            (FLY_AT_PARA == rAnchor.GetAnchorId()) &&
                            RES_DRAWFRMFMT == pFrmFmt->Which() )
                        {
                            // DrawObjecte in Kopf-/Fusszeilen ist nicht
                            // erlaubt!
                            pFrmFmt->DelFrms();
                            pDoc->DelFrmFmt( pFrmFmt );
                            --n;
                        }
                        else
                        {
                            if( bSaveUndo )
                            {
                                pDoc->SetRedlineMode_intern( eOld );
                                // UGLY: temp. enable undo
                                pDoc->GetIDocumentUndoRedo().DoUndo(true);
                                pDoc->GetIDocumentUndoRedo().AppendUndo(
                                    new SwUndoInsLayFmt( pFrmFmt,0,0 ) );
                                pDoc->GetIDocumentUndoRedo().DoUndo(false);
                                pDoc->SetRedlineMode_intern( nsRedlineMode_t::REDLINE_IGNORE );
                            }
                            if( pFrmFmt->GetDepends() )
                            {
                                // beim Insert legen Draw-Objecte einen Frame an
                                // also weg damit.
                                pFrmFmt->DelFrms();
                            }

                            if (FLY_AT_PAGE == rAnchor.GetAnchorId())
                            {
                                if( !rAnchor.GetCntntAnchor() )
                                {
                                    pFrmFmt->MakeFrms();
                                }
                                else if( pCrsr )
                                {
                                    pDoc->SetContainsAtPageObjWithContentAnchor( true );
                                }
                            }
                            else
                                pFrmFmt->MakeFrms();
                        }
                    }
                }
            }
            if( !aFlyFrmArr.empty() )
                aFlyFrmArr.clear();

            pDoc->SetRedlineMode_intern( eOld );
            if( pDoc->IsRedlineOn() )
                pDoc->AppendRedline( new SwRedline( nsRedlineType_t::REDLINE_INSERT, *pUndoPam ), true);
            else
                pDoc->SplitRedline( *pUndoPam );
            pDoc->SetRedlineMode_intern( nsRedlineMode_t::REDLINE_IGNORE );
        }
        if( bSaveUndo )
        {
            pDoc->SetRedlineMode_intern( eOld );
            pUndo->SetInsertRange( *pUndoPam, sal_False );
            // UGLY: temp. enable undo
            pDoc->GetIDocumentUndoRedo().DoUndo(true);
            pDoc->GetIDocumentUndoRedo().AppendUndo( pUndo );
            pDoc->GetIDocumentUndoRedo().DoUndo(false);
            pDoc->SetRedlineMode_intern( nsRedlineMode_t::REDLINE_IGNORE );
        }

        delete pUndoPam;

        pPam = (SwPaM *) pPam->GetNext();
        if( pPam == pEnd )
            break;

        // only read templates? then ignore multi selection! Bug 68593
        if( bFmtsOnly )
            break;

        /*
         * !!! man muss selbst den Status vom Stream zuruecksetzen. !!!
         *     Beim seekg wird der akt. Status, eof- und bad-Bit
         *     gesetzt, warum weiss keiner
         */
        if( pStrm )
        {
            pStrm->Seek(0);
            pStrm->ResetError();
        }
    }

    pDoc->SetInReading( false );
    pDoc->SetInXMLImport( false );

    pDoc->InvalidateNumRules();
    pDoc->UpdateNumRule();
    pDoc->ChkCondColls();
    pDoc->SetAllUniqueFlyNames();
    pDoc->SetLoaded( true );

    pDoc->GetIDocumentUndoRedo().DoUndo(bDocUndo);
    if (!bReadPageDescs)
    {
        if( bSaveUndo )
        {
            pDoc->SetRedlineMode_intern( eOld );
            pDoc->GetIDocumentUndoRedo().EndUndo( UNDO_INSDOKUMENT, NULL );
            pDoc->SetRedlineMode_intern( nsRedlineMode_t::REDLINE_IGNORE );
        }
    }

    // Wenn der Pam nur fuers Lesen konstruiert wurde, jetzt zerstoeren.
    if( !pCrsr )
    {
        delete pPam;          // ein neues aufgemacht.

        // #i42634# Moved common code of SwReader::Read() and
        // SwDocShell::UpdateLinks() to new SwDoc::UpdateLinks():
    // ATM still with Update
        pDoc->UpdateLinks( true );

        // not insert: set the redline mode read from settings.xml
        eOld = static_cast<RedlineMode_t>(
                ePostReadRedlineMode & ~nsRedlineMode_t::REDLINE_IGNORE);

        pDoc->SetFieldsDirty(false, NULL, 0);
    }

    pDoc->SetRedlineMode_intern( eOld );
    pDoc->SetOle2Link( aOLELink );

    if( pCrsr )                 // das Doc ist jetzt modifiziert
        pDoc->SetModified();
    // #i38810# - If links have been updated, the document
    // have to be modified. During update of links the OLE link at the document
    // isn't set. Thus, the document's modified state has to be set again after
    // the OLE link is restored - see above <pDoc->SetOle2Link( aOLELink )>.
    if ( pDoc->LinksUpdated() )
    {
        pDoc->SetModified();
    }

    po->SetReadUTF8( sal_False );
    po->SetBlockMode( sal_False );
    po->SetOrganizerMode( sal_False );
    po->SetIgnoreHTMLComments( sal_False );

    return nError;
}


/*
 * Konstruktoren, Destruktor
 */

SwReader::SwReader(SfxMedium& rMedium, const OUString& rFileName, SwDoc *pDocument)
    : SwDocFac(pDocument), pStrm(0), pMedium(&rMedium), pCrsr(0),
    aFileName(rFileName)
{
    SetBaseURL( rMedium.GetBaseURL() );
}

// In ein existierendes Dokument einlesen

SwReader::SwReader(SvStream& rStrm, const OUString& rFileName, const OUString& rBaseURL, SwPaM& rPam)
    : SwDocFac(rPam.GetDoc()), pStrm(&rStrm), pMedium(0), pCrsr(&rPam),
    aFileName(rFileName)
{
    SetBaseURL( rBaseURL );
}

SwReader::SwReader(SfxMedium& rMedium, const OUString& rFileName, SwPaM& rPam)
    : SwDocFac(rPam.GetDoc()), pStrm(0), pMedium(&rMedium),
    pCrsr(&rPam), aFileName(rFileName)
{
    SetBaseURL( rMedium.GetBaseURL() );
}

SwReader::SwReader( const uno::Reference < embed::XStorage > &rStg, const OUString& rFilename, SwPaM &rPam )
    : SwDocFac(rPam.GetDoc()), pStrm(0), xStg( rStg ), pMedium(0), pCrsr(&rPam), aFileName(rFilename)
{
}

Reader::Reader()
    : pTemplate(0),
    aDStamp( Date::EMPTY ),
    aTStamp( Time::EMPTY ),
    aChkDateTime( DateTime::EMPTY ),
    pStrm(0), pMedium(0), bInsertMode(0),
    bTmplBrowseMode(0), bReadUTF8(0), bBlockMode(0), bOrganizerMode(0),
    bHasAskTemplateName(0), bIgnoreHTMLComments(0)
{
}

Reader::~Reader()
{
    delete pTemplate;
}

OUString Reader::GetTemplateName() const
{
    return OUString();
}

// Die Filter-Vorlage laden, setzen und wieder freigeben
SwDoc* Reader::GetTemplateDoc()
{
    if( !bHasAskTemplateName )
    {
        SetTemplateName( GetTemplateName() );
        bHasAskTemplateName = sal_True;
    }

    if( aTemplateNm.isEmpty() )
        ClearTemplate();
    else
    {
        INetURLObject aTDir( aTemplateNm );
        const OUString aFileName = aTDir.GetMainURL( INetURLObject::NO_DECODE );
        OSL_ENSURE( !aTDir.HasError(), "No absolute path for template name!" );
        DateTime aCurrDateTime( DateTime::SYSTEM );
        bool bLoad = false;

        // Wenn das Template schon mal geladen wurde, nur einmal pro
        // Minute nachschauen, ob es geaendert wurde.
        if( !pTemplate || aCurrDateTime >= aChkDateTime )
        {
            Date aTstDate( Date::EMPTY );
            Time aTstTime( Time::EMPTY );
            if( FStatHelper::GetModifiedDateTimeOfFile(
                            aTDir.GetMainURL( INetURLObject::NO_DECODE ),
                            &aTstDate, &aTstTime ) &&
                ( !pTemplate || aDStamp != aTstDate || aTStamp != aTstTime ))
            {
                bLoad = true;
                aDStamp = aTstDate;
                aTStamp = aTstTime;
            }

            // Erst in einer Minute wieder mal nachschauen, ob sich die
            // Vorlage geaendert hat.
            aChkDateTime = aCurrDateTime;
            aChkDateTime += Time( 0L, 1L );
        }

        if( bLoad )
        {
            ClearTemplate();
            OSL_ENSURE( !pTemplate, "Who holds the template doc?" );

                // If the writer module is not installed,
                // we cannot create a SwDocShell. We could create a
                // SwWebDocShell however, because this exists always
                // for the help.
                SvtModuleOptions aModuleOptions;
                if( aModuleOptions.IsWriter() )
                {
                    SwDocShell *pDocSh =
                        new SwDocShell ( SFX_CREATE_MODE_INTERNAL );
                    SfxObjectShellLock xDocSh = pDocSh;
                    if( pDocSh->DoInitNew( 0 ) )
                    {
                        pTemplate = pDocSh->GetDoc();
                        pTemplate->SetOle2Link( Link() );
                        // always FALSE
                        pTemplate->GetIDocumentUndoRedo().DoUndo( false );
                        pTemplate->set(IDocumentSettingAccess::BROWSE_MODE, bTmplBrowseMode );
                        pTemplate->RemoveAllFmtLanguageDependencies();

                        ReadXML->SetOrganizerMode( sal_True );
                        SfxMedium aMedium( aFileName, sal_False );
                        SwReader aRdr( aMedium, aEmptyStr, pTemplate );
                        aRdr.Read( *ReadXML );
                        ReadXML->SetOrganizerMode( sal_False );

                        pTemplate->acquire();
                    }
                }
        }

        OSL_ENSURE( !pTemplate || FStatHelper::IsDocument( aFileName ) || aTemplateNm=="$$Dummy$$",
                "TemplatePtr but no template exist!" );
    }

    return pTemplate;
}

sal_Bool Reader::SetTemplate( SwDoc& rDoc )
{
    sal_Bool bRet = sal_False;

    GetTemplateDoc();
    if( pTemplate )
    {
        rDoc.RemoveAllFmtLanguageDependencies();
        rDoc.ReplaceStyles( *pTemplate );
        rDoc.SetFixFields(false, NULL);
        bRet = sal_True;
    }

    return bRet;
}

void Reader::ClearTemplate()
{
    if( pTemplate )
    {
        if( 0 == pTemplate->release() )
            delete pTemplate,
        pTemplate = 0;
    }
}

void Reader::SetTemplateName( const OUString& rDir )
{
    if( !rDir.isEmpty() && aTemplateNm != rDir )
    {
        ClearTemplate();
        aTemplateNm = rDir;
    }
}

void Reader::MakeHTMLDummyTemplateDoc()
{
    ClearTemplate();
    pTemplate = new SwDoc;
    pTemplate->acquire();
    pTemplate->set(IDocumentSettingAccess::BROWSE_MODE, bTmplBrowseMode );
    pTemplate->getPrinter( true );
    pTemplate->RemoveAllFmtLanguageDependencies();
    aChkDateTime = Date( 1, 1, 2300 );  // 2300. Jahrtausend sollte reichen
    aTemplateNm = "$$Dummy$$";
}

// alle die die Streams / Storages nicht geoeffnet brauchen,
// muessen die Methode ueberladen
int Reader::SetStrmStgPtr()
{
    OSL_ENSURE( pMedium, "Wo ist das Medium??" );

    if( pMedium->IsStorage() )
    {
        if( SW_STORAGE_READER & GetReaderType() )
        {
            xStg = pMedium->GetStorage();
            return sal_True;
        }
    }
    else
    {
        pStrm = pMedium->GetInStream();
        if ( pStrm && SotStorage::IsStorageFile(pStrm) && (SW_STORAGE_READER & GetReaderType()) )
        {
            pStg = new SotStorage( *pStrm );
            pStrm = NULL;
        }
        else if ( !(SW_STREAM_READER & GetReaderType()) )
        {
            pStrm = NULL;
            return sal_False;
        }

        return sal_True;
    }
    return sal_False;
}


int Reader::GetReaderType()
{
    return SW_STREAM_READER;
}


void Reader::SetFltName( const OUString& )
{
}


void Reader::ResetFrmFmtAttrs( SfxItemSet &rFrmSet )
{
    rFrmSet.Put( SvxLRSpaceItem(RES_LR_SPACE) );
    rFrmSet.Put( SvxULSpaceItem(RES_UL_SPACE) );
    rFrmSet.Put( SvxBoxItem(RES_BOX) );
}


void Reader::ResetFrmFmts( SwDoc& rDoc )
{
    for (sal_uInt16 i=0; i<3; ++i)
    {
        sal_uInt16 nPoolId;
        switch (i)
        {
            default:
                OSL_ENSURE(i == 0, "Impossible");
                //fallthrough
            case 0:
                nPoolId = RES_POOLFRM_FRAME;
                break;
            case 1:
                nPoolId = RES_POOLFRM_GRAPHIC;
                break;
            case 2:
                nPoolId = RES_POOLFRM_OLE;
                break;
        }

        SwFrmFmt *pFrmFmt = rDoc.GetFrmFmtFromPool( nPoolId );

        pFrmFmt->ResetFmtAttr( RES_LR_SPACE );
        pFrmFmt->ResetFmtAttr( RES_UL_SPACE );
        pFrmFmt->ResetFmtAttr( RES_BOX );
    }
}

    // read the sections of the document, which is equal to the medium.
    // returns the count of it
size_t Reader::GetSectionList( SfxMedium&, std::vector<OUString*>& ) const
{
    return 0;
}

// ------------------------------------------------
sal_Bool SwReader::HasGlossaries( const Reader& rOptions )
{
    // Variable uebertragen
    Reader* po = (Reader*) &rOptions;
    po->pStrm = pStrm;
    po->pStg  = pStg;
    po->bInsertMode = sal_False;

    // ist ein Medium angegeben, dann aus diesem die Streams besorgen
    sal_Bool bRet = sal_False;
    if( !( 0 != (po->pMedium = pMedium ) && !po->SetStrmStgPtr() ))
        bRet = po->HasGlossaries();
    return bRet;
}

sal_Bool SwReader::ReadGlossaries( const Reader& rOptions,
                                SwTextBlocks& rBlocks, sal_Bool bSaveRelFiles )
{
    // Variable uebertragen
    Reader* po = (Reader*) &rOptions;
    po->pStrm = pStrm;
    po->pStg  = pStg;
    po->bInsertMode = sal_False;

    // ist ein Medium angegeben, dann aus diesem die Streams besorgen
    sal_Bool bRet = sal_False;
    if( !( 0 != (po->pMedium = pMedium ) && !po->SetStrmStgPtr() ))
        bRet = po->ReadGlossaries( rBlocks, bSaveRelFiles );
    return bRet;
}

sal_Bool Reader::HasGlossaries() const
{
    return sal_False;
}

sal_Bool Reader::ReadGlossaries( SwTextBlocks&, sal_Bool ) const
{
    return sal_False;
}

// ------------------------------------------------

int StgReader::GetReaderType()
{
    return SW_STORAGE_READER;
}




/*
 * Writer
 */

/*
 * Konstruktoren, Destruktoren sind inline (inc/shellio.hxx).
 */

SwWriter::SwWriter(SvStream& rStrm, SwCrsrShell &rShell, sal_Bool bInWriteAll)
    : pStrm(&rStrm), pMedium(0), pOutPam(0), pShell(&rShell),
    rDoc(*rShell.GetDoc()), bWriteAll(bInWriteAll)
{
}

SwWriter::SwWriter(SvStream& rStrm,SwDoc &rDocument)
    : pStrm(&rStrm), pMedium(0), pOutPam(0), pShell(0), rDoc(rDocument),
    bWriteAll(true)
{
}

SwWriter::SwWriter(SvStream& rStrm, SwPaM& rPam, sal_Bool bInWriteAll)
    : pStrm(&rStrm), pMedium(0), pOutPam(&rPam), pShell(0),
    rDoc(*rPam.GetDoc()), bWriteAll(bInWriteAll)
{
}

SwWriter::SwWriter( const uno::Reference < embed::XStorage >& rStg, SwDoc &rDocument)
    : pStrm(0), xStg( rStg ), pMedium(0), pOutPam(0), pShell(0), rDoc(rDocument), bWriteAll(true)
{
}

SwWriter::SwWriter(SfxMedium& rMedium, SwCrsrShell &rShell, sal_Bool bInWriteAll)
    : pStrm(0), pMedium(&rMedium), pOutPam(0), pShell(&rShell),
    rDoc(*rShell.GetDoc()), bWriteAll(bInWriteAll)
{
}

SwWriter::SwWriter(SfxMedium& rMedium, SwDoc &rDocument)
    : pStrm(0), pMedium(&rMedium), pOutPam(0), pShell(0), rDoc(rDocument),
    bWriteAll(true)
{
}

sal_uLong SwWriter::Write( WriterRef& rxWriter, const OUString* pRealFileName )
{
    // #i73788#
    SwPauseThreadStarting aPauseThreadStarting;

    sal_Bool bHasMark = sal_False;
    SwPaM * pPam;

    SwDoc *pDoc = 0;

    if ( pShell && !bWriteAll && pShell->IsTableMode() )
    {
        bWriteAll = sal_True;
        pDoc = new SwDoc;
        pDoc->acquire();

        // kopiere Teile aus einer Tabelle: lege eine Tabelle mit der Breite
        // von der Originalen an und kopiere die selectierten Boxen.
        // Die Groessen werden prozentual korrigiert.

        // lasse ueber das Layout die Boxen suchen
        SwSelBoxes aBoxes;
        GetTblSel( *pShell, aBoxes );
        SwTableNode* pTblNd = (SwTableNode*)aBoxes[0]->GetSttNd()->StartOfSectionNode();
        SwNodeIndex aIdx( pDoc->GetNodes().GetEndOfExtras(), 2 );
        SwCntntNode *pNd = aIdx.GetNode().GetCntntNode();
        OSL_ENSURE( pNd, "Node not found" );
        SwPosition aPos( aIdx, SwIndex( pNd ) );
        pTblNd->GetTable().MakeCopy( pDoc, aPos, aBoxes );
    }

    if( !bWriteAll && ( pShell || pOutPam ))
    {
        if( pShell )
            pPam = pShell->GetCrsr();
        else
            pPam = pOutPam;

        SwPaM *pEnd = pPam;

        // Erste Runde: Nachsehen, ob eine Selektion besteht.
        while(true)
        {
            bHasMark = bHasMark || pPam->HasMark();
            pPam = (SwPaM *) pPam->GetNext();
            if(bHasMark || pPam == pEnd)
                break;
        }

        // Wenn keine Selektion besteht, eine ueber das ganze Dokument aufspannen.
        if(!bHasMark)
        {
            if( pShell )
            {
                pShell->Push();
                pShell->SttEndDoc(sal_True);
                pShell->SetMark();
                pShell->SttEndDoc(sal_False);
            }
            else
            {
                pPam = new SwPaM( *pPam );
                pPam->Move( fnMoveBackward, fnGoDoc );
                pPam->SetMark();
                pPam->Move( fnMoveForward, fnGoDoc );
            }
        }
        // pPam ist immer noch der akt. Cursor !!
    }
    else
    {
        // keine Shell oder alles schreiben -> eigenen Pam erzeugen
        SwDoc* pOutDoc = pDoc ? pDoc : &rDoc;
        pPam = new SwPaM( pOutDoc->GetNodes().GetEndOfContent() );
        if( pOutDoc->IsClipBoard() )
        {
            pPam->Move( fnMoveBackward, fnGoDoc );
            pPam->SetMark();
            pPam->Move( fnMoveForward, fnGoDoc );
        }
        else
        {
            pPam->SetMark();
            pPam->Move( fnMoveBackward, fnGoDoc );
        }
    }

    rxWriter->bWriteAll = bWriteAll;
    SwDoc* pOutDoc = pDoc ? pDoc : &rDoc;

    // falls der Standart PageDesc. immer noch auf initalen Werten steht
    // (wenn z.B. kein Drucker gesetzt wurde) dann setze jetzt auf DIN A4
    // #i37248# - Modifications are only allowed at a new document.
    // <pOutDoc> contains a new document, if <pDoc> is set - see above.
    if ( pDoc && !pOutDoc->getPrinter( false ) )
    {
        const SwPageDesc& rPgDsc = pOutDoc->GetPageDesc( 0 );
        //const SwPageDesc& rPgDsc = *pOutDoc->GetPageDescFromPool( RES_POOLPAGE_STANDARD );
        const SwFmtFrmSize& rSz = rPgDsc.GetMaster().GetFrmSize();
        // Clipboard-Dokument wird immer ohne Drucker angelegt, so ist
        // der Std.PageDesc immer aug LONG_MAX !! Mappe dann auf DIN A4
        if( LONG_MAX == rSz.GetHeight() || LONG_MAX == rSz.GetWidth() )
        {
            SwPageDesc aNew( rPgDsc );
            SwFmtFrmSize aNewSz( rSz );
            Size a4(SvxPaperInfo::GetPaperSize( PAPER_A4 ));
            aNewSz.SetHeight( a4.Width() );
            aNewSz.SetWidth( a4.Height() );
            aNew.GetMaster().SetFmtAttr( aNewSz );
            pOutDoc->ChgPageDesc( 0, aNew );
        }
    }

    sal_Bool bLockedView(sal_False);
    SwEditShell* pESh = pOutDoc->GetEditShell();
    if( pESh )
    {
        bLockedView = pESh->IsViewLocked();
        pESh->LockView( sal_True );    //lock visible section
        pESh->StartAllAction();
    }

    sal_Bool bWasPurgeOle = pOutDoc->get(IDocumentSettingAccess::PURGE_OLE);
    pOutDoc->set(IDocumentSettingAccess::PURGE_OLE, false);

    sal_uLong nError = 0;
    if( pMedium )
        nError = rxWriter->Write( *pPam, *pMedium, pRealFileName );
    else if( pStg )
        nError = rxWriter->Write( *pPam, *pStg, pRealFileName );
    else if( pStrm )
        nError = rxWriter->Write( *pPam, *pStrm, pRealFileName );
    else if( xStg.is() )
        nError = rxWriter->Write( *pPam, xStg, pRealFileName );

    pOutDoc->set(IDocumentSettingAccess::PURGE_OLE, bWasPurgeOle );

    if( pESh )
    {
        pESh->EndAllAction();
        pESh->LockView( bLockedView );
    }

    // Falls nur zum Schreiben eine Selektion aufgespannt wurde, vor der
    // Rueckkehr den alten Crsr wieder herstellen.
    if( !bWriteAll && ( pShell || pOutPam ))
    {
        if(!bHasMark)
        {
            if( pShell )
                pShell->Pop( sal_False );
            else
                delete pPam;
        }
    }
    else
    {
        delete pPam;            // loesche den hier erzeugten Pam
        // Alles erfolgreich geschrieben? Sag' das dem Dokument!
        if ( !IsError( nError ) && !pDoc )
        {
            rDoc.ResetModified();
            // #i38810# - reset also flag, that indicates updated links
            rDoc.SetLinksUpdated( sal_False );
        }
    }

    if ( pDoc )
    {
        if ( !pDoc->release() )
            delete pDoc;
        bWriteAll = sal_False;
    }

    return nError;
}



// ----------------------------------------------------------------------


sal_Bool SetHTMLTemplate( SwDoc & rDoc )
{
    // Vorlagennamen von den Sfx-HTML-Filter besorgen!!!
    if( !ReadHTML->GetTemplateDoc() )
        ReadHTML->MakeHTMLDummyTemplateDoc();

    sal_Bool bRet = ReadHTML->SetTemplate( rDoc );

    SwNodes& rNds = rDoc.GetNodes();
    SwNodeIndex aIdx( rNds.GetEndOfExtras(), 1 );
    SwCntntNode* pCNd = rNds.GoNext( &aIdx );
    if( pCNd )
    {
        pCNd->SetAttr
            ( SwFmtPageDesc(rDoc.GetPageDescFromPool(RES_POOLPAGE_HTML, false) ) );
        pCNd->ChgFmtColl( rDoc.GetTxtCollFromPool( RES_POOLCOLL_TEXT, false ));
    }

    return bRet;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
