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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include <vcl/virdev.hxx>

namespace PictReaderShape {
  /** draws a line from orig to dest knowing penSize

  Attention: in order to draw horizontal/vertical/small lines, this function can instead draw a rectangle or
            a polygon. In this case, we retrieve the line informations from VirtualDev ( GetLineColor )
        and we use them as fill informations ( SetFillColor ). We restore after the VirtualDev state.

        This implies also that this function must be modified if we use real pattern to draw these primitives.
   */
  void drawLine(VirtualDevice *dev, Point const &orig, Point const &dest, Size const &pSize);

  /** draws a rectangle knowing penSize */
  void drawRectangle(VirtualDevice *dev, bool drawFrame, Rectangle const &rect, Size const &pSize);

  /** draws a polygon knowing penSize */
  void drawPolygon(VirtualDevice *dev, bool drawFrame, Polygon const &rect, Size const &pSize);

  /** draws an ellipse knowing penSize */
  void drawEllipse(VirtualDevice *dev, bool drawFrame, Rectangle const &orig, Size const &pSize);

  /** draws a rounded rectangle knowing penSize
      \note ovalSize is two time the size of the corner
  */
  void drawRoundRectangle(VirtualDevice *dev, bool drawFrame, Rectangle const &orig, Size const &ovalS, Size const &pSize);

  /** draws an arc in a b2dpolygon knowing penSize
      \note  - it supposes that angl1 < angl2
  */
  void drawArc(VirtualDevice *dev, bool drawFrame, Rectangle const &orig, const double& angle1, const double& angle2, Size const &pSize);
}
