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

#ifndef INCLUDED_EDITENG_SOURCE_EDITENG_EDITSEL_HXX
#define INCLUDED_EDITENG_SOURCE_EDITENG_EDITSEL_HXX

#include <vcl/seleng.hxx>

class EditView;

class EditSelFunctionSet: public FunctionSet
{
private:
    EditView*       pCurView;

public:
                    EditSelFunctionSet();

    virtual void    BeginDrag() override;

    virtual void    CreateAnchor() override;
    virtual void    DestroyAnchor() override;

    virtual bool    SetCursorAtPoint( const Point& rPointPixel, bool bDontSelectAtCursor = false ) override;

    virtual bool    IsSelectionAtPoint( const Point& rPointPixel ) override;
    virtual void    DeselectAtPoint( const Point& rPointPixel ) override;
    virtual void    DeselectAll() override;

    void            SetCurView( EditView* pView )       { pCurView = pView; }
};

class EditSelectionEngine : public SelectionEngine
{
private:

public:
                    EditSelectionEngine();

    void            SetCurView( EditView* pNewView );
};

#endif // INCLUDED_EDITENG_SOURCE_EDITENG_EDITSEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
