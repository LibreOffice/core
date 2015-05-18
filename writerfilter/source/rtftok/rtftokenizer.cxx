/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtftokenizer.hxx>
#include <svx/dialogs.hrc>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <rtl/strbuf.hxx>
#include <rtfskipdestination.hxx>
#include <com/sun/star/io/BufferSizeExceededException.hpp>

using namespace com::sun::star;

namespace writerfilter
{
namespace rtftok
{

std::vector<RTFSymbol> RTFTokenizer::m_aRTFControlWords;
bool RTFTokenizer::m_bControlWordsSorted;
std::vector<RTFMathSymbol> RTFTokenizer::m_aRTFMathControlWords;
bool RTFTokenizer::m_bMathControlWordsSorted;

RTFTokenizer::RTFTokenizer(RTFListener& rImport, SvStream* pInStream, uno::Reference<task::XStatusIndicator> const& xStatusIndicator)
    : m_rImport(rImport),
      m_pInStream(pInStream),
      m_xStatusIndicator(xStatusIndicator),
      m_nGroup(0),
      m_nLineNumber(0),
      m_nLineStartPos(0),
      m_nGroupStart(0)
{
    if (!RTFTokenizer::m_bControlWordsSorted)
    {
        RTFTokenizer::m_bControlWordsSorted = true;
        m_aRTFControlWords = std::vector<RTFSymbol>(aRTFControlWords, aRTFControlWords + nRTFControlWords);
        std::sort(m_aRTFControlWords.begin(), m_aRTFControlWords.end());
    }
    if (!RTFTokenizer::m_bMathControlWordsSorted)
    {
        RTFTokenizer::m_bMathControlWordsSorted = true;
        m_aRTFMathControlWords = std::vector<RTFMathSymbol>(aRTFMathControlWords, aRTFMathControlWords + nRTFMathControlWords);
        std::sort(m_aRTFMathControlWords.begin(), m_aRTFMathControlWords.end());
    }
}

RTFTokenizer::~RTFTokenizer()
{
}


RTFError RTFTokenizer::resolveParse()
{
    SAL_INFO("writerfilter", OSL_THIS_FUNC);
    char ch;
    RTFError ret;
    // for hex chars
    int b = 0, count = 2;
    sal_uInt32 nPercentSize = 0;
    sal_uInt32 nLastPos = 0;

    if (m_xStatusIndicator.is())
    {
        static ResMgr* pResMgr = ResMgr::CreateResMgr("svx", Application::GetSettings().GetUILanguageTag());
        OUString sDocLoad(ResId(RID_SVXSTR_DOC_LOAD, *pResMgr).toString());

        sal_Size nCurrentPos = Strm().Tell();
        sal_Size nEndPos = nCurrentPos + Strm().remainingSize();
        m_xStatusIndicator->start(sDocLoad, nEndPos);
        nPercentSize = nEndPos / 100;

        m_xStatusIndicator->setValue(nLastPos = nCurrentPos);
    }

    while ((Strm().ReadChar(ch), !Strm().IsEof()))
    {
        //SAL_INFO("writerfilter", OSL_THIS_FUNC << ": parsing character '" << ch << "'");

        sal_Size nCurrentPos = Strm().Tell();
        if (m_xStatusIndicator.is() && nCurrentPos > (nLastPos + nPercentSize))
            m_xStatusIndicator->setValue(nLastPos = nCurrentPos);

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
                    SAL_INFO("writerfilter", OSL_THIS_FUNC << ": hex internal state");
                    b = b << 4;
                    sal_Int8 parsed = asHex(ch);
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
    else if (m_nGroup > 0)
        return RTFError::GROUP_OVER;
    return RTFError::OK;
}

int RTFTokenizer::asHex(char ch)
{
    int ret = 0;
    if (isdigit(ch))
        ret = ch - '0';
    else
    {
        if (islower(ch))
        {
            if (ch < 'a' || ch > 'f')
                return -1;
            ret = ch - 'a';
        }
        else
        {
            if (ch < 'A' || ch > 'F')
                return -1;
            ret = ch - 'A';
        }
        ret += 10;
    }
    return ret;
}


void RTFTokenizer::pushGroup()
{
    m_nGroup++;
}

void RTFTokenizer::popGroup()
{
    m_nGroup--;
}

RTFError RTFTokenizer::resolveKeyword()
{
    char ch;
    OStringBuffer aBuf;
    bool bNeg = false;
    bool bParam = false;
    int nParam = 0;

    Strm().ReadChar(ch);
    if (Strm().IsEof())
        return RTFError::UNEXPECTED_EOF;

    if (!isalpha(ch))
    {
        aBuf.append(ch);
        OString aKeyword = aBuf.makeStringAndClear();
        // control symbols aren't followed by a space, so we can return here
        // without doing any SeekRel()
        return dispatchKeyword(aKeyword, bParam, nParam);
    }
    while (isalpha(ch))
    {
        aBuf.append(ch);
        Strm().ReadChar(ch);
        if (Strm().IsEof())
        {
            ch = ' ';
            break;
        }
    }
    if (aBuf.getLength() > 32)
        // See RTF spec v1.9.1, page 7
        // A control word's name cannot be longer than 32 letters.
        throw io::BufferSizeExceededException();

    if (ch == '-')
    {
        // in case we'll have a parameter, that will be negative
        bNeg = true;
        Strm().ReadChar(ch);
        if (Strm().IsEof())
            return RTFError::UNEXPECTED_EOF;
    }
    if (isdigit(ch))
    {
        OStringBuffer aParameter;

        // we have a parameter
        bParam = true;
        while (isdigit(ch))
        {
            aParameter.append(ch);
            Strm().ReadChar(ch);
            if (Strm().IsEof())
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
    std::vector<RTFMathSymbol>::iterator low = std::lower_bound(m_aRTFMathControlWords.begin(), m_aRTFMathControlWords.end(), rSymbol);
    int i = low - m_aRTFMathControlWords.begin();
    if (low == m_aRTFMathControlWords.end() || rSymbol < *low)
        return false;
    rSymbol = m_aRTFMathControlWords[i];
    return true;
}

RTFError RTFTokenizer::dispatchKeyword(OString& rKeyword, bool bParam, int nParam)
{
    if (m_rImport.getDestination() == Destination::SKIP)
        return RTFError::OK;
    SAL_INFO("writerfilter.rtf", OSL_THIS_FUNC << ": keyword '\\" << rKeyword.getStr() <<
             "' with param? " << (bParam ? 1 : 0) <<" param val: '" << (bParam ? nParam : 0) << "'");
    RTFSymbol aSymbol;
    aSymbol.sKeyword = rKeyword.getStr();
    std::vector<RTFSymbol>::iterator low = std::lower_bound(m_aRTFControlWords.begin(), m_aRTFControlWords.end(), aSymbol);
    int i = low - m_aRTFControlWords.begin();
    if (low == m_aRTFControlWords.end() || aSymbol < *low)
    {
        SAL_INFO("writerfilter", OSL_THIS_FUNC << ": unknown keyword '\\" << rKeyword.getStr() << "'");
        RTFSkipDestination aSkip(m_rImport);
        aSkip.setParsed(false);
        return RTFError::OK;
    }

    RTFError ret;
    switch (m_aRTFControlWords[i].nControlType)
    {
    case CONTROL_FLAG:
        // flags ignore any parameter by definition
        ret = m_rImport.dispatchFlag(m_aRTFControlWords[i].nIndex);
        if (ret != RTFError::OK)
            return ret;
        break;
    case CONTROL_DESTINATION:
        // same for destinations
        ret = m_rImport.dispatchDestination(m_aRTFControlWords[i].nIndex);
        if (ret != RTFError::OK)
            return ret;
        break;
    case CONTROL_SYMBOL:
        // and symbols
        ret = m_rImport.dispatchSymbol(m_aRTFControlWords[i].nIndex);
        if (ret != RTFError::OK)
            return ret;
        break;
    case CONTROL_TOGGLE:
        ret = m_rImport.dispatchToggle(m_aRTFControlWords[i].nIndex, bParam, nParam);
        if (ret != RTFError::OK)
            return ret;
        break;
    case CONTROL_VALUE:
        // values require a parameter by definition
        if (bParam)
        {
            ret = m_rImport.dispatchValue(m_aRTFControlWords[i].nIndex, nParam);
            if (ret != RTFError::OK)
                return ret;
        }
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


}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
