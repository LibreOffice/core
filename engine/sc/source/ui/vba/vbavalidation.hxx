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

#include <ooo/vba/excel/XValidation.hpp>
#include <utility>
#include <vbahelper/vbahelperinterface.hxx>

namespace com::sun::star::table { class XCellRange; }
namespace cpo::uno { class XComponentContext; }

typedef InheritedHelperInterfaceWeakImpl<ov::excel::XValidation > ValidationImpl_BASE;

class ScVbaValidation : public ValidationImpl_BASE
{
    cpo::uno::Reference< css::table::XCellRange > m_xRange;

public:
    ScVbaValidation( const cpo::uno::Reference< ov::XHelperInterface >& xParent, const cpo::uno::Reference< cpo::uno::XComponentContext >& xContext, cpo::uno::Reference< css::table::XCellRange > xRange ) : ValidationImpl_BASE( xParent, xContext ), m_xRange(std::move( xRange)) {}
    // Attributes
    virtual bool getIgnoreBlank() override;
    virtual void setIgnoreBlank( bool _ignoreblank ) override;
    virtual bool getCaseSensitive() override;
    virtual void setCaseSensitive( bool _bCase ) override;
    virtual bool getInCellDropdown() override;
    virtual void setInCellDropdown( bool _incelldropdown ) override;
    virtual bool getShowInput() override;
    virtual void setShowInput( bool _showinput ) override;
    virtual bool getShowError() override;
    virtual void setShowError( bool _showerror ) override;
    virtual OUString getInputTitle() override;
    virtual void setInputTitle( const OUString& _inputtitle ) override;
    virtual OUString getErrorTitle() override;
    virtual void setErrorTitle( const OUString& _errortitle ) override;
    virtual OUString getInputMessage() override;
    virtual void setInputMessage( const OUString& _inputmessage ) override;
    virtual OUString getErrorMessage() override;
    virtual void setErrorMessage( const OUString& _errormessage ) override;
    virtual OUString getFormula1() override ;
    virtual OUString getFormula2() override;
    virtual sal_Int32 getType() override;
    // Methods
    virtual void Delete(  ) override;
    virtual void Add( const cpo::uno::Any& Type, const cpo::uno::Any& AlertStyle, const cpo::uno::Any& Operator, const cpo::uno::Any& Formula1, const cpo::uno::Any& Formula2 ) override;
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual cpo::uno::Sequence<OUString> getServiceNames() override;

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
