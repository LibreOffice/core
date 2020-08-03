/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "rtflistener.hxx"

#include <vector>
#include <unordered_map>

#include <com/sun/star/uno/Reference.h>

#include <rtl/ustring.hxx>
#include <tools/ref.hxx>

namespace com::sun::star::task
{
class XStatusIndicator;
}
class SvStream;

namespace writerfilter::rtftok
{
/// RTF tokenizer that separates control words from text.
class RTFTokenizer final : public virtual SvRefBase
{
public:
    RTFTokenizer(RTFListener& rImport, SvStream* pInStream,
                 css::uno::Reference<css::task::XStatusIndicator> const& xStatusIndicator);
    ~RTFTokenizer() override;

    RTFError resolveParse();
    /// Number of states on the stack.
    int getGroup() const { return m_nGroup; }
    /// To be invoked by the pushState() callback to signal when the importer enters a group.
    void pushGroup();
    /// To be invoked by the popState() callback to signal when the importer leaves a group.
    void popGroup();
    OUString getPosition();
    std::size_t getGroupStart() const { return m_nGroupStart; }
    /// To look up additional properties of a math symbol.
    static bool lookupMathKeyword(RTFMathSymbol& rSymbol);

private:
    SvStream& Strm() { return *m_pInStream; }
    RTFError resolveKeyword();
    RTFError dispatchKeyword(OString const& rKeyword, bool bParam, int nParam);

    RTFListener& m_rImport;
    SvStream* m_pInStream;
    css::uno::Reference<css::task::XStatusIndicator> const& m_xStatusIndicator;
    // This is the same as aRTFControlWords, but mapped by token name for fast lookup
    static std::unordered_map<OString, RTFSymbol> s_aRTFControlWords;
    static bool s_bControlWordsInitialised;
    // This is the same as aRTFMathControlWords, but sorted
    static std::vector<RTFMathSymbol> s_aRTFMathControlWords;
    static bool s_bMathControlWordsSorted;
    /// Same as the size of the importer's states, except that this can be negative for invalid input.
    int m_nGroup;
    sal_Int32 m_nLineNumber;
    std::size_t m_nLineStartPos;
    std::size_t m_nGroupStart;
};
} // namespace writerfilter::rtftok

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
