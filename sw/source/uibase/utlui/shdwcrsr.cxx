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

#include <com/sun/star/text/HoriOrientation.hpp>
#include <shdwcrsr.hxx>
#include <vcl/ptrstyle.hxx>

using namespace ::com::sun::star;

SwShadowCursor::~SwShadowCursor()
{
    if( USHRT_MAX != m_nOldMode )
        DrawCursor( m_nOldMode);
}

void SwShadowCursor::SetPos( const Point& rPt, tools::Long nHeight, sal_uInt16 nMode )
{
    Point aPt( m_pWin->LogicToPixel( rPt ));
    nHeight = m_pWin->LogicToPixel( Size( 0, nHeight )).Height();
    if( m_aOldPt != aPt || m_nOldHeight != nHeight || m_nOldMode != nMode )
    {
        if( USHRT_MAX != m_nOldMode )
            DrawCursor( m_nOldMode);

        DrawCursor( nMode);
        m_nOldMode = nMode;
        m_nOldHeight = nHeight;
        m_aOldPt = aPt;
    }
}

void SwShadowCursor::DrawCursor( sal_uInt16 nMode )
{
    if( text::HoriOrientation::LEFT == nMode )    // Arrow to the right
        m_pWin->SetPointer(PointerStyle::AutoScrollE);
    else   // Arrow to the left
        m_pWin->SetPointer(PointerStyle::AutoScrollW);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
