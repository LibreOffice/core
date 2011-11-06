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



#ifndef _FMRWRK_DEPWIN_HXX
#define _FMRWRK_DEPWIN_HXX

#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#include <vcl/menu.hxx>
#include <svtools/scrwin.hxx>
#include <soldep/connctr.hxx>

class Depper;

class DepWin : public Window
{
private:
    ConnectorList   ConList;

    ObjectWin*      mpNewConWin;
    sal_Bool            mbStartNewCon;
    Point           maNewConStart;
    Point           maNewConEnd;
    ObjectWin*      mpSelectedProject;

public:
    PopupMenu*      mpPopup;
//  Depper*         mpDepperDontuseme;

                    DepWin( Window* pParent, WinBits nWinStyle );
                    ~DepWin();
    void            AddConnector( Connector* pNewCon );
    void            RemoveConnector( Connector* piOldCon );
    void            NewConnector( ObjectWin* pWin );
    ConnectorList*  GetConnectorList();
    void            ClearConnectorList() { ConList.Clear();}
    void            Paint( const Rectangle& rRect );
    void            MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    Command( const CommandEvent& rEvent);
//  void            Resize();
    void            MouseMove( const MouseEvent& rMEvt );
    sal_Bool            IsStartNewCon() { return mbStartNewCon; };
    void            SetPopupHdl( void* pHdl );
    void            SetSelectedProject( ObjectWin* object ) { mpSelectedProject = object; };
    ObjectWin*      GetSelectedProject() { return mpSelectedProject; };
//  DECL_LINK( PopupSelected, PopupMenu* );
    void    DrawOutput( OutputDevice* pDevice, const Point& rOffset  );
};

#endif
