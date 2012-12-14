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

#ifndef SC_DPCONTROL_HXX
#define SC_DPCONTROL_HXX

#include "rtl/ustring.hxx"
#include "tools/gen.hxx"
#include "tools/fract.hxx"

#include <boost/shared_ptr.hpp>
#include <memory>

class OutputDevice;
class Point;
class Size;
class StyleSettings;
class Window;
class ScDocument;

/**
 * This class takes care of physically drawing field button controls inside
 * data pilot tables.
 */
class ScDPFieldButton
{
public:
    ScDPFieldButton(OutputDevice* pOutDev, const StyleSettings* pStyle, const Fraction* pZoomX = NULL, const Fraction* pZoomY = NULL,
                    ScDocument* pDoc = NULL);
    ~ScDPFieldButton();

    void setText(const ::rtl::OUString& rText);
    void setBoundingBox(const Point& rPos, const Size& rSize, bool bLayoutRTL);
    void setDrawBaseButton(bool b);
    void setDrawPopupButton(bool b);
    void setHasHiddenMember(bool b);
    void setPopupPressed(bool b);
    void setPopupLeft(bool b);
    void draw();

    void getPopupBoundingBox(Point& rPos, Size& rSize) const;

private:
    void drawPopupButton();

private:
    Point                   maPos;
    Size                    maSize;
    ::rtl::OUString         maText;
    Fraction                maZoomX;
    Fraction                maZoomY;
    ScDocument*             mpDoc;
    OutputDevice*           mpOutDev;
    const StyleSettings*    mpStyle;
    bool                    mbBaseButton;
    bool                    mbPopupButton;
    bool                    mbHasHiddenMember;
    bool                    mbPopupPressed;
    bool                    mbPopupLeft;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
