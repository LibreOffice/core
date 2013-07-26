/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _LIBLIBREOFFICE_HXX
#define _LIBLIBREOFFICE_HXX

class LODocument
{
public:
    virtual ~LODocument() {}

    virtual bool saveAs (const char *url) = 0;
};

class LibLibreOffice
{
public:
    virtual ~LibLibreOffice () {};

    virtual bool        initialize (const char *installPath) = 0;

    virtual LODocument *documentLoad (const char *url) = 0;

    // return the last error as a string, free me.
    virtual char       *getError() = 0;

};

LibLibreOffice *lo_init (const char *install_path);

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
