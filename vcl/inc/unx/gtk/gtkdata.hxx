/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_VCL_INC_UNX_GTK_GTKDATA_HXX
#define INCLUDED_VCL_INC_UNX_GTK_GTKDATA_HXX

#define GLIB_DISABLE_DEPRECATION_WARNINGS
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#if GTK_CHECK_VERSION(4,0,0)
#include <gdk/x11/gdkx.h>
#else
#include <gdk/gdkx.h>
#endif

#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>
#include <unx/gendata.hxx>
#include <unx/saldisp.hxx>
#include <unx/gtk/gtksys.hxx>
#include <vcl/ptrstyle.hxx>
#include <osl/conditn.hxx>
#include <saltimer.hxx>
#include <o3tl/enumarray.hxx>

#include <string_view>
#include <vector>

namespace com::sun::star::accessibility { class XAccessibleEventListener; }

class GtkSalDisplay;
class DocumentFocusListener;

#if !GTK_CHECK_VERSION(4,0,0)
typedef GdkWindow GdkSurface;
typedef GdkWindowState GdkToplevelState;
#endif

inline void main_loop_run(GMainLoop* pLoop)
{
#if !GTK_CHECK_VERSION(4, 0, 0)
    gdk_threads_leave();
#endif
    g_main_loop_run(pLoop);
#if !GTK_CHECK_VERSION(4, 0, 0)
    gdk_threads_enter();
#endif
}

inline void css_provider_load_from_data(GtkCssProvider *css_provider,
                                        const gchar *data,
                                        gssize length)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    gtk_css_provider_load_from_data(css_provider, data, length);
#else
    gtk_css_provider_load_from_data(css_provider, data, length, nullptr);
#endif
}

inline GtkWidget* widget_get_toplevel(GtkWidget* pWidget)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    GtkRoot* pRoot = gtk_widget_get_root(pWidget);
    return pRoot ? GTK_WIDGET(pRoot) : pWidget;
#else
    return gtk_widget_get_toplevel(pWidget);
#endif
}

inline const char* image_get_icon_name(GtkImage *pImage)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    return gtk_image_get_icon_name(pImage);
#else
    const gchar* icon_name;
    gtk_image_get_icon_name(pImage, &icon_name, nullptr);
    return icon_name;
#endif
}

inline GtkWidget* widget_get_first_child(GtkWidget *pWidget)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    return gtk_widget_get_first_child(pWidget);
#else
    GList* pChildren = gtk_container_get_children(GTK_CONTAINER(pWidget));
    GList* pChild = g_list_first(pChildren);
    GtkWidget* pRet = pChild ? static_cast<GtkWidget*>(pChild->data) : nullptr;
    g_list_free(pChildren);
    return pRet;
#endif
}

inline void style_context_get_color(GtkStyleContext *pStyle, GdkRGBA *pColor)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    return gtk_style_context_get_color(pStyle, pColor);
#else
    return gtk_style_context_get_color(pStyle, gtk_style_context_get_state(pStyle), pColor);
#endif
}

inline GdkSurface* widget_get_surface(GtkWidget* pWidget)
{
#if GTK_CHECK_VERSION(4,0,0)
    return gtk_native_get_surface(gtk_widget_get_native(pWidget));
#else
    return gtk_widget_get_window(pWidget);
#endif
}

inline void widget_set_cursor(GtkWidget *pWidget, GdkCursor *pCursor)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    gtk_widget_set_cursor(pWidget, pCursor);
#else
    gdk_window_set_cursor(gtk_widget_get_window(pWidget), pCursor);
#endif
}

inline cairo_surface_t * surface_create_similar_surface(GdkSurface *pSurface,
                                                        cairo_content_t eContent,
                                                        int nWidth,
                                                        int nHeight)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    return gdk_surface_create_similar_surface(pSurface, eContent, nWidth, nHeight);
#else
    return gdk_window_create_similar_surface(pSurface, eContent, nWidth, nHeight);
#endif
}

inline void im_context_set_client_widget(GtkIMContext *pIMContext, GtkWidget *pWidget)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    gtk_im_context_set_client_widget(pIMContext, pWidget);
#else
    gtk_im_context_set_client_window(pIMContext, pWidget ? gtk_widget_get_window(pWidget) : nullptr);
#endif
}

#if GTK_CHECK_VERSION(4, 0, 0)
typedef double gtk_coord;
#else
typedef int gtk_coord;
#endif

inline bool surface_get_device_position(GdkSurface* pSurface,
                                        GdkDevice* pDevice,
                                        double& x,
                                        double& y,
                                        GdkModifierType* pMask)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    return gdk_surface_get_device_position(pSurface, pDevice,
                                           &x, &y,
                                           pMask);
#else
    int nX(x), nY(y);
    bool bRet = gdk_window_get_device_position(pSurface, pDevice,
                                               &nX, &nY,
                                               pMask);
    x = nX;
    y = nY;
    return bRet;
#endif
}

inline GdkGLContext* surface_create_gl_context(GdkSurface* pSurface)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    return gdk_surface_create_gl_context(pSurface, nullptr);
#else
    return gdk_window_create_gl_context(pSurface, nullptr);
#endif
}

void set_buildable_id(GtkBuildable* pWidget, const OString& rId);
OString get_buildable_id(GtkBuildable* pWidget);

void container_remove(GtkWidget* pContainer, GtkWidget* pChild);
void container_add(GtkWidget* pContainer, GtkWidget* pChild);

#if !GTK_CHECK_VERSION(4, 0, 0)
typedef GtkClipboard GdkClipboard;
#endif

int getButtonPriority(std::string_view rType);

class GtkSalTimer final : public SalTimer
{
    struct SalGtkTimeoutSource *m_pTimeout;
public:
    GtkSalTimer();
    virtual ~GtkSalTimer() override;
    virtual void Start( sal_uInt64 nMS ) override;
    virtual void Stop() override;
    bool         Expired();

    sal_uInt64    m_nTimeoutMS;
};

class DocumentFocusListener final :
    public ::cppu::WeakImplHelper< css::accessibility::XAccessibleEventListener >
{

    o3tl::sorted_vector< css::uno::Reference< css::uno::XInterface > > m_aRefList;

public:
    /// @throws lang::IndexOutOfBoundsException
    /// @throws uno::RuntimeException
    void attachRecursive(
        const css::uno::Reference< css::accessibility::XAccessible >& xAccessible
    );

    /// @throws lang::IndexOutOfBoundsException
    /// @throws uno::RuntimeException
    void attachRecursive(
        const css::uno::Reference< css::accessibility::XAccessible >& xAccessible,
        const css::uno::Reference< css::accessibility::XAccessibleContext >& xContext
    );

    /// @throws lang::IndexOutOfBoundsException
    /// @throws uno::RuntimeException
    void attachRecursive(
        const css::uno::Reference< css::accessibility::XAccessible >& xAccessible,
        const css::uno::Reference< css::accessibility::XAccessibleContext >& xContext,
        const css::uno::Reference< css::accessibility::XAccessibleStateSet >& xStateSet
    );

    /// @throws lang::IndexOutOfBoundsException
    /// @throws uno::RuntimeException
    void detachRecursive(
        const css::uno::Reference< css::accessibility::XAccessible >& xAccessible
    );

    /// @throws lang::IndexOutOfBoundsException
    /// @throws uno::RuntimeException
    void detachRecursive(
        const css::uno::Reference< css::accessibility::XAccessibleContext >& xContext
    );

    /// @throws lang::IndexOutOfBoundsException
    /// @throws uno::RuntimeException
    void detachRecursive(
        const css::uno::Reference< css::accessibility::XAccessibleContext >& xContext,
        const css::uno::Reference< css::accessibility::XAccessibleStateSet >& xStateSet
    );

    /// @throws lang::IndexOutOfBoundsException
    /// @throws uno::RuntimeException
    static css::uno::Reference< css::accessibility::XAccessible > getAccessible(const css::lang::EventObject& aEvent );

    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

    // XAccessibleEventListener
    virtual void SAL_CALL notifyEvent( const css::accessibility::AccessibleEventObject& aEvent ) override;
};

class GtkSalData final : public GenericUnixSalData
{
    GSource*        m_pUserEvent;
    osl::Mutex      m_aDispatchMutex;
    osl::Condition  m_aDispatchCondition;
    std::exception_ptr m_aException;

    rtl::Reference<DocumentFocusListener> m_xDocumentFocusListener;

public:
    GtkSalData( SalInstance *pInstance );
    virtual ~GtkSalData() override;

    DocumentFocusListener & GetDocumentFocusListener();

    void Init();
    virtual void Dispose() override;

    static void initNWF();
    static void deInitNWF();

    void TriggerUserEventProcessing();
    void TriggerAllUserEventsProcessed();

    bool Yield( bool bWait, bool bHandleAllCurrentEvents );
    inline GdkDisplay *GetGdkDisplay();

    virtual void ErrorTrapPush() override;
    virtual bool ErrorTrapPop( bool bIgnoreError = true ) override;

    inline GtkSalDisplay *GetGtkDisplay() const;
    void setException(const std::exception_ptr& exception) { m_aException = exception; }
};

class GtkSalFrame;

class GtkSalDisplay final : public SalGenericDisplay
{
    GtkSalSystem*                   m_pSys;
    GdkDisplay*                     m_pGdkDisplay;
    o3tl::enumarray<PointerStyle, GdkCursor*> m_aCursors;
    bool                            m_bStartupCompleted;

    GdkCursor* getFromSvg( OUString const & name, int nXHot, int nYHot );

public:
             GtkSalDisplay( GdkDisplay* pDisplay );
    virtual ~GtkSalDisplay() override;

    GdkDisplay* GetGdkDisplay() const { return m_pGdkDisplay; }

    GtkSalSystem* getSystem() const { return m_pSys; }

    GtkWidget* findGtkWidgetForNativeHandle(sal_uIntPtr hWindow) const;

    virtual void deregisterFrame( SalFrame* pFrame ) override;
    GdkCursor *getCursor( PointerStyle ePointerStyle );
    virtual int CaptureMouse( SalFrame* pFrame );

    SalX11Screen GetDefaultXScreen() { return m_pSys->GetDisplayDefaultXScreen(); }
    Size         GetScreenSize( int nDisplayScreen );

    void startupNotificationCompleted() { m_bStartupCompleted = true; }

#if !GTK_CHECK_VERSION(4,0,0)
    void screenSizeChanged( GdkScreen const * );
    void monitorsChanged( GdkScreen const * );
#endif

    virtual void TriggerUserEventProcessing() override;
    virtual void TriggerAllUserEventsProcessed() override;
};

inline GtkSalData* GetGtkSalData()
{
    return static_cast<GtkSalData*>(ImplGetSVData()->mpSalData);
}
inline GdkDisplay *GtkSalData::GetGdkDisplay()
{
    return GetGtkDisplay()->GetGdkDisplay();
}

GtkSalDisplay *GtkSalData::GetGtkDisplay() const
{
    return static_cast<GtkSalDisplay *>(GetDisplay());
}

#endif // INCLUDED_VCL_INC_UNX_GTK_GTKDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
