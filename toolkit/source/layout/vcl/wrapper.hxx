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



#ifndef LAYOUT_VCL_WRAPPER_HXX
#define LAYOUT_VCL_WRAPPER_HXX

#include <layout/layout.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/awt/XDialog2.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <cppuhelper/implbase1.hxx>

#include <cstring>

namespace layout
{

namespace css = com::sun::star;

class WindowImpl
{
public:
    Window *mpWindow;
    Context *mpCtx;
    css::uno::Reference< css::awt::XWindow > mxWindow;
    css::uno::Reference< css::awt::XVclWindowPeer > mxVclPeer;
    ::Window *mvclWindow;
    bool bFirstTimeVisible;

    WindowImpl (Context *context, PeerHandle const &peer, Window *window);
    virtual ~WindowImpl ();

    void wrapperGone();
    css::uno::Any getProperty (char const *name);
    void setProperty (char const *name, css::uno::Any any);
    void redraw (bool resize=false);

    // XFocusListener
    virtual void SAL_CALL disposing (css::lang::EventObject const&) throw (css::uno::RuntimeException);
};

class ControlImpl : public WindowImpl
                  , public ::cppu::WeakImplHelper1 <css::awt::XFocusListener>
{
public:
    Link mGetFocusHdl;
    Link mLoseFocusHdl;

    ControlImpl( Context *context, PeerHandle const& peer, Window *window );
    ~ControlImpl ();

    virtual void SetGetFocusHdl (Link const& link);
    Link& GetGetFocusHdl ();
    virtual void SetLoseFocusHdl (Link const& link);
    Link& GetLoseFocusHdl ();
    virtual void UpdateListening (Link const& link);

    // XFocusListener
    virtual void SAL_CALL disposing (css::lang::EventObject const&) throw (css::uno::RuntimeException);
    void SAL_CALL focusGained (css::awt::FocusEvent const& e) throw (css::uno::RuntimeException);
    void SAL_CALL focusLost (css::awt::FocusEvent const& e) throw (css::uno::RuntimeException);
};

inline WindowImpl *Window::getImpl() const{ return (static_cast< WindowImpl * >( mpImpl )); }

// Helpers for defining boiler-plate constructors ...
// Could in-line in top-level but not with safe static_casts.
#define IMPL_GET_IMPL(t) \
    inline t##Impl* t::getImpl() const \
    { \
        return (static_cast<t##Impl *>(mpImpl)); \
    }
#define IMPL_CONSTRUCTORS_BODY(t,par,unoName,body) \
    t::t( Context *context, const char *pId, sal_uInt32 nId ) \
        : par( new t##Impl( context, context->GetPeerHandle( pId, nId ), this ) ) \
    { \
        Window *parent = dynamic_cast<Window*> (context);\
        body;\
        if (parent)\
            SetParent (parent);\
    } \
    t::t( Window *parent, WinBits bits) \
        : par( new t##Impl( parent->getContext(), Window::CreatePeer( parent, bits, unoName ), this ) ) \
    { \
        body;\
        if ( parent )\
            SetParent (parent);\
    } \
    t::t( Window *parent, ResId const& res) \
        : par( new t##Impl( parent->getContext(), Window::CreatePeer( parent, 0, unoName ), this ) ) \
    { \
        body;\
        setRes (res);\
        if (parent)\
            SetParent (parent);\
    }
#define IMPL_CONSTRUCTORS(t,par,unoName) IMPL_CONSTRUCTORS_BODY(t, par, unoName, )
#define IMPL_CONSTRUCTORS_2(t,win_par,other_par,unoName) \
    t::t( Context *context, const char *pId, sal_uInt32 nId ) \
        : win_par( new t##Impl( context, context->GetPeerHandle( pId, nId ), this ) ) \
        , other_par( new other_par##Impl( Window::GetPeer() ) ) \
    { \
    } \
    t::t( Window *parent, WinBits bits) \
        : win_par( new t##Impl( parent->getContext(), Window::CreatePeer( parent, bits, unoName ), this ) ) \
        , other_par( new other_par##Impl( Window::GetPeer() ) ) \
    { \
    }

#define IMPL_IMPL(t, parent) \
    class t##Impl : public parent##Impl \
    { \
    public: \
        t##Impl( Context *context, PeerHandle const& peer, Window *window ) \
            : parent##Impl( context, peer, window ) \
        { \
        } \
    };


} // namespace layout

#endif /* LAYOUT_VCL_WRAPPER_HXX */
