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


#ifndef SETUP_MAIN_HXX
#define SETUP_MAIN_HXX

class SetupApp
{
    DWORD           m_nOSVersion;
    DWORD           m_nMinorVersion;
    boolean         m_bNeedReboot   : 1;
    boolean         m_bAdministrative : 1;

public:
    UINT            m_uiRet;

                    SetupApp();
    virtual        ~SetupApp();

    virtual boolean Initialize( HINSTANCE hInst ) = 0;
    virtual boolean AlreadyRunning() const = 0;
    virtual boolean ReadProfile() = 0;
    virtual boolean GetPatches() = 0;
    virtual boolean ChooseLanguage( long& rLanguage ) = 0;
    virtual boolean CheckVersion() = 0;
    virtual boolean CheckForUpgrade() = 0;
    virtual boolean InstallRuntimes() = 0;
    virtual boolean Install( long nLanguage ) = 0;

    virtual UINT    GetError() const = 0;
    virtual void    DisplayError( UINT nErr ) const = 0;

    void            SetError( UINT nErr ) { m_uiRet = nErr; }
    DWORD           GetOSVersion() const { return m_nOSVersion; }
    DWORD           GetMinorVersion() const { return m_nMinorVersion; }

    boolean         IsAdminInstall() { return m_bAdministrative; }
    void            SetAdminInstall( boolean bValue ) { m_bAdministrative = bValue; }

    void            SetRebootNeeded( boolean bNeedReboot ) { m_bNeedReboot = bNeedReboot; }
    boolean         NeedReboot() const { return m_bNeedReboot; }
};

SetupApp* Create_SetupAppW();

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
