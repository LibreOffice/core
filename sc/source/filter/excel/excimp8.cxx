/*************************************************************************
 *
 *  $RCSfile: excimp8.cxx,v $
 *
 *  $Revision: 1.25 $
 *
 *  last change: $Author: dr $ $Date: 2001-03-15 09:02:52 $
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
#include <svx/unoapi.hxx>

#include <vcl/graph.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/exchange.hxx>

#include <tools/string.hxx>
#include <tools/urlobj.hxx>
#include <tools/solmath.hxx>

#include <unotools/localedatawrapper.hxx>

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
#include "markdata.hxx"

#ifndef _SC_XCLIMPSTREAM_HXX
#include "XclImpStream.hxx"
#endif
#ifndef _SC_XCLIMPHELPER_HXX
#include "XclImpHelper.hxx"
#endif

#include "excimp8.hxx"
#include "xfbuff.hxx"
#include "vfbuff.hxx"
#include "fontbuff.hxx"
#include "excform.hxx"
#include "fltprgrs.hxx"
#include "flttools.hxx"
#include "scextopt.hxx"
#include "stlpool.hxx"
#include "scmsocximexp.hxx"
#include "XclAddInNameTrans.hxx"

using namespace com::sun::star;

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
        rRect.nLeft     = XclImpHelper::CalcX( nAnchTab, nAnchCol, p->nX, HMM_PER_TWIPS, pDoc );
        rRect.nTop      = XclImpHelper::CalcY( nAnchTab, nAnchRow, p->nY, HMM_PER_TWIPS, pDoc );
        rRect.nRight    = XclImpHelper::CalcX( nAnchTab, p->nDCol, p->nDX, HMM_PER_TWIPS, pDoc );
        rRect.nBottom   = XclImpHelper::CalcY( nAnchTab, p->nDRow, p->nDY, HMM_PER_TWIPS, pDoc );

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
        if( p->IsMathType2Math() )
            n |= OLE_MATHTYPE_2_STARMATH;

        if( p->IsWinWord2Writer() )
            n |= OLE_WINWORD_2_STARWRITER;

        if( p->IsExcel2Calc() )
            n |= OLE_EXCEL_2_STARCALC;

        if( p->IsPowerPoint2Impress() )
            n |= OLE_POWERPOINT_2_STARIMPRESS;
    }

    return n;
}




String      ImportExcel8::aSstErrTxt( _STRINGCONST( "*** ERROR IN SST ***" ) );

#define INVALID_POS     0xFFFFFFFF



void TxoCont::ReadTxo( XclImpStream& rStrm )
{
    if( !nStepCount )
    {
        rStrm.Ignore( 10 );
        rStrm >> nTextLen >> nFormCnt;
        nFormCnt /= 8;

        nStepCount = 1;
    }
}


void TxoCont::ReadCont( XclImpStream& rStrm, RootData& rRootData, ScEditEngineDefaulter& rEdEngine )
{
    if( nStepCount == 1 )
    {// Record mit Text
        if( nTextLen )
        {
            if( !pText )
            {
                pText = new String;
                rStrm.AppendUniString( *pText, *rRootData.pCharset, nTextLen );
            }
        }

        nStepCount = 2;
    }
    else if( nStepCount == 2 /*&& nFormCnt*/ )
    {
//      DBG_ASSERT( nFormCnt >= 2, "TxoCont::ReadCont(): Das war nicht abgemacht!" );

        if( pText )
        {
            if( nFormCnt )
                nFormCnt--;

            SvMemoryStream aMemStrm;
            UINT16 nChar, nFont;

            aMemStrm << (UINT16) 0x0001 << (UINT16)(4 * nFormCnt);
            for( UINT16 nIndex = 0; nIndex < nFormCnt; nIndex++ )
            {
                rStrm >> nChar >> nFont;
                rStrm.Ignore( 4 );
                aMemStrm << nChar << nFont;
            }

            XclImpStream aImpStrm( aMemStrm, TRUE );
            aImpStrm.StartNextRecord();
            ShStrTabFormEntry aHelpObj( *pText, aImpStrm, nFormCnt );

            DBG_ASSERT( !pFormText, "TxoCont::ReadCont(): Ich bin doch nicht alleine!!" );

            pFormText = aHelpObj.CreateEditTextObject( rEdEngine, *rRootData.pFontBuffer );
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


void ExcCondForm::Read( XclImpStream& rIn )
{
    rIn >> nNumOfConds;
    rIn.Ignore( 2 );
    rIn >> nRow;
    rIn.Ignore( 2 );
    rIn >> nCol;
    rIn.Ignore( 2 );

    UINT16  nRngCnt, nR1, nR2, nC1, nC2;
    rIn >> nRngCnt;
    while( nRngCnt )
    {
        rIn >> nR1 >> nR2 >> nC1 >> nC2;

        pRangeList->Append( ScRange( nC1, nR1, nTab, nC2, nR2, nTab ) );

        nRngCnt--;
    }
}


void ExcCondForm::ReadCf( XclImpStream& rIn, ExcelToSc& rConv )
{
    if( nNumOfConds )
    {
        nNumOfConds--;

//      const UINT32        nRecPos = rIn.Tell();
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
            ULONG               nFormatsLen = nLenForm1 + nLenForm2 + 6;
            if( nFormatsLen > rIn.GetRecLen() )
                return;

            nFormatsLen = rIn.GetRecLen() - nFormatsLen;    // >0!

            ScDocument*         pDoc = pExcRoot->pDoc;
            String              aStyle( pExcRoot->GetCondFormStyleName( nCondCnt ) );

            const ScTokenArray* pFrmla1 = NULL;
            const ScTokenArray* pFrmla2 = NULL;

            ScAddress           aPos( nCol, nRow, nTab );

            if( !pScCondForm )
            {
                nDummy = 0;
                pScCondForm = new ScConditionalFormat( nDummy, pDoc );
            }

            // create style
            ULONG nPosF = rIn.GetRecPos();      // font
            ULONG nPosL = nPosF;                // line
            ULONG nPosP = nPosF;                // pattern (fill)

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

            if( nPosF )     // font
            {
                UINT8           nAttr1, nAttr2, nAttr3, nUnder;
                UINT16          nBold;
                UINT32          nCol;
                rIn.Seek( nPosF );
                rIn >> nAttr1;          // italic / strike out
                rIn.Ignore( 3 );
                rIn >> nBold;           // boldness
                rIn.Ignore( 2 );
                rIn >> nUnder;          // num of underlines
                rIn.Ignore( 3 );
                rIn >> nCol;            // color
                rIn.Ignore( 4 );
                rIn >> nAttr2;          // strike out DC + italic/bold DC
                rIn.Ignore( 7 );
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
                    rStyleItemSet.Put( *rColBuff.GetColor( (UINT16) nCol ) );
            }

            if( nPosL )     // line
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

            if( nPosP )     // pattern (fill)
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
                rIn.Seek( rIn.GetRecLen() - nLenForm1 - nLenForm2 );

                rConv.Reset( aPos );
                rConv.Convert( pFrmla1, nLenForm1, eFT );
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

                rIn.Seek( rIn.GetRecLen() - nLenForm2 );

                rConv.Reset( aPos );
                rConv.Convert( pFrmla2, nLenForm2, eFT );
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

void XclImpTabIdBuffer::Fill( XclImpStream& rStrm, UINT16 nCount )
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



ImportExcel8::ImportExcel8( SvStorage* pStorage, SvStream& rStream, ScDocument* pDoc, SvStorage* pPivotCache ) :
    ImportExcel( rStream, pDoc ), aEscherObjList( aPosBuffer, pExcRoot )
{
    delete pFormConv;

    pExcRoot->pExtsheetBuffer = new XclImpExternsheetBuffer;
    pExcRoot->pImpTabIdBuffer = new XclImpTabIdBuffer;

    pFormConv = new ExcelToSc8( pExcRoot, aIn, nTab );

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

    pExcRoot->pAddInNameTranslator = new XclAddInNameTranslator;

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
        pLastFormCell->SetString( aIn.ReadUniString( eQuellChar ) );

        pLastFormCell = NULL;
    }
}


void ImportExcel8::Protect( void )
{
    if( aIn.ReaduInt16() )
    {
        uno::Sequence<sal_Int8> aEmptyPass;
        pD->SetTabProtection( nTab, TRUE, aEmptyPass );
    }
}


void ImportExcel8::Verticalpagebreaks( void )
{
    UINT16      n;
    UINT16      nCol;

    aIn >> n;

    while( n )
    {
        aIn >> nCol;
        aIn.Ignore( 4 );        // beide Rows ueberlesen

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
        aIn.Ignore( 4 );        // beide Cols ueberlesen

        aColRowBuff.SetHorizPagebreak( nRow );

        n--;
    }
}


void ImportExcel8::Note( void )
{
    UINT16  nCol, nRow, nId;

    aIn >> nRow >> nCol;
    aIn.Ignore( 2 );
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
    aIn.Ignore( 2 );
    pValueFormBuffer->NewValueFormat( aIn.ReadUniString( eQuellChar ) );
}


void ImportExcel8::Externname( void )
{
    UINT32          nRes;
    UINT16          nOpt;
    UINT8           nLen;

    aIn >> nOpt >> nRes >> nLen;

    String aName( aIn.ReadUniString( eQuellChar, nLen ) );

    if( ( nOpt & 0x0001 ) || ( ( nOpt & 0xFFFE ) == 0x0000 ) )
    {
        // external name
        ScFilterTools::ConvertName( aName );
        pExcRoot->pExtNameBuff->AddName( aName );
    }
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
    aIn.Ignore( 1 );
    aIn >> nIndexCol >> nWeight >> nScript >> nUnderline >> nFamily >> nCharSet;
    aIn.Ignore( 1 );    // Reserved

    aIn >> nLen;

    String aName( aIn.ReadUniString( eQuellChar, nLen ) );

    // Font in Pool batschen
    pExcRoot->pFontBuffer->NewFont(
        nHeight, nAttr0, nScript, nUnderline, nIndexCol, nWeight,
        nFamily, nCharSet, aName );
}


void ImportExcel8::Cont( void )
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
        Msodrawing();
    else if( bLeadingObjRec )
    {
        aIn.PushPosition();
        UINT32      nId;
        aIn >> nId;

        if( ( nId & 0xF000000F ) == 0xF000000F )
        {
            aIn.PopPosition();
            Msodrawing();
        }
        else
            aIn.RejectPosition();

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

    XclImpHelper::DecodeExternsheetUni( aIn, aFileName, aTabName, bSelf );

    if( !aTabName.Len() )
    {
        aTabName = aFileName;
        aFileName.Erase();
    }
    pCurrPivotCache->SetSource( nC1, nR1, nC2, nR2, aFileName, aTabName, bSelf );
}


void ImportExcel8::Obj()
{
    UINT16          nOpcode, nLenRec;
    BOOL            bLoop = TRUE;
    ExcEscherObj*   pObj = NULL;

    aIn.InitializeRecord( FALSE );      // disable internal CONTINUE handling

    while( bLoop && (aIn.GetRecLeft() >= 4) )
    {
        aIn >> nOpcode >> nLenRec;
        aIn.PushPosition();

        switch( nOpcode )
        {
            case 0x00:  bLoop = FALSE;                  break;
            case 0x15:  pObj = ObjFtCmo();              break;
            case 0x08:  ObjFtPioGrbit( pObj );          break;
            case 0x09:  ObjFtPictFmla( pObj, nLenRec ); break;
        }

        aIn.PopPosition();
        // sometimes the last subrecord has an invalid length -> Min()
        aIn.Ignore( Min( (ULONG) nLenRec, aIn.GetRecLeft() ) );
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

    aIn.Ignore( 4 );
    aIn >> nGrbit >> nLen;

    String aName( aIn.ReadUniString( eQuellChar, nLen ) );

    ScFilterTools::ConvertName( aName );
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


void ImportExcel8::Scenman( void )
{
    UINT16              nLastDispl;

    aIn.Ignore( 4 );
    aIn >> nLastDispl;

    aScenList.SetLast( nLastDispl );
}


void ImportExcel8::Scenario( void )
{
    aScenList.Append( new ExcScenario( aIn, *pExcRoot ) );
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

    const BOOL  bCellXF = ( nW8 & 0x0004 ) == 0;

    pXFD->SetCellXF( bCellXF );

    if( bCellXF || HASATTRSET( 0x0800 ) )
        pXFD->SetFont( nW4 );
    if( bCellXF || HASATTRSET( 0x0400 ) )
        pXFD->SetValueFormat( pValueFormBuffer->GetValueFormat( nW6 ) );
    if( bCellXF || HASATTRSET( 0x8000 ) )
    {
        pXFD->SetLocked( TRUEBOOL( nW8 & EXC_XF_LOCKED ) );
        pXFD->SetHidden( TRUEBOOL( nW8 & EXC_XF_HIDDEN ) );
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


void ImportExcel8::Cellmerging( void )
{
    UINT16  n, nR1, nR2, nC1, nC2;

    aIn >> n;

    DBG_ASSERT( aIn.GetRecLeft() >= (ULONG)(n * 8), "*ImportExcel8::Cellmerging(): in die Hose!" );

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

        pFltTab->AppendMerge( nC1, nR1, nC2, nR2 );
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


static sal_Bool lcl_ImportBackgroundGraphic( XclImpStream& rIn, Graphic& rGraphic )
{
    sal_Bool    bRetValue = FALSE;
    sal_uInt32  nSize = (sal_uInt32) rIn.GetRecLen();

    if( nSize > sizeof( BackgroundGraphic ) )
    {
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
                sal_uInt32          x, y, ys;
                ys = nHeight - 1;
                for( y = 0 ; y < nHeight ; y++, ys-- )
                {
                    for( x = 0 ; x < nWidth ; x++ )
                    {
                        rIn >> nBlue >> nGreen >> nRed;

                        pAcc->SetPixel( ys, x, BitmapColor( nRed, nGreen, nBlue ) );

                    }

                    if( bAlignment )
                        rIn.Ignore( 1 );
                }

                aBmp.ReleaseAccess( pAcc );
                rGraphic = aBmp;
                bRetValue = TRUE;
            }
        }
    }
    return bRetValue;
}


void ImportExcel8::BGPic( void )
{
    // no documentation available, but it might be, that it's only wmf format
    DBG_ASSERT( pStyleSheetItemSet, "-ImportExcel::BGPic(): f... no style sheet!" );

    Graphic             aGraphic;
    if( lcl_ImportBackgroundGraphic( aIn, aGraphic ) )
        pStyleSheetItemSet->Put( SvxBrushItem( aGraphic, GPOS_AREA ) );
}


void ImportExcel8::Msodrawinggroup( void )
{
    SvMemoryStream* pMemStrm = new SvMemoryStream;
    aIn.CopyRecordToStream( *pMemStrm );
    const DffRecordHeader* pLatestRecHd = aExcStreamConsumer.Consume( pMemStrm, 0 );
}


void ImportExcel8::Msodrawing( void )
{
    bCond4EscherCont = TRUE;
    aIn.InitializeRecord( FALSE );      // disable internal CONTINUE handling
    ULONG nL = aIn.GetRecLen();

    if ( !aExcStreamConsumer.GetStream() )
        return;

    aIn.PushPosition();

    if( bTabStartDummy )
    {// Dummy fuer ungueltigen ersten Shape einfuegen
        aEscherObjList.Append( new ExcEscherObj( 0, 0, nTab, pExcRoot ) );

        bTabStartDummy = FALSE;
    }
    const ULONG nS = aExcStreamConsumer.GetStream()->Tell();
    if( nL )
    {
        SvMemoryStream* pMemStrm = new SvMemoryStream;
        aIn.CopyRecordToStream( *pMemStrm );
        const DffRecordHeader* pLatestRecHd = aExcStreamConsumer.Consume( pMemStrm, 0 );
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

                aIn.PopPosition();
                aIn.Ignore( 2 );

                aIn >> nFBT;

                if( nFBT != 0xF00D )
                    bMaybeTxo = FALSE;  // != Client Text Box
            }
            else
                bMaybeTxo = FALSE;
        }
    }
}


void ImportExcel8::Msodrawingselection( void )
{
}


void ImportExcel8::Sst( void )
{
    aIn.Ignore( 8 );
    ShStrTabEntry* pEntry;

    while( aIn.GetRecLeft() )
    {
        pEntry = XclImpHelper::CreateUnicodeEntry( aIn, eQuellChar );
        aSharedStringTable.Append( pEntry );
    }
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
        pActCondForm->ReadCf( aIn, *pFormConv );
}


void ImportExcel8::Dval( void )
{
}


void ImportExcel8::Labelsst( void )
{
    UINT16                      nRow, nCol, nXF;
    UINT32                      nSst;

    aIn >> nRow >> nCol >> nXF >> nSst;

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

    if( nRow <= MAXROW && nCol <= MAXCOL )
    {
        ShStrTabEntry*  p = XclImpHelper::CreateUnicodeEntry( aIn, eQuellChar );

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
        pExcRoot->pImpTabIdBuffer->Fill( aIn, (UINT16)(aIn.GetRecLen() >> 1) );
}


void ImportExcel8::Txo( void )
{
    if( pActTxo )
        pActTxo->Clear();
    else
        pActTxo = new TxoCont;

    aIn.InitializeRecord( FALSE );      // disable internal CONTINUE handling

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
        String aName( aIn.ReadUniString( eQuellChar ) );

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

    String      aPromptTitle( aIn.ReadUniString( eQuellChar ) );
    String      aErrorTitle( aIn.ReadUniString( eQuellChar ) );
    String      aPromptMessage( aIn.ReadUniString( eQuellChar ) );
    String      aErrorMessage( aIn.ReadUniString( eQuellChar ) );

    // vals
    if( aIn.GetRecLeft() > 8 )
    {
        aIn.Ignore( aIn.GetRecLeft() - 8 );

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
    UINT16 nRF, nRL, nCF, nCL;
    UINT32 nFlags;

    aIn >> nRF >> nRL >> nCF >> nCL;
    aIn.Ignore( 20 );
    aIn >> nFlags;

    UINT16  nLevel = 0;             // counter for level to climb down in path
    String* pLongname = NULL;       // link / file name
    String* pShortname = NULL;      // 8.3-representation from file name
    String* pTextmark = NULL;       // text mark
    UINT32  nStrLen = 0;

    // description (ignore)
    if( nFlags & EXC_HLINK_DESCR )
    {
        aIn >> nStrLen;
        aIn.IgnoreRawUniString( (UINT16) nStrLen, TRUE );
    }

    // network path
    if( nFlags & EXC_HLINK_NET )
    {
        aIn >> nStrLen;
        pLongname = new String( aIn.ReadRawUniString( eQuellChar, (UINT16) nStrLen, TRUE ) );
        lcl_GetAbs( *pLongname, 0, pD->GetDocumentShell() );
    }
    // file link or URL
    else if( nFlags & EXC_HLINK_BODY )
    {
        UINT32 nID;
        aIn >> nID;
        switch( nID )
        {
            case EXC_HLINK_ID_FILE:
            {
                aIn.Ignore( 12 );
                aIn >> nLevel >> nStrLen;
                pShortname = new String( aIn.ReadRawUniString( eQuellChar, (UINT16) nStrLen, FALSE ) );
                aIn.Ignore( 24 );
                aIn >> nStrLen;
                if( nStrLen )
                {
                    aIn >> nStrLen;
                    aIn.Ignore( 2 );
                    pLongname = new String( aIn.ReadRawUniString( eQuellChar, (UINT16)(nStrLen >> 1), TRUE ) );
                    lcl_GetAbs( *pLongname, nLevel, pD->GetDocumentShell() );
                }
                else
                    lcl_GetAbs( *pShortname, nLevel, pD->GetDocumentShell() );
            }
            break;
            case EXC_HLINK_ID_URL:
            {
                aIn.Ignore( 12 );
                aIn >> nStrLen;
                pLongname = new String( aIn.ReadRawUniString( eQuellChar, (UINT16)(nStrLen >> 1), TRUE ) );
            }
            break;
            default:
                DBG_ERROR( "ImportExcel8::HLink - unknown content id" );
        }
    }

    // text mark
    if( nFlags & EXC_HLINK_MARK )
    {
        aIn >> nStrLen;
        pTextmark = new String( aIn.ReadRawUniString( eQuellChar, (UINT16) nStrLen, TRUE ) );
    }

    DBG_ASSERT( !aIn.GetRecLeft(), "ImportExcel8::HLink - record size mismatch" );

    if( !pLongname && pShortname )
    {
        pLongname = pShortname;
        pShortname = NULL;
    }

    if( pLongname )
    {
        if( pTextmark )
        {
            *pLongname += '#';
            *pLongname += *pTextmark;
        }

        for( UINT16 nCol = nCF ; nCol <= nCL ; nCol++ )
            for( UINT16 nRow = nRF ; nRow <= nRL ; nRow++ )
                InsertHyperlink( nCol, nRow, *pLongname );
    }

    if( pLongname )
        delete pLongname;
    if( pShortname )
        delete pShortname;
    if( pTextmark )
        delete pTextmark;
}


void ImportExcel8::Dimensions( void )
{
    UINT32  nRowFirst, nRowLast;
    UINT16  nColFirst, nColLast;

    aIn >> nRowFirst >> nRowLast >> nColFirst >> nColLast;

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
    aIn.Ignore( 1 );
    aIn >> nLenName >> nLenDef >> nSheet;
    aIn.Ignore( 2 );
    aIn >> nLen;            // length of custom menu text
    nLenSeekRel += nLen;
    aIn >> nLen;            // length of description text
    nLenSeekRel += nLen;
    aIn >> nLen;            // length of help topic text
    nLenSeekRel += nLen;
    aIn >> nLen;            // length of status bar text
    nLenSeekRel += nLen;

    // Namen einlesen
    String              aName( aIn.ReadUniString( eQuellChar, nLenName ) );
    // jetzt steht Lesemarke an der Formel

    sal_Unicode         cFirstChar = aName.GetChar( 0 );

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
        aName.AssignAscii( ScFilterTools::GetBuiltInName( cFirstChar ) );   // built-in name
    else
        ScFilterTools::ConvertName( aName );

    pFormConv->Reset();
    if( nOpt & (EXC_NAME_VB | EXC_NAME_BIG) )
        // function or command?
        pFormConv->GetDummy( pErgebnis );
    else if( bBuiltIn )
    {
        aIn.PushPosition();

        if( bPrintArea )
            pFormConv->Convert( *pPrintRanges, nLenDef, FT_RangeName );
        else if( bPrintTitles )
            pFormConv->Convert( *pPrintTitles, nLenDef, FT_RangeName );

        aIn.PopPosition();

        pFormConv->Convert( pErgebnis, nLenDef, FT_RangeName );

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
        pFormConv->Convert( pErgebnis, nLenDef, FT_RangeName );     // formula

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

    if( !( nXf & 0x8000 ) )
    {
        nXf &= 0x0FFF;  // only bit 0...11 is used for XF-index

        pExcRoot->pXF_Buffer->SetStyle( nXf, aIn.ReadUniString( eQuellChar ) );
    }
}


void ImportExcel8::GetHFString( String& rStr )
{
    aIn.AppendUniString( rStr, eQuellChar );
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

        const String            aStrName( _STRING( "Ctls" ) );
        SvStorage&              rStrg = *pExcRoot->pRootStorage;
        const BOOL              bHasCtrls = rStrg.IsContained( aStrName ) && rStrg.IsStream( aStrName );
        ScMSConvertControls*    pCtrlConv;
        SvStorageStreamRef      xStStream;

        if( bHasCtrls )
        {
            pCtrlConv = new ScMSConvertControls( pD->GetDocumentShell() );
            xStStream = pExcRoot->pRootStorage->OpenStream( aStrName, STREAM_READ | STREAM_SHARE_DENYALL );
        }

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
                if( pFltOpts->IsMathType2Math() )
                    nOLEImpFlags |= OLE_MATHTYPE_2_STARMATH;

                if( pFltOpts->IsWinWord2Writer() )
                    nOLEImpFlags |= OLE_WINWORD_2_STARWRITER;

                if( pFltOpts->IsPowerPoint2Impress() )
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
                                    case OT_CHART:
                                        nChartCnt++;
                                    break;
                                    case OT_OLE:
                                        ((ExcEscherOle*)p)->CreateSdrOle( *pDffMan, nOLEImpFlags );
                                    break;
                                    case OT_CTRL:
                                        if( bHasCtrls )
                                        {
                                            ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
                                                    xShapeRef = GetXShapeForSdrObject( pSdrObj );
                                            if( pCtrlConv->ReadOCXExcelKludgeStream( xStStream, &xShapeRef, TRUE ) )
                                            {
                                                SdrObject*  pNewObj = GetSdrObjectFromXShape( xShapeRef );
                                                if( pNewObj )
                                                    p->SetObj( pNewObj );
                                            }
                                        }
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

        if( bHasCtrls )
            delete pCtrlConv;

        delete pDffMan;
    }

    aEscherObjList.Apply();

    // controls
/*
    ScMSConvertControls     aCtrlConverter( pD->GetDocumentShell() );
    String                  aStrName( String::CreateFromAscii( "Ctls" ) );
    com::sun::star::uno::Reference< com::sun::star::drawing::XShape >*  pShapeRef = NULL;

    SvStorageStreamRef      xStStream = pExcRoot->pRootStorage->OpenStream(
                                aStrName, STREAM_READ | STREAM_SHARE_DENYALL );
    aCtrlConverter.ReadOCXExcelKludgeStream( xStStream, pShapeRef, TRUE );
                                                                // BOOL bFloatingCtrl
*/
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



//___________________________________________________________________
// 3D references

void ImportExcel8::Supbook( void )
{
    pExcRoot->pExtsheetBuffer->AppendSupbook( new XclImpSupbook( aIn, *pExcRoot ) );
}

void ImportExcel8::Xct( void )
{
    XclImpSupbook* pSupbook = pExcRoot->pExtsheetBuffer->GetCurrSupbook();
    if( pSupbook )
    {
        UINT16  nCrnCount;
        UINT16  nTabNum = 0;

        aIn >> nCrnCount;
        if( aIn.GetRecLeft() > 3 )
            aIn >> nTabNum;

        pSupbook->SetCurrScTab( nTabNum );
    }
}

void ImportExcel8::Crn( void )
{
    XclImpSupbook* pSupbook = pExcRoot->pExtsheetBuffer->GetCurrSupbook();
    if( pSupbook )
    {
        if( pSupbook->HasValidScTab() )
        {
            UINT8       nLastCol;
            UINT8       nFirstCol;
            UINT16      nRow;
            UINT16      nTab = pSupbook->GetCurrScTab();
            UINT8       nValType;

            aIn >> nLastCol >> nFirstCol >> nRow;

            ScAddress   aAddr( (UINT16) 0, nRow, nTab );

            for( UINT16 iCol = nFirstCol; (iCol <= nLastCol) && (aIn.GetRecLeft() > 1); iCol++ )
            {
                aAddr.SetCol( iCol );
                aIn >> nValType;
                switch( nValType )
                {
                    case EXC_CRN_DOUBLE:
                    {
                        double fVal;
                        aIn >> fVal;
                        if( aIn.IsValid() )
                            pD->SetValue( iCol, nRow, nTab, fVal );
                    }
                    break;
                    case EXC_CRN_STRING:
                    {
                        String sText( aIn.ReadUniString( eQuellChar ) );
                        if( aIn.IsValid() )
                        {
                            ScStringCell* pStrCell = new ScStringCell( sText );
                            pD->PutCell( aAddr, pStrCell );
                        }
                    }
                    break;
                    case EXC_CRN_BOOL:
                    case EXC_CRN_ERROR:
                    {
                        BOOL    bIsErr = (nValType == EXC_CRN_ERROR);
                        UINT16  nErrBool;
                        double  fVal;

                        aIn >> nErrBool;
                        aIn.Ignore( 6 );

                        if( aIn.IsValid() )
                        {
                            const ScTokenArray* pTok    = ErrorToFormula( bIsErr, (UINT8)nErrBool, fVal );
                            ScFormulaCell*      pCell   = new ScFormulaCell( pD, aAddr, pTok );

                            pCell->SetDouble( fVal );
                            pD->PutCell( aAddr, pCell );
                        }
                    }
                    break;
                }
            }
        }
    }
}

void ImportExcel8::Externsheet( void )
{
    pExcRoot->pExtsheetBuffer->Read( aIn );
    pExcRoot->pExtsheetBuffer->CreateTables( *pExcRoot );
}



XclImpSupbook::XclImpSupbook( XclImpStream& rIn, RootData& rExcRoot )
{
    UINT16 nTabCnt;
    rIn >> nTabCnt;

    bSelf = (rIn.GetRecLeft() < (ULONG)(2 + 2 * nTabCnt));
    if( bSelf ) return;

    String aTabName;
    XclImpSupbookTab* pNewTab;

    ReadDocName( rIn, aFileName, bSelf );

    if( nTabCnt )
    {
        for( UINT16 nTab = 0; nTab < nTabCnt; nTab++ )
        {
            pNewTab = new XclImpSupbookTab;
            ReadTabName( rIn, rExcRoot, pNewTab->aName );
            List::Insert( pNewTab, LIST_APPEND );
        }
    }
    else
    {
        // create dummy list entry
        pNewTab = new XclImpSupbookTab;
        pNewTab->aName = aFileName;
        List::Insert( pNewTab, LIST_APPEND );
    }
}

XclImpSupbook::~XclImpSupbook()
{
    for( XclImpSupbookTab* pTab = (XclImpSupbookTab*) List::First(); pTab; pTab = (XclImpSupbookTab*) List::Next() )
        delete pTab;
}

//static
void XclImpSupbook::ReadDocName( XclImpStream& rStrm, String& rDocName, BOOL& rSelf )
{
    String sTabName;
    XclImpHelper::DecodeExternsheetUni( rStrm, rDocName, sTabName, rSelf );
}

//static
void XclImpSupbook::ReadTabName( XclImpStream& rStrm, RootData& rExcRoot, String& rTabName )
{
    rStrm.AppendUniString( rTabName, *rExcRoot.pCharset );
    ScFilterTools::ConvertName( rTabName );
}

UINT16 XclImpSupbook::GetScTabNum( UINT16 nTab ) const
{
    if( bSelf )
        return nTab;
    const XclImpSupbookTab* pTab = Get( nTab );
    return pTab ? pTab->nScNum : EXC_TAB_INVALID;
}

UINT16 XclImpSupbook::GetScTabNum( const String& rTabName ) const
{
    for( UINT32 nIndex = 0; nIndex < List::Count(); nIndex++ )
    {
        const XclImpSupbookTab* pTab = Get( nIndex );
        if( pTab && (pTab->aName == rTabName) )
            return pTab->nScNum;
    }
    return EXC_TAB_INVALID;
}

void XclImpSupbook::CreateTables( RootData& rRootData, UINT16 nFirst, UINT16 nLast ) const
{
    if( bSelf || (rRootData.pExtDocOpt->nLinkCnt >= 1) )
        return;

    String aURL( ScGlobal::GetAbsDocName( aFileName, rRootData.pDoc->GetDocumentShell() ) );

    for( UINT16 nTab = nFirst; nTab <= nLast; nTab++ )
    {
        XclImpSupbookTab* pSBTab = Get( nTab );
        if( pSBTab )
        {
            UINT16 nNewTabNum;
            String aTabName( ScGlobal::GetDocTabName( aURL, pSBTab->aName ) );

            if( rRootData.pDoc->LinkEmptyTab( nNewTabNum, aTabName, aURL, pSBTab->aName ) )
                pSBTab->nScNum = nNewTabNum;
        }
    }
}



XclImpSupbookBuffer::~XclImpSupbookBuffer()
{
    for( XclImpSupbook* pSupbook = _First(); pSupbook; pSupbook = _Next() )
        delete pSupbook;
}

const XclImpSupbook* XclImpSupbookBuffer::Get( const String& rDocName ) const
{
    for( UINT32 nIndex = 0; nIndex < List::Count(); nIndex++ )
    {
        const XclImpSupbook* pSupbook = Get( nIndex );
        if( pSupbook && (pSupbook->GetName() == rDocName) )
            return pSupbook;
    }
    return NULL;
}



XclImpExternsheetBuffer::~XclImpExternsheetBuffer()
{
    for( XclImpXti* pXti = _First(); pXti; pXti = _Next() )
        delete pXti;
}

BOOL XclImpExternsheetBuffer::FindNextTabRange( UINT16 nSupb, UINT16 nStart, UINT16& rnFirst, UINT16& rnLast )
{
    rnFirst = rnLast = 0xFFFF;
    for( const XclImpXti* pXti = _First(); pXti; pXti = _Next() )
    {
        if( (nSupb == pXti->nSupbook) && (nStart <= pXti->nLast) && (pXti->nFirst < rnFirst) )
        {
            rnFirst = Max( nStart, pXti->nFirst );
            rnLast = pXti->nLast;
        }
    }
    return (rnFirst < 0xFFFF);
}

void XclImpExternsheetBuffer::Read( XclImpStream& rIn )
{
    UINT16 nXtiCount;
    rIn >> nXtiCount;

    XclImpXti* pXti;
    while( nXtiCount )
    {
        pXti = new XclImpXti;
        rIn >> pXti->nSupbook >> pXti->nFirst >> pXti->nLast;
        List::Insert( pXti, LIST_APPEND );
        nXtiCount--;
    }
}

void XclImpExternsheetBuffer::CreateTables( RootData& rRootData )
{
    DBG_ASSERT( !bCreated, "XclImpExternsheetBuffer::CreateTables - multiple call!!" );
    if( bCreated ) return;

    UINT16 nFirst, nLast;

    for( UINT16 nSupbook = 0; nSupbook < aSupbookBuffer.Count(); nSupbook++ )
    {
        const XclImpSupbook* pSupbook = aSupbookBuffer.Get( nSupbook );
        BOOL bLoop = FindNextTabRange( nSupbook, 0, nFirst, nLast );
        while( bLoop && pSupbook )
        {
            pSupbook->CreateTables( rRootData, nFirst, nLast );
            bLoop = FindNextTabRange( nSupbook, nLast + 1, nFirst, nLast );
        }
    }

    bCreated = TRUE;
}

const XclImpSupbook* XclImpExternsheetBuffer::GetSupbook( ULONG nXtiIndex ) const
{
    const XclImpXti* pXti = GetXti( nXtiIndex );
    return pXti ? aSupbookBuffer.Get( pXti->nSupbook ) : NULL;
}



//___________________________________________________________________
// pivot tables

void ImportExcel8::SXView( void )
{
    pCurrPivTab = new XclImpPivotTable( aIn, pExcRoot, (UINT8) nTab );
    aPivotTabList.Append( pCurrPivTab );
}

void ImportExcel8::SXVd( void )
{
    if( !pCurrPivTab )
        return;
    pCurrPivTab->AddViewField( aIn );
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
    pCurrPivTab->ReadRCFieldIDs( aIn );
}

void ImportExcel8::SXLi( void )
{
}   // unnecessary to read this record

void ImportExcel8::SXPi( void )
{
    if( !pCurrPivTab )
        return;

    UINT16  nArrayCnt = (UINT16)(aIn.GetRecLen() / 6);      // SXPI contains 6-byte-arrays
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
    pCurrPivTab->AddDataItem( aIn );
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

void ImportExcel8::SXVs( void )
{
    if( !pCurrPivotCache )
        return;

    UINT16 nSrcType;
    aIn >> nSrcType;
    pCurrPivotCache->SetSourceType( nSrcType );
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

void ImportExcel8::SXVdex( void )
{
    if( !pCurrPivTab )
        return;

    UINT32 nFlags;
    aIn >> nFlags;
    pCurrPivTab->SetShowEmpty( TRUEBOOL( nFlags & 0x00000001 ) );
}



//___________________________________________________________________
// autofilter

void ImportExcel8::FilterMode( void )
{
}

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
        pData->ReadAutoFilter( aIn );
}



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
        ScGlobal::pLocaleData->getNumDecimalSep().GetChar(0), TRUE );
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

void AutoFilterData::ReadAutoFilter( XclImpStream& rStrm )
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

            rStrm.Ignore( 20 );
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
                        rStrm.Ignore( 4 );
                        CreateFromDouble( *aEntry.pStr, XclImpHelper::GetDoubleFromRK( nRK ) );
                    break;
                    case EXC_AFTYPE_DOUBLE:
                        rStrm >> fVal;
                        CreateFromDouble( *aEntry.pStr, fVal );
                    break;
                    case EXC_AFTYPE_STRING:
                        rStrm.Ignore( 4 );
                        rStrm >> nStrLen[ nE ];
                        rStrm.Ignore( 3 );
                        aEntry.pStr->Erase();
                    break;
                    case EXC_AFTYPE_BOOLERR:
                        rStrm >> nBoolErr >> nVal;
                        rStrm.Ignore( 6 );
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
                        rStrm.Ignore( 8 );
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
                rStrm.Ignore( 10 );
        }

        for( nE = 0; nE < 2; nE++ )
            if( nStrLen[ nE ] && pEntryStr[ nE ] )
                pEntryStr[ nE ]->Assign( rStrm.ReadUniString( *pExcRoot->pCharset, nStrLen[ nE ] ) );
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

