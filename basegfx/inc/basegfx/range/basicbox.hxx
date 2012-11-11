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

#ifndef _BGFX_RANGE_BASICBOX_HXX
#define _BGFX_RANGE_BASICBOX_HXX

#include <basegfx/range/basicrange.hxx>
#include <basegfx/basegfxdllapi.h>


namespace basegfx
{
    /** Explicitely different from BasicRange, handling the inside predicates
        differently.

        This is modelled after how polygon fill algorithms set pixel -
        typically excluding rightmost and bottommost ones.
     */
    class BasicBox : public BasicRange< sal_Int32, Int32Traits >
    {
        typedef BasicRange< sal_Int32, Int32Traits > Base;
    public:
        BasicBox() {}

        explicit BasicBox( sal_Int32 nValue ) :
            Base( nValue )
        {
        }

        bool isEmpty() const
        {
            return mnMinimum >= mnMaximum;
        }

        double getCenter() const
        {
            if(isEmpty())
            {
                return 0.0;
            }
            else
            {
                return ((mnMaximum + mnMinimum - 1.0) / 2.0);
            }
        }

        using Base::isInside;

        bool isInside(sal_Int32 nValue) const
        {
            if(isEmpty())
            {
                return false;
            }
            else
            {
                return (nValue >= mnMinimum) && (nValue < mnMaximum);
            }
        }

        using Base::overlaps;

        bool overlaps(const BasicBox& rBox) const
        {
            if(isEmpty())
            {
                return false;
            }
            else
            {
                if(rBox.isEmpty())
                {
                    return false;
                }
                else
                {
                    return !((rBox.mnMaximum <= mnMinimum) || (rBox.mnMinimum >= mnMaximum));
                }
            }
        }

        void grow(sal_Int32 nValue)
        {
            if(!isEmpty())
            {
                bool bLessThanZero(nValue < 0);

                if(nValue > 0 || bLessThanZero)
                {
                    mnMinimum -= nValue;
                    mnMaximum += nValue;

                    if(bLessThanZero)
                    {
                        // test if range did collapse
                        if(mnMinimum > mnMaximum)
                        {
                            // if yes, collapse to center
                            mnMinimum = mnMaximum = ((mnMaximum + mnMinimum - 1) / 2);
                        }
                    }
                }
            }
        }
    };

} // end of namespace basegfx

#endif /* _BGFX_RANGE_BASICBOX_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
