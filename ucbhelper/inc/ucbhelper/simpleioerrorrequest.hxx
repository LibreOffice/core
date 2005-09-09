/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: simpleioerrorrequest.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 16:31:58 $
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

#ifndef _UCBHELPER_SIMPLEIOERRORREQUEST_HXX
#define _UCBHELPER_SIMPLEIOERRORREQUEST_HXX

#ifndef _COM_SUN_STAR_UCB_IOERRORCODE_HPP_
#include <com/sun/star/ucb/IOErrorCode.hpp>
#endif

#ifndef _UCBHELPER_INTERATIONREQUEST_HXX
#include <ucbhelper/interactionrequest.hxx>
#endif

namespace com { namespace sun { namespace star { namespace ucb {
    class XCommandProcessor;
} } } }

namespace ucbhelper {

/**
  * This class implements a simple IO error interaction request. Instances
  * can be passed directly to XInteractionHandler::handle(...). Each
  * instance contains an InteractiveIOException and one interaction
  * continuation: "Abort".
  *
  * @see com::sun::star::ucb::InteractiveIOException
  * @see InteractionAbort
  */
class SimpleIOErrorRequest : public ucbhelper::InteractionRequest
{
public:
    /**
      * Constructor.
      *
      * @param xContext contains the command processor that executes the
      *        command related to the request.
      *
      * @param eError is the error code to pass along with the request.
      *
      * qparam rArgs are additional parameters according to the specification
      *        of the error code. Refer to com/sun/star/ucb/IOErrorCode.idl
      *        for details.
      */
    SimpleIOErrorRequest( const com::sun::star::ucb::IOErrorCode eError,
                          const com::sun::star::uno::Sequence<
                            com::sun::star::uno::Any > & rArgs,
                          const rtl::OUString & rMessage,
                          const com::sun::star::uno::Reference<
                            com::sun::star::ucb::XCommandProcessor > & xContext
                         );
};

} // namespace ucbhelper

#endif /* !_UCBHELPER_SIMPLEIOERRORREQUEST_HXX */
