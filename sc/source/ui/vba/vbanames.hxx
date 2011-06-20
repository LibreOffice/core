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
#ifndef SC_VBA_NAMES_HXX
#define SC_VBA_NAMES_HXX

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
    virtual css::uno::Reference< css::frame::XModel >  getModel() { return mxModel; }

public:
    ScVbaNames( const css::uno::Reference< ov::XHelperInterface >& xParent,  const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::sheet::XNamedRanges >& xNames , const css::uno::Reference< css::frame::XModel >& xModel );

    ScDocument* getScDocument();
    ScDocShell* getScDocShell();

    virtual ~ScVbaNames();

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);

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
                    const css::uno::Any& aRefersToR1C1Local ) throw (css::uno::RuntimeException);

    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource );

    // ScVbaNames_BASE
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();

};
#endif /* SC_VBA_NAMES_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
