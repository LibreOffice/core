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
#ifndef _SW_XMLTEXTBLOCKS_HXX
#define _SW_XMLTEXTBLOCKS_HXX
#include <sfx2/objsh.hxx>
#include <sfx2/docfile.hxx>
#include <swblocks.hxx>

class SfxMedium;
class SwPaM;
class SwDoc;
class SvxMacroTableDtor;
class SwImpBlocks;

#define SWXML_CONVBLOCK     0x0001
#define SWXML_NOROOTCOMMIT  0x0002

class SwXMLTextBlocks : public SwImpBlocks
{
protected:
    BOOL         bAutocorrBlock;
    BOOL         bBlock;
    SfxObjectShellRef xDocShellRef;
    USHORT       nFlags;
    String       aPackageName;
    SfxMediumRef xMedium;

    void ReadInfo();
    void WriteInfo();
    void InitBlockMode ( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& rStorage );
    void ResetBlockMode();

public:
    com::sun::star::uno::Reference < com::sun::star::embed::XStorage > xBlkRoot;
    com::sun::star::uno::Reference < com::sun::star::embed::XStorage > xRoot;
    short               nCurBlk;
    SwXMLTextBlocks( const String& rFile );
    SwXMLTextBlocks( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >&, const String& rFile );
    void   AddName( const String&, const String&, const String&, BOOL bOnlyTxt = FALSE );
    virtual void   AddName( const String&, const String&, BOOL bOnlyTxt = FALSE );
    void GeneratePackageName ( const String& rShort, String& rPackageName );
    virtual ~SwXMLTextBlocks();
    //virtual BOOL   IsOld() const;
    virtual ULONG Delete( USHORT );
    virtual ULONG Rename( USHORT, const String&, const String& );
    virtual ULONG CopyBlock( SwImpBlocks& rImp, String& rShort, const String& rLong);
    virtual void  ClearDoc();
    virtual ULONG GetDoc( USHORT );
    virtual ULONG BeginPutDoc( const String&, const String& );
    virtual ULONG PutDoc();
    virtual ULONG GetText( USHORT, String& );
    virtual ULONG PutText( const String&, const String&, const String& );
    virtual ULONG MakeBlockList();

    virtual short GetFileType ( void ) const;
    virtual ULONG OpenFile( BOOL bReadOnly = TRUE );
    virtual void  CloseFile();

    static BOOL IsFileUCBStorage( const String & rFileName);

    // Methods for the new Autocorrecter
    ULONG GetText( const String& rShort, String& );

    virtual BOOL IsOnlyTextBlock( const String& rShort ) const;
    virtual BOOL IsOnlyTextBlock( USHORT nIdx ) const;
    virtual void SetIsTextOnly( const String& rShort, BOOL bNewValue );
    virtual void SetIsTextOnly( USHORT nIdx, BOOL bNewValue );

    virtual ULONG GetMacroTable( USHORT, SvxMacroTableDtor& rMacroTbl,
                                 sal_Bool bFileAlreadyOpen = sal_False );
    virtual ULONG SetMacroTable( USHORT nIdx,
                                 const SvxMacroTableDtor& rMacroTable,
                                 sal_Bool bFileAlreadyOpen = sal_False );
    virtual BOOL PutMuchEntries( BOOL bOn );

public:
    SwDoc* GetDoc() const { return pDoc; }
    //void  SetDoc( SwDoc * pNewDoc);
    ULONG StartPutBlock( const String& rShort, const String& rPackageName );
    ULONG PutBlock( SwPaM& rPaM, const String& rLong );
    ULONG GetBlockText( const String& rShort, String& rText );
    ULONG PutBlockText( const String& rShort, const String& rName, const String& rText,  const String& rPackageName );
    void MakeBlockText( const String& rText );

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
