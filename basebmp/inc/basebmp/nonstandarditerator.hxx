/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: nonstandarditerator.hxx,v $
 * $Revision: 1.2 $
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

#ifndef INCLUDED_BASEBMP_NONSTANDARDITERATOR_HXX
#define INCLUDED_BASEBMP_NONSTANDARDITERATOR_HXX

#include <vigra/metaprogramming.hxx>

namespace basebmp
{
    /// Base class defining pointer and reference types as VigraFalseType
    struct NonStandardIterator
    {
        typedef vigra::VigraFalseType reference;
        typedef vigra::VigraFalseType index_reference;
        typedef vigra::VigraFalseType pointer;
    };

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_NONSTANDARDITERATOR_HXX */
