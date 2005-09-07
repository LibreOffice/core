/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: registrywnt.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:37:56 $
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

#ifndef _REGISTRYWNT_HXX_
#define _REGISTRYWNT_HXX_

#ifndef _REGISTRY_HXX_
#include "registry.hxx"
#endif

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
