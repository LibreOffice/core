/*************************************************************************
 *
 *  $RCSfile: stgdir.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mba $ $Date: 2000-11-20 12:56:37 $
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

#ifndef _STGDIR_HXX
#define _STGDIR_HXX

#ifndef _STGAVL_HXX
#include "stgavl.hxx"
#endif
#ifndef _STGELEM_HXX
#include "stgelem.hxx"
#endif
#ifndef _STGSTRMS_HXX
#include "stgstrms.hxx"
#endif

class StgIo;
class StgEntry;
class StgDirEntry;
class StgDirStrm;

class BaseStorageStream;
class StgDirEntry : public StgAvlNode
{
    friend class StgIterator;
    friend class StgDirStrm;
    StgEntry     aSave;                     // original dir entry
    StgDirEntry*  pUp;                      // parent directory
    StgDirEntry*  pDown;                    // child directory for storages
    StgDirEntry** ppRoot;                   // root of TOC tree
    StgStrm*     pStgStrm;                  // storage stream
    StgTmpStrm*  pTmpStrm;                  // temporary stream
    StgTmpStrm*  pCurStrm;                  // temp stream after commit
    INT32        nEntry;                    // entry # in TOC stream (temp)
    INT32        nPos;                      // current position
    BOOL         bDirty;                    // dirty directory entry
    BOOL         bCreated;                  // newly created entry
    BOOL         bRemoved;                  // removed per Invalidate()
    BOOL         bRenamed;                  // renamed
    void         InitMembers();             // ctor helper
    virtual short Compare( const StgAvlNode* ) const;
    BOOL         StoreStream( StgIo& );     // store the stream
    BOOL         StoreStreams( StgIo& );    // store all streams
    void         RevertAll();               // revert the whole tree
    BOOL         Strm2Tmp();                // copy stgstream to temp file
    BOOL         Tmp2Strm();                // copy temp file to stgstream
public:
    StgEntry     aEntry;                    // entry data
    INT32        nRefCnt;                   // reference count
    StreamMode   nMode;                     // open mode
    BOOL         bTemp;                     // TRUE: delete on dir flush
    BOOL         bDirect;                   // TRUE: direct mode
    BOOL         bZombie;                   // TRUE: Removed From StgIo
    BOOL         bInvalid;                  // TRUE: invalid entry
    StgDirEntry( const void*, BOOL * pbOk );
    StgDirEntry( const StgEntry& );
    ~StgDirEntry();

    void Invalidate( BOOL=FALSE );          // invalidate all open entries
    void Enum( INT32& );                    // enumerate entries for iteration
    void DelTemp( BOOL );                   // delete temporary entries
    BOOL Store( StgDirStrm& );              // save entry into dir strm
    BOOL IsContained( StgDirEntry* );       // check if subentry

    void SetDirty()  { bDirty = TRUE;     }
    BOOL IsDirty();
    void ClearDirty();

    BOOL Commit();
    BOOL Revert();

    void  OpenStream( StgIo&, BOOL=FALSE );     // set up an approbiate stream
    void  Close();
    INT32 GetSize();
    BOOL  SetSize( INT32 );
    INT32 Seek( INT32 );
    INT32 Tell() { return nPos; }
    INT32 Read( void*, INT32 );
    INT32 Write( const void*, INT32 );
    void  Copy( StgDirEntry& );
    void  Copy( BaseStorageStream& );
};

class StgDirStrm : public StgDataStrm
{
    friend class StgIterator;
    StgDirEntry* pRoot;                         // root of dir tree
    short        nEntries;                      // entries per page
    void         SetupEntry( INT32, StgDirEntry* );
public:
    StgDirStrm( StgIo& );
    ~StgDirStrm();
    virtual BOOL SetSize( INT32 );              // change the size
    BOOL         Store();
    void*        GetEntry( INT32 n, BOOL=FALSE );// get an entry
    StgDirEntry* GetRoot() { return pRoot; }
    StgDirEntry* Find( StgDirEntry&, const String& );
    StgDirEntry* Create( StgDirEntry&, const String&, StgEntryType );
    BOOL         Remove( StgDirEntry&, const String& );
    BOOL         Rename( StgDirEntry&, const String&, const String& );
    BOOL         Move( StgDirEntry&, StgDirEntry&, const String& );
};

class StgIterator : public StgAvlIterator
{
public:
    StgIterator( StgDirEntry& rStg ) : StgAvlIterator( rStg.pDown ) {}
    StgDirEntry* First() { return (StgDirEntry*) StgAvlIterator::First(); }
    StgDirEntry* Next()  { return (StgDirEntry*) StgAvlIterator::Next();  }
    StgDirEntry* Last()  { return (StgDirEntry*) StgAvlIterator::Last();  }
    StgDirEntry* Prev()  { return (StgDirEntry*) StgAvlIterator::Prev();  }
};

#endif
