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

// RegistryException.h: Schnittstelle für die Klasse RegistryException.
//
//////////////////////////////////////////////////////////////////////

#ifndef _REGISTRYEXCEPTION_HXX_
#define _REGISTRYEXCEPTION_HXX_

#include <exception>

//#######################################
// Base class for all Registry exceptions
//#######################################

class RegistryException : public std::exception
{
public:

    RegistryException(long ErrorCode);

    virtual ~RegistryException() throw();

    /**
        @descr  Returns a string that describes the error if
                available, else NULL will be returned. The
                returned string is only temporary so the caller
                has to copy it if he needs the string further.
    */
    virtual const char* what() const throw();

    /**
        @descr  Returns the error code.
    */

    long GetErrorCode() const;

private:
    long    m_ErrorCode;
    void*  m_ErrorMsg;
};

//#######################################
// Thrown when a Registry key is accessed
// that is closed
//#######################################

class RegistryIOException : public RegistryException
{
public:
    RegistryIOException(long ErrorCode);
};

//#######################################
// Thrown when trying to write to a readonly registry key
//#######################################

class RegistryNoWriteAccessException : public RegistryException
{
public:
    RegistryNoWriteAccessException(long ErrorCode);
};

//#######################################
// Thrown when trying to access an registry key, with improper
// access rights
//#######################################

class RegistryAccessDeniedException : public RegistryException
{
public:
    RegistryAccessDeniedException(long ErrorCode);
};

//#######################################
// A specified registry value could not be read because it is not
// available
//#######################################

class RegistryValueNotFoundException : public RegistryException
{
public:
    RegistryValueNotFoundException(long ErrorCode);
};

//#######################################
// A specified registry key was not found
//#######################################

class RegistryKeyNotFoundException : public RegistryException
{
public:
    RegistryKeyNotFoundException(long ErrorCode);
};

//#######################################
// A specified registry operation is invalid
//#######################################

class RegistryInvalidOperationException : public RegistryException
{
public:
    RegistryInvalidOperationException(long ErrorCode);
};

#endif
