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

#ifndef INCLUDED_EDITENG_UNONRULE_HXX
#define INCLUDED_EDITENG_UNONRULE_HXX

#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/ucb/XAnyCompare.hpp>
#include <editeng/editengdllapi.h>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <editeng/numitem.hxx>
#include <comphelper/servicehelper.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

EDITENG_DLLPUBLIC css::uno::Reference< css::container::XIndexReplace > SvxCreateNumRule( const SvxNumRule* pRule ) throw();
EDITENG_DLLPUBLIC css::uno::Reference< css::container::XIndexReplace > SvxCreateNumRule() throw();
const SvxNumRule& SvxGetNumRule( css::uno::Reference< css::container::XIndexReplace > const & xRule ) throw( css::lang::IllegalArgumentException );
EDITENG_DLLPUBLIC css::uno::Reference< css::ucb::XAnyCompare > SvxCreateNumRuleCompare() throw();

class SvxUnoNumberingRules : public ::cppu::WeakAggImplHelper < css::container::XIndexReplace, css::ucb::XAnyCompare,
    css::lang::XUnoTunnel, css::util::XCloneable, css::lang::XServiceInfo >
{
private:
    SvxNumRule maRule;
public:
    SvxUnoNumberingRules( const SvxNumRule& rRule ) throw();
    virtual ~SvxUnoNumberingRules() throw() override;

    UNO3_GETIMPLEMENTATION_DECL( SvxUnoNumberingRules )

    //XIndexReplace
    virtual void SAL_CALL replaceByIndex( sal_Int32 Index, const css::uno::Any& Element ) throw(
    css::lang::IllegalArgumentException, css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException, std::exception) override ;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(css::lang::IndexOutOfBoundsException,
        css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    //XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

    // XAnyCompare
    virtual sal_Int16 SAL_CALL compare( const css::uno::Any& Any1, const css::uno::Any& Any2 ) throw(css::uno::RuntimeException, std::exception) override;

    // XCloneable
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(css::uno::RuntimeException, std::exception) override;

    // internal
    css::uno::Sequence<css::beans::PropertyValue> getNumberingRuleByIndex( sal_Int32 nIndex) const
        throw (css::uno::RuntimeException, std::exception);
    void setNumberingRuleByIndex(const css::uno::Sequence<css::beans::PropertyValue>& rProperties, sal_Int32 nIndex)
        throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, std::exception);

    static sal_Int16 Compare( const css::uno::Any& rAny1, const css::uno::Any& rAny2 );

    const SvxNumRule& getNumRule() const { return maRule; }
};


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
