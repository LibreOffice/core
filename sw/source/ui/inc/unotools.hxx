/*************************************************************************
 *
 *  $RCSfile: unotools.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:43 $
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
#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
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

#ifndef _SV_RESARY_HXX
#include <vcl/resary.hxx>
#endif

/* -----------------09.06.99 14:36-------------------
 *
 * --------------------------------------------------*/
class SwRenameXNamedDlg : public ModalDialog
{
    FixedText       aNewNameFT;
    NoSpaceEdit     aNewNameED;
    GroupBox        aNameGB;
    OKButton        aOk;
    CancelButton    aCancel;
    HelpButton      aHelp;

    String          sRemoveWarning;

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed > &           xNamed;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > &  xNameAccess;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >        xSecondAccess;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >        xThirdAccess;

    DECL_LINK(OkHdl, OKButton*);
    DECL_LINK(ModifyHdl, NoSpaceEdit*);

public:
    SwRenameXNamedDlg(Window* pParent, ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed > & xNamed, ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > & xNameAccess);

    void    SetForbiddenChars(const String& rSet){aNewNameED.SetForbiddenChars(rSet);}

    void    SetAlternativeAccess( ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > & xSecond, ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > & xThird)
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

//online layout and hard zoom value
#define EX_SHOW_BUSINESS_CARDS  0x003

class SwView;
class SwOneExampleFrame
{
    SwFrmCtrlWindow aTopWindow;
    Window&         rWindow;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >         _xControl;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >         _xModel;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >    _xController;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor >     _xCursor;

    Timer           aLoadedTimer;
    Link            aInitializedLink;

    MenuResource    aMenuRes;

    SwView*         pModuleView;

    sal_uInt32          nStyleFlags;

    sal_Bool            bIsInitialized;
    sal_Bool            bServiceAvailable;

    static  sal_Bool    bShowServiceNotAvailableMessage;

    DECL_LINK( TimeoutHdl, Timer* );
    DECL_LINK( PopupHdl, Menu* );

public:
    SwOneExampleFrame(Window& rWin,
                    sal_uInt32 nStyleFlags = EX_SHOW_ONLINE_LAYOUT,
                    const Link* pInitalizedLink = 0,
                    String* pURL = 0);
    ~SwOneExampleFrame();

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > &       GetControl() {return _xControl;}
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > &       GetModel() {return _xModel;}
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController > &  GetController() {return _xController;}
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor > & GetTextCursor() {return   _xCursor;}

    void            ExecUndo();

    sal_Bool            IsInitialized() const {return bIsInitialized;}
    sal_Bool            IsServiceAvailable() const {return bServiceAvailable;}

    void CreatePopup(const Point& rPt);

    static void     CreateErrorMessage(Window* pParent);
};
#endif

