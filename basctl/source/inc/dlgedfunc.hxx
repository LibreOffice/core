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



#ifndef _BASCTL_DLGEDFUNC_HXX
#define _BASCTL_DLGEDFUNC_HXX

#include <vcl/timer.hxx>

class DlgEditor;
class Timer;
class MouseEvent;
namespace basegfx { class B2DPoint; }

//============================================================================
// DlgEdFunc
//============================================================================

class DlgEdFunc /* : public LinkHdl */
{
protected:
    DlgEditor* pParent;
    Timer        aScrollTimer;

    DECL_LINK( ScrollTimeout, Timer * );
    void    ForceScroll( const basegfx::B2DPoint& rPos );

public:
    DlgEdFunc( DlgEditor* pParent );
    virtual ~DlgEdFunc();

    virtual sal_Bool MouseButtonDown( const MouseEvent& rMEvt );
    virtual sal_Bool MouseButtonUp( const MouseEvent& rMEvt );
    virtual sal_Bool MouseMove( const MouseEvent& rMEvt );
    virtual sal_Bool KeyInput( const KeyEvent& rKEvt );
};

//============================================================================
// DlgEdFuncInsert
//============================================================================

class DlgEdFuncInsert : public DlgEdFunc
{
public:
    DlgEdFuncInsert( DlgEditor* pParent );
    ~DlgEdFuncInsert();

    virtual sal_Bool MouseButtonDown( const MouseEvent& rMEvt );
    virtual sal_Bool MouseButtonUp( const MouseEvent& rMEvt );
    virtual sal_Bool MouseMove( const MouseEvent& rMEvt );
};

//============================================================================
// DlgEdFuncSelect
//============================================================================

class DlgEdFuncSelect : public DlgEdFunc
{
protected:
    sal_Bool    bMarkAction;

public:
    DlgEdFuncSelect( DlgEditor* pParent );
    ~DlgEdFuncSelect();

    virtual sal_Bool MouseButtonDown( const MouseEvent& rMEvt );
    virtual sal_Bool MouseButtonUp( const MouseEvent& rMEvt );
    virtual sal_Bool MouseMove( const MouseEvent& rMEvt );
};


#endif //_BASCTL_DLGEDFUNC_HXX
