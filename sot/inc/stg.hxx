/*************************************************************************
 *
 *  $RCSfile: stg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mba $ $Date: 2000-10-16 14:05:36 $
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

#ifndef _STG_HXX
#define _STG_HXX


#ifndef _TOOLS_STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _TOOLS_GLOBNAME_HXX //autogen
#include <tools/globname.hxx>
#endif

class Storage;
class StorageStream;
class StgIo;
class StgDirEntry;
class StgStrm;
class SvGlobalName;

class StorageBase                          // common helper class
{
protected:
    StgIo*       pIo;                      // I/O subsystem
    StgDirEntry* pEntry;                   // the dir entry
    ULONG        nError;                   // error code
    StreamMode   nMode;                    // open mode
    BOOL         bAutoCommit;
    StorageBase( StgIo*, StgDirEntry* );
   ~StorageBase();
    BOOL Validate( BOOL=FALSE ) const;
    BOOL ValidateMode( StreamMode, StgDirEntry* ) const;
public:
    void  ResetError() const;
    void  SetError( ULONG ) const;
    ULONG GetError() const;
    BOOL  Good() const          { return BOOL( nError == SVSTREAM_OK ); }
    StreamMode GetMode() const  { return nMode;  }
    const SvStream* GetSvStream() const;
};

class StorageStream : public StorageBase
{
    friend class Storage;                   // for the following ctor
    ULONG nPos;                             // current position
    StorageStream( StgIo*, StgDirEntry*, StreamMode );
public:
    ~StorageStream();
    ULONG Read( void * pData, ULONG nSize );
    ULONG Write( const void* pData, ULONG nSize );
    ULONG Seek( ULONG nPos );
    ULONG Tell() { return nPos; }
    void  Flush();
    BOOL  SetSize( ULONG nNewSize );
    BOOL  CopyTo( StorageStream * pDestStm );
    BOOL  Commit();
    BOOL  Revert();
};

class SvStorageInfoList;

class Storage : public StorageBase
{
    String aName;
    BOOL   bIsRoot;           // TRUE if root storage
    void   Init( BOOL bCreate );
    Storage( StgIo*, StgDirEntry*, StreamMode );
public:
                      Storage( const String &,
                               StreamMode = STREAM_STD_READWRITE,
                               BOOL bDirect = TRUE );
                      Storage( SvStream& rStrm,
                               BOOL bDirect = TRUE );
    ~Storage();
    static BOOL       IsStorageFile( const String & rFileName );
    static BOOL       IsStorageFile( SvStream* );
    const String&     GetName() const;
    BOOL              IsRoot() const { return bIsRoot; }

                      // eigener Datenbereich
    void              SetClass( const SvGlobalName & rClass,
                                ULONG nOriginalClipFormat,
                                const String & rUserTypeName );
    void              SetConvertClass( const SvGlobalName & rConvertClass,
                                       ULONG nOriginalClipFormat,
                                       const String & rUserTypeName );
    SvGlobalName      GetClassName();// Typ der Daten im Storage
    ULONG             GetFormat();
    String            GetUserName();
    BOOL              ShouldConvert();

                      // Liste aller Elemente
    void              FillInfoList( SvStorageInfoList* ) const;
    BOOL              CopyTo( Storage* pDestStg ) const;
    BOOL              Commit();
    BOOL              Revert();
                      /* Element Methoden     */
                      // Stream mit Verbindung zu Storage erzeugen,
                      // in etwa eine Parent-Child Beziehung
    StorageStream*    OpenStream( const String & rEleName,
                                  StreamMode = STREAM_STD_READWRITE,
                                  BOOL bDirect = TRUE );
    Storage*          OpenStorage( const String & rEleName,
                                   StreamMode = STREAM_STD_READWRITE,
                                   BOOL bDirect = FALSE );
                      // Abfrage auf Storage oder Stream
    BOOL              IsStream( const String& rEleName ) const;
    BOOL              IsStorage( const String& rEleName ) const;
    BOOL              IsContained( const String& rEleName ) const;
                      // Element loeschen
    BOOL              Remove( const String & rEleName );
                      // Elementnamen aendern
    BOOL              Rename( const String & rEleName,
                              const String & rNewName );
    BOOL              CopyTo( const String & rEleName, Storage * pDest,
                              const String & rNewName );
    BOOL              MoveTo( const String & rEleName, Storage * pDest,
                              const String & rNewName );
    BOOL              ValidateFAT();
};

#endif
