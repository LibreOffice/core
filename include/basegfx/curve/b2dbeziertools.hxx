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

#ifndef INCLUDED_BASEGFX_CURVE_B2DBEZIERTOOLS_HXX
#define INCLUDED_BASEGFX_CURVE_B2DBEZIERTOOLS_HXX

#include <sal/types.h>
#include <vector>
#include <basegfx/basegfxdllapi.h>


// predefines

namespace basegfx
{
    class B2DCubicBezier;
}



namespace basegfx
{
    class BASEGFX_DLLPUBLIC B2DCubicBezierHelper
    {
    private:
        ::std::vector< double >         maLengthArray;
        sal_uInt32                      mnEdgeCount;

    public:
        explicit B2DCubicBezierHelper(const B2DCubicBezier& rBase, sal_uInt32 nDivisions = 9);

        double getLength() const { if(!maLengthArray.empty()) return maLengthArray[maLengthArray.size() - 1]; else return 0.0; }
        double distanceToRelative(double fDistance) const;
    };
} // end of namespace basegfx



#endif // INCLUDED_BASEGFX_CURVE_B2DBEZIERTOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
