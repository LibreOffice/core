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
#ifndef SC_VBA_CHARACTERS_HXX
#define SC_VBA_CHARACTERS_HXX

#include <cppuhelper/implbase1.hxx>

#include <ooo/vba/excel/XCharacters.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/text/XSimpleText.hpp>

#include <vbahelper/vbahelperinterface.hxx>
#include "vbapalette.hxx"
typedef InheritedHelperInterfaceImpl1< ov::excel::XCharacters > ScVbaCharacters_BASE;

class ScVbaCharacters : public ScVbaCharacters_BASE
{
private:
    css::uno::Reference< css::text::XTextRange > m_xTextRange;
    css::uno::Reference< css::text::XSimpleText > m_xSimpleText;
    ScVbaPalette m_aPalette;
    sal_Int16 nLength;
    sal_Int16 nStart;
    // Add becuase of MSO has diferent behavior.
    sal_Bool bReplace;
public:
    ScVbaCharacters( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext,  const ScVbaPalette& dPalette, const css::uno::Reference< css::text::XSimpleText >& xRange, const css::uno::Any& Start, const css::uno::Any& Length, sal_Bool bReplace = false  ) throw ( css::lang::IllegalArgumentException );

    virtual ~ScVbaCharacters() {}
    // Attributes
    virtual ::rtl::OUString SAL_CALL getCaption() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setCaption( const ::rtl::OUString& _caption ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getCount() throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getText() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setText( const ::rtl::OUString& _text ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::excel::XFont > SAL_CALL getFont() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setFont( const css::uno::Reference< ov::excel::XFont >& _font ) throw (css::uno::RuntimeException);

    // Methods
    virtual void SAL_CALL Insert( const ::rtl::OUString& String ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Delete(  ) throw (css::uno::RuntimeException);


    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();

};

#endif /* SC_VBA_CHARACTER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
