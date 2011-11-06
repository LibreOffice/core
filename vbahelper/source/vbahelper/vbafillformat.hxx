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


#ifndef SC_VBA_XFILLFORMAT_HXX
#define SC_VBA_XFILLFORMAT_HXX

#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <ooo/vba/msforms/XFillFormat.hpp>
#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceImpl1< ov::msforms::XFillFormat > ScVbaFillFormat_BASE;

class ScVbaFillFormat : public ScVbaFillFormat_BASE
{
private:
    css::uno::Reference< css::drawing::XShape > m_xShape;
    css::uno::Reference< css::beans::XPropertySet > m_xPropertySet;
    css::uno::Reference< ov::msforms::XColorFormat > m_xColorFormat;
    css::drawing::FillStyle m_nFillStyle;
    sal_Int32 m_nForeColor;
    sal_Int32 m_nBackColor;
    sal_Int16 m_nGradientAngle;
private:
    void setFillStyle( css::drawing::FillStyle nFillStyle ) throw (css::uno::RuntimeException);
protected:
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();

public:
    ScVbaFillFormat( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::drawing::XShape > xShape );

    void setForeColorAndInternalStyle( sal_Int32 nForeColor ) throw (css::uno::RuntimeException);
    // Attributes
    virtual sal_Bool SAL_CALL getVisible() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setVisible( sal_Bool _visible ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getTransparency() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setTransparency( double _transparency ) throw (css::uno::RuntimeException);

    // Methods
    virtual void SAL_CALL Solid() throw (css::uno::RuntimeException);
    virtual void SAL_CALL TwoColorGradient( sal_Int32 style, sal_Int32 variant ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::msforms::XColorFormat > SAL_CALL BackColor() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::msforms::XColorFormat > SAL_CALL ForeColor() throw (css::uno::RuntimeException);

};

#endif//SC_VBA_XFILLFORMAT_HXX
