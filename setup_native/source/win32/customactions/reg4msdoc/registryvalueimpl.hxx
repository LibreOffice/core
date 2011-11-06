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

// RegistryValueImpl.h: Schnittstelle für die Klasse RegistryValueImpl.
//
//////////////////////////////////////////////////////////////////////

#ifndef _REGISTRYVALUEIMPL_HXX_
#define _REGISTRYVALUEIMPL_HXX_

#include <memory>
#include <string>

class RegistryValueImpl
{
public:

    //#################################
    // Creation/Destruction
    //#################################

    RegistryValueImpl(const std::wstring& Name, int Value);

    RegistryValueImpl(const std::wstring& Name, const std::wstring& Value);

    RegistryValueImpl(const std::wstring& Name, const std::string& Value);

    #if (_MSC_VER >= 1300)
    RegistryValueImpl::RegistryValueImpl(const RegistryValueImpl& s);
    #endif

    virtual ~RegistryValueImpl();


    //#################################
    // Query
    //#################################


    /** Returns the name of the value
    */
    std::wstring GetName() const;

    /** Return the size of data held
    */
    size_t GetDataSize() const;

    /** Get a pointer to the data buffer
        in order to copy the data
    */
    const void* GetDataBuffer() const;

    /** Returns the data as unicode string

        @precond GetType = STRING
    */
    std::wstring GetDataAsUniString() const;

    /** Returns the data as ansi string

        @precond GetType = STRING
    */
    std::string GetDataAsAnsiString() const;

    /** Returns the data as number

        @precond GetType = NUMBER
    */
    int GetDataAsInt() const;

    /** Returns the type of the data
    */
    int GetType() const;

    //#################################
    // Command
    //#################################


    /** Set a new name
    */
    void SetName(const std::wstring& NewName);

    /**
    */
    void SetValue(const std::wstring& NewValue);

    /**
    */
    void SetValue(const std::string& NewValue);

    /**
    */
    void SetValue(int NewValue);

    //#################################
    // Private data
    //#################################

private:
    std::wstring    m_Name;
    int             m_Type;
    std::wstring    m_StringData;
    int             m_IntData;
};


typedef std::auto_ptr<RegistryValueImpl> RegistryValue;


#endif
