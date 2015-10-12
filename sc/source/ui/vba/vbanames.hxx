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
#ifndef INCLUDED_SC_SOURCE_UI_VBA_VBANAMES_HXX
#define INCLUDED_SC_SOURCE_UI_VBA_VBANAMES_HXX

#include <ooo/vba/excel/XNames.hpp>
#include <ooo/vba/XCollection.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/sheet/XNamedRanges.hpp>
#include <vbahelper/vbacollectionimpl.hxx>

class ScDocument;
class ScDocShell;

typedef CollTestImplHelper< ov::excel::XNames > ScVbaNames_BASE;

class ScVbaNames : public ScVbaNames_BASE
{
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::sheet::XNamedRanges > mxNames;

protected:
    css::uno::Reference< css::frame::XModel >  getModel() { return mxModel; }

public:
    ScVbaNames( const css::uno::Reference< ov::XHelperInterface >& xParent,  const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::sheet::XNamedRanges >& xNames , const css::uno::Reference< css::frame::XModel >& xModel );

    ScDocument* getScDocument();

    virtual ~ScVbaNames();

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException) override;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException) override;

    // Methods
    virtual css::uno::Any SAL_CALL Add( const css::uno::Any& aName ,
                    const css::uno::Any& aRefersTo,
                    const css::uno::Any& aVisible,
                    const css::uno::Any& aMacroType,
                    const css::uno::Any& aShoutcutKey,
                    const css::uno::Any& aCategory,
                    const css::uno::Any& aNameLocal,
                    const css::uno::Any& aRefersToLocal,
                    const css::uno::Any& aCategoryLocal,
                    const css::uno::Any& aRefersToR1C1,
                    const css::uno::Any& aRefersToR1C1Local )
        throw (css::uno::RuntimeException,
               std::exception) override;

    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ) override;

    // ScVbaNames_BASE
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;

};
#endif // INCLUDED_SC_SOURCE_UI_VBA_VBANAMES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
