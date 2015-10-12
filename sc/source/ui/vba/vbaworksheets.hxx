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
#ifndef INCLUDED_SC_SOURCE_UI_VBA_VBAWORKSHEETS_HXX
#define INCLUDED_SC_SOURCE_UI_VBA_VBAWORKSHEETS_HXX

#include <ooo/vba/excel/XWorksheets.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <vbahelper/vbacollectionimpl.hxx>

#include "address.hxx"

typedef CollTestImplHelper< ov::excel::XWorksheets > ScVbaWorksheets_BASE;

class ScVbaWorksheets : public ScVbaWorksheets_BASE
{
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::sheet::XSpreadsheets > m_xSheets;
protected:
    // ScVbaWorksheets_BASE
    virtual css::uno::Any getItemByStringIndex( const OUString& sIndex ) throw (css::uno::RuntimeException) override;
public:
    ScVbaWorksheets( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::container::XIndexAccess >& xSheets, const css::uno::Reference< css::frame::XModel >& xModel );
    ScVbaWorksheets( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::container::XEnumerationAccess >& xEnum,  const css::uno::Reference< css::frame::XModel >& xModel );
    virtual ~ScVbaWorksheets() {}

    bool isSelectedSheets();

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException) override;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException) override;

    // XWorksheets
    virtual css::uno::Any SAL_CALL getVisible() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setVisible( const css::uno::Any& _visible ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL Add( const css::uno::Any& Before, const css::uno::Any& After, const css::uno::Any& Count, const css::uno::Any& Type ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL Delete(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL PrintOut( const css::uno::Any& From, const css::uno::Any& To, const css::uno::Any& Copies, const css::uno::Any& Preview, const css::uno::Any& ActivePrinter, const css::uno::Any& PrintToFile, const css::uno::Any& Collate, const css::uno::Any& PrToFileName ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ) override;
    virtual void SAL_CALL Select( const css::uno::Any& Replace ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL Copy ( const css::uno::Any& Before, const css::uno::Any& After) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL PrintPreview( const css::uno::Any& EnableChanges ) throw (css::uno::RuntimeException, std::exception) override;
    // ScVbaWorksheets_BASE
    virtual css::uno::Any SAL_CALL Item( const css::uno::Any& Index1, const css::uno::Any& Index2 )
        throw (css::lang::IndexOutOfBoundsException, css::script::BasicErrorException, css::uno::RuntimeException) override;
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;

    static bool nameExists( css::uno::Reference <css::sheet::XSpreadsheetDocument>& xSpreadDoc, const OUString & name, SCTAB& nTab ) throw ( css::lang::IllegalArgumentException, css::uno::RuntimeException );
};

#endif // INCLUDED_SC_SOURCE_UI_VBA_VBAWORKSHEETS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
