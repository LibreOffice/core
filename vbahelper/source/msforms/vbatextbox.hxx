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


#ifndef SC_VBA_TEXTBOX_HXX
#define SC_VBA_TEXTBOX_HXX
#include <cppuhelper/implbase2.hxx>
#include <ooo/vba/msforms/XTextBox.hpp>
#include "vbacontrol.hxx"
#include <vbahelper/vbahelper.hxx>

typedef cppu::ImplInheritanceHelper2< ScVbaControl, ov::msforms::XTextBox, css::script::XDefaultProperty > TextBoxImpl_BASE;

class ScVbaTextBox : public TextBoxImpl_BASE
{
    bool mbDialog;
public:
    ScVbaTextBox( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::uno::XInterface >& xControl, const css::uno::Reference< css::frame::XModel >& xModel, ov::AbstractGeometryAttributes* pGeomHelper, bool bDialog = false );
   // Attributes
    virtual css::uno::Any SAL_CALL getValue() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setValue( const css::uno::Any& _value ) throw (css::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL getText() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setText( const rtl::OUString& _text ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getMaxLength() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setMaxLength( sal_Int32 _maxlength ) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getMultiline() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setMultiline( sal_Bool _multiline ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getSpecialEffect() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setSpecialEffect( sal_Int32 nSpecialEffect ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getBorderStyle() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setBorderStyle( sal_Int32 nBorderStyle ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getTextLength() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::msforms::XNewFont > SAL_CALL getFont() throw (css::uno::RuntimeException);
    // XDefaultProperty
    rtl::OUString SAL_CALL getDefaultPropertyName(  ) throw (css::uno::RuntimeException) { return ::rtl::OUString::createFromAscii("Value"); }
    //XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif //SC_VBA_TEXTBOX_HXX
