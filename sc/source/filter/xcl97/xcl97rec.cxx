/*************************************************************************
 *
 *  $RCSfile: xcl97rec.cxx,v $
 *
 *  $Revision: 1.52 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-08 16:29:57 $
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

#ifndef _SVDPOOL_HXX //autogen wg. SdrItemPool
#include <svx/svdpool.hxx>
#endif
#ifndef _SDTAITM_HXX //autogen wg. SdrTextHorzAdjustItem, SdrTextVertAdjustItem
#include <svx/sdtaitm.hxx>
#endif
#ifndef _SVDOTEXT_HXX //autogen wg. SdrTextObj
#include <svx/svdotext.hxx>
#endif
#ifndef _OUTLOBJ_HXX //autogen wg. OutlinerParaObject
#include <svx/outlobj.hxx>
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

#include "scitems.hxx"

#include <offmgr/app.hxx>
#include <offmgr/fltrcfg.hxx>
#include <svx/brshitem.hxx>
#include <svx/boxitem.hxx>
#ifndef _SVX_FRMDIRITEM_HXX
#include <svx/frmdiritem.hxx>
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

using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::drawing::XShape;


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

XclMsodrawing::XclMsodrawing( RootData& rRoot, UINT16 nEscherType, UINT32 nInitialSize )
        :
        XclMsodrawing_Base( *rRoot.pEscher, nInitialSize )
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

XclObjList::XclObjList( RootData& rRoot )
        :
        ExcRoot( &rRoot ),
        pMsodrawingPerSheet( new XclMsodrawing( rRoot, ESCHER_DgContainer ) ),
        pSolverContainer( NULL )
{
}


XclObjList::~XclObjList()
{
    for ( XclObj* p = First(); p; p = Next() )
    {
        delete p;
    }
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
        pSolverContainer = new XclMsodrawing( *pExcRoot, ESCHER_SolverContainer, nSolverSize );

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

XclObj::XclObj( ObjType eType, RootData& rRoot, bool bOwnEscher )
            :
            pClientTextbox( NULL ),
            pTxo( NULL ),
            eObjType( eType ),
            nObjId(0),
            nGrbit( 0x6011 ),   // AutoLine, AutoFill, Printable, Locked
            bFirstOnSheet( rRoot.pObjRecs->Count() == 0 ),
            mbOwnEscher( bOwnEscher )
{
    //! first object continues the first MSODRAWING record
    if ( bFirstOnSheet )
        pMsodrawing = rRoot.pObjRecs->GetMsodrawingPerSheet();
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
            eObjType = otLine;
        break;
        case ESCHER_ShpInst_Rectangle :
        case ESCHER_ShpInst_RoundRectangle :
            eObjType = otRectangle;
        break;
        case ESCHER_ShpInst_Ellipse :
            eObjType = otOval;
        break;
        case ESCHER_ShpInst_Arc :
            eObjType = otArc;
        break;
        case ESCHER_ShpInst_TextBox :
            eObjType = otText;
        break;
        case ESCHER_ShpInst_PictureFrame :
            eObjType = otPicture;
        break;
        default:
            eObjType = otMsOffDrawing;
    }
}


void XclObj::SetText( RootData& rRoot, const SdrTextObj& rObj )
{
    DBG_ASSERT( !pClientTextbox, "XclObj::SetText: already set" );
    if ( !pClientTextbox )
    {
        pMsodrawing->UpdateStopPos();
        pClientTextbox = new XclMsodrawing( rRoot );
        pClientTextbox->GetEscherEx()->AddAtom( 0, ESCHER_ClientTextbox );  // TXO record
        pClientTextbox->UpdateStopPos();
        pTxo = new XclTxo( rObj );
    }
}


void XclObj::SaveCont( XclExpStream& rStrm )
{   // ftCmo subrecord
    DBG_ASSERT( eObjType != otUnknown, "XclObj::SaveCont: unknown type" );
    rStrm << (UINT16) ftCmo << (UINT16) 0x0012;
    rStrm << (UINT16) eObjType << nObjId << nGrbit
        << UINT32(0) << UINT32(0) << UINT32(0);
}


void XclObj::Save( XclExpStream& rStrm )
{
    // MSODRAWING record (msofbtSpContainer)
    if ( !bFirstOnSheet )
        pMsodrawing->Save( rStrm );

    // OBJ
    ExcRecord::Save( rStrm );
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


UINT16 XclObj::GetNum() const
{   // the real OBJ
    return 0x005D;
}


ULONG XclObj::GetLen() const
{   // length of subrecord including ID and LEN
    return 22;
}



// --- class XclObjComment -------------------------------------------

XclObjComment::XclObjComment( RootData& rRoot, const ScAddress& rPos, const String& rStr, bool bVisible )
            :
            XclObj( otComment, rRoot, true )
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

    XclEscherClientAnchor( rRoot.pDoc, rPos ).WriteData( *pEx );
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


void XclObjComment::SaveCont( XclExpStream& rStrm )
{
    // ftCmo subrecord
    XclObj::SaveCont( rStrm );

#if 0
/*
    // ftNts subrecord, Note structure, where the hell is this documented?!?
    // seems like we don't need it ...
    rStrm << UINT16( ftNts ) << UINT16(0x0016)
        << UINT32(0) << UINT32(0) << UINT16(0)
        << UINT16(0xa000) << UINT16(0xcf24) << UINT16(0xf78e) << UINT16(0);
    if ( bFirstOnSheet )
        rStrm << UINT16(0x0010);
    else
        rStrm << UINT16(0x0158);
    rStrm << UINT16(0);
*/
#endif

    // ftEnd subrecord
    rStrm << UINT16(0) << UINT16(0);
}


void XclObjComment::Save( XclExpStream& rStrm )
{
    // content of this record
    XclObj::Save( rStrm );
    // second MSODRAWING record and TXO and CONTINUE records
    XclObj::SaveTextRecs( rStrm );
}


ULONG XclObjComment::GetLen() const
{   // length of all subrecords including IDs and LENs
    return XclObj::GetLen() /* + 4 + 0x0016 */ + 4;
}



// --- class XclObjDropDown ------------------------------------------

XclObjDropDown::XclObjDropDown( RootData& rRoot, const ScAddress& rPos, BOOL bFilt ) :
        XclObj( otComboBox, rRoot, true ),
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

    XclEscherClientAnchor aAnchor( rRoot, 0x0001 );             // MsofbtClientAnchor
    aAnchor.SetDropDownPosition( rPos );
    aAnchor.WriteData( *pEx );

    pEx->AddAtom( 0, ESCHER_ClientData );                       // OBJ record
    pMsodrawing->UpdateStopPos();
    pEx->CloseContainer();  // ESCHER_SpContainer
}

XclObjDropDown::~XclObjDropDown()
{
}

void XclObjDropDown::SaveCont( XclExpStream& rStrm )
{
    // ftCmo subrecord
    XclObj::SaveCont( rStrm );

    // ftSbs subrecord - Scroll bars (dummy)
    rStrm   << (UINT16)0x000C << (UINT16)0x0014
            << (UINT32)0 << (UINT32)0 << (UINT32)0 << (UINT32)0 << (UINT32)0;

    // ftLbsData subrecord - Listbox data
    rStrm   << (UINT16)0x0013 << (UINT16)0x0010
            << (UINT32)0 << (UINT16)0 << (UINT16)0x0301 << (UINT16)0;
    rStrm   << (bIsFiltered ? (UINT16)0x000A : (UINT16)0x0002) << (UINT32)0;

    // ftEnd subrecord
    rStrm << (UINT16)0 << (UINT16)0;
}

ULONG XclObjDropDown::GetLen() const
{
    // length of all subrecords including IDs and LENs
    // (all from XclObj) + ftSbs + ftLbsData + ftEnd
    return XclObj::GetLen() + 24 + 20 + 4;
}



// --- class XclTxo --------------------------------------------------

XclTxo::XclTxo( const String& rStr ) :
    aText( rStr ),
    nGrbit(0),
    nRot(0)
{
}


XclTxo::XclTxo( const SdrTextObj& rTextObj ) :
    nGrbit(0),
    nRot(0)
{
    String aStr;
    const OutlinerParaObject* pParaObj = rTextObj.GetOutlinerParaObject();
    DBG_ASSERT( pParaObj, "XclTxo: no ParaObject" );
    if( pParaObj )
    {
        // alignment in graphic
//-/        SfxItemSet aItemSet( *rTextObj.GetItemPool(), SDRATTR_START, SDRATTR_END );
        const SfxItemSet& rItemSet = rTextObj.GetItemSet();
//-/        rTextObj.TakeAttributes( aItemSet, FALSE, FALSE );

        switch ( ((const SdrTextHorzAdjustItem&) (rItemSet.Get( SDRATTR_TEXT_HORZADJUST ))).GetValue() )
        {
            case SDRTEXTHORZADJUST_LEFT :
                nGrbit |= (1 << 1);
            break;
            case SDRTEXTHORZADJUST_CENTER :
                nGrbit |= (2 << 1);
            break;
            case SDRTEXTHORZADJUST_RIGHT :
                nGrbit |= (3 << 1);
            break;
            case SDRTEXTHORZADJUST_BLOCK :
                nGrbit |= (4 << 1);
            break;
        }

        switch ( ((const SdrTextVertAdjustItem&) (rItemSet.Get( SDRATTR_TEXT_VERTADJUST ))).GetValue() )
        {
            case SDRTEXTVERTADJUST_TOP :
                nGrbit |= (1 << 4);
            break;
            case SDRTEXTVERTADJUST_CENTER :
                nGrbit |= (2 << 4);
            break;
            case SDRTEXTVERTADJUST_BOTTOM :
                nGrbit |= (3 << 4);
            break;
            case SDRTEXTVERTADJUST_BLOCK :
                nGrbit |= (4 << 4);
            break;
        }
        // the text
        const EditTextObject& rEditObj = pParaObj->GetTextObject();
        USHORT nPara = rEditObj.GetParagraphCount();
        for( USHORT j = 0; j < nPara; j++ )
        {
            aStr += rEditObj.GetText( j );
            if ( j < nPara-1 )
                aStr += ( sal_Char ) 0x0a;
        }
//2do: formatting runs
    }
    aText.Assign( aStr );
}


XclTxo::~XclTxo()
{
}


void XclTxo::SaveCont( XclExpStream& rStrm )
{
    UINT16 nTextLen = aText.Len();
    UINT16 nFormatLen = 0;
    if ( nTextLen && !nFormatLen )
        nFormatLen = 8 * 2; // length of CONTINUE record, not count of formats
    rStrm << nGrbit << nRot << UINT32(0) << UINT16(0)
        << nTextLen << nFormatLen << UINT32(0);
}


void XclTxo::Save( XclExpStream& rStrm )
{
    ExcRecord::Save( rStrm );

    UINT16 nTextLen = aText.Len();
    UINT16 nFormatLen = 0;
    if ( nTextLen && !nFormatLen )
        nFormatLen = 8 * 2; // length of CONTINUE record, not count of formats

    // CONTINUE records are only written if there is some text
    if ( nTextLen )
    {
        // CONTINUE text
        rStrm.StartRecord( EXC_ID_CONT, aText.GetBufferSize() + 1 );
        aText.WriteFlagField( rStrm );
        aText.WriteBuffer( rStrm );
        rStrm.EndRecord();

        // CONTINUE formatting runs
        rStrm.StartRecord( EXC_ID_CONT, nFormatLen );
        // write at least two dummy TXORUNs
        rStrm
            << UINT16(0)    // first character
            << UINT16(0)    // normal font
            << UINT32(0)    // Reserved
            << nTextLen
            << UINT16(0)    // normal font
            << UINT32(0)    // Reserved
            ;
        rStrm.EndRecord();
    }
}


UINT16 XclTxo::GetNum() const
{
    return 0x01B6;
}


ULONG XclTxo::GetLen() const
{
    return 18;
}


// --- class XclObjOle -------------------------------------------

XclObjOle::XclObjOle( RootData& rRoot, const SdrObject& rObj )
            :
            XclObj( otPicture, rRoot ),
            rOleObj( rObj ),
            pRootStorage( rRoot.pRootStorage )
{
}


XclObjOle::~XclObjOle()
{
}


void XclObjOle::SaveCont( XclExpStream& rStrm )
{
    // ftCmo subrecord
    XclObj::SaveCont( rStrm );

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
            rStrm << UINT16(ftCf) << UINT16(2) << UINT16(0x0002);

            // ftPioGrbit subrecord, undocumented as usual
            rStrm << UINT16(ftPioGrbit) << UINT16(2) << UINT16(0x0001);

            const UINT8 pData[] = {
                0x05, 0x00,
//              0xac, 0x10, 0xa4, 0x00,     // Xcl changes values on each object
                0x00, 0x00, 0x00, 0x00,     // zeroed
                0x02,                       // GPF if zeroed
//              0x78, 0xa9, 0x86, 0x00,     // Xcl changes values on each object
                0x00, 0x00, 0x00, 0x00,     // zeroed
                0x03
            };
            XclExpUniString aName( xOleStg->GetUserName() );
            UINT16 nPadLen = (UINT16)(aName.GetSize() & 0x01);
            UINT16 nFmlaLen = sizeof(pData) + aName.GetSize() + nPadLen;
            UINT16 nSubRecLen = nFmlaLen + 6;

            // ftPictFmla subrecord, undocumented as usual
            rStrm   << UINT16(ftPictFmla) << nSubRecLen
                    << nFmlaLen;
            rStrm.Write( pData, sizeof(pData) );
            rStrm   << aName;
            if( nPadLen )
                rStrm << UINT8(0);      // pad byte
            rStrm   << nPictureId;
        }
    }

    // ftEnd subrecord
    rStrm << UINT16(0) << UINT16(0);
}


void XclObjOle::Save( XclExpStream& rStrm )
{
    // content of this record
    XclObj::Save( rStrm );
    // second MSODRAWING record and TXO and CONTINUE records
    // we shouldn't have any here, so just in case ...
    XclObj::SaveTextRecs( rStrm );
}


ULONG XclObjOle::GetLen() const
{
    return 0;       // calculated by XclExpStream
}


// ----------------------------------------------------------------------------

#if EXC_INCL_EXP_OCX

XclExpObjControl::XclExpObjControl(
        const XclRoot& rRoot,
        const Reference< XShape >& rxShape,
        const String& rClassName,
        sal_uInt32 nStrmStart, sal_uInt32 nStrmSize ) :
    XclObj( otPicture, *rRoot.mpRD, true ),
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
        ::rtl::OUString aCtrlName;
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
        XclEscherClientAnchor( *rRoot.mpRD, *pSdrObj ).WriteData( rEscherEx );
    rEscherEx.AddAtom( 0, ESCHER_ClientData );                       // OBJ record
    rEscherEx.CloseContainer();  // ESCHER_SpContainer

    pMsodrawing->UpdateStopPos();
}

void XclExpObjControl::SaveCont( XclExpStream& rStrm )
{
    // ftCmo
    XclObj::SaveCont( rStrm );
    // ftCf (clipboard format)
    rStrm << sal_uInt16( ftCf ) << sal_uInt16( 2 ) << sal_uInt16( 2 );
    // ftPioGrbit
    rStrm << sal_uInt16( ftPioGrbit ) << sal_uInt16( 2 ) << sal_uInt16( 0x0031 );

    // ftPictFmla
    XclExpString aClass( maClassName );
    sal_uInt16 nClassNameSize = static_cast< sal_uInt16 >( aClass.GetSize() );
    sal_uInt16 nClassNamePad = nClassNameSize & 1;
    sal_uInt16 nFirstPartSize = 12 + nClassNameSize + nClassNamePad;
    sal_uInt16 nPictFmlaSize = nFirstPartSize + 18;

    rStrm   << sal_uInt16( ftPictFmla ) << sal_uInt16( nPictFmlaSize )
            << sal_uInt16( nFirstPartSize )             // size of first part
            << sal_uInt16( 5 )                          // formula size
            << sal_uInt32( 0 )                          // unknown ID
            << sal_uInt8( 0x02 ) << sal_uInt32( 0 )     // tTbl token with unknown ID
            << sal_uInt8( 3 )                           // pad to word
            << aClass;                                  // "Forms.***.1"
    rStrm.WriteZeroBytes( nClassNamePad );              // pad to word
    rStrm   << mnStrmStart                              // start in 'Ctls' stream
            << mnStrmSize                               // size in 'Ctls' stream
            << sal_uInt32( 0 ) << sal_uInt32( 0 );      // unknown

    // ftEnd
    rStrm << sal_uInt16( ftEnd ) << sal_uInt16( 0 );
}

sal_uInt32 XclExpObjControl::GetLen() const
{
    return 0;       // calculated by XclExpStream
}

#endif

// --- class XclObjAny -------------------------------------------

XclObjAny::XclObjAny( RootData& rRoot )
            :
            XclObj( otUnknown, rRoot )
{
}


XclObjAny::~XclObjAny()
{
}


void XclObjAny::SaveCont( XclExpStream& rStrm )
{
    // ftCmo subrecord
    XclObj::SaveCont( rStrm );
    switch ( eObjType )
    {
        case otGroup :      // ftGmo subrecord
            rStrm << UINT16(ftGmo) << UINT16(2) << UINT16(0);
        break;
    }
    // ftEnd subrecord
    rStrm << UINT16(0) << UINT16(0);
}


void XclObjAny::Save( XclExpStream& rStrm )
{
    // content of this record
    XclObj::Save( rStrm );
    // second MSODRAWING record and TXO and CONTINUE records
    XclObj::SaveTextRecs( rStrm );
}


ULONG XclObjAny::GetLen() const
{   // length of all subrecords including IDs and LENs
    ULONG nLen = XclObj::GetLen() + 4;
    switch ( eObjType )
    {
        case otGroup :      nLen += 6;  break;
    }
    return nLen;
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
    if( rAddText.Len() )
    {
        if( aNoteText.Len() )
            aNoteText.Append( EXC_NEWLINE ).Append( EXC_NEWLINE );
        aNoteText.Append( rAddText );
    }

    // create the Escher object
    mnObjId = rRoot.mpRD->pObjRecs->Add( new XclObjComment( *rRoot.mpRD, maPos, aNoteText, mbVisible ) );

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
    SetXFId( rRoot.GetXFBuffer().Insert( pAttr, pString->IsWrapped() ) );
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
        XclExpPalette::GetColorIdFromIndex( EXC_XF_AUTOCOLOR );

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


// --- class XclCondFormat -------------------------------------------

XclCondFormat::XclCondFormat( const ScConditionalFormat& _rCF, ScRangeList* _pRL, RootData& rER )
                :
                rCF( _rCF )
{
    pRL = _pRL;
    nTabNum = rER.pER->GetScTab();
    nComplLen = 0;

    USHORT                      n;
    USHORT                      nNum = _rCF.Count();
    const ScCondFormatEntry*    pCFE;

    for( n = 0 ; n < nNum ; n++ )
    {
        pCFE = _rCF.GetEntry( n );
        if( pCFE )
            Append( new XclCf( *rER.pER, *pCFE ) );
    }
}


XclCondFormat::~XclCondFormat()
{
    delete pRL;
}


void XclCondFormat::WriteCondfmt( XclExpStream& rStrm )
{
    DBG_ASSERT( Count() < 65536, "XclCondFormat::SaveCont - to much CFs!" );

    rStrm.StartRecord( 0x01B0, 0 );     // real size calculated by XclExpStream

    rStrm << (UINT16) Count() << (UINT16) 0x0001;

    ULONG               nMinMaxPos = rStrm.GetStreamPos();
    UINT16              nRowFirst = 0xFFFF;
    UINT16              nRowLast = 0;
    UINT16              nColFirst = 0xFFFF;
    UINT16              nColLast = 0;
    UINT16              nRefCnt = 0;

    rStrm << nRowFirst << nRowLast << nColFirst << nColLast << nRefCnt;     // dummies

    const ScRange*      pAct = pRL->First();

    while( pAct )
    {
        const ScAddress&    rStart = pAct->aStart;
        const ScAddress&    rEnd = pAct->aEnd;

        if( rStart.Tab() == nTabNum )
        {
            nRefCnt++;

            UINT16      nRF = rStart.Row();
            UINT16      nCF = rStart.Col();
            UINT16      nRL = rEnd.Row();
            UINT16      nCL = rEnd.Col();

            nRowFirst = Min( nRF, nRowFirst );
            nRowLast = Max( nRL, nRowLast );
            nColFirst = Min( nCF, nColFirst );
            nColLast = Max( nCL, nColLast );

            rStrm << nRF << nRL << nCF << nCL;
        }
        pAct = pRL->Next();
    }

    rStrm.EndRecord();

    // write min / max and num of refs
    rStrm.SetStreamPos( nMinMaxPos );
    rStrm << nRowFirst << nRowLast << nColFirst << nColLast << nRefCnt;
}


void XclCondFormat::Save( XclExpStream& rStrm )
{
    // write CONDFMT
    WriteCondfmt( rStrm );

    // write list of CF records
    for( XclCf* pCf = First(); pCf; pCf = Next() )
        pCf->Save( rStrm );
}



// --- class XclCf ---------------------------------------------------

XclCf::XclCf( const XclExpRoot& rRoot, const ScCondFormatEntry& r ) :
    XclExpRoot( rRoot )
{
    nType = 0x01;   // compare
    nFormatLen = 0;
    bHasStyle = bHasFont = bHasLine = bHasPattern = FALSE;

    BOOL bSingForm = TRUE;
    switch( r.GetOperation() )
    {
        case SC_COND_EQUAL:         nOp = 0x03;                                     break;
        case SC_COND_LESS:          nOp = 0x06;                                     break;
        case SC_COND_GREATER:       nOp = 0x05;                                     break;
        case SC_COND_EQLESS:        nOp = 0x08;                                     break;
        case SC_COND_EQGREATER:     nOp = 0x07;                                     break;
        case SC_COND_NOTEQUAL:      nOp = 0x04;                                     break;
        case SC_COND_BETWEEN:       nOp = 0x01; bSingForm = FALSE;                  break;
        case SC_COND_NOTBETWEEN:    nOp = 0x02; bSingForm = FALSE;                  break;
        case SC_COND_DIRECT:        nOp = 0x00;                     nType = 0x02;   break;
        default:                    nOp = 0x00;                     nType = 0x00;
    }

    // creating formats
    const String&       rStyleName = r.GetStyle();
    SfxStyleSheetBase*  pStyle =
        GetDoc().GetStyleSheetPool()->Find( rStyleName, SFX_STYLE_FAMILY_PARA );
    bHasStyle = pStyle ? TRUE : FALSE;
    if( bHasStyle )
    {
        const SfxItemSet&   rSet = pStyle->GetItemSet();
        BOOL bHasItalic     = rSet.GetItemState( ATTR_FONT_POSTURE, TRUE ) == SFX_ITEM_SET;
        BOOL bHasUnderline  = rSet.GetItemState( ATTR_FONT_UNDERLINE, TRUE ) == SFX_ITEM_SET;
        BOOL bHasStrikeOut  = rSet.GetItemState( ATTR_FONT_CROSSEDOUT, TRUE ) == SFX_ITEM_SET;
        BOOL bHasWeight     = rSet.GetItemState( ATTR_FONT_WEIGHT, TRUE ) == SFX_ITEM_SET;
        bHasColor       = rSet.GetItemState( ATTR_FONT_COLOR, TRUE ) == SFX_ITEM_SET;

        bHasFont = bHasItalic || bHasUnderline || bHasStrikeOut || bHasWeight || bHasColor;
        bHasLine = rSet.GetItemState( ATTR_BORDER, TRUE ) == SFX_ITEM_SET;
        bHasPattern = rSet.GetItemState( ATTR_BACKGROUND, TRUE ) == SFX_ITEM_SET;

        // constants for formatsstart
        nStart = 0x00FFFFFF;    // nothing included, all DC
        nFormatLen = 6;

        if( bHasFont )
        {
            nStart |= 0x04000000;
            nStart &= 0xFF3FFFFF;
            nFormatLen += 118;
        }
        if( bHasLine )
        {
            nStart |= 0x10000000;
            nStart &= 0xFFFFC3FF;
            nFormatLen += 8;
        }
        if( bHasPattern )
        {
            nStart |= 0x20000000;
            nStart &= 0xFF3BFFFF;
            nFormatLen += 4;
        }

        // font data
        if( bHasFont )
        {
            Font aFont;
            ScPatternAttr::GetFont( aFont, rSet, SC_AUTOCOL_RAW );

            BOOL bItalic    = ( bHasItalic && aFont.GetItalic() != ITALIC_NONE );
            BOOL bStrikeOut = ( bHasStrikeOut && aFont.GetStrikeout() != STRIKEOUT_NONE );

            nFontData1 = bItalic ? 0x00000002 : 0;
            nFontData1 |= bStrikeOut ? 0x00000080 : 0;

            if( bHasWeight )
                nFontData2 = XclExpFont::GetXclWeight( aFont.GetWeight() );
            else
                nFontData2 = bHasItalic ? 0x00000400 : 0;
            nFontData3 = bHasUnderline ? XclExpFont::GetXclUnderline( aFont.GetUnderline() ) : 0;

            if( bHasColor )
                nIcvTextSer = GetPalette().InsertColor( aFont.GetColor(), xlColorCellText );

            nFontData4 = bHasStrikeOut ? 0x00000018 : 0x00000098;
            nFontData4 |= (bHasWeight || bHasItalic) ? 0 : 0x00000002;
            nFontData5 = bHasUnderline ? 0 : 0x00000001;
            nFontData6 = (bHasWeight || bHasItalic) ? 0 : 0x01;
        }

        // border data
        if( bHasLine )
            maBorder.FillFromItemSet( rSet, GetPalette(), GetBiff() );

        // background / foreground data
        if( bHasPattern )
            maArea.FillFromItemSet( rSet, GetPalette(), GetBiff() );
    }

    ScTokenArray*   pScTokArry1 = r.CreateTokenArry( 0 );
    EC_Codetype     eDummy;
    ExcUPN*         pForm1      = new ExcUPN( mpRD, *pScTokArry1, eDummy, NULL, TRUE );
    nFormLen1 = pForm1->GetLen();

    ScTokenArray*   pScTokArry2 = NULL;
    ExcUPN*         pForm2 = NULL;
    if( bSingForm )
        nFormLen2 = 0;
    else
    {
        pScTokArry2 = r.CreateTokenArry( 1 );
        pForm2 = new ExcUPN( mpRD, *pScTokArry2, eDummy, NULL, TRUE );
        nFormLen2 = pForm2->GetLen();
    }

    DBG_ASSERT( nFormLen1 + nFormLen2 < 4096, "*XclCf::XclCf(): possible overflow of var data" );

    nVarLen = nFormLen1 + nFormLen2;
    pVarData = new sal_Char[ nVarLen ];

    if( nFormLen1 )
        memcpy( pVarData, pForm1->GetData(), nFormLen1 );
    delete pForm1;
    delete pScTokArry1;

    if( pForm2 )
    {
        memcpy( pVarData + nFormLen1, pForm2->GetData(), nFormLen2 );
        delete pForm2;
        delete pScTokArry2;
    }
}


XclCf::~XclCf()
{
    if( pVarData )
        delete pVarData;
}


void XclCf::SaveCont( XclExpStream& rStrm )
{
    const UINT8 pPre[ 68 ] =
    {   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xDF, 0x3F, 0x8A, 0x1D, 0x3C, 0xFC, 0xFD, 0x7E, 0xDF, 0x3F,
        0x4E, 0x4A, 0x54, 0x30, 0x00, 0x00, 0x00, 0x00, 0x65, 0x10,
        0x00, 0x30, 0xF0, 0x59, 0x54, 0x30, 0xEC, 0x04, 0xC8, 0x00,
        0x4C, 0x00, 0x00, 0x00, 0x89, 0x10, 0x00, 0x30, 0xEC, 0x04,
        0xC8, 0x00, 0xF0, 0x59, 0x54, 0x30, 0x4C, 0x00, 0x00, 0x00,
        0x4E, 0x4A, 0x54, 0x30, 0xFF, 0xFF, 0xFF, 0xFF
    };
    const UINT8 pPost[ 17 ] =
    {   0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0xFF, 0xFF, 0xFF, 0x7F, 0x01, 0x00
    };

    rStrm << nType << nOp << nFormLen1 << nFormLen2;
    if( bHasStyle )
    {
        rStrm << nStart << (UINT16)0x0002;
        if( bHasFont )
        {
            rStrm.Write( pPre, 68 );
            rStrm << nFontData1 << nFontData2 << nFontData3;
            if( bHasColor )
                rStrm << (UINT32) GetPalette().GetColorIndex( nIcvTextSer );
            else
                rStrm << (UINT32)0xFFFFFFFF;
            rStrm   << (UINT32)0x00000000 << nFontData4
                    << (UINT32)0x00000001 << nFontData5
                    << nFontData6;
            rStrm.Write( pPost, 17 );
        }
        if( bHasLine )
        {
            sal_uInt16 nLineStyle = 0;
            sal_uInt32 nLineColor = 0;
            maBorder.SetFinalColors( GetPalette() );
            maBorder.FillToCF8( nLineStyle, nLineColor );
            rStrm << nLineStyle << nLineColor << (UINT16)0xBA00;
        }
        if( bHasPattern )
        {
            sal_uInt16 nPattern = 0, nColor = 0;
            maArea.SetFinalColors( GetPalette() );
            maArea.FillToCF8( nPattern, nColor );
            rStrm << nPattern << nColor;
        }
    }
    rStrm.Write( pVarData, nVarLen );
}


UINT16 XclCf::GetNum() const
{
    return 0x01B1;
}


ULONG XclCf::GetLen() const
{
    return 6 + nFormatLen + nVarLen;
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



