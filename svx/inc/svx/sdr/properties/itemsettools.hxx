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

#ifndef _SDR_PROPERTIES_ITEMSETTOOLS_HXX
#define _SDR_PROPERTIES_ITEMSETTOOLS_HXX

#include <sal/types.h>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

class SdrObject;
class SfxItemSet;
class Rectangle;
class Fraction;

//////////////////////////////////////////////////////////////////////////////
// class to remember broadcast start positions
namespace sdr
{
    namespace properties
    {
        class ItemChangeBroadcaster
        {
            sal_uInt32                                      mnCount;
            void*                                           mpData;

        public:
            ItemChangeBroadcaster(const SdrObject& rObj);
            ~ItemChangeBroadcaster();

            sal_uInt32 GetRectangleCount() const;
            const Rectangle& GetRectangle(sal_uInt32 nIndex) const;
        };
    } // end of namespace properties
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        void ScaleItemSet(SfxItemSet& rSet, const Fraction& rScale);
    } // end of namespace properties
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_PROPERTIES_ITEMSETTOOLS_HXX

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
