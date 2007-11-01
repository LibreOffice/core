/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AdvancedSettingsDlg.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 15:38:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#include "unoadmin.hxx"
#include "dbu_reghelper.hxx"
#include "advancedsettingsdlg.hxx"

//.........................................................................
namespace dbaui
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;

    //=========================================================================
    //= OAdvancedSettingsDialog
    //=========================================================================
    class OAdvancedSettingsDialog
            :public ODatabaseAdministrationDialog
            ,public ::comphelper::OPropertyArrayUsageHelper< OAdvancedSettingsDialog >
    {

    protected:
        OAdvancedSettingsDialog(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB);

    public:
        // XTypeProvider
        virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
        virtual ::comphelper::StringSequence SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

        // XServiceInfo - static methods
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::rtl::OUString getImplementationName_Static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                SAL_CALL Create(const ::com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >&);

        // XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException);
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;
    protected:
    // OGenericUnoDialog overridables
        virtual Dialog* createDialog(Window* _pParent);
    };

    //=========================================================================
    //-------------------------------------------------------------------------
    OAdvancedSettingsDialog::OAdvancedSettingsDialog(const Reference< XMultiServiceFactory >& _rxORB)
        :ODatabaseAdministrationDialog(_rxORB)
    {
    }
    //-------------------------------------------------------------------------
    Sequence<sal_Int8> SAL_CALL OAdvancedSettingsDialog::getImplementationId(  ) throw(RuntimeException)
    {
        static ::cppu::OImplementationId aId;
        return aId.getImplementationId();
    }

    //-------------------------------------------------------------------------
    Reference< XInterface > SAL_CALL OAdvancedSettingsDialog::Create(const Reference< XMultiServiceFactory >& _rxFactory)
    {
        return *(new OAdvancedSettingsDialog(_rxFactory));
    }

    //-------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OAdvancedSettingsDialog::getImplementationName() throw(RuntimeException)
    {
        return getImplementationName_Static();
    }

    //-------------------------------------------------------------------------
    ::rtl::OUString OAdvancedSettingsDialog::getImplementationName_Static() throw(RuntimeException)
    {
        return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.comp.dbu.OAdvancedSettingsDialog"));
    }

    //-------------------------------------------------------------------------
    ::comphelper::StringSequence SAL_CALL OAdvancedSettingsDialog::getSupportedServiceNames() throw(RuntimeException)
    {
        return getSupportedServiceNames_Static();
    }

    //-------------------------------------------------------------------------
    ::comphelper::StringSequence OAdvancedSettingsDialog::getSupportedServiceNames_Static() throw(RuntimeException)
    {
        ::comphelper::StringSequence aSupported(1);
        aSupported.getArray()[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.AdvancedDatabaseSettingsDialog"));
        return aSupported;
    }

    //-------------------------------------------------------------------------
    Reference<XPropertySetInfo>  SAL_CALL OAdvancedSettingsDialog::getPropertySetInfo() throw(RuntimeException)
    {
        Reference<XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }

    //-------------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper& OAdvancedSettingsDialog::getInfoHelper()
    {
        return *const_cast<OAdvancedSettingsDialog*>(this)->getArrayHelper();
    }

    //------------------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper* OAdvancedSettingsDialog::createArrayHelper( ) const
    {
        Sequence< Property > aProps;
        describeProperties(aProps);
        return new ::cppu::OPropertyArrayHelper(aProps);
    }
    //------------------------------------------------------------------------------
    Dialog* OAdvancedSettingsDialog::createDialog(Window* _pParent)
    {
        AdvancedSettingsDialog* pDlg = new AdvancedSettingsDialog(_pParent, m_pDatasourceItems, m_xORB,m_aInitialSelection);
        return pDlg;
    }

//.........................................................................
}   // namespace dbaui
//.........................................................................

extern "C" void SAL_CALL createRegistryInfo_OAdvancedSettingsDialog()
{
    static ::dbaui::OMultiInstanceAutoRegistration< ::dbaui::OAdvancedSettingsDialog > aAutoRegistration;
}
