/*************************************************************************
 *
 *  $RCSfile: colorlistener.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 18:21:28 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "classes/colorlistener.hxx"

//__________________________________________
// own includes

#ifndef __FRAMEWORK_THREADHELP_READGUARD_HXX_
#include <threadhelp/readguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_
#include <threadhelp/writeguard.hxx>
#endif

//__________________________________________
// interface includes

#ifndef _COM_SUN_STAR_AWT_XWINDOWPEER_HPP_
#include <com/sun/star/awt/XWindowPeer.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_INVALIDATESTYLE_HPP_
#include <com/sun/star/awt/InvalidateStyle.hpp>
#endif

//__________________________________________
// other includes

#ifndef _RTL_USTRING_H_
#include <rtl/ustring.h>
#endif

#ifndef _SFXSMPLHINT_HXX
#include <svtools/smplhint.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

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
    , m_xWindow     (xWindow                      )
    , m_bListen     (sal_False                    )
    , m_pConfig     (NULL                         )
{
    impl_startListening();
    impl_applyColor();
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

/** callback for color changes.

    @param  rBroadcaster
                should be our referenced config item (or any helper of it!)

    @param  rHint
                transport an ID, which identify the broadcasted message
 */

void ColorListener::Notify( SfxBroadcaster& rBroadCaster, const SfxHint& rHint )
{
    if (((SfxSimpleHint&)rHint).GetId()==SFX_HINT_COLORS_CHANGED)
        impl_applyColor();
}

void ColorListener::impl_applyColor()
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);

    if (m_pConfig)
    {
        css::uno::Reference< css::awt::XWindowPeer > xPeer(m_xWindow, css::uno::UNO_QUERY);
        ::svtools::ColorConfigValue aBackgroundColor = m_pConfig->GetColorValue( ::svtools::APPBACKGROUND );
        if (xPeer.is())
        {
            xPeer->setBackground(aBackgroundColor.nColor);
            xPeer->invalidate(
                css::awt::InvalidateStyle::UPDATE | css::awt::InvalidateStyle::CHILDREN | css::awt::InvalidateStyle::NOTRANSPARENT );
        }
    }

    aReadLock.unlock();
    /* } SAFE */
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
