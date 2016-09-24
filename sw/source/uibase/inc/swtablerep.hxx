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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_SWTABLEREP_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_SWTABLEREP_HXX

#include <sfx2/tabdlg.hxx>
#include "swdllapi.h"
#include "swtypes.hxx"

class SwTabCols;
struct TColumn;
class SW_DLLPUBLIC SwTableRep
{
    TColumn*    pTColumns;

    SwTwips     nTableWidth;
    SwTwips     nSpace;
    SwTwips     nLeftSpace;
    SwTwips     nRightSpace;
    sal_uInt16      nAlign;
    sal_uInt16      nColCount;
    sal_uInt16      nAllCols;
    sal_uInt16      nWidthPercent;
    bool        bLineSelected : 1;
    bool        bWidthChanged : 1;
    bool        bColsChanged : 1;

public:
    SwTableRep( const SwTabCols& rTabCol );
    ~SwTableRep();

    bool        FillTabCols( SwTabCols& rTabCol ) const;

    SwTwips     GetLeftSpace() const            {return nLeftSpace;}
    void        SetLeftSpace(SwTwips nSet)      {nLeftSpace = nSet;}

    SwTwips     GetRightSpace() const           {return nRightSpace;}
    void        SetRightSpace(SwTwips nSet)     {nRightSpace = nSet;}

    SwTwips     GetWidth() const                {return nTableWidth;}
    void        SetWidth(SwTwips nSet)          {nTableWidth = nSet;}

    sal_uInt16      GetWidthPercent() const         {return nWidthPercent;}
    void        SetWidthPercent(sal_uInt16 nSet)    {nWidthPercent = nSet;}

    sal_uInt16      GetAlign() const                {return nAlign;}
    void        SetAlign(sal_uInt16 nSet)           {nAlign = nSet;}

    sal_uInt16      GetColCount() const             {return nColCount;}
    sal_uInt16      GetAllColCount() const          {return nAllCols;}

    bool        HasColsChanged() const          {return bColsChanged;}
    void        SetColsChanged()                {bColsChanged = true;}

    bool        HasWidthChanged() const         {return bWidthChanged;}
    void        SetWidthChanged()               {bWidthChanged  = true;}

    bool        IsLineSelected() const          {return bLineSelected;}
    void        SetLineSelected(bool bSet)      {bLineSelected = bSet;}

    SwTwips     GetSpace() const                { return nSpace;}
    void        SetSpace(SwTwips nSet)          {nSpace = nSet;}

    TColumn*    GetColumns() const              {return pTColumns;}
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
