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

#include <core_resource.hxx>
#include <paramdialog.hxx>
#include <strings.hrc>
#include <strings.hxx>
#include <com/sun/star/util/NumberFormatter.hpp>
#include <comphelper/types.hxx>
#include <connectivity/dbtools.hxx>
#include <vcl/weld.hxx>
#include <o3tl/safeint.hxx>
#include <osl/diagnose.h>
#include <comphelper/diagnose_ex.hxx>

namespace dbaui
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::util;

    // OParameterDialog


    OParameterDialog::OParameterDialog(
            weld::Window* pParent, const Reference< XIndexAccess > & rParamContainer,
            const Reference< XConnection > & _rxConnection, const Reference< XComponentContext >& rxContext)
        : GenericDialogController(pParent, "dbaccess/ui/parametersdialog.ui", "Parameters")
        , m_nCurrentlySelected(-1)
        , m_xConnection(_rxConnection)
        , m_aPredicateInput( rxContext, _rxConnection, getParseContext() )
        , m_aResetVisitFlag("dbaccess OParameterDialog m_aResetVisitFlag")
        , m_xAllParams(m_xBuilder->weld_tree_view("allParamTreeview"))
        , m_xParam(m_xBuilder->weld_entry("paramEntry"))
        , m_xTravelNext(m_xBuilder->weld_button("next"))
        , m_xOKBtn(m_xBuilder->weld_button("ok"))
        , m_xCancelBtn(m_xBuilder->weld_button("cancel"))
    {
        m_xAllParams->set_size_request(-1, m_xAllParams->get_height_rows(10));

        if (rxContext.is())
            m_xFormatter.set( NumberFormatter::create( rxContext ), UNO_QUERY_THROW);
        else {
            OSL_FAIL("OParameterDialog::OParameterDialog: need a service factory!");
        }

        Reference< XNumberFormatsSupplier >  xNumberFormats = ::dbtools::getNumberFormats(m_xConnection, true);
        if (!xNumberFormats.is())
            ::comphelper::disposeComponent(m_xFormatter);
        else
            m_xFormatter->attachNumberFormatsSupplier(xNumberFormats);
        try
        {
            OSL_ENSURE(rParamContainer->getCount(), "OParameterDialog::OParameterDialog : can't handle empty containers !");

            m_aFinalValues.realloc(rParamContainer->getCount());
            PropertyValue* pValues = m_aFinalValues.getArray();

            for (sal_Int32 i = 0, nCount = rParamContainer->getCount(); i<nCount; ++i, ++pValues)
            {
                Reference< XPropertySet >  xParamAsSet;
                rParamContainer->getByIndex(i) >>= xParamAsSet;
                OSL_ENSURE(xParamAsSet.is(),"Parameter is null!");
                if(!xParamAsSet.is())
                    continue;
                pValues->Name = ::comphelper::getString(xParamAsSet->getPropertyValue(PROPERTY_NAME));
                m_xAllParams->append_text(pValues->Name);

                m_aVisitedParams.push_back(VisitFlags::NONE);
                    // not visited, not dirty
            }

            m_xParams = rParamContainer;
        }
        catch(Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }

        Construct();

        m_aResetVisitFlag.SetInvokeHandler(LINK(this, OParameterDialog, OnVisitedTimeout));
    }

    OParameterDialog::~OParameterDialog()
    {
        if (m_aResetVisitFlag.IsActive())
            m_aResetVisitFlag.Stop();
    }

    void OParameterDialog::Construct()
    {
        m_xAllParams->connect_changed(LINK(this, OParameterDialog, OnEntryListBoxSelected));
        m_xParam->connect_focus_out(LINK(this, OParameterDialog, OnValueLoseFocusHdl));
        m_xParam->connect_changed(LINK(this, OParameterDialog, OnValueModified));
        m_xTravelNext->connect_clicked(LINK(this, OParameterDialog, OnButtonClicked));
        m_xOKBtn->connect_clicked(LINK(this, OParameterDialog, OnButtonClicked));
        m_xCancelBtn->connect_clicked(LINK(this, OParameterDialog, OnButtonClicked));

        if (m_xAllParams->n_children())
        {
            m_xAllParams->select(0);
            OnEntrySelected();

            if (m_xAllParams->n_children() == 1)
                m_xTravelNext->set_sensitive(false);

            if (m_xAllParams->n_children() > 1)
                m_xDialog->change_default_widget(m_xOKBtn.get(), m_xTravelNext.get());
        }

        m_xParam->grab_focus();
    }

    IMPL_LINK_NOARG(OParameterDialog, OnValueLoseFocusHdl, weld::Widget&, void)
    {
        CheckValueForError();
    }

    bool OParameterDialog::CheckValueForError()
    {
        if (m_nCurrentlySelected != -1)
        {
            if ( !( m_aVisitedParams[ m_nCurrentlySelected ] & VisitFlags::Dirty ) )
                // nothing to do, the value isn't dirty
                return false;
        }

        bool bRet = false;

        Reference< XPropertySet >  xParamAsSet;
        m_xParams->getByIndex(m_nCurrentlySelected) >>= xParamAsSet;
        if (xParamAsSet.is())
        {
            if (m_xConnection.is() && m_xFormatter.is())
            {
                OUString sParamValue(m_xParam->get_text());
                bool bValid = m_aPredicateInput.normalizePredicateString( sParamValue, xParamAsSet );
                m_xParam->set_text(sParamValue);
                m_xParam->set_message_type(bValid ? weld::EntryMessageType::Normal : weld::EntryMessageType::Error);
                OUString sToolTip;
                if ( bValid )
                {
                    // with this the value isn't dirty anymore
                    if (m_nCurrentlySelected != -1)
                        m_aVisitedParams[m_nCurrentlySelected] &= ~VisitFlags::Dirty;
                }
                else
                {
                    OUString sName;
                    try
                    {
                        sName = ::comphelper::getString(xParamAsSet->getPropertyValue(PROPERTY_NAME));
                    }
                    catch(Exception&)
                    {
                        DBG_UNHANDLED_EXCEPTION("dbaccess");
                    }

                    OUString sMessage(DBA_RES(STR_COULD_NOT_CONVERT_PARAM));
                    sToolTip = sMessage.replaceAll( "$name$", sName );
                    m_xParam->grab_focus();
                    bRet = true;
                }
                m_xParam->set_tooltip_text(sToolTip);
                m_xOKBtn->set_sensitive(bValid);
            }
        }

        return bRet;
    }

    IMPL_LINK(OParameterDialog, OnButtonClicked, weld::Button&, rButton, void)
    {
        if (m_xCancelBtn.get() == &rButton)
        {
            // no interpreting of the given values anymore...
            m_xParam->connect_focus_out(Link<weld::Widget&, void>()); // no direct call from the control anymore ...
            m_xDialog->response(RET_CANCEL);
        }
        else if (m_xOKBtn.get() == &rButton)
        {
            // transfer the current values into the Any
            if (OnEntrySelected())
            {   // there was an error interpreting the current text
                return;
            }

            if (m_xParams.is())
            {
                // write the parameters
                try
                {
                    PropertyValue* pValues = m_aFinalValues.getArray();
                    for (sal_Int32 i = 0, nCount = m_xParams->getCount(); i<nCount; ++i, ++pValues)
                    {
                        Reference< XPropertySet >  xParamAsSet;
                        m_xParams->getByIndex(i) >>= xParamAsSet;

                        OUString sValue;
                        pValues->Value >>= sValue;
                        pValues->Value = m_aPredicateInput.getPredicateValue( sValue, xParamAsSet );
                    }
                }
                catch(Exception&)
                {
                    DBG_UNHANDLED_EXCEPTION("dbaccess");
                }

            }
            m_xDialog->response(RET_OK);
        }
        else if (m_xTravelNext.get() == &rButton)
        {
            if (sal_Int32 nCount = m_xAllParams->n_children())
            {
                sal_Int32 nCurrent = m_xAllParams->get_selected_index();
                OSL_ENSURE(static_cast<size_t>(nCount) == m_aVisitedParams.size(), "OParameterDialog::OnButtonClicked : inconsistent lists !");

                // search the next entry in list we haven't visited yet
                sal_Int32 nNext = (nCurrent + 1) % nCount;
                while ((nNext != nCurrent) && ( m_aVisitedParams[nNext] & VisitFlags::Visited ))
                    nNext = (nNext + 1) % nCount;

                if ( m_aVisitedParams[nNext] & VisitFlags::Visited )
                    // there is no such "not visited yet" entry -> simply take the next one
                    nNext = (nCurrent + 1) % nCount;

                m_xAllParams->select(nNext);
                OnEntrySelected();
            }
        }
    }

    IMPL_LINK_NOARG(OParameterDialog, OnEntryListBoxSelected, weld::TreeView&, void)
    {
        OnEntrySelected();
    }

    bool OParameterDialog::OnEntrySelected()
    {
        if (m_aResetVisitFlag.IsActive())
        {
            LINK(this, OParameterDialog, OnVisitedTimeout).Call(&m_aResetVisitFlag);
            m_aResetVisitFlag.Stop();
        }
        // save the old values
        if (m_nCurrentlySelected != -1)
        {
            // do the transformation of the current text
            if (CheckValueForError())
            {   // there was an error interpreting the text
                m_xAllParams->select(m_nCurrentlySelected);
                return true;
            }

            m_aFinalValues.getArray()[m_nCurrentlySelected].Value <<= m_xParam->get_text();
        }

        // initialize the controls with the new values
        sal_Int32 nSelected = m_xAllParams->get_selected_index();
        OSL_ENSURE(nSelected != -1, "OParameterDialog::OnEntrySelected : no current entry !");

        m_xParam->set_text(::comphelper::getString(m_aFinalValues[nSelected].Value));
        m_nCurrentlySelected = nSelected;

        // with this the value isn't dirty
        OSL_ENSURE(o3tl::make_unsigned(m_nCurrentlySelected) < m_aVisitedParams.size(), "OParameterDialog::OnEntrySelected : invalid current entry !");
        m_aVisitedParams[m_nCurrentlySelected] &= ~VisitFlags::Dirty;

        m_aResetVisitFlag.SetTimeout(1000);
        m_aResetVisitFlag.Start();

        return false;
    }

    IMPL_LINK_NOARG(OParameterDialog, OnVisitedTimeout, Timer*, void)
    {
        OSL_ENSURE(m_nCurrentlySelected != -1, "OParameterDialog::OnVisitedTimeout : invalid call !");

        // mark the currently selected entry as visited
        OSL_ENSURE(o3tl::make_unsigned(m_nCurrentlySelected) < m_aVisitedParams.size(), "OParameterDialog::OnVisitedTimeout : invalid entry !");
        m_aVisitedParams[m_nCurrentlySelected] |= VisitFlags::Visited;

        // was it the last "not visited yet" entry ?
        bool bVisited = false;
        for (auto const& visitedParam : m_aVisitedParams)
        {
            if (!(visitedParam & VisitFlags::Visited))
            {
                bVisited = true;
                break;
            }
        }

        if (!bVisited)
        {
            // yes, there isn't another one -> change the "default button"
            m_xDialog->change_default_widget(m_xTravelNext.get(), m_xOKBtn.get());
        }
    }

    IMPL_LINK(OParameterDialog, OnValueModified, weld::Entry&, rEdit, void)
    {
        // mark the currently selected entry as dirty
        OSL_ENSURE(o3tl::make_unsigned(m_nCurrentlySelected) < m_aVisitedParams.size(), "OParameterDialog::OnValueModified : invalid entry !");
        m_aVisitedParams[m_nCurrentlySelected] |= VisitFlags::Dirty;
        rEdit.set_message_type(weld::EntryMessageType::Normal);
    }

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
