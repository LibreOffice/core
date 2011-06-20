/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
    ScHeaderSelectionEngine* pSelEngine;

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
    ScHeaderSelectionEngine* pSelEngine;

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
