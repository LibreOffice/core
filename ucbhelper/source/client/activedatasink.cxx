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
#include "precompiled_ucbhelper.hxx"

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#include "ucbhelper/activedatasink.hxx"

using namespace com::sun::star;

namespace ucbhelper
{

//=========================================================================
//=========================================================================
//
// ActiveDataSink Implementation.
//
//=========================================================================
//=========================================================================

//=========================================================================
//
// XInterface methods
//
//=========================================================================

XINTERFACE_IMPL_2( ActiveDataSink,
                   lang::XTypeProvider,
                   io::XActiveDataSink );

//=========================================================================
//
// XTypeProvider methods
//
//=========================================================================

XTYPEPROVIDER_IMPL_2( ActiveDataSink,
                      lang::XTypeProvider,
                      io::XActiveDataSink );

//=========================================================================
//
// XActiveDataSink methods.
//
//=========================================================================

// virtual
void SAL_CALL ActiveDataSink::setInputStream(
                        const uno::Reference< io::XInputStream >& aStream )
    throw( uno::RuntimeException )
{
    m_xStream = aStream;
}

//=========================================================================
// virtual
uno::Reference< io::XInputStream > SAL_CALL ActiveDataSink::getInputStream()
    throw( uno::RuntimeException )
{
    return m_xStream;
}

} // namespace ucbhelper
