/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
    cpo::uno::Reference< css::frame::XModel > mxModel;
    cpo::uno::Reference< css::sheet::XNamedRange > mxNamedRange;
    cpo::uno::Reference< css::sheet::XNamedRanges > mxNames;
    OUString getContent( const formula::FormulaGrammar::Grammar eGrammar );
    void setContent( const OUString& sContent, const formula::FormulaGrammar::Grammar eGrammar );
public:
    ScVbaName( const cpo::uno::Reference< ov::XHelperInterface >& xParent,  const cpo::uno::Reference< cpo::uno::XComponentContext >& xContext, cpo::uno::Reference< css::sheet::XNamedRange > xName , cpo::uno::Reference< css::sheet::XNamedRanges > xNames , cpo::uno::Reference< css::frame::XModel > xModel );
    virtual ~ScVbaName() override;

    // Attributes
    virtual OUString getName() override;
    virtual void setName( const OUString &rName ) override;
    virtual OUString getNameLocal() override;
    virtual void setNameLocal( const OUString &rName ) override;
    virtual bool getVisible() override;
    virtual void setVisible( bool bVisible ) override;
    virtual OUString getValue() override;
    virtual void setValue( const OUString &rValue ) override;
    virtual OUString getRefersTo() override;
    virtual void setRefersTo( const OUString &rRefersTo ) override;
    virtual OUString getRefersToLocal() override;
    virtual void setRefersToLocal( const OUString &rRefersTo ) override;
    virtual OUString getRefersToR1C1() override;
    virtual void setRefersToR1C1( const OUString &rRefersTo ) override;
    virtual OUString getRefersToR1C1Local() override;
    virtual void setRefersToR1C1Local( const OUString &rRefersTo ) override;
    virtual cpo::uno::Reference< ov::excel::XRange > getRefersToRange() override;

    // Methods
    virtual void Delete() override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual cpo::uno::Sequence<OUString> getServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
