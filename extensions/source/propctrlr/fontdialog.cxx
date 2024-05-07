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
#include <comphelper/diagnose_ex.hxx>
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

    namespace FontItemIds
    {
        constexpr sal_uInt16 CFID_FONT =             1;
        constexpr sal_uInt16 CFID_HEIGHT =           2;
        constexpr sal_uInt16 CFID_WEIGHT =           3;
        constexpr sal_uInt16 CFID_POSTURE =          4;
        constexpr sal_uInt16 CFID_LANGUAGE =         5;
        constexpr sal_uInt16 CFID_UNDERLINE =        6;
        constexpr sal_uInt16 CFID_STRIKEOUT =        7;
        constexpr TypedWhichId<SvxWordLineModeItem> CFID_WORDLINEMODE(8);
        constexpr sal_uInt16 CFID_CHARCOLOR =        9;
        constexpr sal_uInt16 CFID_RELIEF =           10;
        constexpr TypedWhichId<SvxEmphasisMarkItem> CFID_EMPHASIS(11);

        constexpr sal_uInt16 CFID_CJK_FONT =         12;
        constexpr sal_uInt16 CFID_CJK_HEIGHT =       13;
        constexpr sal_uInt16 CFID_CJK_WEIGHT =       14;
        constexpr sal_uInt16 CFID_CJK_POSTURE =      15;
        constexpr sal_uInt16 CFID_CJK_LANGUAGE =     16;
        constexpr sal_uInt16 CFID_CASEMAP =          17;
        constexpr TypedWhichId<SvxContourItem> CFID_CONTOUR(18);
        constexpr TypedWhichId<SvxShadowedItem> CFID_SHADOWED(19);

        constexpr sal_uInt16 CFID_FONTLIST =         20;

        constexpr sal_uInt16 CFID_FIRST_ITEM_ID =    CFID_FONT;
        constexpr sal_uInt16 CFID_LAST_ITEM_ID =     CFID_FONTLIST;
    }

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
        : SfxTabDialogController(pParent, u"modules/spropctrlr/ui/controlfontdialog.ui"_ustr, u"ControlFontDialog"_ustr, &_rCoreSet)
    {
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        AddTabPage(u"font"_ustr, pFact->GetTabPageCreatorFunc(RID_SVXPAGE_CHAR_NAME), nullptr );
        AddTabPage(u"fonteffects"_ustr, pFact->GetTabPageCreatorFunc(RID_SVXPAGE_CHAR_EFFECTS), nullptr );
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
            SvxFontItem aFontItem(static_cast<FontFamily>(nFontFamily), aFontName, aFontStyleName, PITCH_DONTKNOW, nFontCharset, FontItemIds::CFID_FONT);

            nFontHeight = static_cast<float>(o3tl::convert(nFontHeight, o3tl::Length::pt, o3tl::Length::twip));

            SvxFontHeightItem aSvxFontHeightItem(static_cast<sal_uInt32>(nFontHeight),100,FontItemIds::CFID_HEIGHT);

            FontWeight      eWeight=vcl::unohelper::ConvertFontWeight(nFontWeight);
            FontItalic      eItalic=vcl::unohelper::ConvertFontSlant(nFontSlant);
            FontLineStyle    eUnderline=static_cast<FontLineStyle>(nFontLineStyle);
            FontStrikeout   eStrikeout=static_cast<FontStrikeout>(nFontStrikeout);

            SvxWeightItem       aWeightItem(eWeight,FontItemIds::CFID_WEIGHT);
            SvxPostureItem      aPostureItem(eItalic,FontItemIds::CFID_POSTURE);

            SvxCrossedOutItem   aCrossedOutItem(eStrikeout,FontItemIds::CFID_STRIKEOUT);
            SvxWordLineModeItem aWordLineModeItem(bWordLineMode, FontItemIds::CFID_WORDLINEMODE);

            SvxUnderlineItem    aUnderlineItem(eUnderline,FontItemIds::CFID_UNDERLINE);
            aUnderlineItem.SetColor(Color(ColorTransparency, nTextLineColor));

            SvxColorItem aSvxColorItem(Color(ColorTransparency, nColor32),FontItemIds::CFID_CHARCOLOR);
            SvxLanguageItem aLanguageItem(Application::GetSettings().GetUILanguageTag().getLanguageType(), FontItemIds::CFID_LANGUAGE);

            // the 2 CJK props
            SvxCharReliefItem aFontReliefItem(static_cast<FontRelief>(nFontRelief), FontItemIds::CFID_RELIEF);
            SvxEmphasisMarkItem aEmphasisMarkitem(static_cast<FontEmphasisMark>(nFontEmphasisMark), FontItemIds::CFID_EMPHASIS);

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

            aPropExtractor.invalidateItem(PROPERTY_FONT_NAME, FontItemIds::CFID_FONT, *_pSet);
            aPropExtractor.invalidateItem(PROPERTY_FONT_HEIGHT, FontItemIds::CFID_HEIGHT, *_pSet);
            aPropExtractor.invalidateItem(PROPERTY_FONT_WEIGHT, FontItemIds::CFID_WEIGHT, *_pSet, css::awt::FontWeight::DONTKNOW == nFontWeight);
            aPropExtractor.invalidateItem(PROPERTY_FONT_SLANT, FontItemIds::CFID_POSTURE, *_pSet, css::awt::FontSlant_DONTKNOW == nFontSlant);
            aPropExtractor.invalidateItem(PROPERTY_FONT_UNDERLINE, FontItemIds::CFID_UNDERLINE, *_pSet, css::awt::FontUnderline::DONTKNOW == nFontLineStyle);
            aPropExtractor.invalidateItem(PROPERTY_FONT_STRIKEOUT, FontItemIds::CFID_STRIKEOUT, *_pSet, css::awt::FontStrikeout::DONTKNOW == nFontStrikeout);
            aPropExtractor.invalidateItem(PROPERTY_WORDLINEMODE, FontItemIds::CFID_WORDLINEMODE, *_pSet);
            aPropExtractor.invalidateItem(PROPERTY_TEXTCOLOR, FontItemIds::CFID_CHARCOLOR, *_pSet);
            aPropExtractor.invalidateItem(PROPERTY_FONT_RELIEF, FontItemIds::CFID_RELIEF, *_pSet);
            aPropExtractor.invalidateItem(PROPERTY_FONT_EMPHASIS_MARK, FontItemIds::CFID_EMPHASIS, *_pSet);
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
            SfxItemState eState = _rSet.GetItemState(FontItemIds::CFID_FONT);

            if ( eState == SfxItemState::SET )
            {
                const SvxFontItem& rFontItem =
                    static_cast<const SvxFontItem&>(_rSet.Get(FontItemIds::CFID_FONT));

                lcl_pushBackPropertyValue( _out_properties, PROPERTY_FONT_NAME     , Any(rFontItem.GetFamilyName()));
                lcl_pushBackPropertyValue( _out_properties, PROPERTY_FONT_STYLENAME, Any(rFontItem.GetStyleName()));
                lcl_pushBackPropertyValue( _out_properties, PROPERTY_FONT_FAMILY   , Any(static_cast<sal_Int16>(rFontItem.GetFamily())));
                lcl_pushBackPropertyValue( _out_properties, PROPERTY_FONT_CHARSET  , Any(static_cast<sal_Int16>(rFontItem.GetCharSet())));
            }


            // font height
            eState = _rSet.GetItemState(FontItemIds::CFID_HEIGHT);

            if ( eState == SfxItemState::SET )
            {
                const SvxFontHeightItem& rSvxFontHeightItem =
                    static_cast<const SvxFontHeightItem&>(_rSet.Get(FontItemIds::CFID_HEIGHT));

                float nHeight = static_cast<float>(o3tl::convert(rSvxFontHeightItem.GetHeight(), o3tl::Length::twip, o3tl::Length::pt));
                lcl_pushBackPropertyValue( _out_properties, PROPERTY_FONT_HEIGHT,Any(nHeight));

            }


            // font weight
            eState = _rSet.GetItemState(FontItemIds::CFID_WEIGHT);

            if ( eState == SfxItemState::SET )
            {
                const SvxWeightItem& rWeightItem =
                    static_cast<const SvxWeightItem&>(_rSet.Get(FontItemIds::CFID_WEIGHT));

                float nWeight = vcl::unohelper::ConvertFontWeight(rWeightItem.GetWeight());
                lcl_pushBackPropertyValue( _out_properties, PROPERTY_FONT_WEIGHT,Any(nWeight));
            }


            // font slant
            eState = _rSet.GetItemState(FontItemIds::CFID_POSTURE);

            if ( eState == SfxItemState::SET )
            {
                const SvxPostureItem& rPostureItem =
                    static_cast<const SvxPostureItem&>(_rSet.Get(FontItemIds::CFID_POSTURE));

                css::awt::FontSlant eSlant = vcl::unohelper::ConvertFontSlant(rPostureItem.GetPosture());
                lcl_pushBackPropertyValue( _out_properties, PROPERTY_FONT_SLANT, Any(static_cast<sal_Int16>(eSlant)));
            }


            // font underline
            eState = _rSet.GetItemState(FontItemIds::CFID_UNDERLINE);

            if ( eState == SfxItemState::SET )
            {
                const SvxUnderlineItem& rUnderlineItem =
                    static_cast<const SvxUnderlineItem&>(_rSet.Get(FontItemIds::CFID_UNDERLINE));

                sal_Int16 nUnderline = static_cast<sal_Int16>(rUnderlineItem.GetLineStyle());
                lcl_pushBackPropertyValue( _out_properties, PROPERTY_FONT_UNDERLINE,Any(nUnderline));

                // the text line color is transported in this item, too
                Color nColor = rUnderlineItem.GetColor();

                Any aUnoColor;
                if (COL_AUTO != nColor)
                    aUnoColor <<= nColor;

                lcl_pushBackPropertyValue( _out_properties, PROPERTY_TEXTLINECOLOR, aUnoColor );
            }


            // font strikeout
            eState = _rSet.GetItemState(FontItemIds::CFID_STRIKEOUT);

            if ( eState == SfxItemState::SET )
            {
                const SvxCrossedOutItem& rCrossedOutItem =
                    static_cast<const SvxCrossedOutItem&>(_rSet.Get(FontItemIds::CFID_STRIKEOUT));

                sal_Int16 nStrikeout = static_cast<sal_Int16>(rCrossedOutItem.GetStrikeout());
                lcl_pushBackPropertyValue( _out_properties, PROPERTY_FONT_STRIKEOUT,Any(nStrikeout));
            }


            // font wordline mode
            eState = _rSet.GetItemState(FontItemIds::CFID_WORDLINEMODE);

            if ( eState == SfxItemState::SET )
            {
                const SvxWordLineModeItem& rWordLineModeItem =
                    _rSet.Get(FontItemIds::CFID_WORDLINEMODE);

                lcl_pushBackPropertyValue( _out_properties, PROPERTY_WORDLINEMODE, css::uno::Any(rWordLineModeItem.GetValue()));
            }


            // text color
            eState = _rSet.GetItemState(FontItemIds::CFID_CHARCOLOR);

            if ( eState == SfxItemState::SET )
            {
                const SvxColorItem& rColorItem =
                    static_cast<const SvxColorItem&>(_rSet.Get(FontItemIds::CFID_CHARCOLOR));

                Color nColor = rColorItem.GetValue();

                Any aUnoColor;
                if (COL_AUTO != nColor)
                    aUnoColor <<= nColor;

                lcl_pushBackPropertyValue( _out_properties, PROPERTY_TEXTCOLOR, aUnoColor );
            }


            // font relief
            eState = _rSet.GetItemState(FontItemIds::CFID_RELIEF);

            if ( eState == SfxItemState::SET )
            {
                const SvxCharReliefItem& rReliefItem =
                    static_cast<const SvxCharReliefItem&>(_rSet.Get(FontItemIds::CFID_RELIEF));

                lcl_pushBackPropertyValue( _out_properties, PROPERTY_FONT_RELIEF, Any(static_cast<sal_Int16>(rReliefItem.GetValue())) );
            }


            // font emphasis mark
            eState = _rSet.GetItemState(FontItemIds::CFID_EMPHASIS);

            if ( eState == SfxItemState::SET )
            {
                const SvxEmphasisMarkItem& rEmphMarkItem = _rSet.Get(FontItemIds::CFID_EMPHASIS);

                lcl_pushBackPropertyValue( _out_properties, PROPERTY_FONT_EMPHASIS_MARK, Any(static_cast<sal_Int16>(rEmphMarkItem.GetEmphasisMark())) );
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

    static ItemInfoPackage& getItemInfoPackageCntChrDlg()
    {
        class ItemInfoPackageCntChrDlg : public ItemInfoPackage
        {
            vcl::Font maDefaultVCLFont{Application::GetDefaultDevice()->GetSettings().GetStyleSettings().GetAppFont()};
            typedef std::array<ItemInfoStatic, FontItemIds::CFID_LAST_ITEM_ID - FontItemIds::CFID_FIRST_ITEM_ID + 1> ItemInfoArrayCntChrDlg;
            ItemInfoArrayCntChrDlg maItemInfos {{
                // m_nWhich, m_pItem, m_nSlotID, m_nItemInfoFlags
                { FontItemIds::CFID_FONT, new SvxFontItem(maDefaultVCLFont.GetFamilyType(), maDefaultVCLFont.GetFamilyName(), maDefaultVCLFont.GetStyleName(), maDefaultVCLFont.GetPitch(), maDefaultVCLFont.GetCharSet(), FontItemIds::CFID_FONT), SID_ATTR_CHAR_FONT, SFX_ITEMINFOFLAG_NONE },
                { FontItemIds::CFID_HEIGHT, new SvxFontHeightItem(maDefaultVCLFont.GetFontHeight(), 100, FontItemIds::CFID_HEIGHT), SID_ATTR_CHAR_FONTHEIGHT, SFX_ITEMINFOFLAG_NONE },
                { FontItemIds::CFID_WEIGHT, new SvxWeightItem(maDefaultVCLFont.GetWeight(), FontItemIds::CFID_WEIGHT), SID_ATTR_CHAR_WEIGHT, SFX_ITEMINFOFLAG_NONE },
                { FontItemIds::CFID_POSTURE, new SvxPostureItem(maDefaultVCLFont.GetItalic(), FontItemIds::CFID_POSTURE), SID_ATTR_CHAR_POSTURE, SFX_ITEMINFOFLAG_NONE },
                { FontItemIds::CFID_LANGUAGE, new SvxLanguageItem(Application::GetSettings().GetUILanguageTag().getLanguageType(), FontItemIds::CFID_LANGUAGE), SID_ATTR_CHAR_LANGUAGE, SFX_ITEMINFOFLAG_NONE },
                { FontItemIds::CFID_UNDERLINE, new SvxUnderlineItem(maDefaultVCLFont.GetUnderline(), FontItemIds::CFID_UNDERLINE), SID_ATTR_CHAR_UNDERLINE, SFX_ITEMINFOFLAG_NONE },
                { FontItemIds::CFID_STRIKEOUT, new SvxCrossedOutItem(maDefaultVCLFont.GetStrikeout(), FontItemIds::CFID_STRIKEOUT), SID_ATTR_CHAR_STRIKEOUT, SFX_ITEMINFOFLAG_NONE },
                { FontItemIds::CFID_WORDLINEMODE, new SvxWordLineModeItem(maDefaultVCLFont.IsWordLineMode(), FontItemIds::CFID_WORDLINEMODE), SID_ATTR_CHAR_WORDLINEMODE, SFX_ITEMINFOFLAG_NONE },
                { FontItemIds::CFID_CHARCOLOR, new SvxColorItem(maDefaultVCLFont.GetColor(), FontItemIds::CFID_CHARCOLOR), SID_ATTR_CHAR_COLOR, SFX_ITEMINFOFLAG_NONE },
                { FontItemIds::CFID_RELIEF, new SvxCharReliefItem(maDefaultVCLFont.GetRelief(), FontItemIds::CFID_RELIEF), SID_ATTR_CHAR_RELIEF, SFX_ITEMINFOFLAG_NONE },
                { FontItemIds::CFID_EMPHASIS, new SvxEmphasisMarkItem(maDefaultVCLFont.GetEmphasisMark(), FontItemIds::CFID_EMPHASIS), SID_ATTR_CHAR_EMPHASISMARK, SFX_ITEMINFOFLAG_NONE },
                { FontItemIds::CFID_CJK_FONT, new SvxFontItem(maDefaultVCLFont.GetFamilyType(), maDefaultVCLFont.GetFamilyName(), maDefaultVCLFont.GetStyleName(), maDefaultVCLFont.GetPitch(), maDefaultVCLFont.GetCharSet(), FontItemIds::CFID_CJK_FONT), 0, SFX_ITEMINFOFLAG_NONE },
                { FontItemIds::CFID_CJK_HEIGHT, new SvxFontHeightItem(maDefaultVCLFont.GetFontHeight(), 100, FontItemIds::CFID_CJK_HEIGHT), 0, SFX_ITEMINFOFLAG_NONE },
                { FontItemIds::CFID_CJK_WEIGHT, new SvxWeightItem(maDefaultVCLFont.GetWeight(), FontItemIds::CFID_CJK_WEIGHT), 0, SFX_ITEMINFOFLAG_NONE },
                { FontItemIds::CFID_CJK_POSTURE, new SvxPostureItem(maDefaultVCLFont.GetItalic(), FontItemIds::CFID_CJK_POSTURE), 0, SFX_ITEMINFOFLAG_NONE },
                { FontItemIds::CFID_CJK_LANGUAGE, new SvxLanguageItem(Application::GetSettings().GetUILanguageTag().getLanguageType(), FontItemIds::CFID_CJK_LANGUAGE), 0, SFX_ITEMINFOFLAG_NONE },
                { FontItemIds::CFID_CASEMAP, new SvxCaseMapItem(SvxCaseMap::NotMapped, FontItemIds::CFID_CASEMAP), 0, SFX_ITEMINFOFLAG_NONE },
                { FontItemIds::CFID_CONTOUR, new SvxContourItem(false, FontItemIds::CFID_CONTOUR), 0, SFX_ITEMINFOFLAG_NONE },
                { FontItemIds::CFID_SHADOWED, new SvxShadowedItem(false, FontItemIds::CFID_SHADOWED), 0, SFX_ITEMINFOFLAG_NONE },

                // SvxFontListItem has to be DynamicDefault. It is pool-dependent in the sense
                // that it cannot/should not exist. Additionally SvxFontListItem has NO destructor
                // and does not delete the contained/set FontList - this is done 'handish' in
                // ControlCharacterDialog::destroyItemSet, see below...
                { FontItemIds::CFID_FONTLIST, nullptr, SID_ATTR_CHAR_FONTLIST, SFX_ITEMINFOFLAG_NONE }
            }};

            virtual const ItemInfoStatic& getItemInfoStatic(size_t nIndex) const override { return maItemInfos[nIndex]; }

        public:
            virtual size_t size() const override { return maItemInfos.size(); }
            virtual const ItemInfo& getItemInfo(size_t nIndex, SfxItemPool& /*rPool*/) override { return maItemInfos[nIndex]; }
        };

        static std::unique_ptr<ItemInfoPackageCntChrDlg> g_aItemInfoPackageCntChrDlg;
        if (!g_aItemInfoPackageCntChrDlg)
            g_aItemInfoPackageCntChrDlg.reset(new ItemInfoPackageCntChrDlg);
        return *g_aItemInfoPackageCntChrDlg;
    }

    void ControlCharacterDialog::createItemSet(std::unique_ptr<SfxItemSet>& _rpSet, rtl::Reference<SfxItemPool>& _rpPool, FontList& rFontList)
    {
        // just to be sure...
        _rpSet = nullptr;
        _rpPool = nullptr;
        _rpPool = new SfxItemPool(u"PCRControlFontItemPool"_ustr);
        _rpPool->registerItemInfoPackage(
            getItemInfoPackageCntChrDlg(),
            [&rFontList](sal_uInt16 nWhich)
            {
                SfxPoolItem* pRetval(nullptr);
                if (FontItemIds::CFID_FONTLIST == nWhich)
                    pRetval = new SvxFontListItem(&rFontList, FontItemIds::CFID_FONTLIST);
                return pRetval;
            });

        // and, finally, the set
        _rpSet.reset(new SfxItemSet(*_rpPool));
    }

    void ControlCharacterDialog::destroyItemSet(std::unique_ptr<SfxItemSet>& _rpSet, rtl::Reference<SfxItemPool>& _rpPool)
    {
        // _first_ delete the set (referring the pool)
        _rpSet.reset();

        // delete the pool
        _rpPool = nullptr;
    }

    void ControlCharacterDialog::PageCreated(const OUString& rId, SfxTabPage& rPage)
    {
        SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
        if (rId == "font")
        {
            aSet.Put (static_cast<const SvxFontListItem&>(GetInputSetImpl()->Get(FontItemIds::CFID_FONTLIST)));
            aSet.Put (SfxUInt16Item(SID_DISABLE_CTL,DISABLE_HIDE_LANGUAGE));
            rPage.PageCreated(aSet);
        }
    }
}   // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
