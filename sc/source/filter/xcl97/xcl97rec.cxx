/*************************************************************************
 *
 *  $RCSfile: xcl97rec.cxx,v $
 *
 *  $Revision: 1.59 $
 *
 *  last change: $Author: obo $ $Date: 2003-10-21 08:49:56 $
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

#ifndef _COM_SUN_STAR_FORM_FORMCOMPONENTTYPE_HPP_
#include <com/sun/star/form/FormComponentType.hpp>
#endif

#ifndef _SVDPOOL_HXX //autogen wg. SdrItemPool
#include <svx/svdpool.hxx>
#endif
#ifndef _SDTAITM_HXX //autogen wg. SdrTextHorzAdjustItem, SdrTextVertAdjustItem
#include <svx/sdtaitm.hxx>
#endif
#ifndef _SVDOTEXT_HXX //autogen wg. SdrTextObj
#include <svx/svdotext.hxx>
#endif
#ifndef _EDITOBJ_HXX //autogen wg. EditTextObject
#include <svx/editobj.hxx>
#endif
#ifndef _SVDOOLE2_HXX //autogen wg. SdrOle2Obj
#include <svx/svdoole2.hxx>
#endif
#ifndef _IPOBJ_HXX //autogen wg. SvInPlaceObject
#include <so3/ipobj.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen wg. SvStorage
#include <so3/svstor.hxx>
#endif
#ifndef _SFXITEMSET_HXX //autogen wg. SfxItemSet
#include <svtools/itemset.hxx>
#endif
#ifndef _SVX_UNOAPI_HXX_
#include <svx/unoapi.hxx>
#endif

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif
#ifndef _ZFORMAT_HXX            // SvNumberformat
#include <svtools/zformat.hxx>
#endif
#ifndef SC_CELL_HXX             // ScFormulaCell
#include "cell.hxx"
#endif
#ifndef SC_DRWLAYER_HXX
#include "drwlayer.hxx"
#endif

#include "xcl97rec.hxx"
#include "xcl97esc.hxx"
#include "excupn.hxx"

#ifndef SC_FAPIHELPER_HXX
#include "fapihelper.hxx"
#endif
#ifndef SC_XECONTENT_HXX
#include "xecontent.hxx"
#endif
#ifndef SC_XESTYLE_HXX
#include "xestyle.hxx"
#endif
#ifndef SC_XELINK_HXX
#include "xelink.hxx"
#endif

#include "scitems.hxx"

#include <offmgr/app.hxx>
#include <offmgr/fltrcfg.hxx>
#include <svx/brshitem.hxx>
#include <svx/boxitem.hxx>
#ifndef _SVX_FRMDIRITEM_HXX
#include <svx/frmdiritem.hxx>
#endif
#ifndef _SVX_ADJITEM_HX
#include <svx/adjitem.hxx>
#endif

#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD
#include <svx/msoleexp.hxx>

#include <svtools/useroptions.hxx>
#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif

#include <stdio.h>

#include "document.hxx"
#include "conditio.hxx"
#include "rangelst.hxx"
#include "stlpool.hxx"
#include "viewopti.hxx"
#include "scextopt.hxx"
#include "docoptio.hxx"
#include "patattr.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::drawing::XShape;
using ::com::sun::star::awt::XControlModel;


//___________________________________________________________________

// --- class XclMsodrawing_Base --------------------------------------

XclMsodrawing_Base::XclMsodrawing_Base( XclEscher& rEscher, ULONG nInitialSize )
        :
        pEscher( &rEscher ),
        nStartPos( rEscher.GetEx()->GetLastOffsetMapPos() )
{
    // for safety's sake add this now
    nStopPos = GetEscherEx()->AddCurrentOffsetToMap();
    DBG_ASSERT( GetDataLen() == nInitialSize, "XclMsodrawing_Base ctor: do I really own that data?" );
}


XclMsodrawing_Base::~XclMsodrawing_Base()
{
}


void XclMsodrawing_Base::UpdateStopPos()
{
    if ( nStopPos )
        GetEscherEx()->ReplaceCurrentOffsetInMap( nStopPos );
    else
        nStopPos = GetEscherEx()->AddCurrentOffsetToMap();
}


ULONG XclMsodrawing_Base::GetDataLen() const
{
    if ( nStartPos < nStopPos )
    {
        XclEscherEx* pEx = GetEscherEx();
        return pEx->GetOffsetFromMap( nStopPos ) - pEx->GetOffsetFromMap( nStartPos );
    }
    DBG_ERRORFILE( "XclMsodrawing_Base::GetDataLen: position mismatch" );
    return 0;
}



// --- class XclMsodrawinggroup --------------------------------------

XclMsodrawinggroup::XclMsodrawinggroup( RootData& rRoot, UINT16 nEscherType )
        :
        XclMsodrawing_Base( *rRoot.pEscher )
{
    if ( nEscherType )
    {
        XclEscherEx* pEx = GetEscherEx();
        SvStream& rOut = pEx->GetStream();
        switch ( nEscherType )
        {
            case ESCHER_DggContainer :
            {   // per-document data
                pEx->OpenContainer( nEscherType );
//2do: stuff it with our own document defaults?
#if 0
                pEx->BeginCount();
                pEx->AddOpt( ... );
                pEx->EndCount( ESCHER_OPT, 3 );
#else
                BYTE pDummyOPT[] = {
                    0xBF, 0x00, 0x08, 0x00, 0x08, 0x00, 0x81, 0x01,
                    0x09, 0x00, 0x00, 0x08, 0xC0, 0x01, 0x40, 0x00,
                    0x00, 0x08
                };
                pEx->AddAtom( sizeof(pDummyOPT), ESCHER_OPT, 3, 3 );
                rOut.Write( pDummyOPT, sizeof(pDummyOPT) );
#endif
                BYTE pDummySplitMenuColors[] = {
                    0x0D, 0x00, 0x00, 0x08, 0x0C, 0x00, 0x00, 0x08,
                    0x17, 0x00, 0x00, 0x08, 0xF7, 0x00, 0x00, 0x10
                };
                pEx->AddAtom( sizeof(pDummySplitMenuColors), ESCHER_SplitMenuColors, 0, 4 );
                rOut.Write( pDummySplitMenuColors, sizeof(pDummySplitMenuColors) );
                pEx->CloseContainer();  // ESCHER_DggContainer
            }
            break;
        }
        UpdateStopPos();
    }
}


XclMsodrawinggroup::~XclMsodrawinggroup()
{
}


void XclMsodrawinggroup::SaveCont( XclExpStream& rStrm )
{
    DBG_ASSERT( GetEscherEx()->GetStreamPos() == GetEscherEx()->GetOffsetFromMap( nStartPos ),
        "XclMsodrawinggroup::SaveCont: Escher stream position mismatch" );
    rStrm.CopyFromStream( pEscher->GetStrm(), GetDataLen() );
}


UINT16 XclMsodrawinggroup::GetNum() const
{
    return 0x00EB;
}


ULONG XclMsodrawinggroup::GetLen() const
{
    return GetDataLen();
}



// --- class XclMsodrawing --------------------------------------

XclMsodrawing::XclMsodrawing( const XclExpRoot& rRoot, UINT16 nEscherType, UINT32 nInitialSize ) :
    XclMsodrawing_Base( *rRoot.mpRD->pEscher, nInitialSize )
{
    if ( nEscherType )
    {
        XclEscherEx* pEx = GetEscherEx();
        switch ( nEscherType )
        {
            case ESCHER_DgContainer :
            {   // per-sheet data
                pEx->OpenContainer( nEscherType );
                // open group shape container
                Rectangle aRect( 0, 0, 0, 0 );
                pEx->EnterGroup( &aRect );
            }
            break;
        }
        UpdateStopPos();
    }
}


XclMsodrawing::~XclMsodrawing()
{
}


void XclMsodrawing::SaveCont( XclExpStream& rStrm )
{
    DBG_ASSERT( GetEscherEx()->GetStreamPos() == GetEscherEx()->GetOffsetFromMap( nStartPos ),
        "XclMsodrawing::SaveCont: Escher stream position mismatch" );
    rStrm.CopyFromStream( pEscher->GetStrm(), GetDataLen() );
}


UINT16 XclMsodrawing::GetNum() const
{
    return 0x00EC;
}


ULONG XclMsodrawing::GetLen() const
{
    return GetDataLen();
}


// --- class XclObjList ----------------------------------------------

XclObjList::XclObjList( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot ),
    pMsodrawingPerSheet( new XclMsodrawing( rRoot, ESCHER_DgContainer ) ),
    pSolverContainer( NULL )
{
}


XclObjList::~XclObjList()
{
    for ( XclObj* p = First(); p; p = Next() )
        delete p;
    delete pMsodrawingPerSheet;
    delete pSolverContainer;
}


UINT16 XclObjList::Add( XclObj* pObj )
{
    DBG_ASSERT( Count() < 0xFFFF, "XclObjList::Add: too much for Xcl" );
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


void XclObjList::EndSheet()
{
    XclEscherEx* pEx = pMsodrawingPerSheet->GetEscherEx();

    // Is there still something in the stream? -> The solver container
    UINT32 nSolverSize = pEx->GetStreamPos() - pEx->GetOffsetFromMap( pEx->GetLastOffsetMapPos() );
    if( nSolverSize )
        pSolverContainer = new XclMsodrawing( GetRoot(), ESCHER_SolverContainer, nSolverSize );

    //! close ESCHER_DgContainer created by XclObjList ctor MSODRAWING
    pEx->CloseContainer();
}


void XclObjList::Save( XclExpStream& rStrm )
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
    bFirstOnSheet( rRoot.mpRD->pObjRecs->Count() == 0 ),
    mbOwnEscher( bOwnEscher )
{
    //! first object continues the first MSODRAWING record
    if ( bFirstOnSheet )
        pMsodrawing = rRoot.mpRD->pObjRecs->GetMsodrawingPerSheet();
    else
        pMsodrawing = new XclMsodrawing( rRoot );
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
            mnObjType = EXC_OBJ_CMO_LINE;
        break;
        case ESCHER_ShpInst_Rectangle :
        case ESCHER_ShpInst_RoundRectangle :
            mnObjType = EXC_OBJ_CMO_RECTANGLE;
        break;
        case ESCHER_ShpInst_Ellipse :
            mnObjType = EXC_OBJ_CMO_ELLIPSE;
        break;
        case ESCHER_ShpInst_Arc :
            mnObjType = EXC_OBJ_CMO_ARC;
        break;
        case ESCHER_ShpInst_TextBox :
            mnObjType = EXC_OBJ_CMO_TEXT;
        break;
        case ESCHER_ShpInst_PictureFrame :
            mnObjType = EXC_OBJ_CMO_PICTURE;
        break;
        default:
            mnObjType = EXC_OBJ_CMO_DRAWING;
    }
}


void XclObj::SetText( const XclExpRoot& rRoot, const SdrTextObj& rObj )
{
    DBG_ASSERT( !pClientTextbox, "XclObj::SetText: already set" );
    if ( !pClientTextbox )
    {
        pMsodrawing->UpdateStopPos();
        pClientTextbox = new XclMsodrawing( rRoot );
        pClientTextbox->GetEscherEx()->AddAtom( 0, ESCHER_ClientTextbox );  // TXO record
        pClientTextbox->UpdateStopPos();
        pTxo = new XclTxo( rRoot, rObj );
    }
}


void XclObj::WriteBody( XclExpStream& rStrm )
{
    DBG_ASSERT( mnObjType != EXC_OBJ_CMO_UNKNOWN, "XclObj::WriteBody - unknown type" );

    // create a substream to be able to create subrecords
    SvMemoryStream aMemStrm;
    ::std::auto_ptr< XclExpStream > pXclStrm( new XclExpStream( aMemStrm, rStrm.GetRoot() ) );

    // write the ftCmo subrecord
    pXclStrm->StartRecord( EXC_ID_OBJ_FTCMO, 18 );
    *pXclStrm << mnObjType << nObjId << nGrbit;
    pXclStrm->WriteZeroBytes( 12 );
    pXclStrm->EndRecord();

    // write other subrecords
    WriteSubRecs( *pXclStrm );

    // write the ftEnd subrecord
    pXclStrm->StartRecord( EXC_ID_OBJ_FTEND, 0 );
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


void XclObj::WriteSubRecs( XclExpStream& rStrm )
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

XclObjComment::XclObjComment( const XclExpRoot& rRoot, const ScAddress& rPos, const String& rStr, bool bVisible )
            :
            XclObj( rRoot, EXC_OBJ_CMO_NOTE, true )
{
    nGrbit = 0;     // all off: AutoLine, AutoFill, Printable, Locked
    XclEscherEx* pEx = pMsodrawing->GetEscherEx();
    pEx->OpenContainer( ESCHER_SpContainer );
    pEx->AddShape( ESCHER_ShpInst_TextBox, SHAPEFLAG_HAVEANCHOR | SHAPEFLAG_HAVESPT );
    EscherPropertyContainer aPropOpt;
    aPropOpt.AddOpt( ESCHER_Prop_lTxid, 0 );                        // undocumented
    aPropOpt.AddOpt( ESCHER_Prop_FitTextToShape, 0x00080008 );      // bool field
    aPropOpt.AddOpt( 0x0158, 0x00000000 );                          // undocumented
    aPropOpt.AddOpt( ESCHER_Prop_fillColor, 0x08000050 );
    aPropOpt.AddOpt( ESCHER_Prop_fillBackColor, 0x08000050 );
    aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x00110010 );      // bool field
    aPropOpt.AddOpt( ESCHER_Prop_shadowColor, 0x00000000 );
    aPropOpt.AddOpt( ESCHER_Prop_fshadowObscured, 0x00030003 );     // bool field
    sal_uInt32 nFlags = 0x000A0000;
    ::set_flag( nFlags, 2UL, !bVisible );
    aPropOpt.AddOpt( ESCHER_Prop_fPrint, nFlags );                  // bool field
    aPropOpt.Commit( pEx->GetStream() );

    XclEscherClientAnchor( rRoot.GetDocPtr(), rPos ).WriteData( *pEx );
    pEx->AddAtom( 0, ESCHER_ClientData );                       // OBJ record
    pMsodrawing->UpdateStopPos();
    //! Be sure to construct the MSODRAWING ClientTextbox record _after_ the
    //! base OBJ's MSODRAWING record Escher data is completed.
    pClientTextbox = new XclMsodrawing( rRoot );
    pClientTextbox->GetEscherEx()->AddAtom( 0, ESCHER_ClientTextbox );  // TXO record
    pClientTextbox->UpdateStopPos();
    pEx->CloseContainer();  // ESCHER_SpContainer
    // TXO
    pTxo = new XclTxo( rStr );
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
        XclObj( rRoot, EXC_OBJ_CMO_COMBOBOX, true ),
        bIsFiltered( bFilt )
{
    SetLocked( TRUE );
    SetPrintable( FALSE );
    SetAutoFill( TRUE );
    SetAutoLine( FALSE );
    nGrbit |= 0x0100;   // undocumented
    XclEscherEx* pEx = pMsodrawing->GetEscherEx();
    pEx->OpenContainer( ESCHER_SpContainer );
    pEx->AddShape( ESCHER_ShpInst_HostControl, SHAPEFLAG_HAVEANCHOR | SHAPEFLAG_HAVESPT );
    EscherPropertyContainer aPropOpt;
    aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x01040104 ); // bool field
    aPropOpt.AddOpt( ESCHER_Prop_FitTextToShape, 0x00080008 );      // bool field
    aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x00010000 );      // bool field
    aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x00080000 );     // bool field
    aPropOpt.AddOpt( ESCHER_Prop_fPrint, 0x000A0000 );              // bool field
    aPropOpt.Commit( pEx->GetStream() );

    XclEscherClientAnchor aAnchor( *rRoot.mpRD, 0x0001 );                // MsofbtClientAnchor
    aAnchor.SetDropDownPosition( rPos );
    aAnchor.WriteData( *pEx );

    pEx->AddAtom( 0, ESCHER_ClientData );                       // OBJ record
    pMsodrawing->UpdateStopPos();
    pEx->CloseContainer();  // ESCHER_SpContainer

    // old size + ftSbs + ftLbsData
    SetRecSize( GetRecSize() + 24 + 20 );
}

XclObjDropDown::~XclObjDropDown()
{
}

void XclObjDropDown::WriteSubRecs( XclExpStream& rStrm )
{
    // ftSbs subrecord - Scroll bars (dummy)
    rStrm.StartRecord( EXC_ID_OBJ_FTSBS, 20 );
    rStrm.WriteZeroBytes( 20 );
    rStrm.EndRecord();

    // ftLbsData subrecord - Listbox data
    rStrm.StartRecord( EXC_ID_OBJ_FTLBSDATA, 16 );
    rStrm   << (UINT32)0 << (UINT16)0 << (UINT16)0x0301 << (UINT16)0
            << (UINT16)(bIsFiltered ? 0x000A : 0x0002) << (UINT32)0;
    rStrm.EndRecord();
}


// --- class XclTxo --------------------------------------------------

XclTxo::XclTxo( const String& rString, sal_uInt16 nFontIx ) :
    mpString( new XclExpString( rString ) ),
    meHorAlign( xlTxoHAlign_Default ),
    meVerAlign( xlTxoVAlign_Default ),
    meRotation( xlTxoRot_Default )
{
    if( mpString->Len() )
    {
        // If there is text, Excel *needs* the 2nd CONTINUE record with at least two format runs
        mpString->AppendFormat( 0, nFontIx );
        mpString->AppendFormat( mpString->Len(), EXC_FONT_APP );
    }
}

XclTxo::XclTxo( const XclExpRoot& rRoot, const SdrTextObj& rTextObj ) :
    meHorAlign( xlTxoHAlign_Default ),
    meVerAlign( xlTxoVAlign_Default ),
    meRotation( xlTxoRot_Default )
{
    mpString.reset( XclExpStringHelper::CreateString( rRoot, rTextObj ) );

    // additional alignment and orientation items
    const SfxItemSet& rItemSet = rTextObj.GetItemSet();

    // horizontal alignment
    switch( static_cast< const SvxAdjustItem& >( rItemSet.Get( EE_PARA_JUST ) ).GetAdjust() )
    {
        case SVX_ADJUST_LEFT:           meHorAlign = xlTxoHAlignLeft;       break;
        case SVX_ADJUST_CENTER:         meHorAlign = xlTxoHAlignCenter;     break;
        case SVX_ADJUST_RIGHT:          meHorAlign = xlTxoHAlignRight;      break;
        case SVX_ADJUST_BLOCK:          meHorAlign = xlTxoHAlignJustify;    break;
    }

    // vertical alignment
    switch( static_cast< const SdrTextVertAdjustItem& >( rItemSet.Get( SDRATTR_TEXT_VERTADJUST ) ).GetValue() )
    {
        case SDRTEXTVERTADJUST_TOP:     meVerAlign = xlTxoVAlignTop;        break;
        case SDRTEXTVERTADJUST_CENTER:  meVerAlign = xlTxoVAlignCenter;     break;
        case SDRTEXTVERTADJUST_BOTTOM:  meVerAlign = xlTxoVAlignBottom;     break;
        case SDRTEXTVERTADJUST_BLOCK:   meVerAlign = xlTxoVAlignJustify;    break;
    }

    // rotation
    long nAngle = rTextObj.GetRotateAngle();
    if( (4500 < nAngle) && (nAngle < 13500) )
        meRotation = xlTxoRot90ccw;
    else if( (22500 < nAngle) && (nAngle < 31500) )
        meRotation = xlTxoRot90cw;
    else
        meRotation = xlTxoNoRot;
}

void XclTxo::SaveCont( XclExpStream& rStrm )
{
    DBG_ASSERT( mpString.get(), "XclTxo::SaveCont - missing string" );

    sal_uInt16 nRunLen = 8 * mpString->GetFormatsCount();
    // alignment
    sal_uInt16 nFlags = 0;
    ::insert_value( nFlags, meHorAlign, 1, 3 );
    ::insert_value( nFlags, meVerAlign, 4, 3 );

    rStrm << nFlags << static_cast< sal_uInt16 >( meRotation );
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
        for( XclFormatRunVec::const_iterator aIter = rFormats.begin(), aEnd = rFormats.end(); aIter != aEnd; ++aIter )
            rStrm << *aIter << sal_uInt32( 0 );
        rStrm.EndRecord();
    }
}

UINT16 XclTxo::GetNum() const
{
    return EXC_ID_TXO;
}

ULONG XclTxo::GetLen() const
{
    return 18;
}


// --- class XclObjOle -------------------------------------------

XclObjOle::XclObjOle( const XclExpRoot& rRoot, const SdrObject& rObj ) :
    XclObj( rRoot, EXC_OBJ_CMO_PICTURE ),
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
    UINT32          nPictureId = UINT32(this);
    sprintf( aBuf, "%08X", nPictureId );        // #100211# - checked
    aStorageName.AppendAscii( aBuf );
    SvStorageRef    xOleStg = pRootStorage->OpenStorage( aStorageName,
                            STREAM_READWRITE| STREAM_SHARE_DENYALL );
    if( xOleStg.Is() )
    {
        SvInPlaceObjectRef xObj( ((SdrOle2Obj&)rOleObj).GetObjRef() );
        if ( xObj.Is() )
        {
            // set version to "old" version, because it must be
            // saved in MS notation.
            UINT32                  nFl = 0;
            OfaFilterOptions*       pFltOpts = OFF_APP()->GetFilterOptions();
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
            aOLEExpFilt.ExportOLEObject( *xObj, *xOleStg );

            // ftCf subrecord, undocumented as usual
            rStrm.StartRecord( EXC_ID_OBJ_FTCF, 2 );
            rStrm << UINT16(0x0002);
            rStrm.EndRecord();

            // ftPioGrbit subrecord, undocumented as usual
            rStrm.StartRecord( EXC_ID_OBJ_FTPIOGRBIT, 2 );
            rStrm << UINT16(0x0001);
            rStrm.EndRecord();

            // ftPictFmla subrecord, undocumented as usual
            XclExpUniString aName( xOleStg->GetUserName() );
            UINT16 nPadLen = (UINT16)(aName.GetSize() & 0x01);
            UINT16 nFmlaLen = static_cast< sal_uInt16 >( 12 + aName.GetSize() + nPadLen );
            UINT16 nSubRecLen = nFmlaLen + 6;

            rStrm.StartRecord( EXC_ID_OBJ_FTPICTFMLA, nSubRecLen );
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


// ----------------------------------------------------------------------------

XclExpCtrlLinkHelper::XclExpCtrlLinkHelper( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot ),
    mnEntryCount( 0 )
{
}

void XclExpCtrlLinkHelper::SetCellLink( const ScAddress& rCellLink )
{
    mpCellLink = CreateTokenArray( rCellLink );
}

void XclExpCtrlLinkHelper::SetSourceRange( const ScRange& rSrcRange )
{
    mpSrcRange = CreateTokenArray( rSrcRange );
    mnEntryCount = rSrcRange.aEnd.Col() - rSrcRange.aStart.Col() + 1;
}

void XclExpCtrlLinkHelper::WriteFormula( XclExpStream& rStrm, const ExcUPN& rTokArr ) const
{
    sal_uInt16 nFmlaSize = rTokArr.GetLen();
    rStrm << nFmlaSize << sal_uInt32( 0 );
    rStrm.Write( rTokArr.GetData(), nFmlaSize );
    if( nFmlaSize & 1 )
        rStrm << sal_uInt8( 0 );
}

::std::auto_ptr< ExcUPN > XclExpCtrlLinkHelper::CreateTokenArray( const ScTokenArray& rScTokArr ) const
{
    EC_Codetype eDummy;
    XclExpTokArrPtr pXclTokArr( new ExcUPN( mpRD, rScTokArr, eDummy ) );
    if( !pXclTokArr->GetLen() || !pXclTokArr->GetData() )
        pXclTokArr.reset();
    return pXclTokArr;
}

::std::auto_ptr< ExcUPN > XclExpCtrlLinkHelper::CreateTokenArray( const ScAddress& rPos ) const
{
    XclExpTokArrPtr pXclTokArr;
    XclExpTabIdBuffer& rTabIdBuffer = GetTabIdBuffer();
    if( rTabIdBuffer.IsExportTable( rPos.Tab() ) && !rTabIdBuffer.IsExternal( rPos.Tab() ) )
    {
        ScTokenArray aScTokArr;
        SingleRefData aRef;
        aRef.InitAddress( rPos );
        aScTokArr.AddSingleReference( aRef );
        pXclTokArr = CreateTokenArray( aScTokArr );
    }
    return pXclTokArr;
}

::std::auto_ptr< ExcUPN > XclExpCtrlLinkHelper::CreateTokenArray( const ScRange& rRange ) const
{
    XclExpTokArrPtr pXclTokArr;
    if( rRange.aStart == rRange.aEnd )
    {
        pXclTokArr = CreateTokenArray( rRange.aStart );
    }
    else if( rRange.aStart.Tab() == rRange.aEnd.Tab() )
    {
        XclExpTabIdBuffer& rTabIdBuffer = GetTabIdBuffer();
        if( rTabIdBuffer.IsExportTable( rRange.aStart.Tab() ) && !rTabIdBuffer.IsExternal( rRange.aStart.Tab() ) )
        {
            ScTokenArray aScTokArr;
            ComplRefData aRef;
            aRef.InitRange( rRange );
            aScTokArr.AddDoubleReference( aRef );
            pXclTokArr = CreateTokenArray( aScTokArr );
        }
    }
    return pXclTokArr;
}


// ----------------------------------------------------------------------------

#if EXC_EXP_OCX_CTRL

XclExpObjOcxCtrl::XclExpObjOcxCtrl(
        const XclExpRoot& rRoot,
        const Reference< XShape >& rxShape,
        const String& rClassName,
        sal_uInt32 nStrmStart, sal_uInt32 nStrmSize ) :
    XclObj( rRoot, EXC_OBJ_CMO_PICTURE, true ),
    XclExpCtrlLinkHelper( rRoot ),
    maClassName( rClassName ),
    mnStrmStart( nStrmStart ),
    mnStrmSize( nStrmSize )
{
    SetAutoLine( FALSE );

    XclEscherEx& rEscherEx = *pMsodrawing->GetEscherEx();
    rEscherEx.OpenContainer( ESCHER_SpContainer );
    rEscherEx.AddShape( ESCHER_ShpInst_HostControl, SHAPEFLAG_HAVESPT | SHAPEFLAG_HAVEANCHOR | SHAPEFLAG_OLESHAPE );
    Rectangle aDummyRect;
    EscherPropertyContainer aPropOpt( rEscherEx, rEscherEx.QueryPicStream(), aDummyRect );
    aPropOpt.AddOpt( ESCHER_Prop_FitTextToShape,    0x00080008 );   // bool field
    aPropOpt.AddOpt( ESCHER_Prop_lineColor,         0x08000040 );
    aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash,   0x00080000 );   // bool field

    Reference< XPropertySet > xShapePS( rxShape, UNO_QUERY );
    if( xShapePS.is() )
    {
        // meta file
        if( aPropOpt.CreateGraphicProperties( xShapePS, String( RTL_CONSTASCII_USTRINGPARAM( "MetaFile" ) ), sal_False ) )
        {
            sal_uInt32 nBlipId;
            if( aPropOpt.GetOpt( ESCHER_Prop_pib, nBlipId ) )
                aPropOpt.AddOpt( ESCHER_Prop_pictureId, nBlipId );
        }

        // name of the control
        OUString aCtrlName;
        //! TODO - this does not work - property is empty
        if( ::getPropValue( aCtrlName, xShapePS, PROPNAME( "Name" ) ) && aCtrlName.getLength() )
        {
            XclExpString aCtrlNameEx( aCtrlName, EXC_STR_FORCEUNICODE );
            sal_uInt32 nBufferSize = aCtrlNameEx.GetBufferSize() + 2;   // plus trailing zero
            sal_uInt8* pBuffer = new sal_uInt8[ nBufferSize ];
            aCtrlNameEx.WriteBuffer( pBuffer );
            pBuffer[ nBufferSize - 2 ] = pBuffer[ nBufferSize - 1 ] = 0;
            // aPropOpt takes ownership of pBuffer
            aPropOpt.AddOpt( ESCHER_Prop_wzName, TRUE, nBufferSize, pBuffer, nBufferSize );
        }
    }

    aPropOpt.Commit( rEscherEx.GetStream() );

    if( SdrObject* pSdrObj = ::GetSdrObjectFromXShape( rxShape ) )
        XclEscherClientAnchor( *mpRD, *pSdrObj ).WriteData( rEscherEx );
    rEscherEx.AddAtom( 0, ESCHER_ClientData );                       // OBJ record
    rEscherEx.CloseContainer();  // ESCHER_SpContainer

    pMsodrawing->UpdateStopPos();
}

void XclExpObjOcxCtrl::WriteSubRecs( XclExpStream& rStrm )
{
    // ftCf - clipboard format
    rStrm.StartRecord( EXC_ID_OBJ_FTCF, 2 );
    rStrm << sal_uInt16( 2 );
    rStrm.EndRecord();

    // ftPioGrbit
    rStrm.StartRecord( EXC_ID_OBJ_FTPIOGRBIT, 2 );
    rStrm << sal_uInt16( 0x0031 );
    rStrm.EndRecord();

    // ftPictFmla
    XclExpString aClass( maClassName );
    sal_uInt16 nClassNameSize = static_cast< sal_uInt16 >( aClass.GetSize() );
    sal_uInt16 nClassNamePad = nClassNameSize & 1;
    sal_uInt16 nFirstPartSize = 12 + nClassNameSize + nClassNamePad;
    sal_uInt16 nPictFmlaSize = nFirstPartSize + 18;

    rStrm.StartRecord( EXC_ID_OBJ_FTPICTFMLA, nPictFmlaSize );
    rStrm   << sal_uInt16( nFirstPartSize )             // size of first part
            << sal_uInt16( 5 )                          // formula size
            << sal_uInt32( 0 )                          // unknown ID
            << sal_uInt8( 0x02 ) << sal_uInt32( 0 )     // tTbl token with unknown ID
            << sal_uInt8( 3 )                           // pad to word
            << aClass;                                  // "Forms.***.1"
    rStrm.WriteZeroBytes( nClassNamePad );              // pad to word
    rStrm   << mnStrmStart                              // start in 'Ctls' stream
            << mnStrmSize                               // size in 'Ctls' stream
            << sal_uInt32( 0 ) << sal_uInt32( 0 );      // unknown
    rStrm.EndRecord();

    // TODO: writing the sheet link formulas
    DBG_ERRORFILE( "XclExpObjOcxCtrl::WriteSubRecs - export of sheet links not implemented" );
}

#else

XclExpObjTbxCtrl::XclExpObjTbxCtrl(
        const XclExpRoot& rRoot,
        const Reference< XShape >& rxShape,
        const Reference< XControlModel >& rxCtrlModel ) :
    XclObj( rRoot, EXC_OBJ_CMO_UNKNOWN, true ),
    XclExpCtrlLinkHelper( rRoot ),
    mnHeight( 0 ),
    mnState( 0 ),
    mnLineCount( 0 ),
    mnSelEntry( 0 ),
    mb3DStyle( false ),
    mbMultiSel( false )
{
    namespace FormCompType = ::com::sun::star::form::FormComponentType;

    Reference< XPropertySet > xPropSet( rxCtrlModel, UNO_QUERY );
    if( !rxShape.is() || !xPropSet.is() )
        return;

    mnHeight = rxShape->getSize().Height;
    if( !mnHeight )
        return;

    // control type
    sal_Int16 nClassId;
    if( ::getPropValue( nClassId, xPropSet, PROPNAME( "ClassId" ) ) )
    {
        switch( nClassId )
        {
            case FormCompType::COMMANDBUTTON:   mnObjType = EXC_OBJ_CMO_BUTTON;         break;
            case FormCompType::RADIOBUTTON:     mnObjType = EXC_OBJ_CMO_OPTIONBUTTON;   break;
            case FormCompType::CHECKBOX:        mnObjType = EXC_OBJ_CMO_CHECKBOX;       break;
            case FormCompType::LISTBOX:         mnObjType = EXC_OBJ_CMO_LISTBOX;        break;
            case FormCompType::COMBOBOX:        mnObjType = EXC_OBJ_CMO_COMBOBOX;       break;
            case FormCompType::GROUPBOX:        mnObjType = EXC_OBJ_CMO_GROUPBOX;       break;
            case FormCompType::FIXEDTEXT:       mnObjType = EXC_OBJ_CMO_LABEL;          break;
        }
    }
    if( mnObjType == EXC_OBJ_CMO_UNKNOWN )
        return;

    // OBJ record flags
    SetLocked( TRUE );
    SetPrintable( TRUE );
    SetAutoFill( FALSE );
    SetAutoLine( FALSE );

    // fill Escher properties
    XclEscherEx& rEscherEx = *pMsodrawing->GetEscherEx();
    rEscherEx.OpenContainer( ESCHER_SpContainer );
    rEscherEx.AddShape( ESCHER_ShpInst_HostControl, SHAPEFLAG_HAVEANCHOR | SHAPEFLAG_HAVESPT );
    EscherPropertyContainer aPropOpt;
    aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x01000100 ); // bool field
    aPropOpt.AddOpt( ESCHER_Prop_lTxid, 0 );                        // Text ID
    aPropOpt.AddOpt( ESCHER_Prop_WrapText, 0x00000001 );
    aPropOpt.AddOpt( ESCHER_Prop_FitTextToShape, 0x001A0008 );      // bool field
    aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x00100000 );      // bool field
    aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x00080000 );     // bool field
    aPropOpt.Commit( rEscherEx.GetStream() );

    // anchor
    if( SdrObject* pSdrObj = ::GetSdrObjectFromXShape( rxShape ) )
        XclEscherClientAnchor( *mpRD, *pSdrObj ).WriteData( rEscherEx );
    rEscherEx.AddAtom( 0, ESCHER_ClientData );                       // OBJ record
    pMsodrawing->UpdateStopPos();

    // control label
    OUString aString;
    if( ::getPropValue( aString, xPropSet, PROPNAME( "Label" ) ) && aString.getLength() )
    {
        /*  Be sure to construct the MSODRAWING ClientTextbox record after the
            base OBJ's MSODRAWING record Escher data is completed. */
        pClientTextbox = new XclMsodrawing( GetRoot() );
        pClientTextbox->GetEscherEx()->AddAtom( 0, ESCHER_ClientTextbox );  // TXO record
        pClientTextbox->UpdateStopPos();

        XclFontData aFontData;
        OUString aFontName;
        float fFloatVal;
        sal_Int16 nShortVal;

        if( ::getPropValue( aFontName, xPropSet, PROPNAME( "FontName" ) ) )
            aFontData.maName = XclTools::GetXclFontName( aFontName );
        if( ::getPropValue( fFloatVal, xPropSet, PROPNAME( "FontHeight" ) ) )
            aFontData.SetApiHeight( fFloatVal );
        if( ::getPropValue( nShortVal, xPropSet, PROPNAME( "FontFamily" ) ) )
            aFontData.SetApiFamily( nShortVal );
        if( ::getPropValue( nShortVal, xPropSet, PROPNAME( "FontCharset" ) ) )
            aFontData.SetApiCharSet( nShortVal );
        if( ::getPropValue( nShortVal, xPropSet, PROPNAME( "FontSlant" ) ) )
            aFontData.SetApiPosture( static_cast< ::com::sun::star::awt::FontSlant >( nShortVal ) );
        if( ::getPropValue( fFloatVal, xPropSet, PROPNAME( "FontWeight" ) ) )
            aFontData.SetApiWeight( fFloatVal );
        if( ::getPropValue( nShortVal, xPropSet, PROPNAME( "FontUnderline" ) ) )
            aFontData.SetApiUnderline( nShortVal );
        if( ::getPropValue( nShortVal, xPropSet, PROPNAME( "FontStrikeout" ) ) )
            aFontData.SetApiStrikeout( nShortVal );

        sal_uInt16 nFontIx = EXC_FONT_APP;
        if( aFontData.maName.Len() && aFontData.mnHeight )
        {
            XclExpFont* pFont = new XclExpFont( GetRoot(), aFontData );
            sal_Int32 nApiColor;
            if( ::getPropValue( nApiColor, xPropSet, PROPNAME( "TextColor" ) ) )
            {
                Color aColor( static_cast< sal_uInt32 >( nApiColor ) );
                pFont->SetColorId( rRoot.GetPalette().InsertColor( aColor, xlColorChartText, EXC_COLOR_FONTAUTO ) );
            }
            nFontIx = GetFontBuffer().Insert( pFont );
            // font buffer owns pFont, forget it
        }

        pTxo = new XclTxo( aString, nFontIx );
        pTxo->SetHorAlign( (mnObjType == EXC_OBJ_CMO_BUTTON) ? xlTxoHAlignCenter : xlTxoHAlignLeft );
        pTxo->SetVerAlign( xlTxoVAlignCenter );
    }

    rEscherEx.CloseContainer();  // ESCHER_SpContainer

    // other properties
    ::getPropValue( mnLineCount, xPropSet, PROPNAME( "LineCount" ) );

    sal_Int16 nBorder = 0;
    if( ::getPropValue( nBorder, xPropSet, PROPNAME( "Border" ) ) )
        mb3DStyle = (nBorder == 2);

    sal_Int16 nApiState = 0;
    if( ::getPropValue( nApiState, xPropSet, PROPNAME( "State" ) ) )
    {
        switch( nApiState )
        {
            case 0: mnState = EXC_OBJ_CBLS_STATE_UNCHECK;   break;
            case 1: mnState = EXC_OBJ_CBLS_STATE_CHECK;     break;
            case 2: mnState = EXC_OBJ_CBLS_STATE_TRI;       break;
        }
    }

    // selection
    switch( nClassId )
    {
        case FormCompType::LISTBOX:
        {
            mbMultiSel = ::getPropBool( xPropSet, CREATE_OUSTRING( "MultiSelection" ) );
            Sequence< sal_Int16 > aSelection;
            if( ::getPropValue( aSelection, xPropSet, PROPNAME( "SelectedItems" ) ) )
            {
                sal_Int32 nLen = aSelection.getLength();
                if( nLen > 0 )
                {
                    mnSelEntry = aSelection[ 0 ] + 1;
                    maMultiSel.resize( nLen );
                    const sal_Int16* pnBegin = aSelection.getConstArray();
                    ::std::copy( pnBegin, pnBegin + nLen, maMultiSel.begin() );
                }
            }

            // convert listbox with dropdown button to Excel combobox
            if( ::getPropBool( xPropSet, PROPNAME( "Dropdown" ) ) )
                mnObjType = EXC_OBJ_CMO_COMBOBOX;
        }
        break;
        case FormCompType::COMBOBOX:
        {
            Sequence< OUString > aStringList;
            OUString aDefText;
            if( ::getPropValue( aStringList, xPropSet, PROPNAME( "StringItemList" ) ) &&
                ::getPropValue( aDefText, xPropSet, PROPNAME( "Text" ) ) &&
                aStringList.getLength() && aDefText.getLength() )
            {
                const OUString* pBegin = aStringList.getConstArray();
                const OUString* pEnd = pBegin + aStringList.getLength();
                const OUString* pString = ::std::find( pBegin, pEnd, aDefText );
                if( pString != pEnd )
                    mnSelEntry = static_cast< sal_Int16 >( pString - pBegin + 1 );  // 1-based
                if( mnSelEntry > 0 )
                    maMultiSel.resize( 1, mnSelEntry - 1 );
            }

            // convert combobox without dropdown button to Excel listbox
            if( !::getPropBool( xPropSet, PROPNAME( "Dropdown" ) ) )
                mnObjType = EXC_OBJ_CMO_LISTBOX;
        }
        break;
    }
}

XclExpObjTbxCtrl::~XclExpObjTbxCtrl()
{
}

void XclExpObjTbxCtrl::WriteSubRecs( XclExpStream& rStrm )
{
    switch( mnObjType )
    {
        // *** Check boxes, option buttons ***

        case EXC_OBJ_CMO_CHECKBOX:
        case EXC_OBJ_CMO_OPTIONBUTTON:
        {
            // ftCbls - box properties
            sal_uInt16 nStyle = 0;
            ::set_flag( nStyle, EXC_OBJ_CBLS_3D, mb3DStyle );

            rStrm.StartRecord( EXC_ID_OBJ_FTCBLS, 12 );
            rStrm << mnState;
            rStrm.WriteZeroBytes( 8 );
            rStrm << nStyle;
            rStrm.EndRecord();

            // ftCblsFmla - cell link
            if( const ExcUPN* pCellLink = GetCellLinkTokArr() )
            {
                rStrm.StartRecord( EXC_ID_OBJ_FTCBLSFMLA, 0 );
                WriteFormula( rStrm, *pCellLink );
                rStrm.EndRecord();
            }
        }
        break;

        // *** List boxes, combo boxes ***

        case EXC_OBJ_CMO_LISTBOX:
        case EXC_OBJ_CMO_COMBOBOX:
        {
            sal_uInt16 nEntryCount = GetSourceEntryCount();

            // ftSbs subrecord - Scroll bars
            sal_Int32 nLineHeight = XclTools::GetHmmFromTwips( 200 );   // always 10pt
            sal_uInt16 nVisLines = 0;
            if( mnObjType == EXC_OBJ_CMO_LISTBOX )
                nVisLines = static_cast< sal_uInt16 >( mnHeight / nLineHeight );
            else
                nVisLines = mnLineCount;
            sal_uInt16 nInvisLines = (nEntryCount >= nVisLines) ? (nEntryCount - nVisLines) : 0;
            rStrm.StartRecord( EXC_ID_OBJ_FTSBS, 20 );
            rStrm   << sal_uInt32( 0 )              // reserved
                    << sal_uInt16( 0 )              // thumb position
                    << sal_uInt16( 0 )              // thumb min pos
                    << sal_uInt16( nInvisLines )    // thumb max pos
                    << sal_uInt16( 1 )              // line increment
                    << sal_uInt16( nVisLines )      // page increment
                    << sal_uInt16( 0 )              // 0 = vertical, 1 = horizontal
                    << sal_uInt16( 16 )             // thumb width
                    << sal_uInt16( 1 );             // reserved
            rStrm.EndRecord();

            // ftSbsFmla - cell link
            if( const ExcUPN* pCellLink = GetCellLinkTokArr() )
            {
                rStrm.StartRecord( EXC_ID_OBJ_FTSBSFMLA, 0 );
                WriteFormula( rStrm, *pCellLink );
                rStrm.EndRecord();
            }

            // ftLbsData - source data range and box properties
            sal_uInt16 nStyle = mbMultiSel ? EXC_OBJ_LBS_SEL_MULTI : EXC_OBJ_LBS_SEL_SIMPLE;
            ::set_flag( nStyle, EXC_OBJ_LBS_3D, mb3DStyle );

            rStrm.StartRecord( EXC_ID_OBJ_FTLBSDATA, 0 );

            if( const ExcUPN* pSrcRange = GetSourceRangeTokArr() )
            {
                rStrm << static_cast< sal_uInt16 >( (pSrcRange->GetLen() + 7) & 0xFFFE );
                WriteFormula( rStrm, *pSrcRange );
            }
            else
                rStrm << sal_uInt16( 0 );

            rStrm << nEntryCount << mnSelEntry << nStyle << sal_uInt16( 0 );
            if( mnObjType == EXC_OBJ_CMO_LISTBOX )
            {
                if( nEntryCount )
                {
                    ScfUInt8Vec aSelEx( nEntryCount, 0 );
                    for( ScfInt16Vec::const_iterator aIt = maMultiSel.begin(), aEnd = maMultiSel.end(); aIt != aEnd; ++aIt )
                        if( *aIt < nEntryCount )
                            aSelEx[ *aIt ] = 1;
                    rStrm.Write( &aSelEx[ 0 ], aSelEx.size() );
                }
            }
            else if( mnObjType == EXC_OBJ_CMO_COMBOBOX )
            {
                rStrm << sal_uInt16( 0 ) << mnLineCount;
            }

            rStrm.EndRecord();
        }
        break;
    }
}

#endif

// --- class XclObjAny -------------------------------------------

XclObjAny::XclObjAny( const XclExpRoot& rRoot ) :
    XclObj( rRoot, EXC_OBJ_CMO_UNKNOWN )
{
}

XclObjAny::~XclObjAny()
{
}

void XclObjAny::WriteSubRecs( XclExpStream& rStrm )
{
    if( mnObjType == EXC_OBJ_CMO_GROUP )
        // ftGmo subrecord
        rStrm << EXC_ID_OBJ_FTGMO << UINT16(2) << UINT16(0);
}

void XclObjAny::Save( XclExpStream& rStrm )
{
    if( mnObjType == EXC_OBJ_CMO_GROUP )
        // old size + ftGmo
        SetRecSize( GetRecSize() + 6 );

    // content of this record
    XclObj::Save( rStrm );
}


// ----------------------------------------------------------------------------

XclExpNote::XclExpNote(
        const XclExpRoot& rRoot, const ScAddress& rPos,
        const ScPostIt* pScNote, const String& rAddText ) :
    XclExpRecord( EXC_ID_NOTE ),
    maPos( rPos ),
    mnObjId( 0 ),
    mbVisible( pScNote && pScNote->IsShown() )
{
    String aNoteText;

    // read strings from note object, if present
    if( pScNote )
    {
        aNoteText = pScNote->GetText();
        maAuthor.Assign( pScNote->GetAuthor() );
    }

    // append additional text
    ScGlobal::AddToken( aNoteText, rAddText, '\n', 2 );

    // create the Escher object
    mnObjId = rRoot.mpRD->pObjRecs->Add( new XclObjComment( rRoot, maPos, aNoteText, mbVisible ) );

    SetRecSize( 9 + maAuthor.GetSize() );
}

void XclExpNote::Save( XclExpStream& rStrm )
{
    if( mnObjId )
        XclExpRecord::Save( rStrm );
}

void XclExpNote::WriteBody( XclExpStream& rStrm )
{
    sal_uInt16 nFlags = mbVisible ? EXC_NOTE_VISIBLE : 0;
    rStrm   << (sal_uInt16) maPos.Row() << (sal_uInt16) maPos.Col()
            << nFlags << mnObjId << maAuthor << sal_uInt8( 0 );
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


ULONG ExcBof8_Base::GetLen() const
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


// --- class ExcBofC8 ------------------------------------------------

ExcBofC8::ExcBofC8()
{
    nDocType = 0x0020;
}

// --- class ExcLabelSst ---------------------------------------------

ExcLabelSst::ExcLabelSst(
        const ScAddress& rPos,
        const ScPatternAttr* pAttr,
        RootData& rRoot,
        const String& rNewText ) :
    ExcCell( rPos, pAttr, rRoot )
{
    nIsst = rRoot.pER->GetSst().Insert( new XclExpUniString( rNewText ) );
}


ExcLabelSst::ExcLabelSst(
        const ScAddress& rPos,
        const ScPatternAttr* pAttr,
        RootData& rRootData,
        const ScEditCell& rEdCell ) :
    ExcCell( rPos, pAttr, rRootData )
{
    const XclExpRoot& rRoot = *rRootData.pER;
    XclExpString* pString = XclExpStringHelper::CreateString( rRoot, rEdCell, pAttr );
    SetXFId( rRoot.GetXFBuffer().Insert( pAttr, pString->IsWrapped(), pString->RemoveFontOfChar(0)) );
    nIsst = rRoot.GetSst().Insert( pString );
}


ExcLabelSst::~ExcLabelSst()
{
}


void ExcLabelSst::SaveDiff( XclExpStream& rStrm )
{
    rStrm << nIsst;
}


UINT16 ExcLabelSst::GetNum() const
{
    return 0x00FD;
}


ULONG ExcLabelSst::GetDiffLen() const
{
    return 4;
}



// --- class ExcBundlesheet8 -----------------------------------------

ExcBundlesheet8::ExcBundlesheet8( RootData& rRootData, UINT16 nTab ) :
    ExcBundlesheetBase( rRootData, nTab )
{
    String sTabName;
    rRootData.pDoc->GetName( nTab, sTabName );
    aUnicodeName.Assign( sTabName, EXC_STR_8BITLENGTH );
}


ExcBundlesheet8::ExcBundlesheet8( const String& rString ) :
    ExcBundlesheetBase(),
    aUnicodeName( rString, EXC_STR_8BITLENGTH )
{
}


void ExcBundlesheet8::SaveCont( XclExpStream& rStrm )
{
    nOwnPos = rStrm.GetStreamPos();
    // write dummy position, real position comes later
    rStrm << sal_uInt32( 0 ) << nGrbit << aUnicodeName;
}


ULONG ExcBundlesheet8::GetLen() const
{   // Text max 255 chars
    return 8 + aUnicodeName.GetBufferSize();
}



// --- class ExcWindow18 ---------------------------------------------

ExcWindow18::ExcWindow18( RootData& rRootData )
{
    ScExtDocOptions& rOpt = *rRootData.pExtDocOpt;
    nCurrTable = rOpt.nActTab;
    nSelTabs = rOpt.nSelTabs;
}


void ExcWindow18::SaveCont( XclExpStream& rStrm )
{
    rStrm   << (UINT16) 0x01E0
            << (UINT16) 0x005A
            << (UINT16) 0x3FCF
            << (UINT16) 0x2A4E
            << (UINT16) 0x0038
            << nCurrTable
            << (UINT16) 0x0000
            << nSelTabs
            << (UINT16) 0x0258;
}


UINT16 ExcWindow18::GetNum( void ) const
{
    return 0x003D;
}


ULONG ExcWindow18::GetLen( void ) const
{
    return 18;
}


// --- class ExcPane8 ------------------------------------------------

ExcPane8::ExcPane8( const ScExtTabOptions& rTabOptions ) :
    nSplitX( rTabOptions.nSplitX ),
    nSplitY( rTabOptions.nSplitY ),
    nLeftCol( rTabOptions.nLeftSplitCol ),
    nTopRow( rTabOptions.nTopSplitRow ),
    nActivePane( rTabOptions.nActPane )
{
}


void ExcPane8::SaveCont( XclExpStream& rStrm )
{
    rStrm << nSplitX << nSplitY << nTopRow << nLeftCol << nActivePane;
}


UINT16 ExcPane8::GetNum() const
{
    return 0x0041;
}


ULONG ExcPane8::GetLen() const
{
    return 10;
}


// --- class ExcWindow28 ---------------------------------------------

ExcWindow28::ExcWindow28( const XclExpRoot& rRoot, UINT16 nTab ) :
    XclExpRoot( rRoot ),
    pPaneRec( NULL ),
    nFlags( 0 ),
    nLeftCol( 0 ),
    nTopRow( 0 ),
    nActiveCol( 0 ),
    nActiveRow( 0 ),
    bHorSplit( FALSE ),
    bVertSplit( FALSE )
{
    const ScViewOptions& rViewOpt = GetDoc().GetViewOptions();
    nFlags |= rViewOpt.GetOption( VOPT_FORMULAS ) ? EXC_WIN2_SHOWFORMULAS : 0;
    nFlags |= rViewOpt.GetOption( VOPT_GRID ) ? EXC_WIN2_SHOWGRID : 0;
    nFlags |= rViewOpt.GetOption( VOPT_HEADER ) ? EXC_WIN2_SHOWHEADINGS : 0;
    nFlags |= rViewOpt.GetOption( VOPT_NULLVALS ) ? EXC_WIN2_SHOWZEROS : 0;
    nFlags |= rViewOpt.GetOption( VOPT_OUTLINER ) ? EXC_WIN2_OUTLINE : 0;

    ScExtDocOptions& rOpt = *rRoot.mpRD->pExtDocOpt;
    XclExpPalette& rPal = GetPalette();
    nFlags |= (nTab == rOpt.nActTab) ? (EXC_WIN2_DISPLAYED|EXC_WIN2_SELECTED) : 0;
    nFlags |= rOpt.pGridCol ? 0 : EXC_WIN2_DEFAULTCOLOR;
    nGridColorSer = rOpt.pGridCol ?
        rPal.InsertColor( *rOpt.pGridCol, xlColorGrid ) :
        XclExpPalette::GetColorIdFromIndex( EXC_COLOR_WINDOWTEXT );

    const ScExtTabOptions* pTabOpt = rOpt.GetExtTabOptions( nTab );
    if( pTabOpt )
    {
        nFlags |= pTabOpt->bSelected ? EXC_WIN2_SELECTED : 0;
        nFlags |= pTabOpt->bFrozen ? (EXC_WIN2_FROZEN|EXC_WIN2_FROZENNOSPLIT) : 0;
        nLeftCol = pTabOpt->nLeftCol;
        nTopRow = pTabOpt->nTopRow;
        nActiveCol = pTabOpt->aLastSel.aStart.Col();
        nActiveRow = pTabOpt->aLastSel.aStart.Row();
        bHorSplit = (pTabOpt->nSplitX != 0);
        bVertSplit = (pTabOpt->nSplitY != 0);
        if( bHorSplit || bVertSplit )
            pPaneRec = new ExcPane8( *pTabOpt );
    }
}


ExcWindow28::~ExcWindow28()
{
    if( pPaneRec )
        delete pPaneRec;
}


void ExcWindow28::SaveCont( XclExpStream& rStrm )
{
    rStrm   << nFlags
            << nTopRow
            << nLeftCol
            << (UINT32) GetPalette().GetColorIndex( nGridColorSer )
            << (UINT32) 0
            << (UINT32) 0;
}


void ExcWindow28::Save( XclExpStream& rStrm )
{
    ExcRecord::Save( rStrm );
    if( pPaneRec )
        pPaneRec->Save( rStrm );
    ExcSelection( nActiveCol, nActiveRow, 3 ).Save( rStrm );
    if( bHorSplit )
        ExcSelection( nActiveCol, nActiveRow, 1 ).Save( rStrm );
    if( bVertSplit )
        ExcSelection( nActiveCol, nActiveRow, 2 ).Save( rStrm );
    if( bHorSplit && bVertSplit )
        ExcSelection( nActiveCol, nActiveRow, 0 ).Save( rStrm );
}


UINT16 ExcWindow28::GetNum() const
{
    return 0x023E;
}


ULONG ExcWindow28::GetLen() const
{
    return 18;
}


// --- class XclObproj -----------------------------------------------

UINT16 XclObproj::GetNum() const
{
    return 0x00D3;
}


ULONG XclObproj::GetLen() const
{
    return 0;
}



// --- class XclDConRef ----------------------------------------------

XclDConRef::XclDConRef( const ScRange& rSrcR, const String& rWB ) :
        aSourceRange( rSrcR )
{
    String  sTemp( ( sal_Unicode ) 0x02 );

    sTemp += rWB;
    pWorkbook = new XclExpUniString( sTemp );
}

XclDConRef::~XclDConRef()
{
    delete pWorkbook;
}

void XclDConRef::SaveCont( XclExpStream& rStrm )
{
    rStrm   << (UINT16) aSourceRange.aStart.Row()
            << (UINT16) aSourceRange.aEnd.Row()
            << (UINT8)  aSourceRange.aStart.Col()
            << (UINT8)  aSourceRange.aEnd.Col()
            << *pWorkbook
            << (UINT8)  0x00;
}

UINT16 XclDConRef::GetNum() const
{
    return 0x0051;
}

ULONG XclDConRef::GetLen() const
{
    return 7 + pWorkbook->GetSize();
}



// --- class XclExpCellMerging ---------------------------------------

void XclExpCellMerging::Append( UINT16 nCol1, UINT16 nColCnt, UINT16 nRow1, UINT16 nRowCnt, sal_uInt32 nXFId )
{
    AppendCell( new XclExpMergedCell( nCol1, nCol1 + nColCnt - 1, nRow1, nRow1 + nRowCnt - 1, nXFId ) );
}


BOOL XclExpCellMerging::FindNextMerge( const ScAddress& rPos, UINT16& rnCol )
{
    rnCol = 0xFFFF;
    for( XclExpMergedCell* pCell = FirstCell(); pCell; pCell = NextCell() )
        if( (pCell->nRow1 <= rPos.Row()) && (rPos.Row() <= pCell->nRow2) &&
            (rPos.Col() <= pCell->nCol1) && (pCell->nCol1 < rnCol) )
            rnCol = pCell->nCol1;
    return rnCol < 0xFFFF;
}


BOOL XclExpCellMerging::FindMergeBaseXF( const ScAddress& rPos, sal_uInt32& rnXFId, UINT16& rnColCount )
{
    for( XclExpMergedCell* pCell = FirstCell(); pCell; pCell = NextCell() )
        if( (pCell->nCol1 <= rPos.Col()) && (rPos.Col() <= pCell->nCol2) &&
            (pCell->nRow1 <= rPos.Row()) && (rPos.Row() <= pCell->nRow2) )
        {
            rnXFId = pCell->mnXFId;
            rnColCount = pCell->nCol2 - rPos.Col() + 1;
            return TRUE;
        }
    return FALSE;
}


void XclExpCellMerging::Save( XclExpStream& rStrm )
{
    ULONG nCount = aCellList.Count();
    if( !nCount ) return;

    ULONG nIndex = 0;
    while( nCount )
    {
        ULONG nPortion = Min( nCount, (ULONG)EXC_MERGE_MAXCOUNT );
        nCount -= nPortion;
        rStrm.StartRecord( 0x00E5, 2 + nPortion * 8 );
        rStrm << (UINT16) nPortion;
        while( nPortion-- )
        {
            XclExpMergedCell* pCell = GetCell( nIndex++ );
            if( pCell )
                rStrm << *pCell;
        }
        rStrm.EndRecord();
    }
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


ULONG XclCodename::GetLen() const
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




XclExpUniString ExcEScenario::sUsername;

ExcEScenario::ExcEScenario( ScDocument& rDoc, UINT16 nTab )
{
    String  sTmpName;
    String  sTmpComm;
    Color   aDummyCol;
    UINT16  nDummyFlags;

    rDoc.GetName( nTab, sTmpName );
    sName.Assign( sTmpName, EXC_STR_8BITLENGTH );
    nRecLen = 8 + sName.GetBufferSize();

    rDoc.GetScenarioData( nTab, sTmpComm, aDummyCol, nDummyFlags );
    sComment.Assign( sTmpComm, EXC_STR_DEFAULT, 255 );
    if( sComment.Len() )
        nRecLen += sComment.GetSize();

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
    UINT16  nRow, nCol;
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
                bContLoop = Append( nCol, nRow, sText );
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
    nRecLen += 6 + pCell->GetStringBytes();     // 4 bytes address, 2 bytes ifmt
    return TRUE;
}

void ExcEScenario::SaveCont( XclExpStream& rStrm )
{
    rStrm   << (UINT16) List::Count()       // number of cells
            << (UINT8) 1                    // fLocked
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

ULONG ExcEScenario::GetLen() const
{
    return nRecLen;
}




ExcEScenarioManager::ExcEScenarioManager( ScDocument& rDoc, UINT16 nTab ) :
        nActive( 0 )
{
    if( rDoc.IsScenario( nTab ) )
        return;

    UINT16 nFirstTab    = nTab + 1;
    UINT16 nNewTab      = nFirstTab;

    while( rDoc.IsScenario( nNewTab ) )
    {
        Append( new ExcEScenario( rDoc, nNewTab ) );

        if( rDoc.IsActiveScenario( nNewTab ) )
            nActive = nNewTab - nFirstTab;
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

UINT16 ExcEScenarioManager::GetNum() const
{
    return 0x00AE;
}

ULONG ExcEScenarioManager::GetLen() const
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
const ULONG XclProtection::nMyLen = sizeof( XclProtection::pMyData );

ULONG XclProtection::GetLen( void ) const
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


ULONG XclCalccount::GetLen() const
{
    return 2;
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


ULONG XclIteration::GetLen() const
{
    return 2;
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


ULONG XclDelta::GetLen() const
{
    return 8;
}



