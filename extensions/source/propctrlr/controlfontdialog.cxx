/*************************************************************************
 *
 *  $RCSfile: controlfontdialog.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 16:03:49 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _EXTENSIONS_PROPCTRLR_FONTITEMIDS_HXX_
#include "controlfontdialog.hxx"
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _EXTENSIONS_PROPCTRLR_FONTDIALOG_HXX_
#include "fontdialog.hxx"
#endif
#ifndef _EXTENSIONS_FORMSCTRLR_PCRSTRINGS_HXX_
#include "pcrstrings.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_PROPCONTROLLER_HXX_
#include "propcontroller.hxx"
#endif

extern "C" void SAL_CALL createRegistryInfo_OControlFontDialog()
{
    static ::pcr::OMultiInstanceAutoRegistration< ::pcr::OControlFontDialog > aAutoRegistration;
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
    OControlFontDialog::OControlFontDialog(const Reference< XMultiServiceFactory >& _rxORB)
        :OGenericUnoDialog(_rxORB)
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
    Reference< XInterface > SAL_CALL OControlFontDialog::Create(const Reference< XMultiServiceFactory >& _rxFactory)
    {
        return *(new OControlFontDialog(_rxFactory));
    }

    //---------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OControlFontDialog::getImplementationName() throw(RuntimeException)
    {
        return getImplementationName_Static();
    }

    //---------------------------------------------------------------------
    ::rtl::OUString OControlFontDialog::getImplementationName_Static() throw(RuntimeException)
    {
        return ::rtl::OUString::createFromAscii("org.openoffice.comp.form.ui.OControlFontDialog");
    }

    //---------------------------------------------------------------------
    ::comphelper::StringSequence SAL_CALL OControlFontDialog::getSupportedServiceNames() throw(RuntimeException)
    {
        return getSupportedServiceNames_Static();
    }

    //---------------------------------------------------------------------
    ::comphelper::StringSequence OControlFontDialog::getSupportedServiceNames_Static() throw(RuntimeException)
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
                ControlCharacterDialog::translatePropertiesToItems(pOutput, m_xControlModel);
        }
    }

//........................................................................
}   // namespace pcr
//........................................................................

