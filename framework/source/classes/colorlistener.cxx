/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: colorlistener.cxx,v $
 * $Revision: 1.8 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

#include "classes/colorlistener.hxx"

//__________________________________________
// own includes
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>

//__________________________________________
// interface includes
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/awt/InvalidateStyle.hpp>

//__________________________________________
// other includes
#include <rtl/ustring.h>
#include <rtl/ustrbuf.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <svtools/smplhint.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>

//__________________________________________
// definition

namespace framework
{

DEFINE_XINTERFACE_1( ColorListener                             ,
                     OWeakObject                               ,
                     DIRECT_INTERFACE(css::lang::XEventListener))

//__________________________________________

/** initialize new instance of this class.

    It set the window, on which we must apply our detecte color changes.
    We start listening for changes and(!) window disposing here too.

    @attention  Some ressources will be created on demand!

    @param  xWindow
                reference to the window
 */

ColorListener::ColorListener( const css::uno::Reference< css::awt::XWindow >& xWindow )
    : ThreadHelpBase(&Application::GetSolarMutex())
    , SfxListener   (                             )
    , m_pConfig     (NULL                         )
    , m_xWindow     (xWindow                      )
    , m_bListen     (sal_False                    )
{
    impl_startListening();
    impl_applyColor(sal_True);
}

//__________________________________________

/** deinitialize new instance of this class.

    Because it's done at different places ... we use an impl method!

    see impl_die()
 */

ColorListener::~ColorListener()
{
    impl_die();
}

//__________________________________________
/** callback if color config was changed.

    @param  rBroadcaster
            should be our referenced config item (or any helper of it!)

    @param  rHint
            transport an ID, which identify the broadcasted message
 */
void ColorListener::Notify(SfxBroadcaster&, const SfxHint& rHint)
{
    if (((SfxSimpleHint&)rHint).GetId()==SFX_HINT_COLORS_CHANGED)
        impl_applyColor(sal_True);
}

//__________________________________________
/** callback if window color was changed by any other!
    We own this window .. and we are the only ones, which
    define the background color of this window.
 */
IMPL_LINK(ColorListener, impl_SettingsChanged, void*, pVoid)
{
    // ignore uninteressting notifications
    VclWindowEvent* pEvent = (VclWindowEvent*)pVoid;
    if (pEvent->GetId() != VCLEVENT_WINDOW_DATACHANGED)
        return 0L;

    // SAFE ->
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::awt::XWindow > xWindow       = m_xWindow;
    long                                     nCurrentColor = m_nColor ;
    aReadLock.unlock();
    // <- SAFE

    if (!xWindow.is())
        return 0L;

    Window* pWindow = VCLUnoHelper::GetWindow(xWindow);
    if (!pWindow)
        return 0L;

    OutputDevice* pDevice   = (OutputDevice*)pWindow;
    long          nNewColor = (long)(pDevice->GetBackground().GetColor().GetColor());

    // Was window background changed?
    // NO  -> do nothing
    // YES -> apply our color!
    if (nCurrentColor != nNewColor)
        impl_applyColor(sal_False);

    return 0L;
}

//__________________________________________
/** set a new background color (retrieved from the configuration)
    on the window.

    @param  bInvalidate
            If it's setto TRUE it forces a synchronous update
            of the window background.
 */
void ColorListener::impl_applyColor(sal_Bool bInvalidate)
{
    // SAFE ->
    WriteGuard aWriteLock(m_aLock);

    if (!m_pConfig)
        return;
    ::svtools::ColorConfigValue aBackgroundColor = m_pConfig->GetColorValue(::svtools::APPBACKGROUND);
    m_nColor = aBackgroundColor.nColor;

    css::uno::Reference< css::awt::XWindowPeer > xPeer(m_xWindow, css::uno::UNO_QUERY);

    aWriteLock.unlock();
    // <- SAFE

    if (!xPeer.is())
        return;

    xPeer->setBackground(aBackgroundColor.nColor);
    if (bInvalidate)
    {
        xPeer->invalidate(
            css::awt::InvalidateStyle::UPDATE        |
            css::awt::InvalidateStyle::CHILDREN      |
            css::awt::InvalidateStyle::NOTRANSPARENT );
    }
}

//__________________________________________

/** callback for window destroy.

    We must react here automaticly and forget our window reference.
    We can die immediatly too. Because there is nothing to do any longer.

    @param  aEvent
                must referr to our window.

    @throw  ::com::sun::star::uno::RuntimeException
                if event source doesn't points to our internal saved window.
 */
void SAL_CALL ColorListener::disposing( const css::lang::EventObject& aEvent ) throw(css::uno::RuntimeException)
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);

    if (aEvent.Source!=m_xWindow)
        throw css::uno::RuntimeException(
            DECLARE_ASCII(""),
            css::uno::Reference< css::uno::XInterface >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY) );

    impl_die();

    aReadLock.unlock();
    /* } SAFE */
}

//__________________________________________

/** starts listening for color changes and window destroy.

    We create the needed config singleton on demand here.
 */

void ColorListener::impl_startListening()
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);

    if (!m_bListen)
    {
        Window* pWindow = VCLUnoHelper::GetWindow(m_xWindow);
        if (pWindow)
            pWindow->AddEventListener(LINK(this, ColorListener, impl_SettingsChanged));

        if (!m_pConfig)
            m_pConfig = new ::svtools::ColorConfig();

        StartListening(*(SfxBroadcaster*)m_pConfig);

        css::uno::Reference< css::lang::XComponent > xDispose(m_xWindow, css::uno::UNO_QUERY);
        if (xDispose.is())
            xDispose->addEventListener( css::uno::Reference< css::lang::XEventListener >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY) );

        m_bListen = sal_True;
    }

    aWriteLock.unlock();
    /* } SAFE */
}

//__________________________________________

/** stops listening for color changes and window destroy.
 */
void ColorListener::impl_stopListening()
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);

    if (m_bListen)
    {
        Window* pWindow = VCLUnoHelper::GetWindow(m_xWindow);
        if (pWindow)
            pWindow->RemoveEventListener( LINK( this, ColorListener, impl_SettingsChanged ) );

        EndListeningAll();

        delete m_pConfig;
        m_pConfig = NULL;

        css::uno::Reference< css::lang::XComponent > xDispose(m_xWindow, css::uno::UNO_QUERY);
        if (xDispose.is())
            xDispose->removeEventListener( css::uno::Reference< css::lang::XEventListener >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY) );

        m_bListen = sal_False;
    }

    aWriteLock.unlock();
    /* } SAFE */
}

//__________________________________________

/** release all used references.

    Free all used memory and release any used references.
    Of course cancel all existing listener connections, to be
    shure never be called again.
 */
void ColorListener::impl_die()
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);

    // Attention: Deleting of our broadcaster will may force a Notify() call.
    // To supress that, we have to disable our listener connection first.
    impl_stopListening();
    m_xWindow = css::uno::Reference< css::awt::XWindow >();

    aReadLock.unlock();
    /* } SAFE */
}

} // namespace framework
