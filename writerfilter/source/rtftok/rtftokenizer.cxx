/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Miklos Vajna <vmiklos@frugalware.org>
 * Portions created by the Initial Developer are Copyright (C) 2011 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <tools/stream.hxx>
#include <tools/resmgr.hxx>
#include <svx/dialogs.hrc>
#include <vcl/svapp.hxx>

#include <rtftokenizer.hxx>
#include <rtfskipdestination.hxx>

using rtl::OString;
using rtl::OStringBuffer;
using rtl::OUString;
using rtl::OUStringBuffer;
using rtl::OUStringToOString;

namespace writerfilter {
namespace rtftok {

RTFTokenizer::RTFTokenizer(RTFDocumentImpl& rImport, SvStream* pInStream, uno::Reference<task::XStatusIndicator> const& xStatusIndicator)
    : m_rImport(rImport),
    m_pInStream(pInStream),
    m_xStatusIndicator(xStatusIndicator),
    m_aRTFControlWords(std::vector<RTFSymbol>(aRTFControlWords, aRTFControlWords + nRTFControlWords)),
    m_nGroup(0),
    m_nLineNumber(0),
    m_nLineStartPos(0)
{
    std::sort(m_aRTFControlWords.begin(), m_aRTFControlWords.end());
}

RTFTokenizer::~RTFTokenizer()
{
}

SvStream& RTFTokenizer::Strm()
{
    return *m_pInStream;
}

int RTFTokenizer::resolveParse()
{
    SAL_INFO( "writerfilter", OSL_THIS_FUNC );
    char ch;
    int ret;
    // for hex chars
    int b = 0, count = 2;
    sal_uInt32 nPercentSize = 0;
    sal_uInt32 nLastPos = 0;

    if (m_xStatusIndicator.is())
    {
        static ResMgr* pResMgr = ResMgr::CreateResMgr("svx", Application::GetSettings().GetUILocale());
        OUString sDocLoad(ResId(RID_SVXSTR_DOC_LOAD, *pResMgr).toString());

        sal_uInt32 nCurrentPos = Strm().Tell();
        Strm().Seek(STREAM_SEEK_TO_END);
        sal_uInt32 nEndPos = Strm().Tell();
        Strm().Seek(nCurrentPos);
        m_xStatusIndicator->start(sDocLoad, nEndPos);
        nPercentSize = nEndPos / 100;

        m_xStatusIndicator->setValue(nLastPos = nCurrentPos);
    }

    while ((Strm() >> ch, !Strm().IsEof()))
    {
        //SAL_INFO("writerfilter", OSL_THIS_FUNC << ": parsing character '" << ch << "'");

        sal_uInt32 nCurrentPos;
        if (m_xStatusIndicator.is() && (nCurrentPos = Strm().Tell()) > (nLastPos + nPercentSize))
            m_xStatusIndicator->setValue(nLastPos = nCurrentPos);

        if (m_nGroup < 0)
            return ERROR_GROUP_UNDER;
        if (m_nGroup > 0 && m_rImport.getState().nInternalState == INTERNAL_BIN)
        {
            ret = m_rImport.resolveChars(ch);
            if (ret)
                return ret;
        }
        else
        {
            switch (ch)
            {
                case '{':
                    ret = m_rImport.pushState();
                    if (ret)
                        return ret;
                    break;
                case '}':
                    ret = m_rImport.popState();
                    if (ret)
                        return ret;
                    if (m_nGroup == 0)
                    {
                        if (m_rImport.isSubstream())
                            m_rImport.finishSubstream();
                        return 0;
                    }
                    break;
                case '\\':
                    ret = resolveKeyword();
                    if (ret)
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
                        return ERROR_CHAR_OVER;
                    if (m_rImport.getState().nInternalState == INTERNAL_NORMAL)
                    {
                        ret = m_rImport.resolveChars(ch);
                        if (ret)
                            return ret;
                    }
                    else
                    {
                        SAL_INFO("writerfilter", OSL_THIS_FUNC << ": hex internal state");
                        b = b << 4;
                        sal_Int8 parsed = asHex(ch);
                        if (parsed == -1)
                            return ERROR_HEX_INVALID;
                        b += parsed;
                        count--;
                        if (!count)
                        {
                            ret = m_rImport.resolveChars(b);
                            if (ret)
                                return ret;
                            count = 2;
                            b = 0;
                            m_rImport.getState().nInternalState = INTERNAL_NORMAL;
                        }
                    }
                    break;
            }
        }
    }

    if (m_nGroup < 0)
        return ERROR_GROUP_UNDER;
    else if (m_nGroup > 0)
        return ERROR_GROUP_OVER;
    return 0;
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

int RTFTokenizer::getGroup() const
{
    return m_nGroup;
}

void RTFTokenizer::pushGroup()
{
    m_nGroup++;
}

void RTFTokenizer::popGroup()
{
    m_nGroup--;
}

int RTFTokenizer::resolveKeyword()
{
    char ch;
    OStringBuffer aBuf;
    bool bNeg = false;
    bool bParam = false;
    int nParam = 0;

    Strm() >> ch;
    if (Strm().IsEof())
        return ERROR_EOF;

    if (!isalpha(ch))
    {
        aBuf.append(ch);
        OString aKeyword = aBuf.makeStringAndClear();
        // control symbols aren't followed by a space, so we can return here
        // without doing any SeekRel()
        return dispatchKeyword(aKeyword, bParam, nParam);
    }
    while(isalpha(ch))
    {
        aBuf.append(ch);
        Strm() >> ch;
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
        Strm() >> ch;
        if (Strm().IsEof())
            return ERROR_EOF;
    }
    if (isdigit(ch))
    {
        OStringBuffer aParameter;

        // we have a parameter
        bParam = true;
        while(isdigit(ch))
        {
            aParameter.append(ch);
            Strm() >> ch;
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

int RTFTokenizer::dispatchKeyword(OString& rKeyword, bool bParam, int nParam)
{
    if (m_rImport.getState().nDestinationState == DESTINATION_SKIP)
        return 0;
    /*SAL_INFO("writefilter", OSL_THIS_FUNC << ": keyword '\\" << rKeyword.getStr() <<
               "' with param? " << (bParam ? 1 : 0) <<" param val: '" << (bParam ? nParam : 0) << "'");*/
    RTFSymbol aSymbol;
    aSymbol.sKeyword = rKeyword.getStr();
    std::vector<RTFSymbol>::iterator low = std::lower_bound(m_aRTFControlWords.begin(), m_aRTFControlWords.end(), aSymbol);
    int i = low - m_aRTFControlWords.begin();
    if (low == m_aRTFControlWords.end() || aSymbol < *low)
    {
        SAL_INFO("writerfilter", OSL_THIS_FUNC << ": unknown keyword '\\" << rKeyword.getStr() << "'");
        RTFSkipDestination aSkip(m_rImport);
        aSkip.setParsed(false);
        return 0;
    }

    int ret;
    switch (m_aRTFControlWords[i].nControlType)
    {
        case CONTROL_FLAG:
            // flags ignore any parameter by definition
            ret = m_rImport.dispatchFlag(m_aRTFControlWords[i].nIndex);
            if (ret)
                return ret;
            break;
        case CONTROL_DESTINATION:
            // same for destinations
            ret = m_rImport.dispatchDestination(m_aRTFControlWords[i].nIndex);
            if (ret)
                return ret;
            break;
        case CONTROL_SYMBOL:
            // and symbols
            ret = m_rImport.dispatchSymbol(m_aRTFControlWords[i].nIndex);
            if (ret)
                return ret;
            break;
        case CONTROL_TOGGLE:
            ret = m_rImport.dispatchToggle(m_aRTFControlWords[i].nIndex, bParam, nParam);
            if (ret)
                return ret;
            break;
        case CONTROL_VALUE:
            // values require a parameter by definition
            if (bParam) {
                ret = m_rImport.dispatchValue(m_aRTFControlWords[i].nIndex, nParam);
                if (ret)
                    return ret;
            }
            break;
    }

    return 0;
}

OUString RTFTokenizer::getPosition()
{
    OUStringBuffer aRet;
    aRet.append(m_nLineNumber + 1);
    aRet.append(",");
    aRet.append(sal_Int32(Strm().Tell() - m_nLineStartPos + 1));
    return aRet.makeStringAndClear();
}

} // namespace rtftok
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
