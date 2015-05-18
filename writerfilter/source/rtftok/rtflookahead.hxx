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
    RTFLookahead(SvStream& rStream, sal_Size nGroupStart);
    virtual ~RTFLookahead();
    virtual RTFError dispatchDestination(RTFKeyword nKeyword) SAL_OVERRIDE;
    virtual RTFError dispatchFlag(RTFKeyword nKeyword) SAL_OVERRIDE;
    virtual RTFError dispatchSymbol(RTFKeyword nKeyword) SAL_OVERRIDE;
    virtual RTFError dispatchToggle(RTFKeyword nKeyword, bool bParam, int nParam) SAL_OVERRIDE;
    virtual RTFError dispatchValue(RTFKeyword nKeyword, int nParam) SAL_OVERRIDE;
    virtual RTFError resolveChars(char ch) SAL_OVERRIDE;
    virtual RTFError pushState() SAL_OVERRIDE;
    virtual RTFError popState() SAL_OVERRIDE;
    virtual Destination getDestination() SAL_OVERRIDE;
    virtual void setDestination(Destination eDestination) SAL_OVERRIDE;
    virtual RTFInternalState getInternalState() SAL_OVERRIDE;
    virtual void setInternalState(RTFInternalState nInternalState) SAL_OVERRIDE;
    virtual bool getSkipUnknown() SAL_OVERRIDE;
    virtual void setSkipUnknown(bool bSkipUnknown) SAL_OVERRIDE;
    virtual void finishSubstream() SAL_OVERRIDE;
    virtual bool isSubstream() const SAL_OVERRIDE;
    bool hasTable()
    {
        return m_bHasTable;
    }
private:
    std::shared_ptr<RTFTokenizer> m_pTokenizer;
    SvStream& m_rStream;
    bool m_bHasTable;
};
}
}

#endif // INCLUDED_WRITERFILTER_SOURCE_RTFTOK_RTFLOOKAHEAD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
