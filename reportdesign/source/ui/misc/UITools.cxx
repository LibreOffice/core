/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: UITools.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2008-02-12 13:10:54 $
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
#include "precompiled_reportdesign.hxx"

//Erstmal definieren, damit die Klassendeklarionen angezogen werden.
#define ITEMID_FONT             10
#define ITEMID_POSTURE          11
#define ITEMID_WEIGHT           12
#define ITEMID_SHADOWED         13
#define ITEMID_WORDLINEMODE     14
#define ITEMID_CONTOUR          15
#define ITEMID_CROSSEDOUT       16
#define ITEMID_UNDERLINE        17
#define ITEMID_FONTHEIGHT       18
#define ITEMID_PROPSIZE         19
#define ITEMID_COLOR            20
#define ITEMID_KERNING          21
#define ITEMID_CASEMAP          22
#define ITEMID_LANGUAGE         23
#define ITEMID_ESCAPEMENT       24
#define ITEMID_FONTLIST         25
#define ITEMID_AUTOKERN         26
#define ITEMID_COLOR_TABLE      27
#define ITEMID_BLINK            28
#define ITEMID_EMPHASISMARK     29
#define ITEMID_TWOLINES         30
#define ITEMID_CHARROTATE       31
#define ITEMID_CHARRELIEF       32
#define ITEMID_CHARHIDDEN       33
#define ITEMID_CHARSCALE_W      34
#define ITEMID_BRUSH            35
#define ITEMID_HORJUSTIFY       36
#define ITEMID_VERJUSTIFY       37
//#define ITEMID_IDENT          38
//#define ITEMID_DEGREES        39
//#define ITEMID_CHARROTATE     SID_ATTR_CHAR_ROTATED

#include <svx/charscaleitem.hxx>
#include <svx/algitem.hxx>
#include <svx/svdpagv.hxx>
#include <toolkit/helper/convert.hxx>
#include "SectionView.hxx"
#ifndef RPTUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif
#ifndef _CTRLTOOL_HXX
#include <svtools/ctrltool.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_NULLPOINTEREXCEPTION_HPP_
#include <com/sun/star/lang/NullPointerException.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif
#include <svx/xtable.hxx>       // XColorTable
#include <svx/brshitem.hxx>
#include <svx/fontitem.hxx>
#include <svx/emphitem.hxx>
#include <svx/postitem.hxx>
#include <svx/udlnitem.hxx>
#include <svx/crsditem.hxx>
#include <svx/cntritem.hxx>
#include <svx/langitem.hxx>
#include <svx/wghtitem.hxx>
#include <svx/fhgtitem.hxx>
#include <svx/shdditem.hxx>
#include <svx/escpitem.hxx>
#include <svx/prszitem.hxx>
#include <svx/wrlmitem.hxx>
#include <svx/cmapitem.hxx>
#include <svx/kernitem.hxx>
#include <svx/blnkitem.hxx>
#include <svx/flstitem.hxx>
#include <svx/akrnitem.hxx>
#include <svx/colritem.hxx>
#include <svx/drawitem.hxx>
#include <svx/twolinesitem.hxx>
#include <svx/charreliefitem.hxx>
#include <svx/charrotateitem.hxx>
#include <svx/charhiddenitem.hxx>
#include <svx/svditer.hxx>
#include <svx/xtable.hxx>
#include "RptObject.hxx"
#include <svx/svdview.hxx>
#include <svx/svdpage.hxx>
#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif
#ifndef _SFXITEMSET_HXX //autogen wg. SfxItemSet
#include <svtools/itemset.hxx>
#endif
#ifndef _RPTUI_SLOTID_HRC_
#include "rptui_slotid.hrc"
#endif
#ifndef REPORTDESIGN_SHARED_UISTRINGS_HRC
#include "uistrings.hrc"
#endif
#ifndef _COMPHELPER_PROPERTY_MULTIPLEX_HXX_
#include <comphelper/propmultiplex.hxx>
#endif
#ifndef COMPHELPER_NAMEDVALUECOLLECTION_HXX
#include <comphelper/namedvaluecollection.hxx>
#endif
#include <com/sun/star/report/XGroups.hpp>
#include <com/sun/star/awt/TextAlign.hpp>
#include <com/sun/star/text/ParagraphVertAlign.hpp>
#include <com/sun/star/report/XShape.hpp>
#include <com/sun/star/sdb/XParametersSupplier.hpp>
#include <i18npool/mslangid.hxx>
#ifndef RPTUI_DLGPAGE_HXX
#include "dlgpage.hxx"
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _RPTUI_MODULE_HELPER_RPT_HXX_
#include "ModuleHelper.hxx"
#endif
#ifndef _REPORT_RPTUIDEF_HXX
#include "RptDef.hxx"
#endif
#ifndef _RPTUI_DLGRESID_HRC
#include "RptResId.hrc"
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

namespace rptui
{
using namespace ::com::sun::star;
// -----------------------------------------------------------------------------
SvxCellHorJustify lcl_MapHorizontalAlignment(const sal_Int16 _nAlign)
{
    SvxCellHorJustify eRet = SVX_HOR_JUSTIFY_STANDARD;
    switch(_nAlign)
    {
        case awt::TextAlign::LEFT:
            eRet = SVX_HOR_JUSTIFY_LEFT;
            break;
        case awt::TextAlign::CENTER:
            eRet = SVX_HOR_JUSTIFY_CENTER;
            break;
        case awt::TextAlign::RIGHT:
            eRet = SVX_HOR_JUSTIFY_RIGHT;
            break;
        default:
            OSL_ENSURE(0,"Illegal text alignment value!");
            break;
    }
    return eRet;
}
// -----------------------------------------------------------------------------
sal_Int16 lcl_MapHorizontalAlignment(const SvxCellHorJustify _eAlign)
{
    sal_Int16 nRet = awt::TextAlign::LEFT;
    switch(_eAlign)
    {
        case SVX_HOR_JUSTIFY_LEFT:
            nRet = awt::TextAlign::LEFT;
            break;
        case SVX_HOR_JUSTIFY_CENTER:
            nRet = awt::TextAlign::CENTER;
            break;
        case SVX_HOR_JUSTIFY_RIGHT:
            nRet = awt::TextAlign::RIGHT;
            break;
        case SVX_HOR_JUSTIFY_BLOCK:
        case SVX_HOR_JUSTIFY_REPEAT:
            nRet = awt::TextAlign::CENTER;
            break;
        default:
            OSL_ENSURE(0,"Illegal text alignment value!");
            break;
    }
    return nRet;
}
// -----------------------------------------------------------------------------
SvxCellVerJustify lcl_MapVerticalAlignment(const sal_Int16 _nAlign)
{
    SvxCellVerJustify eRet = SVX_VER_JUSTIFY_STANDARD;
    switch(_nAlign)
    {
        case text::ParagraphVertAlign::AUTOMATIC:
        case text::ParagraphVertAlign::BASELINE:
            eRet = SVX_VER_JUSTIFY_STANDARD;
            break;
        case text::ParagraphVertAlign::TOP:
            eRet = SVX_VER_JUSTIFY_TOP;
            break;
        case text::ParagraphVertAlign::CENTER:
            eRet = SVX_VER_JUSTIFY_CENTER;
            break;
        case text::ParagraphVertAlign::BOTTOM:
            eRet = SVX_VER_JUSTIFY_BOTTOM;
            break;
        default:
            OSL_ENSURE(0,"Illegal text alignment value!");
            break;
    }
    return eRet;
}
// -----------------------------------------------------------------------------
sal_Int16 lcl_MapVerticalAlignment(const SvxCellVerJustify _eAlign)
{
    sal_Int16 nRet = text::ParagraphVertAlign::TOP;
    switch(_eAlign)
    {
        case SVX_VER_JUSTIFY_STANDARD:
            nRet = text::ParagraphVertAlign::AUTOMATIC;
            break;
        case SVX_VER_JUSTIFY_TOP:
            nRet = text::ParagraphVertAlign::TOP;
            break;
        case SVX_VER_JUSTIFY_CENTER:
            nRet = text::ParagraphVertAlign::CENTER;
            break;
        case SVX_VER_JUSTIFY_BOTTOM:
            nRet = text::ParagraphVertAlign::BOTTOM;
            break;
        default:
            OSL_ENSURE(0,"Illegal text alignment value!");
            break;
    }
    return nRet;
}
// -----------------------------------------------------------------------------
sal_Int32 getGroupPosition(const uno::Reference< report::XReportDefinition >& _xReportDefinition,const uno::Reference< report::XGroup >& _xGroup)
{
    OSL_ENSURE(_xReportDefinition.is(),"No report definition -> GPF!");
    uno::Reference< report::XGroups > xGroups = _xReportDefinition->getGroups();
    const sal_Int32 nCount = xGroups->getCount();
    sal_Int32 i = 0;
    for (;i<nCount ; ++i)
    {
        uno::Reference< report::XGroup > xGroup(xGroups->getByIndex(i),uno::UNO_QUERY);
        if ( xGroup == _xGroup )
            break;
    } // for (;i<nCount ; ++i)
    return i;
}
// -----------------------------------------------------------------------------
void adjustSectionName(const uno::Reference< report::XGroup >& _xGroup,sal_Int32 _nPos)
{
    OSL_ENSURE(_xGroup.is(),"Group is NULL -> GPF");
    if ( _xGroup->getHeaderOn() && !_xGroup->getHeader()->getName().getLength() )
    {
        ::rtl::OUString sName = String(ModuleRes(RID_STR_GROUPHEADER));
        sName += ::rtl::OUString::valueOf(_nPos);
        _xGroup->getHeader()->setName(sName);
    } // if ( _xGroup->getHeaderOn() )

    if ( _xGroup->getFooterOn() && !_xGroup->getFooter()->getName().getLength()  )
    {
        ::rtl::OUString sName = String(ModuleRes(RID_STR_GROUPFOOTER));
        sName += ::rtl::OUString::valueOf(_nPos);
        _xGroup->getFooter()->setName(sName);
    } // if ( _xGroup->getHeaderOn() )
}
// -----------------------------------------------------------------------------
::rtl::Reference< comphelper::OPropertyChangeMultiplexer> addStyleListener(const uno::Reference< report::XReportDefinition >& _xReportDefinition,::comphelper::OPropertyChangeListener* _pListener)
{
    ::rtl::Reference< comphelper::OPropertyChangeMultiplexer> pRet = NULL;
    if ( _xReportDefinition.is() )
    {
        uno::Reference<beans::XPropertySet> xPageStyle(getUsedStyle(_xReportDefinition),uno::UNO_QUERY);
        if ( xPageStyle.is() )
        {
            pRet = new comphelper::OPropertyChangeMultiplexer(_pListener,xPageStyle);
            pRet->addProperty(PROPERTY_LEFTMARGIN);
            pRet->addProperty(PROPERTY_RIGHTMARGIN);
            pRet->addProperty(PROPERTY_PAPERSIZE);
        }
    }
    return pRet;
}

// -----------------------------------------------------------------------------
namespace
{
    // -------------------------------------------------------------------------
    Font lcl_getReportControlFont( const uno::Reference<report::XReportControlFormat >& _rxReportControlFormat, awt::FontDescriptor& _out_rControlFont )
    {
        if ( !_rxReportControlFormat.is() )
            throw uno::RuntimeException();

        _out_rControlFont = _rxReportControlFormat->getFontDescriptor();
        Font aDefaultFont = Application::GetDefaultDevice()->GetSettings().GetStyleSettings().GetAppFont();
        return VCLUnoHelper::CreateFont( _out_rControlFont, aDefaultFont );
    }

    // -------------------------------------------------------------------------
    Font lcl_getReportControlFont( const uno::Reference<report::XReportControlFormat >& _rxReportControlFormat )
    {
        awt::FontDescriptor aAwtFont;
        return lcl_getReportControlFont( _rxReportControlFormat, aAwtFont );
    }

    // -------------------------------------------------------------------------
    void lcl_CharPropertiesToItems( const uno::Reference<report::XReportControlFormat >& _rxReportControlFormat,
        SfxItemSet& _rItemSet )
    {
        if ( !_rxReportControlFormat.is() )
            throw lang::NullPointerException();

        // fill it
        awt::FontDescriptor aControlFont;
        const Font aFont( lcl_getReportControlFont( _rxReportControlFormat, aControlFont ) );
        SvxFontItem aFontItem(ITEMID_FONT);
        aFontItem.PutValue( uno::makeAny( aControlFont ) );
        _rItemSet.Put(aFontItem);
        _rItemSet.Put(SvxPostureItem(aFont.GetItalic(),ITEMID_POSTURE));
        _rItemSet.Put(SvxWeightItem(aFont.GetWeight(),ITEMID_WEIGHT));
        _rItemSet.Put(SvxShadowedItem(_rxReportControlFormat->getCharShadowed(),ITEMID_SHADOWED));
        _rItemSet.Put(SvxWordLineModeItem(aFont.IsWordLineMode(),ITEMID_WORDLINEMODE));
        _rItemSet.Put(SvxContourItem(_rxReportControlFormat->getCharContoured(),ITEMID_CONTOUR));
        _rItemSet.Put(SvxAutoKernItem(_rxReportControlFormat->getCharAutoKerning(),ITEMID_AUTOKERN));
        _rItemSet.Put(SvxCrossedOutItem(aFont.GetStrikeout(),ITEMID_CROSSEDOUT));
        _rItemSet.Put(SvxCaseMapItem(static_cast<SvxCaseMap>(_rxReportControlFormat->getCharCaseMap()),ITEMID_CASEMAP));
        _rItemSet.Put(SvxLanguageItem(MsLangId::convertLocaleToLanguage(_rxReportControlFormat->getCharLocale()),ITEMID_LANGUAGE));
        _rItemSet.Put(SvxEscapementItem(_rxReportControlFormat->getCharEscapement(),_rxReportControlFormat->getCharEscapementHeight(),ITEMID_ESCAPEMENT));
        _rItemSet.Put(SvxBlinkItem(_rxReportControlFormat->getCharFlash(),ITEMID_BLINK));
        _rItemSet.Put(SvxCharHiddenItem(_rxReportControlFormat->getCharHidden(),ITEMID_CHARHIDDEN));
        _rItemSet.Put(SvxTwoLinesItem(_rxReportControlFormat->getCharCombineIsOn(),_rxReportControlFormat->getCharCombinePrefix().toChar(),_rxReportControlFormat->getCharCombineSuffix().toChar(),ITEMID_TWOLINES));
        SvxUnderlineItem aUnderLineItem(aFont.GetUnderline(),ITEMID_UNDERLINE);
        aUnderLineItem.SetColor(_rxReportControlFormat->getCharUnderlineColor());
        _rItemSet.Put(aUnderLineItem);
        _rItemSet.Put(SvxFontHeightItem(OutputDevice::LogicToLogic(Size(0, (sal_Int32)aFont.GetHeight()), MAP_POINT, MAP_TWIP).Height(),100,ITEMID_FONTHEIGHT));
        _rItemSet.Put(SvxKerningItem(_rxReportControlFormat->getCharKerning(),ITEMID_KERNING));
        _rItemSet.Put(SvxEmphasisMarkItem(static_cast<FontEmphasisMark>(_rxReportControlFormat->getCharEmphasis()),ITEMID_EMPHASISMARK));
        //_rItemSet.Put(SvxTwoLinesItem());
        _rItemSet.Put(SvxCharReliefItem(static_cast<FontRelief>(_rxReportControlFormat->getCharRelief()),ITEMID_CHARRELIEF));
        _rItemSet.Put(SvxColorItem(::Color(_rxReportControlFormat->getCharColor()),ITEMID_COLOR));
        _rItemSet.Put(SvxCharRotateItem(_rxReportControlFormat->getCharRotation(),sal_False,ITEMID_CHARROTATE));
        _rItemSet.Put(SvxCharScaleWidthItem(_rxReportControlFormat->getCharScaleWidth(),ITEMID_CHARSCALE_W));

        _rItemSet.Put(SvxHorJustifyItem(lcl_MapHorizontalAlignment(_rxReportControlFormat->getParaAdjust()),ITEMID_HORJUSTIFY));
        //_rItemSet.Put(SfxInt32Item(ITEMID_DEGREES,_rxReportControlFormat->getCharRotation()));
        _rItemSet.Put(SvxVerJustifyItem(lcl_MapVerticalAlignment(_rxReportControlFormat->getParaVertAlignment()),ITEMID_VERJUSTIFY));
        //_rItemSet.Put(SfxInt32Item(ITEMID_IDENT,_rxReportControlFormat->getCharRotation()));

        uno::Reference< report::XShape> xShape(_rxReportControlFormat,uno::UNO_QUERY);
        if ( !xShape.is() )
            _rItemSet.Put(SvxBrushItem(::Color(_rxReportControlFormat->getControlBackground()),ITEMID_BRUSH));
    }

    // -------------------------------------------------------------------------
    void lcl_pushBack( uno::Sequence< beans::NamedValue >& _out_rProperties, const sal_Char* _pAsciiName, const uno::Any& _rValue )
    {
        sal_Int32 nLen( _out_rProperties.getLength() );
        _out_rProperties.realloc( nLen + 1 );
        _out_rProperties[ nLen ] = beans::NamedValue( ::rtl::OUString::createFromAscii( _pAsciiName ), _rValue );
    }

    // -------------------------------------------------------------------------
    void lcl_initAwtFont( const Font& _rOriginalFont, const SfxItemSet& _rItemSet, awt::FontDescriptor& _out_rAwtFont )
    {
        Font aNewFont( _rOriginalFont );
        const SfxPoolItem* pItem( NULL );
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_FONT,sal_True,&pItem) && pItem->ISA(SvxFontItem))
        {
            const SvxFontItem* pFontItem = static_cast<const SvxFontItem*>(pItem);
            aNewFont.SetName( pFontItem->GetFamilyName());
            aNewFont.SetStyleName(pFontItem->GetStyleName());
            aNewFont.SetFamily(pFontItem->GetFamily());
            aNewFont.SetPitch(pFontItem->GetPitch());
            aNewFont.SetCharSet(pFontItem->GetCharSet());
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_POSTURE,sal_True,&pItem) && pItem->ISA(SvxPostureItem))
        {
            const SvxPostureItem* pFontItem = static_cast<const SvxPostureItem*>(pItem);
            aNewFont.SetItalic(pFontItem->GetPosture());
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_WEIGHT,sal_True,&pItem) && pItem->ISA(SvxWeightItem))
        {
            const SvxWeightItem* pFontItem = static_cast<const SvxWeightItem*>(pItem);
            aNewFont.SetWeight(pFontItem->GetWeight());
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_WORDLINEMODE,sal_True,&pItem) && pItem->ISA(SvxWordLineModeItem))
        {
            const SvxWordLineModeItem* pFontItem = static_cast<const SvxWordLineModeItem*>(pItem);
            aNewFont.SetWordLineMode(pFontItem->GetValue());
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_CROSSEDOUT,sal_True,&pItem) && pItem->ISA(SvxCrossedOutItem))
        {
            const SvxCrossedOutItem* pFontItem = static_cast<const SvxCrossedOutItem*>(pItem);
            aNewFont.SetStrikeout(pFontItem->GetStrikeout());
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_FONTHEIGHT,sal_True,&pItem) && pItem->ISA(SvxFontHeightItem))
        {
            const SvxFontHeightItem* pFontItem = static_cast<const SvxFontHeightItem*>(pItem);
            aNewFont.SetHeight(OutputDevice::LogicToLogic(Size(0, pFontItem->GetHeight()), MAP_TWIP, MAP_POINT).Height());
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_PROPSIZE,sal_True,&pItem) && pItem->ISA(SvxPropSizeItem))
        {
            //const SvxPropSizeItem* pFontItem = static_cast<const SvxPropSizeItem*>(pItem);
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_FONTLIST,sal_True,&pItem) && pItem->ISA(SvxFontListItem))
        {
            //const SvxFontListItem* pFontItem = static_cast<const SvxFontListItem*>(pItem);
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_CHARROTATE,sal_True,&pItem) && pItem->ISA(SvxCharRotateItem))
        {
            const SvxCharRotateItem* pRotateItem = static_cast<const SvxCharRotateItem*>(pItem);
            aNewFont.SetOrientation(pRotateItem->GetValue());
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_CHARSCALE_W,sal_True,&pItem) && pItem->ISA(SvxCharScaleWidthItem))
        {
            const SvxCharScaleWidthItem* pCharItem = static_cast<const SvxCharScaleWidthItem*>(pItem);
            aNewFont.SetWidthType(VCLUnoHelper::ConvertFontWidth(pCharItem->GetValue()));
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_UNDERLINE,sal_True,&pItem) && pItem->ISA(SvxUnderlineItem))
        {
            const SvxUnderlineItem* pFontItem = static_cast<const SvxUnderlineItem*>(pItem);
            aNewFont.SetUnderline(pFontItem->GetUnderline());
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_COLOR,sal_True,&pItem) && pItem->ISA(SvxColorItem))
        {
            const SvxColorItem* pFontItem = static_cast<const SvxColorItem*>(pItem);
            aNewFont.SetColor(pFontItem->GetValue().GetColor());
        }

        _out_rAwtFont = VCLUnoHelper::CreateFontDescriptor( aNewFont );
    }

    // -------------------------------------------------------------------------
    void lcl_itemsToCharProperties( const Font& _rOriginalControlFont, const SfxItemSet& _rItemSet, uno::Sequence< beans::NamedValue >& _out_rProperties )
    {
        const SfxPoolItem* pItem( NULL );

        // create an AWT font
        awt::FontDescriptor aAwtFont;
        lcl_initAwtFont( _rOriginalControlFont, _rItemSet, aAwtFont );
        lcl_pushBack( _out_rProperties, "Font", uno::makeAny( aAwtFont ) );

        // properties which cannot be represented in an AWT font need to be preserved directly
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_SHADOWED,sal_True,&pItem) && pItem->ISA(SvxShadowedItem))
        {
            const SvxShadowedItem* pFontItem = static_cast<const SvxShadowedItem*>(pItem);
            lcl_pushBack( _out_rProperties, "CharShadowed", uno::makeAny( pFontItem->GetValue() ) );
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_CONTOUR,sal_True,&pItem) && pItem->ISA(SvxContourItem))
        {
            const SvxContourItem* pFontItem = static_cast<const SvxContourItem*>(pItem);
            lcl_pushBack( _out_rProperties, "CharContoured", uno::makeAny( pFontItem->GetValue() ) );
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_UNDERLINE,sal_True,&pItem) && pItem->ISA(SvxUnderlineItem))
        {
            const SvxUnderlineItem* pFontItem = static_cast<const SvxUnderlineItem*>(pItem);
            lcl_pushBack( _out_rProperties, "CharUnderlineColor", uno::makeAny( pFontItem->GetColor().GetColor() ) );
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_HORJUSTIFY,sal_True,&pItem) && pItem->ISA(SvxHorJustifyItem))
        {
            const SvxHorJustifyItem* pJustifyItem = static_cast<const SvxHorJustifyItem*>(pItem);
            lcl_pushBack( _out_rProperties, "ParaAdjust", uno::makeAny( lcl_MapHorizontalAlignment( static_cast< SvxCellHorJustify >( pJustifyItem->GetEnumValue() ) ) ) );
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_VERJUSTIFY,sal_True,&pItem) && pItem->ISA(SvxVerJustifyItem))
        {
            const SvxVerJustifyItem* pJustifyItem = static_cast<const SvxVerJustifyItem*>(pItem);
            lcl_pushBack( _out_rProperties, "ParaVertAlignment", uno::makeAny( lcl_MapVerticalAlignment( static_cast< SvxCellVerJustify >( pJustifyItem->GetEnumValue() ) ) ) );
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_CHARRELIEF,sal_True,&pItem) && pItem->ISA(SvxCharReliefItem))
        {
            const SvxCharReliefItem* pFontItem = static_cast<const SvxCharReliefItem*>(pItem);
            lcl_pushBack( _out_rProperties, "CharRelief", uno::makeAny( static_cast< sal_Int16 >( pFontItem->GetEnumValue() ) ) );
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_CHARHIDDEN,sal_True,&pItem) && pItem->ISA(SvxCharHiddenItem))
        {
            const SvxCharHiddenItem* pFontItem = static_cast<const SvxCharHiddenItem*>(pItem);
            lcl_pushBack( _out_rProperties, "CharHidden", uno::makeAny( pFontItem->GetValue() ) );
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_AUTOKERN,sal_True,&pItem) && pItem->ISA(SvxAutoKernItem))
        {
            const SvxAutoKernItem* pFontItem = static_cast<const SvxAutoKernItem*>(pItem);
            lcl_pushBack( _out_rProperties, "CharAutoKerning", uno::makeAny( pFontItem->GetValue() ) );
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_BRUSH,sal_True,&pItem) && pItem->ISA(SvxBrushItem))
        {
            const SvxBrushItem* pFontItem = static_cast<const SvxBrushItem*>(pItem);
            lcl_pushBack( _out_rProperties, "ControlBackground", uno::makeAny( pFontItem->GetColor().GetColor() ) );
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_BLINK,sal_True,&pItem) && pItem->ISA(SvxBlinkItem))
        {
            const SvxBlinkItem* pFontItem = static_cast<const SvxBlinkItem*>(pItem);
            lcl_pushBack( _out_rProperties, "CharFlash", uno::makeAny( pFontItem->GetValue() ) );
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_EMPHASISMARK,sal_True,&pItem) && pItem->ISA(SvxEmphasisMarkItem))
        {
            const SvxEmphasisMarkItem* pFontItem = static_cast<const SvxEmphasisMarkItem*>(pItem);
            lcl_pushBack( _out_rProperties, "CharEmphasis", uno::makeAny( static_cast< sal_Int16 >( pFontItem->GetEmphasisMark() ) ) );
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_TWOLINES,sal_True,&pItem) && pItem->ISA(SvxTwoLinesItem))
        {
            const SvxTwoLinesItem* pFontItem = static_cast<const SvxTwoLinesItem*>(pItem);
            lcl_pushBack( _out_rProperties, "CharCombineIsOn", uno::makeAny( pFontItem->GetValue() ) );
            lcl_pushBack( _out_rProperties, "CharCombinePrefix", uno::makeAny( ::rtl::OUString( pFontItem->GetStartBracket() ) ) );
            lcl_pushBack( _out_rProperties, "CharCombineSuffix", uno::makeAny( ::rtl::OUString( pFontItem->GetEndBracket() ) ) );
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_COLOR,sal_True,&pItem) && pItem->ISA(SvxColorItem))
        {
            const SvxColorItem* pFontItem = static_cast<const SvxColorItem*>(pItem);
            lcl_pushBack( _out_rProperties, "CharColor", uno::makeAny( pFontItem->GetValue().GetColor() ) );
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_KERNING,sal_True,&pItem) && pItem->ISA(SvxKerningItem))
        {
            const SvxKerningItem* pFontItem = static_cast<const SvxKerningItem*>(pItem);
            lcl_pushBack( _out_rProperties, "CharKerning", uno::makeAny( pFontItem->GetValue() ) );
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_CASEMAP,sal_True,&pItem) && pItem->ISA(SvxCaseMapItem))
        {
            const SvxCaseMapItem* pFontItem = static_cast<const SvxCaseMapItem*>(pItem);
            lcl_pushBack( _out_rProperties, "CharCaseMap", uno::makeAny( pFontItem->GetValue() ) );
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_LANGUAGE,sal_True,&pItem) && pItem->ISA(SvxLanguageItem))
        {
            const SvxLanguageItem* pFontItem = static_cast<const SvxLanguageItem*>(pItem);
            lang::Locale aCharLocale;
            MsLangId::convertLanguageToLocale( pFontItem->GetLanguage(), aCharLocale );
            lcl_pushBack( _out_rProperties, "CharLocale", uno::makeAny( aCharLocale ) );
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_ESCAPEMENT,sal_True,&pItem) && pItem->ISA(SvxEscapementItem))
        {
            const SvxEscapementItem* pFontItem = static_cast<const SvxEscapementItem*>(pItem);
            lcl_pushBack( _out_rProperties, "CharEscapement", uno::makeAny( pFontItem->GetEsc() ) );
            lcl_pushBack( _out_rProperties, "CharEscapementHeight", uno::makeAny( (sal_Int8)pFontItem->GetProp() ) );
        }
    }

    // -------------------------------------------------------------------------
    template< class ATTRIBUTE_TYPE >
    void lcl_applyFontAttribute( const ::comphelper::NamedValueCollection& _rAttrValues, const sal_Char* _pAttributeName,
        const uno::Reference<report::XReportControlFormat >& _rxReportControlFormat,
        void (SAL_CALL report::XReportControlFormat::*pSetter)( ATTRIBUTE_TYPE ) )
    {
        ATTRIBUTE_TYPE aAttributeValue = ATTRIBUTE_TYPE();
        if ( _rAttrValues.get_ensureType( _pAttributeName, aAttributeValue ) )
            (_rxReportControlFormat.get()->*pSetter)( aAttributeValue );
    }

    // -------------------------------------------------------------------------
    void lcl_applyFontAttribute( const ::comphelper::NamedValueCollection& _rAttrValues, const sal_Char* _pAttributeName,
        const uno::Reference<report::XReportControlFormat >& _rxReportControlFormat,
        void (SAL_CALL report::XReportControlFormat::*pSetter)( const ::rtl::OUString& ) )
    {
        ::rtl::OUString aAttributeValue;
        if ( _rAttrValues.get_ensureType( _pAttributeName, aAttributeValue ) )
            (_rxReportControlFormat.get()->*pSetter)( aAttributeValue );
    }

    // -------------------------------------------------------------------------
    void lcl_applyFontAttribute( const ::comphelper::NamedValueCollection& _rAttrValues, const sal_Char* _pAttributeName,
        const uno::Reference<report::XReportControlFormat >& _rxReportControlFormat,
        void (SAL_CALL report::XReportControlFormat::*pSetter)( const lang::Locale& ) )
    {
        lang::Locale aAttributeValue;
        if ( _rAttrValues.get_ensureType( _pAttributeName, aAttributeValue ) )
            (_rxReportControlFormat.get()->*pSetter)( aAttributeValue );
    }
}

// -----------------------------------------------------------------------------
bool openCharDialog( const uno::Reference<report::XReportControlFormat >& _rxReportControlFormat,
        const uno::Reference< awt::XWindow>& _rxParentWindow, uno::Sequence< beans::NamedValue >& _out_rNewValues )
{
    OSL_PRECOND( _rxReportControlFormat.is() && _rxParentWindow.is(), "openCharDialog: invalid parameters!" );
    if ( !_rxReportControlFormat.is() || !_rxParentWindow.is() )
        return false;

    _out_rNewValues = uno::Sequence< beans::NamedValue >();

    // ------------
    // UNO->ItemSet
    static SfxItemInfo aItemInfos[] =
    {
        { SID_ATTR_CHAR_FONT, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_POSTURE, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_WEIGHT, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_SHADOWED, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_WORDLINEMODE, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_CONTOUR, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_STRIKEOUT, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_UNDERLINE, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_FONTHEIGHT, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_PROPSIZE, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_COLOR, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_KERNING, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_CASEMAP, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_LANGUAGE, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_ESCAPEMENT, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_FONTLIST, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_AUTOKERN, SFX_ITEM_POOLABLE },
        { SID_COLOR_TABLE, SFX_ITEM_POOLABLE },
        { SID_ATTR_FLASH, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_EMPHASISMARK, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_TWO_LINES, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_ROTATED, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_RELIEF, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_HIDDEN, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_SCALEWIDTH, SFX_ITEM_POOLABLE },
        { SID_ATTR_BRUSH, SFX_ITEM_POOLABLE },
        { SID_ATTR_ALIGN_HOR_JUSTIFY, SFX_ITEM_POOLABLE },
        //{ SID_ATTR_ALIGN_INDENT, SFX_ITEM_POOLABLE },
        { SID_ATTR_ALIGN_VER_JUSTIFY, SFX_ITEM_POOLABLE }
        //{ SID_ATTR_ALIGN_DEGREES, SFX_ITEM_POOLABLE }
    };
    Window* pParent = VCLUnoHelper::GetWindow( _rxParentWindow );
    ::std::auto_ptr<FontList> pFontList(new FontList( pParent ));
    ::std::auto_ptr<XColorTable> pColorTable( new XColorTable( SvtPathOptions().GetPalettePath() ));
    SfxPoolItem* pDefaults[] =
    {
        new SvxFontItem(ITEMID_FONT),
        new SvxPostureItem(ITALIC_NONE,ITEMID_POSTURE),
        new SvxWeightItem(WEIGHT_NORMAL,ITEMID_WEIGHT),
        new SvxShadowedItem(FALSE,ITEMID_SHADOWED),
        new SvxWordLineModeItem(FALSE,ITEMID_WORDLINEMODE),
        new SvxContourItem(FALSE,ITEMID_CONTOUR),
        new SvxCrossedOutItem(STRIKEOUT_NONE,ITEMID_CROSSEDOUT),
        new SvxUnderlineItem(UNDERLINE_NONE,ITEMID_UNDERLINE),
        new SvxFontHeightItem(240,100,ITEMID_FONTHEIGHT),
        new SvxPropSizeItem(100,ITEMID_PROPSIZE),
        new SvxColorItem(ITEMID_COLOR),
        new SvxKerningItem(0,ITEMID_KERNING),
        new SvxCaseMapItem(SVX_CASEMAP_NOT_MAPPED,ITEMID_CASEMAP),
        new SvxLanguageItem(LANGUAGE_GERMAN,ITEMID_LANGUAGE),
        new SvxEscapementItem(ITEMID_ESCAPEMENT),
        new SvxFontListItem(pFontList.get(),ITEMID_FONTLIST),
        new SvxAutoKernItem(FALSE,ITEMID_AUTOKERN),
        new SvxColorTableItem(pColorTable.get(),ITEMID_COLOR_TABLE),
        new SvxBlinkItem(FALSE,ITEMID_BLINK),
        new SvxEmphasisMarkItem(EMPHASISMARK_NONE,ITEMID_EMPHASISMARK),
        new SvxTwoLinesItem(TRUE,0,0,ITEMID_TWOLINES),
        new SvxCharRotateItem(0,sal_False,ITEMID_CHARROTATE),
        new SvxCharReliefItem(RELIEF_NONE,ITEMID_CHARRELIEF),
        new SvxCharHiddenItem(FALSE,ITEMID_CHARHIDDEN),
        new SvxCharScaleWidthItem(100,ITEMID_CHARSCALE_W),
        new SvxBrushItem(ITEMID_BRUSH),
        new SvxHorJustifyItem(ITEMID_HORJUSTIFY),
        //new SfxInt32Item(ITEMID_IDENT),
        new SvxVerJustifyItem(ITEMID_VERJUSTIFY)
        //new SfxInt32Item(ITEMID_DEGREES)
    };

    static USHORT pRanges[] =
    {
        /*
        SID_ATTR_CHAR_FONT,
        SID_ATTR_CHAR_WEIGHT,
        SID_ATTR_CHAR_FONTHEIGHT,
        SID_ATTR_CHAR_FONTHEIGHT,
        SID_ATTR_CHAR_COLOR,
        SID_ATTR_CHAR_COLOR,
        SID_ATTR_CHAR_LANGUAGE,
        SID_ATTR_CHAR_LANGUAGE,
        SID_ATTR_CHAR_CJK_FONT,
        SID_ATTR_CHAR_CJK_WEIGHT,
        SID_ATTR_CHAR_CTL_FONT,
        SID_ATTR_CHAR_CTL_WEIGHT
        */
        ITEMID_FONT,ITEMID_VERJUSTIFY,
        0
    };

    bool bSuccess = false;
    try
    {
        ::std::auto_ptr<SfxItemPool> pPool( new SfxItemPool(String::CreateFromAscii("ReportCharProperties"), ITEMID_FONT,ITEMID_VERJUSTIFY, aItemInfos, pDefaults) );
        // not needed for font height pPool->SetDefaultMetric( SFX_MAPUNIT_100TH_MM );  // ripped, don't understand why
        pPool->FreezeIdRanges();                        // the same

        ::std::auto_ptr<SfxItemSet> pDescriptor( new SfxItemSet( *pPool, pRanges ) );
        lcl_CharPropertiesToItems( _rxReportControlFormat, *pDescriptor );

        {   // want the dialog to be destroyed before our set
            ORptPageDialog aDlg(pParent, pDescriptor.get(),RID_PAGEDIALOG_CHAR);
            uno::Reference< report::XShape > xShape( _rxReportControlFormat, uno::UNO_QUERY );
            if ( xShape.is() )
                aDlg.RemoveTabPage( RID_PAGE_BACKGROUND );
            bSuccess = ( RET_OK == aDlg.Execute() );
            if ( bSuccess )
            {
                lcl_itemsToCharProperties( lcl_getReportControlFont( _rxReportControlFormat ), *aDlg.GetOutputItemSet(), _out_rNewValues );
            }
        }
    }
    catch(uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    for (sal_uInt16 i=0; i<sizeof(pDefaults)/sizeof(pDefaults[0]); ++i)
        delete pDefaults[i];

    return bSuccess;
}

// -----------------------------------------------------------------------------
void applyCharacterSettings( const uno::Reference< report::XReportControlFormat >& _rxReportControlFormat, const uno::Sequence< beans::NamedValue >& _rSettings )
{
    ::comphelper::NamedValueCollection aSettings( _rSettings );

    try
    {
        awt::FontDescriptor aAwtFont;
        if ( aSettings.get( "Font" ) >>= aAwtFont )
        {
            ::rtl::OUString sTemp = aAwtFont.Name;
            aAwtFont.Name = ::rtl::OUString(); // hack to
            _rxReportControlFormat->setFontDescriptor( aAwtFont );
            _rxReportControlFormat->setCharFontName( sTemp );
        }

        lcl_applyFontAttribute( aSettings, "CharShadowed", _rxReportControlFormat, &report::XReportControlFormat::setCharShadowed );
        lcl_applyFontAttribute( aSettings, "CharContoured", _rxReportControlFormat, &report::XReportControlFormat::setCharContoured );
        lcl_applyFontAttribute( aSettings, "CharUnderlineColor", _rxReportControlFormat, &report::XReportControlFormat::setCharUnderlineColor );
        lcl_applyFontAttribute( aSettings, "ParaAdjust", _rxReportControlFormat, &report::XReportControlFormat::setParaAdjust );
        lcl_applyFontAttribute( aSettings, "ParaVertAlignment", _rxReportControlFormat, &report::XReportControlFormat::setParaVertAlignment );
        lcl_applyFontAttribute( aSettings, "CharRelief", _rxReportControlFormat, &report::XReportControlFormat::setCharRelief );
        lcl_applyFontAttribute( aSettings, "CharHidden", _rxReportControlFormat, &report::XReportControlFormat::setCharHidden );
        lcl_applyFontAttribute( aSettings, "CharAutoKerning", _rxReportControlFormat, &report::XReportControlFormat::setCharAutoKerning );
        lcl_applyFontAttribute( aSettings, "ControlBackground", _rxReportControlFormat, &report::XReportControlFormat::setControlBackground );
        lcl_applyFontAttribute( aSettings, "CharFlash", _rxReportControlFormat, &report::XReportControlFormat::setCharFlash );
        lcl_applyFontAttribute( aSettings, "CharEmphasis", _rxReportControlFormat, &report::XReportControlFormat::setCharEmphasis );
        lcl_applyFontAttribute( aSettings, "CharCombineIsOn", _rxReportControlFormat, &report::XReportControlFormat::setCharCombineIsOn );
        lcl_applyFontAttribute( aSettings, "CharCombinePrefix", _rxReportControlFormat, &report::XReportControlFormat::setCharCombinePrefix );
        lcl_applyFontAttribute( aSettings, "CharCombineSuffix", _rxReportControlFormat, &report::XReportControlFormat::setCharCombineSuffix );
        lcl_applyFontAttribute( aSettings, "CharColor", _rxReportControlFormat, &report::XReportControlFormat::setCharColor );
        lcl_applyFontAttribute( aSettings, "CharKerning", _rxReportControlFormat, &report::XReportControlFormat::setCharKerning );
        lcl_applyFontAttribute( aSettings, "CharCaseMap", _rxReportControlFormat, &report::XReportControlFormat::setCharCaseMap );
        lcl_applyFontAttribute( aSettings, "CharLocale", _rxReportControlFormat, &report::XReportControlFormat::setCharLocale );
        lcl_applyFontAttribute( aSettings, "CharEscapement", _rxReportControlFormat, &report::XReportControlFormat::setCharEscapement );
        lcl_applyFontAttribute( aSettings, "CharEscapementHeight", _rxReportControlFormat, &report::XReportControlFormat::setCharEscapementHeight );
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

// -----------------------------------------------------------------------------
bool openCharDialog(const uno::Reference<report::XReportControlFormat>& _rxReportControlFormat,
                    const uno::Reference< awt::XWindow>& _xWindow)
{
    uno::Sequence< beans::NamedValue > aSettings;
    if ( !openCharDialog( _rxReportControlFormat, _xWindow, aSettings ) )
        return false;
    applyCharacterSettings( _rxReportControlFormat, aSettings );
    return true;
}

// -----------------------------------------------------------------------------
void notifySystemWindow(Window* _pWindow,Window* _pToRegister, ::comphelper::mem_fun1_t<TaskPaneList,Window*> _rMemFunc)
{
    OSL_ENSURE(_pWindow,"Window can not be null!");
    SystemWindow* pSystemWindow = _pWindow ? _pWindow->GetSystemWindow() : NULL;
    if ( pSystemWindow )
    {
        _rMemFunc( pSystemWindow->GetTaskPaneList(), _pToRegister );
    }
}
// -----------------------------------------------------------------------------
SdrObject* isOver(const Rectangle& _rRect,SdrPage& _rPage,SdrView& _rView,bool _bAllObjects,SdrObject* _pIgnore)
{
    SdrObject* pOverlappedObj = NULL;
    SdrObjListIter aIter(_rPage,IM_DEEPNOGROUPS);
    SdrObject* pObjIter = NULL;

    while( !pOverlappedObj && (pObjIter = aIter.Next()) != NULL )
    {
        if ( _pIgnore != pObjIter
            && (_bAllObjects || !_rView.IsObjMarked(pObjIter))
            && dynamic_cast<OUnoObject*>(pObjIter) != NULL )
        {
            Rectangle aRect = _rRect.GetIntersection(pObjIter->GetLastBoundRect());
            if ( !aRect.IsEmpty() && (aRect.Left() != aRect.Right() && aRect.Top() != aRect.Bottom() ) )
                pOverlappedObj = pObjIter;
        }
    }
    return pOverlappedObj;
}
//----------------------------------------------------------------------------
SdrObject* isOver(SdrObject* _pObj,SdrPage& _rPage,SdrView& _rView,bool _bUnMarkedObjects)
{
    SdrObject* pOverlappedObj = NULL;
    OUnoObject* pUnoObj = dynamic_cast<OUnoObject*>(_pObj);
    if ( pUnoObj ) // this doesn't need to be done for shapes
    {
        Rectangle aRect = pUnoObj->GetCurrentBoundRect();
        pOverlappedObj = isOver(aRect,_rPage,_rView,_bUnMarkedObjects,_pObj);
    }
    return pOverlappedObj;
}

// -----------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > getParameterNames( const uno::Reference< sdbc::XRowSet >& _rxRowSet )
{
    uno::Sequence< ::rtl::OUString > aNames;

    try
    {
        uno::Reference< sdb::XParametersSupplier > xSuppParams( _rxRowSet, uno::UNO_QUERY_THROW );
        uno::Reference< container::XIndexAccess > xParams( xSuppParams->getParameters() );
        if ( xParams.is() )
        {
            sal_Int32 count( xParams->getCount() );
            aNames.realloc( count );

            uno::Reference< beans::XPropertySet > xParam;
            ::rtl::OUString sParamName;
            for ( sal_Int32 i=0; i<count; ++i )
            {
                xParam.set( xParams->getByIndex(i), uno::UNO_QUERY_THROW );
                OSL_VERIFY( xParam->getPropertyValue( PROPERTY_NAME ) >>= sParamName );
                aNames[i] = sParamName;
            }
        }
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return aNames;
}
// -----------------------------------------------------------------------------
// check overlapping
void correctOverlapping(SdrObject* pControl,::boost::shared_ptr<OReportSection> _pReportSection,bool _bInsert)
{
    OSectionView* pSectionView = _pReportSection->getView();
    uno::Reference< report::XReportComponent> xComponent(pControl->getUnoShape(),uno::UNO_QUERY);
    Rectangle aRet(VCLPoint(xComponent->getPosition()),VCLSize(xComponent->getSize()));
    aRet.setHeight(aRet.getHeight() + 1);
    aRet.setWidth(aRet.getWidth() + 1);
    bool bOverlapping = true;
    while ( bOverlapping )
    {
        SdrObject* pOverlappedObj = isOver(aRet,*_pReportSection->getPage(),*pSectionView,true,pControl);
        bOverlapping = pOverlappedObj != NULL;
        if ( bOverlapping )
        {
            const Rectangle& aLogicRect = pOverlappedObj->GetLogicRect();
            aRet.Move(0,aLogicRect.Top() + aLogicRect.getHeight() - aRet.Top());
            xComponent->setPositionY(aRet.Top());
        }
    }
    if ( !bOverlapping && _bInsert ) // now insert objects
        pSectionView->InsertObjectAtView(pControl,*pSectionView->GetSdrPageView(),SDRINSERT_ADDMARK);
}
// -----------------------------------------------------------------------------
} // namespace rptui
// -----------------------------------------------------------------------------
