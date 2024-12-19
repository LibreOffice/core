/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <i18nutil/scriptchangescanner.hxx>
#include <i18nutil/unicode.hxx>
#include <i18nutil/scriptclass.hxx>
#include <unicode/uchar.h>
#include <unicode/ubidi.h>
#include <sal/log.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/i18n/CharType.hpp>
#include <com/sun/star/i18n/UnicodeType.hpp>

namespace css = ::com::sun::star;

namespace i18nutil
{
namespace
{
constexpr sal_uInt32 CHAR_NNBSP = 0x202f;

class IcuDirectionChangeScanner : public DirectionChangeScanner
{
private:
    const OUString& m_rText;
    UBiDi* m_pBidi;
    DirectionChange m_stCurr;
    UBiDiLevel m_nInitialDirection;
    int32_t m_nCurrIndex = 0;
    int m_nCount = 0;
    int m_nCurr = 0;
    bool m_bAtEnd = false;

    bool RangeHasStrongLTR(sal_Int32 nStart, sal_Int32 nEnd)
    {
        for (sal_Int32 nCharIdx = nStart; nCharIdx < nEnd; ++nCharIdx)
        {
            auto nCharDir = u_charDirection(m_rText[nCharIdx]);
            if (nCharDir == U_LEFT_TO_RIGHT || nCharDir == U_LEFT_TO_RIGHT_EMBEDDING
                || nCharDir == U_LEFT_TO_RIGHT_OVERRIDE)
            {
                return true;
            }
        }

        return false;
    }

    void PopulateCurr()
    {
        int32_t nEndIndex = 0;
        UBiDiLevel nCurrLevel = 0;
        ubidi_getLogicalRun(m_pBidi, m_nCurrIndex, &nEndIndex, &nCurrLevel);

        bool bHasEmbeddedStrongLTR = false;
        if ((nCurrLevel % 2) == UBIDI_LTR && nCurrLevel > UBIDI_RTL)
        {
            bHasEmbeddedStrongLTR = RangeHasStrongLTR(m_nCurrIndex, nEndIndex);
        }

        m_stCurr = { m_nCurrIndex, nEndIndex, nCurrLevel, bHasEmbeddedStrongLTR };

        m_nCurrIndex = nEndIndex;
        ++m_nCurr;

        m_bAtEnd = false;
    }

public:
    IcuDirectionChangeScanner(const OUString& rText, UBiDiLevel nInitialDirection)
        : m_rText(rText)
        , m_nInitialDirection(nInitialDirection)
    {
        UErrorCode nError = U_ZERO_ERROR;
        m_pBidi = ubidi_openSized(rText.getLength(), 0, &nError);
        nError = U_ZERO_ERROR;

        ubidi_setPara(m_pBidi, reinterpret_cast<const UChar*>(rText.getStr()), rText.getLength(),
                      nInitialDirection, nullptr, &nError);
        nError = U_ZERO_ERROR;

        m_nCount = ubidi_countRuns(m_pBidi, &nError);
        Reset();
    }

    ~IcuDirectionChangeScanner() override { ubidi_close(m_pBidi); }

    void Reset() override
    {
        m_nCurrIndex = 0;
        m_nCurr = 0;
        m_stCurr = { /*start*/ 0, /*end*/ 0, /*level*/ m_nInitialDirection,
                     /*has embedded strong LTR*/ false };
        m_bAtEnd = true;

        if (m_nCurr < m_nCount)
        {
            PopulateCurr();
        }
    }

    bool AtEnd() const override { return m_bAtEnd; }

    void Advance() override
    {
        if (m_nCurr >= m_nCount)
        {
            m_bAtEnd = true;
            return;
        }

        PopulateCurr();
    }

    DirectionChange Peek() const override { return m_stCurr; }

    UBiDiLevel GetLevelAt(sal_Int32 nIndex) const override
    {
        return ubidi_getLevelAt(m_pBidi, nIndex);
    }
};

class GreedyScriptChangeScanner : public ScriptChangeScanner
{
private:
    ScriptChange m_stCurr;
    DirectionChangeScanner* m_pDirScanner;
    const OUString& m_rText;
    sal_Int16 m_nPrevScript;
    sal_Int32 m_nIndex = 0;
    bool m_bAtEnd = false;
    bool m_bApplyAsianToWeakQuotes = false;

public:
    GreedyScriptChangeScanner(const OUString& rText, sal_Int16 nDefaultScriptType,
                              DirectionChangeScanner* pDirScanner)
        : m_pDirScanner(pDirScanner)
        , m_rText(rText)
        , m_nPrevScript(nDefaultScriptType)
    {
        // tdf#66791: For compatibility with other programs, the Asian script is
        // applied to any weak-script quote characters if the enclosing paragraph
        // contains Chinese- or Japanese-script characters.
        sal_Int32 nCjBase = 0;
        while (nCjBase < m_rText.getLength())
        {
            auto nChar = m_rText.iterateCodePoints(&nCjBase);
            auto nScript = GetScriptClass(nChar);
            if (nScript == css::i18n::ScriptType::COMPLEX)
            {
                m_bApplyAsianToWeakQuotes = false;
                break;
            }

            auto nUnicodeScript = u_getIntPropertyValue(nChar, UCHAR_SCRIPT);
            switch (nUnicodeScript)
            {
                case USCRIPT_HAN:
                case USCRIPT_HIRAGANA:
                case USCRIPT_KATAKANA:
                    m_bApplyAsianToWeakQuotes = true;
                    break;

                default:
                    break;
            }
        }

        // In the original Writer algorithm, the application language is used for
        // all leading weak characters. Make a change record for those characters.
        Advance();
        if (m_stCurr.m_nStartIndex == m_stCurr.m_nEndIndex)
        {
            // The text does not start with application-language leading characters.
            // Initialize with a non-empty record.
            Advance();
        }
    }

    bool AtEnd() const override { return m_bAtEnd; }

    void Advance() override
    {
        m_stCurr = ScriptChange{ /*start*/ 0, /*end*/ 0, /*type*/ m_nPrevScript };

        if (m_nIndex >= m_rText.getLength())
        {
            m_bAtEnd = true;
            return;
        }

        auto nRunStart = m_nIndex;
        auto nScript = m_nPrevScript;
        while (m_nIndex < m_rText.getLength())
        {
            auto nPrevIndex = m_nIndex;
            auto nBidiLevel = m_pDirScanner->GetLevelAt(m_nIndex);

            bool bCharIsRtl = (nBidiLevel % 2 == UBIDI_RTL);
            bool bCharIsRtlOrEmbedded = (nBidiLevel > UBIDI_LTR);
            bool bRunHasStrongEmbeddedLTR = false;

            while (bCharIsRtlOrEmbedded && !m_pDirScanner->AtEnd())
            {
                const auto stDirRun = m_pDirScanner->Peek();
                if (m_nIndex >= stDirRun.m_nStartIndex && m_nIndex < stDirRun.m_nEndIndex)
                {
                    bRunHasStrongEmbeddedLTR = stDirRun.m_bHasEmbeddedStrongLTR;
                    break;
                }

                m_pDirScanner->Advance();
            }

            auto nChar = m_rText.iterateCodePoints(&m_nIndex);
            nScript = GetScriptClass(nChar);

            // #i16354# Change script type for RTL text to CTL:
            // 1. All text in RTL runs will use the CTL font
            // #i89825# change the script type also to CTL (hennerdrewes)
            // 2. Text in embedded LTR runs that does not have any strong LTR characters (numbers!)
            // tdf#163660 Asian-script characters inside RTL runs should still use Asian font
            if (bCharIsRtl || (bCharIsRtlOrEmbedded && !bRunHasStrongEmbeddedLTR))
            {
                if (nScript != css::i18n::ScriptType::ASIAN)
                {
                    nScript = css::i18n::ScriptType::COMPLEX;
                }
            }
            else if (nScript == css::i18n::ScriptType::WEAK)
            {
                nScript = m_nPrevScript;
                if (m_bApplyAsianToWeakQuotes)
                {
                    auto nType = unicode::getUnicodeType(nChar);
                    if (nType == css::i18n::UnicodeType::INITIAL_PUNCTUATION
                        || nType == css::i18n::UnicodeType::FINAL_PUNCTUATION)
                    {
                        nScript = css::i18n::ScriptType::ASIAN;
                    }
                }
            }

            if (nScript != m_nPrevScript)
            {
                m_nIndex = nPrevIndex;
                break;
            }
        }

        if (m_nIndex > 0)
        {
            // special case for dotted circle since it can be used with complex
            // before a mark, so we want it associated with the mark's script
            // tdf#112594: another special case for NNBSP followed by a Mongolian
            // character, since NNBSP has special uses in Mongolian (tdf#112594)
            auto nPrevPos = m_nIndex;
            auto nPrevChar = m_rText.iterateCodePoints(&nPrevPos, -1);
            if (m_nIndex < m_rText.getLength()
                && css::i18n::ScriptType::WEAK == GetScriptClass(nPrevChar))
            {
                auto nChar = m_rText.iterateCodePoints(&m_nIndex, 0);
                auto nType = unicode::getUnicodeType(nChar);
                if (nType == css::i18n::UnicodeType::NON_SPACING_MARK
                    || nType == css::i18n::UnicodeType::ENCLOSING_MARK
                    || nType == css::i18n::UnicodeType::COMBINING_SPACING_MARK
                    || (nPrevChar == CHAR_NNBSP
                        && u_getIntPropertyValue(nChar, UCHAR_SCRIPT) == USCRIPT_MONGOLIAN))
                {
                    m_nIndex = nPrevPos;
                }
            }
        }

        m_stCurr = ScriptChange{ nRunStart, m_nIndex, m_nPrevScript };
        m_nPrevScript = nScript;
    }

    ScriptChange Peek() const override { return m_stCurr; }
};
}
}

std::unique_ptr<i18nutil::DirectionChangeScanner>
i18nutil::MakeDirectionChangeScanner(const OUString& rText, sal_uInt8 nInitialDirection)
{
    return std::make_unique<IcuDirectionChangeScanner>(rText, nInitialDirection);
}

std::unique_ptr<i18nutil::ScriptChangeScanner>
i18nutil::MakeScriptChangeScanner(const OUString& rText, sal_Int16 nDefaultScriptType,
                                  DirectionChangeScanner& rDirScanner)
{
    return std::make_unique<GreedyScriptChangeScanner>(rText, nDefaultScriptType, &rDirScanner);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
