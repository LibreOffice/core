// RegistryValueImpl.cpp: Implementierung der Klasse RegistryValueImpl.
//
//////////////////////////////////////////////////////////////////////

#include "registryvalueimpl.hxx"

#ifdef _MSC_VER
#pragma warning(push, 1) /* disable warnings within system headers */
#endif
#include <windows.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <malloc.h>
#include <assert.h>

#include "stringconverter.hxx"

//#################################
// Creation/Destruction
//#################################

//--------------------------------------------
/**
*/
RegistryValueImpl::RegistryValueImpl(const std::wstring& Name, int Value) :
    m_Name(Name),
    m_Type(REG_DWORD),
    m_IntData(Value)
{
}

//--------------------------------------------
/**
*/
RegistryValueImpl::RegistryValueImpl(const std::wstring& Name, const std::wstring& Value) :
    m_Name(Name),
    m_Type(REG_SZ),
    m_StringData(Value),
    m_IntData(0)
{
}

//--------------------------------------------
/**
*/
RegistryValueImpl::RegistryValueImpl(const std::wstring& Name, const std::string& Value) :
    m_Name(Name),
    m_Type(REG_SZ),
    m_IntData(0)
{
    m_StringData = AnsiToUnicodeString(Value);
}

#if (_MSC_VER >= 1300)
RegistryValueImpl::RegistryValueImpl(const RegistryValueImpl& s) :
    m_Name(s.m_Name),
    m_Type(s.m_Type),
    m_StringData(s.m_StringData),
    m_IntData(s.m_IntData)
    {
}
#endif
//--------------------------------------------
/**
*/
RegistryValueImpl::~RegistryValueImpl()
{
}

//#################################
// Query
//#################################

//--------------------------------------------
/** Returns the name of the value
*/
std::wstring RegistryValueImpl::GetName() const
{
    return m_Name;
}

//--------------------------------------------
/** Return the size of data held
*/
size_t RegistryValueImpl::GetDataSize() const
{
    size_t size = 0;

    if (REG_DWORD == m_Type)
        size = sizeof(m_IntData);
    else if (REG_SZ == m_Type)
        size = m_StringData.length() ? ((m_StringData.length() + 1) * sizeof(wchar_t)) : 0;

    return size;
}

//--------------------------------------------
/** Get a pointer to the data buffer
    in order to copy the data
*/
const void* RegistryValueImpl::GetDataBuffer() const
{
    const void* pData = 0;

    if (REG_DWORD == m_Type)
        pData = reinterpret_cast<const void*>(&m_IntData);
    else if (REG_SZ == m_Type)
        pData = reinterpret_cast<const void*>(m_StringData.c_str());

    return pData;
}

//--------------------------------------------
/** Returns the data as string
*/
std::wstring RegistryValueImpl::GetDataAsUniString() const
{
    assert(REG_SZ == m_Type);
    return m_StringData;
}

//--------------------------------------------
/** Returns the data as string
*/
std::string RegistryValueImpl::GetDataAsAnsiString() const
{
    assert(REG_SZ == m_Type);
    return UnicodeToAnsiString(m_StringData);
}

//--------------------------------------------
/** Returns the data as number
*/
int RegistryValueImpl::GetDataAsInt() const
{
    assert(REG_DWORD == m_Type);
    return m_IntData;
}

//--------------------------------------------
/** Returns the type of the data
*/
int RegistryValueImpl::GetType() const
{
    return m_Type;
}


//#################################
// Command
//#################################


//--------------------------------------------
/** Set a new name
*/
void RegistryValueImpl::SetName(const std::wstring& NewName)
{
    m_Name = NewName;
}

//--------------------------------------------
/**
*/
void RegistryValueImpl::SetValue(const std::wstring& NewValue)
{
    m_Type = REG_SZ;
    m_StringData = NewValue;
}

//--------------------------------------------
/**
*/
void RegistryValueImpl::SetValue(const std::string& NewValue)
{
    m_Type = REG_SZ;
    m_StringData = AnsiToUnicodeString(NewValue);
}

//--------------------------------------------
/**
*/
void RegistryValueImpl::SetValue(int NewValue)
{
    m_Type = REG_DWORD;
    m_IntData = NewValue;
}
