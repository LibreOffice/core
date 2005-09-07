/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: registry.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:36:22 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _REGISTRY_HXX_
#include "registry.hxx"
#endif

#include <assert.h>
#include <algorithm>

#pragma warning(disable : 4786)

//-----------------------------------------------------
/** Create instance and open the specified Registry key

    @throws  RegistryWriteAccessDenyException
             RegistryAccessDenyException
             RegistryKeyNotFoundException
*/
RegistryKeyImpl::RegistryKeyImpl(HKEY RootKey, const std::wstring& KeyName) :
    m_hRootKey(RootKey),
    m_hSubKey(0),
    m_KeyName(KeyName),
    m_IsWriteable(false)
{
}

//-----------------------------------------------------
/** Create instance and open the specified Registry key

    @throws  RegistryWriteAccessDenyException
             RegistryAccessDenyException
             RegistryKeyNotFoundException
*/
RegistryKeyImpl::RegistryKeyImpl(HKEY RootKey) :
    m_hRootKey(RootKey),
    m_hSubKey(0),
    m_IsWriteable(false)
{
}

//-----------------------------------------------------
/** Create an instances of the specified Registry key,
    the key is assumed to be already opened.
*/
RegistryKeyImpl::RegistryKeyImpl(HKEY RootKey, HKEY SubKey, const std::wstring& KeyName, bool Writeable) :
    m_hRootKey(RootKey),
    m_hSubKey(SubKey),
    m_KeyName(KeyName),
    m_IsWriteable(Writeable)
{
}

//-----------------------------------------------------
/**
*/
RegistryKeyImpl::~RegistryKeyImpl()
{
    if (IsOpen())
        Close();
}


//############################################
// Queries
//############################################


//-----------------------------------------------------
/** The name of the key at hand, maybe empty
    if this is any of the root keys
*/
std::wstring RegistryKeyImpl::GetName() const
{
    return m_KeyName;
}

//-----------------------------------------------------
/**
*/
bool RegistryKeyImpl::IsOpen() const
{
    return m_hSubKey != 0;
}

//-----------------------------------------------------
/** Is this one of the root keys
    HKEY_CLASSES_ROOT
    HKEY_CURRENT_USER
    etc.
*/
bool RegistryKeyImpl::IsRootKey() const
{
    return (0 == m_KeyName.length());
}

//-----------------------------------------------------
/** Do we have write access on the key at hand
*/
bool RegistryKeyImpl::IsWriteable() const
{
    return m_IsWriteable;
}

//-----------------------------------------------------
/** Convenience function to determine if the
    Registry key at hand has the specified
    value

    @precond IsOpen = true

    throws RegistryAccessDenyException
*/
bool RegistryKeyImpl::HasValue(const std::wstring& Name) const
{
    StringListPtr names = GetSubValueNames();

    StringList::iterator iter_end = names->end();
    StringList::iterator iter = std::find(names->begin(), iter_end, Name);

    return (iter != iter_end);
}

//-----------------------------------------------------
/** Convenience function to determine if the
    Registry key at hand has the specified
    sub-key

    @precond IsOpen = true

    throws RegistryAccessDenyException
*/
bool RegistryKeyImpl::HasSubKey(const std::wstring& Name) const
{
    StringListPtr names = GetSubKeyNames();

    StringList::iterator iter_end = names->end();
    StringList::iterator iter = std::find(names->begin(), iter_end, Name);

    return (iter != iter_end);
}

//-----------------------------------------------------
/**
*/
void RegistryKeyImpl::Close()
{
    LONG rc = RegCloseKey(m_hSubKey);
    assert(ERROR_SUCCESS == rc);

    m_hSubKey = 0;
    m_IsWriteable = false;
}

//-----------------------------------------------------
/** Copies the specified value from RegistryKey to
    the registry key at hand, if a value with this
    name already exist under the registry key at hand
    it will be overwritten

    @precond IsOpen = true
             IsWriteable = true
             RegistryKey.HasSubValue(Name) = true

    @throws RegistryIOException
            RegistryWriteAccessDeniedException
            RegistryValueNotFoundException
*/
void RegistryKeyImpl::CopyValue(const RegistryKey& RegistryKey, const std::wstring& Name)
{
    assert(RegistryKey->HasValue(Name));
    SetValue(RegistryKey->GetValue(Name));
    assert(HasValue(Name));
}

/** Copies the specified value from RegistryKey to
    the registry key at hand under a new name,
    if a value with this name already exist there
    it will be overwritten

    @precond IsOpen = true
             IsWriteable = true
             RegistryKey.HasSubValue(Name) = true

    @throws RegistryIOException
            RegistryWriteAccessDeniedException
            RegistryValueNotFoundException
*/
void RegistryKeyImpl::CopyValue(const RegistryKey& RegistryKey, const std::wstring& Name, const std::wstring& NewName)
{
    assert(RegistryKey->HasValue(Name));

    RegistryValue RegVal = RegistryKey->GetValue(Name);
    RegVal->SetName(NewName);
    SetValue(RegVal);

    assert(HasValue(NewName));
}
