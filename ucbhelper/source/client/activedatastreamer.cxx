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
#include "ucbhelper/activedatastreamer.hxx"

using namespace com::sun::star;

namespace ucbhelper
{

//=========================================================================
//=========================================================================
//
// ActiveDataStreamer Implementation.
//
//=========================================================================
//=========================================================================

//=========================================================================
//
// XInterface methods
//
//=========================================================================

XINTERFACE_IMPL_2( ActiveDataStreamer,
                   lang::XTypeProvider,
                   io::XActiveDataStreamer );

//=========================================================================
//
// XTypeProvider methods
//
//=========================================================================

XTYPEPROVIDER_IMPL_2( ActiveDataStreamer,
                      lang::XTypeProvider,
                      io::XActiveDataStreamer );

//=========================================================================
//
// XActiveDataStreamer methods.
//
//=========================================================================

// virtual
void SAL_CALL ActiveDataStreamer::setStream( const uno::Reference< io::XStream >& xStream )
    throw( uno::RuntimeException )
{
    m_xStream = xStream;
}

//=========================================================================
// virtual
uno::Reference< io::XStream > SAL_CALL ActiveDataStreamer::getStream()
    throw( uno::RuntimeException )
{
    return m_xStream;
}

} // namespace ucbhelper
