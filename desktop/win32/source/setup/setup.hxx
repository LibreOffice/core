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

#include "setup_main.hxx"

//--------------------------------------------------------------------------

#ifdef SetupAppX
 #undef SetupAppX
#endif

#ifdef Create_SetupAppX
 #undef Create_SetupAppX
#endif

#ifdef LanguageDataX
 #undef LanguageDataX
#endif


#ifdef UNICODE
 #define SetupAppX          SetupAppW
 #define Create_SetupAppX   Create_SetupAppW
 #define LanguageDataX      LanguageDataW
#else
 #define SetupAppX          SetupAppA
 #define Create_SetupAppX   Create_SetupAppA
 #define LanguageDataX      LanguageDataA
#endif

//--------------------------------------------------------------------------

struct LanguageDataX
{
    long    m_nLanguageID;
    LPTSTR  m_pTransform;

     LanguageDataX( LPTSTR pData );
    ~LanguageDataX();
};

//--------------------------------------------------------------------------

class SetupAppX : public SetupApp
{
    HINSTANCE   m_hInst;
    HANDLE      m_hMapFile;
    LPTSTR      m_pAppTitle;
    LPTSTR      m_pCmdLine;
    LPTSTR      m_pDatabase;
    LPTSTR      m_pReqVersion;
    LPTSTR      m_pProductName;
    LPTSTR      m_pAdvertise;
    LPTSTR      m_pTmpName;
    LPTSTR      m_pErrorText;
    LPTSTR      m_pModuleFile;
    LPTSTR      m_pPatchFiles;
    LPCTSTR     m_pUpgradeKey;
    LPCTSTR     m_pProductVersion;
    int        *m_pMSIErrorCode;

    boolean     m_bQuiet            : 1;
    boolean     m_bIgnoreAlreadyRunning : 1;
    boolean     m_bRegNoMsoTypes :1;
    boolean     m_bRegAllMsoTypes :1;
    boolean     m_bIsMinorUpgrade :1;
    boolean     m_bSupportsPatch :1;

    FILE       *m_pLogFile;

    long            m_nLanguageID;
    long            m_nLanguageCount;
    LanguageDataX** m_ppLanguageList;

private:

    boolean     GetPathToFile( TCHAR* pFileName, TCHAR **pPath );
    LPCTSTR     GetPathToMSI();

    int         GetNameValue( TCHAR* pLine, TCHAR **pName, TCHAR **pValue );
    boolean     GetProfileSection( LPCTSTR pFileName, LPCTSTR pSection,
                                   DWORD& rSize, LPTSTR *pRetBuf );
    LPTSTR      CopyIniFile( LPCTSTR pIniFile );
    void        ConvertNewline( LPTSTR pText ) const;

    boolean     LaunchInstaller( LPCTSTR pParam );
    HMODULE     LoadMsiLibrary();
    DWORD       WaitForProcess( HANDLE hHandle );

    boolean     GetCmdLineParameters( LPTSTR *pCmdLine );
    DWORD       GetNextArgument( LPCTSTR pStr, LPTSTR *pArg,
                                 LPTSTR *pNext, boolean bStripQuotes = false );
    boolean     IsAdmin();

    boolean     GetCommandLine();

    boolean     IsTerminalServerInstalled() const;
    void        AddFileToPatchList( TCHAR* pPath, TCHAR* pFile );
    boolean     IsPatchInstalled( TCHAR* pBaseDir, TCHAR* pFileName );
    boolean     InstallRuntimes( TCHAR* pProductCode, TCHAR* pFileName );

public:
                    SetupAppX();
                   ~SetupAppX();

    virtual boolean Initialize( HINSTANCE hInst );
    virtual boolean AlreadyRunning() const;
    virtual boolean ReadProfile();
    virtual boolean GetPatches();
    virtual boolean ChooseLanguage( long& rLanguage );
    virtual boolean CheckVersion();
    virtual boolean CheckForUpgrade();
    virtual boolean InstallRuntimes();
    virtual boolean Install( long nLanguage );

    virtual UINT    GetError() const;
    virtual void    DisplayError( UINT nErr ) const;

    void            Log( LPCTSTR pMessage, LPCTSTR pText = NULL ) const;

    long            GetLanguageCount() const { return m_nLanguageCount; }
    long            GetLanguageID( long nIndex ) const;
    void            GetLanguageName( long nLanguage, LPTSTR sName ) const;

    LPCTSTR         GetAppTitle() const { return m_pAppTitle; }
    LPTSTR          SetProdToAppTitle( LPCTSTR pProdName );
    HINSTANCE       GetHInst() const { return m_hInst; }
};

//--------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
