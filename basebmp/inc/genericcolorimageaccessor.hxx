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

#ifndef INCLUDED_BASEBMP_INC_GENERICCOLORIMAGEACCESSOR_HXX
#define INCLUDED_BASEBMP_INC_GENERICCOLORIMAGEACCESSOR_HXX

#include <basebmp/color.hxx>
#include <basebmp/bitmapdevice.hxx>

namespace basebmp
{
    /** Access a BitmapDevice generically

        This accessor deals with an opaque BitmapDevice generically,
        via getPixel()/setPixel() at the published interface.
     */
    class GenericColorImageAccessor
    {
        BitmapDeviceSharedPtr mpDevice;

    public:
        typedef Color value_type;

        explicit GenericColorImageAccessor( BitmapDeviceSharedPtr const& rTarget ) :
            mpDevice(rTarget)
        {}

        template< typename Iterator >
        Color operator()( Iterator const& i ) const
        { return mpDevice->getPixel( basegfx::B2IPoint( i->x,i->y ) ); }

        template< typename Iterator, typename Difference >
        Color operator()( Iterator const& i, Difference const& diff) const
        { return mpDevice->getPixel( basegfx::B2IPoint( i[diff]->x,
                                                        i[diff]->y ) ); }

    };
}

#endif /* INCLUDED_BASEBMP_INC_GENERICCOLORIMAGEACCESSOR_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
