// RegistryException.cpp: Implementierung der Klasse RegistryException.
//
//////////////////////////////////////////////////////////////////////

#ifndef _REGISTRYEXCEPTION_HXX_
#include "registryexception.hxx"
#endif

#include <windows.h>

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
RegistryException::~RegistryException()
{
    if (m_ErrorMsg)
        LocalFree(m_ErrorMsg);
}

/**
*/
const char* RegistryException::what() const
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
