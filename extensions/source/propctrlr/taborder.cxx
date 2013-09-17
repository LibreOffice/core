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
#include "formresid.hrc"
#include "formstrings.hxx"
#include <comphelper/types.hxx>
#include <comphelper/property.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/form/runtime/FormController.hpp>
#include <vcl/scrbar.hxx>
#include <svtools/treelistentry.hxx>
#include <vcl/builder.hxx>

//............................................................................
namespace pcr
{
//............................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::datatransfer;

    //========================================================================
    //= OSimpleTabModel
    //========================================================================
    class OSimpleTabModel : public ::cppu::WeakImplHelper1< XTabControllerModel>
    {
        Sequence< Reference< XControlModel > > m_aModels;

    public:
        OSimpleTabModel( const Sequence< Reference< XControlModel > >& _rModels )
            :m_aModels( _rModels )
        {
        }

        // XTabControllerModel
        virtual void SAL_CALL setControlModels(const Sequence< Reference< XControlModel > >& rModels) throw( RuntimeException ) {m_aModels = rModels;}
        virtual Sequence< Reference< XControlModel > > SAL_CALL getControlModels(void) throw( RuntimeException ) {return m_aModels;}
        virtual void SAL_CALL setGroup(const Sequence< Reference< XControlModel > >& /*Group*/, const OUString& /*GroupName*/) throw( RuntimeException ) {}
        virtual sal_Int32 SAL_CALL getGroupCount(void) throw( RuntimeException ) {return 0;}
        virtual void SAL_CALL getGroup(sal_Int32 /*nGroup*/, Sequence< Reference< XControlModel > >& /*Group*/, OUString& /*Name*/) throw( RuntimeException ) {}
        virtual void SAL_CALL getGroupByName(const OUString& /*Name*/, Sequence< Reference< XControlModel > >& /*Group*/) throw( RuntimeException ) {}
        virtual sal_Bool SAL_CALL getGroupControl(void) throw( RuntimeException ){return sal_False;} ;
        virtual void SAL_CALL setGroupControl(sal_Bool /*GroupControl*/) throw( RuntimeException ){};
    };

    //========================================================================
    //= TabOrderDialog
    //========================================================================
    DBG_NAME(TabOrderDialog)
    //------------------------------------------------------------------------
    TabOrderDialog::TabOrderDialog( Window* _pParent, const Reference< XTabControllerModel >& _rxTabModel,
                    const Reference< XControlContainer >& _rxControlCont, const Reference< XComponentContext >& _rxORB )
        :ModalDialog( _pParent, "TabOrderDialog", "modules/spropctrlr/ui/taborder.ui")
        ,m_xModel( _rxTabModel )
        ,m_xControlContainer( _rxControlCont )
        ,m_xORB( _rxORB )
        ,pImageList( NULL )
    {
        get(m_pLB_Controls, "CTRLtree");
        get(m_pPB_OK, "ok");
        get(m_pPB_MoveUp, "upB");
        get(m_pPB_MoveDown, "downB");
        get(m_pPB_AutoOrder, "autoB");

        DBG_CTOR(TabOrderDialog,NULL);

        m_pPB_MoveUp->SetClickHdl( LINK( this, TabOrderDialog, MoveUpClickHdl ) );
        m_pPB_MoveDown->SetClickHdl( LINK( this, TabOrderDialog, MoveDownClickHdl ) );
        m_pPB_AutoOrder->SetClickHdl( LINK( this, TabOrderDialog, AutoOrderClickHdl ) );
        m_pPB_OK->SetClickHdl( LINK( this, TabOrderDialog, OKClickHdl ) );
        m_pPB_OK->Disable();

        pImageList = new ImageList( PcrRes( RID_IL_FORMEXPLORER ) );

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

    //------------------------------------------------------------------------
    void TabOrderDialog::SetModified()
    {
        m_pPB_OK->Enable();
    }

    //------------------------------------------------------------------------
    TabOrderDialog::~TabOrderDialog()
    {
        m_pLB_Controls->Hide();
        //  delete pLB_Controls;
        delete pImageList;

        DBG_DTOR(TabOrderDialog,NULL);
    }

    //------------------------------------------------------------------------
    Image TabOrderDialog::GetImage( const Reference< XPropertySet >& _rxSet ) const
    {
        sal_uInt16 nImageId = RID_SVXIMG_CONTROL;
        // TODO: classify controls also in Basic propbrw
        if ( _rxSet.is() && ::comphelper::hasProperty( PROPERTY_CLASSID, _rxSet ) )
        {
            switch( ::comphelper::getINT16( _rxSet->getPropertyValue( PROPERTY_CLASSID ) ) )
            {
            case FormComponentType::COMMANDBUTTON:  nImageId = RID_SVXIMG_BUTTON; break;
            case FormComponentType::FIXEDTEXT:      nImageId = RID_SVXIMG_FIXEDTEXT; break;
            case FormComponentType::TEXTFIELD:      nImageId = RID_SVXIMG_EDIT; break;
            case FormComponentType::RADIOBUTTON:    nImageId = RID_SVXIMG_RADIOBUTTON; break;
            case FormComponentType::CHECKBOX:       nImageId = RID_SVXIMG_CHECKBOX; break;
            case FormComponentType::LISTBOX:        nImageId = RID_SVXIMG_LISTBOX; break;
            case FormComponentType::COMBOBOX:       nImageId = RID_SVXIMG_COMBOBOX; break;
            case FormComponentType::GROUPBOX:       nImageId = RID_SVXIMG_GROUPBOX; break;
            case FormComponentType::IMAGEBUTTON:    nImageId = RID_SVXIMG_IMAGEBUTTON; break;
            case FormComponentType::FILECONTROL:    nImageId = RID_SVXIMG_FILECONTROL; break;
            case FormComponentType::HIDDENCONTROL:  nImageId = RID_SVXIMG_HIDDEN; break;
            case FormComponentType::DATEFIELD:      nImageId = RID_SVXIMG_DATEFIELD; break;
            case FormComponentType::TIMEFIELD:      nImageId = RID_SVXIMG_TIMEFIELD; break;
            case FormComponentType::NUMERICFIELD:   nImageId = RID_SVXIMG_NUMERICFIELD; break;
            case FormComponentType::CURRENCYFIELD:  nImageId = RID_SVXIMG_CURRENCYFIELD; break;
            case FormComponentType::PATTERNFIELD:   nImageId = RID_SVXIMG_PATTERNFIELD; break;
            case FormComponentType::IMAGECONTROL:   nImageId = RID_SVXIMG_IMAGECONTROL; break;
            case FormComponentType::GRIDCONTROL:    nImageId = RID_SVXIMG_GRID; break;
            case FormComponentType::SCROLLBAR:      nImageId = RID_SVXIMG_SCROLLBAR; break;
            case FormComponentType::SPINBUTTON:     nImageId = RID_SVXIMG_SPINBUTTON; break;
            case FormComponentType::NAVIGATIONBAR:  nImageId = RID_SVXIMG_NAVIGATIONBAR; break;
            default:
                OSL_FAIL( "TabOrderDialog::GetImage: unknown control type" );
            }
        }

        return pImageList->GetImage( nImageId );
    }

    //------------------------------------------------------------------------
    void TabOrderDialog::FillList()
    {
        DBG_ASSERT( m_xTempModel.is() && m_xControlContainer.is(), "TabOrderDialog::FillList: invalid call!" );
        if ( !m_xTempModel.is() || !m_xControlContainer.is() )
            return;

        m_pLB_Controls->Clear();

        try
        {
            Sequence< Reference< XControlModel > > aControlModels( m_xTempModel->getControlModels() );
            const Reference< XControlModel >* pControlModels = aControlModels.getConstArray();

            OUString aName;
            Image aImage;

            for ( sal_Int32 i=0; i < aControlModels.getLength(); ++i, ++pControlModels )
            {
                Reference< XPropertySet > xControl( *pControlModels, UNO_QUERY );
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
                        m_pLB_Controls->InsertEntry( aName, aImage, aImage, 0, sal_False, LIST_APPEND, xControl.get() );
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

    //------------------------------------------------------------------------
    IMPL_LINK( TabOrderDialog, MoveUpClickHdl, Button*, /*pButton*/ )
    {
        m_pLB_Controls->MoveSelection( -1 );
        return 0;
    }

    //------------------------------------------------------------------------
    IMPL_LINK( TabOrderDialog, MoveDownClickHdl, Button*, /*pButton*/ )
    {
        m_pLB_Controls->MoveSelection( 1 );
        return 0;
    }

    //------------------------------------------------------------------------
    IMPL_LINK( TabOrderDialog, AutoOrderClickHdl, Button*, /*pButton*/ )
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

        return 0;
    }

    //------------------------------------------------------------------------
    IMPL_LINK( TabOrderDialog, OKClickHdl, Button*, /*pButton*/ )
    {
        sal_uLong nEntryCount = m_pLB_Controls->GetEntryCount();
        Sequence< Reference< XControlModel > > aSortedControlModelSeq( nEntryCount );
        Sequence< Reference< XControlModel > > aControlModels( m_xTempModel->getControlModels());
        Reference< XControlModel > * pSortedControlModels = aSortedControlModelSeq.getArray();
        const Reference< XControlModel > * pControlModels = aControlModels.getConstArray();

        for (sal_uLong i=0; i < nEntryCount; i++)
        {
            SvTreeListEntry* pEntry = m_pLB_Controls->GetEntry(i);

            for( sal_Int32 j=0; j<aControlModels.getLength(); j++ )
            {
                Reference< XPropertySet >  xSet(pControlModels[j], UNO_QUERY);
                if ((XPropertySet*)xSet.get() == ((XPropertySet*)pEntry->GetUserData()))
                {
                    pSortedControlModels[i] = pControlModels[j];
                    break;
                }
            }
        }

        // TODO: UNO action (to bracket all the single actions which are being created)
        m_xModel->setControlModels( aSortedControlModelSeq );

        EndDialog( sal_True );
        return 0;
    }

    //========================================================================
    //= TabOrderListBox
    //========================================================================
    DBG_NAME(TabOrderListBox);
    //------------------------------------------------------------------------
    TabOrderListBox::TabOrderListBox( Window* pParent, WinBits nBits  )
        :SvTreeListBox( pParent, nBits  )
    {
        DBG_CTOR(TabOrderListBox,NULL);
        SetDragDropMode(0xFFFF/*SV_DRAGDROP_CTRL_MOVE*/);
            // Hmm. The flag alone is not enough, so to be on the safe side ...

        SetSelectionMode( MULTIPLE_SELECTION );
    }

    extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeTabOrderListBox(Window *pParent,
         VclBuilder::stringmap &rMap)
    {
         WinBits nWinStyle = WB_TABSTOP;
         OString sBorder = VclBuilder::extractCustomProperty(rMap);
         if (!sBorder.isEmpty())
             nWinStyle |= WB_BORDER;
         return new TabOrderListBox(pParent, nWinStyle);
    }

    //------------------------------------------------------------------------
    TabOrderListBox::~TabOrderListBox()
    {
        DBG_DTOR(TabOrderListBox,NULL);
    }

    //------------------------------------------------------------------------
    void TabOrderListBox::ModelHasMoved( SvTreeListEntry* _pSource )
    {
        SvTreeListBox::ModelHasMoved( _pSource );

        ((TabOrderDialog*)GetParentDialog())->SetModified();
    }

    //------------------------------------------------------------------------
    void TabOrderListBox::MoveSelection( long nRelPos )
    {
        OUString aSelEntryPrevText, aSelEntryNextText;
        Image  aImage;
        for (long i=0; i<labs(nRelPos); i++)
        {
            ((TabOrderDialog*)GetParentDialog())->SetModified();

            //////////////////////////////////////////////////////////////////////
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
                    InsertEntry( aSelEntryPrevText, aImage, aImage, 0, sal_False, nSelEntryPos, pData );

                    pSelEntry = NextSelected( pSelEntry );
                }
            }

            else if( nRelPos > 0 )
            {
                SvTreeListEntry* pLastSelected = LastSelected();
                if( !pLastSelected ) return;
                sal_uLong nLastSelPos = GetModel()->GetAbsPos( pLastSelected );

                if( (nLastSelPos + nRelPos - i) > (GetEntryCount()-1) ) return;

#if OSL_DEBUG_LEVEL > 0
                sal_uLong nSelCount = GetSelectionCount();
                (void)nSelCount;
#endif


                SvTreeListEntry* pSelEntry = pLastSelected;
                while( pSelEntry )
                {
                    sal_uLong nSelEntryPos = GetModel()->GetAbsPos( pSelEntry );
                    SvTreeListEntry* pSelEntryNext = GetEntry( nSelEntryPos+1 );
                    void* pData = pSelEntryNext->GetUserData();

                    aSelEntryNextText = GetEntryText( pSelEntryNext );
                    aImage = GetExpandedEntryBmp(pSelEntryNext);

                    GetModel()->Remove( pSelEntryNext );
                    InsertEntry( aSelEntryNextText, aImage, aImage, 0, sal_False, nSelEntryPos, pData );

                    pSelEntry = PrevSelected( pSelEntry );
                }
                long nThumbPos      = GetVScroll()->GetThumbPos();
                long nVisibleSize   = GetVScroll()->GetVisibleSize();
                long nFirstVisible = GetModel()->GetAbsPos( FirstVisible());

                if ( ( nThumbPos + nVisibleSize + 1 ) < (long)( nLastSelPos + 3 ) )
                    GetVScroll()->DoScrollAction(SCROLL_LINEDOWN);
                else if((nThumbPos+nVisibleSize+1) >= (nFirstVisible))
                    GetVScroll()->DoScrollAction(SCROLL_LINEUP);
            }
        }
    }

//............................................................................
}  // namespace pcr
//............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
