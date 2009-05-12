/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: registrywnt.cxx,v $
 * $Revision: 1.7 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

//---------------------------------------
//
//---------------------------------------

#ifdef _MSC_VER
#pragma warning(push, 1) /* disable warnings within system headers */
#endif
#include <windows.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <malloc.h>
#include "registrywnt.hxx"
#include "registryvalueimpl.hxx"
#include "registryexception.hxx"

#include <assert.h>

#ifdef _MSC_VER
#pragma warning(disable : 4786 4350)
#endif

//---------------------------------------
//
//---------------------------------------

const size_t MAX_TMP_BUFF_SIZE = 1024 * sizeof(wchar_t);


//############################################
// Creation
// only possible through WindowsRegistry class
//############################################


//-----------------------------------------------------
/** Create instance and open the specified Registry key
*/
RegistryKeyImplWinNT::RegistryKeyImplWinNT(HKEY RootKey, const std::wstring& KeyName) :
    RegistryKeyImpl(RootKey, KeyName)
{
}

//-----------------------------------------------------
/** Create instance and open the specified Registry key
*/
RegistryKeyImplWinNT::RegistryKeyImplWinNT(HKEY RootKey) :
    RegistryKeyImpl(RootKey)
{
}

//-----------------------------------------------------
/** Create an instances of the specified Registry key,
    the key is assumed to be already opened.
*/
RegistryKeyImplWinNT::RegistryKeyImplWinNT(HKEY RootKey, HKEY SubKey, const std::wstring& KeyName, bool Writeable) :
    RegistryKeyImpl(RootKey, SubKey, KeyName, Writeable)
{
}


//############################################
// Queries
//############################################


//-----------------------------------------------------
/** The number of sub values of the key at hand

    @precond IsOpen = true

    @throws
*/
size_t RegistryKeyImplWinNT::GetSubValueCount() const
{
    assert(IsOpen());

    DWORD nSubValues = 0;

    LONG rc = RegQueryInfoKeyW(
        m_hSubKey,
        0, 0, 0, 0, 0, 0, &nSubValues, 0, 0, 0, 0);

    if (ERROR_INVALID_HANDLE == rc)
        throw RegistryIOException(rc);
    else if (ERROR_SUCCESS != rc)
        throw RegistryException(rc);

    return nSubValues;
}

//-----------------------------------------------------
/** The number of sub-keys of the key at hand

    @precond IsOpen = true

    @throws
*/
size_t RegistryKeyImplWinNT::GetSubKeyCount() const
{
    assert(IsOpen());

    DWORD nSubKeys = 0;

    LONG rc = RegQueryInfoKeyA(
        m_hSubKey,
        0, 0, 0, &nSubKeys, 0, 0, 0, 0, 0, 0, 0);

    if (ERROR_INVALID_HANDLE == rc)
        throw RegistryIOException(rc);
    else if (ERROR_SUCCESS != rc)
        throw RegistryException(rc);

    return nSubKeys;
}

//-----------------------------------------------------
/**
*/
StringListPtr RegistryKeyImplWinNT::GetSubKeyNames() const
{
    assert(IsOpen());

    wchar_t buff[1024];
    DWORD  buff_size = sizeof(buff);
    FILETIME ftime;

    StringList* key_names = new StringList();

    LONG rc = ERROR_SUCCESS;

    for (DWORD i = 0; /* left empty */; i++)
    {
        rc = RegEnumKeyExW(
            m_hSubKey, i, buff, &buff_size,
            0, 0, 0, &ftime);

        if (ERROR_SUCCESS != rc &&
            ERROR_MORE_DATA != rc)
            break;

        buff_size = sizeof(buff);

        key_names->push_back(buff);
    }

    if (ERROR_INVALID_HANDLE == rc)
        throw RegistryIOException(rc);
    else if (ERROR_NO_MORE_ITEMS != rc && ERROR_SUCCESS != rc)
        throw RegistryException(rc);

#if (_MSC_VER < 1300) && !defined(__MINGW32__)
    return key_names;
#else
    return (StringListPtr) key_names;
#endif
}

//-----------------------------------------------------
/**
*/
StringListPtr RegistryKeyImplWinNT::GetSubValueNames() const
{
    assert(IsOpen());

    wchar_t buff[1024];
    DWORD  buff_size = sizeof(buff);

    StringList* value_names = new StringList();

    LONG rc = ERROR_SUCCESS;

    for (DWORD i = 0; /* left empty */; i++)
    {
        rc = RegEnumValueW(
            m_hSubKey, i, buff, &buff_size,
            0, 0, 0, 0);

        if (ERROR_SUCCESS != rc &&
            ERROR_MORE_DATA != rc)
            break;

        buff_size = sizeof(buff);

        value_names->push_back(buff);
    }

    if (ERROR_INVALID_HANDLE == rc)
        throw RegistryIOException(rc);
    else if (ERROR_NO_MORE_ITEMS != rc && ERROR_SUCCESS != rc)
        throw RegistryException(rc);

#if (_MSC_VER < 1300) && !defined(__MINGW32__)
    return value_names;
#else
    return (StringListPtr) value_names;
#endif
}

//-----------------------------------------------------
/** Get the specified registry value

    @precond IsOpen = true
*/
RegistryValue RegistryKeyImplWinNT::GetValue(const std::wstring& Name) const
{
    assert(IsOpen());

    DWORD Type;
    wchar_t buff[MAX_TMP_BUFF_SIZE];
    DWORD   size = sizeof(buff);

    LONG rc = RegQueryValueExW(
        m_hSubKey,
        Name.c_str(),
        0,
        &Type,
        reinterpret_cast<LPBYTE>(buff),
        &size);

    if (ERROR_FILE_NOT_FOUND == rc)
        throw RegistryValueNotFoundException(rc);
    else if (ERROR_ACCESS_DENIED == rc)
        throw RegistryAccessDeniedException(rc);
    else if (ERROR_SUCCESS != rc)
        throw RegistryException(rc);

    RegistryValue regval;

    if (REG_DWORD == Type)
    {
        regval = RegistryValue(new RegistryValueImpl(Name, *(reinterpret_cast<int*>(buff))));
    }
    else if (REG_SZ == Type || REG_EXPAND_SZ == Type || REG_MULTI_SZ == Type)
    {
        if (size > 0)
            regval = RegistryValue(new RegistryValueImpl(Name, std::wstring(reinterpret_cast<wchar_t*>(buff))));
        else
            regval = RegistryValue(new RegistryValueImpl(Name, std::wstring()));
    }
    else
    {
        assert(false);
    }

    return regval;
}

//-----------------------------------------------------
/** Get the specified registry value, return the given
    default value if value not found

    @precond IsOpen = true
*/
RegistryValue RegistryKeyImplWinNT::GetValue(const std::wstring& Name, const RegistryValue& Default) const
{
    assert(IsOpen());

    DWORD Type;
    wchar_t buff[MAX_TMP_BUFF_SIZE];
    DWORD   size = sizeof(buff);

    LONG rc = RegQueryValueExW(
        m_hSubKey,
        Name.c_str(),
        0,
        &Type,
        reinterpret_cast<LPBYTE>(buff),
        &size);

    if (ERROR_FILE_NOT_FOUND == rc)
    {
        #if (_MSC_VER < 1300) && !defined(__MINGW32__)
        return Default;
        #else
        RegistryValue regval_ptr;
        regval_ptr = RegistryValue(new RegistryValueImpl(*Default));
        return regval_ptr;
        #endif
    }

    if (ERROR_ACCESS_DENIED == rc)
        throw RegistryAccessDeniedException(rc);
    else if (ERROR_SUCCESS != rc)
        throw RegistryException(rc);

    RegistryValue regval;

    if (REG_DWORD == Type)
        regval = RegistryValue(new RegistryValueImpl(Name, *reinterpret_cast<int*>(buff)));
    else if (REG_SZ == Type || REG_EXPAND_SZ == Type || REG_MULTI_SZ == Type)
        regval = RegistryValue(new RegistryValueImpl(Name, std::wstring(reinterpret_cast<wchar_t*>(buff))));
    else
        assert(false);

    return regval;
}


//############################################
// Commands
//############################################


//-----------------------------------------------------
/** Open the registry key, has no effect if
    the key is already open

    @precond IsOpen = false

    @throws RegistryKeyNotFoundException
            RegistryWriteAccessDenyException
            RegistryAccessDenyException
*/
void RegistryKeyImplWinNT::Open(bool Writeable)
{
    assert(!IsOpen());

    REGSAM regsam = KEY_READ;

    if (Writeable)
        regsam |= KEY_WRITE;

    LONG rc = RegOpenKeyExW(
        m_hRootKey,
        m_KeyName.c_str(),
        0,
        regsam,
        &m_hSubKey);

    if (ERROR_FILE_NOT_FOUND == rc)
        throw RegistryKeyNotFoundException(rc);
    else if (ERROR_ACCESS_DENIED == rc)
        throw RegistryAccessDeniedException(rc);
    else if (ERROR_SUCCESS != rc)
        throw RegistryException(rc);

    m_IsWriteable = Writeable;

    assert(IsOpen());
}

//-----------------------------------------------------
/** Open the specified sub-key of the registry key
    at hand

    @precond IsOpen = true
             HasSubKey(Name) = true

    @throws RegistryIOException
            RegistryKeyNotFoundException
            RegistryAccessDeniedException
*/
RegistryKey RegistryKeyImplWinNT::OpenSubKey(const std::wstring& Name, bool Writeable)
{
    RegistryKey regkey(new RegistryKeyImplWinNT(m_hSubKey, Name));
    regkey->Open(Writeable);
    return regkey;
}

//-----------------------------------------------------
/** Creates a new sub-key below the key at hand

    @precond IsOpen = true
             IsWriteable = true

    @throws  RegistryIOException
             RegistryWriteAccessDenyException
*/

RegistryKey RegistryKeyImplWinNT::CreateSubKey(const std::wstring& Name)
{
    assert(IsOpen());
    assert(IsWriteable());

    HKEY hRoot = IsRootKey() ? m_hRootKey : m_hSubKey;

    HKEY hKey;

    LONG rc = RegCreateKeyExW(
        hRoot,
        Name.c_str(),
        0,
        0,
        REG_OPTION_NON_VOLATILE,
        KEY_READ | KEY_WRITE,
        0,
        &hKey,
        0);

    if (ERROR_INVALID_HANDLE == rc)
        throw RegistryIOException(rc);
    else if (ERROR_ACCESS_DENIED == rc)
        throw RegistryAccessDeniedException(rc);
    else if (ERROR_SUCCESS != rc)
        throw RegistryException(rc);

    return RegistryKey(new RegistryKeyImplWinNT(hRoot, hKey, Name));
}

//-----------------------------------------------------
/** Deletes a sub-key below the key at hand, the
    key must not have sub-keys

    @precond IsOpen = true
             IsWriteable = true

    @throws  RegistryIOException
             RegistryWriteAccessDenyException
*/
void RegistryKeyImplWinNT::DeleteSubKey(const std::wstring& Name)
{
    assert(IsOpen());
    assert(IsWriteable());
    assert(HasSubKey(Name));

    RegistryKey SubKey = OpenSubKey(Name);

    size_t nSubKeyCount = SubKey->GetSubKeyCount();

    assert(0 == nSubKeyCount);

    if (nSubKeyCount)
        throw RegistryInvalidOperationException(ERROR_NOT_SUPPORTED);

    LONG rc = RegDeleteKeyW(m_hSubKey, Name.c_str());

    if (ERROR_INVALID_HANDLE == rc)
        throw RegistryIOException(rc);
    else if (ERROR_ACCESS_DENIED == rc)
        throw RegistryAccessDeniedException(rc);
    else if (ERROR_SUCCESS != rc)
        throw RegistryException(rc);
}

//-----------------------------------------------------
/** Deletes a sub-key below the key at hand with all
    its sub-keys

    @precond IsOpen = true
             IsWriteable = true;

    @throws  RegistryIOException
             RegistryWriteAccessDenyException
*/
void RegistryKeyImplWinNT::DeleteSubKeyTree(const std::wstring& Name)
{
    ImplDeleteSubKeyTree(m_hSubKey, Name);
}

//-----------------------------------------------------
/** Deletes a sub-key below the key at hand with all
    its sub-keys

    @precond IsOpen = true
             IsWriteable = true;

    @throws  RegistryIOException
             RegistryWriteAccessDenyException
*/
LONG RegistryKeyImplWinNT::ImplDeleteSubKeyTree(HKEY RootKey, const std::wstring& Name)
{
    assert(IsOpen());

    HKEY hKey;

    LONG rc = RegOpenKeyExW(
        RootKey,
        Name.c_str(),
        0,
        KEY_READ | DELETE,
        &hKey);

    if (ERROR_SUCCESS == rc)
    {
        wchar_t* lpSubKey;
        DWORD    nMaxSubKeyLen;

        rc = RegQueryInfoKeyW(
            hKey, 0, 0, 0, 0,
            &nMaxSubKeyLen,
            0, 0, 0, 0, 0, 0);

        nMaxSubKeyLen++; // space for trailing '\0'

        lpSubKey = reinterpret_cast<wchar_t*>(
            _alloca(nMaxSubKeyLen*sizeof(wchar_t)));

        while (ERROR_SUCCESS == rc)
        {
            DWORD nLen = nMaxSubKeyLen;

            rc = RegEnumKeyExW(
                hKey,
                0,       // always index zero
                lpSubKey,
                &nLen,
                0, 0, 0, 0);

            if (ERROR_NO_MORE_ITEMS == rc)
            {
                rc = RegDeleteKeyW(RootKey, Name.c_str());
                break;
            }
            else if (rc == ERROR_SUCCESS)
            {
                rc = ImplDeleteSubKeyTree(hKey, lpSubKey);
            }

        } // while

        RegCloseKey(hKey);

    } // if

    if (ERROR_INVALID_HANDLE == rc)
        throw RegistryIOException(rc);
    else if (ERROR_ACCESS_DENIED == rc)
        throw RegistryAccessDeniedException(rc);
    else if (ERROR_FILE_NOT_FOUND == rc)
        throw RegistryKeyNotFoundException(rc);
    else if (ERROR_SUCCESS != rc)
        throw RegistryException(rc);

    return rc;
}

//-----------------------------------------------------
/** Delete the specified value

        @precond IsOpen = true
                 IsWriteable = true
                 HasValue(Name) = true

        @throws RegistryIOException
                RegistryWriteAccessDeniedException
                RegistryValueNotFoundException
*/
void RegistryKeyImplWinNT::DeleteValue(const std::wstring& Name)
{
    assert(IsOpen());
    assert(HasValue(Name));
    assert(IsWriteable());

    LONG rc = RegDeleteValueW(
        m_hSubKey,
        Name.c_str());

    if (ERROR_INVALID_HANDLE == rc)
        throw RegistryIOException(rc);
    else if (ERROR_ACCESS_DENIED == rc)
        throw RegistryNoWriteAccessException(rc);
    else if (ERROR_FILE_NOT_FOUND == rc)
        throw RegistryValueNotFoundException(rc);
    else if (ERROR_SUCCESS != rc)
        throw RegistryException(rc);
}

//-----------------------------------------------------
/** Set the specified registry value

    @precond IsOpen = true
             IsWriteable = true

    @throws  RegistryIOException
             RegistryWriteAccessDenyException
*/
void RegistryKeyImplWinNT::SetValue(const RegistryValue& Value)
{
    assert(IsOpen());
    assert(IsWriteable());

    LONG rc = RegSetValueExW(
        m_hSubKey,
        Value->GetName().c_str(),
        0,
        Value->GetType(),
        reinterpret_cast<const unsigned char*>(Value->GetDataBuffer()),
        static_cast<DWORD>(Value->GetDataSize()));

    if (ERROR_INVALID_HANDLE == rc)
        throw RegistryIOException(rc);
    else if (ERROR_ACCESS_DENIED == rc)
        throw RegistryAccessDeniedException(rc);
    else if (ERROR_SUCCESS != rc)
        throw RegistryException(rc);
}




