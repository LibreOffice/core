/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_AVMEDIA_SOURCE_INC_MEDIAMISC_HXX
#define INCLUDED_AVMEDIA_SOURCE_INC_MEDIAMISC_HXX

#include <config_features.h>
#include <tools/simplerm.hxx>

#define AVMEDIA_MANAGER_SERVICE_PREFERRED "com.sun.star.comp.avmedia.Manager_VLC"
#ifdef _WIN32
#define AVMEDIA_MANAGER_SERVICE_NAME      "com.sun.star.comp.avmedia.Manager_DirectX"
#else
#ifdef MACOSX
#define AVMEDIA_MANAGER_SERVICE_NAME      "com.sun.star.comp.avmedia.Manager_QuickTime"
#define AVMEDIA_MANAGER_SERVICE_NAME_FALLBACK1 "com.sun.star.comp.avmedia.Manager_MacAVF"
#else
#define AVMEDIA_MANAGER_SERVICE_NAME_OLD  "com.sun.star.comp.avmedia.Manager_GStreamer_0_10"
#define AVMEDIA_MANAGER_SERVICE_NAME      "com.sun.star.comp.avmedia.Manager_GStreamer"
#endif
#endif

#define AVMEDIA_OPENGL_MANAGER_SERVICE_NAME "com.sun.star.media.Manager_OpenGL"

// Mime types
#define AVMEDIA_MIMETYPE_COMMON           "application/vnd.sun.star.media"

#if HAVE_FEATURE_GLTF
#define AVMEDIA_MIMETYPE_JSON             "model/vnd.gltf+json"
#endif

namespace avmedia
{
    const std::locale& GetResLocale();
}

inline OUString AvmResId(const char* pId)
{
    return Translate::get(pId, ::avmedia::GetResLocale());
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
