/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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

class ScViewUtil                                // static Methoden
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
    SCCOL   nContX1;
    SCROW   nContY1;
    SCCOL   nContX2;
    SCROW   nContY2;
public:
            ScUpdateRect( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2 );
    void    SetNew( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2 );
    sal_Bool    GetDiff( SCCOL& rX1, SCROW& rY1, SCCOL& rX2, SCROW& rY2 );
#ifdef OLD_SELECTION_PAINT
    sal_Bool    GetXorDiff( SCCOL& rX1, SCROW& rY1, SCCOL& rX2, SCROW& rY2, sal_Bool& rCont );
    void    GetContDiff( SCCOL& rX1, SCROW& rY1, SCCOL& rX2, SCROW& rY2 );
#endif
};




#endif

