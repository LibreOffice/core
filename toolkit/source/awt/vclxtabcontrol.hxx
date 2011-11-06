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



#ifndef LAYOUT_AWT_VCLXTABCONTROLLER_HXX
#define LAYOUT_AWT_VCLXTABCONTROLLER_HXX

#include <com/sun/star/awt/XSimpleTabController.hpp>
#include <comphelper/uno3.hxx>
#include <layout/core/box-base.hxx>
#include <map>
#include <toolkit/awt/vclxwindow.hxx>

class TabControl;

namespace layoutimpl
{

typedef ::cppu::ImplHelper1 <   ::com::sun::star::awt::XSimpleTabController
                                >   VCLXTabControl_Base;

class VCLXTabControl :public VCLXWindow
                     ,public VCLXTabControl_Base
                     ,public Box_Base
{
    int mTabId;
    bool bRealized;

public:
    VCLXTabControl();

    void AddChild (css::uno::Reference <css::awt::XLayoutConstrains> const &);

protected:
    ~VCLXTabControl();

    // XInterface
    DECLARE_XINTERFACE()

    // XTypeProvider
    DECLARE_XTYPEPROVIDER()

    // XComponent
    void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL draw( sal_Int32 nX, sal_Int32 nY ) throw (::com::sun::star::uno::RuntimeException);

    // XSimpleTabController
    virtual ::sal_Int32 SAL_CALL insertTab() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeTab( ::sal_Int32 ID ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL setTabProps( ::sal_Int32 ID, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& Properties ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > SAL_CALL getTabProps( ::sal_Int32 ID ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL activateTab( ::sal_Int32 ID ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getActiveTabID() throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL addTabListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeTabListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XLayoutContainer
    virtual void SAL_CALL addChild(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XLayoutConstrains >& Child )
        throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::awt::MaxChildrenException);
    virtual void SAL_CALL removeChild( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XLayoutConstrains >& Child )
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL allocateArea( const ::com::sun::star::awt::Rectangle &rArea )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::awt::Size SAL_CALL getMinimumSize()
        throw(::com::sun::star::uno::RuntimeException);

    // unimplemented:
    virtual sal_Bool SAL_CALL hasHeightForWidth()
        throw(css::uno::RuntimeException)
    { return false; }
    virtual sal_Int32 SAL_CALL getHeightForWidth( sal_Int32 /*nWidth*/ )
    throw(css::uno::RuntimeException)
    { return maRequisition.Height; }

    // VclWindowPeer
    virtual void SAL_CALL setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException);

    // VCLXWindow
    void ProcessWindowEvent( const VclWindowEvent& _rVclWindowEvent );

public:
    // Maps page ids to child references
    struct ChildData : public Box_Base::ChildData
    {
        rtl::OUString maTitle;
        ChildData( css::uno::Reference< css::awt::XLayoutConstrains > const& xChild );
    };

    struct ChildProps : public Box_Base::ChildProps
    {
        ChildProps( VCLXTabControl::ChildData *pData );
    };

protected:
    ChildData *createChild( css::uno::Reference< css::awt::XLayoutConstrains > const& xChild );
    ChildProps *createChildProps( Box_Base::ChildData* pData );


    std::map< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XLayoutConstrains >, sal_Int32 > mIdMap;
    // FIXME: we might want to use a Multiplexer
    std::list< ::com::sun::star::uno::Reference
               < ::com::sun::star::awt::XTabListener > > mxTabListeners;

    inline TabControl *getTabControl() const throw (::com::sun::star::uno::RuntimeException);

private:
    VCLXTabControl( const VCLXTabControl& );            // never implemented
    VCLXTabControl& operator=( const VCLXTabControl& ); // never implemented
};

} // namespace layoutimpl

#endif /* LAYOUT_AWT_VCLXTABCONTROLLER_HXX */
