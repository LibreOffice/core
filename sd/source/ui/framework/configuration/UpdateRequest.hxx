/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: UpdateRequest.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-03 15:48:09 $
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

#ifndef SD_FRAMEWORK_UPDATE_REQUEST_HXX
#define SD_FRAMEWORK_UPDATE_REQUEST_HXX

#include "MutexOwner.hxx"

#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XCONFIGURATIONCHANGEREQUEST_HPP_
#include <com/sun/star/drawing/framework/XConfigurationChangeRequest.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XCONFIGURATION_HPP_
#include <com/sun/star/drawing/framework/XConfiguration.hpp>
#endif
#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif


namespace {

typedef ::cppu::WeakComponentImplHelper2 <
      ::com::sun::star::drawing::framework::XConfigurationChangeRequest,
      ::com::sun::star::container::XNamed
    > UpdateRequestInterfaceBase;

} // end of anonymous namespace.



namespace sd { namespace framework {

/** This update request is used to request configuration updates
    asynchronous when no other requests are being processed.  When there are
    other requests then we can simply wait until the last one is executed:
    the configuration is updated when the request queue becomes empty.  This
    is use by this implementation as well.  The execute() method does not
    really do anything.  This request just triggers the update of the
    configuration when it is removed as last request from the queue.
*/
class UpdateRequest
    : private MutexOwner,
      public UpdateRequestInterfaceBase
{
public:
    UpdateRequest (void) throw();
    virtual ~UpdateRequest (void) throw();


    // XConfigurationChangeOperation

    virtual void SAL_CALL execute (
        const ::com::sun::star::uno::Reference<
            com::sun::star::drawing::framework::XConfiguration>& rxConfiguration)
        throw (::com::sun::star::uno::RuntimeException);


    // XNamed

    /** Return a human readable string representation.  This is used for
        debugging purposes.
    */
    virtual ::rtl::OUString SAL_CALL getName (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** This call is ignored because the XNamed interface is (mis)used to
        give access to a human readable name for debugging purposes.
    */
    virtual void SAL_CALL setName (const ::rtl::OUString& rName)
        throw (::com::sun::star::uno::RuntimeException);
};

} } // end of namespace sd::framework

#endif
