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


#ifndef SC_VBA_PICTUREFORMAT_HXX
#define SC_VBA_PICTUREFORMAT_HXX

#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <ooo/vba/msforms/XPictureFormat.hpp>
#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceImpl1< ov::msforms::XPictureFormat > ScVbaPictureFormat_BASE;

class ScVbaPictureFormat : public ScVbaPictureFormat_BASE
{
private:
    css::uno::Reference< css::drawing::XShape > m_xShape;
    css::uno::Reference< css::beans::XPropertySet > m_xPropertySet;
protected:
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
private:
    void checkParameterRangeInDouble( double nRange, double nMin, double nMax ) throw (css::uno::RuntimeException);
public:
    ScVbaPictureFormat( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, css::uno::Reference< css::drawing::XShape > xShape ) throw( css::lang::IllegalArgumentException );

    // Attributes
    virtual double SAL_CALL getBrightness() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setBrightness( double _brightness ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getContrast() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setContrast( double _contrast ) throw (css::uno::RuntimeException);

    // Methods
    virtual void SAL_CALL IncrementBrightness( double increment ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL IncrementContrast( double increment ) throw (css::uno::RuntimeException);
};

#endif//SC_VBA_PICTUREFORMAT_HXX
