/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: datalistener.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:11:48 $
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
#ifndef _SVX_DATALISTENER_HXX
#define _SVX_DATALISTENER_HXX

#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINERLISTENER_HPP_
#include <com/sun/star/container/XContainerListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAMEACTIONLISTENER_HPP_
#include <com/sun/star/frame/XFrameActionListener.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_DOM_EVENTS_XEVENTLISTENER_HPP_
#include <com/sun/star/xml/dom/events/XEventListener.hpp>
#endif

//............................................................................
namespace svxform
{
//............................................................................

    class DataNavigatorWindow;

    typedef cppu::WeakImplHelper3<
        com::sun::star::container::XContainerListener,
        com::sun::star::frame::XFrameActionListener,
        com::sun::star::xml::dom::events::XEventListener > DataListener_t;

    class DataListener : public DataListener_t
    {
    private:
        DataNavigatorWindow*        m_pNaviWin;

    public:
        DataListener( DataNavigatorWindow* pNaviWin );

    protected:
        ~DataListener();

    public:
        // XContainerListener
        virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException);

        // XFrameActionListener
        virtual void SAL_CALL frameAction( const ::com::sun::star::frame::FrameActionEvent& Action ) throw (::com::sun::star::uno::RuntimeException);

        // xml::dom::events::XEventListener
        virtual void SAL_CALL handleEvent( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::dom::events::XEvent >& evt ) throw (::com::sun::star::uno::RuntimeException);

        // lang::XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);
    };

//............................................................................
}   // namespace svxform
//............................................................................

#endif // #ifndef _SVX_DATALISTENER_HXX

