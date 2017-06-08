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
#include <o3tl/typed_flags_set.hxx>

class SwPaM;
class SwDoc;
class SvxMacroTableDtor;

enum class SwXmlFlags {
    NONE         = 0x0000,
    NoRootCommit = 0x0002,
};
namespace o3tl {
    template<> struct typed_flags<SwXmlFlags> : is_typed_flags<SwXmlFlags, 0x0002> {};
}

class SwXMLTextBlocks : public SwImpBlocks
{
protected:
    SfxObjectShellRef       xDocShellRef;
    SwXmlFlags              nFlags;
    OUString                aPackageName;
    tools::SvRef<SfxMedium> xMedium;

    void ReadInfo();
    void WriteInfo();
    void InitBlockMode ( const css::uno::Reference < css::embed::XStorage >& rStorage );
    void ResetBlockMode();

public:
    css::uno::Reference < css::embed::XStorage > xBlkRoot;
    css::uno::Reference < css::embed::XStorage > xRoot;
    SwXMLTextBlocks( const OUString& rFile );
    SwXMLTextBlocks( const css::uno::Reference < css::embed::XStorage >&, const OUString& rFile );
    void   AddName( const OUString&, const OUString&, const OUString&, bool bOnlyText );
    virtual void   AddName( const OUString&, const OUString&, bool bOnlyText = false ) override;
    static OUString GeneratePackageName ( const OUString& rShort );
    virtual ~SwXMLTextBlocks() override;
    virtual ErrCode Delete( sal_uInt16 ) override;
    virtual ErrCode Rename( sal_uInt16, const OUString&, const OUString& ) override;
    virtual ErrCode CopyBlock( SwImpBlocks& rImp, OUString& rShort, const OUString& rLong) override;
    virtual void  ClearDoc() override;
    virtual ErrCode GetDoc( sal_uInt16 ) override;
    virtual ErrCode BeginPutDoc( const OUString&, const OUString& ) override;
    virtual ErrCode PutDoc() override;
    virtual ErrCode PutText( const OUString&, const OUString&, const OUString& ) override;
    virtual ErrCode MakeBlockList() override;

    virtual FileType GetFileType() const override;
    virtual ErrCode OpenFile( bool bReadOnly = true ) override;
    virtual void  CloseFile() override;

    static bool IsFileUCBStorage( const OUString & rFileName);

    // Methods for the new Autocorrecter
    ErrCode GetText( const OUString& rShort, OUString& );

    virtual bool IsOnlyTextBlock( const OUString& rShort ) const override;
    bool IsOnlyTextBlock( sal_uInt16 nIdx ) const;
    void SetIsTextOnly( const OUString& rShort, bool bNewValue );

    virtual ErrCode GetMacroTable( sal_uInt16, SvxMacroTableDtor& rMacroTable ) override;
    virtual ErrCode SetMacroTable( sal_uInt16 nIdx,
                                 const SvxMacroTableDtor& rMacroTable ) override;
    virtual bool PutMuchEntries( bool bOn ) override;

public:
    SwDoc* GetDoc() const { return m_pDoc; }
    //void  SetDoc( SwDoc * pNewDoc);
    ErrCode StartPutBlock( const OUString& rShort, const OUString& rPackageName );
    ErrCode PutBlock();
    ErrCode GetBlockText( const OUString& rShort, OUString& rText );
    ErrCode PutBlockText( const OUString& rShort, const OUString& rText, const OUString& rPackageName );
    void MakeBlockText( const OUString& rText );

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
