/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _REGISTRYWNT_HXX_
#define _REGISTRYWNT_HXX_

#include "registry.hxx"

//---------------------------------------
// constants
//---------------------------------------

class RegistryKeyImplWinNT : public RegistryKeyImpl
{
public:

    //############################################
    // Queries
    //############################################

    /** The number of sub values of the key at hand

        @precond IsOpen = true

        @throws
    */
    virtual size_t GetSubValueCount() const;

    /** The number of sub-keys of the key at hand

        @precond IsOpen = true

        @throws
    */
    virtual size_t GetSubKeyCount() const;

    virtual StringListPtr GetSubKeyNames() const;

    virtual StringListPtr GetSubValueNames() const;

    /** Get the specified registry value

        @precond IsOpen = true
    */
    virtual RegistryValue GetValue(const std::wstring& Name) const;

    /** Get the specified registry value, return the given
        default value if value not found

        @precond IsOpen = true
    */
    virtual RegistryValue GetValue(const std::wstring& Name, const RegistryValue& Default) const;

    //############################################
    // Commands
    //############################################

    /** Open the registry key, has no effect if
        the key is already open

        @precond IsOpen = false

        @throws RegistryWriteAccessDenyException
                RegistryAccessDenyException
    */
    virtual void Open(bool Writeable = true);

    /** Open the specified sub-key of the registry key
        at hand

        @precond IsOpen = true
                 HasSubKey(Name) = true

        @throws RegistryIOException
                RegistryKeyNotFoundException
                RegistryAccessDeniedException
    */
    virtual RegistryKey OpenSubKey(const std::wstring& Name, bool Writeable = true);

    /** Creates a new sub-key below the key at hand

        @precond IsOpen = true
                 IsWriteable = true

        @throws  RegistryIOException
                 RegistryWriteAccessDenyException
    */
    virtual RegistryKey CreateSubKey(const std::wstring& Name);

    /** Deletes a sub-key below the key at hand, the
        key must not have sub-keys

        @precond IsOpen = true
                 IsWriteable = true

        @throws  RegistryIOException
                 RegistryWriteAccessDenyException
    */
    virtual void DeleteSubKey(const std::wstring& Name);

    /** Deletes a sub-key below the key at hand with all
        its sub-keys

        @precond IsOpen = true
                 IsWriteable = true;

        @throws  RegistryIOException
                 RegistryWriteAccessDenyException
    */
    virtual void DeleteSubKeyTree(const std::wstring& Name);

    /** Delete the specified value

        @precond IsOpen = true
                 IsWriteable = true
                 HasValue(Name) = true

        @throws RegistryIOException
                RegistryWriteAccessDeniedException
                RegistryValueNotFoundException
    */
    virtual void DeleteValue(const std::wstring& Name);

    /** Set the specified registry value

        @precond IsOpen = true
                 IsWriteable = true

        @throws  RegistryIOException
                 RegistryWriteAccessDenyException
    */
    virtual void SetValue(const RegistryValue& Value);

    //############################################
    // Creation
    //
    // only possible through WindowsRegistry class
    //############################################

protected:
    /** Create instance and open the specified Registry key

        @throws  RegistryWriteAccessDenyException
                 RegistryAccessDenyException
                 RegistryKeyNotFoundException
    */
    RegistryKeyImplWinNT(HKEY RootKey, const std::wstring& KeyName);

    /** Create instance and open the specified Registry key

        @throws  RegistryWriteAccessDenyException
                 RegistryAccessDenyException
                 RegistryKeyNotFoundException
    */
    RegistryKeyImplWinNT(HKEY RootKey);

    /** Create an instances of the specified Registry key,
    the key is assumed to be already opened.
    */
    RegistryKeyImplWinNT(HKEY RootKey, HKEY SubKey, const std::wstring& KeyName, bool Writeable = true);

private:

    LONG ImplDeleteSubKeyTree(HKEY RootKey, const std::wstring& Name);

//prevent copy and assignment
private:
    RegistryKeyImplWinNT(const RegistryKeyImplWinNT&);
    RegistryKeyImplWinNT& operator=(const RegistryKeyImplWinNT&);

//######################################
// Friend declarations
//######################################

friend class WindowsRegistry;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
