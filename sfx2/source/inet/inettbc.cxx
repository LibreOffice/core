/*************************************************************************
 *
 *  $RCSfile: inettbc.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: hr $ $Date: 2003-09-29 14:52:50 $
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

#include "inettbc.hxx"

#pragma hdrstop

#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXCANCEL_HXX //autogen
#include <svtools/cancel.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_HISTORYOPTIONS_HXX
#include <svtools/historyoptions.hxx>
#endif
#ifndef SVTOOLS_FOLDER_RESTRICTION_HXX
#include <svtools/folderrestriction.hxx>
#endif
#include <vcl/toolbox.hxx>
#ifndef _VOS_THREAD_HXX //autogen
#include <vos/thread.hxx>
#endif
#ifndef _VOS_MUTEX_HXX //autogen
#include <vos/mutex.hxx>
#endif

#include <svtools/itemset.hxx>
#include <svtools/urihelper.hxx>
#include <svtools/pathoptions.hxx>
#include <svtools/asynclink.hxx>
#include <svtools/inettbc.hxx>

#include <unotools/localfilehelper.hxx>

#include "sfx.hrc"
#include "dispatch.hxx"
#include "viewfrm.hxx"
#include "objsh.hxx"
#include "referers.hxx"
#include "sfxtypes.hxx"
#include "helper.hxx"

//***************************************************************************
// SfxURLToolBoxControl_Impl
//***************************************************************************

SFX_IMPL_TOOLBOX_CONTROL(SfxURLToolBoxControl_Impl,SfxStringItem)

SfxURLToolBoxControl_Impl::SfxURLToolBoxControl_Impl( USHORT nId ,
                                                ToolBox& rBox ,
                                                SfxBindings& rBindings )
        : SfxToolBoxControl( nId , rBox , rBindings )
        , aURLForwarder( SID_CURRENT_URL, *this )
{
}

SvtURLBox* SfxURLToolBoxControl_Impl::GetURLBox() const
{
    return (SvtURLBox*) GetToolBox().GetItemWindow(GetId());
}

//***************************************************************************

void SfxURLToolBoxControl_Impl::OpenURL( const String& rName, BOOL bNew ) const
{
    String aName;
    String aFilter;
    String aOptions;

    INetURLObject aObj( rName );
    if ( aObj.GetProtocol() == INET_PROT_NOT_VALID )
    {
        String aBaseURL = GetURLBox()->GetBaseURL();
        aName = SvtURLBox::ParseSmart( rName, aBaseURL, SvtPathOptions().GetWorkPath() );
    }
    else
        aName = rName;

    if ( !aName.Len() )
        return;

    SfxViewFrame *pViewFrame = SfxViewFrame::Current();
    DBG_ASSERT( pViewFrame, "No ViewFrame ?!" );
    if ( pViewFrame )
    {
        pViewFrame = pViewFrame->GetTopViewFrame();
        SfxAllItemSet aSet( pViewFrame->GetPool() );
        aSet.Put( SfxStringItem( SID_FILE_NAME, aName ) );
        aSet.Put( SfxFrameItem( SID_DOCFRAME , pViewFrame ? pViewFrame->GetFrame() : 0 ) );
        aSet.Put( SfxStringItem( SID_REFERER, DEFINE_CONST_UNICODE(SFX_REFERER_USER) ) );
        aSet.Put( SfxStringItem( SID_TARGETNAME, String::CreateFromAscii("_default") ) );

        if ( aFilter.Len() )
        {
            aSet.Put( SfxStringItem( SID_FILTER_NAME, aFilter ) );
            aSet.Put( SfxStringItem( SID_FILE_FILTEROPTIONS, aOptions ) );
        }

        SFX_APP()->GetAppDispatcher_Impl()->Execute( SID_OPENURL, SFX_CALLMODE_RECORD, aSet );
    }
}

Window* SfxURLToolBoxControl_Impl::CreateItemWindow( Window* pParent )
{
    SvtURLBox* pURLBox = new SvtURLBox( pParent );
    pURLBox->SetOpenHdl( LINK( this, SfxURLToolBoxControl_Impl, OpenHdl ) );
    pURLBox->SetSelectHdl( LINK( this, SfxURLToolBoxControl_Impl, SelectHdl ) );
    return pURLBox;
}

IMPL_LINK( SfxURLToolBoxControl_Impl, SelectHdl, void*, pVoid )
{
    SvtURLBox* pURLBox = GetURLBox();
    String aName( pURLBox->GetURL() );

    if ( !pURLBox->IsTravelSelect() && aName.Len() )
    {
/*
        aName = URIHelper::SmartRelToAbs( aName );
        SfxPickList_Impl*  pPickList = SfxPickList_Impl::Get();
        SfxPickEntry_Impl* pEntry = pPickList->GetHistoryPickEntryFromTitle( aName );
        if ( !pEntry )
            pPickList->SetCurHistoryPos( pURLBox->GetEntryPos( aName ) );
 */
        OpenURL( aName, FALSE );
    }

    return 1L;
}

IMPL_LINK( SfxURLToolBoxControl_Impl, OpenHdl, void*, pVoid )
{
    SvtURLBox* pURLBox = GetURLBox();
    OpenURL( pURLBox->GetURL(), pURLBox->IsCtrlOpen() );
    SfxViewFrame* pFrm = SfxViewFrame::Current();
    if( pFrm )
        pFrm->GetFrame()->GrabFocusOnComponent_Impl();
    return 1L;
}

//***************************************************************************

void SfxURLToolBoxControl_Impl::StateChanged
(
    USHORT              nSID,
    SfxItemState        eState,
    const SfxPoolItem*  pState
)
{
    if ( nSID == SID_OPENURL )
    {
        // Disable URL box if command is disabled #111014#
        GetURLBox()->Enable( SFX_ITEM_DISABLED != eState );
    }

    if ( GetURLBox()->IsEnabled() )
    {
        if( nSID == SID_FOCUSURLBOX )
        {
            if ( GetURLBox()->IsVisible() )
                GetURLBox()->GrabFocus();
        }
        else if ( !GetURLBox()->IsModified() && SFX_ITEM_AVAILABLE == eState )
        {
            SvtURLBox* pURLBox = GetURLBox();
            pURLBox->Clear();

            ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > > lList = SvtHistoryOptions().GetList(eHISTORY);
            for (sal_Int32 i=0; i<lList.getLength(); ++i)
            {
                ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > lProps = lList[i];
                for (sal_Int32 p=0; p<lProps.getLength(); ++p)
                {
                    if (lProps[p].Name != HISTORY_PROPERTYNAME_URL)
                        continue;

                    ::rtl::OUString sURL;
                    if (!(lProps[p].Value>>=sURL) || !sURL.getLength())
                        continue;

                    INetURLObject aURL    ( sURL );
                    String        sMainURL( aURL.GetMainURL( INetURLObject::DECODE_WITH_CHARSET ) );
                    String        sFile;

                    if (::utl::LocalFileHelper::ConvertURLToSystemPath(sMainURL,sFile))
                        pURLBox->InsertEntry(sFile);
                    else
                        pURLBox->InsertEntry(sMainURL);
                }
            }

            const SfxStringItem *pURL = PTR_CAST(SfxStringItem,pState);
            String aRep( pURL->GetValue() );
            INetURLObject aURL( aRep );
            INetProtocol eProt = aURL.GetProtocol();
            if ( eProt == INET_PROT_FILE )
            {
                pURLBox->SetText( aURL.PathToFileName() );
            }
            else
                pURLBox->SetText( aURL.GetURLNoPass() );
        }
    }
}

//***************************************************************************
// SfxCancelToolBoxControl_Impl
//***************************************************************************

SFX_IMPL_TOOLBOX_CONTROL(SfxCancelToolBoxControl_Impl,SfxBoolItem)

//***************************************************************************

SfxCancelToolBoxControl_Impl::SfxCancelToolBoxControl_Impl
(
    USHORT nId,
    ToolBox& rBox,
    SfxBindings& rBindings
)
:   SfxToolBoxControl( nId, rBox, rBindings )
{
}

//***************************************************************************

SfxPopupWindowType SfxCancelToolBoxControl_Impl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONTIMEOUT;
}

//***************************************************************************

SfxPopupWindow* SfxCancelToolBoxControl_Impl::CreatePopupWindow()
{
    PopupMenu aMenu;
    BOOL bExecute = FALSE, bSeparator = FALSE;
    USHORT nIndex = 1;
    for ( SfxCancelManager *pCancelMgr = SfxViewFrame::Current()->GetTopViewFrame()->GetCancelManager();
          pCancelMgr;
          pCancelMgr = pCancelMgr->GetParent() )
    {
        for ( USHORT n=0; n<pCancelMgr->GetCancellableCount(); ++n )
        {
            if ( !n && bSeparator )
            {
                aMenu.InsertSeparator();
                bSeparator = FALSE;
            }
            String aItemText = pCancelMgr->GetCancellable(n)->GetTitle();
            if ( aItemText.Len() > 50 )
            {
                aItemText.Erase( 48 );
                aItemText += DEFINE_CONST_UNICODE("...");
            }
            aMenu.InsertItem( nIndex++, aItemText );
            bExecute = TRUE;
            bSeparator = TRUE;
        }
    }

    ToolBox& rToolBox = GetToolBox();
    USHORT nId = bExecute ? nId = aMenu.Execute( &rToolBox, rToolBox.GetPointerPosPixel() ) : 0;
    GetToolBox().EndSelection();
    ClearCache();
    UpdateSlot();
    if ( nId )
    {
        String aSearchText = aMenu.GetItemText(nId);
        for ( SfxCancelManager *pCancelMgr = SfxViewFrame::Current()->GetTopViewFrame()->GetCancelManager();
              pCancelMgr;
              pCancelMgr = pCancelMgr->GetParent() )
        {
            for ( USHORT n = 0; n < pCancelMgr->GetCancellableCount(); ++n )
            {
                SfxCancellable *pCancel = pCancelMgr->GetCancellable(n);
                String aItemText = pCancel->GetTitle();
                if ( aItemText.Len() > 50 )
                {
                    aItemText.Erase( 48 );
                    aItemText += DEFINE_CONST_UNICODE("...");
                }

                if ( aItemText == aSearchText )
                {
                    pCancel->Cancel();
                    return 0;
                }
            }
        }

    }

    return 0;
}

//***************************************************************************

void SfxCancelToolBoxControl_Impl::StateChanged
(
    USHORT              nSID,
    SfxItemState        eState,
    const SfxPoolItem*  pState
)
{
    SfxVoidItem aVoidItem( nSID );
    //SfxToolBoxControl::StateChanged( nSID, eState, pState ? &aVoidItem : 0 );
    SfxToolBoxControl::StateChanged( nSID, eState, pState );
}
