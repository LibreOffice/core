/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_filter.hxx"

#include <com/sun/star/uno/Any.h>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/form/XFormsSupplier.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/form/binding/XBindableValue.hpp>
#include <com/sun/star/form/binding/XValueBinding.hpp>
#include <com/sun/star/form/binding/XListEntrySink.hpp>
#include <com/sun/star/form/binding/XListEntrySource.hpp>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/ScrollBarOrientation.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <comphelper/extract.hxx>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/io/XInputStreamProvider.hpp>
#include <rtl/ustrbuf.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/objsh.hxx>
#include <xmlscript/xmldlg_imexp.hxx>
#include <filter/msfilter/msocximex.hxx>
#include <osl/file.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <comphelper/processfactory.hxx> // shouldn't be needed
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <algorithm>
#include <memory>
#include <com/sun/star/graphic/GraphicObject.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <comphelper/componentcontext.hxx>
#include <unotools/streamwrap.hxx>
#include <sal/macros.h>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/sheet/XSpreadsheetView.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XCellRangeReferrer.hpp>
// #TODO remove this when oox is used for control/userform import
#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/awt/XUnitConversion.hpp>

#ifndef C2U
#define C2U(cChar)  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(cChar))
#endif

using namespace ::com::sun::star;
using namespace ::rtl;
using namespace cppu;

static char sWW8_form[] = "WW-Standard";

sal_uInt8 const OCX_Control::aObjInfo[4] = { 0x00, 0x12, 0x03, 0x00 };

long WriteAlign(SvStorageStream *pS, int nAmount)
{
    if (long nAlign = pS->Tell() % nAmount)
    {
        long nLen = nAmount - nAlign;
        for (long i=0; i< nLen; ++i)
            *pS << sal_uInt8(0x00);
        return nLen;
    }
    return 0;
}
// string import/export =======================================================
/** #117832#  import of form control names
* control name is located in stream ("\3OCXNAME")
* a strings in "\3OCXNAME" stream seem to be terminated by 4 trailing bytes of 0's.
*                              ====
* Note: If the string in the stream is overwritten by a shorter string
* some characters from the original string may remain, the new string however
* will still be terminated in the same way e.g. by 4 bytes with value 0.
*/

bool writeOCXNAME( const OUString& sOCXName, SvStorageStream* pStream )
{
    const sal_Unicode* buffer = sOCXName.getStr();
    for ( sal_Int32 index=0; index < sOCXName.getLength(); index++ )
    {
        sal_uInt16 ch = static_cast< sal_uInt16 >( buffer[ index ] );
        *pStream << ch;
    }
    // write
    *pStream << sal_uInt32(0);
    return ( SVSTREAM_OK == pStream->GetError() );

}

namespace {

const sal_uInt32 SVX_MSOCX_SIZEMASK     = 0x7FFFFFFF;   /// Mask for character buffer size.
const sal_uInt32 SVX_MSOCX_COMPRESSED   = 0x80000000;   /// 1 = compressed Unicode array.


/** Returns true, if the passed length field specifies a compressed character array.
 */
inline bool lclIsCompressed( sal_uInt32 nLenFld )
{
    return (nLenFld & SVX_MSOCX_COMPRESSED) != 0;
}


/** Extracts and returns the memory size of the character buffer.
    @return  Character buffer size (may differ from resulting string length!).
 */
inline sal_uInt32 lclGetBufferSize( sal_uInt32 nLenFld )
{
    return nLenFld & SVX_MSOCX_SIZEMASK;
}

// export ---------------------------------------------------------------------

/** This class implements writing a character array from a Unicode string.

    Usage:
    1)  Construct an instance, either directly with an OUString, or with an UNO
        Any containing an OUString.
    2)  Check with HasData(), if there is something to write.
    3)  Write the string length field with WriteLenField() at the right place.
    4)  Write the encoded character array with WriteCharArray().
 */
class SvxOcxString
{
public:
    /** Constructs an empty string. String data may be set later by assignment. */
    inline explicit             SvxOcxString() : mnLenFld( 0 ) {}
    /** Constructs the string from the passed OUString. */
    inline explicit             SvxOcxString( const OUString& rStr ) { Init( rStr ); }
    /** Constructs the string from the passed UNO Any. */
    inline explicit             SvxOcxString( const uno::Any& rAny ) { Init( rAny ); }

    /** Assigns the passed string to the object. */
    inline SvxOcxString&        operator=( const OUString& rStr ) { Init( rStr ); return *this; }
    /** Assigns the string in the passed UNO Any to the object. */
    inline SvxOcxString&        operator=( const uno::Any& rAny ) { Init( rAny ); return *this; }

    /** Returns true, if the string contains at least one character to write. */
    inline bool                 HasData() const { return maString.getLength() > 0; }

    /** Writes the encoded 32-bit string length field. Aligns stream position to mult. of 4 before. */
    void                        WriteLenField( SvStorageStream& rStrm ) const;
    /** Writes the encoded character array. Aligns stream position to mult. of 4 before. */
    void                        WriteCharArray( SvStorageStream& rStrm ) const;

private:
    inline void                 Init( const OUString& rStr ) { maString = rStr; Init(); }
    void                        Init( const uno::Any& rAny );
    void                        Init();

    OUString                    maString;       /// The initial string data.
    sal_uInt32                  mnLenFld;       /// The encoded string length field.
};

void SvxOcxString::Init( const uno::Any& rAny )
{
    if( !(rAny >>= maString) )
        maString = OUString();
    Init();
}

void SvxOcxString::Init()
{
    mnLenFld = static_cast< sal_uInt32 >( maString.getLength() );
    bool bCompr = true;
    // try to find a character >= 0x100 -> character array will be stored uncompressed then
    if( const sal_Unicode* pChar = maString.getStr() )
        for( const sal_Unicode* pEnd = pChar + maString.getLength(); bCompr && (pChar < pEnd); ++pChar )
            bCompr = (*pChar < 0x100);
    if( bCompr )
        mnLenFld |= SVX_MSOCX_COMPRESSED;
    else
        mnLenFld *= 2;
}

void SvxOcxString::WriteLenField( SvStorageStream& rStrm ) const
{
    if( HasData() )
    {
        WriteAlign( &rStrm, 4);
        rStrm << mnLenFld;
    }
}

void SvxOcxString::WriteCharArray( SvStorageStream& rStrm ) const
{
    if( HasData() )
    {
        const sal_Unicode* pChar = maString.getStr();
        const sal_Unicode* pEnd = pChar + maString.getLength();
        bool bCompr = lclIsCompressed( mnLenFld );

        WriteAlign( &rStrm, 4);
        for( ; pChar < pEnd; ++pChar )
        {
            // write compressed Unicode (not encoded bytestring), or Little-Endian Unicode
            rStrm << static_cast< sal_uInt8 >( *pChar );
            if( !bCompr )
                rStrm << static_cast< sal_uInt8 >( *pChar >> 8 );
        }
    }
}

} // namespace

// ============================================================================

class OCX_UserFormLabel : public OCX_Label
{
public:
    OCX_UserFormLabel(OCX_Control* pParent ) : OCX_Label( pParent )
    {
        mnForeColor = 0x80000012L;
        mnBackColor = 0x8000000FL;
    }
};


sal_uInt16 OCX_Control::nStandardId(0x0200);
sal_uInt16 OCX_FontData::nStandardId(0x0200);

sal_uInt32 OCX_Control::pColor[25] = {
0xC0C0C0, 0x008080, 0x000080, 0x808080, 0xC0C0C0, 0xFFFFFF, 0x000000,
0x000000, 0x000000, 0xFFFFFF, 0xC0C0C0, 0xC0C0C0, 0x808080, 0x000080,
0xFFFFFF, 0xC0C0C0, 0x808080, 0x808080, 0x000000, 0xC0C0C0, 0xFFFFFF,
0x000000, 0xC0C0C0, 0x000000, 0xFFFFC0 };

void OCX_Control::FillSystemColors()
{
    // overwrite the predefined colors with available system colors
    const StyleSettings& rSett = Application::GetSettings().GetStyleSettings();

    pColor[ 0x00 ] = rSett.GetFaceColor().GetColor();
    pColor[ 0x01 ] = rSett.GetWorkspaceColor().GetColor();
    pColor[ 0x02 ] = rSett.GetActiveColor().GetColor();
    pColor[ 0x03 ] = rSett.GetDeactiveColor().GetColor();
    pColor[ 0x04 ] = rSett.GetMenuBarColor().GetColor();
    pColor[ 0x05 ] = rSett.GetWindowColor().GetColor();
    pColor[ 0x07 ] = rSett.GetMenuTextColor().GetColor();
    pColor[ 0x08 ] = rSett.GetWindowTextColor().GetColor();
    pColor[ 0x09 ] = rSett.GetActiveTextColor().GetColor();
    pColor[ 0x0A ] = rSett.GetActiveBorderColor().GetColor();
    pColor[ 0x0B ] = rSett.GetDeactiveBorderColor().GetColor();
    pColor[ 0x0C ] = rSett.GetWorkspaceColor().GetColor();
    pColor[ 0x0D ] = rSett.GetHighlightColor().GetColor();
    pColor[ 0x0E ] = rSett.GetHighlightTextColor().GetColor();
    pColor[ 0x0F ] = rSett.GetFaceColor().GetColor();
    pColor[ 0x10 ] = rSett.GetShadowColor().GetColor();
    pColor[ 0x12 ] = rSett.GetButtonTextColor().GetColor();
    pColor[ 0x13 ] = rSett.GetDeactiveTextColor().GetColor();
    pColor[ 0x14 ] = rSett.GetHighlightColor().GetColor();
    pColor[ 0x15 ] = rSett.GetDarkShadowColor().GetColor();
    pColor[ 0x16 ] = rSett.GetShadowColor().GetColor();
    pColor[ 0x17 ] = rSett.GetHelpTextColor().GetColor();
    pColor[ 0x18 ] = rSett.GetHelpColor().GetColor();
}

sal_uInt8 OCX_FontData::ExportAlign(sal_Int16 nAlign) const
{
    sal_Int8 nRet;
    switch (nAlign)
    {
    default:
    case 0:
        nRet = 1;
        break;
    case 2:
        nRet = 2;
        break;
    case 1:
        nRet = 3;
        break;
    }
    return nRet;
}

sal_uInt32 OCX_Control::SwapColor(sal_uInt32 nColor) const
{
    sal_uInt8
        r(static_cast<sal_uInt8>(nColor&0xFF)),
        g(static_cast<sal_uInt8>(((nColor)>>8)&0xFF)),
        b(static_cast<sal_uInt8>((nColor>>16)&0xFF));
    nColor = (r<<16) + (g<<8) + b;
    return nColor;
}

sal_uInt32 OCX_Control::ExportColor(sal_uInt32 nColor) const
{
    sal_uInt8 nUpper = (sal_uInt8)( nColor >> 24 );
    if (nUpper & 0x80) //Palette color, should be switch on bottom 24 bits
    {
        /*Might as well use my systems ones in the absence of any other ideas*/
        nColor = nColor&0x00FFFFFF;
        DBG_ASSERT (nColor <= 24,"Unknown Palette Index");
        if (nColor > 24)
            nColor = 0xFFFFFF;
        else
            nColor = pColor[nColor];
    }

    //Stored in bgr! rather than rgb
    nColor = SwapColor(nColor);
    return nColor;
}

sal_uInt8 OCX_Control::ExportBorder(sal_uInt16 nBorder,sal_uInt8 &rBorderStyle)
    const
{
    sal_uInt8 nRet;
    switch(nBorder)
    {
        case 0:
            nRet = rBorderStyle = 0;
            break;
        default:
        case 1:
            nRet = 2;
            rBorderStyle = 0;
            break;
        case 2:
            nRet = 0;
            rBorderStyle = 1;
            break;
    }
    return nRet;
}

sal_uInt8 OCX_Control::ExportSpecEffect( sal_Int16 nApiEffect ) const
{
    return (nApiEffect == 2) ? 0 : 2;
}

const uno::Reference< drawing::XDrawPage >&
    SvxMSConvertOCXControls::GetDrawPage()
{
    if( !xDrawPage.is() && pDocSh )
    {
        uno::Reference< drawing::XDrawPageSupplier > xTxtDoc(pDocSh->GetModel(),
            uno::UNO_QUERY);
        DBG_ASSERT(xTxtDoc.is(),"XDrawPageSupplier nicht vom XModel erhalten");
        xDrawPage = xTxtDoc->getDrawPage();
        DBG_ASSERT( xDrawPage.is(), "XDrawPage nicht erhalten" );
    }

    return xDrawPage;
}


const uno::Reference< lang::XMultiServiceFactory >&
    SvxMSConvertOCXControls::GetServiceFactory()
{
    if( !xServiceFactory.is() && pDocSh )
    {
        xServiceFactory = uno::Reference< lang::XMultiServiceFactory >
            (pDocSh->GetBaseModel(), uno::UNO_QUERY);
        DBG_ASSERT( xServiceFactory.is(),
                "XMultiServiceFactory nicht vom Model erhalten" );
    }

    return xServiceFactory;
}

const uno::Reference< drawing::XShapes >& SvxMSConvertOCXControls::GetShapes()
{
    if( !xShapes.is() )
    {
        GetDrawPage();
        if( xDrawPage.is() )
        {

            xShapes = uno::Reference< drawing::XShapes >(xDrawPage,
                uno::UNO_QUERY);
            DBG_ASSERT( xShapes.is(), "XShapes nicht vom XDrawPage erhalten" );
        }
    }
    return xShapes;
}

const uno::Reference< container::XIndexContainer >&
    SvxMSConvertOCXControls::GetFormComps()
{
    if( !xFormComps.is() )
    {
        GetDrawPage();
        if( xDrawPage.is() )
        {
            uno::Reference< form::XFormsSupplier > xFormsSupplier( xDrawPage,
                uno::UNO_QUERY );
            DBG_ASSERT( xFormsSupplier.is(),
                    "XFormsSupplier nicht vom XDrawPage erhalten" );

            uno::Reference< container::XNameContainer >  xNameCont =
                xFormsSupplier->getForms();

            // Das Formular bekommt einen Namen wie "WW-Standard[n]" und
            // wird in jedem Fall neu angelegt.
            UniString sName( sWW8_form, RTL_TEXTENCODING_MS_1252 );
            sal_uInt16 n = 0;

            while( xNameCont->hasByName( sName ) )
            {
                sName.AssignAscii( sWW8_form );
                sName += String::CreateFromInt32( ++n );
            }

            const uno::Reference< lang::XMultiServiceFactory > &rServiceFactory
                = GetServiceFactory();
            if( !rServiceFactory.is() )
                return xFormComps;

            uno::Reference< uno::XInterface >  xCreate =
                rServiceFactory->createInstance(C2U(
                    "com.sun.star.form.component.Form"));
            if( xCreate.is() )
            {
                uno::Reference< beans::XPropertySet > xFormPropSet( xCreate,
                    uno::UNO_QUERY );

                uno::Any aTmp(&sName,getCppuType((OUString *)0));
                xFormPropSet->setPropertyValue( C2U("Name"), aTmp );

                uno::Reference< form::XForm > xForm( xCreate, uno::UNO_QUERY );
                DBG_ASSERT(xForm.is(), "keine Form?");

                uno::Reference< container::XIndexContainer > xForms( xNameCont,
                    uno::UNO_QUERY );
                DBG_ASSERT( xForms.is(), "XForms nicht erhalten" );

                aTmp.setValue( &xForm,
                    ::getCppuType((uno::Reference < form::XForm >*)0));
                xForms->insertByIndex( xForms->getCount(), aTmp );

                xFormComps = uno::Reference< container::XIndexContainer >
                    (xCreate, uno::UNO_QUERY);
            }
        }
    }

    return xFormComps;
}

sal_Bool OCX_GroupBox::Export(SvStorageRef& /* rObj */,
    const uno::Reference< beans::XPropertySet >& /* rPropSet */,
    const awt::Size& /* rSize */ )
{
    sal_Bool bRet=sal_True;
    return bRet;
}

sal_Bool OCX_GroupBox::WriteContents(SvStorageStreamRef& /* rObj */,
    const uno::Reference< beans::XPropertySet >& /* rPropSet */,
    const awt::Size& /* rSize */)
{
    sal_Bool bRet=sal_True;
    return bRet;
}

sal_Bool OCX_CommandButton::WriteContents(SvStorageStreamRef& rContents,
    const uno::Reference< beans::XPropertySet >& rPropSet,
    const awt::Size& rSize )
{
    sal_Bool bRet=sal_True;

    sal_uInt32 nOldPos = rContents->Tell();
    rContents->SeekRel(8);

    uno::Any aTmp = rPropSet->getPropertyValue(C2U("TextColor"));
    if (aTmp.hasValue())
        aTmp >>= mnForeColor;
    *rContents << ExportColor(mnForeColor);

    aTmp = rPropSet->getPropertyValue(C2U("BackgroundColor"));
    if (aTmp.hasValue())
        aTmp >>= mnBackColor;
    *rContents << ExportColor(mnBackColor);

    aTmp = rPropSet->getPropertyValue(C2U("Enabled"));
    fEnabled = any2bool(aTmp);
    sal_uInt8 nTemp=0;//fEnabled;
    if (fEnabled)
        nTemp |= 0x02;
    if (fBackStyle)
        nTemp |= 0x08;
    *rContents << nTemp;
    *rContents << sal_uInt8(0x00);

    nTemp = 0;
    aTmp = rPropSet->getPropertyValue(C2U("MultiLine"));
    fWordWrap = any2bool(aTmp);
    if (fWordWrap)
        nTemp |= 0x80;
    *rContents << nTemp;
    *rContents << sal_uInt8(0x00);

    SvxOcxString aCaption( rPropSet->getPropertyValue(C2U("Label")) );
    aCaption.WriteLenField( *rContents );
    aCaption.WriteCharArray( *rContents );

    WriteAlign(rContents,4);

    *rContents << rSize.Width;
    *rContents << rSize.Height;

    // "take focus on click" is directly in content flags, not in option field...
    mbTakeFocus = any2bool( rPropSet->getPropertyValue( C2U( "FocusOnClick" ) ) );

    nFixedAreaLen = static_cast<sal_uInt16>(rContents->Tell()-nOldPos-4);

    bRet = aFontData.Export(rContents,rPropSet);

    rContents->Seek(nOldPos);
    *rContents << nStandardId;
    *rContents << nFixedAreaLen;

    sal_uInt8 nTmp = 0x27;
    if (aCaption.HasData())
        nTmp |= 0x08;
    *rContents << nTmp;
    nTmp = 0x00;
    if( !mbTakeFocus )  // flag is set, if option is off
        nTmp |= 0x02;
    *rContents << nTmp;
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);

    DBG_ASSERT((rContents.Is() && (SVSTREAM_OK==rContents->GetError())),"damn");
    return bRet;
}



sal_Bool OCX_CommandButton::Export(SvStorageRef &rObj,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    static sal_uInt8 const aCompObj[] = {
            0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
            0xFF, 0xFF, 0xFF, 0xFF, 0x40, 0x32, 0x05, 0xD7,
            0x69, 0xCE, 0xCD, 0x11, 0xA7, 0x77, 0x00, 0xDD,
            0x01, 0x14, 0x3C, 0x57, 0x22, 0x00, 0x00, 0x00,
            0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
            0x74, 0x20, 0x46, 0x6F, 0x72, 0x6d, 0x73, 0x20,
            0x32, 0x2e, 0x30, 0x20, 0x43, 0x6F, 0x6D, 0x6D,
            0x61, 0x6E, 0x64, 0x42, 0x75, 0x74, 0x74, 0x6F,
            0x6E, 0x00, 0x10, 0x00, 0x00, 0x00, 0x45, 0x6D,
            0x62, 0x65, 0x64, 0x64, 0x65, 0x64, 0x20, 0x4F,
            0x62, 0x6A, 0x65, 0x63, 0x74, 0x00, 0x16, 0x00,
            0x00, 0x00, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x2E,
            0x43, 0x6F, 0x6D, 0x6D, 0x61, 0x6E, 0x64, 0x42,
            0x75, 0x74, 0x74, 0x6F, 0x6E, 0x2E, 0x31, 0x00,
            0xF4, 0x39, 0xB2, 0x71, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor( rObj->OpenSotStream( C2U("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenSotStream( C2U("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 const aOCXNAME[] = {
        0x43, 0x00, 0x6F, 0x00, 0x6D, 0x00, 0x6D, 0x00,
        0x61, 0x00, 0x6E, 0x00, 0x64, 0x00, 0x42, 0x00,
        0x75, 0x00, 0x74, 0x00, 0x74, 0x00, 0x6F, 0x00,
        0x6E, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    {
    SvStorageStreamRef xStor2( rObj->OpenSotStream( C2U("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenSotStream( C2U("contents")));

    return WriteContents(xContents,rPropSet,rSize);
}

sal_Bool OCX_ImageButton::WriteContents(SvStorageStreamRef &rContents,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    sal_Bool bRet=sal_True;

    sal_uInt32 nOldPos = rContents->Tell();
    rContents->SeekRel(8);

    uno::Any aTmp=rPropSet->getPropertyValue(C2U("BackgroundColor"));
    if (aTmp.hasValue())
        aTmp >>= mnBackColor;
    *rContents << ExportColor(mnBackColor);

    aTmp = rPropSet->getPropertyValue(C2U("Enabled"));
    fEnabled = any2bool(aTmp);
    sal_uInt8 nTemp=0;//fEnabled;
    if (fEnabled)
        nTemp |= 0x02;
    *rContents << nTemp;
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);

    WriteAlign(rContents,4);

    *rContents << rSize.Width;
    *rContents << rSize.Height;

    nFixedAreaLen = static_cast<sal_uInt16>(rContents->Tell()-nOldPos-4);

    bRet = aFontData.Export(rContents,rPropSet);

    rContents->Seek(nOldPos);
    *rContents << nStandardId;
    *rContents << nFixedAreaLen;

    sal_uInt8 nTmp = 0x26;
    *rContents << nTmp;
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);

    DBG_ASSERT((rContents.Is() && (SVSTREAM_OK==rContents->GetError())),"damn");
    return bRet;
}



sal_Bool OCX_ImageButton::Export(SvStorageRef &rObj,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    static sal_uInt8 const aCompObj[] = {
            0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
            0xFF, 0xFF, 0xFF, 0xFF, 0x40, 0x32, 0x05, 0xD7,
            0x69, 0xCE, 0xCD, 0x11, 0xA7, 0x77, 0x00, 0xDD,
            0x01, 0x14, 0x3C, 0x57, 0x22, 0x00, 0x00, 0x00,
            0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
            0x74, 0x20, 0x46, 0x6F, 0x72, 0x6d, 0x73, 0x20,
            0x32, 0x2e, 0x30, 0x20, 0x43, 0x6F, 0x6D, 0x6D,
            0x61, 0x6E, 0x64, 0x42, 0x75, 0x74, 0x74, 0x6F,
            0x6E, 0x00, 0x10, 0x00, 0x00, 0x00, 0x45, 0x6D,
            0x62, 0x65, 0x64, 0x64, 0x65, 0x64, 0x20, 0x4F,
            0x62, 0x6A, 0x65, 0x63, 0x74, 0x00, 0x16, 0x00,
            0x00, 0x00, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x2E,
            0x43, 0x6F, 0x6D, 0x6D, 0x61, 0x6E, 0x64, 0x42,
            0x75, 0x74, 0x74, 0x6F, 0x6E, 0x2E, 0x31, 0x00,
            0xF4, 0x39, 0xB2, 0x71, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor( rObj->OpenSotStream( C2U("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenSotStream( C2U("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 const aOCXNAME[] = {
        0x43, 0x00, 0x6F, 0x00, 0x6D, 0x00, 0x6D, 0x00,
        0x61, 0x00, 0x6E, 0x00, 0x64, 0x00, 0x42, 0x00,
        0x75, 0x00, 0x74, 0x00, 0x74, 0x00, 0x6F, 0x00,
        0x6E, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    {
    SvStorageStreamRef xStor2( rObj->OpenSotStream( C2U("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenSotStream( C2U("contents")));
    return WriteContents(xContents,rPropSet,rSize);
}

bool lcl_isNamedRange( const rtl::OUString& sAddress, uno::Reference< frame::XModel >& xModel, table::CellRangeAddress& aAddress )
{
    bool bRes = false;
    const static rtl::OUString sNamedRanges( RTL_CONSTASCII_USTRINGPARAM("NamedRanges"));
    uno::Reference< sheet::XCellRangeReferrer > xReferrer;
    try
    {
        uno::Reference< beans::XPropertySet > xPropSet( xModel, uno::UNO_QUERY_THROW );
        uno::Reference< container::XNameAccess > xNamed( xPropSet->getPropertyValue( sNamedRanges ), uno::UNO_QUERY_THROW );
        xReferrer.set ( xNamed->getByName( sAddress ), uno::UNO_QUERY );
    }
    catch( uno::Exception& /*e*/ )
    {
        // do nothing
    }
    if ( xReferrer.is() )
    {
        uno::Reference< sheet::XCellRangeAddressable > xRangeAddressable( xReferrer->getReferredCells(), uno::UNO_QUERY );
        if ( xRangeAddressable.is() )
        {
            aAddress = xRangeAddressable->getRangeAddress();
            bRes = true;
        }
    }
    return bRes;
}

void lcl_ApplyListSourceAndBindableStuff( uno::Reference< frame::XModel >& xModel, const uno::Reference< beans::XPropertySet >& rPropSet, const rtl::OUString& rsCtrlSource, const rtl::OUString& rsRowSource )
{
// XBindable etc.
    uno::Reference< lang::XMultiServiceFactory > xFac;
    if ( xModel.is() )
        xFac.set( xModel, uno::UNO_QUERY );
    uno::Reference< form::binding::XBindableValue > xBindable( rPropSet, uno::UNO_QUERY );
    if (  xFac.is() && rsCtrlSource.getLength() && xBindable.is() )
    {

         // OOo address structures
         // RefCell - convert from XL
         // pretend we converted the imported string address into the
         // appropriate address structure
         uno::Reference< beans::XPropertySet > xConvertor( xFac->createInstance( C2U( "com.sun.star.table.CellAddressConversion" )), uno::UNO_QUERY );
         table::CellAddress aAddress;
         if ( xConvertor.is() )
         {
             // we need this service to properly convert XL notation also
             // Should be easy to extend
             xConvertor->setPropertyValue( C2U( "XL_A1_Representation" ), uno::makeAny( rsCtrlSource ) );
             xConvertor->getPropertyValue( C2U( "Address" ) ) >>= aAddress;
         }

         beans::NamedValue aArg1;
         aArg1.Name = C2U("BoundCell");
         aArg1.Value <<= aAddress;

         uno::Sequence< uno::Any > aArgs(1);
         aArgs[ 0 ]  <<= aArg1;

         uno::Reference< form::binding::XValueBinding > xBinding( xFac->createInstanceWithArguments( C2U("com.sun.star.table.CellValueBinding" ), aArgs ), uno::UNO_QUERY );
         xBindable->setValueBinding( xBinding );
    }
    uno::Reference< form::binding::XListEntrySink > xListEntrySink( rPropSet, uno::UNO_QUERY );
    if (  xFac.is() && rsRowSource.getLength() && xListEntrySink.is() )
    {

         // OOo address structures
         // RefCell - convert from XL
         // pretend we converted the imported string address into the
         // appropriate address structure
         uno::Reference< beans::XPropertySet > xConvertor( xFac->createInstance( C2U( "com.sun.star.table.CellRangeAddressConversion" )), uno::UNO_QUERY );
         table::CellRangeAddress aAddress;
         if ( xConvertor.is() )
         {
             if ( !lcl_isNamedRange( rsRowSource, xModel, aAddress ) )
             {
                 // we need this service to properly convert XL notation also
                 // Should be easy to extend
                 xConvertor->setPropertyValue( C2U( "XL_A1_Representation" ), uno::makeAny( rsRowSource ) );
                 xConvertor->getPropertyValue( C2U( "Address" ) ) >>= aAddress;
             }
         }

         beans::NamedValue aArg1;
         aArg1.Name = C2U("CellRange");
         aArg1.Value <<= aAddress;

         uno::Sequence< uno::Any > aArgs(1);
         aArgs[ 0 ]  <<= aArg1;

         uno::Reference< form::binding::XListEntrySource > xSource( xFac->createInstanceWithArguments( C2U("com.sun.star.table.CellRangeListSource" ), aArgs ), uno::UNO_QUERY );
         xListEntrySink->setListEntrySource( xSource );
    }
}

sal_Bool OCX_OptionButton::WriteContents(SvStorageStreamRef &rContents,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    sal_Bool bRet=sal_True;

    sal_uInt32 nOldPos = rContents->Tell();
    rContents->SeekRel(12);

    pBlockFlags[0] = 0;
    pBlockFlags[1] = 0x01;
    pBlockFlags[2] = 0;
    pBlockFlags[3] = 0x80;
    pBlockFlags[4] = 0;
    pBlockFlags[5] = 0;
    pBlockFlags[6] = 0;
    pBlockFlags[7] = 0;

    uno::Any aTmp = rPropSet->getPropertyValue(C2U("Enabled"));
    fEnabled = any2bool(aTmp);

    aTmp = rPropSet->getPropertyValue(C2U("BackgroundColor"));
    if (aTmp.hasValue())
        aTmp >>= mnBackColor;
    else
        fBackStyle = 0;

    sal_uInt8 nTemp=0;//=fEnabled;
    if (fEnabled)
        nTemp |= 0x02;
    if (fBackStyle)
        nTemp |= 0x08;
    *rContents << nTemp;
    pBlockFlags[0] |= 0x01;
    *rContents << sal_uInt8(0x00);
    nTemp = 0;
    aTmp = rPropSet->getPropertyValue(C2U("MultiLine"));
    fWordWrap = any2bool(aTmp);
    if (fWordWrap)
        nTemp |= 0x80;
    *rContents << nTemp;
    *rContents << sal_uInt8(0x00);

    *rContents << ExportColor(mnBackColor);
    pBlockFlags[0] |= 0x02;

    aTmp = rPropSet->getPropertyValue(C2U("TextColor"));
    if (aTmp.hasValue())
        aTmp >>= mnForeColor;
    *rContents << ExportColor(mnForeColor);
    pBlockFlags[0] |= 0x04;

    nStyle = 5;
    *rContents << nStyle;
    pBlockFlags[0] |= 0x40;

    WriteAlign(rContents,4);
    nValueLen = 1|SVX_MSOCX_COMPRESSED;
    aTmp = rPropSet->getPropertyValue(C2U("DefaultState"));
    sal_Int16 nDefault = sal_Int16();
    aTmp >>= nDefault;
    *rContents << nValueLen;
    pBlockFlags[2] |= 0x40;


    SvxOcxString aCaption( rPropSet->getPropertyValue(C2U("Label")) );
    if (aCaption.HasData())
        pBlockFlags[2] |= 0x80;
    aCaption.WriteLenField( *rContents );

    aTmp = rPropSet->getPropertyValue(C2U("VisualEffect"));
    if (aTmp.hasValue())
    {
        sal_Int16 nApiSpecEffect = sal_Int16();
        aTmp >>= nApiSpecEffect;
        nSpecialEffect = ExportSpecEffect( nApiSpecEffect );
    }
    *rContents << nSpecialEffect;
    pBlockFlags[3] |= 0x04;

    WriteAlign(rContents,4);
    *rContents << rSize.Width;
    *rContents << rSize.Height;

    nDefault += 0x30;
    *rContents << sal_uInt8(nDefault);
    *rContents << sal_uInt8(0x00);

    aCaption.WriteCharArray( *rContents );

    WriteAlign(rContents,4);
    nFixedAreaLen = static_cast<sal_uInt16>(rContents->Tell()-nOldPos-4);
    bRet = aFontData.Export(rContents,rPropSet);

    rContents->Seek(nOldPos);
    *rContents << nStandardId;
    *rContents << nFixedAreaLen;

    *rContents << pBlockFlags[0];
    *rContents << pBlockFlags[1];
    *rContents << pBlockFlags[2];
    *rContents << pBlockFlags[3];
    *rContents << pBlockFlags[4];
    *rContents << pBlockFlags[5];
    *rContents << pBlockFlags[6];
    *rContents << pBlockFlags[7];

    DBG_ASSERT((rContents.Is() &&
        (SVSTREAM_OK==rContents->GetError())),"damn");
    return bRet;
}



sal_Bool OCX_OptionButton::Export(SvStorageRef &rObj,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    static sal_uInt8 const aCompObj[] = {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0x50, 0x1D, 0xD2, 0x8B,
        0x42, 0xEC, 0xCE, 0x11, 0x9E, 0x0D, 0x00, 0xAA,
        0x00, 0x60, 0x02, 0xF3, 0x21, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
        0x32, 0x2E, 0x30, 0x20, 0x4F, 0x70, 0x74, 0x69,
        0x6F, 0x6E, 0x42, 0x75, 0x74, 0x74, 0x6F, 0x6E,
        0x00, 0x10, 0x00, 0x00, 0x00, 0x45, 0x6D, 0x62,
        0x65, 0x64, 0x64, 0x65, 0x64, 0x20, 0x4F, 0x62,
        0x6A, 0x65, 0x63, 0x74, 0x00, 0x15, 0x00, 0x00,
        0x00, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x2E, 0x4F,
        0x70, 0x74, 0x69, 0x6F, 0x6E, 0x42, 0x75, 0x74,
        0x74, 0x6F, 0x6E, 0x2E, 0x31, 0x00, 0xF4, 0x39,
        0xB2, 0x71, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor( rObj->OpenSotStream( C2U("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenSotStream( C2U("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 const aOCXNAME[] = {
            0x4F, 0x00, 0x70, 0x00, 0x74, 0x00, 0x69, 0x00,
            0x6F, 0x00, 0x6E, 0x00, 0x42, 0x00, 0x75, 0x00,
            0x74, 0x00, 0x74, 0x00, 0x6F, 0x00, 0x6E, 0x00,
            0x31, 0x00, 0x00, 0x00, 0x00, 0x00
            };

    {
    SvStorageStreamRef xStor2( rObj->OpenSotStream( C2U("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenSotStream( C2U("contents")));
    return WriteContents(xContents, rPropSet, rSize);
}

sal_Bool OCX_TextBox::WriteContents(SvStorageStreamRef &rContents,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    sal_Bool bRet=sal_True;
    sal_uInt32 nOldPos = rContents->Tell();
    rContents->SeekRel(12);

    pBlockFlags[0] = 0;
    pBlockFlags[1] = 0x01;
    pBlockFlags[2] = 0x00;
    pBlockFlags[3] = 0x80;
    pBlockFlags[4] = 0;
    pBlockFlags[5] = 0;
    pBlockFlags[6] = 0;
    pBlockFlags[7] = 0;


    sal_uInt8 nTemp=0x19;
    uno::Any aTmp = rPropSet->getPropertyValue(C2U("Enabled"));
    fEnabled = any2bool(aTmp);
    if (fEnabled)
        nTemp |= 0x02;

    aTmp = rPropSet->getPropertyValue(C2U("ReadOnly"));
    fLocked = any2bool(aTmp);
    if (fLocked)
        nTemp |= 0x04;

    *rContents << nTemp;
    pBlockFlags[0] |= 0x01;
    *rContents << sal_uInt8(0x48);
    *rContents << sal_uInt8(0x80);

    fMultiLine = any2bool(rPropSet->getPropertyValue(C2U("MultiLine")));
    fHideSelection = any2bool(rPropSet->getPropertyValue(C2U("HideInactiveSelection")));
    nTemp = 0x0C;
    if (fMultiLine)
        nTemp |= 0x80;
    if( fHideSelection )
        nTemp |= 0x20;
    *rContents << nTemp;

    aTmp = rPropSet->getPropertyValue(C2U("BackgroundColor"));
    if (aTmp.hasValue())
        aTmp >>= mnBackColor;
    *rContents << ExportColor(mnBackColor);
    pBlockFlags[0] |= 0x02;

    aTmp = rPropSet->getPropertyValue(C2U("TextColor"));
    if (aTmp.hasValue())
        aTmp >>= mnForeColor;
    *rContents << ExportColor(mnForeColor);
    pBlockFlags[0] |= 0x04;

    aTmp = rPropSet->getPropertyValue( C2U("MaxTextLen"));
    aTmp >>= nMaxLength;
    *rContents << nMaxLength;
    pBlockFlags[0] |= 0x08;

    aTmp = rPropSet->getPropertyValue(C2U("Border"));
    sal_Int16 nBorder = sal_Int16();
    aTmp >>= nBorder;
    nSpecialEffect = ExportBorder(nBorder,nBorderStyle);
    *rContents << nBorderStyle;
    pBlockFlags[0] |= 0x10;

    aTmp = rPropSet->getPropertyValue( C2U("HScroll"));
    sal_Bool bTemp1 = any2bool(aTmp);
    aTmp = rPropSet->getPropertyValue( C2U("VScroll"));
    sal_Bool bTemp2 = any2bool(aTmp);
    if (!bTemp1 && !bTemp2)
        nScrollBars =0;
    else if (bTemp1 && bTemp2)
        nScrollBars = 3;
    else if (!bTemp1 && bTemp2)
        nScrollBars = 2;
    else
        nScrollBars = 1;
    *rContents << nScrollBars;
    pBlockFlags[0] |= 0x20;

    aTmp = rPropSet->getPropertyValue(C2U("EchoChar"));
    sal_uInt16 nTmp = sal_uInt16();
    aTmp >>= nTmp;
    nPasswordChar = static_cast<sal_uInt8>(nTmp);
    *rContents << nPasswordChar;
    pBlockFlags[1] |= 0x02;

    SvxOcxString aValue( rPropSet->getPropertyValue(C2U("DefaultText")) );
    aValue.WriteLenField( *rContents );
    if (aValue.HasData())
        pBlockFlags[2] |= 0x40;

    WriteAlign(rContents,4);
    aTmp = rPropSet->getPropertyValue(C2U("BorderColor"));
    if (aTmp.hasValue())
        aTmp >>= nBorderColor;
    *rContents << ExportColor(nBorderColor);
    pBlockFlags[3] |= 0x02;

    *rContents << nSpecialEffect;
    pBlockFlags[3] |= 0x04;

    WriteAlign(rContents,4);
    *rContents << rSize.Width;
    *rContents << rSize.Height;

    aValue.WriteCharArray( *rContents );

    WriteAlign(rContents,4);

    nFixedAreaLen = static_cast<sal_uInt16>(rContents->Tell()-nOldPos-4);

    bRet = aFontData.Export(rContents,rPropSet);

    rContents->Seek(nOldPos);
    *rContents << nStandardId;
    *rContents << nFixedAreaLen;

    *rContents << pBlockFlags[0];
    *rContents << pBlockFlags[1];
    *rContents << pBlockFlags[2];
    *rContents << pBlockFlags[3];
    *rContents << pBlockFlags[4];
    *rContents << pBlockFlags[5];
    *rContents << pBlockFlags[6];
    *rContents << pBlockFlags[7];

    DBG_ASSERT((rContents.Is() &&
        (SVSTREAM_OK == rContents->GetError())),"damn");
    return bRet;
}


sal_Bool OCX_TextBox::Export(SvStorageRef &rObj,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    static sal_uInt8 const aCompObj[] = {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0x10, 0x1D, 0xD2, 0x8B,
        0x42, 0xEC, 0xCE, 0x11, 0x9E, 0x0D, 0x00, 0xAA,
        0x00, 0x60, 0x02, 0xF3, 0x1C, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
        0x32, 0x2E, 0x30, 0x20, 0x54, 0x65, 0x78, 0x74,
        0x42, 0x6F, 0x78, 0x00, 0x10, 0x00, 0x00, 0x00,
        0x45, 0x6D, 0x62, 0x65, 0x64, 0x64, 0x65, 0x64,
        0x20, 0x4F, 0x62, 0x6A, 0x65, 0x63, 0x74, 0x00,
        0x10, 0x00, 0x00, 0x00, 0x46, 0x6F, 0x72, 0x6D,
        0x73, 0x2E, 0x54, 0x65, 0x78, 0x74, 0x42, 0x6F,
        0x78, 0x2E, 0x31, 0x00, 0xF4, 0x39, 0xB2, 0x71,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor( rObj->OpenSotStream( C2U("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenSotStream( C2U("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 const aOCXNAME[] = {
        0x54, 0x00, 0x65, 0x00, 0x78, 0x00, 0x74, 0x00,
        0x42, 0x00, 0x6F, 0x00, 0x78, 0x00, 0x31, 0x00,
        0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor2( rObj->OpenSotStream( C2U("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenSotStream( C2U("contents")));
    return WriteContents(xContents, rPropSet, rSize);
}

sal_Bool OCX_FieldControl::WriteContents(SvStorageStreamRef &rContents,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    sal_Bool bRet=sal_True;
    sal_uInt32 nOldPos = rContents->Tell();
    rContents->SeekRel(12);

    pBlockFlags[0] = 0;
    pBlockFlags[1] = 0x01;
    pBlockFlags[2] = 0x00;
    pBlockFlags[3] = 0x80;
    pBlockFlags[4] = 0;
    pBlockFlags[5] = 0;
    pBlockFlags[6] = 0;
    pBlockFlags[7] = 0;


    sal_uInt8 nTemp=0x19;
    uno::Any aTmp = rPropSet->getPropertyValue(C2U("Enabled"));
    fEnabled = any2bool(aTmp);
    if (fEnabled)
        nTemp |= 0x02;

    aTmp = rPropSet->getPropertyValue(C2U("ReadOnly"));
    fLocked = any2bool(aTmp);
    if (fLocked)
        nTemp |= 0x04;

    *rContents << nTemp;
    pBlockFlags[0] |= 0x01;
    *rContents << sal_uInt8(0x48);
    *rContents << sal_uInt8(0x80);

    nTemp = 0x2C;
    *rContents << nTemp;

    aTmp = rPropSet->getPropertyValue(C2U("BackgroundColor"));
    if (aTmp.hasValue())
        aTmp >>= mnBackColor;
    *rContents << ExportColor(mnBackColor);
    pBlockFlags[0] |= 0x02;

    aTmp = rPropSet->getPropertyValue(C2U("TextColor"));
    if (aTmp.hasValue())
        aTmp >>= mnForeColor;
    *rContents << ExportColor(mnForeColor);
    pBlockFlags[0] |= 0x04;

    aTmp = rPropSet->getPropertyValue(C2U("Border"));
    sal_Int16 nBorder = sal_Int16();
    aTmp >>= nBorder;
    nSpecialEffect = ExportBorder(nBorder,nBorderStyle);
    *rContents << nBorderStyle;
    pBlockFlags[0] |= 0x10;

#if 0 //Each control has a different Value format, and how to convert each to text has to be found out
    SvxOcxString aValue( rPropSet->getPropertyValue(C2U("DefaultText")) );
    aValue.WriteLenField( *rContents );
    if (aValue.HasData())
        pBlockFlags[2] |= 0x40;
#endif

    *rContents << nSpecialEffect;
    pBlockFlags[3] |= 0x04;

    WriteAlign(rContents,4);
    *rContents << rSize.Width;
    *rContents << rSize.Height;

#if 0
    aValue.WriteCharArray( *rContents );
#endif

    WriteAlign(rContents,4);

    nFixedAreaLen = static_cast<sal_uInt16>(rContents->Tell()-nOldPos-4);

    bRet = aFontData.Export(rContents,rPropSet);

    rContents->Seek(nOldPos);
    *rContents << nStandardId;
    *rContents << nFixedAreaLen;

    *rContents << pBlockFlags[0];
    *rContents << pBlockFlags[1];
    *rContents << pBlockFlags[2];
    *rContents << pBlockFlags[3];
    *rContents << pBlockFlags[4];
    *rContents << pBlockFlags[5];
    *rContents << pBlockFlags[6];
    *rContents << pBlockFlags[7];

    DBG_ASSERT((rContents.Is() &&
        (SVSTREAM_OK==rContents->GetError())),"damn");
    return bRet;
}

sal_Bool OCX_FieldControl::Export(SvStorageRef &rObj,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    static sal_uInt8 const aCompObj[] = {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0x10, 0x1D, 0xD2, 0x8B,
        0x42, 0xEC, 0xCE, 0x11, 0x9E, 0x0D, 0x00, 0xAA,
        0x00, 0x60, 0x02, 0xF3, 0x1C, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
        0x32, 0x2E, 0x30, 0x20, 0x54, 0x65, 0x78, 0x74,
        0x42, 0x6F, 0x78, 0x00, 0x10, 0x00, 0x00, 0x00,
        0x45, 0x6D, 0x62, 0x65, 0x64, 0x64, 0x65, 0x64,
        0x20, 0x4F, 0x62, 0x6A, 0x65, 0x63, 0x74, 0x00,
        0x10, 0x00, 0x00, 0x00, 0x46, 0x6F, 0x72, 0x6D,
        0x73, 0x2E, 0x54, 0x65, 0x78, 0x74, 0x42, 0x6F,
        0x78, 0x2E, 0x31, 0x00, 0xF4, 0x39, 0xB2, 0x71,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor( rObj->OpenSotStream( C2U("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenSotStream( C2U("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 const aOCXNAME[] = {
        0x54, 0x00, 0x65, 0x00, 0x78, 0x00, 0x74, 0x00,
        0x42, 0x00, 0x6F, 0x00, 0x78, 0x00, 0x31, 0x00,
        0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor2( rObj->OpenSotStream( C2U("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenSotStream( C2U("contents")));
    return WriteContents(xContents, rPropSet, rSize);
}

sal_Bool OCX_ToggleButton::Export(
    SvStorageRef &rObj, const uno::Reference< beans::XPropertySet> &rPropSet,
    const awt::Size& rSize )
{
    static sal_uInt8 const aCompObj[] = {
            0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
            0xFF, 0xFF, 0xFF, 0xFF, 0x60, 0x1D, 0xD2, 0x8B,
            0x42, 0xEC, 0xCE, 0x11, 0x9E, 0x0D, 0x00, 0xAA,
            0x00, 0x60, 0x02, 0xF3, 0x21, 0x00, 0x00, 0x00,
            0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
            0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
            0x32, 0x2E, 0x30, 0x20, 0x54, 0x6F, 0x67, 0x67,
            0x6C, 0x65, 0x42, 0x75, 0x74, 0x74, 0x6F, 0x6E,
            0x00, 0x10, 0x00, 0x00, 0x00, 0x45, 0x6D, 0x62,
            0x65, 0x64, 0x64, 0x65, 0x64, 0x20, 0x4F, 0x62,
            0x6A, 0x65, 0x63, 0x74, 0x00, 0x15, 0x00, 0x00,
            0x00, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x2E, 0x54,
            0x6F, 0x67, 0x67, 0x6C, 0x65, 0x42, 0x75, 0x74,
            0x74, 0x6F, 0x6E, 0x2E, 0x31, 0x00, 0xF4, 0x39,
            0xB2, 0x71, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor( rObj->OpenSotStream( C2U("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenSotStream( C2U("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 const aOCXNAME[] = {
        0x54, 0x00, 0x6F, 0x00, 0x67, 0x00, 0x67, 0x00,
        0x6C, 0x00, 0x65, 0x00, 0x42, 0x00, 0x75, 0x00,
        0x74, 0x00, 0x74, 0x00, 0x6F, 0x00, 0x6E, 0x00,
        0x31, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    {
    SvStorageStreamRef xStor2( rObj->OpenSotStream( C2U("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenSotStream( C2U("contents")));

    return WriteContents(xContents,rPropSet,rSize);
}

sal_Bool OCX_ToggleButton::WriteContents(SvStorageStreamRef &rContents,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    sal_Bool bRet=sal_True;
    sal_uInt32 nOldPos = rContents->Tell();
    rContents->SeekRel(12);

    pBlockFlags[0] = 0;
    pBlockFlags[1] = 0x01;
    pBlockFlags[2] = 0;
    pBlockFlags[3] = 0x80;
    pBlockFlags[4] = 0;
    pBlockFlags[5] = 0;
    pBlockFlags[6] = 0;
    pBlockFlags[7] = 0;

    uno::Any aTmp = rPropSet->getPropertyValue(C2U("Enabled"));
    fEnabled = any2bool(aTmp);

    sal_uInt8 nTemp=fEnabled;
    if (fEnabled)
        nTemp = nTemp << 1;
    if (fBackStyle)
        nTemp |= 0x08;
    *rContents << nTemp;
    pBlockFlags[0] |= 0x01;
    *rContents << sal_uInt8(0x00);
    nTemp = 0;
    aTmp = rPropSet->getPropertyValue(C2U("MultiLine"));
    fWordWrap = any2bool(aTmp);
    if (fWordWrap)
        nTemp |= 0x80;
    *rContents << nTemp;
    *rContents << sal_uInt8(0x00);

    aTmp = rPropSet->getPropertyValue(C2U("BackgroundColor"));
    if (aTmp.hasValue())
        aTmp >>= mnBackColor;
    *rContents << ExportColor(mnBackColor);
    pBlockFlags[0] |= 0x02;

    aTmp = rPropSet->getPropertyValue(C2U("TextColor"));
    if (aTmp.hasValue())
        aTmp >>= mnForeColor;
    *rContents << ExportColor(mnForeColor);
    pBlockFlags[0] |= 0x04;

    nStyle = 6;
    *rContents << nStyle;
    pBlockFlags[0] |= 0x40;

    WriteAlign(rContents,4);
    nValueLen = 1|SVX_MSOCX_COMPRESSED;
    bool bDefault = false;
    rPropSet->getPropertyValue(C2U("DefaultState")) >>= bDefault;
    sal_uInt8 nDefault = static_cast< sal_uInt8 >( bDefault ? '1' : '0' );
    *rContents << nValueLen;
    pBlockFlags[2] |= 0x40;

    SvxOcxString aCaption( rPropSet->getPropertyValue(C2U("Label")) );
    aCaption.WriteLenField( *rContents );
    if (aCaption.HasData())
        pBlockFlags[2] |= 0x80;

    WriteAlign(rContents,4);
    *rContents << rSize.Width;
    *rContents << rSize.Height;

    *rContents << nDefault;
    *rContents << sal_uInt8(0x00);

    aCaption.WriteCharArray( *rContents );

    WriteAlign(rContents,4);
    nFixedAreaLen = static_cast<sal_uInt16>(rContents->Tell()-nOldPos-4);
    bRet = aFontData.Export(rContents,rPropSet);
    rContents->Seek(nOldPos);
    *rContents << nStandardId;
    *rContents << nFixedAreaLen;

    *rContents << pBlockFlags[0];
    *rContents << pBlockFlags[1];
    *rContents << pBlockFlags[2];
    *rContents << pBlockFlags[3];
    *rContents << pBlockFlags[4];
    *rContents << pBlockFlags[5];
    *rContents << pBlockFlags[6];
    *rContents << pBlockFlags[7];

    DBG_ASSERT((rContents.Is() &&
        (SVSTREAM_OK==rContents->GetError())),"damn");
    return bRet;
}

sal_Bool OCX_ComboBox::WriteContents(SvStorageStreamRef &rContents,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    sal_Bool bRet=sal_True;
    sal_uInt32 nOldPos = rContents->Tell();
    rContents->SeekRel(12);

    pBlockFlags[0] = 0;
    pBlockFlags[1] = 0x01;
    pBlockFlags[2] = 0x00;
    pBlockFlags[3] = 0x80;
    pBlockFlags[4] = 0;
    pBlockFlags[5] = 0;
    pBlockFlags[6] = 0;
    pBlockFlags[7] = 0;


    sal_uInt8 nTemp=0x19;//fEnabled;
    uno::Any aTmp = rPropSet->getPropertyValue(C2U("Enabled"));
    fEnabled = any2bool(aTmp);
    if (fEnabled)
        nTemp |= 0x02;

    aTmp = rPropSet->getPropertyValue(C2U("ReadOnly"));
    fLocked = any2bool(aTmp);
    if (fLocked)
        nTemp |= 0x04;

    *rContents << nTemp;
    pBlockFlags[0] |= 0x01;
    *rContents << sal_uInt8(0x48);
    *rContents << sal_uInt8(0x80);

    nTemp = 0x0C;
    fHideSelection = any2bool(rPropSet->getPropertyValue(C2U("HideInactiveSelection")));
    if( fHideSelection )
        nTemp |= 0x20;
    *rContents << nTemp;

    aTmp = rPropSet->getPropertyValue(C2U("BackgroundColor"));
    if (aTmp.hasValue())
        aTmp >>= mnBackColor;
    *rContents << ExportColor(mnBackColor);
    pBlockFlags[0] |= 0x02;

    aTmp = rPropSet->getPropertyValue(C2U("TextColor"));
    if (aTmp.hasValue())
        aTmp >>= mnForeColor;
    *rContents << ExportColor(mnForeColor);
    pBlockFlags[0] |= 0x04;

    aTmp = rPropSet->getPropertyValue(C2U("Border"));
    sal_Int16 nBorder = sal_Int16();
    aTmp >>= nBorder;
    nSpecialEffect = ExportBorder(nBorder,nBorderStyle);
    *rContents << nBorderStyle;
    pBlockFlags[0] |= 0x10;

    nStyle = 3;
    *rContents << nStyle;
    pBlockFlags[0] |= 0x40;

    WriteAlign(rContents,2);

    aTmp = rPropSet->getPropertyValue(C2U("LineCount"));
    aTmp >>= nListRows;
    *rContents << nListRows;
    pBlockFlags[1] |= 0x40;

    *rContents << sal_uInt8(1); //DefaultSelected One
    pBlockFlags[2] |= 0x01;

    aTmp = rPropSet->getPropertyValue(C2U("Dropdown"));
    nDropButtonStyle = any2bool(aTmp);
    if (nDropButtonStyle)
        nDropButtonStyle=0x02;
    *rContents << nDropButtonStyle;
    pBlockFlags[2] |= 0x04;

    SvxOcxString aValue( rPropSet->getPropertyValue(C2U("Text")) );
    aValue.WriteLenField( *rContents );
    if (aValue.HasData())
        pBlockFlags[2] |= 0x40;

    WriteAlign(rContents,4);
    aTmp = rPropSet->getPropertyValue(C2U("BorderColor"));
    if (aTmp.hasValue())
        aTmp >>= nBorderColor;
    *rContents << ExportColor(nBorderColor);
    pBlockFlags[3] |= 0x02;

    *rContents << nSpecialEffect;
    pBlockFlags[3] |= 0x04;

    WriteAlign(rContents,4);
    *rContents << rSize.Width;
    *rContents << rSize.Height;

    aValue.WriteCharArray( *rContents );

    WriteAlign(rContents,4);

    nFixedAreaLen = static_cast<sal_uInt16>(rContents->Tell()-nOldPos-4);

    bRet = aFontData.Export(rContents,rPropSet);

    rContents->Seek(nOldPos);
    *rContents << nStandardId;
    *rContents << nFixedAreaLen;

    *rContents << pBlockFlags[0];
    *rContents << pBlockFlags[1];
    *rContents << pBlockFlags[2];
    *rContents << pBlockFlags[3];
    *rContents << pBlockFlags[4];
    *rContents << pBlockFlags[5];
    *rContents << pBlockFlags[6];
    *rContents << pBlockFlags[7];

    DBG_ASSERT((rContents.Is() &&
        (SVSTREAM_OK==rContents->GetError())),"damn");
    return bRet;
}


sal_Bool OCX_ComboBox::Export(SvStorageRef &rObj,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    static sal_uInt8 const aCompObj[] = {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0x30, 0x1D, 0xD2, 0x8B,
        0x42, 0xEC, 0xCE, 0x11, 0x9E, 0x0D, 0x00, 0xAA,
        0x00, 0x60, 0x02, 0xF3, 0x1D, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
        0x32, 0x2E, 0x30, 0x20, 0x43, 0x6F, 0x6D, 0x62,
        0x6F, 0x42, 0x6F, 0x78, 0x00, 0x10, 0x00, 0x00,
        0x00, 0x45, 0x6D, 0x62, 0x65, 0x64, 0x64, 0x65,
        0x64, 0x20, 0x4F, 0x62, 0x6A, 0x65, 0x63, 0x74,
        0x00, 0x11, 0x00, 0x00, 0x00, 0x46, 0x6F, 0x72,
        0x6D, 0x73, 0x2E, 0x43, 0x6F, 0x6D, 0x62, 0x6F,
        0x42, 0x6F, 0x78, 0x2E, 0x31, 0x00, 0xF4, 0x39,
        0xB2, 0x71, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor( rObj->OpenSotStream( C2U("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenSotStream( C2U("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 const aOCXNAME[] = {
        0x43, 0x00, 0x6F, 0x00, 0x6D, 0x00, 0x62, 0x00,
        0x6F, 0x00, 0x42, 0x00, 0x6F, 0x00, 0x78, 0x00,
        0x31, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    {
    SvStorageStreamRef xStor2( rObj->OpenSotStream( C2U("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenSotStream( C2U("contents")));
    return WriteContents(xContents, rPropSet, rSize);
}

sal_Bool OCX_ListBox::WriteContents(SvStorageStreamRef &rContents,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    sal_Bool bRet=sal_True;
    sal_uInt32 nOldPos = rContents->Tell();
    rContents->SeekRel(12);

    pBlockFlags[0] = 0;
    pBlockFlags[1] = 0x01;
    pBlockFlags[2] = 0x01;
    pBlockFlags[3] = 0x80;
    pBlockFlags[4] = 0;
    pBlockFlags[5] = 0;
    pBlockFlags[6] = 0;
    pBlockFlags[7] = 0;

    uno::Any aTmp = rPropSet->getPropertyValue(C2U("Enabled"));
    fEnabled = any2bool(aTmp);
    sal_uInt8 nTemp=fEnabled;
    if (fEnabled)
        nTemp = nTemp << 1;

    aTmp = rPropSet->getPropertyValue(C2U("ReadOnly"));
    fLocked = any2bool(aTmp);
    if (fLocked)
        nTemp |= 0x04;

    *rContents << nTemp;
    pBlockFlags[0] |= 0x01;
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);

    aTmp = rPropSet->getPropertyValue(C2U("BackgroundColor"));
    if (aTmp.hasValue())
        aTmp >>= mnBackColor;
    *rContents << ExportColor(mnBackColor);
    pBlockFlags[0] |= 0x02;

    aTmp = rPropSet->getPropertyValue(C2U("TextColor"));
    if (aTmp.hasValue())
        aTmp >>= mnForeColor;
    *rContents << ExportColor(mnForeColor);
    pBlockFlags[0] |= 0x04;

    aTmp = rPropSet->getPropertyValue(C2U("Border"));
    sal_Int16 nBorder = sal_Int16();
    aTmp >>= nBorder;
    nSpecialEffect = ExportBorder(nBorder,nBorderStyle);
    WriteAlign(rContents,2);
    *rContents << nBorderStyle;
    pBlockFlags[0] |= 0x10;

    aTmp = rPropSet->getPropertyValue(C2U("MultiSelection"));
    nMultiState = any2bool(aTmp);

    if (nMultiState)
    {
        *rContents << nMultiState;
        pBlockFlags[0] |= 0x20;
    }

    nStyle = 2;
    *rContents << nStyle;
    pBlockFlags[0] |= 0x40;


    WriteAlign(rContents,4);

#if 0
    SvxOcxString aValue( rPropSet->getPropertyValue(C2U("DefaultText")) );
    aValue.WriteLenField( *rContents );
    if (aValue.HasData())
        pBlockFlags[2] |= 0x40;

    WriteAlign(rContents,4);
#endif

    WriteAlign(rContents,4);
    aTmp = rPropSet->getPropertyValue(C2U("BorderColor"));
    if (aTmp.hasValue())
        aTmp >>= nBorderColor;
    *rContents << ExportColor(nBorderColor);
    pBlockFlags[3] |= 0x02;

    *rContents << nSpecialEffect;
    pBlockFlags[3] |= 0x04;

    WriteAlign(rContents,4);
    *rContents << rSize.Width;
    *rContents << rSize.Height;

#if 0
    aValue.WriteCharArray( *rContents );
#endif

    WriteAlign(rContents,4);

    nFixedAreaLen = static_cast<sal_uInt16>(rContents->Tell()-nOldPos-4);

    bRet = aFontData.Export(rContents,rPropSet);

    rContents->Seek(nOldPos);
    *rContents << nStandardId;
    *rContents << nFixedAreaLen;

    *rContents << pBlockFlags[0];
    *rContents << pBlockFlags[1];
    *rContents << pBlockFlags[2];
    *rContents << pBlockFlags[3];
    *rContents << pBlockFlags[4];
    *rContents << pBlockFlags[5];
    *rContents << pBlockFlags[6];
    *rContents << pBlockFlags[7];

    DBG_ASSERT((rContents.Is() &&
        (SVSTREAM_OK==rContents->GetError())),"damn");
    return bRet;
}

sal_Bool OCX_ListBox::Export(SvStorageRef &rObj,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    static sal_uInt8 const aCompObj[] = {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x1D, 0xD2, 0x8B,
        0x42, 0xEC, 0xCE, 0x11, 0x9E, 0x0D, 0x00, 0xAA,
        0x00, 0x60, 0x02, 0xF3, 0x1C, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
        0x32, 0x2E, 0x30, 0x20, 0x4C, 0x69, 0x73, 0x74,
        0x42, 0x6F, 0x78, 0x00, 0x10, 0x00, 0x00, 0x00,
        0x45, 0x6D, 0x62, 0x65, 0x64, 0x64, 0x65, 0x64,
        0x20, 0x4F, 0x62, 0x6A, 0x65, 0x63, 0x74, 0x00,
        0x10, 0x00, 0x00, 0x00, 0x46, 0x6F, 0x72, 0x6D,
        0x73, 0x2E, 0x4C, 0x69, 0x73, 0x74, 0x42, 0x6F,
        0x78, 0x2E, 0x31, 0x00, 0xF4, 0x39, 0xB2, 0x71,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor( rObj->OpenSotStream( C2U("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenSotStream( C2U("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 const aOCXNAME[] = {
        0x4C, 0x00, 0x69, 0x00, 0x73, 0x00, 0x74, 0x00,
        0x42, 0x00, 0x6F, 0x00, 0x78, 0x00, 0x31, 0x00,
        0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor2( rObj->OpenSotStream( C2U("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenSotStream( C2U("contents")));
    return WriteContents(xContents, rPropSet, rSize);
}

sal_Bool OCX_Label::WriteContents(SvStorageStreamRef &rContents,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    sal_Bool bRet = sal_True;
    sal_uInt32 nOldPos = rContents->Tell();
    rContents->SeekRel(8);
    pBlockFlags[0] = 0x20;
    pBlockFlags[1] = 0;
    pBlockFlags[2] = 0;
    pBlockFlags[3] = 0;

    uno::Any aTmp = rPropSet->getPropertyValue(C2U("TextColor"));
    if (aTmp.hasValue())
        aTmp >>= mnForeColor;
    *rContents << ExportColor(mnForeColor);
    pBlockFlags[0] |= 0x01;

    aTmp = rPropSet->getPropertyValue(C2U("BackgroundColor"));
    if (aTmp.hasValue())
        aTmp >>= mnBackColor;
    *rContents << ExportColor(mnBackColor);
    pBlockFlags[0] |= 0x02;

    aTmp = rPropSet->getPropertyValue(C2U("Enabled"));
    fEnabled = any2bool(aTmp);
    sal_uInt8 nTemp=fEnabled;
    if (fEnabled)
        nTemp = nTemp << 1;
    *rContents << nTemp;
    *rContents << sal_uInt8(0x00);

    aTmp = rPropSet->getPropertyValue(C2U("MultiLine"));
    fWordWrap = any2bool(aTmp);
    nTemp=fWordWrap;
    nTemp = nTemp << 7;
    *rContents << nTemp;
    *rContents << sal_uInt8(0x00);
    pBlockFlags[0] |= 0x04;

    SvxOcxString aCaption( rPropSet->getPropertyValue(C2U("Label")) );
    aCaption.WriteLenField( *rContents );
    if (aCaption.HasData())
        pBlockFlags[0] |= 0x08;

    WriteAlign(rContents,4);
    aTmp = rPropSet->getPropertyValue(C2U("BorderColor"));
    if (aTmp.hasValue())
        aTmp >>= nBorderColor;
    *rContents << ExportColor(nBorderColor);
    pBlockFlags[0] |= 0x80;

    aTmp = rPropSet->getPropertyValue(C2U("Border"));
    sal_Int16 nBorder = sal_Int16();
    aTmp >>= nBorder;
    sal_uInt8 nNewBorder;
    nSpecialEffect = ExportBorder(nBorder,nNewBorder);
    nBorderStyle = nNewBorder;
    *rContents << nBorderStyle;
    pBlockFlags[1] |= 0x01;
    *rContents << nSpecialEffect;
    pBlockFlags[1] |= 0x02;

    aCaption.WriteCharArray( *rContents );

    WriteAlign(rContents,4);
    *rContents << rSize.Width;
    *rContents << rSize.Height;
    nFixedAreaLen = static_cast<sal_uInt16>(rContents->Tell()-nOldPos-4);

    bRet = aFontData.Export(rContents,rPropSet);

    rContents->Seek(nOldPos);
    *rContents << nStandardId;
    *rContents << nFixedAreaLen;

    *rContents << pBlockFlags[0];
    *rContents << pBlockFlags[1];
    *rContents << pBlockFlags[2];
    *rContents << pBlockFlags[3];

    DBG_ASSERT((rContents.Is() &&
        (SVSTREAM_OK==rContents->GetError())),"damn");
    return bRet;
}



sal_Bool OCX_Label::Export(SvStorageRef &rObj,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    static sal_uInt8 const aCompObj[] = {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0x23, 0x9E, 0x8C, 0x97,
        0xB0, 0xD4, 0xCE, 0x11, 0xBF, 0x2D, 0x00, 0xAA,
        0x00, 0x3F, 0x40, 0xD0, 0x1A, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
        0x32, 0x2E, 0x30, 0x20, 0x4C, 0x61, 0x62, 0x65,
        0x6C, 0x00, 0x10, 0x00, 0x00, 0x00, 0x45, 0x6D,
        0x62, 0x65, 0x64, 0x64, 0x65, 0x64, 0x20, 0x4F,
        0x62, 0x6A, 0x65, 0x63, 0x74, 0x00, 0x0E, 0x00,
        0x00, 0x00, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x2E,
        0x4C, 0x61, 0x62, 0x65, 0x6C, 0x2E, 0x31, 0x00,
        0xF4, 0x39, 0xB2, 0x71, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor( rObj->OpenSotStream( C2U("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenSotStream( C2U("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 const aOCXNAME[] = {
        0x4C, 0x00, 0x61, 0x00, 0x62, 0x00, 0x65, 0x00,
        0x6C, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor2( rObj->OpenSotStream( C2U("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenSotStream( C2U("contents")));
    return WriteContents(xContents, rPropSet, rSize);
}

typedef OCX_Control *(*FNCreate)();


struct OCX_map
{
    FNCreate pCreate;
    const char *sId;
    sal_Int16 nId;
    const char *sName;
};

OCX_map aOCXTab[] =
{
    // Command button MUST be at index 0
    {&OCX_CommandButton::Create,"D7053240-CE69-11CD-a777-00dd01143c57",
        form::FormComponentType::COMMANDBUTTON,"CommandButton"},
    // Toggle button MUST be at index 1
    {&OCX_ToggleButton::Create,"8BD21D60-EC42-11CE-9e0d-00aa006002f3",
        form::FormComponentType::COMMANDBUTTON,"ToggleButton"},
    {&OCX_Label::Create, "978C9E23-D4B0-11CE-bf2d-00aa003f40d0",
        form::FormComponentType::FIXEDTEXT,"Label"},
    {&OCX_TextBox::Create,"8BD21D10-EC42-11CE-9e0d-00aa006002f3",
        form::FormComponentType::TEXTFIELD,"TextBox"},
    {&OCX_ListBox::Create,"8BD21D20-EC42-11CE-9e0d-00aa006002f3",
        form::FormComponentType::LISTBOX,"ListBox"},
    {&OCX_ComboBox::Create,"8BD21D30-EC42-11CE-9e0d-00aa006002f3",
        form::FormComponentType::COMBOBOX,"ComboBox"},
    {&OCX_CheckBox::Create,"8BD21D40-EC42-11CE-9e0d-00aa006002f3",
        form::FormComponentType::CHECKBOX,"CheckBox"},
    {&OCX_OptionButton::Create,"8BD21D50-EC42-11CE-9e0d-00aa006002f3",
        form::FormComponentType::RADIOBUTTON,"OptionButton"},
    {&OCX_Image::Create,"4C599241-6926-101B-9992-00000b65c6f9",
        form::FormComponentType::IMAGECONTROL,"Image"},
    {&OCX_FieldControl::Create,"8BD21D10-EC42-11CE-9e0d-00aa006002f3",
        form::FormComponentType::DATEFIELD,"TextBox"},
    {&OCX_FieldControl::Create,"8BD21D10-EC42-11CE-9e0d-00aa006002f3",
        form::FormComponentType::TIMEFIELD,"TextBox"},
    {&OCX_FieldControl::Create,"8BD21D10-EC42-11CE-9e0d-00aa006002f3",
        form::FormComponentType::NUMERICFIELD,"TextBox"},
    {&OCX_FieldControl::Create,"8BD21D10-EC42-11CE-9e0d-00aa006002f3",
        form::FormComponentType::CURRENCYFIELD,"TextBox"},
    {&OCX_FieldControl::Create,"8BD21D10-EC42-11CE-9e0d-00aa006002f3",
        form::FormComponentType::PATTERNFIELD,"TextBox"},
#if 0
    {&OCX_FieldControl::Create,"8BD21D10-EC42-11CE-9e0d-00aa006002f3",
        form::FormComponentType::FORMULAFIELD,"TextBox"},
#endif
    {&OCX_ImageButton::Create,"D7053240-CE69-11CD-a777-00dd01143c57",
        form::FormComponentType::IMAGEBUTTON,"CommandButton"},
    {&OCX_SpinButton::Create,"79176FB0-B7F2-11CE-97ef-00aa006d2776",
        form::FormComponentType::SPINBUTTON,"SpinButton"},
    {&OCX_ScrollBar::Create,"DFD181E0-5E2F-11CE-a449-00aa004a803d",
        form::FormComponentType::SCROLLBAR,"ScrollBar"},
    {&OCX_GroupBox::Create,"",
        form::FormComponentType::GROUPBOX,""},
    {&OCX_ProgressBar::Create,"",
        form::FormComponentType::CONTROL,""},
    {&HTML_TextBox::Create,"5512D124-5CC6-11CF-8d67-00aa00bdce1d", form::FormComponentType::TEXTFIELD,"TextBox"},
    {&HTML_Select::Create,"5512D122-5CC6-11CF-8d67-00aa00bdce1d",
        form::FormComponentType::LISTBOX,"ListBox"},
};

const int NO_OCX = sizeof( aOCXTab ) / sizeof( *aOCXTab );

SvxMSConvertOCXControls::SvxMSConvertOCXControls(SfxObjectShell *pDSh, SwPaM *pP) :
    pDocSh(pDSh), pPaM(pP), nEdit(0), nCheckbox(0)
{
    DBG_ASSERT( pDocSh, "No DocShell, Cannot do Controls" );
    OCX_Control::FillSystemColors();
}

SvxMSConvertOCXControls::~SvxMSConvertOCXControls()
{
}

OCX_Control * SvxMSConvertOCXControls::OCX_Factory(const String &sName)
{
    for (int i=0;i<NO_OCX;i++)
    {
        if ( sName.EqualsIgnoreCaseAscii( aOCXTab[i].sId ))
            return(aOCXTab[i].pCreate());
    }
    return(NULL);
}

OCX_Control * SvxMSConvertOCXControls::OCX_Factory(
    const uno::Reference< awt::XControlModel > &rControlModel,
    String &rId, String &rName)
{
    rName.Erase();
    rId.Erase();

    uno::Reference< beans::XPropertySet > xPropSet(
        rControlModel,uno::UNO_QUERY);

    uno::Any aTmp = xPropSet->getPropertyValue(
        OUString( RTL_CONSTASCII_USTRINGPARAM( "ClassId" )));
    sal_Int16 nClassId = *(sal_Int16*) aTmp.getValue();

//Begin nasty hack
    /*
    There is a truly horrible thing with EditControls and FormattedField
    Controls, they both pretend to have an EDITBOX ClassId for compability
    reasons, at some stage in the future hopefully there will be a proper
    FormulaField ClassId rather than this piggybacking two controls onto the
    same ClassId, when this happens uncomment the FORMULAFIELD in the OCX_Tab
    and delete this block, cmc.

    And also the nClassId for ImageControls is being reported as "CONTROL"
    rather than IMAGECONTROL
    */
    if (nClassId == form::FormComponentType::TEXTFIELD)
    {
        uno::Reference< lang::XServiceInfo > xInfo(rControlModel,
            uno::UNO_QUERY);
        if (xInfo->
            supportsService(OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.component.FormattedField" ))))
        {
            rId.AppendAscii("8BD21D10-EC42-11CE-9e0d-00aa006002f3");
            rName.AppendAscii("TextBox");
            return new OCX_FieldControl;
        }
    }
    else if (nClassId == form::FormComponentType::CONTROL)
    {
        uno::Reference< lang::XServiceInfo > xInfo(rControlModel,
            uno::UNO_QUERY);
        if (xInfo->
            supportsService(OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.component.ImageControl" ))))
        nClassId = form::FormComponentType::IMAGECONTROL;
    }
//End nasty hack

    const OCX_map* pEntry = 0;

    // distinguish between push button and toggle button
    if( nClassId == form::FormComponentType::COMMANDBUTTON )
    {
        pEntry = any2bool(xPropSet->getPropertyValue(C2U("Toggle"))) ?
            (aOCXTab + 1) : aOCXTab;
    }
    else
    {
        for( int i = 2; (i < NO_OCX) && !pEntry; ++i )
            if( nClassId == aOCXTab[ i ].nId )
                pEntry = aOCXTab + i;
    }

    if( pEntry )
    {
        rId.AppendAscii( pEntry->sId );
        rName.AppendAscii( pEntry->sName );
        return pEntry->pCreate();
    }

    return 0;
}


sal_Bool SvxMSConvertOCXControls::WriteOCXStream( SvStorageRef& rSrc1,
    const uno::Reference< awt::XControlModel > &rControlModel,
    const awt::Size &rSize, String &rName)
{
    sal_Bool bRet=sal_False;

    DBG_ASSERT( rControlModel.is(), "UNO-Control missing Model, panic!" );
    if( !rControlModel.is() )
        return sal_False;

#if 0
    uno::Any aTmp = xPropSet->getPropertyValue(
        OUString(RTL_CONSTASCII_USTRINGPARAM("ClassId")));
    sal_Int16 nClassId = *(sal_Int16*) aTmp.getValue();
#endif

    String sId;
    OCX_Control *pObj = OCX_Factory(rControlModel,sId,rName);
    if (pObj != NULL)
    {
        uno::Reference<beans::XPropertySet> xPropSet(rControlModel,
            uno::UNO_QUERY);

        /* #117832# - also enable export of control name  */
        OUString sCName;
        xPropSet->getPropertyValue(C2U("Name")) >>= sCName;
        pObj->sName = sCName;

        SvGlobalName aName;
        aName.MakeId(sId);
        String sFullName(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(
            "Microsoft Forms 2.0 ")));
        sFullName.Append(rName);
        rSrc1->SetClass(aName,0x5C,sFullName);
        //                    ^^^^, this pathetic 0x5c is the magic number
        // which the lack of originally blocked the implementation of
        // the controls export
        // cmc

        bRet = pObj->Export(rSrc1,xPropSet,rSize);
        SvStorageStreamRef xStor2( rSrc1->OpenSotStream( C2U("\3OCXNAME")));
        /* #117832# - also enable export of control name  */
        writeOCXNAME( sCName, xStor2 );
        delete pObj;
    }
    return bRet;
}


//I think this should work for excel documents, create the "Ctls" stream
//and give it here as rContents, we'll append out streams ole id and
//contents here and that appears to be what Excel is doing
sal_Bool SvxMSConvertOCXControls::WriteOCXExcelKludgeStream(
    SvStorageStreamRef& rContents,
    const uno::Reference< awt::XControlModel > &rControlModel,
    const awt::Size &rSize, String &rName)
{
    sal_Bool bRet=sal_False;

    DBG_ASSERT( rControlModel.is(), "UNO-Control missing Model, panic!" );
    if( !rControlModel.is() )
        return sal_False;

    String sId;
    OCX_Control *pObj = OCX_Factory(rControlModel,sId,rName);
    if (pObj != NULL)
    {
        uno::Reference<beans::XPropertySet> xPropSet(rControlModel,
            uno::UNO_QUERY);

        SvGlobalName aName;
        aName.MakeId(sId);
        String sFullName(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(
            "Microsoft Forms 2.0 ")));
        sFullName.Append(rName);
        SvStream *pS=rContents;
        *pS << aName;
        bRet = pObj->WriteContents(rContents,xPropSet,rSize);
        delete pObj;
        // export needs correct stream position
        rContents->Seek( STREAM_SEEK_TO_END );
    }
    return bRet;
}

sal_Bool OCX_CheckBox::WriteContents(SvStorageStreamRef &rContents,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)

{
    sal_Bool bRet=sal_True;
    sal_uInt32 nOldPos = rContents->Tell();
    rContents->SeekRel(12);

    pBlockFlags[0] = 0;
    pBlockFlags[1] = 0x01;
    pBlockFlags[2] = 0;
    pBlockFlags[3] = 0x80;
    pBlockFlags[4] = 0;
    pBlockFlags[5] = 0;
    pBlockFlags[6] = 0;
    pBlockFlags[7] = 0;

    uno::Any aTmp = rPropSet->getPropertyValue(C2U("Enabled"));
    fEnabled = any2bool(aTmp);

    aTmp = rPropSet->getPropertyValue(C2U("BackgroundColor"));
    fBackStyle = aTmp.hasValue() ? 1 : 0;
    if (fBackStyle)
        aTmp >>= mnBackColor;

    sal_uInt8 nTemp=fEnabled;
    if (fEnabled)
        nTemp = nTemp << 1;
    if (fBackStyle)
        nTemp |= 0x08;
    *rContents << nTemp;
    pBlockFlags[0] |= 0x01;
    *rContents << sal_uInt8(0x00);
    nTemp = 0;
    aTmp = rPropSet->getPropertyValue(C2U("MultiLine"));
    fWordWrap = any2bool(aTmp);
    if (fWordWrap)
        nTemp |= 0x80;
    *rContents << nTemp;
    *rContents << sal_uInt8(0x00);

    *rContents << ExportColor(mnBackColor);
    pBlockFlags[0] |= 0x02;

    aTmp = rPropSet->getPropertyValue(C2U("TextColor"));
    if (aTmp.hasValue())
        aTmp >>= mnForeColor;
    *rContents << ExportColor(mnForeColor);
    pBlockFlags[0] |= 0x04;

    nStyle = 4;
    *rContents << nStyle;
    pBlockFlags[0] |= 0x40;

    aTmp = rPropSet->getPropertyValue(C2U("TriState"));
    nMultiState = any2bool(aTmp);
    *rContents << nMultiState;
    pBlockFlags[2] |= 0x20;

    WriteAlign(rContents,4);
    nValueLen = 1|SVX_MSOCX_COMPRESSED;
    aTmp = rPropSet->getPropertyValue(C2U("DefaultState"));
    sal_Int16 nDefault = sal_Int16();
    aTmp >>= nDefault;
    *rContents << nValueLen;
    pBlockFlags[2] |= 0x40;

    SvxOcxString aCaption( rPropSet->getPropertyValue(C2U("Label")) );
    aCaption.WriteLenField( *rContents );
    if (aCaption.HasData())
        pBlockFlags[2] |= 0x80;

    aTmp = rPropSet->getPropertyValue(C2U("VisualEffect"));
    if (aTmp.hasValue())
    {
        sal_Int16 nApiSpecEffect = sal_Int16();
        aTmp >>= nApiSpecEffect;
        nSpecialEffect = ExportSpecEffect( nApiSpecEffect );
    }
    WriteAlign(rContents,4);
    *rContents << nSpecialEffect;
    pBlockFlags[3] |= 0x04;

    WriteAlign(rContents,4);
    *rContents << rSize.Width;
    *rContents << rSize.Height;

    nDefault += 0x30;
    *rContents << sal_uInt8(nDefault);
    *rContents << sal_uInt8(0x00);

    aCaption.WriteCharArray( *rContents );

    WriteAlign(rContents,4);
    nFixedAreaLen = static_cast<sal_uInt16>(rContents->Tell()-nOldPos-4);
    bRet = aFontData.Export(rContents,rPropSet);
    rContents->Seek(nOldPos);
    *rContents << nStandardId;
    *rContents << nFixedAreaLen;

    *rContents << pBlockFlags[0];
    *rContents << pBlockFlags[1];
    *rContents << pBlockFlags[2];
    *rContents << pBlockFlags[3];
    *rContents << pBlockFlags[4];
    *rContents << pBlockFlags[5];
    *rContents << pBlockFlags[6];
    *rContents << pBlockFlags[7];

    DBG_ASSERT((rContents.Is() &&
        (SVSTREAM_OK==rContents->GetError())),"damn");
    return bRet;
}


sal_Bool OCX_CheckBox::Export(SvStorageRef &rObj,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)

{
    static sal_uInt8 const aCompObj[] = {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0x40, 0x1D, 0xD2, 0x8B,
        0x42, 0xEC, 0xCE, 0x11, 0x9E, 0x0D, 0x00, 0xAA,
        0x00, 0x60, 0x02, 0xF3, 0x1D, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
        0x32, 0x2E, 0x30, 0x20, 0x43, 0x68, 0x65, 0x63,
        0x6B, 0x42, 0x6F, 0x78, 0x00, 0x10, 0x00, 0x00,
        0x00, 0x45, 0x6D, 0x62, 0x65, 0x64, 0x64, 0x65,
        0x64, 0x20, 0x4F, 0x62, 0x6A, 0x65, 0x63, 0x74,
        0x00, 0x11, 0x00, 0x00, 0x00, 0x46, 0x6F, 0x72,
        0x6D, 0x73, 0x2E, 0x43, 0x68, 0x65, 0x63, 0x6B,
        0x42, 0x6F, 0x78, 0x2E, 0x31, 0x00, 0xF4, 0x39,
        0xB2, 0x71, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    {
    SvStorageStreamRef xStor( rObj->OpenSotStream( C2U("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenSotStream( C2U("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 const aOCXNAME[] = {
        0x43, 0x00, 0x68, 0x00, 0x65, 0x00, 0x63, 0x00,
        0x6B, 0x00, 0x42, 0x00, 0x6F, 0x00, 0x78, 0x00,
        0x31, 0x00, 0x00, 0x00, 0x00, 0x00
        };
    {
    SvStorageStreamRef xStor2( rObj->OpenSotStream( C2U("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenSotStream( C2U("contents")));
    return WriteContents(xContents, rPropSet, rSize);
}


sal_Bool OCX_FontData::Export(SvStorageStreamRef &rContent,
    const uno::Reference< beans::XPropertySet > &rPropSet)
{
    sal_uInt8 nFlags=0x00;
    sal_uInt32 nOldPos = rContent->Tell();
    rContent->SeekRel(8);
    SvxOcxString aFontName;
    uno::Any aTmp;

    if (bHasFont)
        aFontName = rPropSet->getPropertyValue(C2U("FontName"));
    if (!aFontName.HasData())
        aFontName = OUString( RTL_CONSTASCII_USTRINGPARAM( "Times New Roman" ) );
    aFontName.WriteLenField( *rContent );
    nFlags |= 0x01;

    if (bHasFont)
    {
        aTmp = rPropSet->getPropertyValue(C2U("FontWeight"));
        float nBold = 0;
        aTmp >>= nBold;

        if (nBold >= 150)
        {
            nFlags |= 0x02;
            sal_uInt8 nTmp=0x01;
            *rContent << nTmp;
            nTmp=0x00;
            *rContent << nTmp;
            *rContent << nTmp;
            *rContent << nTmp;
        }

        aTmp = rPropSet->getPropertyValue(C2U("FontHeight"));
        float nFontHeight = 0;
        aTmp >>= nFontHeight;
        if (nFontHeight)
        {
            nFlags |= 0x04;
            // very strange way of storing font sizes:
            // 1pt->30, 2pt->45, 3pt->60, 4pt->75, 5pt->105, 6pt->120, 7pt->135
            // 8pt->165, 9pt->180, 10pt->195, 11pt->225, ...
            nFontSize = (nFontHeight == 1) ? 30 : (static_cast<sal_uInt32>((nFontHeight*4+1)/3)*15);
            *rContent << nFontSize;
        }

        if (bHasAlign)
        {
            *rContent << sal_uInt16(0x0200);
            nFlags |= 0x10;

            nFlags |= 0x20; // ?

            aTmp = rPropSet->getPropertyValue(C2U("Align"));
            nFlags |= 0x40;
            sal_Int16 nAlign(0);
            if (aTmp.hasValue())
                aTmp >>= nAlign;
            nJustification = ExportAlign(nAlign);
            *rContent << nJustification;
        }
    }

    aFontName.WriteCharArray( *rContent );
    WriteAlign(rContent,4);

    sal_uInt16 nFixedAreaLn = static_cast<sal_uInt16>(rContent->Tell()-nOldPos-4);
    rContent->Seek(nOldPos);
    *rContent << nStandardId;
    *rContent << nFixedAreaLn;
    *rContent << nFlags;
    *rContent << sal_uInt8(0x00);
    *rContent << sal_uInt8(0x00);
    *rContent << sal_uInt8(0x00);

    WriteAlign(rContent,4);
    return sal_True;
}

sal_Bool OCX_Image::WriteContents(SvStorageStreamRef &rContents,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    sal_Bool bRet=sal_True;
    sal_uInt32 nOldPos = rContents->Tell();
    rContents->SeekRel(8);

    pBlockFlags[0] = 0;
    pBlockFlags[1] = 0x02;
    pBlockFlags[2] = 0;
    pBlockFlags[3] = 0;

    uno::Any aTmp = rPropSet->getPropertyValue(
        C2U("BackgroundColor"));
    if (aTmp.hasValue())
        aTmp >>= mnBackColor;
    *rContents << ExportColor(mnBackColor);
    pBlockFlags[0] |= 0x10;


    aTmp = rPropSet->getPropertyValue(C2U("Border"));
    sal_Int16 nBorder = sal_Int16();
    aTmp >>= nBorder;
    nSpecialEffect = ExportBorder(nBorder,nBorderStyle);
    *rContents << nBorderStyle;
    pBlockFlags[0] |= 0x20;

    *rContents << nSpecialEffect;
    pBlockFlags[1] |= 0x01;

    WriteAlign(rContents,4);

    aTmp = rPropSet->getPropertyValue(C2U("Enabled"));
    fEnabled = any2bool(aTmp);
    if (fEnabled)
    {
        sal_uInt8 nTemp = 0x19;
        *rContents << nTemp;
        pBlockFlags[1] |= 0x20;
        *rContents << sal_uInt8(0x00);
        *rContents << sal_uInt8(0x00);
        *rContents << sal_uInt8(0x00);
    }

    aTmp = rPropSet->getPropertyValue(C2U("ImageURL"));
    /*Magically fetch that image and turn it into something that
     *we can store in ms controls, wmf,png,jpg are almost certainly
     *the options we have for export...*/

    WriteAlign(rContents,4);
    *rContents << rSize.Width;
    *rContents << rSize.Height;

    WriteAlign(rContents,4);
    nFixedAreaLen = static_cast<sal_uInt16>(rContents->Tell()-nOldPos-4);

    rContents->Seek(nOldPos);
    *rContents << nStandardId;
    *rContents << nFixedAreaLen;

    *rContents << pBlockFlags[0];
    *rContents << pBlockFlags[1];
    *rContents << pBlockFlags[2];
    *rContents << pBlockFlags[3];
    DBG_ASSERT((rContents.Is() &&
        (SVSTREAM_OK==rContents->GetError())),"damn");
    return bRet;
}


sal_Bool OCX_Image::Export(SvStorageRef &rObj,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    static sal_uInt8 const aCompObj[] = {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0x41, 0x92, 0x59, 0x4C,
        0x26, 0x69, 0x1B, 0x10, 0x99, 0x92, 0x00, 0x00,
        0x0B, 0x65, 0xC6, 0xF9, 0x1A, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
        0x32, 0x2E, 0x30, 0x20, 0x49, 0x6D, 0x61, 0x67,
        0x65, 0x00, 0x10, 0x00, 0x00, 0x00, 0x45, 0x6D,
        0x62, 0x65, 0x64, 0x64, 0x65, 0x64, 0x20, 0x4F,
        0x62, 0x6A, 0x65, 0x63, 0x74, 0x00, 0x0E, 0x00,
        0x00, 0x00, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x2E,
        0x49, 0x6D, 0x61, 0x67, 0x65, 0x2E, 0x31, 0x00,
        0xF4, 0x39, 0xB2, 0x71, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor( rObj->OpenSotStream( C2U("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenSotStream( C2U("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 const aOCXNAME[] = {
        0x49, 0x00, 0x6D, 0x00, 0x61, 0x00, 0x67, 0x00,
        0x65, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor2( rObj->OpenSotStream( C2U("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenSotStream( C2U("contents")));
    return WriteContents(xContents, rPropSet, rSize);
}

// ============================================================================

OCX_SpinButton::OCX_SpinButton() :
    OCX_Control( OUString( RTL_CONSTASCII_USTRINGPARAM( "SpinButton" ) ) ),
    mnBlockFlags( 0 ),
    mnValue( 0 ),
    mnMin( 0 ),
    mnMax( 100 ),
    mnSmallStep( 1 ),
    mnPageStep( 1 ),
    mnOrient( -1 ),
    mnDelay( 50 ),
    mbEnabled( true ),
    mbLocked( false ),
    mbPropThumb( true )
{
    msFormType = C2U("com.sun.star.form.component.SpinButton");
    msDialogType = C2U("com.sun.star.form.component.SpinButton");
    mnBackColor = 0x8000000F;
    mnForeColor = 0x80000012;
}

OCX_Control* OCX_SpinButton::Create()
{
    return new OCX_SpinButton;
}

sal_Bool OCX_SpinButton::Export(
        SvStorageRef &rObj,
        const uno::Reference< beans::XPropertySet>& rPropSet,
        const awt::Size& rSize )
{
    static sal_uInt8 const aCompObj[] =
    {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0xB0, 0x6F, 0x17, 0x79,
        0xF2, 0xB7, 0xCE, 0x11, 0x97, 0xEF, 0x00, 0xAA,
        0x00, 0x6D, 0x27, 0x76, 0x1F, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
        0x32, 0x2E, 0x30, 0x20, 0x53, 0x70, 0x69, 0x6E,
        0x42, 0x75, 0x74, 0x74, 0x6F, 0x6E, 0x00, 0x10,
        0x00, 0x00, 0x00, 0x45, 0x6D, 0x62, 0x65, 0x64,
        0x64, 0x65, 0x64, 0x20, 0x4F, 0x62, 0x6A, 0x65,
        0x63, 0x74, 0x00, 0x13, 0x00, 0x00, 0x00, 0x46,
        0x6E, 0x42, 0x75, 0x74, 0x74, 0x6F, 0x6E, 0x2E,
        0x31, 0x00, 0xF4, 0x39, 0xB2, 0x71, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00
    };

    {
        SvStorageStreamRef xStor( rObj->OpenSotStream( C2U("\1CompObj")));
        xStor->Write(aCompObj,sizeof(aCompObj));
        DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
        SvStorageStreamRef xStor3( rObj->OpenSotStream( C2U("\3ObjInfo")));
        xStor3->Write(aObjInfo,sizeof(aObjInfo));
        DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 const aOCXNAME[] =
    {
        0x53, 0x00, 0x70, 0x00, 0x69, 0x00, 0x6E, 0x00,
        0x42, 0x00, 0x75, 0x00, 0x74, 0x00, 0x74, 0x00,
        0x6F, 0x00, 0x6E, 0x00, 0x31, 0x00, 0x00, 0x00,
        0x00, 0x00
    };

    {
        SvStorageStreamRef xStor2( rObj->OpenSotStream( C2U("\3OCXNAME")));
        xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
        DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenSotStream( C2U("contents")));
    return WriteContents(xContents, rPropSet, rSize);
}

sal_Bool OCX_SpinButton::WriteContents(
        SvStorageStreamRef &rObj,
        const uno::Reference< beans::XPropertySet> &rPropSet,
        const awt::Size& rSize )
{
    if( !rObj.Is() )
        return sal_False;

    mnBlockFlags = 0x00000008;
    nWidth = rSize.Width;
    nHeight = rSize.Height;

    GetInt32Property( mnForeColor, rPropSet, C2U( "SymbolColor" ),     0x00000001 );
    GetInt32Property( mnBackColor, rPropSet, C2U( "BackgroundColor" ), 0x00000002 );
    GetBoolProperty(  mbEnabled,   rPropSet, C2U( "Enabled" ),         0x00000304 );
    GetInt32Property( mnMin,       rPropSet, C2U( "SpinValueMin" ),    0x00000020 );
    GetInt32Property( mnMax,       rPropSet, C2U( "SpinValueMax" ),    0x00000040 );
    GetInt32Property( mnValue,     rPropSet, C2U( "SpinValue" ),       0x00000080 );
    GetInt32Property( mnSmallStep, rPropSet, C2U( "SpinIncrement" ),   0x00000800 );
    GetInt32Property( mnDelay,     rPropSet, C2U( "RepeatDelay" ),     0x00008000 );

    namespace AwtScrollOrient = ::com::sun::star::awt::ScrollBarOrientation;
    sal_Int16 nApiOrient = sal_Int16();
    if( rPropSet->getPropertyValue( C2U( "Orientation" ) ) >>= nApiOrient )
        UpdateInt32Property( mnOrient, (nApiOrient == AwtScrollOrient::VERTICAL) ? 0 : 1, 0x00002000 );

    return WriteData( *rObj );
}

void OCX_SpinButton::UpdateInt32Property(
        sal_Int32& rnCoreValue, sal_Int32 nNewValue, sal_Int32 nBlockFlag )
{
    if( nNewValue != rnCoreValue )
    {
        rnCoreValue = nNewValue;
        mnBlockFlags |= nBlockFlag;
    }
}

void OCX_SpinButton::GetInt32Property(
        sal_Int32& rnCoreValue, const uno::Reference< beans::XPropertySet>& rxPropSet,
        const OUString& rPropName, sal_Int32 nBlockFlag )
{
    sal_Int32 nNewValue = 0;
    if( rxPropSet->getPropertyValue( rPropName ) >>= nNewValue )
        UpdateInt32Property( rnCoreValue, nNewValue, nBlockFlag );
}

void OCX_SpinButton::UpdateBoolProperty(
        bool& rbCoreValue, bool bNewValue, sal_Int32 nBlockFlag )
{
    if( bNewValue != rbCoreValue )
    {
        rbCoreValue = bNewValue;
        mnBlockFlags |= nBlockFlag;
    }
}

void OCX_SpinButton::GetBoolProperty(
        bool& rbCoreValue, const uno::Reference< beans::XPropertySet>& rxPropSet,
        const OUString& rPropName, sal_Int32 nBlockFlag )
{
    UpdateBoolProperty( rbCoreValue,
        any2bool( rxPropSet->getPropertyValue( rPropName ) ), nBlockFlag );
}

sal_Bool OCX_SpinButton::WriteData( SvStream& rStrm ) const
{
    sal_Bool bRet = sal_True;
    sal_uLong nStartPos = rStrm.Tell();

    rStrm << sal_Int32( 0 ) << mnBlockFlags;

    if( mnBlockFlags & 0x00000001 )     rStrm << ExportColor( mnForeColor );
    if( mnBlockFlags & 0x00000002 )     rStrm << ExportColor( mnBackColor );
    if( mnBlockFlags & 0x00000004 )
    {
        sal_Int32 nFlags = 0x00000019;  // always set
        if( mbEnabled ) nFlags |= 0x00000002;
        if( mbLocked )  nFlags |= 0x00000004;
        rStrm << nFlags;
    }
    if( mnBlockFlags & 0x00000020 )     rStrm << mnMin;
    if( mnBlockFlags & 0x00000040 )     rStrm << mnMax;
    if( mnBlockFlags & 0x00000080 )     rStrm << mnValue;
    if( mnBlockFlags & 0x00000100 )     rStrm << sal_Int32( 0 );    // unknown
    if( mnBlockFlags & 0x00000200 )     rStrm << sal_Int32( 0 );    // unknown
    if( mnBlockFlags & 0x00000400 )     rStrm << sal_Int32( 0 );    // unknown
    if( mnBlockFlags & 0x00000800 )     rStrm << mnSmallStep;
    if( mnBlockFlags & 0x00001000 )     rStrm << mnPageStep;
    if( mnBlockFlags & 0x00002000 )     rStrm << mnOrient;
    if( mnBlockFlags & 0x00004000 )     rStrm << sal_Int32( mbPropThumb ? 1 : 0 );
    if( mnBlockFlags & 0x00008000 )     rStrm << mnDelay;
    if( mnBlockFlags & 0x00000008 )     rStrm << nWidth << nHeight;

    sal_uInt16 nSize = static_cast< sal_uInt16 >( rStrm.Tell() - nStartPos - 4 );
    rStrm.Seek( nStartPos );
    rStrm << nStandardId << nSize;

    DBG_ASSERT( rStrm.GetError() == SVSTREAM_OK, "OCX_SpinButton::WriteData - error in stream" );
    return bRet;
}

// ============================================================================

OCX_ScrollBar::OCX_ScrollBar()
{
    sName = OUString( RTL_CONSTASCII_USTRINGPARAM( "ScrollBar" ) );
    mnMax = 32767;
    msFormType = C2U("com.sun.star.form.component.ScrollBar");
    msDialogType = C2U("com.sun.star.form.component.ScrollBar");

}

OCX_Control* OCX_ScrollBar::Create()
{
    return new OCX_ScrollBar;
}

sal_Bool OCX_ScrollBar::Export(
        SvStorageRef &rObj,
        const uno::Reference< beans::XPropertySet>& rPropSet,
        const awt::Size& rSize )
{
    static sal_uInt8 const aCompObj[] =
    {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0xE0, 0x81, 0xD1, 0xDF,
        0x2F, 0x5E, 0xCE, 0x11, 0xA4, 0x49, 0x00, 0xAA,
        0x00, 0x4A, 0x80, 0x3D, 0x1E, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
        0x32, 0x2E, 0x30, 0x20, 0x53, 0x63, 0x72, 0x6F,
        0x6C, 0x6C, 0x42, 0x61, 0x72, 0x00, 0x10, 0x00,
        0x00, 0x00, 0x45, 0x6D, 0x62, 0x65, 0x64, 0x64,
        0x65, 0x64, 0x20, 0x4F, 0x62, 0x6A, 0x65, 0x63,
        0x74, 0x00, 0x12, 0x00, 0x00, 0x00, 0x46, 0x6F,
        0x72, 0x6D, 0x73, 0x2E, 0x53, 0x63, 0x72, 0x6F,
        0x6C, 0x6C, 0x42, 0x61, 0x72, 0x2E, 0x31, 0x00,
        0xF4, 0x39, 0xB2, 0x71, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    {
        SvStorageStreamRef xStor( rObj->OpenSotStream( C2U("\1CompObj")));
        xStor->Write(aCompObj,sizeof(aCompObj));
        DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
        SvStorageStreamRef xStor3( rObj->OpenSotStream( C2U("\3ObjInfo")));
        xStor3->Write(aObjInfo,sizeof(aObjInfo));
        DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 const aOCXNAME[] =
    {
        0x53, 0x00, 0x63, 0x00, 0x72, 0x00, 0x6F, 0x00,
        0x6C, 0x00, 0x6C, 0x00, 0x42, 0x00, 0x61, 0x00,
        0x72, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    {
        SvStorageStreamRef xStor2( rObj->OpenSotStream( C2U("\3OCXNAME")));
        xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
        DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenSotStream( C2U("contents")));
    return WriteContents(xContents, rPropSet, rSize);
}

sal_Bool OCX_ScrollBar::WriteContents(
        SvStorageStreamRef &rObj,
        const uno::Reference< beans::XPropertySet> &rPropSet,
        const awt::Size& rSize )
{
    if( !rObj.Is() )
        return sal_False;

    mnBlockFlags = 0x00000008;
    nWidth = rSize.Width;
    nHeight = rSize.Height;

    GetInt32Property( mnForeColor, rPropSet, C2U( "SymbolColor" ),     0x00000001 );
    GetInt32Property( mnBackColor, rPropSet, C2U( "BackgroundColor" ), 0x00000002 );
    GetBoolProperty(  mbEnabled,   rPropSet, C2U( "Enabled" ),         0x00000304 );
    GetInt32Property( mnMin,       rPropSet, C2U( "ScrollValueMin" ),  0x00000020 );
    GetInt32Property( mnMax,       rPropSet, C2U( "ScrollValueMax" ),  0x00000040 );
    GetInt32Property( mnValue,     rPropSet, C2U( "ScrollValue" ),     0x00000080 );
    GetInt32Property( mnSmallStep, rPropSet, C2U( "LineIncrement" ),   0x00000800 );
    GetInt32Property( mnPageStep,  rPropSet, C2U( "BlockIncrement" ),  0x00001000 );
    GetInt32Property( mnDelay,     rPropSet, C2U( "RepeatDelay" ),     0x00008000 );

    namespace AwtScrollOrient = ::com::sun::star::awt::ScrollBarOrientation;
    sal_Int16 nApiOrient = sal_Int16();
    if( rPropSet->getPropertyValue( C2U( "Orientation" ) ) >>= nApiOrient )
        UpdateInt32Property( mnOrient, (nApiOrient == AwtScrollOrient::VERTICAL) ? 0 : 1, 0x00002000 );

    UpdateBoolProperty( mbPropThumb, true, 0x00004000 );

    return WriteData( *rObj );
}

OCX_ProgressBar::OCX_ProgressBar() :
    OCX_Control( OUString( RTL_CONSTASCII_USTRINGPARAM( "ProgressBar" ) ) ),
    nMin( 0 ),
    nMax( 0 ),
    bFixedSingle(true),
    bEnabled( true ),
    b3d( true )
{
    msDialogType = C2U("com.sun.star.awt.UnoControlProgressBarModel");
    bSetInDialog = true;
}

OCX_Control* OCX_ProgressBar::Create()
{
    return new OCX_ProgressBar;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
