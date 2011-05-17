/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
