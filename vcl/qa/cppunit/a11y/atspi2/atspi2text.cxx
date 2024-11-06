/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/accessibility/AccessibleTextType.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <com/sun/star/accessibility/XAccessibleTextAttributes.hpp>
#include <com/sun/star/accessibility/XAccessibleTextMarkup.hpp>

#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/style/CaseMap.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/style/TabStop.hpp>
#include <com/sun/star/text/FontRelief.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/TextMarkupType.hpp>

#include <i18nlangtag/languagetag.hxx>
#include <tools/UnitConversion.hxx>
#include <rtl/character.hxx>

#include <test/a11y/AccessibilityTools.hxx>

#include "atspi2.hxx"
#include "atspiwrapper.hxx"

using namespace css;

namespace
{
/** @brief Helper class to check text attributes are properly exported to Atspi.
 *
 * This kind of duplicates most of the logic in atktextattributes.cxx, but if we want to check the
 * values are correct (which includes whether they are properly updated for example), we have to do
 * this, even though it means quite some processing for some of the attributes.
 * This has to be kept in sync with how atktextattributes.cxx exposes those attributes. */
class AttributesChecker
{
private:
    uno::Reference<accessibility::XAccessibleText> mxLOText;
    Atspi::Text mxAtspiText;

public:
    AttributesChecker(const uno::Reference<accessibility::XAccessibleText>& xLOText,
                      const Atspi::Text& xAtspiText)
        : mxLOText(xLOText)
        , mxAtspiText(xAtspiText)
    {
    }

private:
    // helper to validate a value represented as a single float in ATSPI
    static bool implCheckFloat(std::string_view atspiValue, float expected)
    {
        float f;
        char dummy;

        CPPUNIT_ASSERT_EQUAL(1, sscanf(atspiValue.data(), "%g%c", &f, &dummy));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, f, 1e-4);

        return true;
    }

    // helper to check simple mappings between LO and ATSPI
    template <typename T>
    static bool implCheckMapping(const T loValue, const std::string_view atspiValue,
                                 const std::unordered_map<T, std::string_view>& map,
                                 const bool retIfMissing = false)
    {
        const auto& iter = map.find(loValue);
        if (iter != map.end())
        {
            CPPUNIT_ASSERT_EQUAL(iter->second, atspiValue);
            return true;
        }
        return retIfMissing;
    }

    // checkers, see atktextattributes.cxx
    bool checkBoolean(std::string_view atspiValue, const beans::PropertyValue& property,
                      const uno::Sequence<beans::PropertyValue>&)
    {
        if (property.Value.get<bool>())
            CPPUNIT_ASSERT_EQUAL(std::string_view("true"), atspiValue);
        else
            CPPUNIT_ASSERT_EQUAL(std::string_view("false"), atspiValue);

        return true;
    }

    bool checkString(std::string_view atspiValue, const beans::PropertyValue& property,
                     const uno::Sequence<beans::PropertyValue>&)
    {
        CPPUNIT_ASSERT_EQUAL(property.Value.get<OUString>(), OUString::fromUtf8(atspiValue));
        return true;
    }

    bool checkFloat(std::string_view atspiValue, const beans::PropertyValue& property,
                    const uno::Sequence<beans::PropertyValue>&)
    {
        return implCheckFloat(atspiValue, property.Value.get<float>());
    }

    bool checkVariant(std::string_view atspiValue, const beans::PropertyValue& property,
                      const uno::Sequence<beans::PropertyValue>&)
    {
        if (property.Value.get<short>() == style::CaseMap::SMALLCAPS)
            CPPUNIT_ASSERT_EQUAL(std::string_view("small_caps"), atspiValue);
        else
            CPPUNIT_ASSERT_EQUAL(std::string_view("normal"), atspiValue);

        return true;
    }

    // See Scale2String
    bool checkScale(std::string_view atspiValue, const beans::PropertyValue& property,
                    const uno::Sequence<beans::PropertyValue>&)
    {
        double v;
        char dummy;

        CPPUNIT_ASSERT_EQUAL(1, sscanf(atspiValue.data(), "%lg%c", &v, &dummy));
        CPPUNIT_ASSERT_EQUAL(property.Value.get<sal_Int16>(), sal_Int16(v * 100));

        return true;
    }

    // see Escapement2VerticalAlign
    bool checkVerticalAlign(std::string_view atspiValue, const beans::PropertyValue& property,
                            const uno::Sequence<beans::PropertyValue>&)
    {
        const sal_Int16 n = property.Value.get<sal_Int16>();

        if (n == 0)
            CPPUNIT_ASSERT_EQUAL(std::string_view("baseline"), atspiValue);
        else if (n == -101)
            CPPUNIT_ASSERT_EQUAL(std::string_view("sub"), atspiValue);
        else if (n == 101)
            CPPUNIT_ASSERT_EQUAL(std::string_view("super"), atspiValue);
        else
        {
            int v;
            char dummy;
            CPPUNIT_ASSERT_EQUAL(1, sscanf(atspiValue.data(), "%d%%%c", &v, &dummy));
            CPPUNIT_ASSERT_EQUAL(int(n), v);
        }

        return true;
    }

    bool checkColor(std::string_view atspiValue, const beans::PropertyValue& property,
                    const uno::Sequence<beans::PropertyValue>&)
    {
        auto color = property.Value.get<sal_Int32>();

        if (color == -1) // automatic, use the component's color
        {
            uno::Reference<accessibility::XAccessibleComponent> xComponent(mxLOText,
                                                                           uno::UNO_QUERY);
            if (xComponent.is())
            {
                if (property.Name == u"CharBackColor")
                    color = xComponent->getBackground();
                else if (property.Name == u"CharColor")
                    color = xComponent->getForeground();
            }
        }

        if (color != -1)
        {
            unsigned int r, g, b;
            char dummy;

            CPPUNIT_ASSERT_EQUAL(3, sscanf(atspiValue.data(), "%u,%u,%u%c", &r, &g, &b, &dummy));
            CPPUNIT_ASSERT_EQUAL((color & 0xFFFFFF),
                                 (static_cast<sal_Int32>(r) << 16 | static_cast<sal_Int32>(g) << 8
                                  | static_cast<sal_Int32>(b)));
            return true;
        }

        return false;
    }

    // See LineSpacing2LineHeight
    bool checkLineHeight(std::string_view atspiValue, const beans::PropertyValue& property,
                         const uno::Sequence<beans::PropertyValue>&)
    {
        const auto lineSpacing = property.Value.get<style::LineSpacing>();
        char dummy;

        if (lineSpacing.Mode == style::LineSpacingMode::PROP)
        {
            int h;

            CPPUNIT_ASSERT_EQUAL(1, sscanf(atspiValue.data(), "%d%%%c", &h, &dummy));
            CPPUNIT_ASSERT_EQUAL(lineSpacing.Height, sal_Int16(h));
        }
        else if (lineSpacing.Mode == style::LineSpacingMode::FIX)
        {
            double pt;

            CPPUNIT_ASSERT_EQUAL(1, sscanf(atspiValue.data(), "%lgpt%c", &pt, &dummy));
            CPPUNIT_ASSERT_DOUBLES_EQUAL(convertMm100ToPoint<double>(lineSpacing.Height), pt, 1e-4);
            CPPUNIT_ASSERT_EQUAL(lineSpacing.Height, sal_Int16(convertPointToMm100(pt)));
        }
        else
            return false;

        return true;
    }

    bool checkStretch(std::string_view atspiValue, const beans::PropertyValue& property,
                      const uno::Sequence<beans::PropertyValue>&)
    {
        const auto n = property.Value.get<sal_Int16>();

        if (n < 0)
            CPPUNIT_ASSERT_EQUAL(std::string_view("condensed"), atspiValue);
        else if (n > 0)
            CPPUNIT_ASSERT_EQUAL(std::string_view("expanded"), atspiValue);
        else
            CPPUNIT_ASSERT_EQUAL(std::string_view("normal"), atspiValue);

        return true;
    }

    bool checkStyle(std::string_view atspiValue, const beans::PropertyValue& property,
                    const uno::Sequence<beans::PropertyValue>&)
    {
        return implCheckMapping(
            property.Value.get<awt::FontSlant>(), atspiValue,
            { { awt::FontSlant_NONE, std::string_view("normal") },
              { awt::FontSlant_OBLIQUE, std::string_view("oblique") },
              { awt::FontSlant_ITALIC, std::string_view("italic") },
              { awt::FontSlant_REVERSE_OBLIQUE, std::string_view("reverse oblique") },
              { awt::FontSlant_REVERSE_ITALIC, std::string_view("reverse italic") } });
    }

    bool checkJustification(std::string_view atspiValue, const beans::PropertyValue& property,
                            const uno::Sequence<beans::PropertyValue>&)
    {
        return implCheckMapping(static_cast<style::ParagraphAdjust>(property.Value.get<short>()),
                                atspiValue,
                                { { style::ParagraphAdjust_LEFT, std::string_view("left") },
                                  { style::ParagraphAdjust_RIGHT, std::string_view("right") },
                                  { style::ParagraphAdjust_BLOCK, std::string_view("fill") },
                                  { style::ParagraphAdjust_STRETCH, std::string_view("fill") },
                                  { style::ParagraphAdjust_CENTER, std::string_view("center") } });
    }

    bool checkShadow(std::string_view atspiValue, const beans::PropertyValue& property,
                     const uno::Sequence<beans::PropertyValue>&)
    {
        if (property.Value.get<bool>())
            CPPUNIT_ASSERT_EQUAL(std::string_view("black"), atspiValue);
        else
            CPPUNIT_ASSERT_EQUAL(std::string_view("none"), atspiValue);

        return true;
    }

    bool checkLanguage(std::string_view atspiValue, const beans::PropertyValue& property,
                       const uno::Sequence<beans::PropertyValue>&)
    {
        auto aLocale = property.Value.get<lang::Locale>();
        LanguageTag aLanguageTag(aLocale);

        CPPUNIT_ASSERT_EQUAL(OUString(aLanguageTag.getLanguage() + "-"
                                      + aLanguageTag.getCountry().toAsciiLowerCase()),
                             OUString::fromUtf8(atspiValue));

        return true;
    }

    bool checkTextRotation(std::string_view atspiValue, const beans::PropertyValue& property,
                           const uno::Sequence<beans::PropertyValue>&)
    {
        return implCheckFloat(atspiValue, property.Value.get<sal_Int16>() / 10.0f);
    }

    bool checkWeight(std::string_view atspiValue, const beans::PropertyValue& property,
                     const uno::Sequence<beans::PropertyValue>&)
    {
        return implCheckFloat(atspiValue, property.Value.get<float>() * 4);
    }

    bool checkCMMValue(std::string_view atspiValue, const beans::PropertyValue& property,
                       const uno::Sequence<beans::PropertyValue>&)
    {
        double v;
        char dummy;

        // CMM is 1/100th of a mm
        CPPUNIT_ASSERT_EQUAL(1, sscanf(atspiValue.data(), "%lgmm%c", &v, &dummy));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(property.Value.get<sal_Int32>() * 0.01, v, 1e-4);

        return true;
    }

    bool checkDirection(std::string_view atspiValue, const beans::PropertyValue& property,
                        const uno::Sequence<beans::PropertyValue>&)
    {
        return implCheckMapping(property.Value.get<sal_Int16>(), atspiValue,
                                { { text::WritingMode2::TB_LR, std::string_view("ltr") },
                                  { text::WritingMode2::LR_TB, std::string_view("ltr") },
                                  { text::WritingMode2::TB_RL, std::string_view("rtl") },
                                  { text::WritingMode2::RL_TB, std::string_view("rtl") },
                                  { text::WritingMode2::PAGE, std::string_view("none") } });
    }

    bool checkWritingMode(std::string_view atspiValue, const beans::PropertyValue& property,
                          const uno::Sequence<beans::PropertyValue>&)
    {
        return implCheckMapping(property.Value.get<sal_Int16>(), atspiValue,
                                { { text::WritingMode2::TB_LR, std::string_view("tb-lr") },
                                  { text::WritingMode2::LR_TB, std::string_view("lr-tb") },
                                  { text::WritingMode2::TB_RL, std::string_view("tb-rl") },
                                  { text::WritingMode2::RL_TB, std::string_view("rl-tb") },
                                  { text::WritingMode2::PAGE, std::string_view("none") } });
    }

    static const beans::PropertyValue*
    findProperty(const uno::Sequence<beans::PropertyValue>& properties, std::u16string_view name)
    {
        auto prop = std::find_if(properties.begin(), properties.end(),
                                 [name](auto& p) { return p.Name == name; });
        if (prop == properties.end())
            prop = nullptr;
        return prop;
    }

    // same as findProperty() above, but with a fast path is @p property is a match
    static const beans::PropertyValue*
    findProperty(const beans::PropertyValue* property,
                 const uno::Sequence<beans::PropertyValue>& properties, std::u16string_view name)
    {
        if (property->Name == name)
            return property;
        return findProperty(properties, name);
    }

    bool checkFontEffect(std::string_view atspiValue, const beans::PropertyValue& property,
                         const uno::Sequence<beans::PropertyValue>& loProperties)
    {
        if (auto charContoured = findProperty(&property, loProperties, u"CharContoured");
            charContoured && charContoured->Value.get<bool>())
        {
            CPPUNIT_ASSERT_EQUAL(std::string_view("outline"), atspiValue);
            return true;
        }

        if (auto charRelief = findProperty(&property, loProperties, u"CharRelief"))
        {
            return implCheckMapping(charRelief->Value.get<sal_Int16>(), atspiValue,
                                    { { text::FontRelief::NONE, std::string_view("none") },
                                      { text::FontRelief::EMBOSSED, std::string_view("emboss") },
                                      { text::FontRelief::ENGRAVED, std::string_view("engrave") } },
                                    true);
        }

        return false;
    }

    bool checkTextDecoration(std::string_view atspiValue, const beans::PropertyValue&,
                             const uno::Sequence<beans::PropertyValue>& loProperties)
    {
        if (atspiValue == "none")
        {
            if (auto prop = findProperty(loProperties, u"CharFlash"))
                CPPUNIT_ASSERT_EQUAL(false, prop->Value.get<bool>());
            if (auto prop = findProperty(loProperties, u"CharUnderline"))
                CPPUNIT_ASSERT_EQUAL(css::awt::FontUnderline::NONE, prop->Value.get<sal_Int16>());
            if (auto prop = findProperty(loProperties, u"CharStrikeout"))
                CPPUNIT_ASSERT(prop->Value.get<sal_Int16>() == css::awt::FontStrikeout::NONE
                               || prop->Value.get<sal_Int16>()
                                      == css::awt::FontStrikeout::DONTKNOW);
        }
        else
        {
            sal_Int32 nIndex = 0;
            const auto atspiValueString = OUString::fromUtf8(atspiValue);

            do
            {
                OUString atspiToken = atspiValueString.getToken(0, ' ', nIndex);
                const beans::PropertyValue* prop;

                if (atspiToken == "blink")
                {
                    CPPUNIT_ASSERT((prop = findProperty(loProperties, u"CharFlash")));
                    CPPUNIT_ASSERT_EQUAL(true, prop->Value.get<bool>());
                }
                else if (atspiToken == "underline")
                {
                    CPPUNIT_ASSERT((prop = findProperty(loProperties, u"CharUnderline")));
                    CPPUNIT_ASSERT(prop->Value.get<sal_Int16>() != css::awt::FontUnderline::NONE);
                }
                else if (atspiToken == "underline")
                {
                    CPPUNIT_ASSERT((prop = findProperty(loProperties, u"CharStrikeout")));
                    CPPUNIT_ASSERT(prop->Value.get<sal_Int16>() != css::awt::FontStrikeout::NONE);
                    CPPUNIT_ASSERT(prop->Value.get<sal_Int16>()
                                   != css::awt::FontStrikeout::DONTKNOW);
                }
                else
                {
                    CPPUNIT_ASSERT_MESSAGE(
                        OUString("Unknown text decoration \"" + atspiToken).toUtf8().getStr(),
                        false);
                }
            } while (nIndex > 0);
        }

        return true;
    }

    static bool implCheckTabStops(std::string_view atspiValue, const beans::PropertyValue& property,
                                  const bool defaultTabs)
    {
        uno::Sequence<style::TabStop> theTabStops;

        if (property.Value >>= theTabStops)
        {
            sal_Unicode lastFillChar = ' ';
            const char* p = atspiValue.data();

            for (const auto& rTabStop : theTabStops)
            {
                if ((style::TabAlign_DEFAULT == rTabStop.Alignment) != defaultTabs)
                    continue;

                const char* tab_align = "";
                switch (rTabStop.Alignment)
                {
                    case style::TabAlign_LEFT:
                        tab_align = "left ";
                        break;
                    case style::TabAlign_CENTER:
                        tab_align = "center ";
                        break;
                    case style::TabAlign_RIGHT:
                        tab_align = "right ";
                        break;
                    case style::TabAlign_DECIMAL:
                        tab_align = "decimal ";
                        break;
                    default:
                        break;
                }

                const char* lead_char = "";
                if (rTabStop.FillChar != lastFillChar)
                {
                    lastFillChar = rTabStop.FillChar;
                    switch (lastFillChar)
                    {
                        case ' ':
                            lead_char = "blank ";
                            break;

                        case '.':
                            lead_char = "dotted ";
                            break;

                        case '-':
                            lead_char = "dashed ";
                            break;

                        case '_':
                            lead_char = "lined ";
                            break;

                        default:
                            lead_char = "custom ";
                            break;
                    }
                }

                // check this matches "<lead_char><tab_align><position>mm"
                CPPUNIT_ASSERT_EQUAL(0, strncmp(p, lead_char, strlen(lead_char)));
                p += strlen(lead_char);
                CPPUNIT_ASSERT_EQUAL(0, strncmp(p, tab_align, strlen(tab_align)));
                p += strlen(tab_align);
                float atspiPosition;
                int nConsumed;
                CPPUNIT_ASSERT_EQUAL(1, sscanf(p, "%gmm%n", &atspiPosition, &nConsumed));
                CPPUNIT_ASSERT_DOUBLES_EQUAL(float(rTabStop.Position * 0.01f), atspiPosition, 1e-4);
                p += nConsumed;

                if (*p)
                {
                    CPPUNIT_ASSERT_EQUAL(' ', *p);
                    p++;
                }
            }

            // make sure there isn't garbage at the end
            CPPUNIT_ASSERT_EQUAL(char(0), *p);

            return true;
        }

        return false;
    }

    bool checkDefaultTabStops(std::string_view atspiValue, const beans::PropertyValue& property,
                              const uno::Sequence<beans::PropertyValue>&)
    {
        return implCheckTabStops(atspiValue, property, true);
    }

    bool checkTabStops(std::string_view atspiValue, const beans::PropertyValue& property,
                       const uno::Sequence<beans::PropertyValue>&)
    {
        return implCheckTabStops(atspiValue, property, false);
    }

public:
    // runner code
    bool check(const uno::Sequence<beans::PropertyValue>& xLOAttributeList,
               const std::unordered_map<std::string, std::string>& xAtspiAttributeList)
    {
        const struct
        {
            const char* loName;
            const char* atspiName;
            bool (AttributesChecker::*checkValue)(
                std::string_view atspiValue, const beans::PropertyValue& property,
                const uno::Sequence<beans::PropertyValue>& loAttributeList);
        } atspiMap[]
            = { //  LO name        AT-SPI name       check function
                { "CharBackColor", "bg-color", &AttributesChecker::checkColor },
                { "CharCaseMap", "variant", &AttributesChecker::checkVariant },
                { "CharColor", "fg-color", &AttributesChecker::checkColor },
                { "CharContoured", "font-effect", &AttributesChecker::checkFontEffect },
                { "CharEscapement", "vertical-align", &AttributesChecker::checkVerticalAlign },
                { "CharFlash", "text-decoration", &AttributesChecker::checkTextDecoration },
                { "CharFontName", "family-name", &AttributesChecker::checkString },
                { "CharHeight", "size", &AttributesChecker::checkFloat },
                { "CharHidden", "invisible", &AttributesChecker::checkBoolean },
                { "CharKerning", "stretch", &AttributesChecker::checkStretch },
                { "CharLocale", "language", &AttributesChecker::checkLanguage },
                { "CharPosture", "style", &AttributesChecker::checkStyle },
                { "CharRelief", "font-effect", &AttributesChecker::checkFontEffect },
                { "CharRotation", "text-rotation", &AttributesChecker::checkTextRotation },
                { "CharScaleWidth", "scale", &AttributesChecker::checkScale },
                { "CharShadowed", "text-shadow", &AttributesChecker::checkShadow },
                { "CharStrikeout", "text-decoration", &AttributesChecker::checkTextDecoration },
                { "CharUnderline", "text-decoration", &AttributesChecker::checkTextDecoration },
                { "CharWeight", "weight", &AttributesChecker::checkWeight },
                { "MMToPixelRatio", "mm-to-pixel-ratio", &AttributesChecker::checkFloat },
                { "ParaAdjust", "justification", &AttributesChecker::checkJustification },
                { "ParaBottomMargin", "pixels-below-lines", &AttributesChecker::checkCMMValue },
                { "ParaFirstLineIndent", "indent", &AttributesChecker::checkCMMValue },
                { "ParaLeftMargin", "left-margin", &AttributesChecker::checkCMMValue },
                { "ParaLineSpacing", "line-height", &AttributesChecker::checkLineHeight },
                { "ParaRightMargin", "right-margin", &AttributesChecker::checkCMMValue },
                { "ParaStyleName", "paragraph-style", &AttributesChecker::checkString },
                { "ParaTabStops", "tab-interval", &AttributesChecker::checkDefaultTabStops },
                { "ParaTabStops", "tab-stops", &AttributesChecker::checkTabStops },
                { "ParaTopMargin", "pixels-above-lines", &AttributesChecker::checkCMMValue },
                { "WritingMode", "direction", &AttributesChecker::checkDirection },
                { "WritingMode", "writing-mode", &AttributesChecker::checkWritingMode }
              };

        for (const auto& prop : xLOAttributeList)
        {
            std::cout << "found run attribute: " << prop.Name << "=" << prop.Value << std::endl;

            /* we need to loop on all entries because there might be more than one for a single
             * property */
            for (const auto& entry : atspiMap)
            {
                if (!prop.Name.equalsAscii(entry.loName))
                    continue;

                const auto atspiIter = xAtspiAttributeList.find(entry.atspiName);
                /* we use an empty value if there isn't one, which can happen if the value cannot
                 * be represented by Atspi, or if the actual LO value is also empty */
                std::string atspiValue;
                if (atspiIter != xAtspiAttributeList.end())
                    atspiValue = atspiIter->second;

                std::cout << "  matching atspi attribute is: " << entry.atspiName << "="
                          << atspiValue << std::endl;
                CPPUNIT_ASSERT(
                    std::invoke(entry.checkValue, this, atspiValue, prop, xLOAttributeList));
            }
        }

        return true;
    }
};
}

/* LO doesn't implement it itself, but ATK provides a fallback.  Add a test here merely for the
 * future when we have a direct AT-SPI implementation for e.g. GTK4.
 * Just like atk-adaptor, we compute the bounding box by combining extents for each character
 * in the range */
static awt::Rectangle getRangeBounds(const uno::Reference<accessibility::XAccessibleText>& xText,
                                     sal_Int32 startOffset, sal_Int32 endOffset)
{
    awt::Rectangle bounds;

    for (auto offset = startOffset; offset < endOffset; offset++)
    {
        const auto chBounds = xText->getCharacterBounds(offset);
        if (offset == 0)
            bounds = chBounds;
        else
        {
            const auto x = std::min(bounds.X, chBounds.X);
            const auto y = std::min(bounds.Y, chBounds.Y);
            bounds.Width = std::max(bounds.X + bounds.Width, chBounds.X + chBounds.Width) - x;
            bounds.Height = std::max(bounds.Y + bounds.Height, chBounds.Y + chBounds.Height) - y;
            bounds.X = x;
            bounds.Y = y;
        }
    }

    return bounds;
}

void Atspi2TestTree::compareTextObjects(
    const uno::Reference<accessibility::XAccessibleText>& xLOText, const Atspi::Text& pAtspiText)
{
    CPPUNIT_ASSERT_EQUAL(xLOText->getCharacterCount(), sal_Int32(pAtspiText.getCharacterCount()));
    CPPUNIT_ASSERT_EQUAL(xLOText->getCaretPosition(), sal_Int32(pAtspiText.getCaretOffset()));
    CPPUNIT_ASSERT_EQUAL(xLOText->getText(), OUString::fromUtf8(pAtspiText.getText(0, -1)));

    const auto characterCount = xLOText->getCharacterCount();
    auto offset = decltype(characterCount){ 0 };
    auto atspiPosition = Atspi::Point{ 0, 0 };

    AttributesChecker attributesChecker(xLOText, pAtspiText);

    auto xLOTextAttrs
        = uno::Reference<accessibility::XAccessibleTextAttributes>(xLOText, uno::UNO_QUERY);
    // default text attributes
    if (xLOTextAttrs.is())
    {
        const auto aAttributeList = xLOTextAttrs->getDefaultAttributes(uno::Sequence<OUString>());
        const auto atspiAttributeList = pAtspiText.getDefaultAttributes();

        attributesChecker.check(aAttributeList, atspiAttributeList);
    }

    if (characterCount > 0)
    {
        const auto atspiComponent = pAtspiText.queryComponent();
        atspiPosition = atspiComponent.getPosition(ATSPI_COORD_TYPE_WINDOW);
    }

    // text run attributes
    uno::Reference<accessibility::XAccessibleTextMarkup> xTextMarkup(xLOText, uno::UNO_QUERY);
    while (offset < characterCount)
    {
        // message for the assertions so we know where it comes from
        OString offsetMsg(OString::Concat("in ") + AccessibilityTools::debugString(xLOText).c_str()
                          + " at offset " + OString::number(offset));

        uno::Sequence<beans::PropertyValue> aAttributeList;

        if (xLOTextAttrs.is())
            aAttributeList = xLOTextAttrs->getRunAttributes(offset, uno::Sequence<OUString>());
        else
            aAttributeList = xLOText->getCharacterAttributes(offset, uno::Sequence<OUString>());

        int atspiStartOffset = 0, atspiEndOffset = 0;
        const auto atspiAttributeList
            = pAtspiText.getAttributeRun(offset, false, &atspiStartOffset, &atspiEndOffset);

        accessibility::TextSegment aTextSegment
            = xLOText->getTextAtIndex(offset, accessibility::AccessibleTextType::ATTRIBUTE_RUN);

        /* Handle misspelled text and tracked changes as atktext.cxx does as it affects the run
         * boundaries.  Also check the attributes are properly forwarded. */
        if (xTextMarkup.is())
        {
            const struct
            {
                sal_Int32 markupType;
                const char* atspiAttribute;
                const char* atspiValue;
            } aTextMarkupTypes[]
                = { { text::TextMarkupType::SPELLCHECK, "text-spelling", "misspelled" },
                    { text::TextMarkupType::TRACK_CHANGE_INSERTION, "text-tracked-change",
                      "insertion" },
                    { text::TextMarkupType::TRACK_CHANGE_DELETION, "text-tracked-change",
                      "deletion" },
                    { text::TextMarkupType::TRACK_CHANGE_FORMATCHANGE, "text-tracked-change",
                      "attribute-change" } };

            for (const auto& aTextMarkupType : aTextMarkupTypes)
            {
                const auto nTextMarkupCount
                    = xTextMarkup->getTextMarkupCount(aTextMarkupType.markupType);
                if (nTextMarkupCount <= 0)
                    continue;

                for (auto nTextMarkupIndex = decltype(nTextMarkupCount){ 0 };
                     nTextMarkupIndex < nTextMarkupCount; ++nTextMarkupIndex)
                {
                    const auto aMarkupTextSegment
                        = xTextMarkup->getTextMarkup(nTextMarkupIndex, aTextMarkupType.markupType);
                    if (aMarkupTextSegment.SegmentStart > offset)
                    {
                        aTextSegment.SegmentEnd
                            = ::std::min(aTextSegment.SegmentEnd, aMarkupTextSegment.SegmentStart);
                        break; // no further iteration.
                    }
                    else if (offset < aMarkupTextSegment.SegmentEnd)
                    {
                        // text markup at <offset>
                        aTextSegment.SegmentStart = ::std::max(aTextSegment.SegmentStart,
                                                               aMarkupTextSegment.SegmentStart);
                        aTextSegment.SegmentEnd
                            = ::std::min(aTextSegment.SegmentEnd, aMarkupTextSegment.SegmentEnd);
                        // check the attribute is set
                        const auto atspiIter
                            = atspiAttributeList.find(aTextMarkupType.atspiAttribute);
                        CPPUNIT_ASSERT_MESSAGE(offsetMsg.getStr(),
                                               atspiIter != atspiAttributeList.end());
                        CPPUNIT_ASSERT_EQUAL_MESSAGE(offsetMsg.getStr(),
                                                     std::string_view(aTextMarkupType.atspiValue),
                                                     std::string_view(atspiIter->second));
                        break; // no further iteration needed.
                    }
                    else
                    {
                        aTextSegment.SegmentStart
                            = ::std::max(aTextSegment.SegmentStart, aMarkupTextSegment.SegmentEnd);
                        // continue iteration.
                    }
                }
            }
        }

        CPPUNIT_ASSERT_EQUAL_MESSAGE(offsetMsg.getStr(), aTextSegment.SegmentStart,
                                     sal_Int32(atspiStartOffset));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(offsetMsg.getStr(), aTextSegment.SegmentEnd,
                                     sal_Int32(atspiEndOffset));

        attributesChecker.check(aAttributeList, atspiAttributeList);

        CPPUNIT_ASSERT_MESSAGE(offsetMsg.getStr(), aTextSegment.SegmentEnd > offset);
        offset = aTextSegment.SegmentEnd;
    }

    // loop over each character
    for (offset = 0; offset < characterCount;)
    {
        const auto aTextSegment
            = xLOText->getTextAtIndex(offset, accessibility::AccessibleTextType::CHARACTER);
        OString offsetMsg(OString::Concat("in ") + AccessibilityTools::debugString(xLOText).c_str()
                          + " at offset " + OString::number(offset));

        // getCharacterAtOffset()
        sal_Int32 nChOffset = 0;
        sal_Int32 cp = aTextSegment.SegmentText.iterateCodePoints(&nChOffset);
        /* do not check unpaired surrogates, because they are unlikely to make any sense and LO's
         * GTK VCL doesn't like them */
        if (!rtl::isSurrogate(cp))
            CPPUNIT_ASSERT_EQUAL_MESSAGE(offsetMsg.getStr(), cp,
                                         pAtspiText.getCharacterAtOffset(offset));

        // getTextAtOffset()
        const struct
        {
            sal_Int16 loTextType;
            AtspiTextBoundaryType atspiBoundaryType;
        } textTypeMap[] = {
            { accessibility::AccessibleTextType::CHARACTER, ATSPI_TEXT_BOUNDARY_CHAR },
            { accessibility::AccessibleTextType::WORD, ATSPI_TEXT_BOUNDARY_WORD_START },
            { accessibility::AccessibleTextType::SENTENCE, ATSPI_TEXT_BOUNDARY_SENTENCE_START },
            { accessibility::AccessibleTextType::LINE, ATSPI_TEXT_BOUNDARY_LINE_START },
        };
        for (const auto& pair : textTypeMap)
        {
            auto loTextSegment = xLOText->getTextAtIndex(offset, pair.loTextType);
            const auto atspiTextRange = pAtspiText.getTextAtOffset(offset, pair.atspiBoundaryType);

            // for WORD there's adjustments to be made, see atktext.cxx:adjust_boundaries()
            if (pair.loTextType == accessibility::AccessibleTextType::WORD
                && !loTextSegment.SegmentText.isEmpty())
            {
                // Determine the start index of the next segment
                const auto loTextSegmentBehind
                    = xLOText->getTextBehindIndex(loTextSegment.SegmentEnd, pair.loTextType);
                if (!loTextSegmentBehind.SegmentText.isEmpty())
                    loTextSegment.SegmentEnd = loTextSegmentBehind.SegmentStart;
                else
                    loTextSegment.SegmentEnd = xLOText->getCharacterCount();

                loTextSegment.SegmentText
                    = xLOText->getTextRange(loTextSegment.SegmentStart, loTextSegment.SegmentEnd);
            }

            OString boundaryMsg(offsetMsg + " with boundary type "
                                + Atspi::TextBoundaryType::getName(pair.atspiBoundaryType).c_str());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(boundaryMsg.getStr(), loTextSegment.SegmentText,
                                         OUString::fromUtf8(atspiTextRange.content));
            /* if the segment is empty, LO API gives -1 offsets, but maps to 0 for AT-SPI.  This is
             * fine, AT-SPI doesn't really say what the offsets should be when the text is empty */
            if (!loTextSegment.SegmentText.isEmpty())
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE(boundaryMsg.getStr(), loTextSegment.SegmentStart,
                                             sal_Int32(atspiTextRange.startOffset));
                CPPUNIT_ASSERT_EQUAL_MESSAGE(boundaryMsg.getStr(), loTextSegment.SegmentEnd,
                                             sal_Int32(atspiTextRange.endOffset));
            }
        }

        // character bounds
        const auto loRect = xLOText->getCharacterBounds(offset);
        auto atspiRect = pAtspiText.getCharacterExtents(offset, ATSPI_COORD_TYPE_WINDOW);
        atspiRect.x -= atspiPosition.x;
        atspiRect.y -= atspiPosition.y;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(offsetMsg.getStr(), loRect.Y, sal_Int32(atspiRect.y));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(offsetMsg.getStr(), loRect.Height,
                                     sal_Int32(atspiRect.height));
        /* for some reason getCharacterBounds() might return negative widths in some cases
         * (including a space at the end of a right-justified line), and ATK will then adjust
         * the X and width values to positive to workaround RTL issues (see
         * https://bugzilla.gnome.org/show_bug.cgi?id=102954), so we work around that */
        if (loRect.Width < 0)
        {
            /* ATK will make x += width; width *= -1, but we don't really want to depend on the
             * ATK behavior so we allow it to match as well */
            CPPUNIT_ASSERT_MESSAGE(offsetMsg.getStr(),
                                   loRect.X == sal_Int32(atspiRect.x)
                                       || loRect.X + loRect.Width == sal_Int32(atspiRect.x));
            CPPUNIT_ASSERT_MESSAGE(offsetMsg.getStr(),
                                   loRect.Width == sal_Int32(atspiRect.width)
                                       || -loRect.Width == sal_Int32(atspiRect.width));
        }
        else
        {
            // normal case
            CPPUNIT_ASSERT_EQUAL_MESSAGE(offsetMsg.getStr(), loRect.X, sal_Int32(atspiRect.x));
            CPPUNIT_ASSERT_EQUAL_MESSAGE(offsetMsg.getStr(), loRect.Width,
                                         sal_Int32(atspiRect.width));
        }

        // indexAtPoint()
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            offsetMsg.getStr(), xLOText->getIndexAtPoint(awt::Point(loRect.X, loRect.Y)),
            sal_Int32(pAtspiText.getOffsetAtPoint(
                atspiPosition.x + loRect.X, atspiPosition.y + loRect.Y, ATSPI_COORD_TYPE_WINDOW)));

        CPPUNIT_ASSERT_MESSAGE(offsetMsg.getStr(), aTextSegment.SegmentEnd > offset);
        offset = aTextSegment.SegmentEnd;
    }

    // getRangeExtents() -- ATK doesn't like empty ranges, so only test when not empty
    if (characterCount > 0)
    {
        const auto loRangeBounds = getRangeBounds(xLOText, 0, characterCount);
        const auto atspiRangeExtents
            = pAtspiText.getRangeExtents(0, characterCount, ATSPI_COORD_TYPE_WINDOW);
        CPPUNIT_ASSERT_EQUAL(loRangeBounds.X, sal_Int32(atspiRangeExtents.x - atspiPosition.x));
        CPPUNIT_ASSERT_EQUAL(loRangeBounds.Y, sal_Int32(atspiRangeExtents.y - atspiPosition.y));
        CPPUNIT_ASSERT_EQUAL(loRangeBounds.Width, sal_Int32(atspiRangeExtents.width));
        CPPUNIT_ASSERT_EQUAL(loRangeBounds.Height, sal_Int32(atspiRangeExtents.height));
    }

    // selection (LO only have one selection, so some of the API doesn't really make sense)
    CPPUNIT_ASSERT_EQUAL(xLOText->getSelectionEnd() != xLOText->getSelectionStart() ? 1 : 0,
                         pAtspiText.getNSelections());

    const auto atspiSelection = pAtspiText.getSelection(0);
    CPPUNIT_ASSERT_EQUAL(xLOText->getSelectionStart(), sal_Int32(atspiSelection.startOffset));
    CPPUNIT_ASSERT_EQUAL(xLOText->getSelectionEnd(), sal_Int32(atspiSelection.endOffset));

    /* We need to take extra care with setSelection() because it could result to scrolling, which
     * might result in node destruction, which can mess up the parent's children enumeration.
     * So we only test nodes that are neither the first nor last child in its parent, hoping that
     * means it won't require scrolling to show the end of the selection. */
    uno::Reference<accessibility::XAccessibleContext> xLOContext(xLOText, uno::UNO_QUERY_THROW);
    const auto nIndexInParent = xLOContext->getAccessibleIndexInParent();
    if (characterCount && nIndexInParent > 0
        && nIndexInParent + 1 < xLOContext->getAccessibleParent()
                                    ->getAccessibleContext()
                                    ->getAccessibleChildCount()
        && pAtspiText.setSelection(0, 0, characterCount))
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xLOText->getSelectionStart());
        CPPUNIT_ASSERT_EQUAL(characterCount, xLOText->getSelectionEnd());
        // try and restore previous selection, if any
        CPPUNIT_ASSERT(xLOText->setSelection(std::max(0, atspiSelection.startOffset),
                                             std::max(0, atspiSelection.endOffset)));
    }

    // scrollSubstringTo() is tested in the parent, because it might dispose ourselves otherwise.

    // TODO: more checks here...
}

#if HAVE_ATSPI2_SCROLL_TO
// like getFirstRelationTargetOfType() but for Atspi objects
static Atspi::Accessible
atspiGetFirstRelationTargetOfType(const Atspi::Accessible& pAtspiAccessible,
                                  const AtspiRelationType relationType)
{
    for (const auto& rel : pAtspiAccessible.getRelationSet())
    {
        if (rel.getRelationType() == relationType && rel.getNTargets() > 0)
            return rel.getTarget(0);
    }

    return nullptr;
}
#endif // HAVE_ATSPI2_SCROLL_TO

/**
 * @brief Gets the index of a Writer child hopping through flows-from relationships
 * @param xContext The accessible context to locate
 * @returns The index of @c xContext in the flows-from chain
 *
 * Gets the index of a child in its parent regardless of whether it is on screen or not.
 *
 * @warning This relying on the flows-from relationships, it only works for the connected nodes,
 *          and might not work for e.g. frames.
 */
sal_Int64 Atspi2TestTree::swChildIndex(uno::Reference<accessibility::XAccessibleContext> xContext)
{
    for (sal_Int64 n = 0;; n++)
    {
        auto xPrev = getFirstRelationTargetOfType(
            xContext, accessibility::AccessibleRelationType_CONTENT_FLOWS_FROM);
        if (!xPrev.is())
            return n;
        xContext = xPrev;
    }
}

/**
 * @brief tests scrolling in Writer.
 * @param xLOContext The @c XAccessibleContext for the writer document
 * @param xAtspiAccessible The AT-SPI2 equivalent of @c xLOContext.
 *
 * Test scrolling (currently XAccessibleText::scrollSubstringTo()) in Writer.
 */
void Atspi2TestTree::testSwScroll(
    const uno::Reference<accessibility::XAccessibleContext>& xLOContext,
    const Atspi::Accessible& xAtspiAccessible)
{
#if HAVE_ATSPI2_SCROLL_TO
    /* Currently LO only implements SCROLL_ANYWHERE, so to be sure we need to find something
     * offscreen and try and bring it in.  LO only has implementation for SwAccessibleParagraph,
     * so we find the last child, and then try and find a FLOWS_TO relationship -- that's a hack
     * based on how LO exposes offscreen children, e.g. not as "real" children.  Once done so, we
     * have to make sure the child is now on screen, so we should find it in the children list.  We
     * cannot rely on anything we had still being visible, as it could very well have scrolled it to
     * the top. */
    assert(accessibility::AccessibleRole::DOCUMENT_TEXT == xLOContext->getAccessibleRole());

    auto nLOChildCount = xLOContext->getAccessibleChildCount();
    if (nLOChildCount <= 0)
        return;

    // find the first off-screen text child
    auto xLONextContext = xLOContext->getAccessibleChild(nLOChildCount - 1)->getAccessibleContext();
    uno::Reference<accessibility::XAccessibleText> xLONextText;
    unsigned int nAfterLast = 0;
    do
    {
        xLONextContext = getFirstRelationTargetOfType(
            xLONextContext, accessibility::AccessibleRelationType_CONTENT_FLOWS_TO);
        xLONextText.set(xLONextContext, uno::UNO_QUERY);
        nAfterLast++;
    } while (xLONextContext.is() && !xLONextText.is());

    if (!xLONextText.is())
        return; // we have nothing off-screen to scroll to

    // get the global index of the off-screen child so we can match it later
    auto nLOChildIndex = swChildIndex(xLONextContext);

    // find the corresponding Atspi child to call the API on
    auto xAtspiNextChild = xAtspiAccessible.getChildAtIndex(nLOChildCount - 1);
    while (nAfterLast-- > 0 && xAtspiNextChild)
        xAtspiNextChild
            = atspiGetFirstRelationTargetOfType(xAtspiNextChild, ATSPI_RELATION_FLOWS_TO);
    /* the child ought to be found and implement the same interfaces, otherwise there's a problem
     * in LO <> Atspi child mapping  */
    CPPUNIT_ASSERT(xAtspiNextChild);
    const auto xAtspiNextText = xAtspiNextChild.queryText();

    // scroll the child into view
    CPPUNIT_ASSERT(xAtspiNextText.scrollSubstringTo(0, 1, ATSPI_SCROLL_ANYWHERE));

    // now, check that the nLOChildIndex is in the visible area (among the regular children)
    nLOChildCount = xLOContext->getAccessibleChildCount();
    CPPUNIT_ASSERT_GREATER(sal_Int64(0), nLOChildCount);
    const auto nLOFirstChildIndex
        = swChildIndex(xLOContext->getAccessibleChild(0)->getAccessibleContext());

    CPPUNIT_ASSERT_LESSEQUAL(nLOChildIndex, nLOFirstChildIndex);
    CPPUNIT_ASSERT_GREATER(nLOChildIndex, nLOFirstChildIndex + nLOChildCount);
#else // !HAVE_ATSPI2_SCROLL_TO
    // unused
    (void)xLOContext;
    (void)xAtspiAccessible;
#endif // !HAVE_ATSPI2_SCROLL_TO
}
