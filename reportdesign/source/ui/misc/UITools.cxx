/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <memory>
#include <toolkit/helper/convert.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <SectionView.hxx>
#include <UITools.hxx>
#include <Formula.hxx>
#include <FunctionHelper.hxx>
#include <reportformula.hxx>

#include <comphelper/diagnose_ex.hxx>

#include <vcl/settings.hxx>
#include <vcl/syswin.hxx>
#include <vcl/svapp.hxx>
#include <vcl/unohelp.hxx>
#include <vcl/window.hxx>

#include <com/sun/star/lang/NullPointerException.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <svx/svdpool.hxx>

#include <editeng/charscaleitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/blinkitem.hxx>
#include <editeng/flstitem.hxx>
#include <editeng/autokernitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/justifyitem.hxx>
#include <svx/drawitem.hxx>
#include <editeng/twolinesitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/charrotateitem.hxx>
#include <editeng/charhiddenitem.hxx>
#include <editeng/memberids.h>
#include <svx/xgrscit.hxx>
#include <svx/svditer.hxx>
#include <svx/xtable.hxx>
#include <svx/svdview.hxx>
#include <svx/svdpage.hxx>
#include <svx/svxdlg.hxx>
#include <svx/unoprov.hxx>
#include <svx/svxids.hrc>

#include <unotools/charclass.hxx>
#include <svtools/ctrltool.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <svl/sharedstringpool.hxx>

#include <comphelper/propmultiplex.hxx>
#include <comphelper/namedvaluecollection.hxx>

#include <connectivity/dbexception.hxx>
#include <connectivity/dbtools.hxx>

#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/report/XShape.hpp>
#include <com/sun/star/sdb/XParametersSupplier.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <i18nlangtag/languagetag.hxx>
#include <dlgpage.hxx>
#include <strings.hxx>
#include <core_resource.hxx>
#include <RptObject.hxx>
#include <RptDef.hxx>
#include <strings.hrc>
#include <ReportDefinition.hxx>
#include <RptModel.hxx>

#include <svx/xflbckit.hxx>
#include <svx/xflbmpit.hxx>
#include <svx/xflbmsli.hxx>
#include <svx/xflbmsxy.hxx>
#include <svx/xflbmtit.hxx>
#include <svx/xflboxy.hxx>
#include <svx/xflbstit.hxx>
#include <svx/xflbtoxy.hxx>

#include <svx/xbtmpit.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xfltrit.hxx>
#include <svx/xflclit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xflftrit.hxx>
#include <svx/xsflclit.hxx>

/// Note that we deliberately overlap an existing item id, so that we can have contiguous item ids for
/// the static defaults.
#define ITEMID_FIRST                    XATTR_FILL_LAST

#define ITEMID_FONT                     TypedWhichId<SvxFontItem>(ITEMID_FIRST)
#define ITEMID_FONTHEIGHT               TypedWhichId<SvxFontHeightItem>(ITEMID_FIRST + 1)
#define ITEMID_LANGUAGE                 TypedWhichId<SvxLanguageItem>(ITEMID_FIRST + 2)

#define ITEMID_POSTURE                  TypedWhichId<SvxPostureItem>(ITEMID_FIRST + 3)
#define ITEMID_WEIGHT                   TypedWhichId<SvxWeightItem>(ITEMID_FIRST + 4)
#define ITEMID_SHADOWED                 TypedWhichId<SvxShadowedItem>(ITEMID_FIRST + 5)
#define ITEMID_WORDLINEMODE             TypedWhichId<SvxWordLineModeItem>(ITEMID_FIRST + 6)
#define ITEMID_CONTOUR                  TypedWhichId<SvxContourItem>(ITEMID_FIRST + 7)
#define ITEMID_CROSSEDOUT               TypedWhichId<SvxCrossedOutItem>(ITEMID_FIRST + 8)
#define ITEMID_UNDERLINE                TypedWhichId<SvxUnderlineItem>(ITEMID_FIRST + 9)

#define ITEMID_COLOR                    TypedWhichId<SvxColorItem>(ITEMID_FIRST + 10)
#define ITEMID_KERNING                  TypedWhichId<SvxKerningItem>(ITEMID_FIRST + 11)
#define ITEMID_CASEMAP                  TypedWhichId<SvxCaseMapItem>(ITEMID_FIRST + 12)

#define ITEMID_ESCAPEMENT               TypedWhichId<SvxEscapementItem>(ITEMID_FIRST + 13)
#define ITEMID_FONTLIST                 ITEMID_FIRST + 14
#define ITEMID_AUTOKERN                 TypedWhichId<SvxAutoKernItem>(ITEMID_FIRST + 15)
#define ITEMID_COLOR_TABLE              TypedWhichId<SvxColorListItem>(ITEMID_FIRST + 16)
#define ITEMID_BLINK                    TypedWhichId<SvxBlinkItem>(ITEMID_FIRST + 17)
#define ITEMID_EMPHASISMARK             TypedWhichId<SvxEmphasisMarkItem>(ITEMID_FIRST + 18)
#define ITEMID_TWOLINES                 TypedWhichId<SvxTwoLinesItem>(ITEMID_FIRST + 19)
#define ITEMID_CHARROTATE               TypedWhichId<SvxCharRotateItem>(ITEMID_FIRST + 20)
#define ITEMID_CHARSCALE_W              TypedWhichId<SvxCharScaleWidthItem>(ITEMID_FIRST + 21)
#define ITEMID_CHARRELIEF               TypedWhichId<SvxCharReliefItem>(ITEMID_FIRST + 22)
#define ITEMID_CHARHIDDEN               TypedWhichId<SvxCharHiddenItem>(ITEMID_FIRST + 23)
#define ITEMID_BRUSH                    TypedWhichId<SvxBrushItem>(ITEMID_FIRST + 24)
#define ITEMID_HORJUSTIFY               TypedWhichId<SvxHorJustifyItem>(ITEMID_FIRST + 25)
#define ITEMID_VERJUSTIFY               TypedWhichId<SvxVerJustifyItem>(ITEMID_FIRST + 26)
#define ITEMID_FONT_ASIAN               TypedWhichId<SvxFontItem>(ITEMID_FIRST + 27)
#define ITEMID_FONTHEIGHT_ASIAN         TypedWhichId<SvxFontHeightItem>(ITEMID_FIRST + 28)
#define ITEMID_LANGUAGE_ASIAN           TypedWhichId<SvxLanguageItem>(ITEMID_FIRST + 29)
#define ITEMID_POSTURE_ASIAN            TypedWhichId<SvxPostureItem>(ITEMID_FIRST + 30)
#define ITEMID_WEIGHT_ASIAN             TypedWhichId<SvxWeightItem>(ITEMID_FIRST + 31)
#define ITEMID_FONT_COMPLEX             TypedWhichId<SvxFontItem>(ITEMID_FIRST + 32)
#define ITEMID_FONTHEIGHT_COMPLEX       TypedWhichId<SvxFontHeightItem>(ITEMID_FIRST + 33)
#define ITEMID_LANGUAGE_COMPLEX         TypedWhichId<SvxLanguageItem>(ITEMID_FIRST + 34)
#define ITEMID_POSTURE_COMPLEX          TypedWhichId<SvxPostureItem>(ITEMID_FIRST + 35)
#define ITEMID_WEIGHT_COMPLEX           TypedWhichId<SvxWeightItem>(ITEMID_FIRST + 36)

static_assert((ITEMID_WEIGHT_COMPLEX - XATTR_FILL_FIRST) == 56, "Item ids are not contiguous");

#define WESTERN 0
#define ASIAN   1
#define COMPLEX 2

namespace rptui
{
using namespace ::com::sun::star;

void adjustSectionName(const uno::Reference< report::XGroup >& _xGroup,sal_Int32 _nPos)
{
    OSL_ENSURE(_xGroup.is(),"Group is NULL -> GPF");
    if ( _xGroup->getHeaderOn() && _xGroup->getHeader()->getName().isEmpty() )
    {
        OUString sName = RptResId(RID_STR_GROUPHEADER) + OUString::number(_nPos);
        _xGroup->getHeader()->setName(sName);
    }

    if ( _xGroup->getFooterOn() && _xGroup->getFooter()->getName().isEmpty() )
    {
        OUString sName = RptResId(RID_STR_GROUPFOOTER) + OUString::number(_nPos);
        _xGroup->getFooter()->setName(sName);
    }
}

::rtl::Reference< comphelper::OPropertyChangeMultiplexer> addStyleListener(const uno::Reference< report::XReportDefinition >& _xReportDefinition,::comphelper::OPropertyChangeListener* _pListener)
{
    ::rtl::Reference< comphelper::OPropertyChangeMultiplexer> pRet;
    if ( _xReportDefinition.is() )
    {
        uno::Reference<beans::XPropertySet> xPageStyle(getUsedStyle(_xReportDefinition),uno::UNO_QUERY);
        if ( xPageStyle.is() )
        {
            pRet = new comphelper::OPropertyChangeMultiplexer(_pListener,xPageStyle);
            pRet->addProperty(PROPERTY_LEFTMARGIN);
            pRet->addProperty(PROPERTY_RIGHTMARGIN);
            pRet->addProperty(PROPERTY_PAPERSIZE);
            pRet->addProperty(PROPERTY_BACKCOLOR);
        }
    }
    return pRet;
}


namespace
{

    vcl::Font lcl_getReportControlFont( const uno::Reference<report::XReportControlFormat >& _rxReportControlFormat, awt::FontDescriptor& _out_rControlFont ,sal_uInt16 _nWichFont)
    {
        if ( !_rxReportControlFormat.is() )
            throw uno::RuntimeException();

        switch(_nWichFont)
        {
            case WESTERN:
                _out_rControlFont = _rxReportControlFormat->getFontDescriptor();
                break;
            case ASIAN:
                _out_rControlFont = _rxReportControlFormat->getFontDescriptorAsian();
                break;
            case COMPLEX:
                _out_rControlFont = _rxReportControlFormat->getFontDescriptorComplex();
                break;

        }

        vcl::Font aDefaultFont = Application::GetDefaultDevice()->GetSettings().GetStyleSettings().GetAppFont();
        return VCLUnoHelper::CreateFont( _out_rControlFont, aDefaultFont );
    }


    vcl::Font lcl_getReportControlFont( const uno::Reference<report::XReportControlFormat >& _rxReportControlFormat,sal_uInt16 _nWhich )
    {
        awt::FontDescriptor aAwtFont;
        return lcl_getReportControlFont( _rxReportControlFormat, aAwtFont, _nWhich );
    }

    vcl::Font lcl_setFont(const uno::Reference<report::XReportControlFormat >& _rxReportControlFormat,
        SfxItemSet& _rItemSet,sal_uInt16 _nWhich,sal_uInt16 _nFont, sal_uInt16 _nFontHeight,sal_uInt16 _nLanguage,sal_uInt16 _nPosture, sal_uInt16 _nWeight)
    {
        // fill it
        awt::FontDescriptor aControlFont;
        const vcl::Font aFont( lcl_getReportControlFont( _rxReportControlFormat, aControlFont,_nWhich ) );

        SvxFontItem aFontItem(_nFont);
        aFontItem.PutValue( uno::Any( aControlFont ), 0 );
        _rItemSet.Put(aFontItem);

        _rItemSet.Put(SvxFontHeightItem(o3tl::convert(aFont.GetFontHeight(), o3tl::Length::pt, o3tl::Length::twip), 100, _nFontHeight));
        lang::Locale aLocale;
        switch(_nWhich)
        {
            default:
                aLocale = _rxReportControlFormat->getCharLocale();
                break;
            case ASIAN:
                aLocale = _rxReportControlFormat->getCharLocaleAsian();
                break;
            case COMPLEX:
                aLocale = _rxReportControlFormat->getCharLocaleComplex();
                break;
        }

        _rItemSet.Put(SvxLanguageItem(LanguageTag(aLocale).makeFallback().getLanguageType(),_nLanguage));

        _rItemSet.Put(SvxPostureItem(aFont.GetItalic(),_nPosture));
        _rItemSet.Put(SvxWeightItem(aFont.GetWeight(),_nWeight));
        return aFont;
    }

    void lcl_fillShapeToItems( const uno::Reference<report::XShape >& _xShape,SfxItemSet& _rItemSet )
    {
        uno::Reference< beans::XPropertySetInfo> xInfo = _xShape->getPropertySetInfo();
        SvxUnoPropertyMapProvider aMap;
        const SfxItemPropertyMap& rPropertyMap = aMap.GetPropertySet(SVXMAP_CUSTOMSHAPE, SdrObject::GetGlobalDrawObjectItemPool())->getPropertyMap();
        for (const auto pProp : rPropertyMap.getPropertyEntries())
        {
            if ( xInfo->hasPropertyByName(pProp->aName) )
            {
                const SfxPoolItem* pItem = _rItemSet.GetItem(pProp->nWID);
                if ( pItem )
                {
                    ::std::unique_ptr<SfxPoolItem> pClone(pItem->CloneSetWhich(pProp->nWID));
                    pClone->PutValue(_xShape->getPropertyValue(pProp->aName), pProp->nMemberId);
                    _rItemSet.Put(std::move(pClone));
                }
            }
        }
    }

    void lcl_fillItemsToShape( const uno::Reference<report::XShape >& _xShape,const SfxItemSet& _rItemSet )
    {
        const uno::Reference< beans::XPropertySetInfo> xInfo = _xShape->getPropertySetInfo();
        SvxUnoPropertyMapProvider aMap;
        const SfxItemPropertyMap& rPropertyMap = aMap.GetPropertySet(SVXMAP_CUSTOMSHAPE, SdrObject::GetGlobalDrawObjectItemPool())->getPropertyMap();
        for (const auto pProp : rPropertyMap.getPropertyEntries())
        {
            if ( SfxItemState::SET == _rItemSet.GetItemState(pProp->nWID) && xInfo->hasPropertyByName(pProp->aName) )
            {
                if ( ( pProp->nFlags & beans::PropertyAttribute::READONLY ) != beans::PropertyAttribute::READONLY )
                {
                    const SfxPoolItem* pItem = _rItemSet.GetItem(pProp->nWID);
                    if ( pItem )
                    {
                        uno::Any aValue;
                        pItem->QueryValue(aValue, pProp->nMemberId);
                        try
                        {
                            _xShape->setPropertyValue(pProp->aName, aValue);
                        }
                        catch(uno::Exception&)
                        { // shapes have a bug so we ignore this one.
                        }
                    }
                }
            }
        }
    }

    void lcl_CharPropertiesToItems( const uno::Reference<report::XReportControlFormat >& _rxReportControlFormat,
        SfxItemSet& _rItemSet )
    {
        if ( !_rxReportControlFormat.is() )
            throw lang::NullPointerException();

        uno::Reference< beans::XPropertySet > xSet(_rxReportControlFormat,uno::UNO_QUERY_THROW);

        // fill it
        const vcl::Font aFont( lcl_setFont(_rxReportControlFormat, _rItemSet,WESTERN,ITEMID_FONT,ITEMID_FONTHEIGHT,ITEMID_LANGUAGE,ITEMID_POSTURE,ITEMID_WEIGHT ) );

        _rItemSet.Put(SvxShadowedItem(_rxReportControlFormat->getCharShadowed(),ITEMID_SHADOWED));
        _rItemSet.Put(SvxWordLineModeItem(aFont.IsWordLineMode(),ITEMID_WORDLINEMODE));
        _rItemSet.Put(SvxContourItem(_rxReportControlFormat->getCharContoured(),ITEMID_CONTOUR));
        _rItemSet.Put(SvxAutoKernItem(_rxReportControlFormat->getCharAutoKerning(),ITEMID_AUTOKERN));
        _rItemSet.Put(SvxCrossedOutItem(aFont.GetStrikeout(),ITEMID_CROSSEDOUT));
        _rItemSet.Put(SvxCaseMapItem(static_cast<SvxCaseMap>(_rxReportControlFormat->getCharCaseMap()),ITEMID_CASEMAP));

        _rItemSet.Put(SvxEscapementItem(_rxReportControlFormat->getCharEscapement(),_rxReportControlFormat->getCharEscapementHeight(),ITEMID_ESCAPEMENT));
        _rItemSet.Put(SvxBlinkItem(_rxReportControlFormat->getCharFlash(),ITEMID_BLINK));
        _rItemSet.Put(SvxCharHiddenItem(_rxReportControlFormat->getCharHidden(),ITEMID_CHARHIDDEN));
        _rItemSet.Put(SvxTwoLinesItem(_rxReportControlFormat->getCharCombineIsOn(),_rxReportControlFormat->getCharCombinePrefix().toChar(),_rxReportControlFormat->getCharCombineSuffix().toChar(),ITEMID_TWOLINES));
        SvxUnderlineItem aUnderLineItem(aFont.GetUnderline(),ITEMID_UNDERLINE);
        aUnderLineItem.SetColor(Color(ColorTransparency, _rxReportControlFormat->getCharUnderlineColor()));
        _rItemSet.Put(aUnderLineItem);
        _rItemSet.Put(SvxKerningItem(_rxReportControlFormat->getCharKerning(),ITEMID_KERNING));
        _rItemSet.Put(SvxEmphasisMarkItem(static_cast<FontEmphasisMark>(_rxReportControlFormat->getCharEmphasis()),ITEMID_EMPHASISMARK));
        _rItemSet.Put(SvxCharReliefItem(static_cast<FontRelief>(_rxReportControlFormat->getCharRelief()),ITEMID_CHARRELIEF));
        _rItemSet.Put(SvxColorItem(::Color(ColorTransparency, _rxReportControlFormat->getCharColor()),ITEMID_COLOR));
        _rItemSet.Put(SvxCharRotateItem(Degree10(_rxReportControlFormat->getCharRotation()),false,ITEMID_CHARROTATE));
        _rItemSet.Put(SvxCharScaleWidthItem(_rxReportControlFormat->getCharScaleWidth(),ITEMID_CHARSCALE_W));

        SvxHorJustifyItem aHorJustifyItem(ITEMID_HORJUSTIFY);
        aHorJustifyItem.PutValue(xSet->getPropertyValue(PROPERTY_PARAADJUST),MID_HORJUST_ADJUST);
        _rItemSet.Put(aHorJustifyItem);
        SvxVerJustifyItem aVerJustifyItem(ITEMID_VERJUSTIFY);
        aVerJustifyItem.PutValue(xSet->getPropertyValue(PROPERTY_VERTICALALIGN),MID_HORJUST_ADJUST);
        _rItemSet.Put(aVerJustifyItem);

        uno::Reference< report::XShape> xShape(_rxReportControlFormat,uno::UNO_QUERY);
        if ( !xShape.is() )
            _rItemSet.Put(SvxBrushItem(::Color(ColorTransparency, _rxReportControlFormat->getControlBackground()),ITEMID_BRUSH));

        lcl_setFont(_rxReportControlFormat, _rItemSet,ASIAN,ITEMID_FONT_ASIAN,ITEMID_FONTHEIGHT_ASIAN,ITEMID_LANGUAGE_ASIAN,ITEMID_POSTURE_ASIAN,ITEMID_WEIGHT_ASIAN );
        lcl_setFont(_rxReportControlFormat, _rItemSet,COMPLEX,ITEMID_FONT_COMPLEX,ITEMID_FONTHEIGHT_COMPLEX,ITEMID_LANGUAGE_COMPLEX,ITEMID_POSTURE_COMPLEX,ITEMID_WEIGHT_COMPLEX );
    }


    void lcl_pushBack( uno::Sequence< beans::NamedValue >& _out_rProperties, const OUString& _sName, const uno::Any& _rValue )
    {
        sal_Int32 nLen( _out_rProperties.getLength() );
        _out_rProperties.realloc( nLen + 1 );
        _out_rProperties.getArray()[ nLen ] = beans::NamedValue( _sName, _rValue );
    }


    void lcl_initAwtFont( const vcl::Font& _rOriginalFont, const SfxItemSet& _rItemSet, awt::FontDescriptor& _out_rAwtFont,
        TypedWhichId<SvxFontItem> _nFont, TypedWhichId<SvxFontHeightItem> _nFontHeight,
        TypedWhichId<SvxPostureItem> _nPosture, TypedWhichId<SvxWeightItem> _nWeight)
    {
        vcl::Font aNewFont( _rOriginalFont );
        if ( const SvxFontItem* pFontItem = _rItemSet.GetItemIfSet( _nFont) )
        {
            aNewFont.SetFamilyName(pFontItem->GetFamilyName());
            aNewFont.SetStyleName(pFontItem->GetStyleName());
            aNewFont.SetFamily(pFontItem->GetFamily());
            aNewFont.SetPitch(pFontItem->GetPitch());
            aNewFont.SetCharSet(pFontItem->GetCharSet());
        }
        if ( const SvxFontHeightItem* pFontItem = _rItemSet.GetItemIfSet( _nFontHeight ) )
        {
            aNewFont.SetFontHeight(o3tl::convert(pFontItem->GetHeight(), o3tl::Length::twip, o3tl::Length::pt));
        }
        if ( const SvxPostureItem* pPostureItem = _rItemSet.GetItemIfSet( _nPosture) )
        {
            aNewFont.SetItalic(pPostureItem->GetPosture());
        }
        if ( const SvxWeightItem* pWeightItem = _rItemSet.GetItemIfSet( _nWeight ) )
        {
            aNewFont.SetWeight(pWeightItem->GetWeight());
        }
        if ( const SvxWordLineModeItem* pWordLineItem = _rItemSet.GetItemIfSet( ITEMID_WORDLINEMODE ) )
        {
            aNewFont.SetWordLineMode(pWordLineItem->GetValue());
        }
        if ( const SvxCrossedOutItem* pCrossedOutItem = _rItemSet.GetItemIfSet( ITEMID_CROSSEDOUT ) )
        {
            aNewFont.SetStrikeout(pCrossedOutItem->GetStrikeout());
        }
        if ( const SvxCharRotateItem* pRotateItem = _rItemSet.GetItemIfSet( ITEMID_CHARROTATE ) )
        {
            aNewFont.SetOrientation(pRotateItem->GetValue());
        }
        if ( const SvxCharScaleWidthItem* pCharItem = _rItemSet.GetItemIfSet( ITEMID_CHARSCALE_W ) )
        {
            aNewFont.SetWidthType(vcl::unohelper::ConvertFontWidth(pCharItem->GetValue()));
        }
        if ( const SvxUnderlineItem* pUnderlineItem = _rItemSet.GetItemIfSet( ITEMID_UNDERLINE ) )
        {
            aNewFont.SetUnderline(pUnderlineItem->GetLineStyle());
        }
        if ( const SvxColorItem* pColorItem = _rItemSet.GetItemIfSet( ITEMID_COLOR ) )
        {
            aNewFont.SetColor(pColorItem->GetValue());
        }

        _out_rAwtFont = VCLUnoHelper::CreateFontDescriptor( aNewFont );
    }


    void lcl_itemsToCharProperties( const vcl::Font& _rOriginalControlFont,const vcl::Font& _rOriginalControlFontAsian,const vcl::Font& _rOriginalControlFontComplex, const SfxItemSet& _rItemSet, uno::Sequence< beans::NamedValue >& _out_rProperties )
    {
        // create an AWT font
        awt::FontDescriptor aAwtFont;
        lcl_initAwtFont( _rOriginalControlFont, _rItemSet, aAwtFont,ITEMID_FONT,ITEMID_FONTHEIGHT,ITEMID_POSTURE, ITEMID_WEIGHT);
        lcl_pushBack( _out_rProperties, "Font", uno::Any( aAwtFont ) );
        lcl_initAwtFont( _rOriginalControlFontAsian, _rItemSet, aAwtFont,ITEMID_FONT_ASIAN,ITEMID_FONTHEIGHT_ASIAN,ITEMID_POSTURE_ASIAN, ITEMID_WEIGHT_ASIAN);
        lcl_pushBack( _out_rProperties, "FontAsian", uno::Any( aAwtFont ) );
        lcl_initAwtFont( _rOriginalControlFontComplex, _rItemSet, aAwtFont,ITEMID_FONT_COMPLEX,ITEMID_FONTHEIGHT_COMPLEX,ITEMID_POSTURE_COMPLEX, ITEMID_WEIGHT_COMPLEX);
        lcl_pushBack( _out_rProperties, "FontComplex", uno::Any( aAwtFont ) );

        // properties which cannot be represented in an AWT font need to be preserved directly
        if ( const SvxShadowedItem* pShadowedItem = _rItemSet.GetItemIfSet( ITEMID_SHADOWED) )
            lcl_pushBack( _out_rProperties, PROPERTY_CHARSHADOWED, uno::Any( pShadowedItem->GetValue() ) );
        if ( const SvxContourItem* pContourItem = _rItemSet.GetItemIfSet( ITEMID_CONTOUR ) )
            lcl_pushBack( _out_rProperties, PROPERTY_CHARCONTOURED, uno::Any( pContourItem->GetValue() ) );
        if ( const SvxUnderlineItem* pUnderlineItem = _rItemSet.GetItemIfSet( ITEMID_UNDERLINE ) )
            lcl_pushBack( _out_rProperties, PROPERTY_CHARUNDERLINECOLOR, uno::Any( pUnderlineItem->GetColor() ) );
        if ( const SvxHorJustifyItem* pJustifyItem = _rItemSet.GetItemIfSet( ITEMID_HORJUSTIFY ) )
        {
            uno::Any aValue;
            pJustifyItem->QueryValue(aValue,MID_HORJUST_ADJUST);
            lcl_pushBack( _out_rProperties, PROPERTY_PARAADJUST, aValue );
        }
        if ( const SvxVerJustifyItem* pJustifyItem = _rItemSet.GetItemIfSet( ITEMID_VERJUSTIFY ) )
        {
            uno::Any aValue;
            pJustifyItem->QueryValue(aValue,MID_HORJUST_ADJUST);
            lcl_pushBack( _out_rProperties, PROPERTY_VERTICALALIGN, aValue );
        }
        if ( const SvxCharReliefItem* pReliefItem = _rItemSet.GetItemIfSet( ITEMID_CHARRELIEF ) )
            lcl_pushBack( _out_rProperties, PROPERTY_CHARRELIEF, uno::Any( static_cast< sal_Int16 >( pReliefItem->GetEnumValue() ) ) );
        if ( const SvxCharHiddenItem* pHiddenItem = _rItemSet.GetItemIfSet( ITEMID_CHARHIDDEN ) )
            lcl_pushBack( _out_rProperties, PROPERTY_CHARHIDDEN, uno::Any( pHiddenItem->GetValue() ) );
        if ( const SvxAutoKernItem* pKernItem = _rItemSet.GetItemIfSet( ITEMID_AUTOKERN ) )
            lcl_pushBack( _out_rProperties, PROPERTY_CHARAUTOKERNING, uno::Any( pKernItem->GetValue() ) );
        if ( const SvxBrushItem* pBrushItem = _rItemSet.GetItemIfSet( ITEMID_BRUSH ) )
            lcl_pushBack( _out_rProperties, PROPERTY_CONTROLBACKGROUND, uno::Any( pBrushItem->GetColor() ) );
        if ( const SvxBlinkItem* pBlinkItem =  _rItemSet.GetItemIfSet( ITEMID_BLINK ) )
            lcl_pushBack( _out_rProperties, PROPERTY_CHARFLASH, uno::Any( pBlinkItem->GetValue() ) );
        if ( const SvxEmphasisMarkItem* pMarkItem = _rItemSet.GetItemIfSet( ITEMID_EMPHASISMARK ) )
            lcl_pushBack( _out_rProperties, PROPERTY_CHAREMPHASIS, uno::Any( static_cast< sal_Int16 >( pMarkItem->GetEmphasisMark() ) ) );
        if ( const SvxTwoLinesItem* pLinesItem = _rItemSet.GetItemIfSet( ITEMID_TWOLINES ) )
        {
            lcl_pushBack( _out_rProperties, PROPERTY_CHARCOMBINEISON, uno::Any( pLinesItem->GetValue() ) );
            lcl_pushBack( _out_rProperties, PROPERTY_CHARCOMBINEPREFIX, uno::Any( OUString( pLinesItem->GetStartBracket() ) ) );
            lcl_pushBack( _out_rProperties, PROPERTY_CHARCOMBINESUFFIX, uno::Any( OUString( pLinesItem->GetEndBracket() ) ) );
        }
        if ( const SvxColorItem* pColorItem = _rItemSet.GetItemIfSet( ITEMID_COLOR ) )
            lcl_pushBack( _out_rProperties, PROPERTY_CHARCOLOR, uno::Any( pColorItem->GetValue() ) );
        if ( const SvxKerningItem* pKernItem = _rItemSet.GetItemIfSet( ITEMID_KERNING ) )
            lcl_pushBack( _out_rProperties, PROPERTY_CHARKERNING, uno::Any( pKernItem->GetValue() ) );
        if ( const SvxCaseMapItem* pCaseMapItem = _rItemSet.GetItemIfSet( ITEMID_CASEMAP ) )
            lcl_pushBack( _out_rProperties, PROPERTY_CHARCASEMAP, uno::Any( pCaseMapItem->GetEnumValue() ) );
        struct Items {
                TypedWhichId<SvxLanguageItem> nWhich;
                OUString sPropertyName;
        };
        const Items pItems[] = { {ITEMID_LANGUAGE,PROPERTY_CHARLOCALE}
                                ,{ITEMID_LANGUAGE_ASIAN,PROPERTY_CHARLOCALEASIAN}
                                ,{ITEMID_LANGUAGE_COMPLEX,PROPERTY_CHARLOCALECOMPLEX}
        };
        for(const auto & k : pItems)
        {
            if ( const SvxLanguageItem* pLanguageItem = _rItemSet.GetItemIfSet( k.nWhich ) )
            {
                lang::Locale aCharLocale( LanguageTag( pLanguageItem->GetLanguage()).getLocale());
                lcl_pushBack( _out_rProperties, k.sPropertyName, uno::Any( aCharLocale ) );
            }
        }
        if ( const SvxEscapementItem* pEscapementItem = _rItemSet.GetItemIfSet( ITEMID_ESCAPEMENT ) )
        {
            lcl_pushBack( _out_rProperties, PROPERTY_CHARESCAPEMENT, uno::Any( pEscapementItem->GetEsc() ) );
            lcl_pushBack( _out_rProperties, PROPERTY_CHARESCAPEMENTHEIGHT, uno::Any(static_cast<sal_Int8>(pEscapementItem->GetProportionalHeight())) );
        }
    }


    template< class ATTRIBUTE_TYPE >
    void lcl_applyFontAttribute( const ::comphelper::NamedValueCollection& _rAttrValues, const OUString& _pAttributeName,
        const uno::Reference<report::XReportControlFormat >& _rxReportControlFormat,
        void (SAL_CALL report::XReportControlFormat::*pSetter)( ATTRIBUTE_TYPE ) )
    {
        ATTRIBUTE_TYPE aAttributeValue = ATTRIBUTE_TYPE();
        if ( _rAttrValues.get_ensureType( _pAttributeName, aAttributeValue ) )
            (_rxReportControlFormat.get()->*pSetter)( aAttributeValue );
    }


    void lcl_applyFontAttribute( const ::comphelper::NamedValueCollection& _rAttrValues, const OUString& _pAttributeName,
        const uno::Reference<report::XReportControlFormat >& _rxReportControlFormat,
        void (SAL_CALL report::XReportControlFormat::*pSetter)( const OUString& ) )
    {
        OUString aAttributeValue;
        if ( _rAttrValues.get_ensureType( _pAttributeName, aAttributeValue ) )
            (_rxReportControlFormat.get()->*pSetter)( aAttributeValue );
    }


    void lcl_applyFontAttribute( const ::comphelper::NamedValueCollection& _rAttrValues, const OUString& _pAttributeName,
        const uno::Reference<report::XReportControlFormat >& _rxReportControlFormat,
        void (SAL_CALL report::XReportControlFormat::*pSetter)( const lang::Locale& ) )
    {
        lang::Locale aAttributeValue;
        if ( _rAttrValues.get_ensureType( _pAttributeName, aAttributeValue ) )
            (_rxReportControlFormat.get()->*pSetter)( aAttributeValue );
    }
}

static ItemInfoPackage& getItemInfoPackageOpenCharDlg()
{
    class ItemInfoPackageOpenCharDlg : public ItemInfoPackage
    {
        typedef std::array<ItemInfoStatic, ITEMID_WEIGHT_COMPLEX - XATTR_FILL_FIRST + 1> ItemInfoArrayOpenCharDlg;
        // const ::Color aNullFillCol(COL_DEFAULT_SHAPE_FILLING); // #i121448# Use defined default color
        // const ::Color aNullLineCol(COL_DEFAULT_SHAPE_STROKE); // #i121448# Use defined default color
        // const basegfx::BGradient aNullGrad;
        // const XHatch aNullHatch(Color(COL_DEFAULT_SHAPE_STROKE));
        ItemInfoArrayOpenCharDlg maItemInfos {{
            // m_nWhich, m_pItem, m_nSlotID, m_nItemInfoFlags
            { XATTR_FILLSTYLE, new XFillStyleItem, 0, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FILLCOLOR, new XFillColorItem("", COL_DEFAULT_SHAPE_FILLING), 0, SFX_ITEMINFOFLAG_SUPPORT_SURROGATE },
            { XATTR_FILLGRADIENT, new XFillGradientItem(basegfx::BGradient()), 0, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FILLHATCH, new XFillHatchItem(COL_DEFAULT_SHAPE_STROKE), 0, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FILLBITMAP, nullptr, 0, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FILLTRANSPARENCE, new XFillTransparenceItem, 0, SFX_ITEMINFOFLAG_NONE },
            { XATTR_GRADIENTSTEPCOUNT, new XGradientStepCountItem, 0, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FILLBMP_TILE, new XFillBmpTileItem, 0, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FILLBMP_POS, new XFillBmpPosItem, 0, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FILLBMP_SIZEX, new XFillBmpSizeXItem, 0, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FILLBMP_SIZEY, new XFillBmpSizeYItem, 0, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FILLFLOATTRANSPARENCE, new XFillFloatTransparenceItem(basegfx::BGradient(), false), 0, SFX_ITEMINFOFLAG_NONE },
            { XATTR_SECONDARYFILLCOLOR, new XSecondaryFillColorItem("", COL_DEFAULT_SHAPE_FILLING), 0, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FILLBMP_SIZELOG, new XFillBmpSizeLogItem, 0, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FILLBMP_TILEOFFSETX, new XFillBmpTileOffsetXItem, 0, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FILLBMP_TILEOFFSETY, new XFillBmpTileOffsetYItem, 0, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FILLBMP_STRETCH, new XFillBmpStretchItem, 0, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FILLBMP_POSOFFSETX, new XFillBmpPosOffsetXItem, 0, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FILLBMP_POSOFFSETY, new XFillBmpPosOffsetYItem, 0, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FILLBACKGROUND, new XFillBackgroundItem, 0, SFX_ITEMINFOFLAG_NONE },

            { ITEMID_FONT, new SvxFontItem(ITEMID_FONT), SID_ATTR_CHAR_FONT, SFX_ITEMINFOFLAG_NONE },
            { ITEMID_FONTHEIGHT, new SvxFontHeightItem(240,100,ITEMID_FONTHEIGHT), SID_ATTR_CHAR_FONTHEIGHT, SFX_ITEMINFOFLAG_NONE },
            { ITEMID_LANGUAGE, new SvxLanguageItem(LANGUAGE_GERMAN,ITEMID_LANGUAGE), SID_ATTR_CHAR_LANGUAGE, SFX_ITEMINFOFLAG_NONE },
            { ITEMID_POSTURE, new SvxPostureItem(ITALIC_NONE,ITEMID_POSTURE), SID_ATTR_CHAR_POSTURE, SFX_ITEMINFOFLAG_NONE },
            { ITEMID_WEIGHT, new SvxWeightItem(WEIGHT_NORMAL,ITEMID_WEIGHT), SID_ATTR_CHAR_WEIGHT, SFX_ITEMINFOFLAG_NONE },
            { ITEMID_SHADOWED, new SvxShadowedItem(false,ITEMID_SHADOWED), SID_ATTR_CHAR_SHADOWED, SFX_ITEMINFOFLAG_NONE },
            { ITEMID_WORDLINEMODE, new SvxWordLineModeItem(false,ITEMID_WORDLINEMODE), SID_ATTR_CHAR_WORDLINEMODE, SFX_ITEMINFOFLAG_NONE },
            { ITEMID_CONTOUR, new SvxContourItem(false,ITEMID_CONTOUR), SID_ATTR_CHAR_CONTOUR, SFX_ITEMINFOFLAG_NONE },
            { ITEMID_CROSSEDOUT, new SvxCrossedOutItem(STRIKEOUT_NONE,ITEMID_CROSSEDOUT), SID_ATTR_CHAR_STRIKEOUT, SFX_ITEMINFOFLAG_NONE },
            { ITEMID_UNDERLINE, new SvxUnderlineItem(LINESTYLE_NONE,ITEMID_UNDERLINE), SID_ATTR_CHAR_UNDERLINE, SFX_ITEMINFOFLAG_NONE },
            { ITEMID_COLOR, new SvxColorItem(ITEMID_COLOR), SID_ATTR_CHAR_COLOR, SFX_ITEMINFOFLAG_NONE },
            { ITEMID_KERNING, new SvxKerningItem(0,ITEMID_KERNING), SID_ATTR_CHAR_KERNING, SFX_ITEMINFOFLAG_NONE },
            { ITEMID_CASEMAP, new SvxCaseMapItem(SvxCaseMap::NotMapped,ITEMID_CASEMAP), SID_ATTR_CHAR_CASEMAP, SFX_ITEMINFOFLAG_NONE },
            { ITEMID_ESCAPEMENT, new SvxEscapementItem(ITEMID_ESCAPEMENT), SID_ATTR_CHAR_ESCAPEMENT, SFX_ITEMINFOFLAG_NONE },

            // needs to be a on-demand item created in the callback
            { ITEMID_FONTLIST, nullptr, SID_ATTR_CHAR_FONTLIST, SFX_ITEMINFOFLAG_NONE },

            { ITEMID_AUTOKERN, new SvxAutoKernItem(false,ITEMID_AUTOKERN), SID_ATTR_CHAR_AUTOKERN, SFX_ITEMINFOFLAG_NONE },
            { ITEMID_COLOR_TABLE, new SvxColorListItem(XColorList::CreateStdColorList(),ITEMID_COLOR_TABLE), SID_COLOR_TABLE, SFX_ITEMINFOFLAG_NONE },
            { ITEMID_BLINK, new SvxBlinkItem(false,ITEMID_BLINK), SID_ATTR_FLASH, SFX_ITEMINFOFLAG_NONE },
            { ITEMID_EMPHASISMARK, new SvxEmphasisMarkItem(FontEmphasisMark::NONE,ITEMID_EMPHASISMARK), SID_ATTR_CHAR_EMPHASISMARK, SFX_ITEMINFOFLAG_NONE },
            { ITEMID_TWOLINES, new SvxTwoLinesItem(true,0,0,ITEMID_TWOLINES), SID_ATTR_CHAR_TWO_LINES, SFX_ITEMINFOFLAG_NONE },
            { ITEMID_CHARROTATE, new SvxCharRotateItem(0_deg10,false,ITEMID_CHARROTATE), SID_ATTR_CHAR_ROTATED, SFX_ITEMINFOFLAG_NONE },
            { ITEMID_CHARSCALE_W, new SvxCharScaleWidthItem(100,ITEMID_CHARSCALE_W), SID_ATTR_CHAR_SCALEWIDTH, SFX_ITEMINFOFLAG_NONE },
            { ITEMID_CHARRELIEF, new SvxCharReliefItem(FontRelief::NONE,ITEMID_CHARRELIEF), SID_ATTR_CHAR_RELIEF, SFX_ITEMINFOFLAG_NONE },
            { ITEMID_CHARHIDDEN, new SvxCharHiddenItem(false,ITEMID_CHARHIDDEN), SID_ATTR_CHAR_HIDDEN, SFX_ITEMINFOFLAG_NONE },
            { ITEMID_BRUSH, new SvxBrushItem(ITEMID_BRUSH), SID_ATTR_BRUSH, SFX_ITEMINFOFLAG_NONE },
            { ITEMID_HORJUSTIFY, new SvxHorJustifyItem(ITEMID_HORJUSTIFY), SID_ATTR_ALIGN_HOR_JUSTIFY, SFX_ITEMINFOFLAG_NONE },
            { ITEMID_VERJUSTIFY, new SvxVerJustifyItem(ITEMID_VERJUSTIFY), SID_ATTR_ALIGN_VER_JUSTIFY, SFX_ITEMINFOFLAG_NONE },

            // Asian
            { ITEMID_FONT_ASIAN , new SvxFontItem(ITEMID_FONT_ASIAN), SID_ATTR_CHAR_CJK_FONT, SFX_ITEMINFOFLAG_NONE },
            { ITEMID_FONTHEIGHT_ASIAN , new SvxFontHeightItem(240,100,ITEMID_FONTHEIGHT_ASIAN), SID_ATTR_CHAR_CJK_FONTHEIGHT, SFX_ITEMINFOFLAG_NONE },
            { ITEMID_LANGUAGE_ASIAN , new SvxLanguageItem(LANGUAGE_GERMAN,ITEMID_LANGUAGE_ASIAN), SID_ATTR_CHAR_CJK_LANGUAGE, SFX_ITEMINFOFLAG_NONE },
            { ITEMID_POSTURE_ASIAN , new SvxPostureItem(ITALIC_NONE,ITEMID_POSTURE_ASIAN), SID_ATTR_CHAR_CJK_POSTURE, SFX_ITEMINFOFLAG_NONE },
            { ITEMID_WEIGHT_ASIAN , new SvxWeightItem(WEIGHT_NORMAL,ITEMID_WEIGHT_ASIAN), SID_ATTR_CHAR_CJK_WEIGHT, SFX_ITEMINFOFLAG_NONE },

            // Complex
            { ITEMID_FONT_COMPLEX , new SvxFontItem(ITEMID_FONT_COMPLEX), SID_ATTR_CHAR_CTL_FONT, SFX_ITEMINFOFLAG_NONE },
            { ITEMID_FONTHEIGHT_COMPLEX , new SvxFontHeightItem(240,100,ITEMID_FONTHEIGHT_COMPLEX), SID_ATTR_CHAR_CTL_FONTHEIGHT, SFX_ITEMINFOFLAG_NONE },
            { ITEMID_LANGUAGE_COMPLEX , new SvxLanguageItem(LANGUAGE_GERMAN,ITEMID_LANGUAGE_COMPLEX), SID_ATTR_CHAR_CTL_LANGUAGE, SFX_ITEMINFOFLAG_NONE },
            { ITEMID_POSTURE_COMPLEX , new SvxPostureItem(ITALIC_NONE,ITEMID_POSTURE_COMPLEX), SID_ATTR_CHAR_CTL_POSTURE, SFX_ITEMINFOFLAG_NONE },
            { ITEMID_WEIGHT_COMPLEX , new SvxWeightItem(WEIGHT_NORMAL,ITEMID_WEIGHT_COMPLEX), SID_ATTR_CHAR_CTL_WEIGHT, SFX_ITEMINFOFLAG_NONE }
        }};

        virtual const ItemInfoStatic& getItemInfoStatic(size_t nIndex) const override { return maItemInfos[nIndex]; }

    public:
        virtual size_t size() const override { return maItemInfos.size(); }
        virtual const ItemInfo& getItemInfo(size_t nIndex, SfxItemPool& /*rPool*/) override
        {
            const ItemInfo& rRetval(maItemInfos[nIndex]);

            // return immediately if we have the static entry and Item
            if (nullptr != rRetval.getItem())
                return rRetval;

            if (XATTR_FILLBITMAP == rRetval.getWhich())
                return *new ItemInfoDynamic(rRetval, new XFillBitmapItem(Graphic()));

            // return in any case
            return rRetval;
        }
    };

    static std::unique_ptr<ItemInfoPackageOpenCharDlg> g_aItemInfoPackageOpenCharDlg;
    if (!g_aItemInfoPackageOpenCharDlg)
        g_aItemInfoPackageOpenCharDlg.reset(new ItemInfoPackageOpenCharDlg);
    return *g_aItemInfoPackageOpenCharDlg;
}

bool openCharDialog( const uno::Reference<report::XReportControlFormat >& _rxReportControlFormat,
        const uno::Reference< awt::XWindow>& _rxParentWindow, uno::Sequence< beans::NamedValue >& _out_rNewValues )
{
    OSL_PRECOND( _rxReportControlFormat.is() && _rxParentWindow.is(), "openCharDialog: invalid parameters!" );
    if ( !_rxReportControlFormat.is() || !_rxParentWindow.is() )
        return false;

    _out_rNewValues = uno::Sequence< beans::NamedValue >();

    // UNO->ItemSet
    static const WhichRangesContainer pRanges(svl::Items<
        XATTR_FILLSTYLE, XATTR_FILLBACKGROUND,
        ITEMID_FONT, ITEMID_WEIGHT_COMPLEX
    >);

    rtl::Reference<SfxItemPool> pPool(new SfxItemPool("ReportCharProperties"));
    // not needed for font height pPool->SetDefaultMetric( MapUnit::Map100thMM );  // ripped, don't understand why

    // here we have to use the callback to create all needed default entries.
    // this uses local aFontList for temporary SvxFontListItem
    FontList aFontList(Application::GetDefaultDevice());
    pPool->registerItemInfoPackage(
        getItemInfoPackageOpenCharDlg(),
        [&aFontList](sal_uInt16 nWhich)
        {
            SfxPoolItem* pRetval(nullptr);
            if (ITEMID_FONTLIST == nWhich)
                pRetval = new SvxFontListItem(&aFontList, ITEMID_FONTLIST);
            return pRetval;
        });

    bool bSuccess = false;
    try
    {
        SfxItemSet aDescriptor( *pPool, pRanges );
        lcl_CharPropertiesToItems( _rxReportControlFormat, aDescriptor );

        {   // want the dialog to be destroyed before our set
            ORptPageDialog aDlg(Application::GetFrameWeld(_rxParentWindow), &aDescriptor, "CharDialog");
            uno::Reference< report::XShape > xShape( _rxReportControlFormat, uno::UNO_QUERY );
            if ( xShape.is() )
                aDlg.RemoveTabPage("background");
            bSuccess = aDlg.run() == RET_OK;
            if ( bSuccess )
            {
                lcl_itemsToCharProperties( lcl_getReportControlFont( _rxReportControlFormat,WESTERN ),
                    lcl_getReportControlFont( _rxReportControlFormat,ASIAN ),
                    lcl_getReportControlFont( _rxReportControlFormat,COMPLEX ), *aDlg.GetOutputItemSet(), _out_rNewValues );
            }
        }
    }
    catch(uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("reportdesign");
    }

    pPool.clear();

    return bSuccess;
}

bool openAreaDialog( const uno::Reference<report::XShape >& _xShape,const uno::Reference< awt::XWindow>& _rxParentWindow )
{
    OSL_PRECOND( _xShape.is() && _rxParentWindow.is(), "openAreaDialog: invalid parameters!" );
    if ( !_xShape.is() || !_rxParentWindow.is() )
        return false;

    std::shared_ptr<rptui::OReportModel> pModel  = ::reportdesign::OReportDefinition::getSdrModel(_xShape->getSection()->getReportDefinition());

    weld::Window* pParent = Application::GetFrameWeld(_rxParentWindow);

    bool bSuccess = false;
    try
    {
        SfxItemPool& rItemPool = pModel->GetItemPool();
        SfxItemSet aDescriptor( rItemPool, rItemPool.GetFirstWhich(), rItemPool.GetLastWhich() );
        lcl_fillShapeToItems(_xShape, aDescriptor);

        {   // want the dialog to be destroyed before our set
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            ScopedVclPtr<AbstractSvxAreaTabDialog> pDialog(
                pFact->CreateSvxAreaTabDialog(pParent, &aDescriptor, pModel.get(), true, false));
            if ( RET_OK == pDialog->Execute() )
            {
                bSuccess = true;
                lcl_fillItemsToShape(_xShape,*pDialog->GetOutputItemSet());
            }
        }
    }
    catch(uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("reportdesign");
    }

    return bSuccess;
}


void applyCharacterSettings( const uno::Reference< report::XReportControlFormat >& _rxReportControlFormat, const uno::Sequence< beans::NamedValue >& _rSettings )
{
    ::comphelper::NamedValueCollection aSettings( _rSettings );

    try
    {
        awt::FontDescriptor aAwtFont;
        if ( aSettings.get( "Font" ) >>= aAwtFont )
        {
            OUString sTemp = aAwtFont.Name;
            aAwtFont.Name.clear(); // hack to
            _rxReportControlFormat->setFontDescriptor( aAwtFont );
            _rxReportControlFormat->setCharFontName( sTemp );
        }
        if ( aSettings.get( "FontAsian" ) >>= aAwtFont )
        {
            OUString sTemp = aAwtFont.Name;
            aAwtFont.Name.clear(); // hack to
            _rxReportControlFormat->setFontDescriptorAsian( aAwtFont );
            _rxReportControlFormat->setCharFontNameAsian( sTemp );
        }
        if ( aSettings.get( "FontComplex" ) >>= aAwtFont )
        {
            OUString sTemp = aAwtFont.Name;
            aAwtFont.Name.clear(); // hack to
            _rxReportControlFormat->setFontDescriptorComplex( aAwtFont );
            _rxReportControlFormat->setCharFontNameComplex( sTemp );
        }

        lcl_applyFontAttribute( aSettings, PROPERTY_CHARSHADOWED, _rxReportControlFormat, &report::XReportControlFormat::setCharShadowed );
        lcl_applyFontAttribute( aSettings, PROPERTY_CHARCONTOURED, _rxReportControlFormat, &report::XReportControlFormat::setCharContoured );
        lcl_applyFontAttribute( aSettings, PROPERTY_CHARUNDERLINECOLOR, _rxReportControlFormat, &report::XReportControlFormat::setCharUnderlineColor );
        lcl_applyFontAttribute( aSettings, PROPERTY_PARAADJUST, _rxReportControlFormat, &report::XReportControlFormat::setParaAdjust );
        lcl_applyFontAttribute( aSettings, PROPERTY_VERTICALALIGN, _rxReportControlFormat, &report::XReportControlFormat::setVerticalAlign );
        lcl_applyFontAttribute( aSettings, PROPERTY_CHARRELIEF, _rxReportControlFormat, &report::XReportControlFormat::setCharRelief );
        lcl_applyFontAttribute( aSettings, PROPERTY_CHARHIDDEN, _rxReportControlFormat, &report::XReportControlFormat::setCharHidden );
        lcl_applyFontAttribute( aSettings, PROPERTY_CHARAUTOKERNING, _rxReportControlFormat, &report::XReportControlFormat::setCharAutoKerning );
        lcl_applyFontAttribute( aSettings, PROPERTY_CONTROLBACKGROUND, _rxReportControlFormat, &report::XReportControlFormat::setControlBackground );
        lcl_applyFontAttribute( aSettings, PROPERTY_CHARFLASH, _rxReportControlFormat, &report::XReportControlFormat::setCharFlash );
        lcl_applyFontAttribute( aSettings, PROPERTY_CHAREMPHASIS, _rxReportControlFormat, &report::XReportControlFormat::setCharEmphasis );
        lcl_applyFontAttribute( aSettings, PROPERTY_CHARCOMBINEISON, _rxReportControlFormat, &report::XReportControlFormat::setCharCombineIsOn );
        lcl_applyFontAttribute( aSettings, PROPERTY_CHARCOMBINEPREFIX, _rxReportControlFormat, &report::XReportControlFormat::setCharCombinePrefix );
        lcl_applyFontAttribute( aSettings, PROPERTY_CHARCOMBINESUFFIX, _rxReportControlFormat, &report::XReportControlFormat::setCharCombineSuffix );
        lcl_applyFontAttribute( aSettings, PROPERTY_CHARCOLOR, _rxReportControlFormat, &report::XReportControlFormat::setCharColor );
        lcl_applyFontAttribute( aSettings, PROPERTY_CHARKERNING, _rxReportControlFormat, &report::XReportControlFormat::setCharKerning );
        lcl_applyFontAttribute( aSettings, PROPERTY_CHARCASEMAP, _rxReportControlFormat, &report::XReportControlFormat::setCharCaseMap );
        lcl_applyFontAttribute( aSettings, PROPERTY_CHARLOCALE, _rxReportControlFormat, &report::XReportControlFormat::setCharLocale );
        lcl_applyFontAttribute( aSettings, PROPERTY_CHARESCAPEMENT, _rxReportControlFormat, &report::XReportControlFormat::setCharEscapement );
        lcl_applyFontAttribute( aSettings, PROPERTY_CHARESCAPEMENTHEIGHT, _rxReportControlFormat, &report::XReportControlFormat::setCharEscapementHeight );
        lcl_applyFontAttribute( aSettings, PROPERTY_CHARLOCALEASIAN, _rxReportControlFormat, &report::XReportControlFormat::setCharLocaleAsian );
        lcl_applyFontAttribute( aSettings, PROPERTY_CHARLOCALECOMPLEX, _rxReportControlFormat, &report::XReportControlFormat::setCharLocaleComplex );
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("reportdesign");
    }
}

void notifySystemWindow(vcl::Window const * _pWindow, vcl::Window* _pToRegister, const ::comphelper::mem_fun1_t<TaskPaneList,vcl::Window*>& rMemFunc)
{
    OSL_ENSURE(_pWindow,"Window can not be null!");
    SystemWindow* pSystemWindow = _pWindow ? _pWindow->GetSystemWindow() : nullptr;
    if ( pSystemWindow )
    {
        rMemFunc( pSystemWindow->GetTaskPaneList(), _pToRegister );
    }
}

SdrObject* isOver(const tools::Rectangle& _rRect, SdrPage const & _rPage, SdrView const & _rView, bool _bAllObjects, SdrObject const * _pIgnore, sal_Int16 _nIgnoreType)
{
    SdrObject* pOverlappedObj = nullptr;
    SdrObjListIter aIter(&_rPage,SdrIterMode::DeepNoGroups);

    while( !pOverlappedObj )
    {
        SdrObject* pObjIter = aIter.Next();
        if( !pObjIter )
            break;
        if ( _pIgnore != pObjIter
            && (_bAllObjects || !_rView.IsObjMarked(pObjIter))
            && (dynamic_cast<OUnoObject*>(pObjIter) != nullptr || dynamic_cast<OOle2Obj*>(pObjIter) != nullptr))
        {
            if (_nIgnoreType == ISOVER_IGNORE_CUSTOMSHAPES && pObjIter->GetObjIdentifier() == SdrObjKind::CustomShape)
            {
                continue;
            }

            if (dynamic_cast<OUnoObject*>(pObjIter) != nullptr || dynamic_cast<OOle2Obj*>(pObjIter) != nullptr)
            {
                tools::Rectangle aRect = _rRect.GetIntersection(pObjIter->GetLastBoundRect());
                if ( !aRect.IsEmpty() && (aRect.Left() != aRect.Right() && aRect.Top() != aRect.Bottom() ) )
                    pOverlappedObj = pObjIter;
            }
        }
    }
    return pOverlappedObj;
}

static bool checkArrayForOccurrence(SdrObject const * _pObjToCheck, rtl::Reference<SdrUnoObj> _pIgnore[], int _nListLength)
{
    for(int i=0;i<_nListLength;i++)
    {
        SdrObject *pIgnore = _pIgnore[i].get();
        if (pIgnore == _pObjToCheck)
        {
            return true;
        }
    }
    return false;
}

SdrObject* isOver(const tools::Rectangle& _rRect,SdrPage const & _rPage,SdrView const & _rView,bool _bAllObjects, rtl::Reference<SdrUnoObj> _pIgnoreList[], int _nIgnoreListLength)
{
    SdrObject* pOverlappedObj = nullptr;
    SdrObjListIter aIter(&_rPage,SdrIterMode::DeepNoGroups);

    while( !pOverlappedObj )
    {
        SdrObject* pObjIter = aIter.Next();
        if( !pObjIter )
            break;
        if (checkArrayForOccurrence(pObjIter, _pIgnoreList, _nIgnoreListLength))
        {
            continue;
        }

        if ( (_bAllObjects || !_rView.IsObjMarked(pObjIter))
             && (dynamic_cast<OUnoObject*>(pObjIter) != nullptr || dynamic_cast<OOle2Obj*>(pObjIter) != nullptr) )
        {
            tools::Rectangle aRect = _rRect.GetIntersection(pObjIter->GetLastBoundRect());
            if ( !aRect.IsEmpty() && (aRect.Left() != aRect.Right() && aRect.Top() != aRect.Bottom() ) )
                pOverlappedObj = pObjIter;
        }
    }
    return pOverlappedObj;
}


SdrObject* isOver(SdrObject const * _pObj,SdrPage const & _rPage,SdrView const & _rView)
{
    SdrObject* pOverlappedObj = nullptr;
    if (dynamic_cast<OUnoObject const *>(_pObj) != nullptr || dynamic_cast<OOle2Obj const *>(_pObj) != nullptr) // this doesn't need to be done for shapes
    {
        tools::Rectangle aRect = _pObj->GetCurrentBoundRect();
        pOverlappedObj = isOver(aRect,_rPage,_rView,false/*_bUnMarkedObjects*/,_pObj);
    }
    return pOverlappedObj;
}


uno::Sequence< OUString > getParameterNames( const uno::Reference< sdbc::XRowSet >& _rxRowSet )
{
    uno::Sequence< OUString > aNames;

    try
    {
        uno::Reference< sdb::XParametersSupplier > xSuppParams( _rxRowSet, uno::UNO_QUERY_THROW );
        uno::Reference< container::XIndexAccess > xParams( xSuppParams->getParameters() );
        if ( xParams.is() )
        {
            sal_Int32 count( xParams->getCount() );
            aNames.realloc( count );
            auto pNames = aNames.getArray();

            uno::Reference< beans::XPropertySet > xParam;
            OUString sParamName;
            for ( sal_Int32 i=0; i<count; ++i )
            {
                xParam.set( xParams->getByIndex(i), uno::UNO_QUERY_THROW );
                OSL_VERIFY( xParam->getPropertyValue( PROPERTY_NAME ) >>= sParamName );
                pNames[i] = sParamName;
            }
        }
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("reportdesign");
    }

    return aNames;
}

tools::Rectangle getRectangleFromControl(SdrObject* _pControl)
{
    if (_pControl)
    {
        uno::Reference< report::XReportComponent > xComponent( _pControl->getUnoShape(), uno::UNO_QUERY);
        if (xComponent.is())
        {
            tools::Rectangle aRect(VCLPoint(xComponent->getPosition()),VCLSize(xComponent->getSize()));
            aRect.setHeight(aRect.getOpenHeight() + 1);
            aRect.setWidth(aRect.getOpenWidth() + 1);
            return aRect;
        }
    }
    return tools::Rectangle();
}

// check overlapping
void correctOverlapping(SdrObject* _pControl,OReportSection const & _aReportSection,bool _bInsert)
{
    OSectionView& rSectionView = _aReportSection.getSectionView();
    uno::Reference< report::XReportComponent> xComponent(_pControl->getUnoShape(),uno::UNO_QUERY);
    tools::Rectangle aRect = getRectangleFromControl(_pControl);

    bool bOverlapping = true;
    while ( bOverlapping )
    {
        SdrObject* pOverlappedObj = isOver(aRect,*_aReportSection.getPage(),rSectionView,true, _pControl);
        bOverlapping = pOverlappedObj != nullptr;
        if ( bOverlapping )
        {
            const tools::Rectangle& aLogicRect = pOverlappedObj->GetLogicRect();
            aRect.Move(0,aLogicRect.Top() + aLogicRect.getOpenHeight() - aRect.Top());
            xComponent->setPositionY(aRect.Top());
        }
    }
    if (_bInsert) // now insert objects
        rSectionView.InsertObjectAtView(_pControl,*rSectionView.GetSdrPageView(), SdrInsertFlags::ADDMARK);
}

void setZoomFactor(const Fraction& _aZoom, vcl::Window& _rWindow)
{
    MapMode aMapMode( _rWindow.GetMapMode() );
    aMapMode.SetScaleX(_aZoom);
    aMapMode.SetScaleY(_aZoom);
    _rWindow.SetMapMode(aMapMode);
}

bool openDialogFormula_nothrow( OUString& _in_out_rFormula
                               , const css::uno::Reference< css::uno::XComponentContext >& _xContext
                               , const uno::Reference< awt::XWindow>& _xInspectorWindow
                               , const css::uno::Reference < css::beans::XPropertySet >& _xRowSet
                               )
{
    OSL_PRECOND( _xInspectorWindow.is(), "openDialogFormula_nothrow: invalid parameters!" );
    if ( !_xInspectorWindow.is() )
        return false;
    bool bSuccess = false;
    ::dbtools::SQLExceptionInfo aErrorInfo;
    uno::Reference< awt::XWindow > xInspectorWindow;
    uno::Reference< lang::XMultiComponentFactory > xFactory;
    uno::Reference<lang::XMultiServiceFactory> xServiceFactory;
    try
    {
        xFactory = _xContext->getServiceManager();
        xServiceFactory.set(xFactory,uno::UNO_QUERY);

        uno::Reference< report::meta::XFunctionManager> xMgr(xFactory->createInstanceWithContext("org.libreoffice.report.pentaho.SOFunctionManager",_xContext),uno::UNO_QUERY);
        if ( xMgr.is() )
        {
            auto pFormulaManager = std::make_shared<FunctionManager>(xMgr);
            ReportFormula aFormula( _in_out_rFormula );

            CharClass aCC(_xContext, LanguageTag(LANGUAGE_SYSTEM));
            svl::SharedStringPool aStringPool(aCC);

            FormulaDialog aDlg(
                Application::GetFrameWeld(_xInspectorWindow), xServiceFactory, pFormulaManager,
                aFormula.getUndecoratedContent(), _xRowSet, aStringPool);

            bSuccess = aDlg.run() == RET_OK;
            if ( bSuccess )
            {
                OUString sFormula = aDlg.getCurrentFormula();
                if ( sFormula[0] == '=' )
                    _in_out_rFormula = OUString::Concat("rpt:") + sFormula.subView(1);
                else
                    _in_out_rFormula = "rpt:" + sFormula;
            }
        }
    }
    catch (const sdb::SQLContext& e) { aErrorInfo = e; }
    catch (const sdbc::SQLWarning& e) { aErrorInfo = e; }
    catch (const sdbc::SQLException& e) { aErrorInfo = e; }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION( "reportdesign", "GeometryHandler::impl_dialogFilter_nothrow" );
    }

    if ( aErrorInfo.isValid() )
        ::dbtools::showError( aErrorInfo, xInspectorWindow, _xContext );

    return bSuccess;
}

} // namespace rptui


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
