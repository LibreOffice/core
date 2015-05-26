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

#include <vcl/wrkwin.hxx>
#include <tools/urlobj.hxx>
#include <vcl/msgbox.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dialoghelper.hxx>
#include <sfx2/filedlghelper.hxx>
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"

#include <cuires.hrc>
#include "helpid.hrc"
#include "svx/xattr.hxx"
#include <svx/xtable.hxx>
#include <svx/xpool.hxx>
#include "svx/drawitem.hxx"
#include "cuitabarea.hxx"
#include "defdlgname.hxx"
#include "dlgname.hxx"
#include <svx/svxdlg.hxx>
#include <dialmgr.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>
#include "paragrph.hrc"
#include <boost/scoped_ptr.hpp>

using namespace com::sun::star;

SvxGradientTabPage::SvxGradientTabPage
(
    vcl::Window* pParent,
    const SfxItemSet& rInAttrs
) :
    SfxTabPage          ( pParent, "GradientPage", "cui/ui/gradientpage.ui", &rInAttrs ),

    rOutAttrs           ( rInAttrs ),

    pnGradientListState ( 0 ),
    pnColorListState    ( 0 ),
    pPageType           ( 0 ),
    nDlgType            ( 0 ),
    pPos                ( 0 ),
    pbAreaTP            ( 0 ),

    pXPool              ( static_cast<XOutdevItemPool*>( rInAttrs.GetPool() )),
    aXFStyleItem        ( drawing::FillStyle_GRADIENT ),
    aXGradientItem      ( OUString(), XGradient( COL_BLACK, COL_WHITE ) ),
    aXFillAttr          ( pXPool ),
    rXFSet              ( aXFillAttr.GetItemSet() )
{
    get(m_pLbGradientType, "gradienttypelb");
    get(m_pFtCenterX,      "centerxft");
    get(m_pMtrCenterX,     "centerxmtr");
    get(m_pFtCenterY,      "centeryft");
    get(m_pMtrCenterY,     "centerymtr");
    get(m_pFtAngle,        "angleft");
    get(m_pMtrAngle,       "anglemtr");
    get(m_pMtrBorder,      "bordermtr");
    get(m_pLbColorFrom,    "colorfromlb");
    get(m_pMtrColorFrom,   "colorfrommtr");
    get(m_pLbColorTo,      "colortolb");
    get(m_pMtrColorTo,     "colortomtr");
    get(m_pLbGradients,    "gradientslb");
    Size aSize = getDrawListBoxOptimalSize(this);
    m_pLbGradients->set_width_request(aSize.Width());
    m_pLbGradients->set_height_request(aSize.Height());
    get(m_pCtlPreview,     "previewctl");
    aSize = getDrawPreviewOptimalSize(this);
    m_pCtlPreview->set_width_request(aSize.Width());
    m_pCtlPreview->set_height_request(aSize.Height());
    get(m_pBtnAdd,         "add");
    get(m_pBtnModify,      "modify");
    get(m_pBtnDelete,      "delete");
    get(m_pBtnLoad,        "load");
    get(m_pBtnSave,        "save");

    m_pLbGradients->SetAccessibleName(GetText());


    // this page needs ExchangeSupport
    SetExchangeSupport();

    // as long as NOT supported by the item

    m_pMtrColorTo->SetValue( 100 );
    m_pMtrColorFrom->SetValue( 100 );

    // setting the output device
    rXFSet.Put( aXFStyleItem );
    rXFSet.Put( aXGradientItem );
    m_pCtlPreview->SetAttributes( aXFillAttr.GetItemSet() );

    // set handler
    m_pLbGradients->SetSelectHdl(
        LINK( this, SvxGradientTabPage, ChangeGradientHdl_Impl ) );
    m_pBtnAdd->SetClickHdl( LINK( this, SvxGradientTabPage, ClickAddHdl_Impl ) );
    m_pBtnModify->SetClickHdl(
        LINK( this, SvxGradientTabPage, ClickModifyHdl_Impl ) );
    m_pBtnDelete->SetClickHdl(
        LINK( this, SvxGradientTabPage, ClickDeleteHdl_Impl ) );

    Link<> aLink = LINK( this, SvxGradientTabPage, ModifiedHdl_Impl );
    m_pLbGradientType->SetSelectHdl( aLink );
    m_pMtrCenterX->SetModifyHdl( aLink );
    m_pMtrCenterY->SetModifyHdl( aLink );
    m_pMtrAngle->SetModifyHdl( aLink );
    m_pMtrBorder->SetModifyHdl( aLink );
    m_pMtrColorFrom->SetModifyHdl( aLink );
    m_pLbColorFrom->SetSelectHdl( aLink );
    m_pMtrColorTo->SetModifyHdl( aLink );
    m_pLbColorTo->SetSelectHdl( aLink );

    m_pBtnLoad->SetClickHdl(
        LINK( this, SvxGradientTabPage, ClickLoadHdl_Impl ) );
    m_pBtnSave->SetClickHdl(
        LINK( this, SvxGradientTabPage, ClickSaveHdl_Impl ) );

    // #i76307# always paint the preview in LTR, because this is what the document does
    m_pCtlPreview->EnableRTL( false );

    setPreviewsToSamePlace(pParent, this);
}

SvxGradientTabPage::~SvxGradientTabPage()
{
    disposeOnce();
}

void SvxGradientTabPage::dispose()
{
    m_pLbGradientType.clear();
    m_pFtCenterX.clear();
    m_pMtrCenterX.clear();
    m_pFtCenterY.clear();
    m_pMtrCenterY.clear();
    m_pFtAngle.clear();
    m_pMtrAngle.clear();
    m_pMtrBorder.clear();
    m_pLbColorFrom.clear();
    m_pMtrColorFrom.clear();
    m_pLbColorTo.clear();
    m_pMtrColorTo.clear();
    m_pLbGradients.clear();
    m_pCtlPreview.clear();
    m_pBtnAdd.clear();
    m_pBtnModify.clear();
    m_pBtnDelete.clear();
    m_pBtnLoad.clear();
    m_pBtnSave.clear();
    SfxTabPage::dispose();
}


void SvxGradientTabPage::Construct()
{
    m_pLbColorFrom->Fill( pColorList );
    m_pLbColorTo->CopyEntries( *m_pLbColorFrom );

    m_pLbGradients->Fill( pGradientList );
}



void SvxGradientTabPage::ActivatePage( const SfxItemSet&  )
{
    sal_Int32 nPos;
    sal_Int32 nCount;

    if( nDlgType == 0 ) // area dialog
    {
        *pbAreaTP = false;

        if( pColorList.is() )
        {
            // ColorList
            if( *pnColorListState & ChangeType::CHANGED ||
                *pnColorListState & ChangeType::MODIFIED )
            {
                if( *pnColorListState & ChangeType::CHANGED )
                    pColorList = static_cast<SvxAreaTabDialog*>( GetParentDialog() )->GetNewColorList();

                // LbColorFrom
                nPos = m_pLbColorFrom->GetSelectEntryPos();
                m_pLbColorFrom->Clear();
                m_pLbColorFrom->Fill( pColorList );
                nCount = m_pLbColorFrom->GetEntryCount();
                if( nCount == 0 )
                    ; // this case should not occur
                else if( nCount <= nPos )
                    m_pLbColorFrom->SelectEntryPos( 0 );
                else
                    m_pLbColorFrom->SelectEntryPos( nPos );

                // LbColorTo
                nPos = m_pLbColorTo->GetSelectEntryPos();
                m_pLbColorTo->Clear();
                m_pLbColorTo->CopyEntries( *m_pLbColorFrom );
                nCount = m_pLbColorTo->GetEntryCount();
                if( nCount == 0 )
                    ; // this case should not occur
                else if( nCount <= nPos )
                    m_pLbColorTo->SelectEntryPos( 0 );
                else
                    m_pLbColorTo->SelectEntryPos( nPos );

                ModifiedHdl_Impl( this );
            }

            // determining (and possibly cutting) the name and
            // displaying it in the GroupBox
            OUString        aString( CUI_RES( RID_SVXSTR_TABLE ) );
            aString         += ": ";
            INetURLObject   aURL( pGradientList->GetPath() );

            aURL.Append( pGradientList->GetName() );
            DBG_ASSERT( aURL.GetProtocol() != INetProtocol::NotValid, "invalid URL" );

            if ( aURL.getBase().getLength() > 18 )
            {
                aString += aURL.getBase().copy( 0, 15 );
                aString += "...";
            }
            else
                aString += aURL.getBase();

            if ( *pPageType == PT_GRADIENT && *pPos != LISTBOX_ENTRY_NOTFOUND )
            {
                m_pLbGradients->SelectEntryPos( *pPos );
            }
            // colors could have been deleted
            ChangeGradientHdl_Impl( this );

            *pPageType = PT_GRADIENT;
            *pPos = LISTBOX_ENTRY_NOTFOUND;
        }
    }
}



SfxTabPage::sfxpg SvxGradientTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if( CheckChanges_Impl() == -1L )
        return KEEP_PAGE;

    if( _pSet )
        FillItemSet( _pSet );

    return LEAVE_PAGE;
}



long SvxGradientTabPage::CheckChanges_Impl()
{
    // is used here in order to NOT lose changes
    XGradient aTmpGradient( m_pLbColorFrom->GetSelectEntryColor(),
                          m_pLbColorTo->GetSelectEntryColor(),
                          (css::awt::GradientStyle) m_pLbGradientType->GetSelectEntryPos(),
                          static_cast<long>(m_pMtrAngle->GetValue() * 10), // should be changed in resource
                          (sal_uInt16) m_pMtrCenterX->GetValue(),
                          (sal_uInt16) m_pMtrCenterY->GetValue(),
                          (sal_uInt16) m_pMtrBorder->GetValue(),
                          (sal_uInt16) m_pMtrColorFrom->GetValue(),
                          (sal_uInt16) m_pMtrColorTo->GetValue() );

    sal_Int32 nPos = m_pLbGradients->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        XGradient aGradient = pGradientList->GetGradient( nPos )->GetGradient();

        if( !( aTmpGradient == aGradient ) )
        {
            ResMgr& rMgr = CUI_MGR();
            Image aWarningBoxImage = WarningBox::GetStandardImage();
            ScopedVclPtrInstance<SvxMessDialog> aMessDlg( GetParentDialog(),
                                                          SVX_RESSTR( RID_SVXSTR_GRADIENT ),
                                                          CUI_RESSTR( RID_SVXSTR_ASK_CHANGE_GRADIENT ),
                                                          &aWarningBoxImage );
            DBG_ASSERT(aMessDlg, "Dialog creation failed!");
            aMessDlg->SetButtonText( MESS_BTN_1,
                                    OUString( ResId( RID_SVXSTR_CHANGE, rMgr ) ) );
            aMessDlg->SetButtonText( MESS_BTN_2,
                                    OUString( ResId( RID_SVXSTR_ADD, rMgr ) ) );

            short nRet = aMessDlg->Execute();

            switch( nRet )
            {
                case RET_BTN_1:
                {
                    ClickModifyHdl_Impl( this );
                    aGradient = pGradientList->GetGradient( nPos )->GetGradient();
                }
                break;

                case RET_BTN_2:
                {
                    ClickAddHdl_Impl( this );
                    nPos = m_pLbGradients->GetSelectEntryPos();
                    aGradient = pGradientList->GetGradient( nPos )->GetGradient();
                }
                break;

                case RET_CANCEL:
                break;
            }
        }
    }
    nPos = m_pLbGradients->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        *pPos = nPos;
    }
    return 0L;
}



bool SvxGradientTabPage::FillItemSet( SfxItemSet* rSet )
{
    if( nDlgType == 0 && *pPageType == PT_GRADIENT && !*pbAreaTP )
    {
        // CheckChanges(); <-- duplicate inquiry ?

        boost::scoped_ptr<XGradient> pXGradient;
        OUString      aString;
        sal_Int32      nPos = m_pLbGradients->GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            pXGradient.reset(new XGradient( pGradientList->GetGradient( nPos )->GetGradient() ));
            aString = m_pLbGradients->GetSelectEntry();

        }
        else
        // gradient was passed (unidentified)
        {
            pXGradient.reset(new XGradient( m_pLbColorFrom->GetSelectEntryColor(),
                        m_pLbColorTo->GetSelectEntryColor(),
                        (css::awt::GradientStyle) m_pLbGradientType->GetSelectEntryPos(),
                        static_cast<long>(m_pMtrAngle->GetValue() * 10), // should be changed in resource
                        (sal_uInt16) m_pMtrCenterX->GetValue(),
                        (sal_uInt16) m_pMtrCenterY->GetValue(),
                        (sal_uInt16) m_pMtrBorder->GetValue(),
                        (sal_uInt16) m_pMtrColorFrom->GetValue(),
                        (sal_uInt16) m_pMtrColorTo->GetValue() ));
        }
        DBG_ASSERT( pXGradient, "XGradient konnte nicht erzeugt werden" );
        rSet->Put( XFillStyleItem( drawing::FillStyle_GRADIENT ) );
        rSet->Put( XFillGradientItem( aString, *pXGradient ) );
    }
    return true;
}



void SvxGradientTabPage::Reset( const SfxItemSet* )
{
    // m_pLbGradients->SelectEntryPos( 0 );
    ChangeGradientHdl_Impl( this );

    // determine state of the buttons
    if( pGradientList->Count() )
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



VclPtr<SfxTabPage> SvxGradientTabPage::Create( vcl::Window* pWindow,
                                               const SfxItemSet* rOutAttrs )
{
    return VclPtr<SvxGradientTabPage>::Create( pWindow, *rOutAttrs );
}



IMPL_LINK( SvxGradientTabPage, ModifiedHdl_Impl, void *, pControl )
{
    css::awt::GradientStyle eXGS = (css::awt::GradientStyle) m_pLbGradientType->GetSelectEntryPos();

    XGradient aXGradient( m_pLbColorFrom->GetSelectEntryColor(),
                          m_pLbColorTo->GetSelectEntryColor(),
                          eXGS,
                          static_cast<long>(m_pMtrAngle->GetValue() * 10), // should be changed in resource
                          (sal_uInt16) m_pMtrCenterX->GetValue(),
                          (sal_uInt16) m_pMtrCenterY->GetValue(),
                          (sal_uInt16) m_pMtrBorder->GetValue(),
                          (sal_uInt16) m_pMtrColorFrom->GetValue(),
                          (sal_uInt16) m_pMtrColorTo->GetValue() );

    // enable/disable controls
    if( pControl == m_pLbGradientType || pControl == this )
        SetControlState_Impl( eXGS );

    // displaying in XOutDev
    rXFSet.Put( XFillGradientItem( OUString(), aXGradient ) );
    m_pCtlPreview->SetAttributes( aXFillAttr.GetItemSet() );

    m_pCtlPreview->Invalidate();

    return 0L;
}



IMPL_LINK_NOARG(SvxGradientTabPage, ClickAddHdl_Impl)
{
    OUString aNewName( SVX_RES( RID_SVXSTR_GRADIENT ) );
    OUString aDesc( CUI_RES( RID_SVXSTR_DESC_GRADIENT ) );
    OUString aName;

    long nCount = pGradientList->Count();
    long j = 1;
    bool bDifferent = false;

    while( !bDifferent )
    {
        aName  = aNewName;
        aName += " ";
        aName += OUString::number( j++ );
        bDifferent = true;

        for( long i = 0; i < nCount && bDifferent; i++ )
            if( aName == pGradientList->GetGradient( i )->GetName() )
                bDifferent = false;
    }

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    boost::scoped_ptr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog( GetParentDialog(), aName, aDesc ));
    ScopedVclPtr<MessageDialog> pWarnBox;
    sal_uInt16 nError   = 1;

    while (pDlg->Execute() == RET_OK)
    {
        pDlg->GetName( aName );

        bDifferent = true;

        for (long i = 0; i < nCount && bDifferent; ++i)
        {
            if( aName == pGradientList->GetGradient( i )->GetName() )
                bDifferent = false;
        }

        if (bDifferent)
        {
            nError = 0;
            break;
        }

        if( !pWarnBox )
        {
            pWarnBox.reset(VclPtr<MessageDialog>::Create( GetParentDialog()
                                        ,"DuplicateNameDialog"
                                        ,"cui/ui/queryduplicatedialog.ui"));
        }

        if( pWarnBox->Execute() != RET_OK )
            break;
    }
    pDlg.reset();
    pWarnBox.reset();

    if( !nError )
    {
        XGradient aXGradient( m_pLbColorFrom->GetSelectEntryColor(),
                              m_pLbColorTo->GetSelectEntryColor(),
                              (css::awt::GradientStyle) m_pLbGradientType->GetSelectEntryPos(),
                              static_cast<long>(m_pMtrAngle->GetValue() * 10), // should be changed in resource
                              (sal_uInt16) m_pMtrCenterX->GetValue(),
                              (sal_uInt16) m_pMtrCenterY->GetValue(),
                              (sal_uInt16) m_pMtrBorder->GetValue(),
                              (sal_uInt16) m_pMtrColorFrom->GetValue(),
                              (sal_uInt16) m_pMtrColorTo->GetValue() );
        XGradientEntry* pEntry = new XGradientEntry( aXGradient, aName );

        pGradientList->Insert( pEntry, nCount );

        m_pLbGradients->Append( *pEntry, pGradientList->GetUiBitmap( nCount ) );

        m_pLbGradients->SelectEntryPos( m_pLbGradients->GetEntryCount() - 1 );

#ifdef WNT
        // hack: #31355# W.P.
        Rectangle aRect( m_pLbGradients->GetPosPixel(), m_pLbGradients->GetSizePixel() );
        if( sal_True ) {                // ??? overlapped with pDlg
                                    // and srolling
            Invalidate( aRect );
        }
#endif

        *pnGradientListState |= ChangeType::MODIFIED;

        ChangeGradientHdl_Impl( this );
    }

    // determine button state
    if( pGradientList->Count() )
    {
        m_pBtnModify->Enable();
        m_pBtnDelete->Enable();
        m_pBtnSave->Enable();
    }
    return 0L;
}



IMPL_LINK_NOARG(SvxGradientTabPage, ClickModifyHdl_Impl)
{
    sal_Int32 nPos = m_pLbGradients->GetSelectEntryPos();

    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        OUString aDesc( CUI_RES( RID_SVXSTR_DESC_GRADIENT ) );
        OUString aName( pGradientList->GetGradient( nPos )->GetName() );
        OUString aOldName = aName;

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        DBG_ASSERT(pFact, "Dialog creation failed!");
        boost::scoped_ptr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog( GetParentDialog(), aName, aDesc ));
        DBG_ASSERT(pDlg, "Dialog creation failed!");

        long nCount = pGradientList->Count();
        bool bLoop = true;

        while( bLoop && pDlg->Execute() == RET_OK )
        {
            pDlg->GetName( aName );
            bool bDifferent = true;

            for( long i = 0; i < nCount && bDifferent; i++ )
            {
                if( aName == pGradientList->GetGradient( i )->GetName() &&
                    aName != aOldName )
                    bDifferent = false;
            }

            if( bDifferent )
            {
                bLoop = false;
                XGradient aXGradient( m_pLbColorFrom->GetSelectEntryColor(),
                                      m_pLbColorTo->GetSelectEntryColor(),
                                      (css::awt::GradientStyle) m_pLbGradientType->GetSelectEntryPos(),
                                      static_cast<long>(m_pMtrAngle->GetValue() * 10), // should be changed in resource
                                      (sal_uInt16) m_pMtrCenterX->GetValue(),
                                      (sal_uInt16) m_pMtrCenterY->GetValue(),
                                      (sal_uInt16) m_pMtrBorder->GetValue(),
                                      (sal_uInt16) m_pMtrColorFrom->GetValue(),
                                      (sal_uInt16) m_pMtrColorTo->GetValue() );

                XGradientEntry* pEntry = new XGradientEntry( aXGradient, aName );

                delete pGradientList->Replace( pEntry, nPos );

                m_pLbGradients->Modify( *pEntry, nPos, pGradientList->GetUiBitmap( nPos ) );

                m_pLbGradients->SelectEntryPos( nPos );

                *pnGradientListState |= ChangeType::MODIFIED;
            }
            else
            {
                ScopedVclPtrInstance<MessageDialog> aBox( GetParentDialog()
                                                          ,"DuplicateNameDialog"
                                                          ,"cui/ui/queryduplicatedialog.ui" );
                aBox->Execute();
            }

        }
    }
    return 0L;
}



IMPL_LINK_NOARG(SvxGradientTabPage, ClickDeleteHdl_Impl)
{
    sal_Int32 nPos = m_pLbGradients->GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        ScopedVclPtrInstance< MessageDialog > aQueryBox( GetParentDialog(),"AskDelGradientDialog","cui/ui/querydeletegradientdialog.ui");

        if ( aQueryBox->Execute() == RET_YES )
        {
            delete pGradientList->Remove( nPos );
            m_pLbGradients->RemoveEntry( nPos );
            m_pLbGradients->SelectEntryPos( 0 );

            m_pCtlPreview->Invalidate();

            ChangeGradientHdl_Impl( this );

            *pnGradientListState |= ChangeType::MODIFIED;
        }
    }
    // determine button state
    if( !pGradientList->Count() )
    {
        m_pBtnModify->Disable();
        m_pBtnDelete->Disable();
        m_pBtnSave->Disable();
    }
    return 0L;
}



IMPL_LINK_NOARG(SvxGradientTabPage, ClickLoadHdl_Impl)
{
    ResMgr& rMgr = CUI_MGR();
    sal_uInt16 nReturn = RET_YES;

    if ( *pnGradientListState & ChangeType::MODIFIED )
    {
        nReturn = ScopedVclPtrInstance<MessageDialog>::Create( GetParentDialog()
                                 ,"AskSaveList"
                                 ,"cui/ui/querysavelistdialog.ui")->Execute();

        if ( nReturn == RET_YES )
            pGradientList->Save();
    }

    if ( nReturn != RET_CANCEL )
    {
        ::sfx2::FileDialogHelper aDlg( com::sun::star::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE, 0 );
        OUString aStrFilterType( "*.sog" );
        aDlg.AddFilter( aStrFilterType, aStrFilterType );
        INetURLObject aFile( SvtPathOptions().GetPalettePath() );
        aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::NO_DECODE ) );

        if( aDlg.Execute() == ERRCODE_NONE )
        {
            EnterWait();

            INetURLObject aURL( aDlg.GetPath() );
            INetURLObject aPathURL( aURL );

            aPathURL.removeSegment();
            aPathURL.removeFinalSlash();

            // save list
            XGradientListRef pGrdList = XPropertyList::AsGradientList(
                XPropertyList::CreatePropertyList(
                    XGRADIENT_LIST,
                    aPathURL.GetMainURL(INetURLObject::NO_DECODE), ""));
            pGrdList->SetName( aURL.getName() );

            if ( pGrdList->Load() )
            {
                pGradientList = pGrdList;
                static_cast<SvxAreaTabDialog*>( GetParentDialog() )->
                    SetNewGradientList( pGradientList );

                m_pLbGradients->Clear();
                m_pLbGradients->Fill( pGradientList );
                Reset( &rOutAttrs );

                pGradientList->SetName( aURL.getName() );

                // determining (possibly cutting) the name
                // and displaying it in the GroupBox
                OUString aString( ResId( RID_SVXSTR_TABLE, rMgr ) );
                aString += ": ";

                if ( aURL.getBase().getLength() > 18 )
                {
                    aString += aURL.getBase().copy( 0, 15 );
                    aString += "...";
                }
                else
                    aString += aURL.getBase();

                *pnGradientListState |= ChangeType::CHANGED;
                *pnGradientListState &= ~ChangeType::MODIFIED;
                LeaveWait();
            }
            else
            {
                LeaveWait();
                ScopedVclPtrInstance<MessageDialog>::Create( GetParentDialog()
                              ,"NoLoadedFileDialog"
                              ,"cui/ui/querynoloadedfiledialog.ui")->Execute();
            }
        }
    }

    // determine button state
    if( pGradientList->Count() )
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
    return 0L;
}



IMPL_LINK_NOARG(SvxGradientTabPage, ClickSaveHdl_Impl)
{
    ::sfx2::FileDialogHelper aDlg(
        com::sun::star::ui::dialogs::TemplateDescription::FILESAVE_SIMPLE, 0 );
    OUString aStrFilterType( "*.sog" );
    aDlg.AddFilter( aStrFilterType, aStrFilterType );

    INetURLObject aFile( SvtPathOptions().GetPalettePath() );
    DBG_ASSERT( aFile.GetProtocol() != INetProtocol::NotValid, "invalid URL" );

    if( !pGradientList->GetName().isEmpty() )
    {
        aFile.Append( pGradientList->GetName() );

        if( aFile.getExtension().isEmpty() )
            aFile.SetExtension( OUString("sog") );
    }

    aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::NO_DECODE ) );
    if ( aDlg.Execute() == ERRCODE_NONE )
    {
        INetURLObject   aURL( aDlg.GetPath() );
        INetURLObject   aPathURL( aURL );

        aPathURL.removeSegment();
        aPathURL.removeFinalSlash();

        pGradientList->SetName( aURL.getName() );
        pGradientList->SetPath( aPathURL.GetMainURL( INetURLObject::NO_DECODE ) );

        if( pGradientList->Save() )
        {
            // determining (possibly cutting) the name
            // and displaying it in the GroupBox
            OUString aString( CUI_RES( RID_SVXSTR_TABLE ) );
            aString  += ": ";

            if ( aURL.getBase().getLength() > 18 )
            {
                aString += aURL.getBase().copy( 0, 15 );
                aString += "...";
            }
            else
                aString += aURL.getBase();

            *pnGradientListState |= ChangeType::SAVED;
            *pnGradientListState &= ~ChangeType::MODIFIED;
        }
        else
        {
            ScopedVclPtrInstance<MessageDialog>::Create( GetParentDialog()
                          ,"NoSaveFileDialog"
                          ,"cui/ui/querynosavefiledialog.ui")->Execute();
        }
    }

    return 0L;
}



IMPL_LINK_NOARG(SvxGradientTabPage, ChangeGradientHdl_Impl)
{
    boost::scoped_ptr<XGradient> pGradient;
    int nPos = m_pLbGradients->GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
        pGradient.reset(new XGradient( pGradientList->GetGradient( nPos )->GetGradient() ));
    else
    {
        const SfxPoolItem* pPoolItem = NULL;
        if( SfxItemState::SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLSTYLE ), true, &pPoolItem ) )
        {
            if( ( drawing::FillStyle_GRADIENT == (drawing::FillStyle) static_cast<const XFillStyleItem*>( pPoolItem )->GetValue() ) &&
                ( SfxItemState::SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLGRADIENT ), true, &pPoolItem ) ) )
            {
                pGradient.reset(new XGradient( static_cast<const XFillGradientItem*>( pPoolItem )->GetGradientValue() ));
            }
        }
        if( !pGradient )
        {
            m_pLbGradients->SelectEntryPos( 0 );
            nPos = m_pLbGradients->GetSelectEntryPos();
            if( nPos != LISTBOX_ENTRY_NOTFOUND )
                pGradient.reset(new XGradient( pGradientList->GetGradient( nPos )->GetGradient() ));
        }
    }

    if( pGradient )
    {
        css::awt::GradientStyle eXGS = pGradient->GetGradientStyle();

        m_pLbGradientType->SelectEntryPos(
            sal::static_int_cast< sal_Int32 >( eXGS ) );
        // if the entry is not in the listbox,
        // colors are added temporarily
        m_pLbColorFrom->SetNoSelection();
        m_pLbColorFrom->SelectEntry( pGradient->GetStartColor() );

        if ( m_pLbColorFrom->GetSelectEntryCount() == 0 )
        {
            m_pLbColorFrom->InsertEntry( pGradient->GetStartColor(),
                                      OUString() );
            m_pLbColorFrom->SelectEntry( pGradient->GetStartColor() );
        }
        m_pLbColorTo->SetNoSelection();
        m_pLbColorTo->SelectEntry( pGradient->GetEndColor() );

        if ( m_pLbColorTo->GetSelectEntryCount() == 0 )
        {
            m_pLbColorTo->InsertEntry( pGradient->GetEndColor(), OUString() );
            m_pLbColorTo->SelectEntry( pGradient->GetEndColor() );
        }

        m_pMtrAngle->SetValue( pGradient->GetAngle() / 10 ); // should be changed in resource
        m_pMtrBorder->SetValue( pGradient->GetBorder() );
        m_pMtrCenterX->SetValue( pGradient->GetXOffset() );
        m_pMtrCenterY->SetValue( pGradient->GetYOffset() );
        m_pMtrColorFrom->SetValue( pGradient->GetStartIntens() );
        m_pMtrColorTo->SetValue( pGradient->GetEndIntens() );

        // disable/enable controls
        SetControlState_Impl( eXGS );

        // fill ItemSet and pass it on to aCtlPreview
        rXFSet.Put( XFillGradientItem( OUString(), *pGradient ) );
        m_pCtlPreview->SetAttributes( aXFillAttr.GetItemSet() );

        m_pCtlPreview->Invalidate();
    }
    return 0L;
}



void SvxGradientTabPage::SetControlState_Impl( css::awt::GradientStyle eXGS )
{
    switch( eXGS )
    {
        case css::awt::GradientStyle_LINEAR:
        case css::awt::GradientStyle_AXIAL:
            m_pFtCenterX->Disable();
            m_pMtrCenterX->Disable();
            m_pFtCenterY->Disable();
            m_pMtrCenterY->Disable();
            m_pFtAngle->Enable();
            m_pMtrAngle->Enable();
            break;

        case css::awt::GradientStyle_RADIAL:
            m_pFtCenterX->Enable();
            m_pMtrCenterX->Enable();
            m_pFtCenterY->Enable();
            m_pMtrCenterY->Enable();
            m_pFtAngle->Disable();
            m_pMtrAngle->Disable();
            break;

        case css::awt::GradientStyle_ELLIPTICAL:
            m_pFtCenterX->Enable();
            m_pMtrCenterX->Enable();
            m_pFtCenterY->Enable();
            m_pMtrCenterY->Enable();
            m_pFtAngle->Enable();
            m_pMtrAngle->Enable();
            break;

        case css::awt::GradientStyle_SQUARE:
        case css::awt::GradientStyle_RECT:
            m_pFtCenterX->Enable();
            m_pMtrCenterX->Enable();
            m_pFtCenterY->Enable();
            m_pMtrCenterY->Enable();
            m_pFtAngle->Enable();
            m_pMtrAngle->Enable();
            break;
        default:
            break;
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
