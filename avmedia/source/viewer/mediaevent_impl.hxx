/*************************************************************************
 *
 *  $RCSfile: mediaevent_impl.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: ka $ $Date: 2004-08-23 09:04:42 $
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

#ifndef _AVMEDIA_MEDIAEVENT_IMPL_HXX
#define _AVMEDIA_MEDIAEVENT_IMPL_HXX

#include "mediawindow.hxx"

#ifndef _CPPUHELPER_COMPBASE4_HXX_
#include <cppuhelper/compbase4.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_XKEYLISTENER_HPP_
#include <com/sun/star/awt/XKeyListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XMOUSELISTENER_HPP_
#include <com/sun/star/awt/XMouseListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XMOUSEMOTIONLISTENER_HPP_
#include <com/sun/star/awt/XMouseMotionListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XFOCUSLISTENER_HPP_
#include <com/sun/star/awt/XFocusListener.hpp>
#endif

namespace avmedia
{
    namespace priv
    {
        // ---------------------------
        // - MediaEventListenersImpl -
        // ---------------------------

        class MediaWindowImpl;

        class MediaEventListenersImpl : public ::cppu::WeakImplHelper4< ::com::sun::star::awt::XKeyListener,
                                                                        ::com::sun::star::awt::XMouseListener,
                                                                        ::com::sun::star::awt::XMouseMotionListener,
                                                                        ::com::sun::star::awt::XFocusListener >
        {
        public:

                    MediaEventListenersImpl( Window& rNotifyWindow );
                    ~MediaEventListenersImpl();

        void        cleanUp();

        protected:

            // XKeyListener
            virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL keyPressed( const ::com::sun::star::awt::KeyEvent& e ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL keyReleased( const ::com::sun::star::awt::KeyEvent& e ) throw (::com::sun::star::uno::RuntimeException);

            // XMouseListener
            virtual void SAL_CALL mousePressed( const ::com::sun::star::awt::MouseEvent& e ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL mouseReleased( const ::com::sun::star::awt::MouseEvent& e ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL mouseEntered( const ::com::sun::star::awt::MouseEvent& e ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL mouseExited( const ::com::sun::star::awt::MouseEvent& e ) throw (::com::sun::star::uno::RuntimeException);

            // XMouseMotionListener
            virtual void SAL_CALL mouseDragged( const ::com::sun::star::awt::MouseEvent& e ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL mouseMoved( const ::com::sun::star::awt::MouseEvent& e ) throw (::com::sun::star::uno::RuntimeException);

            // XFocusListener
            virtual void SAL_CALL focusGained( const ::com::sun::star::awt::FocusEvent& e ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL focusLost( const ::com::sun::star::awt::FocusEvent& e ) throw (::com::sun::star::uno::RuntimeException);

        private:

            Window*                 mpNotifyWindow;
            mutable ::osl::Mutex    maMutex;
        };
    }
}

#endif
