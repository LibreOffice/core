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



#ifndef _EDITSEL_HXX
#define _EDITSEL_HXX

#include <vcl/seleng.hxx>

class EditView;

//  ----------------------------------------------------------------------
//  class EditSelFunctionSet
//  ----------------------------------------------------------------------
class EditSelFunctionSet: public FunctionSet
{
private:
    EditView*       pCurView;

public:
                    EditSelFunctionSet();

    virtual void    BeginDrag();

    virtual void    CreateAnchor();
    virtual void    DestroyAnchor();

    virtual sal_Bool    SetCursorAtPoint( const Point& rPointPixel, sal_Bool bDontSelectAtCursor = sal_False );

    virtual sal_Bool    IsSelectionAtPoint( const Point& rPointPixel );
    virtual void    DeselectAtPoint( const Point& rPointPixel );
    virtual void    DeselectAll();

    void            SetCurView( EditView* pView )       { pCurView = pView; }
    EditView*       GetCurView()                        { return pCurView; }
};

// ----------------------------------------------------------------------
//  class EditSelectionEngine
//  ----------------------------------------------------------------------
class EditSelectionEngine : public SelectionEngine
{
private:

public:
                    EditSelectionEngine();

    void            SetCurView( EditView* pNewView );
    EditView*       GetCurView();
};

#endif // _EDITSEL_HXX
