/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "rtftokenizer.hxx"
#include <tools/stream.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/character.hxx>
#include <sal/log.hxx>
#include "rtfskipdestination.hxx"
#include <com/sun/star/io/BufferSizeExceededException.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <filter/msfilter/rtfutil.hxx>

using namespace com::sun::star;

namespace writerfilter::rtftok
{
std::unordered_map<OString, RTFSymbol> RTFTokenizer::s_aRTFControlWords;
bool RTFTokenizer::s_bControlWordsInitialised;
std::vector<RTFMathSymbol> RTFTokenizer::s_aRTFMathControlWords;
bool RTFTokenizer::s_bMathControlWordsSorted;

RTFTokenizer::RTFTokenizer(RTFListener& rImport, SvStream* pInStream,
                           uno::Reference<task::XStatusIndicator> const& xStatusIndicator)
    : m_rImport(rImport)
    , m_pInStream(pInStream)
    , m_xStatusIndicator(xStatusIndicator)
    , m_nGroup(0)
    , m_nLineNumber(0)
    , m_nLineStartPos(0)
    , m_nGroupStart(0)
{
    if (!RTFTokenizer::s_bControlWordsInitialised)
    {
        RTFTokenizer::s_bControlWordsInitialised = true;
        for (int i = 0; i < nRTFControlWords; ++i)
            s_aRTFControlWords.emplace(OString(aRTFControlWords[i].GetKeyword()),
                                       aRTFControlWords[i]);
    }
    if (!RTFTokenizer::s_bMathControlWordsSorted)
    {
        RTFTokenizer::s_bMathControlWordsSorted = true;
        s_aRTFMathControlWords = std::vector<RTFMathSymbol>(
            aRTFMathControlWords, aRTFMathControlWords + nRTFMathControlWords);
        std::sort(s_aRTFMathControlWords.begin(), s_aRTFMathControlWords.end());
    }
}

RTFTokenizer::~RTFTokenizer() = default;

RTFError RTFTokenizer::resolveParse()
{
    SAL_INFO("writerfilter.rtf", __func__);
    char ch;
    RTFError ret;
    // for hex chars
    int b = 0;
    int count = 2;
    std::size_t nPercentSize = 0;
    sal_uInt64 nLastPos = 0;

    if (m_xStatusIndicator.is())
    {
        OUString sDocLoad(SvxResId(RID_SVXSTR_DOC_LOAD));

        sal_uInt64 const nCurrentPos = Strm().Tell();
        sal_uInt64 const nEndPos = nCurrentPos + Strm().remainingSize();
        m_xStatusIndicator->start(sDocLoad, nEndPos);
        nPercentSize = nEndPos / 100;

        nLastPos = nCurrentPos;
        m_xStatusIndicator->setValue(nLastPos);
    }

    while (Strm().ReadChar(ch), !Strm().eof())
    {
        //SAL_INFO("writerfilter", __func__ << ": parsing character '" << ch << "'");

        sal_uInt64 const nCurrentPos = Strm().Tell();
        if (m_xStatusIndicator.is() && nCurrentPos > (nLastPos + nPercentSize))
        {
            nLastPos = nCurrentPos;
            m_xStatusIndicator->setValue(nLastPos);
        }

        if (m_nGroup < 0)
            return RTFError::GROUP_UNDER;
        if (m_nGroup > 0 && m_rImport.getInternalState() == RTFInternalState::BIN)
        {
            ret = m_rImport.resolveChars(ch);
            if (ret != RTFError::OK)
                return ret;
        }
        else
        {
            switch (ch)
            {
                case '{':
                    m_nGroupStart = Strm().Tell() - 1;
                    ret = m_rImport.pushState();
                    if (ret != RTFError::OK)
                        return ret;
                    break;
                case '}':
                    ret = m_rImport.popState();
                    if (ret != RTFError::OK)
                        return ret;
                    if (m_nGroup == 0)
                    {
                        if (m_rImport.isSubstream())
                            m_rImport.finishSubstream();
                        return RTFError::OK;
                    }
                    break;
                case '\\':
                    ret = resolveKeyword();
                    if (ret != RTFError::OK)
                        return ret;
                    break;
                case 0x0d:
                    break; // ignore this
                case 0x0a:
                    m_nLineNumber++;
                    m_nLineStartPos = nCurrentPos;
                    break;
                default:
                    if (m_nGroup == 0)
                        return RTFError::CHAR_OVER;
                    if (m_rImport.getInternalState() == RTFInternalState::NORMAL)
                    {
                        ret = m_rImport.resolveChars(ch);
                        if (ret != RTFError::OK)
                            return ret;
                    }
                    else
                    {
                        SAL_INFO("writerfilter.rtf", __func__ << ": hex internal state");
                        b = b << 4;
                        sal_Int8 parsed = msfilter::rtfutil::AsHex(ch);
                        if (parsed == -1)
                            return RTFError::HEX_INVALID;
                        b += parsed;
                        count--;
                        if (!count)
                        {
                            ret = m_rImport.resolveChars(b);
                            if (ret != RTFError::OK)
                                return ret;
                            count = 2;
                            b = 0;
                            m_rImport.setInternalState(RTFInternalState::NORMAL);
                        }
                    }
                    break;
            }
        }
    }

    if (m_nGroup < 0)
        return RTFError::GROUP_UNDER;
    if (m_nGroup > 0)
        return RTFError::GROUP_OVER;
    return RTFError::OK;
}

void RTFTokenizer::pushGroup() { m_nGroup++; }

void RTFTokenizer::popGroup() { m_nGroup--; }

RTFError RTFTokenizer::resolveKeyword()
{
    char ch;
    OStringBuffer aBuf(32);
    bool bNeg = false;
    bool bParam = false;
    int nParam = 0;

    Strm().ReadChar(ch);
    if (Strm().eof())
        return RTFError::UNEXPECTED_EOF;

    if (!rtl::isAsciiAlpha(static_cast<unsigned char>(ch)))
    {
        aBuf.append(ch);
        OString aKeyword = aBuf.makeStringAndClear();
        // control symbols aren't followed by a space, so we can return here
        // without doing any SeekRel()
        return dispatchKeyword(aKeyword, bParam, nParam);
    }
    while (rtl::isAsciiAlpha(static_cast<unsigned char>(ch)))
    {
        aBuf.append(ch);
        if (aBuf.getLength() > 32)
            // See RTF spec v1.9.1, page 7
            // A control word's name cannot be longer than 32 letters.
            throw io::BufferSizeExceededException();
        Strm().ReadChar(ch);
        if (Strm().eof())
        {
            ch = ' ';
            break;
        }
    }

    if (ch == '-')
    {
        // in case we'll have a parameter, that will be negative
        bNeg = true;
        Strm().ReadChar(ch);
        if (Strm().eof())
            return RTFError::UNEXPECTED_EOF;
    }
    if (rtl::isAsciiDigit(static_cast<unsigned char>(ch)))
    {
        OStringBuffer aParameter;

        // we have a parameter
        bParam = true;
        while (rtl::isAsciiDigit(static_cast<unsigned char>(ch)))
        {
            aParameter.append(ch);
            Strm().ReadChar(ch);
            if (Strm().eof())
            {
                ch = ' ';
                break;
            }
        }
        nParam = aParameter.makeStringAndClear().toInt32();
        if (bNeg)
            nParam = -nParam;
    }
    if (ch != ' ')
        Strm().SeekRel(-1);
    OString aKeyword = aBuf.makeStringAndClear();
    return dispatchKeyword(aKeyword, bParam, nParam);
}

bool RTFTokenizer::lookupMathKeyword(RTFMathSymbol& rSymbol)
{
    auto low
        = std::lower_bound(s_aRTFMathControlWords.begin(), s_aRTFMathControlWords.end(), rSymbol);
    if (low == s_aRTFMathControlWords.end() || rSymbol < *low)
        return false;
    rSymbol = *low;
    return true;
}

RTFError RTFTokenizer::dispatchKeyword(OString const& rKeyword, bool bParam, int nParam)
{
    if (m_rImport.getDestination() == Destination::SKIP)
    {
        // skip binary data explicitly, to not trip over rtf markup
        // control characters
        if (rKeyword == "bin" && nParam > 0)
            Strm().SeekRel(nParam);
        return RTFError::OK;
    }
    SAL_INFO("writerfilter.rtf", __func__ << ": keyword '\\" << rKeyword << "' with param? "
                                          << (bParam ? 1 : 0) << " param val: '"
                                          << (bParam ? nParam : 0) << "'");
    auto findIt = s_aRTFControlWords.find(rKeyword);
    if (findIt == s_aRTFControlWords.end())
    {
        SAL_INFO("writerfilter.rtf", __func__ << ": unknown keyword '\\" << rKeyword << "'");
        RTFSkipDestination aSkip(m_rImport);
        aSkip.setParsed(false);
        return RTFError::OK;
    }

    RTFError ret;
    RTFSymbol const& rSymbol = findIt->second;
    switch (rSymbol.GetControlType())
    {
        case RTFControlType::FLAG:
            // flags ignore any parameter by definition
            ret = m_rImport.dispatchFlag(rSymbol.GetIndex());
            if (ret != RTFError::OK)
                return ret;
            break;
        case RTFControlType::DESTINATION:
            // same for destinations
            ret = m_rImport.dispatchDestination(rSymbol.GetIndex());
            if (ret != RTFError::OK)
                return ret;
            break;
        case RTFControlType::SYMBOL:
            // and symbols
            ret = m_rImport.dispatchSymbol(rSymbol.GetIndex());
            if (ret != RTFError::OK)
                return ret;
            break;
        case RTFControlType::TOGGLE:
            ret = m_rImport.dispatchToggle(rSymbol.GetIndex(), bParam, nParam);
            if (ret != RTFError::OK)
                return ret;
            break;
        case RTFControlType::VALUE:
            if (!bParam)
                nParam = rSymbol.GetDefValue();
            ret = m_rImport.dispatchValue(rSymbol.GetIndex(), nParam);
            if (ret != RTFError::OK)
                return ret;
            break;
    }

    return RTFError::OK;
}

OUString RTFTokenizer::getPosition()
{
    OUStringBuffer aRet;
    aRet.append(m_nLineNumber + 1);
    aRet.append(",");
    aRet.append(sal_Int32(Strm().Tell() - m_nLineStartPos + 1));
    return aRet.makeStringAndClear();
}

} // namespace writerfilter::rtftok

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
