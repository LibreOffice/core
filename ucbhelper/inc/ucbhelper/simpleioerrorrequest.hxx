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



#ifndef _UCBHELPER_SIMPLEIOERRORREQUEST_HXX
#define _UCBHELPER_SIMPLEIOERRORREQUEST_HXX

#include <com/sun/star/ucb/IOErrorCode.hpp>
#include <ucbhelper/interactionrequest.hxx>

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
