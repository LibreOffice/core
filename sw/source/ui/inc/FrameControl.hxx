/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _FRAMECONTROL_HXX
#define _FRAMECONTROL_HXX

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
