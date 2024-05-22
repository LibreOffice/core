/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <ooo/vba/word/WdColor.hpp>
#include <ooo/vba/word/WdCalendarType.hpp>
#include <ooo/vba/word/WdContentControlType.hpp>
#include <ooo/vba/word/WdLanguageID.hpp>

#include <sal/log.hxx>

#include <ndtxt.hxx>
#include <unotextrange.hxx>

#include "vbacontentcontrol.hxx"
#include "vbacontentcontrollistentries.hxx"
#include "vbarange.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

/**
 * Content controls are the modern version of FormFields, providing inline functionality similar
 * to that of ActiveX form controls. Individual content controls may contain contents
 * such as dates, lists, or paragraphs of formatted text.
 *
 * Not all functions are applicable to each type of control, so use getType verification liberally.
 */
SwVbaContentControl::SwVbaContentControl(const uno::Reference<XHelperInterface>& rParent,
                                         const uno::Reference<uno::XComponentContext>& rContext,
                                         const uno::Reference<text::XTextDocument>& xTextDocument,
                                         std::shared_ptr<SwContentControl> pContentControl)
    : SwVbaContentControl_BASE(rParent, rContext)
    , mxTextDocument(xTextDocument)
    , m_pCC(std::move(pContentControl))
{
    assert(m_pCC && "SwVbaContentControl created without a shared_ptr. Why would you do that?");
}

SwVbaContentControl::~SwVbaContentControl() {}

sal_Bool SwVbaContentControl::getAllowInsertDeleteSection()
{
    SAL_INFO("sw.vba", "SwVbaContentControl::getAllowInsertDeleteSection stub");
    return false;
}

void SwVbaContentControl::setAllowInsertDeleteSection(sal_Bool /*bSet*/)
{
    SAL_INFO("sw.vba", "SwVbaContentControl::setAllowInsertDeleteSection stub");
}

sal_Int32 SwVbaContentControl::getAppearance()
{
    SAL_INFO("sw.vba", "SwVbaContentControl::getAppearance stub");
    // wdContentControlBoundingBox / wdContentControlHidden / wdContentControlTags
    return 0;
}

void SwVbaContentControl::setAppearance(sal_Int32 nSet)
{
    SAL_INFO("sw.vba", "SwVbaContentControl::setAppearance[" << nSet << "] stub");
}

OUString SwVbaContentControl::getBuildingBlockCategory()
{
    SAL_INFO("sw.vba", "SwVbaContentControl::getBuildingBlockCategory stub");
    return OUString();
}

void SwVbaContentControl::setBuildingBlockCategory(const OUString& sSet)
{
    SAL_INFO("sw.vba", "SwVbaContentControl::setBuildingBlockCategory[" << sSet << "] stub");
}

sal_Int32 SwVbaContentControl::getBuildingBlockType()
{
    SAL_INFO("sw.vba", "SwVbaContentControl::getBuildingBlockType stub");
    // returns a WdBuildingBlockTypes that represents the type of building block
    return 0;
}

void SwVbaContentControl::setBuildingBlockType(sal_Int32 nSet)
{
    SAL_INFO("sw.vba", "SwVbaContentControl::setBuildingBlockType[" << nSet << "] stub");
}

sal_Bool SwVbaContentControl::getChecked() { return m_pCC->GetCheckbox() && m_pCC->GetChecked(); }

void SwVbaContentControl::setChecked(sal_Bool bSet)
{
    // Word 2010: if locked, then the checked status is changed, but not the underlying text.
    // Do we really want to do that? That is pretty bizarre behaviour...
    // For now, just implement what seems to be a more logical response.
    // TODO: test with modern versions.
    if (getLockContents())
        return;

    if (m_pCC->GetCheckbox() && m_pCC->GetChecked() != static_cast<bool>(bSet))
    {
        m_pCC->SetChecked(bSet);
        m_pCC->SetShowingPlaceHolder(false);
        if (m_pCC->GetTextAttr())
            m_pCC->GetTextAttr()->Invalidate();
    }
}

sal_Int32 SwVbaContentControl::getColor()
{
    //This is just an assumed implementation - I have no testing environment to confirm.
    OUString sColor = m_pCC->GetColor();
    if (sColor == "wdColorAutomatic")
        return word::WdColor::wdColorAutomatic;
    if (sColor == "wdColorBlack")
        return word::WdColor::wdColorBlack;
    if (sColor == "wdColorBlue")
        return word::WdColor::wdColorBlue;
    if (sColor == "wdColorBlueGray")
        return word::WdColor::wdColorBlueGray;
    if (sColor == "wdColorBrightGreen")
        return word::WdColor::wdColorBrightGreen;
    if (sColor == "wdColorBrown")
        return word::WdColor::wdColorBrown;
    if (sColor == "wdColorDarkBlue")
        return word::WdColor::wdColorDarkBlue;
    if (sColor == "wdColorDarkGreen")
        return word::WdColor::wdColorDarkGreen;
    if (sColor == "wdColorDarkRed")
        return word::WdColor::wdColorDarkRed;
    if (sColor == "wdColorDarkTeal")
        return word::WdColor::wdColorDarkTeal;
    if (sColor == "wdColorDarkYellow")
        return word::WdColor::wdColorDarkYellow;
    if (sColor == "wdColorGold")
        return word::WdColor::wdColorGold;
    if (sColor == "wdColorGray05")
        return word::WdColor::wdColorGray05;
    if (sColor == "wdColorGray10")
        return word::WdColor::wdColorGray10;
    if (sColor == "wdColorGray125")
        return word::WdColor::wdColorGray125;
    if (sColor == "wdColorGray15")
        return word::WdColor::wdColorGray15;
    if (sColor == "wdColorGray20")
        return word::WdColor::wdColorGray20;
    if (sColor == "wdColorGray25")
        return word::WdColor::wdColorGray25;
    if (sColor == "wdColorGray30")
        return word::WdColor::wdColorGray30;
    if (sColor == "wdColorGray35")
        return word::WdColor::wdColorGray35;
    if (sColor == "wdColorGray375")
        return word::WdColor::wdColorGray375;
    if (sColor == "wdColorGray40")
        return word::WdColor::wdColorGray40;
    if (sColor == "wdColorGray45")
        return word::WdColor::wdColorGray45;
    if (sColor == "wdColorGray50")
        return word::WdColor::wdColorGray50;
    if (sColor == "wdColorGray55")
        return word::WdColor::wdColorGray55;
    if (sColor == "wdColorGray60")
        return word::WdColor::wdColorGray60;
    if (sColor == "wdColorGray625")
        return word::WdColor::wdColorGray625;
    if (sColor == "wdColorGray65")
        return word::WdColor::wdColorGray65;
    if (sColor == "wdColorGray70")
        return word::WdColor::wdColorGray70;
    if (sColor == "wdColorGray75")
        return word::WdColor::wdColorGray75;
    if (sColor == "wdColorGray80")
        return word::WdColor::wdColorGray80;
    if (sColor == "wdColorGray85")
        return word::WdColor::wdColorGray85;
    if (sColor == "wdColorGray875")
        return word::WdColor::wdColorGray875;
    if (sColor == "wdColorGray90")
        return word::WdColor::wdColorGray90;
    if (sColor == "wdColorGray95")
        return word::WdColor::wdColorGray95;
    if (sColor == "wdColorGreen")
        return word::WdColor::wdColorGreen;
    if (sColor == "wdColorIndigo")
        return word::WdColor::wdColorIndigo;
    if (sColor == "wdColorLavender")
        return word::WdColor::wdColorLavender;
    if (sColor == "wdColorLightBlue")
        return word::WdColor::wdColorLightBlue;
    if (sColor == "wdColorLightGreen")
        return word::WdColor::wdColorLightGreen;
    if (sColor == "wdColorLightOrange")
        return word::WdColor::wdColorLightOrange;
    if (sColor == "wdColorLightTurquoise")
        return word::WdColor::wdColorLightTurquoise;
    if (sColor == "wdColorLightYellow")
        return word::WdColor::wdColorLightYellow;
    if (sColor == "wdColorLime")
        return word::WdColor::wdColorLime;
    if (sColor == "wdColorOliveGreen")
        return word::WdColor::wdColorOliveGreen;
    if (sColor == "wdColorOrange")
        return word::WdColor::wdColorOrange;
    if (sColor == "wdColorPaleBlue")
        return word::WdColor::wdColorPaleBlue;
    if (sColor == "wdColorPink")
        return word::WdColor::wdColorPink;
    if (sColor == "wdColorPlum")
        return word::WdColor::wdColorPlum;
    if (sColor == "wdColorRed")
        return word::WdColor::wdColorRed;
    if (sColor == "wdColorRose")
        return word::WdColor::wdColorRose;
    if (sColor == "wdColorSeaGreen")
        return word::WdColor::wdColorSeaGreen;
    if (sColor == "wdColorSkyBlue")
        return word::WdColor::wdColorSkyBlue;
    if (sColor == "wdColorTan")
        return word::WdColor::wdColorTan;
    if (sColor == "wdColorTeal")
        return word::WdColor::wdColorTeal;
    if (sColor == "wdColorTurquoise")
        return word::WdColor::wdColorTurquoise;
    if (sColor == "wdColorViolet")
        return word::WdColor::wdColorViolet;
    if (sColor == "wdColorWhite")
        return word::WdColor::wdColorWhite;
    if (sColor == "wdColorYellow")
        return word::WdColor::wdColorYellow;

    return word::WdColor::wdColorBlack;
}

void SwVbaContentControl::setColor(sal_Int32 nWdColor)
{
    switch (nWdColor)
    {
        case word::WdColor::wdColorAqua:
            m_pCC->SetColor(u"wdColorAqua"_ustr);
            break;
        case word::WdColor::wdColorAutomatic:
            m_pCC->SetColor(u"wdColorAutomatic"_ustr);
            break;
        case word::WdColor::wdColorBlack:
            m_pCC->SetColor(u"wdColorBlack"_ustr);
            break;
        case word::WdColor::wdColorBlue:
            m_pCC->SetColor(u"wdColorBlue"_ustr);
            break;
        case word::WdColor::wdColorBlueGray:
            m_pCC->SetColor(u"wdColorBlueGray"_ustr);
            break;
        case word::WdColor::wdColorBrightGreen:
            m_pCC->SetColor(u"wdColorBrightGreen"_ustr);
            break;
        case word::WdColor::wdColorBrown:
            m_pCC->SetColor(u"wdColorBrown"_ustr);
            break;
        case word::WdColor::wdColorDarkBlue:
            m_pCC->SetColor(u"wdColorDarkBlue"_ustr);
            break;
        case word::WdColor::wdColorDarkGreen:
            m_pCC->SetColor(u"wdColorDarkGreen"_ustr);
            break;
        case word::WdColor::wdColorDarkRed:
            m_pCC->SetColor(u"wdColorDarkRed"_ustr);
            break;
        case word::WdColor::wdColorDarkTeal:
            m_pCC->SetColor(u"wdColorDarkTeal"_ustr);
            break;
        case word::WdColor::wdColorDarkYellow:
            m_pCC->SetColor(u"wdColorDarkYellow"_ustr);
            break;
        case word::WdColor::wdColorGold:
            m_pCC->SetColor(u"wdColorGold"_ustr);
            break;
        case word::WdColor::wdColorGray05:
            m_pCC->SetColor(u"wdColorGray05"_ustr);
            break;
        case word::WdColor::wdColorGray10:
            m_pCC->SetColor(u"wdColorGray10"_ustr);
            break;
        case word::WdColor::wdColorGray125:
            m_pCC->SetColor(u"wdColorGray125"_ustr);
            break;
        case word::WdColor::wdColorGray15:
            m_pCC->SetColor(u"wdColorGray15"_ustr);
            break;
        case word::WdColor::wdColorGray20:
            m_pCC->SetColor(u"wdColorGray20"_ustr);
            break;
        case word::WdColor::wdColorGray25:
            m_pCC->SetColor(u"wdColorGray25"_ustr);
            break;
        case word::WdColor::wdColorGray30:
            m_pCC->SetColor(u"wdColorGray30"_ustr);
            break;
        case word::WdColor::wdColorGray35:
            m_pCC->SetColor(u"wdColorGray35"_ustr);
            break;
        case word::WdColor::wdColorGray375:
            m_pCC->SetColor(u"wdColorGray375"_ustr);
            break;
        case word::WdColor::wdColorGray40:
            m_pCC->SetColor(u"wdColorGray40"_ustr);
            break;
        case word::WdColor::wdColorGray45:
            m_pCC->SetColor(u"wdColorGray45"_ustr);
            break;
        case word::WdColor::wdColorGray50:
            m_pCC->SetColor(u"wdColorGray50"_ustr);
            break;
        case word::WdColor::wdColorGray55:
            m_pCC->SetColor(u"wdColorGray55"_ustr);
            break;
        case word::WdColor::wdColorGray60:
            m_pCC->SetColor(u"wdColorGray60"_ustr);
            break;
        case word::WdColor::wdColorGray625:
            m_pCC->SetColor(u"wdColorGray625"_ustr);
            break;
        case word::WdColor::wdColorGray65:
            m_pCC->SetColor(u"wdColorGray65"_ustr);
            break;
        case word::WdColor::wdColorGray70:
            m_pCC->SetColor(u"wdColorGray70"_ustr);
            break;
        case word::WdColor::wdColorGray75:
            m_pCC->SetColor(u"wdColorGray75"_ustr);
            break;
        case word::WdColor::wdColorGray80:
            m_pCC->SetColor(u"wdColorGray80"_ustr);
            break;
        case word::WdColor::wdColorGray85:
            m_pCC->SetColor(u"wdColorGray85"_ustr);
            break;
        case word::WdColor::wdColorGray875:
            m_pCC->SetColor(u"wdColorGray875"_ustr);
            break;
        case word::WdColor::wdColorGray90:
            m_pCC->SetColor(u"wdColorGray90"_ustr);
            break;
        case word::WdColor::wdColorGray95:
            m_pCC->SetColor(u"wdColorGray95"_ustr);
            break;
        case word::WdColor::wdColorGreen:
            m_pCC->SetColor(u"wdColorGreen"_ustr);
            break;
        case word::WdColor::wdColorIndigo:
            m_pCC->SetColor(u"wdColorIndigo"_ustr);
            break;
        case word::WdColor::wdColorLavender:
            m_pCC->SetColor(u"wdColorLavender"_ustr);
            break;
        case word::WdColor::wdColorLightBlue:
            m_pCC->SetColor(u"wdColorLightBlue"_ustr);
            break;
        case word::WdColor::wdColorLightGreen:
            m_pCC->SetColor(u"wdColorLightGreen"_ustr);
            break;
        case word::WdColor::wdColorLightOrange:
            m_pCC->SetColor(u"wdColorLightOrange"_ustr);
            break;
        case word::WdColor::wdColorLightTurquoise:
            m_pCC->SetColor(u"wdColorLightTurquoise"_ustr);
            break;
        case word::WdColor::wdColorLightYellow:
            m_pCC->SetColor(u"wdColorLightYellow"_ustr);
            break;
        case word::WdColor::wdColorLime:
            m_pCC->SetColor(u"wdColorLime"_ustr);
            break;
        case word::WdColor::wdColorOliveGreen:
            m_pCC->SetColor(u"wdColorOliveGreen"_ustr);
            break;
        case word::WdColor::wdColorOrange:
            m_pCC->SetColor(u"wdColorOrange"_ustr);
            break;
        case word::WdColor::wdColorPaleBlue:
            m_pCC->SetColor(u"wdColorPaleBlue"_ustr);
            break;
        case word::WdColor::wdColorPink:
            m_pCC->SetColor(u"wdColorPink"_ustr);
            break;
        case word::WdColor::wdColorPlum:
            m_pCC->SetColor(u"wdColorPlum"_ustr);
            break;
        case word::WdColor::wdColorRed:
            m_pCC->SetColor(u"wdColorRed"_ustr);
            break;
        case word::WdColor::wdColorRose:
            m_pCC->SetColor(u"wdColorRose"_ustr);
            break;
        case word::WdColor::wdColorSeaGreen:
            m_pCC->SetColor(u"wdColorSeaGreen"_ustr);
            break;
        case word::WdColor::wdColorSkyBlue:
            m_pCC->SetColor(u"wdColorSkyBlue"_ustr);
            break;
        case word::WdColor::wdColorTan:
            m_pCC->SetColor(u"wdColorTan"_ustr);
            break;
        case word::WdColor::wdColorTeal:
            m_pCC->SetColor(u"wdColorTeal"_ustr);
            break;
        case word::WdColor::wdColorTurquoise:
            m_pCC->SetColor(u"wdColorTurquoise"_ustr);
            break;
        case word::WdColor::wdColorViolet:
            m_pCC->SetColor(u"wdColorViolet"_ustr);
            break;
        case word::WdColor::wdColorWhite:
            m_pCC->SetColor(u"wdColorWhite"_ustr);
            break;
        default:;
    }
}

sal_Int32 SwVbaContentControl::getDateCalendarType()
{
    SAL_INFO("sw.vba", "SwVbaContentControl::getDateCalendarType stub");
    // returns a WdCalendarTypes that represents the type of building block
    return word::WdCalendarType::wdCalendarWestern;
}

void SwVbaContentControl::setDateCalendarType(sal_Int32 nSet)
{
    SAL_INFO("sw.vba", "SwVbaContentControl::setDateCalendarType[" << nSet << "] stub");
}

OUString SwVbaContentControl::getDateDisplayFormat() { return m_pCC->GetDateFormat(); }

void SwVbaContentControl::setDateDisplayFormat(const OUString& sSet) { m_pCC->SetDateFormat(sSet); }

sal_Int32 SwVbaContentControl::getDateStorageFormat()
{
    SAL_INFO("sw.vba", "SwVbaContentControl::getDateStorageFormat stub");
    // returns a WdContentControlDateStorageFormat when bound to the XML data store.
    return 0;
}

void SwVbaContentControl::setDateStorageFormat(sal_Int32 nSet)
{
    SAL_INFO("sw.vba", "SwVbaContentControl::setDateStorageFormat[" << nSet << "] stub");
}

sal_Int32 SwVbaContentControl::getDateDisplayLocale()
{
    SAL_INFO("sw.vba", "SwVbaContentControl::getDateDisplayLocale stub");
    // returns a WdLanguageID that represents the language format for a date content control.
    return word::WdLanguageID::wdEnglishUS;
}

uno::Any SwVbaContentControl::getDropdownListEntries()
{
    if (!m_pCC->GetDropDown() && !m_pCC->GetComboBox())
        return uno::Any();

    return uno::Any(
        uno::Reference<XCollection>(new SwVbaContentControlListEntries(this, mxContext, m_pCC)));
}

OUString SwVbaContentControl::getID()
{
    // This signed integer is treated in VBA as if it was an unsigned int.
    return OUString::number(static_cast<sal_uInt32>(m_pCC->GetId()));
}

sal_Int32 SwVbaContentControl::getLevel()
{
    SAL_INFO("sw.vba", "SwVbaContentControl::getLevel stub");
    // returns a WdContentControlLevel
    return 0;
}

sal_Bool SwVbaContentControl::getLockContentControl()
{
    std::optional<bool> oLock = m_pCC->GetLock(/*bControl=*/true);
    return oLock.has_value() && *oLock;
}

void SwVbaContentControl::setLockContentControl(sal_Bool bSet)
{
    std::optional<bool> oLock = m_pCC->GetLock(/*bControl=*/false);
    m_pCC->SetLock(/*bContents=*/oLock.has_value() && *oLock, /*bControl=*/bSet);
}

sal_Bool SwVbaContentControl::getLockContents()
{
    // If the theoretical design model says it is locked, then report as locked.
    std::optional<bool> oLock = m_pCC->GetLock(/*bControl=*/false);
    if (oLock.has_value() && *oLock)
        return true;

    // Now check the real implementation.
    // Checkbox/DropDown/Picture are normally locked - but not in this sense. Report as unlocked.
    if (m_pCC->GetType() == SwContentControlType::CHECKBOX
        || m_pCC->GetType() == SwContentControlType::DROP_DOWN_LIST
        || m_pCC->GetType() == SwContentControlType::PICTURE)
    {
        return false;
    }

    return m_pCC->GetReadWrite();
}

void SwVbaContentControl::setLockContents(sal_Bool bSet)
{
    // Set the lock both theoretically and actually.
    std::optional<bool> oLock = m_pCC->GetLock(/*bControl=*/true);
    m_pCC->SetLock(/*bContents=*/bSet, /*bControl=*/oLock.has_value() && *oLock);

    // Checkbox/DropDown/Picture are normally locked in LO implementation - don't unlock them.
    if (m_pCC->GetType() == SwContentControlType::CHECKBOX
        || m_pCC->GetType() == SwContentControlType::DROP_DOWN_LIST
        || m_pCC->GetType() == SwContentControlType::PICTURE)
    {
        return;
    }
    m_pCC->SetReadWrite(bSet);
}

sal_Bool SwVbaContentControl::getMultiLine()
{
    SAL_INFO("sw.vba", "SwVbaContentControl::getMultiLine stub");
    return false;
}

void SwVbaContentControl::setMultiLine(sal_Bool /*bSet*/)
{
    SAL_INFO("sw.vba", "SwVbaContentControl::setMultiLine stub");
}

OUString SwVbaContentControl::getPlaceholderText()
{
    // return m_pCC->GetPlaceholderDocPart(); // This is not correct. Much more complex than this...
    SAL_INFO("sw.vba", "SwVbaContentControl::getPlaceholderText stub");
    return OUString();
}

sal_Bool SwVbaContentControl::getShowingPlaceholderText() { return m_pCC->GetShowingPlaceHolder(); }

uno::Reference<word::XRange> SwVbaContentControl::getRange()
{
    uno::Reference<word::XRange> xRet;
    SwTextNode* pTextNode = m_pCC->GetTextNode();
    if (pTextNode && m_pCC->GetTextAttr())
    {
        // Don't select the text attribute itself at the start.
        SwPosition aStart(*pTextNode, m_pCC->GetTextAttr()->GetStart() + 1);
        // Don't select the CH_TXTATR_BREAKWORD itself at the end.
        SwPosition aEnd(*pTextNode, *m_pCC->GetTextAttr()->End() - 1);
        uno::Reference<text::XTextRange> xText(
            SwXTextRange::CreateXTextRange(pTextNode->GetDoc(), aStart, &aEnd));
        if (xText.is())
            xRet = new SwVbaRange(mxParent, mxContext, mxTextDocument, xText->getStart(),
                                  xText->getEnd());
    }
    return xRet;
}

OUString SwVbaContentControl::getRepeatingSectionItemTitle()
{
    SAL_INFO("sw.vba", "SwVbaContentControl::getRepeatingSectionItemTitle stub");
    return OUString();
}

void SwVbaContentControl::setRepeatingSectionItemTitle(const OUString& rSet)
{
    SAL_INFO("sw.vba", "SwVbaContentControl::setRepeatingSectionItemTitle[" << rSet << "] stub");
}

OUString SwVbaContentControl::getTag() { return m_pCC->GetTag(); }

void SwVbaContentControl::setTag(const OUString& rSet) { return m_pCC->SetTag(rSet); }

sal_Bool SwVbaContentControl::getTemporary()
{
    SAL_INFO("sw.vba", "SwVbaContentControl::getTemporary stub");
    // Is content control removed when user edits (one time use)? Not implemented in LO.
    return false;
}

void SwVbaContentControl::setTemporary(sal_Bool /*bSet*/)
{
    SAL_INFO("sw.vba", "SwVbaContentControl::setTemporary stub");
}

OUString SwVbaContentControl::getTitle() { return m_pCC->GetAlias(); }

void SwVbaContentControl::setTitle(const OUString& rSet) { return m_pCC->SetAlias(rSet); }

sal_Int32 SwVbaContentControl::getType()
{
    SwContentControlType eType = m_pCC->GetType();
    sal_Int32 eVbaType = word::WdContentControlType::wdContentControlRichText;

    switch (eType)
    {
        case SwContentControlType::CHECKBOX:
            eVbaType = word::WdContentControlType::wdContentControlCheckbox;
            break;
        case SwContentControlType::DROP_DOWN_LIST:
            eVbaType = word::WdContentControlType::wdContentControlDropdownList;
            break;
        case SwContentControlType::PICTURE:
            eVbaType = word::WdContentControlType::wdContentControlPicture;
            break;
        case SwContentControlType::DATE:
            eVbaType = word::WdContentControlType::wdContentControlDate;
            break;
        case SwContentControlType::PLAIN_TEXT:
            eVbaType = word::WdContentControlType::wdContentControlText;
            break;
        case SwContentControlType::COMBO_BOX:
            eVbaType = word::WdContentControlType::wdContentControlComboBox;
            break;
        case SwContentControlType::RICH_TEXT:
        default:;
    }
    return eVbaType;
}

void SwVbaContentControl::setType(sal_Int32 nSet)
{
    SAL_INFO("sw.vba", "SwVbaContentControl::setType[" << nSet << "] stub");
    //     SwContentControlType eType = SwContentControlType::RICH_TEXT;
    //     switch(nSet)
    //     {
    //         case word::WdContentControlType::wdContentControlCheckbox:
    //             eType = SwContentControlType::CHECKBOX;
    //             break;
    //         case word::WdContentControlType::wdContentControlDropdownList:
    //             eType = SwContentControlType::DROP_DOWN_LIST;
    //             break;
    //         case word::WdContentControlType::wdContentControlPicture:
    //             eType = SwContentControlType::PICTURE;
    //             break;
    //         case word::WdContentControlType::wdContentControlDate:
    //             eType = SwContentControlType::DATE;
    //             break;
    //         case word::WdContentControlType::wdContentControlText:
    //             eType = SwContentControlType::PLAIN_TEXT;
    //             break;
    //         case word::WdContentControlType::wdContentControlComboBox:
    //             eType = SwContentControlType::COMBO_BOX;
    //             break;
    //         case word::WdContentControlType::wdContentControlRichText:
    //         default:;
    //     }
    //     m_pCC->SetType(eType);
}

void SwVbaContentControl::Copy()
{
    SAL_INFO("sw.vba", "SwVbaContentControl::Copy[" << getID() << "] stub");
}

void SwVbaContentControl::Cut()
{
    if (getLockContentControl() || !m_pCC->GetTextAttr())
        return;

    SAL_INFO("sw.vba",
             "SwVbaContentControl::Cut[" << getID() << "], but missing sending to clipboard");

    m_pCC->GetTextAttr()->Delete(/*bSaveContents=*/getLockContents());
}

void SwVbaContentControl::Delete(const uno::Any& DeleteContents)
{
    if (getLockContentControl() || !m_pCC->GetTextAttr())
        return;

    bool bDeleteContents = false;
    DeleteContents >>= bDeleteContents;

    m_pCC->GetTextAttr()->Delete(/*bSaveContents=*/!bDeleteContents || getLockContents());
}

void SwVbaContentControl::SetCheckedSymbol(sal_Int32 Character, const uno::Any& Font)
{
    if (!m_pCC->GetTextAttr())
        return;

    SAL_INFO_IF(Font.hasValue(), "sw.vba", "SetCheckedSymbol Font[" << Font << "] stub");
    if (Character < 31 || Character > SAL_MAX_UINT16)
        return; // unsupported character. Would such a thing exist in VBA?

    m_pCC->SetCheckedState(OUString(static_cast<sal_Unicode>(Character)));

    if (m_pCC->GetCheckbox() && m_pCC->GetChecked() && !m_pCC->GetShowingPlaceHolder())
        m_pCC->GetTextAttr()->Invalidate();
}

void SwVbaContentControl::SetUnCheckedSymbol(sal_Int32 Character, const uno::Any& Font)
{
    if (!m_pCC->GetTextAttr())
        return;

    SAL_INFO_IF(Font.hasValue(), "sw.vba", "SetUnCheckedSymbol Font[" << Font << "] stub");
    if (Character < 31 || Character > SAL_MAX_UINT16)
        return; // unsupported character. Would such a thing exist in VBA?

    m_pCC->SetUncheckedState(OUString(static_cast<sal_Unicode>(Character)));

    if (m_pCC->GetCheckbox() && !m_pCC->GetChecked() && !m_pCC->GetShowingPlaceHolder())
        m_pCC->GetTextAttr()->Invalidate();
}

void SwVbaContentControl::SetPlaceholderText(const uno::Any& BuildingBlock, const uno::Any& Range,
                                             const uno::Any& Text)
{
    SAL_INFO("sw.vba", "SwVbaContentControl::SetPlaceholderText stub");
    if (BuildingBlock.hasValue())
    {
        // Set placeholder text to the building block - whatever that is.
    }
    else if (Range.hasValue())
    {
        // Set placeholder text to the contents of the Range, however you do that.
    }
    else if (Text.hasValue())
    {
        // Set placeholder text to the provided string
    }
    else
    {
        // Remove placeholder text.
        m_pCC->SetPlaceholderDocPart(u""_ustr);
    }
    if (m_pCC->GetShowingPlaceHolder() && !getLockContents() && m_pCC->GetTextAttr())
    {
        //replace the text and ensure showing placeholder is still set
    }
}

void SwVbaContentControl::Ungroup() { SAL_INFO("sw.vba", "SwVbaContentControl::UnGroup stub"); }

OUString SwVbaContentControl::getServiceImplName() { return u"SwVbaContentControl"_ustr; }

uno::Sequence<OUString> SwVbaContentControl::getServiceNames()
{
    static uno::Sequence<OUString> const aServiceNames{ u"ooo.vba.word.ContentControl"_ustr };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
