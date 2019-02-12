/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#include "vbasystem.hxx"

#include <ooo/vba/word/WdCursorType.hpp>
#include <tools/diagnose_ex.h>
#include <tools/config.hxx>
#include <osl/file.hxx>
#include <tools/urlobj.hxx>
#include <o3tl/char16_t2wchar_t.hxx>

#ifdef _WIN32
#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

using namespace ::ooo::vba;
using namespace ::com::sun::star;

PrivateProfileStringListener::~PrivateProfileStringListener()
{
}

void PrivateProfileStringListener::Initialize( const OUString& rFileName, const OString& rGroupName, const OString& rKey )
{
    maFileName = rFileName;
    maGroupName = rGroupName;
    maKey = rKey;
}
#ifdef _WIN32
static void lcl_getRegKeyInfo( const OString& sKeyInfo, HKEY& hBaseKey, OString& sSubKey )
{
    sal_Int32 nBaseKeyIndex = sKeyInfo.indexOf('\\');
    if( nBaseKeyIndex > 0 )
    {
        OString sBaseKey = sKeyInfo.copy( 0, nBaseKeyIndex );
        sSubKey = sKeyInfo.copy( nBaseKeyIndex + 1 );
        if( sBaseKey == "HKEY_CURRENT_USER" )
        {
            hBaseKey = HKEY_CURRENT_USER;
        }
        else if( sBaseKey == "HKEY_LOCAL_MACHINE" )
        {
            hBaseKey = HKEY_LOCAL_MACHINE;
        }
        else if( sBaseKey == "HKEY_CLASSES_ROOT" )
        {
            hBaseKey = HKEY_CLASSES_ROOT;
        }
        else if( sBaseKey == "HKEY_USERS" )
        {
            hBaseKey = HKEY_USERS;
        }
        else if( sBaseKey == "HKEY_CURRENT_CONFIG" )
        {
            hBaseKey = HKEY_CURRENT_CONFIG;
        }
    }
}
#endif

uno::Any PrivateProfileStringListener::getValueEvent()
{
    // get the private profile string
    OUString sValue;
    if(maFileName.isEmpty())
    {
        // get key/value from Windows registry
#ifdef _WIN32
        HKEY hBaseKey = nullptr;
        OString sSubKey;
        lcl_getRegKeyInfo( maGroupName, hBaseKey, sSubKey );
        if( hBaseKey != nullptr )
        {
            HKEY hKey = nullptr;
            LPCSTR lpSubKey = sSubKey.getStr();
            // We use RegOpenKeyExA here for convenience, because we already have subkey name as 8-bit string
            LONG lResult = RegOpenKeyExA( hBaseKey, lpSubKey, 0, KEY_QUERY_VALUE, &hKey );
            if( ERROR_SUCCESS == lResult )
            {
                OUString sUValName = OStringToOUString(maKey, RTL_TEXTENCODING_DONTKNOW);
                LPCWSTR lpValueName = o3tl::toW(sUValName.getStr());
                WCHAR szBuffer[1024];
                DWORD cbData = sizeof(szBuffer);
                lResult = RegQueryValueExW( hKey, lpValueName, nullptr, nullptr, reinterpret_cast<LPBYTE>(szBuffer), &cbData );
                RegCloseKey( hKey );
                // https://msdn.microsoft.com/en-us/ms724911 mentions that
                // "the string may not have been stored with the proper terminating null characters"
                szBuffer[std::min(size_t(cbData / sizeof(szBuffer[0])), SAL_N_ELEMENTS(szBuffer)-1)] = 0;
                sValue = o3tl::toU(szBuffer);
            }
        }
#else
        throw uno::RuntimeException("Only support on Windows" );
#endif
    }

    // get key/value from a file
    Config aCfg( maFileName );
    aCfg.SetGroup( maGroupName );
    sValue = OStringToOUString(aCfg.ReadKey(maKey), RTL_TEXTENCODING_DONTKNOW);


    return uno::makeAny( sValue );
}

void PrivateProfileStringListener::setValueEvent( const css::uno::Any& value )
{
    // set the private profile string
    OUString aValue;
    value >>= aValue;
    if(maFileName.isEmpty())
    {
        //set value into Windows registry
#ifdef _WIN32
        HKEY hBaseKey = nullptr;
        OString sSubKey;
        lcl_getRegKeyInfo( maGroupName, hBaseKey, sSubKey );
        if( hBaseKey != nullptr )
        {
            HKEY hKey = nullptr;
            LPCSTR lpSubKey = sSubKey.getStr();
            // We use RegCreateKeyExA here for convenience, because we already have subkey name as 8-bit string
            LONG lResult = RegCreateKeyExA( hBaseKey, lpSubKey, 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &hKey, nullptr );
            if( ERROR_SUCCESS == lResult )
            {
                DWORD cbData = sizeof(WCHAR) * (aValue.getLength() + 1);
                OUString sUValName = OStringToOUString(maKey, RTL_TEXTENCODING_DONTKNOW);
                LPCWSTR lpValueName = o3tl::toW(sUValName.getStr());
                lResult = RegSetValueExW( hKey, lpValueName, 0 /* Reserved */, REG_SZ, reinterpret_cast<BYTE const *>(aValue.getStr()), cbData );
                RegCloseKey( hKey );
            }
        }
        return;
#else
        throw uno::RuntimeException("Not implemented" );
#endif
    }

    // set value into a file
    Config aCfg( maFileName );
    aCfg.SetGroup( maGroupName );
    aCfg.WriteKey( maKey, OUStringToOString(aValue, RTL_TEXTENCODING_DONTKNOW) );


}

SwVbaSystem::SwVbaSystem( uno::Reference<uno::XComponentContext > const & xContext ): SwVbaSystem_BASE( uno::Reference< XHelperInterface >(), xContext )
{
}

SwVbaSystem::~SwVbaSystem()
{
}

sal_Int32 SAL_CALL
SwVbaSystem::getCursor()
{
    PointerStyle nPointerStyle = getPointerStyle( getCurrentWordDoc(mxContext) );

    switch( nPointerStyle )
    {
        case PointerStyle::Arrow:
            return word::WdCursorType::wdCursorNorthwestArrow;
        case PointerStyle::Null:
            return word::WdCursorType::wdCursorNormal;
        case PointerStyle::Wait:
            return word::WdCursorType::wdCursorWait;
        case PointerStyle::Text:
            return word::WdCursorType::wdCursorIBeam;
        default:
            return word::WdCursorType::wdCursorNormal;
    }
}

void SAL_CALL
SwVbaSystem::setCursor( sal_Int32 _cursor )
{
    try
    {
        switch( _cursor )
        {
            case word::WdCursorType::wdCursorNorthwestArrow:
            {
                setCursorHelper( getCurrentWordDoc(mxContext), PointerStyle::Arrow, false );
                break;
            }
            case word::WdCursorType::wdCursorWait:
            {
                //It will set the edit window, toobar and statusbar's mouse pointer.
                setCursorHelper( getCurrentWordDoc(mxContext), PointerStyle::Wait, true );
                break;
            }
            case word::WdCursorType::wdCursorIBeam:
            {
                //It will set the edit window, toobar and statusbar's mouse pointer.
                setCursorHelper( getCurrentWordDoc( mxContext ), PointerStyle::Text, true );
                break;
            }
            case word::WdCursorType::wdCursorNormal:
            {
                setCursorHelper( getCurrentWordDoc( mxContext ), PointerStyle::Null, false );
                break;
            }
            default:
                throw uno::RuntimeException("Unknown value for Cursor pointer" );
                // TODO: isn't this a flaw in the API? It should be allowed to throw an
                // IllegalArgumentException, or so
        }
    }
    catch( const uno::Exception& )
    {
    }
}

uno::Any SAL_CALL
SwVbaSystem::PrivateProfileString( const OUString& rFilename, const OUString& rSection, const OUString& rKey )
{
    // FIXME: need to detect whether it is a relative file path
    // we need to detect if this is a URL, if not then assume it's a file path
    OUString sFileUrl;
    if( !rFilename.isEmpty() )
    {
        INetURLObject aObj;
        aObj.SetURL( rFilename );
        bool bIsURL = aObj.GetProtocol() != INetProtocol::NotValid;
        if ( bIsURL )
            sFileUrl = rFilename;
        else
            osl::FileBase::getFileURLFromSystemPath( rFilename, sFileUrl);
    }

    OString aGroupName(OUStringToOString(rSection, RTL_TEXTENCODING_DONTKNOW));
    OString aKey(OUStringToOString(rKey, RTL_TEXTENCODING_DONTKNOW));
    maPrivateProfileStringListener.Initialize( sFileUrl, aGroupName, aKey );

    return uno::makeAny( uno::Reference< XPropValue > ( new ScVbaPropValue( &maPrivateProfileStringListener ) ) );
}

OUString
SwVbaSystem::getServiceImplName()
{
    return OUString("SwVbaSystem");
}

uno::Sequence< OUString >
SwVbaSystem::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        "ooo.vba.word.System"
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
