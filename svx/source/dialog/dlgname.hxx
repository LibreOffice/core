/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgname.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2006-12-19 17:46:06 $
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
#ifndef _SVX_DLG_NAME_HXX
#define _SVX_DLG_NAME_HXX


#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

// #i68101#
#ifndef _SVEDIT_HXX //autogen
#include <svtools/svmedit.hxx>
#endif

/*************************************************************************
|*
|* Dialog zum Editieren eines Namens
|*
\************************************************************************/
class SvxNameDialog : public ModalDialog
{
private:
    FixedText       aFtDescription;
    Edit            aEdtName;
    OKButton        aBtnOK;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;

    Link            aCheckNameHdl;
#if _SOLAR__PRIVATE
    DECL_LINK(ModifyHdl, Edit*);
#endif

public:
    SvxNameDialog( Window* pWindow, const String& rName, const String& rDesc );

    void    GetName( String& rName ){rName = aEdtName.GetText();}

    /** add a callback Link that is called whenever the content of the edit
        field is changed.  The Link result determines wether the OK
        Button is enabled (> 0) or disabled (== 0).

        @param rLink a Callback declared with DECL_LINK and implemented with
               IMPL_LINK, that is executed on modification.

        @param bCheckImmediately If true, the Link is called directly after
               setting it. It is recommended to set this flag to true to avoid
               an inconsistent state if the initial String (given in the CTOR)
               does not satisfy the check condition.

        @todo Remove the parameter bCheckImmediately and incorporate the 'true'
              behaviour as default.
     */
    void    SetCheckNameHdl( const Link& rLink, bool bCheckImmediately = false )
    {
        aCheckNameHdl = rLink;
        if ( bCheckImmediately )
            aBtnOK.Enable( rLink.Call( this ) > 0 );
    }

    void    SetEditHelpId(ULONG nHelpId) {aEdtName.SetHelpId(nHelpId);}
};

///////////////////////////////////////////////////////////////////////////////////////////////
// #i68101#
// Dialog for editing Object Title and Description
// plus uniqueness-callback-linkHandler

class SvxObjectNameDialog : public ModalDialog
{
private:
    // name
    FixedText       aFtName;
    Edit            aEdtName;

    // separator
    FixedLine       aFlSeparator;

    // buttons
    HelpButton      aBtnHelp;
    OKButton        aBtnOK;
    CancelButton    aBtnCancel;

    // callback link for name uniqueness
    Link            aCheckNameHdl;
#if _SOLAR__PRIVATE
    DECL_LINK(ModifyHdl, Edit*);
#endif

public:
    // constructor
    SvxObjectNameDialog(Window* pWindow, const String& rName);

    // data access
    void GetName(String& rName) {rName = aEdtName.GetText(); }

    // set handler
    void SetCheckNameHdl(const Link& rLink, bool bCheckImmediately = false)
    {
        aCheckNameHdl = rLink;

        if(bCheckImmediately)
        {
            aBtnOK.Enable(rLink.Call(this) > 0);
        }
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////
// #i68101#
// Dialog for editing Object Title and Description

class SvxObjectTitleDescDialog : public ModalDialog
{
private:
    // title
    FixedText       aFtTitle;
    Edit            aEdtTitle;

    // description
    FixedText       aFtDescription;
    MultiLineEdit   aEdtDescription;

    // separator
    FixedLine       aFlSeparator;

    // buttons
    HelpButton      aBtnHelp;
    OKButton        aBtnOK;
    CancelButton    aBtnCancel;

public:
    // constructor
    SvxObjectTitleDescDialog(Window* pWindow, const String& rTitle, const String& rDesc);

    // data access
    void GetTitle(String& rTitle) {rTitle = aEdtTitle.GetText(); }
    void GetDescription(String& rDescription) {rDescription = aEdtDescription.GetText(); }
};

///////////////////////////////////////////////////////////////////////////////////////////////

/*************************************************************************
|*
|* Dialog zum Abbrechen, Speichern oder Hinzufuegen
|*
\************************************************************************/
class SvxMessDialog : public ModalDialog
{
private:
    FixedText       aFtDescription;
    PushButton      aBtn1;
    PushButton      aBtn2;
    CancelButton    aBtnCancel;
    FixedImage      aFtImage;
    Image*          pImage;
#if _SOLAR__PRIVATE
    DECL_LINK( Button1Hdl, Button * );
    DECL_LINK( Button2Hdl, Button * );
#endif
public:
    SvxMessDialog( Window* pWindow, const String& rText, const String& rDesc, Image* pImg = NULL );
    ~SvxMessDialog();

    void    SetButtonText( USHORT nBtnId, const String& rNewTxt );
};



#endif // _SVX_DLG_NAME_HXX

