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

#include <rtftokenizer.hxx>

using rtl::OString;
using rtl::OStringBuffer;
using rtl::OUString;
using rtl::OUStringBuffer;
using rtl::OUStringToOString;

namespace writerfilter {
namespace rtftok {

RTFTokenizer::RTFTokenizer(RTFDocumentImpl& rImport, SvStream* pInStream)
    : m_rImport(rImport),
    m_pInStream(pInStream)
{
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
    OSL_TRACE("%s", OSL_THIS_FUNC);
    char ch;
    int ret;
    // for hex chars
    int b = 0, count = 2;

    while ((Strm() >> ch, !Strm().IsEof()))
    {
        //OSL_TRACE("%s: parsing character '%c'", OSL_THIS_FUNC, ch);
        if (m_rImport.getGroup() < 0)
            return ERROR_GROUP_UNDER;
        if (!m_rImport.isEmpty() && m_rImport.getState().nInternalState == INTERNAL_BIN)
        {
            OSL_TRACE("%s: TODO, binary internal state", OSL_THIS_FUNC);
        }
        else
        {
            switch (ch)
            {
                case '{':
                    if ((ret = m_rImport.pushState()))
                        return ret;
                    break;
                case '}':
                    if ((ret = m_rImport.popState()))
                        return ret;
                    if (m_rImport.isSubstream() && m_rImport.getGroup() == 0)
                        return 0;
                    break;
                case '\\':
                    if ((ret = resolveKeyword()))
                        return ret;
                    break;
                case 0x0d:
                case 0x0a:
                    break; // ignore these
                default:
                    if (m_rImport.isEmpty())
                        return ERROR_CHAR_OVER;
                    if (m_rImport.getState().nInternalState == INTERNAL_NORMAL)
                    {
                        if ((ret = m_rImport.resolveChars(ch)))
                            return ret;
                    }
                    else
                    {
                        OSL_TRACE("%s: hex internal state", OSL_THIS_FUNC);
                        b = b << 4;
                        char parsed = asHex(ch);
                        if (parsed == -1)
                            return ERROR_HEX_INVALID;
                        b += parsed;
                        count--;
                        if (!count)
                        {
                            if ((ret = m_rImport.resolveChars(b)))
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

    if (m_rImport.getGroup() < 0)
        return ERROR_GROUP_UNDER;
    else if (m_rImport.getGroup() > 0)
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
    /*OSL_TRACE("%s: keyword '\\%s' with param? %d param val: '%d'", OSL_THIS_FUNC,
            rKeyword.getStr(), (bParam ? 1 : 0), (bParam ? nParam : 0));*/
    int i, ret;
    for (i = 0; i < nRTFControlWords; i++)
    {
        if (!strcmp(rKeyword.getStr(), aRTFControlWords[i].sKeyword))
            break;
    }
    if (i == nRTFControlWords)
    {
        OSL_TRACE("%s: unknown keyword '\\%s'", OSL_THIS_FUNC, rKeyword.getStr());
        m_rImport.skipDestination(false);
        return 0;
    }

    switch (aRTFControlWords[i].nControlType)
    {
        case CONTROL_FLAG:
            // flags ignore any parameter by definition
            if ((ret = m_rImport.dispatchFlag(aRTFControlWords[i].nIndex)))
                return ret;
            break;
        case CONTROL_DESTINATION:
            // same for destinations
            if ((ret = m_rImport.dispatchDestination(aRTFControlWords[i].nIndex)))
                return ret;
            break;
        case CONTROL_SYMBOL:
            // and symbols
            if ((ret = m_rImport.dispatchSymbol(aRTFControlWords[i].nIndex)))
                return ret;
            break;
        case CONTROL_TOGGLE:
            if ((ret = m_rImport.dispatchToggle(aRTFControlWords[i].nIndex, bParam, nParam)))
                return ret;
            break;
        case CONTROL_VALUE:
            // values require a parameter by definition
            if (bParam && (ret = m_rImport.dispatchValue(aRTFControlWords[i].nIndex, nParam)))
                return ret;
            break;
    }

    return 0;
}

} // namespace rtftok
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
