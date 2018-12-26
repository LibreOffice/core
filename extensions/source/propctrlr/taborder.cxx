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

#include "taborder.hxx"

#include "modulepcr.hxx"
#include <strings.hrc>
#include <bitmaps.hlst>
#include "formstrings.hxx"
#include <comphelper/types.hxx>
#include <comphelper/property.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/form/runtime/FormController.hpp>
#include <vcl/scrbar.hxx>
#include <vcl/treelistentry.hxx>
#include <vcl/builderfactory.hxx>
#include <osl/diagnose.h>
#include <tools/debug.hxx>

namespace pcr
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::datatransfer;

    namespace {

    Image GetImage( const Reference< XPropertySet >& _rxSet )
    {
        OUString sImageId = RID_EXTBMP_CONTROL;
        // TODO: classify controls also in Basic propbrw
        if ( _rxSet.is() && ::comphelper::hasProperty( PROPERTY_CLASSID, _rxSet ) )
        {
            switch( ::comphelper::getINT16( _rxSet->getPropertyValue( PROPERTY_CLASSID ) ) )
            {
            case FormComponentType::COMMANDBUTTON:  sImageId = RID_EXTBMP_BUTTON; break;
            case FormComponentType::FIXEDTEXT:      sImageId = RID_EXTBMP_FIXEDTEXT; break;
            case FormComponentType::TEXTFIELD:      sImageId = RID_EXTBMP_EDITBOX; break;
            case FormComponentType::RADIOBUTTON:    sImageId = RID_EXTBMP_RADIOBUTTON; break;
            case FormComponentType::CHECKBOX:       sImageId = RID_EXTBMP_CHECKBOX; break;
            case FormComponentType::LISTBOX:        sImageId = RID_EXTBMP_LISTBOX; break;
            case FormComponentType::COMBOBOX:       sImageId = RID_EXTBMP_COMBOBOX; break;
            case FormComponentType::GROUPBOX:       sImageId = RID_EXTBMP_GROUPBOX; break;
            case FormComponentType::IMAGEBUTTON:    sImageId = RID_EXTBMP_IMAGEBUTTON; break;
            case FormComponentType::FILECONTROL:    sImageId = RID_EXTBMP_FILECONTROL; break;
            case FormComponentType::HIDDENCONTROL:  sImageId = RID_EXTBMP_HIDDEN; break;
            case FormComponentType::DATEFIELD:      sImageId = RID_EXTBMP_DATEFIELD; break;
            case FormComponentType::TIMEFIELD:      sImageId = RID_EXTBMP_TIMEFIELD; break;
            case FormComponentType::NUMERICFIELD:   sImageId = RID_EXTBMP_NUMERICFIELD; break;
            case FormComponentType::CURRENCYFIELD:  sImageId = RID_EXTBMP_CURRENCYFIELD; break;
            case FormComponentType::PATTERNFIELD:   sImageId = RID_EXTBMP_PATTERNFIELD; break;
            case FormComponentType::IMAGECONTROL:   sImageId = RID_EXTBMP_IMAGECONTROL; break;
            case FormComponentType::GRIDCONTROL:    sImageId = RID_EXTBMP_GRID; break;
            case FormComponentType::SCROLLBAR:      sImageId = RID_EXTBMP_SCROLLBAR; break;
            case FormComponentType::SPINBUTTON:     sImageId = RID_EXTBMP_SPINBUTTON; break;
            case FormComponentType::NAVIGATIONBAR:  sImageId = RID_EXTBMP_NAVIGATIONBAR; break;
            default:
                OSL_FAIL( "TabOrderDialog::GetImage: unknown control type" );
            }
        }

        return Image(StockImage::Yes, sImageId);
    }

    }

    //= OSimpleTabModel

    class OSimpleTabModel : public ::cppu::WeakImplHelper< XTabControllerModel>
    {
        Sequence< Reference< XControlModel > > m_aModels;

    public:
        explicit OSimpleTabModel( const Sequence< Reference< XControlModel > >& _rModels )
            :m_aModels( _rModels )
        {
        }

        // XTabControllerModel
        virtual void SAL_CALL setControlModels(const Sequence< Reference< XControlModel > >& rModels) override {m_aModels = rModels;}
        virtual Sequence< Reference< XControlModel > > SAL_CALL getControlModels() override {return m_aModels;}
        virtual void SAL_CALL setGroup(const Sequence< Reference< XControlModel > >& /*Group*/, const OUString& /*GroupName*/) override {}
        virtual sal_Int32 SAL_CALL getGroupCount() override {return 0;}
        virtual void SAL_CALL getGroup(sal_Int32 /*nGroup*/, Sequence< Reference< XControlModel > >& /*Group*/, OUString& /*Name*/) override {}
        virtual void SAL_CALL getGroupByName(const OUString& /*Name*/, Sequence< Reference< XControlModel > >& /*Group*/) override {}
        virtual sal_Bool SAL_CALL getGroupControl() override {return false;} ;
        virtual void SAL_CALL setGroupControl(sal_Bool /*GroupControl*/) override {};
    };


    //= TabOrderDialog


    TabOrderDialog::TabOrderDialog( vcl::Window* _pParent, const Reference< XTabControllerModel >& _rxTabModel,
                    const Reference< XControlContainer >& _rxControlCont, const Reference< XComponentContext >& _rxORB )
        :ModalDialog( _pParent, "TabOrderDialog", "modules/spropctrlr/ui/taborder.ui")
        ,m_xModel( _rxTabModel )
        ,m_xControlContainer( _rxControlCont )
        ,m_xORB( _rxORB )
    {
        get(m_pLB_Controls, "CTRLtree");
        get(m_pPB_OK, "ok");
        get(m_pPB_MoveUp, "upB");
        get(m_pPB_MoveDown, "downB");
        get(m_pPB_AutoOrder, "autoB");


        m_pPB_MoveUp->SetClickHdl( LINK( this, TabOrderDialog, MoveUpClickHdl ) );
        m_pPB_MoveDown->SetClickHdl( LINK( this, TabOrderDialog, MoveDownClickHdl ) );
        m_pPB_AutoOrder->SetClickHdl( LINK( this, TabOrderDialog, AutoOrderClickHdl ) );
        m_pPB_OK->SetClickHdl( LINK( this, TabOrderDialog, OKClickHdl ) );
        m_pPB_OK->Disable();

        if ( m_xModel.is() )
            m_xTempModel = new OSimpleTabModel( m_xModel->getControlModels() );

        if ( m_xTempModel.is() && m_xControlContainer.is() )
            FillList();

        if ( m_pLB_Controls->GetEntryCount() < 2 )
        {
            m_pPB_MoveUp->Disable();
            m_pPB_MoveDown->Disable();
            m_pPB_AutoOrder->Disable();
        }

    }


    void TabOrderDialog::SetModified()
    {
        m_pPB_OK->Enable();
    }


    TabOrderDialog::~TabOrderDialog()
    {
        disposeOnce();
    }

    void TabOrderDialog::dispose()
    {
        m_pLB_Controls->Hide();
        m_pLB_Controls.clear();
        m_pPB_OK.clear();
        m_pPB_MoveUp.clear();
        m_pPB_MoveDown.clear();
        m_pPB_AutoOrder.clear();
        ModalDialog::dispose();
    }

    void TabOrderDialog::FillList()
    {
        DBG_ASSERT( m_xTempModel.is() && m_xControlContainer.is(), "TabOrderDialog::FillList: invalid call!" );
        if ( !m_xTempModel.is() || !m_xControlContainer.is() )
            return;

        m_pLB_Controls->Clear();

        try
        {
            OUString aName;
            Image aImage;

            for ( auto const& rControlModel : m_xTempModel->getControlModels() )
            {
                Reference< XPropertySet > xControl( rControlModel, UNO_QUERY );
                Reference< XPropertySetInfo > xPI;
                if ( xControl.is() )
                    xPI = xControl->getPropertySetInfo();

                if ( xPI.is() )
                {
                    if ( xPI->hasPropertyByName( PROPERTY_TABSTOP ) )
                    {
                        aName = ::comphelper::getString( xControl->getPropertyValue( PROPERTY_NAME ) );
                            // TODO: do Basic controls have a name?
                        aImage = GetImage( xControl );
                        m_pLB_Controls->InsertEntry( aName, aImage, aImage, nullptr, false, TREELIST_APPEND, xControl.get() );
                    }
                }
                else
                {
                    // no property set -> no tab order
                    OSL_FAIL( "TabOrderDialog::FillList: invalid control encountered!" );
                    m_pLB_Controls->Clear();
                    break;
                }
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "TabOrderDialog::FillList: caught an exception!" );
        }

        // select first entry
        SvTreeListEntry* pFirstEntry = m_pLB_Controls->GetEntry( 0 );
        if ( pFirstEntry )
            m_pLB_Controls->Select( pFirstEntry );
    }


    IMPL_LINK_NOARG( TabOrderDialog, MoveUpClickHdl, Button*, void )
    {
        m_pLB_Controls->MoveSelection( -1 );
    }


    IMPL_LINK_NOARG( TabOrderDialog, MoveDownClickHdl, Button*, void )
    {
        m_pLB_Controls->MoveSelection( 1 );
    }


    IMPL_LINK_NOARG( TabOrderDialog, AutoOrderClickHdl, Button*, void )
    {
        try
        {
            Reference< css::form::runtime::XFormController > xTabController = css::form::runtime::FormController::create( m_xORB );

            xTabController->setModel( m_xTempModel );
            xTabController->setContainer( m_xControlContainer );
            xTabController->autoTabOrder();

            SetModified();
            FillList();

            ::comphelper::disposeComponent( xTabController );
        }
        catch( const Exception& )
        {
            OSL_FAIL( "TabOrderDialog::AutoOrderClickHdl: caught an exception!" );
        }
    }


    IMPL_LINK_NOARG( TabOrderDialog, OKClickHdl, Button*, void )
    {
        sal_uLong nEntryCount = m_pLB_Controls->GetEntryCount();
        Sequence< Reference< XControlModel > > aSortedControlModelSeq( nEntryCount );
        Sequence< Reference< XControlModel > > aControlModels( m_xTempModel->getControlModels());
        Reference< XControlModel > * pSortedControlModels = aSortedControlModelSeq.getArray();

        for (sal_uLong i=0; i < nEntryCount; i++)
        {
            SvTreeListEntry* pEntry = m_pLB_Controls->GetEntry(i);

            for( auto const& rControlModel : aControlModels )
            {
                Reference< XPropertySet >  xSet(rControlModel, UNO_QUERY);
                if (xSet.get() == static_cast<XPropertySet*>(pEntry->GetUserData()))
                {
                    pSortedControlModels[i] = rControlModel;
                    break;
                }
            }
        }

        // TODO: UNO action (to bracket all the single actions which are being created)
        m_xModel->setControlModels( aSortedControlModelSeq );

        EndDialog( RET_OK );
    }


    //= TabOrderListBox


    TabOrderListBox::TabOrderListBox( vcl::Window* pParent, WinBits nBits  )
        :SvTreeListBox( pParent, nBits  )
    {
        SetDragDropMode(DragDropMode::ALL/*DragDropMode::CTRL_MOVE*/);
            // Hmm. The flag alone is not enough, so to be on the safe side ...

        SetSelectionMode( SelectionMode::Multiple );
    }

    VCL_BUILDER_FACTORY_CONSTRUCTOR(TabOrderListBox, WB_TABSTOP)

    TabOrderListBox::~TabOrderListBox()
    {
    }


    void TabOrderListBox::ModelHasMoved( SvTreeListEntry* _pSource )
    {
        SvTreeListBox::ModelHasMoved( _pSource );

        static_cast<TabOrderDialog*>(GetParentDialog())->SetModified();
    }


    void TabOrderListBox::MoveSelection( long nRelPos )
    {
        OUString aSelEntryPrevText, aSelEntryNextText;
        Image  aImage;
        for (long i=0; i<labs(nRelPos); i++)
        {
            static_cast<TabOrderDialog*>(GetParentDialog())->SetModified();


            // move entries
            if( nRelPos < 0 )
            {
                SvTreeListEntry* pFirstSelected = FirstSelected();
                if( !pFirstSelected ) return;
                sal_uLong nFirstSelPos = GetModel()->GetAbsPos( pFirstSelected );
                if( nFirstSelPos == 0 ) return;

                SvTreeListEntry* pSelEntry = pFirstSelected;
                while( pSelEntry )
                {
                    sal_uLong nSelEntryPos = GetModel()->GetAbsPos( pSelEntry );
                    SvTreeListEntry* pSelEntryPrev = GetEntry( nSelEntryPos-1 );
                    aSelEntryPrevText = GetEntryText( pSelEntryPrev );
                    aImage = GetExpandedEntryBmp(pSelEntryPrev);
                    void*  pData = pSelEntryPrev->GetUserData();

                    GetModel()->Remove( pSelEntryPrev );
                    InsertEntry( aSelEntryPrevText, aImage, aImage, nullptr, false, nSelEntryPos, pData );

                    pSelEntry = NextSelected( pSelEntry );
                }
            }

            else if( nRelPos > 0 )
            {
                SvTreeListEntry* pLastSelected = LastSelected();
                if( !pLastSelected ) return;
                sal_uLong nLastSelPos = GetModel()->GetAbsPos( pLastSelected );

                if( (nLastSelPos + nRelPos - i) > (GetEntryCount()-1) ) return;

                SvTreeListEntry* pSelEntry = pLastSelected;
                while( pSelEntry )
                {
                    sal_uLong nSelEntryPos = GetModel()->GetAbsPos( pSelEntry );
                    SvTreeListEntry* pSelEntryNext = GetEntry( nSelEntryPos+1 );
                    void* pData = pSelEntryNext->GetUserData();

                    aSelEntryNextText = GetEntryText( pSelEntryNext );
                    aImage = GetExpandedEntryBmp(pSelEntryNext);

                    GetModel()->Remove( pSelEntryNext );
                    InsertEntry( aSelEntryNextText, aImage, aImage, nullptr, false, nSelEntryPos, pData );

                    pSelEntry = PrevSelected( pSelEntry );
                }
                long nThumbPos      = GetVScroll()->GetThumbPos();
                long nVisibleSize   = GetVScroll()->GetVisibleSize();
                long nFirstVisible = GetModel()->GetAbsPos( FirstVisible());

                if ( ( nThumbPos + nVisibleSize + 1 ) < static_cast<long>( nLastSelPos + 3 ) )
                    GetVScroll()->DoScrollAction(ScrollType::LineDown);
                else if((nThumbPos+nVisibleSize+1) >= nFirstVisible)
                    GetVScroll()->DoScrollAction(ScrollType::LineUp);
            }
        }
    }


}  // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
