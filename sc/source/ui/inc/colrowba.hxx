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


