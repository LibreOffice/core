/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: versdlg.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:15:04 $
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

#ifndef _VERSDLG_HXX
#define _VERSDLG_HXX

// #include *****************************************************************

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SVTREEBOX_HXX //autogen
#include <svtools/svtabbx.hxx>
#endif
#ifndef _SVEDIT_HXX //autogen
#include <svtools/svmedit.hxx>
#endif
#include "basedlgs.hxx"

class SfxObjectShell;
struct SfxVersionInfo;

class SfxVersionsTabListBox_Impl  : public SvTabListBox
{
    virtual void                KeyInput( const KeyEvent& rKeyEvent );

public:
                                SfxVersionsTabListBox_Impl(
                                    Window* pParent, const ResId& );
};

class SfxVersionTableDtor;
class SfxVersionDialog : public SfxModalDialog
{
    FixedLine                   aNewGroup;
    PushButton                  aSaveButton;
    CheckBox                    aSaveCheckBox;
    FixedLine                   aExistingGroup;
    FixedText                   aDateTimeText;
    FixedText                   aSavedByText;
    FixedText                   aCommentText;
    SfxVersionsTabListBox_Impl  aVersionBox;
    CancelButton                aCloseButton;
    PushButton                  aOpenButton;
    PushButton                  aViewButton;
    PushButton                  aDeleteButton;
    PushButton                  aCompareButton;
    HelpButton                  aHelpButton;
    SfxViewFrame*               pViewFrame;
    SfxVersionTableDtor*        mpTable;

    DECL_LINK(                  DClickHdl_Impl, Control* );
    DECL_LINK(                  SelectHdl_Impl, Control* );
    DECL_LINK(                  ButtonHdl_Impl, Button* );
    void                        Init_Impl();
    void                        Open_Impl();

public:
                                SfxVersionDialog ( SfxViewFrame* pFrame,
                                    Window *pParent );
    virtual                     ~SfxVersionDialog ();
};

class SfxViewVersionDialog_Impl : public SfxModalDialog
{
    FixedText                   aDateTimeText;
    FixedText                   aSavedByText;
    MultiLineEdit               aEdit;
    OKButton                    aOKButton;
    CancelButton                aCancelButton;
    PushButton                  aCloseButton;
    HelpButton                  aHelpButton;
    SfxVersionInfo*             pInfo;

    DECL_LINK(                  ButtonHdl, Button* );

public:
                                SfxViewVersionDialog_Impl( Window *pParent,
                                    SfxVersionInfo& rInfo, BOOL bEdit );
};

#endif
