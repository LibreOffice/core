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



#ifndef LAYOUT_AWT_VCLXDIALOG_HXX
#define LAYOUT_AWT_VCLXDIALOG_HXX

#include <com/sun/star/awt/XDialog2.hpp>
#include <com/sun/star/awt/XSystemDependentWindowPeer.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <comphelper/uno3.hxx>
#include <layout/core/bin.hxx>
#include <toolkit/awt/vclxtopwindow.hxx>

namespace layoutimpl
{

typedef ::cppu::ImplHelper1 < ::com::sun::star::awt::XDialog2 > VCLXDialog_Base;

class TOOLKIT_DLLPUBLIC VCLXDialog : public VCLXWindow
                                   , public VCLXTopWindow_Base
                                   , public VCLXDialog_Base
                                   , public Bin
{
private:
    bool bRealized, bResizeSafeguard;
    css::uno::Reference< css::awt::XLayoutUnit > mxLayoutUnit;

    VCLXDialog( const VCLXDialog& );            // never implemented
    VCLXDialog& operator=( const VCLXDialog& ); // never implemented

protected:
    vos::IMutex& GetMutexImpl();
    Window* GetWindowImpl();
    ::cppu::OInterfaceContainerHelper& GetTopWindowListenersImpl();

    ~VCLXDialog();

    // XInterface
    DECLARE_XINTERFACE()

    // XTypeProvider
    DECLARE_XTYPEPROVIDER()

    // XComponent
    void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);

    // VclWindowPeer
    virtual void SAL_CALL setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException);

    // VCLXWindow
    void ProcessWindowEvent( const VclWindowEvent& _rVclWindowEvent );

    // ::com::sun::star::awt::XDialog
    void SAL_CALL setTitle( const ::rtl::OUString& Title ) throw(::com::sun::star::uno::RuntimeException);
    ::rtl::OUString SAL_CALL getTitle() throw(::com::sun::star::uno::RuntimeException);
    sal_Int16 SAL_CALL execute() throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL endExecute() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XLayoutContainer
    virtual void SAL_CALL allocateArea( const css::awt::Rectangle &rArea )
        throw (css::uno::RuntimeException);

    void resizedCb();

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &/*aIds*/ )
    {
    }
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) { return ImplGetPropertyIds( aIds ); }


public:
    VCLXDialog();

    // ::com::sun::star::awt::XDialog2
    void SAL_CALL endDialog( sal_Int32 nResult ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setHelpId( const rtl::OUString& id ) throw(::com::sun::star::uno::RuntimeException);

};

} // namespace layoutimpl

#endif /* LAYOUT_AWT_VCLXDIALOG_HXX */
