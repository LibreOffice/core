/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _RTFLOOKAHEAD_HXX_
#define _RTFLOOKAHEAD_HXX_

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
                virtual int dispatchDestination(RTFKeyword nKeyword);
                virtual int dispatchFlag(RTFKeyword nKeyword);
                virtual int dispatchSymbol(RTFKeyword nKeyword);
                virtual int dispatchToggle(RTFKeyword nKeyword, bool bParam, int nParam);
                virtual int dispatchValue(RTFKeyword nKeyword, int nParam);
                virtual int resolveChars(char ch);
                virtual int pushState();
                virtual int popState();
                virtual RTFDestinationState getDestinationState();
                virtual void setDestinationState(RTFDestinationState nDestinationState);
                virtual RTFInternalState getInternalState();
                virtual void setInternalState(RTFInternalState nInternalState);
                virtual bool getSkipUnknown();
                virtual void setSkipUnknown(bool bSkipUnknown);
                virtual void finishSubstream();
                virtual bool isSubstream() const;
                bool hasTable();
            private:
                boost::shared_ptr<RTFTokenizer> m_pTokenizer;
                SvStream& m_rStream;
                bool m_bHasTable;
        };
    } // namespace rtftok
} // namespace writerfilter

#endif // _RTFDOCUMENTIMPL_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
