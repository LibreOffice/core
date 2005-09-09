/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: swrenamexnameddlg.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 10:04:33 $
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
#ifndef _SWRENAMEXNAMEDDLG_HXX
#define _SWRENAMEXNAMEDDLG_HXX


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

#endif
