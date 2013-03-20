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

#pragma once
#if 1

#include <tools/string.hxx>

//=========================================================================

#define LOGINERROR_FLAG_MODIFY_ACCOUNT         1
#define LOGINERROR_FLAG_MODIFY_USER_NAME       2
#define LOGINERROR_FLAG_CAN_REMEMBER_PASSWORD  4
#define LOGINERROR_FLAG_IS_REMEMBER_PASSWORD   8
#define LOGINERROR_FLAG_CAN_USE_SYSCREDS      16
#define LOGINERROR_FLAG_IS_USE_SYSCREDS       32
#define LOGINERROR_FLAG_REMEMBER_PERSISTENT   64

class LoginErrorInfo
{
private:
    String m_aTitle;
    String m_aServer;
    String m_aAccount;
    String m_aUserName;
    String m_aPassword;
    String m_aPasswordToModify;
    String m_aPath;
    String m_aErrorText;
    sal_uInt8   m_nFlags;
    sal_uInt16 m_nRet;
    bool   m_bRecommendToOpenReadonly;

public:
                    LoginErrorInfo()
                    : m_nFlags( LOGINERROR_FLAG_MODIFY_USER_NAME ),
                      m_nRet( ERRCODE_BUTTON_CANCEL )
                    {
                    }

    const String&   GetTitle() const                    { return m_aTitle; }
    const String&   GetServer() const                   { return m_aServer; }
    const String&   GetAccount() const                  { return m_aAccount; }
    const String&   GetUserName() const                 { return m_aUserName; }
    const String&   GetPassword() const                 { return m_aPassword; }
    const String&   GetPasswordToModify() const         { return m_aPasswordToModify; }
    bool            IsRecommendToOpenReadonly() const   { return m_bRecommendToOpenReadonly; }
    const String&   GetPath() const                     { return m_aPath; }
    const String&   GetErrorText() const                { return m_aErrorText; }
    sal_Bool            GetCanRememberPassword() const      { return ( m_nFlags & LOGINERROR_FLAG_CAN_REMEMBER_PASSWORD ); }
    sal_Bool            GetIsRememberPersistent() const     { return ( m_nFlags & LOGINERROR_FLAG_REMEMBER_PERSISTENT ); }
    sal_Bool            GetIsRememberPassword() const       { return ( m_nFlags & LOGINERROR_FLAG_IS_REMEMBER_PASSWORD ); }

    sal_Bool            GetCanUseSystemCredentials() const
                    { return ( m_nFlags & LOGINERROR_FLAG_CAN_USE_SYSCREDS ); }
    sal_Bool            GetIsUseSystemCredentials() const
                    { return ( m_nFlags & LOGINERROR_FLAG_IS_USE_SYSCREDS ) ==
                             LOGINERROR_FLAG_IS_USE_SYSCREDS; }
    sal_uInt8            GetFlags() const        { return m_nFlags; }
    sal_uInt16          GetResult() const       { return m_nRet; }

    void            SetTitle( const String& aTitle )
                    { m_aTitle = aTitle; }
    void            SetServer( const String& aServer )
                    { m_aServer = aServer; }
    void            SetAccount( const String& aAccount )
                    { m_aAccount = aAccount; }
    void            SetUserName( const String& aUserName )
                    { m_aUserName = aUserName; }
    void            SetPassword( const String& aPassword )
                    { m_aPassword = aPassword; }
    void            SetPasswordToModify( const String& aPassword )
                    { m_aPasswordToModify = aPassword; }
    void            SetRecommendToOpenReadonly( bool bVal )
                    { m_bRecommendToOpenReadonly = bVal; }
    void            SetPath( const String& aPath )
                    { m_aPath = aPath; }
    void            SetErrorText( const String& aErrorText )
                    { m_aErrorText = aErrorText; }
    void            SetFlags( sal_uInt8 nFlags )
                    { m_nFlags = nFlags; }

    inline void     SetCanRememberPassword( sal_Bool bSet );
    inline void     SetIsRememberPassword( sal_Bool bSet );
    inline void     SetIsRememberPersistent( sal_Bool bSet );

    inline void     SetCanUseSystemCredentials( sal_Bool bSet );
    inline void     SetIsUseSystemCredentials( sal_Bool bSet );
    inline void     SetModifyAccount( sal_Bool bSet );
    inline void     SetModifyUserName( sal_Bool bSet );

    void            SetResult( sal_uInt16 nRet )
                    { m_nRet = nRet; }
};

inline void LoginErrorInfo::SetCanRememberPassword( sal_Bool bSet )
{
    if ( bSet )
        m_nFlags |= LOGINERROR_FLAG_CAN_REMEMBER_PASSWORD;
    else
        m_nFlags &= ~LOGINERROR_FLAG_CAN_REMEMBER_PASSWORD;
}

inline void LoginErrorInfo::SetIsRememberPassword( sal_Bool bSet )
{
    if ( bSet )
        m_nFlags |= LOGINERROR_FLAG_IS_REMEMBER_PASSWORD;
    else
        m_nFlags &= ~LOGINERROR_FLAG_IS_REMEMBER_PASSWORD;
}

inline void LoginErrorInfo::SetIsRememberPersistent( sal_Bool bSet )
{
    if ( bSet )
        m_nFlags |= LOGINERROR_FLAG_REMEMBER_PERSISTENT;
    else
        m_nFlags &= ~LOGINERROR_FLAG_REMEMBER_PERSISTENT;
}

inline void LoginErrorInfo::SetCanUseSystemCredentials( sal_Bool bSet )
{
    if ( bSet )
        m_nFlags |= LOGINERROR_FLAG_CAN_USE_SYSCREDS;
    else
        m_nFlags &= ~LOGINERROR_FLAG_CAN_USE_SYSCREDS;
}

inline void LoginErrorInfo::SetIsUseSystemCredentials( sal_Bool bSet )
{
    if ( bSet )
        m_nFlags |= LOGINERROR_FLAG_IS_USE_SYSCREDS;
    else
        m_nFlags &= ~LOGINERROR_FLAG_IS_USE_SYSCREDS;
}

inline void LoginErrorInfo::SetModifyAccount( sal_Bool bSet )
{
    if ( bSet )
        m_nFlags |= LOGINERROR_FLAG_MODIFY_ACCOUNT;
    else
        m_nFlags &= ~LOGINERROR_FLAG_MODIFY_ACCOUNT;
}

inline void LoginErrorInfo::SetModifyUserName( sal_Bool bSet )
{
    if ( bSet )
        m_nFlags |= LOGINERROR_FLAG_MODIFY_USER_NAME;
    else
        m_nFlags &= ~LOGINERROR_FLAG_MODIFY_USER_NAME;
}

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
