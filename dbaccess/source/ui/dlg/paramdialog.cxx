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

#include "paramdialog.hxx"
#include "dbu_dlg.hrc"
#include "commontypes.hxx"
#include "moduledbu.hxx"
#include <com/sun/star/util/NumberFormatter.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <comphelper/processfactory.hxx>
#include <connectivity/dbtools.hxx>
#include "dbustrings.hrc"
#include <vcl/svapp.hxx>
#include <vcl/layout.hxx>
#include <osl/diagnose.h>
#include <tools/diagnose_ex.h>
#include "localresaccess.hxx"
#include <unotools/syslocale.hxx>

#define EF_VISITED      0x0001
#define EF_DIRTY        0x0002

namespace dbaui
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::util;
    using namespace ::connectivity;

    // OParameterDialog


    OParameterDialog::OParameterDialog(
            vcl::Window* pParent, const Reference< XIndexAccess > & rParamContainer,
            const Reference< XConnection > & _rxConnection, const Reference< XComponentContext >& rxContext)
        :ModalDialog( pParent, "Parameters", "dbaccess/ui/parametersdialog.ui")
        ,m_nCurrentlySelected(LISTBOX_ENTRY_NOTFOUND)
        ,m_xConnection(_rxConnection)
        ,m_aPredicateInput( rxContext, _rxConnection, getParseContext() )
        ,m_bNeedErrorOnCurrent(true)
    {
        get(m_pAllParams, "allParamTreeview");
        get(m_pParam, "paramEntry");
        get(m_pTravelNext, "next");
        get(m_pOKBtn, "ok");
        get(m_pCancelBtn, "cancel");

        set_height_request(200);

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
                m_pAllParams->InsertEntry(pValues->Name);

                m_aVisitedParams.push_back(0);
                    // not visited, not dirty
            }

            m_xParams = rParamContainer;
        }
        catch(Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        Construct();

        m_aResetVisitFlag.SetTimeoutHdl(LINK(this, OParameterDialog, OnVisitedTimeout));
    }

    OParameterDialog::~OParameterDialog()
    {
        disposeOnce();
    }

    void OParameterDialog::dispose()
    {
        if (m_aResetVisitFlag.IsActive())
            m_aResetVisitFlag.Stop();
        m_pAllParams.clear();
        m_pParam.clear();
        m_pTravelNext.clear();
        m_pOKBtn.clear();
        m_pCancelBtn.clear();
        ModalDialog::dispose();
    }

    void OParameterDialog::Construct()
    {
        m_pAllParams->SetSelectHdl(LINK(this, OParameterDialog, OnEntryListBoxSelected));
        m_pParam->SetLoseFocusHdl(LINK(this, OParameterDialog, OnValueLoseFocusHdl));
        m_pParam->SetModifyHdl(LINK(this, OParameterDialog, OnValueModified));
        m_pTravelNext->SetClickHdl(LINK(this, OParameterDialog, OnButtonClicked));
        m_pOKBtn->SetClickHdl(LINK(this, OParameterDialog, OnButtonClicked));
        m_pCancelBtn->SetClickHdl(LINK(this, OParameterDialog, OnButtonClicked));

        if (m_pAllParams->GetEntryCount())
        {
            m_pAllParams->SelectEntryPos(0);
            OnEntrySelected();

            if (m_pAllParams->GetEntryCount() == 1)
            {
                m_pTravelNext->Enable(false);
            }

            if (m_pAllParams->GetEntryCount() > 1)
            {
                m_pOKBtn->SetStyle(m_pOKBtn->GetStyle() & ~WB_DEFBUTTON);
                m_pTravelNext->SetStyle(m_pTravelNext->GetStyle() | WB_DEFBUTTON);
            }
        }

        m_pParam->GrabFocus();
    }

    IMPL_LINK_NOARG_TYPED(OParameterDialog, OnValueLoseFocusHdl, Control&, void)
    {
        OnValueLoseFocus();
    }

    bool OParameterDialog::OnValueLoseFocus()
    {
        if (m_nCurrentlySelected != LISTBOX_ENTRY_NOTFOUND)
        {
            if ( ( m_aVisitedParams[ m_nCurrentlySelected ] & EF_DIRTY ) == 0 )
                // nothing to do, the value isn't dirty
                return false;
        }

        Reference< XPropertySet >  xParamAsSet;
        m_xParams->getByIndex(m_nCurrentlySelected) >>= xParamAsSet;
        if (xParamAsSet.is())
        {
            if (m_xConnection.is() && m_xFormatter.is())
            {
                OUString sParamValue( m_pParam->GetText() );
                bool bValid = m_aPredicateInput.normalizePredicateString( sParamValue, xParamAsSet );
                m_pParam->SetText( sParamValue );
                if ( bValid )
                {
                    // with this the value isn't dirty anymore
                    if (m_nCurrentlySelected != LISTBOX_ENTRY_NOTFOUND)
                        m_aVisitedParams[m_nCurrentlySelected] &= ~EF_DIRTY;
                }
                else
                {
                    if (!m_bNeedErrorOnCurrent)
                        return true;

                    OUString sName;
                    try
                    {
                        sName = ::comphelper::getString(xParamAsSet->getPropertyValue(PROPERTY_NAME));
                    }
                    catch(Exception&)
                    {
                        DBG_UNHANDLED_EXCEPTION();
                    }

                    OUString sMessage(ModuleRes(STR_COULD_NOT_CONVERT_PARAM));
                    sMessage = sMessage.replaceAll( "$name$", sName );
                    ScopedVclPtrInstance<MessageDialog>(nullptr, sMessage)->Execute();
                    m_pParam->GrabFocus();
                    return true;
                }
            }
        }

        return false;
    }

    IMPL_LINK_TYPED(OParameterDialog, OnButtonClicked, Button*, pButton, void)
    {
        if (m_pCancelBtn == pButton)
        {
            // no interpreting of the given values anymore ....
            m_pParam->SetLoseFocusHdl(Link<Control&,void>()); // no direct call from the control anymore ...
            m_bNeedErrorOnCurrent = false;      // in case of any indirect calls -> no error message
            m_pCancelBtn->SetClickHdl(Link<Button*,void>());
            m_pCancelBtn->Click();
        }
        else if (m_pOKBtn == pButton)
        {
            // transfer the current values into the Any
            if (OnEntrySelected())
            {   // there was an error interpreting the current text
                m_bNeedErrorOnCurrent = true;
                    // we're are out of the complex web :) of direct and indirect calls to OnValueLoseFocus now,
                    // so the next time it is called we need an error message, again ....
                    // (TODO : there surely are better solutions for this ...)
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
                        pValues->Value <<= m_aPredicateInput.getPredicateValue( sValue, xParamAsSet );
                    }
                }
                catch(Exception&)
                {
                    DBG_UNHANDLED_EXCEPTION();
                }

            }
            // to close the dialog (which is more code than a simple EndDialog)
            m_pOKBtn->SetClickHdl(Link<Button*,void>());
            m_pOKBtn->Click();
        }
        else if (m_pTravelNext == pButton)
        {
            if (sal_Int32 nCount = m_pAllParams->GetEntryCount())
            {
                sal_Int32 nCurrent = m_pAllParams->GetSelectEntryPos();
                OSL_ENSURE(static_cast<size_t>(nCount) == m_aVisitedParams.size(), "OParameterDialog::OnButtonClicked : inconsistent lists !");

                // search the next entry in list we haven't visited yet
                sal_Int32 nNext = (nCurrent + 1) % nCount;
                while ((nNext != nCurrent) && ( m_aVisitedParams[nNext] & EF_VISITED ))
                    nNext = (nNext + 1) % nCount;

                if ( m_aVisitedParams[nNext] & EF_VISITED )
                    // there is no such "not visited yet" entry -> simply take the next one
                    nNext = (nCurrent + 1) % nCount;

                m_pAllParams->SelectEntryPos(nNext);
                OnEntrySelected();
                m_bNeedErrorOnCurrent = true;
                    // we're are out of the complex web :) of direct and indirect calls to OnValueLoseFocus now,
                    // so the next time it is called we need an error message, again ....
                    // (TODO : there surely are better solutions for this ...)
            }
        }
    }

    IMPL_LINK_NOARG_TYPED(OParameterDialog, OnEntryListBoxSelected, ListBox&, void)
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
        if (m_nCurrentlySelected != LISTBOX_ENTRY_NOTFOUND)
        {
            // do the transformation of the current text
            if (OnValueLoseFocus())
            {   // there was an error interpreting the text
                m_pAllParams->SelectEntryPos(m_nCurrentlySelected);
                return true;
            }

            m_aFinalValues[m_nCurrentlySelected].Value <<= OUString(m_pParam->GetText());
        }

        // initialize the controls with the new values
        sal_Int32 nSelected = m_pAllParams->GetSelectEntryPos();
        OSL_ENSURE(nSelected != LISTBOX_ENTRY_NOTFOUND, "OParameterDialog::OnEntrySelected : no current entry !");

        m_pParam->SetText(::comphelper::getString(m_aFinalValues[nSelected].Value));
        m_nCurrentlySelected = nSelected;

        // with this the value isn't dirty
        OSL_ENSURE(static_cast<size_t>(m_nCurrentlySelected) < m_aVisitedParams.size(), "OParameterDialog::OnEntrySelected : invalid current entry !");
        m_aVisitedParams[m_nCurrentlySelected] &= ~EF_DIRTY;

        m_aResetVisitFlag.SetTimeout(1000);
        m_aResetVisitFlag.Start();

        return false;
    }

    IMPL_LINK_NOARG_TYPED(OParameterDialog, OnVisitedTimeout, Timer*, void)
    {
        OSL_ENSURE(m_nCurrentlySelected != LISTBOX_ENTRY_NOTFOUND, "OParameterDialog::OnVisitedTimeout : invalid call !");

        // mark the currently selected entry as visited
        OSL_ENSURE(static_cast<size_t>(m_nCurrentlySelected) < m_aVisitedParams.size(), "OParameterDialog::OnVisitedTimeout : invalid entry !");
        m_aVisitedParams[m_nCurrentlySelected] |= EF_VISITED;

        // was it the last "not visited yet" entry ?
        ByteVector::const_iterator aIter;
        for (   aIter = m_aVisitedParams.begin();
                aIter < m_aVisitedParams.end();
                ++aIter
            )
        {
            if (((*aIter) & EF_VISITED) == 0)
                break;
        }
        if (aIter == m_aVisitedParams.end())
        {   // yes, there isn't another one -> change the "default button"
            m_pTravelNext->SetStyle(m_pTravelNext->GetStyle() & ~WB_DEFBUTTON);
            m_pOKBtn->SetStyle(m_pOKBtn->GetStyle() | WB_DEFBUTTON);

            // set to focus to one of the buttons temporary (with this their "default"-state is really updated)
            vcl::Window* pOldFocus = Application::GetFocusWindow();

            // if the old focus window is the value edit do some preparations ...
            Selection aSel;
            if (pOldFocus == m_pParam)
            {
                m_pParam->SetLoseFocusHdl(Link<Control&,void>());
                aSel = m_pParam->GetSelection();
            }
            m_pTravelNext->GrabFocus();
            if (pOldFocus)
                pOldFocus->GrabFocus();

            // restore the settings for the value edit
            if (pOldFocus == m_pParam)
            {
                m_pParam->SetLoseFocusHdl(LINK(this, OParameterDialog, OnValueLoseFocusHdl));
                m_pParam->SetSelection(aSel);
            }
        }
    }

    IMPL_LINK_NOARG_TYPED(OParameterDialog, OnValueModified, Edit&, void)
    {
        // mark the currently selected entry as dirty
        OSL_ENSURE(static_cast<size_t>(m_nCurrentlySelected) < m_aVisitedParams.size(), "OParameterDialog::OnValueModified : invalid entry !");
        m_aVisitedParams[m_nCurrentlySelected] |= EF_DIRTY;

        m_bNeedErrorOnCurrent = true;
    }

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
