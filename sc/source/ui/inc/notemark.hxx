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

#ifndef INCLUDED_SC_SOURCE_UI_INC_NOTEMARK_HXX
#define INCLUDED_SC_SOURCE_UI_INC_NOTEMARK_HXX

#include <vcl/mapmod.hxx>
#include <vcl/timer.hxx>
#include <vcl/vclptr.hxx>
#include <tools/gen.hxx>
#include <address.hxx>
#include <postit.hxx>

namespace vcl { class Window; }

class SdrModel;
class SdrCaptionObj;

class ScNoteMarker
{
private:
    VclPtr<vcl::Window>     m_pWindow;
    VclPtr<vcl::Window>     m_pRightWin;
    VclPtr<vcl::Window>     m_pBottomWin;
    VclPtr<vcl::Window>     m_pDiagWin;
    ScDocument* m_pDoc;
    ScAddress const   m_aDocPos;
    OUString const    m_aUserText;
    tools::Rectangle   m_aVisRect;
    Timer       m_aTimer;
    MapMode const     m_aMapMode;
    bool const        m_bLeft;
    bool const        m_bByKeyboard;

    tools::Rectangle       m_aRect;
    std::unique_ptr<SdrModel>           m_pModel;
    ScCaptionPtr    m_xObject;
    bool            m_bVisible;
    DECL_LINK( TimeHdl, Timer*, void );

public:
                ScNoteMarker( vcl::Window* pWin, vcl::Window* pRight, vcl::Window* pBottom, vcl::Window* pDiagonal,
                                ScDocument* pD, const ScAddress& aPos, const OUString& rUser,
                                const MapMode& rMap, bool bLeftEdge, bool bForce, bool bKeyboard);
                ~ScNoteMarker();

    void        Draw();
    void        InvalidateWin();

    const ScAddress& GetDocPos() const       { return m_aDocPos; }
    bool        IsByKeyboard() const    { return m_bByKeyboard; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
