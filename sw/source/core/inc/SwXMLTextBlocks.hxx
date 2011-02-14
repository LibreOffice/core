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
    sal_Bool         bAutocorrBlock;
    sal_Bool         bBlock;
    SfxObjectShellRef xDocShellRef;
    sal_uInt16       nFlags;
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
    void   AddName( const String&, const String&, const String&, sal_Bool bOnlyTxt = sal_False );
    virtual void   AddName( const String&, const String&, sal_Bool bOnlyTxt = sal_False );
    void GeneratePackageName ( const String& rShort, String& rPackageName );
    virtual ~SwXMLTextBlocks();
    //virtual sal_Bool   IsOld() const;
    virtual sal_uLong Delete( sal_uInt16 );
    virtual sal_uLong Rename( sal_uInt16, const String&, const String& );
    virtual sal_uLong CopyBlock( SwImpBlocks& rImp, String& rShort, const String& rLong);
    virtual void  ClearDoc();
    virtual sal_uLong GetDoc( sal_uInt16 );
    virtual sal_uLong BeginPutDoc( const String&, const String& );
    virtual sal_uLong PutDoc();
    virtual sal_uLong GetText( sal_uInt16, String& );
    virtual sal_uLong PutText( const String&, const String&, const String& );
    virtual sal_uLong MakeBlockList();

    virtual short GetFileType ( void ) const;
    virtual sal_uLong OpenFile( sal_Bool bReadOnly = sal_True );
    virtual void  CloseFile();

    static sal_Bool IsFileUCBStorage( const String & rFileName);

    // Methods for the new Autocorrecter
    sal_uLong GetText( const String& rShort, String& );

    virtual sal_Bool IsOnlyTextBlock( const String& rShort ) const;
    virtual sal_Bool IsOnlyTextBlock( sal_uInt16 nIdx ) const;
    virtual void SetIsTextOnly( const String& rShort, sal_Bool bNewValue );
    virtual void SetIsTextOnly( sal_uInt16 nIdx, sal_Bool bNewValue );

    virtual sal_uLong GetMacroTable( sal_uInt16, SvxMacroTableDtor& rMacroTbl,
                                 sal_Bool bFileAlreadyOpen = sal_False );
    virtual sal_uLong SetMacroTable( sal_uInt16 nIdx,
                                 const SvxMacroTableDtor& rMacroTable,
                                 sal_Bool bFileAlreadyOpen = sal_False );
    virtual sal_Bool PutMuchEntries( sal_Bool bOn );

public:
    SwDoc* GetDoc() const { return pDoc; }
    //void  SetDoc( SwDoc * pNewDoc);
    sal_uLong StartPutBlock( const String& rShort, const String& rPackageName );
    sal_uLong PutBlock( SwPaM& rPaM, const String& rLong );
    sal_uLong GetBlockText( const String& rShort, String& rText );
    sal_uLong PutBlockText( const String& rShort, const String& rName, const String& rText,  const String& rPackageName );
    void MakeBlockText( const String& rText );

};

#endif
