/*************************************************************************
 *
 *  $RCSfile: xiescher.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2003-08-07 15:29:28 $
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

// ============================================================================

#ifndef SC_XIESCHER_HXX
#include "xiescher.hxx"
#endif

#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif
#ifndef _SFX_OBJSH_HXX
#include <sfx2/objsh.hxx>
#endif
#ifndef _SFX_INTERNO_HXX
#include <sfx2/interno.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <svtools/moduleoptions.hxx>
#endif
#ifndef _OFF_APP_HXX
#include <offmgr/app.hxx>
#endif
#ifndef _OFA_FLTRCFG_HXX
#include <offmgr/fltrcfg.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
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

#ifndef _SCH_DLL_HXX
#include <sch/schdll.hxx>
#endif
#ifndef _SCHDLL0_HXX
#include <sch/schdll0.hxx>
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

#ifndef SC_XCLIMPCHARTS_HXX
#include "XclImpCharts.hxx"
#endif
#include "excform.hxx"

#include <stdio.h>

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::lang::XComponent;
using ::com::sun::star::beans::XPropertySet;


// Escher stream consumer =====================================================

XclImpStreamConsumer::XclImpStreamConsumer() :
    aStrm(),
    aHd(),
    pNode( NULL ),
    nBytesLeft( 0 )
{
}

XclImpStreamConsumer::~XclImpStreamConsumer()
{
    while( pNode )
        RemoveNode();
}

void XclImpStreamConsumer::UpdateNode( const DffRecordHeader& rHd )
{
    while( pNode && ((pNode->nPos + pNode->nSize) <= rHd.nFilePos) )
        RemoveNode();
    XclImpStreamNode* pTemp = pNode;
    pNode = new XclImpStreamNode;
    pNode->nPos = rHd.nFilePos;
    pNode->nSize = rHd.nRecLen + 8;
    pNode->pPrev = pTemp;
}

void XclImpStreamConsumer::RemoveNode()
{
    XclImpStreamNode* pTemp = pNode;
    pNode = pNode->pPrev;
    delete pTemp;
}

const DffRecordHeader* XclImpStreamConsumer::ConsumeRecord( XclImpStream& rSrcStrm )
{
    sal_uInt32 nEntry = aStrm.Tell();
    sal_uInt32 nSrcSize = rSrcStrm.GetRecSize();
    if( !nSrcSize )
        return NULL;

    rSrcStrm.Seek( RECORD_SEEK_TO_BEGIN );
    sal_Char* pBuf = new sal_Char[ nSrcSize ];
    rSrcStrm.Read( pBuf, nSrcSize );
    aStrm.Write( pBuf, nSrcSize );
    delete[] pBuf;

    sal_uInt32 nPos = aStrm.Tell();
    aStrm.Seek( nEntry );
    if( nBytesLeft )
    {
        if( nSrcSize < nBytesLeft )
        {
            aStrm.SeekRel( nSrcSize );
            nBytesLeft -= nSrcSize;
        }
        else
        {
            aStrm.SeekRel( nBytesLeft );
            nBytesLeft = 0;
        }
    }
    while( aStrm.Tell() < nPos )
    {
        aStrm >> aHd;
        if(aHd.nRecType == DFF_msofbtSolverContainer )
        {
            aStrm.Seek( nEntry );
            return &aHd;
        }
        else if( aHd.IsContainer() )
            UpdateNode( aHd );
        else if( (aStrm.Tell() + aHd.nRecLen) <= nPos )
            aStrm.SeekRel( aHd.nRecLen );
        else
        {
            nBytesLeft = aStrm.Tell() + aHd.nRecLen - nPos;
            aStrm.Seek( nPos );
        }
    }
    aStrm.Seek( nPos );

    return nBytesLeft ? NULL : &aHd;
}

bool XclImpStreamConsumer::AppendData( sal_Char* pBuf, sal_uInt32 nLen )
{
    if ( (aHd.nRecType != 0) && !aHd.IsContainer() && (nBytesLeft == 0) )
    {
        while( pNode && ((pNode->nPos + pNode->nSize) <= aHd.nFilePos) )
            RemoveNode();
        XclImpStreamNode* pTemp = pNode;
        while( pTemp )
        {
            pTemp->nSize += nLen;               // updating container sizes
            aStrm.Seek( pTemp->nPos + 4 );
            aStrm << pTemp->nSize - 8;
            pTemp = pTemp->pPrev;
        }
        aHd.nRecLen += nLen;
        aStrm.Seek( aHd.nFilePos + 4 );        // updating atom size
        aStrm << aHd.nRecLen;
        aStrm.Seek( STREAM_SEEK_TO_END );
        aStrm.Write( pBuf, nLen );
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
    mnScTab( rRoot.GetScTab() ),
    mnObjId( EXC_OBJ_INVALID_ID ),
    mbSkip( false )
{
}

XclImpEscherObj::XclImpEscherObj(
        const XclImpRoot& rRoot,
        sal_uInt32 nStrmBegin, sal_uInt32 nStrmEnd ) :
    XclImpRoot( rRoot ),
    mnStrmBegin( nStrmBegin ),
    mnStrmEnd( nStrmEnd ),
    mnScTab( rRoot.GetScTab() ),
    mnObjId( EXC_OBJ_INVALID_ID ),
    mbSkip( false )
{
}

XclImpEscherObj::XclImpEscherObj( XclImpEscherObj& rSrcObj ) :
    XclImpRoot( rSrcObj.GetRoot() ),
    maAnchorRect( rSrcObj.maAnchorRect ),
    mpSdrObj( rSrcObj.mpSdrObj ),          // rSrc.mpSdrObj releases the object
    mnStrmBegin( rSrcObj.mnStrmBegin ),
    mnStrmEnd( rSrcObj.mnStrmEnd ),
    mnScTab( rSrcObj.mnScTab ),
    mnObjId( rSrcObj.mnObjId ),
    mbSkip( rSrcObj.mbSkip )
{
}

XclImpEscherObj::~XclImpEscherObj()
{
}

void XclImpEscherObj::SetSdrObj( SdrObject* pSdrObj )
{
    mpSdrObj.reset( pSdrObj );
}

void XclImpEscherObj::InitProgress( ScfProgressBar& rProgress )
{
}

void XclImpEscherObj::Apply( ScfProgressBar& rProgress )
{
    if( IsValid() )
        if( SdrPage* pPage = GetDoc().GetDrawLayer()->GetPage( mnScTab ) )
            pPage->InsertObject( mpSdrObj.release() );
}


// ----------------------------------------------------------------------------

TYPEINIT1( XclImpEscherDrawing, XclImpEscherObj );

XclImpEscherDrawing::XclImpEscherDrawing( XclImpEscherObj& rSrcObj ) :
    XclImpEscherObj( rSrcObj )
{
}


// ----------------------------------------------------------------------------

TYPEINIT1( XclImpEscherTxo, XclImpEscherDrawing );

XclImpEscherTxo::XclImpEscherTxo( XclImpEscherObj& rSrcObj ) :
    XclImpEscherDrawing( rSrcObj ),
    meHorAlign( xlTxoHAlign_Default ),
    meVerAlign( xlTxoVAlign_Default )
{
}

void XclImpEscherTxo::SetAlignment( sal_uInt16 nAlign )
{
    ::extract_value( meHorAlign, nAlign, 1, 3 );
    ::extract_value( meVerAlign, nAlign, 4, 3 );
}

void XclImpEscherTxo::ApplyTextOnSdrObj( SdrObject& rSdrObj )
{
    if( SdrTextObj* pTextObj = PTR_CAST( SdrTextObj, &rSdrObj ) )
    {
        if( mpString.get() )
        {
            if( mpString->IsRich() )
            {
                // rich text
                ::std::auto_ptr< EditTextObject > pEditObj(
                    XclImpStringHelper::CreateTextObject( GetRoot(), *mpString ) );
                OutlinerParaObject* pOPO = new OutlinerParaObject( *pEditObj );
                pOPO->SetOutlinerMode( OUTLINERMODE_TEXTOBJECT );
                pTextObj->NbcSetOutlinerParaObject( pOPO );
            }
            else
            {
                // plain text
                pTextObj->SetText( mpString->GetText() );
            }
        }
    }
}

void XclImpEscherTxo::SetSdrObj( SdrObject* pNewSdrObj )
{
    XclImpEscherDrawing::SetSdrObj( pNewSdrObj );
}


// ----------------------------------------------------------------------------

TYPEINIT1( XclImpEscherNote, XclImpEscherTxo );

XclImpEscherNote::XclImpEscherNote( XclImpEscherObj& rSrcObj ) :
    XclImpEscherTxo( rSrcObj )
{
}

void XclImpEscherNote::Apply( ScfProgressBar& rProgress )
{
    // do not insert the note object, done via ScDetectiveFunc
}


// ----------------------------------------------------------------------------

XclImpCtrlLinkHelper::XclImpCtrlLinkHelper( ScDocument& rDoc ) :
    mrDoc( rDoc )
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
        ::std::auto_ptr< ScRange > pRange( aRangeList.Remove( static_cast< sal_uInt32 >( 0 ) ) );
        if( pRange.get() )
            mpCellLink.reset( new ScAddress( pRange->aStart ) );
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
        mpSrcRange.reset( aRangeList.Remove( static_cast< sal_uInt32 >( 0 ) ) );
}

void XclImpCtrlLinkHelper::InsertStringList( Reference< XPropertySet >& rxPropSet ) const
{
    if( mpSrcRange.get() )
    {
        Sequence< OUString > aStringList;
        // Excel uses only first column
        sal_uInt16 nScCol = mpSrcRange->aStart.Col();
        sal_uInt16 nScRow1 = mpSrcRange->aStart.Row();
        sal_uInt16 nScRow2 = mpSrcRange->aEnd.Row();
        sal_uInt16 nScTab = mpSrcRange->aStart.Tab();
        String aCellText;
        aStringList.realloc( nScRow2 - nScRow1 + 1 );
        sal_Int32 nSeqIndex = 0;

        for( sal_uInt16 nScRow = nScRow1; nScRow <= nScRow2; ++nScRow, ++nSeqIndex )
        {
            mrDoc.GetString( nScCol, nScRow, nScTab, aCellText );
            aStringList[ nSeqIndex ] = aCellText;
        }
        ::setPropValue( rxPropSet, CREATE_OUSTRING( "StringItemList" ), aStringList );
    }
}

void XclImpCtrlLinkHelper::InsertLinkTag( Reference< XPropertySet >& rxPropSet ) const
{
    ::setPropString( rxPropSet, CREATE_OUSTRING( "Tag" ),
        XclTools::GetCtrlLinkTag( mrDoc, mpCellLink.get(), mpSrcRange.get() ) );
}

String XclImpCtrlLinkHelper::GetString( sal_Int16 nPos ) const
{
    String aString;
    if( mpSrcRange.get() && (nPos >= 0) )
        mrDoc.GetString( mpSrcRange->aStart.Col(), mpSrcRange->aStart.Row() + nPos, mpSrcRange->aStart.Tab(), aString );
    return aString;
}


// ----------------------------------------------------------------------------

TYPEINIT1( XclImpEscherTbxCtrl, XclImpEscherTxo );

XclImpEscherTbxCtrl::XclImpEscherTbxCtrl( XclImpEscherObj& rSrcObj, sal_uInt16 nCtrlType ) :
    XclImpEscherTxo( rSrcObj ),
    maLinkHelper( GetDoc() ),
    mnProgressSeg( SCF_INV_SEGMENT ),
    mnCtrlType( nCtrlType ),
    mnState( EXC_OBJ_CBLS_STATE_UNCHECK ),
    mnSelEntry( 0 ),
    mnSelType( EXC_OBJ_LBS_SEL_SIMPLE ),
    mnLineCount( 0 ),
    mb3DStyle( true )
{
}

void XclImpEscherTbxCtrl::ReadCbls( XclImpStream& rStrm )
{
    sal_uInt16 nStyle;
    rStrm >> mnState;
    rStrm.Ignore( 8 );
    rStrm >> nStyle;
    mnState &= EXC_OBJ_CBLS_STATEMASK;
    mb3DStyle = ::get_flag( nStyle, EXC_OBJ_CBLS_3D );
}

void XclImpEscherTbxCtrl::ReadCblsFmla( XclImpStream& rStrm )
{
    maLinkHelper.ReadCellLinkFormula( rStrm );
}

void XclImpEscherTbxCtrl::ReadLbsData( XclImpStream& rStrm )
{
    sal_uInt16 nSubSize;
    rStrm >> nSubSize;
    if( nSubSize )
    {
        // read the address of the data source range
        rStrm.PushPosition();
        maLinkHelper.ReadSrcRangeFormula( rStrm );
        rStrm.PopPosition();
        rStrm.Ignore( nSubSize );
    }

    // other list box settings
    sal_Int16 nEntryCount;
    sal_uInt16 nStyle;
    rStrm >> nEntryCount >> mnSelEntry >> nStyle;
    rStrm.Ignore( 2 );  // linked edit field

    mnSelType = nStyle & EXC_OBJ_LBS_SELMASK;
    mb3DStyle = ::get_flag( nStyle, EXC_OBJ_LBS_3D );

    switch( mnCtrlType )
    {
        case EXC_OBJ_CMO_LISTBOX:
            if( mnSelType != EXC_OBJ_LBS_SEL_SIMPLE )
                for( sal_Int16 nEntry = 0; nEntry < nEntryCount; ++nEntry )
                    if( rStrm.ReaduInt8() != 0 )
                        maMultiSel.push_back( nEntry );
        break;
        case EXC_OBJ_CMO_COMBOBOX:
            rStrm.Ignore( 2 );
            rStrm >> mnLineCount;
        break;
    }

}

OUString XclImpEscherTbxCtrl::GetServiceName() const
{
#define LCL_CREATE_NAME( name ) CREATE_OUSTRING( "com.sun.star.form.component." name )
    switch( mnCtrlType )
    {
        case EXC_OBJ_CMO_BUTTON:        return LCL_CREATE_NAME( "CommandButton" );
        case EXC_OBJ_CMO_CHECKBOX:      return LCL_CREATE_NAME( "CheckBox" );
        case EXC_OBJ_CMO_OPTIONBUTTON:  return LCL_CREATE_NAME( "RadioButton" );
        case EXC_OBJ_CMO_LABEL:         return LCL_CREATE_NAME( "FixedText" );
        case EXC_OBJ_CMO_LISTBOX:       return LCL_CREATE_NAME( "ListBox" );
        case EXC_OBJ_CMO_GROUPBOX:      return LCL_CREATE_NAME( "GroupBox" );
        case EXC_OBJ_CMO_COMBOBOX:      return LCL_CREATE_NAME( "ComboBox" );
    }
    DBG_ERRORFILE( "XclImpEscherTbxCtrl::GetServiceName - unknown control type" );
    return OUString();
#undef LCL_CREATE_NAME
}

void XclImpEscherTbxCtrl::SetProperties( Reference< XPropertySet >& rxPropSet ) const
{
    // the control name
    OUString aName;
    switch( mnCtrlType )
    {
        case EXC_OBJ_CMO_BUTTON:        aName = CREATE_OUSTRING( "CommandButton" ); break;
        case EXC_OBJ_CMO_CHECKBOX:      aName = CREATE_OUSTRING( "CheckBox" );      break;
        case EXC_OBJ_CMO_OPTIONBUTTON:  aName = CREATE_OUSTRING( "OptionButton" );  break;
        case EXC_OBJ_CMO_LABEL:         aName = CREATE_OUSTRING( "Label" );         break;
        case EXC_OBJ_CMO_LISTBOX:       aName = CREATE_OUSTRING( "ListBox" );       break;
        case EXC_OBJ_CMO_GROUPBOX:      aName = CREATE_OUSTRING( "GroupBox" );      break;
        case EXC_OBJ_CMO_COMBOBOX:      aName = CREATE_OUSTRING( "ComboBox" );      break;
    }
    if( aName.getLength() )
        ::setPropValue( rxPropSet, CREATE_OUSTRING( "Name" ), aName );

    // 3D border style
    if( (mnCtrlType == EXC_OBJ_CMO_LISTBOX) || (mnCtrlType == EXC_OBJ_CMO_COMBOBOX) )
        ::setPropValue( rxPropSet, CREATE_OUSTRING( "Border" ), static_cast< sal_Int16 >( mb3DStyle ? 2 : 1 ) );

    // check box/radio button state
    if( (mnCtrlType == EXC_OBJ_CMO_CHECKBOX) || (mnCtrlType == EXC_OBJ_CMO_OPTIONBUTTON) )
    {
        sal_Int16 nApiState = 0;
        switch( mnState )
        {
            case EXC_OBJ_CBLS_STATE_UNCHECK:    nApiState = 0;  break;
            case EXC_OBJ_CBLS_STATE_CHECK:      nApiState = 1;  break;
            case EXC_OBJ_CBLS_STATE_TRI:        nApiState = (mnCtrlType == EXC_OBJ_CMO_CHECKBOX) ? 2 : 1;   break;
        }
        ::setPropBool( rxPropSet, CREATE_OUSTRING( "TriState" ), nApiState == 2 );
        ::setPropValue( rxPropSet, CREATE_OUSTRING( "DefaultState" ), nApiState );
    }

    if( const XclImpString* pString = GetString() )
    {
        // the visible label (caption)
        ::setPropString( rxPropSet, CREATE_OUSTRING( "Label" ), pString->GetText() );

        // font properties
        if( !pString->GetFormats().empty() )
        {
            const XclFormatRun& rFormatRun = pString->GetFormats().front();
            if( const XclImpFont* pFont = GetFontBuffer().GetFont( rFormatRun.mnFontIx ) )
            {
                const XclFontData& rFontData = pFont->GetFontData();
                ::setPropString( rxPropSet, CREATE_OUSTRING( "FontName" ), rFontData.maName );
                sal_Int16 nHeight = static_cast< sal_Int16 >( rFontData.GetApiHeight() + 0.5 );
                ::setPropValue( rxPropSet, CREATE_OUSTRING( "FontHeight" ), nHeight );
                ::setPropValue( rxPropSet, CREATE_OUSTRING( "FontFamily" ), rFontData.GetApiFamily() );
                ::setPropValue( rxPropSet, CREATE_OUSTRING( "FontCharset" ), rFontData.GetApiCharSet() );
                ::setPropValue( rxPropSet, CREATE_OUSTRING( "FontWeight" ), rFontData.GetApiWeight() );
                ::setPropValue( rxPropSet, CREATE_OUSTRING( "FontSlant" ), rFontData.GetApiPosture() );
                ::setPropValue( rxPropSet, CREATE_OUSTRING( "FontUnderline" ), rFontData.GetApiUnderline() );
                ::setPropValue( rxPropSet, CREATE_OUSTRING( "FontStrikeout" ), rFontData.GetApiStrikeout() );
                sal_Int32 nColor = static_cast< sal_Int32 >( GetPalette().GetColorData( rFontData.mnColor ) );
                ::setPropValue( rxPropSet, CREATE_OUSTRING( "TextColor" ), nColor );
            }
        }
    }

    // listbox/combobox contents
    if( (mnCtrlType == EXC_OBJ_CMO_LISTBOX) || (mnCtrlType == EXC_OBJ_CMO_COMBOBOX) )
    {
        // string list
        maLinkHelper.InsertStringList( rxPropSet );

        switch( mnCtrlType )
        {
            case EXC_OBJ_CMO_LISTBOX:
            {
                // selection type
                bool bMultiSel = (mnSelType != EXC_OBJ_LBS_SEL_SIMPLE);
                ::setPropBool( rxPropSet, CREATE_OUSTRING( "MultiSelection" ), bMultiSel );

                // selection
                Sequence< sal_Int16 > aSelection;
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
                if( aSelection.getLength() )
                    ::setPropValue( rxPropSet, CREATE_OUSTRING( "DefaultSelection" ), aSelection );
            }
            break;

            case EXC_OBJ_CMO_COMBOBOX:
                // dropdown button
                ::setPropBool( rxPropSet, CREATE_OUSTRING( "Dropdown" ), true );
                // dropdown line count
                ::setPropValue( rxPropSet, CREATE_OUSTRING( "LineCount" ), mnLineCount );
                // text content
                ::setPropString( rxPropSet, CREATE_OUSTRING( "DefaultText" ),
                    maLinkHelper.GetString( mnSelEntry - 1 ) );
            break;
        }
    }

    // link range address
    maLinkHelper.InsertLinkTag( rxPropSet );
}

void XclImpEscherTbxCtrl::InitProgress( ScfProgressBar& rProgress )
{
    mnProgressSeg = rProgress.AddSegment( 1 );
}

void XclImpEscherTbxCtrl::Apply( ScfProgressBar& rProgress )
{
    if( !mbSkip )
    {
        if( GetObjectManager().CreateSdrObj( *this ) )
        {
            // form controls: set bound rect explicitely
            mpSdrObj->NbcSetSnapRect( maAnchorRect );
            // insert the SdrObj into the draw page
            XclImpEscherObj::Apply( rProgress );
        }
        else
        {
            // no success to create a control -> create a text box
            XclImpEscherTxo::Apply( rProgress );
        }
    }
    rProgress.ActivateSegment( mnProgressSeg );
    rProgress.Progress();
}


// ----------------------------------------------------------------------------

TYPEINIT1( XclImpEscherOle, XclImpEscherObj );

XclImpEscherOle::XclImpEscherOle( XclImpEscherObj& rSrcObj ):
    XclImpEscherObj( rSrcObj ),
    maLinkHelper( GetDoc() ),
    mnCtrlStrmPos( 0 ),
    mnBlipId( 0 ),
    mnProgressSeg( SCF_INV_SEGMENT ),
    mbAsSymbol( false ),
    mbLinked( false ),
    mbControl( false )
{
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
        const XclImpSupbook* pSupbook = GetLinkManager().GetSupbook( nXti );
        const XclImpExtName* pExtName = pSupbook ? pSupbook->GetExtName( nExtName ) : NULL;
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
                    rStrm.AppendUniString( aUserName, n16 );
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

            // try to read additional link data
            if( rStrm.GetRecLeft() > 8 )
            {
                rStrm.Ignore( 8 );
                sal_uInt16 nDataSize;

                // cell link
                rStrm >> nDataSize;
                if( nDataSize )
                {
                    rStrm.PushPosition();
                    maLinkHelper.ReadCellLinkFormula( rStrm );
                    rStrm.PopPosition();
                    rStrm.Ignore( nDataSize );
                }
                // source data range
                rStrm >> nDataSize;
                if( nDataSize )
                {
                    rStrm.PushPosition();
                    maLinkHelper.ReadSrcRangeFormula( rStrm );
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

void XclImpEscherOle::SetProperties( Reference< XPropertySet >& rxPropSet ) const
{
    maLinkHelper.InsertStringList( rxPropSet );
    maLinkHelper.InsertLinkTag( rxPropSet );
}

void XclImpEscherOle::InitProgress( ScfProgressBar& rProgress )
{
    mnProgressSeg = rProgress.AddSegment( 1 );
}

void XclImpEscherOle::Apply( ScfProgressBar& rProgress )
{
    if( !mbSkip )
        GetObjectManager().CreateSdrObj( *this );

    if( IsValid() )
    {
        SfxObjectShell* pDocShell = GetDocShell();
        SdrOle2Obj* pOleSdrObj = PTR_CAST( SdrOle2Obj, mpSdrObj.get() );
        if( pOleSdrObj && pDocShell )
        {
            SvInfoObject* pInfoObj = pDocShell->InsertObject( pOleSdrObj->GetObjRef(), EMPTY_STRING );
            DBG_ASSERT( pInfoObj, "XclImpEscherOle::Apply - no InfoObject" );
            if( pInfoObj )
                // #95381# SetPersistName, not SetName
                pOleSdrObj->SetPersistName( pInfoObj->GetObjName() );
        }
        else if( mpSdrObj->ISA( SdrUnoObj ) )
        {
            // form controls: set bound rect explicitely
            mpSdrObj->NbcSetSnapRect( maAnchorRect );
        }
    }
    XclImpEscherObj::Apply( rProgress );

    rProgress.ActivateSegment( mnProgressSeg );
    rProgress.Progress();
}


// ----------------------------------------------------------------------------

TYPEINIT1( XclImpEscherChart, XclImpEscherObj );

XclImpEscherChart::XclImpEscherChart( XclImpEscherObj& rSrcObj ) :
    XclImpEscherObj( rSrcObj )
{
    mpChart.reset( new XclImpChart( *mpRD ) );
    mpChart->nBaseTab = GetTab();
}

void XclImpEscherChart::SetChartData( XclImpChart* pChart )
{
    mpChart.reset( pChart );
}

void XclImpEscherChart::InitProgress( ScfProgressBar& rProgress )
{
    if( mpChart.get() )
        mpChart->InitProgress( rProgress );
}

void XclImpEscherChart::Apply( ScfProgressBar& rProgress )
{
    if( !mpChart.get() || maAnchorRect.IsEmpty() )
        return;

    const XclImpChart_LinkedData* pLinkData = mpChart->GetSourceData();
    if( !pLinkData ) return;

    mpChart->CloseSourceData();

    SfxObjectShell* pDocShell = GetDocShell();
    if( !pDocShell ) return;

    SvStorageRef xStorage = new SvStorage( String() );
    SvInPlaceObjectRef xIPObj;

    // do not access uninstalled Chart module
    if( SvtModuleOptions().IsChart() )
        xIPObj = &static_cast< SvFactory* >( SvInPlaceObject::ClassFactory() )->CreateAndInit(
            *SCH_MOD()->pSchChartDocShellFactory, xStorage );

    if( xIPObj.Is() )
    {
        pDocShell->InsertObject( xIPObj, String() );

        String aName;
        if( SvInfoObject* pInfoObj = pDocShell->Find( xIPObj ) )
            aName = pInfoObj->GetObjName();
        else
            DBG_ERRORFILE( "XclImpEscherChart::Apply - Inplace object not found" );

        BOOL bEnabled = xIPObj->IsEnableSetModified();
        if( bEnabled )
            xIPObj->EnableSetModified( FALSE );

        Size aSize( xIPObj->GetVisArea().GetSize() );
        if( (aSize.Height() < 1) || (aSize.Width() < 1) )
        {
            aSize.Width() = aSize.Height() = 5000;
            aSize = Window::LogicToLogic( aSize, MapMode( MAP_100TH_MM ), MapMode( xIPObj->GetMapUnit() ) );
            xIPObj->SetVisAreaSize( aSize );
        }

        SdrOle2Obj* pSdrObj = new SdrOle2Obj( xIPObj, aName, maAnchorRect );
        pSdrObj->SetLayer( SC_LAYER_FRONT );
        GetDoc().GetDrawLayer()->GetPage( mpChart->nBaseTab )->InsertObject( pSdrObj );
        pSdrObj->NbcSetLogicRect( maAnchorRect );

        ScChartArray aChartObj( GetDocPtr(), pLinkData->GetRangeList(), aName );

        bool bSwap = pLinkData && pLinkData->GetDir();
        bool bColHdr = bSwap ? mpChart->bHasSeriesNames : mpChart->bHasCategoryNames;
        bool bRowHdr = bSwap ? mpChart->bHasCategoryNames : mpChart->bHasSeriesNames;
        aChartObj.SetHeaders( bColHdr, bRowHdr );

        SchMemChart* pMemChart = aChartObj.CreateMemChart();
        SchDLL::Update( xIPObj, pMemChart );
        delete pMemChart;

        SfxInPlaceObjectRef aSfxObj( xIPObj );
        if( aSfxObj.Is() )
        {
            if( SfxObjectShell* pObjSh = aSfxObj->GetObjectShell() )
            {
                Reference< XComponent > xComp = pObjSh->GetModel().get();
                if( xComp.is() )
                    mpChart->Apply( xComp, maAnchorRect, rProgress );
            }
        }

        if( bEnabled )
            xIPObj->EnableSetModified( TRUE );

        xIPObj->SetModified();
        xIPObj->DoSave();
        xIPObj->DoSaveCompleted();
        xIPObj->GetStorage()->Commit();
    }
}


// Escher object data =========================================================

XclImpEscherAnchor::XclImpEscherAnchor( sal_uInt16 nScTab )
{
    memset( this, 0x00, sizeof( XclImpEscherAnchor ) );
    mnScTab = nScTab;
}

SvStream& operator>>( SvStream& rStrm, XclImpEscherAnchor& rAnchor )
{
    return rStrm
        >> rAnchor.mnLCol >> rAnchor.mnLX
        >> rAnchor.mnTRow >> rAnchor.mnTY
        >> rAnchor.mnRCol >> rAnchor.mnRX
        >> rAnchor.mnBRow >> rAnchor.mnBY;
}


// ----------------------------------------------------------------------------

XclImpObjData::XclImpObjData( XclImpEscherObj* pEscherObj ) :
    maAnchor( pEscherObj ? pEscherObj->GetTab() : 0 ),
    mpEscherObj( pEscherObj )
{
}

void XclImpObjData::SetObj( XclImpEscherObj* pEscherObj )
{
    mpEscherObj.reset( pEscherObj );
}

bool XclImpObjData::ContainsStrmPos( sal_uInt32 nStrmPos ) const
{
    return mpEscherObj.get() && (mpEscherObj->GetStrmBegin() <= nStrmPos) && (nStrmPos < mpEscherObj->GetStrmEnd());
}

// ----------------------------------------------------------------------------

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

XclImpEscherObj* XclImpEscherObjList::GetObj( sal_uInt16 nScTab, sal_uInt16 nObjId ) const
{
    if( nObjId != EXC_OBJ_INVALID_ID )
    {
        // objects are ordered by sheet index
        sal_uInt32 nCacheSize = maObjCache.size();
        if( nScTab < nCacheSize )
        {
            sal_uInt32 nEnd = (nScTab + 1UL < nCacheSize) ? maObjCache[ nScTab + 1 ].mnListIndex : maObjDataList.Count();
            for( sal_uInt32 nIndex = maObjCache[ nScTab ].mnListIndex; nIndex < nEnd; ++nIndex )
                if( XclImpEscherObj* pEscherObj = maObjDataList.GetObject( nIndex )->GetObj() )
                    if( (pEscherObj->GetTab() == nScTab) && (pEscherObj->GetObjId() == nObjId) )
                        return pEscherObj;
        }
    }
    return NULL;
}

XclImpEscherObj* XclImpEscherObjList::GetLastObj() const
{
    if( XclImpObjData* pObjData = maObjDataList.Last() )
        return pObjData->GetObj();
    return NULL;
}

XclImpEscherObj* XclImpEscherObjList::GetObj( sal_uInt32 nStrmPos ) const
{
    XclImpObjData* pObjData = FindObjData( nStrmPos );
    return pObjData ? pObjData->GetObj() : NULL;
}

XclImpEscherAnchor* XclImpEscherObjList::GetAnchor( sal_uInt32 nStrmPos ) const
{
    XclImpObjData* pObjData = FindObjData( nStrmPos );
    return pObjData ? &pObjData->GetAnchor() : NULL;
}

void XclImpEscherObjList::InitProgress( ScfProgressBar& rProgress )
{
    DBG_ASSERT( !rProgress.IsStarted(), "XclImpEscherObjList::InitProgress - progress already started" );
    for( XclImpObjData* pData = maObjDataList.First(); pData; pData = maObjDataList.Next() )
        if( XclImpEscherObj* pEscherObj = pData->GetObj() )
            pEscherObj->InitProgress( rProgress );
}

void XclImpEscherObjList::Apply( ScfProgressBar& rProgress )
{
    for( XclImpObjData* pData = maObjDataList.First(); pData; pData = maObjDataList.Next() )
        if( XclImpEscherObj* pEscherObj = pData->GetObj() )
            pEscherObj->Apply( rProgress );
}

void XclImpEscherObjList::UpdateCache()
{
    if( const XclImpEscherObj* pEscherObj = GetLastObj() )
    {
        sal_uInt16 nScTab = pEscherObj->GetTab();
        sal_uInt32 nStrmPos = pEscherObj->GetStrmBegin();

        // #110252# ignore faked objects without corresponding Escher data (i.e. sheet-charts)
        if( nStrmPos != 0 )
        {
            if( nScTab >= maObjCache.size() )
                maObjCache.resize( nScTab + 1, XclCacheEntry( GetObjCount() - 1, nStrmPos ) );
            else if( maObjCache[ nScTab ].mnStrmPos > nStrmPos )
                maObjCache[ nScTab ].mnStrmPos = nStrmPos;
            DBG_ASSERT( !nScTab || (maObjCache[ nScTab - 1 ].mnStrmPos <= nStrmPos), "XclImpEscherObjList::UpdateCache - cache corrupted" );
        }
    }
}

XclImpObjData* XclImpEscherObjList::FindObjData( sal_uInt32 nStrmPos ) const
{
    sal_uInt32 nCacheSize = maObjCache.size();
    sal_uInt32 nFoundIx;
    bool bFound = false;

    // find the correct cache entry
    for( sal_uInt32 nCacheIx = 0; !bFound && (nCacheIx < nCacheSize); ++nCacheIx )
    {
        sal_uInt32 nStrmEnd = (nCacheIx + 1 < nCacheSize) ? maObjCache[ nCacheIx + 1 ].mnStrmPos : ~0UL;
        bFound = (maObjCache[ nCacheIx ].mnStrmPos <= nStrmPos) && (nStrmPos < nStrmEnd);
        if( bFound )
            nFoundIx = nCacheIx;
    }

    // find the object in the found list range
    if( bFound )
    {
        sal_uInt32 nListEnd = (nFoundIx + 1 < nCacheSize) ? maObjCache[ nFoundIx + 1 ].mnListIndex : maObjDataList.Count();
        for( sal_uInt32 nListIx = maObjCache[ nFoundIx ].mnListIndex; nListIx < nListEnd; ++nListIx )
        {
            XclImpObjData* pObjData = maObjDataList.GetObject( nListIx );
            if( pObjData->ContainsStrmPos( nStrmPos ) )
                return pObjData;
        }
    }
    return NULL;
}


// Escher stream conversion ===================================================

XclImpDffManager::XclImpDffManager(
        const XclImpRoot& rRoot, XclImpObjectManager& rObjManager,
        sal_Int32 nOffsDgg, SvStream* pStData, SdrModel* pSdrModel, sal_Int32 nApplicationScale ) :
    SvxMSDffManager( rObjManager.GetEscherStream(), nOffsDgg, pStData, pSdrModel, nApplicationScale, COL_DEFAULT, 24, NULL, &rRoot.GetTracer().GetBaseTracer() ),
    XclImpRoot( rRoot ),
    mrObjManager( rObjManager ),
    mnOleImpFlags( 0 )
{
    SetSvxMSDffSettings( SVXMSDFF_SETTINGS_CROP_BITMAPS | SVXMSDFF_SETTINGS_IMPORT_EXCEL );

    if( OfaFilterOptions* pFilterOpt = OFF_APP()->GetFilterOptions() )
    {
        if( pFilterOpt->IsMathType2Math() )
            mnOleImpFlags |= OLE_MATHTYPE_2_STARMATH;
        if( pFilterOpt->IsWinWord2Writer() )
            mnOleImpFlags |= OLE_WINWORD_2_STARWRITER;
        if( pFilterOpt->IsPowerPoint2Impress() )
            mnOleImpFlags |= OLE_POWERPOINT_2_STARIMPRESS;
    }
}

void XclImpDffManager::SetSdrObject( XclImpEscherObj* pEscherObj, sal_uInt32 nId, SvxMSDffImportData& rData )
{
    SdrObject* pSdrObj = NULL;
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
            SvStorageRef xSrc( GetRootStorage() );
            SvStorageRef xDst( pDocShell->GetStorage() );

            if( SdrOle2Obj* pOleSdrObj = CreateSdrOLEFromStorage(
                    rStorageName, xSrc, xDst, aGraph, rAnchor, NULL, mnOleImpFlags ) )
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
    sal_uInt32 nFilePos = rStrm.Tell();

    if( XclImpEscherAnchor* pAnchor = mrObjManager.GetEscherAnchorAcc( nFilePos ) )
    {
        rStrm >> *pAnchor;

        ScDocument& rDoc = GetDoc();
        sal_uInt16 nScTab = pAnchor->mnScTab;

        Rectangle& rRect = rObjData.aChildAnchor;
        rRect.nLeft     = XclTools::CalcX( rDoc, nScTab, pAnchor->mnLCol, pAnchor->mnLX, HMM_PER_TWIPS );
        rRect.nTop      = XclTools::CalcY( rDoc, nScTab, pAnchor->mnTRow, pAnchor->mnTY, HMM_PER_TWIPS );
        rRect.nRight    = XclTools::CalcX( rDoc, nScTab, pAnchor->mnRCol, pAnchor->mnRX, HMM_PER_TWIPS );
        rRect.nBottom   = XclTools::CalcY( rDoc, nScTab, pAnchor->mnBRow, pAnchor->mnBY, HMM_PER_TWIPS );
        rObjData.bChildAnchor = sal_True;

        if( XclImpEscherObj* pEscherObj = mrObjManager.GetEscherObjAcc( nFilePos ) )
        {
            pEscherObj->SetAnchor( rRect );
            if( XclImpEscherOle* pOleObj = PTR_CAST( XclImpEscherOle, pEscherObj ) )
                pOleObj->SetBlipId( GetPropertyValue( DFF_Prop_pib ) );
        }
    }
}

SdrObject* XclImpDffManager::ProcessObj(
    SvStream& rStrm, DffObjData& rObjData, void*, Rectangle& rTextRect, SdrObject* pRetSdrObj )
{
    if( pRetSdrObj && ( GetPropertyValue( DFF_Prop_fNoFillHitTest ) & 0x10 ) &&
        ( IsProperty( DFF_Prop_fillColor ) == 0 ) )
    {   // maybe if there is no color, we could do this in ApplyAttributes ( writer ?, calc ? )
        pRetSdrObj->SetItem(XFillColorItem(XubString(), Color(0xffffff)));
    }

    // #98132# don't ask for the text-ID, Escher export doesn't set one
//    XclImpEscherTxo* pTxoObj = NULL;
//    sal_uInt32                      nTextId = GetPropertyValue( DFF_Prop_lTxid, 0 );
//    if( nTextId )
//        pTxoObj = mrObjManager.GetTxoFromStream( rObjData.rSpHd.nFilePos );
    XclImpEscherTxo* pTxoObj = mrObjManager.GetEscherTxoAcc( rObjData.rSpHd.nFilePos );

    // #102378# ...but that leads to another problem: now the first text is
    // applied to the omnipresent first dummy shape in the table. In consequence
    // we will miss this text while processing the real text object.
    // Solution: filter the dummy shape (it has the flag SP_FPATRIARCH set).
    bool bDummy = ::get_flag< sal_uInt32 >( rObjData.nSpFlags, SP_FPATRIARCH );

    if( !bDummy && pTxoObj )
    {
        if( rObjData.eShapeType == mso_sptRectangle )
            DELETEZ( pRetSdrObj );

        // Abstaende an den Raendern der Textbox lesen

        INT32 nDefault = 92076;
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

        INT32 nTextRotationAngle = 0;
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
                sal_Int32            nDX = rTextRect.Right() - rTextRect.Left();
                sal_Int32            nDY = rTextRect.Bottom() - rTextRect.Top();
                rTextRect.Left()       = nCenter.X() - nDY/2;
                rTextRect.Top()        = nCenter.Y() - nDX/2;
                rTextRect.Right()      = rTextRect.Left() + nDY;
                rTextRect.Bottom()     = rTextRect.Top()  + nDX;
            }
        }

        SdrRectObj* pTextSdrObj = new SdrRectObj( OBJ_TEXT, rTextRect );

        if( nTextRotationAngle )
        {
            double fAngle = nTextRotationAngle * nPi180;
            pTextSdrObj->NbcRotate( rTextRect.Center(), nTextRotationAngle, sin( fAngle ), cos( fAngle ) );
        }

        SfxItemSet aItemSet( pSdrModel->GetItemPool() );
        if( !pRetSdrObj )
        {
            if( ( GetPropertyValue( DFF_Prop_fNoFillHitTest ) & 0x10 ) &&
                ( IsProperty( DFF_Prop_fillColor ) == 0 ) )
            {   // maybe if there is no color, we could do this in ApplyAttributes ( writer ?, calc ? )
                pTextSdrObj->SetItem(XFillColorItem(XubString(), Color(0xffffff)));
            }

            ApplyAttributes( rStrm, aItemSet, pTextSdrObj );
        }
        switch( (MSO_WrapMode)GetPropertyValue( DFF_Prop_WrapText, mso_wrapSquare ) )
        {
            case mso_wrapNone :
            {
                if( GetPropertyValue( DFF_Prop_FitTextToShape ) & 2 )   // be sure this is FitShapeToText
                    aItemSet.Put( SdrTextAutoGrowWidthItem( TRUE ) );
            }
                break;

            case mso_wrapByPoints :
                aItemSet.Put( SdrTextContourFrameItem( TRUE ) );
                break;
        }
        aItemSet.Put( SdrTextAutoGrowHeightItem( FALSE ) );

        // Abstaende an den Raendern der Textbox setzen
        aItemSet.Put( SdrTextLeftDistItem( nTextLeft ) );
        aItemSet.Put( SdrTextRightDistItem( nTextRight ) );
        aItemSet.Put( SdrTextUpperDistItem( nTextTop ) );
        aItemSet.Put( SdrTextLowerDistItem( nTextBottom ) );

        // text alignment (#i12188# not stored in Escher stream, but in TXO)
        SdrTextHorzAdjust eSdrHorAlign = SDRTEXTHORZADJUST_LEFT;
        SvxAdjust eEEHorAlign = SVX_ADJUST_LEFT;
        switch( pTxoObj->GetHorAlign() )
        {
            case xlTxoHAlignLeft:       eSdrHorAlign = SDRTEXTHORZADJUST_LEFT;      eEEHorAlign = SVX_ADJUST_LEFT;      break;
            case xlTxoHAlignCenter:     eSdrHorAlign = SDRTEXTHORZADJUST_CENTER;    eEEHorAlign = SVX_ADJUST_CENTER;    break;
            case xlTxoHAlignRight:      eSdrHorAlign = SDRTEXTHORZADJUST_RIGHT;     eEEHorAlign = SVX_ADJUST_RIGHT;     break;
            case xlTxoHAlignJustify:    eSdrHorAlign = SDRTEXTHORZADJUST_BLOCK;     eEEHorAlign = SVX_ADJUST_BLOCK;     break;
        }
        aItemSet.Put( SdrTextHorzAdjustItem( eSdrHorAlign ) );

        SdrTextVertAdjust eSdrVerAlign = SDRTEXTVERTADJUST_TOP;
        switch( pTxoObj->GetVerAlign() )
        {
            case xlTxoVAlignTop:        eSdrVerAlign = SDRTEXTVERTADJUST_TOP;       break;
            case xlTxoHAlignCenter:     eSdrVerAlign = SDRTEXTVERTADJUST_CENTER;    break;
            case xlTxoVAlignBottom:     eSdrVerAlign = SDRTEXTVERTADJUST_BOTTOM;    break;
            case xlTxoVAlignJustify:    eSdrVerAlign = SDRTEXTVERTADJUST_BLOCK;     break;
        }
        aItemSet.Put( SdrTextVertAdjustItem( eSdrVerAlign ) );

        aItemSet.Put( SdrTextMinFrameHeightItem( rTextRect.Bottom() - rTextRect.Top() ) );
        pTextSdrObj->SetModel( pSdrModel );
        // #96092# SetModel() modifies the height of the text box -> restore it with NbcSetSnapRect()
        pTextSdrObj->NbcSetSnapRect( rTextRect );

        pTextSdrObj->SetItemSet( aItemSet );

        // Apply the text data from the Escher object to the SdrObj
        pTxoObj->ApplyTextOnSdrObj( *pTextSdrObj );

        // #i12188# this item has to be set after the text
        pTextSdrObj->SetItem( SvxAdjustItem( eEEHorAlign, EE_PARA_JUST ) );

        // rotate text with shape ?
        if( mnFix16Angle )
        {
            double fAngle = mnFix16Angle * nPi180;
            pTextSdrObj->NbcRotate( rObjData.rBoundRect.Center(), mnFix16Angle, sin( fAngle ), cos( fAngle ) );
        }

        if( pRetSdrObj )
        {
            SdrObject* pGroup = new SdrObjGroup;
            pGroup->GetSubList()->NbcInsertObject( pRetSdrObj );
            pGroup->GetSubList()->NbcInsertObject( pTextSdrObj );
            pRetSdrObj = pGroup;
        }
        else
            pRetSdrObj = pTextSdrObj;
        pTextSdrObj = NULL;

        if( (rObjData.nCalledByGroup == 0) ||
            ((rObjData.nSpFlags & SP_FGROUP) && (rObjData.nCalledByGroup < 2)) )
        {
            sal_uInt32 nTextId = GetPropertyValue( DFF_Prop_lTxid, 0 );
            StoreShapeOrder( rObjData.nShapeId, nTextId, pRetSdrObj );
        }
    }

    mrObjManager.UpdateConnectorRules( rObjData, pRetSdrObj );

    return pRetSdrObj;
}

sal_uInt32 XclImpDffManager::Calc_nBLIPPos( sal_uInt32 nOrgVal, sal_uInt32 nStreamPos ) const
{
    return nStreamPos + 4;
}

FASTBOOL XclImpDffManager::GetColorFromPalette( sal_uInt16 nIndex, Color& rColor ) const
{
    ColorData nColor = GetPalette().GetColorData( nIndex );

    if( nColor == COL_AUTO )
        return FALSE;

    rColor.SetColor( nColor );
    return TRUE;
}

sal_Bool XclImpDffManager::ShapeHasText( sal_uInt32 nShapeId, sal_uInt32 nFilePos ) const
{
    const XclImpEscherTxo* pTxoObj = mrObjManager.GetEscherTxo( nFilePos );
    return pTxoObj && pTxoObj->GetString();
}


// The object manager =========================================================

XclImpObjectManager::XclImpObjectManager( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot ),
    mbStartWithDummy( true )
{
}

XclImpObjectManager::~XclImpObjectManager()
{
}

const XclImpEscherObj* XclImpObjectManager::GetEscherObj( sal_uInt16 nScTab, sal_uInt16 nObjId ) const
{
    return maEscherObjList.GetObj( nScTab, nObjId );
}

XclImpEscherObj* XclImpObjectManager::GetEscherObjAcc( sal_uInt16 nScTab, sal_uInt16 nObjId )
{
    return maEscherObjList.GetObj( nScTab, nObjId );
}

const XclImpEscherObj* XclImpObjectManager::GetEscherObj( sal_uInt32 nStrmPos ) const
{
    return maEscherObjList.GetObj( nStrmPos );
}

XclImpEscherObj* XclImpObjectManager::GetEscherObjAcc( sal_uInt32 nStrmPos )
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

const XclImpEscherAnchor* XclImpObjectManager::GetEscherAnchor( sal_uInt32 nStrmPos ) const
{
    return maEscherObjList.GetAnchor( nStrmPos );
}

XclImpEscherAnchor* XclImpObjectManager::GetEscherAnchorAcc( sal_uInt32 nStrmPos )
{
    return maEscherObjList.GetAnchor( nStrmPos );
}


// *** Text boxes *** ---------------------------------------------------------

const XclImpEscherTxo* XclImpObjectManager::GetEscherTxo( sal_uInt32 nStrmPos ) const
{
    const XclImpEscherObj* pEscherObj = GetEscherObj( nStrmPos );
    return PTR_CAST( XclImpEscherTxo, pEscherObj );
}

XclImpEscherTxo* XclImpObjectManager::GetEscherTxoAcc( sal_uInt32 nStrmPos )
{
    return const_cast< XclImpEscherTxo* >( GetEscherTxo( nStrmPos ) );
}

const XclImpEscherNote* XclImpObjectManager::GetEscherNote( sal_uInt16 nScTab, sal_uInt16 nObjId ) const
{
    const XclImpEscherObj* pEscherObj = GetEscherObj( nScTab, nObjId );
    return PTR_CAST( XclImpEscherNote, pEscherObj );
}


// *** Chart *** --------------------------------------------------------------

bool XclImpObjectManager::IsCurrObjChart() const
{
    return PTR_CAST( XclImpEscherChart, GetLastEscherObj() ) != NULL;
}

XclImpChart* XclImpObjectManager::GetCurrChartData()
{
    if( XclImpEscherChart* pChartObj = PTR_CAST( XclImpEscherChart, GetLastEscherObj() ) )
        return pChartObj->GetChartData();
    return NULL;
}

XclImpChart* XclImpObjectManager::ReplaceChartData( XclImpStream& rStrm, XclChartType eNewType )
{
    XclImpEscherChart* pChartObj = PTR_CAST( XclImpEscherChart, GetLastEscherObj() );
    XclImpChart* pChart = pChartObj ? pChartObj->GetChartData() : NULL;

    DBG_ASSERT( pChart, "XclImpObjectManager::ReplaceChartData - no chart data found" );
    if( !pChart )
        return NULL;

    // #92909# create line chart if no X values present
    // #94149# of course only for XY charts!
    if( (eNewType == ctScatter) && !pChart->HasXValues() )
        eNewType = ctLine;

    XclImpChart* pNewChart = NULL;
    switch( eNewType )
    {
        case ctLine:
        case ctArea:
            pNewChart = new XclImpChartLine( *pChart, rStrm, eNewType == ctArea );
        break;
        case ctBar:
            pNewChart = new XclImpChartBar( *pChart, rStrm );
        break;
        case ctPie:
            pNewChart = new XclImpChartPie( *pChart, rStrm );
        break;
        case ctNet:
            pNewChart = new XclImpChartRadar( *pChart );
        break;
        case ctScatter:
            pNewChart = new XclImpChartScatter( *pChart, rStrm );
        break;
        case ctSurface:
            pNewChart = new XclImpChartSurface( *pChart );
        break;
        default:
            DBG_ERROR( "XclImpObjectManager::ReplaceChartData - unknown chart type" );
            return pChart;
    }

    pChartObj->SetChartData( pNewChart );
    return pNewChart;
}

void XclImpObjectManager::StartNewChartObj()
{
    XclImpEscherObj aTmp( GetRoot() );
    AppendEscherObj( new XclImpEscherChart( aTmp ) );
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
    rStrm.InitializeRecord( false );    // disable internal CONTINUE handling

    if( !maStreamConsumer.HasData() )
        return;

    sal_uInt32 nRecSize = rStrm.GetRecSize();
    sal_uInt32 nStrmPos = maStreamConsumer.Tell();
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
    rStrm.InitializeRecord( false );      // disable internal CONTINUE handling

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
            case EXC_ID_OBJ_FTCBLSFMLA: ReadObjTbxSubRec( rStrm, nSubRecId );       break;
        }

        rStrm.PopPosition();
        // sometimes the last subrecord has an invalid length -> min()
        rStrm.Ignore( ::std::min< sal_uInt32 >( nSubRecSize, rStrm.GetRecLeft() ) );
    }
}

void XclImpObjectManager::ReadTxo( XclImpStream& rStrm )
{
    rStrm.InitializeRecord( false );      // disable internal CONTINUE handling

    sal_uInt16 nAlign, nTextLen, nFormCnt;
    ::std::auto_ptr< XclImpString > pString;

    /*  Let this function read all records in every case, even if text cannot be
        applied to the current object - this skips all CONTINUE records. */

    // step 1: read TXO record
    rStrm >> nAlign;
    rStrm.Ignore( 8 );
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
            rStrm.InitializeRecord( false );
            String aText;
            rStrm.AppendUniString( aText, nTextLen );
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
            rStrm.InitializeRecord( false );

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
        }
    }
}


// private --------------------------------------------------------------------

XclImpDffManager& XclImpObjectManager::GetDffManager()
{
    if( !mpDffManager.get() )
        mpDffManager.reset( new XclImpDffManager( GetRoot(), *this, 0, NULL, GetDoc().GetDrawLayer(), 1440 ) );
    return *mpDffManager;
}

void XclImpObjectManager::UpdateConnectorRules( const DffObjData& rObjData, SdrObject* pSdrObj )
{
    if( mpSolverContainer.get() && pSdrObj )
    {
        for( SvxMSDffConnectorRule* pPtr = (SvxMSDffConnectorRule*)mpSolverContainer->aCList.First();
                pPtr; pPtr = (SvxMSDffConnectorRule*)mpSolverContainer->aCList.Next() )
        {
            if( rObjData.nShapeId == pPtr->nShapeC )
            {
                pPtr->pCObj = pSdrObj;
            }
            else
            {
                if ( rObjData.nShapeId == pPtr->nShapeA )
                {
                    pPtr->pAObj = pSdrObj;
                    pPtr->nSpFlagsA = rObjData.nSpFlags;
                }
                if ( rObjData.nShapeId == pPtr->nShapeB )
                {
                    pPtr->pBObj = pSdrObj;
                    pPtr->nSpFlagsB = rObjData.nSpFlags;
                }
            }
        }
    }
}

void XclImpObjectManager::SetSkipObj( sal_uInt16 nScTab, sal_uInt16 nObjId )
{
    maSkipObjVec.push_back( XclSkipObj( nScTab, nObjId ) );
}

void XclImpObjectManager::Apply()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "sc", "dr104026", "XclImpObjectManager::Apply" );

    // mark objects to be skipped
    for( XclSkipObjVec::const_iterator aIter = maSkipObjVec.begin(), aEnd = maSkipObjVec.end(); aIter != aEnd; ++aIter )
        if( XclImpEscherObj* pEscherObj = maEscherObjList.GetObj( aIter->mnScTab, aIter->mnObjId ) )
            pEscherObj->SetSkip();

    // progress bar
    ScfProgressBar aProgress( GetDocShell(), STR_PROGRESS_CALCULATING );
    maEscherObjList.InitProgress( aProgress );

    // insert the objects into the drawing layer
    maEscherObjList.Apply( aProgress );

    // connector rules
    if( mpSolverContainer.get() )
        GetDffManager().SolveSolver( *mpSolverContainer );
}

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

    switch( nObjType )
    {
        case EXC_OBJ_CMO_GROUP:
        case EXC_OBJ_CMO_LINE:
        case EXC_OBJ_CMO_RECTANGLE:
        case EXC_OBJ_CMO_ELLIPSE:
        case EXC_OBJ_CMO_ARC:
        case EXC_OBJ_CMO_POLYGON:
        case EXC_OBJ_CMO_DRAWING:
            ReplaceEscherObj( new XclImpEscherDrawing( *pEscherObj ) );
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
            ReplaceEscherObj( new XclImpEscherTbxCtrl( *pEscherObj, nObjType ) );
        break;
        case EXC_OBJ_CMO_PICTURE:
            ReplaceEscherObj( new XclImpEscherOle( *pEscherObj ) );
        break;
        case EXC_OBJ_CMO_CHART:
            ReplaceEscherObj( new XclImpEscherChart( *pEscherObj ) );
        break;
        case EXC_OBJ_CMO_DIALOG:        // not supported in Calc
        case EXC_OBJ_CMO_SPIN:          // not supported in Calc
        case EXC_OBJ_CMO_SCROLLBAR:     // not supported in Calc
            pEscherObj->SetSkip();
        break;
        default:
            DBG_ERROR1( "XclImpObjectManager::ReadObjFtCmo - unknown object type 0x%04hX", nObjType );
            ReplaceEscherObj( new XclImpEscherDrawing( *pEscherObj ) );
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
            default:    DBG_ERRORFILE( "XclImpObjectManager::ReadObjTbxSubRec - unknown subrecord" );
        }
    }
}

SvxMSDffSolverContainer& XclImpObjectManager::GetSolverContainer()
{
    if( !mpSolverContainer.get() )
        mpSolverContainer.reset( new SvxMSDffSolverContainer );
    return *mpSolverContainer;
}

XclImpOcxConverter& XclImpObjectManager::GetOcxConverter()
{
    if( !mpOcxConverter.get() )
        mpOcxConverter.reset( new XclImpOcxConverter( GetRoot() ) );
    return *mpOcxConverter;
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

    mpMemStrm.reset( new SvMemoryStream );
    rStrm.CopyToStream( *mpMemStrm, 8 + nPropSetSize );
    mpMemStrm->Seek( STREAM_SEEK_TO_BEGIN );
    maPropReader.ReadPropSet( *mpMemStrm, NULL );
}

sal_uInt32 XclImpEscherPropSet::GetPropertyValue( sal_uInt16 nPropId, sal_uInt32 nDefault ) const
{
    return maPropReader.GetPropertyValue( nPropId, nDefault );
}

void XclImpEscherPropSet::FillToItemSet( SfxItemSet& rItemSet ) const
{
    if( mpMemStrm.get() )
        maPropReader.ApplyAttributes( *mpMemStrm, rItemSet );
}

XclImpStream& operator>>( XclImpStream& rStrm, XclImpEscherPropSet& rPropSet )
{
    rPropSet.Read( rStrm );
    return rStrm;
}


// ============================================================================

