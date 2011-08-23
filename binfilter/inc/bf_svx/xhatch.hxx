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

#ifndef _SVX_XHATCH_HXX
#define _SVX_XHATCH_HXX

#include <bf_svx/xenum.hxx>

#include <tools/color.hxx>
namespace binfilter {

//--------------
// class XHatch
//--------------

class XHatch
{
protected:
    XHatchStyle		eStyle;
    Color			aColor;
    long			nDistance;
    long			nAngle;

public:
                    XHatch() {}
                    XHatch(const Color& rCol, XHatchStyle eStyle = XHATCH_SINGLE,
                           long nDistance = 20, long nAngle = 0);

    int operator==(const XHatch& rHatch) const;

    void			SetHatchStyle(XHatchStyle eNewStyle) { eStyle = eNewStyle; }
    void			SetColor(const Color& rColor) { aColor = rColor; }
    void			SetDistance(long nNewDistance) { nDistance = nNewDistance; }
    void			SetAngle(long nNewAngle) { nAngle = nNewAngle; }

    XHatchStyle		GetHatchStyle() const { return eStyle; }
    Color			GetColor() const { return aColor; }
    long			GetDistance() const { return nDistance; }
    long			GetAngle() const { return nAngle; }
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
