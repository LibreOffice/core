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
#ifndef INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_FUNCTIONS_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_FUNCTIONS_HXX

#include <com/sun/star/report/XFunctions.hpp>
#include <com/sun/star/report/XFunctionsSupplier.hpp>
#include <cppuhelper/compbase.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <list>


namespace reportdesign
{
    typedef ::cppu::WeakComponentImplHelper< com::sun::star::report::XFunctions> FunctionsBase;
    /** \class OFunctions Defines the implementation of a \interface com:::sun::star::report::XFunctions
     * \ingroup reportdesign_api
     *
     */
    class OFunctions : public comphelper::OBaseMutex,
                    public FunctionsBase
    {
        typedef ::std::list< ::com::sun::star::uno::Reference< ::com::sun::star::report::XFunction > >  TFunctions;
        ::cppu::OInterfaceContainerHelper                                                       m_aContainerListeners;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >            m_xContext;
        ::com::sun::star::uno::WeakReference< ::com::sun::star::report::XFunctionsSupplier >    m_xParent;
        TFunctions                                                                              m_aFunctions;
    private:
        OFunctions& operator=(const OFunctions&) = delete;
        OFunctions(const OFunctions&) = delete;
        void checkIndex(sal_Int32 _nIndex);
    protected:
        // TODO: VirtualFunctionFinder: This is virtual function!

        virtual ~OFunctions();

        /** this function is called upon disposing the component
        */
        // TODO: VirtualFunctionFinder: This is virtual function!

        virtual void SAL_CALL disposing() SAL_OVERRIDE;
    public:
        explicit OFunctions( const ::com::sun::star::uno::Reference< ::com::sun::star::report::XFunctionsSupplier >& _xParent
                ,const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& context);

    // XFunctions
        // Methods
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::report::XFunction > SAL_CALL createFunction(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    // XIndexContainer
        virtual void SAL_CALL insertByIndex( ::sal_Int32 Index, const ::com::sun::star::uno::Any& Element ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL removeByIndex( ::sal_Int32 Index ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    // XIndexReplace
        virtual void SAL_CALL replaceByIndex( ::sal_Int32 Index, const ::com::sun::star::uno::Any& Element ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    // XIndexAccess
        virtual ::sal_Int32 SAL_CALL getCount(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    // XElementAccess
        virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL hasElements(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    // XChild
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getParent(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setParent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Parent ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    // XContainer
        virtual void SAL_CALL addContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL removeContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XComponent
        virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
        {
            cppu::WeakComponentImplHelperBase::addEventListener(aListener);
        }
        virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
        {
            cppu::WeakComponentImplHelperBase::removeEventListener(aListener);
        }
    };
}
#endif // INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_FUNCTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
