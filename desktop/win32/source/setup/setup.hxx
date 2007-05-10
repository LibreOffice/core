/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: setup.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: gm $ $Date: 2007-05-10 11:08:40 $
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
    LPTSTR      m_pInstMsiW;
    LPTSTR      m_pInstMsiA;
    LPTSTR      m_pReqVersion;
    LPTSTR      m_pProductName;
    LPTSTR      m_pAdvertise;
    LPTSTR      m_pTmpName;
    LPTSTR      m_pErrorText;
    LPTSTR      m_pModuleFile;
    int        *m_pMSIErrorCode;

    boolean     m_bQuiet            : 1;
    boolean     m_bAdministrative   : 1;
    boolean     m_bIgnoreAlreadyRunning : 1;

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
    boolean     InstallMsi( LPCTSTR pInstaller );

    boolean     IsTerminalServerInstalled() const;

public:
                    SetupAppX();
                   ~SetupAppX();

    virtual boolean Initialize( HINSTANCE hInst );
    virtual boolean AlreadyRunning() const;
    virtual boolean ReadProfile();
    virtual boolean ChooseLanguage( long& rLanguage );
    virtual boolean CheckVersion();
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
