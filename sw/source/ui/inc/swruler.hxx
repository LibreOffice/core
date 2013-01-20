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

#ifndef SW_COMMENT_RULER_HXX
#define SW_COMMENT_RULER_HXX

#include <svx/ruler.hxx>

class ViewShell;
class View;
class Window;
class SwEditWin;

class SwCommentRuler
    : public SvxRuler
{
public:
    SwCommentRuler (
        ViewShell* pViewSh,
        Window* pParent,
        SwEditWin* pWin,
        sal_uInt16 nRulerFlags,
        SfxBindings& rBindings,
        WinBits nWinStyle);
    virtual ~SwCommentRuler ();

    virtual void Paint( const Rectangle& rRect );

protected:
    ViewShell * mpViewShell;
    SwEditWin * mpSwWin;
    bool        mbHighlighted;
    VirtualDevice maVirDev;

    virtual void MouseButtonDown( const MouseEvent& rMEvt );
    virtual void MouseMove(const MouseEvent& rMEvt);
    virtual void Command( const CommandEvent& rCEvt );

    Rectangle    GetCommentControlRegion();

    void         DrawCommentControl();
    void         ImplDrawArrow(long nX, long nY, const Color& rColor, bool bPointRight);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
