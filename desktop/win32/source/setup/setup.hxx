/*************************************************************************
 *
 *  $RCSfile: setup.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2004-06-11 17:48:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2004 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

#ifdef SetupDlgProcX
 #undef SetupDlgProcX
#endif

#ifdef LanguageDataX
 #undef LanguageDataX
#endif


#ifdef UNICODE
 #define SetupAppX          SetupAppW
 #define Create_SetupAppX   Create_SetupAppW
 #define SetupDlgProcX      SetupDlgProcW
 #define LanguageDataX      LanguageDataW
#else
 #define SetupAppX          SetupAppA
 #define Create_SetupAppX   Create_SetupAppA
 #define SetupDlgProcX      SetupDlgProcA
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

    boolean     m_bQuiet;
    boolean     m_bAdministrative;

    FILE       *m_pLogFile;

    long             m_nLanguageCount;
    LanguageDataX**  m_ppLanguageList;

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
