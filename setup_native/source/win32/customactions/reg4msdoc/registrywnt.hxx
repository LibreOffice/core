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
