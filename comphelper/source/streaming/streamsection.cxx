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
#include "precompiled_comphelper.hxx"
#include <comphelper/streamsection.hxx>
#include <osl/diagnose.h>

namespace comphelper
{

//-------------------------------------------------------------------------
OStreamSection::OStreamSection(const staruno::Reference< stario::XDataInputStream >& _rxInput)
    :m_xMarkStream(_rxInput, ::com::sun::star::uno::UNO_QUERY)
    ,m_xInStream(_rxInput)
    ,m_nBlockStart(-1)
    ,m_nBlockLen(-1)
{
    OSL_ENSURE(m_xInStream.is() && m_xMarkStream.is(), "OStreamSection::OStreamSection : invalid argument !");
    if (m_xInStream.is() && m_xMarkStream.is())
    {
        m_nBlockLen = _rxInput->readLong();
        m_nBlockStart = m_xMarkStream->createMark();
    }
}

//-------------------------------------------------------------------------
OStreamSection::OStreamSection(const staruno::Reference< stario::XDataOutputStream >& _rxOutput, sal_Int32 _nPresumedLength)
    :m_xMarkStream(_rxOutput, ::com::sun::star::uno::UNO_QUERY)
    ,m_xOutStream(_rxOutput)
    ,m_nBlockStart(-1)
    ,m_nBlockLen(-1)
{
    OSL_ENSURE(m_xOutStream.is() && m_xMarkStream.is(), "OStreamSection::OStreamSection : invalid argument !");
    if (m_xOutStream.is() && m_xMarkStream.is())
    {
        m_nBlockStart = m_xMarkStream->createMark();
        // a placeholder where we will write the overall length (within the destructor)
        if (_nPresumedLength > 0)
            m_nBlockLen = _nPresumedLength + sizeof(m_nBlockLen);
            // as the caller did not consider - of course - the placeholder we are going to write
        else
            m_nBlockLen = 0;
        m_xOutStream->writeLong(m_nBlockLen);
    }
}

//-------------------------------------------------------------------------
OStreamSection::~OStreamSection()
{
    try
    {   // don't allow any exceptions to leave this block, this may be called during the stack unwinding of an exception
        // handling routing
        if (m_xInStream.is() &&  m_xMarkStream.is())
        {   // we're working on an input stream
            m_xMarkStream->jumpToMark(m_nBlockStart);
            m_xInStream->skipBytes(m_nBlockLen);
            m_xMarkStream->deleteMark(m_nBlockStart);
        }
        else if (m_xOutStream.is() && m_xMarkStream.is())
        {
            sal_Int32 nRealBlockLength = m_xMarkStream->offsetToMark(m_nBlockStart) - sizeof(m_nBlockLen);
            if (m_nBlockLen && (m_nBlockLen == nRealBlockLength))
                // nothing to do : the estimation the caller gave us (in the ctor) was correct
                m_xMarkStream->deleteMark(m_nBlockStart);
            else
            {   // the estimation was wrong (or we didn't get one)
                m_nBlockLen = nRealBlockLength;
                m_xMarkStream->jumpToMark(m_nBlockStart);
                m_xOutStream->writeLong(m_nBlockLen);
                m_xMarkStream->jumpToFurthest();
                m_xMarkStream->deleteMark(m_nBlockStart);
            }
        }
    }
    catch(const staruno::Exception&)
    {
    }
}
// -----------------------------------------------------------------------------
sal_Int32 OStreamSection::available()
{
    sal_Int32 nBytes = 0;
    try
    {   // don't allow any exceptions to leave this block, this may be called during the stack unwinding of an exception
        if (m_xInStream.is() &&  m_xMarkStream.is())
            nBytes = m_xMarkStream->offsetToMark(m_nBlockStart) - sizeof(m_nBlockLen);
    }
    catch(const staruno::Exception&)
    {
    }
    return nBytes;
}
// -----------------------------------------------------------------------------

}   // namespace comphelper


