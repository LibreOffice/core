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
#ifndef INCLUDED_VBAHELPER_VBADOCUMENTSBASE_HXX
#define INCLUDED_VBAHELPER_VBADOCUMENTSBASE_HXX

#include <exception>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Type.hxx>
#include <ooo/vba/XDocumentsBase.hpp>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <vbahelper/vbacollectionimpl.hxx>
#include <vbahelper/vbadllapi.h>
#include <vbahelper/vbahelper.hxx>

namespace com { namespace sun { namespace star {
    namespace beans { struct PropertyValue; }
    namespace container { class XEnumeration; }
    namespace uno { class XComponentContext; }
} } }

namespace ooo { namespace vba {
    class XDocumentsBase;
    class XHelperInterface;
} }

typedef CollTestImplHelper< ooo::vba::XDocumentsBase > VbaDocumentsBase_BASE;

class VBAHELPER_DLLPUBLIC VbaDocumentsBase : public VbaDocumentsBase_BASE
{
public:
    enum DOCUMENT_TYPE
    {
        WORD_DOCUMENT = 1,
        EXCEL_DOCUMENT
    };

private:
    DOCUMENT_TYPE meDocType;

public:
    /// @throws css::uno::RuntimeException
    VbaDocumentsBase( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, DOCUMENT_TYPE eDocType );

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() override = 0;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() override = 0;

    // VbaDocumentsBase_BASE
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ) override = 0;

protected:
    /// @throws css::uno::RuntimeException
    css::uno::Any createDocument();
    /// @throws css::uno::RuntimeException
    css::uno::Any openDocument( const OUString& Filename, const css::uno::Any& ReadOnly, const css::uno::Sequence< css::beans::PropertyValue >& rProps );
};

#endif /* SC_ INCLUDED_VBAHELPER_VBADOCUMENTSBASE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
