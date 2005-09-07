/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: registry.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:36:42 $
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
#define _REGISTRY_HXX_

#include <windows.h>
#include <memory>
#include <vector>
#include <string>

#include "registryvalueimpl.hxx"

//---------------------------------------
// forward declaration
//---------------------------------------

class RegistryKeyImpl;

//---------------------------------------
// typedefs
//---------------------------------------

typedef std::auto_ptr<RegistryKeyImpl>      RegistryKey;
typedef std::vector<std::wstring>               StringList;
typedef std::auto_ptr<StringList>               StringListPtr;

//---------------------------------------
//
//---------------------------------------

class RegistryKeyImpl
{
public:

    //############################################
    // Destruction
    //############################################

    virtual ~RegistryKeyImpl();


    //############################################
    // Queries
    //############################################


    /** The name of the key at hand, maybe empty
        if this is any of the root keys
    */
    std::wstring GetName() const;

    /** The number of sub values of the key at hand

        @precond IsOpen = true

        @throws
    */
    virtual size_t GetSubValueCount() const = 0;

    /** The number of sub-keys of the key at hand

        @precond IsOpen = true

        @throws
    */
    virtual size_t GetSubKeyCount() const = 0;

    bool IsOpen() const;

    /** Do we have write access on the key at hand
    */
    bool IsWriteable() const;

    /** The StringList will be allocated on the heap,
        so this is in fact a transfer of ownership
        to the caller

        @precond IsOpen = true

        @throws RegistryIOException
    */
    virtual StringListPtr GetSubKeyNames() const = 0;

    /** The StringList will be allocated on the heap,
        so this is in fact a transfer of ownership
        to the caller

        @precond IsOpen = true

        @throws RegistryIOException
    */
    virtual StringListPtr GetSubValueNames() const = 0;

    /** Get the specified registry value

        @precond IsOpen = true
    */
    virtual RegistryValue GetValue(const std::wstring& Name) const = 0;

    /** Get the specified registry value, return the given
        default value if value not found

        @precond IsOpen = true
    */
    virtual RegistryValue GetValue(const std::wstring& Name, const RegistryValue& Default) const = 0;

    /** Convenience function to determine if the
        Registry key at hand has the specified
        value

        @precond IsOpen = true

        throws RegistryAccessDenyException
    */
    bool HasValue(const std::wstring& Name) const;

    /** Convenience function to determine if the
        Registry key at hand has the specified
        sub-key

        @precond IsOpen = true

        throws RegistryAccessDenyException
    */
    bool HasSubKey(const std::wstring& Name) const;


    //############################################
    // Commands
    //############################################


    /** Open the registry key, has no effect if
        the key is already open

        @precond IsOpen = false

        @throws RegistryWriteAccessDenyException
                RegistryAccessDenyException
    */
    virtual void Open(bool Writeable = true) = 0;

    /** Close the registry key at hand, further
        using it without re-opening may cause
        RegistryIOExceptions to be thrown

        This is a template method that calls
        ImplClose which has to be overwritten
        by sub-classes
    */
    void Close();

    /** Open the specified sub-key of the registry key
        at hand

        @precond IsOpen = true
                 HasSubKey(Name) = true

        @throws RegistryIOException
                RegistryKeyNotFoundException
                RegistryAccessDeniedException
    */
    virtual RegistryKey OpenSubKey(const std::wstring& Name, bool Writeable = true) = 0;

    /** Creates a new sub-key below the key at hand

        @precond IsOpen = true
                 IsWriteable = true

        @throws  RegistryIOException
                 RegistryWriteAccessDenyException
    */
    virtual RegistryKey CreateSubKey(const std::wstring& Name) = 0;

    /** Deletes a sub-key below the key at hand, the
        key must not have sub-keys

        @precond IsOpen = true
                 IsWriteable = true

        @throws  RegistryIOException
                 RegistryWriteAccessDenyException
    */
    virtual void DeleteSubKey(const std::wstring& Name) = 0;

    /** Deletes a sub-key below the key at hand with all
        its sub-keys

        @precond IsOpen = true
                 IsWriteable = true;

        @throws  RegistryIOException
                 RegistryWriteAccessDenyException
    */
    virtual void DeleteSubKeyTree(const std::wstring& Name) = 0;

    /** Delete the specified value

        @precond IsOpen = true
                 IsWriteable = true
                 HasValue(Name) = true

        @throws RegistryIOException
                RegistryWriteAccessDeniedException
                RegistryValueNotFoundException
    */
    virtual void DeleteValue(const std::wstring& Name) = 0;

    /** Set the specified registry value

        @precond IsOpen = true
                 IsWriteable = true

        @throws  RegistryIOException
                 RegistryWriteAccessDenyException
    */
    virtual void SetValue(const RegistryValue& Value) = 0;


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
    virtual void CopyValue(const RegistryKey& RegistryKey, const std::wstring& Name);

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
    virtual void CopyValue(const RegistryKey& RegistryKey, const std::wstring& Name, const std::wstring& NewName);

    //############################################
    // Creation
    // only possible through WindowsRegistry class
    //############################################


protected:
    /** Create instance of the specified Registry key

        @throws  RegistryWriteAccessDenyException
                 RegistryAccessDenyException
                 RegistryKeyNotFoundException
    */
    RegistryKeyImpl(HKEY RootKey, const std::wstring& KeyName);

    /** Create instance of the specified Registry key.
        RootKey should only one of the predefined
        keys HKEY_CLASSES_ROOT, HKEY_CURRENT_USER,
        HKEY_LOCAL_MACHINE, HKEY_USERS

        @throws  RegistryWriteAccessDenyException
                 RegistryAccessDenyException
                 RegistryKeyNotFoundException
    */
    RegistryKeyImpl(HKEY RootKey);

    /** Create an instances of the specified Registry key,
        the key is assumed to be already opened.
    */
    RegistryKeyImpl(HKEY RootKey, HKEY SubKey, const std::wstring& KeyName, bool Writeable = true);

    /** Is this one of the root keys
        HKEY_CLASSES_ROOT
        HKEY_CURRENT_USER
        etc.
    */
    bool IsRootKey() const;

protected:
    HKEY            m_hRootKey;
    HKEY            m_hSubKey;
    std::wstring    m_KeyName;
    bool                m_IsWriteable;

// prevent copy and assignment
private:
    RegistryKeyImpl(const RegistryKeyImpl&);
    RegistryKeyImpl& operator=(const RegistryKeyImpl&);

//######################################
// Friend declarations
//######################################

friend class WindowsRegistry;
};

#endif
