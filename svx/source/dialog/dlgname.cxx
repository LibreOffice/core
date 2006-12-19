/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgname.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: ihi $ $Date: 2006-12-19 17:45:38 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif

#include <tools/ref.hxx>
#include <tools/list.hxx>
#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

#include "dialogs.hrc"

#include "dlgname.hxx"
#include "defdlgname.hxx" //CHINA001
#include "dlgname.hrc"
#include "dialmgr.hxx"

#define MAX_DESCRIPTION_LINES   ((long)5)

/*************************************************************************
|*
|* Dialog zum Editieren eines Namens
|*
\************************************************************************/

SvxNameDialog::SvxNameDialog( Window* pWindow, const String& rName, const String& rDesc ) :
    ModalDialog     ( pWindow, ResId( RID_SVXDLG_NAME, DIALOG_MGR() ) ),
    aFtDescription      ( this, ResId( FT_DESCRIPTION ) ),
    aEdtName            ( this, ResId( EDT_STRING ) ),
    aBtnOK              ( this, ResId( BTN_OK ) ),
    aBtnCancel          ( this, ResId( BTN_CANCEL ) ),
    aBtnHelp            ( this, ResId( BTN_HELP ) )
{
    FreeResource();

    aFtDescription.SetText( rDesc );
    aEdtName.SetText( rName );
    aEdtName.SetSelection(Selection(SELECTION_MIN, SELECTION_MAX));
    ModifyHdl(&aEdtName);
    aEdtName.SetModifyHdl(LINK(this, SvxNameDialog, ModifyHdl));

    // dynamic height of the description field
    Size aSize = aFtDescription.GetSizePixel();
    long nTxtWidth = aFtDescription.GetCtrlTextWidth( rDesc );
    if ( nTxtWidth > aSize.Width() )
    {
        long nLines = Min( ( nTxtWidth / aSize.Width() + 1 ), MAX_DESCRIPTION_LINES );
        long nHeight = aSize.Height();
        aSize.Height() = nHeight * nLines;
        aFtDescription.SetSizePixel( aSize );
        Point aPnt = aEdtName.GetPosPixel();
        aPnt.Y() += ( aSize.Height() - nHeight );
        aEdtName.SetPosPixel( aPnt );
    }
}

/* -----------------------------27.02.2002 15:22------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK(SvxNameDialog, ModifyHdl, Edit*, EMPTYARG)
{
    if(aCheckNameHdl.IsSet())
        aBtnOK.Enable(aCheckNameHdl.Call(this) > 0);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////
// #i68101#
// Dialog for editing Object Name
// plus uniqueness-callback-linkHandler

SvxObjectNameDialog::SvxObjectNameDialog(
    Window* pWindow,
    const String& rName)
:   ModalDialog(pWindow, ResId(RID_SVXDLG_OBJECT_NAME, DIALOG_MGR())),
    aFtName(this, ResId(NTD_FT_NAME)),
    aEdtName(this, ResId(NTD_EDT_NAME)),
    aFlSeparator(this, ResId(FL_SEPARATOR_A)),
    aBtnHelp(this, ResId(BTN_HELP)),
    aBtnOK(this, ResId(BTN_OK)),
    aBtnCancel(this, ResId(BTN_CANCEL))
{
    FreeResource();

    // set name
    aEdtName.SetText(rName);

    // activate name
    aEdtName.SetSelection(Selection(SELECTION_MIN, SELECTION_MAX));
    ModifyHdl(&aEdtName);
    aEdtName.SetModifyHdl(LINK(this, SvxObjectNameDialog, ModifyHdl));
}

IMPL_LINK(SvxObjectNameDialog, ModifyHdl, Edit*, EMPTYARG)
{
    if(aCheckNameHdl.IsSet())
    {
        aBtnOK.Enable(aCheckNameHdl.Call(this) > 0);
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////
// #i68101#
// Dialog for editing Object Title and Description

SvxObjectTitleDescDialog::SvxObjectTitleDescDialog(
    Window* pWindow,
    const String& rTitle,
    const String& rDescription)
:   ModalDialog(pWindow, ResId(RID_SVXDLG_OBJECT_TITLE_DESC, DIALOG_MGR())),
    aFtTitle(this, ResId(NTD_FT_TITLE)),
    aEdtTitle(this, ResId(NTD_EDT_TITLE)),
    aFtDescription(this, ResId(NTD_FT_DESC)),
    aEdtDescription(this, ResId(NTD_EDT_DESC)),
    aFlSeparator(this, ResId(FL_SEPARATOR_B)),
    aBtnHelp(this, ResId(BTN_HELP)),
    aBtnOK(this, ResId(BTN_OK)),
    aBtnCancel(this, ResId(BTN_CANCEL))
{
    FreeResource();

    // set title & desc
    aEdtTitle.SetText(rTitle);
    aEdtDescription.SetText(rDescription);

    // activate title
    aEdtTitle.SetSelection(Selection(SELECTION_MIN, SELECTION_MAX));
}

///////////////////////////////////////////////////////////////////////////////////////////////

/*************************************************************************
|*
|* Dialog zum Abbrechen, Speichern oder Hinzufuegen
|*
\************************************************************************/

SvxMessDialog::SvxMessDialog( Window* pWindow, const String& rText, const String& rDesc, Image* pImg ) :
    ModalDialog     ( pWindow, ResId( RID_SVXDLG_MESSBOX, DIALOG_MGR() ) ),
    aFtDescription      ( this, ResId( FT_DESCRIPTION ) ),
    aBtn1               ( this, ResId( BTN_1 ) ),
    aBtn2               ( this, ResId( BTN_2 ) ),
    aBtnCancel          ( this, ResId( BTN_CANCEL ) ),
    aFtImage            ( this )
{
    FreeResource();

    if( pImg )
    {
        pImage = new Image( *pImg );
        aFtImage.SetImage( *pImage );
        aFtImage.SetStyle( ( aFtImage.GetStyle()/* | WB_NOTABSTOP */) & ~WB_3DLOOK );
        aFtImage.SetPosSizePixel( LogicToPixel( Point( 3, 6 ), MAP_APPFONT ),
                                  aFtImage.GetImage().GetSizePixel() );
        aFtImage.Show();
    }

    SetText( rText );
    aFtDescription.SetText( rDesc );

    aBtn1.SetClickHdl( LINK( this, SvxMessDialog, Button1Hdl ) );
    aBtn2.SetClickHdl( LINK( this, SvxMessDialog, Button2Hdl ) );
}

SvxMessDialog::~SvxMessDialog()
{
    if( pImage )
        delete pImage;
}

/*************************************************************************/

IMPL_LINK_INLINE_START( SvxMessDialog, Button1Hdl, Button *, EMPTYARG )
{
    EndDialog( RET_BTN_1 );
    return 0;
}
IMPL_LINK_INLINE_END( SvxMessDialog, Button1Hdl, Button *, EMPTYARG )

/*************************************************************************/

IMPL_LINK_INLINE_START( SvxMessDialog, Button2Hdl, Button *, EMPTYARG )
{
    EndDialog( RET_BTN_2 );
    return 0;
}
IMPL_LINK_INLINE_END( SvxMessDialog, Button2Hdl, Button *, EMPTYARG )

/*************************************************************************/

void SvxMessDialog::SetButtonText( USHORT nBtnId, const String& rNewTxt )
{
    switch ( nBtnId )
    {
        case MESS_BTN_1:
            aBtn1.SetText( rNewTxt );
            break;

        case MESS_BTN_2:
            aBtn2.SetText( rNewTxt );
            break;

        default:
            DBG_ERROR( "Falsche Button-Nummer!!!" );
    }
}


