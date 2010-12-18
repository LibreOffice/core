/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
