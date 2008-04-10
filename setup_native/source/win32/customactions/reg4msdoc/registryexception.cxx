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
