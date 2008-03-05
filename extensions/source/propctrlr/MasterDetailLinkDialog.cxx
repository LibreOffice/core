/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MasterDetailLinkDialog.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:09:07 $
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

 #include "precompiled_extensions.hxx"
 #include "MasterDetailLinkDialog.hxx"
 #include "formlinkdialog.hxx"

 extern "C" void SAL_CALL createRegistryInfo_MasterDetailLinkDialog()
{
    ::pcr::OAutoRegistration< ::pcr::MasterDetailLinkDialog > aAutoRegistration;
}

//............................................................................
namespace pcr
{
//............................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;

    //====================================================================
    //= MasterDetailLinkDialog
    //====================================================================
    //---------------------------------------------------------------------
    MasterDetailLinkDialog::MasterDetailLinkDialog(const Reference< XComponentContext >& _rxContext )
        :OGenericUnoDialog( _rxContext )
    {
    }
//---------------------------------------------------------------------
    Sequence<sal_Int8> SAL_CALL MasterDetailLinkDialog::getImplementationId(  ) throw(RuntimeException)
    {
        static ::cppu::OImplementationId aId;
        return aId.getImplementationId();
    }

    //---------------------------------------------------------------------
    Reference< XInterface > SAL_CALL MasterDetailLinkDialog::Create( const Reference< XComponentContext >& _rxContext )
    {
        return *( new MasterDetailLinkDialog( _rxContext ) );
    }

    //---------------------------------------------------------------------
    ::rtl::OUString SAL_CALL MasterDetailLinkDialog::getImplementationName() throw(RuntimeException)
    {
        return getImplementationName_static();
    }

    //---------------------------------------------------------------------
    ::rtl::OUString MasterDetailLinkDialog::getImplementationName_static() throw(RuntimeException)
    {
        return ::rtl::OUString::createFromAscii("org.openoffice.comp.form.ui.MasterDetailLinkDialog");
    }

    //---------------------------------------------------------------------
    ::comphelper::StringSequence SAL_CALL MasterDetailLinkDialog::getSupportedServiceNames() throw(RuntimeException)
    {
        return getSupportedServiceNames_static();
    }

    //---------------------------------------------------------------------
    ::comphelper::StringSequence MasterDetailLinkDialog::getSupportedServiceNames_static() throw(RuntimeException)
    {
        ::comphelper::StringSequence aSupported(1);
        aSupported.getArray()[0] = ::rtl::OUString::createFromAscii("com.sun.star.form.MasterDetailLinkDialog");
        return aSupported;
    }

    //---------------------------------------------------------------------
    Reference<XPropertySetInfo>  SAL_CALL MasterDetailLinkDialog::getPropertySetInfo() throw(RuntimeException)
    {
        Reference<XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }

    //---------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper& MasterDetailLinkDialog::getInfoHelper()
    {
        return *const_cast<MasterDetailLinkDialog*>(this)->getArrayHelper();
    }

    //--------------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper* MasterDetailLinkDialog::createArrayHelper( ) const
    {
        Sequence< Property > aProps;
        describeProperties(aProps);
        return new ::cppu::OPropertyArrayHelper(aProps);
    }

    //--------------------------------------------------------------------------
    Dialog* MasterDetailLinkDialog::createDialog(Window* _pParent)
    {
        return new FormLinkDialog(_pParent,m_xDetail,m_xMaster,Reference< XMultiServiceFactory >( m_xContext->getServiceManager(),UNO_QUERY)
            ,m_sExplanation,m_sDetailLabel,m_sMasterLabel);
    }
    //---------------------------------------------------------------------
    void MasterDetailLinkDialog::implInitialize(const Any& _rValue)
    {
        PropertyValue aProperty;
        if (_rValue >>= aProperty)
        {
            if (0 == aProperty.Name.compareToAscii("Detail"))
            {
                OSL_VERIFY( aProperty.Value >>= m_xDetail );
                return;
            }
            else if (0 == aProperty.Name.compareToAscii("Master"))
            {
                OSL_VERIFY( aProperty.Value >>= m_xMaster );
                return;
            }
            else if (0 == aProperty.Name.compareToAscii("Explanation"))
            {
                OSL_VERIFY( aProperty.Value >>= m_sExplanation );
                return;
            }
            else if (0 == aProperty.Name.compareToAscii("DetailLabel"))
            {
                OSL_VERIFY( aProperty.Value >>= m_sDetailLabel );
                return;
            }
            else if (0 == aProperty.Name.compareToAscii("MasterLabel"))
            {
                OSL_VERIFY( aProperty.Value >>= m_sMasterLabel );
                return;
            }
        }
        MasterDetailLinkDialog_DBase::implInitialize(_rValue);
    }

//............................................................................
}   // namespace pcr
//............................................................................
