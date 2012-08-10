/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _SD_IMPRESSREMOTE_BUFFEREDSTREAMSOCKET_HXX
#define _SD_IMPRESSREMOTE_BUFFEREDSTREAMSOCKET_HXX

#include <boost/noncopyable.hpp>
#include <osl/socket.hxx>
#include <vector>

#define CHARSET RTL_TEXTENCODING_UTF8
#define MAX_LINE_LENGTH 20000

namespace sd
{

    /**
     * A wrapper for an osl StreamSocket to allow reading lines.
     */
    class BufferedStreamSocket :
        public ::osl::StreamSocket,
        private ::boost::noncopyable
    {
        public:
            BufferedStreamSocket( const osl::StreamSocket &aSocket );
            BufferedStreamSocket( const BufferedStreamSocket &aSocket );
            /**
             * Blocks until a line is read.
             * Returns whatever the last call of recv returned, i.e. 0 or less
             * if there was a problem in communications.
             */
            sal_Int32 readLine(OString& aLine);
        private:
            sal_Int32 aRet, aRead;
            std::vector<char> aBuffer;
    };
}

#endif // _SD_IMPRESSREMOTE_BUFFEREDSTREAMSOCKET_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
