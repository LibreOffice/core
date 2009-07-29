/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xcl97rec.cxx,v $
 * $Revision: 1.87.30.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#include <svx/svdpool.hxx>
#include <svx/sdtaitm.hxx>
#include <svx/svdotext.hxx>
#include <svx/editobj.hxx>
#include <svx/svdoole2.hxx>
#include <sot/storage.hxx>
#include <svtools/itemset.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdocapt.hxx>
#include <svx/unoapi.hxx>
#include <svx/writingmodeitem.hxx>
#include <vcl/svapp.hxx>
#include <rtl/math.hxx>
#include <svtools/zformat.hxx>
#include "cell.hxx"
#include "drwlayer.hxx"

#include "xcl97rec.hxx"
#include "xcl97esc.hxx"
#include "editutil.hxx"
#include "xecontent.hxx"
#include "xeescher.hxx"
#include "xestyle.hxx"
#include "xelink.hxx"

#include "scitems.hxx"

#include <svtools/fltrcfg.hxx>
#include <svx/brshitem.hxx>
#include <svx/boxitem.hxx>
#include <svx/frmdiritem.hxx>
#include <svx/adjitem.hxx>
#include <svx/eeitem.hxx>
#include <svx/msoleexp.hxx>

#include <svtools/useroptions.hxx>
#include <unotools/localedatawrapper.hxx>

#include <stdio.h>

#include "document.hxx"
#include "conditio.hxx"
#include "rangelst.hxx"
#include "stlpool.hxx"
#include "viewopti.hxx"
#include "scextopt.hxx"
#include "docoptio.hxx"
#include "patattr.hxx"

#include <oox/core/tokens.hxx>

using ::rtl::OString;
using ::rtl::OUString;
using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::drawing::XShape;


//___________________________________________________________________

// --- class XclExpMsoDrawingBase --------------------------------------

XclExpMsoDrawingBase::XclExpMsoDrawingBase( const XclExpRoot& rRoot, sal_Size nInitialSize ) :
    XclExpRoot( rRoot )
{
    nStartPos = GetEscherEx().GetLastOffsetMapPos();
    // for safety's sake add this now
    nStopPos = GetEscherEx().AddCurrentOffsetToMap();
    (void)nInitialSize; // avoid compiler warning
    DBG_ASSERT( GetDataLen() == nInitialSize, "XclExpMsoDrawingBase ctor: do I really own that data?" );
}

XclExpMsoDrawingBase::~XclExpMsoDrawingBase()
{
}

XclEscherEx& XclExpMsoDrawingBase::GetEscherEx() const
{
    return GetObjectManager().GetEx();
}

void XclExpMsoDrawingBase::UpdateStopPos()
{
    if( nStopPos > 0 )
        GetEscherEx().ReplaceCurrentOffsetInMap( nStopPos );
    else
        nStopPos = GetEscherEx().AddCurrentOffsetToMap();
}

sal_Size XclExpMsoDrawingBase::GetDataLen() const
{
    if( nStartPos < nStopPos )
    {
        XclEscherEx& rEx = GetEscherEx();
        return rEx.GetOffsetFromMap( nStopPos ) - rEx.GetOffsetFromMap( nStartPos );
    }
    DBG_ERRORFILE( "XclExpMsoDrawingBase::GetDataLen - position mismatch" );
    return 0;
}

// --- class XclExpMsoDrawingGroup --------------------------------------

XclExpMsoDrawingGroup::XclExpMsoDrawingGroup( const XclExpRoot& rRoot ) :
    XclExpMsoDrawingBase( rRoot ),
    XclExpRecord( EXC_ID_MSODRAWINGGROUP )
{
    XclEscherEx& rEx = GetEscherEx();
    SvStream& rOut = rEx.GetStream();

    rEx.OpenContainer( ESCHER_DggContainer );
//2do: stuff it with our own document defaults?
#if 0
    rEx.BeginCount();
    rEx.AddOpt( ... );
    rEx.EndCount( ESCHER_OPT, 3 );
#else
    static const sal_uInt8 pDummyOPT[] = {
        0xBF, 0x00, 0x08, 0x00, 0x08, 0x00, 0x81, 0x01,
        0x09, 0x00, 0x00, 0x08, 0xC0, 0x01, 0x40, 0x00,
        0x00, 0x08
    };
    rEx.AddAtom( sizeof(pDummyOPT), ESCHER_OPT, 3, 3 );
    rOut.Write( pDummyOPT, sizeof(pDummyOPT) );
#endif
    static const sal_uInt8 pDummySplitMenuColors[] = {
        0x0D, 0x00, 0x00, 0x08, 0x0C, 0x00, 0x00, 0x08,
        0x17, 0x00, 0x00, 0x08, 0xF7, 0x00, 0x00, 0x10
    };
    rEx.AddAtom( sizeof(pDummySplitMenuColors), ESCHER_SplitMenuColors, 0, 4 );
    rOut.Write( pDummySplitMenuColors, sizeof(pDummySplitMenuColors) );
    rEx.CloseContainer();  // ESCHER_DggContainer

    UpdateStopPos();
}

void XclExpMsoDrawingGroup::WriteBody( XclExpStream& rStrm )
{
    DBG_ASSERT( GetEscherEx().GetStreamPos() == GetEscherEx().GetOffsetFromMap( nStartPos ),
        "XclExpMsoDrawingGroup::WriteBody - Escher stream position mismatch" );
    rStrm.CopyFromStream( GetObjectManager().GetStrm(), GetDataLen() );
}

// --- class XclExpMsoDrawing --------------------------------------

XclExpMsoDrawing::XclExpMsoDrawing( const XclExpRoot& rRoot, UINT16 nEscherType, sal_Size nInitialSize ) :
    XclExpMsoDrawingBase( rRoot, nInitialSize ),
    XclExpRecord( EXC_ID_MSODRAWING )
{
    if ( nEscherType )
    {
        XclEscherEx& rEx = GetEscherEx();
        switch ( nEscherType )
        {
            case ESCHER_DgContainer :
            {   // per-sheet data
                rEx.OpenContainer( nEscherType );
                // open group shape container
                Rectangle aRect( 0, 0, 0, 0 );
                rEx.EnterGroup( &aRect );
            }
            break;
        }
        UpdateStopPos();
    }
}

void XclExpMsoDrawing::WriteBody( XclExpStream& rStrm )
{
    DBG_ASSERT( GetEscherEx().GetStreamPos() == GetEscherEx().GetOffsetFromMap( nStartPos ),
        "XclExpMsoDrawing::WriteBody - Escher stream position mismatch" );
    rStrm.CopyFromStream( GetObjectManager().GetStrm(), GetDataLen() );
}

// --- class XclExpObjList ----------------------------------------------

XclExpObjList::XclExpObjList( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot ),
    pMsodrawingPerSheet( new XclExpMsoDrawing( rRoot, ESCHER_DgContainer ) ),
    pSolverContainer( NULL )
{
}


XclExpObjList::~XclExpObjList()
{
    for ( XclObj* p = First(); p; p = Next() )
        delete p;
    delete pMsodrawingPerSheet;
    delete pSolverContainer;
}


UINT16 XclExpObjList::Add( XclObj* pObj )
{
    DBG_ASSERT( Count() < 0xFFFF, "XclExpObjList::Add: too much for Xcl" );
    if ( Count() < 0xFFFF )
    {
        Insert( pObj, LIST_APPEND );
        UINT16 nCnt = (UINT16) Count();
        pObj->SetId( nCnt );
        return nCnt;
    }
    else
    {
        delete pObj;
        return 0;
    }
}


void XclExpObjList::EndSheet()
{
    XclEscherEx& rEx = pMsodrawingPerSheet->GetEscherEx();

    // Is there still something in the stream? -> The solver container
    sal_Size nSolverSize = rEx.GetStreamPos() - rEx.GetOffsetFromMap( rEx.GetLastOffsetMapPos() );
    if( nSolverSize > 0 )
        pSolverContainer = new XclExpMsoDrawing( GetRoot(), ESCHER_SolverContainer, nSolverSize );

    //! close ESCHER_DgContainer created by XclExpObjList ctor MSODRAWING
    rEx.CloseContainer();
}


void XclExpObjList::Save( XclExpStream& rStrm )
{
    //! Escher must be written, even if there are no objects
    pMsodrawingPerSheet->Save( rStrm );

    for ( XclObj* p = First(); p; p = Next() )
        p->Save( rStrm );

    if( pSolverContainer )
        pSolverContainer->Save( rStrm );
}



// --- class XclObj --------------------------------------------------

XclObj::XclObj( const XclExpRoot& rRoot, sal_uInt16 nObjType, bool bOwnEscher ) :
    XclExpRecord( EXC_ID_OBJ, 26 ),
    pClientTextbox( NULL ),
    pTxo( NULL ),
    mnObjType( nObjType ),
    nObjId(0),
    nGrbit( 0x6011 ),   // AutoLine, AutoFill, Printable, Locked
    bFirstOnSheet( rRoot.GetOldRoot().pObjRecs->Count() == 0 ),
    mbOwnEscher( bOwnEscher )
{
    //! first object continues the first MSODRAWING record
    if ( bFirstOnSheet )
        pMsodrawing = rRoot.GetOldRoot().pObjRecs->GetMsodrawingPerSheet();
    else
        pMsodrawing = new XclExpMsoDrawing( rRoot );
}


XclObj::~XclObj()
{
    if ( !bFirstOnSheet )
        delete pMsodrawing;
    delete pClientTextbox;
    delete pTxo;
}


void XclObj::SetEscherShapeType( UINT16 nType )
{
//2do: what about the other defined ot... types?
    switch ( nType )
    {
        case ESCHER_ShpInst_Line :
            mnObjType = EXC_OBJTYPE_LINE;
        break;
        case ESCHER_ShpInst_Rectangle :
        case ESCHER_ShpInst_RoundRectangle :
            mnObjType = EXC_OBJTYPE_RECTANGLE;
        break;
        case ESCHER_ShpInst_Ellipse :
            mnObjType = EXC_OBJTYPE_OVAL;
        break;
        case ESCHER_ShpInst_Arc :
            mnObjType = EXC_OBJTYPE_ARC;
        break;
        case ESCHER_ShpInst_TextBox :
            mnObjType = EXC_OBJTYPE_TEXT;
        break;
        case ESCHER_ShpInst_PictureFrame :
            mnObjType = EXC_OBJTYPE_PICTURE;
        break;
        default:
            mnObjType = EXC_OBJTYPE_DRAWING;
    }
}


void XclObj::SetText( const XclExpRoot& rRoot, const SdrTextObj& rObj )
{
    DBG_ASSERT( !pClientTextbox, "XclObj::SetText: already set" );
    if ( !pClientTextbox )
    {
        pMsodrawing->UpdateStopPos();
        pClientTextbox = new XclExpMsoDrawing( rRoot );
        pClientTextbox->GetEscherEx().AddAtom( 0, ESCHER_ClientTextbox );    // TXO record
        pClientTextbox->UpdateStopPos();
        pTxo = new XclTxo( rRoot, rObj );
    }
}


void XclObj::WriteBody( XclExpStream& rStrm )
{
    DBG_ASSERT( mnObjType != EXC_OBJTYPE_UNKNOWN, "XclObj::WriteBody - unknown type" );

    // create a substream to be able to create subrecords
    SvMemoryStream aMemStrm;
    ::std::auto_ptr< XclExpStream > pXclStrm( new XclExpStream( aMemStrm, rStrm.GetRoot() ) );

    // write the ftCmo subrecord
    pXclStrm->StartRecord( EXC_ID_OBJCMO, 18 );
    *pXclStrm << mnObjType << nObjId << nGrbit;
    pXclStrm->WriteZeroBytes( 12 );
    pXclStrm->EndRecord();

    // write other subrecords
    WriteSubRecs( *pXclStrm );

    // write the ftEnd subrecord
    pXclStrm->StartRecord( EXC_ID_OBJEND, 0 );
    pXclStrm->EndRecord();

    // copy the data to the OBJ record
    pXclStrm.reset();
    aMemStrm.Seek( 0 );
    rStrm.CopyFromStream( aMemStrm );
}


void XclObj::Save( XclExpStream& rStrm )
{
    // MSODRAWING record (msofbtSpContainer)
    if ( !bFirstOnSheet )
        pMsodrawing->Save( rStrm );

    // OBJ
    XclExpRecord::Save( rStrm );

    // second MSODRAWING record and TXO and CONTINUE records
    SaveTextRecs( rStrm );
}


void XclObj::WriteSubRecs( XclExpStream& /*rStrm*/ )
{
}

void XclObj::SaveTextRecs( XclExpStream& rStrm )
{
    // MSODRAWING record (msofbtClientTextbox)
    if ( pClientTextbox )
        pClientTextbox->Save( rStrm );
    // TXO and CONTINUE records
    if ( pTxo )
        pTxo->Save( rStrm );
}


// --- class XclObjComment -------------------------------------------


XclObjComment::XclObjComment( const XclExpRoot& rRoot, const Rectangle& rRect, const EditTextObject& rEditObj, SdrObject* pCaption, bool bVisible )
            :
            XclObj( rRoot, EXC_OBJTYPE_NOTE, true )
{
    ProcessEscherObj(rRoot, rRect, pCaption, bVisible);
    // TXO
    pTxo = new XclTxo( rRoot, rEditObj, pCaption );
}

void XclObjComment::ProcessEscherObj( const XclExpRoot& rRoot, const Rectangle& rRect, SdrObject* pCaption, const bool bVisible )
{
    Reference<XShape> aXShape;
    EscherPropertyContainer aPropOpt;

    if(pCaption)
    {
        aXShape = GetXShapeForSdrObject(pCaption);
        Reference< XPropertySet > aXPropSet( aXShape, UNO_QUERY );
        if( aXPropSet.is() )
        {
            aPropOpt.CreateFillProperties( aXPropSet,  sal_True);

            aPropOpt.AddOpt( ESCHER_Prop_lTxid, 0 );                        // undocumented
        aPropOpt.AddOpt( 0x0158, 0x00000000 );                          // undocumented

            sal_uInt32 nValue = 0;
            if(!aPropOpt.GetOpt( ESCHER_Prop_FitTextToShape, nValue ))
                aPropOpt.AddOpt( ESCHER_Prop_FitTextToShape, 0x00080008 );      // bool field

            if(aPropOpt.GetOpt( ESCHER_Prop_fillColor, nValue ))
            {
                // If the Colour is the same as the 'ToolTip' System colour then
                // use the default rather than the explicit colour value. This will
                // be incorrect where user has chosen to use this colour explicity.
                Color aColor = Color( (BYTE)nValue, (BYTE)( nValue >> 8 ), (BYTE)( nValue >> 16 ) );
                const StyleSettings& rSett = Application::GetSettings().GetStyleSettings();
                if(aColor == rSett.GetHelpColor().GetColor())
                {
                    aPropOpt.AddOpt( ESCHER_Prop_fillColor, 0x08000050 );
                    aPropOpt.AddOpt( ESCHER_Prop_fillBackColor, 0x08000050 );
                }
            }
            else
                aPropOpt.AddOpt( ESCHER_Prop_fillColor, 0x08000050 );

            if(!aPropOpt.GetOpt( ESCHER_Prop_fillBackColor, nValue ))
                aPropOpt.AddOpt( ESCHER_Prop_fillBackColor, 0x08000050 );
        if(!aPropOpt.GetOpt( ESCHER_Prop_fNoFillHitTest, nValue ))
                aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x00110010 );      // bool field
        if(!aPropOpt.GetOpt( ESCHER_Prop_shadowColor, nValue ))
                aPropOpt.AddOpt( ESCHER_Prop_shadowColor, 0x00000000 );
        if(!aPropOpt.GetOpt( ESCHER_Prop_fshadowObscured, nValue ))     // bool field
                aPropOpt.AddOpt( ESCHER_Prop_fshadowObscured, 0x00030003 );     // bool field
        }
    }

    nGrbit = 0;     // all off: AutoLine, AutoFill, Printable, Locked
    XclEscherEx& rEx = pMsodrawing->GetEscherEx();
    rEx.OpenContainer( ESCHER_SpContainer );
    rEx.AddShape( ESCHER_ShpInst_TextBox, SHAPEFLAG_HAVEANCHOR | SHAPEFLAG_HAVESPT );
    sal_uInt32 nFlags = 0x000A0000;
    ::set_flag( nFlags, sal_uInt32(2), !bVisible );
    aPropOpt.AddOpt( ESCHER_Prop_fPrint, nFlags );                  // bool field
    aPropOpt.Commit( rEx.GetStream() );

    XclExpDffNoteAnchor( rRoot, rRect ).WriteData( rEx );

    rEx.AddAtom( 0, ESCHER_ClientData );                        // OBJ record
    pMsodrawing->UpdateStopPos();
    //! Be sure to construct the MSODRAWING ClientTextbox record _after_ the
    //! base OBJ's MSODRAWING record Escher data is completed.
    pClientTextbox = new XclExpMsoDrawing( rRoot );
    pClientTextbox->GetEscherEx().AddAtom( 0, ESCHER_ClientTextbox );    // TXO record
    pClientTextbox->UpdateStopPos();
    rEx.CloseContainer();   // ESCHER_SpContainer
}


XclObjComment::~XclObjComment()
{
}


void XclObjComment::Save( XclExpStream& rStrm )
{
    // content of this record
    XclObj::Save( rStrm );
}


// --- class XclObjDropDown ------------------------------------------

XclObjDropDown::XclObjDropDown( const XclExpRoot& rRoot, const ScAddress& rPos, BOOL bFilt ) :
        XclObj( rRoot, EXC_OBJTYPE_DROPDOWN, true ),
        bIsFiltered( bFilt )
{
    SetLocked( TRUE );
    SetPrintable( FALSE );
    SetAutoFill( TRUE );
    SetAutoLine( FALSE );
    nGrbit |= 0x0100;   // undocumented
    XclEscherEx& rEx = pMsodrawing->GetEscherEx();
    rEx.OpenContainer( ESCHER_SpContainer );
    rEx.AddShape( ESCHER_ShpInst_HostControl, SHAPEFLAG_HAVEANCHOR | SHAPEFLAG_HAVESPT );
    EscherPropertyContainer aPropOpt;
    aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x01040104 ); // bool field
    aPropOpt.AddOpt( ESCHER_Prop_FitTextToShape, 0x00080008 );      // bool field
    aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x00010000 );      // bool field
    aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x00080000 );     // bool field
    aPropOpt.AddOpt( ESCHER_Prop_fPrint, 0x000A0000 );              // bool field
    aPropOpt.Commit( rEx.GetStream() );

    XclExpDffDropDownAnchor( rRoot, rPos ).WriteData( rEx );

    rEx.AddAtom( 0, ESCHER_ClientData );                        // OBJ record
    pMsodrawing->UpdateStopPos();
    rEx.CloseContainer();   // ESCHER_SpContainer

    // old size + ftSbs + ftLbsData
    AddRecSize( 24 + 20 );
}

XclObjDropDown::~XclObjDropDown()
{
}

void XclObjDropDown::WriteSubRecs( XclExpStream& rStrm )
{
    // ftSbs subrecord - Scroll bars (dummy)
    rStrm.StartRecord( EXC_ID_OBJSBS, 20 );
    rStrm.WriteZeroBytes( 20 );
    rStrm.EndRecord();

    // ftLbsData subrecord - Listbox data
    sal_uInt16 nDropDownFlags = 0;
    ::insert_value( nDropDownFlags, EXC_OBJ_DROPDOWN_SIMPLE, 0, 2 );
    ::set_flag( nDropDownFlags, EXC_OBJ_DROPDOWN_FILTERED, bIsFiltered );
    rStrm.StartRecord( EXC_ID_OBJLBSDATA, 16 );
    rStrm   << (UINT32)0 << (UINT16)0 << (UINT16)0x0301 << (UINT16)0
            << nDropDownFlags << sal_uInt16( 20 ) << sal_uInt16( 130 );
    rStrm.EndRecord();
}


// --- class XclTxo --------------------------------------------------

sal_uInt8 lcl_GetHorAlignFromItemSet( const SfxItemSet& rItemSet )
{
    sal_uInt8 nHorAlign = EXC_OBJ_HOR_LEFT;

    switch( static_cast< const SvxAdjustItem& >( rItemSet.Get( EE_PARA_JUST ) ).GetAdjust() )
    {
        case SVX_ADJUST_LEFT:   nHorAlign = EXC_OBJ_HOR_LEFT;      break;
        case SVX_ADJUST_CENTER: nHorAlign = EXC_OBJ_HOR_CENTER;    break;
        case SVX_ADJUST_RIGHT:  nHorAlign = EXC_OBJ_HOR_RIGHT;     break;
        case SVX_ADJUST_BLOCK:  nHorAlign = EXC_OBJ_HOR_JUSTIFY;   break;
        default:;
    }
    return nHorAlign;
}

sal_uInt8 lcl_GetVerAlignFromItemSet( const SfxItemSet& rItemSet )
{
    sal_uInt8 nVerAlign = EXC_OBJ_VER_TOP;

    switch( static_cast< const SdrTextVertAdjustItem& >( rItemSet.Get( SDRATTR_TEXT_VERTADJUST ) ).GetValue() )
    {
        case SDRTEXTVERTADJUST_TOP:     nVerAlign = EXC_OBJ_VER_TOP;       break;
        case SDRTEXTVERTADJUST_CENTER:  nVerAlign = EXC_OBJ_VER_CENTER;    break;
        case SDRTEXTVERTADJUST_BOTTOM:  nVerAlign = EXC_OBJ_VER_BOTTOM;    break;
        case SDRTEXTVERTADJUST_BLOCK:   nVerAlign = EXC_OBJ_VER_JUSTIFY;   break;
    }
    return nVerAlign;
}

XclTxo::XclTxo( const String& rString, sal_uInt16 nFontIx ) :
    mpString( new XclExpString( rString ) ),
    mnRotation( EXC_OBJ_ORIENT_NONE ),
    mnHorAlign( EXC_OBJ_HOR_LEFT ),
    mnVerAlign( EXC_OBJ_VER_TOP )
{
    if( mpString->Len() )
    {
        // If there is text, Excel *needs* the 2nd CONTINUE record with at least two format runs
        mpString->AppendFormat( 0, nFontIx );
        mpString->AppendFormat( mpString->Len(), EXC_FONT_APP );
    }
}

XclTxo::XclTxo( const XclExpRoot& rRoot, const SdrTextObj& rTextObj ) :
    mpString( XclExpStringHelper::CreateString( rRoot, rTextObj ) ),
    mnRotation( EXC_OBJ_ORIENT_NONE ),
    mnHorAlign( EXC_OBJ_HOR_LEFT ),
    mnVerAlign( EXC_OBJ_VER_TOP )
{
    // additional alignment and orientation items
    const SfxItemSet& rItemSet = rTextObj.GetMergedItemSet();

    // horizontal alignment
    SetHorAlign( lcl_GetHorAlignFromItemSet( rItemSet ) );

    // vertical alignment
    SetVerAlign( lcl_GetVerAlignFromItemSet( rItemSet ) );

    // rotation
    long nAngle = rTextObj.GetRotateAngle();
    if( (4500 < nAngle) && (nAngle < 13500) )
        mnRotation = EXC_OBJ_ORIENT_90CCW;
    else if( (22500 < nAngle) && (nAngle < 31500) )
        mnRotation = EXC_OBJ_ORIENT_90CW;
    else
        mnRotation = EXC_OBJ_ORIENT_NONE;
}

XclTxo::XclTxo( const XclExpRoot& rRoot, const EditTextObject& rEditObj, SdrObject* pCaption ) :
    mpString( XclExpStringHelper::CreateString( rRoot, rEditObj ) ),
    mnRotation( EXC_OBJ_ORIENT_NONE ),
    mnHorAlign( EXC_OBJ_HOR_LEFT ),
    mnVerAlign( EXC_OBJ_VER_TOP )
{
    if(pCaption)
    {
        // Excel has one alignment per NoteObject while Calc supports
        // one alignment per paragraph - use the first paragraph
        // alignment (if set) as our overall alignment.
        String aParaText( rEditObj.GetText( 0 ) );
        if( aParaText.Len() )
        {
            SfxItemSet aSet( rEditObj.GetParaAttribs( 0));
            const SfxPoolItem* pItem = NULL;
            if( aSet.GetItemState( EE_PARA_JUST, TRUE, &pItem ) == SFX_ITEM_SET )
            {
                SvxAdjust eEEAlign = static_cast< const SvxAdjustItem& >( *pItem ).GetAdjust();
                pCaption->SetMergedItem( SvxAdjustItem( eEEAlign, EE_PARA_JUST ) );
            }
        }
        const SfxItemSet& rItemSet = pCaption->GetMergedItemSet();

        // horizontal alignment
        SetHorAlign( lcl_GetHorAlignFromItemSet( rItemSet ) );

        // vertical alignment
        SetVerAlign( lcl_GetVerAlignFromItemSet( rItemSet ) );

        // orientation alignment
        const SvxWritingModeItem& rItem = static_cast< const SvxWritingModeItem& >( rItemSet.Get( SDRATTR_TEXTDIRECTION ) );
        if( rItem.GetValue() == com::sun::star::text::WritingMode_TB_RL )
            mnRotation = EXC_OBJ_ORIENT_90CW;
    }
}

void XclTxo::SaveCont( XclExpStream& rStrm )
{
    DBG_ASSERT( mpString.get(), "XclTxo::SaveCont - missing string" );

    // #i96858# do not save existing string formatting if text is empty
    sal_uInt16 nRunLen = mpString->IsEmpty() ? 0 : (8 * mpString->GetFormatsCount());
    // alignment
    sal_uInt16 nFlags = 0;
    ::insert_value( nFlags, mnHorAlign, 1, 3 );
    ::insert_value( nFlags, mnVerAlign, 4, 3 );

    rStrm << nFlags << mnRotation;
    rStrm.WriteZeroBytes( 6 );
    rStrm << mpString->Len() << nRunLen << sal_uInt32( 0 );
}

void XclTxo::Save( XclExpStream& rStrm )
{
    // Write the TXO part
    ExcRecord::Save( rStrm );

    // CONTINUE records are only written if there is some text
    if( !mpString->IsEmpty() )
    {
        // CONTINUE for character array
        rStrm.StartRecord( EXC_ID_CONT, mpString->GetBufferSize() + 1 );
        rStrm << static_cast< sal_uInt8 >( mpString->GetFlagField() & EXC_STRF_16BIT ); // only Unicode flag
        mpString->WriteBuffer( rStrm );
        rStrm.EndRecord();

        // CONTINUE for formatting runs
        rStrm.StartRecord( EXC_ID_CONT, 8 * mpString->GetFormatsCount() );
        const XclFormatRunVec& rFormats = mpString->GetFormats();
        for( XclFormatRunVec::const_iterator aIt = rFormats.begin(), aEnd = rFormats.end(); aIt != aEnd; ++aIt )
            rStrm << aIt->mnChar << aIt->mnFontIdx << sal_uInt32( 0 );
        rStrm.EndRecord();
    }
}

UINT16 XclTxo::GetNum() const
{
    return EXC_ID_TXO;
}

sal_Size XclTxo::GetLen() const
{
    return 18;
}


// --- class XclObjOle -------------------------------------------

XclObjOle::XclObjOle( const XclExpRoot& rRoot, const SdrObject& rObj ) :
    XclObj( rRoot, EXC_OBJTYPE_PICTURE ),
    rOleObj( rObj ),
    pRootStorage( rRoot.GetRootStorage() )
{
}


XclObjOle::~XclObjOle()
{
}


void XclObjOle::WriteSubRecs( XclExpStream& rStrm )
{
    // write only as embedded, not linked
    String          aStorageName( RTL_CONSTASCII_USTRINGPARAM( "MBD" ) );
    sal_Char        aBuf[ sizeof(UINT32) * 2 + 1 ];
    // FIXME Eeek! Is this just a way to get a unique id?
    UINT32          nPictureId = UINT32(sal_uIntPtr(this) >> 2);
    sprintf( aBuf, "%08X", static_cast< unsigned int >( nPictureId ) );        // #100211# - checked
    aStorageName.AppendAscii( aBuf );
    SotStorageRef    xOleStg = pRootStorage->OpenSotStorage( aStorageName,
                            STREAM_READWRITE| STREAM_SHARE_DENYALL );
    if( xOleStg.Is() )
    {
        uno::Reference < embed::XEmbeddedObject > xObj( ((SdrOle2Obj&)rOleObj).GetObjRef() );
        if ( xObj.is() )
        {
            // set version to "old" version, because it must be
            // saved in MS notation.
            UINT32                  nFl = 0;
            SvtFilterOptions*       pFltOpts = SvtFilterOptions::Get();
            if( pFltOpts )
            {
                if( pFltOpts->IsMath2MathType() )
                    nFl |= OLE_STARMATH_2_MATHTYPE;

                if( pFltOpts->IsWriter2WinWord() )
                    nFl |= OLE_STARWRITER_2_WINWORD;

                if( pFltOpts->IsCalc2Excel() )
                    nFl |= OLE_STARCALC_2_EXCEL;

                if( pFltOpts->IsImpress2PowerPoint() )
                    nFl |= OLE_STARIMPRESS_2_POWERPOINT;
            }

            SvxMSExportOLEObjects   aOLEExpFilt( nFl );
            aOLEExpFilt.ExportOLEObject( xObj, *xOleStg );

            // OBJCF subrecord, undocumented as usual
            rStrm.StartRecord( EXC_ID_OBJCF, 2 );
            rStrm << UINT16(0x0002);
            rStrm.EndRecord();

            // OBJFLAGS subrecord, undocumented as usual
            rStrm.StartRecord( EXC_ID_OBJFLAGS, 2 );
            sal_uInt16 nFlags = EXC_OBJ_PIC_MANUALSIZE;
            ::set_flag( nFlags, EXC_OBJ_PIC_SYMBOL, ((SdrOle2Obj&)rOleObj).GetAspect() == embed::Aspects::MSOLE_ICON );
            rStrm << nFlags;
            rStrm.EndRecord();

            // OBJPICTFMLA subrecord, undocumented as usual
            XclExpString aName( xOleStg->GetUserName() );
            UINT16 nPadLen = (UINT16)(aName.GetSize() & 0x01);
            UINT16 nFmlaLen = static_cast< sal_uInt16 >( 12 + aName.GetSize() + nPadLen );
            UINT16 nSubRecLen = nFmlaLen + 6;

            rStrm.StartRecord( EXC_ID_OBJPICTFMLA, nSubRecLen );
            rStrm   << nFmlaLen
                    << sal_uInt16( 5 ) << sal_uInt32( 0 ) << sal_uInt8( 2 )
                    << sal_uInt32( 0 ) << sal_uInt8( 3 )
                    << aName;
            if( nPadLen )
                rStrm << sal_uInt8( 0 );       // pad byte
            rStrm << nPictureId;
            rStrm.EndRecord();
        }
    }
}


void XclObjOle::Save( XclExpStream& rStrm )
{
    // content of this record
    XclObj::Save( rStrm );
}


// --- class XclObjAny -------------------------------------------

XclObjAny::XclObjAny( const XclExpRoot& rRoot ) :
    XclObj( rRoot, EXC_OBJTYPE_UNKNOWN )
{
}

XclObjAny::~XclObjAny()
{
}

void XclObjAny::WriteSubRecs( XclExpStream& rStrm )
{
    if( mnObjType == EXC_OBJTYPE_GROUP )
        // ftGmo subrecord
        rStrm << EXC_ID_OBJGMO << UINT16(2) << UINT16(0);
}

void XclObjAny::Save( XclExpStream& rStrm )
{
    if( mnObjType == EXC_OBJTYPE_GROUP )
        // old size + ftGmo
        AddRecSize( 6 );

    // content of this record
    XclObj::Save( rStrm );
}


// --- class ExcBof8_Base --------------------------------------------

ExcBof8_Base::ExcBof8_Base()
{
    nVers           = 0x0600;
    nRupBuild       = 0x0dbb;
    nRupYear        = 0x07cc;
//  nFileHistory    = 0x00000001;   // last edited by Microsoft Excel for Windows
    nFileHistory    = 0x00000000;
    nLowestBiffVer  = 0x00000006;   // Biff8
}


void ExcBof8_Base::SaveCont( XclExpStream& rStrm )
{
    rStrm   << nVers << nDocType << nRupBuild << nRupYear
            << nFileHistory << nLowestBiffVer;
}


UINT16 ExcBof8_Base::GetNum() const
{
    return 0x0809;
}


sal_Size ExcBof8_Base::GetLen() const
{
    return 16;
}


// --- class ExcBof8 -------------------------------------------------

ExcBof8::ExcBof8()
{
    nDocType = 0x0010;
}


// --- class ExcBofW8 ------------------------------------------------

ExcBofW8::ExcBofW8()
{
    nDocType = 0x0005;
}


// --- class ExcBundlesheet8 -----------------------------------------

ExcBundlesheet8::ExcBundlesheet8( RootData& rRootData, SCTAB _nTab ) :
    ExcBundlesheetBase( rRootData, static_cast<sal_uInt16>(_nTab) ),
    sUnicodeName( rRootData.pER->GetTabInfo().GetScTabName( _nTab ) )
{
}


ExcBundlesheet8::ExcBundlesheet8( const String& rString ) :
    ExcBundlesheetBase(),
    sUnicodeName( rString )
{
}


XclExpString ExcBundlesheet8::GetName() const
{
    return XclExpString( sUnicodeName, EXC_STR_8BITLENGTH );
}


void ExcBundlesheet8::SaveCont( XclExpStream& rStrm )
{
    nOwnPos = rStrm.GetSvStreamPos();
    // write dummy position, real position comes later
    rStrm << sal_uInt32( 0 ) << nGrbit << GetName();
}


sal_Size ExcBundlesheet8::GetLen() const
{   // Text max 255 chars
    return 8 + GetName().GetBufferSize();
}


void ExcBundlesheet8::SaveXml( XclExpXmlStream& rStrm )
{
    OUString sId;
    rStrm.CreateOutputStream(
            XclXmlUtils::GetStreamName( "xl/", "worksheets/sheet", nTab+1),
            XclXmlUtils::GetStreamName( NULL, "worksheets/sheet", nTab+1),
            rStrm.GetCurrentStream()->getOutputStream(),
            "application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml",
            "http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet",
            &sId );

    rStrm.GetCurrentStream()->singleElement( XML_sheet,
            XML_name,               XclXmlUtils::ToOString( sUnicodeName ).getStr(),
            XML_sheetId,            rtl::OString::valueOf( (sal_Int32)( nTab+1 ) ).getStr(),
            XML_state,              nGrbit == 0x0000 ? "visible" : "hidden",
            FSNS( XML_r, XML_id ),  XclXmlUtils::ToOString( sId ).getStr(),
            FSEND );
}



// --- class XclObproj -----------------------------------------------

UINT16 XclObproj::GetNum() const
{
    return 0x00D3;
}


sal_Size XclObproj::GetLen() const
{
    return 0;
}


// ---- class XclCodename --------------------------------------------

XclCodename::XclCodename( const String& r ) : aName( r )
{
}


void XclCodename::SaveCont( XclExpStream& rStrm )
{
    rStrm << aName;
}


UINT16 XclCodename::GetNum() const
{
    return 0x01BA;
}


sal_Size XclCodename::GetLen() const
{
    return aName.GetSize();
}



// ---- Scenarios ----------------------------------------------------

ExcEScenarioCell::ExcEScenarioCell( UINT16 nC, UINT16 nR, const String& rTxt ) :
        nCol( nC ),
        nRow( nR ),
        sText( rTxt, EXC_STR_DEFAULT, 255 )
{
}

void ExcEScenarioCell::WriteAddress( XclExpStream& rStrm )
{
    rStrm << nRow << nCol;
}

void ExcEScenarioCell::WriteText( XclExpStream& rStrm )
{
    rStrm << sText;
}

void ExcEScenarioCell::SaveXml( XclExpXmlStream& rStrm )
{
    rStrm.GetCurrentStream()->singleElement( XML_inputCells,
            // OOXTODO: XML_deleted,
            // OOXTODO: XML_numFmtId,
            XML_r,      XclXmlUtils::ToOString( ScAddress( nCol, nRow, 0 ) ).getStr(),
            // OOXTODO: XML_undone,
            XML_val,    XclXmlUtils::ToOString( sText ).getStr(),
            FSEND );
}




XclExpString ExcEScenario::sUsername;

ExcEScenario::ExcEScenario( ScDocument& rDoc, SCTAB nTab )
{
    String  sTmpName;
    String  sTmpComm;
    Color   aDummyCol;
    USHORT  nFlags;

    rDoc.GetName( nTab, sTmpName );
    sName.Assign( sTmpName, EXC_STR_8BITLENGTH );
    nRecLen = 8 + sName.GetBufferSize();

    rDoc.GetScenarioData( nTab, sTmpComm, aDummyCol, nFlags );
    sComment.Assign( sTmpComm, EXC_STR_DEFAULT, 255 );
    if( sComment.Len() )
        nRecLen += sComment.GetSize();
    nProtected = (nFlags & SC_SCENARIO_PROTECT) ? 1 : 0;

    if( !sUsername.Len() )
    {
        SvtUserOptions aUserOpt;
        sUsername.Assign( aUserOpt.GetLastName(), EXC_STR_DEFAULT, 255 );
    }
    if( !sUsername.Len() )
        sUsername.Assign( String::CreateFromAscii( "SC" ) );
    nRecLen += sUsername.GetSize();

    const ScRangeList* pRList = rDoc.GetScenarioRanges( nTab );
    if( !pRList )
        return;

    BOOL    bContLoop = TRUE;
    SCROW   nRow;
    SCCOL   nCol;
    String  sText;
    double  fVal;

    for( UINT32 nRange = 0; (nRange < pRList->Count()) && bContLoop; nRange++ )
    {
        const ScRange* pRange = pRList->GetObject( nRange );
        for( nRow = pRange->aStart.Row(); (nRow <= pRange->aEnd.Row()) && bContLoop; nRow++ )
            for( nCol = pRange->aStart.Col(); (nCol <= pRange->aEnd.Col()) && bContLoop; nCol++ )
            {
                if( rDoc.HasValueData( nCol, nRow, nTab ) )
                {
                    rDoc.GetValue( nCol, nRow, nTab, fVal );
                    sText = ::rtl::math::doubleToUString( fVal,
                            rtl_math_StringFormat_Automatic,
                            rtl_math_DecimalPlaces_Max,
                            ScGlobal::pLocaleData->getNumDecimalSep().GetChar(0),
                            TRUE );
                }
                else
                    rDoc.GetString( nCol, nRow, nTab, sText );
                bContLoop = Append( static_cast<sal_uInt16>(nCol),
                        static_cast<sal_uInt16>(nRow), sText );
            }
    }
}

ExcEScenario::~ExcEScenario()
{
    for( ExcEScenarioCell* pCell = _First(); pCell; pCell = _Next() )
        delete pCell;
}

BOOL ExcEScenario::Append( UINT16 nCol, UINT16 nRow, const String& rTxt )
{
    if( List::Count() == EXC_SCEN_MAXCELL )
        return FALSE;

    ExcEScenarioCell* pCell = new ExcEScenarioCell( nCol, nRow, rTxt );
    List::Insert( pCell, LIST_APPEND );
    nRecLen += 6 + pCell->GetStringBytes();        // 4 bytes address, 2 bytes ifmt
    return TRUE;
}

void ExcEScenario::SaveCont( XclExpStream& rStrm )
{
    rStrm   << (UINT16) List::Count()       // number of cells
            << nProtected                   // fProtection
            << (UINT8) 0                    // fHidden
            << (UINT8) sName.Len()          // length of scen name
            << (UINT8) sComment.Len()       // length of comment
            << (UINT8) sUsername.Len();     // length of user name
    sName.WriteFlagField( rStrm );
    sName.WriteBuffer( rStrm );

    rStrm << sUsername;

    if( sComment.Len() )
        rStrm << sComment;

    ExcEScenarioCell* pCell;
    for( pCell = _First(); pCell; pCell = _Next() )
        pCell->WriteAddress( rStrm );           // pos of cell
    for( pCell = _First(); pCell; pCell = _Next() )
        pCell->WriteText( rStrm );              // string content
    rStrm.SetSliceSize( 2 );
    rStrm.WriteZeroBytes( 2 * List::Count() );  // date format
}

UINT16 ExcEScenario::GetNum() const
{
    return 0x00AF;
}

sal_Size ExcEScenario::GetLen() const
{
    return nRecLen;
}

void ExcEScenario::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr& rWorkbook = rStrm.GetCurrentStream();
    rWorkbook->startElement( XML_scenario,
            XML_name,       XclXmlUtils::ToOString( sName ).getStr(),
            XML_locked,     XclXmlUtils::ToPsz( nProtected ),
            // OOXTODO: XML_hidden,
            XML_count,      OString::valueOf( (sal_Int32) List::Count() ).getStr(),
            XML_user,       XESTRING_TO_PSZ( sUsername ),
            XML_comment,    XESTRING_TO_PSZ( sComment ),
            FSEND );

    for( ExcEScenarioCell* pCell = _First(); pCell; pCell = _Next() )
        pCell->SaveXml( rStrm );

    rWorkbook->endElement( XML_scenario );
}




ExcEScenarioManager::ExcEScenarioManager( ScDocument& rDoc, SCTAB nTab ) :
        nActive( 0 )
{
    if( rDoc.IsScenario( nTab ) )
        return;

    SCTAB nFirstTab = nTab + 1;
    SCTAB nNewTab       = nFirstTab;

    while( rDoc.IsScenario( nNewTab ) )
    {
        Append( new ExcEScenario( rDoc, nNewTab ) );

        if( rDoc.IsActiveScenario( nNewTab ) )
            nActive = static_cast<sal_uInt16>(nNewTab - nFirstTab);
        nNewTab++;
    }
}

ExcEScenarioManager::~ExcEScenarioManager()
{
    for( ExcEScenario* pScen = _First(); pScen; pScen = _Next() )
        delete pScen;
}

void ExcEScenarioManager::SaveCont( XclExpStream& rStrm )
{
    rStrm   << (UINT16) List::Count()       // number of scenarios
            << nActive                      // active scen
            << nActive                      // last displayed
            << (UINT16) 0;                  // reference areas
}

void ExcEScenarioManager::Save( XclExpStream& rStrm )
{
    if( List::Count() )
        ExcRecord::Save( rStrm );

    for( ExcEScenario* pScen = _First(); pScen; pScen = _Next() )
        pScen->Save( rStrm );
}

void ExcEScenarioManager::SaveXml( XclExpXmlStream& rStrm )
{
    if( ! List::Count() )
        return;

    sax_fastparser::FSHelperPtr& rWorkbook = rStrm.GetCurrentStream();
    rWorkbook->startElement( XML_scenarios,
            XML_current,    OString::valueOf( (sal_Int32)nActive ).getStr(),
            XML_show,       OString::valueOf( (sal_Int32)nActive ).getStr(),
            // OOXTODO: XML_sqref,
            FSEND );

    for( ExcEScenario* pScen = _First(); pScen; pScen = _Next() )
        pScen->SaveXml( rStrm );

    rWorkbook->endElement( XML_scenarios );
}

UINT16 ExcEScenarioManager::GetNum() const
{
    return 0x00AE;
}

sal_Size ExcEScenarioManager::GetLen() const
{
    return 8;
}



// ---- class XclProtection ------------------------------------------

const BYTE      XclProtection::pMyData[] =
{
    0x12, 0x00, 0x02, 0x00, 0x01, 0x00,         // PROTECT
    0xDD, 0x00, 0x02, 0x00, 0x01, 0x00,         // SCENPROTECT
    0x63, 0x00, 0x02, 0x00, 0x01, 0x00          // OBJPROTECT
};
const sal_Size XclProtection::nMyLen = sizeof( XclProtection::pMyData );

sal_Size XclProtection::GetLen( void ) const
{
    return nMyLen;
}


const BYTE* XclProtection::GetData( void ) const
{
    return pMyData;
}









void XclCalccount::SaveCont( XclExpStream& rStrm )
{
    rStrm << nCount;
}


XclCalccount::XclCalccount( const ScDocument& rDoc )
{
    nCount = rDoc.GetDocOptions().GetIterCount();
}


UINT16 XclCalccount::GetNum() const
{
    return 0x000C;
}


sal_Size XclCalccount::GetLen() const
{
    return 2;
}


void XclCalccount::SaveXml( XclExpXmlStream& rStrm )
{
    rStrm.WriteAttributes(
            XML_iterateCount, OString::valueOf( (sal_Int32)nCount ).getStr(),
            FSEND );
}




void XclIteration::SaveCont( XclExpStream& rStrm )
{
    rStrm << nIter;
}


XclIteration::XclIteration( const ScDocument& rDoc )
{
    nIter = rDoc.GetDocOptions().IsIter()? 1 : 0;
}


UINT16 XclIteration::GetNum() const
{
    return 0x0011;
}


sal_Size XclIteration::GetLen() const
{
    return 2;
}


void XclIteration::SaveXml( XclExpXmlStream& rStrm )
{
    rStrm.WriteAttributes(
            XML_iterate, XclXmlUtils::ToPsz( nIter == 1 ),
            FSEND );
}




void XclDelta::SaveCont( XclExpStream& rStrm )
{
    rStrm << fDelta;
}



XclDelta::XclDelta( const ScDocument& rDoc )
{
    fDelta = rDoc.GetDocOptions().GetIterEps();
}


UINT16 XclDelta::GetNum() const
{
    return 0x0010;
}


sal_Size XclDelta::GetLen() const
{
    return 8;
}


void XclDelta::SaveXml( XclExpXmlStream& rStrm )
{
    rStrm.WriteAttributes(
            XML_iterateDelta, OString::valueOf( fDelta ).getStr(),
            FSEND );
}




XclRefmode::XclRefmode( const ScDocument& rDoc ) :
    XclExpBoolRecord( 0x000F, rDoc.GetAddressConvention() != formula::FormulaGrammar::CONV_XL_R1C1 )
{
}

void XclRefmode::SaveXml( XclExpXmlStream& rStrm )
{
    rStrm.WriteAttributes(
            XML_refMode, GetBool() ? "A1" : "R1C1",
            FSEND );
}

