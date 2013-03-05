/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _LIBLIBREOFFICE_IMPL_HXX
#define _LIBLIBREOFFICE_IMPL_HXX

#include "liblibreoffice.hxx"

typedef int loboolean;
typedef struct _LODocument LODocument;

class LibLibreOffice_Impl : public LibLibreOffice
{
public:
    virtual loboolean initialize (const char *installPath);

    virtual LODocument *documentLoad (const char *url);
    virtual loboolean   documentSave (const char *url);

    virtual ~LibLibreOffice_Impl ();
};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
