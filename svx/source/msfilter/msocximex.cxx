/*************************************************************************
 *
 *  $RCSfile: msocximex.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: rt $ $Date: 2004-05-18 12:41:18 $
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

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif
#ifndef _COM_SUN_STAR_TEXT_VERTORIENTATION_HPP_
#include <com/sun/star/text/VertOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HDL_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_TEXTCONTENTANCHORTYPE_HPP_
#include <com/sun/star/text/TextContentAnchorType.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGE_HPP_
#include <com/sun/star/drawing/XDrawPage.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XCONTROLSHAPE_HPP_
#include <com/sun/star/drawing/XControlShape.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESUPPLIER_HPP_
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMSSUPPLIER_HPP_
#include <com/sun/star/form/XFormsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORM_HPP_
#include <com/sun/star/form/XForm.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMCOMPONENTTYPE_HPP_
#include <com/sun/star/form/FormComponentType.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTWEIGHT_HPP_
#include <com/sun/star/awt/FontWeight.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTSLANT_HPP_
#include <com/sun/star/awt/FontSlant.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTUNDERLINE_HPP_
#include <com/sun/star/awt/FontUnderline.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTSTRIKEOUT_HPP_
#include <com/sun/star/awt/FontStrikeout.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_SCROLLBARORIENTATION_HPP_
#include <com/sun/star/awt/ScrollBarOrientation.hpp>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SFX_OBJSH_HXX
#include <sfx2/objsh.hxx>
#endif
#ifndef _MSOCXIMEX_HXX
#include "msocximex.hxx"
#endif

#ifndef C2S
#define C2S(cChar)  String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(cChar))
#endif

using namespace ::com::sun::star;
#if 0
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
#endif
using namespace ::rtl;
using namespace cppu;


#define WW8_ASCII2STR(s) String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(s))


static char sWW8_form[] = "WW-Standard";


sal_uInt8 __READONLY_DATA OCX_Control::aObjInfo[4] = { 0x00, 0x12, 0x03, 0x00 };


void Align(SvStorageStream *pS,int nAmount,BOOL bFill=FALSE)
{
    if (pS->Tell()%nAmount)
    {
        if (bFill == FALSE)
            pS->SeekRel(nAmount-(pS->Tell()%nAmount));
        else
        {
        int nLen = nAmount-(pS->Tell()%nAmount);
        for(int i=0;i<nLen;i++)
            *pS << sal_uInt8(0x00);
        }
    }
}

// string import/export =======================================================

/*  #110435# (DR, 2003-11-12) ** Import of Unicode strings in form controls **

    Strings may be stored either as compressed or uncompressed Unicode
    character array. There are no encoded byte strings anywhere.

    The string length field stores the length of the character array (not the
    character count of the string) in the lower 31 bits, and the compression
    state in the highest bit.

    A set bit means the character array is compressed. This means all Unicode
    characters are <=0xFF. Therefore the high bytes of all characters are left
    out, and the character array size is equal to the string length.

    A cleared bit means the character array is not compressed. The buffer
    contains Little-Endian Unicode characters, and the resulting string length
    is half the buffer size.

    TODO: This implementation of the new string import is a hack to keep
    msocximex.hxx unchanged. A better implementation would replace the char*
    members of all classes by something more reasonable.
 */

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


// import ---------------------------------------------------------------------

/** Reads the character array of a string in a form control.

    Creates a new character array containing the character data.
    The length field must be read before and passed to this function.
    Aligns stream position to multiple of 4 before.

    @param rStrm
        The input stream.

    @param rpcCharArr
        (out-param) Will point to the created character array,
        or will be 0 if string is empty. The array is NOT null-terminated.
        If the passed pointer points to an old existing array, it will be
        deleted before. Caller must delete the returned array.

    @param nLenFld
        The corresponding string length field read somewhere before.
 */
void lclReadCharArray( SvStorageStream& rStrm, char*& rpcCharArr, sal_uInt32 nLenFld )
{
    delete[] rpcCharArr;
    sal_uInt32 nBufSize = lclGetBufferSize( nLenFld );
    if( nBufSize )
    {
        rpcCharArr = new char[ nBufSize ];
        Align( &rStrm, 4 );
        rStrm.Read( rpcCharArr, nBufSize );
    }
    else
        rpcCharArr = 0;
}


/** Creates an OUString from a character array created with lclReadCharArray().

    The passed parameters must match, that means the length field must be the
    same used to create the passed character array.

    @param pcCharArr
        The character array returned by lclReadCharArray(). May be compressed
        or uncompressed, next parameter nLenFld will specify this.

    @param nLenFld
        MUST be the same string length field that has been passed to
        lclReadCharArray() to create the character array in previous parameter
        pcCharArr.

    @return
        An OUString containing the decoded string data. Will be empty if
        pcCharArr is 0.
 */
OUString lclCreateOUString( const char* pcCharArr, sal_uInt32 nLenFld )
{
    OUStringBuffer aBuffer;
    sal_uInt32 nBufSize = lclGetBufferSize( nLenFld );
    if( lclIsCompressed( nLenFld ) )
    {
        // buffer contains compressed Unicode, not encoded bytestring
        sal_Int32 nStrLen = static_cast< sal_Int32 >( nBufSize );
        aBuffer.setLength( nStrLen );
        const char* pcCurrChar = pcCharArr;
        for( sal_Int32 nChar = 0; nChar < nStrLen; ++nChar, ++pcCurrChar )
            /*  *pcCurrChar may contain negative values and therefore MUST be
                casted to unsigned char, before assigned to a sal_Unicode. */
            aBuffer.setCharAt( nChar, static_cast< unsigned char >( *pcCurrChar ) );
    }
    else
    {
        // buffer contains Little-Endian Unicode
        sal_Int32 nStrLen = static_cast< sal_Int32 >( nBufSize ) / 2;
        aBuffer.setLength( nStrLen );
        const char* pcCurrChar = pcCharArr;
        for( sal_Int32 nChar = 0; nChar < nStrLen; ++nChar )
        {
            /*  *pcCurrChar may contain negative values and therefore MUST be
                casted to unsigned char, before assigned to a sal_Unicode. */
            sal_Unicode cChar = static_cast< unsigned char >( *pcCurrChar++ );
            cChar |= (static_cast< unsigned char >( *pcCurrChar++ ) << 8);
            aBuffer.setCharAt( nChar, cChar );
        }
    }
    return aBuffer.makeStringAndClear();
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
        Align( &rStrm, 4, TRUE );
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

        Align( &rStrm, 4, TRUE );
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

sal_uInt32 OCX_Control::ImportColor(sal_uInt32 nColor) const
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
    else
    {
        //Stored in bgr! rather than rgb
        nColor = SwapColor(nColor);
    }
    return nColor;
}

sal_Int16 OCX_FontData::ImportAlign(sal_uInt8 nJustification) const
{
    sal_Int16 nRet;
    switch (nJustification)
    {
    default:
    case 1:
        nRet = 0;
        break;
    case 2:
        nRet = 2;
        break;
    case 3:
        nRet = 1;
        break;
    }
    return nRet;
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


sal_Int16 OCX_Control::ImportBorder(sal_uInt16 nSpecialEffect,
    sal_uInt16 nBorderStyle) const
{
    if ((nSpecialEffect == 0) && (nBorderStyle == 0))
        return 0;   //No Border
    else if ((nSpecialEffect == 0) && (nBorderStyle == 1))
        return 2;   //Flat Border
    return 1;   //3D Border
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
            rBorderStyle = 1;
            break;
        case 2:
            nRet = 3;
            rBorderStyle = 1;
            break;
    }
    return nRet;
}

sal_Bool OCX_Control::ReadFontData(SvStorageStream *pS)
{
    return aFontData.Read(pS);
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
                rServiceFactory->createInstance(WW8_ASCII2STR(
                    "com.sun.star.form.component.Form"));
            if( xCreate.is() )
            {
                uno::Reference< beans::XPropertySet > xFormPropSet( xCreate,
                    uno::UNO_QUERY );

                uno::Any aTmp(&sName,getCppuType((OUString *)0));
                xFormPropSet->setPropertyValue( WW8_ASCII2STR("Name"), aTmp );

                uno::Reference< form::XForm > xForm( xCreate, uno::UNO_QUERY );
                DBG_ASSERT(xForm.is(), "keine Form?")

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

sal_Bool OCX_CommandButton::Import(
    const uno::Reference< lang::XMultiServiceFactory > &rServiceFactory,
    uno::Reference< form::XFormComponent >  &rFComp,
    awt::Size &rSz)
{
    OUString sServiceName =
        WW8_ASCII2STR("com.sun.star.form.component.CommandButton");
    uno::Reference< uno::XInterface > xCreate =
        rServiceFactory->createInstance( sServiceName );
    if( !xCreate.is() )
        return sal_False;

    rFComp = uno::Reference< form::XFormComponent > (xCreate,uno::UNO_QUERY);
    if( !rFComp.is() )
        return sal_False;

    uno::Reference< beans::XPropertySet >  xPropSet( xCreate, uno::UNO_QUERY );

    rSz.Width = nWidth;
    rSz.Height = nHeight;

    uno::Any aTmp(&sName,getCppuType((OUString *)0));
    xPropSet->setPropertyValue( WW8_ASCII2STR("Name"), aTmp );

    aTmp <<= ImportColor(nForeColor);
    xPropSet->setPropertyValue( WW8_ASCII2STR("TextColor"), aTmp);

    aTmp <<= ImportColor(nBackColor);
    xPropSet->setPropertyValue( WW8_ASCII2STR("BackgroundColor"), aTmp);

    sal_Bool bTemp;
    if ((!(fEnabled)) || (fLocked))
        bTemp = sal_False;
    else
        bTemp = sal_True;
    aTmp = bool2any(bTemp);

    xPropSet->setPropertyValue( WW8_ASCII2STR("Enabled"), aTmp);

    if (pCaption)
    {
        aTmp <<= lclCreateOUString( pCaption, nCaptionLen );
        xPropSet->setPropertyValue( WW8_ASCII2STR("Label"), aTmp);
    }

    aFontData.Import(xPropSet);
    return sal_True;
}

sal_Bool OCX_GroupBox::Export(SvStorageRef &rObj,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    sal_Bool bRet=sal_True;
    return bRet;
}

sal_Bool OCX_GroupBox::WriteContents(SvStorageStreamRef &rObj,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    sal_Bool bRet=sal_True;
    return bRet;
}

sal_Bool OCX_CommandButton::WriteContents(SvStorageStreamRef &rContents,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    sal_Bool bRet=sal_True;

    sal_uInt32 nOldPos = rContents->Tell();
    rContents->SeekRel(8);

    uno::Any aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("TextColor"));
    if (aTmp.hasValue())
        aTmp >>= nForeColor;
    *rContents << ExportColor(nForeColor);

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("BackgroundColor"));
    if (aTmp.hasValue())
        aTmp >>= nBackColor;
    *rContents << ExportColor(nBackColor);

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Enabled"));
    fEnabled = any2bool(aTmp);
    sal_uInt8 nTemp=0;//fEnabled;
    if (fEnabled)
        nTemp |= 0x02;
    *rContents << nTemp;
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);

    SvxOcxString aCaption( rPropSet->getPropertyValue(WW8_ASCII2STR("Label")) );
    aCaption.WriteLenField( *rContents );
    aCaption.WriteCharArray( *rContents );

    Align(rContents,4,TRUE);

    *rContents << rSize.Width;
    *rContents << rSize.Height;

    nFixedAreaLen = static_cast<sal_uInt16>(rContents->Tell()-nOldPos-4);

    bRet = aFontData.Export(rContents,rPropSet);

    rContents->Seek(nOldPos);
    *rContents << nStandardId;
    *rContents << nFixedAreaLen;

    sal_uInt8 nTmp = 0x27;
    if (aCaption.HasData())
        nTmp |= 0x08;
    *rContents << nTmp;
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);

    DBG_ASSERT((rContents.Is() && (SVSTREAM_OK==rContents->GetError())),"damn");
    return bRet;
}



sal_Bool OCX_CommandButton::Export(SvStorageRef &rObj,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    static sal_uInt8 __READONLY_DATA aCompObj[] = {
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
    SvStorageStreamRef xStor( rObj->OpenStream( C2S("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenStream( C2S("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 __READONLY_DATA aOCXNAME[] = {
        0x43, 0x00, 0x6F, 0x00, 0x6D, 0x00, 0x6D, 0x00,
        0x61, 0x00, 0x6E, 0x00, 0x64, 0x00, 0x42, 0x00,
        0x75, 0x00, 0x74, 0x00, 0x74, 0x00, 0x6F, 0x00,
        0x6E, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    {
    SvStorageStreamRef xStor2( rObj->OpenStream( C2S("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenStream( C2S("contents")));

    return WriteContents(xContents,rPropSet,rSize);
}

sal_Bool OCX_ImageButton::WriteContents(SvStorageStreamRef &rContents,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    sal_Bool bRet=sal_True;

    sal_uInt32 nOldPos = rContents->Tell();
    rContents->SeekRel(8);

    uno::Any aTmp=rPropSet->getPropertyValue(WW8_ASCII2STR("BackgroundColor"));
    if (aTmp.hasValue())
        aTmp >>= nBackColor;
    *rContents << ExportColor(nBackColor);

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Enabled"));
    fEnabled = any2bool(aTmp);
    sal_uInt8 nTemp=0;//fEnabled;
    if (fEnabled)
        nTemp |= 0x02;
    *rContents << nTemp;
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);

    Align(rContents,4,TRUE);

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
    static sal_uInt8 __READONLY_DATA aCompObj[] = {
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
    SvStorageStreamRef xStor( rObj->OpenStream( C2S("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenStream( C2S("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 __READONLY_DATA aOCXNAME[] = {
        0x43, 0x00, 0x6F, 0x00, 0x6D, 0x00, 0x6D, 0x00,
        0x61, 0x00, 0x6E, 0x00, 0x64, 0x00, 0x42, 0x00,
        0x75, 0x00, 0x74, 0x00, 0x74, 0x00, 0x6F, 0x00,
        0x6E, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    {
    SvStorageStreamRef xStor2( rObj->OpenStream( C2S("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenStream( C2S("contents")));
    return WriteContents(xContents,rPropSet,rSize);
}


sal_Bool OCX_OptionButton::Import(
    const uno::Reference< lang::XMultiServiceFactory > &rServiceFactory,
    uno::Reference< form::XFormComponent > &rFComp, awt::Size &rSz)
{
    OUString sServiceName =
        WW8_ASCII2STR("com.sun.star.form.component.RadioButton");
    uno::Reference< uno::XInterface >  xCreate =
        rServiceFactory->createInstance( sServiceName );
    if( !xCreate.is() )
        return sal_False;

    rFComp = uno::Reference< form::XFormComponent > (xCreate,uno::UNO_QUERY);
    if( !rFComp.is() )
        return sal_False;

    uno::Reference< beans::XPropertySet >  xPropSet(xCreate,uno::UNO_QUERY);

    rSz.Width = nWidth;
    rSz.Height = nHeight;

    uno::Any aTmp(&sName,getCppuType((OUString *)0));
    xPropSet->setPropertyValue( WW8_ASCII2STR("Name"), aTmp );

    sal_Bool bTemp;
    if ((!(fEnabled)) || (fLocked))
        bTemp = sal_False;
    else
        bTemp = sal_True;
    aTmp = bool2any(bTemp);
    xPropSet->setPropertyValue( WW8_ASCII2STR("Enabled"), aTmp);

    aTmp <<= ImportColor(nForeColor);
    xPropSet->setPropertyValue( WW8_ASCII2STR("TextColor"), aTmp);

    if (pValue)
    {
        INT16 nTmp = pValue[0]-0x30;
        aTmp <<= nTmp;
        xPropSet->setPropertyValue( WW8_ASCII2STR("DefaultState"), aTmp);
    }

    if (pCaption)
    {
        aTmp <<= lclCreateOUString( pCaption, nCaptionLen );
        xPropSet->setPropertyValue( WW8_ASCII2STR("Label"), aTmp);
    }

    aFontData.Import(xPropSet);
    return sal_True;
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

    uno::Any aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Enabled"));
    fEnabled = any2bool(aTmp);
    sal_uInt8 nTemp=0;//=fEnabled;
    if (fEnabled)
        nTemp |= 0x02;
    *rContents << nTemp;
    pBlockFlags[0] |= 0x01;
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);


    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("TextColor"));
    if (aTmp.hasValue())
        aTmp >>= nForeColor;
    *rContents << ExportColor(nForeColor);
    pBlockFlags[0] |= 0x04;

    nStyle = 5;
    *rContents << nStyle;
    pBlockFlags[0] |= 0x40;

    Align(rContents,4,TRUE);
    nValueLen = 1|SVX_MSOCX_COMPRESSED;
    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("DefaultState"));
    INT16 nDefault;
    aTmp >>= nDefault;
    *rContents << nValueLen;
    pBlockFlags[2] |= 0x40;


    SvxOcxString aCaption( rPropSet->getPropertyValue(WW8_ASCII2STR("Label")) );
    if (aCaption.HasData())
        pBlockFlags[2] |= 0x80;
    aCaption.WriteLenField( *rContents );

    Align(rContents,4,TRUE);
    *rContents << rSize.Width;
    *rContents << rSize.Height;

    nDefault += 0x30;
    *rContents << sal_uInt8(nDefault);
    *rContents << sal_uInt8(0x00);

    aCaption.WriteCharArray( *rContents );

    Align(rContents,4,TRUE);
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
    static sal_uInt8 __READONLY_DATA aCompObj[] = {
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
    SvStorageStreamRef xStor( rObj->OpenStream( C2S("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenStream( C2S("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 __READONLY_DATA aOCXNAME[] = {
            0x4F, 0x00, 0x70, 0x00, 0x74, 0x00, 0x69, 0x00,
            0x6F, 0x00, 0x6E, 0x00, 0x42, 0x00, 0x75, 0x00,
            0x74, 0x00, 0x74, 0x00, 0x6F, 0x00, 0x6E, 0x00,
            0x31, 0x00, 0x00, 0x00, 0x00, 0x00
            };

    {
    SvStorageStreamRef xStor2( rObj->OpenStream( C2S("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenStream( C2S("contents")));
    return WriteContents(xContents, rPropSet, rSize);
}


sal_Bool OCX_TextBox::Import(
    const uno::Reference< lang::XMultiServiceFactory > &rServiceFactory,
    uno::Reference< form::XFormComponent >  &rFComp, awt::Size &rSz)
{
    OUString sServiceName =
        WW8_ASCII2STR("com.sun.star.form.component.TextField");
    uno::Reference< uno::XInterface >  xCreate =
        rServiceFactory->createInstance( sServiceName );
    if( !xCreate.is() )
        return sal_False;

    rFComp = uno::Reference< form::XFormComponent > (xCreate,uno::UNO_QUERY);
    if( !rFComp.is() )
        return sal_False;

    uno::Reference< beans::XPropertySet >  xPropSet( xCreate, uno::UNO_QUERY );

    rSz.Width = nWidth;
    rSz.Height = nHeight;

    uno::Any aTmp(&sName,getCppuType((OUString *)0));
    xPropSet->setPropertyValue( WW8_ASCII2STR("Name"), aTmp );

    sal_Bool bTemp;
    if (fEnabled)
        bTemp = sal_True;
    else
        bTemp = sal_False;
    aTmp = bool2any(bTemp);
    xPropSet->setPropertyValue( WW8_ASCII2STR("Enabled"), aTmp);

    if (fLocked)
        bTemp = sal_True;
    else
        bTemp = sal_False;
    aTmp = bool2any(bTemp);
    xPropSet->setPropertyValue( WW8_ASCII2STR("ReadOnly"), aTmp);


    aTmp <<= ImportColor(nForeColor);
    xPropSet->setPropertyValue( WW8_ASCII2STR("TextColor"), aTmp);

    aTmp <<= ImportColor(nBackColor);
    xPropSet->setPropertyValue( WW8_ASCII2STR("BackgroundColor"), aTmp);

    sal_Bool bTmp=fMultiLine;
    aTmp = bool2any(bTmp);
    xPropSet->setPropertyValue( WW8_ASCII2STR("MultiLine"), aTmp);

    sal_uInt16 nTmp = static_cast<sal_uInt16>(nMaxLength);
    aTmp <<= nTmp;
    xPropSet->setPropertyValue( WW8_ASCII2STR("MaxTextLen"), aTmp);


    sal_Bool bTemp1,bTemp2;
    uno::Any aBarsH,aBarsV;
    switch(nScrollBars)
    {
        case 1:
            bTemp1 = sal_True;
            bTemp2 = sal_False;
            break;
        case 2:
            bTemp1 = sal_False;
            bTemp2 = sal_True;
            break;
        case 3:
            bTemp1 = sal_True;
            bTemp2 = sal_True;
            break;
        case 0:
        default:
            bTemp1 = sal_False;
            bTemp2 = sal_False;
            break;
    }

    aBarsH = bool2any(bTemp1);
    aBarsV = bool2any(bTemp2);
    xPropSet->setPropertyValue( WW8_ASCII2STR("HScroll"), aBarsH);
    xPropSet->setPropertyValue( WW8_ASCII2STR("VScroll"), aBarsV);

    nTmp = nPasswordChar;
    aTmp <<= nTmp;
    xPropSet->setPropertyValue( WW8_ASCII2STR("EchoChar"), aTmp);


    aTmp <<= ImportBorder(nSpecialEffect,nBorderStyle);
    xPropSet->setPropertyValue( WW8_ASCII2STR("Border"), aTmp);


    if (pValue)
    {
        aTmp <<= lclCreateOUString( pValue, nValueLen );
        xPropSet->setPropertyValue( WW8_ASCII2STR("DefaultText"), aTmp);
    }

    aFontData.Import(xPropSet);
    return sal_True;
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
    uno::Any aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Enabled"));
    fEnabled = any2bool(aTmp);
    if (fEnabled)
        nTemp |= 0x02;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("ReadOnly"));
    fLocked = any2bool(aTmp);
    if (fLocked)
        nTemp |= 0x04;

    *rContents << nTemp;
    pBlockFlags[0] |= 0x01;
    *rContents << sal_uInt8(0x48);
    *rContents << sal_uInt8(0x80);

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("MultiLine"));
    fMultiLine = any2bool(aTmp);
    nTemp = 0x2C;
    if (fMultiLine)
        nTemp |= 0x80;
    *rContents << nTemp;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("BackgroundColor"));
    if (aTmp.hasValue())
        aTmp >>= nBackColor;
    *rContents << ExportColor(nBackColor);
    pBlockFlags[0] |= 0x02;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("TextColor"));
    if (aTmp.hasValue())
        aTmp >>= nForeColor;
    *rContents << ExportColor(nForeColor);
    pBlockFlags[0] |= 0x04;

    aTmp = rPropSet->getPropertyValue( WW8_ASCII2STR("MaxTextLen"));
    aTmp >>= nMaxLength;
    *rContents << nMaxLength;
    pBlockFlags[0] |= 0x08;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Border"));
    sal_Int16 nBorder;
    aTmp >>= nBorder;
    nSpecialEffect = ExportBorder(nBorder,nBorderStyle);
    *rContents << nBorderStyle;
    pBlockFlags[0] |= 0x10;

    aTmp = rPropSet->getPropertyValue( WW8_ASCII2STR("HScroll"));
    sal_Bool bTemp1 = any2bool(aTmp);
    aTmp = rPropSet->getPropertyValue( WW8_ASCII2STR("VScroll"));
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

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("EchoChar"));
    sal_uInt16 nTmp;
    aTmp >>= nTmp;
    nPasswordChar = static_cast<sal_uInt8>(nTmp);
    *rContents << nPasswordChar;
    pBlockFlags[1] |= 0x02;

    SvxOcxString aValue( rPropSet->getPropertyValue(WW8_ASCII2STR("DefaultText")) );
    aValue.WriteLenField( *rContents );
    if (aValue.HasData())
        pBlockFlags[2] |= 0x40;

    Align(rContents,4,TRUE);
    *rContents << nSpecialEffect;
    pBlockFlags[3] |= 0x04;

    Align(rContents,4,TRUE);
    *rContents << rSize.Width;
    *rContents << rSize.Height;

    aValue.WriteCharArray( *rContents );

    Align(rContents,4,TRUE);

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
    static sal_uInt8 __READONLY_DATA aCompObj[] = {
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
    SvStorageStreamRef xStor( rObj->OpenStream( C2S("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenStream( C2S("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 __READONLY_DATA aOCXNAME[] = {
        0x54, 0x00, 0x65, 0x00, 0x78, 0x00, 0x74, 0x00,
        0x42, 0x00, 0x6F, 0x00, 0x78, 0x00, 0x31, 0x00,
        0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor2( rObj->OpenStream( C2S("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenStream( C2S("contents")));
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
    uno::Any aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Enabled"));
    fEnabled = any2bool(aTmp);
    if (fEnabled)
        nTemp |= 0x02;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("ReadOnly"));
    fLocked = any2bool(aTmp);
    if (fLocked)
        nTemp |= 0x04;

    *rContents << nTemp;
    pBlockFlags[0] |= 0x01;
    *rContents << sal_uInt8(0x48);
    *rContents << sal_uInt8(0x80);

    nTemp = 0x2C;
    *rContents << nTemp;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("BackgroundColor"));
    if (aTmp.hasValue())
        aTmp >>= nBackColor;
    *rContents << ExportColor(nBackColor);
    pBlockFlags[0] |= 0x02;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("TextColor"));
    if (aTmp.hasValue())
        aTmp >>= nForeColor;
    *rContents << ExportColor(nForeColor);
    pBlockFlags[0] |= 0x04;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Border"));
    sal_Int16 nBorder;
    aTmp >>= nBorder;
    nSpecialEffect = ExportBorder(nBorder,nBorderStyle);
    *rContents << nBorderStyle;
    pBlockFlags[0] |= 0x10;

#if 0 //Each control has a different Value format, and how to convert each to text has to be found out
    SvxOcxString aValue( rPropSet->getPropertyValue(WW8_ASCII2STR("DefaultText")) );
    aValue.WriteLenField( *rContents );
    if (aValue.HasData())
        pBlockFlags[2] |= 0x40;
#endif

    Align(rContents,4,TRUE);
    *rContents << nSpecialEffect;
    pBlockFlags[3] |= 0x04;

    Align(rContents,4,TRUE);
    *rContents << rSize.Width;
    *rContents << rSize.Height;

#if 0
    aValue.WriteCharArray( *rContents );
#endif

    Align(rContents,4,TRUE);

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
    static sal_uInt8 __READONLY_DATA aCompObj[] = {
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
    SvStorageStreamRef xStor( rObj->OpenStream( C2S("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenStream( C2S("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 __READONLY_DATA aOCXNAME[] = {
        0x54, 0x00, 0x65, 0x00, 0x78, 0x00, 0x74, 0x00,
        0x42, 0x00, 0x6F, 0x00, 0x78, 0x00, 0x31, 0x00,
        0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor2( rObj->OpenStream( C2S("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenStream( C2S("contents")));
    return WriteContents(xContents, rPropSet, rSize);
}



sal_Bool OCX_ToggleButton::Import(
    const uno::Reference< lang::XMultiServiceFactory > &rServiceFactory,
    uno::Reference< form::XFormComponent > &rFComp, awt::Size &rSz)
{

    OUString sServiceName =
        WW8_ASCII2STR("com.sun.star.form.component.CheckBox");
    uno::Reference< uno::XInterface >  xCreate =
        rServiceFactory->createInstance( sServiceName );
    if( !xCreate.is() )
        return sal_False;

    rFComp = uno::Reference< form::XFormComponent > (xCreate,uno::UNO_QUERY);
    if( !rFComp.is() )
        return sal_False;

    uno::Reference< beans::XPropertySet >  xPropSet(xCreate,uno::UNO_QUERY);

    rSz.Width = nWidth;
    rSz.Height = nHeight;

    uno::Any aTmp(&sName,getCppuType((OUString *)0));
    xPropSet->setPropertyValue( WW8_ASCII2STR("Name"), aTmp );

    sal_Bool bTemp;
    if ((!(fEnabled)) || (fLocked))
        bTemp = sal_False;
    else
        bTemp = sal_True;
    aTmp = bool2any(bTemp);
    xPropSet->setPropertyValue( WW8_ASCII2STR("Enabled"), aTmp);

    bTemp = nMultiState;
    aTmp = bool2any(bTemp);
    xPropSet->setPropertyValue( WW8_ASCII2STR("TriState"), aTmp);

    aTmp <<= ImportColor(nForeColor);
    xPropSet->setPropertyValue( WW8_ASCII2STR("TextColor"), aTmp);

    if (pValue)
    {
        INT16 nTmp=pValue[0]-0x30;
        aTmp <<= nTmp;
        xPropSet->setPropertyValue( WW8_ASCII2STR("DefaultState"), aTmp);
    }

    if (pCaption)
    {
        aTmp <<= lclCreateOUString( pCaption, nCaptionLen );
        xPropSet->setPropertyValue( WW8_ASCII2STR("Label"), aTmp);
    }

    aFontData.Import(xPropSet);
    return sal_True;
}

sal_Bool OCX_Label::Import(
    const uno::Reference< lang::XMultiServiceFactory > &rServiceFactory,
    uno::Reference< form::XFormComponent >  &rFComp,awt::Size &rSz)
{
    OUString sServiceName = WW8_ASCII2STR(
        "com.sun.star.form.component.FixedText");
    uno::Reference< uno::XInterface >  xCreate =
        rServiceFactory->createInstance( sServiceName );
    if( !xCreate.is() )
        return sal_False;

    rFComp = uno::Reference< form::XFormComponent > (xCreate,uno::UNO_QUERY);
    if( !rFComp.is() )
        return sal_False;

    uno::Reference< beans::XPropertySet >  xPropSet(xCreate,uno::UNO_QUERY);

    rSz.Width = nWidth;
    rSz.Height = nHeight;

    uno::Any aTmp(&sName,getCppuType((OUString *)0));
    xPropSet->setPropertyValue( WW8_ASCII2STR("Name"), aTmp );

    sal_Bool bTemp;
    if ((!(fEnabled)) || (fLocked))
        bTemp = sal_False;
    else
        bTemp = sal_True;
    aTmp = bool2any(bTemp);
    xPropSet->setPropertyValue( WW8_ASCII2STR("Enabled"), aTmp);

    aTmp <<= ImportColor(nForeColor);
    xPropSet->setPropertyValue( WW8_ASCII2STR("TextColor"), aTmp);

    aTmp <<= ImportColor(nBackColor);
    xPropSet->setPropertyValue( WW8_ASCII2STR("BackgroundColor"), aTmp);

    aTmp <<= ImportBorder(nSpecialEffect,nBorderStyle);
    xPropSet->setPropertyValue( WW8_ASCII2STR("Border"), aTmp);

    bTemp=fWordWrap;
    aTmp = bool2any(bTemp);
    xPropSet->setPropertyValue( WW8_ASCII2STR("MultiLine"), aTmp);

    if (pCaption)
    {
        aTmp <<= lclCreateOUString( pCaption, nCaptionLen );
        xPropSet->setPropertyValue( WW8_ASCII2STR("Label"), aTmp);
    }

    aFontData.Import(xPropSet);
    return sal_True;
}

sal_Bool OCX_ComboBox::Import(
    const uno::Reference< lang::XMultiServiceFactory >  &rServiceFactory,
    uno::Reference< form::XFormComponent > &rFComp, awt::Size &rSz)
{

    OUString sServiceName = WW8_ASCII2STR(
        "com.sun.star.form.component.ComboBox");
    uno::Reference< uno::XInterface > xCreate =
        rServiceFactory->createInstance( sServiceName );
    if( !xCreate.is() )
        return sal_False;

    rFComp = uno::Reference< form::XFormComponent > (xCreate,uno::UNO_QUERY);
    if( !rFComp.is() )
        return sal_False;

    uno::Reference< beans::XPropertySet >  xPropSet( xCreate, uno::UNO_QUERY );

    rSz.Width = nWidth;
    rSz.Height = nHeight;

    uno::Any aTmp(&sName,getCppuType((OUString *)0));
    xPropSet->setPropertyValue( WW8_ASCII2STR("Name"), aTmp );

    sal_Bool bTmp=fEnabled;
    aTmp = bool2any(bTmp);
    xPropSet->setPropertyValue( WW8_ASCII2STR("Enabled"), aTmp);

    bTmp=fLocked;
    aTmp = bool2any(bTmp);
    xPropSet->setPropertyValue( WW8_ASCII2STR("ReadOnly"), aTmp);

    if (nDropButtonStyle)
        bTmp=sal_True;
    else
        bTmp=sal_False;
    aTmp = bool2any(bTmp);
    xPropSet->setPropertyValue( WW8_ASCII2STR("Dropdown"), aTmp);

    aTmp <<= ImportColor(nForeColor);
    xPropSet->setPropertyValue( WW8_ASCII2STR("TextColor"), aTmp);

    if (pValue)
    {
        aTmp <<= lclCreateOUString( pValue, nValueLen );
        xPropSet->setPropertyValue( WW8_ASCII2STR("DefaultText"), aTmp);
    }

    aTmp <<= ImportColor(nBackColor);
    xPropSet->setPropertyValue( WW8_ASCII2STR("BackgroundColor"), aTmp);

    aTmp <<= ImportBorder(nSpecialEffect,nBorderStyle);
    xPropSet->setPropertyValue( WW8_ASCII2STR("Border"), aTmp);

    sal_Int16 nTmp=static_cast<sal_Int16>(nMaxLength);
    aTmp <<= nTmp;
    xPropSet->setPropertyValue( WW8_ASCII2STR("MaxTextLen"), aTmp);

    aFontData.Import(xPropSet);
    return sal_True;
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
    uno::Any aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Enabled"));
    fEnabled = any2bool(aTmp);
    if (fEnabled)
        nTemp |= 0x02;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("ReadOnly"));
    fLocked = any2bool(aTmp);
    if (fLocked)
        nTemp |= 0x04;

    *rContents << nTemp;
    pBlockFlags[0] |= 0x01;
    *rContents << sal_uInt8(0x48);
    *rContents << sal_uInt8(0x80);
    *rContents << sal_uInt8(0x2C);

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("BackgroundColor"));
    if (aTmp.hasValue())
        aTmp >>= nBackColor;
    *rContents << ExportColor(nBackColor);
    pBlockFlags[0] |= 0x02;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("TextColor"));
    if (aTmp.hasValue())
        aTmp >>= nForeColor;
    *rContents << ExportColor(nForeColor);
    pBlockFlags[0] |= 0x04;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Border"));
    sal_Int16 nBorder;
    aTmp >>= nBorder;
    nSpecialEffect = ExportBorder(nBorder,nBorderStyle);
    *rContents << nBorderStyle;
    pBlockFlags[0] |= 0x10;

    nStyle = 3;
    *rContents << nStyle;
    pBlockFlags[0] |= 0x40;

    Align(rContents,2,TRUE);

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("LineCount"));
    aTmp >>= nListRows;
    *rContents << nListRows;
    pBlockFlags[1] |= 0x40;

    *rContents << sal_uInt8(1); //DefaultSelected One
    pBlockFlags[2] |= 0x01;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Dropdown"));
    nDropButtonStyle = any2bool(aTmp);
    if (nDropButtonStyle)
        nDropButtonStyle=0x02;
    *rContents << nDropButtonStyle;
    pBlockFlags[2] |= 0x04;

    SvxOcxString aValue( rPropSet->getPropertyValue(WW8_ASCII2STR("Text")) );
    aValue.WriteLenField( *rContents );
    if (aValue.HasData())
        pBlockFlags[2] |= 0x40;

    Align(rContents,4,TRUE);
    *rContents << nSpecialEffect;
    pBlockFlags[3] |= 0x04;

    Align(rContents,4,TRUE);
    *rContents << rSize.Width;
    *rContents << rSize.Height;

    aValue.WriteCharArray( *rContents );

    Align(rContents,4,TRUE);

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
    static sal_uInt8 __READONLY_DATA aCompObj[] = {
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
    SvStorageStreamRef xStor( rObj->OpenStream( C2S("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenStream( C2S("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 __READONLY_DATA aOCXNAME[] = {
        0x43, 0x00, 0x6F, 0x00, 0x6D, 0x00, 0x62, 0x00,
        0x6F, 0x00, 0x42, 0x00, 0x6F, 0x00, 0x78, 0x00,
        0x31, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    {
    SvStorageStreamRef xStor2( rObj->OpenStream( C2S("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenStream( C2S("contents")));
    return WriteContents(xContents, rPropSet, rSize);
}



sal_Bool OCX_ListBox::Import(
    const uno::Reference< lang::XMultiServiceFactory > &rServiceFactory,
    uno::Reference< form::XFormComponent > &rFComp, awt::Size &rSz)
{
    OUString sServiceName = WW8_ASCII2STR("com.sun.star.form.component.ListBox");
    uno::Reference< uno::XInterface > xCreate =
        rServiceFactory->createInstance( sServiceName );
    if( !xCreate.is() )
        return sal_False;

    rFComp = uno::Reference< form::XFormComponent > (xCreate,uno::UNO_QUERY);
    if( !rFComp.is() )
        return sal_False;

    uno::Reference< beans::XPropertySet >  xPropSet(xCreate,uno::UNO_QUERY);

    rSz.Width = nWidth;
    rSz.Height = nHeight;

    uno::Any aTmp(&sName,getCppuType((OUString *)0));
    xPropSet->setPropertyValue( WW8_ASCII2STR("Name"), aTmp );

    sal_Bool bTmp=fEnabled;
    aTmp = bool2any(bTmp);
    xPropSet->setPropertyValue( WW8_ASCII2STR("Enabled"), aTmp);

    bTmp=fLocked;
    aTmp = bool2any(bTmp);
    xPropSet->setPropertyValue( WW8_ASCII2STR("ReadOnly"), aTmp);

    aTmp <<= ImportColor(nForeColor);
    xPropSet->setPropertyValue( WW8_ASCII2STR("TextColor"), aTmp);

    sal_Bool bTemp = nMultiState;
    aTmp = bool2any(bTemp);
    xPropSet->setPropertyValue( WW8_ASCII2STR("MultiSelection"), aTmp);

#if 0       //Don't delete this for now until I figure out if I can make this
    if (pValue)
    {
        aTmp <<= lclCreateOUString( pValue, nValueLen );
        xPropSet->setPropertyValue( WW8_ASCII2STR("DefaultText"), aTmp);
    }
#endif

    aTmp <<= ImportColor(nBackColor);
    xPropSet->setPropertyValue( WW8_ASCII2STR("BackgroundColor"), aTmp);

    aTmp <<= ImportBorder(nSpecialEffect,nBorderStyle);
    xPropSet->setPropertyValue( WW8_ASCII2STR("Border"), aTmp);

    aFontData.Import(xPropSet);
    return sal_True;
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

    uno::Any aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Enabled"));
    fEnabled = any2bool(aTmp);
    sal_uInt8 nTemp=fEnabled;
    if (fEnabled)
        nTemp = nTemp << 1;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("ReadOnly"));
    fLocked = any2bool(aTmp);
    if (fLocked)
        nTemp |= 0x04;

    *rContents << nTemp;
    pBlockFlags[0] |= 0x01;
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("BackgroundColor"));
    if (aTmp.hasValue())
        aTmp >>= nBackColor;
    *rContents << ExportColor(nBackColor);
    pBlockFlags[0] |= 0x02;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("TextColor"));
    if (aTmp.hasValue())
        aTmp >>= nForeColor;
    *rContents << ExportColor(nForeColor);
    pBlockFlags[0] |= 0x04;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("MultiSelection"));
    nMultiState = any2bool(aTmp);

    if (nMultiState)
    {
        *rContents << nMultiState;
        pBlockFlags[0] |= 0x20;
    }

    nStyle = 2;
    *rContents << nStyle;
    pBlockFlags[0] |= 0x40;


    Align(rContents,4,TRUE);

#if 0
    SvxOcxString aValue( rPropSet->getPropertyValue(WW8_ASCII2STR("DefaultText")) );
    aValue.WriteLenField( *rContents );
    if (aValue.HasData())
        pBlockFlags[2] |= 0x40;

    Align(rContents,4,TRUE);
#endif

    *rContents << rSize.Width;
    *rContents << rSize.Height;

#if 0
    aValue.WriteCharArray( *rContents );
#endif

    Align(rContents,4,TRUE);

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
    static sal_uInt8 __READONLY_DATA aCompObj[] = {
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
    SvStorageStreamRef xStor( rObj->OpenStream( C2S("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenStream( C2S("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 __READONLY_DATA aOCXNAME[] = {
        0x4C, 0x00, 0x69, 0x00, 0x73, 0x00, 0x74, 0x00,
        0x42, 0x00, 0x6F, 0x00, 0x78, 0x00, 0x31, 0x00,
        0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor2( rObj->OpenStream( C2S("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenStream( C2S("contents")));
    return WriteContents(xContents, rPropSet, rSize);
}

sal_Bool OCX_ModernControl::Read(SvStorageStream *pS)
{
    *pS >> nIdentifier;
    DBG_ASSERT(nIdentifier==nStandardId,
            "A control that has a different identifier");
    *pS >> nFixedAreaLen;
    pS->Read(pBlockFlags,8);

    if (pBlockFlags[0] & 0x01)
    {
        sal_uInt8 nTemp;
        *pS >> nTemp;

        fEnabled = (nTemp & 0x02) >> 1;
        fLocked = (nTemp & 0x04) >> 2;
        fBackStyle = (nTemp & 0x08) >> 3;

        *pS >> nTemp;

        fColumnHeads = (nTemp & 0x04) >> 2;
        fIntegralHeight = (nTemp & 0x08) >> 3;
        fMatchRequired = (nTemp & 0x10) >> 4;
        fAlignment = (nTemp & 0x20) >> 5;

        *pS >> nTemp;

        fDragBehaviour = (nTemp & 0x08) >> 3;
        fEnterKeyBehaviour = (nTemp & 0x10) >> 4;
        fEnterFieldBehaviour = (nTemp & 0x20) >> 5;
        fTabKeyBehaviour = (nTemp & 0x40) >> 6;
        fWordWrap = (nTemp & 0x80) >> 7;

        *pS >> nTemp;
        fSelectionMargin = (nTemp & 0x04) >> 2;
        fAutoWordSelect = (nTemp & 0x08) >> 3;
        fAutoSize = (nTemp & 0x10) >> 4;
        fHideSelection = (nTemp & 0x20) >> 5;
        fAutoTab = (nTemp & 0x40) >> 6;
        fMultiLine = (nTemp & 0x80) >> 7;

    }

    /*If any of these are set they follow eachother in this order one after
    another padded out to the next U32 boundary with 0's
    U8 can abut each other U16 must start on a U16 boundary and are padded to
    that with 0's. A standardish word alignment structure*/

    if (pBlockFlags[0] & 0x02)
        *pS >> nBackColor;
    if (pBlockFlags[0] & 0x04)
        *pS >> nForeColor;
    if (pBlockFlags[0] & 0x08)
        *pS >> nMaxLength;

    if (pBlockFlags[0] & 0x10)
        *pS >> nBorderStyle;
    if (pBlockFlags[0] & 0x20)
        *pS >> nScrollBars;
    if (pBlockFlags[0] & 0x40)
        *pS >> nStyle;// (UI 0 == Data 3, UI 2 = Data 7)
    if (pBlockFlags[0] & 0x80)
        *pS >> nMousePointer;

    if (pBlockFlags[1] & 0x02)
        *pS >> nPasswordChar; //HUH ??? always 0 ??? not sure maybe just padding

    if (pBlockFlags[1] & 0x04)
    {
        Align(pS,4);
        *pS >> nListWidth;
    }

    if (pBlockFlags[1] & 0x08)
    {
        Align(pS,2);
        *pS >> nBoundColumn;
    }
    if (pBlockFlags[1] & 0x10)
    {
        Align(pS,2);
        *pS >> nTextColumn;
    }
    if (pBlockFlags[1] & 0x20)
    {
        Align(pS,2);
        *pS >> nColumnCount;
    }
    if (pBlockFlags[1] & 0x40)
    {
        Align(pS,2);
        *pS >> nListRows;
    }
    if (pBlockFlags[1] & 0x80)
    {
        Align(pS,2);
        *pS >> nUnknown8; //something to do with ColumnWidths
    }
    if (pBlockFlags[2] & 0x01)
        *pS >> nMatchEntry;
    if (pBlockFlags[2] & 0x02)
        *pS >> nListStyle;
    if (pBlockFlags[2] & 0x04)
        *pS >> nShowDropButtonWhen;
    if (pBlockFlags[2] & 0x10)
        *pS >> nDropButtonStyle;
    if (pBlockFlags[2] & 0x20)
        *pS >> nMultiState;

    bool bValue = (pBlockFlags[2] & 0x40) != 0;
    if (bValue)
    {
        Align(pS,4);
        *pS >> nValueLen;
    }
    bool bCaption = (pBlockFlags[2] & 0x80) != 0;
    if (bCaption)
    {
        Align(pS,4);
        *pS >> nCaptionLen;
    }
    if (pBlockFlags[3] & 0x01)
    {
        Align(pS,4);
        *pS >> nHorzPos;
        *pS >> nVertPos;
    }
    if (pBlockFlags[3] & 0x02)
    {
        Align(pS,4);
        *pS >> nBorderColor;
    }
    if (pBlockFlags[3] & 0x04)
    {
        Align(pS,4); // NEW
        *pS >> nSpecialEffect;
    }
    if (pBlockFlags[3] & 0x08)
    {
        Align(pS,2);
        *pS >> nIcon;
        DBG_ASSERT(nIcon == 0xFFFF, "Unexpected nIcon");
    }
    if (pBlockFlags[3] & 0x10)
    {
        Align(pS,2);
        *pS >> nPicture;
        DBG_ASSERT(nPicture == 0xFFFF, "Unexpected nIcon");
    }
    if (pBlockFlags[3] & 0x20)
        *pS >> nAccelerator;
    /*
    if (pBlockFlags[3] & 0x80)
        *pS >> nUnknown9;
    */
    bool bGroupName = (pBlockFlags[4] & 0x01) != 0;
    if (bGroupName)
    {
        Align(pS,4);
        *pS >> nGroupNameLen;
    }

    //End

    Align(pS,4);
    *pS >> nWidth;
    *pS >> nHeight;

    if (bValue)
        lclReadCharArray( *pS, pValue, nValueLen );

    if (bCaption)
        lclReadCharArray( *pS, pCaption, nCaptionLen );

    if (bGroupName)
        lclReadCharArray( *pS, pGroupName, nGroupNameLen );

    Align(pS,4);
    if (nIcon)
    {
        pS->Read(pIconHeader,20);
        *pS >> nIconLen;
        pIcon = new sal_uInt8[nIconLen];
        pS->Read(pIcon,nIconLen);
    }

    if (nPicture)
    {
        pS->Read(pPictureHeader,20);
        *pS >> nPictureLen;
        pPicture = new sal_uInt8[nPictureLen];
        pS->Read(pPicture,nPictureLen);
    }

    return sal_True;
}


sal_Bool OCX_CommandButton::Read(SvStorageStream *pS)
{
    *pS >> nIdentifier;
    DBG_ASSERT(nStandardId==nIdentifier,
        "A control that has a different identifier");
    *pS >> nFixedAreaLen;
    pS->Read(pBlockFlags,4);


    if (pBlockFlags[0] & 0x01)
        *pS >> nForeColor;
    if (pBlockFlags[0] & 0x02)
        *pS >> nBackColor;

    if (pBlockFlags[0] & 0x04)
    {
        sal_uInt8 nTemp;
        *pS >> nTemp;
        fEnabled = (nTemp&0x02)>>1;
        fLocked = (nTemp&0x04)>>2;
        fBackStyle = (nTemp&0x08)>>3;
        *pS >> nTemp;
        *pS >> nTemp;
        fWordWrap = (nTemp&0x80)>>7;
        *pS >> nTemp;
        fAutoSize = (nTemp&0x10)>>4;
    }

    bool bCaption = (pBlockFlags[0] & 0x08) != 0;
    if (bCaption)
        *pS >> nCaptionLen;

    if (pBlockFlags[0] & 0x10) /*Picture Position, a strange mechanism here*/
    {
        *pS >> nVertPos;
        *pS >> nHorzPos;
    }

    if (pBlockFlags[0] & 0x40) /*MousePointer*/
        *pS >> nMousePointer;

    if (pBlockFlags[0] & 0x80)
    {
        Align(pS,2);
        *pS >> nPicture;
    }

    if (pBlockFlags[1] & 0x01)
    {
        Align(pS,2);
        *pS >> nAccelerator;
    }

    if (pBlockFlags[1] & 0x04)
    {
        Align(pS,2);
        *pS >> nIcon;
    }

    if (bCaption)
        lclReadCharArray( *pS, pCaption, nCaptionLen );

    Align(pS,4);
    *pS >> nWidth;
    *pS >> nHeight;

    if (nIcon)
    {
        pS->Read(pIconHeader,20);
        *pS >> nIconLen;
        pIcon = new sal_uInt8[nIconLen];
        pS->Read(pIcon,nIconLen);
    }

    if (nPicture)
    {
        pS->Read(pPictureHeader,20);
        *pS >> nPictureLen;
        pPicture = new sal_uInt8[nPictureLen];
        pS->Read(pPicture,nPictureLen);
    }

    return sal_True;
}

sal_Bool OCX_Label::Read(SvStorageStream *pS)
{
    *pS >> nIdentifier;
    DBG_ASSERT(nStandardId==nIdentifier,
            "A control that has a different identifier");
    *pS >> nFixedAreaLen;
    pS->Read(pBlockFlags,4);


    if (pBlockFlags[0] & 0x01)
        *pS >> nForeColor;
    if (pBlockFlags[0] & 0x02)
        *pS >> nBackColor;


    if (pBlockFlags[0] & 0x04)
    {
        sal_uInt8 nTemp;
        *pS >> nTemp;
        fEnabled = (nTemp&0x02)>>1;
        fLocked = (nTemp&0x04)>>2;
        fBackStyle = (nTemp&0x08)>>3;
        *pS >> nTemp;
        *pS >> nTemp;
        fWordWrap = (nTemp&0x80)>>7;
        *pS >> nTemp;
        fAutoSize = (nTemp&0x10)>>4;
    }
    bool bCaption = (pBlockFlags[0] & 0x08) != 0;
    if (bCaption)
        *pS >> nCaptionLen;

    if (pBlockFlags[0] & 0x10)
    {
        *pS >> nVertPos;
        *pS >> nHorzPos;
    }

    if (pBlockFlags[0] & 0x40)
        *pS >> nMousePointer;

    if (pBlockFlags[0] & 0x80)
    {
        Align(pS,4);
        *pS >> nBorderColor;
    }

    if (pBlockFlags[1] & 0x01)
    {
        Align(pS,2);
        *pS >> nBorderStyle;
    }

    if (pBlockFlags[1] & 0x02)
    {
        Align(pS,2);
        *pS >> nSpecialEffect;
    }

    if (pBlockFlags[1] & 0x04)
    {
        Align(pS,2);
        *pS >> nPicture;
    }

    if (pBlockFlags[1] & 0x08)
    {
        Align(pS,2);
        *pS >> nAccelerator;
    }

    if (pBlockFlags[1] & 0x10)
    {
        Align(pS,2);
        *pS >> nIcon;
    }

    if (bCaption)
        lclReadCharArray( *pS, pCaption, nCaptionLen );

    Align(pS,4);
    *pS >> nWidth;
    *pS >> nHeight;

    if (nIcon)
    {
        pS->Read(pIconHeader,20);
        *pS >> nIconLen;
        pIcon = new sal_uInt8[nIconLen];
        pS->Read(pIcon,nIconLen);
    }

    if (nPicture)
    {
        pS->Read(pPictureHeader,20);
        *pS >> nPictureLen;
        pPicture = new sal_uInt8[nPictureLen];
        pS->Read(pPicture,nPictureLen);
    }

    return sal_True;
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

    uno::Any aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("TextColor"));
    if (aTmp.hasValue())
        aTmp >>= nForeColor;
    *rContents << ExportColor(nForeColor);
    pBlockFlags[0] |= 0x01;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("BackgroundColor"));
    if (aTmp.hasValue())
        aTmp >>= nBackColor;
    *rContents << ExportColor(nBackColor);
    pBlockFlags[0] |= 0x02;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Enabled"));
    fEnabled = any2bool(aTmp);
    sal_uInt8 nTemp=fEnabled;
    if (fEnabled)
        nTemp = nTemp << 1;
    *rContents << nTemp;
    *rContents << sal_uInt8(0x00);

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("MultiLine"));
    fWordWrap = any2bool(aTmp);
    nTemp=fWordWrap;
    nTemp = nTemp << 7;
    *rContents << nTemp;
    *rContents << sal_uInt8(0x00);
    pBlockFlags[0] |= 0x04;

    SvxOcxString aCaption( rPropSet->getPropertyValue(WW8_ASCII2STR("Label")) );
    aCaption.WriteLenField( *rContents );
    if (aCaption.HasData())
        pBlockFlags[0] |= 0x08;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Border"));
    sal_Int16 nBorder;
    aTmp >>= nBorder;
    sal_uInt8 nNewBorder;
    nSpecialEffect = ExportBorder(nBorder,nNewBorder);
    nBorderStyle = nNewBorder;
    *rContents << nBorderStyle;
    pBlockFlags[1] |= 0x01;
    *rContents << nSpecialEffect;
    pBlockFlags[1] |= 0x02;

    aCaption.WriteCharArray( *rContents );

    Align(rContents,4,TRUE);
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
    static sal_uInt8 __READONLY_DATA aCompObj[] = {
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
    SvStorageStreamRef xStor( rObj->OpenStream( C2S("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenStream( C2S("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 __READONLY_DATA aOCXNAME[] = {
        0x4C, 0x00, 0x61, 0x00, 0x62, 0x00, 0x65, 0x00,
        0x6C, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor2( rObj->OpenStream( C2S("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    static sal_uInt8 __READONLY_DATA aTest[] = {
        0x00, 0x02, 0x20, 0x00, 0x2B, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00,
        0x06, 0x00, 0x00, 0x80, 0x4C, 0x61, 0x62, 0x65,
        0x6C, 0x31, 0x18, 0x00, 0xEC, 0x09, 0x00, 0x00,
        0x7B, 0x02, 0x00, 0x00, 0x00, 0x02, 0x20, 0x00,
        0x35, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x80,
        0xC3, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
        0x54, 0x69, 0x6D, 0x65, 0x73, 0x20, 0x4E, 0x65,
        0x77, 0x20, 0x52, 0x6F, 0x6D, 0x61, 0x6E, 0x00,
        };

    SvStorageStreamRef xContents( rObj->OpenStream( C2S("contents")));
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

static const int NO_OCX=18;

OCX_map aOCXTab[NO_OCX] =
{
    {&OCX_CommandButton::Create,"D7053240-CE69-11CD-a777-00dd01143c57",
        form::FormComponentType::COMMANDBUTTON,"CommandButton"},
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
    {&OCX_ToggleButton::Create,"8BD21D60-EC42-11CE-9e0d-00aa006002f3",
        -1,"ToggleButton"},
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
        form::FormComponentType::GROUPBOX,""}
};

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
        OUString::createFromAscii("ClassId"));
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
            supportsService(OUString::createFromAscii(
                "com.sun.star.form.component.FormattedField")))
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
            supportsService(OUString::createFromAscii(
                "com.sun.star.form.component.ImageControl")))
        nClassId = form::FormComponentType::IMAGECONTROL;
    }
//End nasty hack

    //sal_Int16  nClassId = FormComponentType::COMMANDBUTTON;

    for (int i=0;i<NO_OCX;i++)
    {
        if ( nClassId == aOCXTab[i].nId )
        {
            rId.AppendAscii(aOCXTab[i].sId);
            rName.AppendAscii(aOCXTab[i].sName);
            return(aOCXTab[i].pCreate());
        }
    }
    return(NULL);
}


sal_Bool SvxMSConvertOCXControls::ReadOCXStream( SvStorageRef& rSrc1,
        uno::Reference < drawing::XShape > *pShapeRef,BOOL bFloatingCtrl)
{
    sal_Bool bRet=sal_False;

    SvStorageStreamRef xSrc2 = rSrc1->OpenStream( WW8_ASCII2STR("\3OCXNAME") );
    SvStorageStream* pSt = xSrc2;
    pSt->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    xSrc2 = rSrc1->OpenStream( WW8_ASCII2STR("contents") );
    pSt = xSrc2;
    pSt->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    /*Get Class Id of this object, see if it is one of the types
     *that this importer can handle, call the appropiate handler
     to read that control, and call the appropiate handler to
     insert that control
     */

    OCX_Control *pObj=NULL;
    SvGlobalName aTest = rSrc1->GetClassName();
    if (pObj = OCX_Factory(aTest.GetHexName()))
    {

        com::sun::star::awt::Size aSz;
        uno::Reference< form::XFormComponent >  xFComp;
        const uno::Reference< lang::XMultiServiceFactory > & rServiceFactory =
            GetServiceFactory();
        if(!rServiceFactory.is())
            return(sal_False);
        if(bRet = pObj->FullRead(pSt))
            if (pObj->Import(rServiceFactory,xFComp,aSz))
                bRet = InsertControl( xFComp, aSz,pShapeRef,bFloatingCtrl);
        delete pObj;
    }
    return bRet;
}


sal_Bool SvxMSConvertOCXControls::ReadOCXExcelKludgeStream(
    SvStorageStreamRef& rSrc1, uno::Reference < drawing::XShape > *
    pShapeRef,BOOL bFloatingCtrl)
{
    sal_Bool bRet=sal_False;
    /*Get Class Id of this object, see if it is one of the types
     *that this importer can handle, call the appropiate handler
     to read that control, and call the appropiate handler to
     insert that control
     */
    /*The Excel Kludge is to concatenate a class id with a contents
     * stream, and then concatenate all the controls together,
     * This means that you should have the cnts stream wound to the
     * correct location before passing the control stream in here*/
    OCX_Control *pObj=NULL;
    SvStream *pSt = rSrc1;
    pSt->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    SvGlobalName aTest;
    *pSt >> aTest;
    if (pObj = OCX_Factory(aTest.GetHexName()))
    {

        com::sun::star::awt::Size aSz;
        uno::Reference< form::XFormComponent >  xFComp;
        const uno::Reference< lang::XMultiServiceFactory > & rServiceFactory =
            GetServiceFactory();
        if(!rServiceFactory.is())
            return(sal_False);
        if(bRet = pObj->FullRead(rSrc1))
            if (pObj->Import(rServiceFactory,xFComp,aSz))
                bRet = InsertControl( xFComp, aSz,pShapeRef,bFloatingCtrl);
        delete pObj;
    }
    return bRet;
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
        OUString::createFromAscii("ClassId"));
    sal_Int16 nClassId = *(sal_Int16*) aTmp.getValue();
#endif

    OCX_Control *pObj=NULL;
    String sId;
    if (pObj = OCX_Factory(rControlModel,sId,rName))
    {
        uno::Reference<beans::XPropertySet> xPropSet(rControlModel,
            uno::UNO_QUERY);

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

    OCX_Control *pObj=NULL;
    String sId;
    if (pObj = OCX_Factory(rControlModel,sId,rName))
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







sal_Bool OCX_CheckBox::Import(
    const uno::Reference< lang::XMultiServiceFactory >  &rServiceFactory,
    uno::Reference< form::XFormComponent > &rFComp, awt::Size &rSz)
{
    OUString sServiceName = WW8_ASCII2STR("com.sun.star.form.component.CheckBox");
    uno::Reference< uno::XInterface > xCreate =
        rServiceFactory->createInstance( sServiceName );
    if( !xCreate.is() )
        return(sal_False);

    rFComp = uno::Reference< form::XFormComponent > (xCreate,uno::UNO_QUERY);
    if( !rFComp.is() )
        return(sal_False);

    uno::Reference< beans::XPropertySet >  xPropSet( xCreate,uno::UNO_QUERY );

    rSz.Width = nWidth;
    rSz.Height = nHeight;

    uno::Any aTmp(&sName,getCppuType((OUString *)0));
    xPropSet->setPropertyValue( WW8_ASCII2STR("Name"), aTmp );

    sal_Bool bTemp;
    if ((!(fEnabled)) || (fLocked))
        bTemp = sal_False;
    else
        bTemp = sal_True;
    aTmp = bool2any(bTemp);
    xPropSet->setPropertyValue( WW8_ASCII2STR("Enabled"), aTmp);

    aTmp <<= ImportColor(nForeColor);
    xPropSet->setPropertyValue( WW8_ASCII2STR("TextColor"), aTmp);
#if 0
    aTmp <<= ImportColor(nBackColor);
    xPropSet->setPropertyValue( WW8_ASCII2STR("BackgroundColor"), aTmp);
#endif

    bTemp = nMultiState;
    aTmp = bool2any(bTemp);
    xPropSet->setPropertyValue( WW8_ASCII2STR("TriState"), aTmp);

    if (pValue)
    {
        INT16 nTmp=pValue[0]-0x30;
        aTmp <<= nTmp;
        xPropSet->setPropertyValue( WW8_ASCII2STR("DefaultState"), aTmp);
    }

    if (pCaption)
    {
        aTmp <<= lclCreateOUString( pCaption, nCaptionLen );
        xPropSet->setPropertyValue( WW8_ASCII2STR("Label"), aTmp);
    }

    aFontData.Import(xPropSet);
    return(sal_True);
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

    uno::Any aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Enabled"));
    fEnabled = any2bool(aTmp);
    sal_uInt8 nTemp=fEnabled;
    if (fEnabled)
        nTemp = nTemp << 1;
    *rContents << nTemp;
    pBlockFlags[0] |= 0x01;
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("TextColor"));
    if (aTmp.hasValue())
        aTmp >>= nForeColor;
    *rContents << ExportColor(nForeColor);
    pBlockFlags[0] |= 0x04;

    nStyle = 4;
    *rContents << nStyle;
    pBlockFlags[0] |= 0x40;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("TriState"));
    nMultiState = any2bool(aTmp);
    *rContents << nMultiState;
    pBlockFlags[2] |= 0x20;

    Align(rContents,4,TRUE);
    nValueLen = 1|SVX_MSOCX_COMPRESSED;
    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("DefaultState"));
    INT16 nDefault;
    aTmp >>= nDefault;
    *rContents << nValueLen;
    pBlockFlags[2] |= 0x40;

    SvxOcxString aCaption( rPropSet->getPropertyValue(WW8_ASCII2STR("Label")) );
    aCaption.WriteLenField( *rContents );
    if (aCaption.HasData())
        pBlockFlags[2] |= 0x80;

    Align(rContents,4,TRUE);
    *rContents << rSize.Width;
    *rContents << rSize.Height;

    nDefault += 0x30;
    *rContents << sal_uInt8(nDefault);
    *rContents << sal_uInt8(0x00);

    aCaption.WriteCharArray( *rContents );

    Align(rContents,4,TRUE);
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
    static sal_uInt8 __READONLY_DATA aCompObj[] = {
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
    SvStorageStreamRef xStor( rObj->OpenStream( C2S("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenStream( C2S("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 __READONLY_DATA aOCXNAME[] = {
        0x43, 0x00, 0x68, 0x00, 0x65, 0x00, 0x63, 0x00,
        0x6B, 0x00, 0x42, 0x00, 0x6F, 0x00, 0x78, 0x00,
        0x31, 0x00, 0x00, 0x00, 0x00, 0x00
        };
    {
    SvStorageStreamRef xStor2( rObj->OpenStream( C2S("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    static sal_uInt8 __READONLY_DATA aTest[] = {
        0x00, 0x02, 0x34, 0x00, 0x46, 0x01, 0xC0, 0x80,
        0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x80, 0x09, 0x00, 0x00, 0x80,
        0xE2, 0x0E, 0x00, 0x00, 0x95, 0x02, 0x00, 0x00,
        0x30, 0x69, 0x1D, 0x00, 0x43, 0x68, 0x65, 0x63,
        0x6B, 0x42, 0x6F, 0x78, 0x31, 0x20, 0x52, 0x6F,
        0x00, 0x02, 0x20, 0x00, 0x35, 0x00, 0x00, 0x00,
        0x0F, 0x00, 0x00, 0x80, 0xC3, 0x00, 0x00, 0x00,
        0x00, 0x02, 0x00, 0x00, 0x54, 0x69, 0x6D, 0x65,
        0x73, 0x20, 0x4E, 0x65, 0x77, 0x20, 0x52, 0x6F,
        0x6D, 0x61, 0x6E, 0x00,
    };
    SvStorageStreamRef xContents( rObj->OpenStream( C2S("contents")));
    return WriteContents(xContents, rPropSet, rSize);
}


sal_Bool OCX_FontData::Read(SvStorageStream *pS)
{
    *pS >> nIdentifier;
    *pS >> nFixedAreaLen;
    pS->Read(pBlockFlags,4);

    bool bFontName = (pBlockFlags[0] & 0x01) != 0;
    if (bFontName)
        *pS >> nFontNameLen;
    if (pBlockFlags[0] & 0x02)
    {
        sal_uInt8 nTmp;
        *pS >> nTmp;
        fBold = nTmp & 0x01;
        fItalic = (nTmp & 0x02) >> 1;
        fUnderline = (nTmp & 0x04) >> 2;
        fStrike = (nTmp & 0x08) >> 3;
        fUnknown1 = (nTmp & 0xF0) >> 4;
        *pS >> nUnknown2;
        *pS >> nUnknown3;
        *pS >> nUnknown4;
    }
    if (pBlockFlags[0] & 0x04)
    {
        Align(pS,4);
        *pS >> nFontSize;
    }
    if (pBlockFlags[0] & 0x10)
    {
        Align(pS,2);
        *pS >> nLanguageID;
    }
    if (pBlockFlags[0] & 0x40)
    {
        *pS >> nJustification;
    }
    if (pBlockFlags[0] & 0x80)  // font weight before font name
    {
        Align(pS,2);
        *pS >> nFontWeight;
    }

    if (bFontName)
        lclReadCharArray( *pS, pFontName, nFontNameLen );

    Align(pS,4);
    return(TRUE);
}

void OCX_FontData::Import(uno::Reference< beans::XPropertySet > &rPropSet)
{
    uno::Any aTmp;
    if (pFontName)
    {
        aTmp <<= lclCreateOUString( pFontName, nFontNameLen );
        rPropSet->setPropertyValue( WW8_ASCII2STR("FontName"), aTmp);
    }

    if (bHasAlign)
    {
        sal_Int16 nAlign = ImportAlign(nJustification);
        aTmp <<= nAlign;
        rPropSet->setPropertyValue( WW8_ASCII2STR("Align"), aTmp);
    }

    if (fBold)
    {
        aTmp <<= awt::FontWeight::BOLD;
        rPropSet->setPropertyValue( WW8_ASCII2STR("FontWeight"), aTmp);
    }

    if (fItalic)
    {
        aTmp <<= (sal_Int16)awt::FontSlant_ITALIC;
        rPropSet->setPropertyValue( WW8_ASCII2STR("FontSlant"), aTmp);
    }

    if (fUnderline)
    {
        aTmp <<= awt::FontUnderline::SINGLE;
        rPropSet->setPropertyValue( WW8_ASCII2STR("FontUnderline"), aTmp);
    }

    if (fStrike)
    {
        aTmp <<= awt::FontStrikeout::SINGLE;
        rPropSet->setPropertyValue( WW8_ASCII2STR("FontStrikeout"), aTmp);
    }

    aTmp <<= sal_Int16(nFontSize/20);
    rPropSet->setPropertyValue( WW8_ASCII2STR("FontHeight"), aTmp);
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
        aFontName = rPropSet->getPropertyValue(WW8_ASCII2STR("FontName"));
    if (!aFontName.HasData())
        aFontName = OUString( RTL_CONSTASCII_USTRINGPARAM( "Times New Roman" ) );
    aFontName.WriteLenField( *rContent );
    nFlags |= 0x01;

    if (bHasFont)
    {
        aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("FontWeight"));
        float nBold;
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

        aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("FontHeight"));
        float nFontHeight;
        aTmp >>= nFontHeight;
        if (nFontHeight)
        {
            nFlags |= 0x04;
            nFontSize = static_cast<sal_uInt32>(nFontHeight*20);
            *rContent << nFontSize;
        }

        if (bHasAlign)
        {
            *rContent << sal_uInt16(0x0200);
            nFlags |= 0x10;

            nFlags |= 0x20; // ?

            aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Align"));
            nFlags |= 0x40;
            sal_Int16 nAlign(0);
            if (aTmp.hasValue())
                aTmp >>= nAlign;
            nJustification = ExportAlign(nAlign);
            *rContent << nJustification;
        }
    }

    aFontName.WriteCharArray( *rContent );
    Align(rContent,4,TRUE);

    sal_uInt16 nFixedAreaLen = static_cast<sal_uInt16>(rContent->Tell()-nOldPos-4);
    rContent->Seek(nOldPos);
    *rContent << nStandardId;
    *rContent << nFixedAreaLen;
    *rContent << nFlags;
    *rContent << sal_uInt8(0x00);
    *rContent << sal_uInt8(0x00);
    *rContent << sal_uInt8(0x00);

    Align(rContent,4,TRUE);
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
        WW8_ASCII2STR("BackgroundColor"));
    if (aTmp.hasValue())
        aTmp >>= nBackColor;
    *rContents << ExportColor(nBackColor);
    pBlockFlags[0] |= 0x10;


    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Border"));
    sal_Int16 nBorder;
    aTmp >>= nBorder;
    nSpecialEffect = ExportBorder(nBorder,nBorderStyle);
    *rContents << nBorderStyle;
    pBlockFlags[0] |= 0x20;

    *rContents << nSpecialEffect;
    pBlockFlags[1] |= 0x01;

    Align(rContents,4,TRUE);

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Enabled"));
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

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("ImageURL"));
    OUString *pStr = (OUString *)aTmp.getValue();
    /*Magically fetch that image and turn it into something that
     *we can store in ms controls, wmf,png,jpg are almost certainly
     *the options we have for export...*/

    Align(rContents,4,TRUE);
    *rContents << rSize.Width;
    *rContents << rSize.Height;

    Align(rContents,4,TRUE);
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
    static sal_uInt8 __READONLY_DATA aCompObj[] = {
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
    SvStorageStreamRef xStor( rObj->OpenStream( C2S("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenStream( C2S("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 __READONLY_DATA aOCXNAME[] = {
        0x49, 0x00, 0x6D, 0x00, 0x61, 0x00, 0x67, 0x00,
        0x65, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor2( rObj->OpenStream( C2S("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenStream( C2S("contents")));
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
    mnWidth( 0 ),
    mnHeight( 0 ),
    mnBackColor( 0x8000000F ),
    mnForeColor( 0x80000012 ),
    mnDelay( 50 ),
    mbEnabled( true ),
    mbLocked( false ),
    mbPropThumb( true )
{
}

OCX_Control* OCX_SpinButton::Create()
{
    return new OCX_SpinButton;
}

sal_Bool OCX_SpinButton::Read( SvStorageStream *pS )
{
    if( !pS ) return sal_False;

    SvStream& rStrm = *pS;
    sal_uInt16 nId, nSize;
    sal_Int32 nIcon = 0;

    rStrm >> nId >> nSize >> mnBlockFlags;

    DBG_ASSERT( nStandardId == nId, "OCX_SpinButton::Read - unknown identifier" );

    if( mnBlockFlags & 0x00000001 )     rStrm >> mnForeColor;
    if( mnBlockFlags & 0x00000002 )     rStrm >> mnBackColor;
    if( mnBlockFlags & 0x00000004 )
    {
        sal_Int32 nFlags;
        rStrm >> nFlags;
        mbEnabled = (nFlags & 0x00000002) != 0;
        mbLocked = (nFlags & 0x00000004) != 0;
    }
    if( mnBlockFlags & 0x00000010 )     rStrm.SeekRel( 4 );     // mouse pointer
    if( mnBlockFlags & 0x00000020 )     rStrm >> mnMin;
    if( mnBlockFlags & 0x00000040 )     rStrm >> mnMax;
    if( mnBlockFlags & 0x00000080 )     rStrm >> mnValue;
    if( mnBlockFlags & 0x00000100 )     rStrm.SeekRel( 4 );     // unknown
    if( mnBlockFlags & 0x00000200 )     rStrm.SeekRel( 4 );     // unknown
    if( mnBlockFlags & 0x00000400 )     rStrm.SeekRel( 4 );     // unknown
    if( mnBlockFlags & 0x00000800 )     rStrm >> mnSmallStep;
    if( mnBlockFlags & 0x00001000 )     rStrm >> mnPageStep;
    if( mnBlockFlags & 0x00002000 )     rStrm >> mnOrient;
    if( mnBlockFlags & 0x00004000 )
    {
        sal_Int32 nThumb;
        *pS >> nThumb;
        mbPropThumb = nThumb != 0;
    }
    if( mnBlockFlags & 0x00008000 )     rStrm >> mnDelay;
    if( mnBlockFlags & 0x00010000 )     rStrm >> nIcon;
    if( mnBlockFlags & 0x00000008 )     rStrm >> mnWidth >> mnHeight;

    if( nIcon )
    {
        sal_Int32 nIconSize;
        pS->SeekRel( 20 );
        *pS >> nIconSize;
        pS->SeekRel( nIconSize );
    }

    return sal_True;
}

sal_Bool OCX_SpinButton::ReadFontData( SvStorageStream *pS )
{
    // spin buttons and scroll bars do not support font data
    return sal_True;
}

sal_Bool OCX_SpinButton::Import(
        const uno::Reference< lang::XMultiServiceFactory >& rServiceFactory,
        uno::Reference< form::XFormComponent >& rFComp,
        awt::Size &rSz )
{
    if( (mnWidth < 1) || (mnHeight < 1) )
        return sal_False;

    OUString sServiceName = WW8_ASCII2STR( "com.sun.star.form.component.SpinButton" );
    uno::Reference< uno::XInterface > xCreate = rServiceFactory->createInstance( sServiceName );
    if( !xCreate.is() )
        return sal_False;

    rFComp = uno::Reference< form::XFormComponent >( xCreate, uno::UNO_QUERY );
    if( !rFComp.is() )
        return sal_False;

    uno::Reference< beans::XPropertySet > xPropSet( xCreate, uno::UNO_QUERY );

    rSz.Width = mnWidth;
    rSz.Height = mnHeight;

    uno::Any aTmp( &sName, getCppuType((OUString *)0) );
    xPropSet->setPropertyValue( WW8_ASCII2STR( "Name" ), aTmp );

    aTmp <<= ImportColor( mnForeColor );
    xPropSet->setPropertyValue( WW8_ASCII2STR("SymbolColor"), aTmp);

    aTmp <<= ImportColor( mnBackColor );
    xPropSet->setPropertyValue( WW8_ASCII2STR("BackgroundColor"), aTmp);

    aTmp = bool2any( mbEnabled && !mbLocked );
    xPropSet->setPropertyValue( WW8_ASCII2STR("Enabled"), aTmp);

    aTmp <<= mnValue;
    xPropSet->setPropertyValue( WW8_ASCII2STR("DefaultSpinValue"), aTmp );

    aTmp <<= mnMin;
    xPropSet->setPropertyValue( WW8_ASCII2STR("SpinValueMin"), aTmp );

    aTmp <<= mnMax;
    xPropSet->setPropertyValue( WW8_ASCII2STR("SpinValueMax"), aTmp );

    aTmp <<= mnSmallStep;
    xPropSet->setPropertyValue( WW8_ASCII2STR("SpinIncrement"), aTmp );

    namespace AwtScrollOrient = ::com::sun::star::awt::ScrollBarOrientation;
    switch( mnOrient )
    {
        case 0:     aTmp <<= AwtScrollOrient::VERTICAL;     break;
        case 1:     aTmp <<= AwtScrollOrient::HORIZONTAL;   break;
        default:    aTmp <<= (mnWidth < mnHeight) ? AwtScrollOrient::VERTICAL : AwtScrollOrient::HORIZONTAL;
    }
    xPropSet->setPropertyValue( WW8_ASCII2STR("Orientation"), aTmp );

    aTmp = bool2any( true );
    xPropSet->setPropertyValue( WW8_ASCII2STR("Repeat"), aTmp );

    aTmp <<= mnDelay;
    xPropSet->setPropertyValue( WW8_ASCII2STR("RepeatDelay"), aTmp );

    aTmp <<= sal_Int16( 0 );
    xPropSet->setPropertyValue( WW8_ASCII2STR("Border"), aTmp);

    return sal_True;
}

sal_Bool OCX_SpinButton::Export(
        SvStorageRef &rObj,
        const uno::Reference< beans::XPropertySet>& rPropSet,
        const awt::Size& rSize )
{
    static sal_uInt8 __READONLY_DATA aCompObj[] =
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
        SvStorageStreamRef xStor( rObj->OpenStream( C2S("\1CompObj")));
        xStor->Write(aCompObj,sizeof(aCompObj));
        DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
        SvStorageStreamRef xStor3( rObj->OpenStream( C2S("\3ObjInfo")));
        xStor3->Write(aObjInfo,sizeof(aObjInfo));
        DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 __READONLY_DATA aOCXNAME[] =
    {
        0x53, 0x00, 0x70, 0x00, 0x69, 0x00, 0x6E, 0x00,
        0x42, 0x00, 0x75, 0x00, 0x74, 0x00, 0x74, 0x00,
        0x6F, 0x00, 0x6E, 0x00, 0x31, 0x00, 0x00, 0x00,
        0x00, 0x00
    };

    {
        SvStorageStreamRef xStor2( rObj->OpenStream( C2S("\3OCXNAME")));
        xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
        DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenStream( C2S("contents")));
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
    mnWidth = rSize.Width;
    mnHeight = rSize.Height;

    GetInt32Property( mnForeColor, rPropSet, WW8_ASCII2STR( "SymbolColor" ),     0x00000001 );
    GetInt32Property( mnBackColor, rPropSet, WW8_ASCII2STR( "BackgroundColor" ), 0x00000002 );
    GetBoolProperty(  mbEnabled,   rPropSet, WW8_ASCII2STR( "Enabled" ),         0x00000304 );
    GetInt32Property( mnMin,       rPropSet, WW8_ASCII2STR( "SpinValueMin" ),    0x00000020 );
    GetInt32Property( mnMax,       rPropSet, WW8_ASCII2STR( "SpinValueMax" ),    0x00000040 );
    GetInt32Property( mnValue,     rPropSet, WW8_ASCII2STR( "SpinValue" ),       0x00000080 );
    GetInt32Property( mnSmallStep, rPropSet, WW8_ASCII2STR( "SpinIncrement" ),   0x00000800 );
    GetInt32Property( mnDelay,     rPropSet, WW8_ASCII2STR( "RepeatDelay" ),     0x00008000 );

    namespace AwtScrollOrient = ::com::sun::star::awt::ScrollBarOrientation;
    sal_Int16 nApiOrient;
    if( rPropSet->getPropertyValue( WW8_ASCII2STR( "Orientation" ) ) >>= nApiOrient )
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
    sal_Int32 nNewValue;
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
    ULONG nStartPos = rStrm.Tell();

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
    if( mnBlockFlags & 0x00000008 )     rStrm << mnWidth << mnHeight;

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
}

OCX_Control* OCX_ScrollBar::Create()
{
    return new OCX_ScrollBar;
}

sal_Bool OCX_ScrollBar::Import(
        const uno::Reference< lang::XMultiServiceFactory >& rServiceFactory,
        uno::Reference< form::XFormComponent >& rFComp,
        awt::Size &rSz )
{
    if( (mnWidth < 1) || (mnHeight < 1) )
        return sal_False;

    OUString sServiceName = WW8_ASCII2STR( "com.sun.star.form.component.ScrollBar" );
    uno::Reference< uno::XInterface > xCreate = rServiceFactory->createInstance( sServiceName );
    if( !xCreate.is() )
        return sal_False;

    rFComp = uno::Reference< form::XFormComponent >( xCreate, uno::UNO_QUERY );
    if( !rFComp.is() )
        return sal_False;

    uno::Reference< beans::XPropertySet > xPropSet( xCreate, uno::UNO_QUERY );

    rSz.Width = mnWidth;
    rSz.Height = mnHeight;

    uno::Any aTmp( &sName, getCppuType((OUString *)0) );
    xPropSet->setPropertyValue( WW8_ASCII2STR( "Name" ), aTmp );

    aTmp <<= ImportColor( mnForeColor );
    xPropSet->setPropertyValue( WW8_ASCII2STR("SymbolColor"), aTmp);

    aTmp <<= ImportColor( mnBackColor );
    xPropSet->setPropertyValue( WW8_ASCII2STR("BackgroundColor"), aTmp);

    aTmp = bool2any( mbEnabled && !mbLocked );
    xPropSet->setPropertyValue( WW8_ASCII2STR("Enabled"), aTmp);

    aTmp <<= mnValue;
    xPropSet->setPropertyValue( WW8_ASCII2STR("DefaultScrollValue"), aTmp );

    aTmp <<= mnMin;
    xPropSet->setPropertyValue( WW8_ASCII2STR("ScrollValueMin"), aTmp );

    aTmp <<= mnMax;
    xPropSet->setPropertyValue( WW8_ASCII2STR("ScrollValueMax"), aTmp );

    aTmp <<= mnSmallStep;
    xPropSet->setPropertyValue( WW8_ASCII2STR("LineIncrement"), aTmp );

    aTmp <<= mnPageStep;
    xPropSet->setPropertyValue( WW8_ASCII2STR("BlockIncrement"), aTmp );
    if( mbPropThumb && (mnPageStep > 0) )
        xPropSet->setPropertyValue( WW8_ASCII2STR("VisibleSize"), aTmp );

    namespace AwtScrollOrient = ::com::sun::star::awt::ScrollBarOrientation;
    switch( mnOrient )
    {
        case 0:     aTmp <<= AwtScrollOrient::VERTICAL;     break;
        case 1:     aTmp <<= AwtScrollOrient::HORIZONTAL;   break;
        default:    aTmp <<= (mnWidth < mnHeight) ? AwtScrollOrient::VERTICAL : AwtScrollOrient::HORIZONTAL;
    }
    xPropSet->setPropertyValue( WW8_ASCII2STR("Orientation"), aTmp );

    aTmp <<= mnDelay;
    xPropSet->setPropertyValue( WW8_ASCII2STR("RepeatDelay"), aTmp );

    aTmp <<= sal_Int16( 0 );
    xPropSet->setPropertyValue( WW8_ASCII2STR("Border"), aTmp);

    return sal_True;
}

sal_Bool OCX_ScrollBar::Export(
        SvStorageRef &rObj,
        const uno::Reference< beans::XPropertySet>& rPropSet,
        const awt::Size& rSize )
{
    static sal_uInt8 __READONLY_DATA aCompObj[] =
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
        SvStorageStreamRef xStor( rObj->OpenStream( C2S("\1CompObj")));
        xStor->Write(aCompObj,sizeof(aCompObj));
        DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
        SvStorageStreamRef xStor3( rObj->OpenStream( C2S("\3ObjInfo")));
        xStor3->Write(aObjInfo,sizeof(aObjInfo));
        DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 __READONLY_DATA aOCXNAME[] =
    {
        0x53, 0x00, 0x63, 0x00, 0x72, 0x00, 0x6F, 0x00,
        0x6C, 0x00, 0x6C, 0x00, 0x42, 0x00, 0x61, 0x00,
        0x72, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    {
        SvStorageStreamRef xStor2( rObj->OpenStream( C2S("\3OCXNAME")));
        xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
        DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenStream( C2S("contents")));
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
    mnWidth = rSize.Width;
    mnHeight = rSize.Height;

    GetInt32Property( mnForeColor, rPropSet, WW8_ASCII2STR( "SymbolColor" ),     0x00000001 );
    GetInt32Property( mnBackColor, rPropSet, WW8_ASCII2STR( "BackgroundColor" ), 0x00000002 );
    GetBoolProperty(  mbEnabled,   rPropSet, WW8_ASCII2STR( "Enabled" ),         0x00000304 );
    GetInt32Property( mnMin,       rPropSet, WW8_ASCII2STR( "ScrollValueMin" ),  0x00000020 );
    GetInt32Property( mnMax,       rPropSet, WW8_ASCII2STR( "ScrollValueMax" ),  0x00000040 );
    GetInt32Property( mnValue,     rPropSet, WW8_ASCII2STR( "ScrollValue" ),     0x00000080 );
    GetInt32Property( mnSmallStep, rPropSet, WW8_ASCII2STR( "LineIncrement" ),   0x00000800 );
    GetInt32Property( mnPageStep,  rPropSet, WW8_ASCII2STR( "BlockIncrement" ),  0x00001000 );
    GetInt32Property( mnDelay,     rPropSet, WW8_ASCII2STR( "RepeatDelay" ),     0x00008000 );

    namespace AwtScrollOrient = ::com::sun::star::awt::ScrollBarOrientation;
    sal_Int16 nApiOrient;
    if( rPropSet->getPropertyValue( WW8_ASCII2STR( "Orientation" ) ) >>= nApiOrient )
        UpdateInt32Property( mnOrient, (nApiOrient == AwtScrollOrient::VERTICAL) ? 0 : 1, 0x00002000 );

    UpdateBoolProperty( mbPropThumb, true, 0x00004000 );

    return WriteData( *rObj );
}

// ============================================================================

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
