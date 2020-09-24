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
#ifndef INCLUDED_SC_SOURCE_UI_INC_VIEWUTIL_HXX
#define INCLUDED_SC_SOURCE_UI_INC_VIEWUTIL_HXX

#include <address.hxx>
#include <sal/types.h>
#include <i18nlangtag/lang.h>

class SfxItemSet;
class SfxBindings;
class SvxFontItem;
class SfxViewShell;
class SfxViewFrame;
class ScChangeAction;
class ScChangeViewSettings;
class ScDocument;
class ScMarkData;
enum class SvtScriptType;
enum class TransliterationFlags;


enum class ScUpdateMode { All, Marks };

class SC_DLLPUBLIC ScViewUtil
{
public:
    static void ExecuteCharMap( const SvxFontItem&  rOldFont,
                                SfxViewFrame&       rFrame );

    static bool IsActionShown( const ScChangeAction& rAction,
                                const ScChangeViewSettings& rSettings,
                                ScDocument& rDocument );

    static void PutItemScript( SfxItemSet& rShellSet, const SfxItemSet& rCoreSet,
                                sal_uInt16 nWhichId, SvtScriptType nScript );

    static LanguageType GetEffLanguage( ScDocument& rDoc, const ScAddress& rPos );

    static TransliterationFlags GetTransliterationType( sal_uInt16 nSlotID );

    static bool HasFiltered( const ScRange& rRange, const ScDocument& rDoc );
    /** Fit a range to cover nRows number of unfiltered rows.
        @return <TRUE/> if the resulting range covers nRows unfiltered rows. */
    static bool FitToUnfilteredRows( ScRange & rRange, const ScDocument& rDoc, size_t nRows );
    static void UnmarkFiltered( ScMarkData& rMark, const ScDocument& rDoc );

    static void HideDisabledSlot( SfxItemSet& rSet, SfxBindings& rBindings, sal_uInt16 nSlotId );

    /** Returns true, if the passed view shell is in full screen mode. */
    static bool IsFullScreen( const SfxViewShell& rViewShell );
    /** Enters or leaves full screen mode at the passed view shell. */
    static void SetFullScreen( const SfxViewShell& rViewShell, bool bSet );
};

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
    bool    GetDiff( SCCOL& rX1, SCROW& rY1, SCCOL& rX2, SCROW& rY2 );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
