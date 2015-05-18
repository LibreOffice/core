/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_WRITERFILTER_SOURCE_RTFTOK_RTFTOKENIZER_HXX
#define INCLUDED_WRITERFILTER_SOURCE_RTFTOK_RTFTOKENIZER_HXX

#include <rtflistener.hxx>

#include <vector>

#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

class SvStream;

namespace writerfilter
{
namespace rtftok
{
/// RTF tokenizer that separates control words from text.
class RTFTokenizer
{
public:
    RTFTokenizer(RTFListener& rImport, SvStream* pInStream, css::uno::Reference<css::task::XStatusIndicator> const& xStatusIndicator);
    virtual ~RTFTokenizer();

    RTFError resolveParse();
    static int asHex(char ch);
    /// Number of states on the stack.
    int getGroup() const
    {
        return m_nGroup;
    }
    /// To be invoked by the pushState() callback to signal when the importer enters a group.
    void pushGroup();
    /// To be invoked by the popState() callback to single when the importer leaves a group.
    void popGroup();
    OUString getPosition();
    sal_Size getGroupStart()
    {
        return m_nGroupStart;
    }
    /// To look up additional properties of a math symbol.
    static bool lookupMathKeyword(RTFMathSymbol& rSymbol);
private:
    SvStream& Strm()
    {
        return *m_pInStream;
    }
    RTFError resolveKeyword();
    RTFError dispatchKeyword(OString& rKeyword, bool bParam, int nParam);

    RTFListener& m_rImport;
    SvStream* m_pInStream;
    css::uno::Reference<css::task::XStatusIndicator> const& m_xStatusIndicator;
    // This is the same as aRTFControlWords, but sorted
    static std::vector<RTFSymbol> m_aRTFControlWords;
    static bool m_bControlWordsSorted;
    // This is the same as aRTFMathControlWords, but sorted
    static std::vector<RTFMathSymbol> m_aRTFMathControlWords;
    static bool m_bMathControlWordsSorted;
    /// Same as the size of the importer's states, except that this can be negative for invalid input.
    int m_nGroup;
    sal_Int32 m_nLineNumber;
    sal_Size m_nLineStartPos;
    sal_Size m_nGroupStart;
};
}
}

#endif // INCLUDED_WRITERFILTER_SOURCE_RTFTOK_RTFTOKENIZER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
