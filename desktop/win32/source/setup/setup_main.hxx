/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: setup_main.hxx,v $
 * $Revision: 1.8 $
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


#ifndef SETUP_MAIN_HXX
#define SETUP_MAIN_HXX

class SetupApp
{
    DWORD           m_nOSVersion;
    DWORD           m_nMinorVersion;
    boolean         m_bIsWin9x      : 1;
    boolean         m_bNeedReboot   : 1;

public:
    UINT            m_uiRet;

                    SetupApp();
    virtual        ~SetupApp();

    virtual boolean Initialize( HINSTANCE hInst ) = 0;
    virtual boolean AlreadyRunning() const = 0;
    virtual boolean ReadProfile() = 0;
    virtual boolean ChooseLanguage( long& rLanguage ) = 0;
    virtual boolean CheckVersion() = 0;
    virtual boolean Install( long nLanguage ) = 0;

    virtual UINT    GetError() const = 0;
    virtual void    DisplayError( UINT nErr ) const = 0;

    void            SetError( UINT nErr ) { m_uiRet = nErr; }
    boolean         IsWin9x() const { return m_bIsWin9x; }
    DWORD           GetOSVersion() const { return m_nOSVersion; }
    DWORD           GetMinorVersion() const { return m_nMinorVersion; }

    void            SetRebootNeeded( boolean bNeedReboot ) { m_bNeedReboot = bNeedReboot; }
    boolean         NeedReboot() const { return m_bNeedReboot; }
};

SetupApp* Create_SetupAppA();
SetupApp* Create_SetupAppW();

#endif
