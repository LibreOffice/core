/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: colorlistener.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 10:49:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef __FRAMEWORK_CLASSES_COLORLISTENER_HXX_
#define __FRAMEWORK_CLASSES_COLORLISTENER_HXX_

//__________________________________________
// own includes

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

//__________________________________________
// interface includes

#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif

//__________________________________________
// other includes

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif

#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif

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
