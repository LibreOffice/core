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
#ifndef INCLUDED_VBAHELPER_VBADOCUMENTBASE_HXX
#define INCLUDED_VBAHELPER_VBADOCUMENTBASE_HXX

#include <com/sun/star/frame/XModel.hpp>
#include <ooo/vba/XDocumentBase.hpp>
#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceWeakImpl< ooo::vba::XDocumentBase > VbaDocumentBase_BASE;

class VBAHELPER_DLLPUBLIC VbaDocumentBase : public VbaDocumentBase_BASE
{
protected:
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::uno::XInterface > mxVBProject;
protected:
    css::uno::Reference< css::frame::XModel >  getModel() { return mxModel; }
public:
    VbaDocumentBase(    const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext,
            css::uno::Reference< css::frame::XModel > xModel );
    VbaDocumentBase(    css::uno::Sequence< css::uno::Any > const& aArgs, css::uno::Reference< css::uno::XComponentContext >const& xContext );
    virtual ~VbaDocumentBase() {}

    // Attributes
    virtual OUString SAL_CALL getName() throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getPath() throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getFullName() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getSaved() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setSaved( sal_Bool bSave ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getVBProject() throw (css::uno::RuntimeException, std::exception) override;

    // Methods
    virtual void SAL_CALL Close( const css::uno::Any &bSaveChanges,
                                 const css::uno::Any &aFileName,
                                 const css::uno::Any &bRouteWorkbook ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL Protect( const css::uno::Any &aPassword ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Unprotect( const css::uno::Any &aPassword ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL Save() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL Activate() throw (css::uno::RuntimeException, std::exception) override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;

    static OUString getNameFromModel( const css::uno::Reference< css::frame::XModel >& xModel );
};

#endif // INCLUDED_VBAHELPER_VBADOCUMENTBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
