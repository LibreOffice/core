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

#ifndef VCL_SCOPEDBITMAPACCESS_HXX_INCLUDED
#define VCL_SCOPEDBITMAPACCESS_HXX_INCLUDED

namespace vcl
{

/** This template handles BitmapAccess the RAII way.

    Please don't use directly, but through the ready-made typedefs
    ScopedReadAccess and ScopedWriteAccess in classes Bitmap and
    AlphaMask.

    Use as follows:
    Bitmap aBitmap;
    Bitmap::ScopedReadAccess pReadAccess( aBitmap );
    pReadAccess->SetPixel()...

    Bitmap aBitmap2;
    Bitmap::ScopedWriteAccess pWriteAccess( bCond ? aBitmap2.AcquireWriteAccess() : 0, aBitmap2 );
    if ( pWriteAccess )...

    @attention for practical reasons, ScopedBitmapAccess stores a
    reference to the provided bitmap, thus, make sure that the bitmap
    specified at construction time lives at least as long as the
    ScopedBitmapAccess.
 */
template < class Access, class Bitmap, Access* (Bitmap::* Acquire)() > class ScopedBitmapAccess
{
    typedef ScopedBitmapAccess< Access, Bitmap, Acquire > self_type;
    typedef bool (self_type::* unspecified_bool_type)() const;

public:
    explicit ScopedBitmapAccess( Bitmap& rBitmap ) :
        mpAccess( 0 ),
        mrBitmap( rBitmap )
    {
        mpAccess = (mrBitmap.*Acquire)();
    }

    ScopedBitmapAccess( Access* pAccess, Bitmap& rBitmap ) :
        mpAccess( pAccess ),
        mrBitmap( rBitmap )
    {
    }

    ~ScopedBitmapAccess()
    {
        mrBitmap.ReleaseAccess( mpAccess );
    }

    bool operator!() const { return !mpAccess; }
    operator unspecified_bool_type() const
    {
        return mpAccess ? &self_type::operator! : 0;
    }

    Access*         get() { return mpAccess; }
    const Access*   get() const { return mpAccess; }

    Access*         operator->() { return mpAccess; }
    const Access*   operator->() const { return mpAccess; }

    Access&         operator*() { return *mpAccess; }
    const Access&   operator*() const { return *mpAccess; }

private:
    Access*     mpAccess;
    Bitmap&     mrBitmap;
};

}

#endif // VCL_SCOPEDBITMAPACCESS_HXX_INCLUDED

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
