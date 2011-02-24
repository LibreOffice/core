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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#include "paramdialog.hxx"
#include "paramdialog.hrc"
#include "dbu_dlg.hrc"
#include "commontypes.hxx"
#include "moduledbu.hxx"
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <connectivity/dbtools.hxx>
#include "dbustrings.hrc"
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <osl/diagnose.h>
#include <tools/diagnose_ex.h>
#include "localresaccess.hxx"
#include <unotools/syslocale.hxx>

#define EF_VISITED      0x0001
#define EF_DIRTY        0x0002

//.........................................................................
namespace dbaui
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::util;
    using namespace ::connectivity;

    //==================================================================
    //= OParameterDialog
    //==================================================================

    //------------------------------------------------------------------------------
    #define INIT_MEMBERS()                                          \
        :ModalDialog( pParent, ModuleRes(DLG_PARAMETERS))           \
        ,m_aNamesFrame  (this, ModuleRes(FL_PARAMS))                    \
        ,m_aAllParams   (this, ModuleRes(LB_ALLPARAMS))                 \
        ,m_aValueFrame  (this, ModuleRes(FT_VALUE))                     \
        ,m_aParam       (this, ModuleRes(ET_PARAM))                     \
        ,m_aTravelNext  (this, ModuleRes(BT_TRAVELNEXT))                \
        ,m_aOKBtn       (this, ModuleRes(BT_OK))                        \
        ,m_aCancelBtn   (this, ModuleRes(BT_CANCEL))                    \
        ,m_nCurrentlySelected(LISTBOX_ENTRY_NOTFOUND)               \
        ,m_xConnection(_rxConnection)                               \
        ,m_aPredicateInput( _rxORB, _rxConnection, getParseContext() )  \
        ,m_bNeedErrorOnCurrent(sal_True)                            \


    //------------------------------------------------------------------------------
DBG_NAME(OParameterDialog)

    OParameterDialog::OParameterDialog(
            Window* pParent, const Reference< XIndexAccess > & rParamContainer,
            const Reference< XConnection > & _rxConnection, const Reference< XMultiServiceFactory >& _rxORB)
        INIT_MEMBERS()
    {
        DBG_CTOR(OParameterDialog,NULL);

        if (_rxORB.is())
            m_xFormatter = Reference< XNumberFormatter>(_rxORB->createInstance(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.util.NumberFormatter"))), UNO_QUERY);
        else {
            OSL_ASSERT("OParameterDialog::OParameterDialog: need a service factory!");
        }

        Reference< XNumberFormatsSupplier >  xNumberFormats = ::dbtools::getNumberFormats(m_xConnection, sal_True);
        if (!xNumberFormats.is())
            ::comphelper::disposeComponent(m_xFormatter);
        else if (m_xFormatter.is())
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
                m_aAllParams.InsertEntry(pValues->Name);

                if (!pValues->Value.hasValue())
                    // it won't have a value, 'cause it's default constructed. But may be later we support
                    // initializing this dialog with values
                    pValues->Value = makeAny(::rtl::OUString());
                    // default the values to an empty string

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

        FreeResource();
    }

    //------------------------------------------------------------------------------
    OParameterDialog::~OParameterDialog()
    {
        if (m_aResetVisitFlag.IsActive())
            m_aResetVisitFlag.Stop();

        DBG_DTOR(OParameterDialog,NULL);
    }

    //------------------------------------------------------------------------------
    void OParameterDialog::Construct()
    {
        m_aAllParams.SetSelectHdl(LINK(this, OParameterDialog, OnEntrySelected));
        m_aParam.SetLoseFocusHdl(LINK(this, OParameterDialog, OnValueLoseFocus));
        m_aParam.SetModifyHdl(LINK(this, OParameterDialog, OnValueModified));
        m_aTravelNext.SetClickHdl(LINK(this, OParameterDialog, OnButtonClicked));
        m_aOKBtn.SetClickHdl(LINK(this, OParameterDialog, OnButtonClicked));
        m_aCancelBtn.SetClickHdl(LINK(this, OParameterDialog, OnButtonClicked));

        if (m_aAllParams.GetEntryCount())
        {
            m_aAllParams.SelectEntryPos(0);
            LINK(this, OParameterDialog, OnEntrySelected).Call(&m_aAllParams);

            if (m_aAllParams.GetEntryCount() == 1)
            {
                m_aTravelNext.Enable(sal_False);
            }

            if (m_aAllParams.GetEntryCount() > 1)
            {
                m_aOKBtn.SetStyle(m_aOKBtn.GetStyle() & ~WB_DEFBUTTON);
                m_aTravelNext.SetStyle(m_aTravelNext.GetStyle() | WB_DEFBUTTON);
            }
        }

        m_aParam.GrabFocus();
    }

    //------------------------------------------------------------------------------
    IMPL_LINK(OParameterDialog, OnValueLoseFocus, Control*, /*pSource*/)
    {
        if (m_nCurrentlySelected != LISTBOX_ENTRY_NOTFOUND)
        {
            if ( ( m_aVisitedParams[ m_nCurrentlySelected ] & EF_DIRTY ) == 0 )
                // nothing to do, the value isn't dirty
                return 0L;
        }

        // transform the current string according to the param field type
        ::rtl::OUString sTransformedText(m_aParam.GetText());
        Reference< XPropertySet >  xParamAsSet;
        m_xParams->getByIndex(m_nCurrentlySelected) >>= xParamAsSet;
        if (xParamAsSet.is())
        {
            if (m_xConnection.is() && m_xFormatter.is())
            {
                ::rtl::OUString sParamValue( m_aParam.GetText() );
                sal_Bool bValid = m_aPredicateInput.normalizePredicateString( sParamValue, xParamAsSet );
                m_aParam.SetText( sParamValue );
                if ( bValid )
                {
                    // with this the value isn't dirty anymore
                    if (m_nCurrentlySelected != LISTBOX_ENTRY_NOTFOUND)
                        m_aVisitedParams[m_nCurrentlySelected] &= ~EF_DIRTY;
                }
                else
                {
                    if (!m_bNeedErrorOnCurrent)
                        return 1L;

                    m_bNeedErrorOnCurrent = sal_False;  // will be reset in OnValueModified

                    ::rtl::OUString sName;
                    try
                    {
                        sName = ::comphelper::getString(xParamAsSet->getPropertyValue(PROPERTY_NAME));
                    }
                    catch(Exception&)
                    {
                        DBG_UNHANDLED_EXCEPTION();
                    }

                    String sMessage;
                    {
                        LocalResourceAccess aDummy(DLG_PARAMETERS, RSC_MODALDIALOG);
                        sMessage = String(ModuleRes(STR_COULD_NOT_CONVERT_PARAM));
                    }
                    sMessage.SearchAndReplaceAll(String::CreateFromAscii("$name$"), sName.getStr());
                    ErrorBox(NULL, WB_OK, sMessage).Execute();
                    m_aParam.GrabFocus();
                    return 1L;
                }
            }
        }

        return 0L;
    }

    //------------------------------------------------------------------------------
    IMPL_LINK(OParameterDialog, OnButtonClicked, PushButton*, pButton)
    {
        if (&m_aCancelBtn == pButton)
        {
            // no interpreting of the given values anymore ....
            m_aParam.SetLoseFocusHdl(Link());   // no direct call from the control anymore ...
            m_bNeedErrorOnCurrent = sal_False;      // in case of any indirect calls -> no error message
            m_aCancelBtn.SetClickHdl(Link());
            m_aCancelBtn.Click();
        }
        else if (&m_aOKBtn == pButton)
        {
            // transfer the current values into the Any
            if (LINK(this, OParameterDialog, OnEntrySelected).Call(&m_aAllParams) != 0L)
            {   // there was an error interpreting the current text
                m_bNeedErrorOnCurrent = sal_True;
                    // we're are out of the complex web :) of direct and indirect calls to OnValueLoseFocus now,
                    // so the next time it is called we need an error message, again ....
                    // (TODO : there surely are better solutions for this ...)
                return 1L;
            }

            if (m_xParams.is())
            {
                // write the parameters
                try
                {
                    ::rtl::OUString sError;
                    PropertyValue* pValues = m_aFinalValues.getArray();
                    for (sal_Int32 i = 0, nCount = m_xParams->getCount(); i<nCount; ++i, ++pValues)
                    {
                        Reference< XPropertySet >  xParamAsSet;
                        m_xParams->getByIndex(i) >>= xParamAsSet;

                        ::rtl::OUString sValue;
                        pValues->Value >>= sValue;
                        pValues->Value <<= ::rtl::OUString( m_aPredicateInput.getPredicateValue( sValue, xParamAsSet, sal_False ) );
                    }
                }
                catch(Exception&)
                {
                    DBG_UNHANDLED_EXCEPTION();
                }

            }
            // to close the dialog (which is more code than a simple EndDialog)
            m_aOKBtn.SetClickHdl(Link());
            m_aOKBtn.Click();
        }
        else if (&m_aTravelNext == pButton)
        {
            sal_uInt16 nCurrent = m_aAllParams.GetSelectEntryPos();
            sal_uInt16 nCount = m_aAllParams.GetEntryCount();
            OSL_ENSURE(nCount == m_aVisitedParams.size(), "OParameterDialog::OnButtonClicked : inconsistent lists !");

            // search the next entry in list we haven't visited yet
            sal_uInt16 nNext = (nCurrent + 1) % nCount;
            while ((nNext != nCurrent) && ( m_aVisitedParams[nNext] & EF_VISITED ))
                nNext = (nNext + 1) % nCount;

            if ( m_aVisitedParams[nNext] & EF_VISITED )
                // there is no such "not visited yet" entry -> simpy take the next one
                nNext = (nCurrent + 1) % nCount;

            m_aAllParams.SelectEntryPos(nNext);
            LINK(this, OParameterDialog, OnEntrySelected).Call(&m_aAllParams);
            m_bNeedErrorOnCurrent = sal_True;
                // we're are out of the complex web :) of direct and indirect calls to OnValueLoseFocus now,
                // so the next time it is called we need an error message, again ....
                // (TODO : there surely are better solutions for this ...)
        }

        return 0L;
    }

    //------------------------------------------------------------------------------
    IMPL_LINK(OParameterDialog, OnEntrySelected, ListBox*, /*pList*/)
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
            if (LINK(this, OParameterDialog, OnValueLoseFocus).Call(&m_aParam) != 0L)
            {   // there was an error interpreting the text
                m_aAllParams.SelectEntryPos(m_nCurrentlySelected);
                return 1L;
            }

            m_aFinalValues[m_nCurrentlySelected].Value <<= ::rtl::OUString(m_aParam.GetText());
        }

        // initialize the controls with the new values
        sal_uInt16 nSelected = m_aAllParams.GetSelectEntryPos();
        OSL_ENSURE(nSelected != LISTBOX_ENTRY_NOTFOUND, "OParameterDialog::OnEntrySelected : no current entry !");

        m_aParam.SetText(::comphelper::getString(m_aFinalValues[nSelected].Value));
        m_nCurrentlySelected = nSelected;

        // with this the value isn't dirty
        OSL_ENSURE(m_nCurrentlySelected < m_aVisitedParams.size(), "OParameterDialog::OnEntrySelected : invalid current entry !");
        m_aVisitedParams[m_nCurrentlySelected] &= ~EF_DIRTY;

        m_aResetVisitFlag.SetTimeout(1000);
        m_aResetVisitFlag.Start();

        return 0L;
    }

    //------------------------------------------------------------------------------
    IMPL_LINK(OParameterDialog, OnVisitedTimeout, Timer*, /*pTimer*/)
    {
        OSL_ENSURE(m_nCurrentlySelected != LISTBOX_ENTRY_NOTFOUND, "OParameterDialog::OnVisitedTimeout : invalid call !");

        // mark the currently selected entry as visited
        OSL_ENSURE(m_nCurrentlySelected < m_aVisitedParams.size(), "OParameterDialog::OnVisitedTimeout : invalid entry !");
        m_aVisitedParams[m_nCurrentlySelected] |= EF_VISITED;

        // was it the last "not visited yet" entry ?
        ConstByteVectorIterator aIter;
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
            m_aTravelNext.SetStyle(m_aTravelNext.GetStyle() & ~WB_DEFBUTTON);
            m_aOKBtn.SetStyle(m_aOKBtn.GetStyle() | WB_DEFBUTTON);

            // set to focus to one of the buttons temporary (with this their "default"-state is really updated)
            Window* pOldFocus = Application::GetFocusWindow();

            // if the old focus window is the value edit do some preparations ...
            Selection aSel;
            if (pOldFocus == &m_aParam)
            {
                m_aParam.SetLoseFocusHdl(Link());
                aSel = m_aParam.GetSelection();
            }
            m_aTravelNext.GrabFocus();
            if (pOldFocus)
                pOldFocus->GrabFocus();

            // restore the settings for the value edit
            if (pOldFocus == &m_aParam)
            {
                m_aParam.SetLoseFocusHdl(LINK(this, OParameterDialog, OnValueLoseFocus));
                m_aParam.SetSelection(aSel);
            }
        }

        return 0L;
    }

    //------------------------------------------------------------------------------
    IMPL_LINK(OParameterDialog, OnValueModified, Control*, /*pBox*/)
    {
        // mark the currently selected entry as dirty
        OSL_ENSURE(m_nCurrentlySelected < m_aVisitedParams.size(), "OParameterDialog::OnValueModified : invalid entry !");
        m_aVisitedParams[m_nCurrentlySelected] |= EF_DIRTY;

        m_bNeedErrorOnCurrent = sal_True;

        return 0L;
    }


//.........................................................................
}   // namespace dbaui
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
