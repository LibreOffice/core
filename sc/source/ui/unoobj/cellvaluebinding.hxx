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

#ifndef SC_CELLVALUEBINDING_HXX
#define SC_CELLVALUEBINDING_HXX

#include <com/sun/star/form/binding/XValueBinding.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <cppuhelper/compbase5.hxx>
#include <comphelper/propertycontainer.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/proparrhlp.hxx>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>


//.........................................................................
namespace calc
{
//.........................................................................

    //=====================================================================
    //= OCellValueBinding
    //=====================================================================
    class OCellValueBinding;
    // the base for our interfaces
    typedef ::cppu::WeakAggComponentImplHelper5 <   ::com::sun::star::form::binding::XValueBinding
                                                ,   ::com::sun::star::lang::XServiceInfo
                                                ,   ::com::sun::star::util::XModifyBroadcaster
                                                ,   ::com::sun::star::util::XModifyListener
                                                ,   ::com::sun::star::lang::XInitialization
                                                >   OCellValueBinding_Base;
    // the base for the property handling
    typedef ::comphelper::OPropertyContainer        OCellValueBinding_PBase;
    // the second base for property handling
    typedef ::comphelper::OPropertyArrayUsageHelper< OCellValueBinding >
                                                    OCellValueBinding_PABase;

    class OCellValueBinding :public ::comphelper::OBaseMutex
                            ,public OCellValueBinding_Base      // order matters! before OCellValueBinding_PBase, so rBHelper gets initialized
                            ,public OCellValueBinding_PBase
                            ,public OCellValueBinding_PABase
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheetDocument >
                    m_xDocument;            /// the document where our cell lives
        ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell >
                    m_xCell;                /// the cell we're bound to, for double value access
        ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >
                    m_xCellText;            /// the cell we're bound to, for text access
        ::cppu::OInterfaceContainerHelper
                    m_aModifyListeners;     /// our modify listeners
        sal_Bool    m_bInitialized;         /// has XInitialization::initialize been called?
        sal_Bool    m_bListPos;             /// constructed as ListPositionCellBinding?

    public:
        OCellValueBinding(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheetDocument >& _rxDocument,
            sal_Bool _bListPos
        );

        using OCellValueBinding_PBase::getFastPropertyValue;

    protected:
        ~OCellValueBinding( );

    protected:
        // XInterface
        DECLARE_XINTERFACE()

        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

        // XValueBinding
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getSupportedValueTypes(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsType( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getValue( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::form::binding::IncompatibleTypesException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setValue( const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::form::binding::IncompatibleTypesException, ::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);

        // OComponentHelper/XComponent
        virtual void SAL_CALL disposing();

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

        // XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

        // OPropertySetHelper
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
        virtual void SAL_CALL getFastPropertyValue( ::com::sun::star::uno::Any& _rValue, sal_Int32 _nHandle ) const;

        // ::comphelper::OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

        // XModifyBroadcaster
        virtual void SAL_CALL addModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

        // XModifyListener
        virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    private:
        void    checkDisposed( ) const
                    SAL_THROW( ( ::com::sun::star::lang::DisposedException ) );
        void    checkValueType( const ::com::sun::star::uno::Type& _rType ) const
                    SAL_THROW( ( ::com::sun::star::form::binding::IncompatibleTypesException ) );
        void    checkInitialized()
                    SAL_THROW( ( ::com::sun::star::uno::RuntimeException ) );

        /** notifies our modify listeners
            @precond
                our mutex is <em>not</em> locked
        */
        void    notifyModified();

        void    setBooleanFormat();

    private:
        OCellValueBinding();                                        // never implemented
        OCellValueBinding( const OCellValueBinding& );              // never implemented
        OCellValueBinding& operator=( const OCellValueBinding& );   // never implemented

#ifdef DBG_UTIL
    private:
        static  const char* checkConsistency_static( const void* _pThis );
                const char* checkConsistency( ) const;
#endif
    };

//.........................................................................
}   // namespace calc
//.........................................................................

#endif // SC_CELLVALUEBINDING_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
