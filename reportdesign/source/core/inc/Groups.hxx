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
#ifndef INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_GROUPS_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_GROUPS_HXX

#include <com/sun/star/report/XGroups.hpp>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <comphelper/interfacecontainer3.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <vector>


namespace reportdesign
{
    typedef ::cppu::WeakComponentImplHelper< css::report::XGroups> GroupsBase;
    /** \class OGroups Defines the implementation of a \interface com:::sun::star::report::XGroups
     * \ingroup reportdesign_api
     *
     */
    class OGroups : public cppu::BaseMutex,
                    public GroupsBase
    {
        typedef ::std::vector< css::uno::Reference< css::report::XGroup > > TGroups;
        ::comphelper::OInterfaceContainerHelper3<css::container::XContainerListener> m_aContainerListeners;
        css::uno::Reference< css::uno::XComponentContext >            m_xContext;
        css::uno::WeakReference< css::report::XReportDefinition >     m_xParent;
        TGroups                                                       m_aGroups;
    private:
        OGroups& operator=(const OGroups&) = delete;
        OGroups(const OGroups&) = delete;
        void checkIndex(sal_Int32 _nIndex);
    protected:
        // TODO: VirtualFunctionFinder: This is virtual function!

        virtual ~OGroups() override;

        /** this function is called upon disposing the component
        */
        // TODO: VirtualFunctionFinder: This is virtual function!

        virtual void SAL_CALL disposing() override;
    public:
        OGroups( const css::uno::Reference< css::report::XReportDefinition >& _xParent
                ,css::uno::Reference< css::uno::XComponentContext > context);

    // XGroups
        // Attributes
        virtual css::uno::Reference< css::report::XReportDefinition > SAL_CALL getReportDefinition() override;
        // Methods
        virtual css::uno::Reference< css::report::XGroup > SAL_CALL createGroup(  ) override;
    // XIndexContainer
        virtual void SAL_CALL insertByIndex( ::sal_Int32 Index, const css::uno::Any& Element ) override;
        virtual void SAL_CALL removeByIndex( ::sal_Int32 Index ) override;
    // XIndexReplace
        virtual void SAL_CALL replaceByIndex( ::sal_Int32 Index, const css::uno::Any& Element ) override;
    // XIndexAccess
        virtual ::sal_Int32 SAL_CALL getCount(  ) override;
        virtual css::uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) override;
    // XElementAccess
        virtual css::uno::Type SAL_CALL getElementType(  ) override;
        virtual sal_Bool SAL_CALL hasElements(  ) override;
    // XChild
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getParent(  ) override;
        virtual void SAL_CALL setParent( const css::uno::Reference< css::uno::XInterface >& Parent ) override;
    // XContainer
        virtual void SAL_CALL addContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) override;
        virtual void SAL_CALL removeContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) override;

        // XComponent
        virtual void SAL_CALL dispose() override;
        virtual void SAL_CALL addEventListener(const css::uno::Reference< css::lang::XEventListener > & aListener) override
        {
            cppu::WeakComponentImplHelperBase::addEventListener(aListener);
        }
        virtual void SAL_CALL removeEventListener(const css::uno::Reference< css::lang::XEventListener > & aListener) override
        {
            cppu::WeakComponentImplHelperBase::removeEventListener(aListener);
        }
    };
}
#endif // INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_GROUPS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
