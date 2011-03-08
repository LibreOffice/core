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
#ifndef _CHART2_VIEWELEMENTLISTPROVIDER_HXX
#define _CHART2_VIEWELEMENTLISTPROVIDER_HXX

#include <svx/xtable.hxx>
#include <svx/svdpage.hxx>

class FontList;

//.............................................................................
namespace chart
{
//.............................................................................

class DrawModelWrapper;

class ViewElementListProvider
{
public:
    ViewElementListProvider( DrawModelWrapper* pDrawModelWrapper );
    virtual ~ViewElementListProvider();

    XColorTable*    GetColorTable() const;
    XDashList*      GetDashList() const;
    XLineEndList*   GetLineEndList() const;
    XGradientList*  GetGradientList() const;
    XHatchList*     GetHatchList() const;
    XBitmapList*    GetBitmapList() const;

    //create chartspecific symbols for linecharts
    SdrObjList*     GetSymbolList() const;
    Graphic         GetSymbolGraphic( sal_Int32 nStandardSymbol, const SfxItemSet* pSymbolShapeProperties ) const;

    FontList*       getFontList() const;
    //SfxPrinter*   getPrinter();

private:
    DrawModelWrapper*   m_pDrawModelWrapper;
    mutable FontList*   m_pFontList;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
