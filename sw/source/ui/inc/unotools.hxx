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


#ifndef _UNOTOOLS_HXX
#define _UNOTOOLS_HXX


#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#include <actctrl.hxx>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <tools/resary.hxx>
#include <swunodef.hxx>
#include "swdllapi.h"

/* -----------------------------15.12.99 09:55--------------------------------

 ---------------------------------------------------------------------------*/
class SwOneExampleFrame;
class SwFrmCtrlWindow : public Window
{
    SwOneExampleFrame*  pExampleFrame;
public:
    SwFrmCtrlWindow(Window* pParent, WinBits nBits, SwOneExampleFrame*  pFrame);

    virtual void    Command( const CommandEvent& rCEvt );
};
/* -----------------------------15.12.99 12:56--------------------------------

 ---------------------------------------------------------------------------*/
class MenuResource : public Resource
{
    ResStringArray      aMenuArray;

public:
    MenuResource(const ResId& rResId);

    ResStringArray& GetMenuArray() {return aMenuArray;}
};
/* -----------------27.07.99 15:20-------------------

 --------------------------------------------------*/
#define EX_SHOW_ONLINE_LAYOUT   0x001

// hard zoom value
#define EX_SHOW_BUSINESS_CARDS  0x02
//don't modify page size
#define EX_SHOW_DEFAULT_PAGE    0x04

class SwView;
class SW_DLLPUBLIC SwOneExampleFrame
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >         _xControl;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >         _xModel;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >    _xController;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor >     _xCursor;

    SwFrmCtrlWindow aTopWindow;
    Window&         rWindow;
    Timer           aLoadedTimer;
    Link            aInitializedLink;

    MenuResource    aMenuRes;
    String          sArgumentURL;

    SwView*         pModuleView;

    sal_uInt32          nStyleFlags;

    sal_Bool            bIsInitialized;
    sal_Bool            bServiceAvailable;

    static  sal_Bool    bShowServiceNotAvailableMessage;

    SW_DLLPRIVATE DECL_LINK( TimeoutHdl, Timer* );
    SW_DLLPRIVATE DECL_LINK( PopupHdl, Menu* );

    SW_DLLPRIVATE void  CreateControl();
    SW_DLLPRIVATE void  DisposeControl();

public:
    SwOneExampleFrame(Window& rWin,
                    sal_uInt32 nStyleFlags = EX_SHOW_ONLINE_LAYOUT,
                    const Link* pInitalizedLink = 0,
                    String* pURL = 0);
    ~SwOneExampleFrame();

    STAR_REFERENCE( awt::XControl ) &       GetControl()    {return _xControl; }
    STAR_REFERENCE( frame::XModel ) &       GetModel()      {return _xModel;}
    STAR_REFERENCE( frame::XController ) &  GetController() {return _xController;}
    STAR_REFERENCE( text::XTextCursor ) &   GetTextCursor() {return _xCursor;}

    void ClearDocument( sal_Bool bStartTimer = sal_False );

    sal_Bool IsInitialized() const {return bIsInitialized;}
    sal_Bool IsServiceAvailable() const {return bServiceAvailable;}

    void CreatePopup(const Point& rPt);

    static void     CreateErrorMessage(Window* pParent);
};

#endif

