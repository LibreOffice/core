/*************************************************************************
 *
 *  $RCSfile: shellio.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 10:59:15 $
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
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#include <hintids.hxx>

#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVXLINKMGR_HXX
#include <svx/linkmgr.hxx>
#endif
#ifndef _SVX_PAPERINF_HXX //autogen
#include <svx/paperinf.hxx>
#endif

#ifndef _TOOLS_TEMPFILE_HXX
#include <tools/tempfile.hxx>
#endif
#ifndef _NODE_HXX //autogen
#include <node.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _CRSRSH_HXX
#include <crsrsh.hxx>
#endif
#ifndef _UNDOBJ_HXX
#include <undobj.hxx>           // fuer Undo Insert-Dokument
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>           // fuer Undo Insert-Dokument
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _TBLSEL_HXX
#include <tblsel.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _FLTINI_HXX
#include <fltini.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _SW3IO_HXX
#include <sw3io.hxx>
#endif
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
#ifndef _LINKENUM_HXX
#include <linkenum.hxx>
#endif
#ifndef _SWSWERROR_H
#include <swerror.h>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::rtl;

//////////////////////////////////////////////////////////////////////////

ULONG SwReader::Read( const Reader& rOptions )
{
    // Variable uebertragen
    Reader* po = (Reader*) &rOptions;
    po->pStrm = pStrm;
    po->pStg  = pStg;
    po->bInsertMode = 0 != pCrsr;

    // ist ein Medium angegeben, dann aus diesem die Streams besorgen
    if( 0 != (po->pMedium = pMedium ) &&
        !po->SetStrmStgPtr() )
    {
        po->SetReadUTF8( FALSE );
        return ERR_SWG_FILE_FORMAT_ERROR;
    }

    ULONG nError = 0L;

    GetDoc();

    // am Sw3-Reader noch den pIo-Pointer "loeschen"
    if( po == ReadSw3 && pDoc->GetDocShell() &&
        ((Sw3Reader*)po)->GetSw3Io() != pDoc->GetDocShell()->GetIoSystem() )
            ((Sw3Reader*)po)->SetSw3Io( pDoc->GetDocShell()->GetIoSystem() );

    // waehrend des einlesens kein OLE-Modified rufen
    Link aOLELink( pDoc->GetOle2Link() );
    pDoc->SetOle2Link( Link() );

    pDoc->bInReading = TRUE;

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
        if( !pDoc->IsHTMLMode() || ReadHTML != po || !po->pTemplate  )
            po->SetTemplate( *pDoc );
    }

    // Pams sind ringfoermig verkettet. Aufhoeren, wenn man wieder beim
    // ersten ist.
    SwPaM *pEnd = pPam;
    SwUndoInsDoc* pUndo = 0L;

    BOOL bReadPageDescs = FALSE;
    BOOL bDocUndo = pDoc->DoesUndo();
    BOOL bSaveUndo = bDocUndo && pCrsr;
    if( bSaveUndo )
    {
        // das Einlesen von Seitenvorlagen ist nicht Undofaehig!
        if( 0 != ( bReadPageDescs = po->aOpt.IsPageDescs() ) )
        {
            bSaveUndo = FALSE;
            pDoc->DelAllUndoObj();
        }
        else
        {
            pDoc->ClearRedo();
            pDoc->StartUndo( UNDO_INSDOKUMENT );
        }
    }
    pDoc->DoUndo( FALSE );

    SwNodeIndex aSplitIdx( pDoc->GetNodes() );

    SwRedlineMode eOld = pDoc->GetRedlineMode();
    pDoc->SetRedlineMode_intern( REDLINE_IGNORE );

    // Array von FlyFormaten
    SwSpzFrmFmts aFlyFrmArr;
    // only read templates? then ignore multi selection!
    BOOL bFmtsOnly = po->aOpt.IsFmtsOnly();

    while( TRUE )
    {
        if( bSaveUndo )
            pUndo = new SwUndoInsDoc( *pPam );

        SwPaM* pUndoPam = 0L;
        if( bDocUndo || pCrsr )
        {
            // Pam auf den Node davor setzen damit er nicht mit verschoben wird
            const SwNodeIndex& rTmp = pPam->GetPoint()->nNode;
            pUndoPam = new SwPaM( rTmp, rTmp, 0, -1 );
        }

        // Speicher mal alle Fly's
        if( pCrsr )
            aFlyFrmArr.Insert( pDoc->GetSpzFrmFmts(), 0L );

        xub_StrLen nSttCntnt = pPam->GetPoint()->nContent.GetIndex();

        // damit fuer alle Reader die Ende-Position immer stimmt, hier
        // pflegen.
        SwCntntNode* pCNd = pPam->GetCntntNode();
        xub_StrLen nEndCntnt = pCNd ? pCNd->Len() - nSttCntnt : 0;
        SwNodeIndex aEndPos( pPam->GetPoint()->nNode, 1 );

        nError = po->Read( *pDoc, *pPam, aFileName );

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
            for( USHORT n = 0; n < pDoc->GetSpzFrmFmts()->Count(); ++n )
            {
                SwFrmFmt* pFrmFmt = (*pDoc->GetSpzFrmFmts())[ n ];
                const SwFmtAnchor& rAnchor = pFrmFmt->GetAnchor();
                if( USHRT_MAX == aFlyFrmArr.GetPos( pFrmFmt) )
                {
                    if( FLY_PAGE == rAnchor.GetAnchorId() ||
                        ( FLY_AT_CNTNT == rAnchor.GetAnchorId() &&
                            rAnchor.GetCntntAnchor() &&
                            ( pUndoPam->GetPoint()->nNode ==
                            rAnchor.GetCntntAnchor()->nNode ||
                            pUndoPam->GetMark()->nNode ==
                            rAnchor.GetCntntAnchor()->nNode ) ) )
                    {
                        if( bChkHeaderFooter &&
                            FLY_AT_CNTNT == rAnchor.GetAnchorId() &&
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
                                pDoc->AppendUndo( new SwUndoInsLayFmt( pFrmFmt ) );
                            if( pFrmFmt->GetDepends() )
                            {
                                // beim Insert legen Draw-Objecte einen Frame an
                                // also weg damit.
                                pFrmFmt->DelFrms();
                            }

                            if( FLY_PAGE == rAnchor.GetAnchorId() )
                            {
                                if( !rAnchor.GetCntntAnchor() )
                                    pFrmFmt->MakeFrms();
                                else if( pCrsr )
                                    // seitengebundene Flys eingefuegt, dann schalte
                                    // die Optimierungs-Flags vom SwDoc ab. Sonst
                                    // werden die Flys nicht an der Position erzeugt.
                                    pDoc->SetLoaded( FALSE );
                            }
                            else
                                pFrmFmt->MakeFrms();
                        }
                    }
                }
            }
            if( aFlyFrmArr.Count() )
                aFlyFrmArr.Remove( 0, aFlyFrmArr.Count() );

            pDoc->SetRedlineMode_intern( eOld );
            if( pDoc->IsRedlineOn() )
                pDoc->AppendRedline( new SwRedline( REDLINE_INSERT, *pUndoPam ));
            else
                pDoc->SplitRedline( *pUndoPam );
            pDoc->SetRedlineMode_intern( REDLINE_IGNORE );
        }
        if( bSaveUndo )
        {
            pUndo->SetInsertRange( *pUndoPam, FALSE );
            pDoc->AppendUndo( pUndo );
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

    pDoc->bInReading = FALSE;
    pDoc->SetAllUniqueFlyNames();

    if( bReadPageDescs )
        pDoc->DoUndo( TRUE );
    else
    {
        pDoc->DoUndo( bDocUndo );
        if( bSaveUndo )
            pDoc->EndUndo( UNDO_INSDOKUMENT );
    }

    // Wenn der Pam nur fuers Lesen konstruiert wurde, jetzt zerstoeren.
    if( !pCrsr )
    {
        delete pPam;          // ein neues aufgemacht.

        // alle Links updaten und Fehler melden
        // (die Graphic-Links nicht, passiert ueber unseren Grafik-Cache!!)
        // JP 20.03.96: aber nicht wenn die DocShell als INTERNAL
        //              construiert wurde (FileLinks in FileLinks in ...)
        // JP 27.06.96: wenn internal, dann nie Updaten! (rekursionen werden
        //              sonst nicht erkannt! ( Bug )

        SfxObjectCreateMode eMode;
        USHORT nLinkMode = pDoc->GetLinkUpdMode();
        if( nLinkMode != NEVER && pDoc->GetDocShell() &&
            pDoc->GetLinkManager().GetLinks().Count() &&
            SFX_CREATE_MODE_INTERNAL !=
                        ( eMode = pDoc->GetDocShell()->GetCreateMode()) &&
            SFX_CREATE_MODE_ORGANIZER != eMode &&
            SFX_CREATE_MODE_PREVIEW != eMode &&
            !pDoc->GetDocShell()->IsPreview() )
        {
            ViewShell* pVSh = 0;
            if( pDoc->GetRootFrm() && !pDoc->GetEditShell( &pVSh ) && !pVSh )
            {
                ViewShell aVSh( pDoc, ::GetSpellChecker(), ::GetHyphenator(),
                                0, 0 );

                SET_CURR_SHELL( &aVSh );
                pDoc->GetLinkManager().UpdateAllLinks( nLinkMode == MANUAL, TRUE, FALSE );
            }
            else
                pDoc->GetLinkManager().UpdateAllLinks( nLinkMode == MANUAL, TRUE, FALSE );
        }

        eOld = (SwRedlineMode)(pDoc->GetRedlineMode() & ~REDLINE_IGNORE);

        pDoc->SetFieldsDirty( FALSE );
    }

    pDoc->SetRedlineMode_intern( eOld );
    pDoc->SetOle2Link( aOLELink );

    if( pCrsr )                 // das Doc ist jetzt modifiziert
        pDoc->SetModified();

    if( po == ReadSw3 )         // am Sw3-Reader noch den pIo-Pointer "loeschen"
        ((Sw3Reader*)po)->SetSw3Io( 0 );

    po->SetReadUTF8( FALSE );
    return nError;
}


/*
 * Konstruktoren, Destruktor
 */

// Initiales Einlesben


SwReader::SwReader( SvStream& rStrm, const String& rFileName, SwDoc *pDoc )
    : SwDocFac( pDoc ),
    pStrm( &rStrm ),
    pStg( 0 ),
    pMedium( 0 ),
    aFileName( rFileName ),
    pCrsr( 0 )
{
}


SwReader::SwReader( SvStorage& rStg, const String& rFileName, SwDoc *pDoc )
    : SwDocFac( pDoc ),
    pStrm( 0 ),
    pStg( &rStg ),
    pMedium( 0 ),
    aFileName( rFileName ),
    pCrsr( 0 )
{
}


SwReader::SwReader( SfxMedium& rMedium, const String& rFileName, SwDoc *pDoc )
    : SwDocFac( pDoc ),
    pStrm( 0 ),
    pStg( 0 ),
    pMedium( &rMedium ),
    aFileName( rFileName ),
    pCrsr( 0 )
{
}

// In ein existierendes Dokument einlesen

SwReader::SwReader( SvStream& rStrm, const String& rFileName, SwPaM& rPam )
    : SwDocFac( rPam.GetDoc() ),
    aFileName( rFileName ),
    pStrm( &rStrm ),
    pStg( 0 ),
    pMedium( 0 ),
    pCrsr( &rPam  )
{
}

SwReader::SwReader( SvStorage& rStg, const String& rFileName, SwPaM& rPam )
    : SwDocFac( rPam.GetDoc() ),
    aFileName( rFileName ),
    pStg( &rStg ),
    pStrm( 0 ),
    pMedium( 0 ),
    pCrsr( &rPam )
{
}


SwReader::SwReader( SfxMedium& rMedium, const String& rFileName, SwPaM& rPam )
    : SwDocFac( rPam.GetDoc() ),
    aFileName( rFileName ),
    pStg( 0 ),
    pStrm( 0 ),
    pMedium( &rMedium ),
    pCrsr( &rPam )
{
}


Reader::Reader()
    : pStrm(0), pStg(0), pMedium(0), pTemplate(0),
    bTmplBrowseMode( FALSE ), bInsertMode( FALSE ),
    bReadUTF8( FALSE )
{
}

Reader::~Reader()
{
    delete pTemplate;
}

// Die Filter-Vorlage laden, setzen und wieder freigeben
SwDoc* Reader::GetTemplateDoc()
{
    if( !aTemplateNm.Len() )
        ClearTemplate();
    else
    {
        INetURLObject aTDir( URIHelper::SmartRelToAbs(aTemplateNm) );
        DateTime aCurrDateTime;
        BOOL bLoad = FALSE;

        // Wenn das Template schon mal geladen wurde, nur einmal pro
        // Minute nachschauen, ob es geaendert wurde.
        if( !pTemplate || aCurrDateTime >= aChkDateTime )
        {
                try
                {
                    ::ucb::Content aTestContent(
                        aTDir.GetMainURL(), uno::Reference< XCommandEnvironment > ());
                    uno::Any aAny = aTestContent.getPropertyValue( OUString::createFromAscii("DateModified") );
                    if(aAny.hasValue())
                    {
                         const util::DateTime* pDT = (util::DateTime*)aAny.getValue();
                        Date aTestDate(pDT->Day, pDT->Month, pDT->Year);
                        Time aTestTime(pDT->Hours, pDT->Minutes, pDT->Seconds, pDT->HundredthSeconds);

                        if( !pTemplate || aDStamp != aTestDate ||
                                            aTStamp != aTestTime )
                        {
                            bLoad = TRUE;
                            aDStamp = aTestDate;
                            aTStamp = aTestTime;
                        }
                    }
                }
                catch(...)
                {
                }

            // Erst in einer Minute wieder mal nachschauen, ob sich die
            // Vorlage geaendert hat.
            aChkDateTime = aCurrDateTime;
            aChkDateTime += Time( 0L, 1L );
        }

        if( bLoad )
        {
            ClearTemplate();

            SvStorageRef xStor( new SvStorage( aTDir.GetFull(), STREAM_READ ));
            pTemplate = new SwDoc;
            pTemplate->AddLink();

            // sicher ist sicher
            pTemplate->SetBrowseMode( bTmplBrowseMode );

            Sw3Io aIO( *pTemplate );
            aIO.LoadStyles( xStor );
        }
#ifdef DBG_UTIL
                BOOL bExist = FALSE;
                try
                {
                    ::ucb::Content aTestContent(
                        aTDir.GetMainURL(), uno::Reference< XCommandEnvironment > ());
                        bExist = aTestContent.isDocument();

                }
                catch(...)
                {
                }

        ASSERT( !pTemplate || bExist ||
            aTemplateNm.EqualsAscii( "$$Dummy$$" ),
            "TemplatePtr aber Template existiert nicht!" );
#endif
    }

    return pTemplate;
}

BOOL Reader::SetTemplate( SwDoc& rDoc )
{
    BOOL bRet = FALSE;

    GetTemplateDoc();
    if( pTemplate )
    {
        rDoc.ReplaceStyles( *pTemplate );
        rDoc.SetFixFields();
        bRet = TRUE;
    }

    return bRet;
}

void Reader::ClearTemplate()
{
    if( pTemplate )
    {
        if( 0 == pTemplate->RemoveLink() )
            delete pTemplate,
        pTemplate = 0;
    }
}

void Reader::SetTemplateName( const String& rDir )
{
    if( rDir.Len() && aTemplateNm != rDir )
    {
        ClearTemplate();
        aTemplateNm = rDir;
    }
}

void Reader::MakeHTMLDummyTemplateDoc()
{
    ClearTemplate();
    pTemplate = new SwDoc;
    pTemplate->AddLink();
    pTemplate->SetBrowseMode( bTmplBrowseMode );
    pTemplate->GetPrt( TRUE );
    aChkDateTime = Date( 1, 1, 2300 );  // 2300. Jahrtausend sollte reichen
    aTemplateNm.AssignAscii( "$$Dummy$$" );
}

// alle die die Streams / Storages nicht geoeffnet brauchen,
// muessen die Methode ueberladen
int Reader::SetStrmStgPtr()
{
    ASSERT( pMedium, "Wo ist das Medium??" );

    if( pMedium->IsStorage() )
    {
        if( SW_STORAGE_READER & GetReaderType() )
        {
            pStg = pMedium->GetStorage();
            return TRUE;
        }
    }
    else if( SW_STREAM_READER & GetReaderType() )
    {
        pStrm = pMedium->GetInStream();
        return TRUE;
    }
    return FALSE;
}


int Reader::GetReaderType()
{
    return SW_STREAM_READER;
}


void Reader::SetFltName( const String& )
{
}


void Reader::SetNoOutlineNum( SwDoc& rDoc )
{
    // JP 10.03.96: jetzt wieder keine Nummerierung in den Vorlagen

#if 0
    //JP 18.01.96: Alle Ueberschriften sind normalerweise ohne
    //              Kapitelnummer. Darum hier explizit abschalten
    //              weil das Default jetzt wieder auf AN ist.
    SwNumRules aRules( OUTLINE_RULES );
    if( rDoc.GetOutlineNumRules() )
        aRules = *rDoc.GetOutlineNumRules();
    for( BYTE n = 0; n < MAXLEVEL; ++n )
    {
        SwNumFmt aFmt( aRules.Get( n ) );
        aFmt.eType = NUMBER_NONE;
        aRules.Set( n, aFmt );
    }
    rDoc.SetOutlineNumRules( aRules );

    // und UeberschirftBasis ohne Einrueckung!
    SwTxtFmtColl* pCol = rDoc.GetTxtCollFromPool( RES_POOLCOLL_HEADLINE_BASE );
    pCol->ResetAttr( RES_LR_SPACE );
#endif
}


void Reader::ResetFrmFmtAttrs( SfxItemSet &rFrmSet )
{
    rFrmSet.Put( SvxLRSpaceItem() );
    rFrmSet.Put( SvxULSpaceItem() );
    rFrmSet.Put( SvxBoxItem() );
}


void Reader::ResetFrmFmts( SwDoc& rDoc )
{
    for( USHORT i=0; i<3; i++ )
    {
        USHORT nPoolId;
        switch( i )
        {
        case 0: nPoolId = RES_POOLFRM_FRAME;    break;
        case 1: nPoolId = RES_POOLFRM_GRAPHIC;  break;
        case 2: nPoolId = RES_POOLFRM_OLE;      break;
        }

        SwFrmFmt *pFrmFmt = rDoc.GetFrmFmtFromPool( nPoolId );

        pFrmFmt->ResetAttr( RES_LR_SPACE );
        pFrmFmt->ResetAttr( RES_UL_SPACE );
        pFrmFmt->ResetAttr( RES_BOX );
    }
}

// ------------------------------------------------
BOOL SwReader::HasGlossaries( const Reader& rOptions )
{
    // Variable uebertragen
    Reader* po = (Reader*) &rOptions;
    po->pStrm = pStrm;
    po->pStg  = pStg;
    po->bInsertMode = FALSE;

    // ist ein Medium angegeben, dann aus diesem die Streams besorgen
    BOOL bRet = FALSE;
    if( !( 0 != (po->pMedium = pMedium ) && !po->SetStrmStgPtr() ))
        bRet = po->HasGlossaries();
    return bRet;
}

BOOL SwReader::ReadGlossaries( const Reader& rOptions,
                                SwTextBlocks& rBlocks, BOOL bSaveRelFiles )
{
    // Variable uebertragen
    Reader* po = (Reader*) &rOptions;
    po->pStrm = pStrm;
    po->pStg  = pStg;
    po->bInsertMode = FALSE;

    // ist ein Medium angegeben, dann aus diesem die Streams besorgen
    BOOL bRet = FALSE;
    if( !( 0 != (po->pMedium = pMedium ) && !po->SetStrmStgPtr() ))
        bRet = po->ReadGlossaries( rBlocks, bSaveRelFiles );
    return bRet;
}

BOOL Reader::HasGlossaries() const
{
    return FALSE;
}

BOOL Reader::ReadGlossaries( SwTextBlocks&, BOOL ) const
{
    return FALSE;
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


SwWriter::SwWriter( SvStream& rStrm, SwCrsrShell &rShell, BOOL bWriteAll )
    : pStrm( &rStrm ),
    pStg( 0 ),
    pMedium( 0 ),
    pShell( &rShell ),
    pOutPam( 0 ),
    rDoc( *rShell.GetDoc() ),
    bWriteAll( bWriteAll )
{
}


SwWriter::SwWriter(SvStream& rStrm,SwDoc &rDoc)
    :pStrm( &rStrm ),
    pStg( 0 ),
    pMedium( 0 ),
    pShell( 0 ),
    pOutPam( 0 ),
    rDoc( rDoc ),
    bWriteAll( TRUE )
{
}


SwWriter::SwWriter( SvStream& rStrm, SwPaM& rPam, BOOL bWriteAll )
    : pStrm( &rStrm ),
    pStg( 0 ),
    pMedium( 0 ),
    pShell( 0 ),
    pOutPam( &rPam ),
    rDoc( *rPam.GetDoc() ),
    bWriteAll( bWriteAll )
{
}

/*

SwWriter::SwWriter( SvStorage& rStg, SwCrsrShell &rShell, BOOL bWriteAll )
    : pStrm( 0 ),
    pStg( &rStg ),
    pMedium( 0 ),
    pShell( &rShell ),
    pOutPam( 0 ),
    rDoc( *rShell.GetDoc() ),
    bWriteAll( bWriteAll )
{
}
*/


SwWriter::SwWriter(SvStorage& rStg,SwDoc &rDoc)
    :pStrm( 0 ),
    pStg( &rStg ),
    pMedium( 0 ),
    pShell( 0 ),
    pOutPam( 0 ),
    rDoc( rDoc ),
    bWriteAll( TRUE )
{
}
/*

SwWriter::SwWriter( SvStorage& rStg, SwPaM& rPam, BOOL bWriteAll )
    : pStrm( 0 ),
    pStg( &rStg ),
    pMedium( 0 ),
    pShell( 0 ),
    pOutPam( &rPam ),
    rDoc( *rPam.GetDoc() ),
    bWriteAll( bWriteAll )
{
}
*/

SwWriter::SwWriter( SfxMedium& rMedium, SwCrsrShell &rShell, BOOL bWriteAll )
    : pStrm( 0 ),
    pStg( 0 ),
    pMedium( &rMedium ),
    pShell( &rShell ),
    pOutPam( 0 ),
    rDoc( *rShell.GetDoc() ),
    bWriteAll( bWriteAll )
{
}


SwWriter::SwWriter( SfxMedium& rMedium, SwDoc &rDoc)
    :pStrm( 0 ),
    pStg( 0 ),
    pMedium( &rMedium ),
    pShell( 0 ),
    pOutPam( 0 ),
    rDoc( rDoc ),
    bWriteAll( TRUE )
{
}

/*

SwWriter::SwWriter( SfxMedium& rMedium, SwPaM& rPam, BOOL bWriteAll )
    : pStrm( 0 ),
    pStg( 0 ),
    pShell( 0 ),
    pMedium( &rMedium ),
    pOutPam( &rPam ),
    rDoc( *rPam.GetDoc() ),
    bWriteAll( bWriteAll )
{
}
*/


ULONG SwWriter::Write( WriterRef& rxWriter, const String* pRealFileName )
{
    BOOL bHasMark = FALSE;
    SwPaM * pPam;

    SwDoc *pDoc = 0L;

    if ( pShell && !bWriteAll && pShell->IsTableMode() )
    {
        bWriteAll = TRUE;
        pDoc = new SwDoc;
        pDoc->AddLink();

        // kopiere Teile aus einer Tabelle: lege eine Tabelle mit der Breite
        // von der Originalen an und kopiere die selectierten Boxen.
        // Die Groessen werden prozentual korrigiert.

        // lasse ueber das Layout die Boxen suchen
        SwSelBoxes aBoxes;
        GetTblSel( *pShell, aBoxes );
        SwTableNode* pTblNd = (SwTableNode*)aBoxes[0]->GetSttNd()->FindStartNode();
        SwNodeIndex aIdx( pDoc->GetNodes().GetEndOfExtras(), 2 );
        SwCntntNode *pNd = aIdx.GetNode().GetCntntNode();
        ASSERT( pNd, "Node not found" );
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
        while(TRUE)
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
                pShell->SttDoc();
                pShell->SetMark();
                pShell->EndDoc();
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
        pPam->Move( fnMoveBackward, fnGoDoc );
        pPam->SetMark();
        pPam->Move( fnMoveForward, fnGoDoc );
    }

    rxWriter->bWriteAll = bWriteAll;
    SwDoc* pOutDoc = pDoc ? pDoc : &rDoc;

    // falls der Standart PageDesc. immer noch auf initalen Werten steht
    // (wenn z.B. kein Drucker gesetzt wurde) dann setze jetzt auf DIN A4
    if( !pOutDoc->GetPrt() )
    {
        const SwPageDesc& rPgDsc = pOutDoc->GetPageDesc( 0L );
        //const SwPageDesc& rPgDsc = *pOutDoc->GetPageDescFromPool( RES_POOLPAGE_STANDARD );;
        const SwFmtFrmSize& rSz = rPgDsc.GetMaster().GetFrmSize();
        // Clipboard-Dokument wird immer ohne Drucker angelegt, so ist
        // der Std.PageDesc immer aug LONG_MAX !! Mappe dann auf DIN A4
        if( LONG_MAX == rSz.GetHeight() || LONG_MAX == rSz.GetWidth() )
        {
            SwPageDesc aNew( rPgDsc );
            SwFmtFrmSize aNewSz( rSz );
            aNewSz.SetHeight( lA4Height );
            aNewSz.SetWidth( lA4Width );
            aNew.GetMaster().SetAttr( aNewSz );
            pOutDoc->ChgPageDesc( 0, aNew );
        }
    }

    BOOL bWasPurgeOle = pOutDoc->IsPurgeOLE();
    pOutDoc->SetPurgeOLE( FALSE );

    ULONG nError = 0;
    if( pMedium )
        nError = rxWriter->Write( *pPam, *pMedium, pRealFileName );
    else if( pStg )
        nError = rxWriter->Write( *pPam, *pStg, pRealFileName );
    else if( pStrm )
        nError = rxWriter->Write( *pPam, *pStrm, pRealFileName );

    pOutDoc->SetPurgeOLE( bWasPurgeOle );

    // Falls nur zum Schreiben eine Selektion aufgespannt wurde, vor der
    // Rueckkehr den alten Crsr wieder herstellen.
    if( !bWriteAll && ( pShell || pOutPam ))
    {
        if(!bHasMark)
        {
            if( pShell )
                pShell->Pop( FALSE );
            else
                delete pPam;
        }
    }
    else
    {
        delete pPam;            // loesche den hier erzeugten Pam
        // Alles erfolgreich geschrieben? Sag' das dem Dokument!
        if( !IsError( nError ) && !pDoc )
            rDoc.ResetModified();
    }

    if ( pDoc )
    {
        if ( !pDoc->RemoveLink() )
            delete pDoc;
        bWriteAll = FALSE;
    }

    return nError;
}


/*  */

String GetTmpFileName()
{
    String sRet = TempFile::CreateTempName(0);
    return sRet;
}


// ----------------------------------------------------------------------


BOOL SetHTMLTemplate( SwDoc & rDoc )
{
    // Vorlagennamen von den Sfx-HTML-Filter besorgen!!!
    if( !ReadHTML->GetTemplateDoc() )
        ReadHTML->MakeHTMLDummyTemplateDoc();

    BOOL bRet = ReadHTML->SetTemplate( rDoc );

    SwNodes& rNds = rDoc.GetNodes();
    SwNodeIndex aIdx( rNds.GetEndOfExtras(), 1 );
    SwCntntNode* pCNd = rNds.GoNext( &aIdx );
    if( pCNd )
    {
        pCNd->SetAttr( SwFmtPageDesc(
                            rDoc.GetPageDescFromPool(RES_POOLPAGE_HTML) ) );
        pCNd->ChgFmtColl( rDoc.GetTxtCollFromPool( RES_POOLCOLL_TEXT ));
    }

    return bRet;
}

/*************************************************************************

      $Log: not supported by cvs2svn $
      Revision 1.361  2000/09/18 16:04:40  willem.vandorp
      OpenOffice header added.

      Revision 1.360  2000/06/26 12:52:39  os
      INetURLObject::SmartRelToAbs removed

      Revision 1.359  2000/06/13 09:42:56  os
      using UCB

      Revision 1.358  2000/05/11 11:51:49  tl
      if[n]def ONE_LINGU entfernt

      Revision 1.357  2000/05/08 16:42:38  jp
      Changes for Unicode

      Revision 1.356  2000/04/04 15:37:42  jp
      Bug #74638#: dont unload OLE-Objects in a save call

      Revision 1.355  2000/02/11 14:36:34  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.354  2000/02/09 11:59:21  jp
      Task #72579#: WW8Reader can import glossaries

      Revision 1.353  2000/02/02 16:59:06  jp
      Task #72579#: interface of SwReader is changed

      Revision 1.352  2000/01/20 12:53:35  jp
      Bug #72119#: HTML-Template is moved into an other directory

      Revision 1.351  2000/01/20 12:17:45  jp
      Bug #72119#: HTML-Template is moved into an other directory

      Revision 1.350  1999/10/25 19:34:19  tl
      ongoing ONE_LINGU implementation

      Revision 1.349  1999/10/21 17:49:54  jp
      have to change - SearchFile with SfxIniManager, dont use SwFinder for this

      Revision 1.348  1999/09/08 13:49:52  jp
      Bug #68593#: Read - no nulti read if formatsonlyflag set

      Revision 1.347  1999/08/31 08:53:26  TL
      #if[n]def ONE_LINGU inserted (for transition of lingu to StarOne)


      Rev 1.346   31 Aug 1999 10:53:26   TL
   #if[n]def ONE_LINGU inserted (for transition of lingu to StarOne)

      Rev 1.345   26 Aug 1999 15:45:52   OS
   double methods in PathFinder removed

      Rev 1.344   16 Jun 1999 19:53:14   JP
   Change interface of base class Writer

      Rev 1.343   04 May 1999 14:59:28   JP
   FilterExportklasse Writer von SvRef abgeleitet, damit sie immer zerstoert wird

      Rev 1.342   11 Mar 1999 23:54:40   JP
   Task #63171#: Optionen fuer Feld-/LinkUpdate Doc oder Modul lokal

      Rev 1.341   10 Mar 1999 10:40:28   JP
   Bug #63040#: bei normalem Lesen Felder als aktuell kennzeichnen

      Rev 1.340   04 Feb 1999 21:19:06   JP
   Bug #61404#/#61014#: neu: SetFixFields statt zweier Methoden

      Rev 1.339   02 Feb 1999 13:24:34   JP
   Bug #61014#: UpdateAllLinks nur rufen, wenn Liks vorhanden sind (erspart eine temp. ViewSh!)

*************************************************************************/


