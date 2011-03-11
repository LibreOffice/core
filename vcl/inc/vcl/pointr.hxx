/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _VCL_POINTR_HXX
#define _VCL_POINTR_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/ptrstyle.hxx>

class Point;

// -----------
// - Pointer -
// -----------

class VCL_DLLPUBLIC Pointer
{
    PointerStyle    meStyle;

public:
                    Pointer()
                        { meStyle = POINTER_ARROW; }
                    Pointer( PointerStyle eStyle )
                        { meStyle = eStyle; }

    PointerStyle    GetStyle() const { return meStyle; }

    sal_Bool            operator==( const Pointer& rPointer ) const
                        { return (meStyle == rPointer.meStyle); }
    sal_Bool            operator!=( const Pointer& rPointer ) const
                        { return !(Pointer::operator==( rPointer )); }
};

#endif // _VCL_POINTR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
