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

#include "udlg_module.hxx"
#include "roadmapskeleton.hxx"

#include <comphelper/componentcontext.hxx>
#include "svtools/genericunodialog.hxx"

//........................................................................
namespace udlg
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::beans::XPropertySetInfo;
    using ::com::sun::star::beans::Property;
    /** === end UNO using === **/

    //====================================================================
    //= UnoDialogSkeleton
    //====================================================================
    class UnoDialogSkeleton;
    typedef ::svt::OGenericUnoDialog                                                UnoDialogSkeleton_Base;
    typedef ::comphelper::OPropertyArrayUsageHelper< UnoDialogSkeleton >  UnoDialogSkeleton_PBase;

    class UnoDialogSkeleton
                :public UnoDialogSkeleton_Base
                ,public UnoDialogSkeleton_PBase
                ,public UdlgClient
    {
    public:
        UnoDialogSkeleton( const Reference< XComponentContext >& _rxContext );

        // XTypeProvider
        virtual Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw(RuntimeException);
        virtual Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(RuntimeException);

        // XPropertySet
        virtual Reference< XPropertySetInfo >  SAL_CALL getPropertySetInfo() throw(RuntimeException);
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

        // helper for factories
        static Reference< XInterface > SAL_CALL Create( const Reference< XComponentContext >& _rxContext );
        static ::rtl::OUString SAL_CALL getImplementationName_static() throw(RuntimeException);
        static Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_static() throw(RuntimeException);

    protected:
        ~UnoDialogSkeleton();

    protected:
        virtual Dialog* createDialog( Window* _pParent );
        virtual void destroyDialog();

    private:
        ::comphelper::ComponentContext  m_aContext;
    };

    //====================================================================
    //= UnoDialogSkeleton
    //====================================================================
    //--------------------------------------------------------------------
    UnoDialogSkeleton::UnoDialogSkeleton( const Reference< XComponentContext >& _rxContext )
        :UnoDialogSkeleton_Base( _rxContext )
        ,m_aContext( _rxContext )
    {
    }

    //--------------------------------------------------------------------
    UnoDialogSkeleton::~UnoDialogSkeleton()
    {
        // we do this here cause the base class' call to destroyDialog won't reach us anymore : we're within an dtor,
        // so this virtual-method-call the base class does does not work, we're already dead then ...
        if ( m_pDialog )
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            if ( m_pDialog )
                destroyDialog();
        }
    }

    //--------------------------------------------------------------------
    Reference< XInterface > SAL_CALL UnoDialogSkeleton::Create( const Reference< XComponentContext >& _rxContext )
    {
        return *(new UnoDialogSkeleton( _rxContext ) );
    }

    //--------------------------------------------------------------------
    Dialog* UnoDialogSkeleton::createDialog( Window* _pParent )
    {
        return new RoadmapSkeletonDialog( m_aContext, _pParent );
    }

    //--------------------------------------------------------------------
    void UnoDialogSkeleton::destroyDialog()
    {
        UnoDialogSkeleton_Base::destroyDialog();
    }

    //--------------------------------------------------------------------
    Sequence< sal_Int8 > SAL_CALL UnoDialogSkeleton::getImplementationId() throw(RuntimeException)
    {
        static ::cppu::OImplementationId* pId = NULL;
        if ( !pId )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !pId )
            {
                static ::cppu::OImplementationId aId;
                pId = &aId;
            }
        }
        return pId->getImplementationId();
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL UnoDialogSkeleton::getImplementationName_static() throw(RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.svtools.workben.UnoDialogSkeleton" ) );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL UnoDialogSkeleton::getSupportedServiceNames_static() throw(RuntimeException)
    {
        Sequence< ::rtl::OUString > aServices(1);
        aServices[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.example.UnoDialogSample" ) );
        return aServices;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL UnoDialogSkeleton::getImplementationName() throw(RuntimeException)
    {
        return getImplementationName_static();
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL UnoDialogSkeleton::getSupportedServiceNames() throw(RuntimeException)
    {
        return getSupportedServiceNames_static();
    }

    //--------------------------------------------------------------------
    Reference< XPropertySetInfo > SAL_CALL UnoDialogSkeleton::getPropertySetInfo() throw(RuntimeException)
    {
        return createPropertySetInfo( getInfoHelper() );
    }

    //--------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper& SAL_CALL UnoDialogSkeleton::getInfoHelper()
    {
        return *const_cast< UnoDialogSkeleton* >( this )->getArrayHelper();
    }

    //--------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper* UnoDialogSkeleton::createArrayHelper( ) const
    {
        Sequence< Property > aProps;
        describeProperties( aProps );
        return new ::cppu::OPropertyArrayHelper( aProps );
    }

    //--------------------------------------------------------------------
    void createRegistryInfo_UnoDialogSkeleton()
    {
        static OAutoRegistration< UnoDialogSkeleton > aAutoRegistration;
    }

//........................................................................
} // namespace udlg
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
