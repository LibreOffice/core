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
    typedef ::cppu::WeakComponentImplHelper< css::report::XFunctions> FunctionsBase;
    /** \class OFunctions Defines the implementation of a \interface com:::sun::star::report::XFunctions
     * \ingroup reportdesign_api
     *
     */
    class OFunctions : public comphelper::OBaseMutex,
                    public FunctionsBase
    {
        typedef ::std::list< css::uno::Reference< css::report::XFunction > >  TFunctions;
        ::cppu::OInterfaceContainerHelper                             m_aContainerListeners;
        css::uno::Reference< css::uno::XComponentContext >            m_xContext;
        css::uno::WeakReference< css::report::XFunctionsSupplier >    m_xParent;
        TFunctions                                                    m_aFunctions;
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

        virtual void SAL_CALL disposing() override;
    public:
        explicit OFunctions( const css::uno::Reference< css::report::XFunctionsSupplier >& _xParent
                ,const css::uno::Reference< css::uno::XComponentContext >& context);

    // XFunctions
        // Methods
        virtual css::uno::Reference< css::report::XFunction > SAL_CALL createFunction(  ) throw (css::uno::RuntimeException, std::exception) override;
    // XIndexContainer
        virtual void SAL_CALL insertByIndex( ::sal_Int32 Index, const css::uno::Any& Element ) throw (css::lang::IllegalArgumentException, css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeByIndex( ::sal_Int32 Index ) throw (css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    // XIndexReplace
        virtual void SAL_CALL replaceByIndex( ::sal_Int32 Index, const css::uno::Any& Element ) throw (css::lang::IllegalArgumentException, css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    // XIndexAccess
        virtual ::sal_Int32 SAL_CALL getCount(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    // XElementAccess
        virtual css::uno::Type SAL_CALL getElementType(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL hasElements(  ) throw (css::uno::RuntimeException, std::exception) override;
    // XChild
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getParent(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setParent( const css::uno::Reference< css::uno::XInterface >& Parent ) throw (css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;
    // XContainer
        virtual void SAL_CALL addContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;

        // XComponent
        virtual void SAL_CALL dispose() throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addEventListener(const css::uno::Reference< css::lang::XEventListener > & aListener) throw(css::uno::RuntimeException, std::exception) override
        {
            cppu::WeakComponentImplHelperBase::addEventListener(aListener);
        }
        virtual void SAL_CALL removeEventListener(const css::uno::Reference< css::lang::XEventListener > & aListener) throw(css::uno::RuntimeException, std::exception) override
        {
            cppu::WeakComponentImplHelperBase::removeEventListener(aListener);
        }
    };
}
#endif // INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_FUNCTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
