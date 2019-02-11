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

#include <tools/diagnose_ex.h>

#include <vcl/svapp.hxx>
#include <vcl/unohelp.hxx>
#include <vcl/window.hxx>
#include <vcl/settings.hxx>

#include <com/sun/star/lang/NullPointerException.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <svx/svdpool.hxx>

#include <editeng/charscaleitem.hxx>
#include <svx/algitem.hxx>
#include <svx/svdpagv.hxx>
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
#include <svx/xgrscit.hxx>
#include <svx/svditer.hxx>
#include <svx/xtable.hxx>
#include <svx/dialogs.hrc>
#include <svx/svdview.hxx>
#include <svx/svdpage.hxx>
#include <svx/svxdlg.hxx>
#include <svx/unoprov.hxx>

#include <unotools/pathoptions.hxx>
#include <unotools/charclass.hxx>
#include <svtools/ctrltool.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <svl/sharedstringpool.hxx>

#include <comphelper/propmultiplex.hxx>
#include <comphelper/namedvaluecollection.hxx>

#include <connectivity/dbexception.hxx>
#include <connectivity/dbconversion.hxx>
#include <connectivity/dbtools.hxx>

#include <com/sun/star/awt/TextAlign.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/report/XShape.hpp>
#include <com/sun/star/sdb/XParametersSupplier.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <i18nlangtag/languagetag.hxx>
#include <dlgpage.hxx>
#include <rptui_slotid.hrc>
#include <strings.hxx>
#include <core_resource.hxx>
#include <RptObject.hxx>
#include <RptDef.hxx>
#include <strings.hrc>
#include <ReportDefinition.hxx>
#include <RptModel.hxx>

#define ITEMID_FONT                     10
#define ITEMID_FONTHEIGHT               11
#define ITEMID_LANGUAGE                 12

#define ITEMID_POSTURE                  13
#define ITEMID_WEIGHT                   14
#define ITEMID_SHADOWED                 15
#define ITEMID_WORDLINEMODE             16
#define ITEMID_CONTOUR                  17
#define ITEMID_CROSSEDOUT               18
#define ITEMID_UNDERLINE                19

#define ITEMID_COLOR                    20
#define ITEMID_KERNING                  21
#define ITEMID_CASEMAP                  22

#define ITEMID_ESCAPEMENT               23
#define ITEMID_FONTLIST                 24
#define ITEMID_AUTOKERN                 25
#define ITEMID_COLOR_TABLE              26
#define ITEMID_BLINK                    27
#define ITEMID_EMPHASISMARK             28
#define ITEMID_TWOLINES                 29
#define ITEMID_CHARROTATE               30
#define ITEMID_CHARSCALE_W              31
#define ITEMID_CHARRELIEF               32
#define ITEMID_CHARHIDDEN               33
#define ITEMID_BRUSH                    34
#define ITEMID_HORJUSTIFY               35
#define ITEMID_VERJUSTIFY               36
#define ITEMID_FONT_ASIAN               37
#define ITEMID_FONTHEIGHT_ASIAN         38
#define ITEMID_LANGUAGE_ASIAN           39
#define ITEMID_POSTURE_ASIAN            40
#define ITEMID_WEIGHT_ASIAN             41
#define ITEMID_FONT_COMPLEX             42
#define ITEMID_FONTHEIGHT_COMPLEX       43
#define ITEMID_LANGUAGE_COMPLEX         44
#define ITEMID_POSTURE_COMPLEX          45
#define ITEMID_WEIGHT_COMPLEX           46

#define WESTERN 0
#define ASIAN   1
#define COMPLEX 2

namespace rptui
{
using namespace ::com::sun::star;
using namespace formula;

void adjustSectionName(const uno::Reference< report::XGroup >& _xGroup,sal_Int32 _nPos)
{
    OSL_ENSURE(_xGroup.is(),"Group is NULL -> GPF");
    if ( _xGroup->getHeaderOn() && _xGroup->getHeader()->getName().isEmpty() )
    {
        OUString sName = RptResId(RID_STR_GROUPHEADER);
        sName += OUString::number(_nPos);
        _xGroup->getHeader()->setName(sName);
    }

    if ( _xGroup->getFooterOn() && _xGroup->getFooter()->getName().isEmpty() )
    {
        OUString sName = RptResId(RID_STR_GROUPFOOTER);
        sName += OUString::number(_nPos);
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

    const vcl::Font lcl_setFont(const uno::Reference<report::XReportControlFormat >& _rxReportControlFormat,
        SfxItemSet& _rItemSet,sal_uInt16 _nWhich,sal_uInt16 _nFont, sal_uInt16 _nFontHeight,sal_uInt16 _nLanguage,sal_uInt16 _nPosture, sal_uInt16 _nWeight)
    {
        // fill it
        awt::FontDescriptor aControlFont;
        const vcl::Font aFont( lcl_getReportControlFont( _rxReportControlFormat, aControlFont,_nWhich ) );

        SvxFontItem aFontItem(_nFont);
        aFontItem.PutValue( uno::makeAny( aControlFont ), 0 );
        _rItemSet.Put(aFontItem);

        _rItemSet.Put(SvxFontHeightItem(OutputDevice::LogicToLogic(Size(0, aFont.GetFontHeight()), MapMode(MapUnit::MapPoint), MapMode(MapUnit::MapTwip)).Height(), 100, _nFontHeight));
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
        PropertyEntryVector_t aPropVector = rPropertyMap.getPropertyEntries();
        for (const auto& rProp : aPropVector)
        {
            if ( xInfo->hasPropertyByName(rProp.sName) )
            {
                const SfxPoolItem* pItem = _rItemSet.GetItem(rProp.nWID);
                if ( pItem )
                {
                    ::std::unique_ptr<SfxPoolItem> pClone(pItem->Clone());
                    pClone->PutValue(_xShape->getPropertyValue(rProp.sName), rProp.nMemberId);
                    pClone->SetWhich(rProp.nWID);
                    _rItemSet.Put(*pClone);
                }
            }
        }
    }

    void lcl_fillItemsToShape( const uno::Reference<report::XShape >& _xShape,const SfxItemSet& _rItemSet )
    {
        const uno::Reference< beans::XPropertySetInfo> xInfo = _xShape->getPropertySetInfo();
        SvxUnoPropertyMapProvider aMap;
        const SfxItemPropertyMap& rPropertyMap = aMap.GetPropertySet(SVXMAP_CUSTOMSHAPE, SdrObject::GetGlobalDrawObjectItemPool())->getPropertyMap();
        PropertyEntryVector_t aPropVector = rPropertyMap.getPropertyEntries();
        for (const auto& rProp : aPropVector)
        {
            if ( SfxItemState::SET == _rItemSet.GetItemState(rProp.nWID) && xInfo->hasPropertyByName(rProp.sName) )
            {
                if ( ( rProp.nFlags & beans::PropertyAttribute::READONLY ) != beans::PropertyAttribute::READONLY )
                {
                    const SfxPoolItem* pItem = _rItemSet.GetItem(rProp.nWID);
                    if ( pItem )
                    {
                        uno::Any aValue;
                        pItem->QueryValue(aValue,rProp.nMemberId);
                        try
                        {
                            _xShape->setPropertyValue(rProp.sName, aValue);
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
        aUnderLineItem.SetColor(Color(_rxReportControlFormat->getCharUnderlineColor()));
        _rItemSet.Put(aUnderLineItem);
        _rItemSet.Put(SvxKerningItem(_rxReportControlFormat->getCharKerning(),ITEMID_KERNING));
        _rItemSet.Put(SvxEmphasisMarkItem(static_cast<FontEmphasisMark>(_rxReportControlFormat->getCharEmphasis()),ITEMID_EMPHASISMARK));
        _rItemSet.Put(SvxCharReliefItem(static_cast<FontRelief>(_rxReportControlFormat->getCharRelief()),ITEMID_CHARRELIEF));
        _rItemSet.Put(SvxColorItem(::Color(_rxReportControlFormat->getCharColor()),ITEMID_COLOR));
        _rItemSet.Put(SvxCharRotateItem(_rxReportControlFormat->getCharRotation(),false,ITEMID_CHARROTATE));
        _rItemSet.Put(SvxCharScaleWidthItem(_rxReportControlFormat->getCharScaleWidth(),ITEMID_CHARSCALE_W));

        SvxHorJustifyItem aHorJustifyItem(ITEMID_HORJUSTIFY);
        aHorJustifyItem.PutValue(xSet->getPropertyValue(PROPERTY_PARAADJUST),MID_HORJUST_ADJUST);
        _rItemSet.Put(aHorJustifyItem);
        SvxVerJustifyItem aVerJustifyItem(ITEMID_VERJUSTIFY);
        aVerJustifyItem.PutValue(xSet->getPropertyValue(PROPERTY_VERTICALALIGN),MID_HORJUST_ADJUST);
        _rItemSet.Put(aVerJustifyItem);

        uno::Reference< report::XShape> xShape(_rxReportControlFormat,uno::UNO_QUERY);
        if ( !xShape.is() )
            _rItemSet.Put(SvxBrushItem(::Color(_rxReportControlFormat->getControlBackground()),ITEMID_BRUSH));

        lcl_setFont(_rxReportControlFormat, _rItemSet,ASIAN,ITEMID_FONT_ASIAN,ITEMID_FONTHEIGHT_ASIAN,ITEMID_LANGUAGE_ASIAN,ITEMID_POSTURE_ASIAN,ITEMID_WEIGHT_ASIAN );
        lcl_setFont(_rxReportControlFormat, _rItemSet,COMPLEX,ITEMID_FONT_COMPLEX,ITEMID_FONTHEIGHT_COMPLEX,ITEMID_LANGUAGE_COMPLEX,ITEMID_POSTURE_COMPLEX,ITEMID_WEIGHT_COMPLEX );
    }


    void lcl_pushBack( uno::Sequence< beans::NamedValue >& _out_rProperties, const OUString& _sName, const uno::Any& _rValue )
    {
        sal_Int32 nLen( _out_rProperties.getLength() );
        _out_rProperties.realloc( nLen + 1 );
        _out_rProperties[ nLen ] = beans::NamedValue( _sName, _rValue );
    }


    void lcl_initAwtFont( const vcl::Font& _rOriginalFont, const SfxItemSet& _rItemSet, awt::FontDescriptor& _out_rAwtFont,
        sal_uInt16 _nFont, sal_uInt16 _nFontHeight,sal_uInt16 _nPosture, sal_uInt16 _nWeight)
    {
        vcl::Font aNewFont( _rOriginalFont );
        const SfxPoolItem* pItem( nullptr );
        if ( SfxItemState::SET == _rItemSet.GetItemState( _nFont,true,&pItem) && dynamic_cast< const SvxFontItem *>( pItem ) !=  nullptr)
        {
            const SvxFontItem* pFontItem = static_cast<const SvxFontItem*>(pItem);
            aNewFont.SetFamilyName(pFontItem->GetFamilyName());
            aNewFont.SetStyleName(pFontItem->GetStyleName());
            aNewFont.SetFamily(pFontItem->GetFamily());
            aNewFont.SetPitch(pFontItem->GetPitch());
            aNewFont.SetCharSet(pFontItem->GetCharSet());
        }
        if ( SfxItemState::SET == _rItemSet.GetItemState( _nFontHeight,true,&pItem) && dynamic_cast< const SvxFontHeightItem *>( pItem ) !=  nullptr)
        {
            const SvxFontHeightItem* pFontItem = static_cast<const SvxFontHeightItem*>(pItem);
            aNewFont.SetFontHeight(OutputDevice::LogicToLogic(Size(0, pFontItem->GetHeight()), MapMode(MapUnit::MapTwip), MapMode(MapUnit::MapPoint)).Height());
        }
        if ( SfxItemState::SET == _rItemSet.GetItemState( _nPosture,true,&pItem) && dynamic_cast< const SvxPostureItem *>( pItem ) !=  nullptr)
        {
            const SvxPostureItem* pFontItem = static_cast<const SvxPostureItem*>(pItem);
            aNewFont.SetItalic(pFontItem->GetPosture());
        }
        if ( SfxItemState::SET == _rItemSet.GetItemState( _nWeight,true,&pItem) && dynamic_cast< const SvxWeightItem *>( pItem ) !=  nullptr)
        {
            const SvxWeightItem* pFontItem = static_cast<const SvxWeightItem*>(pItem);
            aNewFont.SetWeight(pFontItem->GetWeight());
        }
        if ( SfxItemState::SET == _rItemSet.GetItemState( ITEMID_WORDLINEMODE,true,&pItem) && dynamic_cast< const SvxWordLineModeItem *>( pItem ) !=  nullptr)
        {
            const SvxWordLineModeItem* pFontItem = static_cast<const SvxWordLineModeItem*>(pItem);
            aNewFont.SetWordLineMode(pFontItem->GetValue());
        }
        if ( SfxItemState::SET == _rItemSet.GetItemState( ITEMID_CROSSEDOUT,true,&pItem) && dynamic_cast< const SvxCrossedOutItem *>( pItem ) !=  nullptr)
        {
            const SvxCrossedOutItem* pFontItem = static_cast<const SvxCrossedOutItem*>(pItem);
            aNewFont.SetStrikeout(pFontItem->GetStrikeout());
        }

        if ( SfxItemState::SET == _rItemSet.GetItemState( ITEMID_CHARROTATE,true,&pItem) && dynamic_cast< const SvxCharRotateItem *>( pItem ) !=  nullptr)
        {
            const SvxCharRotateItem* pRotateItem = static_cast<const SvxCharRotateItem*>(pItem);
            aNewFont.SetOrientation(pRotateItem->GetValue());
        }
        if ( SfxItemState::SET == _rItemSet.GetItemState( ITEMID_CHARSCALE_W,true,&pItem) && dynamic_cast< const SvxCharScaleWidthItem *>( pItem ) !=  nullptr)
        {
            const SvxCharScaleWidthItem* pCharItem = static_cast<const SvxCharScaleWidthItem*>(pItem);
            aNewFont.SetWidthType(vcl::unohelper::ConvertFontWidth(pCharItem->GetValue()));
        }
        if ( SfxItemState::SET == _rItemSet.GetItemState( ITEMID_UNDERLINE,true,&pItem) && dynamic_cast< const SvxUnderlineItem *>( pItem ) !=  nullptr)
        {
            const SvxUnderlineItem* pFontItem = static_cast<const SvxUnderlineItem*>(pItem);
            aNewFont.SetUnderline(pFontItem->GetLineStyle());
        }
        if ( SfxItemState::SET == _rItemSet.GetItemState( ITEMID_COLOR,true,&pItem) && dynamic_cast< const SvxColorItem *>( pItem ) !=  nullptr)
        {
            const SvxColorItem* pFontItem = static_cast<const SvxColorItem*>(pItem);
            aNewFont.SetColor(pFontItem->GetValue());
        }

        _out_rAwtFont = VCLUnoHelper::CreateFontDescriptor( aNewFont );
    }


    void lcl_itemsToCharProperties( const vcl::Font& _rOriginalControlFont,const vcl::Font& _rOriginalControlFontAsian,const vcl::Font& _rOriginalControlFontComplex, const SfxItemSet& _rItemSet, uno::Sequence< beans::NamedValue >& _out_rProperties )
    {
        const SfxPoolItem* pItem( nullptr );

        // create an AWT font
        awt::FontDescriptor aAwtFont;
        lcl_initAwtFont( _rOriginalControlFont, _rItemSet, aAwtFont,ITEMID_FONT,ITEMID_FONTHEIGHT,ITEMID_POSTURE, ITEMID_WEIGHT);
        lcl_pushBack( _out_rProperties, "Font", uno::makeAny( aAwtFont ) );
        lcl_initAwtFont( _rOriginalControlFontAsian, _rItemSet, aAwtFont,ITEMID_FONT_ASIAN,ITEMID_FONTHEIGHT_ASIAN,ITEMID_POSTURE_ASIAN, ITEMID_WEIGHT_ASIAN);
        lcl_pushBack( _out_rProperties, "FontAsian", uno::makeAny( aAwtFont ) );
        lcl_initAwtFont( _rOriginalControlFontComplex, _rItemSet, aAwtFont,ITEMID_FONT_COMPLEX,ITEMID_FONTHEIGHT_COMPLEX,ITEMID_POSTURE_COMPLEX, ITEMID_WEIGHT_COMPLEX);
        lcl_pushBack( _out_rProperties, "FontComplex", uno::makeAny( aAwtFont ) );

        // properties which cannot be represented in an AWT font need to be preserved directly
        if ( SfxItemState::SET == _rItemSet.GetItemState( ITEMID_SHADOWED,true,&pItem) && dynamic_cast< const SvxShadowedItem *>( pItem ) !=  nullptr)
        {
            const SvxShadowedItem* pFontItem = static_cast<const SvxShadowedItem*>(pItem);
            lcl_pushBack( _out_rProperties, PROPERTY_CHARSHADOWED, uno::makeAny( pFontItem->GetValue() ) );
        }
        if ( SfxItemState::SET == _rItemSet.GetItemState( ITEMID_CONTOUR,true,&pItem) && dynamic_cast< const SvxContourItem *>( pItem ) !=  nullptr)
        {
            const SvxContourItem* pFontItem = static_cast<const SvxContourItem*>(pItem);
            lcl_pushBack( _out_rProperties, PROPERTY_CHARCONTOURED, uno::makeAny( pFontItem->GetValue() ) );
        }
        if ( SfxItemState::SET == _rItemSet.GetItemState( ITEMID_UNDERLINE,true,&pItem) && dynamic_cast< const SvxUnderlineItem *>( pItem ) !=  nullptr)
        {
            const SvxUnderlineItem* pFontItem = static_cast<const SvxUnderlineItem*>(pItem);
            lcl_pushBack( _out_rProperties, PROPERTY_CHARUNDERLINECOLOR, uno::makeAny( pFontItem->GetColor() ) );
        }
        if ( SfxItemState::SET == _rItemSet.GetItemState( ITEMID_HORJUSTIFY,true,&pItem) && dynamic_cast< const SvxHorJustifyItem *>( pItem ) !=  nullptr)
        {
            const SvxHorJustifyItem* pJustifyItem = static_cast<const SvxHorJustifyItem*>(pItem);
            uno::Any aValue;
            pJustifyItem->QueryValue(aValue,MID_HORJUST_ADJUST);
            lcl_pushBack( _out_rProperties, PROPERTY_PARAADJUST, aValue );
        }
        if ( SfxItemState::SET == _rItemSet.GetItemState( ITEMID_VERJUSTIFY,true,&pItem) && dynamic_cast< const SvxVerJustifyItem *>( pItem ) !=  nullptr)
        {
            const SvxVerJustifyItem* pJustifyItem = static_cast<const SvxVerJustifyItem*>(pItem);
            uno::Any aValue;
            pJustifyItem->QueryValue(aValue,MID_HORJUST_ADJUST);
            lcl_pushBack( _out_rProperties, PROPERTY_VERTICALALIGN, aValue );
        }
        if ( SfxItemState::SET == _rItemSet.GetItemState( ITEMID_CHARRELIEF,true,&pItem) && dynamic_cast< const SvxCharReliefItem *>( pItem ) !=  nullptr)
        {
            const SvxCharReliefItem* pFontItem = static_cast<const SvxCharReliefItem*>(pItem);
            lcl_pushBack( _out_rProperties, PROPERTY_CHARRELIEF, uno::makeAny( static_cast< sal_Int16 >( pFontItem->GetEnumValue() ) ) );
        }
        if ( SfxItemState::SET == _rItemSet.GetItemState( ITEMID_CHARHIDDEN,true,&pItem) && dynamic_cast< const SvxCharHiddenItem *>( pItem ) !=  nullptr)
        {
            const SvxCharHiddenItem* pFontItem = static_cast<const SvxCharHiddenItem*>(pItem);
            lcl_pushBack( _out_rProperties, PROPERTY_CHARHIDDEN, uno::makeAny( pFontItem->GetValue() ) );
        }
        if ( SfxItemState::SET == _rItemSet.GetItemState( ITEMID_AUTOKERN,true,&pItem) && dynamic_cast< const SvxAutoKernItem *>( pItem ) !=  nullptr)
        {
            const SvxAutoKernItem* pFontItem = static_cast<const SvxAutoKernItem*>(pItem);
            lcl_pushBack( _out_rProperties, PROPERTY_CHARAUTOKERNING, uno::makeAny( pFontItem->GetValue() ) );
        }
        if ( SfxItemState::SET == _rItemSet.GetItemState( ITEMID_BRUSH,true,&pItem) && dynamic_cast< const SvxBrushItem *>( pItem ) !=  nullptr)
        {
            const SvxBrushItem* pFontItem = static_cast<const SvxBrushItem*>(pItem);
            lcl_pushBack( _out_rProperties, PROPERTY_CONTROLBACKGROUND, uno::makeAny( pFontItem->GetColor() ) );
        }
        if ( SfxItemState::SET == _rItemSet.GetItemState( ITEMID_BLINK,true,&pItem) && dynamic_cast< const SvxBlinkItem *>( pItem ) !=  nullptr)
        {
            const SvxBlinkItem* pFontItem = static_cast<const SvxBlinkItem*>(pItem);
            lcl_pushBack( _out_rProperties, PROPERTY_CHARFLASH, uno::makeAny( pFontItem->GetValue() ) );
        }
        if ( SfxItemState::SET == _rItemSet.GetItemState( ITEMID_EMPHASISMARK,true,&pItem) && dynamic_cast< const SvxEmphasisMarkItem *>( pItem ) !=  nullptr)
        {
            const SvxEmphasisMarkItem* pFontItem = static_cast<const SvxEmphasisMarkItem*>(pItem);
            lcl_pushBack( _out_rProperties, PROPERTY_CHAREMPHASIS, uno::makeAny( static_cast< sal_Int16 >( pFontItem->GetEmphasisMark() ) ) );
        }
        if ( SfxItemState::SET == _rItemSet.GetItemState( ITEMID_TWOLINES,true,&pItem) && dynamic_cast< const SvxTwoLinesItem *>( pItem ) !=  nullptr)
        {
            const SvxTwoLinesItem* pFontItem = static_cast<const SvxTwoLinesItem*>(pItem);
            lcl_pushBack( _out_rProperties, PROPERTY_CHARCOMBINEISON, uno::makeAny( pFontItem->GetValue() ) );
            lcl_pushBack( _out_rProperties, PROPERTY_CHARCOMBINEPREFIX, uno::makeAny( OUString( pFontItem->GetStartBracket() ) ) );
            lcl_pushBack( _out_rProperties, PROPERTY_CHARCOMBINESUFFIX, uno::makeAny( OUString( pFontItem->GetEndBracket() ) ) );
        }
        if ( SfxItemState::SET == _rItemSet.GetItemState( ITEMID_COLOR,true,&pItem) && dynamic_cast< const SvxColorItem *>( pItem ) !=  nullptr)
        {
            const SvxColorItem* pFontItem = static_cast<const SvxColorItem*>(pItem);
            lcl_pushBack( _out_rProperties, PROPERTY_CHARCOLOR, uno::makeAny( pFontItem->GetValue() ) );
        }
        if ( SfxItemState::SET == _rItemSet.GetItemState( ITEMID_KERNING,true,&pItem) && dynamic_cast< const SvxKerningItem *>( pItem ) !=  nullptr)
        {
            const SvxKerningItem* pFontItem = static_cast<const SvxKerningItem*>(pItem);
            lcl_pushBack( _out_rProperties, PROPERTY_CHARKERNING, uno::makeAny( pFontItem->GetValue() ) );
        }
        if ( SfxItemState::SET == _rItemSet.GetItemState( ITEMID_CASEMAP,true,&pItem) && dynamic_cast< const SvxCaseMapItem *>( pItem ) !=  nullptr)
        {
            const SvxCaseMapItem* pFontItem = static_cast<const SvxCaseMapItem*>(pItem);
            lcl_pushBack( _out_rProperties, PROPERTY_CHARCASEMAP, uno::makeAny( pFontItem->GetEnumValue() ) );
        }
        struct Items {
                sal_uInt16 const nWhich;
                OUString const sPropertyName;
        };
        const Items pItems[] = { {ITEMID_LANGUAGE,OUString(PROPERTY_CHARLOCALE)}
                                ,{ITEMID_LANGUAGE_ASIAN,OUString(PROPERTY_CHARLOCALEASIAN)}
                                ,{ITEMID_LANGUAGE_COMPLEX,OUString(PROPERTY_CHARLOCALECOMPLEX)}
        };
        for(const auto & k : pItems)
        {
            if ( SfxItemState::SET == _rItemSet.GetItemState( k.nWhich,true,&pItem) && dynamic_cast< const SvxLanguageItem *>( pItem ) !=  nullptr)
            {
                const SvxLanguageItem* pFontItem = static_cast<const SvxLanguageItem*>(pItem);
                lang::Locale aCharLocale( LanguageTag( pFontItem->GetLanguage()).getLocale());
                lcl_pushBack( _out_rProperties, k.sPropertyName, uno::makeAny( aCharLocale ) );
            }
        }
        if ( SfxItemState::SET == _rItemSet.GetItemState( ITEMID_ESCAPEMENT,true,&pItem) && dynamic_cast< const SvxEscapementItem *>( pItem ) !=  nullptr)
        {
            const SvxEscapementItem* pFontItem = static_cast<const SvxEscapementItem*>(pItem);
            lcl_pushBack( _out_rProperties, PROPERTY_CHARESCAPEMENT, uno::makeAny( pFontItem->GetEsc() ) );
            lcl_pushBack( _out_rProperties, PROPERTY_CHARESCAPEMENTHEIGHT, uno::makeAny(static_cast<sal_Int8>(pFontItem->GetProportionalHeight())) );
        }
    }


    template< class ATTRIBUTE_TYPE >
    void lcl_applyFontAttribute( const ::comphelper::NamedValueCollection& _rAttrValues, const sal_Char* _pAttributeName,
        const uno::Reference<report::XReportControlFormat >& _rxReportControlFormat,
        void (SAL_CALL report::XReportControlFormat::*pSetter)( ATTRIBUTE_TYPE ) )
    {
        ATTRIBUTE_TYPE aAttributeValue = ATTRIBUTE_TYPE();
        if ( _rAttrValues.get_ensureType( _pAttributeName, aAttributeValue ) )
            (_rxReportControlFormat.get()->*pSetter)( aAttributeValue );
    }


    void lcl_applyFontAttribute( const ::comphelper::NamedValueCollection& _rAttrValues, const sal_Char* _pAttributeName,
        const uno::Reference<report::XReportControlFormat >& _rxReportControlFormat,
        void (SAL_CALL report::XReportControlFormat::*pSetter)( const OUString& ) )
    {
        OUString aAttributeValue;
        if ( _rAttrValues.get_ensureType( _pAttributeName, aAttributeValue ) )
            (_rxReportControlFormat.get()->*pSetter)( aAttributeValue );
    }


    void lcl_applyFontAttribute( const ::comphelper::NamedValueCollection& _rAttrValues, const sal_Char* _pAttributeName,
        const uno::Reference<report::XReportControlFormat >& _rxReportControlFormat,
        void (SAL_CALL report::XReportControlFormat::*pSetter)( const lang::Locale& ) )
    {
        lang::Locale aAttributeValue;
        if ( _rAttrValues.get_ensureType( _pAttributeName, aAttributeValue ) )
            (_rxReportControlFormat.get()->*pSetter)( aAttributeValue );
    }
}


bool openCharDialog( const uno::Reference<report::XReportControlFormat >& _rxReportControlFormat,
        const uno::Reference< awt::XWindow>& _rxParentWindow, uno::Sequence< beans::NamedValue >& _out_rNewValues )
{
    OSL_PRECOND( _rxReportControlFormat.is() && _rxParentWindow.is(), "openCharDialog: invalid parameters!" );
    if ( !_rxReportControlFormat.is() || !_rxParentWindow.is() )
        return false;

    _out_rNewValues = uno::Sequence< beans::NamedValue >();


    // UNO->ItemSet
    static SfxItemInfo aItemInfos[] =
    {
        { SID_ATTR_CHAR_FONT, true },
        { SID_ATTR_CHAR_FONTHEIGHT, true },
        { SID_ATTR_CHAR_LANGUAGE, true },
        { SID_ATTR_CHAR_POSTURE, true },
        { SID_ATTR_CHAR_WEIGHT, true },
        { SID_ATTR_CHAR_SHADOWED, true },
        { SID_ATTR_CHAR_WORDLINEMODE, true },
        { SID_ATTR_CHAR_CONTOUR, true },
        { SID_ATTR_CHAR_STRIKEOUT, true },
        { SID_ATTR_CHAR_UNDERLINE, true },
        { SID_ATTR_CHAR_COLOR, true },
        { SID_ATTR_CHAR_KERNING, true },
        { SID_ATTR_CHAR_CASEMAP, true },
        { SID_ATTR_CHAR_ESCAPEMENT, true },
        { SID_ATTR_CHAR_FONTLIST, true },
        { SID_ATTR_CHAR_AUTOKERN, true },
        { SID_COLOR_TABLE, true },
        { SID_ATTR_FLASH, true },
        { SID_ATTR_CHAR_EMPHASISMARK, true },
        { SID_ATTR_CHAR_TWO_LINES, true },
        { SID_ATTR_CHAR_ROTATED, true },
        { SID_ATTR_CHAR_SCALEWIDTH, true },
        { SID_ATTR_CHAR_RELIEF, true },
        { SID_ATTR_CHAR_HIDDEN, true },
        { SID_ATTR_BRUSH, true },
        { SID_ATTR_ALIGN_HOR_JUSTIFY, true },
        { SID_ATTR_ALIGN_VER_JUSTIFY, true },

        // Asian
        { SID_ATTR_CHAR_CJK_FONT, true },
        { SID_ATTR_CHAR_CJK_FONTHEIGHT, true },
        { SID_ATTR_CHAR_CJK_LANGUAGE, true },
        { SID_ATTR_CHAR_CJK_POSTURE, true },
        { SID_ATTR_CHAR_CJK_WEIGHT, true },
        // Complex
        { SID_ATTR_CHAR_CTL_FONT, true },
        { SID_ATTR_CHAR_CTL_FONTHEIGHT, true },
        { SID_ATTR_CHAR_CTL_LANGUAGE, true },
        { SID_ATTR_CHAR_CTL_POSTURE, true },
        { SID_ATTR_CHAR_CTL_WEIGHT, true }
    };
    VclPtr<vcl::Window> pParent = VCLUnoHelper::GetWindow( _rxParentWindow );
    ::std::unique_ptr<FontList> pFontList(new FontList( pParent ));
    XColorListRef pColorList( XColorList::CreateStdColorList() );
    std::vector<SfxPoolItem*> pDefaults
    {
        new SvxFontItem(ITEMID_FONT),
        new SvxFontHeightItem(240,100,ITEMID_FONTHEIGHT),
        new SvxLanguageItem(LANGUAGE_GERMAN,ITEMID_LANGUAGE),
        new SvxPostureItem(ITALIC_NONE,ITEMID_POSTURE),
        new SvxWeightItem(WEIGHT_NORMAL,ITEMID_WEIGHT),

        new SvxShadowedItem(false,ITEMID_SHADOWED),
        new SvxWordLineModeItem(false,ITEMID_WORDLINEMODE),
        new SvxContourItem(false,ITEMID_CONTOUR),
        new SvxCrossedOutItem(STRIKEOUT_NONE,ITEMID_CROSSEDOUT),
        new SvxUnderlineItem(LINESTYLE_NONE,ITEMID_UNDERLINE),

        new SvxColorItem(ITEMID_COLOR),
        new SvxKerningItem(0,ITEMID_KERNING),
        new SvxCaseMapItem(SvxCaseMap::NotMapped,ITEMID_CASEMAP),
        new SvxEscapementItem(ITEMID_ESCAPEMENT),
        new SvxFontListItem(pFontList.get(),ITEMID_FONTLIST),
        new SvxAutoKernItem(false,ITEMID_AUTOKERN),
        new SvxColorListItem(pColorList.get(),ITEMID_COLOR_TABLE),
        new SvxBlinkItem(false,ITEMID_BLINK),
        new SvxEmphasisMarkItem(FontEmphasisMark::NONE,ITEMID_EMPHASISMARK),
        new SvxTwoLinesItem(true,0,0,ITEMID_TWOLINES),
        new SvxCharRotateItem(0,false,ITEMID_CHARROTATE),
        new SvxCharScaleWidthItem(100,ITEMID_CHARSCALE_W),
        new SvxCharReliefItem(FontRelief::NONE,ITEMID_CHARRELIEF),
        new SvxCharHiddenItem(false,ITEMID_CHARHIDDEN),
        new SvxBrushItem(ITEMID_BRUSH),
        new SvxHorJustifyItem(ITEMID_HORJUSTIFY),
        new SvxVerJustifyItem(ITEMID_VERJUSTIFY),
// Asian
        new SvxFontItem(ITEMID_FONT_ASIAN),
        new SvxFontHeightItem(240,100,ITEMID_FONTHEIGHT_ASIAN),
        new SvxLanguageItem(LANGUAGE_GERMAN,ITEMID_LANGUAGE_ASIAN),
        new SvxPostureItem(ITALIC_NONE,ITEMID_POSTURE_ASIAN),
        new SvxWeightItem(WEIGHT_NORMAL,ITEMID_WEIGHT_ASIAN),
// Complex
        new SvxFontItem(ITEMID_FONT_COMPLEX),
        new SvxFontHeightItem(240,100,ITEMID_FONTHEIGHT_COMPLEX),
        new SvxLanguageItem(LANGUAGE_GERMAN,ITEMID_LANGUAGE_COMPLEX),
        new SvxPostureItem(ITALIC_NONE,ITEMID_POSTURE_COMPLEX),
        new SvxWeightItem(WEIGHT_NORMAL,ITEMID_WEIGHT_COMPLEX)

    };

    OSL_ASSERT( pDefaults.size() == SAL_N_ELEMENTS(aItemInfos) );

    static const sal_uInt16 pRanges[] =
    {
        ITEMID_FONT,ITEMID_WEIGHT_COMPLEX,
        0
    };

    SfxItemPool* pPool( new SfxItemPool("ReportCharProperties", ITEMID_FONT,ITEMID_WEIGHT_COMPLEX, aItemInfos, &pDefaults) );
    // not needed for font height pPool->SetDefaultMetric( MapUnit::Map100thMM );  // ripped, don't understand why
    pPool->FreezeIdRanges();                        // the same
    bool bSuccess = false;
    try
    {
        ::std::unique_ptr<SfxItemSet> pDescriptor( new SfxItemSet( *pPool, pRanges ) );
        lcl_CharPropertiesToItems( _rxReportControlFormat, *pDescriptor );

        {   // want the dialog to be destroyed before our set
            ORptPageDialog aDlg(Application::GetFrameWeld(_rxParentWindow), pDescriptor.get(), "CharDialog");
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

    SfxItemPool::Free(pPool);
    for (SfxPoolItem* pDefault : pDefaults)
        delete pDefault;

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
        ::std::unique_ptr<SfxItemSet> pDescriptor( new SfxItemSet( rItemPool, {{rItemPool.GetFirstWhich(),rItemPool.GetLastWhich()}} ) );
        lcl_fillShapeToItems(_xShape,*pDescriptor);

        {   // want the dialog to be destroyed before our set
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            ScopedVclPtr<AbstractSvxAreaTabDialog> pDialog(pFact->CreateSvxAreaTabDialog(pParent, pDescriptor.get(), pModel.get(), true));
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
    SdrObject* pObjIter = nullptr;

    while( !pOverlappedObj && (pObjIter = aIter.Next()) != nullptr )
    {
        if ( _pIgnore != pObjIter
            && (_bAllObjects || !_rView.IsObjMarked(pObjIter))
            && (dynamic_cast<OUnoObject*>(pObjIter) != nullptr || dynamic_cast<OOle2Obj*>(pObjIter) != nullptr))
        {
            if (_nIgnoreType == ISOVER_IGNORE_CUSTOMSHAPES && pObjIter->GetObjIdentifier() == OBJ_CUSTOMSHAPE)
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

static bool checkArrayForOccurrence(SdrObject const * _pObjToCheck, std::unique_ptr<SdrUnoObj, SdrObjectFreeOp> _pIgnore[], int _nListLength)
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

SdrObject* isOver(const tools::Rectangle& _rRect,SdrPage const & _rPage,SdrView const & _rView,bool _bAllObjects, std::unique_ptr<SdrUnoObj, SdrObjectFreeOp> _pIgnoreList[], int _nIgnoreListLength)
{
    SdrObject* pOverlappedObj = nullptr;
    SdrObjListIter aIter(&_rPage,SdrIterMode::DeepNoGroups);
    SdrObject* pObjIter = nullptr;

    while( !pOverlappedObj && (pObjIter = aIter.Next()) != nullptr )
    {
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

            uno::Reference< beans::XPropertySet > xParam;
            OUString sParamName;
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
            aRect.setHeight(aRect.getHeight() + 1);
            aRect.setWidth(aRect.getWidth() + 1);
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
            aRect.Move(0,aLogicRect.Top() + aLogicRect.getHeight() - aRect.Top());
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
        VclPtr<vcl::Window> pParent = VCLUnoHelper::GetWindow( _xInspectorWindow );

        uno::Reference< report::meta::XFunctionManager> xMgr(xFactory->createInstanceWithContext("org.libreoffice.report.pentaho.SOFunctionManager",_xContext),uno::UNO_QUERY);
        if ( xMgr.is() )
        {
            std::shared_ptr< formula::IFunctionManager > pFormulaManager(new FunctionManager(xMgr) );
            ReportFormula aFormula( _in_out_rFormula );

            LanguageTag aLangTag(LANGUAGE_SYSTEM);
            CharClass aCC(_xContext, aLangTag);
            svl::SharedStringPool aStringPool(aCC);

            ScopedVclPtrInstance<FormulaDialog> aDlg(
                pParent, xServiceFactory, pFormulaManager,
                aFormula.getUndecoratedContent(), _xRowSet, aStringPool);

            bSuccess = aDlg->Execute() == RET_OK;
            if ( bSuccess )
            {
                OUString sFormula = aDlg->getCurrentFormula();
                if ( sFormula[0] == '=' )
                    _in_out_rFormula = "rpt:" + sFormula.copy(1);
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
        OSL_FAIL( "GeometryHandler::impl_dialogFilter_nothrow: caught an exception!" );
    }

    if ( aErrorInfo.isValid() )
        ::dbtools::showError( aErrorInfo, xInspectorWindow, _xContext );

    return bSuccess;
}

} // namespace rptui


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
