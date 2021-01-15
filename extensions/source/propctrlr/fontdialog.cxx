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

#include "fontdialog.hxx"
#include <vcl/outdev.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/unohelp.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/types.hxx>
#include <comphelper/extract.hxx>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include "formstrings.hxx"
#include "fontitemids.hxx"
#include <editeng/charreliefitem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/flstitem.hxx>
#include <svtools/ctrltool.hxx>
#include <tools/diagnose_ex.h>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <svx/svxids.hrc>
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <svx/flagsdef.hxx>


namespace pcr
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;


    //= OFontPropertyExtractor

    namespace {

    class OFontPropertyExtractor
    {
    protected:
        css::uno::Reference< css::beans::XPropertySet >
                    m_xPropValueAccess;
        css::uno::Reference< css::beans::XPropertyState >
                    m_xPropStateAccess;

    public:
        explicit OFontPropertyExtractor( const css::uno::Reference< css::beans::XPropertySet >&
            _rxProps );

    public:
        bool            getCheckFontProperty(const OUString& _rPropName, css::uno::Any& _rValue);
        OUString        getStringFontProperty(const OUString& _rPropName, const OUString& _rDefault);
        sal_Int16       getInt16FontProperty(const OUString& _rPropName, const sal_Int16 _nDefault);
        sal_Int32       getInt32FontProperty(const OUString& _rPropName, const sal_Int32 _nDefault);
        float           getFloatFontProperty(const OUString& _rPropName, const float _nDefault);

        void            invalidateItem(
                            const OUString& _rPropName,
                            sal_uInt16 _nItemId,
                            SfxItemSet& _rSet,
                            bool _bForceInvalidation = false);
    };

    }

    OFontPropertyExtractor::OFontPropertyExtractor(const Reference< XPropertySet >& _rxProps)
        :m_xPropValueAccess(_rxProps)
        ,m_xPropStateAccess(_rxProps, UNO_QUERY)
    {
        OSL_ENSURE(m_xPropValueAccess.is(), "OFontPropertyExtractor::OFontPropertyExtractor: invalid property set!");
    }


    bool OFontPropertyExtractor::getCheckFontProperty(const OUString& _rPropName, Any& _rValue)
    {
        _rValue = m_xPropValueAccess->getPropertyValue(_rPropName);
        if (m_xPropStateAccess.is())
            return PropertyState_DEFAULT_VALUE == m_xPropStateAccess->getPropertyState(_rPropName);

        return false;
    }


    OUString OFontPropertyExtractor::getStringFontProperty(const OUString& _rPropName, const OUString& _rDefault)
    {
        Any aValue;
        if (getCheckFontProperty(_rPropName, aValue))
            return _rDefault;

        return ::comphelper::getString(aValue);
    }


    sal_Int16 OFontPropertyExtractor::getInt16FontProperty(const OUString& _rPropName, const sal_Int16 _nDefault)
    {
        Any aValue;
        if (getCheckFontProperty(_rPropName, aValue))
            return _nDefault;

        sal_Int32 nValue(_nDefault);
        ::cppu::enum2int(nValue, aValue);
        return static_cast<sal_Int16>(nValue);
    }


    sal_Int32 OFontPropertyExtractor::getInt32FontProperty(const OUString& _rPropName, const sal_Int32 _nDefault)
    {
        Any aValue;
        if (getCheckFontProperty(_rPropName, aValue))
            return _nDefault;

        sal_Int32 nValue(_nDefault);
        ::cppu::enum2int(nValue, aValue);
        return nValue;
    }


    float OFontPropertyExtractor::getFloatFontProperty(const OUString& _rPropName, const float _nDefault)
    {
        Any aValue;
        if (getCheckFontProperty(_rPropName, aValue))
            return _nDefault;

        return ::comphelper::getFloat(aValue);
    }


    void OFontPropertyExtractor::invalidateItem(const OUString& _rPropName, sal_uInt16 _nItemId, SfxItemSet& _rSet, bool _bForceInvalidation)
    {
        if  (   _bForceInvalidation
            ||  (   m_xPropStateAccess.is()
                && (PropertyState_AMBIGUOUS_VALUE == m_xPropStateAccess->getPropertyState(_rPropName))
                )
            )
            _rSet.InvalidateItem(_nItemId);
    }

    //= ControlCharacterDialog
    ControlCharacterDialog::ControlCharacterDialog(weld::Window* pParent, const SfxItemSet& _rCoreSet)
        : SfxTabDialogController(pParent, "modules/spropctrlr/ui/controlfontdialog.ui", "ControlFontDialog", &_rCoreSet)
    {
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        AddTabPage("font", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_CHAR_NAME), nullptr );
        AddTabPage("fonteffects", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_CHAR_EFFECTS), nullptr );
    }

    ControlCharacterDialog::~ControlCharacterDialog()
    {
    }

    void ControlCharacterDialog::translatePropertiesToItems(const Reference< XPropertySet >& _rxModel, SfxItemSet* _pSet)
    {
        OSL_ENSURE(_pSet && _rxModel.is(), "ControlCharacterDialog::translatePropertiesToItems: invalid arguments!");
        if (!_pSet || !_rxModel.is())
            return;

        try
        {
            OFontPropertyExtractor aPropExtractor(_rxModel);

            // some items, which may be in default state, have to be filled with non-void information
            vcl::Font aDefaultVCLFont = Application::GetDefaultDevice()->GetSettings().GetStyleSettings().GetAppFont();
            css::awt::FontDescriptor aDefaultFont = VCLUnoHelper::CreateFontDescriptor(aDefaultVCLFont);

            // get the current properties
            OUString aFontName       = aPropExtractor.getStringFontProperty(PROPERTY_FONT_NAME, aDefaultFont.Name);
            OUString aFontStyleName  = aPropExtractor.getStringFontProperty(PROPERTY_FONT_STYLENAME, aDefaultFont.StyleName);
            sal_Int16   nFontFamily         = aPropExtractor.getInt16FontProperty(PROPERTY_FONT_FAMILY, aDefaultFont.Family);
            sal_Int16   nFontCharset        = aPropExtractor.getInt16FontProperty(PROPERTY_FONT_CHARSET, aDefaultFont.CharSet);
            float   nFontHeight             = aPropExtractor.getFloatFontProperty(PROPERTY_FONT_HEIGHT, static_cast<float>(aDefaultFont.Height));
            float   nFontWeight             = aPropExtractor.getFloatFontProperty(PROPERTY_FONT_WEIGHT, aDefaultFont.Weight);
            css::awt::FontSlant nFontSlant  = static_cast<css::awt::FontSlant>(aPropExtractor.getInt16FontProperty(PROPERTY_FONT_SLANT, static_cast<sal_Int16>(aDefaultFont.Slant)));
            sal_Int16 nFontLineStyle        = aPropExtractor.getInt16FontProperty(PROPERTY_FONT_UNDERLINE, aDefaultFont.Underline);
            sal_Int16 nFontStrikeout        = aPropExtractor.getInt16FontProperty(PROPERTY_FONT_STRIKEOUT, aDefaultFont.Strikeout);

            sal_Int32 nTextLineColor        = aPropExtractor.getInt32FontProperty(PROPERTY_TEXTLINECOLOR, sal_uInt32(COL_AUTO));
            sal_Int16 nFontRelief           = aPropExtractor.getInt16FontProperty(PROPERTY_FONT_RELIEF, static_cast<sal_Int16>(aDefaultVCLFont.GetRelief()));
            sal_Int16 nFontEmphasisMark     = aPropExtractor.getInt16FontProperty(PROPERTY_FONT_EMPHASIS_MARK, static_cast<sal_uInt16>(aDefaultVCLFont.GetEmphasisMark()));

            Any aValue;
            bool bWordLineMode          = aPropExtractor.getCheckFontProperty(PROPERTY_WORDLINEMODE, aValue) ? aDefaultFont.WordLineMode : ::cppu::any2bool(aValue);
            sal_Int32 nColor32              = aPropExtractor.getInt32FontProperty(PROPERTY_TEXTCOLOR, 0);

            // build SfxItems with the values
            SvxFontItem aFontItem(static_cast<FontFamily>(nFontFamily), aFontName, aFontStyleName, PITCH_DONTKNOW, nFontCharset, CFID_FONT);

            nFontHeight = static_cast<float>(OutputDevice::LogicToLogic(Size(0, static_cast<sal_Int32>(nFontHeight)), MapMode(MapUnit::MapPoint), MapMode(MapUnit::MapTwip)).Height());
            SvxFontHeightItem aSvxFontHeightItem(static_cast<sal_uInt32>(nFontHeight),100,CFID_HEIGHT);

            FontWeight      eWeight=vcl::unohelper::ConvertFontWeight(nFontWeight);
            FontItalic      eItalic=vcl::unohelper::ConvertFontSlant(nFontSlant);
            FontLineStyle    eUnderline=static_cast<FontLineStyle>(nFontLineStyle);
            FontStrikeout   eStrikeout=static_cast<FontStrikeout>(nFontStrikeout);

            SvxWeightItem       aWeightItem(eWeight,CFID_WEIGHT);
            SvxPostureItem      aPostureItem(eItalic,CFID_POSTURE);

            SvxCrossedOutItem   aCrossedOutItem(eStrikeout,CFID_STRIKEOUT);
            SvxWordLineModeItem aWordLineModeItem(bWordLineMode, CFID_WORDLINEMODE);

            SvxUnderlineItem    aUnderlineItem(eUnderline,CFID_UNDERLINE);
            aUnderlineItem.SetColor(Color(ColorTransparency, nTextLineColor));

            SvxColorItem aSvxColorItem(Color(ColorTransparency, nColor32),CFID_CHARCOLOR);
            SvxLanguageItem aLanguageItem(Application::GetSettings().GetUILanguageTag().getLanguageType(), CFID_LANGUAGE);

            // the 2 CJK props
            SvxCharReliefItem aFontReliefItem(static_cast<FontRelief>(nFontRelief), CFID_RELIEF);
            SvxEmphasisMarkItem aEmphasisMarkitem(static_cast<FontEmphasisMark>(nFontEmphasisMark), CFID_EMPHASIS);

            _pSet->Put(aFontItem);
            _pSet->Put(aSvxFontHeightItem);
            _pSet->Put(aWeightItem);
            _pSet->Put(aPostureItem);
            _pSet->Put(aLanguageItem);
            _pSet->Put(aUnderlineItem);
            _pSet->Put(aCrossedOutItem);
            _pSet->Put(aWordLineModeItem);
            _pSet->Put(aSvxColorItem);
            _pSet->Put(aFontReliefItem);
            _pSet->Put(aEmphasisMarkitem);

            aPropExtractor.invalidateItem(PROPERTY_FONT_NAME, CFID_FONT, *_pSet);
            aPropExtractor.invalidateItem(PROPERTY_FONT_HEIGHT, CFID_HEIGHT, *_pSet);
            aPropExtractor.invalidateItem(PROPERTY_FONT_WEIGHT, CFID_WEIGHT, *_pSet, css::awt::FontWeight::DONTKNOW == nFontWeight);
            aPropExtractor.invalidateItem(PROPERTY_FONT_SLANT, CFID_POSTURE, *_pSet, css::awt::FontSlant_DONTKNOW == nFontSlant);
            aPropExtractor.invalidateItem(PROPERTY_FONT_UNDERLINE, CFID_UNDERLINE, *_pSet, css::awt::FontUnderline::DONTKNOW == nFontLineStyle);
            aPropExtractor.invalidateItem(PROPERTY_FONT_STRIKEOUT, CFID_STRIKEOUT, *_pSet, css::awt::FontStrikeout::DONTKNOW == nFontStrikeout);
            aPropExtractor.invalidateItem(PROPERTY_WORDLINEMODE, CFID_WORDLINEMODE, *_pSet);
            aPropExtractor.invalidateItem(PROPERTY_TEXTCOLOR, CFID_CHARCOLOR, *_pSet);
            aPropExtractor.invalidateItem(PROPERTY_FONT_RELIEF, CFID_RELIEF, *_pSet);
            aPropExtractor.invalidateItem(PROPERTY_FONT_EMPHASIS_MARK, CFID_EMPHASIS, *_pSet);
        }
        catch (const Exception&)
        {
            TOOLS_WARN_EXCEPTION( "extensions.propctrlr", "ControlCharacterDialog::translatePropertiesToItems");
        }

        _pSet->DisableItem(SID_ATTR_CHAR_CJK_FONT);
        _pSet->DisableItem(SID_ATTR_CHAR_CJK_FONTHEIGHT);
        _pSet->DisableItem(SID_ATTR_CHAR_CJK_LANGUAGE);
        _pSet->DisableItem(SID_ATTR_CHAR_CJK_POSTURE);
        _pSet->DisableItem(SID_ATTR_CHAR_CJK_WEIGHT);

        _pSet->DisableItem(SID_ATTR_CHAR_CASEMAP);
        _pSet->DisableItem(SID_ATTR_CHAR_CONTOUR);
        _pSet->DisableItem(SID_ATTR_CHAR_SHADOWED);
    }

    namespace
    {
        void lcl_pushBackPropertyValue( std::vector< NamedValue >& _out_properties, const OUString& _name, const Any& _value )
        {
            _out_properties.push_back( NamedValue( _name, _value ) );
        }
    }

    void ControlCharacterDialog::translateItemsToProperties( const SfxItemSet& _rSet, std::vector< NamedValue >& _out_properties )
    {
        _out_properties.clear();

        try
        {

            // font name
            SfxItemState eState = _rSet.GetItemState(CFID_FONT);

            if ( eState == SfxItemState::SET )
            {
                const SvxFontItem& rFontItem =
                    static_cast<const SvxFontItem&>(_rSet.Get(CFID_FONT));

                lcl_pushBackPropertyValue( _out_properties, PROPERTY_FONT_NAME     , makeAny(rFontItem.GetFamilyName()));
                lcl_pushBackPropertyValue( _out_properties, PROPERTY_FONT_STYLENAME, makeAny(rFontItem.GetStyleName()));
                lcl_pushBackPropertyValue( _out_properties, PROPERTY_FONT_FAMILY   , makeAny(static_cast<sal_Int16>(rFontItem.GetFamily())));
                lcl_pushBackPropertyValue( _out_properties, PROPERTY_FONT_CHARSET  , makeAny(static_cast<sal_Int16>(rFontItem.GetCharSet())));
            }


            // font height
            eState = _rSet.GetItemState(CFID_HEIGHT);

            if ( eState == SfxItemState::SET )
            {
                const SvxFontHeightItem& rSvxFontHeightItem =
                    static_cast<const SvxFontHeightItem&>(_rSet.Get(CFID_HEIGHT));

                float nHeight = static_cast<float>(OutputDevice::LogicToLogic(Size(0, rSvxFontHeightItem.GetHeight()), MapMode(MapUnit::MapTwip), MapMode(MapUnit::MapPoint)).Height());
                lcl_pushBackPropertyValue( _out_properties, PROPERTY_FONT_HEIGHT,makeAny(nHeight));

            }


            // font weight
            eState = _rSet.GetItemState(CFID_WEIGHT);

            if ( eState == SfxItemState::SET )
            {
                const SvxWeightItem& rWeightItem =
                    static_cast<const SvxWeightItem&>(_rSet.Get(CFID_WEIGHT));

                float nWeight = vcl::unohelper::ConvertFontWeight(rWeightItem.GetWeight());
                lcl_pushBackPropertyValue( _out_properties, PROPERTY_FONT_WEIGHT,makeAny(nWeight));
            }


            // font slant
            eState = _rSet.GetItemState(CFID_POSTURE);

            if ( eState == SfxItemState::SET )
            {
                const SvxPostureItem& rPostureItem =
                    static_cast<const SvxPostureItem&>(_rSet.Get(CFID_POSTURE));

                css::awt::FontSlant eSlant = vcl::unohelper::ConvertFontSlant(rPostureItem.GetPosture());
                lcl_pushBackPropertyValue( _out_properties, PROPERTY_FONT_SLANT, makeAny(static_cast<sal_Int16>(eSlant)));
            }


            // font underline
            eState = _rSet.GetItemState(CFID_UNDERLINE);

            if ( eState == SfxItemState::SET )
            {
                const SvxUnderlineItem& rUnderlineItem =
                    static_cast<const SvxUnderlineItem&>(_rSet.Get(CFID_UNDERLINE));

                sal_Int16 nUnderline = static_cast<sal_Int16>(rUnderlineItem.GetLineStyle());
                lcl_pushBackPropertyValue( _out_properties, PROPERTY_FONT_UNDERLINE,makeAny(nUnderline));

                // the text line color is transported in this item, too
                Color nColor = rUnderlineItem.GetColor();

                Any aUnoColor;
                if (COL_AUTO != nColor)
                    aUnoColor <<= nColor;

                lcl_pushBackPropertyValue( _out_properties, PROPERTY_TEXTLINECOLOR, aUnoColor );
            }


            // font strikeout
            eState = _rSet.GetItemState(CFID_STRIKEOUT);

            if ( eState == SfxItemState::SET )
            {
                const SvxCrossedOutItem& rCrossedOutItem =
                    static_cast<const SvxCrossedOutItem&>(_rSet.Get(CFID_STRIKEOUT));

                sal_Int16 nStrikeout = static_cast<sal_Int16>(rCrossedOutItem.GetStrikeout());
                lcl_pushBackPropertyValue( _out_properties, PROPERTY_FONT_STRIKEOUT,makeAny(nStrikeout));
            }


            // font wordline mode
            eState = _rSet.GetItemState(CFID_WORDLINEMODE);

            if ( eState == SfxItemState::SET )
            {
                const SvxWordLineModeItem& rWordLineModeItem =
                    static_cast<const SvxWordLineModeItem&>(_rSet.Get(CFID_WORDLINEMODE));

                lcl_pushBackPropertyValue( _out_properties, PROPERTY_WORDLINEMODE, css::uno::makeAny(rWordLineModeItem.GetValue()));
            }


            // text color
            eState = _rSet.GetItemState(CFID_CHARCOLOR);

            if ( eState == SfxItemState::SET )
            {
                const SvxColorItem& rColorItem =
                    static_cast<const SvxColorItem&>(_rSet.Get(CFID_CHARCOLOR));

                Color nColor = rColorItem.GetValue();

                Any aUnoColor;
                if (COL_AUTO != nColor)
                    aUnoColor <<= nColor;

                lcl_pushBackPropertyValue( _out_properties, PROPERTY_TEXTCOLOR, aUnoColor );
            }


            // font relief
            eState = _rSet.GetItemState(CFID_RELIEF);

            if ( eState == SfxItemState::SET )
            {
                const SvxCharReliefItem& rReliefItem =
                    static_cast<const SvxCharReliefItem&>(_rSet.Get(CFID_RELIEF));

                lcl_pushBackPropertyValue( _out_properties, PROPERTY_FONT_RELIEF, makeAny(static_cast<sal_Int16>(rReliefItem.GetValue())) );
            }


            // font emphasis mark
            eState = _rSet.GetItemState(CFID_EMPHASIS);

            if ( eState == SfxItemState::SET )
            {
                const SvxEmphasisMarkItem& rEmphMarkItem =
                    static_cast<const SvxEmphasisMarkItem&>(_rSet.Get(CFID_EMPHASIS));

                lcl_pushBackPropertyValue( _out_properties, PROPERTY_FONT_EMPHASIS_MARK, makeAny(static_cast<sal_Int16>(rEmphMarkItem.GetEmphasisMark())) );
            }
        }
        catch (const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("extensions.propctrlr");
        }
    }

    void ControlCharacterDialog::translateItemsToProperties( const SfxItemSet& _rSet, const Reference< XPropertySet >& _rxModel)
    {
        OSL_ENSURE( _rxModel.is(), "ControlCharacterDialog::translateItemsToProperties: invalid arguments!" );
        if ( !_rxModel.is())
            return;

        std::vector< NamedValue > aPropertyValues;
        translateItemsToProperties( _rSet, aPropertyValues );
        try
        {
            for ( const NamedValue& rNV : aPropertyValues )
                _rxModel->setPropertyValue( rNV.Name, rNV.Value );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("extensions.propctrlr");
        }
    }

    void ControlCharacterDialog::createItemSet(std::unique_ptr<SfxItemSet>& _rpSet, SfxItemPool*& _rpPool, std::vector<SfxPoolItem*>*& _rpDefaults)
    {
        // just to be sure...
        _rpSet = nullptr;
        _rpPool = nullptr;
        _rpDefaults = nullptr;

        // create and initialize the defaults
        _rpDefaults = new std::vector<SfxPoolItem*>(CFID_LAST_ITEM_ID - CFID_FIRST_ITEM_ID + 1);

        vcl::Font aDefaultVCLFont = Application::GetDefaultDevice()->GetSettings().GetStyleSettings().GetAppFont();

        SfxPoolItem** pCounter = _rpDefaults->data();  // want to modify this without affecting the out param _rppDefaults
        *pCounter++ = new SvxFontItem(aDefaultVCLFont.GetFamilyType(), aDefaultVCLFont.GetFamilyName(), aDefaultVCLFont.GetStyleName(), aDefaultVCLFont.GetPitch(), aDefaultVCLFont.GetCharSet(), CFID_FONT);
        *pCounter++ = new SvxFontHeightItem(aDefaultVCLFont.GetFontHeight(), 100, CFID_HEIGHT);
        *pCounter++ = new SvxWeightItem(aDefaultVCLFont.GetWeight(), CFID_WEIGHT);
        *pCounter++ = new SvxPostureItem(aDefaultVCLFont.GetItalic(), CFID_POSTURE);
        *pCounter++ = new SvxLanguageItem(Application::GetSettings().GetUILanguageTag().getLanguageType(), CFID_LANGUAGE);
        *pCounter++ = new SvxUnderlineItem(aDefaultVCLFont.GetUnderline(), CFID_UNDERLINE);
        *pCounter++ = new SvxCrossedOutItem(aDefaultVCLFont.GetStrikeout(), CFID_STRIKEOUT);
        *pCounter++ = new SvxWordLineModeItem(aDefaultVCLFont.IsWordLineMode(), CFID_WORDLINEMODE);
        *pCounter++ = new SvxColorItem(aDefaultVCLFont.GetColor(), CFID_CHARCOLOR);
        *pCounter++ = new SvxCharReliefItem(aDefaultVCLFont.GetRelief(), CFID_RELIEF);
        *pCounter++ = new SvxEmphasisMarkItem(aDefaultVCLFont.GetEmphasisMark(), CFID_EMPHASIS);

        *pCounter++ = new SvxFontItem(aDefaultVCLFont.GetFamilyType(), aDefaultVCLFont.GetFamilyName(), aDefaultVCLFont.GetStyleName(), aDefaultVCLFont.GetPitch(), aDefaultVCLFont.GetCharSet(), CFID_CJK_FONT);
        *pCounter++ = new SvxFontHeightItem(aDefaultVCLFont.GetFontHeight(), 100, CFID_CJK_HEIGHT);
        *pCounter++ = new SvxWeightItem(aDefaultVCLFont.GetWeight(), CFID_CJK_WEIGHT);
        *pCounter++ = new SvxPostureItem(aDefaultVCLFont.GetItalic(), CFID_CJK_POSTURE);
        *pCounter++ = new SvxLanguageItem(Application::GetSettings().GetUILanguageTag().getLanguageType(), CFID_CJK_LANGUAGE);

        *pCounter++ = new SvxCaseMapItem(SvxCaseMap::NotMapped, CFID_CASEMAP);
        *pCounter++ = new SvxContourItem(false, CFID_CONTOUR);
        *pCounter++ = new SvxShadowedItem(false, CFID_SHADOWED);

        *pCounter++ = new SvxFontListItem (new FontList(Application::GetDefaultDevice()), CFID_FONTLIST);

        // create the pool
        static SfxItemInfo const aItemInfos[CFID_LAST_ITEM_ID - CFID_FIRST_ITEM_ID + 1] =
        {
            { SID_ATTR_CHAR_FONT,               false },
            { SID_ATTR_CHAR_FONTHEIGHT,         false },
            { SID_ATTR_CHAR_WEIGHT,             false },
            { SID_ATTR_CHAR_POSTURE,            false },
            { SID_ATTR_CHAR_LANGUAGE,           false },
            { SID_ATTR_CHAR_UNDERLINE,          false },
            { SID_ATTR_CHAR_STRIKEOUT,          false },
            { SID_ATTR_CHAR_WORDLINEMODE,       false },
            { SID_ATTR_CHAR_COLOR,              false },
            { SID_ATTR_CHAR_RELIEF,             false },
            { SID_ATTR_CHAR_EMPHASISMARK,       false },
            { 0,                                false },
            { 0,                                false },
            { 0,                                false },
            { 0,                                false },
            { 0,                                false },
            { 0,                                false },
            { 0,                                false },
            { 0,                                false },
            { SID_ATTR_CHAR_FONTLIST,           false }
        };

        _rpPool = new SfxItemPool("PCRControlFontItemPool", CFID_FIRST_ITEM_ID, CFID_LAST_ITEM_ID,
            aItemInfos, _rpDefaults);
        _rpPool->FreezeIdRanges();

        // and, finally, the set
        _rpSet.reset(new SfxItemSet(*_rpPool));
    }

    void ControlCharacterDialog::destroyItemSet(std::unique_ptr<SfxItemSet>& _rpSet, SfxItemPool*& _rpPool, std::vector<SfxPoolItem*>*& _rpDefaults)
    {
        // from the pool, get and remember the font list (needs to be deleted)
        const SvxFontListItem& rFontListItem = static_cast<const SvxFontListItem&>(_rpPool->GetDefaultItem(CFID_FONTLIST));
        const FontList* pFontList = rFontListItem.GetFontList();

        // _first_ delete the set (referring the pool)
        _rpSet.reset();

        // delete the pool
        _rpPool->ReleaseDefaults(true);
            // the "true" means delete the items, too
        SfxItemPool::Free(_rpPool);
        _rpPool = nullptr;

        // reset the defaults ptr
        _rpDefaults = nullptr;
            // no need to explicitly delete the defaults, this has been done by the ReleaseDefaults

        delete pFontList;
    }

    void ControlCharacterDialog::PageCreated(const OString& rId, SfxTabPage& rPage)
    {
        SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
        if (rId == "font")
        {
            aSet.Put (static_cast<const SvxFontListItem&>(GetInputSetImpl()->Get(CFID_FONTLIST)));
            aSet.Put (SfxUInt16Item(SID_DISABLE_CTL,DISABLE_HIDE_LANGUAGE));
            rPage.PageCreated(aSet);
        }
    }
}   // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
