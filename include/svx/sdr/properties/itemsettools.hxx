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
            explicit ItemChangeBroadcaster(const SdrObject& rObj);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
