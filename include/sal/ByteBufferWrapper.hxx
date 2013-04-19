/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _SAL_BYTEBUFFERWRAPPER_HXX
#define _SAL_BYTEBUFFERWRAPPER_HXX

#ifdef ANDROID

#include <jni.h>

#include <sal/types.h>

namespace org { namespace libreoffice { namespace touch {

class ByteBufferWrapper
{
private:
    jobject object;

public:
    ByteBufferWrapper(JNIEnv *env, jobject o);

    sal_uInt8* pointer();

    void operator()(sal_uInt8 *p);
};

}; }; };

#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
