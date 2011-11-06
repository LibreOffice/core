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


#ifndef SC_VBA_FRAME_HXX
#define SC_VBA_FRAME_HXX
#include <cppuhelper/implbase1.hxx>
#include <ooo/vba/msforms/XFrame.hpp>

#include "vbacontrol.hxx"
#include <vbahelper/vbahelper.hxx>

typedef cppu::ImplInheritanceHelper1< ScVbaControl, ov::msforms::XFrame > FrameImpl_BASE;

class ScVbaFrame : public FrameImpl_BASE
{
public:
    ScVbaFrame(
        const css::uno::Reference< ov::XHelperInterface >& xParent,
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const css::uno::Reference< css::uno::XInterface >& xControl,
        const css::uno::Reference< css::frame::XModel >& xModel,
        ov::AbstractGeometryAttributes* pGeomHelper,
        const css::uno::Reference< css::awt::XControl >& xDialog );

    // XFrame attributes
    virtual rtl::OUString SAL_CALL getCaption() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setCaption( const rtl::OUString& _caption ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getSpecialEffect() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setSpecialEffect( sal_Int32 nSpecialEffect ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getBorderStyle() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setBorderStyle( sal_Int32 nBorderStyle ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::msforms::XNewFont > SAL_CALL getFont() throw (css::uno::RuntimeException);
    // XFrame methods
    css::uno::Any SAL_CALL Controls( const css::uno::Any& rIndex ) throw (css::uno::RuntimeException);
    //XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();

private:
    css::uno::Reference< css::awt::XControl > mxDialog;
};
#endif //SC_VBA_LABEL_HXX
