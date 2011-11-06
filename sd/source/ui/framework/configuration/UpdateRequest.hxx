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



#ifndef SD_FRAMEWORK_UPDATE_REQUEST_HXX
#define SD_FRAMEWORK_UPDATE_REQUEST_HXX

#include "MutexOwner.hxx"
#include <com/sun/star/drawing/framework/XConfigurationChangeRequest.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/drawing/framework/XConfiguration.hpp>
#include <cppuhelper/compbase2.hxx>


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
