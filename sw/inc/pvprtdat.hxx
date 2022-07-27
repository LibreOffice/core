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

    Color m_nLeftSpace, m_nRightSpace, m_nTopSpace, m_nBottomSpace,
            m_nHorzSpace, m_nVertSpace;
    sal_uInt8 m_nRow, m_nCol;
    bool m_bLandscape : 1;
public:
    SwPagePreviewPrtData()
        : m_nLeftSpace(0), m_nRightSpace(0), m_nTopSpace(0), m_nBottomSpace(0),
            m_nHorzSpace(0), m_nVertSpace(0), m_nRow(1), m_nCol(1),
            m_bLandscape(false)
    {}

    Color GetLeftSpace() const          { return m_nLeftSpace; }
    void SetLeftSpace( Color n )        { m_nLeftSpace = n; }

    Color GetRightSpace() const         { return m_nRightSpace; }
    void SetRightSpace( Color n )       { m_nRightSpace = n; }

    Color GetTopSpace() const           { return m_nTopSpace; }
    void SetTopSpace( Color n )         { m_nTopSpace = n; }

    Color GetBottomSpace() const        { return m_nBottomSpace; }
    void SetBottomSpace(Color n )       { m_nBottomSpace = n; }

    Color GetHorzSpace() const          { return m_nHorzSpace; }
    void SetHorzSpace( Color n )        { m_nHorzSpace = n; }

    Color GetVertSpace() const          { return m_nVertSpace; }
    void SetVertSpace( Color n )        { m_nVertSpace = n; }

    Color GetRow() const                { return m_nRow; }
    void SetRow(sal_uInt8 n )               { m_nRow = n; }

    Color GetCol() const                { return m_nCol; }
    void SetCol( sal_uInt8 n )              { m_nCol = n; }

    bool GetLandscape() const               { return m_bLandscape; }
    void SetLandscape( bool b )         { m_bLandscape = b; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
