/*************************************************************************
 *
 *  $RCSfile: excimp8.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: dr $ $Date: 2000-11-29 09:17:27 $
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
#include "filt_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------------

#define ITEMID_FIELD EE_FEATURE_FIELD

#ifndef SC_ITEMS_HXX
#include <scitems.hxx>
#endif

#include <offmgr/fltrcfg.hxx>
#include <offmgr/app.hxx>

#include <svtools/wmf.hxx>

#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <svx/brshitem.hxx>
#include <svx/editdata.hxx>
#include <svx/editeng.hxx>
#include <svx/editobj.hxx>
#include <svx/editstat.hxx>
#include <svx/svdobj.hxx>
#include <svx/colritem.hxx>
#include <svx/udlnitem.hxx>
#include <svx/wghtitem.hxx>
#include <svx/postitem.hxx>
#include <svx/crsditem.hxx>
#include <svx/flditem.hxx>
#include <svx/xflclit.hxx>
#include <svx/msdffdef.hxx>
#include <svx/svxmsbas.hxx>

#include <svx/svdorect.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdpage.hxx>

#include <vcl/graph.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/exchange.hxx>

#include <tools/string.hxx>
#include <tools/urlobj.hxx>
#include <tools/solmath.hxx>

#ifndef SC_DRWLAYER_HXX
#include <drwlayer.hxx>
#endif

#include "cell.hxx"
#include "document.hxx"
#include "patattr.hxx"
#include "docpool.hxx"
#include "attrib.hxx"
#include "conditio.hxx"
#include "validat.hxx"
#include "dbcolect.hxx"
#include "editutil.hxx"

#include "excimp8.hxx"
#include "xfbuff.hxx"
#include "vfbuff.hxx"
#include "fontbuff.hxx"
#include "excform.hxx"
#include "spstring.hxx"
#include "fltprgrs.hxx"
#include "flttools.hxx"
#include "scextopt.hxx"
#include "stlpool.hxx"


extern const sal_Char* pVBAStorageName;
extern const sal_Char* pVBASubStorageName;

ExcStreamConsumer::ExcStreamConsumer() :
    pStrm               ( NULL ),
    pNode               ( NULL ),
    nBytesLeft          ( 0 )
{
};

ExcStreamConsumer::~ExcStreamConsumer()
{
    delete pStrm;
    while ( pNode )
        RemoveNode();
}

void ExcStreamConsumer::UpdateNode( const DffRecordHeader& rHd )
{
    while ( pNode && ( ( pNode->nPos + pNode->nSize ) <= rHd.nFilePos ) )
        RemoveNode();
    ExcStreamNode* pTemp = pNode;
    pNode = new ExcStreamNode;
    pNode->nPos = rHd.nFilePos;
    pNode->nSize = rHd.nRecLen + 8;
    pNode->pPrev = pTemp;
}

void ExcStreamConsumer::RemoveNode()
{
    ExcStreamNode* pTemp = pNode;
    pNode = pNode->pPrev;
    delete pTemp;
}

const DffRecordHeader* ExcStreamConsumer::Consume( SvStream* pS, sal_uInt32 nLen )
{
    DffRecordHeader* pRetValue = NULL;
    if ( pS )
    {
        sal_uInt32  nEntry = ( pStrm ) ? pStrm->Tell() : 0;
        sal_Bool    bOwnsStream = ( nLen == 0 );

        if ( !nLen )
        {
            pS->Seek( STREAM_SEEK_TO_END );
            nLen = pS->Tell();
            if ( !nLen )
            {
                delete pS;
                return NULL;
            }
            pS->Seek( 0 );
        }
        if ( bOwnsStream && ( pStrm == NULL ) )
        {
            pStrm = pS;
            pStrm->Seek( STREAM_SEEK_TO_END );
        }
        else
        {
            if ( ( bOwnsStream == FALSE ) && ( pStrm == NULL ) )
                pStrm = new SvMemoryStream;
            if ( pStrm )
            {
                sal_Char* pBuf = new sal_Char[ nLen ];
                pS->Read( pBuf, nLen );
                pStrm->Write( pBuf, nLen );
                delete [] pBuf;
                if ( bOwnsStream )
                    delete pS;
            }
            else
                pStrm = pS;
        }
        sal_uInt32 nPos = pStrm->Tell();
        pStrm->Seek( nEntry );
        if ( nBytesLeft )
        {
            if ( nLen < nBytesLeft )
            {
                pStrm->SeekRel( nLen );
                nBytesLeft -= nLen;
            }
            else
            {
                pStrm->SeekRel( nBytesLeft );
                nBytesLeft = 0;
            }
        }
        while ( pStrm->Tell() < nPos )
        {
            *pStrm >> aHd;
            if ( aHd.IsContainer() )
                UpdateNode( aHd );
            else
            {
                if ( ( pStrm->Tell() + aHd.nRecLen ) <= nPos )
                    pStrm->SeekRel( aHd.nRecLen );
                else
                {
                    nBytesLeft = ( pStrm->Tell() + aHd.nRecLen ) - nPos;
                    pStrm->Seek( nPos );
                }
            }
        }
        if ( !nBytesLeft )
            pRetValue = &aHd;
        pStrm->Seek( nPos );
    }
    return pRetValue;
}

sal_Bool ExcStreamConsumer::AppendData( sal_Char* pBuf, sal_uInt32 nLen )
{
    sal_Bool bRetValue = FALSE;
    if ( aHd.nRecType && ( aHd.IsContainer() == FALSE ) && ( nBytesLeft == 0 ) )
    {
        while ( pNode && ( ( pNode->nPos + pNode->nSize ) <= aHd.nFilePos ) )
            RemoveNode();
        ExcStreamNode* pTemp = pNode;
        while ( pTemp )
        {
            pTemp->nSize += nLen;               // updating container sizes
            pStrm->Seek( pTemp->nPos + 4 );
            *pStrm << ( pTemp->nSize - 8 );
            pTemp = pTemp->pPrev;
        }
        aHd.nRecLen += nLen;
        pStrm->Seek( aHd.nFilePos + 4 );        // updating atom size
        *pStrm << aHd.nRecLen;
        pStrm->Seek( STREAM_SEEK_TO_END );
        pStrm->Write( pBuf, nLen );
        return TRUE;
    }
    return bRetValue;
}

struct PosBufferCont
{
    const UINT32        nStart;
    const UINT32        nEnd;
    const UINT32        nObjNum;
    const UINT16        nTabNum;

    ClientAnchorData*   pAnchDat;

    inline              PosBufferCont( const UINT32 nS, const UINT32 nE, const UINT32 nO,
                                        const UINT16 nTab );
    inline              ~PosBufferCont();
    inline BOOL         IsInRange( const UINT32 n ) const;
    inline void         SetAnchor( ClientAnchorData* p );
};


inline PosBufferCont::PosBufferCont( const UINT32 nS, const UINT32 nE, const UINT32 nO,
    const UINT16 nTab ) :
    nStart( nS ), nEnd( nE ), nObjNum( nO ), pAnchDat( NULL ), nTabNum( nTab )
{
}


inline PosBufferCont::~PosBufferCont()
{
    if( pAnchDat )
        delete pAnchDat;
}


inline BOOL PosBufferCont::IsInRange( const UINT32 n ) const
{
    return ( n >= nStart && n <= nEnd );
}


inline void PosBufferCont::SetAnchor( ClientAnchorData* p )
{
    p->nTab = nTabNum;

    if( pAnchDat )
        delete pAnchDat;
    pAnchDat = p;
}



Biff8MSDffManager::Biff8MSDffManager(
                    RootData*           p,
                    PosBuffer&          rPosBuffer,
                    ExcEscherObjList&   rEOL,
                    SvStream&           rStCtrl,
                    long                nOffsDgg,
                    SvStream*           pStData,
                    SdrModel*           pSdrModel_,
                    long                nApplicationScale,
                    ColorData           mnDefaultColor_,
                    ULONG               nDefaultFontHeight_,
                    SvStream*           pStData2_ ) :
    ExcRoot( p ),
    rPosBuff( rPosBuffer ),
    rEscherObjList( rEOL ),
    SvxMSDffManager( rStCtrl, nOffsDgg, pStData, pSdrModel_, nApplicationScale, mnDefaultColor_,
                    nDefaultFontHeight_, pStData2_ )
{
    SetSvxMSDffSettings( SVXMSDFF_SETTINGS_CROP_BITMAPS | SVXMSDFF_SETTINGS_IMPORT_EXCEL );
}


Biff8MSDffManager::~Biff8MSDffManager()
{
}


void Biff8MSDffManager::ProcessClientAnchor2( SvStream& rStr, DffRecordHeader& rH, void*, DffObjData& rD )
{
    rH.SeekToContent( rStr );
    rStr.SeekRel( 2 );
    UINT32                  nFilePos = rStr.Tell();

    ClientAnchorData*   p = new ClientAnchorData;

    if( rPosBuff.SetAnchorData( rStr.Tell(), p ) )
    {
        rStr >> p->nCol >> p->nX >> p->nRow >> p->nY >> p->nDCol >> p->nDX >> p->nDRow >> p->nDY;

        const UINT16        nAnchTab = p->nTab;
        const UINT16        nAnchRow = p->nRow;
        const UINT16        nAnchCol = p->nCol;
        ScDocument*         pDoc = pExcRoot->pDoc;

        Rectangle&          rRect = rD.aChildAnchor;
        rRect.nLeft     = CalcX( nAnchTab, nAnchCol, p->nX, HMM_PER_TWIPS, pDoc );
        rRect.nTop      = CalcY( nAnchTab, nAnchRow, p->nY, HMM_PER_TWIPS, pDoc );
        rRect.nRight    = CalcX( nAnchTab, p->nDCol, p->nDX, HMM_PER_TWIPS, pDoc );
        rRect.nBottom   = CalcY( nAnchTab, p->nDRow, p->nDY, HMM_PER_TWIPS, pDoc );

        rD.bChildAnchor = TRUE;

        UINT32              nObjNum;
        if( rPosBuff.GetObjNum( nFilePos, nObjNum ) )
        {
            ExcEscherObj* pObj = (ExcEscherObj*) rEscherObjList.Get( nObjNum );
            if ( pObj )
            {
                pObj->SetAnchor( rRect );
                if ( pObj->GetObjType() == OT_OLE )
                {
                        ((ExcEscherOle*)pObj)->SetBlipId( GetPropertyValue( DFF_Prop_pib ) );
                }
            }
        }
    }
    else
        delete p;
}


SdrObject* Biff8MSDffManager::ProcessObj(
    SvStream& rSt, DffObjData& rObjData, void* pData, Rectangle& rTextRect, SdrObject* pRet )
{
    if( pRet && ( GetPropertyValue( DFF_Prop_fNoFillHitTest ) & 0x10 ) &&
        ( IsProperty( DFF_Prop_fillColor ) == 0 ) )
    {   // maybe if there is no color, we could do this in ApplyAttributes ( writer ?, calc ? )
        pRet->SetItem(XFillColorItem(XubString(), Color(0xffffff)));

//-/        SfxItemSet* pSet = new SfxItemSet( pSdrModel->GetItemPool() );
//-/        pSet->Put( XFillColorItem( XubString(), Color( 0xffffff ) ) );
//-/        pRet->SetItemSet(*pSet);
//-/
//-/        delete pSet;
    }

    if( maShapeRecords.SeekToContent( rSt, DFF_msofbtClientData, SEEK_FROM_CURRENT_AND_RESTART ) )
    {
        const DffRecordHeader* pHd = maShapeRecords.Current();
        sal_Bool bDummy = FALSE;
    }

    if( maShapeRecords.SeekToContent( rSt, DFF_msofbtClientTextbox, SEEK_FROM_CURRENT_AND_RESTART ) )
    {
        const DffRecordHeader* pHd = maShapeRecords.Current();
        sal_Bool bDummy = FALSE;
    }

    UINT32                      nTextId = GetPropertyValue( DFF_Prop_lTxid, 0 );
    if( nTextId )
    {
        if( rObjData.eShapeType == mso_sptRectangle )
            delete pRet, pRet = NULL;

        SdrObject*              pTObj = NULL;

        // Abstaende an den Raendern der Textbox lesen

        INT32                   nDefault = 92076;
        if( GetPropertyValue( DFF_Prop_FitTextToShape ) & 0x08 )
            nDefault = 20000;   // auto default

        INT32                   nTextLeft = GetPropertyValue( DFF_Prop_dxTextLeft, nDefault );
        INT32                   nTextRight = GetPropertyValue( DFF_Prop_dxTextRight, nDefault );
        INT32                   nTextTop = GetPropertyValue( DFF_Prop_dyTextTop, nDefault / 2 );
        INT32                   nTextBottom = GetPropertyValue( DFF_Prop_dyTextBottom, nDefault / 2 );
        ScaleEmu( nTextLeft );
        ScaleEmu( nTextRight );
        ScaleEmu( nTextTop );
        ScaleEmu( nTextBottom );
        // Die vertikalen Absatzeinrueckungen sind im BoundRect mit drin, hier rausrechnen
        rTextRect.Bottom() -= nTextTop + nTextBottom;

        INT32                   nTextRotationAngle = 0;
        if( IsProperty( DFF_Prop_txflTextFlow ) )
        {
            MSO_TextFlow eTextFlow = (MSO_TextFlow)( GetPropertyValue( DFF_Prop_txflTextFlow ) & 0xFFFF );
            switch( eTextFlow )
            {
                case mso_txflBtoT :                     // Bottom to Top non-@, unten -> oben
                    nTextRotationAngle = 9000;
                break;
                case mso_txflTtoBA :    /* #68110# */   // Top to Bottom @-font, oben -> unten
                case mso_txflTtoBN :                    // Top to Bottom non-@, oben -> unten
                case mso_txflVertN :                    // Vertical, non-@, oben -> unten
                    nTextRotationAngle = 27000;
                break;
                case mso_txflHorzN :                    // Horizontal non-@, normal
                case mso_txflHorzA :                    // Horizontal @-font, normal
                default :
                    nTextRotationAngle = 0;
                break;
            }
            if( nTextRotationAngle )
            {
                if( rObjData.nSpFlags & SP_FFLIPV )
                {
                    if( nTextRotationAngle == 9000 )
                        nTextRotationAngle = 27000;
                    else if( nTextRotationAngle == 27000 )
                        nTextRotationAngle = 9000;
                }
                Point nCenter( rTextRect.Center() );
                long            nDX = rTextRect.Right() - rTextRect.Left();
                long            nDY = rTextRect.Bottom() - rTextRect.Top();
                rTextRect.Left()       = nCenter.X() - nDY/2;
                rTextRect.Top()        = nCenter.Y() - nDX/2;
                rTextRect.Right()      = rTextRect.Left() + nDY;
                rTextRect.Bottom()     = rTextRect.Top()  + nDX;
            }
        }
        pTObj = new SdrRectObj( OBJ_TEXT, rTextRect );


        if( nTextRotationAngle )
        {
            double              f = nTextRotationAngle * nPi180;
            pTObj->NbcRotate( rTextRect.Center(), nTextRotationAngle, sin( f ), cos( f ) );
        }

        SfxItemSet              aSet( pSdrModel->GetItemPool() );
        if( !pRet )
        {
            if( ( GetPropertyValue( DFF_Prop_fNoFillHitTest ) & 0x10 ) &&
                ( IsProperty( DFF_Prop_fillColor ) == 0 ) )
            {   // maybe if there is no color, we could do this in ApplyAttributes ( writer ?, calc ? )
                pTObj->SetItem(XFillColorItem(XubString(), Color(0xffffff)));

//-/                SfxItemSet* pSet = new SfxItemSet( pSdrModel->GetItemPool() );
//-/                pSet->Put( XFillColorItem( XubString(), Color( 0xffffff ) ) );
//-/                pTObj->NbcSetAttributes( *pSet, FALSE );
//-/
//-/                delete pSet;
            }

            ((SvxMSDffManager*)this)->ApplyAttributes( rSt, aSet, pTObj );
        }
        switch( (MSO_WrapMode)GetPropertyValue( DFF_Prop_WrapText, mso_wrapSquare ) )
        {
            case mso_wrapNone :
            {
                if( GetPropertyValue( DFF_Prop_FitTextToShape ) & 2 )   // be sure this is FitShapeToText
                    aSet.Put( SdrTextAutoGrowWidthItem( TRUE ) );
            }
                break;

            case mso_wrapByPoints :
                aSet.Put( SdrTextContourFrameItem( TRUE ) );
                break;
        }

        // Abstaende an den Raendern der Textbox setzen
        aSet.Put( SdrTextLeftDistItem( nTextLeft ) );
        aSet.Put( SdrTextRightDistItem( nTextRight ) );
        aSet.Put( SdrTextUpperDistItem( nTextTop ) );
        aSet.Put( SdrTextLowerDistItem( nTextBottom ) );

        // Textverankerung lesen
        if( IsProperty( DFF_Prop_anchorText ) )
        {
            MSO_Anchor          eTextAnchor = (MSO_Anchor)GetPropertyValue( DFF_Prop_anchorText );

            SdrTextVertAdjust   eTVA = SDRTEXTVERTADJUST_CENTER;
            BOOL                bTVASet = FALSE;
            SdrTextHorzAdjust   eTHA = SDRTEXTHORZADJUST_CENTER;
            BOOL                bTHASet = FALSE;

            switch( eTextAnchor )
            {
                case mso_anchorTop:
                {
                    eTVA = SDRTEXTVERTADJUST_TOP;
                    bTVASet = TRUE;
                }
                    break;
                case mso_anchorTopCentered:
                {
                    eTVA = SDRTEXTVERTADJUST_TOP;
                    bTVASet = TRUE;
                    bTHASet = TRUE;
                }
                    break;

                case mso_anchorMiddle:
                    bTVASet = TRUE;
                    break;
                case mso_anchorMiddleCentered:
                {
                    bTVASet = TRUE;
                    bTHASet = TRUE;
                }
                    break;
                case mso_anchorBottom:
                {
                    eTVA = SDRTEXTVERTADJUST_BOTTOM;
                    bTVASet = TRUE;
                }
                    break;
                case mso_anchorBottomCentered:
                {
                    eTVA = SDRTEXTVERTADJUST_BOTTOM;
                    bTVASet = TRUE;
                    bTHASet = TRUE;
                }
                    break;
            }

            // Einsetzen
            if( bTVASet )
                aSet.Put( SdrTextVertAdjustItem( eTVA ) );
            if( bTHASet )
                aSet.Put( SdrTextHorzAdjustItem( eTHA ) );
        }

        aSet.Put( SdrTextMinFrameHeightItem( rTextRect.Bottom() - rTextRect.Top() ) );
        pTObj->SetModel( pSdrModel );

//-/        pTObj->NbcSetAttributes( aSet, FALSE );
        pTObj->SetItemSet(aSet);


        UINT32                  nObjNum;

        if( rPosBuff.GetObjNum( rObjData.rSpHd.nFilePos, nObjNum ) )
        {
            ExcEscherTxo*       pExcTxoObj = rEscherObjList.GetTxo( nObjNum );
            if( pExcTxoObj )
                pExcTxoObj->Apply( pTObj );
        }

        if( pTObj )
        {   // rotate text with shape ?
            if( mnFix16Angle )
            {
                double          f = mnFix16Angle * nPi180;
                pTObj->NbcRotate( rObjData.rBoundRect.Center(), mnFix16Angle, sin( f ), cos( f ) );
            }

            if( pRet )
            {
                SdrObject*      pGroup = new SdrObjGroup;
                pGroup->GetSubList()->NbcInsertObject( pRet );
                pGroup->GetSubList()->NbcInsertObject( pTObj );
                pRet = pGroup;
            }
            else
                pRet = pTObj;
        }
//      if( !rObjData.bCalledByGroup )
            if( ( rObjData.nCalledByGroup == 0 )
                ||
                ( (rObjData.nSpFlags & SP_FGROUP)
                 && (rObjData.nCalledByGroup < 2) )
              )
            StoreShapeOrder( rObjData.nShapeId, nTextId, pRet );
    }
    return pRet;
}


ULONG Biff8MSDffManager::Calc_nBLIPPos( ULONG nOrgVal, ULONG nStreamPos ) const
{
    return nStreamPos + 4;
}


FASTBOOL Biff8MSDffManager::GetColorFromPalette( USHORT n, Color& r) const
{
    const SvxColorItem* p = pExcRoot->pColor->GetColor( n, FALSE );

    if( p )
    {
        r = p->GetValue();
        return TRUE;
    }
    else
        return FALSE;
}


BOOL Biff8MSDffManager::ShapeHasText( ULONG nShapeId, ULONG nFilePos ) const
{
    UINT32  n;

    if( rPosBuff.GetObjNum( nFilePos, n ) )
    {
        const ExcEscherObj* p = rEscherObjList.Get( n );
        if( p && p->GetObjType() == OT_TXO )
        {
            return ( ( const ExcEscherTxo* ) p )->GetText() != NULL;
        }
    }

    return FALSE;
}


UINT32 Biff8MSDffManager::GetImportOpts( OfaFilterOptions* p )
{
    UINT32      n = 0;
    if( p )
    {
        if( p->IsMathType2StarMath() )
            n |= OLE_MATHTYPE_2_STARMATH;

        if( p->IsWinWord2StarWriter() )
            n |= OLE_WINWORD_2_STARWRITER;

        if( p->IsExcel2StarCalc() )
            n |= OLE_EXCEL_2_STARCALC;

        if( p->IsPowerPoint2StarImpress() )
            n |= OLE_POWERPOINT_2_STARIMPRESS;
    }

    return n;
}




String      ImportExcel8::aSstErrTxt( _STRINGCONST( "*** ERROR IN SST ***" ) );

#define INVALID_POS     0xFFFFFFFF



void TxoCont::ReadTxo( SvStream& r )
{
    if( !nStepCount )
    {
        r.SeekRel( 10 );
        r >> nTextLen >> nFormCnt;

        nStepCount = 1;
    }
}


void TxoCont::ReadCont( SvStream& r, RootData& rRD, ScEditEngineDefaulter& rEdEngine )
{
    if( nStepCount == 1 )
    {// Record mit Text
        if( nTextLen )
        {
            if( !pText )
            {
                INT32   nDummy = 0xFFFF;
                pText = new String( ::ReadUnicodeString( r, nDummy, *rRD.pCharset, nTextLen ) );
            }
        }

        nStepCount = 2;
    }
    else if( nStepCount == 2 /*&& nFormCnt*/ )
    {
//      DBG_ASSERT( nFormCnt >= 2, "TxoCont::ReadCont(): Das war nicht abgemacht!" );

        if( pText )
        {
            SvMemoryStream          aHelpStr;

            if( nFormCnt )
                nFormCnt--;

            UINT32                  n = nFormCnt;
            UINT16                  nChar, nFont;

            while( n )
            {
                r >> nChar >> nFont;
                r.SeekRel( 4 );

                aHelpStr << nChar << nFont;

                n--;
            }

            aHelpStr.Seek( 0 );

            INT32                   nDummy1 = 0xFFFF;
            ShStrTabFormEntry       aHelpObj( *pText, aHelpStr, nDummy1, nFormCnt );

            DBG_ASSERT( !pFormText, "TxoCont::ReadCont(): Ich bin doch nicht alleine!!" );

            pFormText = aHelpObj.CreateEditTextObject( rEdEngine, *rRD.pFontBuffer );
        }

        nStepCount = 3;
    }
    else
    {
        DBG_ERROR( "TxoCont::ReadCont(): Stop Du Verkehrs-Rowdy!" );
    }
}


TxoCont::~TxoCont()
{
    if( pText )
        delete pText;
    if( pFormText )
        delete pFormText;
}


void TxoCont::Clear( void )
{
    if( pText )
    {
        delete pText;
        pText = NULL;
    }
    if( pFormText )
    {
        delete pFormText;
        pFormText = NULL;
    }

    nTextLen = nFormCnt = 0;
    nStepCount = 0;
}




PosBuffer::~PosBuffer()
{
    PosBufferCont*  p = ( PosBufferCont* ) List::First();

    while( p )
    {
        delete p;
        p = ( PosBufferCont* ) List::Next();
    }
}


void PosBuffer::Append( const UINT32 nS, const UINT32 nE, const UINT32 n, const UINT16 nT )
{
    List::Insert( new PosBufferCont( nS, nE, n, nT ), LIST_APPEND );
}


BOOL PosBuffer::GetObjNum( const UINT32 n, UINT32& r )
{
    const PosBufferCont*    p = ( const PosBufferCont* ) List::First();

    while( p )
    {
        if( p->IsInRange( n ) )
        {
            r = p->nObjNum;
            return TRUE;
        }

        p = ( const PosBufferCont* ) List::Next();
    }

    return FALSE;
}


BOOL PosBuffer::SetAnchorData( const UINT32 n, ClientAnchorData* pData )
{
    PosBufferCont*      p = ( PosBufferCont* ) List::First();

    while( p )
    {
        if( p->IsInRange( n ) )
        {
            p->SetAnchor( pData );
            return TRUE;
        }

        p = ( PosBufferCont* ) List::Next();
    }

    return FALSE;
}


const ClientAnchorData* PosBuffer::GetAnchorData( const UINT32 nObjNum ) const
{
    UINT32                  n = 0;
    const PosBufferCont*    p = ( const PosBufferCont* ) List::GetObject( n );

    while( p )
    {
        if( p->nObjNum == nObjNum )
            return p->pAnchDat;

        n++;
        p = ( const PosBufferCont* ) List::GetObject( n );
    }

    return NULL;
}




ExcCondForm::ExcCondForm( RootData* p ) : ExcRoot( p )
{
    nTab = *p->pAktTab;
    nCol = nRow = 0;
    nNumOfConds = nCondCnt = 0;
    pScCondForm = NULL;
    pRangeList = new ScRangeList;
}


ExcCondForm::~ExcCondForm()
{
    delete pRangeList;
}


void ExcCondForm::Read( SvStream& rIn )
{
    rIn >> nNumOfConds;
    rIn.SeekRel( 2 );
    rIn >> nRow;
    rIn.SeekRel( 2 );
    rIn >> nCol;
    rIn.SeekRel( 2 );

    UINT16  nRngCnt, nR1, nR2, nC1, nC2;
    rIn >> nRngCnt;
    while( nRngCnt )
    {
        rIn >> nR1 >> nR2 >> nC1 >> nC2;

        pRangeList->Append( ScRange( nC1, nR1, nTab, nC2, nR2, nTab ) );

        nRngCnt--;
    }
}


void ExcCondForm::ReadCf( SvStream& rIn, const UINT16 nRecLen, ExcelToSc& rConv )
{
    if( nNumOfConds )
    {
        nNumOfConds--;

        const UINT32        nRecPos = rIn.Tell();
        UINT8               nFormType, nFormOperator;
        UINT16              nLenForm1, nLenForm2;
        ULONG               nDummy;

        rIn >> nFormType >> nFormOperator >> nLenForm1 >> nLenForm2;

        ScConditionMode     eMode;
        BOOL                bValid = FALSE;
        BOOL                bSingForm = TRUE;

        if( nFormType == 0x01 )
        {// compare
            bValid = TRUE;

            switch( nFormOperator )
            {
                case 0x01:  eMode = SC_COND_BETWEEN;    bSingForm = FALSE;  break;
                case 0x02:  eMode = SC_COND_NOTBETWEEN; bSingForm = FALSE;  break;
                case 0x03:  eMode = SC_COND_EQUAL;      break;
                case 0x04:  eMode = SC_COND_NOTEQUAL;   break;
                case 0x05:  eMode = SC_COND_GREATER;    break;
                case 0x06:  eMode = SC_COND_LESS;       break;
                case 0x07:  eMode = SC_COND_EQGREATER;  break;
                case 0x08:  eMode = SC_COND_EQLESS;     break;
                default:    eMode = SC_COND_NONE;
            }
        }
        else if( nFormType == 0x02 )
        {
            bValid = TRUE;
            eMode = SC_COND_DIRECT;
        }

        if( bValid )
        {
            UINT16              nFormatsLen = nLenForm1 + nLenForm2 + 6;
            if( nFormatsLen > nRecLen )
                return;

            nFormatsLen = nRecLen - nFormatsLen;    // >0!

            ScDocument*         pDoc = pExcRoot->pDoc;
            String              aStyle( pExcRoot->GetCondFormStyleName( nCondCnt ) );

            const ScTokenArray* pFrmla1 = NULL;
            const ScTokenArray* pFrmla2 = NULL;
            INT32               nAnzBytes;

            ScAddress           aPos( nCol, nRow, nTab );

            if( !pScCondForm )
            {
                nDummy = 0;
                pScCondForm = new ScConditionalFormat( nDummy, pDoc );
            }

            // create style
            UINT32              nPosF = rIn.Tell();
            UINT32              nPosL = nPosF;
            UINT32              nPosP = nPosF;

            switch( nFormatsLen )
            {
                case 10:    nPosF = 0;      nPosL = 0;      nPosP += 7;     break;  // P
                case 14:    nPosF = 0;      nPosL += 6;     nPosP = 0;      break;  // L
                case 18:    nPosF = 0;      nPosL += 6;     nPosP += 15;    break;  // L + P
                case 124:   nPosF += 74;    nPosL = 0;      nPosP = 0;      break;  // F
                case 128:   nPosF += 74;    nPosL = 0;      nPosP += 125;   break;  // F + P
                case 132:   nPosF += 74;    nPosL += 124;   nPosP = 0;      break;  // F + L
                case 136:   nPosF += 74;    nPosL += 124;   nPosP += 133;   break;  // F + L + P
                default:    nPosF = 0;      nPosL = 0;      nPosP = 0;
            }

            SfxItemSet&         rStyleItemSet = pDoc->GetStyleSheetPool()->Make(
                                                    aStyle, SFX_STYLE_FAMILY_PARA,
                                                    SFXSTYLEBIT_USERDEF ).GetItemSet();

            ColorBuffer&        rColBuff = *pExcRoot->pColor;

            if( nPosF )
            {
                UINT8           nAttr1, nAttr2, nAttr3, nUnder;
                UINT16          nBold;
                UINT32          nCol;
                rIn.Seek( nPosF );
                rIn >> nAttr1;          // italic / strike out
                rIn.SeekRel( 3 );
                rIn >> nBold;           // boldness
                rIn.SeekRel( 2 );
                rIn >> nUnder;          // num of underlines
                rIn.SeekRel( 3 );
                rIn >> nCol;            // color
                rIn.SeekRel( 4 );
                rIn >> nAttr2;          // strike out DC + italic/bold DC
                rIn.SeekRel( 7 );
                rIn >> nAttr3;          // underline DC

                BOOL            bItalic = nAttr1 & 0x02;
                BOOL            bStrikeOut = nAttr1 & 0x80;

                BOOL            bHasColor = ( nCol != 0xFFFFFFFF );
                BOOL            bHasBoldItalic = !TRUEBOOL( nAttr2 & 0x02 );
                BOOL            bHasStrikeOut = !TRUEBOOL( nAttr2 & 0x80 );
                BOOL            bHasUnderline = !TRUEBOOL( nAttr3 & 0x01 );

                if( bHasBoldItalic )
                {
                    SvxWeightItem   aWeightItem( FontBuffer::GetWeight( nBold ) );
                    rStyleItemSet.Put( aWeightItem );

                    SvxPostureItem  aAttr( bItalic? ITALIC_NORMAL : ITALIC_NONE );
                    rStyleItemSet.Put( aAttr );
                }

                if( bHasUnderline )
                {
                    FontUnderline   eUnder;
                    switch( nUnder )
                    {
                        case 1:     eUnder = UNDERLINE_SINGLE;      break;
                        case 2:     eUnder = UNDERLINE_DOUBLE;      break;
                        default:    eUnder = UNDERLINE_NONE;
                    }
                    SvxUnderlineItem    aUndItem( eUnder );
                    rStyleItemSet.Put( aUndItem );
                }

                if( bHasStrikeOut )
                {
                    SvxCrossedOutItem   aAttr( bStrikeOut? STRIKEOUT_SINGLE : STRIKEOUT_NONE );
                    rStyleItemSet.Put( aAttr );
                }

                if( bHasColor )
                    rStyleItemSet.Put( *rColBuff.GetColor( nCol ) );
            }

            if( nPosL )
            {
                UINT8           nLineH, nLineV;
                UINT16          nColH, nColV;
                rIn.Seek( nPosL );

                rIn >> nLineV >> nLineH >> nColV >> nColH;

                UINT8           nLineL = nLineV & 0x0F;
                UINT16          nColL = nColV & 0x007F;
                UINT8           nLineR = nLineV >> 4;
                UINT16          nColR = ( nColV >> 7 ) & 0x007F;
                UINT8           nLineT = nLineH & 0x0F;
                UINT16          nColT = nColH & 0x007F;
                UINT8           nLineB = nLineH >> 4;
                UINT16          nColB = ( nColH >> 7 ) & 0x007F;

                XF_Buffer::SetBorder( rStyleItemSet, rColBuff,
                        nLineL, nColL, nLineR, nColR, nLineT, nColT, nLineB, nColB );
            }

            if( nPosP )
            {
                UINT8           nP;
                UINT16          nCol;
                rIn.Seek( nPosP );

                rIn >> nP >> nCol;

                UINT8           nF = nCol & 0x007F;
                UINT8           nB = ( nCol >> 7 ) & 0x007F;

                if( !nP )
                {// no brush set
                    nF = nB;
                    nB = 0xFF;
                    nP = 1;
                }

                XF_Buffer::SetFill( rStyleItemSet, rColBuff, nP, nF, nB );
            }

            // convert formulas
            FORMULA_TYPE        eFT = FT_RangeName;
            if( nLenForm1 )
            {
                rIn.Seek( nRecPos + nRecLen - nLenForm1 - nLenForm2 );

                rConv.Reset( nLenForm1, aPos );
                nAnzBytes = nLenForm1;

                rConv.Convert( pFrmla1, nAnzBytes, eFT );
            }

            ScTokenArray*       pHelp;

            if( nLenForm2 )
            {
                if( pFrmla1 )
                {
                    // copy unique ScTokenArry from formula converter!
                    pHelp = pFrmla1->Clone();
                    pFrmla1 = ( const ScTokenArray* ) pHelp;
                }

                rIn.Seek( nRecPos + nRecLen - nLenForm2 );

                rConv.Reset( nLenForm2, aPos );
                nAnzBytes = nLenForm2;

                rConv.Convert( pFrmla2, nAnzBytes, eFT );
            }

            ScCondFormatEntry   aCFE( eMode, pFrmla1, pFrmla2, pDoc, aPos, aStyle );

            pScCondForm->AddEntry( aCFE );

            if( pFrmla1 && pFrmla2 )
            {
                // if both pointers are non null, 1 is a real copy
                pHelp = ( ScTokenArray* ) pFrmla1;
                delete pHelp;
            }
        }
        nCondCnt++;
    }
}


void ExcCondForm::Apply( void )
{
    if( pScCondForm )
    {
        ULONG           nCondFormat = pExcRoot->pDoc->AddCondFormat( *pScCondForm );
        ScPatternAttr   aPat( pExcRoot->pDoc->GetPool() );
        aPat.GetItemSet().Put( SfxUInt32Item( ATTR_CONDITIONAL, nCondFormat ) );

        const ScRange*  p = pRangeList->First();
        UINT16          nC1, nC2, nR1, nR2;

        while( p )
        {
            nC1 = p->aStart.Col();
            nR1 = p->aStart.Row();
            nC2 = p->aEnd.Col();
            nR2 = p->aEnd.Row();

            if( nC1 > MAXCOL )
                nC1 = MAXCOL;
            if( nC2 > MAXCOL )
                nC2 = MAXCOL;
            if( nR1 > MAXROW )
                nR1 = MAXROW;
            if( nR2 > MAXROW )
                nR2 = MAXROW;

            pExcRoot->pDoc->ApplyPatternAreaTab( nC1, nR1, nC2, nR2, nTab, aPat );

            p = pRangeList->Next();
        }
    }
}




ExcCondFormList::~ExcCondFormList()
{
    ExcCondForm*    p = ( ExcCondForm* ) List::First();

    while( p )
    {
        delete p;
        p = ( ExcCondForm* ) List::Next();
    }
}


void ExcCondFormList::Apply( void )
{
    ExcCondForm*        p = ( ExcCondForm* ) List::First();

    while( p )
    {
        p->Apply();
        p = ( ExcCondForm* ) List::Next();
    }
}



void XclImpTabIdBuffer::Append( UINT16 nTabId )
{
    DBG_ASSERT( nTabId, "XclImpTabIdBuffer::Append - zero value not allowed" );
    if( nTabId )
        UINT16List::Append( nTabId );
}

void XclImpTabIdBuffer::Fill( SvStream& rStrm, UINT16 nCount )
{
    Clear();
    UINT16 nTabId;
    for( UINT16 nIndex = 0; nIndex < nCount; nIndex++ )
    {
        rStrm >> nTabId;
        Append( nTabId );
    }
}

UINT16 XclImpTabIdBuffer::GetIndex( UINT16 nTabId, UINT16 nMaxTabId ) const
{
    UINT16 nReturn = 0;
    for( UINT32 nIndex = 0; nIndex < Count(); nIndex++ )
    {
        UINT16 nValue = Get( nIndex );
        if( nValue == nTabId )
            return nReturn;
        if( nValue <= nMaxTabId )
            nReturn++;
    }
    return 0;
}



ImportExcel8::ImportExcel8( SvStorage* pStorage, SvStream& rStream, ScDocument* pDoc,
                            SvStorage* pPivotCache ) :
    ImportExcel( rStream, pDoc ), aEscherObjList( aPosBuffer, pExcRoot )
{
    delete pFormConv;

    pExcRoot->pXtiBuffer = new XtiBuffer;
    pExcRoot->pSupbookBuffer = new SupbookBuffer;
    pExcRoot->pImpTabIdBuffer = new XclImpTabIdBuffer;

    pFormConv = new ExcelToSc8( pExcRoot, rStream, nTab );

    pActTxo = NULL;

    bLeadingTxo = FALSE;
    bMaybeTxo = FALSE;
    bCond4EscherCont = bLeadingObjRec = FALSE;
    bTabStartDummy = TRUE;

    pActEscherObj = NULL;

    pActChart = NULL;

    pExcRoot->pPivotCacheStorage = pPivotCache;
    pCurrPivTab = NULL;
    pCurrPivotCache = NULL;

    pActCondForm = NULL;
    pCondFormList = NULL;

    pExcRoot->pRootStorage = pStorage;

    bHasBasic = FALSE;
}


ImportExcel8::~ImportExcel8()
{
    if( pActTxo )
        delete pActTxo;

    if( pActEscherObj )
        delete pActEscherObj;

    if( pCondFormList )
        delete pCondFormList;
}


void ImportExcel8::RecString( void )
{
    if( pLastFormCell )
    {
        INT32   nDummy = nBytesLeft;
        pLastFormCell->SetString( ::ReadUnicodeString( aIn, nDummy, eQuellChar ) );

        pLastFormCell = NULL;
    }
}


void ImportExcel8::Protect( void )
{
    if( Read2() )
        pD->SetTabProtection( nTab, TRUE, EMPTY_STRING );
}


void ImportExcel8::Verticalpagebreaks( void )
{
    UINT16      n;
    UINT16      nCol;

    aIn >> n;

    while( n )
    {
        aIn >> nCol;
        aIn.SeekRel( 4 );       // beide Rows ueberlesen

        aColRowBuff.SetVertPagebreak( nCol );

        n--;
    }
}


void ImportExcel8::Horizontalpagebreaks( void )
{
    UINT16      n;
    UINT16      nRow;

    aIn >> n;

    while( n )
    {
        aIn >> nRow;
        aIn.SeekRel( 4 );       // beide Cols ueberlesen

        aColRowBuff.SetHorizPagebreak( nRow );

        n--;
    }
}


void ImportExcel8::Note( void )
{
    UINT16  nCol, nRow, nId;

    aIn >> nRow >> nCol;
    aIn.SeekRel( 2 );
    aIn >> nId;

    if( nRow <= MAXROW && nCol <= MAXCOL )
    {
        if( nId )
        {
            const ExcEscherObj*     pObj = aEscherObjList.Get( nId, nTab );

            if( pObj && pObj->GetObjType() == OT_NOTE )
            {
                const String*   p = ( ( ExcEscherNote* ) pObj )->GetText();

                if( p )
                    pD->SetNote( nCol, nRow, nTab, ScPostIt( *p ) );
            }
        }
    }
    else
        bTabTruncated = TRUE;

    pLastFormCell = NULL;
}


void ImportExcel8::Format( void )
{
    Ignore( 2 );
    String  aFormat( ::ReadUnicodeString( aIn, nBytesLeft, eQuellChar ) );

    pValueFormBuffer->NewValueFormat( aFormat );
}


void ImportExcel8::Externsheet( void )
{
    UINT16  nXtiCnt;

    aIn >> nXtiCnt;
    nBytesLeft -= 2;

    pExcRoot->pXtiBuffer->Read( aIn, nXtiCnt, nBytesLeft );
}


void ImportExcel8::Externname( void )
{
    UINT32          nRes;
    UINT16          nOpt;
    UINT8           nLen;

    aIn >> nOpt >> nRes >> nLen;
    nBytesLeft -= sizeof( nRes ) + sizeof( nOpt ) + sizeof( nLen );

    String      aName;

    if( nLen )
        aName = ::ReadUnicodeString( aIn, nBytesLeft, eQuellChar, nLen );

    if( ( nOpt & 0x0001 ) || ( ( nOpt & 0xFFFE ) == 0x0000 ) )
        // external name
        pExcRoot->pExtNameBuff->AddName( ExcelNameToScName( aName ) );
    else if( nOpt & 0x0010 )
        // ole link
        pExcRoot->pExtNameBuff->AddOLE( aName, nRes );      // nRes is storage ID
    else
        // dde link
        pExcRoot->pExtNameBuff->AddDDE( aName );
}


void ImportExcel8::Font( void )
{
    UINT16  nHeight, nIndexCol, nScript;
    UINT8   nAttr0;
    BYTE    nUnderline, nFamily, nCharSet;
    UINT8   nLen;
    UINT16  nWeight;

    aIn >> nHeight >> nAttr0;
    aIn.SeekRel( 1 );
    aIn >> nIndexCol >> nWeight >> nScript >> nUnderline >> nFamily >> nCharSet;
    aIn.SeekRel( 1 );   // Reserved

    aIn >> nLen;

    String  aName( ::ReadUnicodeString( aIn, nBytesLeft, eQuellChar, nLen ) );

    // Font in Pool batschen
    pExcRoot->pFontBuffer->NewFont(
        nHeight, nAttr0, nScript, nUnderline, nIndexCol, nWeight,
        nFamily, nCharSet, aName );
}


void ImportExcel8::Cont( const UINT16 nLenRecord )
{
    if( pActTxo )
    {
        pActTxo->ReadCont( aIn, *pExcRoot, GetEdEng() );

        if( pActTxo->IsComplete() )
        {
            if( pActEscherObj )
            {
                if( pActEscherObj->GetObjType() == OT_TXO || pActEscherObj->GetObjType() == OT_NOTE )
                {
                    ( ( ExcEscherTxo* ) pActEscherObj )->TakeTxo( *pActTxo );

                    if( bMaybeTxo )
                    {
                        aEscherObjList.MorpheLastObj( pActEscherObj );
                        bMaybeTxo = FALSE;
                    }
                    else
                        aEscherObjList.Append( pActEscherObj );
                }
                else
                    delete pActEscherObj;

                pActEscherObj = NULL;
            }

            delete pActTxo;
            pActTxo = NULL;
        }
    }
    else if( bCond4EscherCont )
        Msodrawing( nLenRecord );
    else if( bLeadingObjRec )
    {
        UINT32      nOldPos = aIn.Tell();
        UINT32      nId;
        aIn >> nId;

        if( ( nId & 0xF000000F ) == 0xF000000F )
        {
            aIn.Seek( nOldPos );
            Msodrawing( nLenRecord );
        }

        bLeadingObjRec = FALSE;
    }
}


void ImportExcel8::Dconref( void )
{
    if( !pCurrPivotCache )
        return;

    UINT16  nR1, nR2;
    UINT8   nC1, nC2;
    String  aFileName, aTabName;
    BOOL    bSelf;

    aIn >> nR1 >> nR2 >> nC1 >> nC2;
    nBytesLeft -= 6;

    EncodeExternSheetUnicode( aIn, aFileName, aTabName, nBytesLeft, bSelf );

    if( !aTabName.Len() )
    {
        aTabName = aFileName;
        aFileName.Erase();
    }
    pCurrPivotCache->SetSource( nC1, nR1, nC2, nR2, aFileName, aTabName, bSelf );
}


void ImportExcel8::Xct( void )
{
    if( pExcRoot->pCurrSupbook )
    {
        UINT16  nCrnCount;
        UINT16  nTabNum = 0;

        aIn >> nCrnCount;
        if( nBytesLeft > 3 )
            aIn >> nTabNum;

        pExcRoot->pCurrSupbook->SetCurrScTab( nTabNum );
    }
}


void ImportExcel8::Crn( void )
{
    if( pExcRoot->pCurrSupbook )
    {
        SupbookE& rSB = *pExcRoot->pCurrSupbook;

        if( rSB.HasValidScTab() )
        {
            UINT8       nLastCol;
            UINT8       nFirstCol;
            UINT16      nRow;
            UINT16      nTab = rSB.GetCurrScTab();
            UINT8       nValType;

            aIn >> nLastCol >> nFirstCol >> nRow;
            nBytesLeft -= 4;

            ScAddress   aAddr( (UINT16) 0, nRow, nTab );

            for( UINT16 iCol = nFirstCol; (iCol <= nLastCol) && (nBytesLeft > 1); iCol++ )
            {
                aAddr.SetCol( iCol );
                aIn >> nValType;
                nBytesLeft--;
                switch( nValType )
                {
                    case EXC_CRN_DOUBLE:
                        if( nBytesLeft >= 8 )
                        {
                            double fVal;
                            aIn >> fVal;
                            nBytesLeft -= 8;
                            pD->SetValue( iCol, nRow, nTab, fVal );
                        }
                        break;
                    case EXC_CRN_STRING:
                        if( nBytesLeft >= 3 )
                        {
                            String sText( ::ReadUnicodeString( aIn, nBytesLeft, eQuellChar ) );
                            ScStringCell* pStrCell = new ScStringCell( sText );
                            pD->PutCell( aAddr, pStrCell );
                        }
                        break;
                    case EXC_CRN_BOOL:
                    case EXC_CRN_ERROR:
                        if( nBytesLeft >= 8 )
                        {
                            BOOL    bIsErr = (nValType == 0x10);
                            UINT16  nErrBool;
                            double  fVal;

                            aIn >> nErrBool;
                            aIn.SeekRel( 6 );
                            nBytesLeft -= 8;

                            const ScTokenArray* pTok    = ErrorToFormula( bIsErr, nErrBool, fVal );
                            ScFormulaCell*      pCell   = new ScFormulaCell( pD, aAddr, pTok );

                            pCell->SetDouble( fVal );
                            pD->PutCell( aAddr, pCell );
                        }
                        break;
                }
            }
        }
    }
}


void ImportExcel8::Obj( const UINT32 nLimitPos )
{
    UINT16          nOpcode, nLenRec;
    UINT32          nNextRec = aIn.Tell();
    BOOL            bLoop = TRUE;
    ExcEscherObj*   pObj = NULL;

    while( bLoop )
    {
        aIn >> nOpcode >> nLenRec;

        nNextRec += 4;
        nNextRec += nLenRec;

        if( aIn.IsEof() || nNextRec > nLimitPos )
            bLoop = FALSE;
        else
        {
            switch( nOpcode )
            {
                case 0x00:
                    bLoop = FALSE;
                    break;
                case 0x15:  pObj = ObjFtCmo();  break;
                case 0x08:  ObjFtPioGrbit( pObj );  break;
                case 0x09:  ObjFtPictFmla( pObj, nLenRec ); break;
            }
        }

        aIn.Seek( nNextRec );
    }

    bLeadingObjRec = TRUE;
}


ExcEscherObj* ImportExcel8::ObjFtCmo( void )
{
    UINT16          nOt, nOid, nGrbit;

    aIn >> nOt >> nOid >> nGrbit;

    if( !pActEscherObj )
        pActEscherObj = new ExcEscherObj( 0, 0, nTab, pExcRoot );

    if ( pActEscherObj )
        pActEscherObj->SetId( nOid );

    bMaybeTxo = FALSE;

    ExcEscherObj* pObj;
    switch( nOt )
    {
        case 0x05:              // Chart
            pActEscherObj = pObj = new ExcEscherChart( pActEscherObj/*, pActChart */);
            break;
        case 0x02:              // rectangel
        case 0x01:              // line
        case 0x03:              // oval
        case 0x04:              // arc
        case 0x09:              // polygon
            aEscherObjList.Append( pObj = new ExcEscherDrwObj( pActEscherObj ) );
            bMaybeTxo = TRUE;
            break;
        case 0x08:              // picture
            aEscherObjList.Append( pObj = new ExcEscherOle( pActEscherObj ) );
            break;
        case 0x06:              // text
            bLeadingTxo = TRUE;
            pActEscherObj = pObj = new ExcEscherTxo( pActEscherObj );
            break;
        case 0x19:              // Note
            bLeadingTxo = TRUE;
            pActEscherObj = pObj = new ExcEscherNote( pActEscherObj );
            break;
        default:
            aEscherObjList.Append( pObj = new ExcEscherDrwObj( pActEscherObj ) );
    }

    if( nOt != 0x05 )
        bCond4EscherCont = FALSE;

    return pObj;
}


void ImportExcel8::ObjFtPioGrbit( ExcEscherObj* pObj )
{
    DBG_ASSERT( pObj && pObj->GetObjType() == OT_OLE, "ImportExcel8::ObjFtPioGrbit: no OLE object" );
    if ( !(pObj && pObj->GetObjType() == OT_OLE) )
        return ;
    ExcEscherOle* pOle = (ExcEscherOle*) pObj;
    UINT16 nBits;
    aIn >> nBits;
    pOle->SetAsSymbol( nBits & 0x0008 );
    pOle->SetLinked( nBits & 0x0002 );
}


void ImportExcel8::ObjFtPictFmla( ExcEscherObj* pObj, UINT16 nLen )
{
    DBG_ASSERT( pObj && pObj->GetObjType() == OT_OLE, "ImportExcel8::ObjFtPictFmla: no OLE object" );
    if ( !(pObj && pObj->GetObjType() == OT_OLE) )
        return ;
    ((ExcEscherOle*)pObj)->ReadPictFmla( aIn, nLen );
}


void ImportExcel8::Boundsheet( void )
{
    UINT8           nLen;
    UINT16          nGrbit;

    aIn.SeekRel( 4 );
    aIn >> nGrbit >> nLen;
    nBytesLeft -= 7;

    String          aName( ::ReadUnicodeString( aIn, nBytesLeft, eQuellChar, nLen ) );

    ExcelNameToScName( aName );
    *pExcRoot->pTabNameBuff << aName;

    if( nBdshtTab > 0 )
    {
        DBG_ASSERT( !pD->HasTable( nBdshtTab ),
            "*ImportExcel::Boundsheet8(): Tabelle schon vorhanden!" );

        pD->MakeTable( nBdshtTab );
    }

    if( ( nGrbit & 0x0001 ) || ( nGrbit & 0x0002 ) )
        pD->SetVisible( nBdshtTab, FALSE );

    pD->RenameTab( nBdshtTab, aName );
    nBdshtTab++;
}


void ImportExcel8::FilterMode( void )
{   }


void ImportExcel8::AutoFilterInfo( void )
{
    AutoFilterData* pData = pAutoFilter->GetByTab( nTab );
    if( pData )
        pData->SetAdvancedRange( NULL );
}


void ImportExcel8::AutoFilter( void )
{
    AutoFilterData* pData = pAutoFilter->GetByTab( nTab );
    if( pData )
        pData->ReadAutoFilter( aIn, nBytesLeft );
}


void ImportExcel8::Scenman( void )
{
    UINT16              nLastDispl;

    aIn.SeekRel( 4 );
    aIn >> nLastDispl;

    aScenList.SetLast( nLastDispl );
}


UINT32 ImportExcel8::Scenario( const UINT16 n )
{
    SvStream*           pStr;

    UINT32              nSumLen;
    UINT32              nNextRecord;
    SvMemoryStream*     pMemStr = CreateContinueStream( n, nSumLen, nNextRecord, FALSE );

    if( pMemStr )
    {
        pStr = pMemStr;
        pStr->Seek( STREAM_SEEK_TO_BEGIN );
    }
    else
        pStr = &aIn;

    aScenList.Append( new ExcScenario( *pStr, *pExcRoot ) );

    if( pMemStr )
        delete pMemStr;

    return nNextRecord;
}


void ImportExcel8::SXView( void )
{
    pCurrPivTab = new XclImpPivotTable( aIn, pExcRoot, (UINT8) nTab, nBytesLeft );
    aPivotTabList.Append( pCurrPivTab );
}


void ImportExcel8::SXVd( void )
{
    if( !pCurrPivTab )
        return;
    pCurrPivTab->AddViewField( aIn, nBytesLeft );
}


void ImportExcel8::SXVi( void )
{
    if( !pCurrPivTab )
        return;

    UINT16  nItemType, nGrBit, nCache;
    aIn >> nItemType >> nGrBit >> nCache;
    pCurrPivTab->AddViewItem( nItemType, nCache, nGrBit );
}


void ImportExcel8::SXIvd( void )
{
    if( !pCurrPivTab )
        return;
    pCurrPivTab->ReadRCFieldIDs( aIn, nBytesLeft );
}


void ImportExcel8::SXLi( void )
{   }   // unnecessary to read this record


void ImportExcel8::SXPi( void )
{
    if( !pCurrPivTab )
        return;

    UINT16  nArrayCnt = nBytesLeft / 6;     // SXPI contains 6-byte-arrays
    UINT16  nSXVD;
    UINT16  nSXVI;
    UINT16  nObjID;

    for( UINT16 iArray = 0; iArray < nArrayCnt; iArray++ )
    {
        aIn >> nSXVD >> nSXVI >> nObjID;
        pCurrPivTab->AddPageItemInfo( nSXVD, nSXVI );
    }
}


void ImportExcel8::SXDi( void )
{
    if( !pCurrPivTab )
        return;
    pCurrPivTab->AddDataItem( aIn, nBytesLeft );
}


void ImportExcel8::SXIdStm( void )
{
    UINT16 nStrId;
    aIn >> nStrId;

    if( !pExcRoot->pImpPivotCacheList )
        pExcRoot->pImpPivotCacheList = new XclImpPivotCacheList;

    pCurrPivotCache = new XclImpPivotCache( pExcRoot, nStrId );
    pExcRoot->pImpPivotCacheList->Append( pCurrPivotCache );
}


void ImportExcel8::Xf( void )
{
#define HASATTRSET(m)       (!(nW12&m))
    static UINT16   nXFCnt = 0;
    UINT16      nW4, nW6, nW8, nW10, nW12, nW14, nW16, nW22;
    UINT32      nL18;
    CellBorder  aBorder;
    CellFill    aFill;
    XF_Data*    pXFD = new XF_Data;

    aIn >> nW4 >> nW6 >> nW8 >> nW10 >> nW12 >> nW14 >> nW16 >> nL18 >> nW22;

    nBytesLeft -= 20;

    const BOOL  bCellXF = ( nW8 & 0x0004 ) == 0;

    pXFD->SetCellXF( bCellXF );

    if( bCellXF || HASATTRSET( 0x0800 ) )
        pXFD->SetFont( nW4 );
    if( bCellXF || HASATTRSET( 0x0400 ) )
        pXFD->SetValueFormat( pValueFormBuffer->GetValueFormat( nW6 ) );
    if( bCellXF || HASATTRSET( 0x8000 ) )
    {
        pXFD->SetLocked( TRUEBOOL( nW8 & 0x0001 ) );
        pXFD->SetHidden( TRUEBOOL( nW8 & 0x0002 ) );
    }
    if( bCellXF )
        pXFD->SetParent( nW8 >> 4 );
    if( bCellXF || HASATTRSET( 0x1000 ) )
    {
        pXFD->SetAlign( ( ExcHorizAlign ) ( nW10 & 0x0007 ) );
        if( nW10 & 0x0008 )
            pXFD->SetWrap( EWT_Wrap );
        pXFD->SetAlign( ( ExcVertAlign ) ( ( nW10 & 0x0070 ) >> 4 ) );
        pXFD->SetTextOrient( ( UINT8 ) ( nW10 >> 8 ) );
    }

    // nW12 >> 5 : 1      fMergeCell

    pXFD->SetIndent( nW12 & 0x000F );

    if( bCellXF || HASATTRSET( 0x2000 ) )
    {
        aBorder.nLeftLine = ( BYTE ) nW14 & 0x000F;
        nW14 >>= 4;
        aBorder.nRightLine = ( BYTE ) nW14 & 0x000F;
        nW14 >>= 4;
        aBorder.nTopLine = ( BYTE ) nW14 & 0x000F;
        nW14 >>= 4;
        aBorder.nBottomLine = ( BYTE ) nW14 & 0x000F;

        aBorder.nLeftColor = nW16 & 0x007F;
        aBorder.nRightColor = ( nW16 >> 7 ) & 0x007F;
        aBorder.nTopColor = ( UINT16 ) ( nL18 & 0x007F );
        aBorder.nBottomColor = ( UINT16 ) ( ( nL18 >> 7 ) & 0x007F );

        pXFD->SetBorder( aBorder );
    }

    if( bCellXF || HASATTRSET( 0x4000 ) )
    {
        aFill.nPattern = ( BYTE ) ( nL18 >> 26 );
        aFill.nForeColor = nW22 & 0x007F;
        aFill.nBackColor = ( nW22 & 0x3F80 ) >> 7;

        pXFD->SetFill( aFill );
    }

    if( nW12 & 0x0020 )
        pXFD->Merge();

    pExcRoot->pXF_Buffer->NewXF( pXFD );
    nXFCnt++;

#undef  HASATTRSET
}


void ImportExcel8::SXVs( void )
{
    if( !pCurrPivotCache )
        return;

    UINT16 nSrcType;
    aIn >> nSrcType;
    pCurrPivotCache->SetSourceType( nSrcType );
}


void ImportExcel8::Cellmerging( void )
{
    UINT16  n, nR1, nR2, nC1, nC2;

    aIn >> n;

    DBG_ASSERT( nBytesLeft >= 2 + n * 8, "*ImportExcel8::Cellmerging(): in die Hose!" );

    while( n )
    {
        aIn >> nR1 >> nR2 >> nC1 >> nC2;

        if( nR1 > MAXROW )
            nR1 = MAXROW;
        if( nR2 > MAXROW )
            nR2 = MAXROW;
        if( nC1 > MAXCOL )
            nC1 = MAXCOL;
        if( nC2 > MAXCOL )
            nC2 = MAXCOL;

        pD->DoMerge( nTab, nC1, nR1, nC2, nR2 );
        n--;
    }
}


struct BackgroundGraphic
{
    sal_uInt32 nMagicNumber;
    sal_uInt32 nUnknown1;
    sal_uInt32 nUnknown2;
    sal_uInt16 nWidth;
    sal_uInt16 nHeight;
    sal_uInt16 nPlanes;
    sal_uInt16 nBitsPerPixel;
};


static sal_Bool lcl_ImportBackgroundGraphic( SvStream& rIn, sal_uInt32 nSize, Graphic& rGraphic )
{
    sal_Bool    bRetValue = FALSE;
    sal_uInt16  nOldNumberFormat( rIn.GetNumberFormatInt() );

    if( nSize > sizeof( BackgroundGraphic ) )
    {
        rIn.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

        BackgroundGraphic   aBackground;
        rIn >> aBackground.nMagicNumber
            >> aBackground.nUnknown1
            >> aBackground.nUnknown2
            >> aBackground.nWidth
            >> aBackground.nHeight
            >> aBackground.nPlanes
            >> aBackground.nBitsPerPixel;
        if( ( aBackground.nMagicNumber == 0x00010009 )
            && ( aBackground.nBitsPerPixel == 24 )
            && ( aBackground.nPlanes == 1 ) )
        {
            sal_Bool            bImportPossible;
            sal_Bool            bAlignment = FALSE;
            sal_uInt32          nSizeLeft = nSize - 20;
            sal_uInt32          nWidth = aBackground.nWidth;
            sal_uInt32          nHeight = aBackground.nHeight;
            const sal_uInt32    nGuessSize = nWidth * nHeight * 3;
            bImportPossible = ( nGuessSize == nSizeLeft );
            if( !bImportPossible )
            {
                bImportPossible = ( ( nGuessSize + nHeight ) == nSizeLeft );
                bAlignment = TRUE;
            }
            if( bImportPossible )
            {
                Bitmap              aBmp( Size( nWidth, nHeight ), aBackground.nBitsPerPixel );
                BitmapWriteAccess*  pAcc = aBmp.AcquireWriteAccess();

                sal_uInt8           nBlue, nGreen, nRed;
                sal_uInt32          x, y;
                for( y = 0 ; y < nHeight ; y++ )
                {
                    for( x = 0 ; x < nWidth ; x++ )
                    {
                        rIn >> nBlue >> nGreen >> nRed;

                        pAcc->SetPixel( y, x, BitmapColor( nRed, nGreen, nBlue ) );

                    }

                    if( bAlignment )
                        rIn.SeekRel( 1 );
                }

                aBmp.ReleaseAccess( pAcc );
                rGraphic = aBmp;
                bRetValue = TRUE;
            }
        }
    }
    rIn.SetNumberFormatInt( nOldNumberFormat );

    return bRetValue;
}


UINT32 ImportExcel8::BGPic( UINT32 n )
{
    // no documentation available, but it might be, that it's only wmf format
    DBG_ASSERT( n <= 0xFFFF, "*ImportExcel8::BGPic(): record to long!" );
    DBG_ASSERT( pStyleSheetItemSet, "-ImportExcel::BGPic(): f... no style sheet!" );

    UINT32              nSumLen;
    UINT32              nNextRecord;

    SvMemoryStream*     pMemStr = CreateContinueStream( ( UINT16 ) n, nSumLen, nNextRecord, TRUE );
    pMemStr->Seek( STREAM_SEEK_TO_BEGIN );

//  BOOL                b = FALSE;
//  if( b )
//  {
//      SvFileStream    aOStr( _STRINGCONST( "svstr.dmp" ), STREAM_STD_WRITE );
//      aOStr << *pMemStr;
//      aOStr.Close();
//  }

    Graphic             aGraphic;
    if( lcl_ImportBackgroundGraphic( *pMemStr, nSumLen, aGraphic ) )
        pStyleSheetItemSet->Put( SvxBrushItem( aGraphic, GPOS_AREA ) );

    delete pMemStr;

    return nNextRecord;
}


UINT32 ImportExcel8::Msodrawinggroup( const UINT32 n )
{
    DBG_ASSERT( n <= 0xFFFF, "*ImportExcel8::Msodrawinggroup(): Record zu lang!" );

    UINT32              nSumLen;
    UINT32              nNextRecord;

    const DffRecordHeader* pLatestRecHd =
                aExcStreamConsumer.Consume( CreateContinueStream( ( UINT16 ) n, nSumLen, nNextRecord, TRUE ), 0 );

    return nNextRecord;
}


void ImportExcel8::Msodrawing( const UINT32 nL )
{
    bCond4EscherCont = TRUE;
    const UINT32        nFallBack = aIn.Tell();

    if ( !aExcStreamConsumer.GetStream() )
        return;

    if( bTabStartDummy )
    {// Dummy fuer ungueltigen ersten Shape einfuegen
        aEscherObjList.Append( new ExcEscherObj( 0, 0, nTab, pExcRoot ) );

        bTabStartDummy = FALSE;
    }
    const UINT32 nS = aExcStreamConsumer.GetStream()->Tell();
    if( nL )
    {
        const DffRecordHeader* pLatestRecHd = aExcStreamConsumer.Consume( &aIn, nL );
        if ( pLatestRecHd )
        {
            switch ( pLatestRecHd->nRecType )
            {
                case DFF_msofbtClientData :
                {
                    sal_Char* pBuf = new sal_Char[ 0x100 ];
                    aExcStreamConsumer.AppendData( pBuf, 0x100 );
                    delete pBuf;
                }
                break;
                case DFF_msofbtClientTextbox :
                {
                    sal_Char* pBuf = new sal_Char[ 0x200 ];
                    aExcStreamConsumer.AppendData( pBuf, 0x200 );
                    delete pBuf;
                }
                break;
            }
        }
        nBytesLeft = 0;
    }
    if( bLeadingTxo )
    {
        DBG_ASSERTWARNING( pActEscherObj && pActEscherObj->GetObjType() == OT_TXO,
                    "ImportExcel8::Msodrawing(): Vorgaenger wech oder falsch!" );
        bLeadingTxo = FALSE;
    }
    else
    {
        if( pActEscherObj )
            delete pActEscherObj;   // aEscherObjList.Append( pActEscherObj );

        pActEscherObj = new ExcEscherObj( nS, aExcStreamConsumer.GetStream()->Tell() - 1, nTab, pExcRoot );

        if( bMaybeTxo )
        {
            if( nL <= 8 )
            {
                UINT16  nFBT;

                aIn.Seek( nFallBack + 2 );

                aIn >> nFBT;

                if( nFBT != 0xF00D )
                    bMaybeTxo = FALSE;  // != Client Text Box
            }
            else
                bMaybeTxo = FALSE;
        }
    }
}


void ImportExcel8::Msodrawingselection( const UINT32 n )
{
}


void ImportExcel8::SXRule( void )
{
}


void ImportExcel8::SXEx( void )
{
}


void ImportExcel8::SXFilt( void )
{
}


void ImportExcel8::SXSelect( void )
{
}


UINT32 ImportExcel8::Sst( void )
{
    SvStream*           pStr;

    UINT16              n = ( UINT16 ) nBytesLeft;
    UINT32              nSumLen;
    UINT32              nNextRecord;
    UINT32List          aCutPosList;
    SvMemoryStream*     pMemStr = CreateContinueStream( n, nSumLen, nNextRecord, FALSE, &aCutPosList );
    INT32               nReadSize;

    if( pMemStr )
    {// Struktur mit Folge-Records
        nReadSize = ( INT32 ) nSumLen;
        pStr = pMemStr;
        pStr->Seek( STREAM_SEEK_TO_BEGIN );
    }
    else
    {// 'Pure' Tabelle
        pStr = &aIn;
        nReadSize = ( INT32 ) n;
    }

    pStr->SeekRel( 8 );
    nReadSize -= 8;

    ShStrTabEntry*      p;

    while( nReadSize > 0 )
    {
        p = CreateUnicodeEntry( *pStr, nReadSize, eQuellChar, 0, &aCutPosList );
        aSharedStringTable.Append( p );
    }

    if( pMemStr )
        delete pMemStr;

    return nNextRecord;
}


SvMemoryStream* ImportExcel8::CreateContinueStream( const UINT16 n, UINT32& rSLen, UINT32& rNPR,
    const BOOL bForceSingle, UINT32List* pCutPosList )
{
    DBG_ASSERT( n, "-ImportExcel8::CreateContinueStream(): Base-Record muss zumindest Daten enthalten!" );

    bCond4EscherCont = FALSE;

    const UINT32        nStartPos = aIn.Tell();
    UINT16              nId;
    SvMemoryStream*     pMemStr;
    UINT32              nNextRecord = nStartPos;

    aIn.SeekRel( n );
    aIn >> nId;

    aIn.Seek( nStartPos );

    if( nId == 0x3C )
    {// Struktur mit Folge-Records
        UINT16          nBufferLen = n;
        UINT16          nLen = n;
        sal_Char*           pBuffer = new sal_Char[ nBufferLen ];

        pMemStr = new SvMemoryStream;

        aIn.Read( pBuffer, nLen );
        pMemStr->Write( pBuffer, nLen );

        aIn >> nId >> nLen;

        do
        {
            if( nLen > nBufferLen )
            {
                delete[] pBuffer;
                nBufferLen = nLen;
                pBuffer = new sal_Char[ nBufferLen ];
            }

            aIn.Read( pBuffer, nLen );

            if( pCutPosList )
            {
//              UINT32  n = pMemStr->Tell();
                pCutPosList->Append( pMemStr->Tell() ); // Schnittstelle merken
            }

            pMemStr->Write( pBuffer, nLen );

            aIn >> nId >> nLen;
        }
        while( nId == 0x3C );

        rSLen = pMemStr->Tell();
        pMemStr->Seek( STREAM_SEEK_TO_END );

        rNPR = aIn.Tell() - 4;

        delete[] pBuffer;

        nBytesLeft = 0;
    }
    else
    {// 'Pure' Record
        rNPR = nStartPos + n;
        if( bForceSingle )
        {
            pMemStr = new SvMemoryStream;
            sal_Char*           pBuffer = new sal_Char[ n ];

            aIn.Read( pBuffer, n );
            pMemStr->Write( pBuffer, n );

            delete[] pBuffer;

            nBytesLeft = 0;
        }
        else
            pMemStr = NULL;
        rSLen = n;
    }

    return pMemStr;
}


ScBaseCell* ImportExcel8::CreateCellFromShStrTabEntry( const ShStrTabEntry* p, const UINT16 nXF )
{
    ScBaseCell*             pRet = NULL;
    if( p )
    {
        if( p->GetString().Len() > 0 )
        {
            if( p->HasFormats() )
            {
                ScEditEngineDefaulter&      rEdEng = GetEdEng();
                EditTextObject* pTextObj = p->CreateEditTextObject(
                                                rEdEng, *pExcRoot->pFontBuffer );

                DBG_ASSERT( pTextObj, "-ImportExcel8::Labelsst(): Keiner hat mich lieb!" );

                pRet = new ScEditCell( pTextObj, pD, rEdEng.GetEditTextObjectPool() );

                delete pTextObj;
            }
            else if( pExcRoot->pXF_Buffer->HasAttribSuperOrSubscript( nXF ) )
            {
                EditTextObject*     pTObj = CreateFormText( 0, p->GetString(), nXF );

                pRet = new ScEditCell( pTObj, pD, GetEdEng().GetEditTextObjectPool() );

                delete pTObj;
            }
            else
                pRet = ScBaseCell::CreateTextCell( p->GetString(), pD );
        }
    }
    else
        pRet = ScBaseCell::CreateTextCell( aSstErrTxt, pD );

    return pRet;
}


void ImportExcel8::SXVdex( void )
{
    if( !pCurrPivTab )
        return;

    UINT32 nFlags;
    aIn >> nFlags;
    pCurrPivTab->SetShowEmpty( TRUEBOOL( nFlags & 0x00000001 ) );
}


void ImportExcel8::Condfmt( void )
{
    pActCondForm = new ExcCondForm( pExcRoot );

    if( !pCondFormList )
        pCondFormList = new ExcCondFormList;

    pCondFormList->Append( pActCondForm );

    pActCondForm->Read( aIn );
}


void ImportExcel8::Cf( void )
{
    if( pActCondForm )
        pActCondForm->ReadCf( aIn, ( UINT16 ) nBytesLeft, *pFormConv );
}


void ImportExcel8::Dval( void )
{
}


void ImportExcel8::Labelsst( void )
{
    UINT16                      nRow, nCol, nXF;
    UINT32                      nSst;

    aIn >> nRow >> nCol >> nXF >> nSst;

    nBytesLeft -= 10;

    if( nRow <= MAXROW && nCol <= MAXCOL )
    {
        const ShStrTabEntry*    p = aSharedStringTable.Get( nSst );

        ScBaseCell*             pCell = CreateCellFromShStrTabEntry( p, nXF );
        if( pCell )
            pD->PutCell( nCol, nRow, nTab, pCell, ( BOOL ) TRUE );

        aColRowBuff.Used( nCol, nRow );

        pFltTab->SetXF( nCol, nRow, nXF );
    }
    else
        bTabTruncated = TRUE;

    pLastFormCell = NULL;
}


void ImportExcel8::Label( void )
{
    UINT16  nRow, nCol, nXF;
    aIn >> nRow >> nCol >> nXF;
    nBytesLeft -= 6;

    if( nRow <= MAXROW && nCol <= MAXCOL )
    {
        ShStrTabEntry*  p = CreateUnicodeEntry( aIn, nBytesLeft, eQuellChar );

        ScBaseCell*     pCell = CreateCellFromShStrTabEntry( p, nXF );
        if( pCell )
            pD->PutCell( nCol, nRow, nTab, pCell, ( BOOL ) TRUE );

        aColRowBuff.Used( nCol, nRow );

        pFltTab->SetXF( nCol, nRow, nXF );

        delete p;
    }
    else
        bTabTruncated = TRUE;

    pLastFormCell = NULL;
}


void ImportExcel8::Tabid( void )
{
    DBG_ASSERT( pExcRoot->pImpTabIdBuffer, "ImportExcel8::Tabid - missing tab id buffer" );
    if( pExcRoot->pImpTabIdBuffer )
        pExcRoot->pImpTabIdBuffer->Fill( aIn, (UINT16)(nBytesLeft >> 1) );
}


void ImportExcel8::Supbook( void )
{
    pExcRoot->pCurrSupbook = new SupbookE( aIn, nBytesLeft, *pExcRoot );
    pExcRoot->pSupbookBuffer->Append( pExcRoot->pCurrSupbook );
}


void ImportExcel8::Txo( void )
{
    if( pActTxo )
        pActTxo->Clear();
    else
        pActTxo = new TxoCont;

    pActTxo->ReadTxo( aIn );

    if( pActEscherObj && pActEscherObj->GetObjType() != OT_NOTE )
        pActEscherObj = new ExcEscherTxo( pActEscherObj );

    if( bMaybeTxo )
    {
        if( !pActEscherObj )
            pActEscherObj = new ExcEscherObj( 0, 0, nTab, pExcRoot );

        pActEscherObj = new ExcEscherTxo( pActEscherObj );
    }
}


void ImportExcel8::Codename( BOOL bWorkbookGlobals )
{
    if( bHasBasic )
    {
        String  aName( ::ReadUnicodeString( aIn, nBytesLeft, eQuellChar ) );

        DBG_ASSERT( pExcRoot->pExtDocOpt, "-ImportExcel8::Codename(): nothing there to store!" );

        if( bWorkbookGlobals )
            pExcRoot->pExtDocOpt->SetCodename( aName );
        else
            pExcRoot->pExtDocOpt->AddCodename( aName );
    }
}


void ImportExcel8::Dv( void )
{
    UINT32      nFlags;

    aIn >> nFlags;
    nBytesLeft -= sizeof( nFlags );

    String      aPromptTitle( ::ReadUnicodeString( aIn, nBytesLeft, eQuellChar ) );
    String      aErrorTitle( ::ReadUnicodeString( aIn, nBytesLeft, eQuellChar ) );
    String      aPromptMessage( ::ReadUnicodeString( aIn, nBytesLeft, eQuellChar ) );
    String      aErrorMessage( ::ReadUnicodeString( aIn, nBytesLeft, eQuellChar ) );

    // vals
    if( nBytesLeft > 8 )
    {
        Ignore( nBytesLeft - 8 );

        UINT16  nR1, nR2, nC1, nC2;

        aIn >> nR1 >> nR2 >> nC1 >> nC2;

        for( UINT16 nC = nC1 ; nC <= nC2 ; nC++ )
            for( UINT16 nR = nR1 ; nR <= nR2 ; nR++ )
            {
                ScValidationMode    eValMode = ( ScValidationMode ) ( nFlags & 0x00000007 );
                                                // StarCalc matches Excel in lower 3 bits!
                ScConditionMode     eMode;
                switch( ( nFlags >> 20 )  & 0x00000007 )
                {
                    case 0x00:  eMode = SC_COND_BETWEEN;    break;
                    case 0x01:  eMode = SC_COND_NOTBETWEEN; break;
                    case 0x02:  eMode = SC_COND_EQUAL;      break;
                    case 0x03:  eMode = SC_COND_NOTEQUAL;   break;
                    case 0x04:  eMode = SC_COND_GREATER;    break;
                    case 0x05:  eMode = SC_COND_LESS;       break;
                    case 0x06:  eMode = SC_COND_EQGREATER;  break;
                    case 0x07:  eMode = SC_COND_EQLESS;     break;
                }
                String              aDummy;
                aDummy.AssignAscii( "1" );
                ScValidationData    aValidData( eValMode, eMode, aDummy, aDummy, pD, ScAddress( nC, nR, nTab ) );

                ULONG               nHandle = pD->AddValidationEntry( aValidData );
            }
    }
}


static BOOL lcl_Test( SvStream& r, const UINT32 n )
{
    UINT32      nOldPos = r.Tell();
    UINT32      nRef;

    r >> nRef;

    r.Seek( nOldPos );

    return nRef == n;
}


static void lcl_GetAbs( String& rPath, UINT16 nDl, SfxObjectShell* pDocShell )
{
    String      aTmpStr;

    if( nDl )
    {
        while( nDl )
        {
            aTmpStr.AppendAscii( "../" );
            nDl--;
        }
    }

    aTmpStr += rPath;

    bool bWasAbs = false;
    rPath = pDocShell->GetMedium()->GetURLObject().smartRel2Abs( aTmpStr, bWasAbs ).GetMainURL();
}


void ImportExcel8::Hlink( void )
{
    UINT16      nRF, nRL, nCF, nCL;

    ReadX( nRF );
    ReadX( nRL );
    ReadX( nCF );
    ReadX( nCL );

    Ignore( 24 );                   // flags are not realy interresting

    UINT16          nDl = 0;        // counter for level to climb down in path
    String*         pL = NULL;      // link / file name
    String*         p8 = NULL;      // 8.3-representation from file name
    String*         pM = NULL;      // mark

    while( nBytesLeft > 0 )
    {
        UINT32      nStartPos = aIn.Tell();
        UINT32      n1, n2;

        aIn >> n1 >> n2;
        nBytesLeft -= sizeof( n1 ) + sizeof( n2 );

        if( n1 == 0x00000303 && n2 == 0x00000000 )
        {   // 8.3 file name or something else...
            Ignore( 8 );
            aIn >> nDl >> n1;
            nBytesLeft -= sizeof( nDl ) + sizeof( n1 );

            if( p8 )
                *p8 = ReadCString( n1, TRUE );
            else
                p8 = new String( ReadCString( n1, TRUE ) );

            lcl_GetAbs( *p8, nDl, pD->GetDocumentShell() );
        }
        else if( n1 == 0xDEADFFFF && n2 == 0x00000000 )
        {   // file name
            Ignore( 16 );
            aIn >> n2 >> n1;
            nBytesLeft -= sizeof( n1 ) + sizeof( n2 );
            Ignore( 2 );

            if( nBytesLeft > 0 )
            {
                if( pL )
                    *pL = ReadWString( n1, TRUE );
                else
                    pL = new String( ReadWString( n1, TRUE ) );

                lcl_GetAbs( *pL, nDl, pD->GetDocumentShell() );
            }
            else
                nBytesLeft = 0;
        }
        else if( n1 == 0x79EAC9E0 && n2 == 0x11CEBAF9 )
        {   // URL
            Ignore( 8 );
            aIn >> n1;
            nBytesLeft -= sizeof( n1 );

            if( pL )
                *pL = ReadWString( n1, TRUE );
            else
                pL = new String( ReadWString( n1, TRUE ) );
        }
        else
        {   // text or mark
            nBytesLeft += 4;
            aIn.Seek( nStartPos + 4 );  // n1 still valid!
            n1 *= 2;                    // n1 was number of chars

            if( p8 || pL )
            {
                if( pM )
                    *pM = ReadWString( n1, TRUE );
                else
                    pM = new String( ReadWString( n1, TRUE ) );
            }
            else
                Ignore( n1 );
        }
    }


    String*         pHlink = NULL;

    if( pL )
    {
        pHlink = pL;
        pL = NULL;
    }
    else if( p8 )
    {
        pHlink = p8;
        p8 = NULL;
    }

    if( pHlink )
    {
        if( pM )
        {
            *pHlink += '#';
            *pHlink += *pM;
        }

        for( UINT16 nCol = nCF ; nCol <= nCL ; nCol++ )
        {
            for( UINT16 nRow = nRF ; nRow <= nRL ; nRow++ )
                InsertHyperlink( nCol, nRow, *pHlink );
        }
    }

    if( pL )
        delete pL;
    if( p8 )
        delete p8;
    if( pM )
        delete pM;
    if( pHlink )
        delete pHlink;
}


String* ImportExcel8::ReadFileHlink( void )
{   // 2 B deleted
    return NULL;
}


String* ImportExcel8::ReadURLHlink( void )
{   // 2 B deleted
    return NULL;
}


void ImportExcel8::Dimensions( void )
{
    UINT32  nRowFirst, nRowLast;
    UINT16  nColFirst, nColLast;

    aIn >> nRowFirst >> nRowLast >> nColFirst >> nColLast;
    nBytesLeft -= 12;

    if( nRowLast > MAXROW )
        nRowLast = MAXROW;
    if( nColLast > MAXCOL )
        nColLast = MAXCOL;
    if( nRowFirst > nRowLast )
        nRowFirst = nRowLast;
    if( nColFirst > nColLast )
        nColFirst = nColLast;

    aColRowBuff.SetDimension(
        ScRange( nColFirst, ( UINT16 ) nRowFirst, nTab, nColLast, ( UINT16 ) nRowLast, nTab ) );
}


void ImportExcel8::Name( void )
{
    const ScTokenArray* pErgebnis;
    UINT16              nLenDef;
    BYTE                nLenName;
    BYTE                nLen;

    UINT16              nOpt;
    UINT16              nLenSeekRel = 0;
    UINT16              nSheet;

    aIn >> nOpt;
    aIn.SeekRel( 1 );
    aIn >> nLenName >> nLenDef >> nSheet;
    aIn.SeekRel( 2 );
    aIn >> nLen;            // length of custom menu text
    nLenSeekRel += nLen;
    aIn >> nLen;            // length of description text
    nLenSeekRel += nLen;
    aIn >> nLen;            // length of help topic text
    nLenSeekRel += nLen;
    aIn >> nLen;            // length of status bar text
    nLenSeekRel += nLen;
    nBytesLeft -= 14;

    pFormConv->Reset( nLenDef );

    // Namen einlesen
    String              aName( ::ReadUnicodeString( aIn, nBytesLeft, eQuellChar, nLenName ) );
    // jetzt steht Lesemarke an der Formel

    sal_Unicode         cFirstChar = aName.GetChar( 0 );

    const UINT32        nFormStart = aIn.Tell();
    const BOOL          bHidden = TRUEBOOL( nOpt & EXC_NAME_HIDDEN );
    const BOOL          bBuiltIn = TRUEBOOL( nOpt & EXC_NAME_BUILTIN );
    const BOOL          bPrintArea = bBuiltIn && ( cFirstChar == EXC_BUILTIN_PRINTAREA );
    const BOOL          bPrintTitles = bBuiltIn && ( cFirstChar == EXC_BUILTIN_PRINTTITLES );
    const BOOL          bAutoFilter = bBuiltIn && ( cFirstChar == EXC_BUILTIN_AUTOFILTER );
    const BOOL          bCriteria = bBuiltIn && ( cFirstChar == EXC_BUILTIN_CRITERIA );
    const BOOL          bExtract = bBuiltIn && ( cFirstChar == EXC_BUILTIN_EXTRACT );
    BOOL                bAppendTabNum = FALSE;
    BOOL                bSkip = FALSE;

    if( bBuiltIn )
    {// Built-in name
        aName.AssignAscii( GetExcBuiltInName( cFirstChar ) );
    }
    else
        aName = ExcelNameToScName( aName );

    if( nOpt & (EXC_NAME_VB | EXC_NAME_BIG) )
        // function or command?
        pFormConv->GetDummy( pErgebnis );
    else if( bBuiltIn )
    {
        if( bPrintArea )
        {// Druckbereich
            long nAnzBytes = nLenDef;
            pFormConv->Convert( *pPrintRanges, nAnzBytes, FT_RangeName );
        }
        else if( bPrintTitles )
        {// Druckbereich
            long nAnzBytes = nLenDef;
            pFormConv->Convert( *pPrintTitles, nAnzBytes, FT_RangeName );
        }
        aIn.Seek( nFormStart );

        long nAnzBytes = nLenDef;
        pFormConv->Convert( pErgebnis, nAnzBytes, FT_RangeName );

        // AutoFilter
        if( pErgebnis && (bAutoFilter || bCriteria || bExtract) )
        {
            ScRange aRange;
            if( pErgebnis->IsReference( aRange ) )  // test & get range
            {
                aName += String::CreateFromInt32( (sal_Int32) aRange.aStart.Tab() );
                bSkip = bAutoFilter;

                if( bAutoFilter )
                    pAutoFilter->Insert( pExcRoot, aRange, aName );
                else if( bCriteria )
                    pAutoFilter->AddAdvancedRange( aRange );
                else if( bExtract )
                    pAutoFilter->AddExtractPos( aRange );
            }
        }
    }
    else
    {// Formel
        long nAnzBytes = nLenDef;
        pFormConv->Convert( pErgebnis, nAnzBytes, FT_RangeName );
    }

    if( !bSkip )
    {
        if( bHidden )
            pExcRoot->pRNameBuff->Store( aName, NULL, nSheet );
        else
            // ohne hidden
            pExcRoot->pRNameBuff->Store( aName, pErgebnis, nSheet, bPrintArea );
    }
}


void ImportExcel8::Style( void )
{
    UINT16      nXf;

    aIn >> nXf;
    nBytesLeft -= sizeof( nXf );

    if( !( nXf & 0x8000 ) )
    {
        nXf &= 0x0FFF;  // only bit 0...11 is used for XF-index

        pExcRoot->pXF_Buffer->SetStyle( nXf, ReadUnicodeString( aIn, nBytesLeft, eQuellChar ) );
    }
}


void ImportExcel8::GetHFString( String& r )
{
    r = ::ReadUnicodeString( aIn, nBytesLeft, eQuellChar );
}


void ImportExcel8::EndSheet( void )
{
    pActCondForm = NULL;

    ImportExcel::EndSheet();
}


void ImportExcel8::PostDocLoad( void )
{
    if( pCondFormList )
        pCondFormList->Apply();

    pAutoFilter->Apply();

    if( aExcStreamConsumer.GetStream() )
    {
        Biff8MSDffManager*      pDffMan = new Biff8MSDffManager( pExcRoot, aPosBuffer, aEscherObjList,
                                            *aExcStreamConsumer.GetStream(), 0, 0, pD->GetDrawLayer(), 1440 );

        const UINT32                    nMax = aPosBuffer.Count();
        const ClientAnchorData*         pAnch;

        const SvxMSDffShapeInfos*       pShpInf = pDffMan->GetShapeInfos();

        if( pShpInf )
        {
            const UINT32                nMax = pShpInf->Count();
            UINT32                      nChartCnt = 0;
            const SvxMSDffShapeInfo*    p;
            ULONG                       nShapeId;
            SdrObject*                  pSdrObj = NULL;
            SvxMSDffImportData*         pMSDffImportData;
            UINT32                      n;
            UINT32                      nObjNum;
            BOOL                        bRangeTest;

            UINT32                      nOLEImpFlags = 0;
            OfaFilterOptions*           pFltOpts = OFF_APP()->GetFilterOptions();
            if( pFltOpts )
            {
                if( pFltOpts->IsMathType2StarMath() )
                    nOLEImpFlags |= OLE_MATHTYPE_2_STARMATH;

                if( pFltOpts->IsWinWord2StarWriter() )
                    nOLEImpFlags |= OLE_WINWORD_2_STARWRITER;

                if( pFltOpts->IsPowerPoint2StarImpress() )
                    nOLEImpFlags |= OLE_POWERPOINT_2_STARIMPRESS;
            }

            for( n = 0 ; n < nMax ; n++ )
            {
                p = pShpInf->GetObject( ( UINT16 ) n );
                DBG_ASSERT( p, "-ImportExcel8::PostDocLoad(): Immer diese falschen Versprechungen!" );

                nShapeId = p->nShapeId;

                if( aPosBuffer.GetObjNum( p->nFilePos, nObjNum ) )
                {
                    pMSDffImportData = new SvxMSDffImportData;

                    pSdrObj = NULL;
                    pDffMan->GetShape( nShapeId, pSdrObj, *pMSDffImportData );

                    if( pSdrObj )
                    {
                        pAnch = aPosBuffer.GetAnchorData( nObjNum );
                        bRangeTest = FALSE;
                        if( pAnch )
                        {
                            bRangeTest = aPivotTabList.IsInPivotRange( pAnch->nCol, pAnch->nRow, pAnch->nTab );
                            if( pAutoFilter )
                                bRangeTest |= pAutoFilter->HasDropDown( pAnch->nCol, pAnch->nRow, pAnch->nTab );
                        }
                        if( bRangeTest )
                        {
                            delete pSdrObj;
                            pSdrObj = NULL;
                        }
                        else
                        {
                            ExcEscherObj* p = (ExcEscherObj*) aEscherObjList.Get( nObjNum );
                            if ( p )
                            {
                                p->SetObj( pSdrObj );
                                switch ( p->GetObjType() )
                                {
                                    case OT_CHART :
                                        nChartCnt++;
                                    break;
                                    case OT_OLE :
                                        ((ExcEscherOle*)p)->CreateSdrOle( *pDffMan, nOLEImpFlags );
                                    break;
                                }
                            }
                        }
                    }

                    delete pMSDffImportData;
                }
            }
            if( pExcRoot->pProgress )
                pExcRoot->pProgress->StartPostLoadProgress( nChartCnt );
        }

        delete pDffMan;
    }

    aEscherObjList.Apply();

    ImportExcel::PostDocLoad();

    // Scenarien bemachen! ACHTUNG: Hier wird Tabellen-Anzahl im Dokument erhoeht!!
    if( aScenList.Count() )
    {
        pD->UpdateChartListenerCollection();    // references in charts must be updated

        aScenList.Apply( *pD );
    }

    // BASIC
    if( bHasBasic )
    {
        OfaFilterOptions*   pFiltOpt = OFF_APP()->GetFilterOptions();

        if( pFiltOpt )
        {
            if( pFiltOpt->IsLoadExcelBasicCode() || pFiltOpt->IsLoadExcelBasicStorage() )
            {
                DBG_ASSERT( pExcRoot->pRootStorage, "-ImportExcel8::PostDocLoad(): no storage, no cookies!" );

                SvxImportMSVBasic   aBasicImport( *pD->GetDocumentShell(), *pExcRoot->pRootStorage,
                                                    pFiltOpt->IsLoadExcelBasicCode(),
                                                    pFiltOpt->IsLoadExcelBasicStorage() );

                aBasicImport.Import( String::CreateFromAscii( pVBAStorageName ),
                                     String::CreateFromAscii( pVBASubStorageName ) );
            }
        }
    }

    // read doc info
    SfxDocumentInfo     aNewDocInfo;
    SfxDocumentInfo&    rOldDocInfo = pD->GetDocumentShell()->GetDocInfo();

    aNewDocInfo.LoadPropertySet( pExcRoot->pRootStorage );

    rOldDocInfo = aNewDocInfo;
    pD->GetDocumentShell()->Broadcast( SfxDocumentInfoHint( &rOldDocInfo ) );
}



void ImportExcel8::InsertHyperlink( const UINT16 nCol, const UINT16 nRow, const String& rURL )
{
    ScAddress       aAddr( nCol, nRow, nTab );

    const CellType  e = pD->GetCellType( aAddr );

    if( e == CELLTYPE_FORMULA || e == CELLTYPE_VALUE )
        return;

    String          aOrgText;
    pD->GetString( nCol, nRow, nTab, aOrgText );
    if( !aOrgText.Len() )
        aOrgText = rURL;

    EditEngine&     rEdEng = GetEdEng();
    ESelection      aAppSel( 0xFFFF, 0xFFFF );

    rEdEng.SetText( EMPTY_STRING );

    SvxURLField     aUrlField( rURL, aOrgText, SVXURLFORMAT_APPDEFAULT );

    rEdEng.QuickInsertField( SvxFieldItem( aUrlField ), aAppSel );

    EditTextObject* pTextObj = rEdEng.CreateTextObject();

    ScBaseCell*     pCell = new ScEditCell( pTextObj, pD, GetEdEng().GetEditTextObjectPool() );

    delete pTextObj;

    pD->PutCell( aAddr, pCell, ( BOOL ) TRUE );
}


String ImportExcel8::ReadWString( UINT16 nLen, const BOOL b )
{
    String          aString;
    DBG_ASSERT( sizeof( sal_Unicode ) == 2, "ImportExcel8::ReadWString(): sal_Unicode not Excel-conform" );

    if( b )
    {
        if( ( INT32 ) nLen > nBytesLeft )
        {
            aIn.SeekRel( nBytesLeft );
            nLen = 0;
            nBytesLeft = 0;
        }
        else
        {
            DBG_ASSERT( nLen % 2 == 0, "*ImportExcel8::ReadWString(): Unicode with odd number of bytes!" );

            nLen /= 2;
            nBytesLeft -= nLen * 2;
        }
    }

    sal_Unicode     c;
    while( nLen )
    {
        aIn >> c;
        if( c )
            aString += c;
        nLen--;
    }

    return aString;
}


String ImportExcel8::ReadCString( UINT16 n, const BOOL b )
{
    if( b && ( INT32 ) n > nBytesLeft )
    {
        aIn.SeekRel( nBytesLeft );
        nBytesLeft = 0;
        n = 0;
    }

    if( n )
    {
        sal_Char*   p = new sal_Char[ n + 1 ];

        aIn.Read( p, n );

        if( b )
            nBytesLeft -= n;

        p[ n ] = 0x00;

        return String( p, eQuellChar );
    }
    else
        return EMPTY_STRING;
}


void ImportExcel8::CreateTmpCtrlStorage( void )
{
//  if( pExcRoot->pCtrlStorage )
    if( pExcRoot->xCtrlStorage.Is() )
        return;     // already done

    SvStorageStream*    pContrIn = pExcRoot->pRootStorage->OpenStream( _STRINGCONST( "Ctls" ), STREAM_STD_READ );
    if( pContrIn )
    {
        SvStorageRef    xStrg( new SvStorage( new SvMemoryStream(), TRUE ) );
        pExcRoot->xCtrlStorage = SvStorageRef( new SvStorage( new SvMemoryStream(), TRUE ) );
//      SvStorage*      pStrg = new SvStorage( new SvMemoryStream(), TRUE );

//      SvStorageStreamRef  xTemp = pStrg->OpenStream( _STRINGCONST( "contents" ) );
        SvStorageStreamRef  xTemp = xStrg->OpenStream( _STRINGCONST( "contents" ) );
        if ( xTemp.Is() && ( xTemp->GetError() == SVSTREAM_OK ) )
        {
            pContrIn->Seek( 16 );   // no need for class id at this point
            *xTemp << *pContrIn;

        SvGlobalName    aName( 0xD7053240, 0xCE69, 0x11CD, 0xA7, 0x77,
                                    0x00, 0xDD, 0x01, 0x14, 0x3C, 0x57 );
        UINT32              nClip = Exchange::RegisterFormatName( _STRING( "Embedded Object" ) );
//      pStrg->SetClass( aName, nClip, _STRING( "Microsoft Forms 2.0 CommandButton" ) );
        xStrg->SetClass( aName, nClip, _STRING( "Microsoft Forms 2.0 CommandButton" ) );

        pExcRoot->xCtrlStorage = xStrg;
        }
/*      else
        {
            delete pStrg;
            pStrg = NULL;
        }*/

//      pExcRoot->pCtrlStorage = pStrg;
    }
}




XtiBuffer::~XtiBuffer()
{
    Xti*    p = ( Xti* ) List::First();
    while( p )
    {
        delete p;
        p = ( Xti* ) List::Next();
    }
}


void XtiBuffer::Read( SvStream& r, UINT32 n, INT32& rBytesLeft )
{
    Xti*    p;

    rBytesLeft -= n * 6;

    while( n )
    {
        p = new Xti;

        r >> p->nSupbook >> p->nFirst >> p->nLast;

        List::Insert( p, LIST_APPEND );

        n--;
    }
}




struct SupbookTab
{
    String      aName;
    UINT16      nScNum;
};




SupbookE::SupbookE( SvStream& rIn, INT32& rBytesLeft, RootData& rExcRoot )
{
    UINT16          nTabCnt;
    SupbookTab*     pNewTab;

    rIn >> nTabCnt;
    rBytesLeft -= 2;

    if( rBytesLeft < 2 + 2 * nTabCnt )
    {// verkuerzter Record _ohne_ Strings
        bSelf = TRUE;       // erste Naeherung...

        rIn.SeekRel( rBytesLeft );
        rBytesLeft = 0;
    }
    else
    {
        bSelf = FALSE;      // erste Naeherung...

        String      aTabName;

        EncodeExternSheetUnicode( rIn, aFileName, aTabName, rBytesLeft, bSelf );

        if( nTabCnt )
        {
            while( nTabCnt )
            {
                pNewTab = new SupbookTab;

                String&     rTabName = pNewTab->aName;
                rTabName = ::ReadUnicodeString( rIn, rBytesLeft, *rExcRoot.pCharset, 0 );
                ExcelNameToScName( rTabName );

                if( rExcRoot.pExtDocOpt->nLinkCnt < 1 )
                {
                    UINT16      nNewTabNum;
                    String      aURL( ScGlobal::GetAbsDocName( aFileName,
                                        rExcRoot.pDoc->GetDocumentShell() ) );
                    String      aTabName( ScGlobal::GetDocTabName( aURL, rTabName ) );

                    if( rExcRoot.pDoc->LinkEmptyTab( nNewTabNum, aTabName, aURL, rTabName ) )
                        pNewTab->nScNum = nNewTabNum;
                    else
                        pNewTab->nScNum = 0xFFFF;
                }
                else
                    pNewTab->nScNum = 0xFFFF;

                List::Insert( pNewTab, LIST_APPEND );

                nTabCnt--;
            }
        }
        else
        {// einen Ersatzeintrag generieren
            pNewTab = new SupbookTab;
            pNewTab->aName = aFileName;
            pNewTab->nScNum = 0xFFFF;
            List::Insert( pNewTab, LIST_APPEND );
        }
    }
}


SupbookE::~SupbookE()
{
    SupbookTab* p = ( SupbookTab* ) List::First();
    while( p )
    {
        delete p;
        p = ( SupbookTab* ) List::Next();
    }
}


BOOL SupbookE::IsValid( const UINT16 n ) const
{
    const SupbookTab*   p = ( const SupbookTab* ) List::GetObject( n );

    if( p )
        return p->nScNum != 0xFFFF;
    else
        return FALSE;
}


UINT16 SupbookE::GetScTabNum( const UINT16 n ) const
{
    const SupbookTab*   p = ( const SupbookTab* ) List::GetObject( n );

    if( p )
        return p->nScNum;
    else
        return 0xFFFF;
}




SupbookBuffer::~SupbookBuffer()
{
    SupbookE*   p = ( SupbookE* ) List::First();
    while( p )
    {
        delete p;
        p = ( SupbookE* ) List::Next();
    }
}




//___________________________________________________________________
// classes AutoFilterRange, AutoFilterRangeBuffer

AutoFilterData::AutoFilterData( RootData* pRoot, const ScRange& rRange, const String& rName ) :
        ExcRoot( pRoot ),
        nFirstEmpty( 0 ),
        bHasDropDown( FALSE ),
        bHasConflict( FALSE )
{
    aParam.nCol1 = rRange.aStart.Col();
    aParam.nRow1 = rRange.aStart.Row();
    aParam.nTab = rRange.aStart.Tab();
    aParam.nCol2 = rRange.aEnd.Col();
    aParam.nRow2 = rRange.aEnd.Row();

    ScDBCollection& rColl = *pRoot->pDoc->GetDBCollection();

    pCurrDBData = rColl.GetDBAtArea( Tab(), StartCol(), StartRow(), EndCol(), EndRow() );
    if( !pCurrDBData )
    {
        pCurrDBData = new ScDBData( rName, Tab(), StartCol(), StartRow(), EndCol(), EndRow() );
        if( pCurrDBData )
            rColl.Insert( pCurrDBData );
    }
}

void AutoFilterData::CreateFromDouble( String& rStr, double fVal )
{
    SolarMath::DoubleToString( rStr, fVal, 'A', INT_MAX,
        ScGlobal::pScInternational->GetNumDecimalSep(), TRUE );
}

void AutoFilterData::SetCellAttribs()
{
    bHasDropDown = TRUE;
    for ( UINT16 nCol = StartCol(); nCol <= EndCol(); nCol++ )
    {
        INT16 nFlag = ((ScMergeFlagAttr*) pExcRoot->pDoc->
            GetAttr( nCol, StartRow(), Tab(), ATTR_MERGE_FLAG ))->GetValue();
        pExcRoot->pDoc->ApplyAttr( nCol, StartRow(), Tab(),
            ScMergeFlagAttr( nFlag | SC_MF_AUTO) );
    }
}

void AutoFilterData::InsertQueryParam()
{
    if( pCurrDBData && !bHasConflict )
    {
        ScRange aAdvRange;
        BOOL    bHasAdv = pCurrDBData->GetAdvancedQuerySource( aAdvRange );
        if( bHasAdv )
            pExcRoot->pDoc->CreateQueryParam( aAdvRange.aStart.Col(),
                aAdvRange.aStart.Row(), aAdvRange.aEnd.Col(), aAdvRange.aEnd.Row(),
                aAdvRange.aStart.Tab(), aParam );

        pCurrDBData->SetQueryParam( aParam );
        if( bHasAdv )
            pCurrDBData->SetAdvancedQuerySource( &aAdvRange );
        else
        {
            pCurrDBData->SetAutoFilter( TRUE );
            SetCellAttribs();
        }
    }
}

BOOL AutoFilterData::HasDropDown( UINT16 nCol, UINT16 nRow, UINT16 nTab ) const
{
    return (bHasDropDown && (StartCol() <= nCol) && (nCol <= EndCol()) &&
            (nRow == StartRow()) && (nTab == Tab()));
}

void AutoFilterData::ReadAutoFilter( SvStream& rStrm, INT32& nLeft )
{
    UINT16 nCol, nFlags;
    rStrm >> nCol >> nFlags;

    ScQueryConnect  eConn       = (nFlags & EXC_AFFLAG_ANDORMASK) ? SC_OR : SC_AND;
    BOOL            bTop10      = TRUEBOOL( nFlags & EXC_AFFLAG_TOP10 );
    BOOL            bTopOfTop10 = TRUEBOOL( nFlags & EXC_AFFLAG_TOP10TOP );
    BOOL            bPercent    = TRUEBOOL( nFlags & EXC_AFFLAG_TOP10PERC );
    UINT16          nCntOfTop10 = nFlags >> 7;
    UINT16          nCount      = aParam.GetEntryCount();

    if( bTop10 )
    {
        if( nFirstEmpty < nCount )
        {
            ScQueryEntry& aEntry = aParam.GetEntry( nFirstEmpty );
            aEntry.bDoQuery = TRUE;
            aEntry.bQueryByString = TRUE;
            aEntry.nField = StartCol() + nCol;
            aEntry.eOp = bTopOfTop10 ?
                (bPercent ? SC_TOPPERC : SC_TOPVAL) : (bPercent ? SC_BOTPERC : SC_BOTVAL);
            aEntry.eConnect = SC_AND;
            aEntry.pStr->Assign( String::CreateFromInt32( (sal_Int32) nCntOfTop10 ) );

            rStrm.SeekRel( 20 );
            nFirstEmpty++;
        }
    }
    else
    {
        UINT8   nE, nType, nOper, nBoolErr, nVal;
        UINT32  nRK;
        double  fVal;
        BOOL    bIgnore;

        UINT8   nStrLen[ 2 ]    = { 0, 0 };
        String* pEntryStr[ 2 ]  = { NULL, NULL };

        for( nE = 0; nE < 2; nE++ )
        {
            if( nFirstEmpty < nCount )
            {
                ScQueryEntry& aEntry = aParam.GetEntry( nFirstEmpty );
                pEntryStr[ nE ] = aEntry.pStr;
                bIgnore = FALSE;

                rStrm >> nType >> nOper;
                switch( nOper )
                {
                    case EXC_AFOPER_LESS:
                        aEntry.eOp = SC_LESS;
                        break;
                    case EXC_AFOPER_EQUAL:
                        aEntry.eOp = SC_EQUAL;
                        break;
                    case EXC_AFOPER_LESSEQUAL:
                        aEntry.eOp = SC_LESS_EQUAL;
                        break;
                    case EXC_AFOPER_GREATER:
                        aEntry.eOp = SC_GREATER;
                        break;
                    case EXC_AFOPER_NOTEQUAL:
                        aEntry.eOp = SC_NOT_EQUAL;
                        break;
                    case EXC_AFOPER_GREATEREQUAL:
                        aEntry.eOp = SC_GREATER_EQUAL;
                        break;
                    default:
                        aEntry.eOp = SC_EQUAL;
                }

                switch( nType )
                {
                    case EXC_AFTYPE_RK:
                        rStrm >> nRK;
                        rStrm.SeekRel( 4 );
                        CreateFromDouble( *aEntry.pStr, ImportExcel::RkToDouble( nRK ) );
                        break;
                    case EXC_AFTYPE_DOUBLE:
                        rStrm >> fVal;
                        CreateFromDouble( *aEntry.pStr, fVal );
                        break;
                    case EXC_AFTYPE_STRING:
                        rStrm.SeekRel( 4 );
                        rStrm >> nStrLen[ nE ];
                        rStrm.SeekRel( 3 );
                        aEntry.pStr->Erase();
                        break;
                    case EXC_AFTYPE_BOOLERR:
                        rStrm >> nBoolErr >> nVal;
                        rStrm.SeekRel( 6 );
                        aEntry.pStr->Assign( String::CreateFromInt32( (sal_Int32) nVal ) );
                        bIgnore = (BOOL) nBoolErr;
                        break;
                    case EXC_AFTYPE_EMPTY:
                        aEntry.bQueryByString = FALSE;
                        aEntry.nVal = SC_EMPTYFIELDS;
                        aEntry.eOp = SC_EQUAL;
                        break;
                    case EXC_AFTYPE_NOTEMPTY:
                        aEntry.bQueryByString = FALSE;
                        aEntry.nVal = SC_NONEMPTYFIELDS;
                        aEntry.eOp = SC_EQUAL;
                        break;
                    default:
                        rStrm.SeekRel( 8 );
                        bIgnore = TRUE;
                }

                if( (nE > 0) && (nCol > 0) && (eConn == SC_OR) && !bIgnore )
                    bHasConflict = TRUE;
                if( !bHasConflict && !bIgnore )
                {
                    aEntry.bDoQuery = TRUE;
                    aEntry.bQueryByString = TRUE;
                    aEntry.nField = StartCol() + nCol;
                    aEntry.eConnect = nE ? eConn : SC_AND;
                    nFirstEmpty++;
                }
            }
            else
                rStrm.SeekRel( 10 );
        }

        nLeft -= 20;
        for( nE = 0; nE < 2; nE++ )
            if( nStrLen[ nE ] && pEntryStr[ nE ] )
                pEntryStr[ nE ]->Assign( ::ReadUnicodeString(
                    rStrm, nLeft, *pExcRoot->pCharset, nStrLen[ nE ] ) );
    }
}

void AutoFilterData::SetAdvancedRange( const ScRange* pRange )
{
    if( pCurrDBData )
        pCurrDBData->SetAdvancedQuerySource( pRange );
}

void AutoFilterData::SetExtractPos( const ScAddress& rAddr )
{
    aParam.nDestCol = rAddr.Col();
    aParam.nDestRow = rAddr.Row();
    aParam.nDestTab = rAddr.Tab();
    aParam.bInplace = FALSE;
    aParam.bDestPers = TRUE;
}

void AutoFilterData::Apply()
{
    InsertQueryParam();

    BYTE nFlags;
    for( UINT16 nRow = StartRow(); nRow <= EndRow(); nRow++ )
    {
        nFlags = pExcRoot->pDoc->GetRowFlags( nRow, Tab() );
        if( nFlags & CR_HIDDEN )
            nFlags |= CR_FILTERED;
        pExcRoot->pDoc->SetRowFlags( nRow, Tab(), nFlags );
    }
}




AutoFilterBuffer::~AutoFilterBuffer()
{
    for( AutoFilterData* pData = _First(); pData; pData = _Next() )
        delete pData;
}

void AutoFilterBuffer::Insert( RootData* pRoot, const ScRange& rRange,
                                const String& rName )
{
    if( !GetByTab( rRange.aStart.Tab() ) )
        Append( new AutoFilterData( pRoot, rRange, rName ) );
}

void AutoFilterBuffer::AddAdvancedRange( const ScRange& rRange )
{
    AutoFilterData* pData = GetByTab( rRange.aStart.Tab() );
    if( pData )
        pData->SetAdvancedRange( &rRange );
}

void AutoFilterBuffer::AddExtractPos( const ScRange& rRange )
{
    AutoFilterData* pData = GetByTab( rRange.aStart.Tab() );
    if( pData )
        pData->SetExtractPos( rRange.aStart );
}

void AutoFilterBuffer::Apply()
{
    for( AutoFilterData* pData = _First(); pData; pData = _Next() )
        pData->Apply();
}

AutoFilterData* AutoFilterBuffer::GetByTab( UINT16 nTab )
{
    for( AutoFilterData* pData = _First(); pData; pData = _Next() )
        if( pData->Tab() == nTab )
            return pData;
    return NULL;
}

BOOL AutoFilterBuffer::HasDropDown( UINT16 nCol, UINT16 nRow, UINT16 nTab )
{
    for( AutoFilterData* pData = _First(); pData; pData = _Next() )
        if( pData->HasDropDown( nCol, nRow, nTab ) )
            return TRUE;
    return FALSE;
}




