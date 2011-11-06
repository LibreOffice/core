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



#ifndef _AVMEDIA_MEDIAEVENT_IMPL_HXX
#define _AVMEDIA_MEDIAEVENT_IMPL_HXX

#include <avmedia/mediawindow.hxx>
#include <cppuhelper/compbase4.hxx>
#include <com/sun/star/awt/XKeyListener.hpp>
#include <com/sun/star/awt/XMouseListener.hpp>
#include <com/sun/star/awt/XMouseMotionListener.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>

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
