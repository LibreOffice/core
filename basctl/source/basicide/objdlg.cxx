/*************************************************************************
 *
 *  $RCSfile: objdlg.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-23 12:07:23 $
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

#ifndef _SFX_IPFRM_HXX
#include <sfx2/ipfrm.hxx>
#endif

#include <ide_pch.hxx>

#pragma hdrstop

#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
#endif
#ifndef _SV_CMDEVT_HXX
#include <vcl/cmdevt.hxx>
#endif
#ifndef _SV_TASKPANELIST_HXX
#include <vcl/taskpanelist.hxx>
#endif
#include <vcl/sound.hxx>
#include <objdlg.hrc>
#include <objdlg.hxx>
#include <bastypes.hxx>
#include <basidesh.hrc>
#include <basidesh.hxx>
#include <moptions.hxx>
#include <iderdll.hxx>
#include <iderdll2.hxx>
#include <sbxitem.hxx>

//#ifndef _SFX_HELP_HXX //autogen
//#include <sfx2/sfxhelp.hxx>
//#endif


ObjectTreeListBox::ObjectTreeListBox( Window* pParent, const ResId& rRes )
    : BasicTreeListBox( pParent, rRes )
{
}

ObjectTreeListBox::~ObjectTreeListBox()
{
}

void ObjectTreeListBox::Command( const CommandEvent& rCEvt )
{
}

void ObjectTreeListBox::MouseButtonDown( const MouseEvent& rMEvt )
{
    BasicTreeListBox::MouseButtonDown( rMEvt );

    if ( rMEvt.IsLeft() && ( rMEvt.GetClicks() == 2 ) )
    {
        BasicEntryDescriptor aDesc( GetEntryDescriptor( GetCurEntry() ) );

        if ( aDesc.GetType() == OBJ_TYPE_METHOD )
        {
            BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
            SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
            SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
            if( pDispatcher )
            {
                SbxItem aSbxItem( SID_BASICIDE_ARG_SBX, aDesc.GetShell(), aDesc.GetLibName(), aDesc.GetName(),
                                  aDesc.GetMethodName(), ConvertType( aDesc.GetType() ) );
                pDispatcher->Execute( SID_BASICIDE_SHOWSBX,
                                        SFX_CALLMODE_SYNCHRON, &aSbxItem, 0L );
            }
        }
    }
}



ObjectCatalog::ObjectCatalog( Window * pParent )
    :FloatingWindow( pParent, IDEResId( RID_BASICIDE_OBJCAT ) )
    ,aMacroTreeList( this, IDEResId( RID_TLB_MACROS ) )
    ,aToolBox(this, IDEResId(RID_TB_TOOLBOX), IDEResId(RID_IMGLST_TB_HC))
    ,aMacroDescr( this, IDEResId( RID_FT_MACRODESCR ) )
{
    FreeResource();

    aToolBox.SetOutStyle( TOOLBOX_STYLE_FLAT );
    aToolBox.SetSizePixel( aToolBox.CalcWindowSizePixel() );
    aToolBox.SetSelectHdl( LINK( this, ObjectCatalog, ToolBoxHdl ) );

    aMacroTreeList.SetWindowBits( WB_HASLINES | WB_HASLINESATROOT |
                                  WB_HASBUTTONS | WB_HASBUTTONSATROOT |
                                  WB_HSCROLL );

    aMacroTreeList.SetSelectHdl( LINK( this, ObjectCatalog, TreeListHighlightHdl ) );

    aMacroTreeList.ScanAllEntries();
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

    // make object catalog keyboard accessible
    pParent->GetSystemWindow()->GetTaskPaneList()->AddWindow( this );
}

ObjectCatalog::~ObjectCatalog()
{
    GetParent()->GetSystemWindow()->GetTaskPaneList()->RemoveWindow( this );
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

IMPL_LINK( ObjectCatalog, ToolBoxHdl, ToolBox*, pToolBox )
{
    USHORT nCurItem = pToolBox->GetCurItemId();
    switch ( nCurItem )
    {
        case TBITEM_SHOW:
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
            SvLBoxEntry* pCurEntry = aMacroTreeList.GetCurEntry();
            DBG_ASSERT( pCurEntry, "Entry?!" );
            BasicEntryDescriptor aDesc( aMacroTreeList.GetEntryDescriptor( pCurEntry ) );
            BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
            pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
            pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
            if ( aDesc.GetType() == OBJ_TYPE_MODULE ||
                 aDesc.GetType() == OBJ_TYPE_DIALOG ||
                 aDesc.GetType() == OBJ_TYPE_METHOD )
            {
                if( pDispatcher )
                {
                    SbxItem aSbxItem( SID_BASICIDE_ARG_SBX, aDesc.GetShell(), aDesc.GetLibName(), aDesc.GetName(),
                                      aDesc.GetMethodName(), aMacroTreeList.ConvertType( aDesc.GetType() ) );
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
    }

    return 0;
}



void ObjectCatalog::CheckButtons()
{
    SvLBoxEntry* pCurEntry = aMacroTreeList.GetCurEntry();
    BasicEntryType eType = pCurEntry ? ((BasicEntry*)pCurEntry->GetUserData())->GetType() : OBJ_TYPE_UNKNOWN;
    if ( eType == OBJ_TYPE_DIALOG || eType == OBJ_TYPE_MODULE || eType == OBJ_TYPE_METHOD )
        aToolBox.EnableItem( TBITEM_SHOW, TRUE );
    else
        aToolBox.EnableItem( TBITEM_SHOW, FALSE );
}



IMPL_LINK_INLINE_START( ObjectCatalog, TreeListHighlightHdl, SvTreeListBox *, pBox )
{
    if ( pBox->IsSelected( pBox->GetHdlEntry() ) )
        UpdateFields();
    return 0;
}
IMPL_LINK_INLINE_END( ObjectCatalog, TreeListHighlightHdl, SvTreeListBox *, pBox )


void ObjectCatalog::UpdateFields()
{
    SvLBoxEntry* pCurEntry = aMacroTreeList.GetCurEntry();
    if ( pCurEntry )
    {
        CheckButtons();
        aMacroDescr.SetText( String() );
        SbxVariable* pVar = aMacroTreeList.FindVariable( pCurEntry );
        if ( pVar )
        {
            SbxInfoRef xInfo = pVar->GetInfo();
            if ( xInfo.Is() )
                aMacroDescr.SetText( xInfo->GetComment() );
        }
    }
}


void ObjectCatalog::UpdateEntries()
{
    aMacroTreeList.UpdateEntries();
}

void ObjectCatalog::SetCurrentEntry( BasicEntryDescriptor& rDesc )
{
    aMacroTreeList.SetCurrentEntry( rDesc );
}

ObjectCatalogToolBox_Impl::ObjectCatalogToolBox_Impl(
    Window * pParent, ResId const & rResId,
    ResId const & rImagesHighContrastId):
    ToolBox(pParent, rResId),
    m_aImagesNormal(GetImageList()),
    m_aImagesHighContrast(rImagesHighContrastId),
    m_bHighContrast(false)
{
    setImages();
}

// virtual
void ObjectCatalogToolBox_Impl::DataChanged(DataChangedEvent const & rDCEvt)
{
    ToolBox::DataChanged(rDCEvt);
    if ((rDCEvt.GetType() == DATACHANGED_SETTINGS
         || rDCEvt.GetType() == DATACHANGED_DISPLAY)
        && (rDCEvt.GetFlags() & SETTINGS_STYLE) != 0)
        setImages();
}

void ObjectCatalogToolBox_Impl::setImages()
{
    bool bHC = GetDisplayBackground().GetColor().IsDark();
    if (bHC != m_bHighContrast)
    {
        SetImageList(bHC ? m_aImagesHighContrast : m_aImagesNormal);
        m_bHighContrast = bHC;
    }
}
