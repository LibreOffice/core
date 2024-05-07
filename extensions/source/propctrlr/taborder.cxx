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

#include <bitmaps.hlst>
#include "formstrings.hxx"
#include <comphelper/types.hxx>
#include <comphelper/property.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/form/runtime/FormController.hpp>
#include <osl/diagnose.h>
#include <tools/debug.hxx>
#include <comphelper/diagnose_ex.hxx>

namespace pcr
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::datatransfer;

    namespace {

    OUString GetImage( const Reference< XPropertySet >& _rxSet )
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

        return sImageId;
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

    }

    //= TabOrderDialog
    TabOrderDialog::TabOrderDialog(weld::Window* _pParent, const Reference< XTabControllerModel >& _rxTabModel,
                    const Reference< XControlContainer >& _rxControlCont, const Reference< XComponentContext >& _rxORB)
        : GenericDialogController( _pParent, u"modules/spropctrlr/ui/taborder.ui"_ustr, u"TabOrderDialog"_ustr)
        , m_xModel( _rxTabModel )
        , m_xControlContainer( _rxControlCont )
        , m_xORB( _rxORB )
        , m_xLB_Controls(m_xBuilder->weld_tree_view(u"CTRLtree"_ustr))
        , m_xPB_OK(m_xBuilder->weld_button(u"ok"_ustr))
        , m_xPB_MoveUp(m_xBuilder->weld_button(u"upB"_ustr))
        , m_xPB_MoveDown(m_xBuilder->weld_button(u"downB"_ustr))
        , m_xPB_AutoOrder(m_xBuilder->weld_button(u"autoB"_ustr))
    {
        m_xLB_Controls->set_size_request(m_xLB_Controls->get_approximate_digit_width() * 60,
                                         m_xLB_Controls->get_height_rows(10));
        m_xLB_Controls->set_selection_mode(SelectionMode::Multiple);

        m_xLB_Controls->connect_model_changed(LINK(this, TabOrderDialog, ModelHasMoved));
        m_xPB_MoveUp->connect_clicked( LINK( this, TabOrderDialog, MoveUpClickHdl ) );
        m_xPB_MoveDown->connect_clicked( LINK( this, TabOrderDialog, MoveDownClickHdl ) );
        m_xPB_AutoOrder->connect_clicked( LINK( this, TabOrderDialog, AutoOrderClickHdl ) );
        m_xPB_OK->connect_clicked( LINK( this, TabOrderDialog, OKClickHdl ) );
        m_xPB_OK->set_sensitive(false);

        if ( m_xModel.is() )
            m_xTempModel = new OSimpleTabModel( m_xModel->getControlModels() );

        if ( m_xTempModel.is() && m_xControlContainer.is() )
            FillList();

        if (m_xLB_Controls->n_children() < 2)
        {
            m_xPB_MoveUp->set_sensitive(false);
            m_xPB_MoveDown->set_sensitive(false);
            m_xPB_AutoOrder->set_sensitive(false);
        }

    }

    void TabOrderDialog::SetModified()
    {
        m_xPB_OK->set_sensitive(true);
    }

    TabOrderDialog::~TabOrderDialog()
    {
    }

    void TabOrderDialog::FillList()
    {
        DBG_ASSERT( m_xTempModel.is() && m_xControlContainer.is(), "TabOrderDialog::FillList: invalid call!" );
        if ( !m_xTempModel.is() || !m_xControlContainer.is() )
            return;

        m_xLB_Controls->clear();

        try
        {
            OUString aName;
            OUString aImage;

            const Sequence<Reference<css::awt::XControlModel>> aControlModels = m_xTempModel->getControlModels();
            for ( auto const& rControlModel : aControlModels )
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
                        OUString sId(weld::toId(xControl.get()));
                        m_xLB_Controls->append(sId, aName, aImage);
                    }
                }
                else
                {
                    // no property set -> no tab order
                    OSL_FAIL( "TabOrderDialog::FillList: invalid control encountered!" );
                    m_xLB_Controls->clear();
                    break;
                }
            }
        }
        catch( const Exception& )
        {
            TOOLS_WARN_EXCEPTION( "extensions.propctrlr", "TabOrderDialog::FillList" );
        }

        // select first entry
        if (m_xLB_Controls->n_children())
            m_xLB_Controls->select(0);
    }

    IMPL_LINK_NOARG( TabOrderDialog, MoveUpClickHdl, weld::Button&, void )
    {
        MoveSelection(-1);
    }

    IMPL_LINK_NOARG( TabOrderDialog, MoveDownClickHdl, weld::Button&, void )
    {
        MoveSelection(1);
    }

    IMPL_LINK_NOARG( TabOrderDialog, AutoOrderClickHdl, weld::Button&, void )
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
            TOOLS_WARN_EXCEPTION( "extensions.propctrlr", "TabOrderDialog::AutoOrderClickHdl" );
        }
    }

    IMPL_LINK_NOARG( TabOrderDialog, OKClickHdl, weld::Button&, void )
    {
        int nEntryCount = m_xLB_Controls->n_children();
        Sequence< Reference< XControlModel > > aSortedControlModelSeq( nEntryCount );
        const Sequence< Reference< XControlModel > > aControlModels( m_xTempModel->getControlModels());
        Reference< XControlModel > * pSortedControlModels = aSortedControlModelSeq.getArray();

        for (int i = 0; i < nEntryCount; ++i)
        {
            XPropertySet* pEntry = weld::fromId<XPropertySet*>(m_xLB_Controls->get_id(i));
            for( auto const& rControlModel : aControlModels )
            {
                Reference< XPropertySet >  xSet(rControlModel, UNO_QUERY);
                if (xSet.get() == pEntry)
                {
                    pSortedControlModels[i] = rControlModel;
                    break;
                }
            }
        }

        // TODO: UNO action (to bracket all the single actions which are being created)
        m_xModel->setControlModels( aSortedControlModelSeq );

        m_xDialog->response(RET_OK);
    }

    IMPL_LINK_NOARG(TabOrderDialog, ModelHasMoved, weld::TreeView&, void)
    {
        SetModified();
    }

    void TabOrderDialog::MoveSelection(int nRelPos)
    {
        std::vector<int> aRows(m_xLB_Controls->get_selected_rows());
        if (aRows.empty())
            return;

        m_xLB_Controls->unselect_all();
        for (int i = 0; i < abs(nRelPos); ++i)
        {
            SetModified();

            // move entries
            if (nRelPos < 0)
            {
                std::sort(aRows.begin(), aRows.end());

                int nFirstSelPos = aRows[0];
                if (nFirstSelPos == 0) return;

                for (auto row : aRows)
                {
                    int nInsertPos = row - 1;
                    m_xLB_Controls->swap(nInsertPos, row);
                }

                for (auto row : aRows)
                    m_xLB_Controls->select(row - 1);
            }
            else if (nRelPos > 0)
            {
                std::sort(aRows.rbegin(), aRows.rend());

                int nLastSelPos = aRows[0];
                if( (nLastSelPos + nRelPos - i) > (m_xLB_Controls->n_children()-1) ) return;

                for (auto row : aRows)
                {
                    int nInsertPos = row + 1;
                    m_xLB_Controls->swap(nInsertPos, row);
                }

                for (auto row : aRows)
                    m_xLB_Controls->select(row + 1);
            }
        }
    }


}  // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
