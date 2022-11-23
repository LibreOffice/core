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

#include "vbacontentcontrol.hxx"
#include "vbacontentcontrollistentries.hxx"

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
                                         SwTextContentControl& rContentControl)
    : SwVbaContentControl_BASE(rParent, rContext)
    , mxTextDocument(xTextDocument)
    , m_rCC(rContentControl)
{
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

sal_Bool SwVbaContentControl::getChecked()
{
    const std::shared_ptr<SwContentControl>& pCC = m_rCC.GetContentControl().GetContentControl();
    return pCC->GetCheckbox() && pCC->GetChecked();
}

void SwVbaContentControl::setChecked(sal_Bool bSet)
{
    std::shared_ptr<SwContentControl> pCC = m_rCC.GetContentControl().GetContentControl();
    if (pCC->GetCheckbox() && pCC->GetChecked() != static_cast<bool>(bSet))
    {
        pCC->SetChecked(bSet);
        pCC->SetShowingPlaceHolder(false);
        m_rCC.Invalidate();
    }
}

sal_Int32 SwVbaContentControl::getColor()
{
    const std::shared_ptr<SwContentControl>& pCC = m_rCC.GetContentControl().GetContentControl();
    //This is just an assumed implementation - I have no testing environment to confirm.
    OUString sColor = pCC->GetColor();
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
    std::shared_ptr<SwContentControl> pCC = m_rCC.GetContentControl().GetContentControl();

    switch (nWdColor)
    {
        case word::WdColor::wdColorAqua:
            pCC->SetColor("wdColorAqua");
            break;
        case word::WdColor::wdColorAutomatic:
            pCC->SetColor("wdColorAutomatic");
            break;
        case word::WdColor::wdColorBlack:
            pCC->SetColor("wdColorBlack");
            break;
        case word::WdColor::wdColorBlue:
            pCC->SetColor("wdColorBlue");
            break;
        case word::WdColor::wdColorBlueGray:
            pCC->SetColor("wdColorBlueGray");
            break;
        case word::WdColor::wdColorBrightGreen:
            pCC->SetColor("wdColorBrightGreen");
            break;
        case word::WdColor::wdColorBrown:
            pCC->SetColor("wdColorBrown");
            break;
        case word::WdColor::wdColorDarkBlue:
            pCC->SetColor("wdColorDarkBlue");
            break;
        case word::WdColor::wdColorDarkGreen:
            pCC->SetColor("wdColorDarkGreen");
            break;
        case word::WdColor::wdColorDarkRed:
            pCC->SetColor("wdColorDarkRed");
            break;
        case word::WdColor::wdColorDarkTeal:
            pCC->SetColor("wdColorDarkTeal");
            break;
        case word::WdColor::wdColorDarkYellow:
            pCC->SetColor("wdColorDarkYellow");
            break;
        case word::WdColor::wdColorGold:
            pCC->SetColor("wdColorGold");
            break;
        case word::WdColor::wdColorGray05:
            pCC->SetColor("wdColorGray05");
            break;
        case word::WdColor::wdColorGray10:
            pCC->SetColor("wdColorGray10");
            break;
        case word::WdColor::wdColorGray125:
            pCC->SetColor("wdColorGray125");
            break;
        case word::WdColor::wdColorGray15:
            pCC->SetColor("wdColorGray15");
            break;
        case word::WdColor::wdColorGray20:
            pCC->SetColor("wdColorGray20");
            break;
        case word::WdColor::wdColorGray25:
            pCC->SetColor("wdColorGray25");
            break;
        case word::WdColor::wdColorGray30:
            pCC->SetColor("wdColorGray30");
            break;
        case word::WdColor::wdColorGray35:
            pCC->SetColor("wdColorGray35");
            break;
        case word::WdColor::wdColorGray375:
            pCC->SetColor("wdColorGray375");
            break;
        case word::WdColor::wdColorGray40:
            pCC->SetColor("wdColorGray40");
            break;
        case word::WdColor::wdColorGray45:
            pCC->SetColor("wdColorGray45");
            break;
        case word::WdColor::wdColorGray50:
            pCC->SetColor("wdColorGray50");
            break;
        case word::WdColor::wdColorGray55:
            pCC->SetColor("wdColorGray55");
            break;
        case word::WdColor::wdColorGray60:
            pCC->SetColor("wdColorGray60");
            break;
        case word::WdColor::wdColorGray625:
            pCC->SetColor("wdColorGray625");
            break;
        case word::WdColor::wdColorGray65:
            pCC->SetColor("wdColorGray65");
            break;
        case word::WdColor::wdColorGray70:
            pCC->SetColor("wdColorGray70");
            break;
        case word::WdColor::wdColorGray75:
            pCC->SetColor("wdColorGray75");
            break;
        case word::WdColor::wdColorGray80:
            pCC->SetColor("wdColorGray80");
            break;
        case word::WdColor::wdColorGray85:
            pCC->SetColor("wdColorGray85");
            break;
        case word::WdColor::wdColorGray875:
            pCC->SetColor("wdColorGray875");
            break;
        case word::WdColor::wdColorGray90:
            pCC->SetColor("wdColorGray90");
            break;
        case word::WdColor::wdColorGray95:
            pCC->SetColor("wdColorGray95");
            break;
        case word::WdColor::wdColorGreen:
            pCC->SetColor("wdColorGreen");
            break;
        case word::WdColor::wdColorIndigo:
            pCC->SetColor("wdColorIndigo");
            break;
        case word::WdColor::wdColorLavender:
            pCC->SetColor("wdColorLavender");
            break;
        case word::WdColor::wdColorLightBlue:
            pCC->SetColor("wdColorLightBlue");
            break;
        case word::WdColor::wdColorLightGreen:
            pCC->SetColor("wdColorLightGreen");
            break;
        case word::WdColor::wdColorLightOrange:
            pCC->SetColor("wdColorLightOrange");
            break;
        case word::WdColor::wdColorLightTurquoise:
            pCC->SetColor("wdColorLightTurquoise");
            break;
        case word::WdColor::wdColorLightYellow:
            pCC->SetColor("wdColorLightYellow");
            break;
        case word::WdColor::wdColorLime:
            pCC->SetColor("wdColorLime");
            break;
        case word::WdColor::wdColorOliveGreen:
            pCC->SetColor("wdColorOliveGreen");
            break;
        case word::WdColor::wdColorOrange:
            pCC->SetColor("wdColorOrange");
            break;
        case word::WdColor::wdColorPaleBlue:
            pCC->SetColor("wdColorPaleBlue");
            break;
        case word::WdColor::wdColorPink:
            pCC->SetColor("wdColorPink");
            break;
        case word::WdColor::wdColorPlum:
            pCC->SetColor("wdColorPlum");
            break;
        case word::WdColor::wdColorRed:
            pCC->SetColor("wdColorRed");
            break;
        case word::WdColor::wdColorRose:
            pCC->SetColor("wdColorRose");
            break;
        case word::WdColor::wdColorSeaGreen:
            pCC->SetColor("wdColorSeaGreen");
            break;
        case word::WdColor::wdColorSkyBlue:
            pCC->SetColor("wdColorSkyBlue");
            break;
        case word::WdColor::wdColorTan:
            pCC->SetColor("wdColorTan");
            break;
        case word::WdColor::wdColorTeal:
            pCC->SetColor("wdColorTeal");
            break;
        case word::WdColor::wdColorTurquoise:
            pCC->SetColor("wdColorTurquoise");
            break;
        case word::WdColor::wdColorViolet:
            pCC->SetColor("wdColorViolet");
            break;
        case word::WdColor::wdColorWhite:
            pCC->SetColor("wdColorWhite");
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

OUString SwVbaContentControl::getDateDisplayFormat()
{
    const std::shared_ptr<SwContentControl>& pCC = m_rCC.GetContentControl().GetContentControl();
    return pCC->GetDateFormat();
}

void SwVbaContentControl::setDateDisplayFormat(const OUString& sSet)
{
    std::shared_ptr<SwContentControl> pCC = m_rCC.GetContentControl().GetContentControl();
    pCC->SetDateFormat(sSet);
}

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
    std::shared_ptr<SwContentControl> pCC = m_rCC.GetContentControl().GetContentControl();
    if (!pCC->GetDropDown() && !pCC->GetComboBox())
        return uno::Any();

    return uno::Any(
        uno::Reference<XCollection>(new SwVbaContentControlListEntries(this, mxContext, m_rCC)));
}

OUString SwVbaContentControl::getID()
{
    //const std::shared_ptr<SwContentControl>& pCC = m_rCC.GetContentControl().GetContentControl();
    //return OUString::number(static_cast<sal_uInt32>(pCC->GetId()));
    return OUString();
}

sal_Int32 SwVbaContentControl::getLevel()
{
    SAL_INFO("sw.vba", "SwVbaContentControl::getLevel stub");
    // returns a WdContentControlLevel
    return 0;
}

sal_Bool SwVbaContentControl::getLockContentControl()
{
    SAL_INFO("sw.vba", "SwVbaContentControl::getLockContentControl stub");
    // returns whether the user can delete a content control from the active document.
    return true;
}

void SwVbaContentControl::setLockContentControl(sal_Bool /*bSet*/)
{
    SAL_INFO("sw.vba", "SwVbaContentControl::setLockContentControl stub");
}

sal_Bool SwVbaContentControl::getLockContents()
{
    // Pseudo-implementation - the need for locking in a form would be very rare.
    // LO uses this for internal purposes. Only expose it to VBA when safe.
    const std::shared_ptr<SwContentControl>& pCC = m_rCC.GetContentControl().GetContentControl();
    // Checkbox/DropDown/Picture are normally locked - but not in this sense. Report as unlocked.
    if (pCC->GetType() == SwContentControlType::CHECKBOX
        || pCC->GetType() == SwContentControlType::DROP_DOWN_LIST
        || pCC->GetType() == SwContentControlType::PICTURE)
    {
        return false;
    }

    return pCC->GetReadWrite();
}

void SwVbaContentControl::setLockContents(sal_Bool bSet)
{
    std::shared_ptr<SwContentControl> pCC = m_rCC.GetContentControl().GetContentControl();
    // Checkbox/DropDown/Picture are normally locked in LO implementation - don't unlock them.
    if (pCC->GetType() == SwContentControlType::CHECKBOX
        || pCC->GetType() == SwContentControlType::DROP_DOWN_LIST
        || pCC->GetType() == SwContentControlType::PICTURE)
    {
        return;
    }
    pCC->SetReadWrite(bSet);
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
    // return pCC->GetPlaceholderDocPart(); // This is not correct. Much more complex than this...
    SAL_INFO("sw.vba", "SwVbaContentControl::getPlaceholderText stub");
    return OUString();
}

sal_Bool SwVbaContentControl::getShowingPlaceholderText()
{
    const std::shared_ptr<SwContentControl>& pCC = m_rCC.GetContentControl().GetContentControl();
    return pCC->GetShowingPlaceHolder();
}

uno::Reference<word::XRange> SwVbaContentControl::getRange()
{
    SAL_INFO("sw.vba", "SwVbaContentControl::getRange stub");
    return uno::Reference<word::XRange>();
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

OUString SwVbaContentControl::getTag()
{
    const std::shared_ptr<SwContentControl>& pCC = m_rCC.GetContentControl().GetContentControl();
    return pCC->GetTag();
}

void SwVbaContentControl::setTag(const OUString& rSet)
{
    std::shared_ptr<SwContentControl> pCC = m_rCC.GetContentControl().GetContentControl();
    return pCC->SetTag(rSet);
}

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

OUString SwVbaContentControl::getTitle()
{
    const std::shared_ptr<SwContentControl>& pCC = m_rCC.GetContentControl().GetContentControl();
    return pCC->GetAlias();
}

void SwVbaContentControl::setTitle(const OUString& rSet)
{
    std::shared_ptr<SwContentControl> pCC = m_rCC.GetContentControl().GetContentControl();
    return pCC->SetAlias(rSet);
}

sal_Int32 SwVbaContentControl::getType()
{
    const std::shared_ptr<SwContentControl>& pCC = m_rCC.GetContentControl().GetContentControl();
    SwContentControlType eType = pCC->GetType();
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
    //     std::shared_ptr<SwContentControl> pCC = m_rCC.GetContentControl().GetContentControl();
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
    //     pCC->SetType(eType);
}

void SwVbaContentControl::Copy()
{
    SAL_INFO("sw.vba", "SwVbaContentControl::Copy[" << getID() << "] stub");
}

void SwVbaContentControl::Cut()
{
    SAL_INFO("sw.vba",
             "SwVbaContentControl::Cut[" << getID() << "], but missing sending to clipboard");

    m_rCC.Delete(/*bSaveContents=*/false);
}

void SwVbaContentControl::Delete(const uno::Any& DeleteContents)
{
    bool bDeleteContents = false;
    DeleteContents >>= bDeleteContents;
    m_rCC.Delete(!bDeleteContents);
}

void SwVbaContentControl::SetCheckedSymbol(sal_Int32 Character, const uno::Any& Font)
{
    SAL_INFO_IF(Font.hasValue(), "sw.vba", "SetCheckedSymbol Font[" << Font << "] stub");
    if (Character < 31 || Character > SAL_MAX_UINT16)
        return; // unsupported character. Would such a thing exist in VBA?

    std::shared_ptr<SwContentControl> pCC = m_rCC.GetContentControl().GetContentControl();
    pCC->SetCheckedState(OUString(static_cast<sal_Unicode>(Character)));

    if (pCC->GetCheckbox() && pCC->GetChecked())
        m_rCC.Invalidate();
}

void SwVbaContentControl::SetUnCheckedSymbol(sal_Int32 Character, const uno::Any& Font)
{
    SAL_INFO_IF(Font.hasValue(), "sw.vba", "SetUnCheckedSymbol Font[" << Font << "] stub");
    if (Character < 31 || Character > SAL_MAX_UINT16)
        return; // unsupported character. Would such a thing exist in VBA?

    std::shared_ptr<SwContentControl> pCC = m_rCC.GetContentControl().GetContentControl();
    pCC->SetUncheckedState(OUString(static_cast<sal_Unicode>(Character)));

    if (pCC->GetCheckbox() && !pCC->GetChecked())
        m_rCC.Invalidate();
}

void SwVbaContentControl::SetPlaceholderText(const uno::Any& BuildingBlock, const uno::Any& Range,
                                             const uno::Any& Text)
{
    SAL_INFO("sw.vba", "SwVbaContentControl::SetPlaceholderText stub");
    std::shared_ptr<SwContentControl> pCC = m_rCC.GetContentControl().GetContentControl();
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
        pCC->SetPlaceholderDocPart("");
    }
    m_rCC.Invalidate();
}

void SwVbaContentControl::Ungroup() { SAL_INFO("sw.vba", "SwVbaContentControl::UnGroup stub"); }

OUString SwVbaContentControl::getServiceImplName() { return "SwVbaContentControl"; }

uno::Sequence<OUString> SwVbaContentControl::getServiceNames()
{
    static uno::Sequence<OUString> const aServiceNames{ "ooo.vba.word.ContentControl" };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
