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

#ifndef m_LOGINERR_HXX
#define m_LOGINERR_HXX

#include <tools/string.hxx>

//=========================================================================

#define LOGINERROR_FLAG_SET_SAVE_PASSWORD    1
#define LOGINERROR_FLAG_MODIFY_ACCOUNT        2
#define LOGINERROR_FLAG_MODIFY_USER_NAME    4
#define LOGINERROR_FLAG_PERSISTENT_PASSWORD    8
#define LOGINERROR_FLAG_CAN_USE_SYSCREDS    16
#define LOGINERROR_FLAG_IS_USE_SYSCREDS        32

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
    BYTE   m_nFlags;
    USHORT m_nRet;
    bool   m_bRecommendToOpenReadonly;

public:
                    LoginErrorInfo()
                    : m_nFlags( LOGINERROR_FLAG_MODIFY_USER_NAME ),
                      m_nRet( ERRCODE_BUTTON_CANCEL )
                    {
                    }

    const String&     GetTitle() const          { return m_aTitle; }
    const String&     GetServer() const          { return m_aServer; }
    const String&     GetAccount() const      { return m_aAccount; }
    const String&     GetUserName() const      { return m_aUserName; }
    const String&   GetPassword() const     { return m_aPassword; }
    const String&   GetPasswordToModify() const  { return m_aPasswordToModify; }
    const bool      IsRecommendToOpenReadonly() const { return m_bRecommendToOpenReadonly; }
    const String&     GetPath() const          { return m_aPath; }
    const String&     GetErrorText() const    { return m_aErrorText; }
    BOOL             GetIsPersistentPassword() const
                    { return ( m_nFlags & LOGINERROR_FLAG_PERSISTENT_PASSWORD ); }
    BOOL              GetIsSavePassword() const
                    { return ( m_nFlags & LOGINERROR_FLAG_SET_SAVE_PASSWORD ); }
    BOOL            GetCanUseSystemCredentials() const
                    { return ( m_nFlags & LOGINERROR_FLAG_CAN_USE_SYSCREDS ); }
    BOOL            GetIsUseSystemCredentials() const
                    { return ( m_nFlags & LOGINERROR_FLAG_IS_USE_SYSCREDS ) ==
                             LOGINERROR_FLAG_IS_USE_SYSCREDS; }
    BYTE            GetFlags() const        { return m_nFlags; }
    USHORT             GetResult() const        { return m_nRet; }

    void             SetTitle( const String& aTitle )
                    { m_aTitle = aTitle; }
    void             SetServer( const String& aServer )
                    { m_aServer = aServer; }
    void             SetAccount( const String& aAccount )
                    { m_aAccount = aAccount; }
    void             SetUserName( const String& aUserName )
                    { m_aUserName = aUserName; }
    void            SetPassword( const String& aPassword )
                    { m_aPassword = aPassword; }
    void            SetPasswordToModify( const String& aPassword )
                    { m_aPasswordToModify = aPassword; }
    void            SetRecommendToOpenReadonly( bool bVal )
                    { m_bRecommendToOpenReadonly = bVal; }
    void             SetPath( const String& aPath )
                    { m_aPath = aPath; }
    void             SetErrorText( const String& aErrorText )
                    { m_aErrorText = aErrorText; }
    void            SetFlags( BYTE nFlags )
                    { m_nFlags = nFlags; }
    inline void        SetSavePassword( BOOL bSet );
    inline void        SetPersistentPassword( BOOL bSet );
    inline void     SetCanUseSystemCredentials( BOOL bSet );
    inline void     SetIsUseSystemCredentials( BOOL bSet );
    inline void        SetModifyAccount( BOOL bSet );
    inline void        SetModifyUserName( BOOL bSet );
    void            SetResult( USHORT nRet )
                    { m_nRet = nRet; }
};

inline void LoginErrorInfo::SetSavePassword( BOOL bSet )
{
    if ( bSet )
        m_nFlags |= LOGINERROR_FLAG_SET_SAVE_PASSWORD;
    else
        m_nFlags &= ~LOGINERROR_FLAG_SET_SAVE_PASSWORD;
}

inline void LoginErrorInfo::SetPersistentPassword( BOOL bSet )
{
    if ( bSet )
        m_nFlags |= LOGINERROR_FLAG_PERSISTENT_PASSWORD;
    else
        m_nFlags &= ~LOGINERROR_FLAG_PERSISTENT_PASSWORD;
}

inline void LoginErrorInfo::SetCanUseSystemCredentials( BOOL bSet )
{
    if ( bSet )
        m_nFlags |= LOGINERROR_FLAG_CAN_USE_SYSCREDS;
    else
        m_nFlags &= ~LOGINERROR_FLAG_CAN_USE_SYSCREDS;
}

inline void LoginErrorInfo::SetIsUseSystemCredentials( BOOL bSet )
{
    if ( bSet )
        m_nFlags |= LOGINERROR_FLAG_IS_USE_SYSCREDS;
    else
        m_nFlags &= ~LOGINERROR_FLAG_IS_USE_SYSCREDS;
}

inline void LoginErrorInfo::SetModifyAccount( BOOL bSet )
{
    if ( bSet )
        m_nFlags |= LOGINERROR_FLAG_MODIFY_ACCOUNT;
    else
        m_nFlags &= ~LOGINERROR_FLAG_MODIFY_ACCOUNT;
}

inline void LoginErrorInfo::SetModifyUserName( BOOL bSet )
{
    if ( bSet )
        m_nFlags |= LOGINERROR_FLAG_MODIFY_USER_NAME;
    else
        m_nFlags &= ~LOGINERROR_FLAG_MODIFY_USER_NAME;
}

#endif
