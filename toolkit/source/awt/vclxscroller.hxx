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



#ifndef LAYOUT_AWT_VCLXSCROLLER_HXX
#define LAYOUT_AWT_VCLXSCROLLER_HXX

#include <comphelper/uno3.hxx>
#include <layout/core/bin.hxx>
#include <toolkit/awt/vclxwindow.hxx>

class ScrollBar;
class FixedImage;

namespace layoutimpl
{

class VCLXScroller :public VCLXWindow
                   ,public Bin
{
public:
    VCLXScroller();

protected:
    ~VCLXScroller();

    // XInterface
    DECLARE_XINTERFACE()

    // XTypeProvider
    DECLARE_XTYPEPROVIDER()

    // XComponent
    void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XLayoutContainer
    virtual void SAL_CALL allocateArea( const ::com::sun::star::awt::Rectangle &rArea )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Size SAL_CALL getMinimumSize()
        throw(::com::sun::star::uno::RuntimeException);

    // VclWindowPeer
    virtual void SAL_CALL setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException);

    // VCLXWindow
    void ProcessWindowEvent( const VclWindowEvent& _rVclWindowEvent );

private:
    VCLXScroller( const VCLXScroller& );            // never implemented
    VCLXScroller& operator=( const VCLXScroller& ); // never implemented

    // because the underlying window is only setup-ed after construction, init
    // scrollbars at play-time
    void ensureScrollBars();

    FixedImage *mpContent;  // dummy
    ScrollBar *mpHorScrollBar, *mpVerScrollBar;
    DECL_LINK( ScrollHdl, ScrollBar* );
};

} // namespace layoutimpl

#endif /* LAYOUT_AWT_VCLXSCROLLER_HXX */
