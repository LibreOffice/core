/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: colorlistener.hxx,v $
 * $Revision: 1.6 $
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

#ifndef __FRAMEWORK_CLASSES_COLORLISTENER_HXX_
#define __FRAMEWORK_CLASSES_COLORLISTENER_HXX_

//__________________________________________
// own includes

#include <threadhelp/threadhelpbase.hxx>
#include <macros/xinterface.hxx>
#include <general.h>

//__________________________________________
// interface includes
#include <com/sun/star/awt/XWindow.hpp>

//__________________________________________
// other includes
#include <cppuhelper/weak.hxx>
#include <svtools/colorcfg.hxx>
#include <tools/link.hxx>

//__________________________________________
// definition

namespace framework
{

//__________________________________________

/** listen for color changes inside configuration.

    This listener looks for color changes provided at the config item
    "ColorConfig". All interested colors will be applied directly to a
    given window set window member. Curren implementation support setting
    of the background only.

    @see    ::svtools::ColorConfig
 */

class ColorListener : public  css::lang::XEventListener
                    , private ThreadHelpBase // attention! Must be the first base class to guarentee right initialize lock ...
                    , public  SfxListener
                    , public  ::cppu::OWeakObject

{
    //______________________________________
    // member

    private:

        /** points to the single config item, which provides the color-change notification. */
        ::svtools::ColorConfig* m_pConfig;

        /** reference to the window, on which we must set the changed color. */
        css::uno::Reference< css::awt::XWindow > m_xWindow;

        /** we must know, if we already registered as listener or not. */
        sal_Bool m_bListen;

        long m_nColor;

    //______________________________________
    // interface

    public:

        FWK_DECLARE_XINTERFACE

         ColorListener( const css::uno::Reference< css::awt::XWindow >& xWindow );
        ~ColorListener(                                                         );

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& aEvent ) throw(css::uno::RuntimeException);

        // SfxListener
        virtual void Notify( SfxBroadcaster& rBroadCaster, const SfxHint& rHint );

        DECL_LINK( impl_SettingsChanged, void* );

    //______________________________________
    // helper

    private:

        void impl_applyColor    ( sal_Bool bInvalidate );
        void impl_startListening();
        void impl_stopListening ();
        void impl_die           ();

}; // class ColorListener

} // namespace framework

#endif // __FRAMEWORK_CLASSES_COLORLISTENER_HXX_
