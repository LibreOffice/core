/*************************************************************************
 *
 *  $RCSfile: winshell.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:11 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _WINSHELL_HXX
#define _WINSHELL_HXX

#include <prewin.h>
#include "shutil.h"
#include <postwin.h>

#include <string.hxx>

//----------------------------------------------------------------------------
// CItemIDList
//----------------------------------------------------------------------------

class CItemIDList
{
public:
    CItemIDList();
    CItemIDList( const CItemIDList & );
    CItemIDList( LPCITEMIDLIST );
    CItemIDList( const String & );
    CItemIDList( int nFolder );
    ~CItemIDList();

    CItemIDList & operator = ( const CItemIDList & );
    CItemIDList & operator += ( const CItemIDList & );
    CItemIDList operator + ( const CItemIDList & ) const;

    int operator == ( const CItemIDList & ) const;
    int operator != ( const CItemIDList & rIDList ) const
        { return ! operator == ( rIDList ); };

    operator LPCITEMIDLIST() const
        { return m_pidl; };

    int GetTokenCount() const;
    CItemIDList GetToken( int nIndex ) const;
    CItemIDList GetParent( int nLevelFromBottom = 1) const;
    CItemIDList GetChild( int nLevelFromBottom = 1 ) const;
    void Split( CItemIDList & rParent, CItemIDList & rChild, int nLevelFromBottom = 1 ) const;

    CItemIDList operator []( int nIndex ) const
        { return GetToken( nIndex ); };

    String GetFilePath() const;

    BOOL IsValid() const
        { return m_pidl != NULL && m_pidl->mkid.cb != sizeof(USHORT); }

    int GetRootID() const
        { return m_nFolder; }

protected:
    ITEMIDLIST  *m_pidl;
    int         m_nFolder;
};


inline CItemIDList CItemIDList::operator + ( const CItemIDList & rIDList ) const
{
    CItemIDList aCopy( *this );
    aCopy += rIDList;
    return aCopy;
}

//----------------------------------------------------------------------------
// Types for CShellFolder
//----------------------------------------------------------------------------

// Notification Events fuer CShellFolder

enum NotificationEvent
{
    NotificationEvent_Error,
    NotificationEvent_Signaled,
    NotificationEvent_Canceled
};

// Volume information

typedef struct _WIN32_VOLUME_DATA
{
    TCHAR   cDeviceName[MAX_PATH];
    TCHAR   cVolumeName[MAX_PATH];
    TCHAR   cFileSystemName[MAX_PATH];
    DWORD   dwSerialNumber;
    DWORD   dwFileSystemFlags;
    DWORD   nMaxComponentLength;
} WIN32_VOLUME_DATA;

// Bekannte Class-IDs fuer wichtige Ordner

// {871C5380-42A0-1069-A2EA-08002B30309D}   Internet Explorer 4.0
static const GUID CLSID_IE4 =
{ 0x871C5380, 0x42A0, 0x1069, { 0xA2, 0xEA, 0x08, 0x00, 0x2B, 0x30, 0x30, 0x9D } };

// {208D2C60-3AEA-1069-A2D7-08002B30309D}   Netzwerkumgebung
static const GUID CLSID_Network =
{ 0x208D2C60, 0x3AEA, 0x1069, { 0xA2, 0xD7, 0x08, 0x00, 0x2B, 0x30, 0x30, 0x9D } };

// {645FF040-5081-101B-9F08-00AA002F954E}   Papierkorb
static const GUID CLSID_RecycleBin =
{ 0x645FF040, 0x5081, 0x101B, { 0x9F, 0x08, 0x00, 0xAA, 0x00, 0x2F, 0x95, 0x4E } };

// {20D04FE0-3AEA-1069-A2D8-08002B30309D}   Arbeitsplatz
static const GUID CLSID_MyComputer =
{ 0x20D04FE0, 0x3AEA, 0x1069, { 0xA2, 0xD8, 0x08, 0x00, 0x2B, 0x30, 0x30, 0x9D } };

// {D6277990-4C6A-11CF-8D87-00AA0060F5BF}   Geplante Vorgänge
static const GUID CLSID_Tasks =
{ 0xD6277990, 0x4C6A, 0x11CF, { 0x8D, 0x87, 0x00, 0xAA, 0x00, 0x60, 0xF5, 0xBF } };

// Fehlt im Header

#define SHGDN_INCLUDE_NONFILESYS    0x2000

#define CSIDL_UNKNOWN   -1
#define CSIDL_ROOT      -2
#define CSIDL_SYSTEM    -3

// Suchmaske fuer IEnumIDList

#define SHCONTF_ALL (SHCONTF_FOLDERS | SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN)

// SHITEM Prefix Kinds

#define SHGII_CONTAINER_MASK        0x70

#define SHGII_COMPUTER              0x20

#define SHGII_COMPUTER_REMOVABLE    0x22
#define SHGII_COMPUTER_FIXED        0x23
#define SHGII_COMPUTER_REMOTE       0x24
#define SHGII_COMPUTER_CDROM        0x25
#define SHGII_COMPUTER_RAMDISK      0x26
#define SHGII_COMPUTER_FLOPPY525    0x28
#define SHGII_COMPUTER_FLOPPY35     0x29
#define SHGII_COMPUTER_NETWORK      0x2A
#define SHGII_COMPUTER_REGITEM      0x2E

#define SHGII_ROOT                  0x10
#define SHGII_ROOT_REGITEM          0x1F

#define SHGII_NETWORK               0x40
#define SHGII_NETWORK_TREE          0x47
#define SHGII_NETWORK_SERVER        0x42
#define SHGII_NETWORK_DIRECTORY     0x43
#define SHGII_NETWORK_PRINTER       0x41

#define SHGII_FILESYSTEM            0x30
#define SHGII_FILESYSTEM_FILE       0x31
#define SHGII_FILESYSTEM_DIRECTORY  0x32

#define SHGII_ANCESTOR              0x80

#define SHITEMCONTAINER( pidl )     ((pidl)->mkid.abID[0] & SHGII_CONTAINER_MASK)
#define SHITEMKIND( pidl )          ((pidl)->mkid.abID[0] & 0x7F)

//----------------------------------------------------------------------------
// CShellFolderData
//----------------------------------------------------------------------------

class CShellFolderData
{
protected:
    CShellFolderData();
    virtual ~CShellFolderData();

    IShellFolder    *m_pShellFolder;
    IEnumIDList     *m_pEnumIDList;
    IShellIcon      *m_pShellIcon;
    BOOL            m_bIsOpen;
    HANDLE          m_hCancelEvent;
    DWORD           m_dwContentFlags;
};

//----------------------------------------------------------------------------
// CShellFolder
//----------------------------------------------------------------------------

// Ganz "normaler" Ordner basierend auf dem IShellFolder Interface

class CShellFolder : public CShellFolderData
{
public:
    CShellFolder( const CItemIDList & );
    CShellFolder( const CShellFolder & );
    CShellFolder( IShellFolder * );

    // Retrieval of Item IDs

    virtual BOOL Reset();

    BOOL GetNextValidID( CItemIDList & );

    virtual BOOL GetNextID( CItemIDList & );
    virtual BOOL ValidateID( const CItemIDList & );

    // Getting information about Items

    virtual BOOL GetAttributesOf( const CItemIDList &, LPDWORD pdwInOut );
    virtual BOOL GetNameOf( const CItemIDList &, String & );

    virtual BOOL GetFileInfo( const CItemIDList &, WIN32_FIND_DATA * );
    virtual BOOL GetVolumeInfo( const CItemIDList & rIDList, WIN32_VOLUME_DATA * );

    // Modifying the folder contents

    virtual BOOL SetNameOf( const CItemIDList &, const String &, CItemIDList & );
    virtual BOOL DeleteItem( const CItemIDList & );

    // Comparison of IDs

    virtual int CompareIDs( const CItemIDList &, const CItemIDList & );

    // UI Components

    virtual IContextMenu *GetContextMenu( int nItems, const CItemIDList * );
    virtual String GetIconLocation( const CItemIDList & );

    // Notifications

    virtual NotificationEvent WaitForChanges();
    virtual void CancelWaitNotifications();

protected:
    CShellFolder() : CShellFolderData() {};

    void Initialize( IShellFolder *pShellFolder );
    void Initialize( LPCITEMIDLIST );
};

//----------------------------------------------------------------------------
// CFileSystemFolder
//----------------------------------------------------------------------------

// Reiner !!! Filesystem-Ordner. Benutzt optimierten Notification Mechanismus

class CFileSystemFolder : public CShellFolder
{
public:
    CFileSystemFolder( LPCSTR pszPath );

    virtual NotificationEvent WaitForChanges();

protected:
    CFileSystemFolder() : CShellFolder() {};

    void Initialize( LPCTSTR pszPath );

    TCHAR   m_szPath[MAX_PATH];
};

//----------------------------------------------------------------------------
// CSpecialFolder
//----------------------------------------------------------------------------

// Wie SHellFolder, aber andere Konstruktion ueber definierte Junktion-Points

class CSpecialFolder : public CShellFolder
{
public:
    CSpecialFolder( int nFolder );

protected:
    CSpecialFolder() : CShellFolder() {};

    void Initialize( int nFolder );

    int m_nFolder;
};

//----------------------------------------------------------------------------
// CMyComputerFolder
//----------------------------------------------------------------------------

// Der MS-Windows "Arbeitsplatz"

class CMyComputerFolder : public CSpecialFolder
{
public:
    CMyComputerFolder() : CSpecialFolder( CSIDL_DRIVES ) {};
};

//----------------------------------------------------------------------------
// CVolumesFolder
//----------------------------------------------------------------------------

// Wie CMyComputersFolder, enthält aber nur die Laufwerke

class CVolumesFolder : public CMyComputerFolder
{
public:
    CVolumesFolder() : CMyComputerFolder() {};

    virtual BOOL ValidateID( const CItemIDList & );
    virtual NotificationEvent WaitForChanges();
};

//----------------------------------------------------------------------------
// CWorkplaceFolder
//----------------------------------------------------------------------------

// Wie CMyComputersFolder, enthält aber keine!!! Laufwerke

class CWorkplaceFolder : public CMyComputerFolder
{
    CWorkplaceFolder() : CMyComputerFolder() {};

    virtual BOOL ValidateID( const CItemIDList & );
    virtual NotificationEvent WaitForChanges();
};

//----------------------------------------------------------------------------
// CDesktopFolder
//----------------------------------------------------------------------------

// Der MS-Windows Desktop

class CDesktopFolder : public CSpecialFolder
{
public:
    CDesktopFolder() : CSpecialFolder( CSIDL_DESKTOP ) {};
};

//----------------------------------------------------------------------------
// CDesktopAncestorsFolder
//----------------------------------------------------------------------------

// Wie CDesktopFolder, enthält aber nur Arbeitsplatz und Netzwerk

class CDesktopAncestorsFolder : public CDesktopFolder
{
public:
    CDesktopAncestorsFolder() : CDesktopFolder() {};

    virtual BOOL ValidateID( const CItemIDList & );
    virtual NotificationEvent WaitForChanges();
};

//----------------------------------------------------------------------------
// CDesktopContentsFolder
//----------------------------------------------------------------------------

// Wie CDesktopFolder, aber ohne!!! Arbeitsplatz und Netzwerk

class CDesktopContentsFolder : public CDesktopFolder
{
public:
    CDesktopContentsFolder() : CDesktopFolder() {};

    virtual BOOL ValidateID( const CItemIDList & );
    virtual NotificationEvent WaitForChanges();
};


#endif // _WINSHELL_HXX
