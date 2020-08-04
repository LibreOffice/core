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

#pragma once

#include <vcl/event.hxx>
#include <vcl/timer.hxx>
#include <tools/link.hxx>
#include <tools/gen.hxx>

namespace basctl
{

class DlgEditor;


// DlgEdFunc


class DlgEdFunc /* : public LinkHdl */
{
protected:
    DlgEditor& rParent;
    Timer      aScrollTimer;

    DECL_LINK( ScrollTimeout, Timer *, void );
    void    ForceScroll( const Point& rPos );

public:
    explicit DlgEdFunc (DlgEditor& rParent);
    virtual ~DlgEdFunc();

    virtual void MouseButtonDown( const MouseEvent& rMEvt );
    virtual bool MouseButtonUp( const MouseEvent& rMEvt );
    virtual void MouseMove( const MouseEvent& rMEvt );
    bool KeyInput( const KeyEvent& rKEvt );
};


// DlgEdFuncInsert


class DlgEdFuncInsert : public DlgEdFunc
{
public:
    explicit DlgEdFuncInsert (DlgEditor& rParent);
    virtual ~DlgEdFuncInsert () override;

    virtual void MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual bool MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void MouseMove( const MouseEvent& rMEvt ) override;
};


// DlgEdFuncSelect


class DlgEdFuncSelect : public DlgEdFunc
{
public:
    explicit DlgEdFuncSelect (DlgEditor& rParent);
    virtual ~DlgEdFuncSelect () override;

    virtual void MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual bool MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void MouseMove( const MouseEvent& rMEvt ) override;
};


} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
