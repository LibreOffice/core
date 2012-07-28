/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "basidesh.hrc"
#include "objdlg.hrc"

#include "basidesh.hxx"
#include "iderdll.hxx"
#include "iderdll2.hxx"
#include "objdlg.hxx"

#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/itemset.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/taskpanelist.hxx>

ObjectCatalog::ObjectCatalog( Window * pParent )
    :BasicDockingWindow( pParent, IDEResId( RID_BASICIDE_OBJCAT ) )
    ,aMacroTreeList( this, IDEResId( RID_TLB_MACROS ) )
    ,aToolBox(this, IDEResId(RID_TB_TOOLBOX))
    ,aMacroDescr( this, IDEResId( RID_FT_MACRODESCR ) )
{
    FreeResource();

    aToolBox.SetOutStyle( TOOLBOX_STYLE_FLAT );
    aToolBox.SetSizePixel( aToolBox.CalcWindowSizePixel() );
    aToolBox.SetSelectHdl( LINK( this, ObjectCatalog, ToolBoxHdl ) );

    aMacroTreeList.SetStyle( WB_BORDER | WB_TABSTOP |
                             WB_HASLINES | WB_HASLINESATROOT |
                             WB_HASBUTTONS | WB_HASBUTTONSATROOT |
                             WB_HSCROLL );

    aMacroTreeList.SetSelectHdl( LINK( this, ObjectCatalog, TreeListHighlightHdl ) );
    aMacroTreeList.SetAccessibleName(String(IDEResId(RID_STR_TLB_MACROS)));
    aMacroTreeList.ScanAllEntries();
    aMacroTreeList.GrabFocus();

    CheckButtons();

    Point aPos = BasicIDEGlobals::GetExtraData()->GetObjectCatalogPos();
    Size aSize = BasicIDEGlobals::GetExtraData()->GetObjectCatalogSize();
    if ( aPos.X() == INVPOSITION )
    {
        // centered after AppWin:
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

    Resize();   // so that the resize-handler arranges the controls

    // make object catalog keyboard accessible
    pParent->GetSystemWindow()->GetTaskPaneList()->AddWindow( this );
}

ObjectCatalog::~ObjectCatalog()
{
    GetParent()->GetSystemWindow()->GetTaskPaneList()->RemoveWindow( this );
}

void ObjectCatalog::Paint( const Rectangle& )
{
    String sOC = GetText();
    long nPos = GetSizePixel().Width()/2-GetTextWidth(sOC)/2;
    DrawText( Point( nPos, 10 ), String( sOC ) );
}

void ObjectCatalog::Move()
{
    BasicIDEGlobals::GetExtraData()->SetObjectCatalogPos( GetPosPixel() );
}

sal_Bool ObjectCatalog::Close()
{
    aCancelHdl.Call( this );
    return sal_True;
}

void ObjectCatalog::Resize()
{
    Size aOutSz = GetOutputSizePixel();
    BasicIDEGlobals::GetExtraData()->SetObjectCatalogSize( aOutSz );

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

    // the buttons above always stay unmodified
}

IMPL_LINK( ObjectCatalog, ToolBoxHdl, ToolBox*, pToolBox )
{
    sal_uInt16 nCurItem = pToolBox->GetCurItemId();
    switch ( nCurItem )
    {
        case TBITEM_SHOW:
        {
            SfxAllItemSet aArgs( SFX_APP()->GetPool() );
            SfxRequest aRequest( SID_BASICIDE_APPEAR, SFX_CALLMODE_SYNCHRON, aArgs );
            SFX_APP()->ExecuteSlot( aRequest );

            SvLBoxEntry* pCurEntry = aMacroTreeList.GetCurEntry();
            DBG_ASSERT( pCurEntry, "Entry?!" );
            BasicEntryDescriptor aDesc( aMacroTreeList.GetEntryDescriptor( pCurEntry ) );
            BasicIDEShell* pIDEShell = BasicIDEGlobals::GetShell();
            SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
            SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
            if ( aDesc.GetType() == OBJ_TYPE_MODULE ||
                 aDesc.GetType() == OBJ_TYPE_DIALOG ||
                 aDesc.GetType() == OBJ_TYPE_METHOD )
            {
                if( pDispatcher )
                {
                    SbxItem aSbxItem( SID_BASICIDE_ARG_SBX, aDesc.GetDocument(), aDesc.GetLibName(), aDesc.GetName(),
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
        aToolBox.EnableItem( TBITEM_SHOW, sal_True );
    else
        aToolBox.EnableItem( TBITEM_SHOW, sal_False );
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

void ObjectCatalog::SetCurrentEntry (IDEBaseWindow* pCurWin)
{
    BasicEntryDescriptor aDesc;
    if (pCurWin)
        aDesc = pCurWin->CreateEntryDescriptor();
    aMacroTreeList.SetCurrentEntry(aDesc);
}

ObjectCatalogToolBox_Impl::ObjectCatalogToolBox_Impl(
    Window * pParent, ResId const & rResId)
    : ToolBox(pParent, rResId)
    , m_aImagesNormal(GetImageList())
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
    SetImageList(m_aImagesNormal);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
