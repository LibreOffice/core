/*************************************************************************
 *
 *  $RCSfile: paraitem.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: mtg $ $Date: 2001-03-22 15:23:23 $
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

#ifndef _COM_SUN_STAR_STYLE_TABSTOP_HPP_
#include <com/sun/star/style/TabStop.hpp>
#endif
#ifndef  _COM_SUN_STAR_STYLE_LINESPACING_HPP_
#include <com/sun/star/style/LineSpacing.hpp>
#endif
#ifndef  _COM_SUN_STAR_STYLE_LINESPACINGMODE_HPP_
#include <com/sun/star/style/LineSpacingMode.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#include <comphelper/types.hxx>

using namespace ::rtl;
using namespace ::com::sun::star;

#define ITEMID_TABSTOP      0
#define ITEMID_LINESPACING  0
#define ITEMID_ADJUST       0
#define ITEMID_ORPHANS      0
#define ITEMID_WIDOWS       0
#define ITEMID_PAGEMODEL    0
#define ITEMID_FMTSPLIT     0
#define ITEMID_HYPHENZONE   0
#define ITEMID_SCRIPTSPACE  0
#define ITEMID_HANGINGPUNCTUATION 0
#define ITEMID_FORBIDDENRULE 0

#include <tools/rtti.hxx>
#include <svtools/sbx.hxx>
#define GLOBALOVERFLOW3

#define _SVX_PARAITEM_CXX

#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif

#ifndef _ARGS_HXX //autogen
#include <svtools/args.hxx>
#endif
#include <svtools/memberid.hrc>
#include "svxitems.hrc"

#include "lspcitem.hxx"
#include "adjitem.hxx"
#include "orphitem.hxx"
#include "widwitem.hxx"
#include "tstpitem.hxx"
#include "pmdlitem.hxx"
#include "spltitem.hxx"
#include "hyznitem.hxx"
#include "scriptspaceitem.hxx"
#include "hngpnctitem.hxx"
#include "forbiddenruleitem.hxx"


// xml stuff
#ifndef _XMLOFF_XMLKYWD_HXX
#include <xmloff/xmlkywd.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif
#include <rtl/ustring>
#include <rtl/ustrbuf.hxx>

#include <unomid.hxx>

#include "itemtype.hxx"
#include "dialmgr.hxx"
#include "paperinf.hxx"

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

#include <algorithm>

using namespace ::rtl;
using namespace ::com::sun::star;

// Konvertierung fuer UNO
#define TWIP_TO_MM100(TWIP)     ((TWIP) >= 0 ? (((TWIP)*127L+36L)/72L) : (((TWIP)*127L-36L)/72L))
#define MM100_TO_TWIP(MM100)    ((MM100) >= 0 ? (((MM100)*72L+63L)/127L) : (((MM100)*72L-63L)/127L))


// STATIC DATA -----------------------------------------------------------


// -----------------------------------------------------------------------


TYPEINIT1_AUTOFACTORY(SvxLineSpacingItem, SfxPoolItem );
TYPEINIT1_AUTOFACTORY(SvxAdjustItem, SfxPoolItem);
TYPEINIT1_AUTOFACTORY(SvxWidowsItem, SfxByteItem);
TYPEINIT1_AUTOFACTORY(SvxOrphansItem, SfxByteItem);
TYPEINIT1_AUTOFACTORY(SvxHyphenZoneItem, SfxPoolItem);
TYPEINIT1_AUTOFACTORY(SvxTabStopItem, SfxPoolItem);
TYPEINIT1_AUTOFACTORY(SvxFmtSplitItem, SfxBoolItem);
TYPEINIT1_AUTOFACTORY(SvxPageModelItem, SfxStringItem);
TYPEINIT1_AUTOFACTORY(SvxScriptSpaceItem, SfxBoolItem);
TYPEINIT1_AUTOFACTORY(SvxHangingPunctuationItem, SfxBoolItem);
TYPEINIT1_AUTOFACTORY(SvxForbiddenRuleItem, SfxBoolItem);


SV_IMPL_VARARR_SORT( SvxTabStopArr, SvxTabStop )

// -----------------------------------------------------------------------

SvxLineSpacingItem::SvxLineSpacingItem( sal_uInt16 nHeight, const sal_uInt16 nId )
    : SfxEnumItemInterface( nId )
{
    nPropLineSpace = 100;
    nInterLineSpace = 0;
    nLineHeight = nHeight;
    eLineSpace = SVX_LINE_SPACE_AUTO;
    eInterLineSpace = SVX_INTER_LINE_SPACE_OFF;
}

// -----------------------------------------------------------------------

int SvxLineSpacingItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    const SvxLineSpacingItem& rLineSpace = (const SvxLineSpacingItem&)rAttr;
    return (
        // Gleiche Linespacing Rule?
        (eLineSpace == rLineSpace.eLineSpace)
        // Bei maximalem und minimalem Linespacing muss das Mass
        // uebereinstimmen.
        && (eLineSpace == SVX_LINE_SPACE_AUTO ||
            nLineHeight == rLineSpace.nLineHeight)
        // Gleiche Interlinespacing Rule?
        && ( eInterLineSpace == rLineSpace.eInterLineSpace )
        // Entweder proportional oder draufaddieren eingestellt.
        && (( eInterLineSpace == SVX_INTER_LINE_SPACE_OFF)
            || (eInterLineSpace == SVX_INTER_LINE_SPACE_PROP
                && nPropLineSpace == rLineSpace.nPropLineSpace)
            || (eInterLineSpace == SVX_INTER_LINE_SPACE_FIX
                && (nInterLineSpace == rLineSpace.nInterLineSpace)))) ?
                1 : 0;
}

/*-----------------18.03.98 16:32-------------------
    os: wer weiss noch, wieso das LineSpacingItem so
    kompliziert ist? Fuer UNO koennen wir das nicht
    gebrauchen. Da gibt es nur zwei Werte:
        - ein sal_uInt16 fuer den Modus
        - ein sal_uInt32 fuer alle Werte (Abstand, Hoehe, rel. Angaben)

--------------------------------------------------*/
sal_Bool SvxLineSpacingItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    style::LineSpacing aLSp;
    switch( eLineSpace )
    {
        case SVX_LINE_SPACE_AUTO:
            if(eInterLineSpace == SVX_INTER_LINE_SPACE_FIX)
            {
                aLSp.Mode = style::LineSpacingMode::LEADING;
                aLSp.Height = nInterLineSpace;
            }
            else if(eInterLineSpace == SVX_INTER_LINE_SPACE_OFF)
            {
                aLSp.Mode = style::LineSpacingMode::PROP;
                aLSp.Height = 100;
            }
            else
            {
                aLSp.Mode = style::LineSpacingMode::PROP;
                aLSp.Height = nPropLineSpace;
            }
        break;
        case SVX_LINE_SPACE_FIX :
        case SVX_LINE_SPACE_MIN :
            aLSp.Mode = eLineSpace == SVX_LINE_SPACE_FIX ? style::LineSpacingMode::FIX : style::LineSpacingMode::MINIMUM;
            aLSp.Height = nMemberId&CONVERT_TWIPS ? TWIP_TO_MM100(nLineHeight) : nLineHeight;
        break;
    }
    rVal <<= aLSp;

    return sal_True;
}
/*-----------------18.03.98 16:32-------------------

--------------------------------------------------*/
sal_Bool SvxLineSpacingItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    style::LineSpacing aLSp;
    if(!(rVal >>= aLSp))
        return sal_False;

    switch( aLSp.Mode )
    {
        case style::LineSpacingMode::LEADING:
        {
            eInterLineSpace = SVX_INTER_LINE_SPACE_FIX;
            eLineSpace = SVX_LINE_SPACE_AUTO;
            nInterLineSpace = aLSp.Height;
        }
        break;
        case style::LineSpacingMode::PROP:
        {
            eLineSpace = SVX_LINE_SPACE_AUTO;
            nPropLineSpace = (sal_Int8)std::min(aLSp.Height, (short)0xFF);
            if(100 == aLSp.Height)
                eInterLineSpace = SVX_INTER_LINE_SPACE_OFF;
            else
                eInterLineSpace = SVX_INTER_LINE_SPACE_PROP;
        }
        break;
        case style::LineSpacingMode::FIX:
        case style::LineSpacingMode::MINIMUM:
        {
            eInterLineSpace =  SVX_INTER_LINE_SPACE_OFF;
            eLineSpace = aLSp.Mode == style::LineSpacingMode::FIX ? SVX_LINE_SPACE_FIX : SVX_LINE_SPACE_MIN;
            nLineHeight = aLSp.Height;
            if(nMemberId&CONVERT_TWIPS)
                nLineHeight = MM100_TO_TWIP(nLineHeight);
        }
        break;
    }
    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool SvxLineSpacingItem::importXML( const OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter )
{
#ifndef SVX_LIGHT

    style::LineSpacing aLSp;
    sal_Int32 nTemp = 0;

    switch( nMemberId )
    {
    case MID_LS_FIXED:
        if( -1 != rValue.indexOf( sal_Unicode( '%' ) ) )
        {
            aLSp.Mode = style::LineSpacingMode::PROP;
            if(!rUnitConverter.convertPercent( nTemp, rValue ))
                return sal_False;

            aLSp.Height = nTemp;
        }
        else if( rValue.compareToAscii( sXML_casemap_normal ) )
        {
            aLSp.Mode = style::LineSpacingMode::PROP;
            aLSp.Height = 100;
        }
        else
        {
            aLSp.Mode = style::LineSpacingMode::FIX;
        }
        break;

    case MID_LS_MINIMUM:
        aLSp.Mode = style::LineSpacingMode::MINIMUM;
        break;

    case MID_LS_DISTANCE:
        aLSp.Mode = style::LineSpacingMode::LEADING;
        break;
    }

    if( aLSp.Mode != style::LineSpacingMode::PROP )
    {
        // get the height
        if(!rUnitConverter.convertMeasure( nTemp, rValue, 0x0000, 0xffff ))
            return sal_False;
        aLSp.Height = nTemp;
    }

    // use PutValue to set set the LineSpace
    uno::Any aAny(&aLSp, ::getCppuType((const style::LineSpacing*)0));
    return PutValue( aAny, 0 );
#else
    return sal_False;
#endif
}

// -----------------------------------------------------------------------


sal_Bool SvxLineSpacingItem::exportXML( OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter ) const
{
#ifndef SVX_LIGHT
    OUStringBuffer aOut;

    uno::Any aAny;
    QueryValue( aAny, 0 );
    style::LineSpacing* pLSp =  (style::LineSpacing*)aAny.getValue();

    const sal_Int16 Mode = pLSp->Mode;
    const sal_Int16 Height = pLSp->Height;

    // check if this memberid is valid for export
    switch( nMemberId )
    {
    case MID_LS_FIXED:
        if( style::LineSpacingMode::PROP != Mode &&
            style::LineSpacingMode::FIX  != Mode )
            return sal_False;
        break;

    case MID_LS_MINIMUM:
        if( style::LineSpacingMode::MINIMUM != Mode )
            return sal_False;
        break;

    case MID_LS_DISTANCE:
        if( style::LineSpacingMode::LEADING != Mode )
            return sal_False;
        break;
    }

    if( style::LineSpacingMode::PROP == Mode )
    {
        rUnitConverter.convertPercent( aOut, Height );
    }
    else
    {
        rUnitConverter.convertMeasure( aOut, Height );
    }

    rValue = aOut.makeStringAndClear();
#endif
    return sal_True;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxLineSpacingItem::Clone( SfxItemPool * ) const
{
    return new SvxLineSpacingItem( *this );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxLineSpacingItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
#ifndef PRODUCT
    rText.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "SvxLineSpacingItem" ));
#else
    rText.Erase();
#endif
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxLineSpacingItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_Int8    nPropSpace;
    short   nInterSpace;
    sal_uInt16  nHeight;
    sal_Int8    nRule, nInterRule;

    rStrm >> nPropSpace
          >> nInterSpace
          >> nHeight
          >> nRule
          >> nInterRule;

    SvxLineSpacingItem* pAttr = new SvxLineSpacingItem( nHeight, Which() );
    pAttr->SetInterLineSpace( nInterSpace );
    pAttr->SetPropLineSpace( nPropSpace );
    pAttr->GetLineSpaceRule() = (SvxLineSpace)nRule;
    pAttr->GetInterLineSpaceRule() = (SvxInterLineSpace)nInterRule;
    return pAttr;
}

// -----------------------------------------------------------------------

SvStream& SvxLineSpacingItem::Store( SvStream& rStrm, sal_uInt16 nItemVersion ) const
{
    rStrm << (sal_Int8)  GetPropLineSpace()
          << (short)  GetInterLineSpace()
          << (sal_uInt16) GetLineHeight()
          << (sal_Int8)   GetLineSpaceRule()
          << (sal_Int8)   GetInterLineSpaceRule();
    return rStrm;
}

// -----------------------------------------------------------------------

sal_uInt16 SvxLineSpacingItem::GetValueCount() const
{
    return SVX_LINESPACE_END;   // SVX_LINESPACE_TWO_LINES + 1
}

// -----------------------------------------------------------------------

XubString SvxLineSpacingItem::GetValueTextByPos( sal_uInt16 nPos ) const
{
    //! Strings demnaechst aus Resource laden
    XubString aText;
    switch ( nPos )
    {
        case SVX_LINESPACE_USER                 : aText.AppendAscii( "Benutzer" );  break;
        case SVX_LINESPACE_ONE_LINE             : aText.AppendAscii( "Einzeilig" ); break;
        case SVX_LINESPACE_ONE_POINT_FIVE_LINES : aText.AppendAscii( "1,5zeilig" ); break;
        case SVX_LINESPACE_TWO_LINES            : aText.AppendAscii( "Zweizeilig" );    break;
    }
    return aText;
}

// -----------------------------------------------------------------------

sal_uInt16 SvxLineSpacingItem::GetEnumValue() const
{
    sal_uInt16 nVal;
    switch ( nPropLineSpace )
    {
        case 100:   nVal = SVX_LINESPACE_ONE_LINE;              break;
        case 150:   nVal = SVX_LINESPACE_ONE_POINT_FIVE_LINES;  break;
        case 200:   nVal = SVX_LINESPACE_TWO_LINES;             break;
        default:    nVal = SVX_LINESPACE_USER;                  break;
    }
    return nVal;
}

// -----------------------------------------------------------------------

void SvxLineSpacingItem::SetEnumValue( sal_uInt16 nVal )
{
    switch ( nVal )
    {
        case SVX_LINESPACE_ONE_LINE:             nPropLineSpace = 100; break;
        case SVX_LINESPACE_ONE_POINT_FIVE_LINES: nPropLineSpace = 150; break;
        case SVX_LINESPACE_TWO_LINES:            nPropLineSpace = 200; break;
    }
}

// class SvxAdjustItem ---------------------------------------------------

SvxAdjustItem::SvxAdjustItem(const SvxAdjust eAdjst, const sal_uInt16 nId )
    : SfxEnumItemInterface( nId ),
    bOneBlock( sal_False ), bLastCenter( sal_False ), bLastBlock( sal_False )
{
    SetAdjust( eAdjst );
}

// -----------------------------------------------------------------------

int SvxAdjustItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    return( ( GetAdjust() == ((SvxAdjustItem&)rAttr).GetAdjust() &&
        bOneBlock == ((SvxAdjustItem&)rAttr).bOneBlock &&
        bLastCenter == ((SvxAdjustItem&)rAttr).bLastCenter &&
        bLastBlock == ((SvxAdjustItem&)rAttr).bLastBlock )
        ? 1 : 0 );
}

/*-----------------18.03.98 16:15-------------------

--------------------------------------------------*/
sal_Bool SvxAdjustItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    switch( nMemberId )
    {
        case MID_PARA_ADJUST      : rVal <<= (sal_Int16)GetAdjust(); break;
        case MID_LAST_LINE_ADJUST : rVal <<= (sal_Int16)GetLastBlock(); break;
        case MID_EXPAND_SINGLE    :
        {
            sal_Bool bValue = bOneBlock;
            rVal.setValue( &bValue, ::getCppuBooleanType() );
            break;
        }
    }
    return sal_True;
}
/*-----------------18.03.98 16:15-------------------

--------------------------------------------------*/

sal_Bool SvxAdjustItem::PutValue( const uno::Any& rVal, BYTE nMemberId  )
{
    switch( nMemberId )
    {
        case MID_PARA_ADJUST              :
        case MID_LAST_LINE_ADJUST :
        {
            sal_Int32 eVal = - 1;
            try
            {
                eVal = ::comphelper::getEnumAsINT32(rVal);
            }
            catch(...) {}
            if(eVal >= 0 && eVal <= 4)
            {
                if(MID_LAST_LINE_ADJUST == nMemberId &&
                    eVal != SVX_ADJUST_LEFT &&
                    eVal != SVX_ADJUST_BLOCK &&
                    eVal != SVX_ADJUST_CENTER)
                        return FALSE;
                if(eVal < (sal_uInt16)SVX_ADJUST_END)
                    nMemberId == MID_PARA_ADJUST ?
                        SetAdjust((SvxAdjust)eVal) :
                            SetLastBlock((SvxAdjust)eVal);
            }
        }
        break;
        case MID_EXPAND_SINGLE :
            bOneBlock = Any2Bool(rVal);
            break;
    }
    return sal_True;
}

// -----------------------------------------------------------------------

#ifndef SVX_LIGHT
SvXMLEnumMapEntry pXML_para_adjust_enums[] =
{
    { sXML_start,       SVX_ADJUST_LEFT },
    { sXML_end,         SVX_ADJUST_RIGHT },
    { sXML_center,      SVX_ADJUST_CENTER },
    { sXML_justify,     SVX_ADJUST_BLOCK },
    { sXML_justified,   SVX_ADJUST_BLOCK },
    { 0, 0 }
};

SvXMLEnumMapEntry pXML_para_align_last_enums[] =
{
    { sXML_start,       SVX_ADJUST_LEFT },
    { sXML_center,      SVX_ADJUST_CENTER },
    { sXML_justify,     SVX_ADJUST_BLOCK },
    { sXML_justified,   SVX_ADJUST_BLOCK },
    { 0, 0 }
};
#endif

sal_Bool SvxAdjustItem::importXML( const OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter )
{
    sal_Bool bOk = sal_False;
#ifndef SVX_LIGHT
    switch( nMemberId )
    {
    case MID_PARA_ADJUST:
        sal_uInt16 eAdjust;
        if( rUnitConverter.convertEnum( eAdjust, rValue, pXML_para_adjust_enums ) )
        {
            SetAdjust( (SvxAdjust)eAdjust );
            bOk = sal_True;
        }
        break;

    case MID_LAST_LINE_ADJUST:
        {
            sal_uInt16 eAdjust;
            if( rUnitConverter.convertEnum( eAdjust, rValue,
                                              pXML_para_align_last_enums ) )
            {
                SetLastBlock( (SvxAdjust)eAdjust );
                bOk = sal_True;
            }
        }
        break;
    case MID_EXPAND_SINGLE:
        {
            sal_Bool bBlock;
            if( rUnitConverter.convertBool( bBlock, rValue ) )
            {
                SetOneWord( bBlock ? SVX_ADJUST_BLOCK : SVX_ADJUST_LEFT );
                bOk = sal_True;
            }
        }
        break;
    }

#endif
    return bOk;
}

// -----------------------------------------------------------------------

sal_Bool SvxAdjustItem::exportXML( OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bOk = sal_False;
#ifndef SVX_LIGHT
    OUStringBuffer aOut;

    switch( nMemberId )
    {
    case MID_PARA_ADJUST:
        bOk = rUnitConverter.convertEnum( aOut,
                                          GetAdjust(),
                                          pXML_para_adjust_enums, sXML_start );
        break;
    case MID_LAST_LINE_ADJUST:
        {
            if( GetAdjust() == SVX_ADJUST_BLOCK )
            {
                SvxAdjust eAdjust = GetLastBlock();
                if( eAdjust != SVX_ADJUST_LEFT )
                    bOk = rUnitConverter.convertEnum( aOut, eAdjust,
                                                  pXML_para_align_last_enums );
            }
        }
        break;
    case MID_EXPAND_SINGLE:
        {
            if( GetAdjust() == SVX_ADJUST_BLOCK &&
                GetLastBlock() == SVX_ADJUST_BLOCK &&
                 GetOneWord() == SVX_ADJUST_BLOCK )
            {
                rUnitConverter.convertBool( aOut, sal_True );
                bOk = sal_True;
            }
        }
        break;
    }

    rValue = aOut.makeStringAndClear();
#endif
    return bOk;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxAdjustItem::Clone( SfxItemPool * ) const
{
    return new SvxAdjustItem( *this );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxAdjustItem::GetPresentation
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
            return SFX_ITEM_PRESENTATION_NONE;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = GetValueTextByPos( (sal_uInt16)GetAdjust() );
            return ePres;
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

sal_uInt16 SvxAdjustItem::GetValueCount() const
{
    return SVX_ADJUST_END;  // SVX_ADJUST_BLOCKLINE + 1
}

// -----------------------------------------------------------------------

XubString SvxAdjustItem::GetValueTextByPos( sal_uInt16 nPos ) const
{
    DBG_ASSERT( nPos <= (sal_uInt16)SVX_ADJUST_BLOCKLINE, "enum overflow!" );
    return SVX_RESSTR(RID_SVXITEMS_ADJUST_BEGIN + nPos);
}

// -----------------------------------------------------------------------

sal_uInt16 SvxAdjustItem::GetEnumValue() const
{
    return GetAdjust();
}

// -----------------------------------------------------------------------

void SvxAdjustItem::SetEnumValue( sal_uInt16 nVal )
{
    SetAdjust( (const SvxAdjust)nVal );
}

// -----------------------------------------------------------------------

sal_uInt16 SvxAdjustItem::GetVersion( sal_uInt16 nFileVersion ) const
{
    return (nFileVersion == SOFFICE_FILEFORMAT_31)
               ? 0 : ADJUST_LASTBLOCK_VERSION;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxAdjustItem::Create(SvStream& rStrm, sal_uInt16 nVersion) const
{
    char eAdjustment;
    rStrm >> eAdjustment;
    SvxAdjustItem *pRet = new SvxAdjustItem( (SvxAdjust)eAdjustment, Which() );
    if( nVersion >= ADJUST_LASTBLOCK_VERSION )
    {
        sal_Int8 nFlags;
        rStrm >> nFlags;
        pRet->bOneBlock = 0 != (nFlags & 0x0001);
        pRet->bLastCenter = 0 != (nFlags & 0x0002);
        pRet->bLastBlock = 0 != (nFlags & 0x0004);
    }
    return pRet;
}

// -----------------------------------------------------------------------

SvStream& SvxAdjustItem::Store( SvStream& rStrm, sal_uInt16 nItemVersion ) const
{
    rStrm << (char)GetAdjust();
    if ( nItemVersion >= ADJUST_LASTBLOCK_VERSION )
    {
        sal_Int8 nFlags = 0;
        if ( bOneBlock )
            nFlags |= 0x0001;
        if ( bLastCenter )
            nFlags |= 0x0002;
        if ( bLastBlock )
            nFlags |= 0x0004;
        rStrm << (sal_Int8) nFlags;
    }
    return rStrm;
}

// class SvxWidowsItem ---------------------------------------------------

SvxWidowsItem::SvxWidowsItem(const BYTE nL, const USHORT nId ) :
    SfxByteItem( nId, nL )
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxWidowsItem::Clone( SfxItemPool * ) const
{
    return new SvxWidowsItem( *this );
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxWidowsItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_Int8 nLines;
    rStrm >> nLines;
    return new SvxWidowsItem( nLines, Which() );
}

// -----------------------------------------------------------------------

SvStream& SvxWidowsItem::Store( SvStream& rStrm, sal_uInt16 nItemVersion ) const
{
    rStrm << (sal_Int8)GetValue();
    return rStrm;
}

//------------------------------------------------------------------------

SfxItemPresentation SvxWidowsItem::GetPresentation
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
            return SFX_ITEM_PRESENTATION_NONE;

        case SFX_ITEM_PRESENTATION_NAMELESS:
            rText = String::CreateFromInt32( GetValue() );
            rText += SVX_RESSTR(RID_SVXITEMS_LINES);
            return SFX_ITEM_PRESENTATION_NAMELESS;

        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText += SVX_RESSTR(RID_SVXITEMS_WIDOWS_COMPLETE);
            rText += String::CreateFromInt32( GetValue() );
            rText += SVX_RESSTR(RID_SVXITEMS_LINES);
            return SFX_ITEM_PRESENTATION_COMPLETE;
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

sal_Bool SvxWidowsItem::importXML( const OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter )
{
#ifndef SVX_LIGHT
    sal_Int32 nValue;
    sal_Bool bOk = rUnitConverter.convertNumber( nValue, rValue, 0, 0xff );
    if( bOk )
        SetValue( nValue );
    return bOk;
#else
    return sal_False;
#endif
}

sal_Bool SvxWidowsItem::exportXML( OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter ) const
{
#ifndef SVX_LIGHT
    OUStringBuffer aOut;
    rUnitConverter.convertNumber( aOut, (sal_Int32) GetValue() );
    rValue = aOut.makeStringAndClear();
#endif
    return sal_True;
}

// class SvxOrphansItem --------------------------------------------------

SvxOrphansItem::SvxOrphansItem(const BYTE nL, const USHORT nId ) :
    SfxByteItem( nId, nL )
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxOrphansItem::Clone( SfxItemPool * ) const
{
    return new SvxOrphansItem( *this );
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxOrphansItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_Int8 nLines;
    rStrm >> nLines;
    return new SvxOrphansItem( nLines, Which() );
}

// -----------------------------------------------------------------------

SvStream& SvxOrphansItem::Store( SvStream& rStrm, sal_uInt16 nItemVersion ) const
{
    rStrm << (sal_Int8) GetValue();
    return rStrm;
}

//------------------------------------------------------------------------

SfxItemPresentation SvxOrphansItem::GetPresentation
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
            return SFX_ITEM_PRESENTATION_NONE;

        case SFX_ITEM_PRESENTATION_NAMELESS:
            rText = String::CreateFromInt32( GetValue() );
            rText += SVX_RESSTR(RID_SVXITEMS_LINES);
            return SFX_ITEM_PRESENTATION_NAMELESS;

        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText += SVX_RESSTR(RID_SVXITEMS_ORPHANS_COMPLETE);
            rText += String::CreateFromInt32( GetValue() );
            rText += SVX_RESSTR(RID_SVXITEMS_LINES);
            return SFX_ITEM_PRESENTATION_COMPLETE;
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

sal_Bool SvxOrphansItem::importXML( const OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter )
{
#ifndef SVX_LIGHT
    sal_Int32 nValue;
    sal_Bool bOk = rUnitConverter.convertNumber( nValue, rValue, 0x00, 0xff );
    if( bOk )
        SetValue( nValue  );
    return bOk;
#else
    return sal_False;
#endif
}

sal_Bool SvxOrphansItem::exportXML( OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter ) const
{
#ifndef SVX_LIGHT
    OUStringBuffer aOut;
    rUnitConverter.convertNumber( aOut, (sal_Int32) GetValue() );
    rValue = aOut.makeStringAndClear();
    return sal_True;
#else
    return sal_False;
#endif
}

// class SvxHyphenZoneItem -----------------------------------------------

SvxHyphenZoneItem::SvxHyphenZoneItem( const sal_Bool bHyph, const sal_uInt16 nId ) :
    SfxPoolItem( nId )
{
    bHyphen = bHyph;
    bPageEnd = sal_True;
    nMinLead = nMinTrail = 0;
    nMaxHyphens = 255;
}

// -----------------------------------------------------------------------
sal_Bool    SvxHyphenZoneItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    switch(nMemberId)
    {
        case  MID_IS_HYPHEN:
            rVal = Bool2Any(bHyphen);
        break;
        case MID_HYPHEN_MIN_LEAD:
            rVal <<= (sal_Int16)nMinLead;
        break;
        case MID_HYPHEN_MIN_TRAIL:
            rVal <<= (sal_Int16)nMinTrail;
        break;
        case MID_HYPHEN_MAX_HYPHENS:
            rVal <<= (sal_Int16)nMaxHyphens;
        break;
    }
    return sal_True;
}
// -----------------------------------------------------------------------
sal_Bool SvxHyphenZoneItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    sal_Int16 nNewVal = 0;

    if( nMemberId != MID_IS_HYPHEN )
        if(!(rVal >>= nNewVal))
            return sal_False;

    switch(nMemberId)
    {
        case  MID_IS_HYPHEN:
            bHyphen = Any2Bool(rVal);
        break;
        case MID_HYPHEN_MIN_LEAD:
            nMinLead = nNewVal;
        break;
        case MID_HYPHEN_MIN_TRAIL:
            nMinTrail = nNewVal;
        break;
        case MID_HYPHEN_MAX_HYPHENS:
            nMaxHyphens = nNewVal;
        break;
    }
    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool SvxHyphenZoneItem::importXML( const OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter )
{
    sal_Bool bOk = sal_False;
#ifndef SVX_LIGHT

    sal_Int32 nTemp;
    switch( nMemberId )
    {
    case MID_IS_HYPHEN:
        sal_Bool bNewHyphen;
        bOk = rUnitConverter.convertBool( bNewHyphen, rValue );
        bHyphen = bNewHyphen;
        break;

    case MID_HYPHEN_MIN_LEAD:
        bOk = rUnitConverter.convertNumber( nTemp, rValue );
        nMinLead = nTemp;
        break;

    case MID_HYPHEN_MIN_TRAIL:
        bOk = rUnitConverter.convertNumber( nTemp, rValue );
        nMinTrail = nTemp;
        break;

    case MID_HYPHEN_MAX_HYPHENS:
        if( rValue.compareToAscii( sXML_no_limit ) )
        {
            nMaxHyphens = 0;
            bOk = sal_True;
        }
        else
        {
            bOk = rUnitConverter.convertNumber( nTemp, rValue );
            nMaxHyphens = nTemp;
        }
        break;
    }
#endif
    return bOk;
}

// -----------------------------------------------------------------------


sal_Bool SvxHyphenZoneItem::exportXML( OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter ) const
{
#ifndef SVX_LIGHT
    OUStringBuffer aOut;

    switch( nMemberId )
    {
    case MID_IS_HYPHEN:
        rUnitConverter.convertBool( aOut, bHyphen );
        break;

    case MID_HYPHEN_MIN_LEAD:
        if( !bHyphen )
            return sal_False;
        rUnitConverter.convertNumber( aOut, (sal_Int32) nMinLead );
        break;

    case MID_HYPHEN_MIN_TRAIL:
        if( !bHyphen )
            return sal_False;
        rUnitConverter.convertNumber( aOut, (sal_Int32) nMinTrail );
        break;

    case MID_HYPHEN_MAX_HYPHENS:
        if( !bHyphen )
            return sal_False;
        if( nMaxHyphens == 0 )
            aOut.appendAscii( sXML_no_limit );
        else
            rUnitConverter.convertNumber( aOut, (sal_Int32) nMaxHyphens );
        break;
    }

    rValue = aOut.makeStringAndClear();

    return sal_True;
#else
    return sal_False;
#endif
}

// -----------------------------------------------------------------------

int SvxHyphenZoneItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    return ( (((SvxHyphenZoneItem&)rAttr).bHyphen == bHyphen)
            && (((SvxHyphenZoneItem&)rAttr).bPageEnd == bPageEnd)
            && (((SvxHyphenZoneItem&)rAttr).nMinLead == nMinLead)
            && (((SvxHyphenZoneItem&)rAttr).nMinTrail == nMinTrail)
            && (((SvxHyphenZoneItem&)rAttr).nMaxHyphens == nMaxHyphens) );
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxHyphenZoneItem::Clone( SfxItemPool * ) const
{
    return new SvxHyphenZoneItem( *this );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxHyphenZoneItem::GetPresentation
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
            return SFX_ITEM_PRESENTATION_NONE;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        {
            sal_uInt16 nId = RID_SVXITEMS_HYPHEN_FALSE;

            if ( bHyphen )
                nId = RID_SVXITEMS_HYPHEN_TRUE;
            rText = SVX_RESSTR(nId);
            rText += cpDelim;
            nId = RID_SVXITEMS_PAGE_END_FALSE;

            if ( bPageEnd )
                nId = RID_SVXITEMS_PAGE_END_TRUE;
            rText += SVX_RESSTR(nId);
            rText += cpDelim;
            rText += String::CreateFromInt32( nMinLead );
            rText += cpDelim;
            rText += String::CreateFromInt32( nMinTrail );
            rText += cpDelim;
            rText += String::CreateFromInt32( nMaxHyphens );
            return SFX_ITEM_PRESENTATION_COMPLETE;
        }
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            sal_uInt16 nId = RID_SVXITEMS_HYPHEN_FALSE;

            if ( bHyphen )
                nId = RID_SVXITEMS_HYPHEN_TRUE;
            rText = SVX_RESSTR(nId);
            rText += cpDelim;
            nId = RID_SVXITEMS_PAGE_END_FALSE;

            if ( bPageEnd )
                nId = RID_SVXITEMS_PAGE_END_TRUE;
            rText += SVX_RESSTR(nId);
            rText += cpDelim;
            rText += String::CreateFromInt32(nMinLead);
            rText += SVX_RESSTR(RID_SVXITEMS_HYPHEN_MINLEAD);
            rText += cpDelim;
            rText += String::CreateFromInt32(nMinTrail);
            rText += SVX_RESSTR(RID_SVXITEMS_HYPHEN_MINTRAIL);
            rText += cpDelim;
            rText += String::CreateFromInt32(nMaxHyphens);
            rText += SVX_RESSTR(RID_SVXITEMS_HYPHEN_MAX);
            return SFX_ITEM_PRESENTATION_COMPLETE;
        }
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxHyphenZoneItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_Int8 bHyphen, bHyphenPageEnd;
    sal_Int8 nMinLead, nMinTrail, nMaxHyphens;
    rStrm >> bHyphen >> bHyphenPageEnd >> nMinLead >> nMinTrail >> nMaxHyphens;
    SvxHyphenZoneItem* pAttr = new SvxHyphenZoneItem( sal_False, Which() );
    pAttr->SetHyphen( sal_Bool( bHyphen != 0 ) );
    pAttr->SetPageEnd( sal_Bool( bHyphenPageEnd != 0 ) );
    pAttr->GetMinLead() = nMinLead;
    pAttr->GetMinTrail() = nMinTrail;
    pAttr->GetMaxHyphens() = nMaxHyphens;
    return pAttr;
}

// -----------------------------------------------------------------------

SvStream& SvxHyphenZoneItem::Store( SvStream& rStrm, sal_uInt16 nItemVersion ) const
{
    rStrm << (sal_Int8) IsHyphen()
          << (sal_Int8) IsPageEnd()
          << (sal_Int8) GetMinLead()
          << (sal_Int8) GetMinTrail()
          << (sal_Int8) GetMaxHyphens();
    return rStrm;
}

// class SvxTabStop ------------------------------------------------------

SvxTabStop::SvxTabStop()
{
    nTabPos = 0;
    eAdjustment = SVX_TAB_ADJUST_LEFT;
    cDecimal = GetpApp()->GetAppInternational().GetNumDecimalSep();
    cFill = cDfltFillChar;
}

// -----------------------------------------------------------------------

SvxTabStop::SvxTabStop( const long nPos, const SvxTabAdjust eAdjst,
                        const sal_Unicode cDec, const sal_Unicode cFil )
{
    nTabPos = nPos;
    eAdjustment = eAdjst;

    if ( cDfltDecimalChar == cDec )
        // default aus der International-Klasse besorgen
        cDecimal = GetpApp()->GetAppInternational().GetNumDecimalSep();
    else
        cDecimal = cDec;
    cFill = cFil;
}

// -----------------------------------------------------------------------

XubString SvxTabStop::GetValueString() const
{
    XubString aStr;

    aStr += sal_Unicode( '(' );
    aStr += UniString::CreateFromInt32(nTabPos);
    aStr += cpDelim;
    aStr += XubString( ResId( RID_SVXITEMS_TAB_ADJUST_BEGIN + (sal_uInt16)eAdjustment, DIALOG_MGR() ) );

    aStr += cpDelim;
    aStr += sal_Unicode('[');
    aStr += XubString( ResId( RID_SVXITEMS_TAB_DECIMAL_CHAR, DIALOG_MGR() ) );
    aStr += cDecimal;
    aStr += sal_Unicode(']');
    aStr += cpDelim;
    aStr += cpDelim;
    aStr += sal_Unicode('[');
    aStr += XubString( ResId( RID_SVXITEMS_TAB_FILL_CHAR, DIALOG_MGR() ) );
    aStr += cFill;
    aStr += sal_Unicode(']');
    aStr += sal_Unicode(')');

    return aStr;
}

// class SvxTabStopItem --------------------------------------------------

SvxTabStopItem::SvxTabStopItem( sal_uInt16 nWhich ) :
    SfxPoolItem( nWhich ),
    SvxTabStopArr( (sal_Int8)SVX_TAB_DEFCOUNT )
{
    const sal_uInt16 nTabs = SVX_TAB_DEFCOUNT, nDist = SVX_TAB_DEFDIST;
    const SvxTabAdjust eAdjst= SVX_TAB_ADJUST_DEFAULT;

    if( nTabs )
    {
        SvxTabStop aInitTab( nDist, eAdjst );
        SvxTabStop* pInitArr =
            (SvxTabStop*)new char[ sizeof(SvxTabStop) * nTabs ];
        for( sal_uInt16 i = 0; i < nTabs; ++i )
        {
            *( pInitArr + i ) = aInitTab;
            aInitTab.GetTabPos() += nDist;
        }
        SvxTabStopArr::InitData( pInitArr, nTabs );
    }
}

// -----------------------------------------------------------------------

SvxTabStopItem::SvxTabStopItem( const sal_uInt16 nTabs,
                                const sal_uInt16 nDist,
                                const SvxTabAdjust eAdjst,
                                sal_uInt16 nWhich ) :
    SfxPoolItem( nWhich ),
    SvxTabStopArr( (sal_Int8)nTabs )
{
    if( nTabs )
    {
        SvxTabStop aInitTab( nDist, eAdjst );
        SvxTabStop* pInitArr =
            (SvxTabStop*)new char[ sizeof(SvxTabStop) * nTabs ];
        for( sal_uInt16 i = 0; i < nTabs; ++i )
        {
            *( pInitArr + i ) = aInitTab;
            aInitTab.GetTabPos() += nDist;
        }
        SvxTabStopArr::InitData( pInitArr, nTabs );
    }
}

// -----------------------------------------------------------------------

SvxTabStopItem::SvxTabStopItem( const SvxTabStopItem& rTSI ) :
    SfxPoolItem( rTSI.Which() ),
    SvxTabStopArr( (sal_Int8)rTSI.Count() )
{
    SvxTabStopArr::Insert( &rTSI );
}

// -----------------------------------------------------------------------

sal_uInt16 SvxTabStopItem::GetPos( const SvxTabStop& rTab ) const
{
    sal_uInt16 nFound;
    return Seek_Entry( rTab, &nFound ) ? nFound : SVX_TAB_NOTFOUND;
}

// -----------------------------------------------------------------------

sal_uInt16 SvxTabStopItem::GetPos( const long nPos ) const
{
    sal_uInt16 nFound;
    return Seek_Entry( SvxTabStop( nPos ), &nFound ) ? nFound : SVX_TAB_NOTFOUND;
}

// -----------------------------------------------------------------------

SvxTabStopItem& SvxTabStopItem::operator=( const SvxTabStopItem& rTSI )
{
    Remove( 0, Count() );
    SvxTabStopArr::Insert( &rTSI );
    return *this;
}


/*
 enum ::com::sun::star::style::TabAlign
{
    TABALIGN_LEFT,
    TABALIGN_CENTER,
    TABALIGN_RIGHT,
    TABALIGN_DECIMAL
};

struct   ::com::sun::star::style::TabStop
{
    long            Position;
 ::com::sun::star::style::TabAlign   ::com::sun::star::drawing::Alignment;
    unsigned short  DecimalChar;
    unsigned short  FillChar;
};
typedef sequence ::com::sun::star::style::TabStop> TabSTopSequence;

 */
/*-----------------19.03.98 08:50-------------------

--------------------------------------------------*/

sal_Bool SvxTabStopItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    switch (nMemberId & ~CONVERT_TWIPS)
    {
        case MID_TABSTOPS:
        {
            sal_uInt16 nCount = Count();
            uno::Sequence< style::TabStop> aSeq(nCount);
            style::TabStop* pArr = aSeq.getArray();
            for(sal_uInt16 i = 0; i < nCount; i++)
            {
                const SvxTabStop& rTab = *(GetStart() + i);
                pArr[i].Position        = nMemberId&CONVERT_TWIPS ? TWIP_TO_MM100(rTab.GetTabPos()) : rTab.GetTabPos();
                switch(rTab.GetAdjustment())
                {
                case  SVX_TAB_ADJUST_LEFT   : pArr[i].Alignment = style::TabAlign_LEFT; break;
                case  SVX_TAB_ADJUST_RIGHT  : pArr[i].Alignment = style::TabAlign_RIGHT; break;
                case  SVX_TAB_ADJUST_DECIMAL: pArr[i].Alignment = style::TabAlign_DECIMAL; break;
                case  SVX_TAB_ADJUST_CENTER : pArr[i].Alignment = style::TabAlign_CENTER; break;
                    default: //SVX_TAB_ADJUST_DEFAULT
                        pArr[i].Alignment = style::TabAlign_DEFAULT;

                }
                pArr[i].DecimalChar     = rTab.GetDecimal();
                pArr[i].FillChar        = rTab.GetFill();
            }
            rVal <<= aSeq;
            break;
        }
        case MID_STD_TAB:
        {
            const SvxTabStop &rTab = *(GetStart());
            rVal <<= nMemberId & CONVERT_TWIPS ? TWIP_TO_MM100(rTab.GetTabPos()) : rTab.GetTabPos();
            break;
        }
    }
    return sal_True;
}
/*-----------------19.03.98 08:50-------------------

--------------------------------------------------*/

sal_Bool SvxTabStopItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    switch ( nMemberId )
    {
        case MID_TABSTOPS:
        {
            uno::Sequence< style::TabStop> aSeq;
            if(!(rVal >>= aSeq))
                return sal_False;

            SvxTabStopArr::Remove( 0, Count() );
            const style::TabStop* pArr = aSeq.getConstArray();
            const sal_uInt16 nCount = (sal_uInt16)aSeq.getLength();
            for(sal_uInt16 i = 0; i < nCount ; i++)
            {
                SvxTabAdjust eAdjust = SVX_TAB_ADJUST_DEFAULT;
                switch(pArr[i].Alignment)
                {
                case style::TabAlign_LEFT   : eAdjust = SVX_TAB_ADJUST_LEFT; break;
                case style::TabAlign_CENTER : eAdjust = SVX_TAB_ADJUST_CENTER; break;
                case style::TabAlign_RIGHT  : eAdjust = SVX_TAB_ADJUST_RIGHT; break;
                case style::TabAlign_DECIMAL: eAdjust = SVX_TAB_ADJUST_DECIMAL; break;
                }
                sal_Unicode cFill = pArr[i].FillChar;
                sal_Unicode cDecimal = pArr[i].DecimalChar;
;
                SvxTabStop aTab( nMemberId&CONVERT_TWIPS ? MM100_TO_TWIP(pArr[i].Position) : pArr[i].Position,
                                    eAdjust,
                                    cDecimal,
                                    cFill );
                Insert(aTab);
            }
            break;
        }
        case MID_STD_TAB:
        {
            sal_Int32 nNewPos;
            if (!(rVal >>= nNewPos) )
                return sal_False;
            const SvxTabStop& rTab = *(GetStart());
            SvxTabStop aNewTab ( nMemberId&CONVERT_TWIPS ? MM100_TO_TWIP ( nNewPos ) : nNewPos,
                                 rTab.GetAdjustment(), rTab.GetDecimal(), rTab.GetFill() );
            Remove ( 0 );
            Insert( aNewTab );
            break;
        }
    }
    return sal_True;
}
// -----------------------------------------------------------------------

int SvxTabStopItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    const SvxTabStopItem& rTSI = (SvxTabStopItem&)rAttr;

    if ( Count() != rTSI.Count() )
        return 0;

    for ( sal_uInt16 i = 0; i < Count(); ++i )
        if( !(*this)[i].IsEqual( rTSI[i] ) )
            return 0;
    return 1;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxTabStopItem::Clone( SfxItemPool * ) const
{
    return new SvxTabStopItem( *this );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxTabStopItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
    rText.Erase();

    if ( ePres > SFX_ITEM_PRESENTATION_NONE )
    {
#ifndef SVX_LIGHT
        sal_Bool bComma = sal_False;

        for ( sal_uInt16 i = 0; i < Count(); ++i )
        {
            if ( SVX_TAB_ADJUST_DEFAULT != ((*this)[i]).GetAdjustment() )
            {
                if ( bComma )
                    rText += sal_Unicode(',');
                rText += GetMetricText(
                    (long)((*this)[i]).GetTabPos(), eCoreUnit, ePresUnit );
                if ( SFX_ITEM_PRESENTATION_COMPLETE == ePres )
                    rText += SVX_RESSTR(GetMetricId(ePresUnit));
                bComma = sal_True;
            }
        }
#endif
    }
    return ePres;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxTabStopItem::Create( SvStream& rStrm, sal_uInt16 ) const
{
    sal_Int8 nTabs;
    rStrm >> nTabs;
    SvxTabStopItem* pAttr =
        new SvxTabStopItem( 0, 0, SVX_TAB_ADJUST_DEFAULT, Which() );

    for ( sal_Int8 i = 0; i < nTabs; i++ )
    {
        long nPos;
        sal_Int8 eAdjust;
        unsigned char cDecimal, cFill;
        rStrm >> nPos >> eAdjust >> cDecimal >> cFill;
        if( !i || SVX_TAB_ADJUST_DEFAULT != eAdjust )
            pAttr->Insert( SvxTabStop
                ( nPos, (SvxTabAdjust)eAdjust, sal_Unicode(cDecimal), sal_Unicode(cFill) ) );
    }
    return pAttr;
}

// -----------------------------------------------------------------------

SvStream& SvxTabStopItem::Store( SvStream& rStrm, sal_uInt16 nItemVersion ) const
{
    //MA 05. Sep. 96: Default-Tabs werden nur noch fuer das default-Attr
    //expandiert. Fuer vollstaendige Rueckwaertskompatibilitaet (<=304)
    //muessten alle Tabs expandiert werden, dass blaeht aber das File u.U.
    //enorm auf.
    //Alles nur SWG!

    const SfxItemPool *pPool = SfxItemPool::GetStoringPool();
    const FASTBOOL bStoreDefTabs = pPool
        && pPool->GetName().EqualsAscii("SWG")
        && ::IsDefaultItem( this );

    const short nTabs = Count();
    sal_uInt16  nCount = 0, nDefDist;
    long nNew;

    if( bStoreDefTabs )
    {
        const SvxTabStopItem& rDefTab = (const SvxTabStopItem &)
            pPool->GetDefaultItem( pPool->GetWhich( SID_ATTR_TABSTOP, sal_False ) );
        nDefDist = sal_uInt16( rDefTab.GetStart()->GetTabPos() );
        const long nPos = nTabs > 0 ? (*this)[nTabs-1].GetTabPos() : 0;
        nCount  = (sal_uInt16)(nPos / nDefDist);
        nNew    = (nCount + 1) * nDefDist;

        if( nNew <= nPos + 50 )
            nNew += nDefDist;

        nCount = nNew < lA3Width ? ( lA3Width - nNew ) / nDefDist + 1 : 0;
    }

    rStrm << (sal_Int8) ( nTabs + nCount );
    for ( short i = 0; i < nTabs; i++ )
    {
        const SvxTabStop& rTab = (*this)[ i ];
        rStrm << (long) rTab.GetTabPos()
              << (sal_Int8) rTab.GetAdjustment()
              << (unsigned char) rTab.GetDecimal()
              << (unsigned char) rTab.GetFill();
    }

    if ( bStoreDefTabs )
        for( ; nCount; --nCount )
        {
            SvxTabStop aSwTabStop(nNew, SVX_TAB_ADJUST_DEFAULT);
            rStrm << (long) aSwTabStop.GetTabPos()
                  << (sal_Int8) aSwTabStop.GetAdjustment()
                  << (unsigned char) aSwTabStop.GetDecimal()
                  << (unsigned char) aSwTabStop.GetFill();
            nNew += nDefDist;
        }

    return rStrm;
}

// -----------------------------------------------------------------------
sal_Bool SvxTabStopItem::Insert( const SvxTabStop& rTab )
{
    sal_uInt16 nTabPos = GetPos(rTab);
    if(SVX_TAB_NOTFOUND != nTabPos )
        Remove(nTabPos);
    return SvxTabStopArr::Insert( rTab );
}
// -----------------------------------------------------------------------
void SvxTabStopItem::Insert( const SvxTabStopItem* pTabs, sal_uInt16 nStart,
                            sal_uInt16 nEnd )
{
    for( sal_uInt16 i = nStart; i < nEnd && i < pTabs->Count(); i++ )
    {
        const SvxTabStop& rTab = (*pTabs)[i];
        sal_uInt16 nTabPos = GetPos(rTab);
        if(SVX_TAB_NOTFOUND != nTabPos)
            Remove(nTabPos);
    }
    SvxTabStopArr::Insert( pTabs, nStart, nEnd );
}



// class SvxFmtSplitItem -------------------------------------------------
SvxFmtSplitItem::~SvxFmtSplitItem()
{
}
// -----------------------------------------------------------------------
SfxPoolItem* SvxFmtSplitItem::Clone( SfxItemPool * ) const
{
    return new SvxFmtSplitItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxFmtSplitItem::Store( SvStream& rStrm, sal_uInt16 nItemVersion ) const
{
    rStrm << (sal_Int8)GetValue();
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxFmtSplitItem::Create( SvStream& rStrm, sal_uInt16 ) const
{
    sal_Int8 bIsSplit;
    rStrm >> bIsSplit;
    return new SvxFmtSplitItem( sal_Bool( bIsSplit != 0 ), Which() );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxFmtSplitItem::GetPresentation
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
            return SFX_ITEM_PRESENTATION_NONE;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            sal_uInt16 nId = RID_SVXITEMS_FMTSPLIT_FALSE;

            if ( GetValue() )
                nId = RID_SVXITEMS_FMTSPLIT_TRUE;
            rText = SVX_RESSTR(nId);
            return ePres;
        }
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// --------------------------------------------------------------------

sal_Bool SvxFmtSplitItem::importXML( const OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter )
{
#ifndef SVX_LIGHT
    SetValue( rValue.compareToAscii(sXML_columnsplit_auto ) );
    return GetValue() || rValue.compareToAscii(sXML_columnsplit_avoid );
#else
    return sal_False;
#endif
}

sal_Bool SvxFmtSplitItem::exportXML( OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter ) const
{
#ifndef SVX_LIGHT
    if(GetValue())
    {
        rValue = OUString(sXML_columnsplit_auto, sizeof(sXML_columnsplit_auto),
            gsl_getSystemTextEncoding());
    }
    else
    {
        rValue = OUString(sXML_columnsplit_avoid, sizeof(sXML_columnsplit_avoid),
            gsl_getSystemTextEncoding());
    }
    return sal_True;
#else
    return sal_False;
#endif
}

// --------------------------------------------------------------------

SfxPoolItem* SvxPageModelItem::Clone( SfxItemPool* ) const
{
    return new SvxPageModelItem( *this );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxPageModelItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
    rText.Erase();
    FASTBOOL bSet = ( GetValue().Len() > 0 );

    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            return SFX_ITEM_PRESENTATION_NONE;

        case SFX_ITEM_PRESENTATION_NAMELESS:
            if ( bSet )
                rText = GetValue();
            return SFX_ITEM_PRESENTATION_NAMELESS;

        case SFX_ITEM_PRESENTATION_COMPLETE:
            if ( bSet )
            {
                rText = SVX_RESSTR(RID_SVXITEMS_PAGEMODEL_COMPLETE);
                rText += GetValue();
            }
            return SFX_ITEM_PRESENTATION_COMPLETE;
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

//------------------------------------------------------------------------

SvxScriptSpaceItem::SvxScriptSpaceItem( sal_Bool bOn, const sal_uInt16 nId )
    : SfxBoolItem( nId, bOn )
{
}

SfxPoolItem* SvxScriptSpaceItem::Clone( SfxItemPool *pPool ) const
{
    return new SvxScriptSpaceItem( GetValue(), Which() );
}

SfxPoolItem* SvxScriptSpaceItem::Create(SvStream & rStrm, USHORT) const
{
    sal_Bool bFlag;
    rStrm >> bFlag;
    return new SvxScriptSpaceItem( bFlag, Which() );
}

USHORT  SvxScriptSpaceItem::GetVersion( USHORT nFFVer ) const
{
    DBG_ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
            SOFFICE_FILEFORMAT_40==nFFVer ||
            SOFFICE_FILEFORMAT_50==nFFVer,
            "SvxTwoLinesItem: Gibt es ein neues Fileformat?" );

    return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
}

SfxItemPresentation SvxScriptSpaceItem::GetPresentation(
        SfxItemPresentation ePres,
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
            rText = SVX_RESSTR( !GetValue()
                                    ? RID_SVXITEMS_SCRPTSPC_OFF
                                    : RID_SVXITEMS_SCRPTSPC_ON );
            return ePres;
        }
        break;
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

//------------------------------------------------------------------------

SvxHangingPunctuationItem::SvxHangingPunctuationItem(
                                    sal_Bool bOn, const sal_uInt16 nId )
    : SfxBoolItem( nId, bOn )
{
}

SfxPoolItem* SvxHangingPunctuationItem::Clone( SfxItemPool *pPool ) const
{
    return new SvxHangingPunctuationItem( GetValue(), Which() );
}

SfxPoolItem* SvxHangingPunctuationItem::Create(SvStream & rStrm, USHORT) const
{
    sal_Bool nValue;
    rStrm >> nValue;
    return new SvxHangingPunctuationItem( nValue, Which() );
}

USHORT SvxHangingPunctuationItem::GetVersion( USHORT nFFVer ) const
{
    DBG_ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
            SOFFICE_FILEFORMAT_40==nFFVer ||
            SOFFICE_FILEFORMAT_50==nFFVer,
            "SvxHangingPunctuationItem: Gibt es ein neues Fileformat?" );

    return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
}

SfxItemPresentation SvxHangingPunctuationItem::GetPresentation(
        SfxItemPresentation ePres,
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
            rText = SVX_RESSTR( !GetValue()
                                    ? RID_SVXITEMS_HNGPNCT_OFF
                                    : RID_SVXITEMS_HNGPNCT_ON );
            return ePres;
        }
        break;
    }
    return SFX_ITEM_PRESENTATION_NONE;
}
//------------------------------------------------------------------------

SvxForbiddenRuleItem::SvxForbiddenRuleItem(
                                    sal_Bool bOn, const sal_uInt16 nId )
    : SfxBoolItem( nId, bOn )
{
}
/* -----------------------------29.11.00 11:23--------------------------------

 ---------------------------------------------------------------------------*/
SfxPoolItem* SvxForbiddenRuleItem::Clone( SfxItemPool *pPool ) const
{
    return new SvxForbiddenRuleItem( GetValue(), Which() );
}
/* -----------------------------29.11.00 11:23--------------------------------

 ---------------------------------------------------------------------------*/
SfxPoolItem* SvxForbiddenRuleItem::Create(SvStream & rStrm, USHORT) const
{
    sal_Bool nValue;
    rStrm >> nValue;
    return new SvxForbiddenRuleItem( nValue, Which() );
}
/* -----------------------------29.11.00 11:23--------------------------------

 ---------------------------------------------------------------------------*/
USHORT SvxForbiddenRuleItem::GetVersion( USHORT nFFVer ) const
{
    DBG_ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
            SOFFICE_FILEFORMAT_40==nFFVer ||
            SOFFICE_FILEFORMAT_50==nFFVer,
            "SvxForbiddenRuleItem: Gibt es ein neues Fileformat?" );

    return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
}
/* -----------------------------29.11.00 11:23--------------------------------

 ---------------------------------------------------------------------------*/
SfxItemPresentation SvxForbiddenRuleItem::GetPresentation(
        SfxItemPresentation ePres,
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
            rText = SVX_RESSTR( !GetValue()
                                    ? RID_SVXITEMS_FORBIDDEN_RULE_OFF
                                    : RID_SVXITEMS_FORBIDDEN_RULE_ON );
            return ePres;
        }
        break;
    }
    return SFX_ITEM_PRESENTATION_NONE;
}



