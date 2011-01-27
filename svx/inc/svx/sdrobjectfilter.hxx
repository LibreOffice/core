/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
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
************************************************************************/

#ifndef SDROBJECTFILTER_HXX
#define SDROBJECTFILTER_HXX

#include "svx/svxdllapi.h"

class SdrObject;

//........................................................................
namespace svx
{
//........................................................................

    //====================================================================
    //= SdrObjectFilter
    //====================================================================
    /** specifies a boolean predicate on the set of all SdrObjects - vulgo a filter.
    */
    class SVX_DLLPUBLIC SAL_NO_VTABLE ISdrObjectFilter
    {
    public:
        virtual bool    includeObject( const SdrObject& i_rObject ) const = 0;

        virtual         ~ISdrObjectFilter() = 0;
    };

//........................................................................
} // namespace svx
//........................................................................

#endif // SDROBJECTFILTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
