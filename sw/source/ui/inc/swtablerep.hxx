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
#ifndef _SWTABLEREP_HXX
#define _SWTABLEREP_HXX


#include <sfx2/tabdlg.hxx>
#include "swdllapi.h"
#include "swtypes.hxx"

class SwTabCols;
struct TColumn;
class SW_DLLPUBLIC SwTableRep
{
    TColumn*    pTColumns;

    SwTwips     nTblWidth;
    SwTwips     nSpace;
    SwTwips     nLeftSpace;
    SwTwips     nRightSpace;
    sal_uInt16      nAlign;
    sal_uInt16      nColCount;
    sal_uInt16      nAllCols;
    sal_uInt16      nWidthPercent;
    sal_Bool        bComplex : 1;
    sal_Bool        bLineSelected : 1;
    sal_Bool        bWidthChanged : 1;
    sal_Bool        bColsChanged : 1;

public:
    SwTableRep( const SwTabCols& rTabCol, sal_Bool bComplex );
    ~SwTableRep();

    sal_Bool        FillTabCols( SwTabCols& rTabCol ) const;

    SwTwips     GetLeftSpace() const            {return nLeftSpace;}
    void        SetLeftSpace(SwTwips nSet)      {nLeftSpace = nSet;}

    SwTwips     GetRightSpace() const           {return nRightSpace;}
    void        SetRightSpace(SwTwips nSet)     {nRightSpace = nSet;}

    SwTwips     GetWidth() const                {return nTblWidth;}
    void        SetWidth(SwTwips nSet)          {nTblWidth = nSet;}

    sal_uInt16      GetWidthPercent() const         {return nWidthPercent;}
    void        SetWidthPercent(sal_uInt16 nSet)    {nWidthPercent = nSet;}

    sal_uInt16      GetAlign() const                {return nAlign;}
    void        SetAlign(sal_uInt16 nSet)           {nAlign = nSet;}

    sal_Bool        IsComplex() const               {return bComplex;}
    sal_uInt16      GetColCount() const             {return nColCount;}
    sal_uInt16      GetAllColCount() const          {return nAllCols;}

    sal_Bool        HasColsChanged() const          {return bColsChanged;}
    void        SetColsChanged()                {bColsChanged = sal_True;}

    sal_Bool        HasWidthChanged() const         {return bWidthChanged;}
    void        SetWidthChanged()               {bWidthChanged  = sal_True;}

    sal_Bool        IsLineSelected() const          {return bLineSelected;}
    void        SetLineSelected(sal_Bool bSet)      {bLineSelected = bSet;}

    SwTwips     GetSpace() const                { return nSpace;}
    void        SetSpace(SwTwips nSet)          {nSpace = nSet;}

    TColumn*    GetColumns() const              {return pTColumns;}
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
