/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: controlfontdialog.cxx,v $
 * $Revision: 1.8 $
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
#include "precompiled_extensions.hxx"

#ifndef _EXTENSIONS_PROPCTRLR_FONTITEMIDS_HXX_
#include "controlfontdialog.hxx"
#endif
#include <cppuhelper/typeprovider.hxx>
#include "fontdialog.hxx"
#include "formstrings.hxx"
#include "pcrcommon.hxx"

extern "C" void SAL_CALL createRegistryInfo_OControlFontDialog()
{
    ::pcr::OAutoRegistration< ::pcr::OControlFontDialog > aAutoRegistration;
}

//........................................................................
namespace pcr
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;

    //====================================================================
    //= OControlFontDialog
    //====================================================================
    //---------------------------------------------------------------------
    OControlFontDialog::OControlFontDialog(const Reference< XComponentContext >& _rxContext )
        :OGenericUnoDialog( _rxContext )
        ,m_pFontItems(NULL)
        ,m_pItemPool(NULL)
        ,m_pItemPoolDefaults(NULL)
    {
        registerProperty(PROPERTY_INTROSPECTEDOBJECT, OWN_PROPERTY_ID_INTROSPECTEDOBJECT,
            PropertyAttribute::BOUND | PropertyAttribute::TRANSIENT,
            &m_xControlModel, ::getCppuType(&m_xControlModel));
    }

    //---------------------------------------------------------------------
    OControlFontDialog::~OControlFontDialog()
    {
        if (m_pDialog)
        {
            ::osl::MutexGuard aGuard(m_aMutex);
            if (m_pDialog)
                destroyDialog();
        }
    }

    //---------------------------------------------------------------------
    Sequence<sal_Int8> SAL_CALL OControlFontDialog::getImplementationId(  ) throw(RuntimeException)
    {
        static ::cppu::OImplementationId aId;
        return aId.getImplementationId();
    }

    //---------------------------------------------------------------------
    Reference< XInterface > SAL_CALL OControlFontDialog::Create( const Reference< XComponentContext >& _rxContext )
    {
        return *( new OControlFontDialog( _rxContext ) );
    }

    //---------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OControlFontDialog::getImplementationName() throw(RuntimeException)
    {
        return getImplementationName_static();
    }

    //---------------------------------------------------------------------
    ::rtl::OUString OControlFontDialog::getImplementationName_static() throw(RuntimeException)
    {
        return ::rtl::OUString::createFromAscii("org.openoffice.comp.form.ui.OControlFontDialog");
    }

    //---------------------------------------------------------------------
    ::comphelper::StringSequence SAL_CALL OControlFontDialog::getSupportedServiceNames() throw(RuntimeException)
    {
        return getSupportedServiceNames_static();
    }

    //---------------------------------------------------------------------
    ::comphelper::StringSequence OControlFontDialog::getSupportedServiceNames_static() throw(RuntimeException)
    {
        ::comphelper::StringSequence aSupported(1);
        aSupported.getArray()[0] = ::rtl::OUString::createFromAscii("com.sun.star.form.ControlFontDialog");
        return aSupported;
    }

    //---------------------------------------------------------------------
    Reference<XPropertySetInfo>  SAL_CALL OControlFontDialog::getPropertySetInfo() throw(RuntimeException)
    {
        Reference<XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }

    //---------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper& OControlFontDialog::getInfoHelper()
    {
        return *const_cast<OControlFontDialog*>(this)->getArrayHelper();
    }

    //--------------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper* OControlFontDialog::createArrayHelper( ) const
    {
        Sequence< Property > aProps;
        describeProperties(aProps);
        return new ::cppu::OPropertyArrayHelper(aProps);
    }

    //--------------------------------------------------------------------------
    Dialog* OControlFontDialog::createDialog(Window* _pParent)
    {
        ControlCharacterDialog::createItemSet(m_pFontItems, m_pItemPool, m_pItemPoolDefaults);

        OSL_ENSURE(m_xControlModel.is(), "OControlFontDialog::createDialog: no introspectee set!");
        if (m_xControlModel.is())
            ControlCharacterDialog::translatePropertiesToItems(m_xControlModel, m_pFontItems);
        // TODO: we need a mechanism to prevent that somebody creates us, sets an introspectee, executes us,
        // sets a new introspectee and re-executes us. In this case, the dialog returned here (upon the first
        // execute) will be re-used upon the second execute, and thus it won't be initialized correctly.

        ControlCharacterDialog* pDialog = new ControlCharacterDialog(_pParent, *m_pFontItems);
        return pDialog;
    }

    //-------------------------------------------------------------------------
    void OControlFontDialog::destroyDialog()
    {
        OGenericUnoDialog::destroyDialog();
        ControlCharacterDialog::destroyItemSet(m_pFontItems, m_pItemPool, m_pItemPoolDefaults);
    }

    //-------------------------------------------------------------------------
    void OControlFontDialog::executedDialog(sal_Int16 _nExecutionResult)
    {
        OSL_ENSURE(m_pDialog, "OControlFontDialog::executedDialog: no dialog anymore?!!");
        if (m_pDialog && (sal_True == _nExecutionResult) && m_xControlModel.is())
        {
            const SfxItemSet* pOutput = static_cast<ControlCharacterDialog*>(m_pDialog)->GetOutputItemSet();
            if (pOutput)
                ControlCharacterDialog::translateItemsToProperties( *pOutput, m_xControlModel );
        }
    }

//........................................................................
}   // namespace pcr
//........................................................................

