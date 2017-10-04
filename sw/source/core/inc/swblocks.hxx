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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_SWBLOCKS_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_SWBLOCKS_HXX

#include <tools/datetime.hxx>
#include <o3tl/sorted_vector.hxx>

class SwPaM;
class SwDoc;
class SvxMacroTableDtor;

// Name of a text block:

class SwBlockName
{
    friend class SwImpBlocks;
    sal_uInt16 nHashS, nHashL;     // Hash codes for testing
public:
    OUString aShort;               /// Shortname
    OUString aLong;                /// Longname
    OUString aPackageName;         /// Package name
    bool bIsOnlyTextFlagInit : 1;   /// Is the Flag valid?
    bool bIsOnlyText : 1;           /// Unformatted text

    SwBlockName( const OUString& rShort, const OUString& rLong );
    SwBlockName( const OUString& rShort, const OUString& rLong, const OUString& rPackageName );

    /// For sorting in the array
    bool operator< ( const SwBlockName& r ) const { return aShort <  r.aShort; }
};

class SwBlockNames : public o3tl::sorted_vector<SwBlockName*, o3tl::less_ptr_to<SwBlockName> > {};

class SwImpBlocks
{
    friend class SwTextBlocks;
protected:
    OUString m_aFile;                     // Physical file name
    OUString m_aName;                     // Logical file name
    OUString m_aCurrentText;                      // Current text
    OUString m_aShort, m_aLong;             // Short- and longname (PutDoc)
    OUString m_sBaseURL;                  // Base URL - has to be set at the Readers and Writers
    SwBlockNames m_aNames;                // List of all Blocks
    Date m_aDateModified;                 // For aligning the Actions
    tools::Time m_aTimeModified;
    rtl::Reference<SwDoc> m_xDoc;                        // Document to be switched
    sal_uInt16 m_nCurrentIndex;                    // Current Index
    bool m_bReadOnly : 1;
    bool m_bInPutMuchBlocks : 1;          // Put several block entries
    bool m_bInfoChanged : 1;              // Whether any info of TextBlock changed

    explicit SwImpBlocks( const OUString& );

    enum class FileType {
        NoFile,  // Not present
        None,    // No TB file
        SW3,     // SW3 file
        XML      // XML Block List
    };
    static FileType GetFileType( const OUString& );
    virtual FileType GetFileType() const = 0;

    virtual void   ClearDoc();          // Delete Doc content
    SwPaM* MakePaM();                   // Span PaM over Doc
    virtual void   AddName( const OUString&, const OUString&, bool bOnlyText = false );
    bool   IsFileChanged() const;
    void   Touch();

public:
    virtual ~SwImpBlocks();

    static sal_uInt16 Hash( const OUString& );        /// Hashcode for Block names
    sal_uInt16 GetCount() const;                      /// Get count of Text Blocks
    sal_uInt16 GetIndex( const OUString& ) const;     /// Index for shortnames
    sal_uInt16 GetLongIndex( const OUString& ) const; /// Index for longnames
    OUString GetShortName( sal_uInt16 ) const;        /// Return shortname for index
    OUString GetLongName( sal_uInt16 ) const;         /// Return longname for index
    OUString GetPackageName( sal_uInt16 ) const;      /// Return packagename for index

    const OUString& GetFileName() const {return m_aFile;}      /// Return physical file name
    void SetName( const OUString& rName )             /// Logic name
        { m_aName = rName; m_bInfoChanged = true; }
    const OUString& GetName()
        { return m_aName; }

    const OUString&     GetBaseURL() const { return m_sBaseURL;}
    void                SetBaseURL( const OUString& rURL ) { m_sBaseURL = rURL; }

    virtual ErrCode Delete( sal_uInt16 ) = 0;
    virtual ErrCode Rename( sal_uInt16, const OUString&, const OUString& ) = 0;
    virtual ErrCode CopyBlock( SwImpBlocks& rImp, OUString& rShort, const OUString& rLong) = 0;
    virtual ErrCode GetDoc( sal_uInt16 ) = 0;
    virtual ErrCode BeginPutDoc( const OUString&, const OUString& ) = 0;
    virtual ErrCode PutDoc() = 0;
    virtual ErrCode PutText( const OUString&, const OUString&, const OUString& ) = 0;
    virtual ErrCode MakeBlockList() = 0;

    virtual ErrCode OpenFile( bool bReadOnly = true ) = 0;
    virtual void  CloseFile() = 0;

    virtual bool IsOnlyTextBlock( const OUString& rShort ) const;

    virtual ErrCode GetMacroTable( sal_uInt16 nIdx, SvxMacroTableDtor& rMacroTable );
    virtual ErrCode SetMacroTable( sal_uInt16 nIdx,
                                 const SvxMacroTableDtor& rMacroTable );
    virtual bool PutMuchEntries( bool bOn );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
