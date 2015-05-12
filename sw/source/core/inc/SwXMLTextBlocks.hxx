/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SW_SOURCE_CORE_INC_SWXMLTEXTBLOCKS_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_SWXMLTEXTBLOCKS_HXX

#include <sfx2/objsh.hxx>
#include <sfx2/docfile.hxx>
#include <swblocks.hxx>

class SwPaM;
class SwDoc;
class SvxMacroTableDtor;

#define SWXML_CONVBLOCK     0x0001
#define SWXML_NOROOTCOMMIT  0x0002

class SwXMLTextBlocks : public SwImpBlocks
{
protected:
    bool         bAutocorrBlock;
    bool         bBlock;
    SfxObjectShellRef xDocShellRef;
    sal_uInt16       nFlags;
    OUString aPackageName;
    SfxMediumRef xMedium;

    void ReadInfo();
    void WriteInfo();
    void InitBlockMode ( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& rStorage );
    void ResetBlockMode();

public:
    com::sun::star::uno::Reference < com::sun::star::embed::XStorage > xBlkRoot;
    com::sun::star::uno::Reference < com::sun::star::embed::XStorage > xRoot;
    short               nCurBlk;
    SwXMLTextBlocks( const OUString& rFile );
    SwXMLTextBlocks( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >&, const OUString& rFile );
    void   AddName( const OUString&, const OUString&, const OUString&, bool bOnlyText = false );
    virtual void   AddName( const OUString&, const OUString&, bool bOnlyText = false ) SAL_OVERRIDE;
    static OUString GeneratePackageName ( const OUString& rShort );
    virtual ~SwXMLTextBlocks();
    virtual sal_uLong Delete( sal_uInt16 ) SAL_OVERRIDE;
    virtual sal_uLong Rename( sal_uInt16, const OUString&, const OUString& ) SAL_OVERRIDE;
    virtual sal_uLong CopyBlock( SwImpBlocks& rImp, OUString& rShort, const OUString& rLong) SAL_OVERRIDE;
    virtual void  ClearDoc() SAL_OVERRIDE;
    virtual sal_uLong GetDoc( sal_uInt16 ) SAL_OVERRIDE;
    virtual sal_uLong BeginPutDoc( const OUString&, const OUString& ) SAL_OVERRIDE;
    virtual sal_uLong PutDoc() SAL_OVERRIDE;
    virtual sal_uLong GetText( sal_uInt16, OUString& ) SAL_OVERRIDE;
    virtual sal_uLong PutText( const OUString&, const OUString&, const OUString& ) SAL_OVERRIDE;
    virtual sal_uLong MakeBlockList() SAL_OVERRIDE;

    virtual short GetFileType() const SAL_OVERRIDE;
    virtual sal_uLong OpenFile( bool bReadOnly = true ) SAL_OVERRIDE;
    virtual void  CloseFile() SAL_OVERRIDE;

    static bool IsFileUCBStorage( const OUString & rFileName);

    // Methods for the new Autocorrecter
    sal_uLong GetText( const OUString& rShort, OUString& );

    virtual bool IsOnlyTextBlock( const OUString& rShort ) const SAL_OVERRIDE;
    bool IsOnlyTextBlock( sal_uInt16 nIdx ) const;
    void SetIsTextOnly( const OUString& rShort, bool bNewValue );

    virtual sal_uLong GetMacroTable( sal_uInt16, SvxMacroTableDtor& rMacroTable,
                                 bool bFileAlreadyOpen = false ) SAL_OVERRIDE;
    virtual sal_uLong SetMacroTable( sal_uInt16 nIdx,
                                 const SvxMacroTableDtor& rMacroTable,
                                 bool bFileAlreadyOpen = false ) SAL_OVERRIDE;
    virtual bool PutMuchEntries( bool bOn ) SAL_OVERRIDE;

public:
    SwDoc* GetDoc() const { return pDoc; }
    //void  SetDoc( SwDoc * pNewDoc);
    sal_uLong StartPutBlock( const OUString& rShort, const OUString& rPackageName );
    sal_uLong PutBlock( SwPaM& rPaM, const OUString& rLong );
    sal_uLong GetBlockText( const OUString& rShort, OUString& rText );
    sal_uLong PutBlockText( const OUString& rShort, const OUString& rName, const OUString& rText, const OUString& rPackageName );
    void MakeBlockText( const OUString& rText );

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
