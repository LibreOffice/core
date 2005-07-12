/*************************************************************************
 *
 *  $RCSfile: xiescher.cxx,v $
 *
 *  $Revision: 1.36 $
 *
 *  last change: $Author: kz $ $Date: 2005-07-12 12:22:35 $
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

#ifndef SC_XIESCHER_HXX
#include "xiescher.hxx"
#endif

#ifndef _COM_SUN_STAR_EMBED_ASPECTS_HPP_
#include <com/sun/star/embed/Aspects.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XEMBEDPERSIST_HPP_
#include <com/sun/star/embed/XEmbedPersist.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_VISUALEFFECT_HPP_
#include <com/sun/star/awt/VisualEffect.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_SCROLLBARORIENTATION_HPP_
#include <com/sun/star/awt/ScrollBarOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_VERTICALALIGNMENT_HPP_
#include <com/sun/star/style/VerticalAlignment.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart/XChartDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_SCRIPTEVENTDESCRIPTOR_HPP_
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#endif

#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif
#ifndef _SFX_OBJSH_HXX
#include <sfx2/objsh.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <svtools/moduleoptions.hxx>
#endif
#ifndef _SVT_FLTRCFG_HXX
#include <svtools/fltrcfg.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _COMPHELPER_CLASSIDS_HXX
#include <comphelper/classids.hxx>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif

#ifndef _SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif
#ifndef _SVDOGRP_HXX
#include <svx/svdogrp.hxx>
#endif
#ifndef _SVDOOLE2_HXX
#include <svx/svdoole2.hxx>
#endif
#ifndef _SVDPAGE_HXX
#include <svx/svdpage.hxx>
#endif
#ifndef _EDITOBJ_HXX
#include <svx/editobj.hxx>
#endif
#ifndef _OUTLINER_HXX
#include <svx/outliner.hxx>
#endif
#ifndef _OUTLOBJ_HXX
#include <svx/outlobj.hxx>
#endif
#ifndef _SVDOUNO_HXX
#include <svx/svdouno.hxx>
#endif
#ifndef _SVX_UNOAPI_HXX_
#include <svx/unoapi.hxx>
#endif
#ifndef _SVDITER_HXX
#include <svx/svditer.hxx>
#endif
#ifndef _SVX_WRITINGMODEITEM_HXX
#include <svx/writingmodeitem.hxx>
#endif
#ifndef _SVDOEDGE_HXX
#include <svx/svdoedge.hxx>
#endif

#ifndef _SCH_DLL_HXX
#include <sch/schdll.hxx>
#endif
#ifndef _SCH_MEMCHRT_HXX
#include <sch/memchrt.hxx>
#endif

#ifndef SC_ITEMS_HXX
#include "scitems.hxx"
#endif
#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD
#ifndef _SVX_COLRITEM_HXX
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_XFLCLIT_HXX
#include <svx/xflclit.hxx>
#endif
#ifndef _SVX_ADJITEM_HX
#include <svx/adjitem.hxx>
#endif

#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_DRWLAYER_HXX
#include "drwlayer.hxx"
#endif
#ifndef SC_CHARTARR_HXX
#include "chartarr.hxx"
#endif
#ifndef SC_DETFUNC_HXX
#include "detfunc.hxx"
#endif
#ifndef __GLOBSTR_HRC_
#include "globstr.hrc"
#endif

#ifndef SC_FPROGRESSBAR_HXX
#include "fprogressbar.hxx"
#endif
#ifndef SC_XLOCX_HXX
#include "xlocx.hxx"
#endif
#ifndef SC_XLTRACER_HXX
#include "xltracer.hxx"
#endif
#ifndef SC_XILINK_HXX
#include "xilink.hxx"
#endif
#ifndef SC_XISTYLE_HXX
#include "xistyle.hxx"
#endif
#ifndef SC_XIPAGE_HXX
#include "xipage.hxx"
#endif
#ifndef SC_XICHART_HXX
#include "xichart.hxx"
#endif

#include "excform.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::embed::XEmbeddedObject;
using ::com::sun::star::embed::XEmbedPersist;
using ::com::sun::star::chart::XChartDocument;
using ::com::sun::star::script::ScriptEventDescriptor;

// Escher stream consumer =====================================================

XclImpStreamConsumer::XclImpStreamConsumer() :
    mpNode( 0 ),
    mnBytesLeft( 0 )
{
}

XclImpStreamConsumer::~XclImpStreamConsumer()
{
    while( mpNode )
        RemoveNode();
}

void XclImpStreamConsumer::UpdateNode( const DffRecordHeader& rHd )
{
    while( mpNode && ((mpNode->mnPos + mpNode->mnSize) <= rHd.nFilePos) )
        RemoveNode();
    XclImpStreamNode* pTemp = mpNode;
    mpNode = new XclImpStreamNode;
    mpNode->mnPos = rHd.nFilePos;
    mpNode->mnSize = rHd.nRecLen + 8;
    mpNode->mpPrev = pTemp;
}

void XclImpStreamConsumer::RemoveNode()
{
    XclImpStreamNode* pTemp = mpNode;
    mpNode = mpNode->mpPrev;
    delete pTemp;
}

const DffRecordHeader* XclImpStreamConsumer::ConsumeRecord( XclImpStream& rSrcStrm )
{
    ULONG nEntry = maStrm.Tell();
    ULONG nSrcSize = rSrcStrm.GetRecSize();
    if( !nSrcSize )
        return 0;

    rSrcStrm.Seek( EXC_REC_SEEK_TO_BEGIN );
    sal_Char* pBuf = new sal_Char[ nSrcSize ];
    rSrcStrm.Read( pBuf, nSrcSize );
    maStrm.Write( pBuf, nSrcSize );
    delete[] pBuf;

    ULONG nPos = maStrm.Tell();
    maStrm.Seek( nEntry );
    if( mnBytesLeft )
    {
        if( nSrcSize < mnBytesLeft )
        {
            maStrm.SeekRel( nSrcSize );
            mnBytesLeft -= nSrcSize;
        }
        else
        {
            maStrm.SeekRel( mnBytesLeft );
            mnBytesLeft = 0;
        }
    }
    while( maStrm.Tell() < nPos )
    {
        maStrm >> maHd;
        if( maHd.nRecType == DFF_msofbtSolverContainer )
        {
            maStrm.Seek( nEntry );
            return &maHd;
        }
        else if( maHd.IsContainer() )
            UpdateNode( maHd );
        else if( (maStrm.Tell() + maHd.nRecLen) <= nPos )
            maStrm.SeekRel( maHd.nRecLen );
        else
        {
            mnBytesLeft = maStrm.Tell() + maHd.nRecLen - nPos;
            maStrm.Seek( nPos );
        }
    }
    maStrm.Seek( nPos );

    return mnBytesLeft ? 0 : &maHd;
}

bool XclImpStreamConsumer::AppendData( sal_Char* pBuf, ULONG nLen )
{
    if ( (maHd.nRecType != 0) && !maHd.IsContainer() && (mnBytesLeft == 0) )
    {
        while( mpNode && ((mpNode->mnPos + mpNode->mnSize) <= maHd.nFilePos) )
            RemoveNode();
        XclImpStreamNode* pTemp = mpNode;
        while( pTemp )
        {
            pTemp->mnSize += nLen;               // updating container sizes
            maStrm.Seek( pTemp->mnPos + 4 );
            maStrm << pTemp->mnSize - 8;
            pTemp = pTemp->mpPrev;
        }
        maHd.nRecLen += nLen;
        maStrm.Seek( maHd.nFilePos + 4 );        // updating atom size
        maStrm << maHd.nRecLen;
        maStrm.Seek( STREAM_SEEK_TO_END );
        maStrm.Write( pBuf, nLen );
        return true;
    }
    return false;
}

// Escher objects =============================================================

TYPEINIT0( XclImpEscherObj );

XclImpEscherObj::XclImpEscherObj( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot ),
    mnStrmBegin( 0 ),
    mnStrmEnd( 0 ),
    mnScTab( rRoot.GetCurrScTab() ),
    mnObjId( EXC_OBJ_INVALID_ID ),
    mbAreaObj( false ),
    mbSkip( false ),
    mbPrintable( true )
{
}

XclImpEscherObj::XclImpEscherObj( const XclImpRoot& rRoot, ULONG nStrmBegin, ULONG nStrmEnd ) :
    XclImpRoot( rRoot ),
    mnStrmBegin( nStrmBegin ),
    mnStrmEnd( nStrmEnd ),
    mnScTab( rRoot.GetCurrScTab() ),
    mnObjId( EXC_OBJ_INVALID_ID ),
    mbAreaObj( false ),
    mbSkip( false ),
    mbPrintable( true )
{
}

XclImpEscherObj::XclImpEscherObj( XclImpEscherObj& rSrcObj ) :
    XclImpRoot( rSrcObj.GetRoot() ),
    maAnchorRect( rSrcObj.maAnchorRect ),
    mxSdrObj( rSrcObj.mxSdrObj ),          // rSrc.mxSdrObj releases the object
    mnStrmBegin( rSrcObj.mnStrmBegin ),
    mnStrmEnd( rSrcObj.mnStrmEnd ),
    mnScTab( rSrcObj.mnScTab ),
    mnObjId( rSrcObj.mnObjId ),
    mbAreaObj( rSrcObj.mbAreaObj ),
    mbSkip( rSrcObj.mbSkip ),
    mbPrintable( rSrcObj.mbPrintable )
{
}

XclImpEscherObj::~XclImpEscherObj()
{
}

bool XclImpEscherObj::IsValidSize( const Rectangle& rRect ) const
{
    // XclEscherAnchor rounds up the width, width of 3 is the result of an Excel width of 0
    return mbAreaObj ?
        ((rRect.GetWidth() > 3) && (rRect.GetHeight() > 1)) :
        ((rRect.GetWidth() > 3) || (rRect.GetHeight() > 1));
}

bool XclImpEscherObj::IsValidSize() const
{
    return IsValidSize( maAnchorRect );
}

bool XclImpEscherObj::IsValid() const
{
    return !mbSkip && mxSdrObj.get() && IsValidSize();
}

void XclImpEscherObj::SetSdrObj( SdrObject* pSdrObj )
{
    mxSdrObj.reset( pSdrObj );
}

sal_uInt32 XclImpEscherObj::GetProgressSize() const
{
    return 1;
}

bool XclImpEscherObj::Apply( ScfProgressBar& rProgress )
{
    bool bInserted = false;
    if( IsValid() )
    {
        if( SdrPage* pPage = GetSdrPage( mnScTab ) )
        {
            pPage->InsertObject( mxSdrObj.release() );
            bInserted = true;
        }
        // Trace if object is not printable.
        if( !GetPrintable() && !ISA( XclImpEscherTbxCtrl ) )
        {
            bool bOcxCtrl = false;
            if( XclImpEscherOle* pOleObj = PTR_CAST( XclImpEscherOle, this ) )
                bOcxCtrl = pOleObj->IsControl();
            if( !bOcxCtrl )
                GetTracer().TraceObjectNotPrintable();
        }
    }
    rProgress.Progress();
    return bInserted;
}

// ----------------------------------------------------------------------------

TYPEINIT1( XclImpEscherDrawing, XclImpEscherObj );

XclImpEscherDrawing::XclImpEscherDrawing( XclImpEscherObj& rSrcObj, bool bAreaObj ) :
    XclImpEscherObj( rSrcObj )
{
    SetAreaObj( bAreaObj );
}

bool XclImpEscherDrawing::Apply( ScfProgressBar& rProgress )
{
    // #119010# - allow for the possibility that valid connectors
    // may have no height or width set.
    if( !GetIsSkip() && GetSdrObj() && GetSdrObj()->ISA(SdrEdgeObj) && !IsValidSize() )
        SetAreaObj( false );
    return XclImpEscherObj::Apply( rProgress );
}

// ----------------------------------------------------------------------------

TYPEINIT1( XclImpEscherTxo, XclImpEscherDrawing );

XclImpEscherTxo::XclImpEscherTxo( XclImpEscherObj& rSrcObj ) :
    XclImpEscherDrawing( rSrcObj, true ),
    meHorAlign( xlTxoHAlign_Default ),
    meVerAlign( xlTxoVAlign_Default ),
    meRotation( xlTxoRot_Default )
{
}

void XclImpEscherTxo::SetAlignment( sal_uInt16 nAlign )
{
    ::extract_value( meHorAlign, nAlign, 1, 3 );
    ::extract_value( meVerAlign, nAlign, 4, 3 );
}

void XclImpEscherTxo::SetRotation( sal_uInt16 nOrient )
{
    meRotation = static_cast<XclTxoRotation>(nOrient);
}

void XclImpEscherTxo::ApplyTextOnSdrObj( SdrObject& rSdrObj ) const
{
    if( SdrTextObj* pTextObj = PTR_CAST( SdrTextObj, &rSdrObj ) )
    {
        if( mxString.get() )
        {
            if( mxString->IsRich() )
            {
                // rich text
                ::std::auto_ptr< EditTextObject > pEditObj(
                XclImpStringHelper::CreateTextObject( GetRoot(), *mxString ) );
                OutlinerParaObject* pOPO = new OutlinerParaObject( *pEditObj );
                pOPO->SetOutlinerMode( OUTLINERMODE_TEXTOBJECT );
                pTextObj->NbcSetOutlinerParaObject( pOPO );
            }
            else
            {
                // plain text
                pTextObj->SetText( mxString->GetText() );
            }
        }

        // horizontal text alignment (#i12188# not stored in Escher stream, but in TXO)
        SvxAdjust eEEHorAlign = SVX_ADJUST_LEFT;
        switch( meHorAlign )
        {
            case xlTxoHAlignLeft:      eEEHorAlign = SVX_ADJUST_LEFT;     break;
            case xlTxoHAlignCenter:    eEEHorAlign = SVX_ADJUST_CENTER;   break;
            case xlTxoHAlignRight:     eEEHorAlign = SVX_ADJUST_RIGHT;    break;
            case xlTxoHAlignJustify:   eEEHorAlign = SVX_ADJUST_BLOCK;    break;
        }
        pTextObj->SetMergedItem( SvxAdjustItem( eEEHorAlign, EE_PARA_JUST ) );

        // vertical text alignment (#i12188# not stored in Escher stream, but in TXO)
        SdrTextVertAdjust eSdrVerAlign = SDRTEXTVERTADJUST_TOP;
        switch( meVerAlign )
        {
            case xlTxoVAlignTop:        eSdrVerAlign = SDRTEXTVERTADJUST_TOP;       break;
            case xlTxoHAlignCenter:     eSdrVerAlign = SDRTEXTVERTADJUST_CENTER;    break;
            case xlTxoVAlignBottom:     eSdrVerAlign = SDRTEXTVERTADJUST_BOTTOM;    break;
            case xlTxoVAlignJustify:    eSdrVerAlign = SDRTEXTVERTADJUST_BLOCK;     break;
        }
        pTextObj->SetMergedItem( SdrTextVertAdjustItem( eSdrVerAlign ) );

    }
}

// ----------------------------------------------------------------------------

TYPEINIT1( XclImpEscherNote, XclImpEscherTxo );

XclImpEscherNote::XclImpEscherNote( XclImpEscherObj& rSrcObj ) :
    XclImpEscherTxo( rSrcObj ),
    maScPos( ScAddress::UNINITIALIZED )
{
}

bool XclImpEscherNote::Apply( ScfProgressBar& rProgress )
{
    if( IsValid() && maScPos.IsValid() )
    {
        ScPostIt aNote( GetDocPtr() );
        if( GetDoc().GetNote( maScPos.Col(), maScPos.Row(), mnScTab, aNote ) )
        {
            aNote.SetRectangle( maAnchorRect );
            SdrObject* pObj = mxSdrObj.get();

            // get the actual container from this group object.
            if( pObj->IsGroupObject() )
            {
                SdrObjListIter aIter( *pObj->GetSubList() );
                pObj = aIter.Next();
            }
            if( pObj )
            {
                namespace csst = ::com::sun::star::text;
                csst::WritingMode eWriteMode = csst::WritingMode_LR_TB;
                switch( GetRotation() )
                {
                    case xlTxoNoRot:        eWriteMode = csst::WritingMode_LR_TB;   break;
                    case xlTxoRotStacked:   eWriteMode = csst::WritingMode_TB_RL;   break;
                    case xlTxoRot90ccw:     eWriteMode = csst::WritingMode_TB_RL;   break;
                    case xlTxoRot90cw:      eWriteMode = csst::WritingMode_TB_RL;   break;
                }
                pObj->SetMergedItem( SvxWritingModeItem( eWriteMode ) );
                pObj->SetMergedItem( SdrTextAutoGrowWidthItem( false ) );
                pObj->SetMergedItem( SdrTextAutoGrowHeightItem( false ) );
                aNote.SetAndApplyItemSet( pObj->GetMergedItemSet() );
            }
            GetDoc().SetNote( maScPos.Col(), maScPos.Row(), mnScTab, aNote );

            if( aNote.IsShown() )
            {
                ScDetectiveFunc aDetFunc( GetDocPtr(), mnScTab );
                aDetFunc.ShowComment( maScPos.Col(), maScPos.Row(), TRUE );
            }
        }
    }
    rProgress.Progress();
    // notes are not inserted into the drawing layer -> return false
    return false;
}

// ----------------------------------------------------------------------------

XclImpCtrlLinkHelper::XclImpCtrlLinkHelper( XclCtrlBindMode eBindMode ) :
    meBindMode( eBindMode )
{
}

void XclImpCtrlLinkHelper::ReadCellLinkFormula( XclImpStream& rStrm )
{
    sal_uInt16 nFmlaSize;
    rStrm >> nFmlaSize;
    rStrm.Ignore( 4 );
    ScRangeList aRangeList;
    if( rStrm.GetRoot().GetFmlaConverter().GetAbsRefs( aRangeList, nFmlaSize ) )
    {
        // Use first cell of first range
        ::std::auto_ptr< ScRange > pRange( aRangeList.Remove( 0UL ) );
        if( pRange.get() )
            mxCellLink.reset( new ScAddress( pRange->aStart ) );
    }
}

void XclImpCtrlLinkHelper::ReadSrcRangeFormula( XclImpStream& rStrm )
{
    sal_uInt16 nFmlaSize;
    rStrm >> nFmlaSize;
    rStrm.Ignore( 4 );
    ScRangeList aRangeList;
    if( rStrm.GetRoot().GetFmlaConverter().GetAbsRefs( aRangeList, nFmlaSize ) )
        // Use first range
        mxSrcRange.reset( aRangeList.Remove( 0UL ) );
}

// ----------------------------------------------------------------------------

TYPEINIT1( XclImpEscherTbxCtrl, XclImpEscherTxo );

XclImpEscherTbxCtrl::XclImpEscherTbxCtrl( XclImpEscherObj& rSrcObj, sal_uInt16 nCtrlType ) :
    XclImpEscherTxo( rSrcObj ),
    XclImpCtrlLinkHelper( xlBindPosition ),
    mnCtrlType( nCtrlType ),
    mnState( EXC_OBJ_CBLS_STATE_UNCHECK ),
    mnSelEntry( 0 ),
    mnSelType( EXC_OBJ_LBS_SEL_SIMPLE ),
    mnLineCount( 0 ),
    mnScrollValue( 0 ),
    mnScrollMin( 0 ),
    mnScrollMax( 100 ),
    mnScrollStep( 1 ),
    mnScrollPage( 10 ),
    mbFlatButton( false ),
    mbFlatBorder( false ),
    mbScrollHor( false )
{
}

void XclImpEscherTbxCtrl::ReadCbls( XclImpStream& rStrm )
{
    sal_uInt16 nStyle;
    rStrm >> mnState;
    rStrm.Ignore( 8 );
    rStrm >> nStyle;
    mnState &= EXC_OBJ_CBLS_STATEMASK;
    mbFlatButton = ::get_flag( nStyle, EXC_OBJ_CBLS_FLAT );
}

void XclImpEscherTbxCtrl::ReadCblsFmla( XclImpStream& rStrm )
{
    ReadCellLinkFormula( rStrm );
}

void XclImpEscherTbxCtrl::ReadLbsData( XclImpStream& rStrm )
{
    sal_uInt16 nSubSize;
    rStrm >> nSubSize;
    if( nSubSize )
    {
        // read the address of the data source range
        rStrm.PushPosition();
        ReadSrcRangeFormula( rStrm );
        rStrm.PopPosition();
        rStrm.Ignore( nSubSize );
    }

    // other list box settings
    sal_Int16 nEntryCount;
    sal_uInt16 nStyle, nComboStyle;
    rStrm >> nEntryCount >> mnSelEntry >> nStyle;
    rStrm.Ignore( 2 );  // linked edit field

    mnSelType = nStyle & EXC_OBJ_LBS_SELMASK;
    mbFlatBorder = ::get_flag( nStyle, EXC_OBJ_LBS_FLAT );

    switch( mnCtrlType )
    {
        case EXC_OBJ_CMO_LISTBOX:
            if( mnSelType != EXC_OBJ_LBS_SEL_SIMPLE )
                for( sal_Int16 nEntry = 0; nEntry < nEntryCount; ++nEntry )
                    if( rStrm.ReaduInt8() != 0 )
                        maMultiSel.push_back( nEntry );
        break;
        case EXC_OBJ_CMO_COMBOBOX:
            rStrm >> nComboStyle >> mnLineCount;
            // comboboxes of auto-filters have 'simple' style, they don't have an edit area
            if( (nComboStyle & EXC_OBJ_LBS_COMBOMASK) == EXC_OBJ_LBS_COMBO_SIMPLE )
                SetSkip();
        break;
    }

}

void XclImpEscherTbxCtrl::ReadSbs( XclImpStream& rStrm )
{
    sal_uInt16 nOrient, nStyle;
    rStrm.Ignore( 4 );
    rStrm >> mnScrollValue >> mnScrollMin >> mnScrollMax >> mnScrollStep >> mnScrollPage >> nOrient;
    rStrm.Ignore( 2 );
    rStrm >> nStyle;

    mbScrollHor = ::get_flag( nOrient, EXC_OBJ_SBS_HORIZONTAL );
    mbFlatButton = ::get_flag( nStyle, EXC_OBJ_SBS_FLAT );
}

void XclImpEscherTbxCtrl::ReadGboData( XclImpStream& rStrm )
{
    sal_uInt16 nStyle;
    rStrm.Ignore( 4 );
    rStrm >> nStyle;
    mbFlatBorder = ::get_flag( nStyle, EXC_OBJ_GBO_FLAT );
}

void XclImpEscherTbxCtrl::ReadMacro( XclImpStream& rStrm )
{
    maMacroName.Erase();
    if( rStrm.GetRecLeft() > 6 )
    {
        // macro is stored in a tNameXR token containing a link to a defined name
        sal_uInt16 nFmlaSize;
        rStrm >> nFmlaSize;
        rStrm.Ignore( 4 );
        DBG_ASSERT( nFmlaSize == 7, "XclImpEscherTbxCtrl::ReadMacro - unexpected formula size" );
        if( nFmlaSize == 7 )
        {
            sal_uInt8 nTokenId;
            sal_uInt16 nExtSheet, nExtName;
            rStrm >> nTokenId >> nExtSheet >> nExtName;
            DBG_ASSERT( nTokenId == XclTokenArrayHelper::GetTokenId( EXC_TOKID_NAMEX, EXC_TOKCLASS_REF ),
                "XclImpEscherTbxCtrl::ReadMacro - tNameXR token expected" );
            if( nTokenId == XclTokenArrayHelper::GetTokenId( EXC_TOKID_NAMEX, EXC_TOKCLASS_REF ) )
                maMacroName = GetLinkManager().GetMacroName( nExtSheet, nExtName );
        }
    }
}

OUString XclImpEscherTbxCtrl::GetServiceName() const
{
    return XclTbxControlHelper::GetServiceName( mnCtrlType );
}

void XclImpEscherTbxCtrl::WriteToPropertySet( ScfPropertySet& rPropSet ) const
{
    rPropSet.SetBoolProperty( CREATE_OUSTRING( "Printable" ), GetPrintable() );

    namespace AwtVisualEffect = ::com::sun::star::awt::VisualEffect;
    namespace AwtScrollOrient = ::com::sun::star::awt::ScrollBarOrientation;
    using ::com::sun::star::style::VerticalAlignment_MIDDLE;

    // control name -----------------------------------------------------------

    OUString aName = XclTbxControlHelper::GetControlName( mnCtrlType );
    if( aName.getLength() )
        rPropSet.SetProperty( CREATE_OUSTRING( "Name" ), aName );

    // control label ----------------------------------------------------------

    if( const XclImpString* pString = GetString() )
    {
        // the visible label (caption)
        rPropSet.SetStringProperty( CREATE_OUSTRING( "Label" ), pString->GetText() );

        // font properties
        if( !pString->GetFormats().empty() )
        {
            const XclFormatRun& rFormatRun = pString->GetFormats().front();
            if( const XclImpFont* pFont = GetFontBuffer().GetFont( rFormatRun.mnXclFont ) )
            {
                const XclFontData& rFontData = pFont->GetFontData();
                rPropSet.SetStringProperty( CREATE_OUSTRING( "FontName" ), rFontData.maName );
                sal_Int16 nHeight = static_cast< sal_Int16 >( rFontData.GetApiHeight() + 0.5 );
                rPropSet.SetProperty( CREATE_OUSTRING( "FontHeight" ), nHeight );
                rPropSet.SetProperty( CREATE_OUSTRING( "FontFamily" ), rFontData.GetApiFamily() );
                rPropSet.SetProperty( CREATE_OUSTRING( "FontCharset" ), rFontData.GetApiCharSet() );
                rPropSet.SetProperty( CREATE_OUSTRING( "FontWeight" ), rFontData.GetApiWeight() );
                rPropSet.SetProperty( CREATE_OUSTRING( "FontSlant" ), rFontData.GetApiPosture() );
                rPropSet.SetProperty( CREATE_OUSTRING( "FontUnderline" ), rFontData.GetApiUnderline() );
                rPropSet.SetProperty( CREATE_OUSTRING( "FontStrikeout" ), rFontData.GetApiStrikeout() );
                rPropSet.SetColorProperty( CREATE_OUSTRING( "TextColor" ), GetPalette().GetColor( rFontData.mnColor ) );
            }
        }
    }

    // special control contents -----------------------------------------------

    switch( mnCtrlType )
    {
        // checkbox/option button

        case EXC_OBJ_CMO_CHECKBOX:
        case EXC_OBJ_CMO_OPTIONBUTTON:
        {
            bool bCheckBox = mnCtrlType == EXC_OBJ_CMO_CHECKBOX;

            sal_Int16 nApiState = 0;
            switch( mnState )
            {
                case EXC_OBJ_CBLS_STATE_UNCHECK:    nApiState = 0;                  break;
                case EXC_OBJ_CBLS_STATE_CHECK:      nApiState = 1;                  break;
                case EXC_OBJ_CBLS_STATE_TRI:        nApiState = bCheckBox ? 2 : 1;  break;
            }
            if( bCheckBox )
                rPropSet.SetBoolProperty( CREATE_OUSTRING( "TriState" ), nApiState == 2 );
            rPropSet.SetProperty( CREATE_OUSTRING( "DefaultState" ), nApiState );

            sal_Int16 nApiBorder = mbFlatButton ? AwtVisualEffect::FLAT : AwtVisualEffect::LOOK3D;
            rPropSet.SetProperty( CREATE_OUSTRING( "VisualEffect" ), nApiBorder );

            // #i40279# always centered vertically
            rPropSet.SetProperty( CREATE_OUSTRING( "VerticalAlign" ), VerticalAlignment_MIDDLE );
        }
        break;

        // listbox/combobox

        case EXC_OBJ_CMO_LISTBOX:
        case EXC_OBJ_CMO_COMBOBOX:
        {
            sal_Int16 nApiBorder = mbFlatBorder ? AwtVisualEffect::FLAT : AwtVisualEffect::LOOK3D;
            rPropSet.SetProperty( CREATE_OUSTRING( "Border" ), nApiBorder );

            Sequence< sal_Int16 > aSelection;

            switch( mnCtrlType )
            {
                case EXC_OBJ_CMO_LISTBOX:
                {
                    // selection type
                    bool bMultiSel = (mnSelType != EXC_OBJ_LBS_SEL_SIMPLE);
                    rPropSet.SetBoolProperty( CREATE_OUSTRING( "MultiSelection" ), bMultiSel );

                    // selection
                    if( bMultiSel )
                    {
                        aSelection.realloc( static_cast< sal_Int32 >( maMultiSel.size() ) );
                        sal_Int32 nSeqIndex = 0;
                        for( ScfInt16Vec::const_iterator aIter = maMultiSel.begin(), aEnd = maMultiSel.end();
                                aIter != aEnd; ++aIter, ++nSeqIndex )
                            aSelection[ nSeqIndex ] = *aIter;
                    }
                    else if( mnSelEntry > 0 )
                    {
                        aSelection.realloc( 1 );
                        aSelection[ 0 ] = mnSelEntry - 1;
                    }
                }
                break;

                case EXC_OBJ_CMO_COMBOBOX:
                {
                    // dropdown button
                    rPropSet.SetBoolProperty( CREATE_OUSTRING( "Dropdown" ), true );
                    // dropdown line count
                    rPropSet.SetProperty( CREATE_OUSTRING( "LineCount" ), mnLineCount );
                    // selection
                    if( mnSelEntry > 0 )
                    {
                        aSelection.realloc( 1 );
                        aSelection[ 0 ] = mnSelEntry - 1;
                    }
                }
                break;
            }

            if( !GetCellLink() && aSelection.getLength() )
                rPropSet.SetProperty( CREATE_OUSTRING( "DefaultSelection" ), aSelection );
        }
        break;

        // spin button

        case EXC_OBJ_CMO_SPIN:
        {
            // Calc's "Border" property is not the 3D/flat style effect in Excel (#i34712#)
            rPropSet.SetProperty( CREATE_OUSTRING( "Border" ), AwtVisualEffect::NONE );
            rPropSet.SetProperty< sal_Int32 >( CREATE_OUSTRING( "SpinValueMin" ), mnScrollMin );
            rPropSet.SetProperty< sal_Int32 >( CREATE_OUSTRING( "SpinValueMax" ), mnScrollMax );
            rPropSet.SetProperty< sal_Int32 >( CREATE_OUSTRING( "SpinIncrement" ), mnScrollStep );
            // Excel spin buttons always vertical
            rPropSet.SetProperty( CREATE_OUSTRING( "Orientation" ), AwtScrollOrient::VERTICAL );
            rPropSet.SetProperty< sal_Int32 >( CREATE_OUSTRING( "DefaultSpinValue" ), mnScrollValue );
        }
        break;

        // scrollbar

        case EXC_OBJ_CMO_SCROLLBAR:
        {
            sal_Int32 nApiOrient = mbScrollHor ? AwtScrollOrient::HORIZONTAL : AwtScrollOrient::VERTICAL;
            sal_Int32 nVisSize = std::min< sal_Int32 >( mnScrollPage, 1 );

            // Calc's "Border" property is not the 3D/flat style effect in Excel (#i34712#)
            rPropSet.SetProperty( CREATE_OUSTRING( "Border" ), AwtVisualEffect::NONE );
            rPropSet.SetProperty< sal_Int32 >( CREATE_OUSTRING( "ScrollValueMin" ), mnScrollMin );
            rPropSet.SetProperty< sal_Int32 >( CREATE_OUSTRING( "ScrollValueMax" ), mnScrollMax );
            rPropSet.SetProperty< sal_Int32 >( CREATE_OUSTRING( "LineIncrement" ), mnScrollStep );
            rPropSet.SetProperty< sal_Int32 >( CREATE_OUSTRING( "BlockIncrement" ), mnScrollPage );
            rPropSet.SetProperty( CREATE_OUSTRING( "VisibleSize" ), nVisSize );
            rPropSet.SetProperty( CREATE_OUSTRING( "Orientation" ), nApiOrient );
            rPropSet.SetProperty< sal_Int32 >( CREATE_OUSTRING( "DefaultScrollValue" ), mnScrollValue );
        }
        break;
    }
}

bool XclImpEscherTbxCtrl::FillMacroDescriptor( ScriptEventDescriptor& rEvent ) const
{
    if( maMacroName.Len() )
    {
        // type of action is dependent on control type
        rEvent.ListenerType = XclTbxControlHelper::GetListenerType( mnCtrlType );
        rEvent.EventMethod = XclTbxControlHelper::GetEventMethod( mnCtrlType );
        if( rEvent.ListenerType.getLength() && rEvent.EventMethod.getLength() )
        {
            // set the macro name
            rEvent.ScriptType = XclTbxControlHelper::GetScriptType();
            rEvent.ScriptCode = XclTbxControlHelper::GetScMacroName( maMacroName );
            return true;
        }
    }
    return false;
}

bool XclImpEscherTbxCtrl::Apply( ScfProgressBar& rProgress )
{
    bool bInserted = false;
    // do not use IsValid() - the SdrObject is still missing
    if( !GetIsSkip() && IsValidSize() )
    {
        if( GetObjectManager().CreateSdrObj( *this ) )
        {
            // form controls: set bound rect explicitly
            mxSdrObj->NbcSetSnapRect( maAnchorRect );
            // #i30543# insert into control layer
            mxSdrObj->NbcSetLayer( SC_LAYER_CONTROLS );
            // insert the SdrObj into the draw page
            bInserted = XclImpEscherObj::Apply( rProgress );
        }
        else
        {
            // no success to create a control -> create a text box
            bInserted = XclImpEscherTxo::Apply( rProgress );
        }
    }
    else
    {
        rProgress.Progress();   // invalid objects are included in progress bar
    }
    return bInserted;
}

// ----------------------------------------------------------------------------

TYPEINIT1( XclImpEscherOle, XclImpEscherObj );

XclImpEscherOle::XclImpEscherOle( XclImpEscherObj& rSrcObj ):
    XclImpEscherObj( rSrcObj ),
    XclImpCtrlLinkHelper( xlBindContent ),
    mnCtrlStrmPos( 0 ),
    mnBlipId( 0 ),
    mbAsSymbol( false ),
    mbLinked( false ),
    mbControl( false )
{
    SetAreaObj( true );
}

void XclImpEscherOle::ReadPioGrbit( XclImpStream& rStrm )
{
    sal_uInt16 nFlags;
    rStrm >> nFlags;
    mbAsSymbol = ::get_flag( nFlags, EXC_OBJ_PIO_SYMBOL );
    mbLinked = ::get_flag( nFlags, EXC_OBJ_PIO_LINKED );
}

void XclImpEscherOle::ReadPictFmla( XclImpStream& rStrm, sal_uInt16 nRecSize )
{
    sal_uInt32 nStorageId;
    sal_uInt16 nFmlaLen;
    rStrm >> nFmlaLen;

    String aUserName;
    sal_uInt32 nPos0 = rStrm.GetRecPos();        // fmla start
    bool bOk = true;

    // --- linked ---

    if( mbLinked )
    {
        bool bSizeOk = (sizeof(nFmlaLen) + nFmlaLen == nRecSize);
        DBG_ASSERT( bSizeOk, "XclImpEscherOle::ReadPictFmla - bad linked size" );
        rStrm.Ignore( 7 );
        sal_uInt16 nXti, nExtName;
        rStrm >> nXti >> nExtName;
        const XclImpExtName* pExtName = GetLinkManager().GetExternName( nXti, nExtName );
        bOk = (pExtName && (pExtName->GetType() == xlExtOLE));
        DBG_ASSERT( bOk, "XclImpEscherOle::ReadPictFmla - EXTERNNAME not found or not OLE" );
        if( bOk )
            nStorageId = pExtName->GetStorageId();
    }

    // --- embedded ---

    else if( nFmlaLen + 2 < nRecSize )  // #107158# ignore picture links (are embedded OLE obj's too)
    {
        sal_uInt16 n16;
        rStrm >> n16;     // should be 5 but who knows ...
        DBG_ASSERT( n16 + 4 <= nFmlaLen, "XclImpEscherOle::ReadPictFmla - embedded length mismatch" );
        if( n16 + 4 <= nFmlaLen )
        {
            rStrm.Ignore( n16 + 4 );
            sal_uInt8 n8;
            rStrm >> n8;
            DBG_ASSERT( n8 == 0x03, "XclImpEscherOle::ReadPictFmla - no name start" );
            if( n8 == 0x03 )
            {
                rStrm >> n16;     // string length
                if( n16 )
                {   // the 4th way Xcl stores a unicode string: not even a Grbit byte present if length 0
                    aUserName = rStrm.ReadUniString( n16 );
                    // 0:= ID follows, 1:= pad byte + ID
#ifndef PRODUCT
                    sal_Int32 nLeft = sal_Int32(nFmlaLen) - (rStrm.GetRecPos() - nPos0);
                    DBG_ASSERT( nLeft == 0 || nLeft == 1, "XclImpEscherOle::ReadPictFmla - unknown left over" );
#endif

                    // is it a form control?
                    mbControl = aUserName.EqualsAscii( "Forms.", 0, 6 );
                }
            }
        }
        rStrm.Seek( nPos0 + nFmlaLen );
        rStrm >> nStorageId;
        if( IsControl() )
        {
            mnCtrlStrmPos = nStorageId;
            bOk = false;    // do not create the storage name for controls

            if( aUserName.EqualsAscii( "Forms.HTML:Hidden.1" ) )
            {
                // #i26521# form controls to be ignored
                SetSkip();
            }
            else if( rStrm.GetRecLeft() > 8 )
            {
                // read additional link data
                rStrm.Ignore( 8 );
                sal_uInt16 nDataSize;

                // cell link
                rStrm >> nDataSize;
                if( nDataSize )
                {
                    rStrm.PushPosition();
                    ReadCellLinkFormula( rStrm );
                    rStrm.PopPosition();
                    rStrm.Ignore( nDataSize );
                }
                // source data range
                rStrm >> nDataSize;
                if( nDataSize )
                {
                    rStrm.PushPosition();
                    ReadSrcRangeFormula( rStrm );
                    rStrm.PopPosition();
                    rStrm.Ignore( nDataSize );
                }
            }
        }
        else if( nStorageId )
        {
            bool bSizeOk = sizeof(nFmlaLen) + nFmlaLen + sizeof(nStorageId) == nRecSize;
            DBG_ASSERT( bSizeOk, "XclImpEscherOle::ReadPictFmla - bad embedded size" );
        }
        else
            bOk = false;    // no storage, internal
    }

    if( bOk )
    {
        maStorageName.AssignAscii( mbLinked ? "LNK" : "MBD" );
        sal_Char aBuf[ 2 * sizeof( sal_uInt32 ) + 1 ];
        sprintf( aBuf, "%08X", nStorageId );    // #100211# - checked
        maStorageName.AppendAscii( aBuf );
    }
}

void XclImpEscherOle::WriteToPropertySet( ScfPropertySet& rPropSet ) const
{
    rPropSet.SetBoolProperty( CREATE_OUSTRING( "Printable" ), GetPrintable() );
    // #118053 set control name
    if( maName.getLength() > 0 )
        rPropSet.SetProperty( CREATE_OUSTRING( "Name" ), maName );
}

bool XclImpEscherOle::Apply( ScfProgressBar& rProgress )
{
    // do not use IsValid() - the SdrObject is still missing
    if( !GetIsSkip() && IsValidSize() && GetObjectManager().CreateSdrObj( *this ) )
    {
        SfxObjectShell* pDocShell = GetDocShell();
        SdrOle2Obj* pOleSdrObj = PTR_CAST( SdrOle2Obj, mxSdrObj.get() );
        if( pOleSdrObj && pDocShell )
        {
            Reference< XEmbeddedObject > xObj = pOleSdrObj->GetObjRef();
            OUString aOldName( pOleSdrObj->GetPersistName() );

            if ( pDocShell->GetEmbeddedObjectContainer().HasEmbeddedObject( aOldName ) )
            {
                // the object persistence should be already in the storage,
                // but the object still might not be inserted into the container
                if ( !pDocShell->GetEmbeddedObjectContainer().HasEmbeddedObject( xObj ) )
                {
                    // a filter code is allowed to call the following method
                    pDocShell->GetEmbeddedObjectContainer().AddEmbeddedObject( xObj, aOldName );
                }
            }
            else
            {
                // if the object is still not in container it must be iserted there, the name must be generated in this case
                OUString aNewName;
                pDocShell->GetEmbeddedObjectContainer().InsertEmbeddedObject( xObj, aNewName );
                if ( aOldName != aNewName )
                    // #95381# SetPersistName, not SetName
                    pOleSdrObj->SetPersistName( aNewName );
            }
        }
        else if( mxSdrObj->ISA( SdrUnoObj ) )
        {
            // form controls: set bound rect explicitly
            mxSdrObj->NbcSetSnapRect( maAnchorRect );
            // #i30543# insert into control layer
            mxSdrObj->NbcSetLayer( SC_LAYER_CONTROLS );
        }
    }
    return XclImpEscherObj::Apply( rProgress );
}

// ----------------------------------------------------------------------------

TYPEINIT1( XclImpEscherChart, XclImpEscherObj );

XclImpEscherChart::XclImpEscherChart( XclImpEscherObj& rSrcObj, bool bOwnTab ) :
    XclImpEscherObj( rSrcObj ),
    mbOwnTab( bOwnTab )
{
    SetAreaObj( true );
}

void XclImpEscherChart::ReadChartSubStream( XclImpStream& rStrm )
{
    DBG_ASSERT( rStrm.GetRecId() == EXC_ID5_BOF, "XclImpEscherChart::ReadChartSubStream - no BOF record found" );
    if( rStrm.GetRecId() == EXC_ID5_BOF )
    {
        mxChart.reset( new XclImpChart( GetRoot(), mbOwnTab ) );
        mxChart->ReadChartSubStream( rStrm );
        if( mbOwnTab )
            SetTabChartAnchor();
    }
}

sal_uInt32 XclImpEscherChart::GetProgressSize() const
{
    return mxChart.is() ? mxChart->GetProgressSize() : 0;
}

bool XclImpEscherChart::Apply( ScfProgressBar& rProgress )
{
    // do not use IsValid() - the SdrObject is still missing
    if( !mxChart || GetIsSkip() || !IsValidSize() )
        return false;

    // Trace if chart object is not printable.
    if( !GetPrintable() )
        GetTracer().TraceObjectNotPrintable();

    SfxObjectShell* pDocShell = GetDocShell();
    if( !pDocShell ) return false;

    bool bInserted = false;
    if( SvtModuleOptions().IsChart() )
    {
        OUString aName;
        Reference< XEmbeddedObject > xObj = pDocShell->GetEmbeddedObjectContainer().
                CreateEmbeddedObject( SvGlobalName( SO3_SCH_CLASSID ).GetByteSequence(), aName );

        sal_Int64 nAspect = ::com::sun::star::embed::Aspects::MSOLE_CONTENT;
        ::com::sun::star::awt::Size aAwtSize = xObj->getVisualAreaSize( nAspect );
        Size aSize( aAwtSize.Width, aAwtSize.Height );
        if( (aSize.Height() < 1) || (aSize.Width() < 1) )
        {
            MapUnit aUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( nAspect ) );
            aSize.Width() = aSize.Height() = 5000;
            aSize = Window::LogicToLogic( aSize, MapMode( MAP_100TH_MM ), MapMode( aUnit ) );
            aAwtSize.Width = aSize.Width();
            aAwtSize.Height = aSize.Height();
            xObj->setVisualAreaSize( nAspect, aAwtSize );
        }

        SdrOle2Obj* pSdrObj = new SdrOle2Obj( svt::EmbeddedObjectRef( xObj, nAspect ), aName, maAnchorRect );
        pSdrObj->NbcSetLayer( SC_LAYER_FRONT );
        if( SdrPage* pPage = GetSdrPage( GetScTab() ) )
        {
            pPage->InsertObject( pSdrObj );
            bInserted = true;
        }
        pSdrObj->NbcSetLogicRect( maAnchorRect );

        // create the chart array (core representation of source data)
        ScChartArray aChartObj( GetDocPtr(), mxChart->GetSourceData(), aName );
        aChartObj.SetHeaders( mxChart->HasHeaderRow(), mxChart->HasHeaderColumn() );

        SchMemChart* pMemChart = aChartObj.CreateMemChart();
        SchDLL::Update( xObj, pMemChart );
        pSdrObj->GetNewReplacement();
        delete pMemChart;

        if( svt::EmbeddedObjectRef::TryRunningState( xObj ) )
        {
            Reference< XChartDocument > xChartDoc( xObj->getComponent(), UNO_QUERY );
            mxChart->Convert( xChartDoc, rProgress );

            Reference< XEmbedPersist > xPers( xObj, UNO_QUERY );
            if( xPers.is() )
                xPers->storeOwn();
        }
    }
    return bInserted;
}

void XclImpEscherChart::SetTabChartAnchor()
{
    // set uninitialized page to landscape
    if ( !GetPageSettings().GetPageData().mbValid )
        GetPageSettings().SetPaperSize( EXC_PAPERSIZE_DEFAULT, false );

    // calculate size of the chart object
    const XclPageData& rPageData = GetPageSettings().GetPageData();
    Size aPaperSize( rPageData.GetScPaperSize( GetPrinter() ) );

    long nWidth = XclTools::GetHmmFromTwips( aPaperSize.Width() );
    long nHeight = XclTools::GetHmmFromTwips( aPaperSize.Height() );

    // subtract page margins, give 1cm extra space
    nWidth -= (XclTools::GetHmmFromInch( rPageData.mfLeftMargin + rPageData.mfRightMargin ) + 1000);
    nHeight -= (XclTools::GetHmmFromInch( rPageData.mfTopMargin + rPageData.mfBottomMargin ) + 1000);

    // print column/row headers?
    if( rPageData.mbPrintHeadings )
    {
        nWidth -= 2000;
        nHeight -= 1000;
    }

    SetAnchor( Rectangle( 500, 500, nWidth, nHeight ) );
}

// Escher object data =========================================================

XclImpObjData::XclImpObjData( XclImpEscherObj* pEscherObj ) :
    maAnchor( 0 ),
    mbInserted( false )
{
    SetObj( pEscherObj );
}

void XclImpObjData::SetObj( XclImpEscherObj* pEscherObj )
{
    mxEscherObj.reset( pEscherObj );
    maAnchor.mnScTab = pEscherObj ? pEscherObj->GetScTab() : 0;
}

bool XclImpObjData::ContainsStrmPos( ULONG nStrmPos ) const
{
    return mxEscherObj.get() && (mxEscherObj->GetStrmBegin() <= nStrmPos) && (nStrmPos < mxEscherObj->GetStrmEnd());
}

sal_uInt32 XclImpObjData::GetProgressSize() const
{
    return mxEscherObj.get() ? mxEscherObj->GetProgressSize() : 0;
}

void XclImpObjData::Apply( ScfProgressBar& rProgress )
{
    mbInserted = mxEscherObj.get() && mxEscherObj->Apply( rProgress );
}

void XclImpObjData::ExtendUsedArea( const XclImpRoot& rRoot, ScRange& rScUsedArea, SCTAB nScTab ) const
{
    if( mbInserted && mxEscherObj.get() && (mxEscherObj->GetScTab() == nScTab) )
    {
        // #i44077# object inserted -> update used area for OLE object import
        ScRange aScRange( ScAddress::UNINITIALIZED );
        if( rRoot.GetAddressConverter().ConvertRange( aScRange, maAnchor.maXclRange, nScTab, nScTab, false ) )
        {
            // reduce range, if object ends directly on borders between two columns or rows
            if( (maAnchor.mnRX == 0) && (aScRange.aStart.Col() < aScRange.aEnd.Col()) )
                aScRange.aEnd.IncCol( -1 );
            if( (maAnchor.mnBY == 0) && (aScRange.aStart.Row() < aScRange.aEnd.Row()) )
                aScRange.aEnd.IncRow( -1 );
            rScUsedArea.ExtendTo( aScRange );
        }
    }
}

// ----------------------------------------------------------------------------

XclImpEscherObjList::XclImpEscherObjList( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot )
{
}

void XclImpEscherObjList::AppendObj( XclImpEscherObj* pEscherObj )
{
    DBG_ASSERT( pEscherObj, "XclImpEscherObjList::AppendObj - missing object" );
    maObjDataList.Append( new XclImpObjData( pEscherObj ) );
    UpdateCache();
}

void XclImpEscherObjList::AppendDummyObj( XclImpEscherObj* pEscherObj )
{
    // do not modify the cache
    maObjDataList.Append( new XclImpObjData( pEscherObj ) );
    if( pEscherObj )
        pEscherObj->SetSkip();
}

void XclImpEscherObjList::ReplaceLastObj( XclImpEscherObj* pEscherObj )
{
    if( XclImpObjData* pObjData = maObjDataList.Last() )
    {
        DBG_ASSERT( pEscherObj, "XclImpEscherObjList::ReplaceLastObj - missing object" );
        pObjData->SetObj( pEscherObj );
        UpdateCache();
    }
    else
        AppendObj( pEscherObj );
}

XclImpEscherObj* XclImpEscherObjList::GetObj( SCTAB nScTab, sal_uInt16 nObjId ) const
{
    if( (nObjId != EXC_OBJ_INVALID_ID) && (nScTab >= 0) )
    {
        // objects are ordered by sheet index
        size_t nCacheSize = maObjCache.size();
        size_t nCacheIdx = static_cast< size_t >( nScTab );
        if( nCacheIdx < nCacheSize )
        {
            ULONG nEnd = (nCacheIdx + 1 < nCacheSize) ? maObjCache[ nCacheIdx + 1 ].mnListIdx : maObjDataList.Count();
            for( ULONG nListIdx = maObjCache[ nCacheIdx ].mnListIdx; nListIdx < nEnd; ++nListIdx )
                if( XclImpEscherObj* pEscherObj = maObjDataList.GetObject( nListIdx )->GetObj() )
                    if( (pEscherObj->GetScTab() == nScTab) && (pEscherObj->GetObjId() == nObjId) )
                        return pEscherObj;
        }
    }
    return 0;
}

XclImpEscherObj* XclImpEscherObjList::GetLastObj() const
{
    if( XclImpObjData* pObjData = maObjDataList.Last() )
        return pObjData->GetObj();
    return 0;
}

XclImpEscherObj* XclImpEscherObjList::GetObj( ULONG nStrmPos ) const
{
    XclImpObjData* pObjData = FindObjData( nStrmPos );
    return pObjData ? pObjData->GetObj() : 0;
}

XclEscherAnchor* XclImpEscherObjList::GetAnchor( ULONG nStrmPos ) const
{
    XclImpObjData* pObjData = FindObjData( nStrmPos );
    return pObjData ? &pObjData->GetAnchor() : 0;
}

void XclImpEscherObjList::Apply( ScfProgressBar& rProgress )
{
    DBG_ASSERT( !rProgress.IsStarted(), "XclImpEscherObjList::InitProgress - progress already started" );
    XclImpObjData* pData = 0;

    // initialize progress bar
    sal_uInt32 nSegSize = 0;
    for( pData = maObjDataList.First(); pData; pData = maObjDataList.Next() )
        nSegSize += pData->GetProgressSize();

    // insert the objects into the drawing layer
    sal_Int32 nSeg = rProgress.AddSegment( std::max< sal_uInt32 >( nSegSize, 1 ) );
    rProgress.ActivateSegment( nSeg );
    for( pData = maObjDataList.First(); pData; pData = maObjDataList.Next() )
        pData->Apply( rProgress );
}

void XclImpEscherObjList::ExtendUsedArea( ScRange& rScUsedArea, SCTAB nScTab ) const
{
    for( const XclImpObjData* pData = maObjDataList.First(); pData; pData = maObjDataList.Next() )
        pData->ExtendUsedArea( GetRoot(), rScUsedArea, nScTab );
}

void XclImpEscherObjList::UpdateCache()
{
    if( const XclImpEscherObj* pEscherObj = GetLastObj() )
    {
        SCTAB nScTab = pEscherObj->GetScTab();
        ULONG nStrmPos = pEscherObj->GetStrmBegin();

        // #110252# ignore faked objects without corresponding Escher data (i.e. sheet-charts)
        if( (nScTab >= 0) && (nStrmPos != 0) )
        {
            size_t nCacheIdx = static_cast< size_t >( nScTab );
            if( nCacheIdx >= maObjCache.size() )
                maObjCache.resize( nCacheIdx + 1, XclCacheEntry( GetObjCount() - 1, nStrmPos ) );
            else if( maObjCache[ nCacheIdx ].mnStrmPos > nStrmPos )
                maObjCache[ nCacheIdx ].mnStrmPos = nStrmPos;
            DBG_ASSERT( (nCacheIdx == 0) || (maObjCache[ nCacheIdx - 1 ].mnStrmPos <= nStrmPos), "XclImpEscherObjList::UpdateCache - cache corrupted" );
        }
    }
}

XclImpObjData* XclImpEscherObjList::FindObjData( ULONG nStrmPos ) const
{
    size_t nCacheSize = maObjCache.size();
    size_t nFoundIdx = 0;
    bool bFound = false;

    // find the correct cache entry
    for( size_t nCacheIdx = 0; !bFound && (nCacheIdx < nCacheSize); ++nCacheIdx )
    {
        ULONG nStrmEnd = (nCacheIdx + 1 < nCacheSize) ? maObjCache[ nCacheIdx + 1 ].mnStrmPos : STREAM_SEEK_TO_END;
        bFound = (maObjCache[ nCacheIdx ].mnStrmPos <= nStrmPos) && (nStrmPos < nStrmEnd);
        if( bFound )
            nFoundIdx = nCacheIdx;
    }

    // find the object in the found list range
    if( bFound )
    {
        ULONG nListEnd = (nFoundIdx + 1 < nCacheSize) ? maObjCache[ nFoundIdx + 1 ].mnListIdx : maObjDataList.Count();
        for( ULONG nListIdx = maObjCache[ nFoundIdx ].mnListIdx; nListIdx < nListEnd; ++nListIdx )
        {
            XclImpObjData* pObjData = maObjDataList.GetObject( nListIdx );
            if( pObjData->ContainsStrmPos( nStrmPos ) )
                return pObjData;
        }
    }
    return 0;
}

// Escher stream conversion ===================================================

XclImpDffManager::XclImpDffManager(
        const XclImpRoot& rRoot, XclImpObjectManager& rObjManager,
        long nOffsDgg, SvStream* pStData, SdrModel* pSdrModel, long nApplicationScale ) :
    SvxMSDffManager( rObjManager.GetEscherStream(), rRoot.GetBasePath(), nOffsDgg, pStData, pSdrModel, nApplicationScale, COL_DEFAULT, 24, 0, &rRoot.GetTracer().GetBaseTracer() ),
    XclImpRoot( rRoot ),
    mrObjManager( rObjManager ),
    mnOleImpFlags( 0 )
{
    SetSvxMSDffSettings( SVXMSDFF_SETTINGS_CROP_BITMAPS | SVXMSDFF_SETTINGS_IMPORT_EXCEL | SVXMSDFF_SETTINGS_IMPORT_IAS );

    if( SvtFilterOptions* pFilterOpt = SvtFilterOptions::Get() )
    {
        if( pFilterOpt->IsMathType2Math() )
            mnOleImpFlags |= OLE_MATHTYPE_2_STARMATH;
        if( pFilterOpt->IsWinWord2Writer() )
            mnOleImpFlags |= OLE_WINWORD_2_STARWRITER;
        if( pFilterOpt->IsPowerPoint2Impress() )
            mnOleImpFlags |= OLE_POWERPOINT_2_STARIMPRESS;
    }
}

void XclImpDffManager::SetSdrObject( XclImpEscherObj* pEscherObj, ULONG nId, SvxMSDffImportData& rData )
{
    SdrObject* pSdrObj = 0;
    bool bRet = GetShape( nId, pSdrObj, rData );
    if( bRet )
        pEscherObj->SetSdrObj( pSdrObj );
    else
        delete pSdrObj;
}

bool XclImpDffManager::CreateSdrOleObj( XclImpEscherOle& rOleObj )
{
    SfxObjectShell* pDocShell = GetDocShell();
    const Rectangle& rAnchor = rOleObj.GetAnchor();
    const String& rStorageName = rOleObj.GetStorageName();

    if( pDocShell && !rAnchor.IsEmpty() && rStorageName.Len() )
    {
        Graphic aGraph;
        if( GetBLIP( rOleObj.GetBlipId(), aGraph ) )
        {
            ErrCode nError = ERRCODE_NONE;
            SotStorageRef xSrc = GetRootStorage();
            if( SdrOle2Obj* pOleSdrObj = CreateSdrOLEFromStorage(
                    rStorageName, xSrc, pDocShell->GetStorage(), aGraph, rAnchor, NULL, nError, mnOleImpFlags ) )
            {
                rOleObj.SetSdrObj( pOleSdrObj );
                return true;
            }
        }
    }
    return false;
}

void XclImpDffManager::ProcessClientAnchor2( SvStream& rStrm, DffRecordHeader& rHeader, void*, DffObjData& rObjData )
{
    rHeader.SeekToContent( rStrm );
    rStrm.SeekRel( 2 );
    ULONG nFilePos = rStrm.Tell();

    if( XclEscherAnchor* pAnchor = mrObjManager.GetEscherAnchorAcc( nFilePos ) )
    {
        rStrm >> *pAnchor;
        rObjData.aChildAnchor = pAnchor->GetRect( GetDoc(), MAP_100TH_MM );
        rObjData.bChildAnchor = sal_True;

        if( XclImpEscherObj* pEscherObj = mrObjManager.GetEscherObjAcc( nFilePos ) )
        {
            pEscherObj->SetAnchor( rObjData.aChildAnchor );
            if( XclImpEscherOle* pOleObj = PTR_CAST( XclImpEscherOle, pEscherObj ) )
                pOleObj->SetBlipId( GetPropertyValue( DFF_Prop_pib ) );
        }
    }
}

SdrObject* XclImpDffManager::ProcessObj(
    SvStream& rStrm, DffObjData& rObjData, void*, Rectangle& rTextRect, SdrObject* pRetSdrObj )
{
    // #118052# import control name
    XclImpEscherObj* pObj = mrObjManager.GetEscherObjAcc( rObjData.rSpHd.nFilePos );
    if( XclImpEscherOle* pOleObj = PTR_CAST( XclImpEscherOle, pObj ) )
    {
        sal_uInt32 nNameLen = GetPropertyValue( DFF_Prop_wzName );
        if( nNameLen > 0 )
        {
            SeekToContent( DFF_Prop_wzName, mrObjManager.GetEscherStream() );
            sal_Int32 nStrLen = nNameLen / 2;
            OUStringBuffer aNameBfr( nStrLen );
            sal_uInt16 cChar = 0;
            for( sal_Int32 nIdx = 0; nIdx < nStrLen; ++nIdx )
            {
                mrObjManager.GetEscherStream() >> cChar;
                aNameBfr.append( static_cast< sal_Unicode >( cChar ) );
            }
            OUString aName( aNameBfr.makeStringAndClear() );
            pOleObj->SetName( aName );
        }
    }

    /*  #102378# Do not process the omnipresent first dummy shape in the table
        (it has the flag SP_FPATRIARCH set). */
    if( pRetSdrObj && !::get_flag< sal_uInt32 >( rObjData.nSpFlags, SP_FPATRIARCH ) )
    {
        // #i30816# objects in groups need manual check for valid size
        if( rObjData.nCalledByGroup > 0 )
            if( const XclImpEscherObj* pEscherObj = mrObjManager.GetEscherObj( rObjData.rSpHd.nFilePos ) )
                if( !pEscherObj->IsValidSize( rObjData.aChildAnchor ) )
                    DELETEZ( pRetSdrObj );

        if( pRetSdrObj )
        {
            // maybe if there is no color, we could do this in ApplyAttributes (writer?, calc?)
            if( GetPropertyBool( DFF_Prop_fFilled ) && !IsProperty( DFF_Prop_fillColor ) )
                pRetSdrObj->SetMergedItem( XFillColorItem( EMPTY_STRING, Color( COL_WHITE ) ) );

            // automatic margin is handled by host
            if( GetPropertyBool( DFF_Prop_AutoTextMargin ) )
            {
                long nMargin = EXC_ESCHER_AUTOMARGIN;
                ScaleEmu( nMargin );
                pRetSdrObj->SetMergedItem( SdrTextLeftDistItem( nMargin ) );
                pRetSdrObj->SetMergedItem( SdrTextRightDistItem( nMargin ) );
                pRetSdrObj->SetMergedItem( SdrTextUpperDistItem( nMargin ) );
                pRetSdrObj->SetMergedItem( SdrTextLowerDistItem( nMargin ) );
            }
            // #i39167# always default to "full width" for text & textless
            // objects regardless of alignment.
            pRetSdrObj->SetMergedItem( SdrTextHorzAdjustItem( SDRTEXTHORZADJUST_BLOCK ) );

            // text data and text alignment properties
            // #98132# don't ask for a text-ID, Escher export doesn't set one
            if( const XclImpEscherTxo* pTxoObj = mrObjManager.GetEscherTxo( rObjData.rSpHd.nFilePos ) )
                pTxoObj->ApplyTextOnSdrObj( *pRetSdrObj );

            // connector rules
            mrObjManager.UpdateConnectorRules( rObjData, *pRetSdrObj );
        }
    }

    return pRetSdrObj;
}

ULONG XclImpDffManager::Calc_nBLIPPos( ULONG nOrgVal, ULONG nStreamPos ) const
{
    return nStreamPos + 4;
}

FASTBOOL XclImpDffManager::GetColorFromPalette( USHORT nIndex, Color& rColor ) const
{
    ColorData nColor = GetPalette().GetColorData( static_cast< sal_uInt16 >( nIndex ) );

    if( nColor == COL_AUTO )
        return FALSE;

    rColor.SetColor( nColor );
    return TRUE;
}

sal_Bool XclImpDffManager::ShapeHasText( ULONG nShapeId, ULONG nFilePos ) const
{
    const XclImpEscherTxo* pTxoObj = mrObjManager.GetEscherTxo( nFilePos );
    return pTxoObj && pTxoObj->GetString();
}

// The object manager =========================================================

XclImpObjectManager::XclImpObjectManager( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot ),
    maEscherObjList( rRoot ),
    mbStartWithDummy( true )
{
}

XclImpObjectManager::~XclImpObjectManager()
{
}

const XclImpEscherObj* XclImpObjectManager::GetEscherObj( SCTAB nScTab, sal_uInt16 nObjId ) const
{
    return maEscherObjList.GetObj( nScTab, nObjId );
}

XclImpEscherObj* XclImpObjectManager::GetEscherObjAcc( SCTAB nScTab, sal_uInt16 nObjId )
{
    return maEscherObjList.GetObj( nScTab, nObjId );
}

const XclImpEscherObj* XclImpObjectManager::GetEscherObj( ULONG nStrmPos ) const
{
    return maEscherObjList.GetObj( nStrmPos );
}

XclImpEscherObj* XclImpObjectManager::GetEscherObjAcc( ULONG nStrmPos )
{
    return maEscherObjList.GetObj( nStrmPos );
}

const XclImpEscherObj* XclImpObjectManager::GetLastEscherObj() const
{
    return maEscherObjList.GetLastObj();
}

XclImpEscherObj* XclImpObjectManager::GetLastEscherObjAcc()
{
    return maEscherObjList.GetLastObj();
}

const XclEscherAnchor* XclImpObjectManager::GetEscherAnchor( ULONG nStrmPos ) const
{
    return maEscherObjList.GetAnchor( nStrmPos );
}

XclEscherAnchor* XclImpObjectManager::GetEscherAnchorAcc( ULONG nStrmPos )
{
    return maEscherObjList.GetAnchor( nStrmPos );
}

// *** Text boxes *** ---------------------------------------------------------

const XclImpEscherTxo* XclImpObjectManager::GetEscherTxo( ULONG nStrmPos ) const
{
    const XclImpEscherObj* pEscherObj = GetEscherObj( nStrmPos );
    return PTR_CAST( XclImpEscherTxo, pEscherObj );
}

XclImpEscherTxo* XclImpObjectManager::GetEscherTxoAcc( ULONG nStrmPos )
{
    return const_cast< XclImpEscherTxo* >( GetEscherTxo( nStrmPos ) );
}

const XclImpEscherNote* XclImpObjectManager::GetEscherNote( SCTAB nScTab, sal_uInt16 nObjId ) const
{
    const XclImpEscherObj* pEscherObj = GetEscherObj( nScTab, nObjId );
    return PTR_CAST( XclImpEscherNote, pEscherObj );
}

// *** Chart *** --------------------------------------------------------------

void XclImpObjectManager::StartNewChartObj()
{
    XclImpEscherObj aTmp( GetRoot() );
    AppendEscherObj( new XclImpEscherChart( aTmp, true ) );
}

// *** OLE / controls *** -----------------------------------------------------

bool XclImpObjectManager::CreateSdrObj( XclImpEscherOle& rOleObj )
{
    return rOleObj.IsControl() ?
        GetOcxConverter().CreateSdrUnoObj( rOleObj ) :
        GetDffManager().CreateSdrOleObj( rOleObj );
}

bool XclImpObjectManager::CreateSdrObj( XclImpEscherTbxCtrl& rCtrlObj )
{
    return GetOcxConverter().CreateSdrUnoObj( rCtrlObj );
}

// *** Read Excel records *** -------------------------------------------------

void XclImpObjectManager::ReadMsodrawinggroup( XclImpStream& rStrm )
{
    maStreamConsumer.ConsumeRecord( rStrm );
}

void XclImpObjectManager::ReadMsodrawing( XclImpStream& rStrm )
{
    rStrm.ResetRecord( false );     // disable internal CONTINUE handling

    if( !maStreamConsumer.HasData() )
        return;

    sal_uInt32 nRecSize = rStrm.GetRecSize();
    ULONG nStrmPos = maStreamConsumer.Tell();
    bool bClientTextBox = false;

    static sal_Char aBuf[ 0x0200 ];

    if( nRecSize )
    {
        const DffRecordHeader* pLatestRecHd = maStreamConsumer.ConsumeRecord( rStrm );
        if( pLatestRecHd )
        {
            switch( pLatestRecHd->nRecType )
            {
                case DFF_msofbtClientData:
                    maStreamConsumer.AppendData( aBuf, 0x0100 );
                break;
                case DFF_msofbtClientTextbox:
                    maStreamConsumer.AppendData( aBuf, 0x0200 );
                    if( nRecSize == 8 )
                        bClientTextBox = true;
                break;
                case DFF_msofbtSolverContainer:
                    maStreamConsumer.GetStream() >> GetSolverContainer();
                break;
            }
        }
    }

    // create a new escher object, if this MSODRAWING is not a client text box
    if( !bClientTextBox )
        AppendEscherObj( new XclImpEscherObj( GetRoot(), nStrmPos, maStreamConsumer.Tell() ) );
}

void XclImpObjectManager::ReadMsodrawingselection( XclImpStream& rStrm )
{
    // not supported
}

void XclImpObjectManager::ReadObj( XclImpStream& rStrm )
{
    rStrm.ResetRecord( false );     // disable internal CONTINUE handling

    sal_uInt16 nSubRecId, nSubRecSize;
    bool bLoop = true;
    while( bLoop && (rStrm.GetRecLeft() >= 4) )
    {
        rStrm >> nSubRecId >> nSubRecSize;
        rStrm.PushPosition();

        switch( nSubRecId )
        {
            case EXC_ID_OBJ_FTEND:      bLoop = false;                              break;
            case EXC_ID_OBJ_FTCMO:      ReadObjFtCmo( rStrm );                      break;
            case EXC_ID_OBJ_FTPIOGRBIT: ReadObjFtPioGrbit( rStrm );                 break;
            case EXC_ID_OBJ_FTPICTFMLA: ReadObjFtPictFmla( rStrm, nSubRecSize );    break;
            // TBX form control subrecords
            case EXC_ID_OBJ_FTCBLS:
            case EXC_ID_OBJ_FTSBSFMLA:
            case EXC_ID_OBJ_FTLBSDATA:
            case EXC_ID_OBJ_FTCBLSFMLA:
            case EXC_ID_OBJ_FTSBS:
            case EXC_ID_OBJ_FTGBODATA:
            case EXC_ID_OBJ_FTMACRO:    ReadObjTbxSubRec( rStrm, nSubRecId );       break;
        }

        rStrm.PopPosition();
        // sometimes the last subrecord has an invalid length -> min()
        rStrm.Ignore( ::std::min< sal_uInt32 >( nSubRecSize, rStrm.GetRecLeft() ) );
    }
}

void XclImpObjectManager::ReadTxo( XclImpStream& rStrm )
{
    rStrm.ResetRecord( false );     // disable internal CONTINUE handling

    sal_uInt16 nAlign, nOrient, nTextLen, nFormCnt;
    ::std::auto_ptr< XclImpString > pString;

    /*  Let this function read all records in every case, even if text cannot be
        applied to the current object - this skips all CONTINUE records. */

    // step 1: read TXO record
    rStrm >> nAlign;
    rStrm >> nOrient;
    rStrm.Ignore( 6 );
    rStrm >> nTextLen >> nFormCnt;
    nFormCnt /= 8;

    // step 2: read 1st CONTINUE with string
    bool bValid = true;
    if( nTextLen )
    {
        rStrm.StartNextRecord();
        bValid = rStrm.IsValid() && (rStrm.GetRecId() == EXC_ID_CONT);
        DBG_ASSERT( bValid, "XclImpObjectManager::ReadTxo - missing CONTINUE record" );
        if( bValid )
        {
            rStrm.ResetRecord( false );
            String aText( rStrm.ReadUniString( nTextLen ) );
            pString.reset( new XclImpString( aText ) );
        }
    }

    // step 3: read 2nd CONTINUE with format runs
    if( bValid && pString.get() && nFormCnt )
    {
        rStrm.StartNextRecord();
        bValid = rStrm.IsValid() && (rStrm.GetRecId() == EXC_ID_CONT);
        DBG_ASSERT( bValid, "XclImpObjectManager::ReadTxo - missing CONTINUE record" );
        if( bValid )
        {
            rStrm.ResetRecord( false );

            sal_uInt16 nChar, nFont;
            sal_uInt16 nCount = nFormCnt - 1;

            for( sal_uInt16 nIndex = 0; nIndex < nCount; ++nIndex )
            {
                rStrm >> nChar >> nFont;
                rStrm.Ignore( 4 );
                pString->AppendFormat( nChar, nFont );
            }
        }
    }

    if( XclImpEscherObj* pEscherObj = GetLastEscherObjAcc() )
    {
        // only Escher Drawing objects can have text (Charts, OLE cannot)
        if( bValid && pString.get() && pEscherObj->ISA( XclImpEscherDrawing ) )
        {
            XclImpEscherTxo* pTxoObj = PTR_CAST( XclImpEscherTxo, pEscherObj );
            if( !pTxoObj )
                // not yet a text object - create it
                ReplaceEscherObj( pTxoObj = new XclImpEscherTxo( *pEscherObj ) );

            pTxoObj->SetString( pString.release() );
            pTxoObj->SetAlignment( nAlign );
            pTxoObj->SetRotation( nOrient );
        }
    }
}

void XclImpObjectManager::ReadChartSubStream( XclImpStream& rStrm )
{
    if( XclImpEscherChart* pChartObj = PTR_CAST( XclImpEscherChart, GetLastEscherObj() ) )
        pChartObj->ReadChartSubStream( rStrm );
    else
        XclTools::SkipSubStream( rStrm );
}

// *** Misc *** ---------------------------------------------------------------

XclImpDffManager& XclImpObjectManager::GetDffManager()
{
    //TODO/MBA: needs to be fixed!
    if( !mxDffManager.get() )
        mxDffManager.reset( new XclImpDffManager( GetRoot(), *this, 0, 0, GetDoc().GetDrawLayer(), 1440 ) );
    return *mxDffManager;
}

void XclImpObjectManager::UpdateConnectorRules( const DffObjData& rObjData, SdrObject& rSdrObj )
{
    for( SvxMSDffConnectorRule* pRule = GetFirstConnectorRule(); pRule; pRule = GetNextConnectorRule() )
    {
        if( rObjData.nShapeId == pRule->nShapeC )
        {
            pRule->pCObj = &rSdrObj;
        }
        else
        {
            // #i37900# - remove fix for #i12638#
            if( rObjData.nShapeId == pRule->nShapeA )
            {
                pRule->pAObj = &rSdrObj;
                pRule->nSpFlagsA = rObjData.nSpFlags;
            }
            if( rObjData.nShapeId == pRule->nShapeB )
            {
                pRule->pBObj = &rSdrObj;
                pRule->nSpFlagsB = rObjData.nSpFlags;
            }
        }
    }
}

void XclImpObjectManager::SetSkipObj( SCTAB nScTab, sal_uInt16 nObjId )
{
    maSkipObjVec.push_back( XclSkipObj( nScTab, nObjId ) );
}

void XclImpObjectManager::Apply( ScfProgressBar& rProgress )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "sc", "dr104026", "XclImpObjectManager::Apply" );

    // mark objects to be skipped
    for( XclSkipObjVec::const_iterator aIt = maSkipObjVec.begin(), aEnd = maSkipObjVec.end(); aIt != aEnd; ++aIt )
        if( XclImpEscherObj* pEscherObj = maEscherObjList.GetObj( aIt->mnScTab, aIt->mnObjId ) )
            pEscherObj->SetSkip();

    // insert the objects into the drawing layer
    maEscherObjList.Apply( rProgress );

    // connector rules
    if( mxSolverContainer.get() )
        GetDffManager().SolveSolver( *mxSolverContainer );
}

void XclImpObjectManager::ExtendUsedArea( ScRange& rScUsedArea, SCTAB nScTab ) const
{
    maEscherObjList.ExtendUsedArea( rScUsedArea, nScTab );
}

// private --------------------------------------------------------------------

void XclImpObjectManager::AppendEscherObj( XclImpEscherObj* pEscherObj )
{
    if( mbStartWithDummy )    // insert dummy object for invalid first shape
    {
        maEscherObjList.AppendDummyObj( new XclImpEscherObj( GetRoot() ) );
        mbStartWithDummy = false;
    }
    maEscherObjList.AppendObj( pEscherObj );
}

void XclImpObjectManager::ReplaceEscherObj( XclImpEscherObj* pEscherObj )
{
    maEscherObjList.ReplaceLastObj( pEscherObj );
}

void XclImpObjectManager::ReadObjFtCmo( XclImpStream& rStrm )
{
    XclImpEscherObj* pEscherObj = GetLastEscherObjAcc();
    if( !pEscherObj )
        return;

    sal_uInt16 nObjType, nObjId, nFlags;
    rStrm >> nObjType >> nObjId >> nFlags;

    pEscherObj->SetObjId( nObjId );
    pEscherObj->SetPrintable(::get_flag( nFlags, EXC_OBJ_CMO_PRINTABLE) );

    switch( nObjType )
    {
        case EXC_OBJ_CMO_GROUP:
        case EXC_OBJ_CMO_LINE:
        case EXC_OBJ_CMO_ARC:
            ReplaceEscherObj( new XclImpEscherDrawing( *pEscherObj, false ) );
        break;
        case EXC_OBJ_CMO_RECTANGLE:
        case EXC_OBJ_CMO_ELLIPSE:
        case EXC_OBJ_CMO_POLYGON:
        case EXC_OBJ_CMO_DRAWING:
            ReplaceEscherObj( new XclImpEscherDrawing( *pEscherObj, true ) );
        break;
        case EXC_OBJ_CMO_TEXT:
            ReplaceEscherObj( new XclImpEscherTxo( *pEscherObj ) );
        break;
        case EXC_OBJ_CMO_NOTE:
            ReplaceEscherObj( new XclImpEscherNote( *pEscherObj ) );
        break;
        case EXC_OBJ_CMO_BUTTON:
        case EXC_OBJ_CMO_CHECKBOX:
        case EXC_OBJ_CMO_OPTIONBUTTON:
        case EXC_OBJ_CMO_LABEL:
        case EXC_OBJ_CMO_GROUPBOX:
        case EXC_OBJ_CMO_LISTBOX:
        case EXC_OBJ_CMO_COMBOBOX:
        case EXC_OBJ_CMO_SPIN:
        case EXC_OBJ_CMO_SCROLLBAR:
            ReplaceEscherObj( new XclImpEscherTbxCtrl( *pEscherObj, nObjType ) );
        break;
        case EXC_OBJ_CMO_PICTURE:
            ReplaceEscherObj( new XclImpEscherOle( *pEscherObj ) );
        break;
        case EXC_OBJ_CMO_CHART:
            ReplaceEscherObj( new XclImpEscherChart( *pEscherObj, false ) );
        break;
        case EXC_OBJ_CMO_EDIT:          // only in dialogs
        case EXC_OBJ_CMO_DIALOG:        // not supported
            pEscherObj->SetSkip();
        break;
        default:
            DBG_ERROR1( "XclImpObjectManager::ReadObjFtCmo - unknown object type 0x%04hX", nObjType );
            ReplaceEscherObj( new XclImpEscherDrawing( *pEscherObj, true ) );
    }
}

void XclImpObjectManager::ReadObjFtPioGrbit( XclImpStream& rStrm )
{
    if( XclImpEscherOle* pOleObj = PTR_CAST( XclImpEscherOle, GetLastEscherObj() ) )
        pOleObj->ReadPioGrbit( rStrm );
    else
        DBG_ERRORFILE( "XclImpObjectManager::ReadObjFtPioGrbit - no OLE object" );
}

void XclImpObjectManager::ReadObjFtPictFmla( XclImpStream& rStrm, sal_uInt16 nRecSize )
{
    if( XclImpEscherOle* pOleObj = PTR_CAST( XclImpEscherOle, GetLastEscherObj() ) )
        pOleObj->ReadPictFmla( rStrm, nRecSize );
    else
        DBG_ERRORFILE( "XclImpObjectManager::ReadObjFtPictFmla - no OLE object" );
}

void XclImpObjectManager::ReadObjTbxSubRec( XclImpStream& rStrm, sal_uInt16 nSubRecId )
{
    if( XclImpEscherTbxCtrl* pCtrlObj = PTR_CAST( XclImpEscherTbxCtrl, GetLastEscherObj() ) )
    {
        switch( nSubRecId )
        {
            case EXC_ID_OBJ_FTCBLS:     pCtrlObj->ReadCbls( rStrm );        break;
            case EXC_ID_OBJ_FTLBSDATA:  pCtrlObj->ReadLbsData( rStrm );     break;
            case EXC_ID_OBJ_FTSBSFMLA:  // equal to ftCblsFmla
            case EXC_ID_OBJ_FTCBLSFMLA: pCtrlObj->ReadCblsFmla( rStrm );    break;
            case EXC_ID_OBJ_FTSBS:      pCtrlObj->ReadSbs( rStrm );         break;
            case EXC_ID_OBJ_FTGBODATA:  pCtrlObj->ReadGboData( rStrm );     break;
            case EXC_ID_OBJ_FTMACRO:    pCtrlObj->ReadMacro( rStrm );       break;

            default:    DBG_ERRORFILE( "XclImpObjectManager::ReadObjTbxSubRec - unknown subrecord" );
        }
    }
}

XclImpOcxConverter& XclImpObjectManager::GetOcxConverter()
{
    if( !mxOcxConverter.get() )
        mxOcxConverter.reset( new XclImpOcxConverter( GetRoot() ) );
    return *mxOcxConverter;
}

SvxMSDffSolverContainer& XclImpObjectManager::GetSolverContainer()
{
    if( !mxSolverContainer.get() )
        mxSolverContainer.reset( new SvxMSDffSolverContainer );
    return *mxSolverContainer;
}

SvxMSDffConnectorRule* XclImpObjectManager::GetFirstConnectorRule()
{
    void* pRule = mxSolverContainer.get() ? mxSolverContainer->aCList.First() : 0;
    return static_cast< SvxMSDffConnectorRule* >( pRule );
}

SvxMSDffConnectorRule* XclImpObjectManager::GetNextConnectorRule()
{
    void* pRule = mxSolverContainer.get() ? mxSolverContainer->aCList.Next() : 0;
    return static_cast< SvxMSDffConnectorRule* >( pRule );
}

// Escher property set helper =================================================

XclImpEscherPropSet::XclImpEscherPropSet( const XclImpDffManager& rDffManager ) :
    maPropReader( rDffManager )
{
}

void XclImpEscherPropSet::Read( XclImpStream& rStrm )
{
    sal_uInt32 nPropSetSize;

    rStrm.PushPosition();
    rStrm.Ignore( 4 );
    rStrm >> nPropSetSize;
    rStrm.PopPosition();

    mxMemStrm.reset( new SvMemoryStream );
    rStrm.CopyToStream( *mxMemStrm, 8 + nPropSetSize );
    mxMemStrm->Seek( STREAM_SEEK_TO_BEGIN );
    maPropReader.ReadPropSet( *mxMemStrm, 0 );
}

sal_uInt32 XclImpEscherPropSet::GetPropertyValue( sal_uInt16 nPropId, sal_uInt32 nDefault ) const
{
    return maPropReader.GetPropertyValue( nPropId, nDefault );
}

void XclImpEscherPropSet::FillToItemSet( SfxItemSet& rItemSet ) const
{
    if( mxMemStrm.get() )
        maPropReader.ApplyAttributes( *mxMemStrm, rItemSet );
}

XclImpStream& operator>>( XclImpStream& rStrm, XclImpEscherPropSet& rPropSet )
{
    rPropSet.Read( rStrm );
    return rStrm;
}

// ============================================================================

