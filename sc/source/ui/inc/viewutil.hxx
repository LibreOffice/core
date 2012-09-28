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
#ifndef SC_VIEWUTIL_HXX
#define SC_VIEWUTIL_HXX

#include "address.hxx"
#include <tools/solar.h>
#include <sal/types.h>

class String;
class SfxItemSet;
class SfxBindings;
class SvxFontItem;
class SfxViewShell;
class SfxViewFrame;

class ScChangeAction;
class ScChangeViewSettings;
class ScDocument;
class ScAddress;
class ScRange;
class ScMarkData;

enum ScUpdateMode { SC_UPDATE_ALL, SC_UPDATE_CHANGED, SC_UPDATE_MARKS };

// ---------------------------------------------------------------------------

class SC_DLLPUBLIC ScViewUtil
{
public:
    static sal_Bool ExecuteCharMap( const SvxFontItem&  rOldFont,
                                SfxViewFrame&       rFrame,
                                SvxFontItem&        rNewFont,
                                String&             rString );

    static sal_Bool IsActionShown( const ScChangeAction& rAction,
                                const ScChangeViewSettings& rSettings,
                                ScDocument& rDocument );

    static void PutItemScript( SfxItemSet& rShellSet, const SfxItemSet& rCoreSet,
                                sal_uInt16 nWhichId, sal_uInt16 nScript );

    static sal_uInt16 GetEffLanguage( ScDocument* pDoc, const ScAddress& rPos );

    static sal_Int32 GetTransliterationType( sal_uInt16 nSlotID );

    static bool HasFiltered( const ScRange& rRange, ScDocument* pDoc );
    /** Fit a range to cover nRows number of unfiltered rows.
        @return <TRUE/> if the resulting range covers nRows unfiltered rows. */
    static bool FitToUnfilteredRows( ScRange & rRange, ScDocument * pDoc, size_t nRows );
    static void UnmarkFiltered( ScMarkData& rMark, ScDocument* pDoc );

    static void HideDisabledSlot( SfxItemSet& rSet, SfxBindings& rBindings, sal_uInt16 nSlotId );

    /** Returns true, if the passed view shell is in full screen mode. */
    static bool IsFullScreen( SfxViewShell& rViewShell );
    /** Enters or leaves full screen mode at the passed view shell. */
    static void SetFullScreen( SfxViewShell& rViewShell, bool bSet );
};

// ---------------------------------------------------------------------------

class ScUpdateRect
{
private:
    SCCOL   nOldStartX;
    SCROW   nOldStartY;
    SCCOL   nOldEndX;
    SCROW   nOldEndY;
    SCCOL   nNewStartX;
    SCROW   nNewStartY;
    SCCOL   nNewEndX;
    SCROW   nNewEndY;
public:
            ScUpdateRect( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2 );
    void    SetNew( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2 );
    sal_Bool    GetDiff( SCCOL& rX1, SCROW& rY1, SCCOL& rX2, SCROW& rY2 );
};




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
