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

#ifndef INCLUDED_SW_INC_PVPRTDAT_HXX
#define INCLUDED_SW_INC_PVPRTDAT_HXX

#include <sal/types.h>

class SwPagePreviewPrtData
{

    sal_Int32 m_nLeftSpace, m_nRightSpace, m_nTopSpace, m_nBottomSpace,
            m_nHorzSpace, m_nVertSpace;
    sal_Int16 m_nRow, m_nCol;
    bool m_bLandscape : 1;
public:
    SwPagePreviewPrtData()
        : m_nLeftSpace(0), m_nRightSpace(0), m_nTopSpace(0), m_nBottomSpace(0),
            m_nHorzSpace(0), m_nVertSpace(0), m_nRow(1), m_nCol(1),
            m_bLandscape(false)
    {}

    sal_Int32 GetLeftSpace() const          { return m_nLeftSpace; }
    void SetLeftSpace( sal_Int32 n )        { m_nLeftSpace = n; }

    sal_Int32 GetRightSpace() const         { return m_nRightSpace; }
    void SetRightSpace( sal_Int32 n )       { m_nRightSpace = n; }

    sal_Int32 GetTopSpace() const           { return m_nTopSpace; }
    void SetTopSpace( sal_Int32 n )         { m_nTopSpace = n; }

    sal_Int32 GetBottomSpace() const        { return m_nBottomSpace; }
    void SetBottomSpace(sal_Int32 n )       { m_nBottomSpace = n; }

    sal_Int32 GetHorzSpace() const          { return m_nHorzSpace; }
    void SetHorzSpace( sal_Int32 n )        { m_nHorzSpace = n; }

    sal_Int32 GetVertSpace() const          { return m_nVertSpace; }
    void SetVertSpace( sal_Int32 n )        { m_nVertSpace = n; }

    sal_Int16 GetRow() const                { return m_nRow; }
    void SetRow(sal_Int16 n )               { m_nRow = n; }

    sal_Int16 GetCol() const                { return m_nCol; }
    void SetCol( sal_Int16 n )              { m_nCol = n; }

    bool GetLandscape() const               { return m_bLandscape; }
    void SetLandscape( bool b )         { m_bLandscape = b; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
