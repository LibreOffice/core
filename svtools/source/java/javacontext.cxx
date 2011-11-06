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
#include "precompiled_svtools.hxx"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Type.hxx"
#include <svtools/svtdata.hxx>
#include <svtools/javacontext.hxx>
#include <svtools/javainteractionhandler.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::task;
namespace svt
{

JavaContext::JavaContext( const Reference< XCurrentContext > & ctx )
    :
    m_aRefCount(0),
    m_xNextContext( ctx ),
    m_bShowErrorsOnce(false)
{
}

JavaContext::JavaContext( const Reference< XCurrentContext > & ctx,
                          bool bShowErrorsOnce)
    : m_aRefCount(0),
      m_xNextContext( ctx ),
      m_bShowErrorsOnce(bShowErrorsOnce)
{
}

JavaContext::~JavaContext()
{
}

Any SAL_CALL JavaContext::queryInterface(const Type& aType )
    throw (RuntimeException)
{
    if (aType == getCppuType(reinterpret_cast<Reference<XInterface>*>(0)))
        return Any(Reference<XInterface>(static_cast<XInterface*>(this)));
    else if (aType == getCppuType(reinterpret_cast<Reference<XCurrentContext>*>(0)))
        return Any(Reference<XCurrentContext>( static_cast<XCurrentContext*>(this)));
    return Any();
}

void SAL_CALL JavaContext::acquire(  ) throw ()
{
    osl_incrementInterlockedCount( &m_aRefCount );
}

void SAL_CALL JavaContext::release(  ) throw ()
{
    if (! osl_decrementInterlockedCount( &m_aRefCount ))
        delete this;
}

Any SAL_CALL JavaContext::getValueByName( const ::rtl::OUString& Name) throw (RuntimeException)
{
    Any retVal;

    if ( 0 == Name.compareToAscii( JAVA_INTERACTION_HANDLER_NAME ))
    {
        {
            osl::MutexGuard aGuard(osl::Mutex::getGlobalMutex());
            if (!m_xHandler.is())
                m_xHandler = Reference< XInteractionHandler >(
                    new JavaInteractionHandler(m_bShowErrorsOnce));
        }
        retVal = makeAny(m_xHandler);

    }
    else if( m_xNextContext.is() )
    {
        // Call next context in chain if found
        retVal = m_xNextContext->getValueByName( Name );
    }
    return retVal;
}


}
