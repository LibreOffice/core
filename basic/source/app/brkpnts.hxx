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



#include <vcl/window.hxx>

#define MARKER_NOMARKER 0xFFFF


class SbModule;
class BreakpointListe;
struct Breakpoint;
class ImageList;

DECLARE_LIST( BreakpointList, Breakpoint* )

class BreakpointWindow : public Window, public BreakpointList
{
using Window::Scroll;

public:
    BreakpointWindow( Window *pParent );
//  ~BreakpointWindow();

    void        Reset();

    void        SetModule( SbModule *pMod );
    void        SetBPsInModule();

    void        InsertBreakpoint( sal_uInt16 nLine );
    void        ToggleBreakpoint( sal_uInt16 nLine );
    void        AdjustBreakpoints( sal_uIntPtr nLine, sal_Bool bInserted );

    void        LoadBreakpoints( String aFilename );
    void        SaveBreakpoints( String aFilename );

protected:
    Breakpoint* FindBreakpoint( sal_uIntPtr nLine );

private:
    long            nCurYOffset;
    sal_uInt16          nMarkerPos;
    SbModule*       pModule;
    sal_Bool            bErrorMarker;
    static ImageList *pImages;

protected:
    virtual void    Paint( const Rectangle& );
    Breakpoint*     FindBreakpoint( const Point& rMousePos );
    void            ShowMarker( sal_Bool bShow );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );

public:

//  void            SetModulWindow( ModulWindow* pWin )
//                      { pModulWindow = pWin; }

    void            SetMarkerPos( sal_uInt16 nLine, sal_Bool bErrorMarker = sal_False );

    virtual void        Scroll( long nHorzScroll, long nVertScroll,
                                sal_uInt16 nFlags = 0 );
    long&           GetCurYOffset()         { return nCurYOffset; }
};








