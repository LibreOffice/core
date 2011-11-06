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


#ifndef SC_VBA_CONTROL_HXX
#define SC_VBA_CONTROL_HXX

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/script/XDefaultProperty.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <ooo/vba/msforms/XControl.hpp>

#include <vbahelper/vbahelper.hxx>
#include <vbahelper/vbahelperinterface.hxx>
#include <memory>

//typedef ::cppu::WeakImplHelper1< ov::msforms::XControl > ControlImpl_BASE;
//template SAL_DLLPUBLIC_IMPORT InheritedHelperInterfaceImpl1< ov::msforms::XControl >;
typedef InheritedHelperInterfaceImpl1< ov::msforms::XControl > ControlImpl_BASE;

class ScVbaControl : public ControlImpl_BASE
{
private:
    com::sun::star::uno::Reference< com::sun::star::lang::XEventListener > m_xEventListener;
protected:
    // awt control has nothing similar to Tag property of Mso controls,
    // whether it is necessary is another question
    ::rtl::OUString m_aControlTag;

    std::auto_ptr< ov::AbstractGeometryAttributes > mpGeometryHelper;
    css::uno::Reference< css::beans::XPropertySet > m_xProps;
    css::uno::Reference< css::uno::XInterface > m_xControl;
    css::uno::Reference< css::frame::XModel > m_xModel;

    virtual css::uno::Reference< css::awt::XWindowPeer > getWindowPeer() throw (css::uno::RuntimeException);
public:
    ScVbaControl( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext,
                    const css::uno::Reference< css::uno::XInterface >& xControl, const css::uno::Reference< css::frame::XModel >& xModel, ov::AbstractGeometryAttributes* pHelper );
    virtual ~ScVbaControl();
    // This class will own the helper, so make sure it is allocated from
    // the heap
    void setGeometryHelper( ov::AbstractGeometryAttributes* pHelper );
    // XControl
    virtual sal_Bool SAL_CALL getEnabled() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setEnabled( sal_Bool _enabled ) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getVisible() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setVisible( sal_Bool _visible ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getHeight() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setHeight( double _height ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getWidth() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setWidth( double _width ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getLeft() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setLeft( double _left ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getTop() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setTop( double _top ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL SetFocus(  ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Move( double Left, double Top, const ::com::sun::star::uno::Any& Width, const ::com::sun::star::uno::Any& Height ) throw (::com::sun::star::uno::RuntimeException);

    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getObject() throw (css::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL getControlSource() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setControlSource( const rtl::OUString& _controlsource ) throw (css::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL getRowSource() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setRowSource( const rtl::OUString& _rowsource ) throw (css::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setName( const rtl::OUString& _name ) throw (css::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL getControlTipText() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setControlTipText( const rtl::OUString& ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTag() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setTag( const ::rtl::OUString& aTag ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getTabIndex() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setTabIndex( sal_Int32 nTabIndex ) throw (css::uno::RuntimeException);
    //remove resouce because ooo.vba.excel.XControl is a wrapper of com.sun.star.drawing.XControlShape
    virtual void removeResouce() throw( css::uno::RuntimeException );
    //XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};


class ScVbaControlFactory
{
public:
    static css::uno::Reference< ov::msforms::XControl > createShapeControl(
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const css::uno::Reference< css::drawing::XControlShape >& xControlShape,
        const css::uno::Reference< css::frame::XModel >& xModel ) throw (css::uno::RuntimeException);

    static css::uno::Reference< ov::msforms::XControl > createUserformControl(
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const css::uno::Reference< css::awt::XControl >& xControl,
        const css::uno::Reference< css::awt::XControl >& xDialog,
        const css::uno::Reference< css::frame::XModel >& xModel,
        double fOffsetX, double fOffsetY ) throw (css::uno::RuntimeException);

private:
    ScVbaControlFactory();
    ~ScVbaControlFactory();
};

#endif//SC_VBA_CONTROL_HXX
