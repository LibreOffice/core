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
#pragma once

#include <ooo/vba/excel/XName.hpp>

#include <vbahelper/vbahelperinterface.hxx>
#include <formula/grammar.hxx>

namespace com::sun::star::sheet { class XNamedRange; }
namespace com::sun::star::sheet { class XNamedRanges; }

typedef InheritedHelperInterfaceWeakImpl< ov::excel::XName > NameImpl_BASE;

class ScVbaName : public NameImpl_BASE
{
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::sheet::XNamedRange > mxNamedRange;
    css::uno::Reference< css::sheet::XNamedRanges > mxNames;
    OUString getContent( const formula::FormulaGrammar::Grammar eGrammar );
    void setContent( const OUString& sContent, const formula::FormulaGrammar::Grammar eGrammar );
public:
    ScVbaName( const css::uno::Reference< ov::XHelperInterface >& xParent,  const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::sheet::XNamedRange >& xName , const css::uno::Reference< css::sheet::XNamedRanges >& xNames , const css::uno::Reference< css::frame::XModel >& xModel );
    virtual ~ScVbaName() override;

    // Attributes
    virtual OUString SAL_CALL getName() override;
    virtual void SAL_CALL setName( const OUString &rName ) override;
    virtual OUString SAL_CALL getNameLocal() override;
    virtual void SAL_CALL setNameLocal( const OUString &rName ) override;
    virtual sal_Bool SAL_CALL getVisible() override;
    virtual void SAL_CALL setVisible( sal_Bool bVisible ) override;
    virtual OUString SAL_CALL getValue() override;
    virtual void SAL_CALL setValue( const OUString &rValue ) override;
    virtual OUString SAL_CALL getRefersTo() override;
    virtual void SAL_CALL setRefersTo( const OUString &rRefersTo ) override;
    virtual OUString SAL_CALL getRefersToLocal() override;
    virtual void SAL_CALL setRefersToLocal( const OUString &rRefersTo ) override;
    virtual OUString SAL_CALL getRefersToR1C1() override;
    virtual void SAL_CALL setRefersToR1C1( const OUString &rRefersTo ) override;
    virtual OUString SAL_CALL getRefersToR1C1Local() override;
    virtual void SAL_CALL setRefersToR1C1Local( const OUString &rRefersTo ) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL getRefersToRange() override;

    // Methods
    virtual void SAL_CALL Delete() override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
