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

#ifndef BASCTL_DLGEDFUNC_HXX
#define BASCTL_DLGEDFUNC_HXX

namespace basctl
{

class DlgEditor;


// DlgEdFunc


class DlgEdFunc /* : public LinkHdl */
{
protected:
    DlgEditor& rParent;
    Timer      aScrollTimer;

    DECL_LINK( ScrollTimeout, Timer * );
    void    ForceScroll( const Point& rPos );

public:
    explicit DlgEdFunc (DlgEditor& rParent);
    virtual ~DlgEdFunc();

    virtual bool MouseButtonDown( const MouseEvent& rMEvt );
    virtual bool MouseButtonUp( const MouseEvent& rMEvt );
    virtual bool MouseMove( const MouseEvent& rMEvt );
    virtual bool KeyInput( const KeyEvent& rKEvt );
};


// DlgEdFuncInsert


class DlgEdFuncInsert : public DlgEdFunc
{
public:
    explicit DlgEdFuncInsert (DlgEditor& rParent);
    virtual ~DlgEdFuncInsert ();

    virtual bool MouseButtonDown( const MouseEvent& rMEvt );
    virtual bool MouseButtonUp( const MouseEvent& rMEvt );
    virtual bool MouseMove( const MouseEvent& rMEvt );
};


// DlgEdFuncSelect


class DlgEdFuncSelect : public DlgEdFunc
{
protected:
    bool    bMarkAction;

public:
    explicit DlgEdFuncSelect (DlgEditor& rParent);
    virtual ~DlgEdFuncSelect ();

    virtual bool MouseButtonDown( const MouseEvent& rMEvt );
    virtual bool MouseButtonUp( const MouseEvent& rMEvt );
    virtual bool MouseMove( const MouseEvent& rMEvt );
};


} // namespace basctl

#endif // BASCTL_DLGEDFUNC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
