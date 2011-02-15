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
#ifndef _FSYS_HXX
#define _FSYS_HXX

#include <tools/solar.h>
#include <tools/datetime.hxx>
#include <tools/string.hxx>
#include <tools/link.hxx>
#include <tools/wldcrd.hxx>
#include <tools/errcode.hxx>
#include "tools/toolsdllapi.h"

#include <cstdarg>
#include <vector>

#define FEAT_FSYS_DOUBLESPEED

// --------------
// - FSys-Types -
// --------------

class DirEntry;
class FSysSortList;
class FileStatList;
struct FileCopier_Impl;
class SvFileStream;
class BigInt;

typedef ::std::vector< DirEntry* > DirEntryList;

#define FSYS_BUFSIZE                1024
#define FSYS_SHORTNAME_DELIMITER    '@'

// FSysAccess
typedef int FSysAccess;
#define FSYS_ACCESS_FORCED          1
#define FSYS_ACCESS_FLOPPY          FSYS_ACCESS_FORCED
#define FSYS_ACCESS_CACHED          2

// DirEntryKind
typedef int DirEntryKind;
#define FSYS_KIND_NONE              ((DirEntryKind)     0)
#define FSYS_KIND_UNKNOWN           FSYS_KIND_NONE
#define FSYS_KIND_FILE              ((DirEntryKind)     1)
#define FSYS_KIND_DIR               ((DirEntryKind)     2)
#define FSYS_KIND_DEV               ((DirEntryKind)     4)
#define FSYS_KIND_BLOCK             ((DirEntryKind)     8)
#define FSYS_KIND_CHAR              ((DirEntryKind)    16)
#define FSYS_KIND_WILD              ((DirEntryKind)    32)
#define FSYS_KIND_BLOCK_REMOTE      ((DirEntryKind)    64)  //TPF: fuer RFS
#define FSYS_KIND_REMOVEABLE        ((DirEntryKind)   128)
#define FSYS_KIND_FIXED             ((DirEntryKind)   256)
#define FSYS_KIND_REMOTE            ((DirEntryKind)   512)
#define FSYS_KIND_RAM               ((DirEntryKind)  1024)
#define FSYS_KIND_CDROM             ((DirEntryKind)  2048)
#define FSYS_KIND_ALL               ((DirEntryKind)  4095)
#define FSYS_KIND_VISIBLE           ((DirEntryKind)  4096)

// FSysSort
typedef int FSysSort;
#define FSYS_SORT_NONE              ((FSysSort)   0)
#define FSYS_SORT_SIZE              ((FSysSort)   1)
#define FSYS_SORT_CREATED           ((FSysSort)   2)
#define FSYS_SORT_MODIFYED          ((FSysSort)   4)
#define FSYS_SORT_ACCESSED          ((FSysSort)   8)
#define FSYS_SORT_ASCENDING         ((FSysSort)  16)
#define FSYS_SORT_DESCENDING        ((FSysSort)  32)
#define FSYS_SORT_EXT               ((FSysSort)  64)
#define FSYS_SORT_NAME              ((FSysSort) 128)
#define FSYS_SORT_KIND              ((FSysSort) 256)
#define FSYS_SORT_CREATOR           ((FSysSort) 512)
#define FSYS_SORT_END               ((FSysSort)1024)

// DirEntryFlag
enum DirEntryFlag
{
    FSYS_FLAG_NORMAL,
    FSYS_FLAG_VOLUME,                      // Dir( FSYS_FLAG_VOLUME ) und GetDevice()
    FSYS_FLAG_ABSROOT,                     // z.B. "a:\" oder "\"
    FSYS_FLAG_RELROOT,                     // z.B. "a:", "a:." oder "."
    FSYS_FLAG_CURRENT = FSYS_FLAG_RELROOT, // Synonym fuer FSYS_FLAG_RELROOT
    FSYS_FLAG_PARENT,                      // z.B. ".."
    FSYS_FLAG_INVALID
};

// FSysPathStyle
enum FSysPathStyle
{
    FSYS_STYLE_HOST,
    FSYS_STYLE_FAT,
    FSYS_STYLE_MSDOS = FSYS_STYLE_FAT,
    FSYS_STYLE_VFAT,
    FSYS_STYLE_WIN95 = FSYS_STYLE_VFAT,
    FSYS_STYLE_HPFS,
    FSYS_STYLE_OS2 = FSYS_STYLE_HPFS,
    FSYS_STYLE_NTFS,
    FSYS_STYLE_NWFS,
    FSYS_STYLE_SYSV,
    FSYS_STYLE_BSD,
    FSYS_STYLE_UNX = FSYS_STYLE_BSD,
    FSYS_STYLE_MAC,
    FSYS_STYLE_DETECT,
    FSYS_STYLE_UNKNOWN,
    FSYS_STYLE_URL
};

// FSysAction
typedef int FSysAction;
#define FSYS_ACTION_COPYFILE        0x01    // not only create hardlink
#define FSYS_ACTION_RECURSIVE       0x02    // deep through dircetory structure
#define FSYS_ACTION_USERECYCLEBIN   0x04    // move to recycle bin
#define FSYS_ACTION_MOVE            0x08    // delete after copy (=> move)
#define FSYS_ACTION_CONTINUE        0x10    // continue on error
#define FSYS_ACTION_KEEP_EXISTING   0x20    // do not overwrite objects in
                                            // target folder in case of name
                                            // clashes
#define FSYS_ACTION_STANDARD        0

// Fuer RFS
#define RFS_IDENTIFIER  "-rfs-"
#define RFS_LOWER       "-rfs-"
#define RFS_UPPER       "-RFS-"

typedef ULONG FSysError;

// FSysExact
enum FSysExact
{
    FSYS_NOTEXACT,
    FSYS_EXACT
};

// ------------
// - Char-Set -
// ------------

#if defined(WIN) || defined(W30)

// MS-Windows has different char-sets for file-system and user-interface
String Gui2FSys( const String& rStr );
String FSys2Gui( const String& rStr );
#define GUI2FSYS(s) Gui2FSys( s )
#define FSYS2GUI(s) FSys2Gui( s )

#else

// all other OS have the same char-set for both
#define GUI2FSYS(s) s
#define FSYS2GUI(s) s

#endif

// ------------
// - FileStat -
// ------------

struct dirent;
class TOOLS_DLLPUBLIC FileStat
{
    friend class    CORmFSys;
    friend class    Dir;
    friend struct   DirReader_Impl;
    friend void     ImpInitFileStat( FileStat&, dirent* );

    ULONG           nError;
    DirEntryKind    nKindFlags;
    ULONG           nSize;
    String          aCreator;
    String          aType;
    Date            aDateCreated;
    Time            aTimeCreated;
    Date            aDateModified;
    Time            aTimeModified;
    Date            aDateAccessed;
    Time            aTimeAccessed;
private:
    TOOLS_DLLPRIVATE void           ImpInit( void* );

protected:
                    // Implementation
                    FileStat( const void *pInfo,      // CInfoPBRec
                              const void *pVolInfo ); // ParamBlockRec

public:
                    FileStat();
                    FileStat( const DirEntry& rDirEntry,
                              FSysAccess nAccess = FSYS_ACCESS_FLOPPY );
    BOOL            Update( const DirEntry& rDirEntry,
                              BOOL bForceAccess = TRUE );

    ULONG           GetError() const { return ERRCODE_TOERROR(nError); }
    ULONG           GetErrorCode() const { return nError; }

    ULONG           GetSize() const { return nSize; }

    DirEntryKind    GetKind() const { return nKindFlags; }
    BOOL            IsKind( DirEntryKind nKind ) const;

    String          GetType() const { return aType; }
    String          GetCreator() const { return aCreator; }

    Date            DateCreated() const  { return aDateCreated;  }
    Time            TimeCreated() const  { return aTimeCreated;  }
    Date            DateModified() const { return aDateModified; }
    Time            TimeModified() const { return aTimeModified; }
    Date            DateAccessed() const { return aDateAccessed; }
    Time            TimeAccessed() const { return aTimeAccessed; }
    BOOL            IsYounger( const FileStat& rIsOlder ) const;

#define TF_FSYS_READONLY_FLAG
    static ULONG    SetReadOnlyFlag( const DirEntry &rEntry, BOOL bRO = TRUE );
    static BOOL     GetReadOnlyFlag( const DirEntry &rEntry );
    static BOOL     HasReadOnlyFlag();

    static ErrCode  QueryDiskSpace( const String &rPath,
                                    BigInt &rFreeBytes, BigInt &rTotalBytes );

    static void     SetDateTime( const String& rFileName,
                                 const DateTime& rNewDateTime );
};

// ------------
// - DirEntry -
// ------------

class DirEntryStack;

const char* ImpCheckDirEntry( const void* p );

class TOOLS_DLLPUBLIC DirEntry
{
friend struct DirReader_Impl;
friend class FileCopier;

#ifdef FEAT_FSYS_DOUBLESPEED
    FileStat*           pStat;      // optional
#endif
    ByteString          aName;
    DirEntry*           pParent;
    ULONG               nError;
    DirEntryFlag        eFlag;

private:
    TOOLS_DLLPRIVATE            DirEntry( const ByteString& rInitName,
                                  DirEntryFlag aDirFlag,
                                  FSysPathStyle eStyle );

    friend class Dir;
    friend class FileStat;
    friend const char* ImpCheckDirEntry( const void* p );

    TOOLS_DLLPRIVATE FSysError          ImpParseName( const ByteString& rIntiName,
                                      FSysPathStyle eParser );
    TOOLS_DLLPRIVATE FSysError          ImpParseOs2Name( const ByteString& rPfad,
                                         FSysPathStyle eStyle );
    TOOLS_DLLPRIVATE FSysError          ImpParseUnixName( const ByteString& rPfad,
                                          FSysPathStyle eStyle );
    TOOLS_DLLPRIVATE USHORT             ImpTryUrl( DirEntryStack& rStack, const String& rPfad, FSysPathStyle eStyle );
    TOOLS_DLLPRIVATE const DirEntry*    ImpGetTopPtr() const;
    TOOLS_DLLPRIVATE DirEntry*          ImpGetTopPtr();
    TOOLS_DLLPRIVATE DirEntry*          ImpGetPreTopPtr();
    TOOLS_DLLPRIVATE BOOL               ImpToRel( String aStart );

protected:
    void                ImpTrim( FSysPathStyle eStyle );
    const ByteString&   ImpTheName() const;
    DirEntryFlag        ImpTheFlag() const { return eFlag; };
    DirEntry*           ImpChangeParent( DirEntry* pNewParent, BOOL bNormalize = TRUE );
    DirEntry*           ImpGetParent() { return pParent; }
#ifdef FEAT_FSYS_DOUBLESPEED
    FileStat*           ImpGetStat() const { return pStat; }
    void                ImpSetStat( FileStat *p ) { pStat = p; }
#endif

protected:
    void                SetError( ULONG nErr ) { nError = nErr; }
    DirEntry*           GetParent() { return pParent; }
public:
                        DirEntry( DirEntryFlag aDirFlag = FSYS_FLAG_CURRENT );
                        DirEntry( const DirEntry& rEntry );
                        DirEntry( const ByteString& rInitName,
                                   FSysPathStyle eParser = FSYS_STYLE_HOST );
                        DirEntry( const String& rInitName,
                                   FSysPathStyle eParser = FSYS_STYLE_HOST );
                        ~DirEntry();

    BOOL                IsLongNameOnFAT() const;
    BOOL                IsCaseSensitive (FSysPathStyle eFormatter = FSYS_STYLE_HOST) const;

    ULONG               GetError() const { return nError; }
    BOOL                IsValid() const;
    DirEntryFlag        GetFlag() const { return eFlag; };

    void                SetExtension( const String& rExt, char cSep = '.' );
    String              GetExtension( char cSep = '.' ) const;
    String              CutExtension( char cSep = '.' );
    void                SetName( const String& rName, FSysPathStyle eFormatter = FSYS_STYLE_HOST );
    inline const String GetNameDirect() const { return String(aName, osl_getThreadTextEncoding()); }
    String              GetName( FSysPathStyle eFormatter = FSYS_STYLE_HOST ) const;
    String              CutName( FSysPathStyle eFormatter = FSYS_STYLE_HOST );
    void                SetBase( const String& rBase, char cSep = '.' );
    String              GetBase(char cSep = '.' ) const;
    DirEntry            GetPath() const;
    DirEntry            GetDevice() const;
    String              GetVolume() const;
    String              GetFull( FSysPathStyle eFormatter = FSYS_STYLE_HOST,
                                 BOOL bWithDelimiter = FALSE,
                                 USHORT nMaxChars = STRING_MAXLEN ) const;

    DirEntry            TempName( DirEntryKind = FSYS_KIND_NONE ) const;
    static const DirEntry& SetTempNameBase( const String &rBaseName );
    BOOL                MakeShortName( const String& rLongName,
                                       DirEntryKind eCreateKind = FSYS_KIND_NONE,
                                       BOOL bUseTilde = TRUE,
                                       FSysPathStyle eStyle = FSYS_STYLE_DETECT );

    bool                IsAbs() const;
    BOOL                ToAbs();
    BOOL                Find( const String& rPfad, char cDelim = 0 );
    BOOL                ToRel();
    BOOL                ToRel( const DirEntry& rRefDir );
    USHORT              CutRelParents();

    BOOL                SetCWD( BOOL bSloppy = FALSE ) const;
    BOOL                MakeDir( BOOL bSloppy = FALSE ) const;
    BOOL                Exists( FSysAccess nAccess = FSYS_ACCESS_FLOPPY ) const;
    BOOL                First();

    USHORT              Level() const;
    const DirEntry&     operator []( USHORT nParentLevel ) const;
    BOOL                Contains( const DirEntry &rSubEntry ) const;

    FSysError           CopyTo( const DirEntry& rDestDir,
                                FSysAction nActions = FSYS_ACTION_STANDARD ) const;
    FSysError           MoveTo( const DirEntry& rDestDir ) const;
    FSysError           Kill( FSysAction nActions = FSYS_ACTION_STANDARD ) const;

    DirEntry&           operator =( const DirEntry& rOrigDir );
    DirEntry            operator +( const DirEntry& rSubDir ) const;
    DirEntry&           operator +=( const DirEntry& rSubDir );
    BOOL                operator ==( const DirEntry& rAnotherDir ) const;
    BOOL                operator !=( const DirEntry& rAnotherDir ) const
                            { return !(DirEntry::operator==( rAnotherDir )); }

    StringCompare       NameCompare( const DirEntry &rWith ) const;
    inline StringCompare NameCompareDirect( const DirEntry &rWith ) const
                        {
#ifdef UNX
                            return rWith.aName.CompareTo( aName );
#else
                            return rWith.aName.CompareIgnoreCaseToAscii( aName );
#endif
                        }

    static String       GetAccessDelimiter( FSysPathStyle eFormatter = FSYS_STYLE_HOST );
    static String       GetSearchDelimiter( FSysPathStyle eFormatter = FSYS_STYLE_HOST );
    static USHORT       GetMaxNameLen( FSysPathStyle eFormatter = FSYS_STYLE_HOST );
    static FSysPathStyle GetPathStyle( const String &rDevice );
    static String       ConvertNameToSystem( const String & rName );
    static String       ConvertSystemToName( const String & rName );
    static BOOL         IsRFSAvailable();
};

// --------------
// - FileCopier -
// --------------

class TOOLS_DLLPUBLIC FileCopier
{
    DirEntry            aSource;
    DirEntry            aTarget;
    ULONG               nBytesTotal;
    ULONG               nBytesCopied;
    Link                aProgressLink;
    USHORT              nBlockSize;
    FileCopier_Impl*    pImp;

private:
    TOOLS_DLLPRIVATE FSysError          DoCopy_Impl(
        const DirEntry &rSource, const DirEntry &rTarget );

protected:
    virtual BOOL        Progress();
    virtual ErrCode     Error( ErrCode eErr,
                               const DirEntry *pSource, const DirEntry *pTarget );

public:
                        FileCopier();
                        FileCopier( const DirEntry &rSource,
                                    const DirEntry &rTarget );
                        FileCopier( const FileCopier &rCopier );
                        virtual ~FileCopier();

    FileCopier&         operator = ( const FileCopier &rCopier );

    void                SetBlockSize( USHORT nBytes ) { nBlockSize = nBytes; }
    USHORT              GetBlockSize() const { return nBlockSize; }

    ULONG               GetBytesTotal() const { return nBytesTotal; }
    ULONG               GetBytesCopied() const { return nBytesCopied; }

    void                SetSource( const DirEntry &rSource ) { aSource = rSource; }
    void                SetTarget( const DirEntry &rTarget ) { aTarget = rTarget; }
    const DirEntry&     GetSource() const { return aSource; }
    const DirEntry&     GetTarget() const { return aTarget; }

    FSysError           Execute( FSysAction nActions = FSYS_ACTION_STANDARD );
    FSysError           ExecuteExact( FSysAction nActions = FSYS_ACTION_STANDARD,
                                                                 FSysExact  eExact = FSYS_NOTEXACT);

    const DirEntry*     GetErrorSource() const;
    const DirEntry*     GetErrorTarget() const;
    ErrCode             GetError() const;

    void                SetProgressHdl( const Link& rLink ) { aProgressLink = rLink; }
    const Link&         GetProgressHdl() const { return aProgressLink; }
    void                SetErrorHdl( const Link& rLink );
    const Link&         GetErrorHdl() const;
};

// -------
// - Dir -
// -------

struct DirReader_Impl;
class TOOLS_DLLPUBLIC Dir : public DirEntry
{
friend struct DirReader_Impl;
friend class CORmFSys;

    DirReader_Impl* pReader;        // systemabhaengig
    DirEntryList*   pLst;
    FSysSortList*   pSortLst;       // NULL, wenn kein Sort gefordert
    FileStatList*   pStatLst;       // NULL, wenn keine Stat's benoetigt
    WildCard        aNameMask;
    DirEntryKind eAttrMask;

private:
    TOOLS_DLLPRIVATE            Dir( const Dir& );          // not allowed
    TOOLS_DLLPRIVATE Dir&           operator=( const Dir& );    // not allowed

#ifdef _DIR_CXX
    TOOLS_DLLPRIVATE FSysError      ImpSetSort( std::va_list pArgs, FSysSort nSort );
    TOOLS_DLLPRIVATE void           Construct( DirEntryKind nKind = FSYS_KIND_DIR|FSYS_KIND_FILE );
#endif

#ifndef _TOOLS_HXX
protected:
    BOOL            ImpInsertPointReached( const DirEntry& rIsSmaller,
                                           const FileStat& rNewStat,
                                           size_t nCurPos,
                                           size_t nSortIndex ) const;
    void            ImpSortedInsert( const DirEntry *pNewEntry,
                                     const FileStat *pNewStat );
#endif

public:
                    Dir();
                    Dir( const DirEntry& rDirEntry,
                         DirEntryKind nKind = FSYS_KIND_ALL );
                    Dir( const DirEntry& rDirEntry,
                         DirEntryKind nKind,
                         FSysSort nSort, ... );
                    ~Dir();

    const WildCard& GetNameMask() const { return aNameMask; }

    FSysError       SetSort( FSysSort nSort, ... );

    void            Reset();
    USHORT          Scan( USHORT nCount = 5 );
    size_t          Count( BOOL bUpdated = TRUE ) const;
    BOOL            Update();

    Dir&            operator +=( const Dir& rDir );
    DirEntry&       operator []( size_t nIndex ) const;
};

// we don't need this stuff for bootstraping
#ifndef BOOTSTRAP

//========================================================================

/** FSysRedirector is an abstract base class for a hook to redirect
    mirrored directory trees.

    <P>One instance of a subclass can be instanciated and registered
    using the method FSysRedirector::Register(FSysRedirector*).
 */

class FSysRedirector
{
    static FSysRedirector*  _pRedirector;
    static BOOL             _bEnabled;

public:
    /** This method must called with the one and only instance of the
        subclass which implements the redirection.

        <P>It must be called with 0 when the instance is destroyed.
     */
    static void             Register( FSysRedirector *pRedirector );

    //-----------------------------------------------------------------------
    /** This method returns the currently registererd instance of
        a subclass which implements the redirection.

        <P>If no redirector is registered, it returns 0.
     */
    static FSysRedirector*  Redirector();

    //-----------------------------------------------------------------------
    /** This method is to be used to redirect a file system path.

        <P>It will not redirect while redirection is disabled.

        <P>It may block while another thread is accessing the redirector
        or another thread has disabled redirections.

        @param String &rPath<BR>
                This inout-argument accepts a file:-URL even as a native
                file system path name to redirect in 'rPath'. It returns the
                redirected (modified) path too, which can be of both formats
                too.

        @return BOOL<BR>
                TRUE, if the path is redirected
                FALSE, if the path is not redirected (unchanged)
     */
    static void             DoRedirect( String &rPath );
};

#endif // BOOTSTRP

//========================================================================

void FSysEnableSysErrorBox( BOOL bEnable );

//========================================================================

#if defined(DBG_UTIL)
void FSysTest();
#endif

#endif // #ifndef _FSYS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
