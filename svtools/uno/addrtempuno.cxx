/*************************************************************************
 *
 *  $RCSfile: addrtempuno.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2005-04-13 11:50:32 $
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SVT_GENERICUNODIALOG_HXX_
#include "genericunodialog.hxx"
#endif
#ifndef _SVT_DOC_ADDRESSTEMPLATE_HXX_
#include "addresstemplate.hxx"
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif

class SfxItemSet;
class SfxItemPool;
class SfxPoolItem;

// .......................................................................
namespace svt
{
// .......................................................................

#define UNODIALOG_PROPERTY_ID_ALIASES       100
#define UNODIALOG_PROPERTY_ALIASES          "FieldMapping"

    using namespace com::sun::star::uno;
    using namespace com::sun::star::lang;
    using namespace com::sun::star::util;
    using namespace com::sun::star::beans;

    //=========================================================================
    //= OAddressBookSourceDialogUno
    //=========================================================================
    typedef OGenericUnoDialog OAddressBookSourceDialogUnoBase;
    class OAddressBookSourceDialogUno
            :public OAddressBookSourceDialogUnoBase
            ,public ::comphelper::OPropertyArrayUsageHelper< OAddressBookSourceDialogUno >
    {
    protected:
        Sequence< AliasProgrammaticPair >   m_aAliases;
        ::rtl::OUString                     m_sDataSource;
        ::rtl::OUString                     m_sTable;

    protected:
        OAddressBookSourceDialogUno(const Reference< XMultiServiceFactory >& _rxORB);

    public:
        // XTypeProvider
        virtual Sequence<sal_Int8> SAL_CALL getImplementationId(  ) throw(RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw(RuntimeException);
        virtual ::comphelper::StringSequence SAL_CALL getSupportedServiceNames() throw(RuntimeException);

        // XServiceInfo - static methods
        static Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw( RuntimeException );
        static ::rtl::OUString getImplementationName_Static(void) throw( RuntimeException );
        static Reference< XInterface >
                SAL_CALL Create(const Reference< com::sun::star::lang::XMultiServiceFactory >&);

        // XPropertySet
        virtual Reference< XPropertySetInfo>  SAL_CALL getPropertySetInfo() throw(RuntimeException);
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

    protected:
    // OGenericUnoDialog overridables
        virtual Dialog* createDialog(Window* _pParent);

        virtual void implInitialize(const com::sun::star::uno::Any& _rValue);

        virtual void executedDialog(sal_Int16 _nExecutionResult);
    };


    //=========================================================================
    //= OAddressBookSourceDialogUno
    //=========================================================================
    Reference< XInterface > SAL_CALL OAddressBookSourceDialogUno_CreateInstance( const Reference< XMultiServiceFactory >& _rxFactory)
    {
        return OAddressBookSourceDialogUno::Create(_rxFactory);
    }

    //-------------------------------------------------------------------------
    OAddressBookSourceDialogUno::OAddressBookSourceDialogUno(const Reference< XMultiServiceFactory >& _rxORB)
        :OGenericUnoDialog(_rxORB)
    {
        registerProperty(::rtl::OUString::createFromAscii(UNODIALOG_PROPERTY_ALIASES), UNODIALOG_PROPERTY_ID_ALIASES, PropertyAttribute::READONLY,
            &m_aAliases, getCppuType(&m_aAliases));
    }

    //-------------------------------------------------------------------------
    Sequence<sal_Int8> SAL_CALL OAddressBookSourceDialogUno::getImplementationId(  ) throw(RuntimeException)
    {
        static ::cppu::OImplementationId aId;
        return aId.getImplementationId();
    }

    //-------------------------------------------------------------------------
    Reference< XInterface > SAL_CALL OAddressBookSourceDialogUno::Create(const Reference< XMultiServiceFactory >& _rxFactory)
    {
        return *(new OAddressBookSourceDialogUno(_rxFactory));
    }

    //-------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OAddressBookSourceDialogUno::getImplementationName() throw(RuntimeException)
    {
        return getImplementationName_Static();
    }

    //-------------------------------------------------------------------------
    ::rtl::OUString OAddressBookSourceDialogUno::getImplementationName_Static() throw(RuntimeException)
    {
        return ::rtl::OUString::createFromAscii("com.sun.star.comp.svtools.OAddressBookSourceDialogUno");
    }

    //-------------------------------------------------------------------------
    ::comphelper::StringSequence SAL_CALL OAddressBookSourceDialogUno::getSupportedServiceNames() throw(RuntimeException)
    {
        return getSupportedServiceNames_Static();
    }

    //-------------------------------------------------------------------------
    ::comphelper::StringSequence OAddressBookSourceDialogUno::getSupportedServiceNames_Static() throw(RuntimeException)
    {
        ::comphelper::StringSequence aSupported(1);
        aSupported.getArray()[0] = ::rtl::OUString::createFromAscii("com.sun.star.ui.AddressBookSourceDialog");
        return aSupported;
    }

    //-------------------------------------------------------------------------
    Reference<XPropertySetInfo>  SAL_CALL OAddressBookSourceDialogUno::getPropertySetInfo() throw(RuntimeException)
    {
        Reference<XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }

    //-------------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper& OAddressBookSourceDialogUno::getInfoHelper()
    {
        return *const_cast<OAddressBookSourceDialogUno*>(this)->getArrayHelper();
    }

    //------------------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper* OAddressBookSourceDialogUno::createArrayHelper( ) const
    {
        Sequence< Property > aProps;
        describeProperties(aProps);
        return new ::cppu::OPropertyArrayHelper(aProps);
    }

    //------------------------------------------------------------------------------
    void OAddressBookSourceDialogUno::executedDialog(sal_Int16 _nExecutionResult)
    {
        OAddressBookSourceDialogUnoBase::executedDialog(_nExecutionResult);

        if ( _nExecutionResult )
            if ( m_pDialog )
                static_cast< AddressBookSourceDialog* >( m_pDialog )->getFieldMapping( m_aAliases );
    }

    //------------------------------------------------------------------------------
    void OAddressBookSourceDialogUno::implInitialize(const com::sun::star::uno::Any& _rValue)
    {
        PropertyValue aVal;
        if (_rValue >>= aVal)
        {
            if (0 == aVal.Name.compareToAscii("DataSource"))
            {
#if OSL_DEBUG_LEVEL > 0
                sal_Bool bSuccess =
#endif
                aVal.Value >>= m_sDataSource;
                OSL_ENSURE( bSuccess, "OAddressBookSourceDialogUno::implInitialize: invalid type for DataSource!" );
                return;
            }

            if (0 == aVal.Name.compareToAscii("Command"))
            {
#if OSL_DEBUG_LEVEL > 0
                sal_Bool bSuccess =
#endif
                aVal.Value >>= m_sTable;
                OSL_ENSURE( bSuccess, "OAddressBookSourceDialogUno::implInitialize: invalid type for Command!" );
                return;
            }
        }

        OAddressBookSourceDialogUnoBase::implInitialize( _rValue );
    }

    //------------------------------------------------------------------------------
    Dialog* OAddressBookSourceDialogUno::createDialog(Window* _pParent)
    {
        if ( m_sDataSource.getLength() && m_sTable.getLength() )
            return new AddressBookSourceDialog(_pParent, m_xORB, m_sDataSource, m_sTable, m_aAliases );
        else
            return new AddressBookSourceDialog( _pParent, m_xORB );
    }

// .......................................................................
}   // namespace svt
// .......................................................................

