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

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/table/BorderLine.hpp>
#include <com/sun/star/text/XTextColumns.hpp>

#include <o3tl/string_view.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <editeng/eeitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/ulspitem.hxx>
#include <svl/hint.hxx>
#include <svl/intitem.hxx>
#include <svl/itemset.hxx>

#include <svx/xflbmtit.hxx>
#include <svx/xflbstit.hxx>
#include <svx/xlnclit.hxx>
#include <editeng/bulletitem.hxx>
#include <editeng/lrspitem.hxx>
#include <svx/unoshprp.hxx>
#include <svx/unoshape.hxx>
#include <svx/svdpool.hxx>
#include <svx/sdmetitm.hxx>
#include <svx/sdtaaitm.hxx>
#include <svx/sdtacitm.hxx>
#include <svx/sdtayitm.hxx>
#include <svx/sdtaiitm.hxx>
#include <svx/SvxXTextColumns.hxx>
#include <svx/xit.hxx>
#include <svx/xflclit.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <stlsheet.hxx>
#include <sdresid.hxx>
#include <sdpage.hxx>
#include <drawdoc.hxx>
#include <stlpool.hxx>
#include <strings.hrc>
#include <app.hrc>
#include <strings.hxx>
#include <glob.hxx>
#include <DrawViewShell.hxx>
#include <ViewShellBase.hxx>

#include <cstddef>
#include <memory>
#include <string_view>

using ::osl::MutexGuard;
using ::com::sun::star::table::BorderLine;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::drawing;

#define WID_STYLE_HIDDEN    7997
#define WID_STYLE_DISPNAME  7998
#define WID_STYLE_FAMILY    7999

static SvxItemPropertySet& GetStylePropertySet()
{
    static const SfxItemPropertyMapEntry aFullPropertyMap_Impl[] =
    {
        { u"Family"_ustr,                 WID_STYLE_FAMILY,       ::cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY,    0},
        { u"UserDefinedAttributes"_ustr,  SDRATTR_XMLATTRIBUTES,  cppu::UnoType<XNameContainer>::get(), 0,     0},
        { u"DisplayName"_ustr,            WID_STYLE_DISPNAME,     ::cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY,    0},
        { u"Hidden"_ustr,                 WID_STYLE_HIDDEN,       cppu::UnoType<bool>::get(),       0,     0},

        SVX_UNOEDIT_NUMBERING_PROPERTY,
        SHADOW_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        FILL_PROPERTIES
        EDGERADIUS_PROPERTIES
        TEXT_PROPERTIES_DEFAULTS
        CONNECTOR_PROPERTIES
        SPECIAL_DIMENSIONING_PROPERTIES_DEFAULTS
        { u"TopBorder"_ustr,                    SDRATTR_TABLE_BORDER,           ::cppu::UnoType<BorderLine>::get(), 0, TOP_BORDER },
        { u"BottomBorder"_ustr,                 SDRATTR_TABLE_BORDER,           ::cppu::UnoType<BorderLine>::get(), 0, BOTTOM_BORDER },
        { u"LeftBorder"_ustr,                   SDRATTR_TABLE_BORDER,           ::cppu::UnoType<BorderLine>::get(), 0, LEFT_BORDER },
        { u"RightBorder"_ustr,                  SDRATTR_TABLE_BORDER,           ::cppu::UnoType<BorderLine>::get(), 0, RIGHT_BORDER },
    };

    static SvxItemPropertySet aPropSet( aFullPropertyMap_Impl, SdrObject::GetGlobalDrawObjectItemPool() );
    return aPropSet;
}

class ModifyListenerForwarder : public SfxListener
{
public:
    explicit ModifyListenerForwarder( SdStyleSheet* pStyleSheet );

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

private:
    SdStyleSheet* mpStyleSheet;
};

ModifyListenerForwarder::ModifyListenerForwarder( SdStyleSheet* pStyleSheet )
: mpStyleSheet( pStyleSheet )
{
    if( pStyleSheet )
    {
        SfxBroadcaster& rBC = static_cast< SfxBroadcaster& >( *pStyleSheet );
        StartListening( rBC );
    }
}

void ModifyListenerForwarder::Notify(SfxBroadcaster& /*rBC*/, const SfxHint& /*rHint*/)
{
    if( mpStyleSheet )
        mpStyleSheet->notifyModifyListener();
}

SdStyleSheet::SdStyleSheet(const OUString& rDisplayName, SfxStyleSheetBasePool& _rPool, SfxStyleFamily eFamily, SfxStyleSearchBits _nMask)
: SdStyleSheetBase( rDisplayName, _rPool, eFamily, _nMask)
, msApiName( rDisplayName )
, mxPool( &_rPool )
{
}

SdStyleSheet::~SdStyleSheet()
{
    delete pSet;
    pSet = nullptr;    // that following destructors also get a change
}

void SdStyleSheet::SetApiName( const OUString& rApiName )
{
    msApiName = rApiName;
}

OUString const & SdStyleSheet::GetApiName() const
{
    if( !msApiName.isEmpty() )
        return msApiName;
    else
        return GetName();
}

bool SdStyleSheet::SetParent(const OUString& rParentName)
{
    bool bResult = false;

    if (SfxStyleSheet::SetParent(rParentName))
    {
        // PseudoStyleSheets do not have their own ItemSets
        if (nFamily != SfxStyleFamily::Pseudo)
        {
            if( !rParentName.isEmpty() )
            {
                SfxStyleSheetBase* pStyle = m_pPool->Find(rParentName, nFamily);
                if (pStyle)
                {
                    bResult = true;
                    SfxItemSet& rParentSet = pStyle->GetItemSet();
                    GetItemSet().SetParent(&rParentSet);
                    Broadcast( SfxHint( SfxHintId::DataChanged ) );
                }
            }
            else
            {
                bResult = true;
                GetItemSet().SetParent(nullptr);
                Broadcast( SfxHint( SfxHintId::DataChanged ) );
            }
        }
        else
        {
            bResult = true;
        }
    }
    return bResult;
}

/**
 * create if necessary and return ItemSets
 */
SfxItemSet& SdStyleSheet::GetItemSet()
{
    if (nFamily == SfxStyleFamily::Para || nFamily == SfxStyleFamily::Page)
    {
        // we create the ItemSet 'on demand' if necessary
        if (!pSet)
        {
            pSet = new SfxItemSetFixed<
                    XATTR_LINE_FIRST, XATTR_LINE_LAST,
                    XATTR_FILL_FIRST, XATTR_FILL_LAST,
                    SDRATTR_SHADOW_FIRST, SDRATTR_SHADOW_LAST,
                    SDRATTR_TEXT_MINFRAMEHEIGHT, SDRATTR_TEXT_WORDWRAP,
                    SDRATTR_EDGE_FIRST, SDRATTR_MEASURE_LAST,
                    SDRATTR_3D_FIRST, SDRATTR_3D_LAST,
                    EE_PARA_START, EE_CHAR_END>(GetPool()->GetPool());
            bMySet = true;
        }

        return *pSet;
    }

    else if( nFamily == SfxStyleFamily::Frame )
    {
        if (!pSet)
        {
            pSet = new SfxItemSetFixed<
                    XATTR_LINE_FIRST, XATTR_LINE_LAST,
                    XATTR_FILL_FIRST, XATTR_FILL_LAST,
                    SDRATTR_SHADOW_FIRST, SDRATTR_SHADOW_LAST,
                    SDRATTR_TEXT_MINFRAMEHEIGHT, SDRATTR_XMLATTRIBUTES,
                    SDRATTR_TEXT_WORDWRAP, SDRATTR_TEXT_WORDWRAP,
                    SDRATTR_TABLE_FIRST, SDRATTR_TABLE_LAST,
                    EE_PARA_START, EE_CHAR_END>(GetPool()->GetPool());

            bMySet = true;
        }

        return *pSet;
    }

    // this is a dummy template for the internal template of the
    // current presentation layout; return the ItemSet of that template
    else
    {

        SdStyleSheet* pSdSheet = GetRealStyleSheet();

        if (pSdSheet)
        {
            return pSdSheet->GetItemSet();
        }
        else
        {
            if (!pSet)
            {
                pSet = new SfxItemSetFixed<
                        XATTR_LINE_FIRST, XATTR_LINE_LAST,
                        XATTR_FILL_FIRST, XATTR_FILL_LAST,
                        SDRATTR_SHADOW_FIRST, SDRATTR_SHADOW_LAST,
                        SDRATTR_TEXT_MINFRAMEHEIGHT, SDRATTR_TEXT_WORDWRAP,
                        SDRATTR_EDGE_FIRST, SDRATTR_MEASURE_LAST,
                        SDRATTR_3D_FIRST, SDRATTR_3D_LAST,
                        EE_PARA_START, EE_CHAR_END>(GetPool()->GetPool());
                bMySet = true;
            }

            return(*pSet);
        }
    }
}

/**
 * A template is used when it is referenced by inserted object or by a used
 * template.
 */
bool SdStyleSheet::IsUsed() const
{
    bool bResult = false;

    ForAllListeners(
        [this, &bResult] (SfxListener* pListener)
        {
            if( pListener == this )
                return false; // continue

            const svl::StyleSheetUser* const pUser(dynamic_cast<svl::StyleSheetUser*>(pListener));
            if (pUser)
                bResult = pUser->isUsedByModel();
            if (bResult)
                return true; // break loop
            return false;
        });

    if( !bResult )
    {
        std::unique_lock aGuard( m_aMutex );

        if( maModifyListeners.getLength(aGuard) )
        {
            std::vector<css::uno::Reference<XModifyListener>> aModifyListeners( maModifyListeners.getElements(aGuard) );
            bResult = std::any_of(aModifyListeners.begin(), aModifyListeners.end(),
                [](const Reference<XInterface>& rListener) {
                    Reference< XStyle > xStyle( rListener, UNO_QUERY );
                    try
                    {
                        Reference<XPropertySet> xPropertySet(xStyle, UNO_QUERY_THROW);
                        if (xPropertySet->getPropertyValue(u"IsPhysical"_ustr).get<bool>())
                            return true;
                    }
                    catch (const Exception&)
                    {
                    }
                    return xStyle.is() && xStyle->isInUse();
                });
        }
    }
    return bResult;
}

/**
 * Checks if a cell style is used in two places at once.
 * Typically we modify the formatting of a single place,
 * so such style shouldn't be edited directly.
 */
bool SdStyleSheet::IsEditable()
{
    if (GetFamily() != SfxStyleFamily::Frame)
        return true;

    if (!IsUserDefined())
        return false;

    bool bFoundOne = false;
    ForAllListeners(
        [this, &bFoundOne] (SfxListener* pListener)
        {
            if (pListener != this && dynamic_cast<SdStyleSheet*>(pListener))
            {
                bFoundOne = true;
                return true; // break loop
            }
            return false;
        });
    if (bFoundOne)
        return false;

    std::unique_lock aGuard(m_aMutex);
    return maModifyListeners.getLength(aGuard) <= 1;
}

/**
 * Determine the style sheet for which this dummy is for.
 */
SdStyleSheet* SdStyleSheet::GetRealStyleSheet() const
{
    OUString aRealStyle;
    OUString aSep( SD_LT_SEPARATOR );
    SdStyleSheet* pRealStyle = nullptr;
    SdDrawDocument* pDoc = static_cast<SdStyleSheetPool*>(m_pPool)->GetDoc();

    ::sd::DrawViewShell* pDrawViewShell = nullptr;

    ::sd::ViewShellBase* pBase = dynamic_cast< ::sd::ViewShellBase* >( SfxViewShell::Current() );
    if( pBase )
        pDrawViewShell = dynamic_cast< ::sd::DrawViewShell* >( pBase->GetMainViewShell().get() );

    if (pDrawViewShell && pDrawViewShell->GetDoc() == pDoc)
    {
        SdPage* pPage = pDrawViewShell->getCurrentPage();
        if( pPage )
        {
            aRealStyle = pPage->GetLayoutName();
            // cut after separator string

            if( aRealStyle.indexOf(aSep) >= 0)
            {
                aRealStyle = aRealStyle.copy(0,(aRealStyle.indexOf(aSep) + aSep.getLength()));
            }
        }
    }
    if (aRealStyle.isEmpty())
    {
        SdPage* pPage = pDoc->GetSdPage(0, PageKind::Standard);

        if (pPage)
        {
            aRealStyle = pDoc->GetSdPage(0, PageKind::Standard)->GetLayoutName();
        }
        else
        {
            /* no page available yet. This can happen when actualizing the
               document templates.  */
            SfxStyleSheetIterator aIter(m_pPool, SfxStyleFamily::Page);
            SfxStyleSheetBase* pSheet = aIter.First();
            if( pSheet )
                aRealStyle = pSheet->GetName();
        }

        if( aRealStyle.indexOf(aSep) >= 0)
        {
            aRealStyle = aRealStyle.copy(0,(aRealStyle.indexOf(aSep) + aSep.getLength()));
        }
    }

    /* now map from the name (specified for country language) to the internal
       name (independent of the country language)  */
    OUString aInternalName;
    OUString aStyleName(aName);

    if (aStyleName == SdResId(STR_PSEUDOSHEET_TITLE))
    {
        aInternalName = STR_LAYOUT_TITLE;
    }
    else if (aStyleName == SdResId(STR_PSEUDOSHEET_SUBTITLE))
    {
        aInternalName = STR_LAYOUT_SUBTITLE;
    }
    else if (aStyleName == SdResId(STR_PSEUDOSHEET_BACKGROUND))
    {
        aInternalName = STR_LAYOUT_BACKGROUND;
    }
    else if (aStyleName == SdResId(STR_PSEUDOSHEET_BACKGROUNDOBJECTS))
    {
        aInternalName = STR_LAYOUT_BACKGROUNDOBJECTS;
    }
    else if (aStyleName == SdResId(STR_PSEUDOSHEET_NOTES))
    {
        aInternalName = STR_LAYOUT_NOTES;
    }
    else
    {
        OUString aOutlineStr(SdResId(STR_PSEUDOSHEET_OUTLINE));
        sal_Int32 nPos = aStyleName.indexOf(aOutlineStr);
        if (nPos >= 0)
        {
            std::u16string_view aNumStr(aStyleName.subView(aOutlineStr.getLength()));
            aInternalName = STR_LAYOUT_OUTLINE + aNumStr;
        }
    }

    aRealStyle += aInternalName;
    pRealStyle = static_cast< SdStyleSheet* >( m_pPool->Find(aRealStyle, SfxStyleFamily::Page) );

#ifdef DBG_UTIL
    if( !pRealStyle )
    {
        SfxStyleSheetIterator aIter(m_pPool, SfxStyleFamily::Page);
        if( aIter.Count() > 0 )
            // StyleSheet not found, but pool already loaded
            DBG_ASSERT(pRealStyle, "Internal StyleSheet not found");
    }
#endif

    return pRealStyle;
}

/**
 * Determine pseudo style sheet which stands for this style sheet.
 */
SdStyleSheet* SdStyleSheet::GetPseudoStyleSheet() const
{
    SdStyleSheet* pPseudoStyle = nullptr;
    OUString aSep( SD_LT_SEPARATOR );
    OUString aStyleName(aName);
        // without layout name and separator

    if( aStyleName.indexOf(aSep) >=0 )
    {
        aStyleName = aStyleName.copy (aStyleName.indexOf(aSep) + aSep.getLength());
    }

    if (aStyleName == STR_LAYOUT_TITLE)
    {
        aStyleName = SdResId(STR_PSEUDOSHEET_TITLE);
    }
    else if (aStyleName == STR_LAYOUT_SUBTITLE)
    {
        aStyleName = SdResId(STR_PSEUDOSHEET_SUBTITLE);
    }
    else if (aStyleName == STR_LAYOUT_BACKGROUND)
    {
        aStyleName = SdResId(STR_PSEUDOSHEET_BACKGROUND);
    }
    else if (aStyleName == STR_LAYOUT_BACKGROUNDOBJECTS)
    {
        aStyleName = SdResId(STR_PSEUDOSHEET_BACKGROUNDOBJECTS);
    }
    else if (aStyleName == STR_LAYOUT_NOTES)
    {
        aStyleName = SdResId(STR_PSEUDOSHEET_NOTES);
    }
    else
    {
        OUString aOutlineStr(STR_LAYOUT_OUTLINE);
        sal_Int32 nPos = aStyleName.indexOf(aOutlineStr);
        if (nPos != -1)
        {
            std::u16string_view aNumStr(aStyleName.subView(aOutlineStr.getLength()));
            aStyleName = SdResId(STR_PSEUDOSHEET_OUTLINE) + aNumStr;
        }
    }

    pPseudoStyle = static_cast<SdStyleSheet*>(m_pPool->Find(aStyleName, SfxStyleFamily::Pseudo));
    DBG_ASSERT(pPseudoStyle, "PseudoStyleSheet missing");

    return pPseudoStyle;
}

void SdStyleSheet::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    // first, base class functionality
    SfxStyleSheet::Notify(rBC, rHint);

    if (nFamily != SfxStyleFamily::Pseudo)
        return;

    /* if the dummy gets a notify about a changed attribute, he takes care that
       the actual meant style sheet sends broadcasts. */
    if (rHint.GetId() == SfxHintId::DataChanged)
    {
        SdStyleSheet* pRealStyle = GetRealStyleSheet();
        if (pRealStyle)
            pRealStyle->Broadcast(rHint);
    }
}

/**
 * Adjust the bullet width and the left text indent of the provided ItemSets to
 * their font height. The new values are calculated that the ratio to the font
 * height is as in the style sheet.
 *
 * @param bOnlyMissingItems If sal_True, only not set items are completed. With
 * sal_False, are items are overwritten.
 */
void SdStyleSheet::AdjustToFontHeight(SfxItemSet& rSet, bool bOnlyMissingItems)
{
    /* If not explicit set, adjust bullet width and text indent to new font
       height. */
    SfxStyleFamily eFamily = nFamily;
    OUString aStyleName(aName);
    if (eFamily == SfxStyleFamily::Pseudo)
    {
        SfxStyleSheet* pRealStyle = GetRealStyleSheet();
        eFamily = pRealStyle->GetFamily();
        aStyleName = pRealStyle->GetName();
    }

    if (!(eFamily == SfxStyleFamily::Page &&
          aStyleName.indexOf(STR_LAYOUT_OUTLINE) != -1 &&
          rSet.GetItemState(EE_CHAR_FONTHEIGHT) == SfxItemState::SET))
        return;

    const SfxItemSet* pCurSet = &GetItemSet();
    sal_uInt32 nNewHeight = rSet.Get(EE_CHAR_FONTHEIGHT).GetHeight();
    sal_uInt32 nOldHeight = pCurSet->Get(EE_CHAR_FONTHEIGHT).GetHeight();

    if (rSet.GetItemState(EE_PARA_BULLET) != SfxItemState::SET || !bOnlyMissingItems)
    {
        const SvxBulletItem& rBItem = pCurSet->Get(EE_PARA_BULLET);
        double fBulletFraction = double(rBItem.GetWidth()) / nOldHeight;
        SvxBulletItem aNewBItem(rBItem);
        aNewBItem.SetWidth(static_cast<sal_uInt32>(fBulletFraction * nNewHeight));
        rSet.Put(aNewBItem);
    }

    if (rSet.GetItemState(EE_PARA_LRSPACE) != SfxItemState::SET || !bOnlyMissingItems)
    {
        const SvxLRSpaceItem& rLRItem = pCurSet->Get(EE_PARA_LRSPACE);
        double fIndentFraction = double(rLRItem.GetTextLeft()) / nOldHeight;
        SvxLRSpaceItem aNewLRItem(rLRItem);
        aNewLRItem.SetTextLeft(fIndentFraction * nNewHeight);
        double fFirstIndentFraction = double(rLRItem.GetTextFirstLineOffset()) / nOldHeight;
        aNewLRItem.SetTextFirstLineOffset(static_cast<short>(fFirstIndentFraction * nNewHeight));
        rSet.Put(aNewLRItem);
    }

    if (rSet.GetItemState(EE_PARA_ULSPACE) != SfxItemState::SET || !bOnlyMissingItems)
    {
        const SvxULSpaceItem& rULItem = pCurSet->Get(EE_PARA_ULSPACE);
        SvxULSpaceItem aNewULItem(rULItem);
        double fLowerFraction = double(rULItem.GetLower()) / nOldHeight;
        aNewULItem.SetLower(static_cast<sal_uInt16>(fLowerFraction * nNewHeight));
        double fUpperFraction = double(rULItem.GetUpper()) / nOldHeight;
        aNewULItem.SetUpper(static_cast<sal_uInt16>(fUpperFraction * nNewHeight));
        rSet.Put(aNewULItem);
    }
}

bool SdStyleSheet::HasFollowSupport() const
{
    return false;
}

bool SdStyleSheet::HasParentSupport() const
{
    return true;
}

bool SdStyleSheet::HasClearParentSupport() const
{
    return true;
}

namespace
{
struct ApiNameMap
{
    std::u16string_view mpApiName;
    sal_uInt32 mnHelpId;
} const pApiNameMap[]
    = { { std::u16string_view(u"title"), HID_PSEUDOSHEET_TITLE },
        { std::u16string_view(u"subtitle"), HID_PSEUDOSHEET_SUBTITLE },
        { std::u16string_view(u"background"), HID_PSEUDOSHEET_BACKGROUND },
        { std::u16string_view(u"backgroundobjects"), HID_PSEUDOSHEET_BACKGROUNDOBJECTS },
        { std::u16string_view(u"notes"), HID_PSEUDOSHEET_NOTES },
        { std::u16string_view(u"standard"), HID_STANDARD_STYLESHEET_NAME },
        { std::u16string_view(u"objectwithoutfill"), HID_POOLSHEET_OBJWITHOUTFILL },
        { std::u16string_view(u"Object with no fill and no line"), HID_POOLSHEET_OBJNOLINENOFILL },

        { std::u16string_view(u"Text"), HID_POOLSHEET_TEXT },
        { std::u16string_view(u"A4"), HID_POOLSHEET_A4 },
        { std::u16string_view(u"Title A4"), HID_POOLSHEET_A4_TITLE },
        { std::u16string_view(u"Heading A4"), HID_POOLSHEET_A4_HEADLINE },
        { std::u16string_view(u"Text A4"), HID_POOLSHEET_A4_TEXT },
        { std::u16string_view(u"A0"), HID_POOLSHEET_A0 },
        { std::u16string_view(u"Title A0"), HID_POOLSHEET_A0_TITLE },
        { std::u16string_view(u"Heading A0"), HID_POOLSHEET_A0_HEADLINE },
        { std::u16string_view(u"Text A0"), HID_POOLSHEET_A0_TEXT },

        { std::u16string_view(u"Graphic"), HID_POOLSHEET_GRAPHIC },
        { std::u16string_view(u"Shapes"), HID_POOLSHEET_SHAPES },
        { std::u16string_view(u"Filled"), HID_POOLSHEET_FILLED },
        { std::u16string_view(u"Filled Blue"), HID_POOLSHEET_FILLED_BLUE },
        { std::u16string_view(u"Filled Green"), HID_POOLSHEET_FILLED_GREEN },
        { std::u16string_view(u"Filled Red"), HID_POOLSHEET_FILLED_RED },
        { std::u16string_view(u"Filled Yellow"), HID_POOLSHEET_FILLED_YELLOW },
        { std::u16string_view(u"Outlined"), HID_POOLSHEET_OUTLINE },
        { std::u16string_view(u"Outlined Blue"), HID_POOLSHEET_OUTLINE_BLUE },
        { std::u16string_view(u"Outlined Green"), HID_POOLSHEET_OUTLINE_GREEN },
        { std::u16string_view(u"Outlined Red"), HID_POOLSHEET_OUTLINE_RED },
        { std::u16string_view(u"Outlined Yellow"), HID_POOLSHEET_OUTLINE_YELLOW },
        { std::u16string_view(u"Lines"), HID_POOLSHEET_LINES },
        { std::u16string_view(u"Arrow Line"), HID_POOLSHEET_MEASURE },
        { std::u16string_view(u"Arrow Dashed"), HID_POOLSHEET_LINES_DASHED }
      };

OUString GetApiNameForHelpId(sal_uInt32 nId)
{
    if ((nId >= HID_PSEUDOSHEET_OUTLINE1) && (nId <= HID_PSEUDOSHEET_OUTLINE9))
        return "outline" + OUStringChar(sal_Unicode('1' + (nId - HID_PSEUDOSHEET_OUTLINE1)));

    for (const auto& i : pApiNameMap)
        if (nId == i.mnHelpId)
            return OUString(i.mpApiName);

    return OUString();
}

sal_uInt32 GetHelpIdForApiName(std::u16string_view sName)
{
    std::u16string_view sRest;
    if (o3tl::starts_with(sName, u"outline", &sRest))
    {
        if (sRest.length() == 1)
        {
            sal_Unicode ch = sRest.front();
            if ('1' <= ch && ch <= '9')
                return HID_PSEUDOSHEET_OUTLINE1 + ch - '1';
        }
        // No other pre-defined names start with "outline"
        return 0;
    }

    for (const auto& i : pApiNameMap)
        if (sName == i.mpApiName)
            return i.mnHelpId;

    return 0;
}
}

void SdStyleSheet::SetHelpId( const OUString& r, sal_uInt32 nId )
{
    SfxStyleSheet::SetHelpId( r, nId );

    const OUString sNewApiName = GetApiNameForHelpId(nId);
    if (!sNewApiName.isEmpty())
        msApiName = sNewApiName;
}

OUString SdStyleSheet::GetFamilyString( SfxStyleFamily eFamily )
{
    switch( eFamily )
    {
    case SfxStyleFamily::Frame:
        return u"cell"_ustr;
    default:
        OSL_FAIL( "SdStyleSheet::GetFamilyString(), illegal family!" );
        [[fallthrough]];
    case SfxStyleFamily::Para:
        return u"graphics"_ustr;
    }
}

void SdStyleSheet::throwIfDisposed()
{
    if( !mxPool.is() )
        throw DisposedException();
}

rtl::Reference<SdStyleSheet> SdStyleSheet::CreateEmptyUserStyle( SfxStyleSheetBasePool& rPool, SfxStyleFamily eFamily )
{
    OUString aName;
    sal_Int32 nIndex = 1;
    do
    {
        aName = "user" + OUString::number( nIndex++ );
    }
    while( rPool.Find( aName, eFamily ) != nullptr );

    return new SdStyleSheet(aName, rPool, eFamily, SfxStyleSearchBits::UserDefined);
}

// XInterface

void SAL_CALL SdStyleSheet::release(  ) noexcept
{
    if (osl_atomic_decrement( &m_refCount ) != 0)
        return;

    // restore reference count:
    osl_atomic_increment( &m_refCount );
    if (! m_bDisposed) try
    {
        dispose();
    }
    catch (RuntimeException const&)
    {
        // don't break throw ()
        TOOLS_WARN_EXCEPTION( "sd", "" );
    }
    OSL_ASSERT( m_bDisposed );
    SdStyleSheetBase::release();
}

// XComponent

void SAL_CALL SdStyleSheet::dispose(  )
{
    {
        std::unique_lock aGuard(m_aMutex);
        if (m_bDisposed || m_bInDispose)
            return;

        m_bInDispose = true;
    }
    try
    {
        std::unique_lock aGuard(m_aMutex);
        // side effect: keeping a reference to this
        EventObject aEvt( static_cast< OWeakObject * >( this ) );
        try
        {
            maModifyListeners.disposeAndClear( aGuard, aEvt );
            maEventListeners.disposeAndClear( aGuard, aEvt );
            disposing();
        }
        catch (...)
        {
            // bDisposed and bInDispose must be set in this order:
            m_bDisposed = true;
            m_bInDispose = false;
            throw;
        }
        // bDisposed and bInDispose must be set in this order:
        m_bDisposed = true;
        m_bInDispose = false;
    }
    catch (RuntimeException &)
    {
        throw;
    }
    catch (const Exception & exc)
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw css::lang::WrappedTargetRuntimeException(
            "unexpected UNO exception caught: " + exc.Message ,
            nullptr, anyEx );
    }
}

void SdStyleSheet::disposing()
{
    SolarMutexGuard aGuard;
    if (bMySet)
    {
        delete pSet;
    }
    pSet = nullptr;
    m_pPool = nullptr;
    mxPool.clear();
}

void SAL_CALL SdStyleSheet::addEventListener( const Reference< XEventListener >& xListener )
{
    std::unique_lock aGuard( m_aMutex );
    if (m_bDisposed || m_bInDispose)
    {
        aGuard.unlock();
        EventObject aEvt( static_cast< OWeakObject * >( this ) );
        xListener->disposing( aEvt );
    }
    else
    {
        maEventListeners.addInterface( aGuard, xListener );
    }
}

void SAL_CALL SdStyleSheet::removeEventListener( const Reference< XEventListener >& xListener  )
{
    std::unique_lock aGuard( m_aMutex );
    maEventListeners.removeInterface( aGuard, xListener );
}

// XModifyBroadcaster

void SAL_CALL SdStyleSheet::addModifyListener( const Reference< XModifyListener >& xListener )
{
    std::unique_lock aGuard( m_aMutex );
    if (m_bDisposed || m_bInDispose)
    {
        aGuard.unlock();
        EventObject aEvt( static_cast< OWeakObject * >( this ) );
        xListener->disposing( aEvt );
    }
    else
    {
        if (!mpModifyListenerForwarder)
            mpModifyListenerForwarder.reset( new ModifyListenerForwarder( this ) );
        maModifyListeners.addInterface( aGuard, xListener );
    }
}

void SAL_CALL SdStyleSheet::removeModifyListener( const Reference< XModifyListener >& xListener )
{
    std::unique_lock aGuard( m_aMutex );
    maModifyListeners.removeInterface( aGuard, xListener );
}

void SdStyleSheet::notifyModifyListener()
{
    std::unique_lock aGuard( m_aMutex );

    if( maModifyListeners.getLength(aGuard) )
    {
        EventObject aEvt( static_cast< OWeakObject * >( this ) );
        maModifyListeners.notifyEach(aGuard, &XModifyListener::modified, aEvt);
    }
}

// XServiceInfo
OUString SAL_CALL SdStyleSheet::getImplementationName()
{
    return u"SdStyleSheet"_ustr;
}

sal_Bool SAL_CALL SdStyleSheet::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

Sequence< OUString > SAL_CALL SdStyleSheet::getSupportedServiceNames()
{
    return { u"com.sun.star.style.Style"_ustr,
             u"com.sun.star.drawing.FillProperties"_ustr,
             u"com.sun.star.drawing.LineProperties"_ustr,
             u"com.sun.star.drawing.ShadowProperties"_ustr,
             u"com.sun.star.drawing.ConnectorProperties"_ustr,
             u"com.sun.star.drawing.MeasureProperties"_ustr,
             u"com.sun.star.style.ParagraphProperties"_ustr,
             u"com.sun.star.style.CharacterProperties"_ustr,
             u"com.sun.star.drawing.TextProperties"_ustr,
             u"com.sun.star.drawing.Text"_ustr };
}

bool SdStyleSheet::SetName(const OUString& rNewName, bool bReindexNow)
{
    const bool bResult = SfxUnoStyleSheet::SetName(rNewName, bReindexNow);
    // Don't overwrite predefined API names
    if (bResult && GetHelpIdForApiName(msApiName) == 0)
    {
        msApiName = rNewName;
        Broadcast(SfxHint(SfxHintId::DataChanged));
    }
    return bResult;
}

// XNamed
OUString SAL_CALL SdStyleSheet::getName()
{
    SolarMutexGuard aGuard;
    throwIfDisposed();
    return GetApiName();
}

void SAL_CALL SdStyleSheet::setName( const OUString& rName  )
{
    SolarMutexGuard aGuard;
    throwIfDisposed();
    SetName(rName);
}

// XStyle

sal_Bool SAL_CALL SdStyleSheet::isUserDefined()
{
    SolarMutexGuard aGuard;
    throwIfDisposed();
    return IsUserDefined();
}

sal_Bool SAL_CALL SdStyleSheet::isInUse()
{
    SolarMutexGuard aGuard;
    throwIfDisposed();
    return IsUsed();
}

OUString SAL_CALL SdStyleSheet::getParentStyle()
{
    SolarMutexGuard aGuard;
    throwIfDisposed();

    if( !GetParent().isEmpty() )
    {
        SdStyleSheet* pParentStyle = static_cast< SdStyleSheet* >( mxPool->Find( GetParent(), nFamily ) );
        if( pParentStyle )
            return pParentStyle->GetApiName();
    }
    return OUString();
}

void SAL_CALL SdStyleSheet::setParentStyle( const OUString& rParentName  )
{
    SolarMutexGuard aGuard;
    throwIfDisposed();

    if( !rParentName.isEmpty() )
    {
        OUString const name(GetName());
        sal_Int32 const sep(name.indexOf(SD_LT_SEPARATOR));
        OUString const master((sep == -1) ? OUString() : name.copy(0, sep));
        std::shared_ptr<SfxStyleSheetIterator> aSSSI = std::make_shared<SfxStyleSheetIterator>(mxPool.get(), nFamily);
        for (SfxStyleSheetBase *pStyle = aSSSI->First(); pStyle; pStyle = aSSSI->Next())
        {
            // we hope that we have only sd style sheets
            SdStyleSheet* pSdStyleSheet = static_cast<SdStyleSheet*>(pStyle);
            OUString const curName(pStyle->GetName());
            sal_Int32 const curSep(curName.indexOf(SD_LT_SEPARATOR));
            OUString const curMaster((curSep == -1)
                    ? OUString() : curName.copy(0, curSep));
            // check that the master matches, as msApiName exists once per
            // master page
            if (pSdStyleSheet->msApiName == rParentName && master == curMaster)
            {
                if( pStyle != this )
                {
                    SetParent(curName);
                }
                return;
            }
        }
        throw NoSuchElementException();
    }
    else
    {
        SetParent( rParentName );
    }
}

// XPropertySet/XMultiPropertySet utility functions

// Does not broadcast
// Must be guarded by solar mutex; must not be disposed
void SdStyleSheet::setPropertyValue_Impl(const OUString& aPropertyName, const css::uno::Any& aValue)
{
    const SfxItemPropertyMapEntry* pEntry = getPropertyMapEntry( aPropertyName );
    if( pEntry == nullptr )
    {
        throw UnknownPropertyException( aPropertyName, static_cast<cppu::OWeakObject*>(this));
    }

    if( pEntry->nWID == WID_STYLE_HIDDEN )
    {
        bool bValue = false;
        if ( aValue >>= bValue )
            SetHidden( bValue );
        return;
    }
    if( pEntry->nWID == SDRATTR_TEXTDIRECTION )
        return; // not yet implemented for styles

    if( pEntry->nWID == WID_STYLE_FAMILY )
        throw PropertyVetoException();

    if( (pEntry->nWID == EE_PARA_NUMBULLET) && (GetFamily() == SfxStyleFamily::Page) )
    {
        OUString aStr;
        const sal_uInt32 nTempHelpId = GetHelpId( aStr );

        if( (nTempHelpId >= HID_PSEUDOSHEET_OUTLINE2) && (nTempHelpId <= HID_PSEUDOSHEET_OUTLINE9) )
            return;
    }

    SfxItemSet &rStyleSet = GetItemSet();

    if( pEntry->nWID == OWN_ATTR_FILLBMP_MODE )
    {
        BitmapMode eMode;
        if( aValue >>= eMode )
        {
            rStyleSet.Put( XFillBmpStretchItem( eMode == BitmapMode_STRETCH ) );
            rStyleSet.Put( XFillBmpTileItem( eMode == BitmapMode_REPEAT ) );
            return;
        }
        throw IllegalArgumentException();
    }

    if (pEntry->nWID == OWN_ATTR_TEXTCOLUMNS)
    {
        if (css::uno::Reference<css::text::XTextColumns> xColumns; aValue >>= xColumns)
        {
            rStyleSet.Put(SfxInt16Item(SDRATTR_TEXTCOLUMNS_NUMBER, xColumns->getColumnCount()));
            if (css::uno::Reference<css::beans::XPropertySet> xPropSet{ xColumns,
                                                                        css::uno::UNO_QUERY })
            {
                auto aVal = xPropSet->getPropertyValue(u"AutomaticDistance"_ustr);
                if (sal_Int32 nSpacing; aVal >>= nSpacing)
                    rStyleSet.Put(SdrMetricItem(SDRATTR_TEXTCOLUMNS_SPACING, nSpacing));
            }
            return;
        }
        throw IllegalArgumentException();
    }

    SfxItemSet aSet( GetPool()->GetPool(), pEntry->nWID, pEntry->nWID);
    aSet.Put( rStyleSet );

    if( !aSet.Count() )
    {
        if( EE_PARA_NUMBULLET == pEntry->nWID )
        {
            vcl::Font aBulletFont;
            SdStyleSheetPool::PutNumBulletItem( this, aBulletFont );
            aSet.Put( rStyleSet );
        }
        else
        {
            aSet.Put( GetPool()->GetPool().GetUserOrPoolDefaultItem( pEntry->nWID ) );
        }
    }

    if( pEntry->nMemberId == MID_NAME &&
        ( pEntry->nWID == XATTR_FILLBITMAP || pEntry->nWID == XATTR_FILLGRADIENT ||
          pEntry->nWID == XATTR_FILLHATCH || pEntry->nWID == XATTR_FILLFLOATTRANSPARENCE ||
          pEntry->nWID == XATTR_LINESTART || pEntry->nWID == XATTR_LINEEND || pEntry->nWID == XATTR_LINEDASH) )
    {
        OUString aTempName;
        if(!(aValue >>= aTempName ))
            throw IllegalArgumentException();

        SvxShape::SetFillAttribute( pEntry->nWID, aTempName, aSet );
    }
    else if(!SvxUnoTextRangeBase::SetPropertyValueHelper( pEntry, aValue, aSet ))
    {
        SvxItemPropertySet_setPropertyValue( pEntry, aValue, aSet );
    }

    rStyleSet.Put( aSet );
}

// Must be guarded by solar mutex; must not be disposed
css::uno::Any SdStyleSheet::getPropertyValue_Impl(const OUString& PropertyName)
{
    const SfxItemPropertyMapEntry* pEntry = getPropertyMapEntry( PropertyName );
    if( pEntry == nullptr )
    {
        throw UnknownPropertyException( PropertyName, static_cast<cppu::OWeakObject*>(this));
    }

    Any aAny;

    if( pEntry->nWID == WID_STYLE_FAMILY )
    {
        if( nFamily == SfxStyleFamily::Page )
        {
            const OUString aLayoutName( GetName() );
            aAny <<= aLayoutName.copy( 0, aLayoutName.indexOf( SD_LT_SEPARATOR) );
        }
        else
        {
            aAny <<= GetFamilyString(nFamily);
        }
    }
    else if( pEntry->nWID == WID_STYLE_DISPNAME )
    {
        OUString aDisplayName;
        if ( nFamily == SfxStyleFamily::Page )
        {
            const SdStyleSheet* pStyleSheet = GetPseudoStyleSheet();
            if (pStyleSheet != nullptr)
                aDisplayName = pStyleSheet->GetName();
        }

        if (aDisplayName.isEmpty())
            aDisplayName = GetName();

        aAny <<= aDisplayName;
    }
    else if( pEntry->nWID == SDRATTR_TEXTDIRECTION )
    {
        aAny <<= false;
    }
    else if( pEntry->nWID == OWN_ATTR_FILLBMP_MODE )
    {
        SfxItemSet &rStyleSet = GetItemSet();

        const XFillBmpStretchItem* pStretchItem = rStyleSet.GetItem<XFillBmpStretchItem>(XATTR_FILLBMP_STRETCH);
        const XFillBmpTileItem* pTileItem = rStyleSet.GetItem<XFillBmpTileItem>(XATTR_FILLBMP_TILE);

        if( pStretchItem && pTileItem )
        {
            if( pTileItem->GetValue() )
                aAny <<= BitmapMode_REPEAT;
            else if( pStretchItem->GetValue() )
                aAny <<= BitmapMode_STRETCH;
            else
                aAny <<= BitmapMode_NO_REPEAT;
        }
    }
    else if( pEntry->nWID == WID_STYLE_HIDDEN )
    {
        aAny <<= IsHidden( );
    }
    else if (pEntry->nWID == OWN_ATTR_TEXTCOLUMNS)
    {
        const SfxItemSet& rStyleSet = GetItemSet();

        auto xIf = SvxXTextColumns_createInstance();
        css::uno::Reference<css::text::XTextColumns> xCols(xIf, css::uno::UNO_QUERY_THROW);
        xCols->setColumnCount(rStyleSet.Get(SDRATTR_TEXTCOLUMNS_NUMBER).GetValue());
        css::uno::Reference<css::beans::XPropertySet> xProp(xIf, css::uno::UNO_QUERY_THROW);
        xProp->setPropertyValue(
            u"AutomaticDistance"_ustr,
            css::uno::Any(rStyleSet.Get(SDRATTR_TEXTCOLUMNS_SPACING).GetValue()));
        aAny <<= xIf;
    }
    else
    {
        SfxItemSet aSet( GetPool()->GetPool(), pEntry->nWID, pEntry->nWID);

        const SfxPoolItem* pItem;
        SfxItemSet& rStyleSet = GetItemSet();

        if( rStyleSet.GetItemState( pEntry->nWID, true, &pItem ) == SfxItemState::SET )
            aSet.Put(  *pItem );

        if( !aSet.Count() )
            aSet.Put( GetPool()->GetPool().GetUserOrPoolDefaultItem( pEntry->nWID ) );

        if(SvxUnoTextRangeBase::GetPropertyValueHelper( aSet, pEntry, aAny ))
            return aAny;

        // Get value of ItemSet
        aAny = SvxItemPropertySet_getPropertyValue( pEntry, aSet );
    }

    if( pEntry->aType != aAny.getValueType() )
    {
        // since the sfx uint16 item now exports a sal_Int32, we may have to fix this here
        if( ( pEntry->aType == ::cppu::UnoType<sal_Int16>::get()) && aAny.getValueType() == ::cppu::UnoType<sal_Int32>::get() )
        {
            sal_Int32 nValue = 0;
            aAny >>= nValue;
            aAny <<= static_cast<sal_Int16>(nValue);
        }
        else
        {
            OSL_FAIL("SvxShape::GetAnyForItem() Returnvalue has wrong Type!" );
        }
    }

    return aAny;
}

// XPropertySet

Reference< XPropertySetInfo > SdStyleSheet::getPropertySetInfo()
{
    throwIfDisposed();
    static Reference< XPropertySetInfo > xInfo = GetStylePropertySet().getPropertySetInfo();
    return xInfo;
}

void SAL_CALL SdStyleSheet::setPropertyValue( const OUString& aPropertyName, const Any& aValue )
{
    SolarMutexGuard aGuard;
    throwIfDisposed();

    setPropertyValue_Impl(aPropertyName, aValue);
    Broadcast(SfxHint(SfxHintId::DataChanged));
}

Any SAL_CALL SdStyleSheet::getPropertyValue( const OUString& PropertyName )
{
    SolarMutexGuard aGuard;
    throwIfDisposed();

    return getPropertyValue_Impl(PropertyName);
}

void SAL_CALL SdStyleSheet::addPropertyChangeListener( const OUString& , const Reference< XPropertyChangeListener >&  ) {}
void SAL_CALL SdStyleSheet::removePropertyChangeListener( const OUString& , const Reference< XPropertyChangeListener >&  ) {}
void SAL_CALL SdStyleSheet::addVetoableChangeListener( const OUString& , const Reference< XVetoableChangeListener >&  ) {}
void SAL_CALL SdStyleSheet::removeVetoableChangeListener( const OUString& , const Reference< XVetoableChangeListener >&  ) {}

// XMultiPropertySet

void SAL_CALL SdStyleSheet::setPropertyValues(const css::uno::Sequence<OUString>& aPropertyNames,
                                              const css::uno::Sequence<css::uno::Any>& aValues)
{
    const sal_Int32 nCount = aPropertyNames.getLength();

    if (nCount != aValues.getLength())
        throw css::lang::IllegalArgumentException();

    if (!nCount)
        return;

    SolarMutexGuard aGuard;
    throwIfDisposed();

    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        try
        {
            setPropertyValue_Impl(aPropertyNames[i], aValues[i]);
        }
        catch (const css::beans::UnknownPropertyException&)
        {
            // ignore this, some code likes to liberally sprinkle properties all over stuff that doesn't support those properties
        }
    }

    Broadcast(SfxHint(SfxHintId::DataChanged));
}

css::uno::Sequence<css::uno::Any>
SAL_CALL SdStyleSheet::getPropertyValues(const css::uno::Sequence<OUString>& aPropertyNames)
{
    SolarMutexGuard aGuard;
    throwIfDisposed();

    const sal_Int32 nCount = aPropertyNames.getLength();
    css::uno::Sequence<css::uno::Any> aValues(nCount);
    Any* pAny = aValues.getArray();

    for (sal_Int32 i = 0; i < nCount; ++i)
        pAny[i] = getPropertyValue_Impl(aPropertyNames[i]);

    return aValues;
}

void SAL_CALL SdStyleSheet::addPropertiesChangeListener(const css::uno::Sequence<OUString>&, const css::uno::Reference<css::beans::XPropertiesChangeListener>&) {}
void SAL_CALL SdStyleSheet::removePropertiesChangeListener(const css::uno::Reference<css::beans::XPropertiesChangeListener>&) {}
void SAL_CALL SdStyleSheet::firePropertiesChangeEvent(const css::uno::Sequence<OUString>&, const css::uno::Reference<css::beans::XPropertiesChangeListener>&) {}

// XPropertyState

PropertyState SAL_CALL SdStyleSheet::getPropertyState( const OUString& PropertyName )
{
    SolarMutexGuard aGuard;

    throwIfDisposed();

    const SfxItemPropertyMapEntry* pEntry = getPropertyMapEntry( PropertyName );

    if( pEntry == nullptr )
        throw UnknownPropertyException( PropertyName, static_cast<cppu::OWeakObject*>(this));

    if( pEntry->nWID == WID_STYLE_FAMILY )
    {
        return PropertyState_DIRECT_VALUE;
    }
    else if( pEntry->nWID == SDRATTR_TEXTDIRECTION )
    {
        return PropertyState_DEFAULT_VALUE;
    }
    else if( pEntry->nWID == OWN_ATTR_FILLBMP_MODE )
    {
        const SfxItemSet& rSet = GetItemSet();

        if( rSet.GetItemState( XATTR_FILLBMP_STRETCH, false ) == SfxItemState::SET ||
            rSet.GetItemState( XATTR_FILLBMP_TILE, false ) == SfxItemState::SET )
        {
            return PropertyState_DIRECT_VALUE;
        }
        else
        {
            return PropertyState_AMBIGUOUS_VALUE;
        }
    }
    else if (pEntry->nWID == OWN_ATTR_TEXTCOLUMNS)
    {
        const SfxItemSet& rSet = GetItemSet();

        const auto eState1 = rSet.GetItemState(SDRATTR_TEXTCOLUMNS_NUMBER, false);
        const auto eState2 = rSet.GetItemState(SDRATTR_TEXTCOLUMNS_SPACING, false);
        if (eState1 == SfxItemState::SET || eState2 == SfxItemState::SET)
            return PropertyState_DIRECT_VALUE;
        else if (eState1 == SfxItemState::DEFAULT && eState2 == SfxItemState::DEFAULT)
            return PropertyState_DEFAULT_VALUE;
        else
            return PropertyState_AMBIGUOUS_VALUE;
    }
    else
    {
        SfxItemSet &rStyleSet = GetItemSet();

        PropertyState eState;

        switch( rStyleSet.GetItemState( pEntry->nWID, false ) )
        {
        case SfxItemState::SET:
            eState = PropertyState_DIRECT_VALUE;
            break;
        case SfxItemState::DEFAULT:
            eState = PropertyState_DEFAULT_VALUE;
            break;
        default:
            eState = PropertyState_AMBIGUOUS_VALUE;
            break;
        }

        // if an item is set, this doesn't mean we want it :)
        if( PropertyState_DIRECT_VALUE == eState )
        {
            switch( pEntry->nWID )
            {
            case XATTR_FILLBITMAP:
            case XATTR_FILLGRADIENT:
            case XATTR_FILLHATCH:
            case XATTR_FILLFLOATTRANSPARENCE:
            case XATTR_LINEEND:
            case XATTR_LINESTART:
            case XATTR_LINEDASH:
                {
                    const NameOrIndex* pItem = rStyleSet.GetItem<NameOrIndex>(pEntry->nWID);
                    if( ( pItem == nullptr ) || pItem->GetName().isEmpty() )
                        eState = PropertyState_DEFAULT_VALUE;
                }
                break;
            case XATTR_FILLCOLOR:
                if (pEntry->nMemberId == MID_COLOR_THEME_INDEX)
                {
                    const XFillColorItem* pColor = rStyleSet.GetItem<XFillColorItem>(pEntry->nWID);
                    if (pColor->getComplexColor().getThemeColorType() == model::ThemeColorType::Unknown)
                    {
                        eState = PropertyState_DEFAULT_VALUE;
                    }
                }
                else if (pEntry->nMemberId == MID_COLOR_LUM_MOD)
                {
                    const XFillColorItem* pColor = rStyleSet.GetItem<XFillColorItem>(pEntry->nWID);
                    sal_Int16 nLumMod = 10000;
                    for (auto const& rTransform : pColor->getComplexColor().getTransformations())
                    {
                        if (rTransform.meType == model::TransformationType::LumMod)
                            nLumMod = rTransform.mnValue;
                    }
                    if (nLumMod == 10000)
                    {
                        eState = PropertyState_DEFAULT_VALUE;
                    }
                }
                else if (pEntry->nMemberId == MID_COLOR_LUM_OFF)
                {
                    const XFillColorItem* pColor = rStyleSet.GetItem<XFillColorItem>(pEntry->nWID);
                    sal_Int16 nLumOff = 0;
                    for (auto const& rTransform : pColor->getComplexColor().getTransformations())
                    {
                        if (rTransform.meType == model::TransformationType::LumOff)
                            nLumOff = rTransform.mnValue;
                    }
                    if (nLumOff == 0)
                    {
                        eState = PropertyState_DEFAULT_VALUE;
                    }
                }
                else if (pEntry->nMemberId == MID_COMPLEX_COLOR)
                {
                    auto const* pColor = rStyleSet.GetItem<XFillColorItem>(pEntry->nWID);
                    if (pColor->getComplexColor().getType() == model::ColorType::Unused)
                    {
                        eState = PropertyState_DEFAULT_VALUE;
                    }
                }
                break;
            case XATTR_LINECOLOR:
                if (pEntry->nMemberId == MID_COMPLEX_COLOR)
                {
                    auto const* pColor = rStyleSet.GetItem<XLineColorItem>(pEntry->nWID);
                    if (pColor->getComplexColor().getType() == model::ColorType::Unused)
                    {
                        eState = PropertyState_DEFAULT_VALUE;
                    }
                }
                break;
            }
        }

        return eState;
    }
}

Sequence< PropertyState > SAL_CALL SdStyleSheet::getPropertyStates( const Sequence< OUString >& aPropertyName )
{
    SolarMutexGuard aGuard;

    throwIfDisposed();

    sal_Int32 nCount = aPropertyName.getLength();

    Sequence< PropertyState > aPropertyStateSequence( nCount );

    std::transform(aPropertyName.begin(), aPropertyName.end(), aPropertyStateSequence.getArray(),
        [this](const OUString& rName) -> PropertyState { return getPropertyState(rName); });

    return aPropertyStateSequence;
}

void SAL_CALL SdStyleSheet::setPropertyToDefault( const OUString& PropertyName )
{
    SolarMutexGuard aGuard;

    throwIfDisposed();

    const SfxItemPropertyMapEntry* pEntry = getPropertyMapEntry( PropertyName );
    if( pEntry == nullptr )
        throw UnknownPropertyException( PropertyName, static_cast<cppu::OWeakObject*>(this));

    SfxItemSet &rStyleSet = GetItemSet();

    if( pEntry->nWID == OWN_ATTR_FILLBMP_MODE )
    {
        rStyleSet.ClearItem( XATTR_FILLBMP_STRETCH );
        rStyleSet.ClearItem( XATTR_FILLBMP_TILE );
    }
    else
    {
        rStyleSet.ClearItem( pEntry->nWID );
    }
    Broadcast(SfxHint(SfxHintId::DataChanged));
}

Any SAL_CALL SdStyleSheet::getPropertyDefault( const OUString& aPropertyName )
{
    SolarMutexGuard aGuard;

    throwIfDisposed();

    const SfxItemPropertyMapEntry* pEntry = getPropertyMapEntry( aPropertyName );
    if( pEntry == nullptr )
        throw UnknownPropertyException( aPropertyName, static_cast<cppu::OWeakObject*>(this));
    Any aRet;
    if( pEntry->nWID == WID_STYLE_FAMILY )
    {
        aRet <<= GetFamilyString(nFamily);
    }
    else if( pEntry->nWID == SDRATTR_TEXTDIRECTION )
    {
        aRet <<= false;
    }
    else if( pEntry->nWID == OWN_ATTR_FILLBMP_MODE )
    {
        aRet <<= BitmapMode_REPEAT;
    }
    else
    {
        SfxItemPool& rMyPool = GetPool()->GetPool();
        SfxItemSet aSet( rMyPool, pEntry->nWID, pEntry->nWID);
        aSet.Put( rMyPool.GetUserOrPoolDefaultItem( pEntry->nWID ) );
        aRet = SvxItemPropertySet_getPropertyValue( pEntry, aSet );
    }
    return aRet;
}

/** this is used because our property map is not sorted yet */
const SfxItemPropertyMapEntry* SdStyleSheet::getPropertyMapEntry( std::u16string_view rPropertyName )
{
    return GetStylePropertySet().getPropertyMapEntry(rPropertyName);
}

//Broadcast that a SdStyleSheet has changed, taking into account outline sublevels
//which need to be explicitly broadcast as changing if their parent style was
//the one that changed
void SdStyleSheet::BroadcastSdStyleSheetChange(SfxStyleSheetBase const * pStyleSheet,
    PresentationObjects ePO, SfxStyleSheetBasePool* pSSPool)
{
    SdStyleSheet* pRealSheet = static_cast<SdStyleSheet const *>(pStyleSheet)->GetRealStyleSheet();
    pRealSheet->Broadcast(SfxHint(SfxHintId::DataChanged));

    if( (ePO < PresentationObjects::Outline_1) || (ePO > PresentationObjects::Outline_8) )
        return;

    OUString sStyleName(SdResId(STR_PSEUDOSHEET_OUTLINE) + " ");

    for( sal_uInt16 n = static_cast<sal_uInt16>(ePO) - static_cast<sal_uInt16>(PresentationObjects::Outline_1) + 2; n < 10; n++ )
    {
        OUString aName( sStyleName + OUString::number(n) );

        SfxStyleSheetBase* pSheet = pSSPool->Find( aName, SfxStyleFamily::Pseudo);

        if(pSheet)
        {
            SdStyleSheet* pRealStyleSheet = static_cast<SdStyleSheet*>(pSheet)->GetRealStyleSheet();
            pRealStyleSheet->Broadcast(SfxHint(SfxHintId::DataChanged));
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
