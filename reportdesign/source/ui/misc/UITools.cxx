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


#include <toolkit/helper/convert.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include "SectionView.hxx"
#include "UITools.hxx"
#include "Formula.hxx"
#include "FunctionHelper.hxx"
#include "reportformula.hxx"

#include <tools/diagnose_ex.h>
#include <tools/string.hxx>

#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <com/sun/star/lang/NullPointerException.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <svx/svdpool.hxx>

#include <editeng/charscaleitem.hxx>
#include <svx/algitem.hxx>
#include <svx/svdpagv.hxx>
#include <svx/xtable.hxx>       // XColorList
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
#include <editeng/prszitem.hxx>
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
#include <svtools/ctrltool.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>

#include <comphelper/propmultiplex.hxx>
#include <comphelper/namedvaluecollection.hxx>

#include <connectivity/dbexception.hxx>
#include <connectivity/dbconversion.hxx>
#include <connectivity/dbtools.hxx>

#include <com/sun/star/report/XGroups.hpp>
#include <com/sun/star/awt/TextAlign.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/report/XShape.hpp>
#include <com/sun/star/report/Function.hpp>
#include <com/sun/star/sdb/XParametersSupplier.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <i18nlangtag/languagetag.hxx>
#include "dlgpage.hxx"
#include <vcl/msgbox.hxx>
#include "rptui_slotid.hrc"
#include "uistrings.hrc"
#include "RptObject.hxx"
#include "ModuleHelper.hxx"
#include "RptDef.hxx"
#include "RptResId.hrc"
#include "ReportDefinition.hxx"
#include "RptModel.hxx"

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
// -----------------------------------------------------------------------------
void adjustSectionName(const uno::Reference< report::XGroup >& _xGroup,sal_Int32 _nPos)
{
    OSL_ENSURE(_xGroup.is(),"Group is NULL -> GPF");
    if ( _xGroup->getHeaderOn() && _xGroup->getHeader()->getName().isEmpty() )
    {
        OUString sName = String(ModuleRes(RID_STR_GROUPHEADER));
        sName += OUString::number(_nPos);
        _xGroup->getHeader()->setName(sName);
    }

    if ( _xGroup->getFooterOn() && _xGroup->getFooter()->getName().isEmpty() )
    {
        OUString sName = String(ModuleRes(RID_STR_GROUPFOOTER));
        sName += OUString::number(_nPos);
        _xGroup->getFooter()->setName(sName);
    }
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
            pRet->addProperty(PROPERTY_BACKCOLOR);
        }
    }
    return pRet;
}

// -----------------------------------------------------------------------------
namespace
{
    // -------------------------------------------------------------------------
    Font lcl_getReportControlFont( const uno::Reference<report::XReportControlFormat >& _rxReportControlFormat, awt::FontDescriptor& _out_rControlFont ,sal_uInt16 _nWichFont)
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

        Font aDefaultFont = Application::GetDefaultDevice()->GetSettings().GetStyleSettings().GetAppFont();
        return VCLUnoHelper::CreateFont( _out_rControlFont, aDefaultFont );
    }

    // -------------------------------------------------------------------------
    Font lcl_getReportControlFont( const uno::Reference<report::XReportControlFormat >& _rxReportControlFormat,sal_uInt16 _nWhich )
    {
        awt::FontDescriptor aAwtFont;
        return lcl_getReportControlFont( _rxReportControlFormat, aAwtFont, _nWhich );
    }
    // -------------------------------------------------------------------------
    const Font lcl_setFont(const uno::Reference<report::XReportControlFormat >& _rxReportControlFormat,
        SfxItemSet& _rItemSet,sal_uInt16 _nWhich,sal_uInt16 _nFont, sal_uInt16 _nFontHeight,sal_uInt16 _nLanguage,sal_uInt16 _nPosture, sal_uInt16 _nWeight)
    {
        // fill it
        awt::FontDescriptor aControlFont;
        const Font aFont( lcl_getReportControlFont( _rxReportControlFormat, aControlFont,_nWhich ) );

        SvxFontItem aFontItem(_nFont);
        aFontItem.PutValue( uno::makeAny( aControlFont ) );
        _rItemSet.Put(aFontItem);

        _rItemSet.Put(SvxFontHeightItem(OutputDevice::LogicToLogic(Size(0, (sal_Int32)aFont.GetHeight()), MAP_POINT, MAP_TWIP).Height(),100,_nFontHeight));
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
        const SfxItemPropertyMap* pPropertyMap = aMap.GetPropertySet(SVXMAP_CUSTOMSHAPE, SdrObject::GetGlobalDrawObjectItemPool())->getPropertyMap();
        PropertyEntryVector_t aPropVector = pPropertyMap->getPropertyEntries();
        PropertyEntryVector_t::const_iterator aIt = aPropVector.begin();
        while( aIt != aPropVector.end() )
        {
            if ( xInfo->hasPropertyByName(aIt->sName) )
            {
                const SfxPoolItem* pItem = _rItemSet.GetItem(aIt->nWID);
                if ( pItem )
                {
                    SAL_WNODEPRECATED_DECLARATIONS_PUSH
                    ::std::auto_ptr<SfxPoolItem> pClone(pItem->Clone());
                    SAL_WNODEPRECATED_DECLARATIONS_POP
                    pClone->PutValue(_xShape->getPropertyValue(aIt->sName), aIt->nMemberId);
                    _rItemSet.Put(*pClone, aIt->nWID);
                }
            }
            ++aIt;
        }
    }

    void lcl_fillItemsToShape( const uno::Reference<report::XShape >& _xShape,const SfxItemSet& _rItemSet )
    {
        const uno::Reference< beans::XPropertySetInfo> xInfo = _xShape->getPropertySetInfo();
        SvxUnoPropertyMapProvider aMap;
        const SfxItemPropertyMap* pPropertyMap = aMap.GetPropertySet(SVXMAP_CUSTOMSHAPE, SdrObject::GetGlobalDrawObjectItemPool())->getPropertyMap();
        PropertyEntryVector_t aPropVector = pPropertyMap->getPropertyEntries();
        PropertyEntryVector_t::const_iterator aIt = aPropVector.begin();
        while( aIt != aPropVector.end() )
        {
            if ( SFX_ITEM_SET == _rItemSet.GetItemState(aIt->nWID) && xInfo->hasPropertyByName(aIt->sName) )
            {
                const beans::Property aProp = xInfo->getPropertyByName( aIt->sName );
                if ( ( aIt->nFlags & beans::PropertyAttribute::READONLY ) != beans::PropertyAttribute::READONLY )
                {
                    const SfxPoolItem* pItem = _rItemSet.GetItem(aIt->nWID);
                    if ( pItem )
                    {
                        uno::Any aValue;
                        pItem->QueryValue(aValue,aIt->nMemberId);
                        try
                        {
                            _xShape->setPropertyValue(aIt->sName, aValue);
                        }
                        catch(uno::Exception&)
                        { // shapes have a bug so we ignore this one.
                        }
                    }
                }
            }
            ++aIt;
        }
    }
    // -------------------------------------------------------------------------
    void lcl_CharPropertiesToItems( const uno::Reference<report::XReportControlFormat >& _rxReportControlFormat,
        SfxItemSet& _rItemSet )
    {
        if ( !_rxReportControlFormat.is() )
            throw lang::NullPointerException();

        uno::Reference< beans::XPropertySet > xSet(_rxReportControlFormat,uno::UNO_QUERY_THROW);

        // fill it
        const Font aFont( lcl_setFont(_rxReportControlFormat, _rItemSet,WESTERN,ITEMID_FONT,ITEMID_FONTHEIGHT,ITEMID_LANGUAGE,ITEMID_POSTURE,ITEMID_WEIGHT ) );

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
        aUnderLineItem.SetColor(_rxReportControlFormat->getCharUnderlineColor());
        _rItemSet.Put(aUnderLineItem);
        _rItemSet.Put(SvxKerningItem(_rxReportControlFormat->getCharKerning(),ITEMID_KERNING));
        _rItemSet.Put(SvxEmphasisMarkItem(static_cast<FontEmphasisMark>(_rxReportControlFormat->getCharEmphasis()),ITEMID_EMPHASISMARK));
        _rItemSet.Put(SvxCharReliefItem(static_cast<FontRelief>(_rxReportControlFormat->getCharRelief()),ITEMID_CHARRELIEF));
        _rItemSet.Put(SvxColorItem(::Color(_rxReportControlFormat->getCharColor()),ITEMID_COLOR));
        _rItemSet.Put(SvxCharRotateItem(_rxReportControlFormat->getCharRotation(),sal_False,ITEMID_CHARROTATE));
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

    // -------------------------------------------------------------------------
    void lcl_pushBack( uno::Sequence< beans::NamedValue >& _out_rProperties, const OUString& _sName, const uno::Any& _rValue )
    {
        sal_Int32 nLen( _out_rProperties.getLength() );
        _out_rProperties.realloc( nLen + 1 );
        _out_rProperties[ nLen ] = beans::NamedValue( _sName, _rValue );
    }

    // -------------------------------------------------------------------------
    void lcl_initAwtFont( const Font& _rOriginalFont, const SfxItemSet& _rItemSet, awt::FontDescriptor& _out_rAwtFont,
        sal_uInt16 _nFont, sal_uInt16 _nFontHeight,sal_uInt16 _nPosture, sal_uInt16 _nWeight)
    {
        Font aNewFont( _rOriginalFont );
        const SfxPoolItem* pItem( NULL );
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( _nFont,sal_True,&pItem) && pItem->ISA(SvxFontItem))
        {
            const SvxFontItem* pFontItem = static_cast<const SvxFontItem*>(pItem);
            aNewFont.SetName( pFontItem->GetFamilyName());
            aNewFont.SetStyleName(pFontItem->GetStyleName());
            aNewFont.SetFamily(pFontItem->GetFamily());
            aNewFont.SetPitch(pFontItem->GetPitch());
            aNewFont.SetCharSet(pFontItem->GetCharSet());
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( _nFontHeight,sal_True,&pItem) && pItem->ISA(SvxFontHeightItem))
        {
            const SvxFontHeightItem* pFontItem = static_cast<const SvxFontHeightItem*>(pItem);
            aNewFont.SetHeight(OutputDevice::LogicToLogic(Size(0, pFontItem->GetHeight()), MAP_TWIP, MAP_POINT).Height());
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( _nPosture,sal_True,&pItem) && pItem->ISA(SvxPostureItem))
        {
            const SvxPostureItem* pFontItem = static_cast<const SvxPostureItem*>(pItem);
            aNewFont.SetItalic(pFontItem->GetPosture());
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( _nWeight,sal_True,&pItem) && pItem->ISA(SvxWeightItem))
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
            aNewFont.SetUnderline(pFontItem->GetLineStyle());
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_COLOR,sal_True,&pItem) && pItem->ISA(SvxColorItem))
        {
            const SvxColorItem* pFontItem = static_cast<const SvxColorItem*>(pItem);
            aNewFont.SetColor(pFontItem->GetValue().GetColor());
        }

        _out_rAwtFont = VCLUnoHelper::CreateFontDescriptor( aNewFont );
    }

    // -------------------------------------------------------------------------
    void lcl_itemsToCharProperties( const Font& _rOriginalControlFont,const Font& _rOriginalControlFontAsian,const Font& _rOriginalControlFontComplex, const SfxItemSet& _rItemSet, uno::Sequence< beans::NamedValue >& _out_rProperties )
    {
        const SfxPoolItem* pItem( NULL );

        // create an AWT font
        awt::FontDescriptor aAwtFont;
        lcl_initAwtFont( _rOriginalControlFont, _rItemSet, aAwtFont,ITEMID_FONT,ITEMID_FONTHEIGHT,ITEMID_POSTURE, ITEMID_WEIGHT);
        lcl_pushBack( _out_rProperties, OUString("Font"), uno::makeAny( aAwtFont ) );
        lcl_initAwtFont( _rOriginalControlFontAsian, _rItemSet, aAwtFont,ITEMID_FONT_ASIAN,ITEMID_FONTHEIGHT_ASIAN,ITEMID_POSTURE_ASIAN, ITEMID_WEIGHT_ASIAN);
        lcl_pushBack( _out_rProperties, OUString("FontAsian"), uno::makeAny( aAwtFont ) );
        lcl_initAwtFont( _rOriginalControlFontComplex, _rItemSet, aAwtFont,ITEMID_FONT_COMPLEX,ITEMID_FONTHEIGHT_COMPLEX,ITEMID_POSTURE_COMPLEX, ITEMID_WEIGHT_COMPLEX);
        lcl_pushBack( _out_rProperties, OUString("FontComplex"), uno::makeAny( aAwtFont ) );

        // properties which cannot be represented in an AWT font need to be preserved directly
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_SHADOWED,sal_True,&pItem) && pItem->ISA(SvxShadowedItem))
        {
            const SvxShadowedItem* pFontItem = static_cast<const SvxShadowedItem*>(pItem);
            lcl_pushBack( _out_rProperties, PROPERTY_CHARSHADOWED, uno::makeAny( pFontItem->GetValue() ) );
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_CONTOUR,sal_True,&pItem) && pItem->ISA(SvxContourItem))
        {
            const SvxContourItem* pFontItem = static_cast<const SvxContourItem*>(pItem);
            lcl_pushBack( _out_rProperties, PROPERTY_CHARCONTOURED, uno::makeAny( pFontItem->GetValue() ) );
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_UNDERLINE,sal_True,&pItem) && pItem->ISA(SvxUnderlineItem))
        {
            const SvxUnderlineItem* pFontItem = static_cast<const SvxUnderlineItem*>(pItem);
            lcl_pushBack( _out_rProperties, PROPERTY_CHARUNDERLINECOLOR, uno::makeAny( pFontItem->GetColor().GetColor() ) );
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_HORJUSTIFY,sal_True,&pItem) && pItem->ISA(SvxHorJustifyItem))
        {
            const SvxHorJustifyItem* pJustifyItem = static_cast<const SvxHorJustifyItem*>(pItem);
            uno::Any aValue;
            pJustifyItem->QueryValue(aValue,MID_HORJUST_ADJUST);
            lcl_pushBack( _out_rProperties, PROPERTY_PARAADJUST, aValue );
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_VERJUSTIFY,sal_True,&pItem) && pItem->ISA(SvxVerJustifyItem))
        {
            const SvxVerJustifyItem* pJustifyItem = static_cast<const SvxVerJustifyItem*>(pItem);
            uno::Any aValue;
            pJustifyItem->QueryValue(aValue,MID_HORJUST_ADJUST);
            lcl_pushBack( _out_rProperties, PROPERTY_VERTICALALIGN, aValue );
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_CHARRELIEF,sal_True,&pItem) && pItem->ISA(SvxCharReliefItem))
        {
            const SvxCharReliefItem* pFontItem = static_cast<const SvxCharReliefItem*>(pItem);
            lcl_pushBack( _out_rProperties, PROPERTY_CHARRELIEF, uno::makeAny( static_cast< sal_Int16 >( pFontItem->GetEnumValue() ) ) );
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_CHARHIDDEN,sal_True,&pItem) && pItem->ISA(SvxCharHiddenItem))
        {
            const SvxCharHiddenItem* pFontItem = static_cast<const SvxCharHiddenItem*>(pItem);
            lcl_pushBack( _out_rProperties, PROPERTY_CHARHIDDEN, uno::makeAny( pFontItem->GetValue() ) );
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_AUTOKERN,sal_True,&pItem) && pItem->ISA(SvxAutoKernItem))
        {
            const SvxAutoKernItem* pFontItem = static_cast<const SvxAutoKernItem*>(pItem);
            lcl_pushBack( _out_rProperties, PROPERTY_CHARAUTOKERNING, uno::makeAny( pFontItem->GetValue() ) );
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_BRUSH,sal_True,&pItem) && pItem->ISA(SvxBrushItem))
        {
            const SvxBrushItem* pFontItem = static_cast<const SvxBrushItem*>(pItem);
            lcl_pushBack( _out_rProperties, PROPERTY_CONTROLBACKGROUND, uno::makeAny( pFontItem->GetColor().GetColor() ) );
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_BLINK,sal_True,&pItem) && pItem->ISA(SvxBlinkItem))
        {
            const SvxBlinkItem* pFontItem = static_cast<const SvxBlinkItem*>(pItem);
            lcl_pushBack( _out_rProperties, PROPERTY_CHARFLASH, uno::makeAny( pFontItem->GetValue() ) );
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_EMPHASISMARK,sal_True,&pItem) && pItem->ISA(SvxEmphasisMarkItem))
        {
            const SvxEmphasisMarkItem* pFontItem = static_cast<const SvxEmphasisMarkItem*>(pItem);
            lcl_pushBack( _out_rProperties, PROPERTY_CHAREMPHASIS, uno::makeAny( static_cast< sal_Int16 >( pFontItem->GetEmphasisMark() ) ) );
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_TWOLINES,sal_True,&pItem) && pItem->ISA(SvxTwoLinesItem))
        {
            const SvxTwoLinesItem* pFontItem = static_cast<const SvxTwoLinesItem*>(pItem);
            lcl_pushBack( _out_rProperties, PROPERTY_CHARCOMBINEISON, uno::makeAny( pFontItem->GetValue() ) );
            lcl_pushBack( _out_rProperties, PROPERTY_CHARCOMBINEPREFIX, uno::makeAny( OUString( pFontItem->GetStartBracket() ) ) );
            lcl_pushBack( _out_rProperties, PROPERTY_CHARCOMBINESUFFIX, uno::makeAny( OUString( pFontItem->GetEndBracket() ) ) );
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_COLOR,sal_True,&pItem) && pItem->ISA(SvxColorItem))
        {
            const SvxColorItem* pFontItem = static_cast<const SvxColorItem*>(pItem);
            lcl_pushBack( _out_rProperties, PROPERTY_CHARCOLOR, uno::makeAny( pFontItem->GetValue().GetColor() ) );
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_KERNING,sal_True,&pItem) && pItem->ISA(SvxKerningItem))
        {
            const SvxKerningItem* pFontItem = static_cast<const SvxKerningItem*>(pItem);
            lcl_pushBack( _out_rProperties, PROPERTY_CHARKERNING, uno::makeAny( pFontItem->GetValue() ) );
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_CASEMAP,sal_True,&pItem) && pItem->ISA(SvxCaseMapItem))
        {
            const SvxCaseMapItem* pFontItem = static_cast<const SvxCaseMapItem*>(pItem);
            lcl_pushBack( _out_rProperties, PROPERTY_CHARCASEMAP, uno::makeAny( pFontItem->GetValue() ) );
        }
        struct Items {
                sal_uInt16 nWhich;
                OUString sPropertyName;
        };
        const Items pItems[] = { {ITEMID_LANGUAGE,OUString(PROPERTY_CHARLOCALE)}
                                ,{ITEMID_LANGUAGE_ASIAN,OUString(PROPERTY_CHARLOCALEASIAN)}
                                ,{ITEMID_LANGUAGE_COMPLEX,OUString(PROPERTY_CHARLOCALECOMPLEX)}
        };
        for(size_t k = 0; k < sizeof(pItems)/sizeof(pItems[0]);++k)
        {
            if ( SFX_ITEM_SET == _rItemSet.GetItemState( pItems[k].nWhich,sal_True,&pItem) && pItem->ISA(SvxLanguageItem))
            {
                const SvxLanguageItem* pFontItem = static_cast<const SvxLanguageItem*>(pItem);
                lang::Locale aCharLocale( LanguageTag( pFontItem->GetLanguage()).getLocale());
                lcl_pushBack( _out_rProperties, pItems[k].sPropertyName, uno::makeAny( aCharLocale ) );
            }
        }
        if ( SFX_ITEM_SET == _rItemSet.GetItemState( ITEMID_ESCAPEMENT,sal_True,&pItem) && pItem->ISA(SvxEscapementItem))
        {
            const SvxEscapementItem* pFontItem = static_cast<const SvxEscapementItem*>(pItem);
            lcl_pushBack( _out_rProperties, PROPERTY_CHARESCAPEMENT, uno::makeAny( pFontItem->GetEsc() ) );
            lcl_pushBack( _out_rProperties, PROPERTY_CHARESCAPEMENTHEIGHT, uno::makeAny( (sal_Int8)pFontItem->GetProp() ) );
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
        void (SAL_CALL report::XReportControlFormat::*pSetter)( const OUString& ) )
    {
        OUString aAttributeValue;
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
        { SID_ATTR_CHAR_FONTHEIGHT, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_LANGUAGE, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_POSTURE, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_WEIGHT, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_SHADOWED, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_WORDLINEMODE, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_CONTOUR, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_STRIKEOUT, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_UNDERLINE, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_COLOR, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_KERNING, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_CASEMAP, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_ESCAPEMENT, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_FONTLIST, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_AUTOKERN, SFX_ITEM_POOLABLE },
        { SID_COLOR_TABLE, SFX_ITEM_POOLABLE },
        { SID_ATTR_FLASH, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_EMPHASISMARK, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_TWO_LINES, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_ROTATED, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_SCALEWIDTH, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_RELIEF, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_HIDDEN, SFX_ITEM_POOLABLE },
        { SID_ATTR_BRUSH, SFX_ITEM_POOLABLE },
        { SID_ATTR_ALIGN_HOR_JUSTIFY, SFX_ITEM_POOLABLE },
        { SID_ATTR_ALIGN_VER_JUSTIFY, SFX_ITEM_POOLABLE },

        // Asian
        { SID_ATTR_CHAR_CJK_FONT, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_CJK_FONTHEIGHT, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_CJK_LANGUAGE, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_CJK_POSTURE, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_CJK_WEIGHT, SFX_ITEM_POOLABLE },
        // Complex
        { SID_ATTR_CHAR_CTL_FONT, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_CTL_FONTHEIGHT, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_CTL_LANGUAGE, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_CTL_POSTURE, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_CTL_WEIGHT, SFX_ITEM_POOLABLE }
    };
    Window* pParent = VCLUnoHelper::GetWindow( _rxParentWindow );
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<FontList> pFontList(new FontList( pParent ));
    XColorListRef pColorList( XColorList::CreateStdColorList() );
    SAL_WNODEPRECATED_DECLARATIONS_POP
    SfxPoolItem* pDefaults[] =
    {
        new SvxFontItem(ITEMID_FONT),
        new SvxFontHeightItem(240,100,ITEMID_FONTHEIGHT),
        new SvxLanguageItem(LANGUAGE_GERMAN,ITEMID_LANGUAGE),
        new SvxPostureItem(ITALIC_NONE,ITEMID_POSTURE),
        new SvxWeightItem(WEIGHT_NORMAL,ITEMID_WEIGHT),

        new SvxShadowedItem(sal_False,ITEMID_SHADOWED),
        new SvxWordLineModeItem(sal_False,ITEMID_WORDLINEMODE),
        new SvxContourItem(sal_False,ITEMID_CONTOUR),
        new SvxCrossedOutItem(STRIKEOUT_NONE,ITEMID_CROSSEDOUT),
        new SvxUnderlineItem(UNDERLINE_NONE,ITEMID_UNDERLINE),

        new SvxColorItem(ITEMID_COLOR),
        new SvxKerningItem(0,ITEMID_KERNING),
        new SvxCaseMapItem(SVX_CASEMAP_NOT_MAPPED,ITEMID_CASEMAP),
        new SvxEscapementItem(ITEMID_ESCAPEMENT),
        new SvxFontListItem(pFontList.get(),ITEMID_FONTLIST),
        new SvxAutoKernItem(sal_False,ITEMID_AUTOKERN),
        new SvxColorListItem(pColorList.get(),ITEMID_COLOR_TABLE),
        new SvxBlinkItem(sal_False,ITEMID_BLINK),
        new SvxEmphasisMarkItem(EMPHASISMARK_NONE,ITEMID_EMPHASISMARK),
        new SvxTwoLinesItem(sal_True,0,0,ITEMID_TWOLINES),
        new SvxCharRotateItem(0,sal_False,ITEMID_CHARROTATE),
        new SvxCharScaleWidthItem(100,ITEMID_CHARSCALE_W),
        new SvxCharReliefItem(RELIEF_NONE,ITEMID_CHARRELIEF),
        new SvxCharHiddenItem(sal_False,ITEMID_CHARHIDDEN),
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

    OSL_ASSERT((sizeof(pDefaults)/sizeof(pDefaults[0])) == (sizeof(aItemInfos)/sizeof(aItemInfos[0])));

    static const sal_uInt16 pRanges[] =
    {
        ITEMID_FONT,ITEMID_WEIGHT_COMPLEX,
        0
    };

    SfxItemPool* pPool( new SfxItemPool(OUString("ReportCharProperties"), ITEMID_FONT,ITEMID_WEIGHT_COMPLEX, aItemInfos, pDefaults) );
    // not needed for font height pPool->SetDefaultMetric( SFX_MAPUNIT_100TH_MM );  // ripped, don't understand why
    pPool->FreezeIdRanges();                        // the same
    bool bSuccess = false;
    try
    {
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<SfxItemSet> pDescriptor( new SfxItemSet( *pPool, pRanges ) );
        SAL_WNODEPRECATED_DECLARATIONS_POP
        lcl_CharPropertiesToItems( _rxReportControlFormat, *pDescriptor );

        {   // want the dialog to be destroyed before our set
            ORptPageDialog aDlg(pParent, pDescriptor.get(),RID_PAGEDIALOG_CHAR);
            uno::Reference< report::XShape > xShape( _rxReportControlFormat, uno::UNO_QUERY );
            if ( xShape.is() )
                aDlg.RemoveTabPage( RID_PAGE_BACKGROUND );
            bSuccess = ( RET_OK == aDlg.Execute() );
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
        DBG_UNHANDLED_EXCEPTION();
    }

    SfxItemPool::Free(pPool);
    for (sal_uInt16 i=0; i<sizeof(pDefaults)/sizeof(pDefaults[0]); ++i)
        delete pDefaults[i];

    return bSuccess;
}
// -----------------------------------------------------------------------------
bool openAreaDialog( const uno::Reference<report::XShape >& _xShape,const uno::Reference< awt::XWindow>& _rxParentWindow )
{
    OSL_PRECOND( _xShape.is() && _rxParentWindow.is(), "openAreaDialog: invalid parameters!" );
    if ( !_xShape.is() || !_rxParentWindow.is() )
        return false;

    ::boost::shared_ptr<rptui::OReportModel> pModel  = ::reportdesign::OReportDefinition::getSdrModel(_xShape->getSection()->getReportDefinition());

    Window* pParent = VCLUnoHelper::GetWindow( _rxParentWindow );

    bool bSuccess = false;
    try
    {
        SfxItemPool& rItemPool = pModel->GetItemPool();
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<SfxItemSet> pDescriptor( new SfxItemSet( rItemPool, rItemPool.GetFirstWhich(),rItemPool.GetLastWhich() ) );
        SAL_WNODEPRECATED_DECLARATIONS_POP
        lcl_fillShapeToItems(_xShape,*pDescriptor);

        {   // want the dialog to be destroyed before our set
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            SAL_WNODEPRECATED_DECLARATIONS_PUSH
            ::std::auto_ptr<AbstractSvxAreaTabDialog> pDialog(pFact->CreateSvxAreaTabDialog( pParent,pDescriptor.get(),pModel.get() ));
            SAL_WNODEPRECATED_DECLARATIONS_POP
            if ( ( bSuccess = ( RET_OK == pDialog->Execute() ) ) )
                lcl_fillItemsToShape(_xShape,*pDialog->GetOutputItemSet());
        }
    }
    catch(uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }

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
            OUString sTemp = aAwtFont.Name;
            aAwtFont.Name = OUString(); // hack to
            _rxReportControlFormat->setFontDescriptor( aAwtFont );
            _rxReportControlFormat->setCharFontName( sTemp );
        }
        if ( aSettings.get( "FontAsian" ) >>= aAwtFont )
        {
            OUString sTemp = aAwtFont.Name;
            aAwtFont.Name = OUString(); // hack to
            _rxReportControlFormat->setFontDescriptorAsian( aAwtFont );
            _rxReportControlFormat->setCharFontNameAsian( sTemp );
        }
        if ( aSettings.get( "FontComplex" ) >>= aAwtFont )
        {
            OUString sTemp = aAwtFont.Name;
            aAwtFont.Name = OUString(); // hack to
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
        DBG_UNHANDLED_EXCEPTION();
    }
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
SdrObject* isOver(const Rectangle& _rRect, SdrPage& _rPage, SdrView& _rView, bool _bAllObjects, SdrObject* _pIgnore, sal_Int16 _nIgnoreType)
{
    SdrObject* pOverlappedObj = NULL;
    SdrObjListIter aIter(_rPage,IM_DEEPNOGROUPS);
    SdrObject* pObjIter = NULL;

    while( !pOverlappedObj && (pObjIter = aIter.Next()) != NULL )
    {
        if ( _pIgnore != pObjIter
            && (_bAllObjects || !_rView.IsObjMarked(pObjIter))
            && (dynamic_cast<OUnoObject*>(pObjIter) != NULL || dynamic_cast<OOle2Obj*>(pObjIter) != NULL))
        {
            if (_nIgnoreType == ISOVER_IGNORE_CUSTOMSHAPES && pObjIter->GetObjIdentifier() == OBJ_CUSTOMSHAPE)
            {
                continue;
            }

            if (dynamic_cast<OUnoObject*>(pObjIter) != NULL || dynamic_cast<OOle2Obj*>(pObjIter) != NULL)
            {
                Rectangle aRect = _rRect.GetIntersection(pObjIter->GetLastBoundRect());
                if ( !aRect.IsEmpty() && (aRect.Left() != aRect.Right() && aRect.Top() != aRect.Bottom() ) )
                    pOverlappedObj = pObjIter;
            }
        }
    }
    return pOverlappedObj;
}
// -----------------------------------------------------------------------------
bool checkArrayForOccurrence(SdrObject* _pObjToCheck, SdrUnoObj* _pIgnore[], int _nListLength)
{
    for(int i=0;i<_nListLength;i++)
    {
        SdrObject *pIgnore = _pIgnore[i];
        if (pIgnore == _pObjToCheck)
        {
            return true;
        }
    }
    return false;
}

SdrObject* isOver(const Rectangle& _rRect,SdrPage& _rPage,SdrView& _rView,bool _bAllObjects, SdrUnoObj * _pIgnoreList[], int _nIgnoreListLength)
{
    SdrObject* pOverlappedObj = NULL;
    SdrObjListIter aIter(_rPage,IM_DEEPNOGROUPS);
    SdrObject* pObjIter = NULL;

    while( !pOverlappedObj && (pObjIter = aIter.Next()) != NULL )
    {
        if (checkArrayForOccurrence(pObjIter, _pIgnoreList, _nIgnoreListLength))
        {
            continue;
        }

        if ( (_bAllObjects || !_rView.IsObjMarked(pObjIter))
             && (dynamic_cast<OUnoObject*>(pObjIter) != NULL || dynamic_cast<OOle2Obj*>(pObjIter) != NULL) )
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
    if (dynamic_cast<OUnoObject*>(_pObj) != NULL || dynamic_cast<OOle2Obj*>(_pObj) != NULL) // this doesn't need to be done for shapes
    {
        Rectangle aRect = _pObj->GetCurrentBoundRect();
        pOverlappedObj = isOver(aRect,_rPage,_rView,_bUnMarkedObjects,_pObj);
    }
    return pOverlappedObj;
}

// -----------------------------------------------------------------------------
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
        DBG_UNHANDLED_EXCEPTION();
    }

    return aNames;
}
// -----------------------------------------------------------------------------
Rectangle getRectangleFromControl(SdrObject* _pControl)
{
    if (_pControl)
    {
        uno::Reference< report::XReportComponent > xComponent( _pControl->getUnoShape(), uno::UNO_QUERY);
        if (xComponent.is())
        {
            Rectangle aRect(VCLPoint(xComponent->getPosition()),VCLSize(xComponent->getSize()));
            aRect.setHeight(aRect.getHeight() + 1);
            aRect.setWidth(aRect.getWidth() + 1);
            return aRect;
        }
    }
    return Rectangle();
}
// -----------------------------------------------------------------------------
// check overlapping
void correctOverlapping(SdrObject* _pControl,OReportSection& _aReportSection,bool _bInsert)
{
    OSectionView& rSectionView = _aReportSection.getSectionView();
    uno::Reference< report::XReportComponent> xComponent(_pControl->getUnoShape(),uno::UNO_QUERY);
    Rectangle aRect = getRectangleFromControl(_pControl);

    bool bOverlapping = true;
    while ( bOverlapping )
    {
        SdrObject* pOverlappedObj = isOver(aRect,*_aReportSection.getPage(),rSectionView,true, _pControl);
        bOverlapping = pOverlappedObj != NULL;
        if ( bOverlapping )
        {
            const Rectangle& aLogicRect = pOverlappedObj->GetLogicRect();
            aRect.Move(0,aLogicRect.Top() + aLogicRect.getHeight() - aRect.Top());
            xComponent->setPositionY(aRect.Top());
        }
    }
    if ( !bOverlapping && _bInsert ) // now insert objects
        rSectionView.InsertObjectAtView(_pControl,*rSectionView.GetSdrPageView(),SDRINSERT_ADDMARK);
}
// -----------------------------------------------------------------------------
void setZoomFactor(const Fraction& _aZoom,Window& _rWindow)
{
    MapMode aMapMode( _rWindow.GetMapMode() );
    aMapMode.SetScaleX(_aZoom);
    aMapMode.SetScaleY(_aZoom);
    _rWindow.SetMapMode(aMapMode);
}
// -----------------------------------------------------------------------------
bool openDialogFormula_nothrow( OUString& _in_out_rFormula
                               , const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _xContext
                               , const uno::Reference< awt::XWindow>& _xInspectorWindow
                               , const ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySet >& _xRowSet
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
        Window* pParent = VCLUnoHelper::GetWindow( _xInspectorWindow );

        uno::Reference< report::meta::XFunctionManager> xMgr(xFactory->createInstanceWithContext("org.libreoffice.report.pentaho.SOFunctionManager",_xContext),uno::UNO_QUERY);
        if ( xMgr.is() )
        {
            ::boost::shared_ptr< formula::IFunctionManager > pFormulaManager(new FunctionManager(xMgr) );
            ReportFormula aFormula( _in_out_rFormula );
            FormulaDialog aDlg(pParent,xServiceFactory,pFormulaManager,aFormula.getUndecoratedContent(),_xRowSet);
            bSuccess = aDlg.Execute() == RET_OK;
            if ( bSuccess )
            {
                String sFormula = aDlg.getCurrentFormula();
                xub_StrLen nIndex = 0;
                if ( sFormula.GetChar(0) == '=' )
                    nIndex = 1;
                _in_out_rFormula = OUString("rpt:") + sFormula.Copy(nIndex);
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
// -----------------------------------------------------------------------------
} // namespace rptui
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
