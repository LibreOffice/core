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

#include <memory>
#include <swdllapi.h>
#include <swtypes.hxx>

class SwTabCols;

struct TColumn
{
    SwTwips nWidth;
    bool    bVisible;
};

class SW_DLLPUBLIC SwTableRep
{
    std::vector<TColumn> m_aTColumns;

    SwTwips     m_nTableWidth;
    SwTwips     m_nSpace;
    SwTwips     m_nLeftSpace;
    SwTwips     m_nRightSpace;
    sal_uInt16      m_nAlign;
    sal_uInt16      m_nColCount;
    sal_uInt16      m_nAllCols;
    sal_uInt16      m_nWidthPercent;
    bool        m_bLineSelected : 1;
    bool        m_bWidthChanged : 1;
    bool        m_bColsChanged : 1;

public:
    SwTableRep( const SwTabCols& rTabCol );
    ~SwTableRep();

    SwTableRep( const SwTableRep& rCopy ) = default;
    SwTableRep( SwTableRep&& rCopy ) = default;
    SwTableRep& operator=(const SwTableRep& rCopy) = default;
    SwTableRep& operator=(SwTableRep&& rCopy) = default;

    bool        FillTabCols( SwTabCols& rTabCol ) const;

    SwTwips     GetLeftSpace() const            {return m_nLeftSpace;}
    void        SetLeftSpace(SwTwips nSet)      {m_nLeftSpace = nSet;}

    SwTwips     GetRightSpace() const           {return m_nRightSpace;}
    void        SetRightSpace(SwTwips nSet)     {m_nRightSpace = nSet;}

    SwTwips     GetWidth() const                {return m_nTableWidth;}
    void        SetWidth(SwTwips nSet)          {m_nTableWidth = nSet;}

    sal_uInt16      GetWidthPercent() const         {return m_nWidthPercent;}
    void        SetWidthPercent(sal_uInt16 nSet)    {m_nWidthPercent = nSet;}

    sal_uInt16      GetAlign() const                {return m_nAlign;}
    void        SetAlign(sal_uInt16 nSet)           {m_nAlign = nSet;}

    sal_uInt16      GetColCount() const             {return m_nColCount;}
    sal_uInt16      GetAllColCount() const          {return m_nAllCols;}

    bool        HasColsChanged() const          {return m_bColsChanged;}
    void        SetColsChanged()                {m_bColsChanged = true;}

    bool        HasWidthChanged() const         {return m_bWidthChanged;}
    void        SetWidthChanged()               {m_bWidthChanged  = true;}

    bool        IsLineSelected() const          {return m_bLineSelected;}
    void        SetLineSelected(bool bSet)      {m_bLineSelected = bSet;}

    SwTwips     GetSpace() const                { return m_nSpace;}
    void        SetSpace(SwTwips nSet)          {m_nSpace = nSet;}

    TColumn*    GetColumns()                    {return m_aTColumns.data();}
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
