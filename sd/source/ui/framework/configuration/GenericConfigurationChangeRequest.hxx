/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: GenericConfigurationChangeRequest.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 13:30:39 $
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

#ifndef SD_FRAMEWORK_GENERIC_CONFIGURATTION_CHANGE_REQUEST_HXX
#define SD_FRAMEWORK_GENERIC_CONFIGURATTION_CHANGE_REQUEST_HXX

#include "MutexOwner.hxx"

#include <com/sun/star/drawing/framework/XConfigurationChangeRequest.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/drawing/framework/XConfiguration.hpp>
#include <com/sun/star/drawing/framework/XResourceId.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <cppuhelper/compbase2.hxx>

namespace css = ::com::sun::star;

namespace {

typedef ::cppu::WeakComponentImplHelper2 <
      ::com::sun::star::drawing::framework::XConfigurationChangeRequest,
      ::com::sun::star::container::XNamed
    > GenericConfigurationChangeRequestInterfaceBase;

} // end of anonymous namespace.


namespace sd { namespace framework {

/** This implementation of the XConfigurationChangeRequest interface
    represents a single explicit request for a configuration change.  On its
    execution it may result in other, implicit, configuration changes.  For
    example this is the case when the deactivation of a unique resource is
    requested: the resources linked to it have to be deactivated as well.
*/
class GenericConfigurationChangeRequest
    : private MutexOwner,
      public GenericConfigurationChangeRequestInterfaceBase
{
public:
    /** This enum specified whether the activation or deactivation of a
        resource is requested.
    */
    enum Mode { Activation, Deactivation };

    /** Create a new object that represents the request for activation or
        deactivation of the specified resource.
        @param rxsResourceId
            Id of the resource that is to be activated or deactivated.
        @param eMode
            The mode specifies whether to activate or to deactivate the
            resource.
    */
    GenericConfigurationChangeRequest (
        const ::com::sun::star::uno::Reference<com::sun::star::drawing::framework::XResourceId>&
            rxResourceId,
        const Mode eMode)
        throw (::com::sun::star::lang::IllegalArgumentException);

    virtual ~GenericConfigurationChangeRequest (void) throw();


    // XConfigurationChangeOperation

    /** The requested configuration change is executed on the given
        configuration.  Additionally to the explicitly requested change
        other changes have to be made as well.  See class description for an
        example.
        @param rxConfiguration
            The configuration to which the requested change is made.
    */
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

private:
    const css::uno::Reference<css::drawing::framework::XResourceId> mxResourceId;
    const Mode meMode;
};

} } // end of namespace sd::framework

#endif
