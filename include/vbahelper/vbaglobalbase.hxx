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
#ifndef INCLUDED_VBAHELPER_VBAGLOBALBASE_HXX
#define INCLUDED_VBAHELPER_VBAGLOBALBASE_HXX

#include <exception>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <ooo/vba/XGlobalsBase.hpp>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <vbahelper/vbadllapi.h>
#include <vbahelper/vbahelper.hxx>
#include <vbahelper/vbahelperinterface.hxx>

namespace com { namespace sun { namespace star {
    namespace beans { struct PropertyValue; }
    namespace uno { class XComponentContext; }
    namespace uno { class XInterface; }
} } }

namespace ooo { namespace vba {
    class XHelperInterface;
} }

typedef InheritedHelperInterfaceWeakImpl< ov::XGlobalsBase > Globals_BASE;
class VBAHELPER_DLLPUBLIC VbaGlobalsBase : public Globals_BASE
{
    const OUString msDocCtxName;
protected:
    bool hasServiceName( const OUString& serviceName );
    void init(  const css::uno::Sequence< css::beans::PropertyValue >& aInitArgs );

public:
    VbaGlobalsBase( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const OUString& sDocCtxName );
    virtual ~VbaGlobalsBase() override;
    // XMultiServiceFactory
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance( const OUString& aServiceSpecifier ) override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArguments( const OUString& ServiceSpecifier, const css::uno::Sequence< css::uno::Any >& Arguments ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames(  ) override;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
