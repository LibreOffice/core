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



/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#ifndef _TEST_ACTIVEDATASINK_HXX_
#define _TEST_ACTIVEDATASINK_HXX_

#include <cppuhelper/weak.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <com/sun/star/task/XInteractionHandler.hpp>

namespace test_ftp {


    class Test_InteractionHandler
        : public cppu::OWeakObject,
          public com::sun::star::task::XInteractionHandler
    {
    public:

        // XInterface

        virtual com::sun::star::uno::Any SAL_CALL
        queryInterface( const com::sun::star::uno::Type& rType )
            throw( com::sun::star::uno::RuntimeException );


        virtual void SAL_CALL acquire( void ) throw();

        virtual void SAL_CALL release( void ) throw();


        // XInteractionHandler

        virtual void SAL_CALL
        handle( const ::com::sun::star::uno::Reference<
                ::com::sun::star::task::XInteractionRequest >& Request )
            throw (::com::sun::star::uno::RuntimeException);

    private:

    };

}


#endif
