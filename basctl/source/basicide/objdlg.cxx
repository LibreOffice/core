/*************************************************************************
 *
 *  $RCSfile: objdlg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ab $ $Date: 2000-11-28 14:12:28 $
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


#include <ide_pch.hxx>

#pragma hdrstop

#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
#endif
#ifndef _SV_CMDEVT_HXX
#include <vcl/cmdevt.hxx>
#endif
#include <vcl/sound.hxx>
#include <objdlg.hrc>
#include <objdlg.hxx>
#include <bastypes.hxx>
#include <basidesh.hrc>
#include <moptions.hxx>
#include <iderdll.hxx>
#include <iderdll2.hxx>
#include <sbxitem.hxx>

//#ifndef _SFX_HELP_HXX //autogen
//#include <sfx2/sfxhelp.hxx>
//#endif

#ifndef _SV_CLIP_HXX //autogen
#include <vcl/clip.hxx>
#endif

#ifndef _SV_DRAG_HXX //autogen
#include <vcl/drag.hxx>
#endif




ObjectTreeListBox::ObjectTreeListBox( Window* pParent, const ResId& rRes )
    : BasicTreeListBox( pParent, rRes )
{
}

ObjectTreeListBox::~ObjectTreeListBox()
{
}

void ObjectTreeListBox::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == COMMAND_STARTDRAG )
    {
        String aText;

        BYTE nType = GetSelectedType();
        if ( ( nType == OBJTYPE_METHOD ) || ( nType == OBJTYPE_METHODINOBJ ) ||
              ( nType == OBJTYPE_PROPERTY ) || ( nType == OBJTYPE_SUBOBJ ) )
        {
            String aLib, aModOrObj, aSubOrPropOrSObj, aPropOrSubInSObj;
            GetSelectedSbx( aLib, aModOrObj, aSubOrPropOrSObj, aPropOrSubInSObj );
            aText = aModOrObj;
            aText += String( RTL_CONSTASCII_USTRINGPARAM( "." ) );
            aText += aSubOrPropOrSObj;
            if ( aPropOrSubInSObj.Len() )
            {
                aText += '.';
                aText += aPropOrSubInSObj;
            }

            DragServer::Clear();
            DragServer::CopyString( aText );
            // Region?
            ExecuteDrag( Pointer( POINTER_COPYDATA ), Pointer( POINTER_COPYDATA ), DRAG_COPYABLE );
            DragServer::Clear();
        }
    }
}

void ObjectTreeListBox::MouseButtonDown( const MouseEvent& rMEvt )
{
    BasicTreeListBox::MouseButtonDown( rMEvt );

    if ( rMEvt.IsLeft() && ( rMEvt.GetClicks() == 2 ) )
    {
        SbxVariable* pSbx = FindVariable( GetCurEntry() );
        if ( pSbx && pSbx->ISA( SbMethod ) )
        {
            SbxItem aSbxItem( SID_BASICIDE_ARG_SBX, pSbx );
            SfxViewFrame* pCurFrame = SfxViewFrame::Current();
            DBG_ASSERT( pCurFrame != NULL, "No current view frame!" );
            SfxDispatcher* pDispatcher = pCurFrame ? pCurFrame->GetDispatcher() : NULL;
            if( pDispatcher )
            {
                pDispatcher->Execute( SID_BASICIDE_SHOWSBX,
                                        SFX_CALLMODE_SYNCHRON, &aSbxItem, 0L );
            }
        }
    }
}



ObjectCatalog::ObjectCatalog( Window * pParent )
    : FloatingWindow( pParent, IDEResId( RID_BASICIDE_OBJCAT ) ),
        aMacroTreeList( this, IDEResId( RID_TLB_MACROS ) ),
//      aShowButton( this, IDEResId( RID_PB_SHOW ) ),
//      aOptionButton( this, IDEResId( RID_PB_OPTIONS ) ),
//      aLittleHelp( this, IDEResId( RID_PB_LITTLEHELP ) ),
        aToolBox( this, IDEResId( RID_TB_TOOLBOX ) ),
        aMacroDescr( this, IDEResId( RID_FT_MACRODESCR ) )
{
    FreeResource();

//  aShowButton.SetClickHdl( LINK( this, ObjectCatalog, ButtonHdl ) );
//  aOptionButton.SetClickHdl( LINK( this, ObjectCatalog, ButtonHdl ) );
//  aLittleHelp.SetClickHdl( LINK( this, ObjectCatalog, ButtonHdl ) );

    aToolBox.SetSizePixel( aToolBox.CalcWindowSizePixel() );
    aToolBox.SetSelectHdl( LINK( this, ObjectCatalog, ToolBoxHdl ) );


    aMacroTreeList.SetWindowBits( WB_HASLINES );
    aMacroTreeList.SetSelectHdl( LINK( this, ObjectCatalog, TreeListHighlightHdl ) );
    aMacroTreeList.SetDragDropMode( SV_DRAGDROP_CTRL_COPY );

    aMacroTreeList.ScanAllBasics();
    aMacroTreeList.ExpandAllTrees();
    CheckButtons();

    Point aPos = IDE_DLL()->GetExtraData()->GetObjectCatalogPos();
    Size aSize = IDE_DLL()->GetExtraData()->GetObjectCatalogSize();
    if ( aPos.X() == INVPOSITION )
    {
        // Zentriert nach AppWin:
        Window* pWin = GetParent();
        aPos = pWin->OutputToScreenPixel( Point( 0, 0 ) );
        Size aAppWinSz = pWin->GetSizePixel();
        Size aDlgWinSz = GetSizePixel();
        aPos.X() += aAppWinSz.Width() / 2;
        aPos.X() -= aDlgWinSz.Width() / 2;
        aPos.Y() += aAppWinSz.Height() / 2;
        aPos.Y() -= aDlgWinSz.Height() / 2;
    }
    SetPosPixel( aPos );
    if ( aSize.Width() )
        SetOutputSizePixel( aSize );

    Resize();   // damit der Resize-Handler die Controls anordnet
}


void __EXPORT ObjectCatalog::Move()
{
    IDE_DLL()->GetExtraData()->SetObjectCatalogPos( GetPosPixel() );
}

BOOL __EXPORT ObjectCatalog::Close()
{
    aCancelHdl.Call( this );
    return TRUE;
}

void __EXPORT ObjectCatalog::Resize()
{
    Size aOutSz = GetOutputSizePixel();
    IDE_DLL()->GetExtraData()->SetObjectCatalogSize( aOutSz );

    Point aTreePos = aMacroTreeList.GetPosPixel();
    Size aDescrSz = aMacroDescr.GetSizePixel();

    Size aTreeSz;
    long nCtrlWidth = aOutSz.Width() - 2*aTreePos.X();
    aTreeSz.Width() = nCtrlWidth;
    aTreeSz.Height() = aOutSz.Height() - aTreePos.Y() -
                        2*aTreePos.X() - aDescrSz.Height();

    if ( aTreeSz.Height() > 0 )
    {
        aMacroTreeList.SetSizePixel( aTreeSz );

        Point aDescrPos( aTreePos.X(), aTreePos.Y()+aTreeSz.Height()+aTreePos.X() );

        aMacroDescr.SetPosSizePixel( aDescrPos, Size( nCtrlWidth, aDescrSz.Height() ) );

        String aDesc = aMacroDescr.GetText();
        aMacroDescr.SetText(String());
        aMacroDescr.SetText(aDesc);
    }

    // Die Buttons oben bleiben immer unveraendert stehen...
}


void ObjectCatalog::ScanBasic( BasicManager* pBasMgr,  const String& rName  )
{
    aMacroTreeList.ScanBasic( pBasMgr, rName );
}



IMPL_LINK( ObjectCatalog, ToolBoxHdl, ToolBox*, pToolBox )
{
    USHORT nCurItem = pToolBox->GetCurItemId();
    switch ( nCurItem )
    {
        case TBITEM_SHOW:
        {
            SfxViewFrame* pCurFrame = SfxViewFrame::Current();
            DBG_ASSERT( pCurFrame != NULL, "No current view frame!" );
            SfxDispatcher* pDispatcher = pCurFrame ? pCurFrame->GetDispatcher() : NULL;
            if( pDispatcher )
            {
                pDispatcher->Execute( SID_BASICIDE_APPEAR, SFX_CALLMODE_SYNCHRON );
            }
            SvLBoxEntry* pCurEntry = aMacroTreeList.GetCurEntry();
            DBG_ASSERT( pCurEntry, "Entry?!" );
            SbxVariable* pSbx = aMacroTreeList.FindVariable(pCurEntry );
            if ( pSbx )
            {
                DBG_ASSERT( pSbx->ISA( SbxVariable ), "Var?!" );
                SbxItem aSbxItem( SID_BASICIDE_ARG_SBX, pSbx );
                if( pDispatcher )
                {
                    pDispatcher->Execute( SID_BASICIDE_SHOWSBX,
                                          SFX_CALLMODE_SYNCHRON, &aSbxItem, 0L );
                }
            }
            else
            {
                ErrorBox( this, WB_OK, String( IDEResId( RID_STR_OBJNOTFOUND ) ) ).Execute();
                aMacroTreeList.GetModel()->Remove( pCurEntry );
                CheckButtons();
            }
        }
        break;
        case TBITEM_PROPS:
        {
            SbxVariable* pVar = aMacroTreeList.FindVariable( aMacroTreeList.GetCurEntry() );
            DBG_ASSERT( pVar, "Variable nicht gefunden!" );
            if ( pVar )
            {
                MacroOptionsDialog* pDlg = new MacroOptionsDialog( this, pVar );
                pDlg->Execute();
                delete pDlg;
                UpdateFields();
            }
        }
        break;
        case TBITEM_HELP:
        {
            SvLBoxEntry* pCurEntry = aMacroTreeList.GetCurEntry();
            if ( pCurEntry )
            {
                BasicEntry* pInfo = (BasicEntry*)pCurEntry->GetUserData();
                DBG_ASSERT( pInfo, "Keine Info?" );
                SbxVariable* pVar = aMacroTreeList.FindVariable( pCurEntry );
                DBG_ASSERT( pVar, "Variable nicht gefunden!" );
                if ( pVar )
                {
                    SbxInfoRef xInfo = pVar->GetInfo();
                    if ( xInfo.Is() )
                    {
                        ByteString aHelpFile( xInfo->GetHelpFile(), RTL_TEXTENCODING_UTF8 );
                        USHORT nHelpId = (USHORT)xInfo->GetHelpId();
                        //SfxHelp::ShowHelp( nHelpId, TRUE, aHelpFile.GetBuffer() );
                    }
                }
            }
        }
        break;
    }

    return 0;
}



void ObjectCatalog::CheckButtons()
{
    SvLBoxEntry* pCurEntry = aMacroTreeList.GetCurEntry();

    BYTE nType = pCurEntry ? ((BasicEntry*)pCurEntry->GetUserData())->GetType() : 0;
    if ( ( nType == OBJTYPE_OBJECT ) || ( nType == OBJTYPE_MODULE ) ||
         ( nType == OBJTYPE_METHOD ) || ( nType == OBJTYPE_LIB ) )
    {
        aToolBox.EnableItem( TBITEM_PROPS, TRUE );
    }
    else
        aToolBox.EnableItem( TBITEM_PROPS, FALSE );

    if ( ( nType == OBJTYPE_OBJECT ) || ( nType == OBJTYPE_MODULE ) )
    {
        aToolBox.EnableItem( TBITEM_SHOW, TRUE );
    }
    else if ( ( nType == OBJTYPE_METHOD ) || ( nType == OBJTYPE_METHODINOBJ ) )
    {
        if ( nType == OBJTYPE_METHOD )
            aToolBox.EnableItem( TBITEM_SHOW, TRUE );
        else
            aToolBox.EnableItem( TBITEM_SHOW, FALSE );
    }
    else
    {
        aToolBox.EnableItem( TBITEM_SHOW, FALSE );
    }
}



IMPL_LINK_INLINE_START( ObjectCatalog, TreeListHighlightHdl, SvTreeListBox *, pBox )
{
    if ( pBox->IsSelected( pBox->GetHdlEntry() ) )
        UpdateFields();
    return 0;
}
IMPL_LINK_INLINE_END( ObjectCatalog, TreeListHighlightHdl, SvTreeListBox *, pBox )


/*
BYTE ObjectCatalog::GetSelectedType()
{
    return aMacroTreeList.GetSelectedType();
}

BasicManager* ObjectCatalog::GetSelectedSbx( String& rLib, String& rModOrObj, String& rSubOrProp )
{
    return aMacroTreeList.GetSelectedSbx( rLib, rModOrObj, rSubOrProp );
}

*/


void ObjectCatalog::UpdateFields()
{
    SvLBoxEntry* pCurEntry = aMacroTreeList.GetCurEntry();
    if ( pCurEntry )
    {
        CheckButtons();
        aMacroDescr.SetText( String() );
//      aMacroTreeList.SetHelpText( String() );
        aToolBox.EnableItem( TBITEM_HELP, FALSE );
        SbxVariable* pVar = aMacroTreeList.FindVariable( pCurEntry );
        if ( pVar )
        {
            SbxInfoRef xInfo = pVar->GetInfo();
            if ( xInfo.Is() )
            {
                aMacroDescr.SetText( xInfo->GetComment() );
//              aMacroTreeList.SetHelpText( xInfo->GetComment() );
                if ( xInfo->GetHelpFile().Len() )
                    aToolBox.EnableItem( TBITEM_HELP, TRUE );
            }
        }
    }
}


void ObjectCatalog::UpdateEntries()
{
    aMacroTreeList.UpdateEntries();
}

