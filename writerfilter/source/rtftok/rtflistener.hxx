/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _RTFLISTENER_HXX_
#define _RTFLISTENER_HXX_

#include <rtfcontrolwords.hxx>

namespace writerfilter {
    namespace rtftok {
        enum RTFInternalState
        {
            INTERNAL_NORMAL,
            INTERNAL_BIN,
            INTERNAL_HEX
        };

        enum RTFErrors
        {
            ERROR_OK,
            ERROR_GROUP_UNDER,
            ERROR_GROUP_OVER,
            ERROR_EOF,
            ERROR_HEX_INVALID,
            ERROR_CHAR_OVER
        };

        /**
         * RTFTokenizer needs a class implementing this this interface. While
         * RTFTokenizer separates control words (and their arguments) from
         * text, the class implementing this interface is expected to map the
         * raw RTF tokens to dmapper tokens.
         */
        class RTFListener
        {
            public:
                virtual ~RTFListener() { }
                // Dispatching of control words and characters.
                virtual int dispatchDestination(RTFKeyword nKeyword) = 0;
                virtual int dispatchFlag(RTFKeyword nKeyword) = 0;
                virtual int dispatchSymbol(RTFKeyword nKeyword) = 0;
                virtual int dispatchToggle(RTFKeyword nKeyword, bool bParam, int nParam) = 0;
                virtual int dispatchValue(RTFKeyword nKeyword, int nParam) = 0;
                virtual int resolveChars(char ch) = 0;

                // State handling.
                virtual int pushState() = 0;
                virtual int popState() = 0;

                virtual RTFDestinationState getDestinationState() = 0;
                virtual void setDestinationState(RTFDestinationState nDestinationState) = 0;
                virtual RTFInternalState getInternalState() = 0;
                virtual void setInternalState(RTFInternalState nInternalState) = 0;
                virtual bool getSkipUnknown() = 0;
                virtual void setSkipUnknown(bool bSkipUnknown) = 0;

                // Substream handling.
                virtual void finishSubstream() = 0;
                virtual bool isSubstream() const = 0;
        };
    } // namespace rtftok
} // namespace writerfilter

#endif // _RTFLISTENER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
