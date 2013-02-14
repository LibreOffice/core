/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _SD_IMPRESSREMOTE_OSXBLUETOOTHWRAPPER_HXX
#define _SD_IMPRESSREMOTE_OSXBLUETOOTHWRAPPER_HXX

#include <rtl/string.hxx>
#include <osl/conditn.hxx>
#include <osl/mutex.hxx>
#include <vector>

#include "IBluetoothSocket.hxx"

namespace sd
{
    class OSXBluetoothWrapper : public IBluetoothSocket
    {
        IOBluetoothRFCOMMChannel* mpChannel;
        int                       mnMTU;
        osl::Condition            mHaveBytes;
        osl::Mutex                mMutex;
        std::vector<char>         mBuffer;

    public:
        OSXBluetoothWrapper( IOBluetoothRFCOMMChannel* channel );
        virtual sal_Int32 readLine( rtl::OString& aLine );
        virtual sal_Int32 write( const void* pBuffer, sal_uInt32 len );
        void appendData(void* pBuffer, size_t len );
    };
}

#endif // _SD_IMPRESSREMOTE_OSXBLUETOOTHWRAPPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
