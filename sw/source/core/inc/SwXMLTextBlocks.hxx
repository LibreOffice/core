/*************************************************************************
 *
 *  $RCSfile: SwXMLTextBlocks.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: mtg $ $Date: 2001-03-19 11:11:42 $
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
 *  Contributor(s): Martin Gallwey (gallwey@sun.com)
 *
 *
 ************************************************************************/
#ifndef _SW_XMLTEXTBLOCKS_HXX
#define _SW_XMLTEXTBLOCKS_HXX

#ifndef _SWBLOCKS_HXX
#include <swblocks.hxx>
#endif

#ifndef SW_SWDLL_HXX
#include <swdll.hxx>
#endif
#ifndef _DATE_HXX //autogen
#include <tools/date.hxx>
#endif

#ifndef _PERSIST_HXX //autogen
#include <so3/persist.hxx>
#endif

#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif

// Note...these headers only needed for the IsUCBStorage function
#include <tools/urlobj.hxx>
#include <unotools/streamhelper.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/localfilehelper.hxx>
#include <sot/stg.hxx>

#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif

#ifndef _EMBOBJ_HXX //autogen
#include <so3/embobj.hxx>
#endif


class SfxMedium;
class SwPaM;
class SwDoc;
class SvxMacroTableDtor;
class SwImpBlocks;
class SwDocShell;
class SvEmbeddedObjectRef;

#define SWXML_CONVBLOCK     0x0001
#define SWXML_NOROOTCOMMIT  0x0002

class SwXMLTextBlocks : public SwImpBlocks
{
protected:
    BOOL         bAutocorrBlock;
    BOOL         bBlock;
    SvEmbeddedObjectRef xDocShellRef;
    USHORT       nFlags;
    String       aPackageName;
public:
    SvStorageRef        xBlkRoot;
    SvStorageRef        xRoot;
    short               nCurBlk;
    SwXMLTextBlocks( const String& );
    SwXMLTextBlocks( SvStorage& );
    void   AddName( const String&, const String&, const String&, BOOL bOnlyTxt = FALSE );
    virtual void   AddName( const String&, const String&, BOOL bOnlyTxt = FALSE );
    void GeneratePackageName ( const String& rShort, String& rPackageName );
    void   SetCurrentText( const String& rText );
    virtual ~SwXMLTextBlocks();
    //virtual BOOL   IsOld() const;
    virtual ULONG Delete( USHORT );
    virtual ULONG Rename( USHORT, const String&, const String& );
    virtual ULONG CopyBlock( SwImpBlocks& rImp, String& rShort, const String& rLong);
    virtual ULONG GetDoc( USHORT );
    virtual ULONG BeginPutDoc( const String&, const String& );
    virtual ULONG PutDoc();
    virtual ULONG GetText( USHORT, String& );
    virtual ULONG PutText( const String&, const String&, const String& );
    virtual ULONG MakeBlockList();

    virtual short GetFileType ( void ) const;
    virtual ULONG OpenFile( BOOL bReadOnly = TRUE );
    virtual void  CloseFile();

    ULONG SetConvertMode( BOOL );
    static BOOL IsFileUCBStorage( const String & rFileName);

    // Methods for the new Autocorrecter
    ULONG GetText( const String& rShort, String& );

    virtual BOOL IsOnlyTextBlock( const String& rShort ) const;
    virtual BOOL IsOnlyTextBlock( USHORT nIdx ) const;
    virtual void SetIsTextOnly( const String& rShort, BOOL bNewValue );
    virtual void SetIsTextOnly( USHORT nIdx, BOOL bNewValue );

    virtual ULONG GetMacroTable( USHORT, SvxMacroTableDtor& rMacroTbl );
    virtual ULONG SetMacroTable( USHORT nIdx, const SvxMacroTableDtor& rMacroTabl );
    virtual BOOL PutMuchEntries( BOOL bOn );
    void ReadInfo();
    void WriteInfo();

public:
    SwDoc* GetDoc() const { return pDoc; }
    //void  SetDoc( SwDoc * pNewDoc);
    ULONG StartPutBlock( const String& rShort, const String& rPackageName );
    ULONG PutBlock( SwPaM& rPaM, const String& rLong );
    ULONG GetBlockText( const String& rShort, String& rText );
    ULONG PutBlockText( const String& rShort, const String& rName, const String& rText,  const String& rPackageName );
    void MakeBlockText( const String& rText );
    void AddTextNode( const ::rtl::OUString& rText );

};

#endif
