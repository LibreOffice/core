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

// RegistryException.cpp: Implementierung der Klasse RegistryException.
//
//////////////////////////////////////////////////////////////////////

#include "registryexception.hxx"

#ifdef _MSC_VER
#pragma warning(push, 1) /* disable warnings within system headers */
#endif
#include <windows.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

RegistryException::RegistryException(long ErrorCode) :
    m_ErrorCode(ErrorCode),
    m_ErrorMsg(0)
{
}

/**
*/
RegistryException::~RegistryException() throw()
{
    if (m_ErrorMsg)
        LocalFree(m_ErrorMsg);
}

/**
*/
const char* RegistryException::what() const throw()
{
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        m_ErrorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR) &m_ErrorMsg,
        0,
        NULL);

    return reinterpret_cast<char*>(m_ErrorMsg);
}

/**
*/
long RegistryException::GetErrorCode() const
{
    return m_ErrorCode;
}

//#######################################
// Thrown when a Registry key is accessed
// that is closed
//#######################################

RegistryIOException::RegistryIOException(long ErrorCode) :
    RegistryException(ErrorCode)
{
};

//#######################################
//
//#######################################

RegistryNoWriteAccessException::RegistryNoWriteAccessException(long ErrorCode) :
    RegistryException(ErrorCode)
{
};

//#######################################
//
//#######################################

RegistryAccessDeniedException::RegistryAccessDeniedException(long ErrorCode) :
    RegistryException(ErrorCode)
{
};

//#######################################
//
//#######################################

RegistryValueNotFoundException::RegistryValueNotFoundException(long ErrorCode) :
    RegistryException(ErrorCode)
{
};

//#######################################
//
//#######################################

RegistryKeyNotFoundException::RegistryKeyNotFoundException(long ErrorCode) :
    RegistryException(ErrorCode)
{
};

//#######################################
//
//#######################################

RegistryInvalidOperationException::RegistryInvalidOperationException(long ErrorCode) :
    RegistryException(ErrorCode)
{
};
