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

#ifndef VBA_FONT_HXX
#define VBA_FONT_HXX

#include <vbahelper/vbafontbase.hxx>
#include <ooo/vba/word/XFont.hpp>

typedef cppu::ImplInheritanceHelper1< VbaFontBase, ov::word::XFont > SwVbaFont_BASE;

class SwVbaFont : public SwVbaFont_BASE
{
public:
    SwVbaFont( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::container::XIndexAccess >& xPalette, css::uno::Reference< css::beans::XPropertySet > xPropertySet ) throw ( css::uno::RuntimeException );
    virtual ~SwVbaFont(){}

    // Attributes
    virtual void SAL_CALL setColorIndex( const css::uno::Any& _colorindex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getColorIndex() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getUnderline() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setUnderline( const css::uno::Any& _underline ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getSubscript() throw ( css::uno::RuntimeException );
    virtual css::uno::Any SAL_CALL getSuperscript() throw ( css::uno::RuntimeException );

    virtual css::uno::Any SAL_CALL getBold() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getItalic() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getStrikethrough() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getShadow() throw (css::uno::RuntimeException);
    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif
