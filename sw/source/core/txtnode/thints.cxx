/*************************************************************************
 *
 *  $RCSfile: thints.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: jp $ $Date: 2001-02-15 20:09:26 $
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
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SOT_FACTORY_HXX
#include <sot/factory.hxx>
#endif
#ifndef _XMLOFF_XMLCNITM_HXX
#include <xmloff/xmlcnitm.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX //autogen
#include <svx/langitem.hxx>
#endif
#ifndef _SVX_EMPHITEM_HXX //autogen
#include <svx/emphitem.hxx>
#endif
#ifndef _SVX_TWOLINESITEM_HXX
#include <svx/twolinesitem.hxx>
#endif
#ifndef _SVX_CHARSCALEITEM_HXX
#include <svx/charscaleitem.hxx>
#endif
#ifndef _SVX_CHARROTATEITEM_HXX
#include <svx/charrotateitem.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#include <com/sun/star/i18n/ScriptType.hdl>
#endif

#ifndef _TXTINET_HXX //autogen
#include <txtinet.hxx>
#endif
#ifndef _TXTFLCNT_HXX //autogen
#include <txtflcnt.hxx>
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _FMTINFMT_HXX //autogen
#include <fmtinfmt.hxx>
#endif
#ifndef _TXTATR_HXX //autogen
#include <txtatr.hxx>
#endif
#ifndef _FCHRFMT_HXX //autogen
#include <fchrfmt.hxx>
#endif
#ifndef _FMTFLCNT_HXX //autogen
#include <fmtflcnt.hxx>
#endif
#ifndef _FMTFTN_HXX //autogen
#include <fmtftn.hxx>
#endif
#ifndef _TXTTXMRK_HXX //autogen
#include <txttxmrk.hxx>
#endif
#ifndef _TXTRFMRK_HXX //autogen
#include <txtrfmrk.hxx>
#endif
#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _TXTFLD_HXX //autogen
#include <txtfld.hxx>
#endif
#ifndef _CHARATR_HXX
#include <charatr.hxx>
#endif
#ifndef _CHARFMT_HXX //autogen
#include <charfmt.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _FTNIDX_HXX //autogen
#include <ftnidx.hxx>
#endif
#ifndef _FMTRUBY_HXX
#include <fmtruby.hxx>
#endif
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif
#ifndef _CALBCK_HXX
#include <calbck.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>            // fuer SwFmtChg
#endif
#ifndef _ROLBCK_HXX
#include <rolbck.hxx>           // fuer SwRegHistory
#endif
#ifndef _DDEFLD_HXX
#include <ddefld.hxx>
#endif
#ifndef _DOCUFLD_HXX
#include <docufld.hxx>
#endif
#ifndef _EXPFLD_HXX
#include <expfld.hxx>
#endif
#ifndef _USRFLD_HXX
#include <usrfld.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _SWFONT_HXX
#include <swfont.hxx>
#endif


#ifndef PRODUCT
#define CHECK    Check();
#else
#define CHECK
#endif

using namespace ::com::sun::star::i18n;

/*************************************************************************
 *                      SwTxtNode::MakeTxtAttr()
 *************************************************************************/

    // lege ein neues TextAttribut an und fuege es SwpHints-Array ein
SwTxtAttr* SwTxtNode::MakeTxtAttr( const SfxPoolItem& rAttr,
                                   xub_StrLen nStt, xub_StrLen nEnd, BOOL bPool )
{
    // das neue Attribut im Pool anlegen
    const SfxPoolItem& rNew = bPool ? GetDoc()->GetAttrPool().Put( rAttr ) :
                              rAttr;

    SwTxtAttr* pNew = 0;
    BYTE nScript = SW_LATIN;
    switch( rNew.Which() )
    {
    case RES_CHRATR_CASEMAP:
        pNew = new SwTxtCaseMap( (SvxCaseMapItem&)rNew, nStt, nEnd );
        break;
    case RES_CHRATR_COLOR:
        pNew = new SwTxtColor( (SvxColorItem&)rNew, nStt, nEnd );
        break;
    case RES_CHRATR_CHARSETCOLOR:
        pNew = new SwTxtCharSetColor( (SvxCharSetColorItem&)rNew, nStt, nEnd );
        break;
    case RES_CHRATR_CONTOUR:
        pNew = new SwTxtContour( (SvxContourItem&)rNew, nStt, nEnd );
        break;
    case RES_CHRATR_CROSSEDOUT:
        pNew = new SwTxtCrossedOut( (SvxCrossedOutItem&)rNew, nStt, nEnd );
        break;
    case RES_CHRATR_ESCAPEMENT:
        pNew = new SwTxtEscapement( (SvxEscapementItem&)rNew, nStt, nEnd );
        break;
    case RES_CHRATR_CTL_FONT: nScript += SW_CTL - SW_CJK;   // no break;
    case RES_CHRATR_CJK_FONT: nScript += SW_CJK - SW_LATIN; // no break;
    case RES_CHRATR_FONT:
        pNew = new SwTxtFont( (SvxFontItem&)rNew, nStt, nEnd, nScript );
        break;
    case RES_CHRATR_CTL_FONTSIZE: nScript += SW_CTL - SW_CJK;   // no break;
    case RES_CHRATR_CJK_FONTSIZE: nScript += SW_CJK - SW_LATIN; // no break;
    case RES_CHRATR_FONTSIZE:
        pNew = new SwTxtSize( (SvxFontHeightItem&)rNew, nStt, nEnd, nScript );
        break;
    case RES_CHRATR_KERNING:
        pNew = new SwTxtKerning( (SvxKerningItem&)rNew, nStt, nEnd );
        break;
    case RES_CHRATR_CTL_LANGUAGE: nScript += SW_CTL - SW_CJK;   // no break;
    case RES_CHRATR_CJK_LANGUAGE: nScript += SW_CJK - SW_LATIN; // no break;
    case RES_CHRATR_LANGUAGE:
        pNew = new SwTxtLanguage( (SvxLanguageItem&)rNew, nStt, nEnd, nScript );
        break;
    case RES_CHRATR_CTL_POSTURE: nScript += SW_CTL - SW_CJK;    // no break;
    case RES_CHRATR_CJK_POSTURE: nScript += SW_CJK - SW_LATIN;  // no break;
    case RES_CHRATR_POSTURE:
        pNew = new SwTxtPosture( (SvxPostureItem&)rNew, nStt, nEnd, nScript );
        break;
    case RES_CHRATR_SHADOWED:
        pNew = new SwTxtShadowed( (SvxShadowedItem&)rNew, nStt, nEnd );
        break;
    case RES_CHRATR_AUTOKERN:
        pNew = new SwTxtAutoKern( (SvxAutoKernItem&)rNew, nStt, nEnd );
        break;
    case RES_CHRATR_WORDLINEMODE:
        pNew = new SwTxtWordLineMode( (SvxWordLineModeItem&)rNew, nStt, nEnd );
        break;
    case RES_CHRATR_UNDERLINE:
        pNew = new SwTxtUnderline( (SvxUnderlineItem&)rNew, nStt, nEnd );
        break;
    case RES_CHRATR_CTL_WEIGHT: nScript += SW_CTL - SW_CJK; // no break;
    case RES_CHRATR_CJK_WEIGHT: nScript += SW_CJK - SW_LATIN;   // no break;
    case RES_CHRATR_WEIGHT:
        pNew = new SwTxtWeight( (SvxWeightItem&)rNew, nStt, nEnd, nScript );
        break;
    case RES_CHRATR_EMPHASIS_MARK:
        pNew = new SwTxtEmphasisMark( (SvxEmphasisMarkItem&)rNew, nStt, nEnd );
        break;

    case RES_TXTATR_CHARFMT:
        {
            SwFmtCharFmt &rFmtCharFmt = (SwFmtCharFmt&) rNew;
            if( !rFmtCharFmt.GetCharFmt() )
                rFmtCharFmt.SetCharFmt( GetDoc()->GetDfltCharFmt() );

            pNew = new SwTxtCharFmt( rFmtCharFmt, nStt, nEnd );
        }
        break;
    case RES_TXTATR_INETFMT:
        pNew = new SwTxtINetFmt( (SwFmtINetFmt&)rNew, nStt, nEnd );
        break;
    case RES_TXTATR_FIELD:
        pNew = new SwTxtFld( (SwFmtFld&)rNew, nStt );
        break;
    case RES_TXTATR_FLYCNT:
        {
            // erst hier wird das Frame-Format kopiert (mit Inhalt) !!
            pNew = new SwTxtFlyCnt( (SwFmtFlyCnt&)rNew, nStt );
            // Kopie von einem Text-Attribut
            if( ((SwFmtFlyCnt&)rAttr).GetTxtFlyCnt() )
                // dann muss das Format Kopiert werden
                ((SwTxtFlyCnt*)pNew)->CopyFlyFmt( GetDoc() );
        }
        break;
    case RES_TXTATR_FTN:
        pNew = new SwTxtFtn( (SwFmtFtn&)rNew, nStt );
        // ggfs. SeqNo kopieren
        if( ((SwFmtFtn&)rAttr).GetTxtFtn() )
            ((SwTxtFtn*)pNew)->SetSeqNo( ((SwFmtFtn&)rAttr).GetTxtFtn()->GetSeqRefNo() );
        break;
    case RES_TXTATR_HARDBLANK:
        pNew = new SwTxtHardBlank( (SwFmtHardBlank&)rNew, nStt );
        break;
    case RES_CHRATR_NOHYPHEN:
        pNew = new SwTxtNoHyphenHere( (SvxNoHyphenItem&)rNew, nStt, nEnd );
        break;
    case RES_CHRATR_BLINK:
        pNew = new SwTxtBlink( (SvxBlinkItem&)rNew, nStt, nEnd );
        break;
    case RES_CHRATR_BACKGROUND:
        pNew = new SwTxtBackground( (SvxBrushItem&)rNew, nStt, nEnd );
        break;
    case RES_CHRATR_TWO_LINES:
        pNew = new SwTxt2Lines( (SvxTwoLinesItem&)rNew, nStt, nEnd );
        break;
    case RES_CHRATR_ROTATE:
        pNew = new SwTxtCharRotate( (SvxCharRotateItem&)rNew, nStt, nEnd );
        break;
    case RES_CHRATR_SCALEW:
        pNew = new SwTxtCharScaleWidth( (SvxCharScaleWidthItem&)rNew, nStt, nEnd );
        break;
    case RES_TXTATR_REFMARK:
        pNew = nStt == nEnd
                ? new SwTxtRefMark( (SwFmtRefMark&)rNew, nStt )
                : new SwTxtRefMark( (SwFmtRefMark&)rNew, nStt, &nEnd );
        break;
    case RES_TXTATR_TOXMARK:
        pNew = new SwTxtTOXMark( (SwTOXMark&)rNew, nStt, &nEnd );
        break;
    case RES_UNKNOWNATR_CONTAINER:
    case RES_TXTATR_UNKNOWN_CONTAINER:
        pNew = new SwTxtXMLAttrContainer( (SvXMLAttrContainerItem&)rNew,
                                        nStt, nEnd );
        break;
    case RES_TXTATR_CJK_RUBY:
        pNew = new SwTxtRuby( (SwFmtRuby&)rNew, nStt, nEnd );
        break;
    }
    ASSERT( pNew, "was fuer ein TextAttribut soll hier angelegt werden?" );
    return pNew;
}

// loesche das Text-Attribut (muss beim Pool abgemeldet werden!)
void SwTxtNode::DestroyAttr( SwTxtAttr* pAttr )
{
    if( pAttr )
    {
        // einige Sachen muessen vorm Loeschen der "Format-Attribute" erfolgen
        SwDoc* pDoc = GetDoc();
        switch( pAttr->Which() )
        {
        case RES_TXTATR_FLYCNT:
            {
                // siehe auch die Anmerkung "Loeschen von Formaten
                // zeichengebundener Frames" in fesh.cxx, SwFEShell::DelFmt()
                SwFrmFmt* pFmt = pAttr->GetFlyCnt().GetFrmFmt();
                if( pFmt )      // vom Undo auf 0 gesetzt ??
                    pDoc->DelLayoutFmt( (SwFlyFrmFmt*)pFmt );
            }
            break;

        case RES_TXTATR_FTN:
            ((SwTxtFtn*)pAttr)->SetStartNode( 0 );
            break;

        case RES_TXTATR_FIELD:
            if( !pDoc->IsInDtor() )
            {
                // Wenn wir ein HiddenParaField sind, dann muessen wir
                // ggf. fuer eine Neuberechnung des Visible-Flags sorgen.
                const SwField* pFld = pAttr->GetFld().GetFld();

                //JP 06-08-95: DDE-Felder bilden eine Ausnahme
                ASSERT( RES_DDEFLD == pFld->GetTyp()->Which() ||
                        this == ((SwTxtFld*)pAttr)->GetpTxtNode(),
                        "Wo steht denn dieses Feld?" )

                // bestimmte Felder mussen am Doc das Calculations-Flag updaten
                switch( pFld->GetTyp()->Which() )
                {
                case RES_HIDDENPARAFLD:
                    SetCalcVisible();
                    // kein break !
                case RES_DBSETNUMBERFLD:
                case RES_GETEXPFLD:
                case RES_DBFLD:
                case RES_SETEXPFLD:
                case RES_HIDDENTXTFLD:
                case RES_DBNUMSETFLD:
                case RES_DBNEXTSETFLD:
                    if( !pDoc->IsNewFldLst() && GetNodes().IsDocNodes() )
                        pDoc->InsDelFldInFldLst( FALSE, *(SwTxtFld*)pAttr );
                    break;
                case RES_DDEFLD:
                    if( GetNodes().IsDocNodes() &&
                        ((SwTxtFld*)pAttr)->GetpTxtNode() )
                        ((SwDDEFieldType*)pFld->GetTyp())->DecRefCnt();
                    break;
                }
            }
            break;

        }
        pAttr->RemoveFromPool( pDoc->GetAttrPool() );
        delete pAttr;
    }
}

/*************************************************************************
 *                      SwTxtNode::Insert()
 *************************************************************************/

// lege ein neues TextAttribut an und fuege es ins SwpHints-Array ein
SwTxtAttr* SwTxtNode::Insert( const SfxPoolItem& rAttr,
                              xub_StrLen nStt, xub_StrLen nEnd, USHORT nMode )
{
    SwTxtAttr* pNew = MakeTxtAttr( rAttr, nStt, nEnd );
    return (pNew && Insert( pNew, nMode )) ? pNew : 0;
}


// uebernehme den Pointer auf das Text-Attribut

BOOL SwTxtNode::Insert( SwTxtAttr *pAttr, USHORT nMode )
{
    BOOL bHiddenPara = FALSE;

    ASSERT( *pAttr->GetStart() <= Len(), "StartIdx hinter Len!" );

    if( !pAttr->GetEnd() )
    {
        USHORT nInsMode = nMode;
        switch( pAttr->Which() )
        {
            case RES_TXTATR_FLYCNT:
            {
                SwTxtFlyCnt *pFly = (SwTxtFlyCnt *)pAttr;
                SwFrmFmt* pFmt = pAttr->GetFlyCnt().GetFrmFmt();
                if( !(SETATTR_NOTXTATRCHR & nInsMode) )
                {
                    // Wir muessen zuerst einfuegen, da in SetAnchor()
                    // dem FlyFrm GetStart() uebermittelt wird.
                    //JP 11.05.98: falls das Anker-Attribut schon richtig
                    // gesetzt ist, dann korrigiere dieses nach dem Einfuegen
                    // des Zeichens. Sonst muesste das immer  ausserhalb
                    // erfolgen (Fehleranfaellig !)
                    const SwFmtAnchor* pAnchor = 0;
                    pFmt->GetItemState( RES_ANCHOR, FALSE,
                                            (const SfxPoolItem**)&pAnchor );

                    SwIndex aIdx( this, *pAttr->GetStart() );
                    Insert( GetCharOfTxtAttr(*pAttr), aIdx );
                    nInsMode |= SETATTR_NOTXTATRCHR;

                    if( pAnchor && FLY_IN_CNTNT == pAnchor->GetAnchorId() &&
                        pAnchor->GetCntntAnchor() &&
                        pAnchor->GetCntntAnchor()->nNode == *this &&
                        pAnchor->GetCntntAnchor()->nContent == aIdx )
                        ((SwIndex&)pAnchor->GetCntntAnchor()->nContent)--;
                }
                pFly->SetAnchor( this );

                // Format-Pointer kann sich im SetAnchor geaendert haben!
                // (Kopieren in andere Docs!)
                pFmt = pAttr->GetFlyCnt().GetFrmFmt();
                SwDoc *pDoc = pFmt->GetDoc();

                if( RES_DRAWFRMFMT == pFmt->Which() &&
                    pDoc->IsInHeaderFooter(
                        pFmt->GetAnchor().GetCntntAnchor()->nNode ) )
                {
                    // das soll nicht meoglich sein; hier verhindern
                    // Der Dtor des TxtHints loescht nicht das Zeichen.
                    // Wenn ein CH_TXTATR_.. vorliegt, dann muss man
                    // dieses explizit loeschen
                    if( SETATTR_NOTXTATRCHR & nInsMode )
                    {
                        // loesche das Zeichen aus dem String !
                        ASSERT( ( CH_TXTATR_BREAKWORD ==
                                        aText.GetChar(*pAttr->GetStart() ) ||
                                  CH_TXTATR_INWORD ==
                                          aText.GetChar(*pAttr->GetStart())),
                                "where is my attribu character" );
                        aText.Erase( *pAttr->GetStart(), 1 );
                        // Indizies Updaten
                        SwIndex aTmpIdx( this, *pAttr->GetStart() );
                        Update( aTmpIdx, 1, TRUE );
                    }
                    // Format loeschen nicht ins Undo aufnehmen!!
                    BOOL bUndo = pDoc->DoesUndo();
                    pDoc->DoUndo( FALSE );
                    DestroyAttr( pAttr );
                    pDoc->DoUndo( bUndo );
                    return FALSE;
                }
                break;
            }
            case RES_TXTATR_FTN :
            {
                // Fussnoten, man kommt an alles irgendwie heran.
                // CntntNode erzeugen und in die Inserts-Section stellen
                SwDoc *pDoc = GetDoc();
                SwNodes &rNodes = pDoc->GetNodes();

                // FussNote in nicht Content-/Redline-Bereich einfuegen ??
                if( StartOfSectionIndex() < rNodes.GetEndOfAutotext().GetIndex() )
                {
                    // das soll nicht meoglich sein; hier verhindern
                    // Der Dtor des TxtHints loescht nicht das Zeichen.
                    // Wenn ein CH_TXTATR_.. vorliegt, dann muss man
                    // dieses explizit loeschen
                    if( SETATTR_NOTXTATRCHR & nInsMode )
                    {
                        // loesche das Zeichen aus dem String !
                        ASSERT( ( CH_TXTATR_BREAKWORD ==
                                        aText.GetChar(*pAttr->GetStart() ) ||
                                  CH_TXTATR_INWORD ==
                                          aText.GetChar(*pAttr->GetStart())),
                                "where is my attribu character" );
                        aText.Erase( *pAttr->GetStart(), 1 );
                        // Indizies Updaten
                        SwIndex aTmpIdx( this, *pAttr->GetStart() );
                        Update( aTmpIdx, 1, TRUE );
                    }
                    DestroyAttr( pAttr );
                    return FALSE;
                }

                // wird eine neue Fussnote eingefuegt ??
                BOOL bNewFtn = 0 == ((SwTxtFtn*)pAttr)->GetStartNode();
                if( bNewFtn )
                    ((SwTxtFtn*)pAttr)->MakeNewTextSection( GetNodes() );
                else if ( !GetpSwpHints() || !GetpSwpHints()->IsInSplitNode() )
                {
                    // loesche alle Frames der Section, auf die der StartNode zeigt
                    ULONG nSttIdx =
                        ((SwTxtFtn*)pAttr)->GetStartNode()->GetIndex();
                    ULONG nEndIdx = rNodes[ nSttIdx++ ]->EndOfSectionIndex();
                    SwCntntNode* pCNd;
                    for( ; nSttIdx < nEndIdx; ++nSttIdx )
                        if( 0 != ( pCNd = rNodes[ nSttIdx ]->GetCntntNode() ))
                            pCNd->DelFrms();
                }

                if( !(SETATTR_NOTXTATRCHR & nInsMode) )
                {
                    // Wir muessen zuerst einfuegen, da sonst gleiche Indizes
                    // entstehen koennen und das Attribut im _SortArr_ am
                    // Dokument nicht eingetrage wird.
                    SwIndex aNdIdx( this, *pAttr->GetStart() );
                    Insert( GetCharOfTxtAttr(*pAttr), aNdIdx );
                    nInsMode |= SETATTR_NOTXTATRCHR;
                }

                // Wir tragen uns am FtnIdx-Array des Docs ein ...
                SwTxtFtn* pTxtFtn = 0;
                if( !bNewFtn )
                {
                    // eine alte Ftn wird umgehaengt (z.B. SplitNode)
                    for( USHORT n = 0; n < pDoc->GetFtnIdxs().Count(); ++n )
                        if( pAttr == pDoc->GetFtnIdxs()[n] )
                        {
                            // neuen Index zuweisen, dafuer aus dem SortArray
                            // loeschen und neu eintragen
                            pTxtFtn = pDoc->GetFtnIdxs()[n];
                            pDoc->GetFtnIdxs().Remove( n );
                            break;
                        }
                    // wenn ueber Undo der StartNode gesetzt wurde, kann
                    // der Index noch gar nicht in der Verwaltung stehen !!
                }
                if( !pTxtFtn )
                    pTxtFtn = (SwTxtFtn*)pAttr;

                // fuers Update der Nummern und zum Sortieren
                // muss der Node gesetzt sein.
                ((SwTxtFtn*)pAttr)->ChgTxtNode( this );

                // FussNote im Redline-Bereich NICHT ins FtnArray einfuegen!
                if( StartOfSectionIndex() > rNodes.GetEndOfRedlines().GetIndex() )
                {
#ifndef PRODUCT
                    const BOOL bSuccess =
#endif
                        pDoc->GetFtnIdxs().Insert( pTxtFtn );
#ifndef PRODUCT
                    ASSERT( bSuccess, "FtnIdx nicht eingetragen." );
#endif
                }
                SwNodeIndex aTmpIndex( *this );
                pDoc->GetFtnIdxs().UpdateFtn( aTmpIndex);
                ((SwTxtFtn*)pAttr)->SetSeqRefNo();
            }
            break;

            case RES_TXTATR_FIELD:
                {
                    // fuer HiddenParaFields Benachrichtigungsmechanismus
                    // anwerfen
                    if( RES_HIDDENPARAFLD ==
                        pAttr->GetFld().GetFld()->GetTyp()->Which() )
                    bHiddenPara = TRUE;
                }
                break;

        }
        // Fuer SwTxtHints ohne Endindex werden CH_TXTATR_..
        // eingefuegt, aStart muss danach um einen zurueckgesetzt werden.
        // Wenn wir im SwTxtNode::Copy stehen, so wurde das Zeichen bereits
        // mitkopiert. In solchem Fall ist SETATTR_NOTXTATRCHR angegeben worden.
        if( !(SETATTR_NOTXTATRCHR & nInsMode) )
        {
            SwIndex aIdx( this, *pAttr->GetStart() );
            Insert( GetCharOfTxtAttr(*pAttr), aIdx );
        }
    }
    else
        ASSERT( *pAttr->GetEnd() <= Len(), "EndIdx hinter Len!" );

    if ( !pSwpHints )
        pSwpHints = new SwpHints();

    // 4263: AttrInsert durch TextInsert => kein Adjust
    pSwpHints->Insert( pAttr, *this, nMode );

    // 47375: In pSwpHints->Insert wird u.a. Merge gerufen und das Hints-Array
    // von ueberfluessigen Hints befreit, dies kann u.U. sogar der frisch
    // eingefuegte Hint pAttr sein, der dann zerstoert wird!!
    if( USHRT_MAX == pSwpHints->GetPos( pAttr ) )
        return FALSE;

    if(bHiddenPara)
        SetCalcVisible();
    return TRUE;
}


/*************************************************************************
 *                      SwTxtNode::Delete()
 *************************************************************************/

void SwTxtNode::Delete( SwTxtAttr *pAttr, BOOL bThisOnly )
{
    if ( !pSwpHints )
        return;
    if( bThisOnly )
    {
        xub_StrLen* pEndIdx = pAttr->GetEnd();
        if( !pEndIdx )
        {
            // hat es kein Ende kann es nur das sein, was hier steht!
            // Unbedingt Copy-konstruieren!
            const SwIndex aIdx( this, *pAttr->GetStart() );
            Erase( aIdx, 1 );
        }
        else
        {
            // den MsgHint jetzt fuettern, weil gleich sind
            // Start und End weg.
            SwUpdateAttr aHint( *pAttr->GetStart(), *pEndIdx, pAttr->Which() );
            pSwpHints->Delete( pAttr );
            pAttr->RemoveFromPool( GetDoc()->GetAttrPool() );
            delete pAttr;
            SwModify::Modify( 0, &aHint );     // die Frames benachrichtigen

            if( pSwpHints && pSwpHints->CanBeDeleted() )
                DELETEZ( pSwpHints );
        }

        return;
    }
    Delete( pAttr->Which(), *pAttr->GetStart(), *pAttr->GetAnyEnd() );
}

/*************************************************************************
 *                      SwTxtNode::Delete()
 *************************************************************************/

void SwTxtNode::Delete( USHORT nTxtWhich, xub_StrLen nStart, xub_StrLen nEnd )
{
    if ( !pSwpHints )
        return;

    const xub_StrLen *pEndIdx;
    const xub_StrLen *pSttIdx;
    SwTxtAttr* pTxtHt;

    for( USHORT nPos = 0; pSwpHints && nPos < pSwpHints->Count(); nPos++ )
    {
        pTxtHt = pSwpHints->GetHt( nPos );
        const USHORT nWhich = pTxtHt->Which();
        if( nWhich == nTxtWhich &&
            *( pSttIdx = pTxtHt->GetStart()) == nStart )
        {
            pEndIdx = pTxtHt->GetEnd();

            // Text-Attribute sind voellig dynamisch, so dass diese nur
            // mit ihrer Start-Position verglichen werden.
            if( !pEndIdx )
            {
                // Unbedingt Copy-konstruieren!
                const SwIndex aIdx( this, *pSttIdx );
                Erase( aIdx, 1 );
                break;
            }
            else if( *pEndIdx == nEnd )
            {
                // den MsgHint jetzt fuettern, weil gleich sind
                // Start und End weg.
                // Das CalcVisibleFlag bei HiddenParaFields entfaellt,
                // da dies das Feld im Dtor selbst erledigt.
                SwUpdateAttr aHint( *pSttIdx, *pEndIdx, nTxtWhich );
                pSwpHints->DeleteAtPos( nPos );    // gefunden, loeschen,
                pTxtHt->RemoveFromPool( GetDoc()->GetAttrPool() );
                delete pTxtHt;
                SwModify::Modify( 0, &aHint );     // die Frames benachrichtigen
                break;
            }
        }
    }
    if( pSwpHints && pSwpHints->CanBeDeleted() )
        DELETEZ( pSwpHints );
}

/*************************************************************************
 *                      SwTxtNode::DelSoftHyph()
 *************************************************************************/

void SwTxtNode::DelSoftHyph( const xub_StrLen nStart, const xub_StrLen nEnd )
{
    xub_StrLen nFndPos = nStart, nEndPos = nEnd;
    while( STRING_NOTFOUND !=
            ( nFndPos = aText.Search( CHAR_SOFTHYPHEN, nFndPos )) &&
            nFndPos < nEndPos )
    {
        const SwIndex aIdx( this, nFndPos );
        Erase( aIdx, 1 );
        --nEndPos;
    }
}

// setze diese Attribute am TextNode. Wird der gesamte Bereich umspannt,
// dann setze sie nur im AutoAttrSet (SwCntntNode:: SetAttr)
BOOL SwTxtNode::SetAttr( const SfxItemSet& rSet, xub_StrLen nStt,
                         xub_StrLen nEnd, USHORT nMode )
{
    if( !rSet.Count() )
        return FALSE;

    // teil die Sets auf (fuer Selektion in Nodes)
    const SfxItemSet* pSet = &rSet;
    SfxItemSet aTxtSet( *rSet.GetPool(), RES_TXTATR_BEGIN, RES_TXTATR_END-1 );

    // gesamter Bereich
    if( !nStt && nEnd == aText.Len() && !(nMode & SETATTR_NOFORMATATTR ) )
    {
        // sind am Node schon Zeichenvorlagen gesetzt, muss man diese Attribute
        // (rSet) immer als TextAttribute setzen, damit sie angezeigt werden.
        int bHasCharFmts = FALSE;
        if( pSwpHints )
            for( USHORT n = 0; n < pSwpHints->Count(); ++n )
                if( (*pSwpHints)[ n ]->IsCharFmtAttr() )
                {
                    bHasCharFmts = TRUE;
                    break;
                }

        if( !bHasCharFmts )
        {
            aTxtSet.Put( rSet );
            if( aTxtSet.Count() != rSet.Count() )
            {
                BOOL bRet = SwCntntNode::SetAttr( rSet );
                if( !aTxtSet.Count() )
                    return bRet;
            }
            pSet = &aTxtSet;
        }
    }

    if ( !pSwpHints )
        pSwpHints = new SwpHints();

    USHORT nWhich, nCount = 0;
    SwTxtAttr* pNew;
    SfxItemIter aIter( *pSet );
    const SfxPoolItem* pItem = aIter.GetCurItem();
    do {
        if( pItem && (SfxPoolItem*)-1 != pItem &&
            (( RES_CHRATR_BEGIN <= ( nWhich = pItem->Which()) &&
              RES_CHRATR_END > nWhich ) ||
            ( RES_TXTATR_BEGIN <= nWhich && RES_TXTATR_END > nWhich ) ||
            ( RES_UNKNOWNATR_BEGIN <= nWhich && RES_UNKNOWNATR_END > nWhich )) )
        {
            if( RES_TXTATR_CHARFMT == pItem->Which() &&
                GetDoc()->GetDfltCharFmt()==((SwFmtCharFmt*)pItem)->GetCharFmt())
            {
                SwIndex aIndex( this, nStt );
                RstAttr( aIndex, nEnd - nStt, RES_TXTATR_CHARFMT, 0 );
                DontExpandFmt( aIndex );
            }
            else
            {
                pNew = MakeTxtAttr( *pItem, nStt, nEnd );
                if( pNew )
                {
                    // Attribut ohne Ende, aber Bereich markiert ?
                    if( nEnd != nStt && !pNew->GetEnd() )
                    {
                        ASSERT( !this, "Attribut ohne Ende aber Bereich vorgegeben" );
                        DestroyAttr( pNew );        // nicht einfuegen
                    }
                    else if( Insert( pNew, nMode ))
                        ++nCount;
                }
            }
        }
        if( aIter.IsAtEnd() )
            break;
        pItem = aIter.NextItem();
    } while( TRUE );

    if( pSwpHints && pSwpHints->CanBeDeleted() )
        DELETEZ( pSwpHints );

    return nCount ? TRUE : FALSE;
}

BOOL lcl_Included( const USHORT nWhich, const SwTxtAttr *pAttr )
{
#ifdef DEBUG
    static long nTest = 0;
    ++nTest;
#endif
    BOOL bRet;
    SwCharFmt* pFmt = RES_TXTATR_INETFMT == pAttr->Which() ?
                        ((SwTxtINetFmt*)pAttr)->GetCharFmt() :
                        pAttr->GetCharFmt().GetCharFmt();
    if( pFmt )
        bRet = SFX_ITEM_SET == pFmt->GetAttrSet().GetItemState( nWhich, TRUE );
    else
        bRet = FALSE;
    return bRet;
}

void lcl_MergeAttr( SfxItemSet& rSet, const SfxPoolItem& rAttr )
{
    rSet.Put( rAttr );
}

void lcl_MergeAttr_ExpandChrFmt( SfxItemSet& rSet, const SfxPoolItem& rAttr )
{
    if( RES_TXTATR_CHARFMT == rAttr.Which() ||
        RES_TXTATR_INETFMT == rAttr.Which() )
    {
        // aus der Vorlage die Attribute holen:
        SwCharFmt* pFmt = RES_TXTATR_INETFMT == rAttr.Which() ?
                        ((SwFmtINetFmt&)rAttr).GetTxtINetFmt()->GetCharFmt() :
                        ((SwFmtCharFmt&)rAttr).GetCharFmt();
        if( pFmt )
        {
            const SfxItemSet& rCFSet = pFmt->GetAttrSet();
            SfxWhichIter aIter( rCFSet );
            register USHORT nWhich = aIter.FirstWhich();
            while( nWhich )
            {
                if( ( nWhich < RES_CHRATR_END ) &&
                    ( SFX_ITEM_SET == rCFSet.GetItemState( nWhich, TRUE ) ) )
                    rSet.Put( rCFSet.Get( nWhich ) );
                nWhich = aIter.NextWhich();
            }
#if 0
            SfxItemSet aTmpSet( *rSet.GetPool(), rSet.GetRanges() );
            aTmpSet.Set( pFmt->GetAttrSet(), TRUE );
/*
????? JP 31.01.95 ????  wie jetzt ???
            rSet.MergeValues( aTmpSet );

            // jetzt alle zusammen "mergen"
            rSet.Differentiate( aTmpSet );
*/
            rSet.Put( aTmpSet );
#endif
        }
    }

    // aufnehmen als MergeWert (falls noch nicht gesetzt neu setzen!)
#if 0
/* wenn mehrere Attribute ueberlappen werden diese gemergt !!
 z.B
            1234567890123456789
              |------------|        Font1
                 |------|           Font2
                    ^  ^
                    |--|        Abfragebereich: -> uneindeutig
*/
    else if( SFX_ITEM_DEFAULT == rSet.GetItemState( rAttr.Which(), FALSE ))
        rSet.Put( rAttr );
    else
        rSet.MergeValue( rAttr );
#else
/* wenn mehrere Attribute ueberlappen gewinnt der letze !!
 z.B
            1234567890123456789
              |------------|        Font1
                 |------|           Font2
                    ^  ^
                    |--|        Abfragebereich: -> Gueltig ist Font2
*/
        rSet.Put( rAttr );
#endif
}

// erfrage die Attribute vom TextNode ueber den Bereich
BOOL SwTxtNode::GetAttr( SfxItemSet& rSet, xub_StrLen nStart, xub_StrLen nEnd,
                            BOOL bOnlyTxtAttr, BOOL bGetFromChrFmt ) const
{
    if( pSwpHints )
    {
        /* stelle erstmal fest, welche Text-Attribut in dem Bereich gueltig
         * sind. Dabei gibt es folgende Faelle:
         *  UnEindeutig wenn: (wenn != Format-Attribut)
         *      - das Attribut liegt vollstaendig im Bereich
         *      - das Attributende liegt im Bereich
         *      - der Attributanfang liegt im Bereich:
         * Eindeutig (im Set mergen):
         *      - das Attrib umfasst den Bereich
         * nichts tun:
         *      das Attribut liegt ausserhalb des Bereiches
         */

        void (*fnMergeAttr)( SfxItemSet&, const SfxPoolItem& )
            = bGetFromChrFmt ? &lcl_MergeAttr_ExpandChrFmt
                             : &lcl_MergeAttr;

        // dann besorge mal die Auto-(Fmt)Attribute
        SfxItemSet aFmtSet( *rSet.GetPool(), rSet.GetRanges() );
        if( !bOnlyTxtAttr )
            SwCntntNode::GetAttr( aFmtSet );

        const USHORT nSize = pSwpHints->Count();
        register USHORT n;
        register xub_StrLen nAttrStart;
        register const xub_StrLen* pAttrEnd;

        if( nStart == nEnd )                // kein Bereich:
        {
            for( n = 0; n < nSize; ++n )        //
            {
                const SwTxtAttr* pHt = (*pSwpHints)[n];
                nAttrStart = *pHt->GetStart();
                if( nAttrStart > nEnd )         // ueber den Bereich hinaus
                    break;

                if( 0 == ( pAttrEnd = pHt->GetEnd() ))      // nie Attribute ohne Ende
                    continue;

                if( ( nAttrStart < nStart &&
                        ( pHt->DontExpand() ? nStart < *pAttrEnd
                                            : nStart <= *pAttrEnd )) ||
                    ( nStart == nAttrStart &&
                        ( nAttrStart == *pAttrEnd || !nStart )))
                    (*fnMergeAttr)( rSet, pHt->GetAttr() );
            }
        }
        else                            // es ist ein Bereich definiert
        {
            SwTxtAttr** aAttrArr = 0;
            const USHORT coArrSz = RES_TXTATR_WITHEND_END - RES_CHRATR_BEGIN +
                                    ( RES_UNKNOWNATR_END -
                                                    RES_UNKNOWNATR_BEGIN );

            for( n = 0; n < nSize; ++n )
            {
                const SwTxtAttr* pHt = (*pSwpHints)[n];
                nAttrStart = *pHt->GetStart();
                if( nAttrStart > nEnd )         // ueber den Bereich hinaus
                    break;

                if( 0 == ( pAttrEnd = pHt->GetEnd() ))      // nie Attribute ohne Ende
                    continue;

                BOOL bChkInvalid = FALSE;
                if( nAttrStart <= nStart )      // vor oder genau Start
                {
                    if( *pAttrEnd <= nStart )   // liegt davor
                        continue;

                    if( nEnd <= *pAttrEnd )     // hinter oder genau Ende
                        (*fnMergeAttr)( aFmtSet, pHt->GetAttr() );
                    else
//                  else if( pHt->GetAttr() != aFmtSet.Get( pHt->Which() ) )
                        // uneindeutig
                        bChkInvalid = TRUE;
                }
                else if( nAttrStart < nEnd      // reicht in den Bereich
)//                      && pHt->GetAttr() != aFmtSet.Get( pHt->Which() ) )
                    bChkInvalid = TRUE;

                if( bChkInvalid )
                {
                    // uneindeutig ?
                    if( !aAttrArr )
                    {
                        aAttrArr = new SwTxtAttr* [ coArrSz ];
                        memset( aAttrArr, 0, sizeof( SwTxtAttr* ) * coArrSz );
                    }

                    const SwTxtAttr** ppPrev;
                    if( RES_CHRATR_BEGIN <= pHt->Which() &&
                        pHt->Which() < RES_TXTATR_WITHEND_END )
                         ppPrev = (const SwTxtAttr**)&aAttrArr[
                                        pHt->Which() - RES_CHRATR_BEGIN ];
                    else if( RES_UNKNOWNATR_BEGIN <= pHt->Which() &&
                        pHt->Which() < RES_UNKNOWNATR_END )
                         ppPrev = (const SwTxtAttr**)&aAttrArr[
                                        pHt->Which() - RES_UNKNOWNATR_BEGIN
                                        + ( RES_TXTATR_WITHEND_END -
                                            RES_CHRATR_BEGIN ) ];
                    else
                        ppPrev = 0;

                    if( !*ppPrev )
                    {
                        if( nAttrStart > nStart )
                        {
                            rSet.InvalidateItem( pHt->Which() );
                            *ppPrev = (SwTxtAttr*)-1;
                        }
                        else
                            *ppPrev = pHt;
                    }
                    else if( (SwTxtAttr*)-1 != *ppPrev )
                    {
                        if( *(*ppPrev)->GetEnd() == nAttrStart &&
                            (*ppPrev)->GetAttr() == pHt->GetAttr() )
                            *ppPrev = pHt;
                        else
                        {
                            rSet.InvalidateItem( pHt->Which() );
                            *ppPrev = (SwTxtAttr*)-1;
                        }
                    }
                }
            }
            if( aAttrArr )
            {
                const SwTxtAttr* pAttr;
                for( n = 0; n < coArrSz; ++n )
                    if( 0 != ( pAttr = aAttrArr[ n ] ) &&
                        (SwTxtAttr*)-1 != pAttr )
                    {
                        USHORT nWh;
                        if( n < (RES_TXTATR_WITHEND_END -
                                            RES_CHRATR_BEGIN ))
                            nWh = n + RES_CHRATR_BEGIN;
                        else
                            nWh = n - ( RES_TXTATR_WITHEND_END -
                                                  RES_CHRATR_BEGIN ) +
                                                RES_UNKNOWNATR_BEGIN;

                        if( nEnd <= *pAttr->GetEnd() )  // hinter oder genau Ende
                        {
                            if( pAttr->GetAttr() != aFmtSet.Get( nWh ) )
                                (*fnMergeAttr)( rSet, pAttr->GetAttr() );
                        }
                        else
                            // uneindeutig
                            rSet.InvalidateItem( nWh );
                    }
                __DELETE( coArrSz ) aAttrArr;
            }
        }
        if( aFmtSet.Count() )
        {
            // aus dem Format-Set alle entfernen, die im TextSet auch gesetzt sind
            aFmtSet.Differentiate( rSet );
            // jetzt alle zusammen "mergen"
            rSet.Put( aFmtSet );
        }
    }
    else if( !bOnlyTxtAttr )
        // dann besorge mal die Auto-(Fmt)Attribute
        SwCntntNode::GetAttr( rSet );

    return rSet.Count() ? TRUE : FALSE;
}

int lcl_IsNewAttrInSet( const SwpHints& rHints, const SfxPoolItem& rItem,
                        const xub_StrLen nEnd )
{
    int bIns = TRUE;
    for( USHORT i = 0; i < rHints.Count(); ++i )
    {
        const SwTxtAttr *pOther = rHints[ i ];
        if( *pOther->GetStart() )
            break;

        if( pOther->GetEnd() &&
            *pOther->GetEnd() == nEnd &&
            ( pOther->IsCharFmtAttr() || pOther->Which() == rItem.Which() ) )
        {
            bIns = FALSE;
            break;
        }
    }
    return bIns;
}

void SwTxtNode::FmtToTxtAttr( SwTxtNode* pNd )
{
    SfxItemSet aThisSet( GetDoc()->GetAttrPool(), aCharFmtSetRange );
    if( GetpSwAttrSet() && GetpSwAttrSet()->Count() )
        aThisSet.Put( *GetpSwAttrSet() );

    if ( !pSwpHints )
        pSwpHints = new SwpHints();

    if( pNd == this )
    {
        if( aThisSet.Count() )
        {
            SfxItemIter aIter( aThisSet );
            const SfxPoolItem* pItem = aIter.GetCurItem();
            while( TRUE )
            {
                if( lcl_IsNewAttrInSet( *pSwpHints, *pItem, GetTxt().Len() ) )
                {
                    pSwpHints->SwpHintsArr::Insert(
                            MakeTxtAttr( *pItem, 0, GetTxt().Len() ) );
                    GetpSwAttrSet()->ClearItem( pItem->Which() );
                }

                if( aIter.IsAtEnd() )
                    break;
                pItem = aIter.NextItem();
            }
        }

    }
    else
    {
        SfxItemSet aNdSet( pNd->GetDoc()->GetAttrPool(), aCharFmtSetRange );
        if( pNd->GetpSwAttrSet() && pNd->GetpSwAttrSet()->Count() )
            aNdSet.Put( *pNd->GetpSwAttrSet() );

        if ( !pNd->pSwpHints )
            pNd->pSwpHints = new SwpHints();

        if( aThisSet.Count() )
        {
            SfxItemIter aIter( aThisSet );
            const SfxPoolItem* pItem = aIter.GetCurItem(), *pNdItem;
            while( TRUE )
            {
                if( ( SFX_ITEM_SET != aNdSet.GetItemState( pItem->Which(), FALSE,
                    &pNdItem ) || *pItem != *pNdItem ) &&
                    lcl_IsNewAttrInSet( *pSwpHints, *pItem, GetTxt().Len() ) )
                {
                    pSwpHints->SwpHintsArr::Insert(
                            MakeTxtAttr( *pItem, 0, GetTxt().Len() ) );
                    GetpSwAttrSet()->ClearItem( pItem->Which() );
                }
                aNdSet.ClearItem( pItem->Which() );

                if( aIter.IsAtEnd() )
                    break;
                pItem = aIter.NextItem();
            }
        }

        if( aNdSet.Count() )
        {
            SfxItemIter aIter( aNdSet );
            const SfxPoolItem* pItem = aIter.GetCurItem();
            while( TRUE )
            {
                if( lcl_IsNewAttrInSet( *pNd->pSwpHints, *pItem, pNd->GetTxt().Len() ) )
                    pNd->pSwpHints->SwpHintsArr::Insert(
                            pNd->MakeTxtAttr( *pItem, 0, pNd->GetTxt().Len() ) );
                pNd->GetpSwAttrSet()->ClearItem( pItem->Which() );

                if( aIter.IsAtEnd() )
                    break;
                pItem = aIter.NextItem();
            }

            SwFmtChg aTmp1( pNd->GetFmtColl() );
            pNd->SwModify::Modify( &aTmp1, &aTmp1 );
        }
    }

    if( pNd->pSwpHints->CanBeDeleted() )
        DELETEZ( pNd->pSwpHints );
}

/*************************************************************************
 *                      SwpHints::CalcFlags()
 *************************************************************************/

void SwpHints::CalcFlags()
{
    bDDEFlds = bFtn = FALSE;
    const USHORT nSize = Count();
    const SwTxtAttr* pAttr;
    for( USHORT nPos = 0; nPos < nSize; ++nPos )
        switch( ( pAttr = (*this)[ nPos ])->Which() )
        {
        case RES_TXTATR_FTN:
            bFtn = TRUE;
            if( bDDEFlds )
                return;
            break;
        case RES_TXTATR_FIELD:
            {
                const SwField* pFld = pAttr->GetFld().GetFld();
                if( RES_DDEFLD == pFld->GetTyp()->Which() )
                {
                    bDDEFlds = TRUE;
                    if( bFtn )
                        return;
                }
            }
            break;
        }
}

/*************************************************************************
 *                      SwpHints::CalcVisibleFlag()
 *************************************************************************/

BOOL SwpHints::CalcVisibleFlag()
{
    BOOL bOldVis = bVis;
    bCalcVis = FALSE;
    BOOL            bNewVis  = TRUE;
    const USHORT    nSize = Count();
    const SwTxtAttr *pTxtHt;

    for( USHORT nPos = 0; nPos < nSize; ++nPos )
    {
        pTxtHt = (*this)[ nPos ];
        const USHORT nWhich = pTxtHt->Which();

        if( RES_TXTATR_FIELD == nWhich )
        {
            const SwFmtFld& rFld = pTxtHt->GetFld();
            if( RES_HIDDENPARAFLD == rFld.GetFld()->GetTyp()->Which())
            {
                if( !((SwHiddenParaField*)rFld.GetFld())->IsHidden() )
                {
                    SetVisible(TRUE);
                    return !bOldVis;
                }
                else
                    bNewVis = FALSE;
            }
        }
    }
    SetVisible( bNewVis );
    return bOldVis != bNewVis;
}


/*************************************************************************
 *                      SwpHints::Resort()
 *************************************************************************/

// Ein Hint hat seinen Anfangswert geaendert.
// Hat sich dadurch die Sortierreihenfolge
// geaendert, so wird solange umsortiert, bis
// sie wieder stimmt.

BOOL SwpHints::Resort( const USHORT nPos )
{
    const SwTxtAttr *pTmp;

    if ( ( nPos+1 < Count() &&
           *(*this)[nPos]->GetStart() > *(*this)[nPos+1]->GetStart() ) ||
         ( nPos > 0 &&
           *(*this)[nPos]->GetStart() < *(*this)[nPos-1]->GetStart() ) )
    {
        pTmp = (*this)[nPos];
        DeleteAtPos( nPos );
        SwpHintsArr::Insert( pTmp );
        // Wenn tatsaechlich umsortiert wurde, muss die
        // Position i nochmal bearbeitet werden.
        return TRUE;
    }
    return FALSE;
}


/*************************************************************************
 *                      SwpHints::NoteInHistory()
 *************************************************************************/

void SwpHints::NoteInHistory( SwTxtAttr *pAttr, const BOOL bNew )
{
    if ( pHistory )
        pHistory->Add( pAttr, bNew );
}

/*************************************************************************
 *                      SwpHints::ClearDummies()
 *************************************************************************/

/*
 * ClearDummies: Hints, die genau den gleichen Bereich umfassen wie
 * ein nachfolgender mit gleichem Attribut oder eine nachfolgende Zeichen-
 * vorlage, muessen entfernt werden.
 * Solche Hints entstehen, wenn sie urspruenglich einen groesseren, den
 * Nachfolger umfassenden Bereich hatten, die Aussenanteile aber durch
 * SwTxtNode::RstAttr oder SwTxtNode::Update geloescht wurden.
 */

void SwpHints::ClearDummies( SwTxtNode &rNode )
{
    USHORT i = 0;
    while ( i < Count() )
    {
        SwTxtAttr *pHt = GetHt( i++ );
        const USHORT nWhich = pHt->Which();
        const xub_StrLen *pEnd = pHt->GetEnd();
        if ( pEnd && !pHt->IsOverlapAllowedAttr() && !pHt->IsCharFmtAttr() )
            for( USHORT j = i; j < Count(); ++j  )
            {
                SwTxtAttr *pOther = GetHt(j);
                if ( *pOther->GetStart() > *pHt->GetStart() )
                    break;

                if( pOther->Which() == nWhich || pOther->IsCharFmtAttr() )
                {
                    //JP 03.10.95: nicht zusammenfassen, zu kompliziert
                    //          fuer WIN95-Compiler!!
                    if( *pEnd == *pOther->GetEnd() &&
                        ( pOther->Which() == nWhich ||
                          lcl_Included( nWhich, pOther ) ) )
                    {
                        // Vorsicht: Geloeschte Dummies wuerden bei einer
                        // Undo-Aktion die Macht an sich reissen ...
                        // if( pHistory ) pHistory->Add( pHt );
                        rNode.DestroyAttr( Cut( --i ) );
                        break;
                    }
                }
            }
    }
}

/*************************************************************************
 *                      SwpHints::Merge( )
 *************************************************************************/

/*
 * Merge: Gleichartigen, gleichwertige Hints, die aneinandergrenzen,
 * koennen verschmolzen werden, wenn an ihrer gemeinsamen Kante nicht ein
 * gleichartiges Attribut endet oder beginnt, welches einen von der beiden
 * Kandidaten umfasst, auch Zeichenvorlage duerfen nicht ueberlappt werden.
 */

BOOL SwpHints::Merge( SwTxtNode &rNode )
{
    USHORT i = 0;
    BOOL bMerged = FALSE;
    while ( i < Count() )
    {
        SwTxtAttr *pHt = GetHt( i++ );
        const xub_StrLen *pEnd = pHt->GetEnd();
        if ( pEnd && !pHt->IsDontMergeAttr() )
        {
            const USHORT nWhich = pHt->Which();
            for ( USHORT j = i; j < Count(); j++  )
            {
                SwTxtAttr *pOther = GetHt(j);
                if ( *pOther->GetStart() > *pEnd )
                    break;   // keine beruehrenden Attribute mehr vorhanden

                if( *pOther->GetStart() == *pEnd &&
                     ( pOther->Which() == nWhich ||
                       pOther->IsCharFmtAttr() ||
                       ( pHt->IsCharFmtAttr() && !pHt->IsDontMergeAttr() )))
                {
                    // Beruehrendes Attribut gefunden mit gleichem Typ bzw.
                    // Zeichenvorlage.
                    // Bei Attribut mit anderem Wert bzw. Zeichenvorlage
                    // ist keine Verschmelzung mehr moeglich
                    if( pOther->Which() == nWhich &&
                         pOther->GetAttr() == pHt->GetAttr() )
                    {
                        // Unser Partner pOther erfuellt alle Voraussetzungen,
                        // jetzt muessen wir uns selbst noch ueberpruefen, ob wir
                        // nicht von einem gleichartigen Attribut oder einer
                        // Zeichenvorlage umfasst werden, die das gleiche Ende
                        // wie wir selbst haben.
                        BOOL bMerge = TRUE;
                        for ( USHORT k = 0; k+1 < i; k++ )
                        {
                            SwTxtAttr *pAnOther = GetHt(k);
                            if( ( pAnOther->Which() == nWhich ||
                                  pAnOther->IsCharFmtAttr() ||
                                  pHt->IsCharFmtAttr() )
                                 && pAnOther->GetEnd()
                                 && *pAnOther->GetEnd() == *pEnd )
                            {
                                bMerge = FALSE; // kein Verschmelzen von i+j
                                break;
                            }
                        }
                        if ( bMerge )
                        {
                            // Auch Verschmelzen muss der History mitgeteilt
                            // werden, da sonst ein Delete sein Attribut nicht
                            // mehr findet, wenn es verschmolzen wurde!
                            if( pHistory )
                            {
                                pHistory->Add( pHt );
                                pHistory->Add( pOther );
                            }
                            *pHt->GetEnd() = *pOther->GetEnd();
                            pHt->SetDontExpand( FALSE );
                            if( pHistory ) pHistory->Add( pHt, TRUE );
                            rNode.DestroyAttr( Cut( j ) );
                            --i;
                            bMerged = TRUE;
                        }
                    }
                    break;
                }
            }
        }
    }
    if ( bMerged )
        ClearDummies( rNode );
    return bMerged;
}

/*************************************************************************
 *                      SwpHints::Forget( ... )
 *************************************************************************/

/*
 * Forget: Hints, die genau den gleichen Bereich umfassen wie
 * ein nachfolgender mit gleichem Attribut oder eine nachfolgende Zeichen-
 * vorlage, duerfen nicht eingefuegt werden.
 * Solche Hints koennen entstehen, wenn durch SwTxtNode::RstAttr
 * ein Attribut in zwei Teile zerlegt wird und der zweite Teil einen
 * identischen Bereich mit einem inneren Attribut bekaeme.
 */

BOOL SwpHints::Forget( const USHORT i, const USHORT nWhich,
                 const xub_StrLen nStrt, const xub_StrLen nEnd )
{
    BOOL bRet = FALSE;
    for ( USHORT j = i+1; j < Count(); j++ )
    {
        SwTxtAttr *pHt = GetHt( j );
        if ( *pHt->GetStart() > nStrt )
            break;
        const USHORT nWhch = pHt->Which();
        const xub_StrLen *pEnd = pHt->GetEnd();
        if ( pEnd && *pEnd == nEnd &&
             ( nWhch == nWhich ||
             ( pHt->IsCharFmtAttr() && lcl_Included( nWhich, pHt ) ) ) )
        {
            bRet = TRUE;
            break;
        }
    }
    return bRet;
}

/*************************************************************************
 *                      SwpHints::Insert()
 *************************************************************************/

/*
 * Insert: Der neue Hint wird immer eingefuegt. Wenn dabei ein
 * ueberlappender oder gleicher Hintbereich mit gleichem Attribut
 * und Wert gefunden, wird der neue Hint entsprechend veraendert
 * und der alte herausgenommen (und zerstoert:
 * SwpHints::Destroy()).
 */

void SwpHints::Insert( SwTxtAttr *pHint, SwTxtNode &rNode, USHORT nMode )
{
    // Irgendwann ist immer Schluss
    if( USHRT_MAX == Count() )
        return;

    // Felder bilden eine Ausnahme:
    // 1) Sie koennen nie ueberlappen
    // 2) Wenn zwei Felder genau aneinander liegen,
    //    sollen sie nicht zu einem verschmolzen werden.
    // Wir koennen also auf die while-Schleife verzichten

    xub_StrLen *pHtEnd = pHint->GetEnd();
    USHORT nWhich = pHint->Which();

    switch( nWhich )
    {
    case RES_TXTATR_CHARFMT:
        ((SwTxtCharFmt*)pHint)->ChgTxtNode( &rNode );
        break;
    case RES_TXTATR_INETFMT:
        {
            ((SwTxtINetFmt*)pHint)->ChgTxtNode( &rNode );
            SwCharFmt* pFmt = rNode.GetDoc()->GetCharFmtFromPool( RES_POOLCHR_INET_NORMAL );
            pFmt->Add( (SwTxtINetFmt*)pHint );
        }
        break;
    case RES_TXTATR_FIELD:
        {
            BOOL bDelFirst = 0 != ((SwTxtFld*)pHint)->GetpTxtNode();
            ((SwTxtFld*)pHint)->ChgTxtNode( &rNode );
            SwDoc* pDoc = rNode.GetDoc();
            const SwField* pFld = ((SwTxtFld*)pHint)->GetFld().GetFld();

            if( !pDoc->IsNewFldLst() )
            {
                // was fuer ein Feld ist es denn ??
                // bestimmte Felder mussen am Doc das Calculations-Flag updaten
                switch( pFld->GetTyp()->Which() )
                {
                case RES_DBFLD:
                case RES_SETEXPFLD:
                case RES_HIDDENPARAFLD:
                case RES_HIDDENTXTFLD:
                case RES_DBNUMSETFLD:
                case RES_DBNEXTSETFLD:
                    {
                        if( bDelFirst )
                            pDoc->InsDelFldInFldLst( FALSE, *(SwTxtFld*)pHint );
                        if( rNode.GetNodes().IsDocNodes() )
                            pDoc->InsDelFldInFldLst( TRUE, *(SwTxtFld*)pHint );
                    }
                    break;
                case RES_DDEFLD:
                    if( rNode.GetNodes().IsDocNodes() )
                        ((SwDDEFieldType*)pFld->GetTyp())->IncRefCnt();
                    break;
                }
            }

            // gehts ins normale Nodes-Array?
            if( rNode.GetNodes().IsDocNodes() )
            {
                BOOL bInsFldType = FALSE;
                switch( pFld->GetTyp()->Which() )
                {
                case RES_SETEXPFLD:
                    bInsFldType = ((SwSetExpFieldType*)pFld->GetTyp())->IsDeleted();
                    if( GSE_SEQ & ((SwSetExpFieldType*)pFld->GetTyp())->GetType() )
                    {
                        // bevor die ReferenzNummer gesetzt wird, sollte
                        // das Feld am richtigen FeldTypen haengen!
                        SwSetExpFieldType* pFldType = (SwSetExpFieldType*)
                                    pDoc->InsertFldType( *pFld->GetTyp() );
                        if( pFldType != pFld->GetTyp() )
                        {
                            SwFmtFld* pFmtFld = (SwFmtFld*)&((SwTxtFld*)pHint)
                                                                ->GetFld();
                            pFldType->Add( pFmtFld );          // ummelden
                            pFmtFld->GetFld()->ChgTyp( pFldType );
                        }
                        pFldType->SetSeqRefNo( *(SwSetExpField*)pFld );
                    }
                    break;
                case RES_USERFLD:
                    bInsFldType = ((SwUserFieldType*)pFld->GetTyp())->IsDeleted();
                    break;

                case RES_DDEFLD:
                    if( pDoc->IsNewFldLst() )
                        ((SwDDEFieldType*)pFld->GetTyp())->IncRefCnt();
                    bInsFldType = ((SwDDEFieldType*)pFld->GetTyp())->IsDeleted();
                    break;
                }
                if( bInsFldType )
                    pDoc->InsDeletedFldType( *pFld->GetTyp() );
            }
        }
        break;
    case RES_TXTATR_FTN :
        ((SwTxtFtn*)pHint)->ChgTxtNode( &rNode );
        break;
    case RES_TXTATR_REFMARK:
        ((SwTxtRefMark*)pHint)->ChgTxtNode( &rNode );
        if( rNode.GetNodes().IsDocNodes() )
        {
            //search for a refernce with the same name
            SwTxtAttr* pHt;
            xub_StrLen *pHtEnd, *pHintEnd;
            for( USHORT n = 0, nEnd = Count(); n < nEnd; ++n )
                if( RES_TXTATR_REFMARK == (pHt = GetHt( n ))->Which() &&
                    pHint->GetAttr() == pHt->GetAttr() &&
                    0 != ( pHtEnd = pHt->GetEnd() ) &&
                    0 != ( pHintEnd = pHint->GetEnd() ) )
                {
                    SwComparePosition eCmp = ::ComparePosition(
                            *pHt->GetStart(), *pHtEnd,
                            *pHint->GetStart(), *pHintEnd );
                    BOOL bDelOld = TRUE, bChgStart = FALSE, bChgEnd = FALSE;
                    switch( eCmp )
                    {
                    case POS_BEFORE:
                    case POS_BEHIND:    bDelOld = FALSE; break;

                    case POS_OUTSIDE:   bChgStart = bChgEnd = TRUE; break;

                    case POS_COLLIDE_END:
                    case POS_OVERLAP_BEFORE:    bChgStart = TRUE; break;
                    case POS_COLLIDE_START:
                    case POS_OVERLAP_BEHIND:    bChgEnd = TRUE; break;
                    }

                    if( bChgStart )
                        *pHint->GetStart() = *pHt->GetStart();
                    if( bChgEnd )
                        *pHintEnd = *pHtEnd;

                    if( bDelOld )
                    {
                        if( pHistory )
                            pHistory->Add( pHt );
                        rNode.DestroyAttr( Cut( n-- ) );
                        --nEnd;
                    }
                }
        }
        break;
    case RES_TXTATR_TOXMARK:
        ((SwTxtTOXMark*)pHint)->ChgTxtNode( &rNode );
        break;

    case RES_TXTATR_CJK_RUBY:
        {
            ((SwTxtRuby*)pHint)->ChgTxtNode( &rNode );
            SwCharFmt* pFmt = rNode.GetDoc()->GetCharFmtFromPool(
                                                    RES_POOLCHR_RUBYTEXT );
            pFmt->Add( (SwTxtRuby*)pHint );
        }
        break;
    }

    if( SETATTR_DONTEXPAND & nMode )
        pHint->SetDontExpand( TRUE );

    // SwTxtAttrs ohne Ende werden sonderbehandelt:
    // Sie werden natuerlich in das Array insertet, aber sie werden nicht
    // in die pPrev/Next/On/Off-Verkettung aufgenommen.
    // Der Formatierer erkennt diese TxtHints an dem CH_TXTATR_.. im Text !
    xub_StrLen nHtStart = *pHint->GetStart();
    if( !pHtEnd )
    {
        SwpHintsArr::Insert( pHint );
        CalcFlags();
        CHECK;

        // ... und die Abhaengigen benachrichtigen
        if ( rNode.GetDepends() )
        {
            SwUpdateAttr aHint( nHtStart, nHtStart, nWhich );
            rNode.Modify( 0, &aHint );
        }
        return;
    }

    // ----------------------------------------------------------------
    // Ab hier gibt es nur noch pHint mit einem EndIdx !!!

    if( *pHtEnd < nHtStart )
    {
        ASSERT( *pHtEnd >= nHtStart,
                    "+SwpHints::Insert: invalid hint, end < start" );

        // Wir drehen den Quatsch einfach um:
        *pHint->GetStart() = *pHtEnd;
        *pHtEnd = nHtStart;
        nHtStart = *pHint->GetStart();
    }

    // AMA: Damit wir endlich mit ueberlappenden Hints fertig werden ...
    //      das neue Verfahren:

    if( !(SETATTR_NOHINTADJUST & nMode) && !pHint->IsOverlapAllowedAttr() )
    {
        const SfxPoolItem* pParaItem;
        xub_StrLen nMaxEnd = *pHtEnd;
        xub_StrLen nHtEnd = *pHtEnd;
        BOOL bParaAttr = rNode.GetpSwAttrSet() &&
            ( SFX_ITEM_SET == rNode.GetpSwAttrSet()->GetItemState( nWhich,
                FALSE, &pParaItem ) ) && ( pParaItem == &pHint->GetAttr() );
        BOOL bReplace = !( SETATTR_DONTREPLACE & nMode );
        SwpHtStart_SAR *pTmpHints = 0;

        USHORT i;
        // Wir wollen zwar von nHtStart bis nMaxEnd, muessen aber ggf.
        // stueckeln (Attribute duerfen keine Zeichenvorlagen ueberlappen).
        // Das erste Stueck wird also von nHtStart bis zum ersten Start/Ende
        // einer Zeichenvorlage gehen usw. bis nHtEnd = nMaxEnd erreicht ist.
        do {
            BOOL bINet = nWhich == RES_TXTATR_INETFMT;
            BOOL bForgetAttr = bParaAttr;
            // Muessen wir uns aufspalten?
            if ( !bINet && !pHint->IsDontMergeAttr() )
            {
                // Ab der zweiten Runde muessen wir zunaechst einen neuen
                // Hint erzeugen.
                if ( nHtEnd != nMaxEnd )
                {
                    pHint = rNode.MakeTxtAttr( bParaAttr ? *pParaItem :
                                                            pHint->GetAttr(),
                                                nHtStart, nMaxEnd );
                    nHtEnd = *pHint->GetEnd();
                }

                for ( i = 0; i < Count(); i++)
                {
                    SwTxtAttr *pOther = GetHt(i);
                    // Wir suchen nach Zeichenvorlagen, die uns schneiden
                    // oder in uns liegen
                    BOOL bOtherFmt = pOther->IsCharFmtAttr();
                    if( bOtherFmt ||
                        ( RES_TXTATR_CHARFMT == nWhich && pOther->GetEnd() ) )
                    {
                        if( bForgetAttr && bOtherFmt &&
                            *pOther->GetStart() <= nHtStart &&
                            *pOther->GetEnd() >= nHtStart )
                            bForgetAttr = FALSE;
                        /* Die Flags bCheckInclude und bOtherFmt sollen die
                         * Anzahl der Aufrufe von lcl_Include minimieren, da
                         * dieses wg. IsVisitedURL() teuer ist. */
                        BOOL bCheckInclude = FALSE;
                        if( *pOther->GetStart() > nHtStart
                            && *pOther->GetStart() < nHtEnd
                            && ( bReplace || *pOther->GetEnd() > nHtEnd ) )
                        {
                            if( !bOtherFmt )
                            {
                                bOtherFmt = !lcl_Included( pOther->Which(),
                                                           pHint );
                                bCheckInclude = TRUE;
                            }
                            if( bOtherFmt )
                                nHtEnd = *pOther->GetStart();
                        }
                        if( *pOther->GetEnd() > nHtStart
                            && *pOther->GetEnd() < nHtEnd
                            && ( bReplace || *pOther->GetStart() < nHtStart ) )
                        {
                            if( bOtherFmt || ( !bCheckInclude &&
                                    !lcl_Included( pOther->Which(), pHint ) ) )
                                nHtEnd = *pOther->GetEnd();
                        }
                    }
                }
                *pHint->GetEnd() = nHtEnd;
            }

            i = 0;
            while(i < Count())
            {
                SwTxtAttr *pOther = GetHt(i);
                const USHORT nOtherWhich = pOther->Which();
                BOOL bCheckInclude = pHint->IsCharFmtAttr() &&
                                     pOther->IsCharFmtAttr() &&
                                     nWhich != nOtherWhich;

                BOOL bOtherCharFmt = RES_TXTATR_CHARFMT == nOtherWhich;
                if( nOtherWhich == nWhich || bCheckInclude )
                {
                    if(0 == pOther->GetEnd())
                    {
                        if( *pOther->GetStart() == nHtStart &&
                            nOtherWhich == nWhich &&
                            pOther->GetAttr() == pHint->GetAttr() )
                        {
                            // Gibts schon, alten raus.
                            if( pHistory )
                                pHistory->Add( pOther );
                            rNode.DestroyAttr( Cut(i) );
                        }
                        else
                            ++i;
                        continue;
                    }
                    else
                    {
                        // Attribute mit Anfang und Ende.
                        const Range aHintRg( nHtStart, nHtEnd );
                        const Range aOtherRg( *pOther->GetStart(),
                                                *pOther->GetEnd() );

                        if( aOtherRg.IsInside( aHintRg.Min() ) ||
                            aHintRg.IsInside( aOtherRg.Min() ) )
                        {
                            // aBig umspannt beide Ranges
                            const Range aBig(
                                    Min( aHintRg.Min(), aOtherRg.Min()),
                                    Max( aHintRg.Max(), aOtherRg.Max()));

                            // Gleiches Attribut
                            // oder Zeichenvorlage:
                            //      Bereiche duerfen nicht ueberlappen.

                            // Zuerst wurde geprueft, ob sich der neue mit dem
                            // alten ueberschneidet, danach gibt es nur noch
                            // drei Faelle zu beachten:
                            // 1) der neue umschliesst den alten
                            // 2) der neue ueberlappt den alten hinten
                            // 3) der neue ueberlappt den alten vorne

                            BOOL bNoINet = RES_TXTATR_INETFMT != nOtherWhich ||
                                           nWhich == nOtherWhich;

                            // 1) der neue umschliesst den alten
                            if( aBig == aHintRg )
                            {
                                BOOL bTmpReplace = bReplace ||
                                    ( aHintRg == aOtherRg &&
                                      nWhich == nOtherWhich &&
                                      !pHint->IsCharFmtAttr() );
                                if( bNoINet && bTmpReplace &&
                                    ( !bOtherCharFmt || nWhich == nOtherWhich ) )
                                {
                                    if( !bCheckInclude ||
                                        lcl_Included( nOtherWhich, pHint ) )
                                    {
                                        if( pHistory ) pHistory->Add( pOther );
                                        rNode.DestroyAttr( Cut(i) );
                                    }
                                    else
                                        ++i;
                                    continue;
                                }
                            }
                            else if( !bReplace && aBig == aOtherRg )
                                bForgetAttr = FALSE;
                            // 2) der neue ueberlappt hinten
                            else if( aBig.Max() == aHintRg.Max() )
                            {
                                if( bNoINet &&
                                    ( !bOtherCharFmt || RES_TXTATR_CHARFMT != nWhich ) &&
                                    ( bReplace || aHintRg.Max() != aOtherRg.Max() ) )
                                {
                                    if( ( bCheckInclude &&
                                          lcl_Included( nOtherWhich, pHint ) ) ||
                                          ( !bCheckInclude && !bOtherCharFmt ) )
                                    {
                                        if( nMaxEnd == nHtStart )
                                            bForgetAttr = FALSE;
                                        if( pHistory ) pHistory->Add( pOther );
                                        *pOther->GetEnd() = nHtStart;
                                        if( pHistory ) pHistory->Add( pOther, TRUE );
                                        // ChainDelEnd( pOther );
                                        // ChainEnds( pOther );
                                    }
                                    else if( bINet &&
                                             aHintRg.Max() != aOtherRg.Max() &&
                                             aHintRg.Min() < aOtherRg.Max() )
                                    {
                                    // Wenn ein INetFmt eingefuegt wird, muss
                                    // sich ein anderes Attribut ggf. aufspal-
                                    // ten. Es wird beiseite gestellt und in
                                    // einem spaeteren Durchgang eingefuegt.
                                    // Beim Einfuegen spaltet es sich selbst.
                                        if( pHistory ) pHistory->Add( pOther );
                                        if( !pTmpHints )
                                            pTmpHints = new SwpHtStart_SAR();
                                        pTmpHints->C40_INSERT( SwTxtAttr, pOther,
                                            pTmpHints->Count() );
                                        Cut( i );
                                        continue;
                                    }
                                }
                            }
                            // 3) der neue ueberlappt vorne
                            else if( aBig.Min() == aHintRg.Min() )
                            {
                                if( bNoINet &&
                                    ( RES_TXTATR_CHARFMT != nOtherWhich ||
                                      RES_TXTATR_CHARFMT != nWhich ) &&
                                    ( bReplace || aHintRg.Min() != aOtherRg.Min() ) )
                                {
                                    if( ( bCheckInclude &&
                                          lcl_Included( nOtherWhich, pHint ) ) ||
                                        ( !bCheckInclude && !bOtherCharFmt ) )
                                    {
                                        if( pHistory ) pHistory->Add( pOther );

                                        *pOther->GetStart() = nHtEnd;

                                        if( pHistory ) pHistory->Add( pOther, TRUE );
                                        // ChainDelStart( pOther );
                                        // ChainStarts( pOther );

                                        // nehme die History weg, damit beim Resort
                                        // nicht doppelt eingetragen wird!

                                        SwRegHistory * pSave = pHistory;
                                        pHistory = 0;
                                        const BOOL bOk = Resort(i);
                                        pHistory = pSave;
                                        if( bOk )
                                            continue;
                                    }
                                    else if( bINet &&
                                             aHintRg.Min() != aOtherRg.Min() &&
                                             aHintRg.Max() > aOtherRg.Min() )
                                    {
                                    // Wenn ein INetFmt eingefuegt wird, muss
                                    // sich ein anderes Attribut ggf. aufspal-
                                    // ten. Es wird beiseite gestellt und in
                                    // einem spaeteren Durchgang eingefuegt.
                                    // Beim Einfuegen spaltet es sich selbst.
                                        if( pHistory ) pHistory->Add( pOther );
                                        if( !pTmpHints )
                                            pTmpHints = new SwpHtStart_SAR();
                                        pTmpHints->C40_INSERT( SwTxtAttr, pOther,
                                            pTmpHints->Count() );
                                        Cut( i );
                                        continue;
                                    }
                                }
                            }
                            else
                                bForgetAttr = FALSE;
                        }
                    }
                }
                ++i;
            }

            ClearDummies( rNode );
            // Nur wenn wir nicht sowieso schon durch die Absatzattribute
            // gueltig sind, werden wir eingefuegt ...
            if( bForgetAttr )
                rNode.DestroyAttr( pHint );
            else
            {
                SwpHintsArr::Insert( pHint );
                if ( pHistory )
                    pHistory->Add( pHint, TRUE );
            }
            // InsertChain( pHint );

            // ... und die Abhaengigen benachrichtigen
            if ( rNode.GetDepends() )
            {
                SwUpdateAttr aHint( nHtStart, nHtEnd, nWhich );
                rNode.Modify( 0, &aHint );
            }
            // Falls es noch 'ne Runde gibt:
            nHtStart = nHtEnd;
            if( nMaxEnd > nHtEnd )
                continue;
            if( !pTmpHints )
                break;
            pHint = pTmpHints->GetObject(0);
            nWhich = pHint->Which();
            nHtStart = *pHint->GetStart();
            nHtEnd = *pHint->GetEnd();
            nMaxEnd = nHtEnd;
            bParaAttr = FALSE;
            pTmpHints->Remove(0);
            if( !pTmpHints->Count() )
            {
                delete pTmpHints;
                pTmpHints = NULL;
            }
        } while ( TRUE );

        // Jetzt wollen wir mal gucken, ob wir das SwpHintsArray nicht
        // etwas vereinfachen koennen ...
        Merge( rNode );
    }
    else
    {
        SwpHintsArr::Insert( pHint );
        if ( pHistory )
            pHistory->Add( pHint, TRUE );
        // InsertChain( pHint );

        // ... und die Abhaengigen benachrichtigen
        if ( rNode.GetDepends() )
        {
            SwUpdateAttr aHint( nHtStart,
                nHtStart == *pHtEnd ? *pHtEnd + 1 : *pHtEnd, nWhich );
            rNode.Modify( 0, &aHint );
        }
    }
}

/*************************************************************************
 *                      SwpHints::DeleteAtPos()
 *************************************************************************/

void SwpHints::DeleteAtPos( const USHORT nPos )
{
    SwTxtAttr *pHint = GetHt(nPos);
    // ChainDelete( pHint );
    if( pHistory ) pHistory->Add( pHint );
    SwpHintsArr::DeleteAtPos( nPos );

    if( RES_TXTATR_FIELD == pHint->Which() )
    {
        SwFieldType* pFldTyp = ((SwTxtFld*)pHint)->GetFld().GetFld()->GetTyp();
        if( RES_DDEFLD == pFldTyp->Which() )
        {
            const SwTxtNode* pNd = ((SwTxtFld*)pHint)->GetpTxtNode();
            if( pNd && pNd->GetNodes().IsDocNodes() )
                ((SwDDEFieldType*)pFldTyp)->DecRefCnt();
            ((SwTxtFld*)pHint)->ChgTxtNode( 0 );
        }
        else if( !bVis && RES_HIDDENPARAFLD == pFldTyp->Which() )
            bCalcVis = TRUE;
    }
    CalcFlags();
    CHECK;
}

// Ist der Hint schon bekannt, dann suche die Position und loesche ihn.
// Ist er nicht im Array, so gibt es ein ASSERT !!

void SwpHints::Delete( SwTxtAttr* pTxtHt )
{
    // Attr 2.0: SwpHintsArr::Delete( pTxtHt );
    const USHORT nPos = GetStartOf( pTxtHt );
    ASSERT( USHRT_MAX != nPos, "Attribut nicht im Attribut-Array!" );
    if( USHRT_MAX != nPos )
        DeleteAtPos( nPos );
}

void SwTxtNode::ClearSwpHintsArr( int bDelAll, int bDelFields )
{
    if( pSwpHints )
    {
        USHORT nPos = 0;
        while( nPos < pSwpHints->Count() )
        {
            SwTxtAttr* pDel = pSwpHints->GetHt( nPos );
            BOOL bDel = bDelAll;
            if( !bDelAll )
                switch( pDel->Which() )
                {
                case RES_TXTATR_FLYCNT:
                case RES_TXTATR_FTN:
                    break;

                case RES_TXTATR_FIELD:
                case RES_TXTATR_HARDBLANK:
                    if( bDelFields )
                        bDel = TRUE;
                    break;
                default:
                    bDel = TRUE; break;
                }
            if( bDel )
            {
                pSwpHints->SwpHintsArr::DeleteAtPos( nPos );
                DestroyAttr( pDel );
            }
            else
                ++nPos;
        }
    }
}

FASTBOOL SwTxtNode::IsInSymbolFont( xub_StrLen nPos ) const
{
    SfxItemSet aSet( (SfxItemPool&)GetDoc()->GetAttrPool(),
                    RES_CHRATR_FONT, RES_CHRATR_FONT );
    if( GetAttr( aSet, nPos, nPos ))
        return RTL_TEXTENCODING_SYMBOL == ((SvxFontItem&)aSet.Get( RES_CHRATR_FONT ))
                                    .GetCharSet();
    return FALSE;

//JP 07.10.95: waere so auch nicht schlecht!
//  SwTxtFrmInfo aFInfo( GetFrm() );
//  return aFInfo.IsBullet( nPos );
}


USHORT SwTxtNode::GetLang( const xub_StrLen nBegin, const xub_StrLen nLen) const
{
    USHORT nWhichId = RES_CHRATR_LANGUAGE;
    USHORT nRet = LANGUAGE_DONTKNOW;
    if( pSwpHints )
    {
        if( pBreakIt->xBreak.is() )
            switch( pBreakIt->xBreak->getScriptType( aText, nBegin ) )
            {
            case ScriptType::ASIAN:     nWhichId = RES_CHRATR_CJK_LANGUAGE; break;
            case ScriptType::COMPLEX:   nWhichId = RES_CHRATR_CTL_LANGUAGE; break;
            }

        xub_StrLen nEnd = nBegin + nLen;
        for( USHORT i = 0, nSize = pSwpHints->Count(); i < nSize; ++i )
        {
            // ist der Attribut-Anfang schon groesser als der Idx ?
            const SwTxtAttr *pHt = pSwpHints->operator[](i);
            xub_StrLen nAttrStart = *pHt->GetStart();
            if( nEnd < nAttrStart )
                break;

            const USHORT nWhich = pHt->Which();

            if( ( pHt->IsCharFmtAttr() && lcl_Included( nWhichId, pHt ) )
                || nWhichId == nWhich )
            {
                const xub_StrLen *pEndIdx = pHt->GetEnd();
                // Ueberlappt das Attribut den Bereich?

                if( pEndIdx &&
                    nLen ? ( nAttrStart < nEnd && nBegin < *pEndIdx )
                         : (( nAttrStart < nBegin &&
                                ( pHt->DontExpand() ? nBegin < *pEndIdx
                                                    : nBegin <= *pEndIdx )) ||
                            ( nBegin == nAttrStart &&
                                ( nAttrStart == *pEndIdx || !nBegin ))) )
                {
                    const SfxPoolItem* pItem;
                    if( RES_TXTATR_CHARFMT == nWhich )
                        pItem = &pHt->GetCharFmt().GetCharFmt()->GetAttr( nWhichId );
                    else if( RES_TXTATR_INETFMT == nWhich )
                        pItem = &((SwTxtINetFmt*)pHt)->GetCharFmt()->GetAttr( nWhichId );
                    else
                        pItem = &pHt->GetAttr();

                    USHORT nLng = ((SvxLanguageItem*)pItem)->GetLanguage();

                    // Umfasst das Attribut den Bereich komplett?
                    if( nAttrStart <= nBegin && nEnd <= *pEndIdx )
                        nRet = nLng;
                    else if( LANGUAGE_DONTKNOW == nRet )
                        nRet = nLng; // partielle Ueberlappung, der 1. gewinnt
                }
            }
        }
    }
    if( LANGUAGE_DONTKNOW == nRet )
    {
        if( nBegin )
        {
            nWhichId = RES_CHRATR_LANGUAGE;
            if( pBreakIt->xBreak.is() )
                switch( pBreakIt->xBreak->getScriptType( aText, 0 ) )
                {
                case ScriptType::ASIAN:     nWhichId = RES_CHRATR_CJK_LANGUAGE; break;
                case ScriptType::COMPLEX:   nWhichId = RES_CHRATR_CTL_LANGUAGE; break;
                }
        }

        nRet = ((SvxLanguageItem&)GetSwAttrSet().Get( nWhichId )).GetLanguage();
    }
    return nRet;
}


sal_Unicode GetCharOfTxtAttr( const SwTxtAttr& rAttr )
{
    sal_Unicode cRet = CH_TXTATR_BREAKWORD;
    switch ( rAttr.Which() )
    {
    case RES_TXTATR_REFMARK:
    case RES_TXTATR_TOXMARK:

//  case RES_TXTATR_FIELD:          ??????
//  case RES_TXTATR_FLYCNT,                             // 29

    case RES_TXTATR_FTN:
        cRet = CH_TXTATR_INWORD;
        break;

        // depends on the character ??
//  case RES_TXTATR_HARDBLANK:
//      cRet = CH_TXTATR_INWORD;
//      break;
    }
    return cRet;
}


