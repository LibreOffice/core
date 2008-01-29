/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xiescher.cxx,v $
 *
 *  $Revision: 1.55 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:27:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#include <stdio.h>

#include "xiescher.hxx"

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/XEmbedPersist.hpp>
#include <com/sun/star/awt/VisualEffect.hpp>
#include <com/sun/star/awt/ScrollBarOrientation.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/form/binding/XBindableValue.hpp>
#include <com/sun/star/form/binding/XValueBinding.hpp>
#include <com/sun/star/form/binding/XListEntrySink.hpp>
#include <com/sun/star/form/binding/XListEntrySource.hpp>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>

#include <rtl/logfile.hxx>
#include <sfx2/objsh.hxx>
#include <svtools/moduleoptions.hxx>
#include <svtools/fltrcfg.hxx>
#include <comphelper/types.hxx>
#include <comphelper/classids.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <basic/sbstar.hxx>
#include <basic/sbmod.hxx>
#include <basic/sbmeth.hxx>

#include <svx/svdobj.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>
#include <svx/editobj.hxx>
#include <svx/outliner.hxx>
#include <svx/outlobj.hxx>
#include <svx/svdouno.hxx>
#include <svx/unoapi.hxx>
#include <svx/svditer.hxx>
#include <svx/writingmodeitem.hxx>
#include <svx/svdoedge.hxx>

#include "scitems.hxx"
#include <svx/eeitem.hxx>
#include <svx/colritem.hxx>
#include <svx/xflclit.hxx>
#include <svx/adjitem.hxx>

#include "document.hxx"
#include "drwlayer.hxx"
#include "userdat.hxx"
#include "chartarr.hxx"
#include "detfunc.hxx"
#include "unonames.hxx"
#include "convuno.hxx"
#include "postit.hxx"
#include "globstr.hrc"

#include "fprogressbar.hxx"
#include "xlocx.hxx"
#include "xltracer.hxx"
#include "xistream.hxx"
#include "xihelper.hxx"
#include "xiformula.hxx"
#include "xilink.hxx"
#include "xistyle.hxx"
#include "xipage.hxx"
#include "xichart.hxx"
#include "xicontent.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::beans::NamedValue;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::frame::XModel;
using ::com::sun::star::awt::XControlModel;
using ::com::sun::star::embed::XEmbeddedObject;
using ::com::sun::star::embed::XEmbedPersist;
using ::com::sun::star::form::binding::XBindableValue;
using ::com::sun::star::form::binding::XValueBinding;
using ::com::sun::star::form::binding::XListEntrySink;
using ::com::sun::star::form::binding::XListEntrySource;
using ::com::sun::star::script::ScriptEventDescriptor;
using ::com::sun::star::table::CellAddress;
using ::com::sun::star::table::CellRangeAddress;

/** helper class which mimics the auto_ptr< SdrObject > semantics, but calls SdrObject::Free instead
    of deleting the SdrObject directly
*/
class SdrObjectPtr
{
private:
    SdrObject*  mpObject;

public:
    SdrObjectPtr() : mpObject( NULL ) { }
    SdrObjectPtr( SdrObject* _pObject ) : mpObject( _pObject ) { }
    ~SdrObjectPtr() { free(); }

    const SdrObject* operator->() const { return mpObject; }
          SdrObject* operator->()       { return mpObject; }

    void reset( SdrObject* _pObject = NULL ) { free(); mpObject = _pObject; }

    SdrObject* get() const { return mpObject; }
    SdrObject& operator*() { return *mpObject; }

    SdrObject* release() { SdrObject* pRet = get(); mpObject = NULL; return pRet; }

    SdrObjectPtr& operator=( SdrObjectPtr& _rLHS )
    {
        SdrObject* pNew = _rLHS.release();
        reset( pNew );
        return *this;
    }

private:
    SdrObjectPtr( const SdrObjectPtr& );            // not implemented

private:
    void free() { SdrObject::Free( mpObject ); }
};

// Text box data ==============================================================

namespace {

SvxAdjust lclGetSvxHorAlignment( sal_uInt8 nXclAlign )
{
    SvxAdjust eHorAlign = SVX_ADJUST_LEFT;
    switch( nXclAlign )
    {
        case EXC_TXO_HOR_LEFT:      eHorAlign = SVX_ADJUST_LEFT;    break;
        case EXC_TXO_HOR_CENTER:    eHorAlign = SVX_ADJUST_CENTER;  break;
        case EXC_TXO_HOR_RIGHT:     eHorAlign = SVX_ADJUST_RIGHT;   break;
        case EXC_TXO_HOR_JUSTIFY:   eHorAlign = SVX_ADJUST_BLOCK;   break;
    }
    return eHorAlign;
}

SdrTextVertAdjust lclGetSvxVerAlignment( sal_uInt8 nXclAlign )
{
    SdrTextVertAdjust eVerAlign = SDRTEXTVERTADJUST_TOP;
    switch( nXclAlign )
    {
        case EXC_TXO_VER_TOP:       eVerAlign = SDRTEXTVERTADJUST_TOP;      break;
        case EXC_TXO_VER_CENTER:    eVerAlign = SDRTEXTVERTADJUST_CENTER;   break;
        case EXC_TXO_VER_BOTTOM:    eVerAlign = SDRTEXTVERTADJUST_BOTTOM;   break;
        case EXC_TXO_VER_JUSTIFY:   eVerAlign = SDRTEXTVERTADJUST_BLOCK;    break;
    }
    return eVerAlign;
}

::com::sun::star::text::WritingMode lclGetApiWritingMode( sal_uInt16 nXclOrient )
{
    // this is only a fake, drawing does not support real text orientation
    namespace csst = ::com::sun::star::text;
    csst::WritingMode eWriteMode = csst::WritingMode_LR_TB;
    switch( nXclOrient )
    {
        case EXC_TXO_TEXTROT_NONE:      eWriteMode = csst::WritingMode_LR_TB;   break;
        case EXC_TXO_TEXTROT_STACKED:   eWriteMode = csst::WritingMode_TB_RL;   break;
        case EXC_TXO_TEXTROT_90_CCW:    eWriteMode = csst::WritingMode_TB_RL;   break;
        case EXC_TXO_TEXTROT_90_CW:     eWriteMode = csst::WritingMode_TB_RL;   break;
    }
    return eWriteMode;
}

} // namespace

// ----------------------------------------------------------------------------

XclImpTxoData::XclImpTxoData( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot )
{
}

void XclImpTxoData::ReadTxo( XclImpStream& rStrm )
{
    mxString.reset();

    // Step 1: TXO record
    rStrm >> maData;

    // Step 2: First CONTINUE with string
    bool bValid = true;
    if( maData.mnTextLen > 0 )
    {
        bValid = (rStrm.GetNextRecId() == EXC_ID_CONT) && rStrm.StartNextRecord();
        DBG_ASSERT( bValid, "XclImpTxoData::ReadTxo - missing CONTINUE record" );
        if( bValid )
            mxString.reset( new XclImpString( rStrm.ReadUniString( maData.mnTextLen ) ) );
    }

    // Step 3: Second CONTINUE with formatting runs
    if( maData.mnFormatSize > 0 )
    {
        bValid = (rStrm.GetNextRecId() == EXC_ID_CONT) && rStrm.StartNextRecord();
        DBG_ASSERT( bValid, "XclImpTxoData::ReadTxo - missing CONTINUE record" );
        if( bValid && mxString.is() )
        {
            // number of formatting runs, each takes 8 bytes, but ignore last run
            sal_uInt16 nFormatRuns = maData.mnFormatSize / 8 - 1;
            for( sal_uInt16 nRun = 0; nRun < nFormatRuns; ++nRun )
            {
                sal_uInt16 nChar, nFont;
                rStrm >> nChar >> nFont;
                rStrm.Ignore( 4 );
                mxString->AppendFormat( nChar, nFont );
            }
        }
    }
}

void XclImpTxoData::ProcessSdrObject( SdrObject& rSdrObj ) const
{
    if( SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( &rSdrObj ) )
    {
        if( mxString.is() )
        {
            if( mxString->IsRich() )
            {
                // rich text
                ::std::auto_ptr< EditTextObject > xEditObj(
                    XclImpStringHelper::CreateTextObject( GetRoot(), *mxString ) );
                OutlinerParaObject* pOutlineObj = new OutlinerParaObject( *xEditObj );
                pOutlineObj->SetOutlinerMode( OUTLINERMODE_TEXTOBJECT );
                // text object takes ownership of the outliner object
                pTextObj->NbcSetOutlinerParaObject( pOutlineObj );
            }
            else
            {
                // plain text
                pTextObj->SetText( mxString->GetText() );
            }
        }

        // #i12188# text alignment not stored in Escher stream, but in TXO record
        SvxAdjust eHorAlign = lclGetSvxHorAlignment( maData.GetXclHorAlignment() );
        pTextObj->SetMergedItem( SvxAdjustItem( eHorAlign, EE_PARA_JUST ) );
        SdrTextVertAdjust eVerAlign = lclGetSvxVerAlignment( maData.GetXclVerAlignment() );
        pTextObj->SetMergedItem( SdrTextVertAdjustItem( eVerAlign ) );
        ::com::sun::star::text::WritingMode eWriteMode = lclGetApiWritingMode( GetOrientation() );
        pTextObj->SetMergedItem( SvxWritingModeItem( eWriteMode, SDRATTR_TEXTDIRECTION ) );
    }
}

// Escher objects =============================================================

XclImpDrawObjBase::XclImpDrawObjBase( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot ),
    maObjId( rRoot.GetCurrScTab(), EXC_OBJ_INVALID_ID ),
    mnObjType( EXC_OBJ_CMO_UNKNOWN ),
    mnObjFlags( 0 ),
    mnShapeId( 0 ),
    mnShapeFlags( 0 ),
    mnShapeBlipId( 0 ),
    mbValid( true ),
    mbAreaObj( false ),
    mbInsSdr( true )
{
}

XclImpDrawObjBase::~XclImpDrawObjBase()
{
}

XclImpDrawObjRef XclImpDrawObjBase::ReadObjCmo( XclImpStream& rStrm )
{
    XclImpDrawObjRef xDrawObj;

    sal_uInt16 nObjType, nObjId, nFlags;
    rStrm >> nObjType >> nObjId >> nFlags;

    switch( nObjType )
    {
        case EXC_OBJ_CMO_GROUP:
        case EXC_OBJ_CMO_LINE:
        case EXC_OBJ_CMO_ARC:
            xDrawObj.reset( new XclImpDrawingObj( rStrm.GetRoot(), false ) );
        break;
        case EXC_OBJ_CMO_RECTANGLE:
        case EXC_OBJ_CMO_ELLIPSE:
        case EXC_OBJ_CMO_POLYGON:
        case EXC_OBJ_CMO_DRAWING:
        case EXC_OBJ_CMO_TEXT:
            xDrawObj.reset( new XclImpDrawingObj( rStrm.GetRoot(), true ) );
        break;
        case EXC_OBJ_CMO_NOTE:
            xDrawObj.reset( new XclImpNoteObj( rStrm.GetRoot() ) );
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
            xDrawObj.reset( new XclImpTbxControlObj( rStrm.GetRoot() ) );
        break;
        case EXC_OBJ_CMO_PICTURE:
            xDrawObj.reset( new XclImpOleObj( rStrm.GetRoot() ) );
        break;
        case EXC_OBJ_CMO_CHART:
            xDrawObj.reset( new XclImpChartObj( rStrm.GetRoot(), false ) );
        break;
        case EXC_OBJ_CMO_EDIT:          // only in dialogs
        case EXC_OBJ_CMO_DIALOG:        // not supported
        break;
        default:
            DBG_ERROR1( "XclImpDrawObjBase::ReadObjCmo - unknown object type 0x%04hX", nObjType );
            xDrawObj.reset( new XclImpDrawingObj( rStrm.GetRoot(), true ) );
    }

    if( xDrawObj.is() )
        xDrawObj->SetObjData( nObjType, nObjId, nFlags );
    return xDrawObj;
}

void XclImpDrawObjBase::ReadSubRecord( XclImpStream& rStrm, sal_uInt16 nSubRecId, sal_uInt16 /*nSubRecSize*/ )
{
    switch( nSubRecId )
    {
        case EXC_ID_OBJ_FTMACRO:
            ReadMacro( rStrm );
        break;
    }
}

void XclImpDrawObjBase::ReadClientAnchor( SvStream& rEscherStrm, const DffRecordHeader& rHeader )
{
    DBG_ASSERT( rHeader.nRecType == DFF_msofbtClientAnchor, "XclImpDrawObjBase::ReadClientAnchor - no client anchor record" );
    mxAnchor.reset( new XclEscherAnchor( GetScTab() ) );
    rHeader.SeekToContent( rEscherStrm );
    rEscherStrm.SeekRel( 2 );
    rEscherStrm >> *mxAnchor;
}

void XclImpDrawObjBase::SetObjData( sal_uInt16 nObjType, sal_uInt16 nObjId, sal_uInt16 nObjFlags )
{
    maObjId.mnObjId = nObjId;
    mnObjType = nObjType;
    mnObjFlags = nObjFlags;
}

void XclImpDrawObjBase::SetShapeData( sal_uInt32 nShapeId, sal_uInt32 nShapeFlags, sal_uInt32 nShapeBlipId )
{
    mnShapeId = nShapeId;
    mnShapeFlags = nShapeFlags;
    mnShapeBlipId = nShapeBlipId;
}

void XclImpDrawObjBase::SetClientAnchor( const XclEscherAnchor& rAnchor )
{
    mxAnchor.reset( new XclEscherAnchor( rAnchor ) );
}

bool XclImpDrawObjBase::IsValidSize( const Rectangle& rAnchorRect ) const
{
    // XclEscherAnchor rounds up the width, width of 3 is the result of an Excel width of 0
    return mbAreaObj ?
        ((rAnchorRect.GetWidth() > 3) && (rAnchorRect.GetHeight() > 1)) :
        ((rAnchorRect.GetWidth() > 3) || (rAnchorRect.GetHeight() > 1));
}

ScRange XclImpDrawObjBase::GetUsedArea() const
{
    ScRange aScUsedArea( ScAddress::INITIALIZE_INVALID );
    if( mxAnchor.is() )
    {
        // #i44077# object inserted -> update used area for OLE object import
        if( GetAddressConverter().ConvertRange( aScUsedArea, mxAnchor->maXclRange, GetScTab(), GetScTab(), false ) )
        {
            // reduce range, if object ends directly on borders between two columns or rows
            if( (mxAnchor->mnRX == 0) && (aScUsedArea.aStart.Col() < aScUsedArea.aEnd.Col()) )
                aScUsedArea.aEnd.IncCol( -1 );
            if( (mxAnchor->mnBY == 0) && (aScUsedArea.aStart.Row() < aScUsedArea.aEnd.Row()) )
                aScUsedArea.aEnd.IncRow( -1 );
        }
    }
    return aScUsedArea;
}

Rectangle XclImpDrawObjBase::GetAnchorRect() const
{
    Rectangle aAnchorRect;
    if( mxAnchor.is() )
        aAnchorRect = mxAnchor->GetRect( GetDoc(), MAP_100TH_MM );
    return aAnchorRect;
}

sal_Size XclImpDrawObjBase::GetProgressSize() const
{
    return DoGetProgressSize();
}

void XclImpDrawObjBase::ProcessSdrObject( SdrObject& rSdrObj ) const
{
    // call virtual function for object type specific processing
    DoProcessSdrObj( rSdrObj );
}

void XclImpDrawObjBase::ReadMacro( XclImpStream& rStrm )
{
    maMacroName.Erase();
    if( rStrm.GetRecLeft() > 6 )
    {
        // macro is stored in a tNameXR token containing a link to a defined name
        sal_uInt16 nFmlaSize;
        rStrm >> nFmlaSize;
        rStrm.Ignore( 4 );
        DBG_ASSERT( nFmlaSize == 7, "XclImpDrawObjBase::ReadMacro - unexpected formula size" );
        if( nFmlaSize == 7 )
        {
            sal_uInt8 nTokenId;
            sal_uInt16 nExtSheet, nExtName;
            rStrm >> nTokenId >> nExtSheet >> nExtName;
            DBG_ASSERT( nTokenId == XclTokenArrayHelper::GetTokenId( EXC_TOKID_NAMEX, EXC_TOKCLASS_REF ),
                "XclImpDrawObjBase::ReadMacro - tNameXR token expected" );
            if( nTokenId == XclTokenArrayHelper::GetTokenId( EXC_TOKID_NAMEX, EXC_TOKCLASS_REF ) )
            {
                maMacroName = GetLinkManager().GetMacroName( nExtSheet, nExtName );
                // #i38718# missing module name - try to find the macro in the imported modules
                if( maMacroName.Len() && (maMacroName.Search( '.' ) == STRING_NOTFOUND) )
                    if( SfxObjectShell* pDocShell = GetDocShell() )
                        if( StarBASIC* pBasic = pDocShell->GetBasic() )
                            if( SbMethod* pMethod = dynamic_cast< SbMethod* >( pBasic->Find( maMacroName, SbxCLASS_METHOD ) ) )
                                if( SbModule* pModule = pMethod->GetModule() )
                                    maMacroName.Insert( '.', 0 ).Insert( pModule->GetName(), 0 );
            }
        }
    }
}

sal_Size XclImpDrawObjBase::DoGetProgressSize() const
{
    return 1;
}

void XclImpDrawObjBase::DoProcessSdrObj( SdrObject& /*rSdrObj*/ ) const
{
    // trace if object is not printable
    if( !IsPrintable() )
        GetTracer().TraceObjectNotPrintable();
}

// ----------------------------------------------------------------------------

XclImpDrawingObj::XclImpDrawingObj( const XclImpRoot& rRoot, bool bAreaObj ) :
    XclImpDrawObjBase( rRoot )
{
    SetAreaObj( bAreaObj );
}

void XclImpDrawingObj::DoProcessSdrObj( SdrObject& rSdrObj ) const
{
    // set text data
    if( mxTxoData.is() )
        mxTxoData->ProcessSdrObject( rSdrObj );
    // base class processing
    XclImpDrawObjBase::DoProcessSdrObj( rSdrObj );
}

// ----------------------------------------------------------------------------

XclImpNoteObj::XclImpNoteObj( const XclImpRoot& rRoot ) :
    XclImpDrawingObj( rRoot, true ),
    maScPos( ScAddress::INITIALIZE_INVALID ),
    mnNoteFlags( 0 )
{
    // note object will be processed, but not inserted into the draw page
    SetSkipInsertSdr();
}

void XclImpNoteObj::SetNoteData( const ScAddress& rScPos, sal_uInt16 nNoteFlags )
{
    maScPos = rScPos;
    mnNoteFlags = nNoteFlags;
}

void XclImpNoteObj::DoProcessSdrObj( SdrObject& rSdrObj ) const
{
    if( maScPos.IsValid() )
    {
        if( const XclImpString* pString = GetString() )
        {
            SCCOL nScCol = maScPos.Col();
            SCROW nScRow = maScPos.Row();
            SCTAB nScTab = GetScTab();
            bool bVisible = ::get_flag( mnNoteFlags, EXC_NOTE_VISIBLE );

            // create the note object
            ::std::auto_ptr< EditTextObject > pEditObj(
                XclImpStringHelper::CreateNoteObject( GetRoot(), *pString ) );
            // ScPostIt does not take ownership of the passed note
            ScPostIt aNote( pEditObj.get(), GetDocPtr() );
            aNote.SetRectangle( rSdrObj.GetSnapRect() );
            aNote.SetShown( bVisible );

            // get the actual container from this group object
            SdrObject* pBoxSdrObj = &rSdrObj;
            if( rSdrObj.IsGroupObject() && rSdrObj.GetSubList() )
            {
                SdrObjListIter aIt( *rSdrObj.GetSubList() );
                pBoxSdrObj = aIt.Next();
            }

            // set textbox properties
            if( pBoxSdrObj )
            {
                XclImpDrawingObj::DoProcessSdrObj( *pBoxSdrObj );
                pBoxSdrObj->SetMergedItem( SdrTextAutoGrowWidthItem( false ) );
                pBoxSdrObj->SetMergedItem( SdrTextAutoGrowHeightItem( false ) );
                aNote.SetAndApplyItemSet( pBoxSdrObj->GetMergedItemSet() );
            }

            // insert the note into the document
            GetDoc().SetNote( nScCol, nScRow, nScTab, aNote );

            // make the note visible via ScDetectiveFunc
            if( bVisible )
            {
                ScDetectiveFunc aDetFunc( GetDocPtr(), nScTab );
                aDetFunc.ShowComment( nScCol, nScRow, TRUE );
            }
        }
    }
}

// ----------------------------------------------------------------------------

namespace {

void lclReadRangeList( ScRangeList& rScRanges, XclImpStream& rStrm )
{
    XclTokenArray aXclTokArr;
    aXclTokArr.ReadSize( rStrm );
    rStrm.Ignore( 4 );
    aXclTokArr.ReadArray( rStrm );
    rStrm.GetRoot().GetFormulaCompiler().CreateRangeList( rScRanges, EXC_FMLATYPE_CONTROL, aXclTokArr, rStrm );
}

} // namespace

XclImpControlObjHelper::XclImpControlObjHelper( XclCtrlBindMode eBindMode ) :
    meBindMode( eBindMode )
{
}

XclImpControlObjHelper::~XclImpControlObjHelper()
{
}

void XclImpControlObjHelper::ReadCellLinkFormula( XclImpStream& rStrm )
{
    ScRangeList aScRanges;
    lclReadRangeList( aScRanges, rStrm );
    if( aScRanges.Count() > 0 )
    {
        // Use first cell of first range
        ::std::auto_ptr< ScRange > xScRange( aScRanges.Remove( 0UL ) );
        if( xScRange.get() )
            mxCellLink.reset( new ScAddress( xScRange->aStart ) );
    }
}

void XclImpControlObjHelper::ReadSrcRangeFormula( XclImpStream& rStrm )
{
    ScRangeList aScRanges;
    lclReadRangeList( aScRanges, rStrm );
    // Use first range
    if( aScRanges.Count() > 0 )
        mxSrcRange.reset( aScRanges.Remove( 0UL ) );
}

ScfPropertySet XclImpControlObjHelper::GetControlPropSet() const
{
    return ScfPropertySet( mxCtrlModel );
}

void XclImpControlObjHelper::ConvertSheetLinks( const XclImpRoot& rRoot, SdrObject& /* rSdrObj */ ) const
{
    // get service factory from Calc document
    Reference< XMultiServiceFactory > xFactory;
    if( SfxObjectShell* pDocShell = rRoot.GetDocShell() )
        xFactory.set( pDocShell->GetModel(), UNO_QUERY );
    if( !mxCtrlModel.is() || !xFactory.is() )
        return;

    // *** cell link *** ------------------------------------------------------

    Reference< XBindableValue > xBindable( mxCtrlModel, UNO_QUERY );
    if( mxCellLink.is() && xBindable.is() )
    {
        // create argument sequence for createInstanceWithArguments()
        CellAddress aApiAddress;
        ScUnoConversion::FillApiAddress( aApiAddress, *mxCellLink );

        NamedValue aValue;
        aValue.Name = CREATE_OUSTRING( SC_UNONAME_BOUNDCELL );
        aValue.Value <<= aApiAddress;

        Sequence< Any > aArgs( 1 );
        aArgs[ 0 ] <<= aValue;

        // create the CellValueBinding instance
        Reference< XInterface > xInt;
        try
        {
            switch( meBindMode )
            {
                case EXC_CTRL_BINDCONTENT:
                    xInt = xFactory->createInstanceWithArguments(
                        CREATE_OUSTRING( SC_SERVICENAME_VALBIND ), aArgs );
                break;
                case EXC_CTRL_BINDPOSITION:
                    xInt = xFactory->createInstanceWithArguments(
                        CREATE_OUSTRING( SC_SERVICENAME_LISTCELLBIND ), aArgs );
                break;
                default:
                    DBG_ERRORFILE( "XclImpOcxConverter::ConvertSheetLinks - unknown binding mode" );
            }
        }
        catch( const Exception& )
        {
        }

        // set the binding at the control
        Reference< XValueBinding > xBinding( xInt, UNO_QUERY );
        if( xBinding.is() )
            xBindable->setValueBinding( xBinding );
    }

    // *** source range *** ---------------------------------------------------

    Reference< XListEntrySink > xEntrySink( mxCtrlModel, UNO_QUERY );
    if( mxSrcRange.is() && xEntrySink.is() )
    {
        // create argument sequence for createInstanceWithArguments()
        CellRangeAddress aApiRange;
        ScUnoConversion::FillApiRange( aApiRange, *mxSrcRange );

        NamedValue aValue;
        aValue.Name = CREATE_OUSTRING( SC_UNONAME_CELLRANGE );
        aValue.Value <<= aApiRange;

        Sequence< Any > aArgs( 1 );
        aArgs[ 0 ] <<= aValue;

        // create the CellValueBinding instance
        Reference< XInterface > xInt;
        try
        {
            xInt = xFactory->createInstanceWithArguments(
                CREATE_OUSTRING( SC_SERVICENAME_LISTSOURCE ), aArgs );
        }
        catch( const Exception& )
        {
        }

        // set the binding at the control
        Reference< XListEntrySource > xEntrySource( xInt, UNO_QUERY );
        if( xEntrySource.is() )
            xEntrySink->setListEntrySource( xEntrySource );
    }
}

// ----------------------------------------------------------------------------

XclImpTbxControlObj::XclImpTbxControlObj( const XclImpRoot& rRoot ) :
    XclImpDrawingObj( rRoot, true ),
    XclImpControlObjHelper( EXC_CTRL_BINDPOSITION ),
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

void XclImpTbxControlObj::ReadSubRecord( XclImpStream& rStrm, sal_uInt16 nSubRecId, sal_uInt16 nSubRecSize )
{
    switch( nSubRecId )
    {
        case EXC_ID_OBJ_FTCBLS:
            ReadCbls( rStrm );
        break;
        case EXC_ID_OBJ_FTLBSDATA:
            ReadLbsData( rStrm );
        break;
        case EXC_ID_OBJ_FTSBSFMLA:  // equal to ftCblsFmla
        case EXC_ID_OBJ_FTCBLSFMLA:
            ReadCblsFmla( rStrm );
        break;
        case EXC_ID_OBJ_FTSBS:
            ReadSbs( rStrm );
        break;
        case EXC_ID_OBJ_FTGBODATA:
            ReadGboData( rStrm );
        break;
        default:
            XclImpDrawObjBase::ReadSubRecord( rStrm, nSubRecId, nSubRecSize );
    }
}

OUString XclImpTbxControlObj::GetServiceName() const
{
    return XclControlObjHelper::GetTbxServiceName( GetObjType() );
}

bool XclImpTbxControlObj::FillMacroDescriptor( ScriptEventDescriptor& rEvent ) const
{
    if( GetMacroName().Len() > 0 )
    {
        // type of action is dependent on control type
        rEvent.ListenerType = XclControlObjHelper::GetTbxListenerType( GetObjType() );
        rEvent.EventMethod = XclControlObjHelper::GetTbxEventMethod( GetObjType() );
        if( (rEvent.ListenerType.getLength() > 0) && (rEvent.EventMethod.getLength() > 0) )
        {
            // set the macro name
            rEvent.ScriptType = XclControlObjHelper::GetTbxScriptType();
            rEvent.ScriptCode = XclControlObjHelper::GetScMacroName( GetMacroName() );
            return true;
        }
    }
    return false;
}

void XclImpTbxControlObj::DoProcessSdrObj( SdrObject& rSdrObj ) const
{
    // do not call DoProcessSdrObj() from base class to skip text processing

    ScfPropertySet aPropSet = GetControlPropSet();
    if( !aPropSet.Is() )
        return;

    namespace AwtVisualEffect = ::com::sun::star::awt::VisualEffect;
    namespace AwtScrollOrient = ::com::sun::star::awt::ScrollBarOrientation;
    using ::com::sun::star::style::VerticalAlignment_MIDDLE;

    // control name, printable, sheet links -----------------------------------

    // control printable?
    aPropSet.SetBoolProperty( CREATE_OUSTRING( "Printable" ), IsPrintable() );

    // #118053# #i51348# set internal name of the control (use name from SdrObject, if extant)
    OUString aCtrlName = rSdrObj.GetName();
    if( aCtrlName.getLength() == 0 )
        aCtrlName = XclControlObjHelper::GetTbxControlName( GetObjType() );
    if( aCtrlName.getLength() > 0 )
        aPropSet.SetProperty( CREATE_OUSTRING( "Name" ), aCtrlName );

    // sheet links
    ConvertSheetLinks( GetRoot(), rSdrObj );

    // control label ----------------------------------------------------------

    if( const XclImpString* pString = GetString() )
    {
        // the visible label (caption)
        aPropSet.SetStringProperty( CREATE_OUSTRING( "Label" ), pString->GetText() );

        // font properties
        const XclFormatRunVec& rFormatRuns = pString->GetFormats();
        if( !rFormatRuns.empty() )
            GetFontBuffer().WriteFontProperties(
                aPropSet, EXC_FONTPROPSET_CONTROL, rFormatRuns.front().mnFontIdx );
    }

    // special control contents -----------------------------------------------

    switch( GetObjType() )
    {
        // checkbox/option button

        case EXC_OBJ_CMO_CHECKBOX:
        case EXC_OBJ_CMO_OPTIONBUTTON:
        {
            bool bCheckBox = GetObjType() == EXC_OBJ_CMO_CHECKBOX;

            sal_Int16 nApiState = 0;
            switch( mnState )
            {
                case EXC_OBJ_CBLS_STATE_UNCHECK:    nApiState = 0;                  break;
                case EXC_OBJ_CBLS_STATE_CHECK:      nApiState = 1;                  break;
                case EXC_OBJ_CBLS_STATE_TRI:        nApiState = bCheckBox ? 2 : 1;  break;
            }
            if( bCheckBox )
                aPropSet.SetBoolProperty( CREATE_OUSTRING( "TriState" ), nApiState == 2 );
            aPropSet.SetProperty( CREATE_OUSTRING( "DefaultState" ), nApiState );

            sal_Int16 nApiBorder = mbFlatButton ? AwtVisualEffect::FLAT : AwtVisualEffect::LOOK3D;
            aPropSet.SetProperty( CREATE_OUSTRING( "VisualEffect" ), nApiBorder );

            // #i40279# always centered vertically
            aPropSet.SetProperty( CREATE_OUSTRING( "VerticalAlign" ), VerticalAlignment_MIDDLE );
        }
        break;

        // listbox/combobox

        case EXC_OBJ_CMO_LISTBOX:
        case EXC_OBJ_CMO_COMBOBOX:
        {
            sal_Int16 nApiBorder = mbFlatBorder ? AwtVisualEffect::FLAT : AwtVisualEffect::LOOK3D;
            aPropSet.SetProperty( CREATE_OUSTRING( "Border" ), nApiBorder );

            Sequence< sal_Int16 > aSelection;

            switch( GetObjType() )
            {
                case EXC_OBJ_CMO_LISTBOX:
                {
                    // selection type
                    bool bMultiSel = (mnSelType != EXC_OBJ_LBS_SEL_SIMPLE);
                    aPropSet.SetBoolProperty( CREATE_OUSTRING( "MultiSelection" ), bMultiSel );

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
                    aPropSet.SetBoolProperty( CREATE_OUSTRING( "Dropdown" ), true );
                    // dropdown line count
                    aPropSet.SetProperty( CREATE_OUSTRING( "LineCount" ), mnLineCount );
                    // selection
                    if( mnSelEntry > 0 )
                    {
                        aSelection.realloc( 1 );
                        aSelection[ 0 ] = mnSelEntry - 1;
                    }
                }
                break;
            }

            if( !HasCellLink() && aSelection.getLength() )
                aPropSet.SetProperty( CREATE_OUSTRING( "DefaultSelection" ), aSelection );
        }
        break;

        // spin button

        case EXC_OBJ_CMO_SPIN:
        {
            // Calc's "Border" property is not the 3D/flat style effect in Excel (#i34712#)
            aPropSet.SetProperty( CREATE_OUSTRING( "Border" ), AwtVisualEffect::NONE );
            aPropSet.SetProperty< sal_Int32 >( CREATE_OUSTRING( "SpinValueMin" ), mnScrollMin );
            aPropSet.SetProperty< sal_Int32 >( CREATE_OUSTRING( "SpinValueMax" ), mnScrollMax );
            aPropSet.SetProperty< sal_Int32 >( CREATE_OUSTRING( "SpinIncrement" ), mnScrollStep );
            // Excel spin buttons always vertical
            aPropSet.SetProperty( CREATE_OUSTRING( "Orientation" ), AwtScrollOrient::VERTICAL );
            aPropSet.SetProperty< sal_Int32 >( CREATE_OUSTRING( "DefaultSpinValue" ), mnScrollValue );
        }
        break;

        // scrollbar

        case EXC_OBJ_CMO_SCROLLBAR:
        {
            sal_Int32 nApiOrient = mbScrollHor ? AwtScrollOrient::HORIZONTAL : AwtScrollOrient::VERTICAL;
            sal_Int32 nVisSize = std::min< sal_Int32 >( static_cast< sal_Int32 >( mnScrollPage ), 1 );

            // Calc's "Border" property is not the 3D/flat style effect in Excel (#i34712#)
            aPropSet.SetProperty( CREATE_OUSTRING( "Border" ), AwtVisualEffect::NONE );
            aPropSet.SetProperty< sal_Int32 >( CREATE_OUSTRING( "ScrollValueMin" ), mnScrollMin );
            aPropSet.SetProperty< sal_Int32 >( CREATE_OUSTRING( "ScrollValueMax" ), mnScrollMax );
            aPropSet.SetProperty< sal_Int32 >( CREATE_OUSTRING( "LineIncrement" ), mnScrollStep );
            aPropSet.SetProperty< sal_Int32 >( CREATE_OUSTRING( "BlockIncrement" ), mnScrollPage );
            aPropSet.SetProperty( CREATE_OUSTRING( "VisibleSize" ), nVisSize );
            aPropSet.SetProperty( CREATE_OUSTRING( "Orientation" ), nApiOrient );
            aPropSet.SetProperty< sal_Int32 >( CREATE_OUSTRING( "DefaultScrollValue" ), mnScrollValue );
        }
        break;
    }
}

void XclImpTbxControlObj::ReadCbls( XclImpStream& rStrm )
{
    sal_uInt16 nStyle;
    rStrm >> mnState;
    rStrm.Ignore( 8 );
    rStrm >> nStyle;
    mnState &= EXC_OBJ_CBLS_STATEMASK;
    mbFlatButton = ::get_flag( nStyle, EXC_OBJ_CBLS_FLAT );
}

void XclImpTbxControlObj::ReadCblsFmla( XclImpStream& rStrm )
{
    ReadCellLinkFormula( rStrm );
}

void XclImpTbxControlObj::ReadLbsData( XclImpStream& rStrm )
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

    switch( GetObjType() )
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
                SetInvalid();
        break;
    }
}

void XclImpTbxControlObj::ReadSbs( XclImpStream& rStrm )
{
    sal_uInt16 nOrient, nStyle;
    rStrm.Ignore( 4 );
    rStrm >> mnScrollValue >> mnScrollMin >> mnScrollMax >> mnScrollStep >> mnScrollPage >> nOrient;
    rStrm.Ignore( 2 );
    rStrm >> nStyle;

    mbScrollHor = ::get_flag( nOrient, EXC_OBJ_SBS_HORIZONTAL );
    mbFlatButton = ::get_flag( nStyle, EXC_OBJ_SBS_FLAT );
}

void XclImpTbxControlObj::ReadGboData( XclImpStream& rStrm )
{
    sal_uInt16 nStyle;
    rStrm.Ignore( 4 );
    rStrm >> nStyle;
    mbFlatBorder = ::get_flag( nStyle, EXC_OBJ_GBO_FLAT );
}

// ----------------------------------------------------------------------------

XclImpOleObj::XclImpOleObj( const XclImpRoot& rRoot ) :
    XclImpDrawObjBase( rRoot ),
    XclImpControlObjHelper( EXC_CTRL_BINDCONTENT ),
    mnCtlsStrmPos( 0 ),
    mbAsSymbol( false ),
    mbLinked( false ),
    mbControl( false ),
    mbUseCtlsStrm( false )
{
    SetAreaObj( true );
}

void XclImpOleObj::ReadSubRecord( XclImpStream& rStrm, sal_uInt16 nSubRecId, sal_uInt16 nSubRecSize )
{
    switch( nSubRecId )
    {
        case EXC_ID_OBJ_FTPIOGRBIT:
            ReadPioGrbit( rStrm );
        break;
        case EXC_ID_OBJ_FTPICTFMLA:
            ReadPictFmla( rStrm, nSubRecSize );
        break;
        default:
            XclImpDrawObjBase::ReadSubRecord( rStrm, nSubRecId, nSubRecSize );
    }
}

void XclImpOleObj::DoProcessSdrObj( SdrObject& rSdrObj ) const
{
    if( IsControl() )
    {
        // do not call XclImpDrawObjBase::DoProcessSdrObj(), it would trace missing "printable" feature

        ScfPropertySet aPropSet = GetControlPropSet();
        // printable
        aPropSet.SetBoolProperty( CREATE_OUSTRING( "Printable" ), IsPrintable() );
        // #118053# #i51348# set name from SdrObject as internal name of the control
        if( rSdrObj.GetName().Len() > 0 )
            aPropSet.SetStringProperty( CREATE_OUSTRING( "Name" ), rSdrObj.GetName() );
        // sheet links
        ConvertSheetLinks( GetRoot(), rSdrObj );
    }
    else
    {
        // trace missing "printable" feature
        XclImpDrawObjBase::DoProcessSdrObj( rSdrObj );

        SfxObjectShell* pDocShell = GetDocShell();
        SdrOle2Obj* pOleSdrObj = dynamic_cast< SdrOle2Obj* >( &rSdrObj );
        if( pOleSdrObj && pDocShell )
        {
            comphelper::EmbeddedObjectContainer& rEmbObjCont = pDocShell->GetEmbeddedObjectContainer();
            Reference< XEmbeddedObject > xEmbObj = pOleSdrObj->GetObjRef();
            OUString aOldName( pOleSdrObj->GetPersistName() );

            /*  The object persistence should be already in the storage, but
                the object still might not be inserted into the container. */
            if( rEmbObjCont.HasEmbeddedObject( aOldName ) )
            {
                if( !rEmbObjCont.HasEmbeddedObject( xEmbObj ) )
                    // filter code is allowed to call the following method
                    rEmbObjCont.AddEmbeddedObject( xEmbObj, aOldName );
            }
            else
            {
                /*  If the object is still not in container it must be iserted
                    there, the name must be generated in this case. */
                OUString aNewName;
                rEmbObjCont.InsertEmbeddedObject( xEmbObj, aNewName );
                if( aOldName != aNewName )
                    // #95381# SetPersistName, not SetName
                    pOleSdrObj->SetPersistName( aNewName );
            }
        }
    }
}

void XclImpOleObj::ReadPioGrbit( XclImpStream& rStrm )
{
    sal_uInt16 nFlags;
    rStrm >> nFlags;
    mbAsSymbol = ::get_flag( nFlags, EXC_OBJ_PIO_SYMBOL );
    mbLinked = ::get_flag( nFlags, EXC_OBJ_PIO_LINKED );
    mbControl = ::get_flag( nFlags, EXC_OBJ_PIO_CONTROL );
    mbUseCtlsStrm = ::get_flag( nFlags, EXC_OBJ_PIO_CTLSSTREAM );
}

void XclImpOleObj::ReadPictFmla( XclImpStream& rStrm, sal_uInt16 nRecSize )
{
    sal_uInt32 nStorageId = 0;
    sal_uInt16 nFmlaLen;
    rStrm >> nFmlaLen;

    // --- linked ---

    if( mbLinked )
    {
        DBG_ASSERT( nFmlaLen + 2 == nRecSize, "XclImpOleObj::ReadPictFmla - bad linked size" );
        rStrm.Ignore( 7 );
        sal_uInt16 nXti, nExtName;
        rStrm >> nXti >> nExtName;
        const XclImpExtName* pExtName = GetLinkManager().GetExternName( nXti, nExtName );
        bool bOk = pExtName && (pExtName->GetType() == xlExtOLE);
        DBG_ASSERT( bOk, "XclImpOleObj::ReadPictFmla - EXTERNNAME not found or not OLE" );
        if( bOk )
            nStorageId = pExtName->GetStorageId();
    }

    // --- embedded ---

    else if( nFmlaLen + 2 < nRecSize )  // #107158# ignore picture links (are embedded OLE obj's too)
    {
        String aUserName;
        sal_Size nPos0 = rStrm.GetRecPos();        // fmla start

        sal_uInt16 n16;
        rStrm >> n16;     // should be 5 but who knows ...
        DBG_ASSERT( n16 + 4 <= nFmlaLen, "XclImpOleObj::ReadPictFmla - embedded length mismatch" );
        if( n16 + 4 <= nFmlaLen )
        {
            rStrm.Ignore( n16 + 4 );
            sal_uInt8 n8;
            rStrm >> n8;
            DBG_ASSERT( n8 == 0x03, "XclImpOleObj::ReadPictFmla - no name start" );
            if( n8 == 0x03 )
            {
                rStrm >> n16;     // string length
                if( n16 )
                {   // the 4th way Xcl stores a unicode string: not even a Grbit byte present if length 0
                    aUserName = rStrm.ReadUniString( n16 );
                    // 0:= ID follows, 1:= pad byte + ID
#ifndef PRODUCT
                    sal_sSize nLeft = static_cast< sal_sSize >( nFmlaLen + nPos0 - rStrm.GetRecPos() );
                    DBG_ASSERT( (nLeft == 0) || (nLeft == 1), "XclImpOleObj::ReadPictFmla - unknown left over" );
#endif
                }
            }
        }
        rStrm.Seek( nPos0 + nFmlaLen );
        rStrm >> nStorageId;

        if( IsControl() )
        {
            mnCtlsStrmPos = static_cast< sal_Size >( nStorageId );
            nStorageId = 0;

            if( aUserName.EqualsAscii( "Forms.HTML:Hidden.1" ) )
            {
                // #i26521# form controls to be ignored
                SetInvalid();
            }
            else if( rStrm.GetRecLeft() > 8 )
            {
                // data size in 'Ctls'
                rStrm.Ignore( 4 );

                // a class ID (16-bit string), e.g. for progress bar control
                sal_uInt32 nClassIdSize;
                rStrm >> nClassIdSize;
                DBG_ASSERT( rStrm.GetRecLeft() >= nClassIdSize + 4, "XclImpOleObj::ReadPictFmla - missing data" );
                if( rStrm.GetRecLeft() >= nClassIdSize + 4 )
                {
                    rStrm.Ignore( nClassIdSize );

                    // read additional link data
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
        }
    }

    if( nStorageId != 0 )
    {
        if( mbLinked )
            maStorageName = EXC_STORAGE_OLE_LINKED;
        else
            maStorageName = EXC_STORAGE_OLE_EMBEDDED;
        sal_Char aBuf[ 2 * sizeof( sal_uInt32 ) + 1 ];
        sprintf( aBuf, "%08X", static_cast< unsigned int >( nStorageId ) );    // #100211# - checked
        maStorageName.AppendAscii( aBuf );
    }
}

// ----------------------------------------------------------------------------

XclImpChartObj::XclImpChartObj( const XclImpRoot& rRoot, bool bOwnTab ) :
    XclImpDrawObjBase( rRoot ),
    mbOwnTab( bOwnTab )
{
    SetAreaObj( true );
}

void XclImpChartObj::ReadObj5( XclImpStream& rStrm )
{
    const XclImpPalette& rPal = GetPalette();

    // area formatting
    sal_uInt8 nBackIdx, nPattIdx, nPattern, nAreaFlags;
    rStrm >> nBackIdx >> nPattIdx >> nPattern >> nAreaFlags;
    mxAreaFmt.reset( new XclChAreaFormat );
    mxAreaFmt->maPattColor = rPal.GetColor( nPattIdx );
    mxAreaFmt->maBackColor = rPal.GetColor( nBackIdx );
    mxAreaFmt->mnPattern = nPattern;
    ::set_flag( mxAreaFmt->mnFlags, EXC_CHAREAFORMAT_AUTO, nAreaFlags & 1 );

    // line formatting
    sal_uInt8 nLineIdx, nLineType, nLineWeight, nLineFlags;
    rStrm >> nLineIdx >> nLineType >> nLineWeight >> nLineFlags;
    mxLineFmt.reset( new XclChLineFormat );
    mxLineFmt->maColor = rPal.GetColor( nLineIdx );
    switch( nLineType )
    {
        case 0:     mxLineFmt->mnPattern = EXC_CHLINEFORMAT_SOLID;      break;
        case 1:     mxLineFmt->mnPattern = EXC_CHLINEFORMAT_DASH;       break;
        case 2:     mxLineFmt->mnPattern = EXC_CHLINEFORMAT_DOT;        break;
        case 3:     mxLineFmt->mnPattern = EXC_CHLINEFORMAT_DASHDOT;    break;
        case 4:     mxLineFmt->mnPattern = EXC_CHLINEFORMAT_DASHDOTDOT; break;
        case 5:     mxLineFmt->mnPattern = EXC_CHLINEFORMAT_DARKTRANS;  break;
        case 6:     mxLineFmt->mnPattern = EXC_CHLINEFORMAT_MEDTRANS;   break;
        case 7:     mxLineFmt->mnPattern = EXC_CHLINEFORMAT_LIGHTTRANS; break;
        case 255:   mxLineFmt->mnPattern = EXC_CHLINEFORMAT_NONE;       break;
        default:    mxLineFmt->mnPattern = EXC_CHLINEFORMAT_SOLID;
    }
    switch( nLineWeight )
    {
        case 0:     mxLineFmt->mnWeight = EXC_CHLINEFORMAT_HAIR;    break;
        case 1:     mxLineFmt->mnWeight = EXC_CHLINEFORMAT_SINGLE;  break;
        case 2:     mxLineFmt->mnWeight = EXC_CHLINEFORMAT_DOUBLE;  break;
        case 3:     mxLineFmt->mnWeight = EXC_CHLINEFORMAT_TRIPLE;  break;
        default:    mxLineFmt->mnWeight = EXC_CHLINEFORMAT_HAIR;
    }
    ::set_flag( mxLineFmt->mnFlags, EXC_CHLINEFORMAT_AUTO, nLineFlags & 1 );
}

void XclImpChartObj::ReadChartSubStream( XclImpStream& rStrm )
{
    DBG_ASSERT( rStrm.GetRecId() == EXC_ID5_BOF, "XclImpChartObj::ReadChartSubStream - no BOF record found" );
    if( rStrm.GetRecId() == EXC_ID5_BOF )
    {
        mxChart.reset( new XclImpChart( GetRoot(), mbOwnTab ) );
        mxChart->ReadChartSubStream( rStrm );
        if( mxLineFmt.is() && mxAreaFmt.is() )
            mxChart->SetChartFrameFormat( *mxLineFmt, *mxAreaFmt );
        if( mbOwnTab )
            FinalizeTabChart();
    }
}

SdrObject* XclImpChartObj::CreateSdrObject( const Rectangle& rAnchorRect, ScfProgressBar& rProgress ) const
{
    SdrObjectPtr xSdrObj;
    SfxObjectShell* pDocShell = GetDocShell();
    if( SvtModuleOptions().IsChart() && pDocShell && mxChart.is() )
    {
        // create embedded chart object
        OUString aEmbObjName;
        Reference< XEmbeddedObject > xEmbObj = pDocShell->GetEmbeddedObjectContainer().
                CreateEmbeddedObject( SvGlobalName( SO3_SCH_CLASSID ).GetByteSequence(), aEmbObjName );

        /*  Set the size to the embedded object, this prevents that font sizes
            of text objects are changed in the chart when the object is
            inserted into the draw page. */
        sal_Int64 nAspect = ::com::sun::star::embed::Aspects::MSOLE_CONTENT;
        MapUnit aUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xEmbObj->getMapUnit( nAspect ) );
        Size aSize( Window::LogicToLogic( rAnchorRect.GetSize(), MapMode( MAP_100TH_MM ), MapMode( aUnit ) ) );
        ::com::sun::star::awt::Size aAwtSize;
        aAwtSize.Width = aSize.Width();
        aAwtSize.Height = aSize.Height();
        xEmbObj->setVisualAreaSize( nAspect, aAwtSize );

        // create the container OLE object
        SdrOle2Obj* pSdrOleObj = new SdrOle2Obj( svt::EmbeddedObjectRef( xEmbObj, nAspect ), aEmbObjName, rAnchorRect );
        xSdrObj.reset( pSdrOleObj );
        xSdrObj->NbcSetLayer( SC_LAYER_FRONT );

        // convert Excel chart to OOo Chart
        if( svt::EmbeddedObjectRef::TryRunningState( xEmbObj ) )
        {
            Reference< XModel > xModel( xEmbObj->getComponent(), UNO_QUERY );
            mxChart->Convert( xModel, rProgress );

            Reference< XEmbedPersist > xPers( xEmbObj, UNO_QUERY );
            if( xPers.is() )
                xPers->storeOwn();
        }
    }

    return xSdrObj.release();
}

sal_Size XclImpChartObj::DoGetProgressSize() const
{
    return mxChart.is() ? mxChart->GetProgressSize() : 0;
}

void XclImpChartObj::FinalizeTabChart()
{
    /*  #i44077# Calculate and store Escher anchor for sheet charts.
        Needed to get used area if this chart is inserted as OLE object. */
    DBG_ASSERT( mbOwnTab, "XclImpChartObj::FinalizeTabChart - not allowed for embedded chart objects" );

    // set uninitialized page to landscape
    if( !GetPageSettings().GetPageData().mbValid )
        GetPageSettings().SetPaperSize( EXC_PAPERSIZE_DEFAULT, false );

    // calculate size of the chart object
    const XclPageData& rPageData = GetPageSettings().GetPageData();
    Size aPaperSize( rPageData.GetScPaperSize() );

    long nWidth = XclTools::GetHmmFromTwips( aPaperSize.Width() );
    long nHeight = XclTools::GetHmmFromTwips( aPaperSize.Height() );

    // subtract page margins, give 1cm extra space
    nWidth -= (XclTools::GetHmmFromInch( rPageData.mfLeftMargin + rPageData.mfRightMargin ) + 2000);
    nHeight -= (XclTools::GetHmmFromInch( rPageData.mfTopMargin + rPageData.mfBottomMargin ) + 1000);

    // print column/row headers?
    if( rPageData.mbPrintHeadings )
    {
        nWidth -= 2000;
        nHeight -= 1000;
    }

    // create the Escher anchor
    XclEscherAnchor aAnchor( GetScTab() );
    aAnchor.SetRect( GetDoc(), Rectangle( 1000, 500, nWidth, nHeight ), MAP_100TH_MM );
    SetClientAnchor( aAnchor );
}

// Escher stream conversion ===================================================

void XclImpSolverContainer::ReadSolverContainer( SvStream& rEscherStrm )
{
    rEscherStrm >> *this;
}

void XclImpSolverContainer::InsertSdrObjectInfo( const XclImpDrawObjBase& rDrawObj, SdrObject* pSdrObj )
{
    sal_uInt32 nShapeId = rDrawObj.GetShapeId();
    if( (nShapeId != 0) && pSdrObj )
        maSdrInfoMap[ nShapeId ].Set( pSdrObj, rDrawObj.GetShapeFlags() );
}

void XclImpSolverContainer::RemoveSdrObjectInfo( const XclImpDrawObjBase& rDrawObj )
{
    maSdrInfoMap.erase( rDrawObj.GetShapeId() );
}

void XclImpSolverContainer::UpdateConnectorRules()
{
    for( SvxMSDffConnectorRule* pRule = GetFirstRule(); pRule; pRule = GetNextRule() )
    {
        UpdateConnection( pRule->nShapeA, pRule->pAObj, &pRule->nSpFlagsA );
        UpdateConnection( pRule->nShapeB, pRule->pBObj, &pRule->nSpFlagsB );
        UpdateConnection( pRule->nShapeC, pRule->pCObj );
    }
}

void XclImpSolverContainer::RemoveConnectorRules()
{
    // base class from SVX uses plain untyped tools/List
    for( SvxMSDffConnectorRule* pRule = GetFirstRule(); pRule; pRule = GetNextRule() )
        delete pRule;
    aCList.Clear();
}

SvxMSDffConnectorRule* XclImpSolverContainer::GetFirstRule()
{
    return static_cast< SvxMSDffConnectorRule* >( aCList.First() );
}

SvxMSDffConnectorRule* XclImpSolverContainer::GetNextRule()
{
    return static_cast< SvxMSDffConnectorRule* >( aCList.Next() );
}

void XclImpSolverContainer::UpdateConnection( sal_uInt32 nShapeId, SdrObject*& rpSdrObj, sal_uInt32* pnShapeFlags )
{
    XclImpSdrInfoMap::const_iterator aIt = maSdrInfoMap.find( nShapeId );
    if( aIt != maSdrInfoMap.end() )
    {
        rpSdrObj = aIt->second.mpSdrObj;
        if( pnShapeFlags )
            *pnShapeFlags = aIt->second.mnShapeFlags;
    }
}

// ----------------------------------------------------------------------------

XclImpDffManager::XclImpDffManager(
        const XclImpRoot& rRoot, XclImpObjectManager& rObjManager, SvStream& rEscherStrm ) :
    SvxMSDffManager( rEscherStrm, rRoot.GetBasePath(), 0, 0, rRoot.GetDoc().GetDrawLayer(), 1440, COL_DEFAULT, 24, 0, &rRoot.GetTracer().GetBaseTracer() ),
    XclImpRoot( rRoot ),
    mrObjManager( rObjManager ),
    mxOcxConverter( new XclImpOcxConverter( rRoot ) ),
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

XclImpDffManager::~XclImpDffManager()
{
}

void XclImpDffManager::StartProgressBar( sal_Size nProgressSize )
{
    mxProgress.reset( new ScfProgressBar( GetDocShell(), STR_PROGRESS_CALCULATING ) );
    mxProgress->AddSegment( nProgressSize );
    mxProgress->Activate();
}

void XclImpDffManager::ProcessDrawingGroup( SvStream& rEscherStrm )
{
    rEscherStrm.Seek( STREAM_SEEK_TO_BEGIN );
    DffRecordHeader aHeader;
    rEscherStrm >> aHeader;
    if( aHeader.nRecType == DFF_msofbtDggContainer )
        ProcessDggContainer( rEscherStrm, aHeader );
    else
    {
        DBG_ERRORFILE( "XclImpDffManager::ProcessDrawingGroup - unexpected record" );
    }
}

void XclImpDffManager::ProcessDrawing( SvStream& rEscherStrm, sal_Size nStrmPos )
{
    rEscherStrm.Seek( nStrmPos );
    DffRecordHeader aHeader;
    rEscherStrm >> aHeader;
    if( aHeader.nRecType == DFF_msofbtDgContainer )
        ProcessDgContainer( rEscherStrm, aHeader );
    else
    {
        DBG_ERRORFILE( "XclImpDffManager::ProcessDrawing - unexpected record" );
    }
}

void XclImpDffManager::ProcessTabChart( const XclImpChartObj& rChartObj )
{
    Rectangle aAnchorRect = rChartObj.GetAnchorRect();
    if( rChartObj.IsValidSize( aAnchorRect ) )
    {
        SdrObjectPtr xSdrObj( CreateSdrObject( rChartObj, aAnchorRect ) );
        InsertSdrObject( rChartObj, xSdrObj.release() );
        UpdateUsedArea( rChartObj );
    }
}

ScRange XclImpDffManager::GetUsedArea( SCTAB nScTab ) const
{
    ScRange aScUsedArea( ScAddress::INITIALIZE_INVALID );
    ScRangeMap::const_iterator aIt = maUsedAreaMap.find( nScTab );
    if( aIt != maUsedAreaMap.end() )
        aScUsedArea = aIt->second;
    return aScUsedArea;
}

// virtual functions ----------------------------------------------------------

void XclImpDffManager::ProcessClientAnchor2( SvStream& rEscherStrm,
        DffRecordHeader& rHeader, void* /*pClientData*/, DffObjData& rObjData )
{
    // find the OBJ record data related to the processed shape
    if( XclImpDrawObjBase* pDrawObj = mrObjManager.FindDrawObj( rObjData.rSpHd ).get() )
    {
        pDrawObj->ReadClientAnchor( rEscherStrm, rHeader );
        rObjData.aChildAnchor = pDrawObj->GetAnchorRect();
        rObjData.bChildAnchor = sal_True;
    }
}

SdrObject* XclImpDffManager::ProcessObj( SvStream& rEscherStrm,
        DffObjData& rObjData, void* pClientData, Rectangle& /*rTextRect*/, SdrObject* pOldSdrObj )
{
    /*  pOldSdrObj passes a generated SdrObject. This function owns this object
        and can modify it. The function has either to return it back to caller
        or to delete it by itself. */
    SdrObjectPtr xSdrObj( pOldSdrObj );

    // find the OBJ record data related to the processed shape
    XclImpDrawObjRef xDrawObj = mrObjManager.FindDrawObj( rObjData.rSpHd );
    const Rectangle& rAnchorRect = rObjData.aChildAnchor;

    // #102378# Do not process the global page group shape (flag SP_FPATRIARCH)
    bool bGlobalPageGroup = ::get_flag< sal_uInt32 >( rObjData.nSpFlags, SP_FPATRIARCH );
    if( !xDrawObj || !xDrawObj->IsValid() || bGlobalPageGroup )
        return 0;   // simply return, xSdrObj will be destroyed

    /*  Pass pointer to top-level object back to caller. If the processed
        object is embedded in a group, the pointer is already set to the
        top-level parent object. */
    XclImpDrawObjBase** ppTopLevelObj = reinterpret_cast< XclImpDrawObjBase** >( pClientData );
    bool bIsTopLevel = !ppTopLevelObj || !*ppTopLevelObj;
    if( ppTopLevelObj && bIsTopLevel )
        *ppTopLevelObj = xDrawObj.get();

    // #119010# connectors don't have to be area objects
    if( dynamic_cast< SdrEdgeObj* >( xSdrObj.get() ) )
        xDrawObj->SetAreaObj( false );

    /*  Check for valid size for all objects. Needed to ignore lots of invisible
        phantom objects from deleted rows or columns (for performance reasons).
        #i30816# Include objects embedded in groups.
        #i58780# Ignore group shapes, size is not initialized. */
    bool bEmbeddedGroup = !bIsTopLevel && dynamic_cast< SdrObjGroup* >( xSdrObj.get() );
    if( !bEmbeddedGroup && !xDrawObj->IsValidSize( rAnchorRect ) )
        return 0;   // simply return, xSdrObj will be destroyed

    // set shape information from Escher stream
    xDrawObj->SetShapeData( rObjData.nShapeId, rObjData.nSpFlags, GetPropertyValue( DFF_Prop_pib ) );

    /*  Connect textbox data (string, alignment, text orientation) to object.
        #98132# don't ask for a text-ID, Escher export doesn't set one. */
    if( XclImpDrawingObj* pDrawingObj = dynamic_cast< XclImpDrawingObj* >( xDrawObj.get() ) )
        pDrawingObj->SetTxoData( mrObjManager.FindTxoData( rObjData.rSpHd ) );

    // try to create a custom SdrObject that overwrites the passed object
    SdrObjectPtr xNewSdrObj( CreateCustomSdrObject( *xDrawObj, rAnchorRect ) );
    if( xNewSdrObj.get() )
    {
        // copy old shape name to new shape, if it does not have an own name
        if( xNewSdrObj->GetName().Len() == 0 )
        {
            if( xSdrObj.get() )
                xNewSdrObj->SetName( xSdrObj->GetName() );
            else
                xNewSdrObj->SetName( GetPropertyString( DFF_Prop_wzName, rEscherStrm ) );
        }
        // transfer ownership to xSdrObj
        xSdrObj = xNewSdrObj;
    }
    else if( xSdrObj.get() && (dynamic_cast< XclImpDrawingObj* >( xDrawObj.get() ) || dynamic_cast< XclImpOleObj* >( xDrawObj.get() )) )
    {
        // macro name and hyperlink for drawing objects and pictures (not for controls and charts)
        const String& rMacro = xDrawObj->GetMacroName();
#ifdef ISSUE66550_HLINK_FOR_SHAPES
        OUString aHlink = ReadHlinkProperty( rEscherStrm );
        if( (rMacro.Len() > 0) || (aHlink.getLength() > 0) )
#else
        if( rMacro.Len() > 0 )
#endif
        {
            if( ScMacroInfo* pInfo = ScDrawLayer::GetMacroInfo( xSdrObj.get(), TRUE ) )
            {
                pInfo->SetMacro( XclControlObjHelper::GetScMacroName( rMacro ) );
#ifdef ISSUE66550_HLINK_FOR_SHAPES
                pInfo->SetHlink( aHlink );
#endif
            }
        }
    }

    // process the SdrObject
    if( xSdrObj.get() )
    {
        // maybe if there is no color, we could do this in ApplyAttributes (writer?, calc?)
        if( GetPropertyBool( DFF_Prop_fFilled ) && !IsProperty( DFF_Prop_fillColor ) )
            xSdrObj->SetMergedItem( XFillColorItem( EMPTY_STRING, Color( COL_WHITE ) ) );

        // automatic margin is handled by host
        if( GetPropertyBool( DFF_Prop_AutoTextMargin ) )
        {
            sal_Int32 nMargin = EXC_ESCHER_AUTOMARGIN;
            ScaleEmu( nMargin );
            xSdrObj->SetMergedItem( SdrTextLeftDistItem( nMargin ) );
            xSdrObj->SetMergedItem( SdrTextRightDistItem( nMargin ) );
            xSdrObj->SetMergedItem( SdrTextUpperDistItem( nMargin ) );
            xSdrObj->SetMergedItem( SdrTextLowerDistItem( nMargin ) );
        }

        // #i39167# full width for text and textless objects regardless of alignment
        xSdrObj->SetMergedItem( SdrTextHorzAdjustItem( SDRTEXTHORZADJUST_BLOCK ) );

        // additional processing on the SdrObject
        xDrawObj->ProcessSdrObject( *xSdrObj );

        // add the area used by this object to the internal map of used areas
        UpdateUsedArea( *xDrawObj );

        /*  If the SdrObject will not be inserted into the draw page, delete it
            here. Happens e.g. for notes: The ProcessSdrObject() call above has
            inserted the note into the document, and the SdrObject is not
            needed anymore. */
        if( !xDrawObj->IsInsertSdr() )
            xSdrObj.reset();
    }

    /*  Store the relation between shape ID and SdrObject for connectors. Must
        be done here (and not in InsertSdrObject() function), otherwise all
        SdrObjects embedded in groups would be lost. */
    if( xSdrObj.get() )
        maSolverCont.InsertSdrObjectInfo( *xDrawObj, xSdrObj.get() );

    return xSdrObj.release();
}

ULONG XclImpDffManager::Calc_nBLIPPos( ULONG /*nOrgVal*/, ULONG nStreamPos ) const
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

// private --------------------------------------------------------------------

OUString XclImpDffManager::ReadHlinkProperty( SvStream& rEscherStrm ) const
{
    /*  Reads hyperlink data from a complex Escher property. Contents of this
        property are equal to the HLINK record, import of this record is
        implemented in class XclImpHyperlink. This function has to create an
        instance of the XclImpStream class to be able to reuse the
        functionality of XclImpHyperlink. */
    OUString aString;
    sal_uInt32 nBufferSize = GetPropertyValue( DFF_Prop_pihlShape );
    if( (0 < nBufferSize) && (nBufferSize <= 0xFFFF) && SeekToContent( DFF_Prop_pihlShape, rEscherStrm ) )
    {
        // create a faked BIFF record that can be read by XclImpStream class
        SvMemoryStream aMemStream;
        aMemStream << sal_uInt16( 0 ) << static_cast< sal_uInt16 >( nBufferSize );

        // copy from Escher stream to memory stream
        ::std::vector< sal_uInt8 > aBuffer( nBufferSize );
        sal_uInt8* pnData = &aBuffer.front();
        if( rEscherStrm.Read( pnData, nBufferSize ) == nBufferSize )
        {
            aMemStream.Write( pnData, nBufferSize );

            // create BIFF import stream to be able to use XclImpHyperlink class
            XclImpStream aXclStrm( aMemStream, GetRoot() );
            if( aXclStrm.StartNextRecord() )
                aString = XclImpHyperlink::ReadEmbeddedData( aXclStrm );
        }
    }
    return aString;
}

void XclImpDffManager::ProcessDggContainer( SvStream& rEscherStrm, const DffRecordHeader& rDggHeader )
{
    // seek to end of drawing group container
    rDggHeader.SeekToEndOfRecord( rEscherStrm );
}

void XclImpDffManager::ProcessDgContainer( SvStream& rEscherStrm, const DffRecordHeader& rDgHeader )
{
    sal_Size nEndPos = rDgHeader.GetRecEndFilePos();
    while( rEscherStrm.Tell() < nEndPos )
    {
        DffRecordHeader aHeader;
        rEscherStrm >> aHeader;
        switch( aHeader.nRecType )
        {
            case DFF_msofbtSolverContainer:
                ProcessSolverContainer( rEscherStrm, aHeader );
            break;
            case DFF_msofbtSpgrContainer:
                ProcessShGrContainer( rEscherStrm, aHeader );
            break;
            default:
                aHeader.SeekToEndOfRecord( rEscherStrm );
        }
    }
    // seek to end of drawing page container
    rDgHeader.SeekToEndOfRecord( rEscherStrm );

    // #i12638# #i37900# connector rules
    maSolverCont.UpdateConnectorRules();
    SolveSolver( maSolverCont );
    maSolverCont.RemoveConnectorRules();
}

void XclImpDffManager::ProcessShGrContainer( SvStream& rEscherStrm, const DffRecordHeader& rShGrHeader )
{
    sal_Size nEndPos = rShGrHeader.GetRecEndFilePos();
    while( rEscherStrm.Tell() < nEndPos )
    {
        DffRecordHeader aHeader;
        rEscherStrm >> aHeader;
        switch( aHeader.nRecType )
        {
            case DFF_msofbtSpgrContainer:
            case DFF_msofbtSpContainer:
                ProcessShContainer( rEscherStrm, aHeader );
            break;
            default:
                aHeader.SeekToEndOfRecord( rEscherStrm );
        }
    }
    // seek to end of shape group container
    rShGrHeader.SeekToEndOfRecord( rEscherStrm );
}

void XclImpDffManager::ProcessSolverContainer( SvStream& rEscherStrm, const DffRecordHeader& rSolverHeader )
{
    // solver container wants to read the solver container header again
    rSolverHeader.SeekToBegOfRecord( rEscherStrm );
    // read the entire solver container
    rEscherStrm >> maSolverCont;
    // seek to end of solver container
    rSolverHeader.SeekToEndOfRecord( rEscherStrm );
}

void XclImpDffManager::ProcessShContainer( SvStream& rEscherStrm, const DffRecordHeader& rShHeader )
{
    rShHeader.SeekToBegOfRecord( rEscherStrm );
    Rectangle aDummy;
    const XclImpDrawObjBase* pDrawObj = 0;
    /*  The call to ImportObj() creates and returns a new SdrObject for the
        processed shape. This function takes ownership of the returned object.
        If the shape is a group object, all embedded objects are created
        recursively, and the returned group object contains them all.
        ImportObj() calls the virtual functions ProcessClientAnchor2() and
        ProcessObj() of the XclImpDffManager class. it writes the pointer to
        the related draw object data (OBJ record) into pDrawObj. */
    SdrObjectPtr xSdrObj( ImportObj( rEscherStrm, &pDrawObj, aDummy, aDummy, 0, 0 ) );
    if( pDrawObj && xSdrObj.get() )
        InsertSdrObject( *pDrawObj, xSdrObj.release() );
    rShHeader.SeekToEndOfRecord( rEscherStrm );
}

void XclImpDffManager::InsertSdrObject( const XclImpDrawObjBase& rDrawObj, SdrObject* pSdrObj )
{
    /*  Take ownership of the passed object. If insertion fails (e.g. rDrawObj
        states to skip insertion, or missing draw page), the object is
        automatically deleted. */
    SdrObjectPtr xSdrObj( pSdrObj );
    if( xSdrObj.get() && rDrawObj.IsInsertSdr() )
        if( SdrPage* pSdrPage = GetSdrPage( rDrawObj.GetScTab() ) )
            pSdrPage->InsertObject( xSdrObj.release() );
    // SdrObject still here? Insertion failed, remove data from shape ID map.
    if( xSdrObj.get() )
        maSolverCont.RemoveSdrObjectInfo( rDrawObj );
}

SdrObject* XclImpDffManager::CreateCustomSdrObject( XclImpDrawObjBase& rDrawObj, const Rectangle& rAnchorRect )
{
    SdrObjectPtr xSdrObj;
    if( XclImpOleObj* pOleObj = dynamic_cast< XclImpOleObj* >( &rDrawObj ) )
        xSdrObj.reset( CreateSdrObject( *pOleObj, rAnchorRect ) );
    else if( XclImpTbxControlObj* pTbxCtrlObj = dynamic_cast< XclImpTbxControlObj* >( &rDrawObj ) )
        xSdrObj.reset( CreateSdrObject( *pTbxCtrlObj, rAnchorRect ) );
    else if( XclImpChartObj* pChartObj = dynamic_cast< XclImpChartObj* >( &rDrawObj ) )
        xSdrObj.reset( CreateSdrObject( *pChartObj, rAnchorRect ) );
    else
        mxProgress->Progress();
    return xSdrObj.release();
}

SdrObject* XclImpDffManager::CreateSdrObject( XclImpOleObj& rOleObj, const Rectangle& rAnchorRect )
{
    SdrObjectPtr xSdrObj;
    if( rOleObj.IsControl() )
    {
        // form control objects are created by the XclImpOcxConverter class
        xSdrObj.reset( mxOcxConverter->CreateSdrObject( rOleObj, rAnchorRect ) );
    }
    else
    {
        // it is a normal OLE object
        SfxObjectShell* pDocShell = GetDocShell();
        const String& rStorageName = rOleObj.GetStorageName();
        if( pDocShell && rStorageName.Len() )
        {
            Graphic aGraph;
            Rectangle aVisArea;
            if( GetBLIP( rOleObj.GetShapeBlipId(), aGraph, &aVisArea ) )
            {
                SotStorageRef xSrcStrg = GetRootStorage();
                ErrCode nError = ERRCODE_NONE;
                xSdrObj.reset( CreateSdrOLEFromStorage( rStorageName, xSrcStrg,
                    pDocShell->GetStorage(), aGraph, rAnchorRect, aVisArea, 0, nError, mnOleImpFlags,
                    rOleObj.IsIconified()
                        ? ::com::sun::star::embed::Aspects::MSOLE_ICON
                        : ::com::sun::star::embed::Aspects::MSOLE_CONTENT ) );
            }
        }
    }
    mxProgress->Progress();
    return xSdrObj.release();
}

SdrObject* XclImpDffManager::CreateSdrObject( XclImpTbxControlObj& rTbxCtrlObj, const Rectangle& rAnchorRect )
{
    SdrObjectPtr xSdrObj( mxOcxConverter->CreateSdrObject( rTbxCtrlObj, rAnchorRect ) );
    mxProgress->Progress();
    return xSdrObj.release();
}

SdrObject* XclImpDffManager::CreateSdrObject( const XclImpChartObj& rChartObj, const Rectangle& rAnchorRect )
{
    return rChartObj.CreateSdrObject( rAnchorRect, *mxProgress );
}

void XclImpDffManager::UpdateUsedArea( const XclImpDrawObjBase& rDrawObj )
{
    ScRange aScObjArea = rDrawObj.GetUsedArea();
    if( aScObjArea.IsValid() )
    {
        ScRange* pScTabArea = 0;
        ScRangeMap::iterator aIt = maUsedAreaMap.find( rDrawObj.GetScTab() );
        if( aIt == maUsedAreaMap.end() )
        {
            pScTabArea = &maUsedAreaMap[ rDrawObj.GetScTab() ];
            pScTabArea->SetInvalid();
        }
        else
            pScTabArea = &aIt->second;

        if( pScTabArea )
            pScTabArea->ExtendTo( aScObjArea );
    }
}

// The object manager =========================================================

XclImpObjectManager::XclImpObjectManager( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot )
{
}

XclImpObjectManager::~XclImpObjectManager()
{
}

// *** Read Excel records *** -------------------------------------------------

void XclImpObjectManager::ReadObj5( XclImpStream& rStrm )
{
    DBG_ASSERT_BIFF( GetBiff() == EXC_BIFF5 );

    sal_uInt16 nObjType, nObjId, nFlags, nMacroSize;
    XclEscherAnchor aAnchor( GetCurrScTab() );

    rStrm.Ignore( 4 );
    rStrm >> nObjType >> nObjId >> nFlags >> aAnchor >> nMacroSize;
    rStrm.Ignore( 6 );

    // TODO: currently only charts are supported
    if( nObjType == EXC_OBJ_CMO_CHART )
    {
        // create and read chart object data
        XclImpChartObjRef xChartObj( new XclImpChartObj( GetRoot(), false ) );
        xChartObj->SetClientAnchor( aAnchor );
        xChartObj->ReadObj5( rStrm );
        if( StartChartSubStream( rStrm ) )
            xChartObj->ReadChartSubStream( rStrm );
        // no Escher stream in BIFF5 - abuse sheet chart functionality
        maTabCharts.push_back( xChartObj );
    }
}

void XclImpObjectManager::ReadMsoDrawingGroup( XclImpStream& rStrm )
{
    DBG_ASSERT_BIFF( GetBiff() == EXC_BIFF8 );
    // Excel continues this record with MSODRAWINGGROUP and CONTINUE records, hmm.
    rStrm.ResetRecord( true, EXC_ID_MSODRAWINGGROUP );
    ReadEscherRecord( rStrm );
}

void XclImpObjectManager::ReadMsoDrawing( XclImpStream& rStrm )
{
    DBG_ASSERT_BIFF( GetBiff() == EXC_BIFF8 );
    // disable internal CONTINUE handling
    rStrm.ResetRecord( false );
    /*  #i60510# real life: MSODRAWINGSELECTION record may contain garbage -
        this makes it impossible to process the Escher stream in one run.
        Store stream start position for every sheet separately, will be used
        to seek the stream to these positions later, when processing the next
        sheet. */
    size_t nTabIdx = static_cast< size_t >( GetCurrScTab() );
    if( nTabIdx >= maTabStrmPos.size() )
    {
        maTabStrmPos.resize( nTabIdx, STREAM_SEEK_TO_END );
        maTabStrmPos.push_back( maEscherStrm.Tell() );
    }
    // read leading MSODRAWING record
    ReadEscherRecord( rStrm );

    // read following drawing records, but do not start following unrelated record
    bool bLoop = true;
    while( bLoop ) switch( rStrm.GetNextRecId() )
    {
        case EXC_ID_MSODRAWING:
        case EXC_ID_MSODRAWINGSEL:
        case EXC_ID_CONT:
            rStrm.StartNextRecord();
            ReadEscherRecord( rStrm );
        break;
        case EXC_ID_OBJ:
            rStrm.StartNextRecord();
            ReadObj8( rStrm );
        break;
        case EXC_ID_TXO:
            rStrm.StartNextRecord();
            ReadTxo( rStrm );
        break;
        default:
            bLoop = false;
    }

    // re-enable internal CONTINUE handling
    rStrm.ResetRecord( true );
}

void XclImpObjectManager::ReadNote( XclImpStream& rStrm )
{
    DBG_ASSERT_BIFF( GetBiff() == EXC_BIFF8 );
    XclAddress aXclPos;
    sal_uInt16 nFlags, nObjId;
    rStrm >> aXclPos >> nFlags >> nObjId;

    if( nObjId != EXC_OBJ_INVALID_ID )
    {
        SCTAB nScTab = GetCurrScTab();
        ScAddress aScNotePos( ScAddress::UNINITIALIZED );
        if( GetAddressConverter().ConvertAddress( aScNotePos, aXclPos, nScTab, true ) )
            if( XclImpNoteObj* pNoteObj = dynamic_cast< XclImpNoteObj* >( FindDrawObj( XclObjId( nScTab, nObjId ) ).get() ) )
                pNoteObj->SetNoteData( aScNotePos, nFlags );
    }
}

void XclImpObjectManager::ReadTabChart( XclImpStream& rStrm )
{
    DBG_ASSERT_BIFF( GetBiff() >= EXC_BIFF5 );
    XclImpChartObjRef xChartObj( new XclImpChartObj( GetRoot(), true ) );
    xChartObj->ReadChartSubStream( rStrm );
    maTabCharts.push_back( xChartObj );
}

// *** Drawing objects *** ----------------------------------------------------

XclImpDrawObjRef XclImpObjectManager::FindDrawObj( const DffRecordHeader& rHeader ) const
{
    /*  maObjMap stores objects by position of the client data (OBJ record) in
        the Escher stream, which is always behind shape start position of the
        passed header. The function upper_bound() finds the first element in
        the map whose key is greater then the start position of the header. Its
        end position is used to test whether the found object is really related
        to the shape. */
    XclImpDrawObjRef xDrawObj;
    XclImpObjMap::const_iterator aIt = maObjMap.upper_bound( rHeader.GetRecBegFilePos() );
    if( (aIt != maObjMap.end()) && (aIt->first <= rHeader.GetRecEndFilePos()) )
        xDrawObj = aIt->second;
    return xDrawObj;
}

XclImpDrawObjRef XclImpObjectManager::FindDrawObj( const XclObjId& rObjId ) const
{
    XclImpDrawObjRef xDrawObj;
    XclImpObjMapById::const_iterator aIt = maObjMapId.find( rObjId );
    if( aIt != maObjMapId.end() )
        xDrawObj = aIt->second;
    return xDrawObj;
}

XclImpTxoDataRef XclImpObjectManager::FindTxoData( const DffRecordHeader& rHeader ) const
{
    /*  maTxoMap stores textbox data by position of the client data (TXO record)
        in the Escher stream, which is always behind shape start position of
        the passed header. The function upper_bound() finds the first element
        in the map whose key is greater then the start position of the header.
        Its end position is used to test whether the found object is really
        related to the shape. */
    XclImpTxoDataRef xTxoData;
    XclImpTxoMap::const_iterator aIt = maTxoMap.upper_bound( rHeader.GetRecBegFilePos() );
    if( (aIt != maTxoMap.end()) && (aIt->first <= rHeader.GetRecEndFilePos()) )
        xTxoData = aIt->second;
    return xTxoData;
}

void XclImpObjectManager::SetInvalidObj( SCTAB nScTab, sal_uInt16 nObjId )
{
    maInvalidObjs.push_back( XclObjId( nScTab, nObjId ) );
}

// *** Drawing object conversion *** ------------------------------------------

XclImpDffManager& XclImpObjectManager::GetDffManager()
{
    if( !mxDffManager )
        mxDffManager.reset( new XclImpDffManager( GetRoot(), *this, maEscherStrm ) );
    return *mxDffManager;
}

void XclImpObjectManager::ConvertObjects()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "sc", "dr104026", "XclImpObjectManager::ConvertObjects" );

    // do nothing if the document does not contain a drawing layer
    if( GetDoc().GetDrawLayer() )
    {
        // process list of identifiers of invalid objects
        for( XclObjIdVec::const_iterator aVIt = maInvalidObjs.begin(), aVEnd = maInvalidObjs.end(); aVIt != aVEnd; ++aVIt )
            if( XclImpDrawObjBase* pDrawObj = FindDrawObj( *aVIt ).get() )
                pDrawObj->SetInvalid();

        // get progress bar size for all valid objects
        sal_Size nProgressSize = GetProgressSize();
        if( nProgressSize > 0 )
        {
            XclImpDffManager& rDffManager = GetDffManager();
            rDffManager.StartProgressBar( nProgressSize );
            // process the global container, contains pictures
            if( !maTabStrmPos.empty() && (maTabStrmPos.front() > 0) )
                rDffManager.ProcessDrawingGroup( maEscherStrm );
            // process the sheet records, this inserts the objects into the drawing layer
            for( StreamPosVec::const_iterator aPIt = maTabStrmPos.begin(), aPEnd = maTabStrmPos.end(); aPIt != aPEnd; ++aPIt )
                if( *aPIt != STREAM_SEEK_TO_END )
                    rDffManager.ProcessDrawing( maEscherStrm, *aPIt );
            // chart sheets
            for( XclImpChartObjList::const_iterator aLIt = maTabCharts.begin(), aLEnd = maTabCharts.end(); aLIt != aLEnd; ++aLIt )
                rDffManager.ProcessTabChart( **aLIt );
        }
    }
}

ScRange XclImpObjectManager::GetUsedArea( SCTAB nScTab ) const
{
    ScRange aScUsedArea( ScAddress::INITIALIZE_INVALID );
    if( mxDffManager.is() )
        aScUsedArea = mxDffManager->GetUsedArea( nScTab );
    return aScUsedArea;
}

// private --------------------------------------------------------------------

void XclImpObjectManager::ReadEscherRecord( XclImpStream& rStrm )
{
    sal_Size nRecSize = rStrm.GetRecSize();
    if( nRecSize > 0 )
    {
        ScfUInt8Vec aBuffer( nRecSize );
        // read from input stream
        rStrm.Seek( EXC_REC_SEEK_TO_BEGIN );
        rStrm.Read( &aBuffer.front(), nRecSize );
        // write to Escher stream
        maEscherStrm.Seek( STREAM_SEEK_TO_END );
        maEscherStrm.Write( &aBuffer.front(), nRecSize );
    }
}

void XclImpObjectManager::ReadObj8( XclImpStream& rStrm )
{
    XclImpDrawObjRef xDrawObj;
    bool bLoop = true;
    while( bLoop && (rStrm.GetRecLeft() >= 4) )
    {
        sal_uInt16 nSubRecId, nSubRecSize;
        rStrm >> nSubRecId >> nSubRecSize;
        rStrm.PushPosition();

        switch( nSubRecId )
        {
            case EXC_ID_OBJ_FTEND:
                bLoop = false;
            break;
            case EXC_ID_OBJ_FTCMO:
                DBG_ASSERT( !xDrawObj, "XclImpObjectManager::ReadObj8 - multiple FTCMO subrecords" );
                xDrawObj = XclImpDrawObjBase::ReadObjCmo( rStrm );
                bLoop = xDrawObj.is();
            break;
            default:
                DBG_ASSERT( xDrawObj.is(), "XclImpObjectManager::ReadObj8 - missing leading FTCMO subrecord" );
                if( xDrawObj.is() )
                    xDrawObj->ReadSubRecord( rStrm, nSubRecId, nSubRecSize );
        }

        rStrm.PopPosition();
        // sometimes the last subrecord has an invalid length -> min()
        rStrm.Ignore( ::std::min< sal_Size >( nSubRecSize, rStrm.GetRecLeft() ) );
    }

    // try to read the chart substream
    if( XclImpChartObj* pChartObj = dynamic_cast< XclImpChartObj* >( xDrawObj.get() ) )
    {
        // read complete chart substream from BOF to EOF records
        if( StartChartSubStream( rStrm ) )
            pChartObj->ReadChartSubStream( rStrm );
        // #90118# be able to read following CONTINUE record as MSODRAWING
        rStrm.ResetRecord( false );
    }

    // store the new object in the internal containers
    if( xDrawObj.is() )
    {
        maObjMap[ maEscherStrm.Tell() ] = xDrawObj;
        maObjMapId[ xDrawObj->GetObjId() ] = xDrawObj;
    }
}

void XclImpObjectManager::ReadTxo( XclImpStream& rStrm )
{
    XclImpTxoDataRef xTxo( new XclImpTxoData( GetRoot() ) );
    xTxo->ReadTxo( rStrm );
    maTxoMap[ maEscherStrm.Tell() ] = xTxo;
}

bool XclImpObjectManager::StartChartSubStream( XclImpStream& rStrm )
{
    bool bChartSubStrm = (rStrm.GetNextRecId() == EXC_ID5_BOF) && rStrm.StartNextRecord();
    DBG_ASSERT( bChartSubStrm, "XclImpObjectManager::StartChartSubStream - missing chart substream" );
    if( bChartSubStrm )
    {
        rStrm.ResetRecord( true );
        sal_uInt16 nBofType;
        rStrm.Ignore( 2 );
        rStrm >> nBofType;
        DBG_ASSERT( nBofType == EXC_BOF_CHART, "XclImpObjectManager::StartChartSubStream - no chart BOF record" );
        // return true, even if BOF record contains wrong substream identifier
    }
    return bChartSubStrm;
}

sal_Size XclImpObjectManager::GetProgressSize() const
{
    sal_Size nProgressSize = 0;
    for( XclImpObjMap::const_iterator aMIt = maObjMap.begin(), aMEnd = maObjMap.end(); aMIt != aMEnd; ++aMIt )
        nProgressSize += aMIt->second->GetProgressSize();
    for( XclImpChartObjList::const_iterator aLIt = maTabCharts.begin(), aLEnd = maTabCharts.end(); aLIt != aLEnd; ++aLIt )
        nProgressSize += (*aLIt)->GetProgressSize();
    return nProgressSize;
}

// Escher property set helper =================================================

XclImpEscherPropSet::XclImpEscherPropSet( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot ),
    maObjManager( rRoot ),
    mrDffManager( maObjManager.GetDffManager() )
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
    mrDffManager.ReadPropSet( *mxMemStrm, 0 );
}

sal_uInt32 XclImpEscherPropSet::GetPropertyValue( sal_uInt16 nPropId, sal_uInt32 nDefault ) const
{
    return mrDffManager.GetPropertyValue( nPropId, nDefault );
}

void XclImpEscherPropSet::FillToItemSet( SfxItemSet& rItemSet ) const
{
    if( mxMemStrm.get() )
        mrDffManager.ApplyAttributes( *mxMemStrm, rItemSet );
}

XclImpStream& operator>>( XclImpStream& rStrm, XclImpEscherPropSet& rPropSet )
{
    rPropSet.Read( rStrm );
    return rStrm;
}

// ============================================================================

