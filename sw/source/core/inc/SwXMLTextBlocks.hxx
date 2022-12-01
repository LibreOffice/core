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

#pragma once

#include <sal/config.h>

#include <string_view>

#include <sfx2/objsh.hxx>
#include "swblocks.hxx"
#include <o3tl/typed_flags_set.hxx>

class SwDoc;
class SvxMacroTableDtor;

enum class SwXmlFlags {
    NONE         = 0x0000,
    NoRootCommit = 0x0002,
};
namespace o3tl {
    template<> struct typed_flags<SwXmlFlags> : is_typed_flags<SwXmlFlags, 0x0002> {};
}

class SwXMLTextBlocks final : public SwImpBlocks
{
    SfxObjectShellRef       m_xDocShellRef;
    SwXmlFlags              m_nFlags;
    OUString                m_aPackageName;
    tools::SvRef<SfxMedium> m_xMedium;
    css::uno::Reference < css::embed::XStorage > m_xBlkRoot;
    css::uno::Reference < css::embed::XStorage > m_xRoot;

    void ReadInfo();
    void WriteInfo();
    void InitBlockMode ( const css::uno::Reference < css::embed::XStorage >& rStorage );
    void ResetBlockMode();

public:
    SwXMLTextBlocks( const OUString& rFile );
    SwXMLTextBlocks( const css::uno::Reference < css::embed::XStorage >&, const OUString& rFile );
    void   AddName( const OUString&, const OUString&, const OUString&, bool bOnlyText );
    virtual void   AddName( const OUString&, const OUString&, bool bOnlyText = false ) override;
    static OUString GeneratePackageName ( std::u16string_view rShort );
    virtual ~SwXMLTextBlocks() override;
    virtual ErrCode Delete( sal_uInt16 ) override;
    virtual ErrCode Rename( sal_uInt16, const OUString& ) override;
    virtual ErrCode CopyBlock( SwImpBlocks& rImp, OUString& rShort, const OUString& rLong) override;
    virtual void  ClearDoc() override;
    virtual ErrCode GetDoc( sal_uInt16 ) override;
    virtual ErrCode BeginPutDoc( const OUString&, const OUString& ) override;
    virtual ErrCode PutDoc() override;
    virtual ErrCode PutText( const OUString&, const OUString&, const OUString& ) override;
    virtual ErrCode MakeBlockList() override;

    virtual ErrCode OpenFile( bool bReadOnly = true ) override;
    virtual void  CloseFile() override;

    static bool IsFileUCBStorage( const OUString & rFileName);

    // Methods for the new Autocorrecter
    ErrCode GetText( std::u16string_view rShort, OUString& );

    virtual bool IsOnlyTextBlock( const OUString& rShort ) const override;
    bool IsOnlyTextBlock( sal_uInt16 nIdx ) const;
    void SetIsTextOnly( const OUString& rShort, bool bNewValue );

    virtual ErrCode GetMacroTable( sal_uInt16, SvxMacroTableDtor& rMacroTable ) override;
    virtual ErrCode SetMacroTable( sal_uInt16 nIdx,
                                 const SvxMacroTableDtor& rMacroTable ) override;
    virtual bool PutMuchEntries( bool bOn ) override;

    SwDoc* GetDoc() const { return m_xDoc.get(); }
    //void  SetDoc( SwDoc * pNewDoc);
    ErrCode StartPutBlock( const OUString& rShort, const OUString& rPackageName );
    ErrCode PutBlock();
    ErrCode GetBlockText( std::u16string_view rShort, OUString& rText );
    ErrCode PutBlockText( const OUString& rShort, std::u16string_view rText, const OUString& rPackageName );
    void MakeBlockText( std::u16string_view rText );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
