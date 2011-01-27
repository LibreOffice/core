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
    USHORT      nAlign;
    USHORT      nColCount;
    USHORT      nAllCols;
    USHORT      nWidthPercent;
    BOOL        bComplex : 1;
    BOOL        bLineSelected : 1;
    BOOL        bWidthChanged : 1;
    BOOL        bColsChanged : 1;

public:
    SwTableRep( const SwTabCols& rTabCol, BOOL bComplex );
    ~SwTableRep();

    BOOL        FillTabCols( SwTabCols& rTabCol ) const;

    SwTwips     GetLeftSpace() const            {return nLeftSpace;}
    void        SetLeftSpace(SwTwips nSet)      {nLeftSpace = nSet;}

    SwTwips     GetRightSpace() const           {return nRightSpace;}
    void        SetRightSpace(SwTwips nSet)     {nRightSpace = nSet;}

    SwTwips     GetWidth() const                {return nTblWidth;}
    void        SetWidth(SwTwips nSet)          {nTblWidth = nSet;}

    USHORT      GetWidthPercent() const         {return nWidthPercent;}
    void        SetWidthPercent(USHORT nSet)    {nWidthPercent = nSet;}

    USHORT      GetAlign() const                {return nAlign;}
    void        SetAlign(USHORT nSet)           {nAlign = nSet;}

    BOOL        IsComplex() const               {return bComplex;}
    USHORT      GetColCount() const             {return nColCount;}
    USHORT      GetAllColCount() const          {return nAllCols;}

    BOOL        HasColsChanged() const          {return bColsChanged;}
    void        SetColsChanged()                {bColsChanged = TRUE;}

    BOOL        HasWidthChanged() const         {return bWidthChanged;}
    void        SetWidthChanged()               {bWidthChanged  = TRUE;}

    BOOL        IsLineSelected() const          {return bLineSelected;}
    void        SetLineSelected(BOOL bSet)      {bLineSelected = bSet;}

    SwTwips     GetSpace() const                { return nSpace;}
    void        SetSpace(SwTwips nSet)          {nSpace = nSet;}

    TColumn*    GetColumns() const              {return pTColumns;}
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
