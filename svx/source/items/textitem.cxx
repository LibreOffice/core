/*************************************************************************
 *
 *  $RCSfile: textitem.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: os $ $Date: 2001-02-19 11:45:35 $
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

// include ---------------------------------------------------------------

#ifndef _COM_SUN_STAR_STYLE_CASEMAP_HPP_
#include <com/sun/star/style/CaseMap.hpp>
#endif

#ifndef _SV_BITMAPEX_HXX
#include <vcl/bitmapex.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#include <toolkit/unohlp.hxx>

#pragma hdrstop

#define ITEMID_FONTLIST         0
#define ITEMID_FONT             0
#define ITEMID_POSTURE          0
#define ITEMID_WEIGHT           0
#define ITEMID_FONTHEIGHT       0
#define ITEMID_UNDERLINE        0
#define ITEMID_CROSSEDOUT       0
#define ITEMID_SHADOWED         0
#define ITEMID_AUTOKERN         0
#define ITEMID_WORDLINEMODE     0
#define ITEMID_CONTOUR          0
#define ITEMID_PROPSIZE         0
#define ITEMID_COLOR            0
#define ITEMID_CHARSETCOLOR     0
#define ITEMID_KERNING          0
#define ITEMID_CASEMAP          0
#define ITEMID_ESCAPEMENT       0
#define ITEMID_LANGUAGE         0
#define ITEMID_NOLINEBREAK      0
#define ITEMID_NOHYPHENHERE     0
#define ITEMID_BLINK            0
#define ITEMID_EMPHASISMARK     0
#define ITEMID_TWOLINES         0
#define ITEMID_CHARROTATE       0
#define ITEMID_CHARSCALE_W      0

#include <svtools/sbx.hxx>
#define GLOBALOVERFLOW3


#include <svtools/memberid.hrc>
#include "svxids.hrc"
#include "svxitems.hrc"

#ifndef NOOLDSV //autogen
#include <vcl/system.hxx>
#endif
#ifndef _VCL_VCLENUM_HXX
#include <vcl/vclenum.hxx>
#endif

#ifndef _BIGINT_HXX //autogen
#include <tools/bigint.hxx>
#endif

// xml stuff
#ifndef _XMLOFF_XMLKYWD_HXX
#include <xmloff/xmlkywd.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif
#include <rtl/ustring>
#include <rtl/ustrbuf.hxx>
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif

#ifndef _COM_SUN_STAR_STYLE_CASEMAP_HPP_
#include <com/sun/star/style/CaseMap.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_SIMPLEFONTMETRIC_HPP_
#include <com/sun/star/awt/SimpleFontMetric.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTWEIGHT_HPP_
#include <com/sun/star/awt/FontWeight.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTSLANT_HPP_
#include <com/sun/star/awt/FontSlant.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_CHARSET_HPP_
#include <com/sun/star/awt/CharSet.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTDESCRIPTOR_HPP_
#include <com/sun/star/awt/FontDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTWIDTH_HPP_
#include <com/sun/star/awt/FontWidth.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XFONT_HPP_
#include <com/sun/star/awt/XFont.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTTYPE_HPP_
#include <com/sun/star/awt/FontType.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTUNDERLINE_HPP_
#include <com/sun/star/awt/FontUnderline.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTSTRIKEOUT_HPP_
#include <com/sun/star/awt/FontStrikeout.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTFAMILY_HPP_
#include <com/sun/star/awt/FontFamily.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTPITCH_HPP_
#include <com/sun/star/awt/FontPitch.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_FONTEMPHASIS_HPP_
#include <com/sun/star/text/FontEmphasis.hpp>
#endif

#ifndef _SVX_UNOMID_HXX
#include <unomid.hxx>
#endif


#include "flstitem.hxx"
#include "fontitem.hxx"
#include "postitem.hxx"
#include "wghtitem.hxx"
#include "fhgtitem.hxx"
#include "fwdtitem.hxx"
#include "udlnitem.hxx"
#include "crsditem.hxx"
#include "shdditem.hxx"
#include "akrnitem.hxx"
#include "wrlmitem.hxx"
#include "cntritem.hxx"
#include "prszitem.hxx"
#include "colritem.hxx"
#include "cscoitem.hxx"
#include "kernitem.hxx"
#include "cmapitem.hxx"
#include "escpitem.hxx"
#include "langitem.hxx"
#include "nlbkitem.hxx"
#include "nhypitem.hxx"
#include "lcolitem.hxx"
#include "blnkitem.hxx"
#include "emphitem.hxx"
#include "twolinesitem.hxx"
#include "scripttypeitem.hxx"
#include "charrotateitem.hxx"
#include "charscaleitem.hxx"
#include "itemtype.hxx"
#include "dialmgr.hxx"
#include "langtab.hxx"
#include "dlgutil.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::text;

// Konvertierung fuer UNO
#define TWIP_TO_MM100(TWIP)     ((TWIP) >= 0 ? (((TWIP)*127L+36L)/72L) : (((TWIP)*127L-36L)/72L))
#define MM100_TO_TWIP(MM100)    ((MM100) >= 0 ? (((MM100)*72L+63L)/127L) : (((MM100)*72L-63L)/127L))

// STATIC DATA -----------------------------------------------------------

// -----------------------------------------------------------------------

TYPEINIT1(SvxFontListItem, SfxPoolItem);
TYPEINIT1_AUTOFACTORY(SvxFontItem, SfxPoolItem);
TYPEINIT1_AUTOFACTORY(SvxPostureItem, SfxEnumItem);
TYPEINIT1_AUTOFACTORY(SvxWeightItem, SfxEnumItem);
TYPEINIT1_AUTOFACTORY(SvxFontHeightItem, SfxPoolItem);
TYPEINIT1_AUTOFACTORY(SvxFontWidthItem, SfxPoolItem);
TYPEINIT1_AUTOFACTORY(SvxUnderlineItem, SfxEnumItem);
TYPEINIT1_AUTOFACTORY(SvxCrossedOutItem, SfxEnumItem);
TYPEINIT1_AUTOFACTORY(SvxShadowedItem, SfxBoolItem);
TYPEINIT1_AUTOFACTORY(SvxAutoKernItem, SfxBoolItem);
TYPEINIT1_AUTOFACTORY(SvxWordLineModeItem, SfxBoolItem);
TYPEINIT1_AUTOFACTORY(SvxContourItem, SfxBoolItem);
TYPEINIT1_AUTOFACTORY(SvxPropSizeItem, SfxUInt16Item);
TYPEINIT1_AUTOFACTORY(SvxColorItem, SfxPoolItem);
TYPEINIT1_AUTOFACTORY(SvxCharSetColorItem, SvxColorItem);
TYPEINIT1_AUTOFACTORY(SvxKerningItem, SfxInt16Item);
TYPEINIT1_AUTOFACTORY(SvxCaseMapItem, SfxEnumItem);
TYPEINIT1_AUTOFACTORY(SvxEscapementItem, SfxPoolItem);
TYPEINIT1_AUTOFACTORY(SvxLanguageItem, SfxEnumItem);
TYPEINIT1_AUTOFACTORY(SvxNoLinebreakItem, SfxBoolItem);
TYPEINIT1_AUTOFACTORY(SvxNoHyphenItem, SfxBoolItem);
TYPEINIT1_AUTOFACTORY(SvxLineColorItem, SvxColorItem);
TYPEINIT1_AUTOFACTORY(SvxBlinkItem, SfxBoolItem);
TYPEINIT1_AUTOFACTORY(SvxEmphasisMarkItem, SfxUInt16Item);
TYPEINIT1_AUTOFACTORY(SvxTwoLinesItem, SfxPoolItem);
TYPEINIT1_AUTOFACTORY(SvxScriptTypeItem, SfxUInt16Item);
TYPEINIT1_AUTOFACTORY(SvxCharRotateItem, SfxUInt16Item);
TYPEINIT1_AUTOFACTORY(SvxCharScaleWidthItem, SfxUInt16Item);

TYPEINIT1(SvxScriptSetItem, SfxSetItem );


// class SvxFontListItem -------------------------------------------------

SvxFontListItem::SvxFontListItem( const FontList* pFontLst,
                                  const USHORT nId ) :
    SfxPoolItem( nId ),
    pFontList( pFontLst )
{
}

// -----------------------------------------------------------------------

SvxFontListItem::SvxFontListItem( const SvxFontListItem& rItem ) :

    SfxPoolItem( rItem ),
    pFontList( rItem.GetFontList() )
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxFontListItem::Clone( SfxItemPool* ) const
{
    return new SvxFontListItem( *this );
}

// -----------------------------------------------------------------------

int SvxFontListItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    return( pFontList == ((SvxFontListItem&)rAttr).pFontList );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxFontListItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
    rText.Erase();
    return SFX_ITEM_PRESENTATION_NONE;
}

// class SvxFontItem -----------------------------------------------------

SvxFontItem::SvxFontItem( const USHORT nId ) :
    SfxPoolItem( nId )
{
    eFamily = FAMILY_SWISS;
    ePitch = PITCH_VARIABLE;
    eTextEncoding = RTL_TEXTENCODING_DONTKNOW;
}

// -----------------------------------------------------------------------

SvxFontItem::SvxFontItem( const FontFamily eFam, const XubString& aName,
                  const XubString& aStName, const FontPitch eFontPitch,
                  const rtl_TextEncoding eFontTextEncoding, const USHORT nId ) :

    SfxPoolItem( nId ),

    aFamilyName(aName),
    aStyleName(aStName)
{
    eFamily = eFam;
    ePitch = eFontPitch;
    eTextEncoding = eFontTextEncoding;
}

// -----------------------------------------------------------------------

sal_Bool SvxFontItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    switch(nMemberId)
    {
        case MID_FONT_FAMILY_NAME   :
            rVal <<= OUString(aFamilyName.GetBuffer());
        break;
        case MID_FONT_STYLE_NAME:
            rVal <<= OUString(aStyleName.GetBuffer());
        break;
        case MID_FONT_FAMILY    : rVal <<= (sal_Int16)(eFamily);    break;
        case MID_FONT_CHAR_SET  : rVal <<= (sal_Int16)(eTextEncoding);  break;
        case MID_FONT_PITCH     : rVal <<= (sal_Int16)(ePitch); break;
    }
    return sal_True;
}
// -----------------------------------------------------------------------
sal_Bool SvxFontItem::PutValue( const uno::Any& rVal, BYTE nMemberId)
{
    switch(nMemberId)
    {
        case MID_FONT_FAMILY_NAME   :
        {
            OUString aStr;
            if(!(rVal >>= aStr))
                return sal_False;
            aFamilyName = aStr.getStr();
        }
        break;
        case MID_FONT_STYLE_NAME:
        {
            OUString aStr;
            if(!(rVal >>= aStr))
                return sal_False;
            aStyleName = aStr.getStr();
        }
        break;
        case MID_FONT_FAMILY :
        {
            sal_Int16 nFamily;
            if(!(rVal >>= nFamily))
                return sal_False;
            eFamily = (FontFamily)nFamily;
        }
        break;
        case MID_FONT_CHAR_SET  :
        {
            sal_Int16 nSet;
            if(!(rVal >>= nSet))
                return sal_False;
            eTextEncoding = (rtl_TextEncoding)nSet;
        }
        break;
        case MID_FONT_PITCH     :
        {
            sal_Int16 nPitch;
            if(!(rVal >>= nPitch))
                return sal_False;
            ePitch =  (FontPitch)nPitch;
        }
        break;
    }
    return sal_True;
}

// -----------------------------------------------------------------------

#ifndef SVX_LIGHT

void xml_in_font_family_impl( SvxFontItem& rItem, const OUString& rValue )
{
    String sValue;

    sal_Int32 nPos = 0;
    do
    {
        sal_Int32 nFirst = nPos;
        nPos = SvXMLUnitConverter::indexOfComma( rValue, nPos );
        sal_Int32 nLast = (-1 == nPos ? rValue.getLength() : nPos);
        if( nLast > 0 )
            nLast--;

        // skip trailing blanks
        while( sal_Unicode(' ') == rValue[nLast] && nLast > nFirst )
            nLast--;

        // skip leading blanks
        while( sal_Unicode(' ') == rValue[nFirst] && nFirst <= nLast )
            nFirst++;

        // remove quotes
        sal_Unicode c = rValue[nFirst];
        if( nFirst < nLast && (sal_Unicode('\'') == c || sal_Unicode('\"') == c) && rValue[nLast] == c )
        {
            nFirst++;
            nLast--;
        }

        if( nFirst <= nLast )
        {
            if( sValue.Len() != 0 )
                sValue += sal_Unicode(';');

            OUString sTemp = rValue.copy( nFirst, nLast-nFirst+1 );
            sValue += sTemp.getStr();
        }

        if( -1 != nPos )
            nPos++;
    }
    while( -1 != nPos );

    if( sValue.Len() )
        rItem.GetFamilyName() = sValue;
}

static SvXMLEnumMapEntry __READONLY_DATA aFontFamilyGenericMap[] =
{
    { sXML_decorative,  FAMILY_DECORATIVE },

    { sXML_modern,      FAMILY_MODERN   },
    { sXML_roman,       FAMILY_ROMAN    },
    { sXML_script,      FAMILY_SCRIPT   },
    { sXML_swiss,       FAMILY_SWISS    },
    { sXML_system,      FAMILY_SYSTEM   },
    { 0,                0               }
};

static SvXMLEnumMapEntry __READONLY_DATA aFontPitchMap[] =
{
    { sXML_fixed,       PITCH_FIXED     },

    { sXML_variable,    PITCH_VARIABLE  },
    { 0,                0               }
};
#endif

sal_Bool SvxFontItem::importXML( const OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter )
{
#ifndef SVX_LIGHT
    switch( nMemberId )
    {
    case MID_FONT_FAMILY_NAME:
        xml_in_font_family_impl( *this, rValue );
        break;
    case MID_FONT_STYLE_NAME:
        aStyleName = rValue.getStr();
        break;
    case MID_FONT_FAMILY:
        {
            sal_uInt16 eNewFamily;
            if( !rUnitConverter.convertEnum( eNewFamily, rValue, aFontFamilyGenericMap ) )
                return sal_False;
            eFamily =  (FontFamily)eNewFamily;
        }
        break;
    case MID_FONT_CHAR_SET:
        if( rValue.compareToAscii(sXML_x_symbol ) )
            eTextEncoding = RTL_TEXTENCODING_SYMBOL;
        break;
    case MID_FONT_PITCH:
        {
            USHORT eNewPitch;
            if( !rUnitConverter.convertEnum( eNewPitch, rValue, aFontPitchMap ) )
                return sal_False;
            ePitch = (FontPitch)eNewPitch;
        }
        break;
    }

    return sal_True;
#else
    return sal_False;
#endif
}

// -----------------------------------------------------------------------

#ifndef SVX_LIGHT
void xml_out_font_family_impl( const SvxFontItem& rFont, OUString& rValue )
{
    OUString sFamilies( rFont.GetFamilyName().GetBuffer() );

    OUStringBuffer sValue( sFamilies.getLength() + 2L );
    sal_Int32 nPos = 0L;
    do
    {
        sal_Int32 nFirst = nPos;
        nPos = sFamilies.indexOf( sal_Unicode(';'), nPos );
        sal_Int32 nLast = (-1L == nPos ? sFamilies.getLength() : nPos);
        if( nLast > 0L )
            nLast--;

        // skip trailing blanks
        while( sal_Unicode(' ') == sFamilies[nLast] && nLast > nFirst )
            nLast--;

        // skip leading blanks
        while( sal_Unicode(' ') == sFamilies[nFirst] && nFirst <= nLast )
            nFirst++;

        if( nFirst <= nLast )
        {
            if( sValue.getLength() != 0L )
            {
                sValue.append( sal_Unicode( ',' ) );
                sValue.append( sal_Unicode( ' ' ));
            }
            sal_Int32 nLen = nLast-nFirst+1L;
            OUString sFamily( sFamilies.copy( nFirst, nLen ) );
            sal_Bool bQuote = sal_False;
            for( sal_Int32 i=0; i < nLen; i++ )
            {
                sal_Unicode c = sFamily[i];
                if( sal_Unicode(' ') == c || sal_Unicode(',') == c )
                {
                    bQuote = sal_True;
                    break;
                }
            }
            if( bQuote )
                sValue.append( sal_Unicode('\'') );
            sValue.append( sFamily );
            if( bQuote )
                sValue.append( sal_Unicode('\'') );
        }

        if( -1L != nPos )
            nPos++;
    }
    while( -1L != nPos );

    rValue = sValue.makeStringAndClear();
}
#endif

sal_Bool SvxFontItem::exportXML( OUString& rValue, USHORT nMemberId, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bOk = sal_False;
#ifndef SVX_LIGHT

    OUStringBuffer aOut;

    switch( nMemberId )
    {
    case MID_FONT_FAMILY_NAME:
        xml_out_font_family_impl( *this, rValue );
        return sal_True;

    case MID_FONT_STYLE_NAME:
        if( aStyleName.Len() != 0 )
        {
            OUString sTmp( aStyleName.GetBuffer() );
            aOut.append( sTmp );
            bOk = sal_True;
        }
        break;

    case MID_FONT_FAMILY:
        if( eFamily != FAMILY_DONTKNOW )
            bOk = rUnitConverter.convertEnum( aOut, eFamily, aFontFamilyGenericMap );
        break;

    case MID_FONT_CHAR_SET:
        // MIB 30.9.99: As long as we don't support unicode at the core, we
        // must know if a font is a symbol font. This is not required to
        // convert characters, but to set the right encoding within the
        // font attributes.

        // see:
        // ftp://ftp.isi.edu/in-notes/iana/assignments/character-sets
        // http://msdn.microsoft.com/workshop/author/dhtml/reference/charsets/charset4.asp
        // Mozilla (libi18n/csnametb.c)
        if( eTextEncoding == RTL_TEXTENCODING_SYMBOL )
        {
            aOut.appendAscii( sXML_x_symbol );
            bOk = sal_True;
        }
        break;

    case MID_FONT_PITCH:
        if( PITCH_DONTKNOW != ePitch )
            bOk = rUnitConverter.convertEnum( aOut, ePitch, aFontPitchMap, sXML_fixed );
        break;
    }

    rValue = aOut.makeStringAndClear();

#endif
    return bOk;
}

// -----------------------------------------------------------------------

int SvxFontItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    const SvxFontItem& rItem = (const SvxFontItem&)rAttr;

    int bRet = ( eFamily == rItem.eFamily &&
                 aFamilyName == rItem.aFamilyName &&
                 aStyleName == rItem.aStyleName );

    if ( bRet )
    {
        if ( ePitch != rItem.ePitch || eTextEncoding != rItem.eTextEncoding )
        {
            bRet = sal_False;
            DBG_WARNING( "FontItem::operator==(): nur Pitch oder rtl_TextEncoding unterschiedlich" );
        }
    }
    return bRet;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxFontItem::Clone( SfxItemPool * ) const
{
    return new SvxFontItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxFontItem::Store( SvStream& rStrm , USHORT nItemVersion ) const
{
    rStrm << (BYTE) GetFamily()
          << (BYTE) GetPitch()
          << (BYTE) GetStoreCharSet( GetCharSet(), rStrm.GetVersion() );

    // UNICODE: rStrm << GetFamilyName();
    rStrm.WriteByteString(GetFamilyName());

    // UNICODE: rStrm << GetStyleName();
    rStrm.WriteByteString(GetStyleName());

    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxFontItem::Create(SvStream& rStrm, USHORT) const
{
    BYTE eFamily, eFontPitch, eFontTextEncoding;
    String aName, aStyle;
    rStrm >> eFamily;
    rStrm >> eFontPitch;
    rStrm >> eFontTextEncoding;

    // UNICODE: rStrm >> aName;
    rStrm.ReadByteString(aName);

    // UNICODE: rStrm >> aStyle;
    rStrm.ReadByteString(aStyle);

    // irgendwann wandelte sich der StarBats vom ANSI- zum SYMBOL-Font
    if ( RTL_TEXTENCODING_SYMBOL != eFontTextEncoding && aName.EqualsAscii("StarBats") )
        eFontTextEncoding = RTL_TEXTENCODING_SYMBOL;

    return new SvxFontItem(  (FontFamily)eFamily, aName, aStyle,
                             (FontPitch)eFontPitch,  (rtl_TextEncoding)eFontTextEncoding,
                            Which() );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxFontItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = aFamilyName;
            return ePres;
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// class SvxPostureItem --------------------------------------------------

SvxPostureItem::SvxPostureItem( const FontItalic ePosture, const USHORT nId ) :
    SfxEnumItem( nId, ePosture )
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxPostureItem::Clone( SfxItemPool * ) const
{
    return new SvxPostureItem( *this );
}

// -----------------------------------------------------------------------

USHORT SvxPostureItem::GetValueCount() const
{
    return ITALIC_NORMAL + 1;   // auch ITALIC_NONE geh"ort dazu
}

// -----------------------------------------------------------------------

SvStream& SvxPostureItem::Store( SvStream& rStrm , USHORT nItemVersion ) const
{
    rStrm << (BYTE)GetValue();
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxPostureItem::Create(SvStream& rStrm, USHORT) const
{
    BYTE nPosture;
    rStrm >> nPosture;
    return new SvxPostureItem( (const FontItalic)nPosture, Which() );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxPostureItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = GetValueTextByPos( GetValue() );
            return ePres;
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

XubString SvxPostureItem::GetValueTextByPos( USHORT nPos ) const
{
    DBG_ASSERT( nPos <= (USHORT)ITALIC_NORMAL, "enum overflow!" );

    XubString sTxt;
    FontItalic eItalic = (FontItalic)nPos;
    USHORT nId = 0;

    switch ( eItalic )
    {
        case ITALIC_NONE:       nId = RID_SVXITEMS_ITALIC_NONE;     break;
        case ITALIC_OBLIQUE:    nId = RID_SVXITEMS_ITALIC_OBLIQUE;  break;
        case ITALIC_NORMAL:     nId = RID_SVXITEMS_ITALIC_NORMAL;   break;
    }

    if ( nId )
        sTxt = SVX_RESSTR( nId );
    return sTxt;
}


/*-----------------13.03.98 14:28-------------------

--------------------------------------------------*/
sal_Bool SvxPostureItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    switch( nMemberId )
    {
        case MID_ITALIC:
            rVal = Bool2Any(GetBoolValue());
            break;
        case MID_POSTURE:
            rVal <<= (awt::FontSlant)GetValue();    // Werte von awt::FontSlant und FontItalic sind gleich
            break;
    }
    return sal_True;
}
/*-----------------13.03.98 14:28-------------------

--------------------------------------------------*/
sal_Bool SvxPostureItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    switch( nMemberId )
    {
        case MID_ITALIC:
            SetBoolValue(Any2Bool(rVal));
        break;
        case MID_POSTURE:
        {
            awt::FontSlant eSlant;
            if(!(rVal >>= eSlant))
            {
                sal_Int32 nValue;
                if(!(rVal >>= nValue))
                    return sal_False;

                eSlant = (awt::FontSlant)nValue;
            }
            SetValue((sal_Int32)eSlant);
        }
    }
    return sal_True;
}

#ifndef SVX_LIGHT
static SvXMLEnumMapEntry __READONLY_DATA aPostureGenericMap[] =
{
    { sXML_posture_normal,      ITALIC_NONE     },
    { sXML_posture_italic,      ITALIC_NORMAL   },
    { sXML_posture_oblique,     ITALIC_OBLIQUE  },
    { 0,                        0               }
};
#endif

// -----------------------------------------------------------------------

sal_Bool SvxPostureItem::importXML( const OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter )
{
#ifndef SVX_LIGHT
    sal_uInt16 ePosture;
    if( rUnitConverter.convertEnum( ePosture, rValue, aPostureGenericMap ) )
    {
        SetValue( ePosture );
        return sal_True;
    }

#endif
    return sal_False;
}

// -----------------------------------------------------------------------


sal_Bool SvxPostureItem::exportXML( OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter ) const
{
#ifndef SVX_LIGHT
    OUStringBuffer aOut;
    if( rUnitConverter.convertEnum( aOut, GetValue(), aPostureGenericMap ) )
    {
        rValue = aOut.makeStringAndClear();
        return sal_True;
    }

#endif
    return sal_False;
}

// -----------------------------------------------------------------------

int SvxPostureItem::HasBoolValue() const
{
    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool SvxPostureItem::GetBoolValue() const
{
    return ( (FontItalic)GetValue() >= ITALIC_OBLIQUE );
}

// -----------------------------------------------------------------------

void SvxPostureItem::SetBoolValue( sal_Bool bVal )
{
    SetValue( bVal ? ITALIC_NORMAL : ITALIC_NONE );
}

// class SvxWeightItem ---------------------------------------------------

SvxWeightItem::SvxWeightItem( const FontWeight eWght, const USHORT nId ) :
    SfxEnumItem( nId, eWght )
{
}



// -----------------------------------------------------------------------

int SvxWeightItem::HasBoolValue() const
{
    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool SvxWeightItem::GetBoolValue() const
{
    return  (FontWeight)GetValue() >= WEIGHT_BOLD;
}

// -----------------------------------------------------------------------

void SvxWeightItem::SetBoolValue( sal_Bool bVal )
{
    SetValue( bVal ? WEIGHT_BOLD : WEIGHT_NORMAL );
}

// -----------------------------------------------------------------------

USHORT SvxWeightItem::GetValueCount() const
{
    return WEIGHT_BLACK;    // WEIGHT_DONTKNOW geh"ort nicht dazu
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxWeightItem::Clone( SfxItemPool * ) const
{
    return new SvxWeightItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxWeightItem::Store( SvStream& rStrm , USHORT nItemVersion ) const
{
    rStrm << (BYTE)GetValue();
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxWeightItem::Create(SvStream& rStrm, USHORT) const
{
    BYTE nWeight;
    rStrm >> nWeight;
    return new SvxWeightItem( (FontWeight)nWeight, Which() );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxWeightItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = GetValueTextByPos( GetValue() );
            return ePres;
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

XubString SvxWeightItem::GetValueTextByPos( USHORT nPos ) const
{
    DBG_ASSERT( nPos <= (USHORT)WEIGHT_BLACK, "enum overflow!" );
    return SVX_RESSTR( RID_SVXITEMS_WEIGHT_BEGIN + nPos );
}

/*-----------------13.03.98 14:18-------------------

--------------------------------------------------*/
sal_Bool SvxWeightItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    switch( nMemberId )
    {
        case MID_BOLD   :
            rVal = Bool2Any(GetBoolValue());
        break;
        case MID_WEIGHT:
        {
            rVal <<= (float)( VCLUnoHelper::ConvertFontWeight( (FontWeight)GetValue() ) );
        }
        break;
    }
    return sal_True;
}
/*-----------------13.03.98 14:18-------------------

--------------------------------------------------*/
sal_Bool SvxWeightItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    switch( nMemberId )
    {
        case MID_BOLD   :
            SetBoolValue(Any2Bool(rVal));
        break;
        case MID_WEIGHT:
        {
            float fValue;
            if(!(rVal >>= fValue))
            {
                sal_Int32 nValue;
                if(!(rVal >>= nValue))
                    return sal_False;
                fValue = (float)nValue;
            }
            SetValue( VCLUnoHelper::ConvertFontWeight(fValue) );
        }
        break;
    }
    return sal_True;
}

// -----------------------------------------------------------------------

// TODO:.is this apropriate?
#ifndef SVX_LIGHT

static struct
{
    FontWeight eWeight;
    USHORT nValue;
}
aFontWeightMap[] =
{
    { WEIGHT_DONTKNOW,      0 },
    { WEIGHT_THIN,          100 },
    { WEIGHT_ULTRALIGHT,    150 },
    { WEIGHT_LIGHT,         250 },
    { WEIGHT_SEMILIGHT,     350 },
    { WEIGHT_NORMAL,        400 },
    { WEIGHT_MEDIUM,        450 },
    { WEIGHT_SEMIBOLD,      600 },
    { WEIGHT_BOLD,          700 },
    { WEIGHT_ULTRABOLD,     800 },
    { WEIGHT_BLACK,         900 },
    { (FontWeight)USHRT_MAX,       1000 }
};
#endif

sal_Bool SvxWeightItem::importXML( const OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter )
{
#ifndef SVX_LIGHT
    sal_uInt16 nWeight = 0;

    if( rValue.compareToAscii( sXML_weight_normal ) )
    {
        nWeight = 400;
    }
    else if( rValue.compareToAscii( sXML_weight_bold ) )
    {
        nWeight = 700;
    }
    else
    {
        sal_Int32 nTemp;
        if( !rUnitConverter.convertNumber( nTemp, rValue, 100, 900 ) )
            return sal_False;
        nWeight = nTemp;
    }

    for( int i = 0; aFontWeightMap[i].eWeight != USHRT_MAX; i++ )
    {
        if( (nWeight >= aFontWeightMap[i].nValue) && (nWeight <= aFontWeightMap[i+1].nValue) )
        {
            sal_uInt16 nDiff1 = aFontWeightMap[i].nValue - nWeight;
            sal_uInt16 nDiff2 = nWeight - aFontWeightMap[i+1].nValue;

            if( nDiff1 < nDiff2 )
                SetValue( aFontWeightMap[i].eWeight );
            else
                SetValue( aFontWeightMap[i+1].eWeight );

            return sal_True;
        }
    }

    SetValue( WEIGHT_DONTKNOW );
#endif
    return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool SvxWeightItem::exportXML( OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter ) const
{
#ifndef SVX_LIGHT
    sal_uInt16 nWeight = 0;

    for( int i = 0; aFontWeightMap[i].eWeight != -1; i++ )
    {
        if( aFontWeightMap[i].eWeight == GetValue() )
        {
             nWeight = aFontWeightMap[i].nValue;
             break;
        }
    }

    OUStringBuffer aOut;

    if( 400 == nWeight )
        aOut.appendAscii( sXML_weight_normal );
    else if( 700 == nWeight )
        aOut.appendAscii( sXML_weight_bold );
    else
        rUnitConverter.convertNumber( aOut, (sal_Int32) nWeight );

    rValue = aOut.makeStringAndClear();
    return sal_True;
#else
    return sal_False;
#endif
}

// class SvxFontHeightItem -----------------------------------------------

SvxFontHeightItem::SvxFontHeightItem( const ULONG nSz,
                                      const USHORT nPrp,
                                      const USHORT nId ) :
    SfxPoolItem( nId )
{
    SetHeight( nSz,nPrp );  // mit den Prozenten rechnen
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxFontHeightItem::Clone( SfxItemPool * ) const
{
    return new SvxFontHeightItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxFontHeightItem::Store( SvStream& rStrm , USHORT nItemVersion ) const
{
    rStrm << (USHORT)GetHeight();

    if( FONTHEIGHT_UNIT_VERSION <= nItemVersion )
        rStrm << GetProp() << (USHORT)GetPropUnit();
    else
    {
        // JP 30.06.98: beim Export in alte Versionen geht die relative
        // Angabe verloren, wenn es keine Prozentuale ist
        USHORT nProp = GetProp();
        if( SFX_MAPUNIT_RELATIVE != GetPropUnit() )
            nProp = 100;
        rStrm << nProp;
    }
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxFontHeightItem::Create( SvStream& rStrm,
                                                 USHORT nVersion ) const
{
    USHORT nsize, nprop = 0, nPropUnit = SFX_MAPUNIT_RELATIVE;

    rStrm >> nsize;

    if( FONTHEIGHT_16_VERSION <= nVersion )
        rStrm >> nprop;
    else
    {
        BYTE nP;
        rStrm  >> nP;
        nprop = (USHORT)nP;
    }

    if( FONTHEIGHT_UNIT_VERSION <= nVersion )
        rStrm >> nPropUnit;

    SvxFontHeightItem* pItem = new SvxFontHeightItem( nsize, 100, Which() );
    pItem->SetProp( nprop, (SfxMapUnit)nPropUnit );
    return pItem;
}

// -----------------------------------------------------------------------

int SvxFontHeightItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
    return GetHeight() == ((SvxFontHeightItem&)rItem).GetHeight() &&
            GetProp() == ((SvxFontHeightItem&)rItem).GetProp() &&
            GetPropUnit() == ((SvxFontHeightItem&)rItem).GetPropUnit();
}

/*-----------------13.03.98 14:53-------------------

--------------------------------------------------*/
sal_Bool SvxFontHeightItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    //  In StarOne sind im uno::Any immer 1/100mm. Ueber die MemberId wird
    //  gesteuert, ob der Wert im Item 1/100mm oder Twips sind.

    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
        case MID_FONTHEIGHT:
        {
            //  Point (also Twips) sind gefragt,
            //  also umrechnen, wenn CONVERT_TWIPS nicht gesetzt ist
            long nTwips = bConvert ? nHeight : MM100_TO_TWIP(nHeight);
            rVal <<= (float)( nTwips / 20.0 );
        }
        break;
        case MID_FONTHEIGHT_PROP:
            rVal <<= (sal_Int16)(SFX_MAPUNIT_RELATIVE == ePropUnit ? nProp : 100);
        break;
        case MID_FONTHEIGHT_DIFF:
        {
            float fRet = (float)nProp;
            switch( ePropUnit )
            {
                case SFX_MAPUNIT_RELATIVE:
                    fRet = 0.;
                break;
                case SFX_MAPUNIT_100TH_MM:
                    fRet = MM100_TO_TWIP(fRet);
                    fRet /= 20.;
                break;
                case SFX_MAPUNIT_POINT:

                break;
                case SFX_MAPUNIT_TWIP:
                    fRet /= 20.;
                break;
            }
            rVal <<= fRet;
        }
        break;
    }
    return sal_True;
}
/* -----------------01.07.98 13:43-------------------
 *  Relative Abweichung aus der Hoehe herausrechnen
 * --------------------------------------------------*/
sal_uInt32 lcl_GetRealHeight_Impl(sal_uInt32 nHeight, sal_uInt16 nProp, SfxMapUnit eProp, sal_Bool bCoreInTwip)
{
    sal_uInt32 nRet = nHeight;
    short nDiff = 0;
    switch( eProp )
    {
        case SFX_MAPUNIT_RELATIVE:
            nRet *= 100;
            nRet /= nProp;
        break;
        case SFX_MAPUNIT_POINT:
        {
            short nTemp = (short)nDiff;
            nDiff = nTemp * 20;
            if(!bCoreInTwip)
                nDiff = (short)TWIP_TO_MM100((long)(nDiff));
        }
        break;
        case SFX_MAPUNIT_100TH_MM:
            //dann ist die Core doch wohl auch in 1/100 mm
            nDiff = (short)nProp;
        break;
        case SFX_MAPUNIT_TWIP:
            // hier doch sicher TWIP
            nDiff = ((short)nProp);
        break;
    }
    nRet -= nDiff;

    return nRet;
}

/*-----------------13.03.98 14:53-------------------

--------------------------------------------------*/
sal_Bool SvxFontHeightItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
        case MID_FONTHEIGHT:
        {
            ePropUnit = SFX_MAPUNIT_RELATIVE;
            nProp = 100;
            float fPoint;
            if(!(rVal >>= fPoint))
            {
                sal_Int32 nValue;
                if(!(rVal >>= nValue))
                    return sal_False;
                fPoint = (float)nValue;
            }

            nHeight = (long)( fPoint * 20.0 + 0.5 );        // Twips
            if (!bConvert)
                nHeight = TWIP_TO_MM100(nHeight);   // umrechnen, wenn das Item 1/100mm enthaelt
        }
        break;
        case MID_FONTHEIGHT_PROP:
        {
            sal_Int16 nNew;
            if(!(rVal >>= nNew))
                return sal_True;

            nHeight = lcl_GetRealHeight_Impl(nHeight, nProp, ePropUnit, bConvert);

            nHeight *= nNew;
            nHeight /= 100;
            nProp = nNew;
            ePropUnit = SFX_MAPUNIT_RELATIVE;
        }
        break;
        case MID_FONTHEIGHT_DIFF:
        {
            nHeight = lcl_GetRealHeight_Impl(nHeight, nProp, ePropUnit, bConvert);
            float fValue;
            if(!(rVal >>= fValue))
            {
                sal_Int32 nValue;
                if(!(rVal >>= nValue))
                    return sal_False;
                fValue = (float)nValue;
            }
            sal_uInt16 nCoreDiffValue = (sal_uInt16)(fValue * 20.);
            nHeight += bConvert ? nCoreDiffValue : TWIP_TO_MM100(nCoreDiffValue);
            nProp = (sal_uInt16)fValue;
            ePropUnit = SFX_MAPUNIT_POINT;
        }
        break;
    }
    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool SvxFontHeightItem::importXML( const OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter )
{
    sal_Bool bOk = sal_False;
#ifndef SVX_LIGHT

    sal_Int32 nPrc = 100;
    sal_Int32 nAbs = 0;

    if( rValue.indexOf( sal_Unicode('%') ) != -1 )
        bOk = rUnitConverter.convertPercent( nPrc, rValue );
    else
        bOk = rUnitConverter.convertMeasure( nAbs, rValue );

    if( bOk )
        SetHeight( (sal_uInt32)nAbs, (sal_uInt16)nPrc );

#endif
    return bOk;
}

// -----------------------------------------------------------------------

sal_Bool SvxFontHeightItem::exportXML( OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter ) const
{
#ifndef SVX_LIGHT
    OUStringBuffer aOut;

    if( GetProp() != 100 )
        rUnitConverter.convertPercent( aOut, GetProp() );
    else
        SvXMLUnitConverter::convertMeasure( aOut, GetHeight(),
                                            rUnitConverter.getCoreMeasureUnit(),
                                            MAP_POINT );

    rValue = aOut.makeStringAndClear();

    return sal_True;
#else
    return sal_False;
#endif
}

//------------------------------------------------------------------------

SfxItemPresentation SvxFontHeightItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
#ifndef SVX_LIGHT
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            if( SFX_MAPUNIT_RELATIVE != ePropUnit )
            {
                ( rText = String::CreateFromInt32( (short)nProp ) ) +=
                        SVX_RESSTR( GetMetricId( ePropUnit ) );
                if( 0 <= (short)nProp )
                    rText.Insert( sal_Unicode('+'), 0 );
            }
            else if( 100 == nProp )
            {
                rText = GetMetricText( (long)nHeight,
                                        eCoreUnit, SFX_MAPUNIT_POINT );
                rText += SVX_RESSTR(GetMetricId(SFX_MAPUNIT_POINT));
            }
            else
                ( rText = String::CreateFromInt32( nProp )) += sal_Unicode('%');
            return ePres;
        }
    }
#endif
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

USHORT SvxFontHeightItem::GetVersion(USHORT nFileVersion) const
{
    return (nFileVersion <= SOFFICE_FILEFORMAT_40)
               ? FONTHEIGHT_16_VERSION
               : FONTHEIGHT_UNIT_VERSION;
}

// -----------------------------------------------------------------------

int SvxFontHeightItem::ScaleMetrics( long nMult, long nDiv )
{
    nHeight = (sal_uInt32)Scale( nHeight, nMult, nDiv );
    return 1;
}

// -----------------------------------------------------------------------

int SvxFontHeightItem::HasMetrics() const
{
    return 1;
}

void SvxFontHeightItem::SetHeight( sal_uInt32 nNewHeight, const USHORT nNewProp,
                                    SfxMapUnit eUnit )
{
    DBG_ASSERT( GetRef() == 0, "SetValue() with pooled item" );

#ifndef SVX_LIGHT
    if( SFX_MAPUNIT_RELATIVE != eUnit )
        nHeight = nNewHeight + ::ItemToControl( (short)nNewProp, eUnit,
                                                SFX_FUNIT_TWIP );
    else
#endif // !SVX_LIGHT
    if( 100 != nNewProp )
        nHeight = sal_uInt32(( nNewHeight * nNewProp ) / 100 );
    else
        nHeight = nNewHeight;

    nProp = nNewProp;
    ePropUnit = eUnit;
}

void SvxFontHeightItem::SetHeight( sal_uInt32 nNewHeight, USHORT nNewProp,
                                 SfxMapUnit eMetric, SfxMapUnit eCoreMetric )
{
    DBG_ASSERT( GetRef() == 0, "SetValue() with pooled item" );

#ifndef SVX_LIGHT
    if( SFX_MAPUNIT_RELATIVE != eMetric )
        nHeight = nNewHeight +
                ::ControlToItem( ::ItemToControl((short)nNewProp, eMetric,
                                        SFX_FUNIT_TWIP ), SFX_FUNIT_TWIP,
                                        eCoreMetric );
    else
#endif // !SVX_LIGHT
    if( 100 != nNewProp )
        nHeight = sal_uInt32(( nNewHeight * nNewProp ) / 100 );
    else
        nHeight = nNewHeight;

    nProp = nNewProp;
    ePropUnit = eMetric;
}

// class SvxFontWidthItem -----------------------------------------------

SvxFontWidthItem::SvxFontWidthItem( const USHORT nSz, const USHORT nPrp, const USHORT nId ) :
    SfxPoolItem( nId )
{
    nWidth = nSz;
    nProp = nPrp;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxFontWidthItem::Clone( SfxItemPool * ) const
{
    return new SvxFontWidthItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxFontWidthItem::Store( SvStream& rStrm , USHORT nItemVersion ) const
{
    rStrm << GetWidth() << GetProp();
    return rStrm;
}

// -----------------------------------------------------------------------

int SvxFontWidthItem::ScaleMetrics( long nMult, long nDiv )
{
    nWidth = (USHORT)Scale( nWidth, nMult, nDiv );
    return 1;
}

// -----------------------------------------------------------------------

int SvxFontWidthItem::HasMetrics() const
{
    return 1;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxFontWidthItem::Create( SvStream& rStrm,
                                                 USHORT nVersion ) const
{
    USHORT nS;
    USHORT nP;

    rStrm >> nS;
    rStrm >> nP;
    SvxFontWidthItem* pItem = new SvxFontWidthItem( 0, nP, Which() );
    pItem->SetWidthValue( nS );
    return pItem;
}

// -----------------------------------------------------------------------

int SvxFontWidthItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
    return GetWidth() == ((SvxFontWidthItem&)rItem).GetWidth() &&
            GetProp() == ((SvxFontWidthItem&)rItem).GetProp();
}

/*-----------------13.03.98 16:03-------------------

--------------------------------------------------*/
sal_Bool SvxFontWidthItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    switch(nMemberId)
    {
        case MID_FONTWIDTH:
            rVal <<= (sal_Int16)(nWidth);
        break;
        case MID_FONTWIDTH_PROP:
            rVal <<= (sal_Int16)(nProp);
        break;
    }
    return sal_True;
}
/*-----------------13.03.98 16:03-------------------

--------------------------------------------------*/
sal_Bool SvxFontWidthItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    sal_Int16 nVal;
    if(!(rVal >>= nVal))
        return sal_False;

    switch(nMemberId)
    {
        case MID_FONTWIDTH:
            nProp = nVal;
        break;
        case MID_FONTWIDTH_PROP:
            nWidth = nVal;
        break;
    }
    return sal_True;
}

//------------------------------------------------------------------------

SfxItemPresentation SvxFontWidthItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
#ifndef SVX_LIGHT
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            if ( 100 == nProp )
            {
                rText = GetMetricText( (long)nWidth,
                                        eCoreUnit, SFX_MAPUNIT_POINT );
                rText += SVX_RESSTR(GetMetricId(SFX_MAPUNIT_POINT));
            }
            else
                ( rText = String::CreateFromInt32( nProp )) += sal_Unicode('%');
            return ePres;
        }
    }
#endif
    return SFX_ITEM_PRESENTATION_NONE;
}

// class SvxUnderlineItem ------------------------------------------------

SvxUnderlineItem::SvxUnderlineItem( const FontUnderline eSt, const USHORT nId )
    : SfxEnumItem( nId, eSt ), mColor( COL_TRANSPARENT )
{
}

// -----------------------------------------------------------------------

int SvxUnderlineItem::HasBoolValue() const
{
    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool SvxUnderlineItem::GetBoolValue() const
{
    return  (FontUnderline)GetValue() != UNDERLINE_NONE;
}

// -----------------------------------------------------------------------

void SvxUnderlineItem::SetBoolValue( sal_Bool bVal )
{
    SetValue( bVal ? UNDERLINE_SINGLE : UNDERLINE_NONE );
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxUnderlineItem::Clone( SfxItemPool * ) const
{
    SvxUnderlineItem* pNew = new SvxUnderlineItem( *this );
    pNew->SetColor( GetColor() );
    return pNew;
}

// -----------------------------------------------------------------------

USHORT SvxUnderlineItem::GetValueCount() const
{
    return UNDERLINE_DOTTED + 1;    // auch UNDERLINE_NONE geh"ort dazu
}

// -----------------------------------------------------------------------

SvStream& SvxUnderlineItem::Store( SvStream& rStrm , USHORT nItemVersion ) const
{
    rStrm << (BYTE)GetValue();
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxUnderlineItem::Create(SvStream& rStrm, USHORT) const
{
    BYTE nState;
    rStrm >> nState;
    return new SvxUnderlineItem(  (FontUnderline)nState, Which() );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxUnderlineItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
#ifndef SVX_LIGHT
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = GetValueTextByPos( GetValue() );
            if( !mColor.GetTransparency() )
                ( rText += cpDelim ) += ::GetColorString( mColor );
            return ePres;
    }
#endif
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

XubString SvxUnderlineItem::GetValueTextByPos( USHORT nPos ) const
{
    DBG_ASSERT( nPos <= (USHORT)UNDERLINE_BOLDWAVE, "enum overflow!" );
    return SVX_RESSTR( RID_SVXITEMS_UL_BEGIN + nPos );
}

/*-----------------13.03.98 16:25-------------------

--------------------------------------------------*/
sal_Bool SvxUnderlineItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    switch(nMemberId)
    {
    case MID_UNDERLINED:
        rVal = Bool2Any(GetBoolValue());
        break;
    case MID_UNDERLINE:
        rVal <<= (sal_Int16)(GetValue());
        break;
    case MID_UL_COLOR:
        rVal <<= (sal_Int32)( mColor.GetColor() );
        break;
    case MID_UL_HASCOLOR:
        rVal = Bool2Any( !mColor.GetTransparency() );
        break;
    }
    return sal_True;

}
/*-----------------13.03.98 16:28-------------------

--------------------------------------------------*/
sal_Bool SvxUnderlineItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    sal_Bool bRet = sal_True;
    switch(nMemberId)
    {
    case MID_UNDERLINED:
        SetBoolValue(Any2Bool(rVal));
    break;
    case MID_UNDERLINE:
    {
        sal_Int32 nValue;
        if(!(rVal >>= nValue))
            bRet = sal_False;
        else
            SetValue((sal_Int16)nValue);
    }
    break;
    case MID_UL_COLOR:
    {
        sal_Int32 nCol;
        if( !( rVal >>= nCol ) )
            bRet = sal_False;
        else
        {
            // Keep transparence, because it contains the information
            // whether the font color or the stored color should be used
            sal_uInt8 nTrans = mColor.GetTransparency();
            mColor = Color( nCol );
            mColor.SetTransparency( nTrans );
        }
    }
    break;
    case MID_UL_HASCOLOR:
        mColor.SetTransparency( Any2Bool( rVal ) ? 0 : 0xff );
    break;
    }
    return bRet;
}

int SvxUnderlineItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
    return SfxEnumItem::operator==( rItem ) &&
           GetColor() == ((SvxUnderlineItem&)rItem).GetColor();
}
// -----------------------------------------------------------------------

#ifndef SVX_LIGHT

SvXMLEnumMapEntry pXML_underline_enums[] =
{
    { sXML_underline_none,              UNDERLINE_NONE },
    { sXML_underline_single,            UNDERLINE_SINGLE },
    { sXML_underline_double,            UNDERLINE_DOUBLE },
    { sXML_underline_dotted,            UNDERLINE_DOTTED },
    { sXML_underline_dash,              UNDERLINE_DASH },
    { sXML_underline_long_dash,         UNDERLINE_LONGDASH },
    { sXML_underline_dot_dash,          UNDERLINE_DASHDOT },
    { sXML_underline_dot_dot_dash,      UNDERLINE_DASHDOTDOT },
    { sXML_underline_wave,              UNDERLINE_WAVE },
    { sXML_underline_bold,              UNDERLINE_BOLD },
    { sXML_underline_bold_dotted,       UNDERLINE_BOLDDOTTED },
    { sXML_underline_bold_dash,         UNDERLINE_BOLDDASH },
    { sXML_underline_bold_long_dash,    UNDERLINE_BOLDLONGDASH },
    { sXML_underline_bold_dot_dash,     UNDERLINE_BOLDDASHDOT },
    { sXML_underline_bold_dot_dot_dash, UNDERLINE_BOLDDASHDOTDOT },
    { sXML_underline_bold_wave,         UNDERLINE_BOLDWAVE },
    { sXML_underline_double_wave,       UNDERLINE_DOUBLEWAVE },
    { sXML_underline_small_wave,        UNDERLINE_SMALLWAVE },
    { 0,                                0 }
};
#endif

sal_Bool SvxUnderlineItem::importXML( const OUString& rValue,
                                    sal_uInt16 nMemberId,
                                    const SvXMLUnitConverter& rUnitConverter )
{
    sal_Bool bRet =
#ifdef SVX_LIGHT
            sal_False;
#else
            sal_True;

    Color aTempColor;
    sal_uInt16 eUnderline;
    switch( nMemberId )
    {
    case MID_UNDERLINED:
    case MID_UNDERLINE:
        if( rUnitConverter.convertEnum( eUnderline, rValue,
                                        pXML_underline_enums ) )
            SetValue( eUnderline );
        else
            bRet = sal_False;
        break;

    case MID_UL_COLOR:
        if( 0 == rValue.compareToAscii( sXML_transparent ) )
            mColor.SetTransparency( 0xff );
        else if( rUnitConverter.convertColor( aTempColor, rValue ) )
        {
            mColor = aTempColor;
            mColor.SetTransparency( 0 );
        }
        else
            bRet = sal_False;
        break;
    }
#endif
    return bRet;
}

// -----------------------------------------------------------------------


sal_Bool SvxUnderlineItem::exportXML( OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet =
#ifdef SVX_LIGHT
            sal_False;
#else
            sal_True;

    OUStringBuffer aOut;
    switch( nMemberId )
    {
    case MID_UNDERLINED:
    case MID_UNDERLINE:
        bRet = rUnitConverter.convertEnum( aOut, GetValue(),
                                            pXML_underline_enums );
        break;

    case MID_UL_COLOR:
        if( mColor.GetTransparency() )
            aOut.appendAscii( sXML_transparent );
        else
            rUnitConverter.convertColor( aOut, mColor );
        break;
    }
    rValue = aOut.makeStringAndClear();
#endif
    return bRet;
}

// class SvxCrossedOutItem -----------------------------------------------

SvxCrossedOutItem::SvxCrossedOutItem( const FontStrikeout eSt, const USHORT nId )
    : SfxEnumItem( nId, eSt )
{
}

// -----------------------------------------------------------------------

int SvxCrossedOutItem::HasBoolValue() const
{
    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool SvxCrossedOutItem::GetBoolValue() const
{
    return (FontStrikeout)GetValue() != STRIKEOUT_NONE;
}

// -----------------------------------------------------------------------

void SvxCrossedOutItem::SetBoolValue( sal_Bool bVal )
{
    SetValue( bVal ? STRIKEOUT_SINGLE : STRIKEOUT_NONE );
}

// -----------------------------------------------------------------------

USHORT SvxCrossedOutItem::GetValueCount() const
{
    return STRIKEOUT_DOUBLE + 1;    // auch STRIKEOUT_NONE geh"ort dazu
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxCrossedOutItem::Clone( SfxItemPool * ) const
{
    return new SvxCrossedOutItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxCrossedOutItem::Store( SvStream& rStrm , USHORT nItemVersion ) const
{
    rStrm << (BYTE)GetValue();
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxCrossedOutItem::Create(SvStream& rStrm, USHORT) const
{
    BYTE eCross;
    rStrm >> eCross;
    return new SvxCrossedOutItem(  (FontStrikeout)eCross, Which() );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxCrossedOutItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = GetValueTextByPos( GetValue() );
            return ePres;
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

XubString SvxCrossedOutItem::GetValueTextByPos( USHORT nPos ) const
{
    DBG_ASSERT( nPos <= (USHORT)STRIKEOUT_X, "enum overflow!" );
    return SVX_RESSTR( RID_SVXITEMS_STRIKEOUT_BEGIN + nPos );
}

/*-----------------13.03.98 16:28-------------------

--------------------------------------------------*/
sal_Bool SvxCrossedOutItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    switch(nMemberId)
    {
        case MID_CROSSED_OUT:
            rVal = Bool2Any(GetBoolValue());
        break;
        case MID_CROSS_OUT:
            rVal <<= (sal_Int16)(GetValue());
        break;
    }
    return sal_True;
}
/*-----------------13.03.98 16:29-------------------

--------------------------------------------------*/
sal_Bool SvxCrossedOutItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    switch(nMemberId)
    {
        case MID_CROSSED_OUT:
            SetBoolValue(Any2Bool(rVal));
        break;
        case MID_CROSS_OUT:
        {
            sal_Int32 nValue;
            if(!(rVal >>= nValue))
                return sal_False;
            SetValue((sal_Int16)nValue);
        }
        break;
    }
    return sal_True;
}

#ifndef SVX_LIGHT
SvXMLEnumMapEntry pXML_crossedout_enums[] =
{
    { sXML_crossedout_none,      STRIKEOUT_NONE },
    { sXML_crossedout_single,    STRIKEOUT_SINGLE },
    { sXML_crossedout_double,    STRIKEOUT_DOUBLE },
    { sXML_crossedout_thick,     STRIKEOUT_BOLD },
    { sXML_crossedout_slash,     STRIKEOUT_SLASH },
    { sXML_crossedout_cross,     STRIKEOUT_X }
};
#endif

// -----------------------------------------------------------------------

sal_Bool SvxCrossedOutItem::importXML( const OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter )
{
#ifndef SVX_LIGHT
    sal_uInt16 eCross;
    if( rUnitConverter.convertEnum( eCross, rValue, pXML_crossedout_enums ) )
    {
        SetValue( eCross );
        return sal_True;
    }
#endif
    return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool SvxCrossedOutItem::exportXML( OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter ) const
{
#ifndef SVX_LIGHT

    OUStringBuffer aOut;
    if( rUnitConverter.convertEnum( aOut, GetValue(), pXML_crossedout_enums ) )
    {
        rValue = aOut.makeStringAndClear();
        return sal_True;
    }
#endif
    return sal_False;
}

// class SvxShadowedItem -------------------------------------------------

SvxShadowedItem::SvxShadowedItem( const sal_Bool bShadowed, const USHORT nId ) :
    SfxBoolItem( nId, bShadowed )
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxShadowedItem::Clone( SfxItemPool * ) const
{
    return new SvxShadowedItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxShadowedItem::Store( SvStream& rStrm , USHORT nItemVersion ) const
{
    rStrm << (BYTE) GetValue();
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxShadowedItem::Create(SvStream& rStrm, USHORT) const
{
    BYTE nState;
    rStrm >> nState;
    return new SvxShadowedItem( nState, Which() );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxShadowedItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            USHORT nId = RID_SVXITEMS_SHADOWED_FALSE;

            if ( GetValue() )
                nId = RID_SVXITEMS_SHADOWED_TRUE;
            rText = SVX_RESSTR(nId);
            return ePres;
        }
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

sal_Bool SvxShadowedItem::importXML( const OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter )
{
#ifndef SVX_LIGHT

    SetValue( !rValue.compareToAscii( sXML_none ) );
    return sal_True;
#else
    return sal_False;
#endif
}

sal_Bool SvxShadowedItem::exportXML( OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter ) const
{
#ifndef SVX_LIGHT
    if(GetValue())
    {
        rValue = OUString("1pt 1pt", 7 , gsl_getSystemTextEncoding());
    }
    else
    {
        rValue = OUString(sXML_none, sizeof(sXML_none), gsl_getSystemTextEncoding());
    }
    return sal_True;
#else
    return sal_False;
#endif
}


// class SvxAutoKernItem -------------------------------------------------

SvxAutoKernItem::SvxAutoKernItem( const sal_Bool bAutoKern, const USHORT nId ) :
    SfxBoolItem( nId, bAutoKern )
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxAutoKernItem::Clone( SfxItemPool * ) const
{
    return new SvxAutoKernItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxAutoKernItem::Store( SvStream& rStrm , USHORT nItemVersion ) const
{
    rStrm << (BYTE) GetValue();
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxAutoKernItem::Create(SvStream& rStrm, USHORT) const
{
    BYTE nState;
    rStrm >> nState;
    return new SvxAutoKernItem( nState, Which() );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxAutoKernItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            USHORT nId = RID_SVXITEMS_AUTOKERN_FALSE;

            if ( GetValue() )
                nId = RID_SVXITEMS_AUTOKERN_TRUE;
            rText = SVX_RESSTR(nId);
            return ePres;
        }
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

sal_Bool SvxAutoKernItem::importXML( const OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter )
{
#ifndef SVX_LIGHT
    sal_Bool bValue;
    if( rUnitConverter.convertBool( bValue, rValue ) )
    {
        SetValue( bValue );
        return sal_True;
    }
#endif
    return sal_False;
}

sal_Bool SvxAutoKernItem::exportXML( OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter ) const
{
#ifndef SVX_LIGHT

    OUStringBuffer aOut;

    rUnitConverter.convertBool( aOut, GetValue() );
    rValue = aOut.makeStringAndClear();
    return sal_True;
#else
    return sal_False;
#endif
}

// class SvxWordLineModeItem ---------------------------------------------

SvxWordLineModeItem::SvxWordLineModeItem( const sal_Bool bWordLineMode,
                                          const USHORT nId ) :
    SfxBoolItem( nId, bWordLineMode )
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxWordLineModeItem::Clone( SfxItemPool * ) const
{
    return new SvxWordLineModeItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxWordLineModeItem::Store( SvStream& rStrm , USHORT nItemVersion ) const
{
    rStrm << (sal_Bool) GetValue();
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxWordLineModeItem::Create(SvStream& rStrm, USHORT) const
{
    sal_Bool bValue;
    rStrm >> bValue;
    return new SvxWordLineModeItem( bValue, Which() );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxWordLineModeItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            USHORT nId = RID_SVXITEMS_WORDLINE_FALSE;

            if ( GetValue() )
                nId = RID_SVXITEMS_WORDLINE_TRUE;
            rText = SVX_RESSTR(nId);
            return ePres;
        }
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

sal_Bool SvxWordLineModeItem::importXML( const OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter )
{
#ifndef SVX_LIGHT
    sal_Bool bValue;
    if( rUnitConverter.convertBool( bValue, rValue ) )
    {
        SetValue( !bValue );
        return sal_True;
    }
#endif
    return sal_False;
}

sal_Bool SvxWordLineModeItem::exportXML( OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter ) const
{
#ifndef SVX_LIGHT
    OUStringBuffer aOut;
    rUnitConverter.convertBool( aOut, !GetValue() );
    rValue = aOut.makeStringAndClear();
    return sal_True;
#else
    return sal_False;
#endif
}

// class SvxContourItem --------------------------------------------------

SvxContourItem::SvxContourItem( const sal_Bool bContoured, const USHORT nId ) :
    SfxBoolItem( nId, bContoured )
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxContourItem::Clone( SfxItemPool * ) const
{
    return new SvxContourItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxContourItem::Store( SvStream& rStrm , USHORT nItemVersion ) const
{
    rStrm << (sal_Bool) GetValue();
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxContourItem::Create(SvStream& rStrm, USHORT) const
{
    sal_Bool bValue;
    rStrm >> bValue;
    return new SvxContourItem( bValue, Which() );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxContourItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            USHORT nId = RID_SVXITEMS_CONTOUR_FALSE;

            if ( GetValue() )
                nId = RID_SVXITEMS_CONTOUR_TRUE;
            rText = SVX_RESSTR(nId);
            return ePres;
        }
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

sal_Bool SvxContourItem::importXML( const OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter )
{
#ifndef SVX_LIGHT
    sal_Bool bValue;
    if( rUnitConverter.convertBool( bValue, rValue ) )
    {
        SetValue( bValue );
        return sal_True;
    }
#endif
    return sal_False;
}

sal_Bool SvxContourItem::exportXML( OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter ) const
{
#ifndef SVX_LIGHT
    OUStringBuffer aOut;
    rUnitConverter.convertBool( aOut, GetValue() );
    rValue = aOut.makeStringAndClear();
    return sal_True;
#else
    return sal_False;
#endif
}


// class SvxPropSizeItem -------------------------------------------------

SvxPropSizeItem::SvxPropSizeItem( const USHORT nPercent, const USHORT nId ) :
    SfxUInt16Item( nId, nPercent )
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxPropSizeItem::Clone( SfxItemPool * ) const
{
    return new SvxPropSizeItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxPropSizeItem::Store( SvStream& rStrm , USHORT nItemVersion ) const
{
    rStrm << (USHORT) GetValue();
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxPropSizeItem::Create(SvStream& rStrm, USHORT) const
{
    USHORT nSize;
    rStrm >> nSize;
    return new SvxPropSizeItem( nSize, Which() );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxPropSizeItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
    rText.Erase();
    return SFX_ITEM_PRESENTATION_NONE;
}

// class SvxColorItem ----------------------------------------------------

SvxColorItem::SvxColorItem( const USHORT nId ) :
    SfxPoolItem( nId ),
    mColor( COL_BLACK )
{
}

// -----------------------------------------------------------------------

SvxColorItem::SvxColorItem( const Color& rCol, const USHORT nId ) :
    SfxPoolItem( nId ),
    mColor( rCol )
{
}

// -----------------------------------------------------------------------

SvxColorItem::SvxColorItem( SvStream &rStrm, const USHORT nId ) :
    SfxPoolItem( nId )
{
    Color aColor;
    rStrm >> aColor;
    mColor = aColor;
}

// -----------------------------------------------------------------------

SvxColorItem::SvxColorItem( const SvxColorItem &rCopy ) :
    SfxPoolItem( rCopy ),
    mColor( rCopy.mColor )
{
}

// -----------------------------------------------------------------------

SvxColorItem::~SvxColorItem()
{
}

// -----------------------------------------------------------------------

int SvxColorItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    return  mColor == ( (const SvxColorItem&)rAttr ).mColor;
}

// -----------------------------------------------------------------------

sal_Bool SvxColorItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    rVal <<= (sal_Int32)(mColor.GetColor());
    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool SvxColorItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    sal_Int32 nColor;
    if(!(rVal >>= nColor))
        return sal_False;

    mColor.SetColor( nColor );
    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool SvxColorItem::importXML( const OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter )
{
#ifndef SVX_LIGHT
    return rUnitConverter.convertColor( mColor, rValue );
#else
    return sal_False;
#endif
}

// -----------------------------------------------------------------------

sal_Bool SvxColorItem::exportXML( OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter ) const
{
#ifndef SVX_LIGHT

    OUStringBuffer aOut;
    rUnitConverter.convertColor( aOut, mColor );
    rValue = aOut.makeStringAndClear();
    return sal_True;
#else
    return sal_False;
#endif
}


// -----------------------------------------------------------------------

SfxPoolItem* SvxColorItem::Clone( SfxItemPool * ) const
{
    return new SvxColorItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxColorItem::Store( SvStream& rStrm , USHORT nItemVersion ) const
{
    rStrm << mColor;
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxColorItem::Create(SvStream& rStrm, USHORT nVer ) const
{
    return new SvxColorItem( rStrm, Which() );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxColorItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
#ifndef SVX_LIGHT
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = ::GetColorString( mColor );
            return ePres;
    }
#endif
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

void SvxColorItem::SetValue( const Color& rNewCol )
{
    mColor = rNewCol;
}

// class SvxCharSetColorItem ---------------------------------------------

SvxCharSetColorItem::SvxCharSetColorItem( const USHORT nId ) :
    SvxColorItem( nId ),

    eFrom( RTL_TEXTENCODING_DONTKNOW )
{
}

// -----------------------------------------------------------------------

SvxCharSetColorItem::SvxCharSetColorItem( const Color& rCol,
                                          const rtl_TextEncoding eFrom,
                                          const USHORT nId ) :
    SvxColorItem( rCol, nId ),

    eFrom( eFrom )
{
}


// -----------------------------------------------------------------------

SfxPoolItem* SvxCharSetColorItem::Clone( SfxItemPool * ) const
{
    return new SvxCharSetColorItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxCharSetColorItem::Store( SvStream& rStrm , USHORT nItemVersion ) const
{
    rStrm << (BYTE) GetStoreCharSet( GetCharSet(), rStrm.GetVersion() )
          << GetValue();
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxCharSetColorItem::Create(SvStream& rStrm, USHORT) const
{
    BYTE cSet;
    Color aColor;
    rStrm >> cSet >> aColor;
    return new SvxCharSetColorItem( aColor,  (rtl_TextEncoding)cSet, Which() );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxCharSetColorItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
    rText.Erase();
    return SFX_ITEM_PRESENTATION_NONE;
}

// class SvxKerningItem --------------------------------------------------

SvxKerningItem::SvxKerningItem( const short nKern, const USHORT nId ) :
    SfxInt16Item( nId, nKern )
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxKerningItem::Clone( SfxItemPool * ) const
{
    return new SvxKerningItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxKerningItem::Store( SvStream& rStrm , USHORT nItemVersion ) const
{
    rStrm << (short) GetValue();
    return rStrm;
}

// -----------------------------------------------------------------------

int SvxKerningItem::ScaleMetrics( long nMult, long nDiv )
{
    SetValue( (sal_Int16)Scale( GetValue(), nMult, nDiv ) );
    return 1;
}

// -----------------------------------------------------------------------

int SvxKerningItem::HasMetrics() const
{
    return 1;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxKerningItem::Create(SvStream& rStrm, USHORT) const
{
    short nValue;
    rStrm >> nValue;
    return new SvxKerningItem( nValue, Which() );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxKerningItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
#ifndef SVX_LIGHT
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
            rText = GetMetricText( (long)GetValue(), eCoreUnit, ePresUnit );
            rText += SVX_RESSTR(GetMetricId(ePresUnit));
            return ePres;
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            rText = SVX_RESSTR(RID_SVXITEMS_KERNING_COMPLETE);
            USHORT nId = 0;

            if ( GetValue() > 0 )
                nId = RID_SVXITEMS_KERNING_EXPANDED;
            else if ( GetValue() < 0 )
                nId = RID_SVXITEMS_KERNING_CONDENSED;

            if ( nId )
                rText += SVX_RESSTR(nId);
            rText += GetMetricText( (long)GetValue(), eCoreUnit, ePresUnit );
            rText += SVX_RESSTR(GetMetricId(ePresUnit));
            return ePres;
        }
    }
#endif
    return SFX_ITEM_PRESENTATION_NONE;
}

sal_Bool SvxKerningItem::importXML( const OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter )
{
#ifndef SVX_LIGHT

    if( rValue.compareToAscii( sXML_kerning_normal )  )
    {
        SetValue( 0 );
        return sal_True;
    }
    else
    {
        sal_Int32 eKerning;
        if( rUnitConverter.convertMeasure( eKerning, rValue ) )
        {
            SetValue(eKerning);
            return sal_True;
        }
    }
#endif
    return sal_False;
}

sal_Bool SvxKerningItem::exportXML( OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter ) const
{
#ifndef SVX_LIGHT

    OUStringBuffer aOut;

    if( GetValue() == 0 )
        aOut.appendAscii( sXML_kerning_normal );
    else
        rUnitConverter.convertMeasure( aOut, GetValue() );

    rValue = aOut.makeStringAndClear();
    return sal_True;
#else
    return sal_False;
#endif
}
/* -----------------------------19.02.01 12:21--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool SvxKerningItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    sal_Int16 nVal = GetValue();
    if(nMemberId & CONVERT_TWIPS)
        nVal = TWIP_TO_MM100(nVal);
    rVal <<= nVal;
    return sal_True;
}
// -----------------------------------------------------------------------
sal_Bool SvxKerningItem::PutValue( const uno::Any& rVal, BYTE nMemberId)
{
    sal_Int16 nVal;
    if(!(rVal >>= nVal))
        return sal_False;
    if(nMemberId & CONVERT_TWIPS)
        nVal = MM100_TO_TWIP(nVal);
    SetValue(nVal);
    return sal_True;
}

// class SvxCaseMapItem --------------------------------------------------

SvxCaseMapItem::SvxCaseMapItem( const SvxCaseMap eMap, const USHORT nId ) :
    SfxEnumItem( nId, eMap )
{
}

// -----------------------------------------------------------------------

USHORT SvxCaseMapItem::GetValueCount() const
{
    return SVX_CASEMAP_END; // SVX_CASEMAP_KAPITAELCHEN + 1
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxCaseMapItem::Clone( SfxItemPool * ) const
{
    return new SvxCaseMapItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxCaseMapItem::Store( SvStream& rStrm , USHORT nItemVersion ) const
{
    rStrm << (BYTE) GetValue();
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxCaseMapItem::Create(SvStream& rStrm, USHORT) const
{
    BYTE cMap;
    rStrm >> cMap;
    return new SvxCaseMapItem( (const SvxCaseMap)cMap, Which() );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxCaseMapItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = GetValueTextByPos( GetValue() );
            return ePres;
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

XubString SvxCaseMapItem::GetValueTextByPos( USHORT nPos ) const
{
    DBG_ASSERT( nPos < (USHORT)SVX_CASEMAP_END, "enum overflow!" );
    return SVX_RESSTR( RID_SVXITEMS_CASEMAP_BEGIN + nPos );
}

/*-----------------13.03.98 16:29-------------------

--------------------------------------------------*/
sal_Bool SvxCaseMapItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    sal_Int16 nRet = style::CaseMap::NONE;
    switch( GetValue() )
    {
//      case SVX_CASEMAP_NOT_MAPPED  :      nRet = style::CaseMap::NONE     ; break;
        case SVX_CASEMAP_VERSALIEN   :      nRet = style::CaseMap::UPPERCASE; break;
        case SVX_CASEMAP_GEMEINE     :      nRet = style::CaseMap::LOWERCASE; break;
        case SVX_CASEMAP_TITEL       :      nRet = style::CaseMap::TITLE    ; break;
        case SVX_CASEMAP_KAPITAELCHEN:      nRet = style::CaseMap::SMALLCAPS; break;
    }
    rVal <<= (sal_Int16)(nRet);
    return sal_True;
}
/*-----------------13.03.98 16:29-------------------

--------------------------------------------------*/
sal_Bool SvxCaseMapItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    sal_uInt16 nVal;
    if(!(rVal >>= nVal))
        return sal_False;

    switch( nVal )
    {
    case style::CaseMap::NONE    :  nVal = SVX_CASEMAP_NOT_MAPPED  ; break;
    case style::CaseMap::UPPERCASE:  nVal = SVX_CASEMAP_VERSALIEN   ; break;
    case style::CaseMap::LOWERCASE:  nVal = SVX_CASEMAP_GEMEINE     ; break;
    case style::CaseMap::TITLE    :  nVal = SVX_CASEMAP_TITEL       ; break;
    case style::CaseMap::SMALLCAPS:  nVal = SVX_CASEMAP_KAPITAELCHEN; break;
    }
    SetValue(nVal);
    return sal_True;
}

// -----------------------------------------------------------------------

#ifndef SVX_LIGHT
static SvXMLEnumMapEntry pXML_casemap_enums[] =
{
    { sXML_none,                SVX_CASEMAP_NOT_MAPPED },
    { sXML_casemap_lowercase,   SVX_CASEMAP_GEMEINE },
    { sXML_casemap_uppercase,   SVX_CASEMAP_VERSALIEN },
    { sXML_casemap_capitalize,  SVX_CASEMAP_TITEL },
    { 0,0 }
};
#endif

sal_Bool SvxCaseMapItem::importXML( const OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter )
{
#ifndef SVX_LIGHT
    sal_uInt16 nVal = SVX_CASEMAP_NOT_MAPPED;
    switch( nMemberId )
    {
    case MID_CASEMAP_VARIANT:
        if( rValue.compareToAscii( sXML_casemap_small_caps ) )
            nVal = SVX_CASEMAP_KAPITAELCHEN;
        else
            if( !rValue.compareToAscii(sXML_casemap_normal ) )
                return sal_False;
        break;
    case MID_CASEMAP_TRANS:
        if(!rUnitConverter.convertEnum( nVal, rValue, pXML_casemap_enums ))
            return sal_False;
        break;
    }

    SetValue( (sal_uInt16)nVal );
    return sal_True;
#else
    return sal_False;
#endif
}

// -----------------------------------------------------------------------

sal_Bool SvxCaseMapItem::exportXML( OUString& rValue, sal_uInt16 nMemberId,
                                const SvXMLUnitConverter& rUnitConverter ) const
{
#ifndef SVX_LIGHT

    sal_uInt16 nVal = GetValue();

    OUStringBuffer aOut;

    switch( nMemberId )
    {
    case MID_CASEMAP_VARIANT:
        if( nVal == SVX_CASEMAP_KAPITAELCHEN || nVal == SVX_CASEMAP_NOT_MAPPED )
            aOut.appendAscii( nVal == SVX_CASEMAP_KAPITAELCHEN ?
                              sXML_casemap_small_caps :
                              sXML_casemap_normal );
        else
            return sal_False;
        break;
    case MID_CASEMAP_TRANS:
        if(!rUnitConverter.convertEnum( aOut, nVal, pXML_casemap_enums ))
            return sal_False;
        break;
    }

    rValue = aOut.makeStringAndClear();
    return sal_True;
#else
    return sal_False;
#endif
}


// class SvxEscapementItem -----------------------------------------------

SvxEscapementItem::SvxEscapementItem( const USHORT nId ) :
    SfxEnumItemInterface( nId ),

    nEsc    ( 0 ),
    nProp   ( 100 )
{
}

// -----------------------------------------------------------------------

SvxEscapementItem::SvxEscapementItem( const SvxEscapement eEscape,
                                      const USHORT nId ) :
    SfxEnumItemInterface( nId ),
    nProp( 100 )
{
    SetEscapement( eEscape );
    if( nEsc )
        nProp = 58;
}

// -----------------------------------------------------------------------

SvxEscapementItem::SvxEscapementItem( const short nEsc,
                                      const BYTE nProp,
                                      const USHORT nId ) :
    SfxEnumItemInterface( nId ),
    nEsc    ( nEsc ),
    nProp   ( nProp )
{
}

// -----------------------------------------------------------------------

int SvxEscapementItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    return( nEsc  == ((SvxEscapementItem&)rAttr).nEsc &&
            nProp == ((SvxEscapementItem&)rAttr).nProp );
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxEscapementItem::Clone( SfxItemPool * ) const
{
    return new SvxEscapementItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxEscapementItem::Store( SvStream& rStrm , USHORT nItemVersion ) const
{
    short nEsc = GetEsc();
    if( SOFFICE_FILEFORMAT_31 == rStrm.GetVersion() )
    {
        if( DFLT_ESC_AUTO_SUPER == nEsc )
            nEsc = DFLT_ESC_SUPER;
        else if( DFLT_ESC_AUTO_SUB == nEsc )
            nEsc = DFLT_ESC_SUB;
    }
    rStrm << (BYTE) GetProp()
          << (short) nEsc;
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxEscapementItem::Create(SvStream& rStrm, USHORT) const
{
    BYTE nProp;
    short nEsc;
    rStrm >> nProp >> nEsc;
    return new SvxEscapementItem( nEsc, nProp, Which() );
}

// -----------------------------------------------------------------------

USHORT SvxEscapementItem::GetValueCount() const
{
    return SVX_ESCAPEMENT_END;  // SVX_ESCAPEMENT_SUBSCRIPT + 1
}

//------------------------------------------------------------------------

SfxItemPresentation SvxEscapementItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            rText = GetValueTextByPos( GetEnumValue() );

            if ( nEsc != 0 )
            {
                if( DFLT_ESC_AUTO_SUPER == nEsc || DFLT_ESC_AUTO_SUB == nEsc )
                    rText += String( SVX_RESSTR(RID_SVXITEMS_ESCAPEMENT_AUTO) );
                else
                    ( rText += String::CreateFromInt32( nEsc )) += sal_Unicode('%');
            }
            return ePres;
        }
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

XubString SvxEscapementItem::GetValueTextByPos( USHORT nPos ) const
{
    DBG_ASSERT( nPos < (USHORT)SVX_ESCAPEMENT_END, "enum overflow!" );
    return SVX_RESSTR(RID_SVXITEMS_ESCAPEMENT_BEGIN + nPos);
}

// -----------------------------------------------------------------------

USHORT SvxEscapementItem::GetEnumValue() const
{
    if ( nEsc < 0 )
        return SVX_ESCAPEMENT_SUBSCRIPT;
    else if ( nEsc > 0 )
        return SVX_ESCAPEMENT_SUPERSCRIPT;
    return SVX_ESCAPEMENT_OFF;
}

// -----------------------------------------------------------------------

void SvxEscapementItem::SetEnumValue( USHORT nVal )
{
    SetEscapement( (const SvxEscapement)nVal );
}

/*-----------------13.03.98 17:05-------------------

--------------------------------------------------*/
sal_Bool SvxEscapementItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    switch(nMemberId)
    {
        case MID_ESC:
            rVal <<= (sal_Int16)(nEsc);
        break;
        case MID_ESC_HEIGHT:
            rVal <<= (sal_Int8)(nProp);
        break;
        case MID_AUTO_ESC:
            rVal = Bool2Any(DFLT_ESC_AUTO_SUB == nEsc || DFLT_ESC_AUTO_SUPER == nEsc);
        break;
    }
    return sal_True;
}
/*-----------------13.03.98 17:05-------------------

--------------------------------------------------*/
sal_Bool SvxEscapementItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    switch(nMemberId)
    {
        case MID_ESC:
        {
            sal_Int16 nVal;
            if( (rVal >>= nVal) && (Abs(nVal) <= 101))
                nEsc = nVal;
            else
                return sal_False;
        }
        break;
        case MID_ESC_HEIGHT:
        {
            sal_Int8 nVal;
            if( (rVal >>= nVal) && (nVal <= 100))
                nProp = nVal;
            else
                return sal_False;
        }
        break;
        case MID_AUTO_ESC:
        {
            BOOL bVal = Any2Bool(rVal);
            if(bVal)
            {
                if(nEsc < 0)
                    nEsc = DFLT_ESC_AUTO_SUB;
                else
                    nEsc = DFLT_ESC_AUTO_SUPER;
            }
            else
                if(DFLT_ESC_AUTO_SUPER == nEsc )
                    --nEsc;
                else if(DFLT_ESC_AUTO_SUB == nEsc)
                    ++nEsc;
        }
        break;
    }
    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool SvxEscapementItem::importXML( const OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter )
{
#ifndef SVX_LIGHT

    SvXMLTokenEnumerator aTokens( rValue );

    OUString aToken;
    if( ! aTokens.getNextToken( aToken ) )
        return sal_False;

    if( aToken.compareToAscii( sXML_escapement_sub ) )
    {
        nEsc = DFLT_ESC_AUTO_SUB;
    }
    else if( aToken.compareToAscii( sXML_escapement_super ) )
    {
        nEsc = DFLT_ESC_AUTO_SUPER;
    }
    else
    {
        sal_Int32 nNewEsc;
        if( !rUnitConverter.convertPercent( nNewEsc, aToken ) )
            return sal_False;

        nEsc = (sal_uInt16) nNewEsc;
    }

    if( aTokens.getNextToken( aToken ) )
    {
        sal_Int32 nNewProp;
        if( !rUnitConverter.convertPercent( nNewProp, aToken ) )
            return sal_False;
        nProp = (sal_uInt16)nNewProp;
    }
    else
    {
        nProp = (sal_uInt16) DFLT_ESC_PROP;
    }

    return sal_True;
#else
    return sal_False;
#endif
}

// -----------------------------------------------------------------------

sal_Bool SvxEscapementItem::exportXML( OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter ) const
{
#ifndef SVX_LIGHT
    OUStringBuffer aOut;
    if( nEsc == DFLT_ESC_AUTO_SUPER )
    {
        aOut.appendAscii( sXML_escapement_super );
    }
    else if( nEsc == DFLT_ESC_AUTO_SUB )
    {
        aOut.appendAscii( sXML_escapement_sub );
    }
    else
    {
        rUnitConverter.convertPercent( aOut, nEsc );
    }

    aOut.append( sal_Unicode(' '));
    rUnitConverter.convertPercent( aOut, nProp );

    rValue = aOut.makeStringAndClear();
    return sal_True;
#else
    return sal_False;
#endif
}

// class SvxLanguageItem -------------------------------------------------

SvxLanguageItem::SvxLanguageItem( const LanguageType eLang, const USHORT nId )
    : SfxEnumItem( nId , eLang )
{
}

// -----------------------------------------------------------------------

USHORT SvxLanguageItem::GetValueCount() const
{
    return LANGUAGE_COUNT;  // aus tlintl.hxx
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxLanguageItem::Clone( SfxItemPool * ) const
{
    return new SvxLanguageItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxLanguageItem::Store( SvStream& rStrm , USHORT nItemVersion ) const
{
    rStrm << (USHORT) GetValue();
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxLanguageItem::Create(SvStream& rStrm, USHORT) const
{
    USHORT nValue;
    rStrm >> nValue;
    return new SvxLanguageItem( (LanguageType)nValue, Which() );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxLanguageItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
#ifndef SVX_LIGHT
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            SvxLanguageTable aLangTable;
            rText = aLangTable.GetString( (LanguageType)GetValue() );
            return ePres;
        }
    }
#endif // !SVX_LIGHT
    return SFX_ITEM_PRESENTATION_NONE;
}

/*-----------------14.03.98 14:13-------------------

--------------------------------------------------*/
sal_Bool SvxLanguageItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    switch(nMemberId)
    {
        case MID_LANG_INT:
            rVal <<= (sal_Int16)(GetValue());
        break;
        case MID_LANG_LOCALE:
            String sLanguage, sCountry;
            ::ConvertLanguageToIsoNames( GetValue(), sLanguage, sCountry );
            lang::Locale aRet;
            aRet.Language = sLanguage;
            aRet.Country = sCountry;
            rVal <<= aRet;
        break;
    }
    return sal_True;
}
/*-----------------14.03.98 14:13-------------------

--------------------------------------------------*/
sal_Bool SvxLanguageItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    switch(nMemberId)
    {
        case MID_LANG_INT:
        {
            sal_Int32 nValue;
            if(!(rVal >>= nValue))
                return sal_False;

            SetValue((sal_Int16)nValue);
        }
        break;
        case MID_LANG_LOCALE:
        {
            lang::Locale aLocale;
            if(!(rVal >>= aLocale))
                return sal_False;

            if (aLocale.Language.getLength() || aLocale.Country.getLength())
                SetValue(ConvertIsoNamesToLanguage( aLocale.Language, aLocale.Country ));
            else
                SetValue(LANGUAGE_NONE);
        }
        break;
    }
    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool SvxLanguageItem::importXML( const OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter )
{
#ifndef SVX_LIGHT
    sal_Bool bRet = sal_False;
    switch( nMemberId )
    {
    case MID_LANGUAGE:
    case MID_COUNTRY:
        {
            String sLanguage, sCountry;
            if( LANGUAGE_DONTKNOW != GetLanguage() &&
                LANGUAGE_NONE != GetLanguage() &&
                LANGUAGE_SYSTEM != GetLanguage() )
            {
                ConvertLanguageToIsoNames( GetLanguage(), sLanguage, sCountry );
                DBG_ASSERT( !sCountry.Len(),
        "SvxLanguageItem::importXML: setting contry first is unimplemented" );
            }

            String sValue;
            if( !rValue.compareToAscii( sXML_none ) )
                sValue = rValue.getStr();

            if( MID_LANGUAGE == nMemberId )
                sLanguage = sValue;
            else
                sCountry = sValue;

            if( 0 == sLanguage.Len() && 0 == sCountry.Len() )
            {
                SetLanguage( LANGUAGE_NONE );
                bRet = sal_True;
            }
            else
            {
                LanguageType eLang = ConvertIsoNamesToLanguage( sLanguage,
                                                                sCountry );
                if( LANGUAGE_NONE != eLang )
                {
                    SetLanguage( eLang );
                    bRet = sal_True;
                }
            }
        }
        break;
    case MID_LANG_COUNTRY:
        if( rValue.getLength() )
        {
            LanguageType eLang = ConvertIsoStringToLanguage( rValue.getStr() );
            if( LANGUAGE_NONE != eLang )
            {
                SetLanguage( eLang );
                bRet = sal_True;
            }
        }
        else
        {
            SetLanguage( LANGUAGE_NONE );
            bRet = sal_True;
        }
        break;
    }

    return bRet;
#else
    return sal_False;
#endif
}

// -----------------------------------------------------------------------


sal_Bool SvxLanguageItem::exportXML( OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter ) const
{
#ifndef SVX_LIGHT
    sal_Bool bRet = sal_False;
    switch( nMemberId )
    {
    case MID_LANGUAGE:
    case MID_COUNTRY:
        if( LANGUAGE_NONE == GetLanguage() ||
            LANGUAGE_DONTKNOW == GetLanguage() ||
            LANGUAGE_SYSTEM == GetLanguage() )
        {
            rValue = OUString( sXML_none, sizeof(sXML_none), gsl_getSystemTextEncoding() );
            bRet = sal_True;
        }
        else
        {
            String sLang, sCountry;
            ConvertLanguageToIsoNames( GetLanguage(), sLang, sCountry );
            if( MID_LANGUAGE == nMemberId )
                rValue = sLang.GetBuffer();
            else
                rValue = sCountry.GetBuffer();
            bRet = 0 != rValue.getLength();
        }
        break;

    case MID_LANG_COUNTRY:
        if( LANGUAGE_NONE == GetLanguage() ||
            LANGUAGE_DONTKNOW == GetLanguage() ||
            LANGUAGE_SYSTEM == GetLanguage() )
        {
            rValue = OUString();
            bRet = sal_True;
        }
        else
        {
            rValue = ConvertLanguageToIsoString( GetLanguage() ).GetBuffer();
            bRet = 0 != rValue.getLength();
        }
        break;
    }

    return bRet;
#else
    return sal_False;
#endif
}

// class SvxNoLinebreakItem ----------------------------------------------

SvxNoLinebreakItem::SvxNoLinebreakItem( const sal_Bool bBreak, const USHORT nId ) :
    SfxBoolItem( nId, bBreak )
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxNoLinebreakItem::Clone( SfxItemPool* ) const
{
    return new SvxNoLinebreakItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxNoLinebreakItem::Store( SvStream& rStrm , USHORT nItemVersion ) const
{
    rStrm << (sal_Bool)GetValue();
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxNoLinebreakItem::Create(SvStream& rStrm, USHORT) const
{
    sal_Bool bValue;
    rStrm >> bValue;
    return new SvxNoLinebreakItem( bValue, Which() );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxNoLinebreakItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
    rText.Erase();
    return SFX_ITEM_PRESENTATION_NONE;
}

// class SvxNoHyphenItem -------------------------------------------------

SvxNoHyphenItem::SvxNoHyphenItem( const sal_Bool bHyphen, const USHORT nId ) :
    SfxBoolItem( nId , bHyphen )
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxNoHyphenItem::Clone( SfxItemPool* ) const
{
    return new SvxNoHyphenItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxNoHyphenItem::Store( SvStream& rStrm , USHORT nItemVersion ) const
{
    rStrm << (sal_Bool) GetValue();
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxNoHyphenItem::Create( SvStream& rStrm, USHORT ) const
{
    sal_Bool bValue;
    rStrm >> bValue;
    return new SvxNoHyphenItem( bValue, Which() );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxNoHyphenItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
    rText.Erase();
    return SFX_ITEM_PRESENTATION_NONE;
}

/*
 * Dummy-Item fuer ToolBox-Controls:
 *
 */

// -----------------------------------------------------------------------
// class SvxLineColorItem (== SvxColorItem)
// -----------------------------------------------------------------------

SvxLineColorItem::SvxLineColorItem( const USHORT nId ) :
    SvxColorItem( nId )
{
}

// -----------------------------------------------------------------------

SvxLineColorItem::SvxLineColorItem( const Color& rCol, const USHORT nId ) :
    SvxColorItem( rCol, nId )
{
}

// -----------------------------------------------------------------------

SvxLineColorItem::SvxLineColorItem( SvStream &rStrm, const USHORT nId ) :
    SvxColorItem( rStrm, nId )
{
}

// -----------------------------------------------------------------------

SvxLineColorItem::SvxLineColorItem( const SvxLineColorItem &rCopy ) :
    SvxColorItem( rCopy )
{
}

// -----------------------------------------------------------------------

SvxLineColorItem::~SvxLineColorItem()
{
}

//------------------------------------------------------------------------

SfxItemPresentation SvxLineColorItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText,
    const International * pInternational
)   const
{
    return SvxColorItem::GetPresentation( ePres, eCoreUnit, ePresUnit,
                                          rText, pInternational );
}

// class SvxBlinkItem -------------------------------------------------


SvxBlinkItem::SvxBlinkItem( const sal_Bool bBlink, const USHORT nId ) :
    SfxBoolItem( nId, bBlink )
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxBlinkItem::Clone( SfxItemPool * ) const
{
    return new SvxBlinkItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxBlinkItem::Store( SvStream& rStrm , USHORT nItemVersion ) const
{
    rStrm << (BYTE) GetValue();
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxBlinkItem::Create(SvStream& rStrm, USHORT) const
{
    BYTE nState;
    rStrm >> nState;
    return new SvxBlinkItem( nState, Which() );
}

// -----------------------------------------------------------------------

SfxItemPresentation SvxBlinkItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            USHORT nId = RID_SVXITEMS_BLINK_FALSE;

            if ( GetValue() )
                nId = RID_SVXITEMS_BLINK_TRUE;
            rText = SVX_RESSTR(nId);
            return ePres;
        }
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

sal_Bool SvxBlinkItem::importXML( const OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter )
{
#ifndef SVX_LIGHT
    sal_Bool bValue;
    if( rUnitConverter.convertBool( bValue, rValue ) )
    {
        SetValue( bValue );
        return sal_True;
    }
#endif
    return sal_False;
}

sal_Bool SvxBlinkItem::exportXML( OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter ) const
{
#ifndef SVX_LIGHT
    OUStringBuffer aOut;

    rUnitConverter.convertBool( aOut, GetValue() );
    rValue = aOut.makeStringAndClear();
    return sal_True;
#else
    return sal_False;
#endif
}

// class SvxEmphaisMarkItem ---------------------------------------------------

SvxEmphasisMarkItem::SvxEmphasisMarkItem( const FontEmphasisMark nValue,
                                        const USHORT nId )
    : SfxUInt16Item( nId, nValue )
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxEmphasisMarkItem::Clone( SfxItemPool * ) const
{
    return new SvxEmphasisMarkItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxEmphasisMarkItem::Store( SvStream& rStrm,
                                     USHORT nItemVersion ) const
{
    rStrm << (sal_uInt16)GetValue();
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxEmphasisMarkItem::Create( SvStream& rStrm, USHORT ) const
{
    sal_uInt16 nValue;
    rStrm >> nValue;
    return new SvxEmphasisMarkItem( (FontEmphasisMark)nValue, Which() );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxEmphasisMarkItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText,
    const International *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            {
                sal_uInt16 nVal = GetValue();
                rText = SVX_RESSTR( RID_SVXITEMS_EMPHASIS_BEGIN_STYLE +
                                        ( EMPHASISMARK_STYLE & nVal ));
                USHORT nId = ( EMPHASISMARK_POS_ABOVE & nVal )
                                ? RID_SVXITEMS_EMPHASIS_ABOVE_POS
                                : ( EMPHASISMARK_POS_BELOW & nVal )
                                    ? RID_SVXITEMS_EMPHASIS_BELOW_POS
                                    : 0;
                if( nId )
                    rText += SVX_RESSTR( nId );
                return ePres;
            }
            break;
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

sal_Bool SvxEmphasisMarkItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    switch( nMemberId )
    {
    case MID_EMPHASIS:
    {
        sal_Int16 nValue = GetValue();
        sal_Int16 nRet = 0;
        switch(nValue & EMPHASISMARK_STYLE)
        {
            case EMPHASISMARK_NONE   : nRet = FontEmphasis::NONE;           break;
            case EMPHASISMARK_DOT    : nRet = FontEmphasis::DOT_ABOVE;      break;
            case EMPHASISMARK_CIRCLE : nRet = FontEmphasis::CIRCLE_ABOVE;   break;
            case EMPHASISMARK_DISC   : nRet = FontEmphasis::DISK_ABOVE;     break;
            case EMPHASISMARK_ACCENT : nRet = FontEmphasis::ACCENT_ABOVE;   break;
        }
        if(nRet && nValue & EMPHASISMARK_POS_BELOW)
            nRet += 10;
        rVal <<= nRet;
    }
    break;
    }
    return sal_True;
}

sal_Bool SvxEmphasisMarkItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    sal_Bool bRet = sal_True;
    switch( nMemberId )
    {
    case MID_EMPHASIS:
    {
        sal_Int32 nValue = -1;
        rVal >>= nValue;
        switch(nValue)
        {
            case FontEmphasis::NONE        : nValue = EMPHASISMARK_NONE;   break;
            case FontEmphasis::DOT_ABOVE   : nValue = EMPHASISMARK_DOT|EMPHASISMARK_POS_ABOVE;    break;
            case FontEmphasis::CIRCLE_ABOVE: nValue = EMPHASISMARK_CIRCLE|EMPHASISMARK_POS_ABOVE; break;
            case FontEmphasis::DISK_ABOVE  : nValue = EMPHASISMARK_DISC|EMPHASISMARK_POS_ABOVE;   break;
            case FontEmphasis::ACCENT_ABOVE: nValue = EMPHASISMARK_ACCENT|EMPHASISMARK_POS_ABOVE; break;
            case FontEmphasis::DOT_BELOW   : nValue = EMPHASISMARK_DOT|EMPHASISMARK_POS_BELOW;    break;
            case FontEmphasis::CIRCLE_BELOW: nValue = EMPHASISMARK_CIRCLE|EMPHASISMARK_POS_BELOW; break;
            case FontEmphasis::DISK_BELOW  : nValue = EMPHASISMARK_DISC|EMPHASISMARK_POS_BELOW;   break;
            case FontEmphasis::ACCENT_BELOW: nValue = EMPHASISMARK_ACCENT|EMPHASISMARK_POS_BELOW; break;
            default: return sal_False;
        }
        SetValue( (sal_Int16)nValue );
    }
    break;
    }
    return bRet;
}

USHORT SvxEmphasisMarkItem::GetVersion( USHORT nFFVer ) const
{
    DBG_ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
            SOFFICE_FILEFORMAT_40==nFFVer ||
            SOFFICE_FILEFORMAT_50==nFFVer,
            "SvxEmphasisMarkItem: Gibt es ein neues Fileformat?" );

    return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
}


/*************************************************************************
|*    class SvxTwoLinesItem
*************************************************************************/

SvxTwoLinesItem::SvxTwoLinesItem( sal_Bool bFlag, sal_Unicode nStartBracket,
                                    sal_Unicode nEndBracket, sal_uInt16 nW )
    : SfxPoolItem( nW ),
    bOn( bFlag ), cStartBracket( nStartBracket ), cEndBracket( nEndBracket )
{
}

SvxTwoLinesItem::SvxTwoLinesItem( const SvxTwoLinesItem& rAttr )
    : SfxPoolItem( rAttr.Which() ),
    bOn( rAttr.bOn ), cStartBracket( rAttr.cStartBracket ),
    cEndBracket( rAttr.cEndBracket )
{
}

SvxTwoLinesItem::~SvxTwoLinesItem()
{
}

int SvxTwoLinesItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rAttr ), "not equal attribute types" );
    return bOn == ((SvxTwoLinesItem&)rAttr).bOn &&
           cStartBracket == ((SvxTwoLinesItem&)rAttr).cStartBracket &&
           cEndBracket == ((SvxTwoLinesItem&)rAttr).cEndBracket;
}

SfxPoolItem* SvxTwoLinesItem::Clone( SfxItemPool* ) const
{
    return new SvxTwoLinesItem( *this );
}

sal_Bool SvxTwoLinesItem::QueryValue( com::sun::star::uno::Any& rVal,
                                BYTE nMemberId ) const
{
    sal_Bool bRet = sal_True;
    switch( nMemberId )
    {
    case MID_TWOLINES:
        rVal = Bool2Any( bOn );
        break;
    case MID_START_BRACKET:
        {
            OUString s;
            if( cStartBracket )
                s = OUString( cStartBracket );
            rVal <<= s;
        }
        break;
    case MID_END_BRACKET:
        {
            OUString s;
            if( cEndBracket )
                s = OUString( cEndBracket );
            rVal <<= s;
        }
        break;
    default:
        bRet = sal_False;
        break;
    }
    return bRet;
}

sal_Bool SvxTwoLinesItem::PutValue( const com::sun::star::uno::Any& rVal,
                                    BYTE nMemberId )
{
    sal_Bool bRet = sal_False;
    OUString s;
    switch( nMemberId )
    {
    case MID_TWOLINES:
        bOn = Any2Bool( rVal );
        bRet = sal_True;
        break;
    case MID_START_BRACKET:
        if( rVal >>= s )
        {
            cStartBracket = s.getLength() ? s[ 0 ] : 0;
            bRet = sal_True;
        }
        break;
    case MID_END_BRACKET:
        if( rVal >>= s )
        {
            cEndBracket = s.getLength() ? s[ 0 ] : 0;
            bRet = sal_True;
        }
        break;
    }
    return bRet;
}

SfxItemPresentation SvxTwoLinesItem::GetPresentation( SfxItemPresentation ePres,
                            SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric,
                            String &rText, const International* pIntl ) const
{
    switch( ePres )
    {
    case SFX_ITEM_PRESENTATION_NONE:
        rText.Erase();
        break;
    case SFX_ITEM_PRESENTATION_NAMELESS:
    case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            if( !GetValue() )
                rText = SVX_RESSTR( RID_SVXITEMS_TWOLINES_OFF );
            else
            {
                rText = SVX_RESSTR( RID_SVXITEMS_TWOLINES );
                if( GetStartBracket() )
                    rText.Insert( GetStartBracket(), 0 );
                if( GetEndBracket() )
                    rText += GetEndBracket();
            }
            return ePres;
        }
        break;
    }
    return SFX_ITEM_PRESENTATION_NONE;
}


SfxPoolItem* SvxTwoLinesItem::Create( SvStream & rStrm, USHORT nVer) const
{
    sal_Bool bOn;
    sal_Unicode cStart, cEnd;
    rStrm >> bOn >> cStart >> cEnd;
    return new SvxTwoLinesItem( bOn, cStart, cEnd, Which() );
}

SvStream& SvxTwoLinesItem::Store(SvStream & rStrm, USHORT nIVer) const
{
    rStrm << GetValue() << GetStartBracket() << GetEndBracket();
    return rStrm;
}

USHORT SvxTwoLinesItem::GetVersion( USHORT nFFVer ) const
{
    DBG_ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
            SOFFICE_FILEFORMAT_40==nFFVer ||
            SOFFICE_FILEFORMAT_50==nFFVer,
            "SvxTwoLinesItem: Gibt es ein neues Fileformat?" );

    return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
}


/*************************************************************************
|*    class SvxCharRotateItem
*************************************************************************/

SvxCharRotateItem::SvxCharRotateItem( sal_uInt16 nValue,
                                       sal_Bool bFitIntoLine,
                                       const sal_uInt16 nW )
    : SfxUInt16Item( nW, nValue ), bFitToLine( bFitIntoLine )
{
}

SfxPoolItem* SvxCharRotateItem::Clone( SfxItemPool* ) const
{
    return new SvxCharRotateItem( GetValue(), IsFitToLine(), Which() );
}

SfxPoolItem* SvxCharRotateItem::Create( SvStream& rStrm, USHORT ) const
{
    sal_uInt16 nVal;
    sal_Bool b;
    rStrm >> nVal >> b;
    return new SvxCharRotateItem( nVal, b, Which() );
}

SvStream& SvxCharRotateItem::Store( SvStream & rStrm, USHORT ) const
{
    sal_Bool bFlag = IsFitToLine();
    rStrm << GetValue() << bFlag;
    return rStrm;
}

USHORT SvxCharRotateItem::GetVersion( USHORT nFFVer ) const
{
    return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
}

SfxItemPresentation SvxCharRotateItem::GetPresentation(
        SfxItemPresentation ePres,
        SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric,
        String &rText, const International*  ) const
{
    switch( ePres )
    {
    case SFX_ITEM_PRESENTATION_NONE:
        rText.Erase();
        break;
    case SFX_ITEM_PRESENTATION_NAMELESS:
    case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            if( !GetValue() )
                rText = SVX_RESSTR( RID_SVXITEMS_CHARROTATE_OFF );
            else
            {
                rText = SVX_RESSTR( RID_SVXITEMS_CHARROTATE );
                rText.SearchAndReplaceAscii( "$(ARG1)",
                            String::CreateFromInt32( GetValue() ));
                if( IsFitToLine() )
                    rText += SVX_RESSTR( RID_SVXITEMS_CHARROTATE_FITLINE );
            }
            return ePres;
        }
        break;
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

sal_Bool SvxCharRotateItem::QueryValue( com::sun::star::uno::Any& rVal,
                                BYTE nMemberId ) const
{
    sal_Bool bRet = sal_True;
    switch( nMemberId )
    {
    case MID_ROTATE:
        rVal <<= (sal_Int16)GetValue();
        break;
    case MID_FITTOLINE:
        rVal = Bool2Any( IsFitToLine() );
        break;
    default:
        bRet = sal_False;
        break;
    }
    return bRet;
}

sal_Bool SvxCharRotateItem::PutValue( const com::sun::star::uno::Any& rVal,
                                    BYTE nMemberId )
{
    sal_Bool bRet = sal_True;
    sal_Int16 nVal;
    switch( nMemberId )
    {
    case MID_ROTATE:
        rVal >>= nVal;
        if(!nVal || 90 == nVal || 270 == nVal)
            SetValue( (USHORT)nVal );
        else
            bRet = sal_False;
        break;

    case MID_FITTOLINE:
        SetFitToLine( Any2Bool( rVal ) );
        break;
    default:
        bRet = sal_False;
    }
    return bRet;
}

/*************************************************************************
|*    class SvxCharScaleItem
*************************************************************************/

SvxCharScaleWidthItem::SvxCharScaleWidthItem( sal_uInt16 nValue,
                                               const sal_uInt16 nW )
    : SfxUInt16Item( nW, nValue )
{
}

SfxPoolItem* SvxCharScaleWidthItem::Clone( SfxItemPool* ) const
{
    return new SvxCharScaleWidthItem( GetValue(), Which() );
}

SfxPoolItem* SvxCharScaleWidthItem::Create( SvStream& rStrm, USHORT ) const
{
    sal_uInt16 nVal;
    rStrm >> nVal;
    return new SvxCharScaleWidthItem( nVal, Which() );
}

USHORT SvxCharScaleWidthItem::GetVersion( USHORT nFFVer ) const
{
    return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
}

SfxItemPresentation SvxCharScaleWidthItem::GetPresentation(
        SfxItemPresentation ePres,
        SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric,
        String &rText, const International*  ) const
{
    switch( ePres )
    {
    case SFX_ITEM_PRESENTATION_NONE:
        rText.Erase();
        break;
    case SFX_ITEM_PRESENTATION_NAMELESS:
    case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            if( !GetValue() )
                rText = SVX_RESSTR( RID_SVXITEMS_CHARSCALE_OFF );
            else
            {
                rText = SVX_RESSTR( RID_SVXITEMS_CHARSCALE );
                rText.SearchAndReplaceAscii( "$(ARG1)",
                            String::CreateFromInt32( GetValue() ));
            }
            return ePres;
        }
        break;
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

/*************************************************************************
|*    class SvxScriptTypeItemItem
*************************************************************************/

SvxScriptTypeItem::SvxScriptTypeItem( sal_uInt16 nType )
    : SfxUInt16Item( SID_ATTR_CHAR_SCRIPTTYPE, nType )
{
}
SfxPoolItem* SvxScriptTypeItem::Clone( SfxItemPool *pPool ) const
{
    return new SvxScriptTypeItem( GetValue() );
}

/*************************************************************************
|*    class SvxScriptSetItem
*************************************************************************/

SvxScriptSetItem::SvxScriptSetItem( USHORT nSlotId, SfxItemPool& rPool )
    : SfxSetItem( nSlotId, new SfxItemSet( rPool,
                        SID_ATTR_CHAR_FONT, SID_ATTR_CHAR_FONT ))
{
    USHORT nLatin, nAsian, nComplex;
    GetWhichIds( nLatin, nAsian, nComplex );

    USHORT aIds[ 9 ] = { 0 };
    aIds[ 0 ] = aIds[ 1 ] = nLatin;
    aIds[ 2 ] = aIds[ 3 ] = nAsian;
    aIds[ 4 ] = aIds[ 5 ] = nComplex;
    aIds[ 6 ] = aIds[ 7 ] = SID_ATTR_CHAR_SCRIPTTYPE;
    aIds[ 8 ] = 0;

    GetItemSet().SetRanges( aIds );
}

SfxPoolItem* SvxScriptSetItem::Clone( SfxItemPool *pPool ) const
{
    SvxScriptSetItem* p = new SvxScriptSetItem( Which(), *GetItemSet().GetPool() );
    p->GetItemSet().Put( GetItemSet(), FALSE );
    return p;
}

SfxPoolItem* SvxScriptSetItem::Create( SvStream &, USHORT ) const
{
    return 0;
}

const SfxPoolItem* SvxScriptSetItem::GetItemOfScriptSet(
                            const SfxItemSet& rSet, USHORT nId )
{
    const SfxPoolItem* pI;
    SfxItemState eSt = rSet.GetItemState( nId, FALSE, &pI );
    if( SFX_ITEM_SET != eSt )
        pI = SFX_ITEM_DEFAULT == eSt ? &rSet.Get( nId ) : 0;
    return pI;
}

const SfxPoolItem* SvxScriptSetItem::GetItemOfScript( USHORT nScript ) const
{
    USHORT nLatin, nAsian, nComplex;
    GetWhichIds( nLatin, nAsian, nComplex );

    const SfxItemSet& rSet = GetItemSet();
    const SfxPoolItem *pRet, *pAsn, *pCmplx;
    switch( nScript )
    {
    default:                //no one valid -> match to latin
    //  case SCRIPTTYPE_LATIN:
        pRet = GetItemOfScriptSet( rSet, nLatin );
        break;
    case SCRIPTTYPE_ASIAN:
        pRet = GetItemOfScriptSet( rSet, nAsian );
        break;
    case SCRIPTTYPE_COMPLEX:
        pRet = GetItemOfScriptSet( rSet, nComplex );
        break;

    case SCRIPTTYPE_LATIN|SCRIPTTYPE_ASIAN:
        if( 0 == (pRet = GetItemOfScriptSet( rSet, nLatin )) ||
            0 == (pAsn = GetItemOfScriptSet( rSet, nAsian )) ||
            *pRet != *pAsn )
            pRet = 0;
        break;
        break;

    case SCRIPTTYPE_LATIN|SCRIPTTYPE_COMPLEX:
        if( 0 == (pRet = GetItemOfScriptSet( rSet, nLatin )) ||
            0 == (pCmplx = GetItemOfScriptSet( rSet, nComplex )) ||
            *pRet != *pCmplx )
            pRet = 0;
        break;

    case SCRIPTTYPE_ASIAN|SCRIPTTYPE_COMPLEX:
        if( 0 == (pRet = GetItemOfScriptSet( rSet, nAsian )) ||
            0 == (pCmplx = GetItemOfScriptSet( rSet, nComplex )) ||
            *pRet != *pCmplx )
            pRet = 0;
        break;

    case SCRIPTTYPE_LATIN|SCRIPTTYPE_ASIAN|SCRIPTTYPE_COMPLEX:
        if( 0 == (pRet = GetItemOfScriptSet( rSet, nLatin )) ||
            0 == (pAsn = GetItemOfScriptSet( rSet, nAsian )) ||
            0 == (pCmplx = GetItemOfScriptSet( rSet, nComplex )) ||
            *pRet != *pAsn || *pRet != *pCmplx )
            pRet = 0;
        break;
    }
    return pRet;
}


void SvxScriptSetItem::PutItemForScriptType( USHORT nScriptType,
                                             const SfxPoolItem& rItem )
{
    USHORT nLatin, nAsian, nComplex;
    GetWhichIds( nLatin, nAsian, nComplex );

    SfxPoolItem* pCpy = rItem.Clone();
    if( SCRIPTTYPE_LATIN & nScriptType )
    {
        pCpy->SetWhich( nLatin );
        GetItemSet().Put( *pCpy );
    }
    if( SCRIPTTYPE_ASIAN & nScriptType )
    {
        pCpy->SetWhich( nAsian );
        GetItemSet().Put( *pCpy );
    }
    if( SCRIPTTYPE_COMPLEX & nScriptType )
    {
        pCpy->SetWhich( nComplex );
        GetItemSet().Put( *pCpy );
    }
    delete pCpy;
}

void SvxScriptSetItem::GetWhichIds( USHORT& rLatin, USHORT& rAsian,
                                    USHORT& rComplex ) const
{
    const SfxItemPool& rPool = *GetItemSet().GetPool();
    GetSlotIds( Which(), rLatin, rAsian, rComplex );
    rLatin = rPool.GetWhich( rLatin );
    rAsian = rPool.GetWhich( rAsian );
    rComplex = rPool.GetWhich( rComplex );
}

void SvxScriptSetItem::GetSlotIds( USHORT nSlotId, USHORT& rLatin,
                                    USHORT& rAsian, USHORT& rComplex )
{
    switch( nSlotId )
    {
    default:
        DBG_ASSERT( FALSE, "wrong SlotId for class SvxScriptSetItem" );
        // no break - default to font - Id Range !!

    case SID_ATTR_CHAR_FONT:
        rLatin = SID_ATTR_CHAR_FONT;
        rAsian = SID_ATTR_CHAR_CJK_FONT;
        rComplex = SID_ATTR_CHAR_CTL_FONT;
        break;
    case SID_ATTR_CHAR_FONTHEIGHT:
        rLatin = SID_ATTR_CHAR_FONTHEIGHT;
        rAsian = SID_ATTR_CHAR_CJK_FONTHEIGHT;
        rComplex = SID_ATTR_CHAR_CTL_FONTHEIGHT;
        break;
    case SID_ATTR_CHAR_WEIGHT:
        rLatin = SID_ATTR_CHAR_WEIGHT;
        rAsian = SID_ATTR_CHAR_CJK_WEIGHT;
        rComplex = SID_ATTR_CHAR_CTL_WEIGHT;
        break;
    case SID_ATTR_CHAR_POSTURE:
        rLatin = SID_ATTR_CHAR_POSTURE;
        rAsian = SID_ATTR_CHAR_CJK_POSTURE;
        rComplex = SID_ATTR_CHAR_CTL_POSTURE;
        break;
    case SID_ATTR_CHAR_LANGUAGE:
        rLatin = SID_ATTR_CHAR_LANGUAGE;
        rAsian = SID_ATTR_CHAR_CJK_LANGUAGE;
        rComplex = SID_ATTR_CHAR_CTL_LANGUAGE;
        break;
    }
}

void GetDefaultFonts( SvxFontItem& rLatin, SvxFontItem& rAsian,
                        SvxFontItem& rComplex )
{
    rtl_TextEncoding eFontTextEncoding = ::gsl_getSystemTextEncoding();
    rLatin.GetFamily() = FAMILY_ROMAN;
    rLatin.GetFamilyName() = System::GetStandardFont( STDFONT_ROMAN ).GetName();
    rLatin.GetStyleName().Erase();
    rLatin.GetPitch() = PITCH_VARIABLE;
    rLatin.GetCharSet() = eFontTextEncoding;

    rAsian.GetFamily() = FAMILY_DONTKNOW;
    rAsian.GetStyleName().Erase();
    rAsian.GetPitch() = PITCH_DONTKNOW;
    rAsian.GetCharSet() = RTL_TEXTENCODING_DONTKNOW;

    rComplex.GetFamily() = FAMILY_DONTKNOW;
    rComplex.GetStyleName().Erase();
    rComplex.GetPitch() = PITCH_DONTKNOW;
    rComplex.GetCharSet() = RTL_TEXTENCODING_DONTKNOW;

    rAsian.GetFamilyName().AssignAscii( RTL_CONSTASCII_STRINGPARAM(
                        "MS Mincho;HG Mincho L;MS PGothic" ));
    rComplex.GetFamilyName().AssignAscii( RTL_CONSTASCII_STRINGPARAM(
                        "Simplified Arabic" ));

    USHORT nLng = System::GetLanguage();
    switch( nLng )
    {
    case LANGUAGE_CHINESE:
     case LANGUAGE_CHINESE_TRADITIONAL:
     case LANGUAGE_CHINESE_HONGKONG:
     case LANGUAGE_CHINESE_SINGAPORE:
     case LANGUAGE_CHINESE_MACAU:
        rAsian.GetFamilyName().AssignAscii( RTL_CONSTASCII_STRINGPARAM(
                        "PmingLiU;Ming" ));
        break;

     case LANGUAGE_CHINESE_SIMPLIFIED:
        rAsian.GetFamilyName().AssignAscii( RTL_CONSTASCII_STRINGPARAM(
                        "SimSun;Song" ));
        break;
    case LANGUAGE_KOREAN:
    case LANGUAGE_KOREAN_JOHAB:
        rAsian.GetFamilyName().AssignAscii( RTL_CONSTASCII_STRINGPARAM(
                        "Batang;Myeomgjo;Gulim" ));
        break;
    }

    String sFirst( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM(
                    "Andale WT UI;" )));
    String sLast( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM(
                    "Arial Unicode MS" )));
    (rAsian.GetFamilyName().Insert( sFirst, 0 ) += ';' ) += sLast;
    ((rComplex.GetFamilyName() += ';' ) += sFirst ) += sLast;
}


