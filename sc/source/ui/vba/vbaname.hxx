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
#ifndef INCLUDED_SC_SOURCE_UI_VBA_VBANAME_HXX
#define INCLUDED_SC_SOURCE_UI_VBA_VBANAME_HXX

#include <ooo/vba/excel/XName.hpp>
#include <com/sun/star/sheet/XNamedRange.hpp>
#include <com/sun/star/sheet/XNamedRanges.hpp>

#include <vbahelper/vbahelperinterface.hxx>
#include <formula/grammar.hxx>
class ScDocument;

typedef InheritedHelperInterfaceWeakImpl< ov::excel::XName > NameImpl_BASE;

class ScVbaName : public NameImpl_BASE
{
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::sheet::XNamedRange > mxNamedRange;
    css::uno::Reference< css::sheet::XNamedRanges > mxNames;
    OUString getContent( const formula::FormulaGrammar::Grammar eGrammar, bool prependEquals = true );
    void setContent( const OUString& sContent, const formula::FormulaGrammar::Grammar eGrammar, bool removeEquals = true );
public:
    ScVbaName( const css::uno::Reference< ov::XHelperInterface >& xParent,  const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::sheet::XNamedRange >& xName , const css::uno::Reference< css::sheet::XNamedRanges >& xNames , const css::uno::Reference< css::frame::XModel >& xModel );
    virtual ~ScVbaName();

    // Attributes
    virtual OUString SAL_CALL getName() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setName( const OUString &rName ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getNameLocal() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setNameLocal( const OUString &rName ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL getVisible() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setVisible( sal_Bool bVisible ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getValue() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setValue( const OUString &rValue ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getRefersTo() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setRefersTo( const OUString &rRefersTo ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getRefersToLocal() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setRefersToLocal( const OUString &rRefersTo ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getRefersToR1C1() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setRefersToR1C1( const OUString &rRefersTo ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getRefersToR1C1Local() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setRefersToR1C1Local( const OUString &rRefersTo ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL getRefersToRange() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Methods
    virtual void SAL_CALL Delete() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XHelperInterface
    virtual OUString getServiceImplName() SAL_OVERRIDE;
    virtual css::uno::Sequence<OUString> getServiceNames() SAL_OVERRIDE;
};
#endif // INCLUDED_SC_SOURCE_UI_VBA_VBANAME_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
