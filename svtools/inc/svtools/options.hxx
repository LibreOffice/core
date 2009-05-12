/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: options.hxx,v $
 * $Revision: 1.3 $
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

#ifndef INCLUDED_SVTOOLS_OPTIONS_HXX
#define INCLUDED_SVTOOLS_OPTIONS_HXX

#include "sal/config.h"
#include "svtools/svldllapi.h"

namespace svt { namespace detail {

// A base class for the various option classes supported by
// svtools/source/config/itemholderbase.hxx (which must be public, as it is
// shared between svl and svt):
class SVL_DLLPUBLIC Options {
public:
    Options();

    virtual ~Options() = 0;

private:
    SVL_DLLPRIVATE Options(Options &); // not defined
    SVL_DLLPRIVATE void operator =(Options &); // not defined
};

} }

#endif
