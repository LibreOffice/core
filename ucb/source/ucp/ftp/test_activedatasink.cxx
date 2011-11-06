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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#ifndef _TEST_ACTIVEDATASINK_HXX_
#include "test_activedatasink.hxx"
#endif


using namespace test_ftp;
using namespace com::sun::star::uno;
using namespace com::sun::star::io;


Any SAL_CALL Test_ActiveDataSink::queryInterface( const Type& rType ) throw( RuntimeException ) {
    Any aRet = ::cppu::queryInterface(rType,
                                      SAL_STATIC_CAST( XActiveDataSink*,this ));

    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}



void SAL_CALL Test_ActiveDataSink::acquire( void ) throw() {
    OWeakObject::acquire();
}



void SAL_CALL Test_ActiveDataSink::release( void ) throw() {
    OWeakObject::release();
}
