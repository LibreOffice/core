/*************************************************************************
 *
 *  $RCSfile: colorlistener.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 18:19:32 $
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

    //______________________________________
    // interface

    public:

        DECLARE_XINTERFACE

         ColorListener( const css::uno::Reference< css::awt::XWindow >& xWindow );
        ~ColorListener(                                                         );

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& aEvent ) throw(css::uno::RuntimeException);

        // SfxListener
        virtual void Notify( SfxBroadcaster& rBroadCaster, const SfxHint& rHint );

    //______________________________________
    // helper

    private:

        void impl_applyColor    ();
        void impl_startListening();
        void impl_stopListening ();
        void impl_die           ();

}; // class ColorListener

} // namespace framework

#endif // __FRAMEWORK_CLASSES_COLORLISTENER_HXX_
