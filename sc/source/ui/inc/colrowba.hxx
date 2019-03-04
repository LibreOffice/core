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

#ifndef INCLUDED_SC_SOURCE_UI_INC_COLROWBA_HXX
#define INCLUDED_SC_SOURCE_UI_INC_COLROWBA_HXX

#include "hdrcont.hxx"
#include "viewdata.hxx"

class ScHeaderFunctionSet;
class ScHeaderSelectionEngine;
class ScTabView;

class ScColBar : public ScHeaderControl
{
    ScHSplitPos const        meWhich;
    ScHeaderFunctionSet*     mpFuncSet;

public:
                ScColBar( vcl::Window* pParent, ScHSplitPos eWhich,
                          ScHeaderFunctionSet* pFuncSet, ScHeaderSelectionEngine* pEng,
                          ScTabView* pTab );
                virtual ~ScColBar() override;

    virtual SCCOLROW    GetPos() const override;
    virtual sal_uInt16  GetEntrySize( SCCOLROW nEntryNo ) const override;
    virtual OUString    GetEntryText( SCCOLROW nEntryNo ) const override;

    virtual bool        IsLayoutRTL() const override;

    virtual void        SetEntrySize( SCCOLROW nPos, sal_uInt16 nNewSize ) override;
    virtual void        HideEntries( SCCOLROW nStart, SCCOLROW nEnd ) override;

    virtual void        SetMarking( bool bSet ) override;
    virtual void        SelectWindow() override;
    virtual bool        IsDisabled() const override;
    virtual bool        ResizeAllowed() const override;

    virtual void        DrawInvert( long nDragPos ) override;

    virtual OUString    GetDragHelp( long nVal ) override;
};

class ScRowBar : public ScHeaderControl
{
    ScVSplitPos const        meWhich;
    ScHeaderFunctionSet*     mpFuncSet;

public:
                ScRowBar( vcl::Window* pParent, ScVSplitPos eWhich,
                          ScHeaderFunctionSet* pFuncSet, ScHeaderSelectionEngine* pEng,
                          ScTabView* pTab );
                virtual ~ScRowBar() override;

    virtual SCCOLROW    GetPos() const override;
    virtual sal_uInt16  GetEntrySize( SCCOLROW nEntryNo ) const override;
    virtual OUString    GetEntryText( SCCOLROW nEntryNo ) const override;

    virtual bool        IsMirrored() const override;
    virtual SCCOLROW    GetHiddenCount( SCCOLROW nEntryNo ) const override;

    virtual void        SetEntrySize( SCCOLROW nPos, sal_uInt16 nNewSize ) override;
    virtual void        HideEntries( SCCOLROW nStart, SCCOLROW nEnd ) override;

    virtual void        SetMarking( bool bSet ) override;
    virtual void        SelectWindow() override;
    virtual bool        IsDisabled() const override;
    virtual bool        ResizeAllowed() const override;

    virtual void        DrawInvert( long nDragPos ) override;

    virtual OUString    GetDragHelp( long nVal ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
