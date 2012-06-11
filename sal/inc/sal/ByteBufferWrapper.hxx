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
  JNIEnv *env;
  jobject object;

public:
  ByteBufferWrapper(JNIEnv *e, jobject o) :
    env(e)
  {
    object = env->NewGlobalRef(o);
  }

  sal_uInt8* pointer()
  {
    return (sal_uInt8 *) env->GetDirectBufferAddress(object);
  }

  void operator()(sal_uInt8 * /* p */)
  {
    env->DeleteGlobalRef(object);
  }
};

}; }; };

#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
