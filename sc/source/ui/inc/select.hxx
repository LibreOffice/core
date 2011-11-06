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



#ifndef SC_SELECT_HXX
#define SC_SELECT_HXX

#ifndef _SELENG_HXX //autogen
#include <vcl/seleng.hxx>
#endif

#include "viewdata.hxx"     // ScSplitPos

// ---------------------------------------------------------------------------

class ScTabView;
class ScViewData;


class ScViewSelectionEngine : public SelectionEngine
{
private:
    ScSplitPos      eWhich;
public:
                    ScViewSelectionEngine( Window* pWindow, ScTabView* pView,
                                                    ScSplitPos eSplitPos );

    ScSplitPos      GetWhich() const            { return eWhich; }
    void            SetWhich(ScSplitPos eNew)   { eWhich = eNew; }
};


class ScViewFunctionSet : public FunctionSet            // View (Gridwin / Tastatur)
{
private:
    ScViewData*             pViewData;
    ScViewSelectionEngine*  pEngine;

    sal_Bool            bAnchor;
    sal_Bool            bStarted;
    ScAddress       aAnchorPos;

    ScSplitPos      GetWhich();

public:
                    ScViewFunctionSet( ScViewData* pNewViewData );

    void            SetSelectionEngine( ScViewSelectionEngine* pSelEngine );

    void            SetAnchor( SCCOL nPosX, SCROW nPosY );
    void            SetAnchorFlag( sal_Bool bSet );

    virtual void    BeginDrag();
    virtual void    CreateAnchor();
    virtual void    DestroyAnchor();
    virtual sal_Bool    SetCursorAtPoint( const Point& rPointPixel, sal_Bool bDontSelectAtCursor = sal_False );
    virtual sal_Bool    IsSelectionAtPoint( const Point& rPointPixel );
    virtual void    DeselectAtPoint( const Point& rPointPixel );
    virtual void    DeselectAll();

    sal_Bool            SetCursorAtCell( SCsCOL nPosX, SCsROW nPosY, sal_Bool bScroll );
};


// ---------------------------------------------------------------------------


class ScHeaderFunctionSet : public FunctionSet          // Spalten- / Zeilenkoepfe
{
private:
    ScViewData*     pViewData;
    sal_Bool            bColumn;                // Col- / Rowbar
    ScSplitPos      eWhich;

    sal_Bool            bAnchor;
    SCCOLROW        nCursorPos;

public:
                    ScHeaderFunctionSet( ScViewData* pNewViewData );

    void            SetColumn( sal_Bool bSet );
    void            SetWhich( ScSplitPos eNew );

    virtual void    BeginDrag();
    virtual void    CreateAnchor();
    virtual void    DestroyAnchor();
    virtual sal_Bool    SetCursorAtPoint( const Point& rPointPixel, sal_Bool bDontSelectAtCursor = sal_False );
    virtual sal_Bool    IsSelectionAtPoint( const Point& rPointPixel );
    virtual void    DeselectAtPoint( const Point& rPointPixel );
    virtual void    DeselectAll();

    void            SetAnchorFlag(sal_Bool bSet)    { bAnchor = bSet; }
};


class ScHeaderSelectionEngine : public SelectionEngine
{
public:
                    ScHeaderSelectionEngine( Window* pWindow, ScHeaderFunctionSet* pFuncSet );
};



#endif
