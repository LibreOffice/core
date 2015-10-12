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

#include "modulepcr.hxx"
#include "pcrcommon.hxx"
#include "pcrservices.hxx"
#include "inspectormodelbase.hxx"

#include <com/sun/star/ucb/AlreadyInitializedException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <comphelper/broadcasthelper.hxx>
#include <comphelper/uno3.hxx>


namespace pcr
{


    using ::com::sun::star::inspection::XObjectInspectorModel;
    using ::com::sun::star::lang::XInitialization;
    using ::com::sun::star::lang::XServiceInfo;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::inspection::PropertyCategoryDescriptor;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::ucb::AlreadyInitializedException;
    using ::com::sun::star::beans::XPropertySetInfo;
    using ::com::sun::star::uno::makeAny;


    //= ObjectInspectorModel

    class ObjectInspectorModel : public ImplInspectorModel
    {
    private:
        Sequence< Any >             m_aFactories;

    public:
        ObjectInspectorModel();

        // XObjectInspectorModel
        virtual Sequence< Any > SAL_CALL getHandlerFactories() throw (RuntimeException, std::exception) override;
        virtual Sequence< PropertyCategoryDescriptor > SAL_CALL describeCategories(  ) throw (RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getPropertyOrderIndex( const OUString& PropertyName ) throw (RuntimeException, std::exception) override;

        // XInitialization
        virtual void SAL_CALL initialize( const Sequence< Any >& aArguments ) throw (Exception, RuntimeException, std::exception) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw (RuntimeException, std::exception) override;
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (RuntimeException, std::exception) override;

        // XServiceInfo - static versions
        static OUString getImplementationName_static(  ) throw(RuntimeException);
        static Sequence< OUString > getSupportedServiceNames_static(  ) throw(RuntimeException);
        static Reference< XInterface > SAL_CALL
                        Create(const Reference< XComponentContext >&);

    protected:
        void    createDefault();
        void    createWithHandlerFactories( const Sequence< Any >& _rFactories );
        void    createWithHandlerFactoriesAndHelpSection( const Sequence< Any >& _rFactories, sal_Int32 _nMinHelpTextLines, sal_Int32 _nMaxHelpTextLines );

    private:
        /** checks a given condition to be <TRUE/>, and throws an IllegalArgumentException if not
        */
        void    impl_verifyArgument_throw( bool _bCondition, sal_Int16 _nArgumentPosition );
    };


    //= ObjectInspectorModel

    ObjectInspectorModel::ObjectInspectorModel()
        :ImplInspectorModel()
    {
    }


    Sequence< Any > SAL_CALL ObjectInspectorModel::getHandlerFactories() throw (RuntimeException, std::exception)
    {
        return m_aFactories;
    }


    Sequence< PropertyCategoryDescriptor > SAL_CALL ObjectInspectorModel::describeCategories(  ) throw (RuntimeException, std::exception)
    {
        // no category info provided by this default implementation
        return Sequence< PropertyCategoryDescriptor >( );
    }


    ::sal_Int32 SAL_CALL ObjectInspectorModel::getPropertyOrderIndex( const OUString& /*PropertyName*/ ) throw (RuntimeException, std::exception)
    {
        // no ordering provided by this default implementation
        return 0;
    }


    void SAL_CALL ObjectInspectorModel::initialize( const Sequence< Any >& _arguments ) throw (Exception, RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( m_aFactories.getLength() )
            throw AlreadyInitializedException();

        StlSyntaxSequence< Any > arguments( _arguments );
        if ( arguments.empty() )
        {   // constructor: "createDefault()"
            createDefault();
            return;
        }

        Sequence< Any > factories;
        impl_verifyArgument_throw( arguments[0] >>= factories, 1 );

        if ( arguments.size() == 1 )
        {   // constructor: "createWithHandlerFactories( any[] )"
            createWithHandlerFactories( factories );
            return;
        }

        sal_Int32 nMinHelpTextLines( 0 ), nMaxHelpTextLines( 0 );
        if ( arguments.size() == 3 )
        {   // constructor: "createWithHandlerFactoriesAndHelpSection( any[], long, long )"
            impl_verifyArgument_throw( arguments[1] >>= nMinHelpTextLines, 2 );
            impl_verifyArgument_throw( arguments[2] >>= nMaxHelpTextLines, 3 );
            createWithHandlerFactoriesAndHelpSection( factories, nMinHelpTextLines, nMaxHelpTextLines );
            return;
        }

        impl_verifyArgument_throw( false, 2 );
    }


    OUString SAL_CALL ObjectInspectorModel::getImplementationName(  ) throw (RuntimeException, std::exception)
    {
        return getImplementationName_static();
    }


    Sequence< OUString > SAL_CALL ObjectInspectorModel::getSupportedServiceNames(  ) throw (RuntimeException, std::exception)
    {
        return getSupportedServiceNames_static();
    }


    OUString ObjectInspectorModel::getImplementationName_static(  ) throw(RuntimeException)
    {
        return OUString( "org.openoffice.comp.extensions.ObjectInspectorModel" );
    }


    Sequence< OUString > ObjectInspectorModel::getSupportedServiceNames_static(  ) throw(RuntimeException)
    {
        OUString sService( "com.sun.star.inspection.ObjectInspectorModel" );
        return Sequence< OUString >( &sService, 1 );
    }


    Reference< XInterface > SAL_CALL ObjectInspectorModel::Create(const Reference< XComponentContext >& /* _rxContext */ )
    {
        return *( new ObjectInspectorModel() );
    }


    void ObjectInspectorModel::createDefault()
    {
        m_aFactories.realloc( 1 );
        m_aFactories[0] <<= OUString( "com.sun.star.inspection.GenericPropertyHandler" );
    }


    void ObjectInspectorModel::createWithHandlerFactories( const Sequence< Any >& _rFactories )
    {
        impl_verifyArgument_throw( _rFactories.getLength() > 0, 1 );
        m_aFactories = _rFactories;
    }


    void ObjectInspectorModel::createWithHandlerFactoriesAndHelpSection( const Sequence< Any >& _rFactories, sal_Int32 _nMinHelpTextLines, sal_Int32 _nMaxHelpTextLines )
    {
        impl_verifyArgument_throw( _rFactories.getLength() > 0, 1 );
        impl_verifyArgument_throw( _nMinHelpTextLines >= 1, 2 );
        impl_verifyArgument_throw( _nMaxHelpTextLines >= 1, 3 );
        impl_verifyArgument_throw( _nMinHelpTextLines <= _nMaxHelpTextLines, 2 );

        m_aFactories = _rFactories;
        enableHelpSectionProperties( _nMinHelpTextLines, _nMaxHelpTextLines );
    }


    void ObjectInspectorModel::impl_verifyArgument_throw( bool _bCondition, sal_Int16 _nArgumentPosition )
    {
        if ( !_bCondition )
            throw IllegalArgumentException( OUString(), *this, _nArgumentPosition );
    }


} // namespace pcr



extern "C" void SAL_CALL createRegistryInfo_ObjectInspectorModel()
{
    ::pcr::OAutoRegistration< ::pcr::ObjectInspectorModel > aObjectInspectorModelRegistration;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
