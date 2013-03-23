/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * [ Copyright (C) 2011 SUSE <cbosdonnat@suse.com> (initial developer) ]
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */
#pragma once
#if 1

class SwEditWin;
class SwFrm;
class Point;

/** Class representing a control linked to a SwFrm.
  */
class SwFrameControl
{
    SwEditWin*            m_pEditWin;
    const SwFrm*          m_pFrm;

public:
    SwFrameControl( SwEditWin* pEditWin, const SwFrm* pFrm ) :
        m_pEditWin( pEditWin ), m_pFrm( pFrm ) {};
    virtual ~SwFrameControl( ) {};

    SwFrameControl( const SwFrameControl& rCopy ) :
        m_pEditWin( rCopy.m_pEditWin ),
        m_pFrm( rCopy.m_pFrm )
    {
    }

    const SwFrameControl& operator=( const SwFrameControl& rCopy )
    {
        m_pEditWin = rCopy.m_pEditWin;
        m_pFrm = rCopy.m_pFrm;
        return *this;
    }

    const SwFrm* GetFrame( ) { return m_pFrm; }
    SwEditWin*   GetEditWin( ) { return m_pEditWin; }

    virtual void SetReadonly( bool bReadonly ) = 0;
    virtual void ShowAll( bool bShow ) = 0;

    /// Returns true if the point is inside the control.
    virtual bool Contains( const Point &rDocPt ) const = 0;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
