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

#ifndef SC_COLROWBAR_HXX
#define SC_COLROWBAR_HXX

#include "hdrcont.hxx"
#include "viewdata.hxx"

class ScHeaderFunctionSet;
class ScHeaderSelectionEngine;

class ScColBar : public ScHeaderControl
{
    ScViewData*              pViewData;
    ScHSplitPos              eWhich;
    ScHeaderFunctionSet*     pFuncSet;

public:
                ScColBar( Window* pParent, ScViewData* pData, ScHSplitPos eWhichPos,
                            ScHeaderFunctionSet* pFunc, ScHeaderSelectionEngine* pEng );
                ~ScColBar();

    virtual SCCOLROW    GetPos() const SAL_OVERRIDE;
    virtual sal_uInt16  GetEntrySize( SCCOLROW nEntryNo ) const SAL_OVERRIDE;
    virtual OUString    GetEntryText( SCCOLROW nEntryNo ) const SAL_OVERRIDE;

    virtual bool        IsLayoutRTL() const SAL_OVERRIDE;

    virtual void        SetEntrySize( SCCOLROW nPos, sal_uInt16 nNewSize ) SAL_OVERRIDE;
    virtual void        HideEntries( SCCOLROW nStart, SCCOLROW nEnd ) SAL_OVERRIDE;

    virtual void        SetMarking( bool bSet ) SAL_OVERRIDE;
    virtual void        SelectWindow() SAL_OVERRIDE;
    virtual bool        IsDisabled() const SAL_OVERRIDE;
    virtual bool        ResizeAllowed() const SAL_OVERRIDE;

    virtual void        DrawInvert( long nDragPos ) SAL_OVERRIDE;

    virtual OUString    GetDragHelp( long nVal ) SAL_OVERRIDE;

            bool        UseNumericHeader() const;
};


class ScRowBar : public ScHeaderControl
{
    ScViewData*              pViewData;
    ScVSplitPos              eWhich;
    ScHeaderFunctionSet*     pFuncSet;

public:
                ScRowBar( Window* pParent, ScViewData* pData, ScVSplitPos eWhichPos,
                            ScHeaderFunctionSet* pFunc, ScHeaderSelectionEngine* pEng );
                ~ScRowBar();

    virtual SCCOLROW    GetPos() const SAL_OVERRIDE;
    virtual sal_uInt16  GetEntrySize( SCCOLROW nEntryNo ) const SAL_OVERRIDE;
    virtual OUString    GetEntryText( SCCOLROW nEntryNo ) const SAL_OVERRIDE;

    virtual bool        IsMirrored() const SAL_OVERRIDE;
    virtual SCROW       GetHiddenCount( SCROW nEntryNo ) const SAL_OVERRIDE;

    virtual void        SetEntrySize( SCCOLROW nPos, sal_uInt16 nNewSize ) SAL_OVERRIDE;
    virtual void        HideEntries( SCCOLROW nStart, SCCOLROW nEnd ) SAL_OVERRIDE;

    virtual void        SetMarking( bool bSet ) SAL_OVERRIDE;
    virtual void        SelectWindow() SAL_OVERRIDE;
    virtual bool        IsDisabled() const SAL_OVERRIDE;
    virtual bool        ResizeAllowed() const SAL_OVERRIDE;

    virtual void        DrawInvert( long nDragPos ) SAL_OVERRIDE;

    virtual OUString    GetDragHelp( long nVal ) SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
