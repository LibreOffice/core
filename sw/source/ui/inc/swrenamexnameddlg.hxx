/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SWRENAMEXNAMEDDLG_HXX
#define _SWRENAMEXNAMEDDLG_HXX


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



/* -----------------09.06.99 14:36-------------------
 *
 * --------------------------------------------------*/
class SwRenameXNamedDlg : public ModalDialog
{
    FixedLine       aNameFL;
    FixedText       aNewNameFT;
    NoSpaceEdit     aNewNameED;
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
