/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
    // Add because of MSO has different behavior.
    sal_Bool bReplace;
public:
    ScVbaCharacters( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext,  const ScVbaPalette& dPalette, const css::uno::Reference< css::text::XSimpleText >& xRange, const css::uno::Any& Start, const css::uno::Any& Length, sal_Bool bReplace = sal_False  ) throw ( css::lang::IllegalArgumentException );

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

