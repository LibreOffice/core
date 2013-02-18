/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _SD_IMPRESSREMOTE_IBLUETOOTHSOCKET_HXX
#define _SD_IMPRESSREMOTE_IBLUETOOTHSOCKET_HXX

#include <rtl/string.hxx>
#include <boost/noncopyable.hpp>

namespace sd
{
    /** Interface for bluetooth data io
     */
    struct IBluetoothSocket : private boost::noncopyable
    {
        virtual ~IBluetoothSocket() {}

        /** Blocks until a line is read.

            @return whatever the last call of recv returned, i.e. 0 or less
                    if there was a problem in communications.
         */
        virtual sal_Int32 readLine(rtl::OString& aLine) = 0;

        /** Write a number of bytes

            @return number of bytes actually written
         */
        virtual sal_Int32 write( const void* pBuffer, sal_uInt32 n ) = 0;
    };
}

#endif // _SD_IMPRESSREMOTE_IBLUETOOTHSOCKET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
