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

// ---------------------------------------------------------------------------


class ScColBar : public ScHeaderControl
{
    ScViewData*              pViewData;
    ScHSplitPos              eWhich;
    ScHeaderFunctionSet*     pFuncSet;

public:
                ScColBar( Window* pParent, ScViewData* pData, ScHSplitPos eWhichPos,
                            ScHeaderFunctionSet* pFunc, ScHeaderSelectionEngine* pEng );
                ~ScColBar();

    virtual SCCOLROW    GetPos();
    virtual sal_uInt16      GetEntrySize( SCCOLROW nEntryNo );
    virtual String      GetEntryText( SCCOLROW nEntryNo );

    virtual sal_Bool        IsLayoutRTL();                      // only for columns

    virtual void        SetEntrySize( SCCOLROW nPos, sal_uInt16 nNewSize );
    virtual void        HideEntries( SCCOLROW nStart, SCCOLROW nEnd );

    virtual void        SetMarking( sal_Bool bSet );
    virtual void        SelectWindow();
    virtual sal_Bool        IsDisabled();
    virtual sal_Bool        ResizeAllowed();

    virtual void        DrawInvert( long nDragPos );

    virtual String      GetDragHelp( long nVal );

            sal_Bool        UseNumericHeader() const;
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

    virtual SCCOLROW    GetPos();
    virtual sal_uInt16      GetEntrySize( SCCOLROW nEntryNo );
    virtual String      GetEntryText( SCCOLROW nEntryNo );

    virtual sal_Bool        IsMirrored();                       // only for columns
    virtual SCROW       GetHiddenCount( SCROW nEntryNo );   // only for columns

    virtual void        SetEntrySize( SCCOLROW nPos, sal_uInt16 nNewSize );
    virtual void        HideEntries( SCCOLROW nStart, SCCOLROW nEnd );

    virtual void        SetMarking( sal_Bool bSet );
    virtual void        SelectWindow();
    virtual sal_Bool        IsDisabled();
    virtual sal_Bool        ResizeAllowed();

    virtual void        DrawInvert( long nDragPos );

    virtual String      GetDragHelp( long nVal );
};



#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
