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

#ifndef INCLUDED_VCL_SOURCE_FILTER_SGVSPLN_HXX
#define INCLUDED_VCL_SOURCE_FILTER_SGVSPLN_HXX

/*
 * Calculates the coefficients of parametrised natural or periodic cubic
 * polynom-splines. The corner points of the polygon passed are used as
 * support points. n returns the number of partial polynoms. This function
 * returns TRUE if no error occurred. Only in this case memory for the
 * coefficient array has been allocated, which can be freed by the caller
 * using a delete.
 */
bool CalcSpline(tools::Polygon& rPoly, bool Periodic, sal_uInt16& n,
                double*& ax, double*& ay, double*& bx, double*& by,
                double*& cx, double*& cy, double*& dx, double*& dy, double*& T);

/*
 * Converts a parametrised cubic spline (natural or periodic) to an approximate
 * polygon. The function returns false, if an error occurred during the
 * calculation of the coefficients or the polygon became too large (>PolyMax=16380).
 * In the first case the polygon has 0, in the second case PolyMax points.
 * To prevent coordinate overflows we limit them to +/-32000.
 */
bool Spline2Poly(tools::Polygon& rSpln, bool Periodic, tools::Polygon& rPoly);

#endif // INCLUDED_VCL_SOURCE_FILTER_SGVSPLN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
