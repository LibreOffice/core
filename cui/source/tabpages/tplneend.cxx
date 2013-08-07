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

#include <tools/shl.hxx>
#include <tools/urlobj.hxx>
#include <vcl/msgbox.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/app.hxx>
#include <sfx2/filedlghelper.hxx>
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"

#include <cuires.hrc>
#include "helpid.hrc"
#include <svx/dialmgr.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdopath.hxx>
#include "svx/drawitem.hxx"
#include <svx/xpool.hxx>
#include <svx/xtable.hxx>
#include "cuitabline.hxx"
#include <svx/svxdlg.hxx>
#include <dialmgr.hxx>
#include "svx/dlgutil.hxx"
#include <basegfx/range/b2drange.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <svx/dialogs.hrc>

#define XOUT_WIDTH    150

SvxLineEndDefTabPage::SvxLineEndDefTabPage
(
    Window* pParent,
    const SfxItemSet& rInAttrs
) :

    SfxTabPage( pParent
              , "LineEndPage"
              , "cui/ui/lineendstabpage.ui"
              , rInAttrs ),
    rOutAttrs           ( rInAttrs ),
    pPolyObj            ( NULL ),

    pXPool              ( (XOutdevItemPool*) rInAttrs.GetPool() ),
    aXLStyle            ( XLINE_SOLID ),
    aXWidth             ( XOUT_WIDTH ),
    aXColor             ( String(), COL_BLACK ),
    aXLineAttr          ( pXPool ),
    rXLSet              ( aXLineAttr.GetItemSet() ),
    pLineEndList        ( NULL )
{
    get(m_pEdtName,"EDT_NAME");
    get(m_pLbLineEnds,"LB_LINEENDS");
    get(m_pBtnAdd,"BTN_ADD");
    get(m_pBtnModify,"BTN_MODIFY");
    get(m_pBtnDelete,"BTN_DELETE");
    get(m_pBtnLoad,"BTN_LOAD");
    get(m_pBtnSave,"BTN_SAVE");
    get(m_pCtlPreview,"CTL_PREVIEW");

    // this page needs ExchangeSupport
    SetExchangeSupport();

    rXLSet.Put( aXLStyle );
    rXLSet.Put( aXWidth );
    rXLSet.Put( aXColor );
    rXLSet.Put( XLineStartWidthItem( m_pCtlPreview->GetOutputSize().Height()  / 2 ) );
    rXLSet.Put( XLineEndWidthItem( m_pCtlPreview->GetOutputSize().Height() / 2 ) );

    // #i34740#
    m_pCtlPreview->SetLineAttributes(aXLineAttr.GetItemSet());

    m_pBtnAdd->SetClickHdl( LINK( this, SvxLineEndDefTabPage, ClickAddHdl_Impl ) );
    m_pBtnModify->SetClickHdl( LINK( this, SvxLineEndDefTabPage, ClickModifyHdl_Impl ) );
    m_pBtnDelete->SetClickHdl( LINK( this, SvxLineEndDefTabPage, ClickDeleteHdl_Impl ) );
    m_pBtnLoad->SetClickHdl( LINK( this, SvxLineEndDefTabPage, ClickLoadHdl_Impl ) );
    m_pBtnSave->SetClickHdl( LINK( this, SvxLineEndDefTabPage, ClickSaveHdl_Impl ) );

    m_pLbLineEnds->SetSelectHdl( LINK( this, SvxLineEndDefTabPage, SelectLineEndHdl_Impl ) );

}

void SvxLineEndDefTabPage::Resize()
{
    rXLSet.Put(XLineStartWidthItem(m_pCtlPreview->GetOutputSize().Height()  / 2 ));
    rXLSet.Put(XLineEndWidthItem(m_pCtlPreview->GetOutputSize().Height() / 2 ));
    SfxTabPage::Resize();
}

//------------------------------------------------------------------------

SvxLineEndDefTabPage::~SvxLineEndDefTabPage()
{
}

// -----------------------------------------------------------------------

void SvxLineEndDefTabPage::Construct()
{
    m_pLbLineEnds->Fill( pLineEndList );

    bool bCreateArrowPossible = true;

    if( !pPolyObj )
    {
        bCreateArrowPossible = false;
    }
    else if( !pPolyObj->ISA( SdrPathObj ) )
    {
        SdrObjTransformInfoRec aInfoRec;
        pPolyObj->TakeObjInfo( aInfoRec );
        SdrObject* pNewObj = 0;
        if( aInfoRec.bCanConvToPath )
            pNewObj = pPolyObj->ConvertToPolyObj( sal_True, sal_False );

        bCreateArrowPossible = pNewObj && pNewObj->ISA( SdrPathObj );
        SdrObject::Free( pNewObj );
    }

    if( !bCreateArrowPossible )
        m_pBtnAdd->Disable();
}

// -----------------------------------------------------------------------

void SvxLineEndDefTabPage::ActivatePage( const SfxItemSet& )
{
    if( *pDlgType == 0 ) // area dialog
    {
        // ActivatePage() is called before the dialog receives PageCreated() !!!
        if( pLineEndList.is() )
        {
            if( *pPosLineEndLb != LISTBOX_ENTRY_NOTFOUND )
            {
                m_pLbLineEnds->SelectEntryPos( *pPosLineEndLb );
                SelectLineEndHdl_Impl( this );
            }
            INetURLObject   aURL( pLineEndList->GetPath() );

            aURL.Append( pLineEndList->GetName() );
            DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );
            *pPageType = 0; // 3
            *pPosLineEndLb = LISTBOX_ENTRY_NOTFOUND;
        }
    }
}

// -----------------------------------------------------------------------

int SvxLineEndDefTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    CheckChanges_Impl();

    if( _pSet )
        FillItemSet( *_pSet );

    return( LEAVE_PAGE );
}

// -----------------------------------------------------------------------

void SvxLineEndDefTabPage::CheckChanges_Impl()
{
    sal_uInt16 nPos = m_pLbLineEnds->GetSelectEntryPos();

    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        OUString aString = m_pEdtName->GetText();

        if( aString != m_pLbLineEnds->GetSelectEntry() )
        {
            MessageDialog aQueryBox( GetParentDialog()
                                    ,"AskChangeLineEndDialog"
                                    ,"cui/ui/querychangelineenddialog.ui");

            if ( aQueryBox.Execute() == RET_YES )
                ClickModifyHdl_Impl( this );
        }
    }
    nPos = m_pLbLineEnds->GetSelectEntryPos();

    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        *pPosLineEndLb = nPos;
}

// -----------------------------------------------------------------------

sal_Bool SvxLineEndDefTabPage::FillItemSet( SfxItemSet& rSet )
{
    if( *pDlgType == 0 ) // line dialog
    {
        if( *pPageType == 3 )
        {
            CheckChanges_Impl();

            long nPos = m_pLbLineEnds->GetSelectEntryPos();
            XLineEndEntry* pEntry = pLineEndList->GetLineEnd( nPos );

            rSet.Put( XLineStartItem( pEntry->GetName(), pEntry->GetLineEnd() ) );
            rSet.Put( XLineEndItem( pEntry->GetName(), pEntry->GetLineEnd() ) );
        }
    }
    return( sal_True );
}

// -----------------------------------------------------------------------

void SvxLineEndDefTabPage::Reset( const SfxItemSet& )
{
    m_pLbLineEnds->SelectEntryPos( 0 );

    // Update lineend
    if( pLineEndList->Count() > 0 )
    {
        int nPos = m_pLbLineEnds->GetSelectEntryPos();

        XLineEndEntry* pEntry = pLineEndList->GetLineEnd( nPos );

        m_pEdtName->SetText( m_pLbLineEnds->GetSelectEntry() );

        rXLSet.Put( XLineStartItem( OUString(), pEntry->GetLineEnd() ) );
        rXLSet.Put( XLineEndItem( OUString(), pEntry->GetLineEnd() ) );

        // #i34740#
        m_pCtlPreview->SetLineAttributes(aXLineAttr.GetItemSet());

        m_pCtlPreview->Invalidate();
    }

    // determine button state
    if( pLineEndList->Count() )
    {
        m_pBtnModify->Enable();
        m_pBtnDelete->Enable();
        m_pBtnSave->Enable();
    }
    else
    {
        m_pBtnModify->Disable();
        m_pBtnDelete->Disable();
        m_pBtnSave->Disable();
    }
}

// -----------------------------------------------------------------------

SfxTabPage* SvxLineEndDefTabPage::Create( Window* pWindow, const SfxItemSet& rSet )
{
    return( new SvxLineEndDefTabPage( pWindow, rSet ) );
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxLineEndDefTabPage, SelectLineEndHdl_Impl)
{
    if( pLineEndList->Count() > 0 )
    {
        int nPos = m_pLbLineEnds->GetSelectEntryPos();

        XLineEndEntry* pEntry = pLineEndList->GetLineEnd( nPos );

        m_pEdtName->SetText( m_pLbLineEnds->GetSelectEntry() );

        rXLSet.Put( XLineStartItem( OUString(), pEntry->GetLineEnd() ) );
        rXLSet.Put( XLineEndItem( OUString(), pEntry->GetLineEnd() ) );

        // #i34740#
        m_pCtlPreview->SetLineAttributes(aXLineAttr.GetItemSet());

        m_pCtlPreview->Invalidate();

        // Is not set before, in order to only take the new style,
        // if there is an entry selected in the ListBox
        *pPageType = 3;
    }
    return( 0L );
}

//------------------------------------------------------------------------

long SvxLineEndDefTabPage::ChangePreviewHdl_Impl( void* )
{
    m_pCtlPreview->Invalidate();
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxLineEndDefTabPage, ClickModifyHdl_Impl)
{
    sal_uInt16 nPos = m_pLbLineEnds->GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        ResMgr& rMgr = CUI_MGR();
        OUString aDesc( ResId( RID_SVXSTR_DESC_LINEEND, rMgr ) );
        String aName( m_pEdtName->GetText() );
        long nCount = pLineEndList->Count();
        sal_Bool bDifferent = sal_True;

        // check whether the name is existing already
        for ( long i = 0; i < nCount && bDifferent; i++ )
            if ( aName == pLineEndList->GetLineEnd( i )->GetName() )
                bDifferent = sal_False;

        // if yes, repeat and demand a new name
        if ( !bDifferent )
        {
            MessageDialog aWarningBox( GetParentDialog()
                                      ,"DuplicateNameDialog"
                                      ,"cui/ui/queryduplicatedialog.ui");
            aWarningBox.Execute();

            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            DBG_ASSERT(pFact, "Dialogdiet fail!");
            AbstractSvxNameDialog* pDlg = pFact->CreateSvxNameDialog( GetParentDialog(), aName, aDesc );
            DBG_ASSERT(pDlg, "Dialogdiet fail!");
            sal_Bool bLoop = sal_True;

            while( !bDifferent && bLoop && pDlg->Execute() == RET_OK )
            {
                pDlg->GetName( aName );
                bDifferent = sal_True;

                for( long i = 0; i < nCount && bDifferent; i++ )
                {
                    if( aName == pLineEndList->GetLineEnd( i )->GetName() )
                        bDifferent = sal_False;
                }

                if( bDifferent )
                    bLoop = sal_False;
                else
                    aWarningBox.Execute();
            }
            delete( pDlg );
        }

        // if not existing, enter the entry
        if( bDifferent )
        {
            const XLineEndEntry* pEntry = pLineEndList->GetLineEnd( nPos );

            m_pEdtName->SetText( aName );

            const XLineEndEntry aEntry(pEntry->GetLineEnd(), aName);

            m_pLbLineEnds->Modify( aEntry, nPos, pLineEndList->GetUiBitmap( nPos ) );
            m_pLbLineEnds->SelectEntryPos( nPos );

            *pnLineEndListState |= CT_MODIFIED;

            *pPageType = 3;
        }
    }
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxLineEndDefTabPage, ClickAddHdl_Impl)
{
    if( pPolyObj )
    {
        const SdrObject* pNewObj;
        SdrObject* pConvPolyObj = NULL;

        if( pPolyObj->ISA( SdrPathObj ) )
        {
            pNewObj = pPolyObj;
        }
        else
        {
            SdrObjTransformInfoRec aInfoRec;
            pPolyObj->TakeObjInfo( aInfoRec );

            if( aInfoRec.bCanConvToPath )
            {
                pNewObj = pConvPolyObj = pPolyObj->ConvertToPolyObj( sal_True, sal_False );

                if( !pNewObj || !pNewObj->ISA( SdrPathObj ) )
                    return( 0L ); // cancel, additional safety, which
                            // has no use for group objects though.
            }
            else return( 0L ); // cancel
        }

        basegfx::B2DPolyPolygon aNewPolyPolygon(((SdrPathObj*)pNewObj)->GetPathPoly());
        basegfx::B2DRange aNewRange(basegfx::tools::getRange(aNewPolyPolygon));

        // normalize
        aNewPolyPolygon.transform(basegfx::tools::createTranslateB2DHomMatrix( -aNewRange.getMinX(), -aNewRange.getMinY()));

        SdrObject::Free( pConvPolyObj );

        XLineEndEntry* pEntry;

        ResMgr& rMgr = CUI_MGR();
        OUString aNewName( SVX_RES( RID_SVXSTR_LINEEND ) );
        OUString aDesc( ResId( RID_SVXSTR_DESC_LINEEND, rMgr ) );
        String aName;

        long nCount = pLineEndList->Count();
        long j = 1;
        sal_Bool bDifferent = sal_False;

        while ( !bDifferent )
        {
            aName = aNewName;
            aName += sal_Unicode(' ');
            aName += OUString::number( j++ );
            bDifferent = sal_True;

            for( long i = 0; i < nCount && bDifferent; i++ )
                if ( aName == pLineEndList->GetLineEnd( i )->GetName() )
                    bDifferent = sal_False;
        }

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        DBG_ASSERT(pFact, "Dialogdiet fail!");
        AbstractSvxNameDialog* pDlg = pFact->CreateSvxNameDialog( GetParentDialog(), aName, aDesc );
        DBG_ASSERT(pDlg, "Dialogdiet fail!");
        sal_Bool bLoop = sal_True;

        while ( bLoop && pDlg->Execute() == RET_OK )
        {
            pDlg->GetName( aName );
            bDifferent = sal_True;

            for( long i = 0; i < nCount && bDifferent; i++ )
            {
                if( aName == pLineEndList->GetLineEnd( i )->GetName() )
                    bDifferent = sal_False;
            }

            if( bDifferent )
            {
                bLoop = sal_False;
                pEntry = new XLineEndEntry( aNewPolyPolygon, aName );

                long nLineEndCount = pLineEndList->Count();
                pLineEndList->Insert( pEntry, nLineEndCount );

                // add to the ListBox
                m_pLbLineEnds->Append( *pEntry, pLineEndList->GetUiBitmap( nLineEndCount ) );
                m_pLbLineEnds->SelectEntryPos( m_pLbLineEnds->GetEntryCount() - 1 );

                *pnLineEndListState |= CT_MODIFIED;

                SelectLineEndHdl_Impl( this );
            }
            else
            {
                MessageDialog aBox( GetParentDialog()
                                   ,"DuplicateNameDialog"
                                   ,"cui/ui/queryduplicatedialog.ui");
                aBox.Execute();
            }
        }
        delete pDlg;
    }
    else
        m_pBtnAdd->Disable();

    // determine button state
    if ( pLineEndList->Count() )
    {
        m_pBtnModify->Enable();
        m_pBtnDelete->Enable();
        m_pBtnSave->Enable();
    }
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxLineEndDefTabPage, ClickDeleteHdl_Impl)
{
    sal_uInt16 nPos = m_pLbLineEnds->GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        MessageDialog aQueryBox( GetParentDialog()
                                ,"AskDelLineEndDialog"
                                ,"cui/ui/querydeletelineenddialog.ui");

        if ( aQueryBox.Execute() == RET_YES )
        {
            delete pLineEndList->Remove( nPos );
            m_pLbLineEnds->RemoveEntry( nPos );
            m_pLbLineEnds->SelectEntryPos( 0 );

            SelectLineEndHdl_Impl( this );
            *pPageType = 0; // LineEnd shall not be taken over

            *pnLineEndListState |= CT_MODIFIED;

            ChangePreviewHdl_Impl( this );
        }
    }
    // determine button state
    if( !pLineEndList->Count() )
    {
        m_pBtnModify->Disable();
        m_pBtnDelete->Disable();
        m_pBtnSave->Disable();
    }
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxLineEndDefTabPage, ClickLoadHdl_Impl)
{
    ResMgr& rMgr = CUI_MGR();
    sal_uInt16 nReturn = RET_YES;

    if ( *pnLineEndListState & CT_MODIFIED )
    {
        nReturn = MessageDialog( GetParentDialog()
                                ,"AskSaveList"
                                ,"cui/ui/querysavelistdialog.ui").Execute();

        if ( nReturn == RET_YES )
            pLineEndList->Save();
    }

    if ( nReturn != RET_CANCEL )
    {
        ::sfx2::FileDialogHelper aDlg(com::sun::star::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE, 0 );
        OUString aStrFilterType( "*.soe" );
        aDlg.AddFilter( aStrFilterType, aStrFilterType );
        INetURLObject aFile( SvtPathOptions().GetPalettePath() );
        aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::NO_DECODE ) );

        if( aDlg.Execute() == ERRCODE_NONE )
        {
            INetURLObject aURL( aDlg.GetPath() );
            INetURLObject aPathURL( aURL );

            aPathURL.removeSegment();
            aPathURL.removeFinalSlash();

            XLineEndListRef pLeList = XPropertyList::CreatePropertyList(XLINE_END_LIST, aPathURL.GetMainURL( INetURLObject::NO_DECODE ) )->AsLineEndList();
            pLeList->SetName( aURL.getName() );
            if( pLeList->Load() )
            {
                pLineEndList = pLeList;
                ( (SvxLineTabDialog*) GetParentDialog() )->SetNewLineEndList( pLineEndList );
                m_pLbLineEnds->Clear();
                m_pLbLineEnds->Fill( pLineEndList );
                Reset( rOutAttrs );

                pLineEndList->SetName( aURL.getName() );

                *pnLineEndListState |= CT_CHANGED;
                *pnLineEndListState &= ~CT_MODIFIED;
            }
            else
                MessageDialog( GetParentDialog()
                              ,"NoLoadedFileDialog"
                              ,"cui/ui/querynoloadedfiledialog.ui").Execute();
        }
    }

    // determine button state
    if ( pLineEndList->Count() )
    {
        m_pBtnModify->Enable();
        m_pBtnDelete->Enable();
        m_pBtnSave->Enable();
    }
    else
    {
        m_pBtnModify->Disable();
        m_pBtnDelete->Disable();
        m_pBtnSave->Disable();
    }
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxLineEndDefTabPage, ClickSaveHdl_Impl)
{
    ::sfx2::FileDialogHelper aDlg( com::sun::star::ui::dialogs::TemplateDescription::FILESAVE_SIMPLE, 0 );
    OUString aStrFilterType( "*.soe" );
    aDlg.AddFilter( aStrFilterType, aStrFilterType );

    INetURLObject aFile( SvtPathOptions().GetPalettePath() );
    DBG_ASSERT( aFile.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    if( pLineEndList->GetName().Len() )
    {
        aFile.Append( pLineEndList->GetName() );

        if( aFile.getExtension().isEmpty() )
            aFile.SetExtension( "soe" );
    }

    aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::NO_DECODE ) );
    if ( aDlg.Execute() == ERRCODE_NONE )
    {
        INetURLObject   aURL( aDlg.GetPath() );
        INetURLObject   aPathURL( aURL );

        aPathURL.removeSegment();
        aPathURL.removeFinalSlash();

        pLineEndList->SetName( aURL.getName() );
        pLineEndList->SetPath( aPathURL.GetMainURL( INetURLObject::NO_DECODE ) );

        if( pLineEndList->Save() )
        {
            *pnLineEndListState |= CT_SAVED;
            *pnLineEndListState &= ~CT_MODIFIED;
        }
        else
        {
            ErrorBox( GetParentDialog(), WinBits( WB_OK ), OUString( CUI_RES( RID_SVXSTR_WRITE_DATA_ERROR ) ) ).Execute();
        }
    }
    return( 0L );
}

void SvxLineEndDefTabPage::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxTabPage::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        sal_uInt16 nOldSelect = m_pLbLineEnds->GetSelectEntryPos();
        m_pLbLineEnds->Clear();
        m_pLbLineEnds->Fill( pLineEndList );
        m_pLbLineEnds->SelectEntryPos( nOldSelect );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
