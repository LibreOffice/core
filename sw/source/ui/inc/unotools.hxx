/*************************************************************************
 *
 *  $RCSfile: unotools.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: os $ $Date: 2001-06-29 06:06:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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


/* -----------------09.06.99 14:36-------------------
 *
 * --------------------------------------------------*/
class SwRenameXNamedDlg : public ModalDialog
{
    FixedText       aNewNameFT;
    NoSpaceEdit     aNewNameED;
    FixedLine       aNameFL;
    OKButton        aOk;
    CancelButton    aCancel;
    HelpButton      aHelp;

    String          sRemoveWarning;

    STAR_REFERENCE( container::XNamed ) &   xNamed;
    STAR_REFERENCE( container::XNameAccess ) & xNameAccess;
    STAR_REFERENCE( container::XNameAccess )   xSecondAccess;
    STAR_REFERENCE( container::XNameAccess )   xThirdAccess;

    DECL_LINK(OkHdl, OKButton*);
    DECL_LINK(ModifyHdl, NoSpaceEdit*);

public:
    SwRenameXNamedDlg( Window* pParent,
                    STAR_REFERENCE( container::XNamed ) & xNamed,
                    STAR_REFERENCE( container::XNameAccess ) & xNameAccess );

    void    SetForbiddenChars( const String& rSet )
        { aNewNameED.SetForbiddenChars( rSet ); }

    void SetAlternativeAccess(
            STAR_REFERENCE( container::XNameAccess ) & xSecond,
            STAR_REFERENCE( container::XNameAccess ) & xThird )
    {
        xSecondAccess = xSecond;
        xThirdAccess = xThird;
    }
};
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

class SwView;
class SwOneExampleFrame
{
    STAR_REFERENCE( awt::XControl )         _xControl;
    STAR_REFERENCE( frame::XModel )         _xModel;
    STAR_REFERENCE( frame::XController )    _xController;
    STAR_REFERENCE( text::XTextCursor )     _xCursor;

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

    DECL_LINK( TimeoutHdl, Timer* );
    DECL_LINK( PopupHdl, Menu* );

    void    CreateControl();
    void    DisposeControl();
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

