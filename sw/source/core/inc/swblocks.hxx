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

// Name eines Textblocks:

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
    bool operator==( const SwBlockName& r ) const { return aShort == r.aShort; }
    bool operator< ( const SwBlockName& r ) const { return aShort <  r.aShort; }
};

class SwBlockNames : public o3tl::sorted_vector<SwBlockName*, o3tl::less_ptr_to<SwBlockName> > {};

class SwImpBlocks
{
    friend class SwTextBlocks;
protected:
    OUString aFile;                     // Physical file name
    OUString aName;                     // Logical file name
    OUString aCur;                      // Current text
    OUString aShort, aLong;             // Short- and longname (PutDoc)
    OUString sBaseURL;                  // Base URL - has to be set at the Readers and Writers
    SwBlockNames aNames;                // List of all Blocks
    Date aDateModified;                 // For aligning the Actions
    tools::Time aTimeModified;
    SwDoc* pDoc;                        // Document to be switched
    sal_uInt16 nCur;                    // Current Index
    bool bReadOnly : 1;
    bool bInPutMuchBlocks : 1;          // Put serveral block entries
    bool bInfoChanged : 1;              // Whether any info of TextBlock changed

    SwImpBlocks( const OUString&, bool = false );
    virtual ~SwImpBlocks();

    static short GetFileType( const OUString& );
    virtual short GetFileType() const = 0;
#define SWBLK_NO_FILE   0               // Not present
#define SWBLK_NONE      1               // No TB file
#define SWBLK_SW2       2               // SW2 file
#define SWBLK_SW3       3               // SW3 file
#define SWBLK_XML       4               // XML Block List

    virtual void   ClearDoc();          // Delete Doc content
    SwPaM* MakePaM();                   // Span PaM over Doc
    virtual void   AddName( const OUString&, const OUString&, bool bOnlyText = false );
    bool   IsFileChanged() const;
    void   Touch();

public:
    static sal_uInt16 Hash( const OUString& );        /// Hashcode for Block names
    sal_uInt16 GetCount() const;                      /// Get count of Text Blocks
    sal_uInt16 GetIndex( const OUString& ) const;     /// Index for shortnames
    sal_uInt16 GetLongIndex( const OUString& ) const; /// Index for longnames
    OUString GetShortName( sal_uInt16 ) const;        /// Return shortname for index
    OUString GetLongName( sal_uInt16 ) const;         /// Return longname for index
    OUString GetPackageName( sal_uInt16 ) const;      /// Return packagename for index

    OUString GetFileName() const {return aFile;}      /// Return physical file name
    void SetName( const OUString& rName )             /// Logic name
        { aName = rName; bInfoChanged = true; }
    OUString GetName()
        { return aName; }

    OUString            GetBaseURL() const { return sBaseURL;}
    void                SetBaseURL( const OUString& rURL ) { sBaseURL = rURL; }

    virtual sal_uLong Delete( sal_uInt16 ) = 0;
    virtual sal_uLong Rename( sal_uInt16, const OUString&, const OUString& ) = 0;
    virtual sal_uLong CopyBlock( SwImpBlocks& rImp, OUString& rShort, const OUString& rLong) = 0;
    virtual sal_uLong GetDoc( sal_uInt16 ) = 0;
    virtual sal_uLong BeginPutDoc( const OUString&, const OUString& ) = 0;
    virtual sal_uLong PutDoc() = 0;
    virtual sal_uLong PutText( const OUString&, const OUString&, const OUString& ) = 0;
    virtual sal_uLong MakeBlockList() = 0;

    virtual sal_uLong OpenFile( bool bReadOnly = true ) = 0;
    virtual void  CloseFile() = 0;

    virtual bool IsOnlyTextBlock( const OUString& rShort ) const;

    virtual sal_uLong GetMacroTable( sal_uInt16 nIdx, SvxMacroTableDtor& rMacroTable,
                                 bool bFileAlreadyOpen = false );
    virtual sal_uLong SetMacroTable( sal_uInt16 nIdx,
                                 const SvxMacroTableDtor& rMacroTable,
                                 bool bFileAlreadyOpen = false );
    virtual bool PutMuchEntries( bool bOn );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
