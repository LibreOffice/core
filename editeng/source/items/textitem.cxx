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

#include <com/sun/star/style/CaseMap.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/frame/status/FontHeight.hpp>
#include <math.h>
#include <sal/log.hxx>
#include <o3tl/safeint.hxx>
#include <osl/diagnose.h>
#include <comphelper/configuration.hxx>
#include <unotools/fontdefs.hxx>
#include <unotools/intlwrapper.hxx>
#include <unotools/syslocale.hxx>
#include <utility>
#include <vcl/outdev.hxx>
#include <vcl/unohelp.hxx>
#include <svtools/unitconv.hxx>

#include <editeng/editids.hrc>
#include <editeng/editrids.hrc>
#include <tools/bigint.hxx>
#include <tools/mapunit.hxx>
#include <tools/UnitConversion.hxx>

#include <rtl/math.hxx>
#include <rtl/ustring.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <svl/itemset.hxx>

#include <svtools/langtab.hxx>
#include <svl/itempool.hxx>
#include <svtools/ctrltool.hxx>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/text/FontEmphasis.hpp>
#include <editeng/rsiditem.hxx>
#include <editeng/memberids.h>
#include <editeng/flstitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/autokernitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/nhypitem.hxx>
#include <editeng/blinkitem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/twolinesitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/charrotateitem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/itemtype.hxx>
#include <editeng/eerdll.hxx>
#include <docmodel/color/ComplexColorJSON.hxx>
#include <docmodel/uno/UnoComplexColor.hxx>
#include <docmodel/color/ComplexColor.hxx>
#include <libxml/xmlwriter.h>
#include <unordered_map>

using namespace ::com::sun::star;
using namespace ::com::sun::star::text;

SfxPoolItem* SvxFontItem::CreateDefault() {return new SvxFontItem(0);}
SfxPoolItem* SvxPostureItem::CreateDefault() { return new SvxPostureItem(ITALIC_NONE, 0);}
SfxPoolItem* SvxWeightItem::CreateDefault() {return new SvxWeightItem(WEIGHT_NORMAL, 0);}
SfxPoolItem* SvxFontHeightItem::CreateDefault() {return new SvxFontHeightItem(240, 100, 0);}
SfxPoolItem* SvxUnderlineItem::CreateDefault() {return new SvxUnderlineItem(LINESTYLE_NONE, 0);}
SfxPoolItem* SvxOverlineItem::CreateDefault() {return new SvxOverlineItem(LINESTYLE_NONE, 0);}
SfxPoolItem* SvxCrossedOutItem::CreateDefault() {return new SvxCrossedOutItem(STRIKEOUT_NONE, 0);}
SfxPoolItem* SvxShadowedItem::CreateDefault() {return new SvxShadowedItem(false, 0);}
SfxPoolItem* SvxAutoKernItem::CreateDefault() {return new SvxAutoKernItem(false, 0);}
SfxPoolItem* SvxWordLineModeItem::CreateDefault() {return new SvxWordLineModeItem(false, 0);}
SfxPoolItem* SvxContourItem::CreateDefault() {return new SvxContourItem(false, 0);}
SfxPoolItem* SvxColorItem::CreateDefault() {return new SvxColorItem(0);}
SfxPoolItem* SvxKerningItem::CreateDefault() {return new SvxKerningItem(0, 0);}
SfxPoolItem* SvxCaseMapItem::CreateDefault() {return new SvxCaseMapItem(SvxCaseMap::NotMapped, 0);}
SfxPoolItem* SvxEscapementItem::CreateDefault() {return new SvxEscapementItem(0);}
SfxPoolItem* SvxLanguageItem::CreateDefault() {return new SvxLanguageItem(LANGUAGE_GERMAN, 0);}
SfxPoolItem* SvxEmphasisMarkItem::CreateDefault() {return new SvxEmphasisMarkItem(FontEmphasisMark::NONE, TypedWhichId<SvxEmphasisMarkItem>(0));}
SfxPoolItem* SvxCharRotateItem::CreateDefault() {return new SvxCharRotateItem(0_deg10, false, TypedWhichId<SvxCharRotateItem>(0));}
SfxPoolItem* SvxCharScaleWidthItem::CreateDefault() {return new SvxCharScaleWidthItem(100, TypedWhichId<SvxCharScaleWidthItem>(0));}
SfxPoolItem* SvxCharReliefItem::CreateDefault() {return new SvxCharReliefItem(FontRelief::NONE, 0);}

// class SvxFontListItem -------------------------------------------------

SvxFontListItem::SvxFontListItem( const FontList* pFontLst,
                                  const sal_uInt16 nId ) :
    SfxPoolItem( nId ),
    pFontList( pFontLst )
{
    if ( pFontList )
    {
        sal_Int32 nCount = pFontList->GetFontNameCount();
        aFontNameSeq.realloc( nCount );
        auto pFontNameSeq = aFontNameSeq.getArray();

        for ( sal_Int32 i = 0; i < nCount; i++ )
            pFontNameSeq[i] = pFontList->GetFontName(i).GetFamilyName();
    }
}

SvxFontListItem* SvxFontListItem::Clone( SfxItemPool* ) const
{
    return new SvxFontListItem( *this );
}

bool SvxFontListItem::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));

    return( pFontList == static_cast<const SvxFontListItem&>(rAttr).pFontList );
}

bool SvxFontListItem::QueryValue( css::uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    rVal <<= aFontNameSeq;
    return true;
}


bool SvxFontListItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper& /*rIntl*/
)   const
{
    rText.clear();
    return false;
}

// class SvxFontItem -----------------------------------------------------

typedef std::unordered_map<size_t, const SfxPoolItem*> SvxFontItemMap;

namespace
{
    class SvxFontItemInstanceManager : public ItemInstanceManager
    {
        SvxFontItemMap  maRegistered;

    public:
        SvxFontItemInstanceManager()
        : ItemInstanceManager(typeid(SvxFontItem).hash_code())
        {
        }

    private:
        static size_t hashCode(const SfxPoolItem&);

        // standard interface, accessed exclusively
        // by implCreateItemEntry/implCleanupItemEntry
        virtual const SfxPoolItem* find(const SfxPoolItem&) const override;
        virtual void add(const SfxPoolItem&) override;
        virtual void remove(const SfxPoolItem&) override;
    };

    size_t SvxFontItemInstanceManager::hashCode(const SfxPoolItem& rItem)
    {
        const SvxFontItem& rFontItem(static_cast<const SvxFontItem&>(rItem));
        std::size_t seed(0);
        o3tl::hash_combine(seed, rItem.Which());
        o3tl::hash_combine(seed, rFontItem.GetFamilyName().hashCode());
        o3tl::hash_combine(seed, rFontItem.GetStyleName().hashCode());
        o3tl::hash_combine(seed, rFontItem.GetFamily());
        o3tl::hash_combine(seed, rFontItem.GetPitch());
        o3tl::hash_combine(seed, rFontItem.GetCharSet());
        return seed;
    }

    const SfxPoolItem* SvxFontItemInstanceManager::find(const SfxPoolItem& rItem) const
    {
        SvxFontItemMap::const_iterator aHit(maRegistered.find(hashCode(rItem)));
        if (aHit != maRegistered.end())
            return aHit->second;
        return nullptr;
    }

    void SvxFontItemInstanceManager::add(const SfxPoolItem& rItem)
    {
        maRegistered.insert({hashCode(rItem), &rItem});
    }

    void SvxFontItemInstanceManager::remove(const SfxPoolItem& rItem)
    {
        maRegistered.erase(hashCode(rItem));
    }
}

ItemInstanceManager* SvxFontItem::getItemInstanceManager() const
{
    static SvxFontItemInstanceManager aInstanceManager;
    return &aInstanceManager;
}

SvxFontItem::SvxFontItem(
    const sal_uInt16 nId)
: SfxPoolItem( nId )
, aFamilyName()
, aStyleName()
, eFamily(FAMILY_SWISS)
, ePitch(PITCH_VARIABLE)
, eTextEncoding(RTL_TEXTENCODING_DONTKNOW)
{
}

SvxFontItem::SvxFontItem(
    const FontFamily eFam,
    OUString aName,
    OUString aStName,
    const FontPitch eFontPitch,
    const rtl_TextEncoding eFontTextEncoding,
    const sal_uInt16 nId)
: SfxPoolItem( nId )
, aFamilyName(std::move(aName))
, aStyleName(std::move(aStName))
, eFamily(eFam)
, ePitch(eFontPitch)
, eTextEncoding(eFontTextEncoding)
{
}

bool SvxFontItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
        case 0:
        {
            css::awt::FontDescriptor aFontDescriptor;
            aFontDescriptor.Name = aFamilyName;
            aFontDescriptor.StyleName = aStyleName;
            aFontDescriptor.Family = static_cast<sal_Int16>(eFamily);
            aFontDescriptor.CharSet = static_cast<sal_Int16>(eTextEncoding);
            aFontDescriptor.Pitch = static_cast<sal_Int16>(ePitch);
            rVal <<= aFontDescriptor;
        }
        break;
        case MID_FONT_FAMILY_NAME:
            rVal <<= aFamilyName;
        break;
        case MID_FONT_STYLE_NAME:
            rVal <<= aStyleName;
        break;
        case MID_FONT_FAMILY    : rVal <<= static_cast<sal_Int16>(eFamily);    break;
        case MID_FONT_CHAR_SET  : rVal <<= static_cast<sal_Int16>(eTextEncoding);  break;
        case MID_FONT_PITCH     : rVal <<= static_cast<sal_Int16>(ePitch); break;
    }
    return true;
}

bool SvxFontItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId)
{
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
        case 0:
        {
            css::awt::FontDescriptor aFontDescriptor;
            if ( !( rVal >>= aFontDescriptor ))
                return false;

            ASSERT_CHANGE_REFCOUNTED_ITEM;
            aFamilyName = aFontDescriptor.Name;
            aStyleName = aFontDescriptor.StyleName;
            eFamily = static_cast<FontFamily>(aFontDescriptor.Family);
            eTextEncoding = static_cast<rtl_TextEncoding>(aFontDescriptor.CharSet);
            ePitch = static_cast<FontPitch>(aFontDescriptor.Pitch);
        }
        break;
        case MID_FONT_FAMILY_NAME   :
        {
            OUString aStr;
            if(!(rVal >>= aStr))
                return false;
            ASSERT_CHANGE_REFCOUNTED_ITEM;
            aFamilyName = aStr;
        }
        break;
        case MID_FONT_STYLE_NAME:
        {
            OUString aStr;
            if(!(rVal >>= aStr))
                return false;
            ASSERT_CHANGE_REFCOUNTED_ITEM;
            aStyleName = aStr;
        }
        break;
        case MID_FONT_FAMILY :
        {
            sal_Int16 nFamily = sal_Int16();
            if(!(rVal >>= nFamily))
                return false;
            ASSERT_CHANGE_REFCOUNTED_ITEM;
            eFamily = static_cast<FontFamily>(nFamily);
        }
        break;
        case MID_FONT_CHAR_SET  :
        {
            sal_Int16 nSet = sal_Int16();
            if(!(rVal >>= nSet))
                return false;
            ASSERT_CHANGE_REFCOUNTED_ITEM;
            eTextEncoding = static_cast<rtl_TextEncoding>(nSet);
        }
        break;
        case MID_FONT_PITCH     :
        {
            sal_Int16 nPitch = sal_Int16();
            if(!(rVal >>= nPitch))
                return false;
            ASSERT_CHANGE_REFCOUNTED_ITEM;
            ePitch =  static_cast<FontPitch>(nPitch);
        }
        break;
    }
    return true;
}

void SvxFontItem::SetFamilyName(const OUString& rFamilyName)
{
    if (aFamilyName == rFamilyName)
        return;

    ASSERT_CHANGE_REFCOUNTED_ITEM;
    aFamilyName = rFamilyName;
}

void SvxFontItem::SetStyleName(const OUString &rStyleName)
{
    if (aStyleName == rStyleName)
        return;

    ASSERT_CHANGE_REFCOUNTED_ITEM;
    aStyleName = rStyleName;
}

void SvxFontItem::SetFamily(FontFamily _eFamily)
{
    if (eFamily == _eFamily)
        return;

    ASSERT_CHANGE_REFCOUNTED_ITEM;
    eFamily = _eFamily;
}

void SvxFontItem::SetPitch(FontPitch _ePitch)
{
    if (ePitch == _ePitch)
        return;

    ASSERT_CHANGE_REFCOUNTED_ITEM;
    ePitch = _ePitch;
}

void SvxFontItem::SetCharSet(rtl_TextEncoding _eEncoding)
{
    if (eTextEncoding == _eEncoding)
        return;

    ASSERT_CHANGE_REFCOUNTED_ITEM;
    eTextEncoding = _eEncoding;
}

bool SvxFontItem::operator==( const SfxPoolItem& rAttr ) const
{
    if (this == &rAttr)
        return true;

    assert(SfxPoolItem::operator==(rAttr));
    const SvxFontItem& rItem(static_cast<const SvxFontItem&>(rAttr));
    bool bRet = ( eFamily == rItem.eFamily &&
                 aFamilyName == rItem.aFamilyName &&
                 aStyleName == rItem.aStyleName );

    if ( bRet )
    {
        if ( ePitch != rItem.ePitch || eTextEncoding != rItem.eTextEncoding )
        {
            bRet = false;
            SAL_INFO( "editeng.items", "FontItem::operator==(): only pitch or rtl_TextEncoding different ");
        }
    }

    return bRet;
}

SvxFontItem* SvxFontItem::Clone( SfxItemPool * ) const
{
    return new SvxFontItem( *this );
}

bool SvxFontItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper& /*rIntl*/
)   const
{
    rText = aFamilyName;
    return true;
}


void SvxFontItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SvxFontItem"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("familyName"), BAD_CAST(aFamilyName.toUtf8().getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("styleName"), BAD_CAST(aStyleName.toUtf8().getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("family"), BAD_CAST(OString::number(eFamily).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("pitch"), BAD_CAST(OString::number(ePitch).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("textEncoding"), BAD_CAST(OString::number(eTextEncoding).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
}

// class SvxPostureItem --------------------------------------------------

typedef std::unordered_map<size_t, const SfxPoolItem*> SvxPostureItemMap;

namespace
{
    class SvxPostureItemInstanceManager : public ItemInstanceManager
    {
        SvxPostureItemMap  maRegistered;

    public:
        SvxPostureItemInstanceManager()
        : ItemInstanceManager(typeid(SvxPostureItem).hash_code())
        {
        }

    private:
        static size_t hashCode(const SfxPoolItem&);

        // standard interface, accessed exclusively
        // by implCreateItemEntry/implCleanupItemEntry
        virtual const SfxPoolItem* find(const SfxPoolItem&) const override;
        virtual void add(const SfxPoolItem&) override;
        virtual void remove(const SfxPoolItem&) override;
    };

    size_t SvxPostureItemInstanceManager::hashCode(const SfxPoolItem& rItem)
    {
        auto const & rPostureItem = static_cast<const SvxPostureItem&>(rItem);
        std::size_t seed(0);
        o3tl::hash_combine(seed, rPostureItem.Which());
        o3tl::hash_combine(seed, rPostureItem. GetEnumValue());
        return seed;
    }

    const SfxPoolItem* SvxPostureItemInstanceManager::find(const SfxPoolItem& rItem) const
    {
        auto aHit(maRegistered.find(hashCode(rItem)));
        if (aHit != maRegistered.end())
            return aHit->second;
        return nullptr;
    }

    void SvxPostureItemInstanceManager::add(const SfxPoolItem& rItem)
    {
        maRegistered.insert({hashCode(rItem), &rItem});
    }

    void SvxPostureItemInstanceManager::remove(const SfxPoolItem& rItem)
    {
        maRegistered.erase(hashCode(rItem));
    }
}

ItemInstanceManager* SvxPostureItem::getItemInstanceManager() const
{
    static SvxPostureItemInstanceManager aInstanceManager;
    return &aInstanceManager;
}

SvxPostureItem::SvxPostureItem( const FontItalic ePosture, const sal_uInt16 nId ) :
    SfxEnumItem( nId, ePosture )
{
}

SvxPostureItem* SvxPostureItem::Clone( SfxItemPool * ) const
{
    return new SvxPostureItem( *this );
}

sal_uInt16 SvxPostureItem::GetValueCount() const
{
    return ITALIC_NORMAL + 1;   // ITALIC_NONE also belongs here
}


bool SvxPostureItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper& /*rIntl*/
)   const
{
    rText = GetValueTextByPos( GetValue() );
    return true;
}


OUString SvxPostureItem::GetValueTextByPos( sal_uInt16 nPos )
{
    DBG_ASSERT( nPos <= sal_uInt16(ITALIC_NORMAL), "enum overflow!" );

    FontItalic eItalic = static_cast<FontItalic>(nPos);
    TranslateId pId;

    switch ( eItalic )
    {
        case ITALIC_NONE:       pId = RID_SVXITEMS_ITALIC_NONE;     break;
        case ITALIC_OBLIQUE:    pId = RID_SVXITEMS_ITALIC_OBLIQUE;  break;
        case ITALIC_NORMAL:     pId = RID_SVXITEMS_ITALIC_NORMAL;   break;
        default: ;//prevent warning
    }

    return pId ? EditResId(pId) : OUString();
}

bool SvxPostureItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
        case MID_ITALIC:
            rVal <<= GetBoolValue();
            break;
        case MID_POSTURE:
            rVal <<= vcl::unohelper::ConvertFontSlant(GetValue());
            break;
    }
    return true;
}

bool SvxPostureItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
        case MID_ITALIC:
            ASSERT_CHANGE_REFCOUNTED_ITEM;
            SetBoolValue(Any2Bool(rVal));
        break;
        case MID_POSTURE:
        {
            awt::FontSlant eSlant;
            if(!(rVal >>= eSlant))
            {
                sal_Int32 nValue = 0;
                if(!(rVal >>= nValue))
                    return false;

                eSlant = static_cast<awt::FontSlant>(nValue);
            }
            ASSERT_CHANGE_REFCOUNTED_ITEM;
            SetValue(vcl::unohelper::ConvertFontSlant(eSlant));
        }
    }
    return true;
}

bool SvxPostureItem::HasBoolValue() const
{
    return true;
}

bool SvxPostureItem::GetBoolValue() const
{
    return ( GetValue() >= ITALIC_OBLIQUE );
}

void SvxPostureItem::SetBoolValue( bool bVal )
{
    ASSERT_CHANGE_REFCOUNTED_ITEM;
    SetValue( bVal ? ITALIC_NORMAL : ITALIC_NONE );
}

void SvxPostureItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SvxPostureItem"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("whichId"), "%d", Which());
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("value"), "%d", GetValue());
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("presentation"), BAD_CAST(GetValueTextByPos(GetValue()).toUtf8().getStr()));
    (void)xmlTextWriterEndElement(pWriter);
}

// class SvxWeightItem ---------------------------------------------------

ItemInstanceManager* SvxWeightItem::getItemInstanceManager() const
{
    static DefaultItemInstanceManager aInstanceManager(typeid(SvxWeightItem).hash_code());
    return &aInstanceManager;
}

SvxWeightItem::SvxWeightItem( const FontWeight eWght, const sal_uInt16 nId ) :
    SfxEnumItem( nId, eWght )
{
}


bool SvxWeightItem::HasBoolValue() const
{
    return true;
}


bool SvxWeightItem::GetBoolValue() const
{
    return GetValue() >= WEIGHT_BOLD;
}


void SvxWeightItem::SetBoolValue( bool bVal )
{
    ASSERT_CHANGE_REFCOUNTED_ITEM;
    SetValue( bVal ? WEIGHT_BOLD : WEIGHT_NORMAL );
}


sal_uInt16 SvxWeightItem::GetValueCount() const
{
    return WEIGHT_BLACK;    // WEIGHT_DONTKNOW does not belong
}

SvxWeightItem* SvxWeightItem::Clone( SfxItemPool * ) const
{
    return new SvxWeightItem( *this );
}

bool SvxWeightItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper& /*rIntl*/
)   const
{
    rText = GetValueTextByPos( GetValue() );
    return true;
}

OUString SvxWeightItem::GetValueTextByPos( sal_uInt16 nPos )
{
    static TranslateId RID_SVXITEMS_WEIGHTS[] =
    {
        RID_SVXITEMS_WEIGHT_DONTKNOW,
        RID_SVXITEMS_WEIGHT_THIN,
        RID_SVXITEMS_WEIGHT_ULTRALIGHT,
        RID_SVXITEMS_WEIGHT_LIGHT,
        RID_SVXITEMS_WEIGHT_SEMILIGHT,
        RID_SVXITEMS_WEIGHT_NORMAL,
        RID_SVXITEMS_WEIGHT_MEDIUM,
        RID_SVXITEMS_WEIGHT_SEMIBOLD,
        RID_SVXITEMS_WEIGHT_BOLD,
        RID_SVXITEMS_WEIGHT_ULTRABOLD,
        RID_SVXITEMS_WEIGHT_BLACK
    };

    static_assert(std::size(RID_SVXITEMS_WEIGHTS) - 1 == WEIGHT_BLACK, "must match");
    assert(nPos <= sal_uInt16(WEIGHT_BLACK) && "enum overflow!" );
    return EditResId(RID_SVXITEMS_WEIGHTS[nPos]);
}

bool SvxWeightItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
        case MID_BOLD   :
            rVal <<= GetBoolValue();
        break;
        case MID_WEIGHT:
        {
            rVal <<= vcl::unohelper::ConvertFontWeight( GetValue() );
        }
        break;
    }
    return true;
}

bool SvxWeightItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
        case MID_BOLD   :
            ASSERT_CHANGE_REFCOUNTED_ITEM;
            SetBoolValue(Any2Bool(rVal));
        break;
        case MID_WEIGHT:
        {
            double fValue = 0;
            if(!(rVal >>= fValue))
            {
                sal_Int32 nValue = 0;
                if(!(rVal >>= nValue))
                    return false;
                fValue = static_cast<float>(nValue);
            }
            ASSERT_CHANGE_REFCOUNTED_ITEM;
            SetValue( vcl::unohelper::ConvertFontWeight(static_cast<float>(fValue)) );
        }
        break;
    }
    return true;
}

void SvxWeightItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SvxWeightItem"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("whichId"), "%d", Which());
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("value"), "%d", GetValue());
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("presentation"), BAD_CAST(GetValueTextByPos(GetValue()).toUtf8().getStr()));
    (void)xmlTextWriterEndElement(pWriter);
}

// class SvxFontHeightItem -----------------------------------------------

typedef std::unordered_map<size_t, const SfxPoolItem*> SvxFontHeightItemMap;

namespace
{
    class SvxFontHeightItemInstanceManager : public ItemInstanceManager
    {
        SvxFontHeightItemMap  maRegistered;

    public:
        SvxFontHeightItemInstanceManager()
        : ItemInstanceManager(typeid(SvxFontHeightItem).hash_code())
        {
        }

    private:
        static size_t hashCode(const SfxPoolItem&);

        // standard interface, accessed exclusively
        // by implCreateItemEntry/implCleanupItemEntry
        virtual const SfxPoolItem* find(const SfxPoolItem&) const override;
        virtual void add(const SfxPoolItem&) override;
        virtual void remove(const SfxPoolItem&) override;
    };

    size_t SvxFontHeightItemInstanceManager::hashCode(const SfxPoolItem& rItem)
    {
        auto const & rFontHeightItem = static_cast<const SvxFontHeightItem&>(rItem);
        std::size_t seed(0);
        o3tl::hash_combine(seed, rFontHeightItem.Which());
        o3tl::hash_combine(seed, rFontHeightItem.GetHeight());
        o3tl::hash_combine(seed, rFontHeightItem.GetProp());
        o3tl::hash_combine(seed, rFontHeightItem.GetPropUnit());
        return seed;
    }

    const SfxPoolItem* SvxFontHeightItemInstanceManager::find(const SfxPoolItem& rItem) const
    {
        auto aHit(maRegistered.find(hashCode(rItem)));
        if (aHit != maRegistered.end())
            return aHit->second;
        return nullptr;
    }

    void SvxFontHeightItemInstanceManager::add(const SfxPoolItem& rItem)
    {
        maRegistered.insert({hashCode(rItem), &rItem});
    }

    void SvxFontHeightItemInstanceManager::remove(const SfxPoolItem& rItem)
    {
        maRegistered.erase(hashCode(rItem));
    }
}

ItemInstanceManager* SvxFontHeightItem::getItemInstanceManager() const
{
    static SvxFontHeightItemInstanceManager aInstanceManager;
    return &aInstanceManager;
}

SvxFontHeightItem::SvxFontHeightItem( const sal_uInt32 nSz,
                                      const sal_uInt16 nPrp,
                                      const sal_uInt16 nId ) :
    SfxPoolItem( nId )
{
    SetHeight( nSz,nPrp );  // calculate in percentage
}

SvxFontHeightItem* SvxFontHeightItem::Clone( SfxItemPool * ) const
{
    return new SvxFontHeightItem( *this );
}

bool SvxFontHeightItem::operator==( const SfxPoolItem& rItem ) const
{
    assert(SfxPoolItem::operator==(rItem));
    return GetHeight() == static_cast<const SvxFontHeightItem&>(rItem).GetHeight() &&
            GetProp() == static_cast<const SvxFontHeightItem&>(rItem).GetProp() &&
            GetPropUnit() == static_cast<const SvxFontHeightItem&>(rItem).GetPropUnit();
}

bool SvxFontHeightItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    //  In StarOne is the uno::Any always 1/100mm. Through the MemberId it is
    //  controlled if the value in the Item should be 1/100mm or Twips.

    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
        case 0:
        {
            css::frame::status::FontHeight aFontHeight;

            // Point (i.e. Twips) is asked for, thus re-calculate if
            // CONVERT_TWIPS is not set.
            if( bConvert )
            {
                aFontHeight.Height = o3tl::convert<double>(nHeight, o3tl::Length::twip, o3tl::Length::pt);
            }
            else
            {
                double fPoints = o3tl::convert<double>(nHeight, o3tl::Length::mm100, o3tl::Length::pt);
                aFontHeight.Height = rtl::math::round(fPoints, 1);
            }

            aFontHeight.Prop = MapUnit::MapRelative == ePropUnit ? nProp : 100;

            float fRet = nProp;
            switch( ePropUnit )
            {
                case MapUnit::MapRelative:
                    fRet = 0.;
                break;
                case MapUnit::Map100thMM:
                    fRet = o3tl::convert(fRet, o3tl::Length::mm100, o3tl::Length::pt);
                break;
                case MapUnit::MapPoint:

                break;
                case MapUnit::MapTwip:
                    fRet = o3tl::convert(fRet, o3tl::Length::twip, o3tl::Length::pt);
                break;
                default: ;//prevent warning
            }
            aFontHeight.Diff = fRet;
            rVal <<= aFontHeight;
        }
        break;
        case MID_FONTHEIGHT:
        {
            // Point (i.e. Twips) is asked for, thus re-calculate if
            // CONVERT_TWIPS is not set.
            if( bConvert )
            {
                rVal <<= static_cast<float>(o3tl::convert<double>(nHeight, o3tl::Length::twip, o3tl::Length::pt));
            }
            else
            {
                double fPoints = o3tl::convert<double>(nHeight, o3tl::Length::mm100, o3tl::Length::pt);
                rVal <<= static_cast<float>(::rtl::math::round(fPoints, 1));
            }
        }
        break;
        case MID_FONTHEIGHT_PROP:
            rVal <<= static_cast<sal_Int16>(MapUnit::MapRelative == ePropUnit ? nProp : 100);
        break;
        case MID_FONTHEIGHT_DIFF:
        {
            float fRet = nProp;
            switch( ePropUnit )
            {
                case MapUnit::MapRelative:
                    fRet = 0.;
                break;
                case MapUnit::Map100thMM:
                    fRet = o3tl::convert(fRet, o3tl::Length::mm100, o3tl::Length::pt);
                break;
                case MapUnit::MapPoint:

                break;
                case MapUnit::MapTwip:
                    fRet = o3tl::convert(fRet, o3tl::Length::twip, o3tl::Length::pt);
                break;
                default: ;//prevent warning
            }
            rVal <<= fRet;
        }
        break;
    }
    return true;
}

// Try to reconstruct the original height input value from the modified height
// and the prop data; this seems somewhat futile given the various ways how the
// modified height is calculated (with and without conversion between twips and
// 100th mm; with an additional eCoreMetric input in one of the SetHeight
// overloads), and indeed known to occasionally produce nRet values that would
// be negative, so just guard against negative results here and throw the hands
// up in despair:
static sal_uInt32 lcl_GetRealHeight_Impl(sal_uInt32 nHeight, sal_uInt16 nProp, MapUnit eProp, bool bCoreInTwip)
{
    sal_uInt32 nRet = nHeight;
    short nDiff = 0;
    switch( eProp )
    {
        case MapUnit::MapRelative:
            if (nProp)
            {
                nRet *= 100;
                nRet /= nProp;
            }
            break;
        case MapUnit::MapPoint:
        {
            short nTemp = static_cast<short>(nProp);
            nDiff = nTemp * 20;
            if(!bCoreInTwip)
                nDiff = static_cast<short>(convertTwipToMm100(static_cast<tools::Long>(nDiff)));
            break;
        }
        case MapUnit::Map100thMM:
            //then the core is surely also in 1/100 mm
            nDiff = static_cast<short>(nProp);
            break;
        case MapUnit::MapTwip:
            // Here surely TWIP
            nDiff = static_cast<short>(nProp);
            break;
        default:
            break;
    }
    nRet = (nDiff < 0 || nRet >= o3tl::make_unsigned(nDiff))
        ? nRet - nDiff : 0;
        //TODO: overflow in case nDiff < 0 and nRet - nDiff > SAL_MAX_UINT32

    return nRet;
}

bool SvxFontHeightItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
        case 0:
        {
            css::frame::status::FontHeight aFontHeight;
            if ( rVal >>= aFontHeight )
            {
                // Height
                ePropUnit = MapUnit::MapRelative;
                nProp = 100;
                double fPoint = aFontHeight.Height;
                if( fPoint < 0. || fPoint > 10000. )
                    return false;

                ASSERT_CHANGE_REFCOUNTED_ITEM;
                nHeight = static_cast<tools::Long>( fPoint * 20.0 + 0.5 );        // Twips
                if (!bConvert)
                    nHeight = convertTwipToMm100(nHeight);  // Convert, if the item contains 1/100mm

                nProp = aFontHeight.Prop;
            }
            else
                return false;
        }
        break;
        case MID_FONTHEIGHT:
        {
            ePropUnit = MapUnit::MapRelative;
            nProp = 100;
            double fPoint = 0;
            if(!(rVal >>= fPoint))
            {
                sal_Int32 nValue = 0;
                if(!(rVal >>= nValue))
                    return false;
                fPoint = static_cast<float>(nValue);
            }

            if (fPoint < 0. || fPoint > 10000.)
                return false;
            static bool bFuzzing = comphelper::IsFuzzing();
            if (bFuzzing && fPoint > 120)
            {
                SAL_WARN("editeng.items", "SvxFontHeightItem ignoring font size of " << fPoint << " for performance");
                return false;
            }

            ASSERT_CHANGE_REFCOUNTED_ITEM;
            nHeight = static_cast<tools::Long>( fPoint * 20.0 + 0.5 );        // Twips
            if (!bConvert)
                nHeight = convertTwipToMm100(nHeight);  // Convert, if the item contains 1/100mm
        }
        break;
        case MID_FONTHEIGHT_PROP:
        {
            sal_Int16 nNew = sal_Int16();
            if(!(rVal >>= nNew))
                return true;

            ASSERT_CHANGE_REFCOUNTED_ITEM;
            nHeight = lcl_GetRealHeight_Impl(nHeight, nProp, ePropUnit, bConvert);

            nHeight *= nNew;
            nHeight /= 100;
            nProp = nNew;
            ePropUnit = MapUnit::MapRelative;
        }
        break;
        case MID_FONTHEIGHT_DIFF:
        {
            nHeight = lcl_GetRealHeight_Impl(nHeight, nProp, ePropUnit, bConvert);
            float fValue = 0;
            if(!(rVal >>= fValue))
            {
                sal_Int32 nValue = 0;
                if(!(rVal >>= nValue))
                    return false;
                fValue = static_cast<float>(nValue);
            }
            ASSERT_CHANGE_REFCOUNTED_ITEM;
            sal_Int16 nCoreDiffValue = static_cast<sal_Int16>(fValue * 20.);
            nHeight += bConvert ? nCoreDiffValue : convertTwipToMm100(nCoreDiffValue);
            nProp = static_cast<sal_uInt16>(static_cast<sal_Int16>(fValue));
            ePropUnit = MapUnit::MapPoint;
        }
        break;
    }
    return true;
}


bool SvxFontHeightItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             eCoreUnit,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper& rIntl
)   const
{
    if( MapUnit::MapRelative != ePropUnit )
    {
        rText = OUString::number( static_cast<short>(nProp) ) +
                " " + EditResId( GetMetricId( ePropUnit ) );
        if( 0 <= static_cast<short>(nProp) )
            rText = "+" + rText;
    }
    else if( 100 == nProp )
    {
        rText = GetMetricText( static_cast<tools::Long>(nHeight),
                                eCoreUnit, MapUnit::MapPoint, &rIntl ) +
                " " + EditResId(GetMetricId(MapUnit::MapPoint));
    }
    else
        rText = OUString::number( nProp ) + "%";
    return true;
}


void SvxFontHeightItem::ScaleMetrics( tools::Long nMult, tools::Long nDiv )
{
    ASSERT_CHANGE_REFCOUNTED_ITEM;
    nHeight = static_cast<sal_uInt32>(BigInt::Scale( nHeight, nMult, nDiv ));
}


bool SvxFontHeightItem::HasMetrics() const
{
    return true;
}

void SvxFontHeightItem::SetHeight( sal_uInt32 nNewHeight, const sal_uInt16 nNewProp,
                                   MapUnit eUnit )
{
    DBG_ASSERT( GetRefCount() == 0, "SetValue() with pooled item" );

    ASSERT_CHANGE_REFCOUNTED_ITEM;
    if( MapUnit::MapRelative != eUnit )
        nHeight = nNewHeight + ::ItemToControl( short(nNewProp), eUnit,
                                                FieldUnit::TWIP );
    else if( 100 != nNewProp )
        nHeight = sal_uInt32(( nNewHeight * nNewProp ) / 100 );
    else
        nHeight = nNewHeight;

    nProp = nNewProp;
    ePropUnit = eUnit;
}

void SvxFontHeightItem::SetHeight( sal_uInt32 nNewHeight, sal_uInt16 nNewProp,
                                   MapUnit eMetric, MapUnit eCoreMetric )
{
    DBG_ASSERT( GetRefCount() == 0, "SetValue() with pooled item" );

    ASSERT_CHANGE_REFCOUNTED_ITEM;
    if( MapUnit::MapRelative != eMetric )
        nHeight = nNewHeight +
                ::ControlToItem( ::ItemToControl(static_cast<short>(nNewProp), eMetric,
                                        FieldUnit::TWIP ), FieldUnit::TWIP,
                                        eCoreMetric );
    else if( 100 != nNewProp )
        nHeight = sal_uInt32(( nNewHeight * nNewProp ) / 100 );
    else
        nHeight = nNewHeight;

    nProp = nNewProp;
    ePropUnit = eMetric;
}

void SvxFontHeightItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SvxFontHeightItem"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("height"), BAD_CAST(OString::number(nHeight).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("prop"), BAD_CAST(OString::number(nProp).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("propUnit"), BAD_CAST(OString::number(static_cast<int>(ePropUnit)).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
}

// class SvxTextLineItem ------------------------------------------------

SvxTextLineItem::SvxTextLineItem( const FontLineStyle eSt, const sal_uInt16 nId )
    : SfxEnumItem(nId, eSt)
    , maColor(COL_TRANSPARENT)
{
}


bool SvxTextLineItem::HasBoolValue() const
{
    return true;
}


bool SvxTextLineItem::GetBoolValue() const
{
    return  GetValue() != LINESTYLE_NONE;
}


void SvxTextLineItem::SetBoolValue( bool bVal )
{
    ASSERT_CHANGE_REFCOUNTED_ITEM;
    SetValue( bVal ? LINESTYLE_SINGLE : LINESTYLE_NONE );
}

SvxTextLineItem* SvxTextLineItem::Clone( SfxItemPool * ) const
{
    return new SvxTextLineItem( *this );
}

sal_uInt16 SvxTextLineItem::GetValueCount() const
{
    return LINESTYLE_DOTTED + 1;    // LINESTYLE_NONE also belongs here
}


bool SvxTextLineItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper& /*rIntl*/
)   const
{
    rText = GetValueTextByPos( GetValue() );
    if( !maColor.IsTransparent() )
        rText += cpDelim + ::GetColorString(maColor);
    return true;
}


OUString SvxTextLineItem::GetValueTextByPos( sal_uInt16 /*nPos*/ ) const
{
    OSL_FAIL("SvxTextLineItem::GetValueTextByPos: Pure virtual method");
    return OUString();
}

bool SvxTextLineItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
    case MID_TEXTLINED:
        rVal <<= GetBoolValue();
        break;
    case MID_TL_STYLE:
        rVal <<= static_cast<sal_Int16>(GetValue());
        break;
    case MID_TL_COLOR:
        rVal <<= maColor;
        break;
    case MID_TL_COMPLEX_COLOR:
    {
        auto xComplexColor = model::color::createXComplexColor(maComplexColor);
        rVal <<= xComplexColor;
        break;
    }
    case MID_TL_HASCOLOR:
        rVal <<= maColor.GetAlpha() == 255;
        break;
    }
    return true;
}

bool SvxTextLineItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch(nMemberId)
    {
    case MID_TEXTLINED:
        ASSERT_CHANGE_REFCOUNTED_ITEM;
        SetBoolValue(Any2Bool(rVal));
    break;
    case MID_TL_STYLE:
    {
        sal_Int32 nValue = 0;
        if(!(rVal >>= nValue))
            bRet = false;
        else
        {
            ASSERT_CHANGE_REFCOUNTED_ITEM;
            SetValue(static_cast<FontLineStyle>(nValue));
        }
    }
    break;
    case MID_TL_COLOR:
    {
        Color nCol;
        if( !( rVal >>= nCol ) )
            bRet = false;
        else
        {
            // Keep transparence, because it contains the information
            // whether the font color or the stored color should be used
            ASSERT_CHANGE_REFCOUNTED_ITEM;
            sal_uInt8 nAlpha = maColor.GetAlpha();
            maColor = nCol;
            maColor.SetAlpha( nAlpha );
        }
    }
    break;
    case MID_TL_COMPLEX_COLOR:
    {
        css::uno::Reference<css::util::XComplexColor> xComplexColor;
        if (!(rVal >>= xComplexColor))
            return false;

        if (xComplexColor.is())
        {
            ASSERT_CHANGE_REFCOUNTED_ITEM;
            maComplexColor = model::color::getFromXComplexColor(xComplexColor);
        }
    }
    break;
    case MID_TL_HASCOLOR:
        ASSERT_CHANGE_REFCOUNTED_ITEM;
        maColor.SetAlpha( Any2Bool( rVal ) ? 255 : 0 );
    break;
    }
    return bRet;
}

bool SvxTextLineItem::operator==( const SfxPoolItem& rItem ) const
{
    return SfxEnumItem::operator==( rItem ) &&
           maColor == static_cast<const SvxTextLineItem&>(rItem).maColor &&
           maComplexColor == static_cast<const SvxTextLineItem&>(rItem).maComplexColor;
}

// class SvxUnderlineItem ------------------------------------------------

ItemInstanceManager* SvxUnderlineItem::getItemInstanceManager() const
{
    static DefaultItemInstanceManager aInstanceManager(typeid(SvxUnderlineItem).hash_code());
    return &aInstanceManager;
}

SvxUnderlineItem::SvxUnderlineItem( const FontLineStyle eSt, const sal_uInt16 nId )
    : SvxTextLineItem( eSt, nId )
{
}

SvxUnderlineItem* SvxUnderlineItem::Clone( SfxItemPool * ) const
{
    return new SvxUnderlineItem( *this );
}

OUString SvxUnderlineItem::GetValueTextByPos( sal_uInt16 nPos ) const
{
    static TranslateId RID_SVXITEMS_UL[] =
    {
        RID_SVXITEMS_UL_NONE,
        RID_SVXITEMS_UL_SINGLE,
        RID_SVXITEMS_UL_DOUBLE,
        RID_SVXITEMS_UL_DOTTED,
        RID_SVXITEMS_UL_DONTKNOW,
        RID_SVXITEMS_UL_DASH,
        RID_SVXITEMS_UL_LONGDASH,
        RID_SVXITEMS_UL_DASHDOT,
        RID_SVXITEMS_UL_DASHDOTDOT,
        RID_SVXITEMS_UL_SMALLWAVE,
        RID_SVXITEMS_UL_WAVE,
        RID_SVXITEMS_UL_DOUBLEWAVE,
        RID_SVXITEMS_UL_BOLD,
        RID_SVXITEMS_UL_BOLDDOTTED,
        RID_SVXITEMS_UL_BOLDDASH,
        RID_SVXITEMS_UL_BOLDLONGDASH,
        RID_SVXITEMS_UL_BOLDDASHDOT,
        RID_SVXITEMS_UL_BOLDDASHDOTDOT,
        RID_SVXITEMS_UL_BOLDWAVE
    };
    static_assert(std::size(RID_SVXITEMS_UL) - 1 == LINESTYLE_BOLDWAVE, "must match");
    assert(nPos <= sal_uInt16(LINESTYLE_BOLDWAVE) && "enum overflow!");
    return EditResId(RID_SVXITEMS_UL[nPos]);
}

// class SvxOverlineItem ------------------------------------------------

ItemInstanceManager* SvxOverlineItem::getItemInstanceManager() const
{
    static DefaultItemInstanceManager aInstanceManager(typeid(SvxOverlineItem).hash_code());
    return &aInstanceManager;
}

SvxOverlineItem::SvxOverlineItem( const FontLineStyle eSt, const sal_uInt16 nId )
    : SvxTextLineItem( eSt, nId )
{
}

SvxOverlineItem* SvxOverlineItem::Clone( SfxItemPool * ) const
{
    return new SvxOverlineItem( *this );
}

OUString SvxOverlineItem::GetValueTextByPos( sal_uInt16 nPos ) const
{
    static TranslateId RID_SVXITEMS_OL[] =
    {
        RID_SVXITEMS_OL_NONE,
        RID_SVXITEMS_OL_SINGLE,
        RID_SVXITEMS_OL_DOUBLE,
        RID_SVXITEMS_OL_DOTTED,
        RID_SVXITEMS_OL_DONTKNOW,
        RID_SVXITEMS_OL_DASH,
        RID_SVXITEMS_OL_LONGDASH,
        RID_SVXITEMS_OL_DASHDOT,
        RID_SVXITEMS_OL_DASHDOTDOT,
        RID_SVXITEMS_OL_SMALLWAVE,
        RID_SVXITEMS_OL_WAVE,
        RID_SVXITEMS_OL_DOUBLEWAVE,
        RID_SVXITEMS_OL_BOLD,
        RID_SVXITEMS_OL_BOLDDOTTED,
        RID_SVXITEMS_OL_BOLDDASH,
        RID_SVXITEMS_OL_BOLDLONGDASH,
        RID_SVXITEMS_OL_BOLDDASHDOT,
        RID_SVXITEMS_OL_BOLDDASHDOTDOT,
        RID_SVXITEMS_OL_BOLDWAVE
    };
    static_assert(std::size(RID_SVXITEMS_OL) - 1 == LINESTYLE_BOLDWAVE, "must match");
    assert(nPos <= sal_uInt16(LINESTYLE_BOLDWAVE) && "enum overflow!");
    return EditResId(RID_SVXITEMS_OL[nPos]);
}

// class SvxCrossedOutItem -----------------------------------------------

ItemInstanceManager* SvxCrossedOutItem::getItemInstanceManager() const
{
    static DefaultItemInstanceManager aInstanceManager(typeid(SvxCrossedOutItem).hash_code());
    return &aInstanceManager;
}

SvxCrossedOutItem::SvxCrossedOutItem( const FontStrikeout eSt, const sal_uInt16 nId )
    : SfxEnumItem( nId, eSt )
{
}


bool SvxCrossedOutItem::HasBoolValue() const
{
    return true;
}


bool SvxCrossedOutItem::GetBoolValue() const
{
    return GetValue() != STRIKEOUT_NONE;
}


void SvxCrossedOutItem::SetBoolValue( bool bVal )
{
    ASSERT_CHANGE_REFCOUNTED_ITEM;
    SetValue( bVal ? STRIKEOUT_SINGLE : STRIKEOUT_NONE );
}


sal_uInt16 SvxCrossedOutItem::GetValueCount() const
{
    return STRIKEOUT_DOUBLE + 1;    // STRIKEOUT_NONE belongs also here
}

SvxCrossedOutItem* SvxCrossedOutItem::Clone( SfxItemPool * ) const
{
    return new SvxCrossedOutItem( *this );
}

bool SvxCrossedOutItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper& /*rIntl*/
)   const
{
    rText = GetValueTextByPos( GetValue() );
    return true;
}

OUString SvxCrossedOutItem::GetValueTextByPos( sal_uInt16 nPos )
{
    static TranslateId RID_SVXITEMS_STRIKEOUT[] =
    {
        RID_SVXITEMS_STRIKEOUT_NONE,
        RID_SVXITEMS_STRIKEOUT_SINGLE,
        RID_SVXITEMS_STRIKEOUT_DOUBLE,
        RID_SVXITEMS_STRIKEOUT_DONTKNOW,
        RID_SVXITEMS_STRIKEOUT_BOLD,
        RID_SVXITEMS_STRIKEOUT_SLASH,
        RID_SVXITEMS_STRIKEOUT_X
    };
    static_assert(std::size(RID_SVXITEMS_STRIKEOUT) - 1 == STRIKEOUT_X, "must match");
    assert(nPos <= sal_uInt16(STRIKEOUT_X) && "enum overflow!");
    return EditResId(RID_SVXITEMS_STRIKEOUT[nPos]);
}

bool SvxCrossedOutItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
        case MID_CROSSED_OUT:
            rVal <<= GetBoolValue();
        break;
        case MID_CROSS_OUT:
            rVal <<= static_cast<sal_Int16>(GetValue());
        break;
    }
    return true;
}

bool SvxCrossedOutItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
        case MID_CROSSED_OUT:
            SetBoolValue(Any2Bool(rVal));
        break;
        case MID_CROSS_OUT:
        {
            sal_Int32 nValue = 0;
            if(!(rVal >>= nValue))
                return false;
            ASSERT_CHANGE_REFCOUNTED_ITEM;
            SetValue(static_cast<FontStrikeout>(nValue));
        }
        break;
    }
    return true;
}
// class SvxShadowedItem -------------------------------------------------

SvxShadowedItem::SvxShadowedItem( const bool bShadowed, const sal_uInt16 nId ) :
    SfxBoolItem( nId, bShadowed )
{
}

SvxShadowedItem* SvxShadowedItem::Clone( SfxItemPool * ) const
{
    return new SvxShadowedItem( *this );
}

bool SvxShadowedItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper& /*rIntl*/
)   const
{
    TranslateId pId = RID_SVXITEMS_SHADOWED_FALSE;

    if ( GetValue() )
        pId = RID_SVXITEMS_SHADOWED_TRUE;
    rText = EditResId(pId);
    return true;
}

// class SvxAutoKernItem -------------------------------------------------

SvxAutoKernItem::SvxAutoKernItem( const bool bAutoKern, const sal_uInt16 nId ) :
    SfxBoolItem( nId, bAutoKern )
{
}

SvxAutoKernItem* SvxAutoKernItem::Clone( SfxItemPool * ) const
{
    return new SvxAutoKernItem( *this );
}

bool SvxAutoKernItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper& /*rIntl*/
)   const
{
    TranslateId pId = RID_SVXITEMS_AUTOKERN_FALSE;

    if ( GetValue() )
        pId = RID_SVXITEMS_AUTOKERN_TRUE;
    rText = EditResId(pId);
    return true;
}

// class SvxWordLineModeItem ---------------------------------------------

SvxWordLineModeItem::SvxWordLineModeItem( const bool bWordLineMode,
                                          const sal_uInt16 nId ) :
    SfxBoolItem( nId, bWordLineMode )
{
}

SvxWordLineModeItem* SvxWordLineModeItem::Clone( SfxItemPool * ) const
{
    return new SvxWordLineModeItem( *this );
}

bool SvxWordLineModeItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper& /*rIntl*/
)   const
{
    TranslateId pId = RID_SVXITEMS_WORDLINE_FALSE;

    if ( GetValue() )
        pId = RID_SVXITEMS_WORDLINE_TRUE;
    rText = EditResId(pId);
    return true;
}

// class SvxContourItem --------------------------------------------------

SvxContourItem::SvxContourItem( const bool bContoured, const sal_uInt16 nId ) :
    SfxBoolItem( nId, bContoured )
{
}

SvxContourItem* SvxContourItem::Clone( SfxItemPool * ) const
{
    return new SvxContourItem( *this );
}

bool SvxContourItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper& /*rIntl*/
)   const
{
    TranslateId pId = RID_SVXITEMS_CONTOUR_FALSE;

    if ( GetValue() )
        pId = RID_SVXITEMS_CONTOUR_TRUE;
    rText = EditResId(pId);
    return true;
}

// class SvxColorItem ----------------------------------------------------
SvxColorItem::SvxColorItem( const sal_uInt16 nId ) :
    SfxPoolItem(nId),
    mColor( COL_BLACK )
{
}

SvxColorItem::SvxColorItem( const Color& rCol, const sal_uInt16 nId ) :
    SfxPoolItem( nId ),
    mColor( rCol )
{
}

SvxColorItem::SvxColorItem(Color const& rColor, model::ComplexColor const& rComplexColor, const sal_uInt16 nId)
    : SfxPoolItem(nId)
    , mColor(rColor)
    , maComplexColor(rComplexColor)
{
}

SvxColorItem::~SvxColorItem()
{
}

bool SvxColorItem::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    const SvxColorItem& rColorItem = static_cast<const SvxColorItem&>(rAttr);

    return mColor == rColorItem.mColor &&
           maComplexColor == rColorItem.maComplexColor;
}

bool SvxColorItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch (nMemberId)
    {
        case MID_COLOR_ALPHA:
        {
            auto fTransparency = static_cast<double>(255 - mColor.GetAlpha()) * 100 / 255;
            rVal <<= static_cast<sal_Int16>(basegfx::fround(fTransparency));
            break;
        }
        case MID_GRAPHIC_TRANSPARENT:
        {
            rVal <<= mColor.GetAlpha() == 0;
            break;
        }
        case MID_COLOR_THEME_INDEX:
        {
            rVal <<= sal_Int16(maComplexColor.getThemeColorType());
            break;
        }
        case MID_COLOR_TINT_OR_SHADE:
        {
            sal_Int16 nValue = 0;
            for (auto const& rTransform : maComplexColor.getTransformations())
            {
                if (rTransform.meType == model::TransformationType::Tint)
                    nValue = rTransform.mnValue;
                else if (rTransform.meType == model::TransformationType::Shade)
                    nValue = -rTransform.mnValue;
            }
            rVal <<= nValue;
            break;
        }
        case MID_COLOR_LUM_MOD:
        {
            sal_Int16 nValue = 10000;
            for (auto const& rTransform : maComplexColor.getTransformations())
            {
                if (rTransform.meType == model::TransformationType::LumMod)
                    nValue = rTransform.mnValue;
            }
            rVal <<= nValue;
            break;
        }
        case MID_COLOR_LUM_OFF:
        {
            sal_Int16 nValue = 0;
            for (auto const& rTransform : maComplexColor.getTransformations())
            {
                if (rTransform.meType == model::TransformationType::LumOff)
                    nValue = rTransform.mnValue;
            }
            rVal <<= nValue;
            break;
        }
        case MID_COMPLEX_COLOR_JSON:
        {
            rVal <<= OStringToOUString(model::color::convertToJSON(maComplexColor), RTL_TEXTENCODING_UTF8);
            break;
        }
        case MID_COMPLEX_COLOR:
        {
            auto xComplexColor = model::color::createXComplexColor(maComplexColor);
            rVal <<= xComplexColor;
            break;
        }
        case MID_COLOR_RGB:
        default:
        {
            rVal <<= mColor;
            break;
        }
    }
    return true;
}

bool SvxColorItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
        case MID_COLOR_ALPHA:
        {
            sal_Int16 nTransparency = 0;
            bool bRet = rVal >>= nTransparency;
            if (bRet)
            {
                auto fTransparency = static_cast<double>(nTransparency) * 255 / 100;
                mColor.SetAlpha(255 - static_cast<sal_uInt8>(basegfx::fround(fTransparency)));
            }
            return bRet;
        }
        case MID_GRAPHIC_TRANSPARENT:
        {
            mColor.SetAlpha( Any2Bool( rVal ) ? 0 : 255 );
            return true;
        }
        case MID_COLOR_THEME_INDEX:
        {
            sal_Int16 nIndex = -1;
            if (!(rVal >>= nIndex))
                return false;
            maComplexColor.setThemeColor(model::convertToThemeColorType(nIndex));
        }
        break;
        case MID_COLOR_TINT_OR_SHADE:
        {
            sal_Int16 nTintShade = 0;
            if (!(rVal >>= nTintShade))
                return false;

            maComplexColor.removeTransformations(model::TransformationType::Tint);
            maComplexColor.removeTransformations(model::TransformationType::Shade);

            if (nTintShade > 0)
                maComplexColor.addTransformation({model::TransformationType::Tint, nTintShade});
            else if (nTintShade < 0)
            {
                sal_Int16 nShade = o3tl::narrowing<sal_Int16>(-nTintShade);
                maComplexColor.addTransformation({model::TransformationType::Shade, nShade});
            }
        }
        break;
        case MID_COLOR_LUM_MOD:
        {
            sal_Int16 nLumMod = 10000;
            if (!(rVal >>= nLumMod))
                return false;
            maComplexColor.removeTransformations(model::TransformationType::LumMod);
            maComplexColor.addTransformation({model::TransformationType::LumMod, nLumMod});
        }
        break;
        case MID_COLOR_LUM_OFF:
        {
            sal_Int16 nLumOff = 0;
            if (!(rVal >>= nLumOff))
                return false;
            maComplexColor.removeTransformations(model::TransformationType::LumOff);
            maComplexColor.addTransformation({model::TransformationType::LumOff, nLumOff});
        }
        break;
        case MID_COMPLEX_COLOR_JSON:
        {
            OUString sComplexColorJson;
            if (!(rVal >>= sComplexColorJson))
                return false;

            if (sComplexColorJson.isEmpty())
                return false;

            OString aJSON = OUStringToOString(sComplexColorJson, RTL_TEXTENCODING_ASCII_US);
            if (!model::color::convertFromJSON(aJSON, maComplexColor))
                return false;
        }
        break;
        case MID_COMPLEX_COLOR:
        {
            css::uno::Reference<css::util::XComplexColor> xComplexColor;
            if (!(rVal >>= xComplexColor))
                return false;

            if (xComplexColor.is())
                maComplexColor = model::color::getFromXComplexColor(xComplexColor);
        }
        break;
        case MID_COLOR_RGB:
        default:
        {
            if (!(rVal >>= mColor))
                return false;
        }
        break;
    }
    return true;
}

SvxColorItem* SvxColorItem::Clone( SfxItemPool * ) const
{
    return new SvxColorItem( *this );
}

bool SvxColorItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper& /*rIntl*/
)   const
{
    rText = ::GetColorString( mColor );
    return true;
}

void SvxColorItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SvxColorItem"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));

    std::stringstream ss;
    ss << mColor;
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(ss.str().c_str()));

    OUString aStr;
    IntlWrapper aIntlWrapper(SvtSysLocale().GetUILanguageTag());
    GetPresentation( SfxItemPresentation::Complete, MapUnit::Map100thMM, MapUnit::Map100thMM, aStr, aIntlWrapper);
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("presentation"), BAD_CAST(OUStringToOString(aStr, RTL_TEXTENCODING_UTF8).getStr()));

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("complex-color"));

    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("type"),
                                      BAD_CAST(OString::number(sal_Int16(maComplexColor.getType())).getStr()));

    for (auto const& rTransform : maComplexColor.getTransformations())
    {
        (void)xmlTextWriterStartElement(pWriter, BAD_CAST("transformation"));
        (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("type"),
                                      BAD_CAST(OString::number(sal_Int16(rTransform.meType)).getStr()));
        (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                      BAD_CAST(OString::number(rTransform.mnValue).getStr()));
        (void)xmlTextWriterEndElement(pWriter);
    }

    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterEndElement(pWriter);
}

// class SvxKerningItem --------------------------------------------------

SvxKerningItem::SvxKerningItem( const short nKern, const sal_uInt16 nId ) :
    SfxInt16Item( nId, nKern )
{
}

SvxKerningItem* SvxKerningItem::Clone( SfxItemPool * ) const
{
    return new SvxKerningItem( *this );
}

void SvxKerningItem::ScaleMetrics( tools::Long nMult, tools::Long nDiv )
{
    SetValue( static_cast<sal_Int16>(BigInt::Scale( GetValue(), nMult, nDiv )) );
}


bool SvxKerningItem::HasMetrics() const
{
    return true;
}


bool SvxKerningItem::GetPresentation
(
    SfxItemPresentation ePres,
    MapUnit             eCoreUnit,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper& rIntl
)   const
{
    switch ( ePres )
    {
        case SfxItemPresentation::Nameless:
            rText = GetMetricText( static_cast<tools::Long>(GetValue()), eCoreUnit, MapUnit::MapPoint, &rIntl ) +
                    " " + EditResId(GetMetricId(MapUnit::MapPoint));
            return true;
        case SfxItemPresentation::Complete:
        {
            rText = EditResId(RID_SVXITEMS_KERNING_COMPLETE);
            TranslateId pId;

            if ( GetValue() > 0 )
                pId = RID_SVXITEMS_KERNING_EXPANDED;
            else if ( GetValue() < 0 )
                pId = RID_SVXITEMS_KERNING_CONDENSED;

            if (pId)
                rText += EditResId(pId);
            rText += GetMetricText( static_cast<tools::Long>(GetValue()), eCoreUnit, MapUnit::MapPoint, &rIntl ) +
                    " " + EditResId(GetMetricId(MapUnit::MapPoint));
            return true;
        }
        default: ; //prevent warning
    }
    return false;
}

bool SvxKerningItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    sal_Int16 nVal = GetValue();
    if(nMemberId & CONVERT_TWIPS)
        nVal = static_cast<sal_Int16>(convertTwipToMm100(nVal));
    rVal <<= nVal;
    return true;
}

bool SvxKerningItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId)
{
    sal_Int16 nVal = sal_Int16();
    if(!(rVal >>= nVal))
        return false;
    if(nMemberId & CONVERT_TWIPS)
        nVal = o3tl::toTwips(nVal, o3tl::Length::mm100);
    SetValue(nVal);
    return true;
}

// class SvxCaseMapItem --------------------------------------------------

SvxCaseMapItem::SvxCaseMapItem( const SvxCaseMap eMap, const sal_uInt16 nId ) :
    SfxEnumItem( nId, eMap )
{
}

sal_uInt16 SvxCaseMapItem::GetValueCount() const
{
    return sal_uInt16(SvxCaseMap::End); // SvxCaseMap::SmallCaps + 1
}

SvxCaseMapItem* SvxCaseMapItem::Clone( SfxItemPool * ) const
{
    return new SvxCaseMapItem( *this );
}

bool SvxCaseMapItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper& /*rIntl*/
)   const
{
    rText = GetValueTextByPos( static_cast<sal_uInt16>(GetValue()) );
    return true;
}

OUString SvxCaseMapItem::GetValueTextByPos( sal_uInt16 nPos )
{
    static TranslateId RID_SVXITEMS_CASEMAP[] =
    {
        RID_SVXITEMS_CASEMAP_NONE,
        RID_SVXITEMS_CASEMAP_UPPERCASE,
        RID_SVXITEMS_CASEMAP_LOWERCASE,
        RID_SVXITEMS_CASEMAP_TITLE,
        RID_SVXITEMS_CASEMAP_SMALLCAPS
    };

    static_assert(std::size(RID_SVXITEMS_CASEMAP) == size_t(SvxCaseMap::End), "must match");
    assert(nPos < sal_uInt16(SvxCaseMap::End) && "enum overflow!");
    return EditResId(RID_SVXITEMS_CASEMAP[nPos]);
}

bool SvxCaseMapItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    sal_Int16 nRet = style::CaseMap::NONE;
    switch( GetValue() )
    {
        case SvxCaseMap::Uppercase   :      nRet = style::CaseMap::UPPERCASE; break;
        case SvxCaseMap::Lowercase     :      nRet = style::CaseMap::LOWERCASE; break;
        case SvxCaseMap::Capitalize       :      nRet = style::CaseMap::TITLE    ; break;
        case SvxCaseMap::SmallCaps:      nRet = style::CaseMap::SMALLCAPS; break;
        default: break;
    }
    rVal <<= nRet;
    return true;
}

bool SvxCaseMapItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    sal_uInt16 nVal = sal_uInt16();
    if(!(rVal >>= nVal))
        return false;

    SvxCaseMap eVal;
    switch( nVal )
    {
    case style::CaseMap::NONE    :   eVal = SvxCaseMap::NotMapped; break;
    case style::CaseMap::UPPERCASE:  eVal = SvxCaseMap::Uppercase;  break;
    case style::CaseMap::LOWERCASE:  eVal = SvxCaseMap::Lowercase; break;
    case style::CaseMap::TITLE    :  eVal = SvxCaseMap::Capitalize; break;
    case style::CaseMap::SMALLCAPS:  eVal = SvxCaseMap::SmallCaps; break;
    default: return false;
    }
    SetValue(eVal);
    return true;
}

// class SvxEscapementItem -----------------------------------------------

SvxEscapementItem::SvxEscapementItem( const sal_uInt16 nId ) :
    SfxEnumItemInterface( nId ),

    nEsc    ( 0 ),
    nProp   ( 100 )
{
}


SvxEscapementItem::SvxEscapementItem( const SvxEscapement eEscape,
                                      const sal_uInt16 nId ) :
    SfxEnumItemInterface( nId ),
    nProp( 100 )
{
    SetEscapement( eEscape );
    if( nEsc )
        nProp = DFLT_ESC_PROP;
}


SvxEscapementItem::SvxEscapementItem( const short _nEsc,
                                      const sal_uInt8 _nProp,
                                      const sal_uInt16 nId ) :
    SfxEnumItemInterface( nId ),
    nEsc    ( _nEsc ),
    nProp   ( _nProp )
{
}


bool SvxEscapementItem::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));

    return( nEsc  == static_cast<const SvxEscapementItem&>(rAttr).nEsc &&
            nProp == static_cast<const SvxEscapementItem&>(rAttr).nProp );
}

SvxEscapementItem* SvxEscapementItem::Clone( SfxItemPool * ) const
{
    return new SvxEscapementItem( *this );
}

sal_uInt16 SvxEscapementItem::GetValueCount() const
{
    return sal_uInt16(SvxEscapement::End);  // SvxEscapement::Subscript + 1
}


bool SvxEscapementItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper& /*rIntl*/
)   const
{
    rText = GetValueTextByPos( GetEnumValue() );

    if ( nEsc != 0 )
    {
        if( DFLT_ESC_AUTO_SUPER == nEsc || DFLT_ESC_AUTO_SUB == nEsc )
            rText += EditResId(RID_SVXITEMS_ESCAPEMENT_AUTO);
        else
            rText += OUString::number( nEsc ) + "%";
    }
    return true;
}

OUString SvxEscapementItem::GetValueTextByPos( sal_uInt16 nPos )
{
    static TranslateId RID_SVXITEMS_ESCAPEMENT[] =
    {
        RID_SVXITEMS_ESCAPEMENT_OFF,
        RID_SVXITEMS_ESCAPEMENT_SUPER,
        RID_SVXITEMS_ESCAPEMENT_SUB
    };

    static_assert(std::size(RID_SVXITEMS_ESCAPEMENT) == size_t(SvxEscapement::End), "must match");
    assert(nPos < sal_uInt16(SvxEscapement::End) && "enum overflow!");
    return EditResId(RID_SVXITEMS_ESCAPEMENT[nPos]);
}

sal_uInt16 SvxEscapementItem::GetEnumValue() const
{
    if ( nEsc < 0 )
        return sal_uInt16(SvxEscapement::Subscript);
    else if ( nEsc > 0 )
        return sal_uInt16(SvxEscapement::Superscript);
    return sal_uInt16(SvxEscapement::Off);
}


void SvxEscapementItem::SetEnumValue( sal_uInt16 nVal )
{
    SetEscapement( static_cast<SvxEscapement>(nVal) );
}

bool SvxEscapementItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
        case MID_ESC:
            rVal <<= static_cast<sal_Int16>(nEsc);
        break;
        case MID_ESC_HEIGHT:
            rVal <<= static_cast<sal_Int8>(nProp);
        break;
        case MID_AUTO_ESC:
            rVal <<= (DFLT_ESC_AUTO_SUB == nEsc || DFLT_ESC_AUTO_SUPER == nEsc);
        break;
    }
    return true;
}

bool SvxEscapementItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
        case MID_ESC:
        {
            sal_Int16 nVal = sal_Int16();
            if( (rVal >>= nVal) && (std::abs(nVal) <= MAX_ESC_POS+1))
                nEsc = nVal;
            else
                return false;
        }
        break;
        case MID_ESC_HEIGHT:
        {
            sal_Int8 nVal = sal_Int8();
            if( (rVal >>= nVal) && (nVal <= 100))
                nProp = nVal;
            else
                return false;
        }
        break;
        case MID_AUTO_ESC:
        {
            bool bVal = Any2Bool(rVal);
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
    return true;
}

// class SvxLanguageItem -------------------------------------------------

ItemInstanceManager* SvxLanguageItem::getItemInstanceManager() const
{
    static DefaultItemInstanceManager aInstanceManager(typeid(SvxLanguageItem).hash_code());
    return &aInstanceManager;
}

SvxLanguageItem::SvxLanguageItem( const LanguageType eLang, const sal_uInt16 nId )
    : SvxLanguageItem_Base( nId , eLang )
{
}


sal_uInt16 SvxLanguageItem::GetValueCount() const
{
    // #i50205# got rid of class International
    SAL_WARN( "editeng.items", "SvxLanguageItem::GetValueCount: supposed to return a count of what?");
    // Could be SvtLanguageTable::GetEntryCount() (all locales with resource string)?
    // Could be LocaleDataWrapper::getInstalledLanguageTypes() (all locales with locale data)?
    return 0;
}

SvxLanguageItem* SvxLanguageItem::Clone( SfxItemPool * ) const
{
    return new SvxLanguageItem( *this );
}

bool SvxLanguageItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper& /*rIntl*/
)   const
{
    rText = SvtLanguageTable::GetLanguageString( GetValue() );
    return true;
}

bool SvxLanguageItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
        case MID_LANG_INT:  // for basic conversions!
            rVal <<= static_cast<sal_Int16>(static_cast<sal_uInt16>(GetValue()));
        break;
        case MID_LANG_LOCALE:
            lang::Locale aRet( LanguageTag::convertToLocale( GetValue(), false));
            rVal <<= aRet;
        break;
    }
    return true;
}

bool SvxLanguageItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
        case MID_LANG_INT:  // for basic conversions!
        {
            sal_Int32 nValue = 0;
            if(!(rVal >>= nValue))
                return false;

            ASSERT_CHANGE_REFCOUNTED_ITEM;
            SetValue(LanguageType(nValue));
        }
        break;
        case MID_LANG_LOCALE:
        {
            lang::Locale aLocale;
            if(!(rVal >>= aLocale))
                return false;

            ASSERT_CHANGE_REFCOUNTED_ITEM;
            SetValue( LanguageTag::convertToLanguageType( aLocale, false));
        }
        break;
    }
    return true;
}

// class SvxNoHyphenItem -------------------------------------------------

SvxNoHyphenItem::SvxNoHyphenItem( const bool bNoHyphen, const sal_uInt16 nId ) :
    SfxBoolItem( nId, bNoHyphen )
{
}

SvxNoHyphenItem* SvxNoHyphenItem::Clone( SfxItemPool* ) const
{
    return new SvxNoHyphenItem( *this );
}

bool SvxNoHyphenItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper& /*rIntl*/
)   const
{
    if ( GetValue() )
        rText = EditResId(RID_SVXITEMS_NOHYPHENATION_TRUE);
    else
        rText.clear();

    return GetValue();
}

/*
 * Dummy item for ToolBox controls:
 *
 */


// class SvxBlinkItem -------------------------------------------------


SvxBlinkItem::SvxBlinkItem( const bool bBlink, const sal_uInt16 nId ) :
    SfxBoolItem( nId, bBlink )
{
}

SvxBlinkItem* SvxBlinkItem::Clone( SfxItemPool * ) const
{
    return new SvxBlinkItem( *this );
}

bool SvxBlinkItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper& /*rIntl*/
)   const
{
    TranslateId pId = RID_SVXITEMS_BLINK_FALSE;

    if ( GetValue() )
        pId = RID_SVXITEMS_BLINK_TRUE;
    rText = EditResId(pId);
    return true;
}

// class SvxEmphaisMarkItem ---------------------------------------------------

ItemInstanceManager* SvxEmphasisMarkItem::getItemInstanceManager() const
{
    static DefaultItemInstanceManager aInstanceManager(typeid(SvxEmphasisMarkItem).hash_code());
    return &aInstanceManager;
}

SvxEmphasisMarkItem::SvxEmphasisMarkItem( const FontEmphasisMark nValue,
                                        TypedWhichId<SvxEmphasisMarkItem> nId )
    : SfxUInt16Item( nId, static_cast<sal_uInt16>(nValue) )
{
}

SvxEmphasisMarkItem* SvxEmphasisMarkItem::Clone( SfxItemPool * ) const
{
    return new SvxEmphasisMarkItem( *this );
}

bool SvxEmphasisMarkItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper& /*rIntl*/
)   const
{
    static TranslateId RID_SVXITEMS_EMPHASIS[] =
    {
        RID_SVXITEMS_EMPHASIS_NONE_STYLE,
        RID_SVXITEMS_EMPHASIS_DOT_STYLE,
        RID_SVXITEMS_EMPHASIS_CIRCLE_STYLE,
        RID_SVXITEMS_EMPHASIS_DISC_STYLE,
        RID_SVXITEMS_EMPHASIS_ACCENT_STYLE
    };

    FontEmphasisMark nVal = GetEmphasisMark();
    rText = EditResId(RID_SVXITEMS_EMPHASIS[
                           static_cast<sal_uInt16>(static_cast<FontEmphasisMark>( nVal & FontEmphasisMark::Style ))]);
    TranslateId pId = ( FontEmphasisMark::PosAbove & nVal )
                    ? RID_SVXITEMS_EMPHASIS_ABOVE_POS
                    : ( FontEmphasisMark::PosBelow & nVal )
                        ? RID_SVXITEMS_EMPHASIS_BELOW_POS
                        : TranslateId();
    if( pId )
        rText += EditResId( pId );
    return true;
}

bool SvxEmphasisMarkItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
    case MID_EMPHASIS:
    {
        FontEmphasisMark nValue = GetEmphasisMark();
        sal_Int16 nRet = 0;
        switch(nValue & FontEmphasisMark::Style)
        {
            case FontEmphasisMark::NONE   : nRet = FontEmphasis::NONE;           break;
            case FontEmphasisMark::Dot    : nRet = FontEmphasis::DOT_ABOVE;      break;
            case FontEmphasisMark::Circle : nRet = FontEmphasis::CIRCLE_ABOVE;   break;
            case FontEmphasisMark::Disc   : nRet = FontEmphasis::DISK_ABOVE;     break;
            case FontEmphasisMark::Accent : nRet = FontEmphasis::ACCENT_ABOVE;   break;
            default: break;
        }
        if(nRet && nValue & FontEmphasisMark::PosBelow)
            nRet += 10;
        rVal <<= nRet;
    }
    break;
    }
    return true;
}

bool SvxEmphasisMarkItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
    case MID_EMPHASIS:
    {
        sal_Int32 nValue = -1;
        rVal >>= nValue;
        FontEmphasisMark nMark;
        switch(nValue)
        {
            case FontEmphasis::NONE        : nMark = FontEmphasisMark::NONE;   break;
            case FontEmphasis::DOT_ABOVE   : nMark = FontEmphasisMark::Dot|FontEmphasisMark::PosAbove;    break;
            case FontEmphasis::CIRCLE_ABOVE: nMark = FontEmphasisMark::Circle|FontEmphasisMark::PosAbove; break;
            case FontEmphasis::DISK_ABOVE  : nMark = FontEmphasisMark::Disc|FontEmphasisMark::PosAbove;   break;
            case FontEmphasis::ACCENT_ABOVE: nMark = FontEmphasisMark::Accent|FontEmphasisMark::PosAbove; break;
            case FontEmphasis::DOT_BELOW   : nMark = FontEmphasisMark::Dot|FontEmphasisMark::PosBelow;    break;
            case FontEmphasis::CIRCLE_BELOW: nMark = FontEmphasisMark::Circle|FontEmphasisMark::PosBelow; break;
            case FontEmphasis::DISK_BELOW  : nMark = FontEmphasisMark::Disc|FontEmphasisMark::PosBelow;   break;
            case FontEmphasis::ACCENT_BELOW: nMark = FontEmphasisMark::Accent|FontEmphasisMark::PosBelow; break;
            default: return false;
        }
        ASSERT_CHANGE_REFCOUNTED_ITEM;
        SetValue( static_cast<sal_Int16>(nMark) );
    }
    break;
    }
    return true;
}

/*************************************************************************
|*    class SvxTwoLinesItem
*************************************************************************/

SvxTwoLinesItem::SvxTwoLinesItem( bool bFlag, sal_Unicode nStartBracket,
                                    sal_Unicode nEndBracket, sal_uInt16 nW )
    : SfxPoolItem( nW ),
    cStartBracket( nStartBracket ), cEndBracket( nEndBracket ), bOn( bFlag )
{
}

SvxTwoLinesItem::~SvxTwoLinesItem()
{
}

bool SvxTwoLinesItem::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return bOn == static_cast<const SvxTwoLinesItem&>(rAttr).bOn &&
           cStartBracket == static_cast<const SvxTwoLinesItem&>(rAttr).cStartBracket &&
           cEndBracket == static_cast<const SvxTwoLinesItem&>(rAttr).cEndBracket;
}

SvxTwoLinesItem* SvxTwoLinesItem::Clone( SfxItemPool* ) const
{
    return new SvxTwoLinesItem( *this );
}

bool SvxTwoLinesItem::QueryValue( css::uno::Any& rVal,
                                sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch( nMemberId )
    {
    case MID_TWOLINES:
        rVal <<= bOn;
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
        bRet = false;
        break;
    }
    return bRet;
}

bool SvxTwoLinesItem::PutValue( const css::uno::Any& rVal,
                                    sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = false;
    OUString s;
    switch( nMemberId )
    {
    case MID_TWOLINES:
        bOn = Any2Bool( rVal );
        bRet = true;
        break;
    case MID_START_BRACKET:
        if( rVal >>= s )
        {
            cStartBracket = s.isEmpty() ? 0 : s[ 0 ];
            bRet = true;
        }
        break;
    case MID_END_BRACKET:
        if( rVal >>= s )
        {
            cEndBracket = s.isEmpty() ? 0 : s[ 0 ];
            bRet = true;
        }
        break;
    }
    return bRet;
}

bool SvxTwoLinesItem::GetPresentation( SfxItemPresentation /*ePres*/,
                            MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/,
                            OUString &rText, const IntlWrapper& /*rIntl*/ ) const
{
    if( !GetValue() )
        rText = EditResId( RID_SVXITEMS_TWOLINES_OFF );
    else
    {
        rText = EditResId( RID_SVXITEMS_TWOLINES );
        if( GetStartBracket() )
            rText = OUStringChar(GetStartBracket()) + rText;
        if( GetEndBracket() )
            rText += OUStringChar(GetEndBracket());
    }
    return true;
}


/*************************************************************************
|*    class SvxTextRotateItem
*************************************************************************/

SvxTextRotateItem::SvxTextRotateItem(Degree10 nValue, TypedWhichId<SvxTextRotateItem> nW)
    : SfxUInt16Item(nW, nValue.get())
{
}

SvxTextRotateItem* SvxTextRotateItem::Clone(SfxItemPool*) const
{
    return new SvxTextRotateItem(*this);
}

bool SvxTextRotateItem::GetPresentation(
    SfxItemPresentation /*ePres*/,
    MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/,
    OUString &rText, const IntlWrapper&) const
{
    if (!GetValue())
        rText = EditResId(RID_SVXITEMS_TEXTROTATE_OFF);
    else
    {
        rText = EditResId(RID_SVXITEMS_TEXTROTATE);
        rText = rText.replaceFirst("$(ARG1)",
            OUString::number(toDegrees(GetValue())));
    }
    return true;
}

bool SvxTextRotateItem::QueryValue(css::uno::Any& rVal,
    sal_uInt8 nMemberId) const
{
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch (nMemberId)
    {
    case MID_ROTATE:
        rVal <<= static_cast<sal_Int16>(GetValue());
        break;
    default:
        bRet = false;
        break;
    }
    return bRet;
}

bool SvxTextRotateItem::PutValue(const css::uno::Any& rVal, sal_uInt8 nMemberId)
{
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch (nMemberId)
    {
    case MID_ROTATE:
    {
        sal_Int16 nVal = 0;
        if ((rVal >>= nVal) && (0 == nVal || 900 == nVal || 2700 == nVal))
            SetValue(Degree10(nVal));
        else
            bRet = false;
        break;
    }
    default:
        bRet = false;
    }
    return bRet;
}

void SvxTextRotateItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SvxTextRotateItem"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(OString::number(GetValue().get()).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
}


/*************************************************************************
|*    class SvxCharRotateItem
*************************************************************************/

SvxCharRotateItem::SvxCharRotateItem( Degree10 nValue,
                                       bool bFitIntoLine,
                                       TypedWhichId<SvxCharRotateItem> nW )
    : SvxTextRotateItem(nValue, nW), bFitToLine( bFitIntoLine )
{
}

SvxCharRotateItem* SvxCharRotateItem::Clone( SfxItemPool* ) const
{
    return new SvxCharRotateItem( *this );
}

bool SvxCharRotateItem::GetPresentation(
        SfxItemPresentation /*ePres*/,
        MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/,
        OUString &rText, const IntlWrapper&) const
{
    if( !GetValue() )
        rText = EditResId( RID_SVXITEMS_CHARROTATE_OFF );
    else
    {
        rText = EditResId( RID_SVXITEMS_CHARROTATE );
        rText = rText.replaceFirst( "$(ARG1)",
                    OUString::number( toDegrees(GetValue()) ));
        if( IsFitToLine() )
            rText += EditResId( RID_SVXITEMS_CHARROTATE_FITLINE );
    }
    return true;
}

bool SvxCharRotateItem::QueryValue( css::uno::Any& rVal,
                                sal_uInt8 nMemberId ) const
{
    bool bRet = true;
    switch(nMemberId & ~CONVERT_TWIPS)
    {
    case MID_ROTATE:
        SvxTextRotateItem::QueryValue(rVal, nMemberId);
        break;
    case MID_FITTOLINE:
        rVal <<= IsFitToLine();
        break;
    default:
        bRet = false;
        break;
    }
    return bRet;
}

bool SvxCharRotateItem::PutValue( const css::uno::Any& rVal,
                                    sal_uInt8 nMemberId )
{
    bool bRet = true;
    switch(nMemberId & ~CONVERT_TWIPS)
    {
    case MID_ROTATE:
        {
            bRet = SvxTextRotateItem::PutValue(rVal, nMemberId);
            break;
        }

    case MID_FITTOLINE:
        SetFitToLine( Any2Bool( rVal ) );
        break;
    default:
        bRet = false;
    }
    return bRet;
}

bool SvxCharRotateItem::operator==( const SfxPoolItem& rItem ) const
{
    assert(SfxPoolItem::operator==(rItem));
    return SvxTextRotateItem::operator==( rItem ) &&
           IsFitToLine() == static_cast<const SvxCharRotateItem&>(rItem).IsFitToLine();
}

void SvxCharRotateItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SvxCharRotateItem"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(OString::number(GetValue().get()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("fitToLine"), BAD_CAST(OString::boolean(IsFitToLine()).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
}

/*************************************************************************
|*    class SvxCharScaleItem
*************************************************************************/

SvxCharScaleWidthItem::SvxCharScaleWidthItem( sal_uInt16 nValue,
                                               TypedWhichId<SvxCharScaleWidthItem> nW )
    : SfxUInt16Item( nW, nValue )
{
}

SvxCharScaleWidthItem* SvxCharScaleWidthItem::Clone( SfxItemPool* ) const
{
    return new SvxCharScaleWidthItem( *this );
}

bool SvxCharScaleWidthItem::GetPresentation(
        SfxItemPresentation /*ePres*/,
        MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/,
        OUString &rText, const IntlWrapper&) const
{
    if( !GetValue() )
        rText = EditResId( RID_SVXITEMS_CHARSCALE_OFF );
    else
    {
        rText = EditResId( RID_SVXITEMS_CHARSCALE );
        rText = rText.replaceFirst( "$(ARG1)",
                    OUString::number( GetValue() ));
    }
    return true;
}

bool SvxCharScaleWidthItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    // SfxUInt16Item::QueryValue returns sal_Int32 in Any now... (srx642w)
    // where we still want this to be a sal_Int16
    sal_Int16 nValue = sal_Int16();
    if (rVal >>= nValue)
    {
        SetValue( static_cast<sal_uInt16>(nValue) );
        return true;
    }

    SAL_WARN("editeng.items", "SvxCharScaleWidthItem::PutValue - Wrong type!" );
    return false;
}

bool SvxCharScaleWidthItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    // SfxUInt16Item::QueryValue returns sal_Int32 in Any now... (srx642w)
    // where we still want this to be a sal_Int16
    rVal <<= static_cast<sal_Int16>(GetValue());
    return true;
}

/*************************************************************************
|*    class SvxCharReliefItem
*************************************************************************/

ItemInstanceManager* SvxCharReliefItem::getItemInstanceManager() const
{
    static DefaultItemInstanceManager aInstanceManager(typeid(SvxCharReliefItem).hash_code());
    return &aInstanceManager;
}

SvxCharReliefItem::SvxCharReliefItem( FontRelief eValue,
                                         const sal_uInt16 nId )
    : SfxEnumItem( nId, eValue )
{
}

SvxCharReliefItem* SvxCharReliefItem::Clone( SfxItemPool * ) const
{
    return new SvxCharReliefItem( *this );
}

static TranslateId RID_SVXITEMS_RELIEF[] =
{
    RID_SVXITEMS_RELIEF_NONE,
    RID_SVXITEMS_RELIEF_EMBOSSED,
    RID_SVXITEMS_RELIEF_ENGRAVED
};

OUString SvxCharReliefItem::GetValueTextByPos(sal_uInt16 nPos)
{
    assert(nPos < std::size(RID_SVXITEMS_RELIEF) && "enum overflow");
    return EditResId(RID_SVXITEMS_RELIEF[nPos]);
}

sal_uInt16 SvxCharReliefItem::GetValueCount() const
{
    return std::size(RID_SVXITEMS_RELIEF) - 1;
}

bool SvxCharReliefItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper& /*rIntl*/
)   const
{
    rText = GetValueTextByPos( static_cast<sal_uInt16>(GetValue()) );
    return true;
}

bool SvxCharReliefItem::PutValue( const css::uno::Any& rVal,
                                        sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch( nMemberId )
    {
    case MID_RELIEF:
        {
            sal_Int16 nVal = -1;
            rVal >>= nVal;
            if(nVal >= 0 && nVal <= sal_Int16(FontRelief::Engraved))
            {
                ASSERT_CHANGE_REFCOUNTED_ITEM;
                SetValue( static_cast<FontRelief>(nVal) );
            }
            else
                bRet = false;
        }
        break;
    default:
        bRet = false;
        break;
    }
    return bRet;
}

bool SvxCharReliefItem::QueryValue( css::uno::Any& rVal,
                                        sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch( nMemberId )
    {
    case MID_RELIEF:
        rVal <<= static_cast<sal_Int16>(GetValue());
        break;
    default:
        bRet = false;
        break;
    }
    return bRet;
}

/*************************************************************************
|*    class SvxScriptSetItem
*************************************************************************/

SvxScriptSetItem::SvxScriptSetItem( sal_uInt16 nSlotId, SfxItemPool& rPool )
    : SfxSetItem( nSlotId, SfxItemSet( rPool,
                        svl::Items<SID_ATTR_CHAR_FONT, SID_ATTR_CHAR_FONT> ))
{
    sal_uInt16 nLatin, nAsian, nComplex;
    GetWhichIds( nLatin, nAsian, nComplex );
    GetItemSet().MergeRange( nLatin, nLatin );
    GetItemSet().MergeRange( nAsian, nAsian );
    GetItemSet().MergeRange( nComplex, nComplex );
}

SvxScriptSetItem* SvxScriptSetItem::Clone( SfxItemPool * ) const
{
    SvxScriptSetItem* p = new SvxScriptSetItem( Which(), *GetItemSet().GetPool() );
    p->GetItemSet().Put( GetItemSet(), false );
    return p;
}

const SfxPoolItem* SvxScriptSetItem::GetItemOfScriptSet(
                            const SfxItemSet& rSet, sal_uInt16 nId )
{
    const SfxPoolItem* pI;
    SfxItemState eSt = rSet.GetItemState( nId, false, &pI );
    if( SfxItemState::SET != eSt )
        pI = SfxItemState::DEFAULT == eSt ? &rSet.Get( nId ) : nullptr;
    return pI;
}

const SfxPoolItem* SvxScriptSetItem::GetItemOfScript( sal_uInt16 nSlotId, const SfxItemSet& rSet, SvtScriptType nScript )
{
    sal_uInt16 nLatin, nAsian, nComplex;
    GetWhichIds( nSlotId, rSet, nLatin, nAsian, nComplex );

    const SfxPoolItem *pRet, *pAsn, *pCmplx;
    if (nScript == SvtScriptType::ASIAN)
    {
        pRet = GetItemOfScriptSet( rSet, nAsian );
    } else if (nScript == SvtScriptType::COMPLEX)
    {
        pRet = GetItemOfScriptSet( rSet, nComplex );
    } else if (nScript == (SvtScriptType::LATIN|SvtScriptType::ASIAN))
    {
        if( nullptr == (pRet = GetItemOfScriptSet( rSet, nLatin )) ||
            nullptr == (pAsn = GetItemOfScriptSet( rSet, nAsian )) ||
            *pRet != *pAsn )
            pRet = nullptr;
    } else if (nScript == (SvtScriptType::LATIN|SvtScriptType::COMPLEX))
    {
        if( nullptr == (pRet = GetItemOfScriptSet( rSet, nLatin )) ||
            nullptr == (pCmplx = GetItemOfScriptSet( rSet, nComplex )) ||
            *pRet != *pCmplx )
            pRet = nullptr;
    } else if (nScript == (SvtScriptType::ASIAN|SvtScriptType::COMPLEX))
    {
        if( nullptr == (pRet = GetItemOfScriptSet( rSet, nAsian )) ||
            nullptr == (pCmplx = GetItemOfScriptSet( rSet, nComplex )) ||
            *pRet != *pCmplx )
            pRet = nullptr;
    } else if (nScript == (SvtScriptType::LATIN|SvtScriptType::ASIAN|SvtScriptType::COMPLEX))
    {
        if( nullptr == (pRet = GetItemOfScriptSet( rSet, nLatin )) ||
            nullptr == (pAsn = GetItemOfScriptSet( rSet, nAsian )) ||
            nullptr == (pCmplx = GetItemOfScriptSet( rSet, nComplex )) ||
            *pRet != *pAsn || *pRet != *pCmplx )
            pRet = nullptr;
    } else {
        //no one valid -> match to latin
        pRet = GetItemOfScriptSet( rSet, nLatin );
    }
    return pRet;
}

const SfxPoolItem* SvxScriptSetItem::GetItemOfScript( SvtScriptType nScript ) const
{
    return GetItemOfScript( Which(), GetItemSet(), nScript );
}

void SvxScriptSetItem::PutItemForScriptType( SvtScriptType nScriptType,
                                             const SfxPoolItem& rItem )
{
    sal_uInt16 nLatin, nAsian, nComplex;
    GetWhichIds( nLatin, nAsian, nComplex );

    if( SvtScriptType::LATIN & nScriptType )
    {
        GetItemSet().Put( rItem.CloneSetWhich(nLatin) );
    }
    if( SvtScriptType::ASIAN & nScriptType )
    {
        GetItemSet().Put( rItem.CloneSetWhich(nAsian) );
    }
    if( SvtScriptType::COMPLEX & nScriptType )
    {
        GetItemSet().Put( rItem.CloneSetWhich(nComplex) );
    }
}

void SvxScriptSetItem::GetWhichIds( sal_uInt16 nSlotId, const SfxItemSet& rSet, sal_uInt16& rLatin, sal_uInt16& rAsian, sal_uInt16& rComplex )
{
    const SfxItemPool& rPool = *rSet.GetPool();
    GetSlotIds( nSlotId, rLatin, rAsian, rComplex );
    rLatin = rPool.GetWhichIDFromSlotID( rLatin );
    rAsian = rPool.GetWhichIDFromSlotID( rAsian );
    rComplex = rPool.GetWhichIDFromSlotID( rComplex );
}

void SvxScriptSetItem::GetWhichIds( sal_uInt16& rLatin, sal_uInt16& rAsian,
                                    sal_uInt16& rComplex ) const
{
    GetWhichIds( Which(), GetItemSet(), rLatin, rAsian, rComplex );
}

void SvxScriptSetItem::GetSlotIds( sal_uInt16 nSlotId, sal_uInt16& rLatin,
                                    sal_uInt16& rAsian, sal_uInt16& rComplex )
{
    switch( nSlotId )
    {
    default:
        SAL_WARN( "editeng.items", "wrong SlotId for class SvxScriptSetItem" );
        [[fallthrough]]; // default to font - Id Range !!

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
    case SID_ATTR_CHAR_SHADOWED:
        rLatin = SID_ATTR_CHAR_SHADOWED;
        rAsian = SID_ATTR_CHAR_SHADOWED;
        rComplex = SID_ATTR_CHAR_SHADOWED;
        break;
    case SID_ATTR_CHAR_STRIKEOUT:
        rLatin = SID_ATTR_CHAR_STRIKEOUT;
        rAsian = SID_ATTR_CHAR_STRIKEOUT;
        rComplex = SID_ATTR_CHAR_STRIKEOUT;
        break;
    }
}

void GetDefaultFonts( SvxFontItem& rLatin, SvxFontItem& rAsian, SvxFontItem& rComplex )
{
    const sal_uInt16 nItemCnt = 3;

    static struct
    {
        DefaultFontType nFontType;
        LanguageType    nLanguage;
    }
    const aOutTypeArr[ nItemCnt ] =
    {
        {  DefaultFontType::LATIN_TEXT, LANGUAGE_ENGLISH_US },
        {  DefaultFontType::CJK_TEXT, LANGUAGE_ENGLISH_US },
        {  DefaultFontType::CTL_TEXT, LANGUAGE_ARABIC_SAUDI_ARABIA }
    };

    SvxFontItem* aItemArr[ nItemCnt ] = { &rLatin, &rAsian, &rComplex };

    for ( sal_uInt16 n = 0; n < nItemCnt; ++n )
    {
        vcl::Font aFont( OutputDevice::GetDefaultFont( aOutTypeArr[ n ].nFontType,
                                                  aOutTypeArr[ n ].nLanguage,
                                                  GetDefaultFontFlags::OnlyOne ) );
        SvxFontItem* pItem = aItemArr[ n ];
        pItem->SetFamily( aFont.GetFamilyType() );
        pItem->SetFamilyName( aFont.GetFamilyName() );
        pItem->SetStyleName( OUString() );
        pItem->SetPitch( aFont.GetPitch());
        pItem->SetCharSet(aFont.GetCharSet());
    }
}

// class SvxRsidItem -----------------------------------------------------

typedef std::unordered_map<size_t, const SfxPoolItem*> SvxRsidItemMap;

namespace
{
    class SvxRsidItemInstanceManager : public ItemInstanceManager
    {
        SvxRsidItemMap  maRegistered;

    public:
        SvxRsidItemInstanceManager()
        : ItemInstanceManager(typeid(SvxRsidItem).hash_code())
        {
        }

    private:
        static size_t hashCode(const SfxPoolItem&);

        // standard interface, accessed exclusively
        // by implCreateItemEntry/implCleanupItemEntry
        virtual const SfxPoolItem* find(const SfxPoolItem&) const override;
        virtual void add(const SfxPoolItem&) override;
        virtual void remove(const SfxPoolItem&) override;
    };

    size_t SvxRsidItemInstanceManager::hashCode(const SfxPoolItem& rItem)
    {
        auto const & rRsidItem = static_cast<const SvxRsidItem&>(rItem);
        std::size_t seed(0);
        o3tl::hash_combine(seed, rRsidItem.Which());
        o3tl::hash_combine(seed, rRsidItem.GetValue());
        return seed;
    }

    const SfxPoolItem* SvxRsidItemInstanceManager::find(const SfxPoolItem& rItem) const
    {
        auto aHit(maRegistered.find(hashCode(rItem)));
        if (aHit != maRegistered.end())
            return aHit->second;
        return nullptr;
    }

    void SvxRsidItemInstanceManager::add(const SfxPoolItem& rItem)
    {
        maRegistered.insert({hashCode(rItem), &rItem});
    }

    void SvxRsidItemInstanceManager::remove(const SfxPoolItem& rItem)
    {
        maRegistered.erase(hashCode(rItem));
    }
}

ItemInstanceManager* SvxRsidItem::getItemInstanceManager() const
{
    static SvxRsidItemInstanceManager aInstanceManager;
    return &aInstanceManager;
}

bool SvxRsidItem::QueryValue( uno::Any& rVal, sal_uInt8 ) const
{
    rVal <<= GetValue();
    return true;
}

bool SvxRsidItem::PutValue( const uno::Any& rVal, sal_uInt8 )
{
    sal_uInt32 nRsid = 0;
    if( !( rVal >>= nRsid ) )
        return false;

    SetValue( nRsid );
    return true;
}

SvxRsidItem* SvxRsidItem::Clone( SfxItemPool * ) const
{
    return new SvxRsidItem( *this );
}

bool SvxRsidItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper& /*rIntl*/
)   const
{
    rText.clear();
    return false;
}

void SvxRsidItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SvxRsidItem"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("whichId"), "%d", Which());
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("value"), "%" SAL_PRIuUINT32, GetValue());
    (void)xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
