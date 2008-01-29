/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xeescher.cxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:25:42 $
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

#include "xeescher.hxx"

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/awt/VisualEffect.hpp>
#include <com/sun/star/awt/ScrollBarOrientation.hpp>
#include <com/sun/star/form/binding/XBindableValue.hpp>
#include <com/sun/star/form/binding/XValueBinding.hpp>
#include <com/sun/star/form/binding/XListEntrySink.hpp>
#include <com/sun/star/form/binding/XListEntrySource.hpp>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>

#include <svx/unoapi.hxx>
#include <svx/svdoole2.hxx>

#include "editutil.hxx"
#include "unonames.hxx"
#include "convuno.hxx"
#include "postit.hxx"

#include "fapihelper.hxx"
#include "xechart.hxx"
#include "xeformula.hxx"
#include "xelink.hxx"
#include "xename.hxx"
#include "xestyle.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::lang::XServiceInfo;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::drawing::XShape;
using ::com::sun::star::frame::XModel;
using ::com::sun::star::embed::XEmbeddedObject;
using ::com::sun::star::awt::XControlModel;
using ::com::sun::star::form::binding::XBindableValue;
using ::com::sun::star::form::binding::XValueBinding;
using ::com::sun::star::form::binding::XListEntrySink;
using ::com::sun::star::form::binding::XListEntrySource;
using ::com::sun::star::script::ScriptEventDescriptor;
using ::com::sun::star::table::CellAddress;
using ::com::sun::star::table::CellRangeAddress;

// ============================================================================

XclExpControlObjHelper::XclExpControlObjHelper( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot ),
    mnEntryCount( 0 )
{
}

XclExpControlObjHelper::~XclExpControlObjHelper()
{
}

void XclExpControlObjHelper::ConvertSheetLinks( Reference< XShape > xShape )
{
    mxCellLink.reset();
    mxSrcRange.reset();
    mnEntryCount = 0;

    // get control model
    Reference< XControlModel > xCtrlModel = XclControlObjHelper::GetModelFromShape( xShape );
    if( !xCtrlModel.is() )
        return;

    // *** cell link *** ------------------------------------------------------

    Reference< XBindableValue > xBindable( xCtrlModel, UNO_QUERY );
    if( xBindable.is() )
    {
        Reference< XServiceInfo > xServInfo( xBindable->getValueBinding(), UNO_QUERY );
        if( xServInfo.is() && xServInfo->supportsService( CREATE_OUSTRING( SC_SERVICENAME_VALBIND ) ) )
        {
            ScfPropertySet aBindProp( xServInfo );
            CellAddress aApiAddress;
            if( aBindProp.GetProperty( aApiAddress, CREATE_OUSTRING( SC_UNONAME_BOUNDCELL ) ) )
            {
                ScAddress aCellLink;
                ScUnoConversion::FillScAddress( aCellLink, aApiAddress );
                if( GetTabInfo().IsExportTab( aCellLink.Tab() ) )
                    mxCellLink = GetFormulaCompiler().CreateFormula( EXC_FMLATYPE_CONTROL, aCellLink );
            }
        }
    }

    // *** source range *** ---------------------------------------------------

    Reference< XListEntrySink > xEntrySink( xCtrlModel, UNO_QUERY );
    if( xEntrySink.is() )
    {
        Reference< XServiceInfo > xServInfo( xEntrySink->getListEntrySource(), UNO_QUERY );
        if( xServInfo.is() && xServInfo->supportsService( CREATE_OUSTRING( SC_SERVICENAME_LISTSOURCE ) ) )
        {
            ScfPropertySet aSinkProp( xServInfo );
            CellRangeAddress aApiRange;
            if( aSinkProp.GetProperty( aApiRange, CREATE_OUSTRING( SC_UNONAME_CELLRANGE ) ) )
            {
                ScRange aSrcRange;
                ScUnoConversion::FillScRange( aSrcRange, aApiRange );
                if( (aSrcRange.aStart.Tab() == aSrcRange.aEnd.Tab()) && GetTabInfo().IsExportTab( aSrcRange.aStart.Tab() ) )
                    mxSrcRange = GetFormulaCompiler().CreateFormula( EXC_FMLATYPE_CONTROL, aSrcRange );
                mnEntryCount = static_cast< sal_uInt16 >( aSrcRange.aEnd.Col() - aSrcRange.aStart.Col() + 1 );
            }
        }
    }
}

void XclExpControlObjHelper::WriteFormula( XclExpStream& rStrm, const XclTokenArray& rTokArr ) const
{
    sal_uInt16 nFmlaSize = rTokArr.GetSize();
    rStrm << nFmlaSize << sal_uInt32( 0 );
    rTokArr.WriteArray( rStrm );
    if( nFmlaSize & 1 )             // pad to 16-bit
        rStrm << sal_uInt8( 0 );
}

void XclExpControlObjHelper::WriteFormulaSubRec( XclExpStream& rStrm, sal_uInt16 nSubRecId, const XclTokenArray& rTokArr ) const
{
    rStrm.StartRecord( nSubRecId, (rTokArr.GetSize() + 5) & ~1 );
    WriteFormula( rStrm, rTokArr );
    rStrm.EndRecord();
}

// ----------------------------------------------------------------------------

#if EXC_EXP_OCX_CTRL

XclExpOcxControlObj::XclExpOcxControlObj( const XclExpRoot& rRoot, Reference< XShape > xShape,
        const String& rClassName, sal_uInt32 nStrmStart, sal_uInt32 nStrmSize ) :
    XclObj( rRoot, EXC_OBJ_CMO_PICTURE, true ),
    XclExpControlObjHelper( rRoot ),
    maClassName( rClassName ),
    mnStrmStart( nStrmStart ),
    mnStrmSize( nStrmSize )
{
    ScfPropertySet aCtrlProp( XclControlObjHelper::GetModelFromShape( xShape ) );

    // OBJ record flags
    SetLocked( TRUE );
    SetPrintable( aCtrlProp.GetBoolProperty( CREATE_OUSTRING( "Printable" ) ) );
    SetAutoFill( FALSE );
    SetAutoLine( FALSE );

    // fill DFF property set
    XclEscherEx& rEscherEx = *pMsodrawing->GetEscherEx();
    rEscherEx.OpenContainer( ESCHER_SpContainer );
    rEscherEx.AddShape( ESCHER_ShpInst_HostControl, SHAPEFLAG_HAVESPT | SHAPEFLAG_HAVEANCHOR | SHAPEFLAG_OLESHAPE );
    Rectangle aDummyRect;
    EscherPropertyContainer aPropOpt( rEscherEx, rEscherEx.QueryPicStream(), aDummyRect );
    aPropOpt.AddOpt( ESCHER_Prop_FitTextToShape,    0x00080008 );   // bool field
    aPropOpt.AddOpt( ESCHER_Prop_lineColor,         0x08000040 );
    aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash,   0x00080000 );   // bool field

    // #i51348# name of the control, may overwrite shape name
    OUString aCtrlName;
    if( aCtrlProp.GetProperty( aCtrlName, CREATE_OUSTRING( "Name" ) ) && (aCtrlName.getLength() > 0) )
        aPropOpt.AddOpt( ESCHER_Prop_wzName, aCtrlName );

    // meta file
    //! TODO - needs check
    Reference< XPropertySet > xShapePS( xShape, UNO_QUERY );
    if( xShapePS.is() && aPropOpt.CreateGraphicProperties( xShapePS, CREATE_STRING( "MetaFile" ), sal_False ) )
    {
        sal_uInt32 nBlipId;
        if( aPropOpt.GetOpt( ESCHER_Prop_pib, nBlipId ) )
            aPropOpt.AddOpt( ESCHER_Prop_pictureId, nBlipId );
    }

    // write DFF property set to stream
    aPropOpt.Commit( rEscherEx.GetStream() );

    // anchor
    if( SdrObject* pSdrObj = ::GetSdrObjectFromXShape( xShape ) )
        XclExpEscherAnchor( rRoot, *pSdrObj ).WriteData( rEscherEx );
    rEscherEx.AddAtom( 0, ESCHER_ClientData );                       // OBJ record
    rEscherEx.CloseContainer();  // ESCHER_SpContainer

    pMsodrawing->UpdateStopPos();

    // spreadsheet links
    ConvertSheetLinks( xShape );
}

void XclExpOcxControlObj::WriteSubRecs( XclExpStream& rStrm )
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

    const XclTokenArray* pCellLink = GetCellLinkTokArr();
    sal_uInt16 nCellLinkSize = pCellLink ? ((pCellLink->GetSize() + 7) & 0xFFFE) : 0;

    const XclTokenArray* pSrcRange = GetSourceRangeTokArr();
    sal_uInt16 nSrcRangeSize = pSrcRange ? ((pSrcRange->GetSize() + 7) & 0xFFFE) : 0;

    sal_uInt16 nPictFmlaSize = nFirstPartSize + nCellLinkSize + nSrcRangeSize + 18;
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
            << sal_uInt32( 0 );                         // unknown
    // cell link
    rStrm << nCellLinkSize;
    if( pCellLink )
        WriteFormula( rStrm, *pCellLink );
    // list source range
    rStrm << nSrcRangeSize;
    if( pSrcRange )
        WriteFormula( rStrm, *pSrcRange );

    rStrm.EndRecord();
}

#else

XclExpTbxControlObj::XclExpTbxControlObj( const XclExpRoot& rRoot, Reference< XShape > xShape ) :
    XclObj( rRoot, EXC_OBJ_CMO_UNKNOWN, true ),
    XclExpControlObjHelper( rRoot ),
    mnHeight( 0 ),
    mnState( 0 ),
    mnLineCount( 0 ),
    mnSelEntry( 0 ),
    mnScrollValue( 0 ),
    mnScrollMin( 0 ),
    mnScrollMax( 100 ),
    mnScrollStep( 1 ),
    mnScrollPage( 10 ),
    mbFlatButton( false ),
    mbFlatBorder( false ),
    mbMultiSel( false ),
    mbScrollHor( false )
{
    namespace FormCompType = ::com::sun::star::form::FormComponentType;
    namespace AwtVisualEffect = ::com::sun::star::awt::VisualEffect;
    namespace AwtScrollOrient = ::com::sun::star::awt::ScrollBarOrientation;

    ScfPropertySet aCtrlProp( XclControlObjHelper::GetModelFromShape( xShape ) );
    if( !xShape.is() || !aCtrlProp.Is() )
        return;

    mnHeight = xShape->getSize().Height;
    if( mnHeight <= 0 )
        return;

    // control type
    sal_Int16 nClassId = 0;
    if( aCtrlProp.GetProperty( nClassId, CREATE_OUSTRING( "ClassId" ) ) )
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
            case FormCompType::SCROLLBAR:       mnObjType = EXC_OBJ_CMO_SCROLLBAR;      break;
            case FormCompType::SPINBUTTON:      mnObjType = EXC_OBJ_CMO_SPIN;           break;
        }
    }
    if( mnObjType == EXC_OBJ_CMO_UNKNOWN )
        return;

    // OBJ record flags
    SetLocked( TRUE );
    SetPrintable( aCtrlProp.GetBoolProperty( CREATE_OUSTRING( "Printable" ) ) );
    SetAutoFill( FALSE );
    SetAutoLine( FALSE );

    // fill DFF property set
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

    // #i51348# name of the control, may overwrite shape name
    OUString aCtrlName;
    if( aCtrlProp.GetProperty( aCtrlName, CREATE_OUSTRING( "Name" ) ) && (aCtrlName.getLength() > 0) )
        aPropOpt.AddOpt( ESCHER_Prop_wzName, aCtrlName );

    // write DFF property set to stream
    aPropOpt.Commit( rEscherEx.GetStream() );

    // anchor
    if( SdrObject* pSdrObj = ::GetSdrObjectFromXShape( xShape ) )
        XclExpEscherAnchor( rRoot, *pSdrObj ).WriteData( rEscherEx );
    rEscherEx.AddAtom( 0, ESCHER_ClientData );                       // OBJ record
    pMsodrawing->UpdateStopPos();

    // control label
    OUString aString;
    if( aCtrlProp.GetProperty( aString, CREATE_OUSTRING( "Label" ) ) )
    {
        /*  Be sure to construct the MSODRAWING record containing the
            ClientTextbox atom after the base OBJ's MSODRAWING record data is
            completed. */
        pClientTextbox = new XclMsodrawing( GetRoot() );
        pClientTextbox->GetEscherEx()->AddAtom( 0, ESCHER_ClientTextbox );  // TXO record
        pClientTextbox->UpdateStopPos();

        sal_uInt16 nXclFont = EXC_FONT_APP;
        if( aString.getLength() > 0 )
        {
            XclFontData aFontData;
            GetFontPropSetHelper().ReadFontProperties( aFontData, aCtrlProp, EXC_FONTPROPSET_CONTROL );
            if( (aFontData.maName.Len() > 0) && (aFontData.mnHeight > 0) )
                nXclFont = GetFontBuffer().Insert( aFontData, EXC_COLOR_CTRLTEXT );
        }

        pTxo = new XclTxo( aString, nXclFont );
        pTxo->SetHorAlign( (mnObjType == EXC_OBJ_CMO_BUTTON) ? EXC_TXO_HOR_CENTER : EXC_TXO_HOR_LEFT );
        pTxo->SetVerAlign( EXC_TXO_VER_CENTER );
    }

    rEscherEx.CloseContainer();  // ESCHER_SpContainer

    // other properties
    aCtrlProp.GetProperty( mnLineCount, CREATE_OUSTRING( "LineCount" ) );

    // border style
    sal_Int16 nApiButton = AwtVisualEffect::LOOK3D;
    sal_Int16 nApiBorder = AwtVisualEffect::LOOK3D;
    switch( nClassId )
    {
        case FormCompType::LISTBOX:
        case FormCompType::COMBOBOX:
            aCtrlProp.GetProperty( nApiBorder, CREATE_OUSTRING( "Border" ) );
        break;
        case FormCompType::CHECKBOX:
        case FormCompType::RADIOBUTTON:
            aCtrlProp.GetProperty( nApiButton, CREATE_OUSTRING( "VisualEffect" ) );
            nApiBorder = AwtVisualEffect::NONE;
        break;
        // Push button cannot be set to flat in Excel
        case FormCompType::COMMANDBUTTON:
            nApiBorder = AwtVisualEffect::LOOK3D;
        break;
        // Label does not support a border in Excel
        case FormCompType::FIXEDTEXT:
            nApiBorder = AwtVisualEffect::NONE;
        break;
        /*  Scroll bar and spin button have a "Border" property, but it is
            really used for a border, and not for own 3D/flat look (#i34712#). */
        case FormCompType::SCROLLBAR:
        case FormCompType::SPINBUTTON:
            nApiButton = AwtVisualEffect::LOOK3D;
            nApiBorder = AwtVisualEffect::NONE;
        break;
        // Group box does not support flat style (#i34712#)
        case FormCompType::GROUPBOX:
            nApiBorder = AwtVisualEffect::LOOK3D;
        break;
    }
    mbFlatButton = nApiButton != AwtVisualEffect::LOOK3D;
    mbFlatBorder = nApiBorder != AwtVisualEffect::LOOK3D;

    // control state
    sal_Int16 nApiState = 0;
    if( aCtrlProp.GetProperty( nApiState, CREATE_OUSTRING( "State" ) ) )
    {
        switch( nApiState )
        {
            case 0: mnState = EXC_OBJ_CBLS_STATE_UNCHECK;   break;
            case 1: mnState = EXC_OBJ_CBLS_STATE_CHECK;     break;
            case 2: mnState = EXC_OBJ_CBLS_STATE_TRI;       break;
        }
    }

    // special control contents
    switch( nClassId )
    {
        case FormCompType::LISTBOX:
        {
            mbMultiSel = aCtrlProp.GetBoolProperty( CREATE_OUSTRING( "MultiSelection" ) );
            Sequence< sal_Int16 > aSelection;
            if( aCtrlProp.GetProperty( aSelection, CREATE_OUSTRING( "SelectedItems" ) ) )
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
            if( aCtrlProp.GetBoolProperty( CREATE_OUSTRING( "Dropdown" ) ) )
                mnObjType = EXC_OBJ_CMO_COMBOBOX;
        }
        break;

        case FormCompType::COMBOBOX:
        {
            Sequence< OUString > aStringList;
            OUString aDefText;
            if( aCtrlProp.GetProperty( aStringList, CREATE_OUSTRING( "StringItemList" ) ) &&
                aCtrlProp.GetProperty( aDefText, CREATE_OUSTRING( "Text" ) ) &&
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
            if( !aCtrlProp.GetBoolProperty( CREATE_OUSTRING( "Dropdown" ) ) )
                mnObjType = EXC_OBJ_CMO_LISTBOX;
        }
        break;

        case FormCompType::SCROLLBAR:
        {
            sal_Int32 nApiValue = 0;
            if( aCtrlProp.GetProperty( nApiValue, CREATE_OUSTRING( "ScrollValueMin" ) ) )
                mnScrollMin = limit_cast< sal_Int16 >( nApiValue, EXC_OBJ_SBS_MINSCROLL, EXC_OBJ_SBS_MAXSCROLL );
            if( aCtrlProp.GetProperty( nApiValue, CREATE_OUSTRING( "ScrollValueMax" ) ) )
                mnScrollMax = limit_cast< sal_Int16 >( nApiValue, mnScrollMin, EXC_OBJ_SBS_MAXSCROLL );
            if( aCtrlProp.GetProperty( nApiValue, CREATE_OUSTRING( "ScrollValue" ) ) )
                mnScrollValue = limit_cast< sal_Int16 >( nApiValue, mnScrollMin, mnScrollMax );
            if( aCtrlProp.GetProperty( nApiValue, CREATE_OUSTRING( "LineIncrement" ) ) )
                mnScrollStep = limit_cast< sal_Int16 >( nApiValue, EXC_OBJ_SBS_MINSCROLL, EXC_OBJ_SBS_MAXSCROLL );
            if( aCtrlProp.GetProperty( nApiValue, CREATE_OUSTRING( "BlockIncrement" ) ) )
                mnScrollPage = limit_cast< sal_Int16 >( nApiValue, EXC_OBJ_SBS_MINSCROLL, EXC_OBJ_SBS_MAXSCROLL );
            if( aCtrlProp.GetProperty( nApiValue, CREATE_OUSTRING( "Orientation" ) ) )
                mbScrollHor = nApiValue == AwtScrollOrient::HORIZONTAL;
        }
        break;

        case FormCompType::SPINBUTTON:
        {
            sal_Int32 nApiValue = 0;
            if( aCtrlProp.GetProperty( nApiValue, CREATE_OUSTRING( "SpinValueMin" ) ) )
                mnScrollMin = limit_cast< sal_Int16 >( nApiValue, EXC_OBJ_SBS_MINSCROLL, EXC_OBJ_SBS_MAXSCROLL );
            if( aCtrlProp.GetProperty( nApiValue, CREATE_OUSTRING( "SpinValueMax" ) ) )
                mnScrollMax = limit_cast< sal_Int16 >( nApiValue, mnScrollMin, EXC_OBJ_SBS_MAXSCROLL );
            if( aCtrlProp.GetProperty( nApiValue, CREATE_OUSTRING( "SpinValue" ) ) )
                mnScrollValue = limit_cast< sal_Int16 >( nApiValue, mnScrollMin, mnScrollMax );
            if( aCtrlProp.GetProperty( nApiValue, CREATE_OUSTRING( "SpinIncrement" ) ) )
                mnScrollStep = limit_cast< sal_Int16 >( nApiValue, EXC_OBJ_SBS_MINSCROLL, EXC_OBJ_SBS_MAXSCROLL );
            if( aCtrlProp.GetProperty( nApiValue, CREATE_OUSTRING( "Orientation" ) ) )
                mbScrollHor = nApiValue == AwtScrollOrient::HORIZONTAL;
        }
        break;
    }

    // spreadsheet links
    ConvertSheetLinks( xShape );
}

bool XclExpTbxControlObj::SetMacroLink( const ScriptEventDescriptor& rEvent )
{
    if( rEvent.ListenerType.getLength() && (rEvent.ListenerType == XclControlObjHelper::GetTbxListenerType( mnObjType )) &&
        rEvent.EventMethod.getLength() && (rEvent.EventMethod == XclControlObjHelper::GetTbxEventMethod( mnObjType )) &&
        (rEvent.ScriptType == XclControlObjHelper::GetTbxScriptType()) )
    {
        // macro name is stored in a NAME record, and referred to by a formula containing a tNameXR token
        String aMacroName( XclControlObjHelper::GetXclMacroName( rEvent.ScriptCode ) );
        if( aMacroName.Len() )
        {
            sal_uInt16 nExtSheet = GetLocalLinkManager().FindExtSheet( EXC_EXTSH_OWNDOC );
            sal_uInt16 nNameIdx = GetNameManager().InsertMacroCall( aMacroName, true, false );
            mxMacroLink = GetFormulaCompiler().CreateNameXFormula( nExtSheet, nNameIdx );
            return true;
        }
    }
    return false;
}

void XclExpTbxControlObj::WriteSubRecs( XclExpStream& rStrm )
{
    switch( mnObjType )
    {
        // *** Push buttons, labels ***

        case EXC_OBJ_CMO_BUTTON:
        case EXC_OBJ_CMO_LABEL:
            // ftMacro - macro link
            WriteMacroSubRec( rStrm );
        break;

        // *** Check boxes, option buttons ***

        case EXC_OBJ_CMO_CHECKBOX:
        case EXC_OBJ_CMO_OPTIONBUTTON:
        {
            // ftCbls - box properties
            sal_uInt16 nStyle = 0;
            ::set_flag( nStyle, EXC_OBJ_CBLS_FLAT, mbFlatButton );

            rStrm.StartRecord( EXC_ID_OBJ_FTCBLS, 12 );
            rStrm << mnState;
            rStrm.WriteZeroBytes( 8 );
            rStrm << nStyle;
            rStrm.EndRecord();

            // ftMacro - macro link
            WriteMacroSubRec( rStrm );
            // ftCblsFmla subrecord - cell link
            WriteCellLinkSubRec( rStrm, EXC_ID_OBJ_FTCBLSFMLA );
        }
        break;

        // *** List boxes, combo boxes ***

        case EXC_OBJ_CMO_LISTBOX:
        case EXC_OBJ_CMO_COMBOBOX:
        {
            sal_uInt16 nEntryCount = GetSourceEntryCount();

            // ftSbs subrecord - Scroll bars
            sal_Int32 nLineHeight = XclTools::GetHmmFromTwips( 200 );   // always 10pt
            if( mnObjType == EXC_OBJ_CMO_LISTBOX )
                mnLineCount = static_cast< sal_uInt16 >( mnHeight / nLineHeight );
            mnScrollValue = 0;
            mnScrollMin = 0;
            sal_uInt16 nInvisLines = (nEntryCount >= mnLineCount) ? (nEntryCount - mnLineCount) : 0;
            mnScrollMax = limit_cast< sal_Int16 >( nInvisLines, EXC_OBJ_SBS_MINSCROLL, EXC_OBJ_SBS_MAXSCROLL );
            mnScrollStep = 1;
            mnScrollPage = limit_cast< sal_Int16 >( mnLineCount, EXC_OBJ_SBS_MINSCROLL, EXC_OBJ_SBS_MAXSCROLL );
            mbScrollHor = false;
            WriteSbs( rStrm );

            // ftMacro - macro link
            WriteMacroSubRec( rStrm );
            // ftSbsFmla subrecord - cell link
            WriteCellLinkSubRec( rStrm, EXC_ID_OBJ_FTSBSFMLA );

            // ftLbsData - source data range and box properties
            sal_uInt16 nStyle = mbMultiSel ? EXC_OBJ_LBS_SEL_MULTI : EXC_OBJ_LBS_SEL_SIMPLE;
            ::set_flag( nStyle, EXC_OBJ_LBS_FLAT, mbFlatBorder );

            rStrm.StartRecord( EXC_ID_OBJ_FTLBSDATA, 0 );

            if( const XclTokenArray* pSrcRange = GetSourceRangeTokArr() )
            {
                rStrm << static_cast< sal_uInt16 >( (pSrcRange->GetSize() + 7) & 0xFFFE );
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
                rStrm << EXC_OBJ_LBS_COMBO_STD << mnLineCount;
            }

            rStrm.EndRecord();
        }
        break;

        // *** Spin buttons, scrollbars ***

        case EXC_OBJ_CMO_SPIN:
        case EXC_OBJ_CMO_SCROLLBAR:
        {
            // ftSbs subrecord - scroll bars
            WriteSbs( rStrm );
            // ftMacro - macro link
            WriteMacroSubRec( rStrm );
            // ftSbsFmla subrecord - cell link
            WriteCellLinkSubRec( rStrm, EXC_ID_OBJ_FTSBSFMLA );
        }
        break;

        // *** Group boxes ***

        case EXC_OBJ_CMO_GROUPBOX:
        {
            // ftMacro - macro link
            WriteMacroSubRec( rStrm );

            // ftGboData subrecord - group box properties
            sal_uInt16 nStyle = 0;
            ::set_flag( nStyle, EXC_OBJ_GBO_FLAT, mbFlatBorder );

            rStrm.StartRecord( EXC_ID_OBJ_FTGBODATA, 6 );
            rStrm   << sal_uInt32( 0 )
                    << nStyle;
            rStrm.EndRecord();
        }
        break;
    }
}

void XclExpTbxControlObj::WriteMacroSubRec( XclExpStream& rStrm )
{
    if( mxMacroLink.is() )
        WriteFormulaSubRec( rStrm, EXC_ID_OBJ_FTMACRO, *mxMacroLink );
}

void XclExpTbxControlObj::WriteCellLinkSubRec( XclExpStream& rStrm, sal_uInt16 nSubRecId )
{
    if( const XclTokenArray* pCellLink = GetCellLinkTokArr() )
        WriteFormulaSubRec( rStrm, nSubRecId, *pCellLink );
}

void XclExpTbxControlObj::WriteSbs( XclExpStream& rStrm )
{
    sal_uInt16 nOrient = 0;
    ::set_flag( nOrient, EXC_OBJ_SBS_HORIZONTAL, mbScrollHor );
    sal_uInt16 nStyle = EXC_OBJ_SBS_DEFAULTFLAGS;
    ::set_flag( nStyle, EXC_OBJ_SBS_FLAT, mbFlatButton );

    rStrm.StartRecord( EXC_ID_OBJ_FTSBS, 20 );
    rStrm   << sal_uInt32( 0 )              // reserved
            << mnScrollValue                // thumb position
            << mnScrollMin                  // thumb min pos
            << mnScrollMax                  // thumb max pos
            << mnScrollStep                 // line increment
            << mnScrollPage                 // page increment
            << nOrient                      // 0 = vertical, 1 = horizontal
            << sal_uInt16( 15 )             // thumb width
            << nStyle;                      // flags/style
    rStrm.EndRecord();
}

#endif

// ----------------------------------------------------------------------------

XclExpChartObj::XclExpChartObj( const XclExpRoot& rRoot, Reference< XShape > xShape ) :
    XclObj( rRoot, EXC_OBJ_CMO_CHART ),
    XclExpRoot( rRoot )
{
    // create the MSODRAWING record contents for the chart object
    XclEscherEx& rEscherEx = *pMsodrawing->GetEscherEx();
    rEscherEx.OpenContainer( ESCHER_SpContainer );
    rEscherEx.AddShape( ESCHER_ShpInst_HostControl, SHAPEFLAG_HAVEANCHOR | SHAPEFLAG_HAVESPT );
    EscherPropertyContainer aPropOpt;
    aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x01040104 );
    aPropOpt.AddOpt( ESCHER_Prop_FitTextToShape, 0x00080008 );
    aPropOpt.AddOpt( ESCHER_Prop_fillColor, 0x0800004E );
    aPropOpt.AddOpt( ESCHER_Prop_fillBackColor, 0x0800004D );
    aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x00110010 );
    aPropOpt.AddOpt( ESCHER_Prop_lineColor, 0x0800004D );
    aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x00080008 );
    aPropOpt.AddOpt( ESCHER_Prop_fshadowObscured, 0x00020000 );
    aPropOpt.AddOpt( ESCHER_Prop_fPrint, 0x00080000 );
    aPropOpt.Commit( rEscherEx.GetStream() );

    // client anchor
    if( SdrObject* pSdrObj = ::GetSdrObjectFromXShape( xShape ) )
        XclExpEscherAnchor( rRoot, *pSdrObj ).WriteData( rEscherEx );

    // client data (the following OBJ record)
    rEscherEx.AddAtom( 0, ESCHER_ClientData );
    rEscherEx.CloseContainer();  // ESCHER_SpContainer
    pMsodrawing->UpdateStopPos();

    // load the chart OLE object
    if( SdrOle2Obj* pSdrOleObj = dynamic_cast< SdrOle2Obj* >( ::GetSdrObjectFromXShape( xShape ) ) )
        svt::EmbeddedObjectRef::TryRunningState( pSdrOleObj->GetObjRef() );

    // create the chart substream object
    ScfPropertySet aShapeProp( xShape );
    Reference< XModel > xModel;
    aShapeProp.GetProperty( xModel, CREATE_OUSTRING( "Model" ) );
    ::com::sun::star::awt::Rectangle aBoundRect;
    aShapeProp.GetProperty( aBoundRect, CREATE_OUSTRING( "BoundRect" ) );
    Size aSize( aBoundRect.Width, aBoundRect.Height );
    mxChart.reset( new XclExpChart( rRoot, xModel, aSize ) );
}

XclExpChartObj::~XclExpChartObj()
{
}

void XclExpChartObj::Save( XclExpStream& rStrm )
{
    // content of OBJ record
    XclObj::Save( rStrm );
    // chart substream
    mxChart->Save( rStrm );
}

// ============================================================================

XclExpNote::XclExpNote( const XclExpRoot& rRoot, const ScAddress& rScPos,
        const ScPostIt* pScNote, const String& rAddText ) :
    XclExpRecord( EXC_ID_NOTE ),
    maScPos( rScPos ),
    mnObjId( EXC_OBJ_INVALID_ID ),
    mbVisible( pScNote && pScNote->IsShown() )
{
    // get the main note text
    String aNoteText;
    if( pScNote )
        aNoteText = pScNote->GetText();
    // append additional text
    ScGlobal::AddToken( aNoteText, rAddText, '\n', 2 );

    // initialize record dependent on BIFF type
    switch( rRoot.GetBiff() )
    {
        case EXC_BIFF5:
            maNoteText = ByteString( aNoteText, rRoot.GetTextEncoding() );
        break;

        case EXC_BIFF8:
        {
            ::std::auto_ptr< EditTextObject > xObj;
            Rectangle aRect;
            ::std::auto_ptr< SdrCaptionObj > xCaption;
            ScDocument& rDoc = rRoot.GetDoc();

            // #i79416# create dummy note for cells without note but with content in rAddText
            ::std::auto_ptr< ScPostIt > xDummyNote;
            if( pScNote )
            {
                aRect = pScNote->GetRectangle();
            }
            else
            {
                xDummyNote.reset( new ScPostIt( rAddText, &rDoc ) );
                xDummyNote->SetItemSet( xDummyNote->DefaultItemSet() );
                aRect = xDummyNote->DefaultRectangle( rScPos );
                pScNote = xDummyNote.get();
            }

            // read strings from note object, if present
            if( const EditTextObject* pEditObj = pScNote->GetEditTextObject() )
            {
                xObj.reset( pEditObj->Clone() );
                // append additional text to original note if any
                if( pScNote->GetText() != aNoteText )
                {
                    EditEngine& rEE = rRoot.GetEditEngine();
                    rEE.SetText( aNoteText );
                    ::std::auto_ptr< EditTextObject > xNewTextObj( rEE.CreateTextObject() );
                    xObj->Insert( *xNewTextObj, pEditObj->GetParagraphCount() );
                }
                maAuthor.Assign( pScNote->GetAuthor() );
                const SfxItemSet& rSet = pScNote->GetItemSet();
                Point aDummyTailPos;

                // In order to transform the SfxItemSet to an EscherPropertyContainer
                // and export the properties, we need to recreate the drawing object and
                // pass this to XclObjComment() for processing.
                xCaption.reset( new SdrCaptionObj( aRect, aDummyTailPos ) );

                pScNote->InsertObject( xCaption.get(), rDoc, rScPos.Tab(), sal_True );
                xCaption->SetMergedItemSet( rSet );
            }

            // create the object record
            if( xObj.get() )
                mnObjId = rRoot.GetOldRoot().pObjRecs->Add( new XclObjComment( rRoot, aRect, *xObj, xCaption.get(), mbVisible ) );

            if( pScNote )
                pScNote->RemoveObject( xCaption.get(), rDoc, rScPos.Tab() );

            SetRecSize( 9 + maAuthor.GetSize() );
        }
        break;

        default:    DBG_ERROR_BIFF();
    }
}

void XclExpNote::Save( XclExpStream& rStrm )
{
    switch( rStrm.GetRoot().GetBiff() )
    {
        case EXC_BIFF5:
        {
            // write the NOTE record directly, there may be the need to create more than one
            const sal_Char* pcBuffer = maNoteText.GetBuffer();
            sal_uInt16 nCharsLeft = static_cast< sal_uInt16 >( maNoteText.Len() );

            while( nCharsLeft )
            {
                sal_uInt16 nWriteChars = ::std::min( nCharsLeft, EXC_NOTE5_MAXLEN );

                rStrm.StartRecord( EXC_ID_NOTE, 6 + nWriteChars );
                if( pcBuffer == maNoteText.GetBuffer() )
                {
                    // first record: row, col, length of complete text
                    rStrm   << static_cast< sal_uInt16 >( maScPos.Row() )
                            << static_cast< sal_uInt16 >( maScPos.Col() )
                            << nCharsLeft;  // still contains full length
                }
                else
                {
                    // next records: -1, 0, length of current text segment
                    rStrm   << sal_uInt16( 0xFFFF )
                            << sal_uInt16( 0 )
                            << nWriteChars;
                }
                rStrm.Write( pcBuffer, nWriteChars );
                rStrm.EndRecord();

                pcBuffer += nWriteChars;
                nCharsLeft = nCharsLeft - nWriteChars;
            }
        }
        break;

        case EXC_BIFF8:
            if( mnObjId != EXC_OBJ_INVALID_ID )
                XclExpRecord::Save( rStrm );
        break;

        default:    DBG_ERROR_BIFF();
    }
}

void XclExpNote::WriteBody( XclExpStream& rStrm )
{
    // BIFF5/BIFF7 is written separately
    DBG_ASSERT_BIFF( rStrm.GetRoot().GetBiff() == EXC_BIFF8 );

    sal_uInt16 nFlags = 0;
    ::set_flag( nFlags, EXC_NOTE_VISIBLE, mbVisible );

    rStrm   << static_cast< sal_uInt16 >( maScPos.Row() )
            << static_cast< sal_uInt16 >( maScPos.Col() )
            << nFlags
            << mnObjId
            << maAuthor
            << sal_uInt8( 0 );
}

// ============================================================================

