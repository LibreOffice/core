/*************************************************************************
 *
 *  $RCSfile: moduldl2.cxx,v $
 *
 *  $Revision: 1.46 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-23 12:05:59 $
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


#define GLOBALOVERFLOW

#ifndef _SFX_IPFRM_HXX
#include <sfx2/ipfrm.hxx>
#endif

#include <ide_pch.hxx>

#include <svtools/filedlg.hxx>

#pragma hdrstop

#include <sot/storinfo.hxx>

#include <moduldlg.hrc>
#include <moduldlg.hxx>
#include <basidesh.hrc>
#include <basidesh.hxx>
#include <bastypes.hxx>
#include <basobj.hxx>
#include <baside2.hrc>
#include <iderdll.hxx>
#include <iderdll2.hxx>
#include <svx/passwd.hxx>
#include <sbxitem.hxx>
#include <basdoc.hxx>

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif

#ifndef _COM_SUN_STAR_UI_DIALOGS_XFILEPICKER_HPP_
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XFILTERMANAGER_HPP_
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_TEMPLATEDESCRIPTION_HPP_
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XLIBRYARYCONTAINER2_HPP_
#include <com/sun/star/script/XLibraryContainer2.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XLIBRARYCONTAINERPASSWORD_HPP_
#include <com/sun/star/script/XLibraryContainerPassword.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XSIMPLEFILEACCESS_HPP_
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#endif

#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

using namespace ::comphelper;
using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::ui::dialogs;


//----------------------------------------------------------------------------
//  BasicLibUserData
//----------------------------------------------------------------------------

class BasicLibUserData
{
private:
    SfxObjectShell* m_pShell;

public:
                    BasicLibUserData( SfxObjectShell* pShell ) { m_pShell = pShell; }
                    virtual         ~BasicLibUserData() {};

    SfxObjectShell* GetShell() const { return m_pShell; }
};


//----------------------------------------------------------------------------
//  BasicLibLBoxString
//----------------------------------------------------------------------------

class BasicLibLBoxString : public SvLBoxString
{
public:
    BasicLibLBoxString( SvLBoxEntry* pEntry, USHORT nFlags, const String& rTxt ) :
        SvLBoxString( pEntry, nFlags, rTxt ) {}

    virtual void Paint( const Point& rPos, SvLBox& rDev, USHORT nFlags, SvLBoxEntry* pEntry );
};

//----------------------------------------------------------------------------

void BasicLibLBoxString::Paint( const Point& rPos, SvLBox& rDev, USHORT, SvLBoxEntry* pEntry )
{
    // Change text color if library is read only:
    bool bReadOnly = false;
    if (pEntry && pEntry->GetUserData())
    {
        SfxObjectShell * pShell
            = static_cast< BasicLibUserData * >(pEntry->GetUserData())->
            GetShell();
        rtl::OUString aLibName(
            static_cast< SvLBoxString * >(pEntry->GetItem(1))->GetText());
        Reference< script::XLibraryContainer2 > xModLibContainer(
            BasicIDE::GetModuleLibraryContainer(pShell), UNO_QUERY);
        Reference< script::XLibraryContainer2 > xDlgLibContainer(
            BasicIDE::GetDialogLibraryContainer(pShell), UNO_QUERY);
        bReadOnly
            = (xModLibContainer.is() && xModLibContainer->hasByName(aLibName)
               && xModLibContainer->isLibraryReadOnly(aLibName))
            || (xDlgLibContainer.is() && xDlgLibContainer->hasByName(aLibName)
                && xDlgLibContainer->isLibraryReadOnly(aLibName));
    }
    if (bReadOnly)
        rDev.DrawCtrlText(rPos, GetText(), 0, STRING_LEN, TEXT_DRAW_DISABLE);
    else
        rDev.DrawText(rPos, GetText());
}


//----------------------------------------------------------------------------
//  BasicCheckBox
//----------------------------------------------------------------------------

BasicCheckBox::BasicCheckBox( Window* pParent, const ResId& rResId )
    :SvTabListBox( pParent, rResId )
    ,m_pShell( 0 )
{
    nMode = LIBMODE_MANAGER;
    long aTabs[] = { 1, 12 };   // Mindestens einen braucht die TabPos...
                                // 12 wegen der Checkbox
    SetTabs( aTabs );
    Init();
}

//----------------------------------------------------------------------------

__EXPORT BasicCheckBox::~BasicCheckBox()
{
    delete pCheckButton;

    // delete user data
    SvLBoxEntry* pEntry = First();
    while ( pEntry )
    {
        delete (BasicLibUserData*)pEntry->GetUserData();
        pEntry = Next( pEntry );
    }
}

//----------------------------------------------------------------------------

void BasicCheckBox::Init()
{
    pCheckButton = new SvLBoxButtonData(this);

    if ( nMode == LIBMODE_CHOOSER )
        EnableCheckButton( pCheckButton );
    else
        EnableCheckButton( 0 );

    SetHighlightRange();
}

//----------------------------------------------------------------------------

void BasicCheckBox::SetMode( USHORT n )
{
    nMode = n;

    if ( nMode == LIBMODE_CHOOSER )
        EnableCheckButton( pCheckButton );
    else
        EnableCheckButton( 0 );
}

//----------------------------------------------------------------------------

SvLBoxEntry* BasicCheckBox::InsertEntry( const String& rStr, ULONG nPos )
{
    return SvTabListBox::InsertEntry( rStr, nPos, 0 );
}

//----------------------------------------------------------------------------

void BasicCheckBox::RemoveEntry( ULONG nPos )
{
    if ( nPos < GetEntryCount() )
        SvTreeListBox::GetModel()->Remove( GetEntry( nPos ) );
}

//----------------------------------------------------------------------------

SvLBoxEntry* BasicCheckBox::FindEntry( const String& rName )
{
    ULONG nCount = GetEntryCount();
    for ( ULONG i = 0; i < nCount; i++ )
    {
        SvLBoxEntry* pEntry = GetEntry( i );
        DBG_ASSERT( pEntry, "pEntry?!" );
        if ( rName.CompareIgnoreCaseToAscii( GetEntryText( pEntry, 0 ) ) == COMPARE_EQUAL )
            return pEntry;
    }
    return 0;
}

//----------------------------------------------------------------------------

void BasicCheckBox::SelectEntryPos( ULONG nPos, BOOL bSelect )
{
    if ( nPos < GetEntryCount() )
        Select( GetEntry( nPos ), bSelect );
}

//----------------------------------------------------------------------------

ULONG BasicCheckBox::GetSelectEntryPos() const
{
    return GetModel()->GetAbsPos( FirstSelected() );
}

//----------------------------------------------------------------------------

ULONG BasicCheckBox::GetCheckedEntryCount() const
{
    ULONG   nCheckCount = 0;
    ULONG   nCount      = GetEntryCount();

    for (ULONG i=0; i<nCount; i++ )
    {
        if ( IsChecked( i ) )
            nCheckCount++;
    }

    return nCheckCount;
}

//----------------------------------------------------------------------------

void BasicCheckBox::CheckEntryPos( ULONG nPos, BOOL bCheck )
{
    if ( nPos < GetEntryCount() )
    {
        SvLBoxEntry* pEntry = GetEntry( nPos );

        if ( bCheck != GetCheckButtonState( pEntry ) )
            SetCheckButtonState( pEntry,
                                 bCheck
                                    ? SvButtonState(SV_BUTTON_CHECKED)
                                    : SvButtonState(SV_BUTTON_UNCHECKED) );
    }
}

//----------------------------------------------------------------------------

BOOL BasicCheckBox::IsChecked( ULONG nPos ) const
{
    if ( nPos < GetEntryCount() )
        return (GetCheckButtonState( GetEntry( nPos ) ) == SV_BUTTON_CHECKED);
    return FALSE;
}

//----------------------------------------------------------------------------

void BasicCheckBox::InitEntry( SvLBoxEntry* pEntry, const XubString& rTxt, const Image& rImg1, const Image& rImg2 )
{
    SvTabListBox::InitEntry( pEntry, rTxt, rImg1, rImg2 );

    if ( nMode == LIBMODE_MANAGER )
    {
        // initialize all columns with own string class (column 0 == bitmap)
        USHORT nCount = pEntry->ItemCount();
        for ( USHORT nCol = 1; nCol < nCount; ++nCol )
        {
            SvLBoxString* pCol = (SvLBoxString*)pEntry->GetItem( nCol );
            BasicLibLBoxString* pStr = new BasicLibLBoxString( pEntry, 0, pCol->GetText() );
            pEntry->ReplaceItem( pStr, nCol );
        }
    }
}

//----------------------------------------------------------------------------

BOOL __EXPORT BasicCheckBox::EditingEntry( SvLBoxEntry* pEntry, Selection& )
{
    if ( nMode != LIBMODE_MANAGER )
        return FALSE;

    DBG_ASSERT( pEntry, "Kein Eintrag?" );

    // check, if Standard library
    String aLibName = GetEntryText( pEntry, 0 );
    if ( aLibName.EqualsIgnoreCaseAscii( "Standard" ) )
    {
        ErrorBox( this, WB_OK | WB_DEF_OK, String( IDEResId( RID_STR_CANNOTCHANGENAMESTDLIB ) ) ).Execute();
        return FALSE;
    }

    // check, if library is readonly
    ::rtl::OUString aOULibName( aLibName );
    Reference< script::XLibraryContainer2 > xModLibContainer( BasicIDE::GetModuleLibraryContainer( m_pShell ), UNO_QUERY );
    Reference< script::XLibraryContainer2 > xDlgLibContainer( BasicIDE::GetDialogLibraryContainer( m_pShell ), UNO_QUERY );
    if ( ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) && xModLibContainer->isLibraryReadOnly( aOULibName ) && !xModLibContainer->isLibraryLink( aOULibName ) ) ||
         ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) && xDlgLibContainer->isLibraryReadOnly( aOULibName ) && !xDlgLibContainer->isLibraryLink( aOULibName ) ) )
    {
        ErrorBox( this, WB_OK | WB_DEF_OK, String( IDEResId( RID_STR_LIBISREADONLY ) ) ).Execute();
        return FALSE;
    }

    // TODO: check if library is reference/link

    // Prueffen, ob Referenz...
    /*
    USHORT nLib = pBasMgr->GetLibId( GetEntryText( pEntry, 0 ) );
    DBG_ASSERT( nLib != LIB_NOTFOUND, "LibId ?!" );
    if ( pBasMgr->IsReference( nLib ) )
    {
        ErrorBox( this, WB_OK | WB_DEF_OK, String( IDEResId( RID_STR_CANNOTCHANGENAMEREFLIB ) ) ).Execute();
        return FALSE;
    }
    */
    return TRUE;
}

//----------------------------------------------------------------------------

BOOL __EXPORT BasicCheckBox::EditedEntry( SvLBoxEntry* pEntry, const String& rNewText )
{
    BOOL bValid = ( rNewText.Len() <= 30 ) && BasicIDE::IsValidSbxName( rNewText );
    String aCurText( GetEntryText( pEntry, 0 ) );
    if ( bValid && ( aCurText != rNewText ) )
    {
        try
        {
            ::rtl::OUString aOUOldName( aCurText );
            ::rtl::OUString aOUNewName( rNewText );

            Reference< script::XLibraryContainer2 > xModLibContainer( BasicIDE::GetModuleLibraryContainer( m_pShell ), UNO_QUERY );
            if ( xModLibContainer.is() )
            {
                xModLibContainer->renameLibrary( aOUOldName, aOUNewName );
            }

            Reference< script::XLibraryContainer2 > xDlgLibContainer( BasicIDE::GetDialogLibraryContainer( m_pShell ), UNO_QUERY );
            if ( xDlgLibContainer.is() )
            {
                xDlgLibContainer->renameLibrary( aOUOldName, aOUNewName );
            }

            BasicIDE::MarkDocShellModified( m_pShell );
            SfxBindings* pBindings = BasicIDE::GetBindingsPtr();
            if ( pBindings )
            {
                pBindings->Invalidate( SID_BASICIDE_LIBSELECTOR );
                pBindings->Update( SID_BASICIDE_LIBSELECTOR );
            }
        }
        catch ( container::ElementExistException& )
        {
            ErrorBox( this, WB_OK | WB_DEF_OK, String( IDEResId( RID_STR_SBXNAMEALLREADYUSED ) ) ).Execute();
            return FALSE;
        }
        catch ( container::NoSuchElementException& e )
        {
            ByteString aBStr( String(e.Message), RTL_TEXTENCODING_ASCII_US );
            DBG_ERROR( aBStr.GetBuffer() );
            return FALSE;
        }
    }

    if ( !bValid )
    {
        if ( rNewText.Len() > 30 )
            ErrorBox( this, WB_OK | WB_DEF_OK, String( IDEResId( RID_STR_LIBNAMETOLONG ) ) ).Execute();
        else
            ErrorBox( this, WB_OK | WB_DEF_OK, String( IDEResId( RID_STR_BADSBXNAME ) ) ).Execute();
    }

    return bValid;
}

//----------------------------------------------------------------------------
// NewObjectDialog
//----------------------------------------------------------------------------

IMPL_LINK(NewObjectDialog, OkButtonHandler, Button *, EMPTYARG)
{
    if (BasicIDE::IsValidSbxName(aEdit.GetText()))
        EndDialog(1);
    else
    {
        ErrorBox(this, WB_OK | WB_DEF_OK,
                 String(IDEResId(RID_STR_BADSBXNAME))).Execute();
        aEdit.GrabFocus();
    }
    return 0;
}

NewObjectDialog::NewObjectDialog(Window * pParent, USHORT nMode,
                                 bool bCheckName)
    : ModalDialog( pParent, IDEResId( RID_DLG_NEWLIB ) ),
        aText( this, IDEResId( RID_FT_NEWLIB ) ),
        aEdit( this, IDEResId( RID_ED_LIBNAME ) ),
        aOKButton( this, IDEResId( RID_PB_OK ) ),
        aCancelButton( this, IDEResId( RID_PB_CANCEL ) )
{
    FreeResource();
    aEdit.GrabFocus();

    if ( nMode == NEWOBJECTMODE_LIB )
    {
        SetText( String( IDEResId( RID_STR_NEWLIB ) ) );
    }
    else if ( nMode == NEWOBJECTMODE_MOD )
    {
        SetText( String( IDEResId( RID_STR_NEWMOD ) ) );
    }
    else if ( nMode == NEWOBJECTMODE_METH )
    {
        SetText( String( IDEResId( RID_STR_NEWMETH ) ) );
    }
    else
    {
        SetText( String( IDEResId( RID_STR_NEWDLG ) ) );
    }

    if (bCheckName)
        aOKButton.SetClickHdl(LINK(this, NewObjectDialog, OkButtonHandler));
}

//----------------------------------------------------------------------------

NewObjectDialog::~NewObjectDialog()
{
}


//----------------------------------------------------------------------------
//  LibPage
//----------------------------------------------------------------------------

LibPage::LibPage( Window * pParent )
    :TabPage( pParent, IDEResId( RID_TP_LIBS ) )
    ,aBasicsText( this, IDEResId( RID_STR_BASICS ) )
    ,aBasicsBox( this, IDEResId( RID_LB_BASICS ) )
    ,aLibText( this, IDEResId( RID_STR_LIB ) )
    ,aLibBox( this, IDEResId( RID_TRLBOX ) )
    ,aEditButton( this, IDEResId( RID_PB_EDIT ) )
    ,aCloseButton( this, IDEResId( RID_PB_CLOSE ) )
    ,aPasswordButton( this, IDEResId( RID_PB_PASSWORD ) )
    ,aNewLibButton( this, IDEResId( RID_PB_NEWLIB ) )
    ,aInsertLibButton( this, IDEResId( RID_PB_APPEND ) )
    ,aDelButton( this, IDEResId( RID_PB_DELETE ) )
    ,m_pCurShell( 0 )
    ,m_eCurLocation( LIBRARY_LOCATION_UNKNOWN )
{
    FreeResource();
    pTabDlg = 0;

    aEditButton.SetClickHdl( LINK( this, LibPage, ButtonHdl ) );
    aNewLibButton.SetClickHdl( LINK( this, LibPage, ButtonHdl ) );
    aPasswordButton.SetClickHdl( LINK( this, LibPage, ButtonHdl ) );
    aInsertLibButton.SetClickHdl( LINK( this, LibPage, ButtonHdl ) );
    aDelButton.SetClickHdl( LINK( this, LibPage, ButtonHdl ) );
    aCloseButton.SetClickHdl( LINK( this, LibPage, ButtonHdl ) );
    aLibBox.SetSelectHdl( LINK( this, LibPage, TreeListHighlightHdl ) );

    aBasicsBox.SetSelectHdl( LINK( this, LibPage, BasicSelectHdl ) );

    aLibBox.SetMode( LIBMODE_MANAGER );
    aLibBox.EnableInplaceEditing( TRUE );
    aLibBox.SetWindowBits( WB_HSCROLL );
    aCloseButton.GrabFocus();

    long aTabs[] = { 2, 30, 120 };
    aLibBox.SetTabs( aTabs, MAP_PIXEL );

    FillListBox();
    aBasicsBox.SelectEntryPos( 0 );
    SetCurLib();

    CheckButtons();
}

//----------------------------------------------------------------------------

LibPage::~LibPage()
{
    USHORT nCount = aBasicsBox.GetEntryCount();
    for ( USHORT i = 0; i < nCount; ++i )
    {
        BasicShellEntry* pEntry = (BasicShellEntry*)aBasicsBox.GetEntryData( i );
        delete pEntry;
    }
}

//----------------------------------------------------------------------------

void LibPage::CheckButtons()
{
    SvLBoxEntry* pCur = aLibBox.GetCurEntry();
    if ( pCur )
    {
        String aLibName = aLibBox.GetEntryText( pCur, 0 );
        ::rtl::OUString aOULibName( aLibName );
        Reference< script::XLibraryContainer2 > xModLibContainer( BasicIDE::GetModuleLibraryContainer( m_pCurShell ), UNO_QUERY );
        Reference< script::XLibraryContainer2 > xDlgLibContainer( BasicIDE::GetDialogLibraryContainer( m_pCurShell ), UNO_QUERY );

        if ( m_eCurLocation == LIBRARY_LOCATION_SHARE )
        {
            aPasswordButton.Disable();
            aNewLibButton.Disable();
            aInsertLibButton.Disable();
            aDelButton.Disable();
        }
        else if ( aLibName.EqualsIgnoreCaseAscii( "Standard" ) )
        {
            aPasswordButton.Disable();
            aNewLibButton.Enable();
            aInsertLibButton.Enable();
            aDelButton.Disable();
            if ( !aLibBox.HasFocus() )
                aCloseButton.GrabFocus();
        }
        else if ( ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) && xModLibContainer->isLibraryReadOnly( aOULibName ) ) ||
                  ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) && xDlgLibContainer->isLibraryReadOnly( aOULibName ) ) )
        {
            aPasswordButton.Disable();
            aNewLibButton.Enable();
            aInsertLibButton.Enable();
            if ( ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) && xModLibContainer->isLibraryReadOnly( aOULibName ) && !xModLibContainer->isLibraryLink( aOULibName ) ) ||
                 ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) && xDlgLibContainer->isLibraryReadOnly( aOULibName ) && !xDlgLibContainer->isLibraryLink( aOULibName ) ) )
                aDelButton.Disable();
            else
                aDelButton.Enable();
        }
        else
        {
            if ( xModLibContainer.is() && !xModLibContainer->hasByName( aOULibName ) )
                aPasswordButton.Disable();
            else
                aPasswordButton.Enable();

            aNewLibButton.Enable();
            aInsertLibButton.Enable();
            aDelButton.Enable();
        }
    }
}

//----------------------------------------------------------------------------

void __EXPORT LibPage::ActivatePage()
{
    SetCurLib();
}

//----------------------------------------------------------------------------


void __EXPORT LibPage::DeactivatePage()
{
}

//----------------------------------------------------------------------------


IMPL_LINK_INLINE_START( LibPage, TreeListHighlightHdl, SvTreeListBox *, pBox )
{
    if ( pBox->IsSelected( pBox->GetHdlEntry() ) )
        CheckButtons();
    return 0;
}
IMPL_LINK_INLINE_END( LibPage, TreeListHighlightHdl, SvTreeListBox *, pBox )

//----------------------------------------------------------------------------

IMPL_LINK_INLINE_START( LibPage, BasicSelectHdl, ListBox *, pBox )
{
    SetCurLib();
    CheckButtons();
    return 0;
}
IMPL_LINK_INLINE_END( LibPage, BasicSelectHdl, ListBox *, pBox )

//----------------------------------------------------------------------------

IMPL_LINK( LibPage, ButtonHdl, Button *, pButton )
{
    if ( pButton == &aEditButton )
    {
        SfxViewFrame* pViewFrame = SfxViewFrame::Current();
        SfxDispatcher* pDispatcher = ( pViewFrame && !pViewFrame->ISA( SfxInPlaceFrame ) ) ? pViewFrame->GetDispatcher() : NULL;
        if ( pDispatcher )
        {
            pDispatcher->Execute( SID_BASICIDE_APPEAR, SFX_CALLMODE_SYNCHRON );
        }
        else
        {
            SfxAllItemSet aArgs( SFX_APP()->GetPool() );
            SfxRequest aRequest( SID_BASICIDE_APPEAR, SFX_CALLMODE_SYNCHRON, aArgs );
            SFX_APP()->ExecuteSlot( aRequest );
        }
        SfxObjectShellItem aShellItem( SID_BASICIDE_ARG_SHELL, m_pCurShell );
        SvLBoxEntry* pCurEntry = aLibBox.GetCurEntry();
        DBG_ASSERT( pCurEntry, "Entry?!" );
        String aLibName( aLibBox.GetEntryText( pCurEntry, 0 ) );
        SfxStringItem aLibNameItem( SID_BASICIDE_ARG_LIBNAME, aLibName );
        BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
        pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
        pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
        if ( pDispatcher )
        {
            pDispatcher->Execute( SID_BASICIDE_LIBSELECTED,
                                    SFX_CALLMODE_ASYNCHRON, &aShellItem, &aLibNameItem, 0L );
        }
        EndTabDialog( 1 );
    }
    else if ( pButton == &aNewLibButton )
        NewLib();
    else if ( pButton == &aInsertLibButton )
        InsertLib();
    else if ( pButton == &aDelButton )
        DeleteCurrent();
    else if ( pButton == &aCloseButton )
    {
        EndTabDialog( 0 );
    }
    else if ( pButton == &aPasswordButton )
    {
        SvLBoxEntry* pCurEntry = aLibBox.GetCurEntry();
        String aLibName( aLibBox.GetEntryText( pCurEntry, 0 ) );
        ::rtl::OUString aOULibName( aLibName );

        // load module library (if not loaded)
        Reference< script::XLibraryContainer > xModLibContainer = BasicIDE::GetModuleLibraryContainer( m_pCurShell );
        if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) && !xModLibContainer->isLibraryLoaded( aOULibName ) )
        {
            BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
            if ( pIDEShell )
                pIDEShell->GetViewFrame()->GetWindow().EnterWait();
            xModLibContainer->loadLibrary( aOULibName );
            if ( pIDEShell )
                pIDEShell->GetViewFrame()->GetWindow().LeaveWait();
        }

        // load dialog library (if not loaded)
        Reference< script::XLibraryContainer > xDlgLibContainer = BasicIDE::GetDialogLibraryContainer( m_pCurShell );
        if ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) && !xDlgLibContainer->isLibraryLoaded( aOULibName ) )
        {
            BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
            if ( pIDEShell )
                pIDEShell->GetViewFrame()->GetWindow().EnterWait();
            xDlgLibContainer->loadLibrary( aOULibName );
            if ( pIDEShell )
                pIDEShell->GetViewFrame()->GetWindow().LeaveWait();
        }

        // check, if library is password protected
        if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) )
        {
            Reference< script::XLibraryContainerPassword > xPasswd( xModLibContainer, UNO_QUERY );
            if ( xPasswd.is() )
            {
                BOOL bProtected = xPasswd->isLibraryPasswordProtected( aOULibName );

                // change password dialog
                SvxPasswordDialog* pDlg = new SvxPasswordDialog( this, TRUE, !bProtected );
                pDlg->SetCheckPasswordHdl( LINK( this, LibPage, CheckPasswordHdl ) );

                if ( pDlg->Execute() == RET_OK )
                {
                    BOOL bNewProtected = xPasswd->isLibraryPasswordProtected( aOULibName );

                    if ( bNewProtected != bProtected )
                    {
                        ULONG nPos = (ULONG)aLibBox.GetModel()->GetAbsPos( pCurEntry );
                        aLibBox.GetModel()->Remove( pCurEntry );
                        ImpInsertLibEntry( aLibName, nPos );
                        aLibBox.SetCurEntry( aLibBox.GetEntry( nPos ) );
                    }

                    BasicIDE::MarkDocShellModified( m_pCurShell );
                }
                delete pDlg;
            }
        }
    }
    CheckButtons();
    return 0;
}

//----------------------------------------------------------------------------

IMPL_LINK_INLINE_START( LibPage, CheckPasswordHdl, SvxPasswordDialog *, pDlg )
{
    long nRet = 0;

    SvLBoxEntry* pCurEntry = aLibBox.GetCurEntry();
    ::rtl::OUString aOULibName( aLibBox.GetEntryText( pCurEntry, 0 ) );
    Reference< script::XLibraryContainerPassword > xPasswd( BasicIDE::GetModuleLibraryContainer( m_pCurShell ), UNO_QUERY );

    if ( xPasswd.is() )
    {
        try
        {
            ::rtl::OUString aOUOldPassword( pDlg->GetOldPassword() );
            ::rtl::OUString aOUNewPassword( pDlg->GetNewPassword() );
            xPasswd->changeLibraryPassword( aOULibName, aOUOldPassword, aOUNewPassword );
            nRet = 1;
        }
        catch (...)
        {
        }
    }

    return nRet;
}
IMPL_LINK_INLINE_END( LibPage, CheckPasswordHdl, SvxPasswordDialog *, pDlg )

//----------------------------------------------------------------------------

void LibPage::NewLib()
{
    createLibImpl( static_cast<Window*>( this ), m_pCurShell, &aLibBox, NULL);
}

//----------------------------------------------------------------------------

void LibPage::InsertLib()
{
    // file open dialog
    Reference< lang::XMultiServiceFactory > xMSF( ::comphelper::getProcessServiceFactory() );
    Reference < XFilePicker > xFP;
    if( xMSF.is() )
    {
        Sequence <Any> aServiceType(1);
        aServiceType[0] <<= TemplateDescription::FILEOPEN_SIMPLE;
        xFP = Reference< XFilePicker >( xMSF->createInstanceWithArguments(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.dialogs.FilePicker" ) ), aServiceType ), UNO_QUERY );
    }
    xFP->setTitle( String( IDEResId( RID_STR_APPENDLIBS ) ) );

    // filter
    Reference< XFilterManager > xFltMgr(xFP, UNO_QUERY);
    xFltMgr->appendFilter( String( IDEResId( RID_STR_BASIC ) ), String( RTL_CONSTASCII_USTRINGPARAM( "*.xlc;*.xlb;*.sbl;*.sxw;*.sxc;*.sxi;*.sdw;*.sdc;*.sdd" ) ) );

    // set display directory and filter
    String aPath( IDE_DLL()->GetExtraData()->GetAddLibPath() );
    if ( aPath.Len() )
    {
        xFP->setDisplayDirectory( aPath );
        xFltMgr->setCurrentFilter( IDE_DLL()->GetExtraData()->GetAddLibFilter() );
    }
    else
    {
        // macro path from configuration management
        xFP->setDisplayDirectory( SvtPathOptions().GetWorkPath() );
        xFltMgr->setCurrentFilter( String( IDEResId( RID_STR_BASIC ) ) );
    }

    if ( xFP->execute() == RET_OK )
    {
        IDE_DLL()->GetExtraData()->SetAddLibPath( xFP->getDisplayDirectory() );
        IDE_DLL()->GetExtraData()->SetAddLibFilter( xFltMgr->getCurrentFilter() );

        // library containers for import
        Reference< script::XLibraryContainer2 > xModLibContImport;
        Reference< script::XLibraryContainer2 > xDlgLibContImport;

        // file URLs
        Sequence< ::rtl::OUString > aFiles = xFP->getFiles();
        INetURLObject aURLObj( aFiles[0] );
        INetURLObject aModURLObj( aURLObj );
        INetURLObject aDlgURLObj( aURLObj );

        String aBase = aURLObj.getBase();
        String aModBase = String::CreateFromAscii( "script" );
        String aDlgBase = String::CreateFromAscii( "dialog" );

        if ( aBase == aModBase || aBase == aDlgBase )
        {
            aModURLObj.setBase( aModBase );
            aDlgURLObj.setBase( aDlgBase );
        }

        if ( xMSF.is() )
        {
            Reference< XSimpleFileAccess > xSFA( xMSF->createInstance(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ucb.SimpleFileAccess" ) ) ), UNO_QUERY );

            if ( xSFA.is() )
            {
                ::rtl::OUString aModURL( aModURLObj.GetMainURL( INetURLObject::NO_DECODE ) );
                if ( xSFA->exists( aModURL ) )
                {
                    Sequence <Any> aSeqModURL(1);
                    aSeqModURL[0] <<= aModURL;
                    xModLibContImport = Reference< script::XLibraryContainer2 >( xMSF->createInstanceWithArguments(
                                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.script.ScriptLibraryContainer" ) ), aSeqModURL ), UNO_QUERY );
                }

                ::rtl::OUString aDlgURL( aDlgURLObj.GetMainURL( INetURLObject::NO_DECODE ) );
                if ( xSFA->exists( aDlgURL ) )
                {
                    Sequence <Any> aSeqDlgURL(1);
                    aSeqDlgURL[0] <<= aDlgURL;
                    xDlgLibContImport = Reference< script::XLibraryContainer2 >( xMSF->createInstanceWithArguments(
                                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.script.DialogLibraryContainer" ) ), aSeqDlgURL ), UNO_QUERY );
                }
            }
        }

        if ( xModLibContImport.is() || xDlgLibContImport.is() )
        {
            LibDialog* pLibDlg = 0;

            Reference< script::XLibraryContainer > xModLibContImp( xModLibContImport, UNO_QUERY );
            Reference< script::XLibraryContainer > xDlgLibContImp( xDlgLibContImport, UNO_QUERY );
            Sequence< ::rtl::OUString > aLibNames = BasicIDE::GetMergedLibraryNames( xModLibContImp, xDlgLibContImp );
            sal_Int32 nLibCount = aLibNames.getLength();
            const ::rtl::OUString* pLibNames = aLibNames.getConstArray();
            for ( sal_Int32 i = 0 ; i < nLibCount ; i++ )
            {
                // library import dialog
                if ( !pLibDlg )
                {
                    pLibDlg = new LibDialog( this );
                    pLibDlg->SetStorageName( aURLObj.getName() );
                    pLibDlg->GetLibBox().SetMode( LIBMODE_CHOOSER );
                }

                // libbox entries
                String aLibName( pLibNames[ i ] );
                String aOULibName( aLibName );
                if ( !( ( xModLibContImport.is() && xModLibContImport->hasByName( aOULibName ) && xModLibContImport->isLibraryLink( aOULibName ) ) ||
                        ( xDlgLibContImport.is() && xDlgLibContImport->hasByName( aOULibName ) && xDlgLibContImport->isLibraryLink( aOULibName ) ) ) )
                {
                    SvLBoxEntry* pEntry = pLibDlg->GetLibBox().InsertEntry( aLibName );
                    USHORT nPos = (USHORT) pLibDlg->GetLibBox().GetModel()->GetAbsPos( pEntry );
                    pLibDlg->GetLibBox().CheckEntryPos( nPos, TRUE);
                }
            }

            if ( !pLibDlg )
                InfoBox( this, String( IDEResId( RID_STR_NOLIBINSTORAGE ) ) ).Execute();
            else
            {
                BOOL bChanges = FALSE;
                String aExtension( aURLObj.getExtension() );
                String aLibExtension( String::CreateFromAscii( "xlb" ) );
                String aContExtension( String::CreateFromAscii( "xlc" ) );

                // disable reference checkbox for documents and sbls
                if ( aExtension != aLibExtension && aExtension != aContExtension )
                    pLibDlg->EnableReference( FALSE );

                if ( pLibDlg->Execute() )
                {
                    ULONG nNewPos = aLibBox.GetEntryCount();
                    BOOL bRemove = FALSE;
                    BOOL bReplace = pLibDlg->IsReplace();
                    BOOL bReference = pLibDlg->IsReference();
                    for ( USHORT nLib = 0; nLib < pLibDlg->GetLibBox().GetEntryCount(); nLib++ )
                    {
                        if ( pLibDlg->GetLibBox().IsChecked( nLib ) )
                        {
                            SvLBoxEntry* pEntry = pLibDlg->GetLibBox().GetEntry( nLib );
                            DBG_ASSERT( pEntry, "Entry?!" );
                            String aLibName( pLibDlg->GetLibBox().GetEntryText( pEntry, 0 ) );
                            ::rtl::OUString aOULibName( aLibName );
                            Reference< script::XLibraryContainer2 > xModLibContainer( BasicIDE::GetModuleLibraryContainer( m_pCurShell ), UNO_QUERY );
                            Reference< script::XLibraryContainer2 > xDlgLibContainer( BasicIDE::GetDialogLibraryContainer( m_pCurShell ), UNO_QUERY );

                            // check, if the library is already existing
                            if ( ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) ) ||
                                 ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) ) )
                            {
                                if ( bReplace )
                                {
                                    // check, if the library is the Standard library
                                    if ( aLibName.EqualsAscii( "Standard" ) )
                                    {
                                        ErrorBox( this, WB_OK | WB_DEF_OK, String( IDEResId( RID_STR_REPLACESTDLIB ) ) ).Execute();
                                        continue;
                                    }

                                    // check, if the library is readonly and not a link
                                    if ( ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) && xModLibContainer->isLibraryReadOnly( aOULibName ) && !xModLibContainer->isLibraryLink( aOULibName ) ) ||
                                         ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) && xDlgLibContainer->isLibraryReadOnly( aOULibName ) && !xDlgLibContainer->isLibraryLink( aOULibName ) ) )
                                    {
                                        String aErrStr( IDEResId( RID_STR_REPLACELIB ) );
                                        aErrStr.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "XX" ) ), aLibName );
                                        aErrStr += '\n';
                                        aErrStr += String( IDEResId( RID_STR_LIBISREADONLY ) );
                                        ErrorBox( this, WB_OK | WB_DEF_OK, aErrStr ).Execute();
                                        continue;
                                    }

                                    // remove existing libraries
                                    bRemove = TRUE;
                                }
                                else
                                {
                                    String aErrStr;
                                    if ( bReference )
                                        aErrStr = String( IDEResId( RID_STR_REFNOTPOSSIBLE ) );
                                    else
                                        aErrStr = String( IDEResId( RID_STR_IMPORTNOTPOSSIBLE ) );
                                    aErrStr.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "XX" ) ), aLibName );
                                    aErrStr += '\n';
                                    aErrStr += String( IDEResId( RID_STR_SBXNAMEALLREADYUSED ) );
                                    ErrorBox( this, WB_OK | WB_DEF_OK, aErrStr ).Execute();
                                    continue;
                                }
                            }

                            // check, if the library is password protected
                            BOOL bOK = FALSE;
                            String aPassword;
                            if ( xModLibContImport.is() && xModLibContImport->hasByName( aOULibName ) )
                            {
                                Reference< script::XLibraryContainerPassword > xPasswd( xModLibContImport, UNO_QUERY );
                                if ( xPasswd.is() && xPasswd->isLibraryPasswordProtected( aOULibName ) && !xPasswd->isLibraryPasswordVerified( aOULibName ) && !bReference )
                                {
                                    bOK = QueryPassword( xModLibContImp, aLibName, aPassword, TRUE, TRUE );

                                    if ( !bOK )
                                    {
                                        String aErrStr( IDEResId( RID_STR_NOIMPORT ) );
                                        aErrStr.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "XX" ) ), aLibName );
                                        ErrorBox( this, WB_OK | WB_DEF_OK, aErrStr ).Execute();
                                        continue;
                                    }
                                }
                            }

                            // remove existing libraries
                            if ( bRemove )
                            {
                                // remove listbox entry
                                SvLBoxEntry* pEntry = aLibBox.FindEntry( aLibName );
                                if ( pEntry )
                                    aLibBox.SvTreeListBox::GetModel()->Remove( pEntry );

                                // remove module library
                                if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) )
                                    xModLibContainer->removeLibrary( aOULibName );

                                // remove dialog library
                                if ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) )
                                    xDlgLibContainer->removeLibrary( aOULibName );
                            }

                            // copy module library
                            if ( xModLibContImport.is() && xModLibContImport->hasByName( aOULibName ) && xModLibContainer.is() && !xModLibContainer->hasByName( aOULibName ) )
                            {
                                Reference< container::XNameContainer > xModLib;
                                if ( bReference )
                                {
                                    // storage URL
                                    INetURLObject aModStorageURLObj( aModURLObj );
                                    if ( aExtension == aContExtension )
                                    {
                                        sal_Int32 nCount = aModStorageURLObj.getSegmentCount();
                                        aModStorageURLObj.insertName( aLibName, false, nCount-1 );
                                        aModStorageURLObj.setExtension( aLibExtension );
                                        aModStorageURLObj.setFinalSlash();
                                    }
                                    ::rtl::OUString aModStorageURL( aModStorageURLObj.GetMainURL( INetURLObject::NO_DECODE ) );

                                    // create library link
                                    xModLib = Reference< container::XNameContainer >( xModLibContainer->createLibraryLink( aOULibName, aModStorageURL, TRUE ), UNO_QUERY);
                                }
                                else
                                {
                                    // create library
                                    xModLib = xModLibContainer->createLibrary( aOULibName );
                                    if ( xModLib.is() )
                                    {
                                        // get import library
                                        Reference< container::XNameContainer > xModLibImport;
                                        Any aElement = xModLibContImport->getByName( aOULibName );
                                        aElement >>= xModLibImport;

                                        if ( xModLibImport.is() )
                                        {
                                            // load library
                                            if ( !xModLibContImport->isLibraryLoaded( aOULibName ) )
                                                xModLibContImport->loadLibrary( aOULibName );

                                            // copy all modules
                                            Sequence< ::rtl::OUString > aModNames = xModLibImport->getElementNames();
                                            sal_Int32 nModCount = aModNames.getLength();
                                            const ::rtl::OUString* pModNames = aModNames.getConstArray();
                                            for ( sal_Int32 i = 0 ; i < nModCount ; i++ )
                                            {
                                                ::rtl::OUString aOUModName( pModNames[ i ] );
                                                Any aElement = xModLibImport->getByName( aOUModName );
                                                xModLib->insertByName( aOUModName, aElement );
                                            }

                                            // set password
                                            if ( bOK )
                                            {
                                                Reference< script::XLibraryContainerPassword > xPasswd( xModLibContainer, UNO_QUERY );
                                                if ( xPasswd.is() )
                                                {
                                                    try
                                                    {
                                                        ::rtl::OUString aOUPassword( aPassword );
                                                        xPasswd->changeLibraryPassword( aOULibName, ::rtl::OUString(), aOUPassword );
                                                    }
                                                    catch (...)
                                                    {
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            // copy dialog library
                            if ( xDlgLibContImport.is() && xDlgLibContImport->hasByName( aOULibName ) && xDlgLibContainer.is() && !xDlgLibContainer->hasByName( aOULibName ) )
                            {
                                Reference< container::XNameContainer > xDlgLib;
                                if ( bReference )
                                {
                                    // storage URL
                                    INetURLObject aDlgStorageURLObj( aDlgURLObj );
                                    if ( aExtension == aContExtension )
                                    {
                                        sal_Int32 nCount = aDlgStorageURLObj.getSegmentCount();
                                        aDlgStorageURLObj.insertName( aLibName, false, nCount - 1 );
                                        aDlgStorageURLObj.setExtension( aLibExtension );
                                        aDlgStorageURLObj.setFinalSlash();
                                    }
                                    ::rtl::OUString aDlgStorageURL( aDlgStorageURLObj.GetMainURL( INetURLObject::NO_DECODE ) );

                                    // create library link
                                    xDlgLib = Reference< container::XNameContainer >( xDlgLibContainer->createLibraryLink( aOULibName, aDlgStorageURL, TRUE ), UNO_QUERY);
                                }
                                else
                                {
                                    // create library
                                    xDlgLib = xDlgLibContainer->createLibrary( aOULibName );
                                    if ( xDlgLib.is() )
                                    {
                                        // get import library
                                        Reference< container::XNameContainer > xDlgLibImport;
                                        Any aElement = xDlgLibContImport->getByName( aOULibName );
                                        aElement >>= xDlgLibImport;

                                        if ( xDlgLibImport.is() )
                                        {
                                            // load library
                                            if ( !xDlgLibContImport->isLibraryLoaded( aOULibName ) )
                                                xDlgLibContImport->loadLibrary( aOULibName );

                                            // copy all dialogs
                                            Sequence< ::rtl::OUString > aDlgNames = xDlgLibImport->getElementNames();
                                            sal_Int32 nDlgCount = aDlgNames.getLength();
                                            const ::rtl::OUString* pDlgNames = aDlgNames.getConstArray();
                                            for ( sal_Int32 i = 0 ; i < nDlgCount ; i++ )
                                            {
                                                ::rtl::OUString aOUDlgName( pDlgNames[ i ] );
                                                Any aElement = xDlgLibImport->getByName( aOUDlgName );
                                                xDlgLib->insertByName( aOUDlgName, aElement );
                                            }
                                        }
                                    }
                                }
                            }

                            // insert listbox entry
                            ImpInsertLibEntry( aLibName, aLibBox.GetEntryCount() );
                            bChanges = TRUE;
                        }
                    }

                    SvLBoxEntry* pFirstNew = aLibBox.GetEntry( nNewPos );
                    if ( pFirstNew )
                        aLibBox.SetCurEntry( pFirstNew );
                }

                delete pLibDlg;
                if ( bChanges )
                    BasicIDE::MarkDocShellModified( m_pCurShell );
            }
        }
    }
}

//----------------------------------------------------------------------------

void LibPage::DeleteCurrent()
{
    SvLBoxEntry* pCurEntry = aLibBox.GetCurEntry();
    String aLibName( aLibBox.GetEntryText( pCurEntry, 0 ) );

    // check, if library is link
    BOOL bIsLibraryLink = FALSE;
    ::rtl::OUString aOULibName( aLibName );
    Reference< script::XLibraryContainer2 > xModLibContainer( BasicIDE::GetModuleLibraryContainer( m_pCurShell ), UNO_QUERY );
    Reference< script::XLibraryContainer2 > xDlgLibContainer( BasicIDE::GetDialogLibraryContainer( m_pCurShell ), UNO_QUERY );
    if ( ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) && xModLibContainer->isLibraryLink( aOULibName ) ) ||
         ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) && xDlgLibContainer->isLibraryLink( aOULibName ) ) )
    {
        bIsLibraryLink = TRUE;
    }

    if ( QueryDelLib( aLibName, bIsLibraryLink, this ) )
    {
        // inform BasicIDE
        SfxObjectShellItem aShellItem( SID_BASICIDE_ARG_SHELL, m_pCurShell );
        SfxStringItem aLibNameItem( SID_BASICIDE_ARG_LIBNAME, aLibName );
        BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
        SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
        SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
        if( pDispatcher )
        {
            pDispatcher->Execute( SID_BASICIDE_LIBREMOVED,
                                  SFX_CALLMODE_SYNCHRON, &aShellItem, &aLibNameItem, 0L );
        }

        // remove library from module and dialog library containers
        if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) )
            xModLibContainer->removeLibrary( aOULibName );
        if ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) )
            xDlgLibContainer->removeLibrary( aOULibName );

        ((SvLBox&)aLibBox).GetModel()->Remove( pCurEntry );
        BasicIDE::MarkDocShellModified( m_pCurShell );
    }
}

//----------------------------------------------------------------------------

void LibPage::EndTabDialog( USHORT nRet )
{
    DBG_ASSERT( pTabDlg, "TabDlg nicht gesetzt!" );
    if ( pTabDlg )
        pTabDlg->EndDialog( nRet );
}

//----------------------------------------------------------------------------

void LibPage::FillListBox()
{
    InsertListBoxEntry( 0, LIBRARY_LOCATION_USER );
    InsertListBoxEntry( 0, LIBRARY_LOCATION_SHARE );

    SfxObjectShell* pShell = SfxObjectShell::GetFirst();
    while ( pShell )
    {
        // only if there's a corresponding window (not for remote documents)
        if ( SfxViewFrame::GetFirst( pShell ) && !pShell->ISA( BasicDocShell ) )
            InsertListBoxEntry( pShell, LIBRARY_LOCATION_DOCUMENT );

        pShell = SfxObjectShell::GetNext( *pShell );
    }
}

//----------------------------------------------------------------------------

void LibPage::InsertListBoxEntry( SfxObjectShell* pShell, LibraryLocation eLocation )
{
    String aEntryText( BasicIDE::GetTitle( pShell, eLocation, SFX_TITLE_FILENAME ) );
    USHORT nPos = aBasicsBox.InsertEntry( aEntryText, LISTBOX_APPEND );
    aBasicsBox.SetEntryData( nPos, new BasicShellEntry( pShell, eLocation ) );
}

//----------------------------------------------------------------------------

void LibPage::SetCurLib()
{
    USHORT nSelPos = aBasicsBox.GetSelectEntryPos();
    BasicShellEntry* pEntry = (BasicShellEntry*)aBasicsBox.GetEntryData( nSelPos );
    if ( pEntry )
    {
        SfxObjectShell* pShell = pEntry->GetShell();
        LibraryLocation eLocation = pEntry->GetLocation();
        if ( pShell != m_pCurShell || eLocation != m_eCurLocation )
        {
            m_pCurShell = pShell;
            m_eCurLocation = eLocation;
            aLibBox.SetShell( pShell );
            aLibBox.Clear();

            // get a sorted list of library names
            Sequence< ::rtl::OUString > aLibNames = BasicIDE::GetLibraryNames( pShell );
            sal_Int32 nLibCount = aLibNames.getLength();
            const ::rtl::OUString* pLibNames = aLibNames.getConstArray();

            for ( sal_Int32 i = 0 ; i < nLibCount ; i++ )
            {
                String aLibName( pLibNames[ i ] );
                if ( eLocation == BasicIDE::GetLibraryLocation( pShell, aLibName ) )
                    ImpInsertLibEntry( aLibName, i );
            }

            SvLBoxEntry* pEntry = aLibBox.FindEntry( String::CreateFromAscii( "Standard" ) );
            if ( !pEntry )
                pEntry = aLibBox.GetEntry( 0 );
            aLibBox.SetCurEntry( pEntry );
        }
    }
}

//----------------------------------------------------------------------------

SvLBoxEntry* LibPage::ImpInsertLibEntry( const String& rLibName, ULONG nPos )
{
    // check, if library is password protected
    BOOL bProtected = FALSE;
    ::rtl::OUString aOULibName( rLibName );
    Reference< script::XLibraryContainer2 > xModLibContainer( BasicIDE::GetModuleLibraryContainer( m_pCurShell ), UNO_QUERY );
    if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) )
    {
        Reference< script::XLibraryContainerPassword > xPasswd( xModLibContainer, UNO_QUERY );
        if ( xPasswd.is() )
        {
            bProtected = xPasswd->isLibraryPasswordProtected( aOULibName );
        }
    }

    SvLBoxEntry* pNewEntry = aLibBox.InsertEntry( rLibName, nPos );
    pNewEntry->SetUserData( new BasicLibUserData( m_pCurShell ) );

    if (bProtected)
    {
        Image aImage(IDEResId(RID_IMG_LOCKED));
        aLibBox.SetExpandedEntryBmp(pNewEntry, aImage, BMP_COLOR_NORMAL);
        aLibBox.SetCollapsedEntryBmp(pNewEntry, aImage, BMP_COLOR_NORMAL);
        aImage = Image(IDEResId(RID_IMG_LOCKED_HC));
        aLibBox.SetExpandedEntryBmp(pNewEntry, aImage,
                                    BMP_COLOR_HIGHCONTRAST);
        aLibBox.SetCollapsedEntryBmp(pNewEntry, aImage,
                                     BMP_COLOR_HIGHCONTRAST);
    }

    // check, if library is link
    if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) && xModLibContainer->isLibraryLink( aOULibName ) )
    {
        String aLinkURL = xModLibContainer->getLibraryLinkURL( aOULibName );
        aLibBox.SetEntryText( aLinkURL, pNewEntry, 1 );
    }

    return pNewEntry;
}

//----------------------------------------------------------------------------

// Helper function
void createLibImpl( Window* pWin, SfxObjectShell* pShell,
                    BasicCheckBox* pLibBox, BasicTreeListBox* pBasicBox )
{
    // create library name
    String aLibName;
    String aLibStdName( String( RTL_CONSTASCII_USTRINGPARAM( "Library" ) ) );
    //String aLibStdName( IDEResId( RID_STR_STDLIBNAME ) );
    BOOL bValid = FALSE;
    USHORT i = 1;
    while ( !bValid )
    {
        aLibName = aLibStdName;
        aLibName += String::CreateFromInt32( i );
        if ( !BasicIDE::HasModuleLibrary( pShell, aLibName ) && !BasicIDE::HasDialogLibrary( pShell, aLibName ) )
            bValid = TRUE;
        i++;
    }

    std::auto_ptr< NewObjectDialog > xNewDlg( new NewObjectDialog( pWin, NEWOBJECTMODE_LIB ) );
    xNewDlg->SetObjectName( aLibName );

    if ( xNewDlg->Execute() )
    {
        if ( xNewDlg->GetObjectName().Len() )
            aLibName = xNewDlg->GetObjectName();

        if ( aLibName.Len() > 30 )
        {
            ErrorBox( pWin, WB_OK | WB_DEF_OK, String( IDEResId( RID_STR_LIBNAMETOLONG ) ) ).Execute();
        }
        else if ( !BasicIDE::IsValidSbxName( aLibName ) )
        {
            ErrorBox( pWin, WB_OK | WB_DEF_OK,
                        String( IDEResId( RID_STR_BADSBXNAME ) ) ).Execute();
        }
        else if ( BasicIDE::HasModuleLibrary( pShell, aLibName ) || BasicIDE::HasDialogLibrary( pShell, aLibName ) )
        {
            ErrorBox( pWin, WB_OK | WB_DEF_OK,
                        String( IDEResId( RID_STR_SBXNAMEALLREADYUSED2 ) ) ).Execute();
        }
        else
        {
            try
            {
                // create module and dialog library
                Reference< container::XNameContainer > xModLib = BasicIDE::CreateModuleLibrary( pShell, aLibName );
                Reference< container::XNameContainer > xDlgLib = BasicIDE::CreateDialogLibrary( pShell, aLibName );

                if( pLibBox )
                {
                    SvLBoxEntry* pEntry = pLibBox->InsertEntry( aLibName );
                    pEntry->SetUserData( new BasicLibUserData( pShell ) );
                    pLibBox->SetCurEntry( pEntry );
                }

                // create a module
                String aModName = BasicIDE::CreateModuleName( pShell, aLibName );
                ::rtl::OUString aModule = BasicIDE::CreateModule( pShell, aLibName, aModName, TRUE );
                SbxItem aSbxItem( SID_BASICIDE_ARG_SBX, pShell, aLibName, aModName, BASICIDE_TYPE_MODULE );
                BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
                SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
                SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
                if( pDispatcher )
                {
                    pDispatcher->Execute( SID_BASICIDE_SBXINSERTED,
                                          SFX_CALLMODE_SYNCHRON, &aSbxItem, 0L );
                }

                if( pBasicBox )
                {
                    SvLBoxEntry* pEntry = pBasicBox->GetCurEntry();
                    SvLBoxEntry* pRootEntry = NULL;
                    while( pEntry )
                    {
                        pRootEntry = pEntry;
                        pEntry = pBasicBox->GetParent( pEntry );
                    }

                    USHORT nMode = pBasicBox->GetMode();
                    bool bDlgMode = ( nMode & BROWSEMODE_DIALOGS ) && !( nMode & BROWSEMODE_MODULES );
                    USHORT nId = bDlgMode ? RID_IMG_DLGLIB : RID_IMG_MODLIB;
                    USHORT nIdHC = bDlgMode ? RID_IMG_DLGLIB_HC : RID_IMG_MODLIB_HC;
                    SvLBoxEntry* pNewLibEntry = pBasicBox->AddEntry(
                        aLibName,
                        Image( IDEResId( nId ) ),
                        Image( IDEResId( nIdHC ) ),
                        pRootEntry, false,
                        std::auto_ptr< BasicEntry >( new BasicEntry( OBJ_TYPE_LIBRARY ) ) );
                    DBG_ASSERT( pNewLibEntry, "InsertEntry fehlgeschlagen!" );

                    if( pNewLibEntry )
                    {
                        SvLBoxEntry* pEntry = pBasicBox->AddEntry(
                            aModName,
                            Image( IDEResId( RID_IMG_MODULE ) ),
                            Image( IDEResId( RID_IMG_MODULE_HC ) ),
                            pNewLibEntry, false,
                            std::auto_ptr< BasicEntry >( new BasicEntry( OBJ_TYPE_MODULE ) ) );
                        DBG_ASSERT( pEntry, "InsertEntry fehlgeschlagen!" );
                        pBasicBox->SetCurEntry( pEntry );
                        pBasicBox->Select( pBasicBox->GetCurEntry() );      // OV-Bug?!
                    }
                }
            }
            catch ( container::ElementExistException& e )
            {
                ByteString aBStr( String(e.Message), RTL_TEXTENCODING_ASCII_US );
                DBG_ERROR( aBStr.GetBuffer() );
            }
            catch ( container::NoSuchElementException& e )
            {
                ByteString aBStr( String(e.Message), RTL_TEXTENCODING_ASCII_US );
                DBG_ERROR( aBStr.GetBuffer() );
            }
        }
    }
}

//----------------------------------------------------------------------------

