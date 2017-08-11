/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_WRITERFILTER_SOURCE_RTFTOK_RTFLOOKAHEAD_HXX
#define INCLUDED_WRITERFILTER_SOURCE_RTFTOK_RTFLOOKAHEAD_HXX

#include <memory>
#include <rtflistener.hxx>
#include <rtftokenizer.hxx>
#include <comphelper/string.hxx>

class SvStream;

namespace writerfilter
{
namespace rtftok
{
/**
 * This acts like an importer, but used for looking ahead, e.g. to
 * determine if the current group contains a table, etc.
 */
class RTFLookahead : public RTFListener
{
public:
    RTFLookahead(SvStream& rStream, sal_uInt64 nGroupStart);
    ~RTFLookahead() override;
    RTFError dispatchDestination(RTFKeyword nKeyword) override;
    RTFError dispatchFlag(RTFKeyword nKeyword) override;
    RTFError dispatchSymbol(RTFKeyword nKeyword) override;
    RTFError dispatchToggle(RTFKeyword nKeyword, bool bParam, int nParam) override;
    RTFError dispatchValue(RTFKeyword nKeyword, int nParam) override;
    RTFError resolveChars(char ch) override;
    RTFError pushState() override;
    RTFError popState() override;
    Destination getDestination() override;
    void setDestination(Destination eDestination) override;
    RTFInternalState getInternalState() override;
    void setInternalState(RTFInternalState nInternalState) override;
    bool getSkipUnknown() override;
    void setSkipUnknown(bool bSkipUnknown) override;
    void finishSubstream() override;
    bool isSubstream() const override;
    sal_Int32 hasChftn()
    {
        return m_nHasChftn;
    }
    sal_Int32 hasFootnote()
    {
        return m_nHasFootnote;
    }
    OUString getString( sal_uInt32 nGroup )
    {
        return m_aText.size() > nGroup ? m_aText[nGroup].toString() : OUString();
    }
    bool hasTable()
    {
        return m_bHasTable;
    }
private:
    std::shared_ptr<RTFTokenizer> m_pTokenizer;
    SvStream& m_rStream;
    // level of the subgroup that contains a footnote
    sal_Int32 m_nHasChftn;
    sal_Int32 m_nHasFootnote;
    std::vector< OUStringBuffer > m_aText;
    bool m_bHasTable;
};
} // namespace rtftok
} // namespace writerfilter

#endif // INCLUDED_WRITERFILTER_SOURCE_RTFTOK_RTFLOOKAHEAD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
