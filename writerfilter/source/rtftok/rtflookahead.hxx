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

#include <rtflistener.hxx>
#include <rtftokenizer.hxx>

class SvStream;

namespace writerfilter {
    namespace rtftok {
        /**
         * This acts like an importer, but used for looking ahead, e.g. to
         * determine if the current group contains a table, etc.
         */
        class RTFLookahead : public RTFListener
        {
            public:
                RTFLookahead(SvStream& rStream, sal_Size nGroupStart);
                virtual ~RTFLookahead();
                virtual int dispatchDestination(RTFKeyword nKeyword) SAL_OVERRIDE;
                virtual int dispatchFlag(RTFKeyword nKeyword) SAL_OVERRIDE;
                virtual int dispatchSymbol(RTFKeyword nKeyword) SAL_OVERRIDE;
                virtual int dispatchToggle(RTFKeyword nKeyword, bool bParam, int nParam) SAL_OVERRIDE;
                virtual int dispatchValue(RTFKeyword nKeyword, int nParam) SAL_OVERRIDE;
                virtual int resolveChars(char ch) SAL_OVERRIDE;
                virtual int pushState() SAL_OVERRIDE;
                virtual int popState() SAL_OVERRIDE;
                virtual RTFDestinationState getDestinationState() SAL_OVERRIDE;
                virtual void setDestinationState(RTFDestinationState nDestinationState) SAL_OVERRIDE;
                virtual RTFInternalState getInternalState() SAL_OVERRIDE;
                virtual void setInternalState(RTFInternalState nInternalState) SAL_OVERRIDE;
                virtual bool getSkipUnknown() SAL_OVERRIDE;
                virtual void setSkipUnknown(bool bSkipUnknown) SAL_OVERRIDE;
                virtual void finishSubstream() SAL_OVERRIDE;
                virtual bool isSubstream() const SAL_OVERRIDE;
                bool hasTable();
            private:
                boost::shared_ptr<RTFTokenizer> m_pTokenizer;
                SvStream& m_rStream;
                bool m_bHasTable;
        };
    } // namespace rtftok
} // namespace writerfilter

#endif // INCLUDED_WRITERFILTER_SOURCE_RTFTOK_RTFLOOKAHEAD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
