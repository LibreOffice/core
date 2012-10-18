/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "taborder.hxx"
#include "taborder.hrc"

#include "modulepcr.hxx"
#include "formresid.hrc"
#include "formstrings.hxx"
#include <comphelper/types.hxx>
#include <comphelper/property.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/awt/XTabController.hpp>
#include <vcl/scrbar.hxx>

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
        virtual void SAL_CALL setGroup(const Sequence< Reference< XControlModel > >& /*Group*/, const ::rtl::OUString& /*GroupName*/) throw( RuntimeException ) {}
        virtual sal_Int32 SAL_CALL getGroupCount(void) throw( RuntimeException ) {return 0;}
        virtual void SAL_CALL getGroup(sal_Int32 /*nGroup*/, Sequence< Reference< XControlModel > >& /*Group*/, ::rtl::OUString& /*Name*/) throw( RuntimeException ) {}
        virtual void SAL_CALL getGroupByName(const ::rtl::OUString& /*Name*/, Sequence< Reference< XControlModel > >& /*Group*/) throw( RuntimeException ) {}
        virtual sal_Bool SAL_CALL getGroupControl(void) throw( RuntimeException ){return sal_False;} ;
        virtual void SAL_CALL setGroupControl(sal_Bool /*GroupControl*/) throw( RuntimeException ){};
    };

    //========================================================================
    //= TabOrderDialog
    //========================================================================
    DBG_NAME(TabOrderDialog)
    //------------------------------------------------------------------------
    TabOrderDialog::TabOrderDialog( Window* _pParent, const Reference< XTabControllerModel >& _rxTabModel,
                    const Reference< XControlContainer >& _rxControlCont, const Reference< XMultiServiceFactory >& _rxORB )
        :ModalDialog( _pParent, PcrRes( RID_DLG_TABORDER ) )
        ,m_xModel( _rxTabModel )
        ,m_xControlContainer( _rxControlCont )
        ,m_xORB( _rxORB )
        ,aFT_Controls( this, PcrRes( FT_CONTROLS ) )
        ,aLB_Controls( this, PcrRes( CTRL_TREE ) )
        ,aPB_OK( this, PcrRes( PB_OK ) )
        ,aPB_CANCEL( this, PcrRes( PB_CANCEL ) )
        ,aPB_HELP( this, PcrRes( PB_HELP ) )
        ,aPB_MoveUp( this, PcrRes( PB_MOVE_UP ) )
        ,aPB_MoveDown( this, PcrRes( PB_MOVE_DOWN ) )
        ,aPB_AutoOrder( this, PcrRes( PB_AUTO_ORDER ) )
        ,pImageList( NULL )
    {
        DBG_CTOR(TabOrderDialog,NULL);

        aPB_MoveUp.SetClickHdl( LINK( this, TabOrderDialog, MoveUpClickHdl ) );
        aPB_MoveDown.SetClickHdl( LINK( this, TabOrderDialog, MoveDownClickHdl ) );
        aPB_AutoOrder.SetClickHdl( LINK( this, TabOrderDialog, AutoOrderClickHdl ) );
        aPB_OK.SetClickHdl( LINK( this, TabOrderDialog, OKClickHdl ) );
        aPB_OK.Disable();

        pImageList = new ImageList( PcrRes( RID_IL_FORMEXPLORER ) );

        if ( m_xModel.is() )
            m_xTempModel = new OSimpleTabModel( m_xModel->getControlModels() );

        if ( m_xTempModel.is() && m_xControlContainer.is() )
            FillList();

        if ( aLB_Controls.GetEntryCount() < 2 )
        {
            aPB_MoveUp.Disable();
            aPB_MoveDown.Disable();
            aPB_AutoOrder.Disable();
        }

        FreeResource();
    }

    //------------------------------------------------------------------------
    void TabOrderDialog::SetModified()
    {
        aPB_OK.Enable();
    }

    //------------------------------------------------------------------------
    TabOrderDialog::~TabOrderDialog()
    {
        aLB_Controls.Hide();
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

        aLB_Controls.Clear();

        try
        {
            Sequence< Reference< XControlModel > > aControlModels( m_xTempModel->getControlModels() );
            const Reference< XControlModel >* pControlModels = aControlModels.getConstArray();

            ::rtl::OUString aName;
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
                        aLB_Controls.InsertEntry( aName, aImage, aImage, 0, sal_False, LIST_APPEND, xControl.get() );
                    }
                }
                else
                {
                    // no property set -> no tab order
                    OSL_FAIL( "TabOrderDialog::FillList: invalid control encountered!" );
                    aLB_Controls.Clear();
                    break;
                }
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "TabOrderDialog::FillList: caught an exception!" );
        }

        // select first entry
        SvTreeListEntry* pFirstEntry = aLB_Controls.GetEntry( 0 );
        if ( pFirstEntry )
            aLB_Controls.Select( pFirstEntry );
    }

    //------------------------------------------------------------------------
    IMPL_LINK( TabOrderDialog, MoveUpClickHdl, Button*, /*pButton*/ )
    {
        aLB_Controls.MoveSelection( -1 );
        return 0;
    }

    //------------------------------------------------------------------------
    IMPL_LINK( TabOrderDialog, MoveDownClickHdl, Button*, /*pButton*/ )
    {
        aLB_Controls.MoveSelection( 1 );
        return 0;
    }

    //------------------------------------------------------------------------
    IMPL_LINK( TabOrderDialog, AutoOrderClickHdl, Button*, /*pButton*/ )
    {
        try
        {
            Reference< XTabController > xTabController;
            if ( m_xORB.is() )
                xTabController = xTabController.query( m_xORB->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.FormController" ) ) ) );
            DBG_ASSERT( xTabController.is(), "TabOrderDialog::AutoOrderClickHdl: could not instantiate a tab controller!" );
            if ( !xTabController.is() )
                return 0;

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
        sal_uLong nEntryCount = aLB_Controls.GetEntryCount();
        Sequence< Reference< XControlModel > > aSortedControlModelSeq( nEntryCount );
        Sequence< Reference< XControlModel > > aControlModels( m_xTempModel->getControlModels());
        Reference< XControlModel > * pSortedControlModels = aSortedControlModelSeq.getArray();
        const Reference< XControlModel > * pControlModels = aControlModels.getConstArray();

        for (sal_uLong i=0; i < nEntryCount; i++)
        {
            SvTreeListEntry* pEntry = aLB_Controls.GetEntry(i);

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
    TabOrderListBox::TabOrderListBox( Window* pParent, const ResId& rResId  )
        :SvTreeListBox( pParent, rResId  )
    {
        DBG_CTOR(TabOrderListBox,NULL);
        SetDragDropMode(0xFFFF/*SV_DRAGDROP_CTRL_MOVE*/);
            // Hmm. The flag alone is not enough, so to be on the safe side ...

        SetSelectionMode( MULTIPLE_SELECTION );
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

        ((TabOrderDialog*)Window::GetParent())->SetModified();
    }

    //------------------------------------------------------------------------
    void TabOrderListBox::MoveSelection( long nRelPos )
    {
        rtl::OUString aSelEntryPrevText, aSelEntryNextText;
        Image  aImage;
        for (long i=0; i<labs(nRelPos); i++)
        {
            ((TabOrderDialog*)Window::GetParent())->SetModified();

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
