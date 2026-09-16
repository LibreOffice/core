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

#include <ooo/vba/excel/XCharacters.hpp>
#include <cpo/uno/XComponentContext.hpp>
#include <com/sun/star/text/XSimpleText.hpp>

#include <vbahelper/vbahelperinterface.hxx>
#include "vbapalette.hxx"
typedef InheritedHelperInterfaceWeakImpl< ov::excel::XCharacters > ScVbaCharacters_BASE;

class ScVbaCharacters : public ScVbaCharacters_BASE
{
private:
    cpo::uno::Reference< css::text::XTextRange > m_xTextRange;
    cpo::uno::Reference< css::text::XSimpleText > m_xSimpleText;
    ScVbaPalette m_aPalette;
    // Add because of MSO has different behavior.
    bool bReplace;
public:
    /// @throws css::lang::IllegalArgumentException
    /// @throws cpo::uno::RuntimeException
    ScVbaCharacters( const cpo::uno::Reference< ov::XHelperInterface >& xParent, const cpo::uno::Reference< cpo::uno::XComponentContext >& xContext,  const ScVbaPalette& dPalette, cpo::uno::Reference< css::text::XSimpleText > xRange, const cpo::uno::Any& Start, const cpo::uno::Any& Length, bool bReplace = false  );

    // Attributes
    virtual OUString getCaption() override;
    virtual void setCaption( const OUString& _caption ) override;
    virtual ::sal_Int32 getCount() override;
    virtual OUString getText() override;
    virtual void setText( const OUString& _text ) override;
    virtual cpo::uno::Reference< ov::excel::XFont > getFont() override;
    virtual void setFont( const cpo::uno::Reference< ov::excel::XFont >& _font ) override;

    // Methods
    virtual void Insert( const OUString& String ) override;
    virtual void Delete(  ) override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual cpo::uno::Sequence<OUString> getServiceNames() override;

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
