/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unotools.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 12:13:23 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _UNOTOOLS_HXX
#define _UNOTOOLS_HXX


#ifndef _SV_DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif
#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _ACTCTRL_HXX //autogen
#include <actctrl.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTCURSOR_HPP_
#include <com/sun/star/text/XTextCursor.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROL_HPP_
#include <com/sun/star/awt/XControl.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif

#ifndef _TOOLS_RESARY_HXX
#include <tools/resary.hxx>
#endif
#ifndef _SWUNODEF_HXX
#include <swunodef.hxx>
#endif
#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif

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

    void ClearDocument( BOOL bStartTimer = FALSE );

    sal_Bool IsInitialized() const {return bIsInitialized;}
    sal_Bool IsServiceAvailable() const {return bServiceAvailable;}

    void CreatePopup(const Point& rPt);

    static void     CreateErrorMessage(Window* pParent);
};

#endif

