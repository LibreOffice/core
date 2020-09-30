/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_SKIA_INTERFACE_HXX
#define INCLUDED_VCL_SKIA_INTERFACE_HXX

#include <vcl/dllapi.h>

#include <config_features.h>

#if HAVE_FEATURE_SKIA

class SkMatrix;
class SkPath;
class BitmapEx;

/**
Interface class giving direct access to Skia features of an output device.
*/
class VCL_DLLPUBLIC SkiaOutDevInterface
{
public:
    virtual ~SkiaOutDevInterface();
};

#endif // HAVE_FEATURE_SKIA

#endif // INCLUDED_VCL_SKIA_INTERFACE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
