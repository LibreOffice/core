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
#ifndef INCLUDED_SC_SOURCE_UI_VBA_VBACHARACTERS_HXX
#define INCLUDED_SC_SOURCE_UI_VBA_VBACHARACTERS_HXX

#include <ooo/vba/excel/XCharacters.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/text/XSimpleText.hpp>

#include <vbahelper/vbahelperinterface.hxx>
#include "vbapalette.hxx"
typedef InheritedHelperInterfaceWeakImpl< ov::excel::XCharacters > ScVbaCharacters_BASE;

class ScVbaCharacters : public ScVbaCharacters_BASE
{
private:
    css::uno::Reference< css::text::XTextRange > m_xTextRange;
    css::uno::Reference< css::text::XSimpleText > m_xSimpleText;
    ScVbaPalette m_aPalette;
    sal_Int16 nLength;
    sal_Int16 nStart;
    // Add because of MSO has different behavior.
    bool bReplace;
public:
    ScVbaCharacters( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext,  const ScVbaPalette& dPalette, const css::uno::Reference< css::text::XSimpleText >& xRange, const css::uno::Any& Start, const css::uno::Any& Length, bool bReplace = false  ) throw ( css::lang::IllegalArgumentException, css::uno::RuntimeException );

    virtual ~ScVbaCharacters() {}
    // Attributes
    virtual OUString SAL_CALL getCaption() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setCaption( const OUString& _caption ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::sal_Int32 SAL_CALL getCount() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getText() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setText( const OUString& _text ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Reference< ov::excel::XFont > SAL_CALL getFont() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setFont( const css::uno::Reference< ov::excel::XFont >& _font ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Methods
    virtual void SAL_CALL Insert( const OUString& String ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL Delete(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XHelperInterface
    virtual OUString getServiceImplName() SAL_OVERRIDE;
    virtual css::uno::Sequence<OUString> getServiceNames() SAL_OVERRIDE;

};

#endif /* SC_VBA_CHARACTER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
