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
#ifndef INCLUDED_SW_INC_MODCFG_HXX
#define INCLUDED_SW_INC_MODCFG_HXX

#include <unotools/configitem.hxx>
#include "swdllapi.h"
#include "authratr.hxx"
#include "SwCapObjType.hxx"
#include "tblenum.hxx"
#include "itabenum.hxx"
#include <tools/globname.hxx>
#include <o3tl/typed_flags_set.hxx>

#include <vector>
#include <memory>

class InsCaptionOpt;

// text format for the sending of messages ------------------------------
enum class MailTextFormats
{
    NONE      = 0x00,
    HTML      = 0x01,
    RTF       = 0x02,
    OFFICE    = 0x04
};
namespace o3tl
{
    template<> struct typed_flags<MailTextFormats> : is_typed_flags<MailTextFormats, 0x07> {};
}


class InsCaptionOptArr
{
private:
    typedef std::vector<std::unique_ptr<InsCaptionOpt>> InsCapOptArr;
    InsCapOptArr m_InsCapOptArr;
public:
    InsCaptionOpt* Find(const SwCapObjType eType, const SvGlobalName *pOleId = nullptr);
    void Insert(InsCaptionOpt* pObj);
};

class SwRevisionConfig : public utl::ConfigItem
{
    friend class SwModuleOptions;

    AuthorCharAttr  aInsertAttr;        //Revision/TextDisplay/Insert/Attribute  // Redlining: author character attributes
                                        //Revision/TextDisplay/Insert/Color
    AuthorCharAttr  aDeletedAttr;       //Revision/TextDisplay/Delete/Attribute
                                        //Revision/TextDisplay/Delete/Color
    AuthorCharAttr  aFormatAttr;        //Revision/TextDisplay/ChangeAttribute/Attribute
                                        //Revision/TextDisplay/ChangeAttribute/Color
    sal_uInt16      nMarkAlign;         //Revision/LinesChanged/Mark
    Color           aMarkColor;         //Revision/LinesChanged/Color

    static const css::uno::Sequence<OUString>& GetPropertyNames();

    virtual void            ImplCommit() override;

public:
    SwRevisionConfig();
    virtual ~SwRevisionConfig() override;

    virtual void Notify( const css::uno::Sequence< OUString >& aPropertyNames ) override;
    void                    Load();
    using ConfigItem::SetModified;
};

enum class SwCompareMode
{
    Auto = 0,
    ByWord,
    ByChar
};

class SwCompareConfig : public utl::ConfigItem
{
    friend class SwModuleOptions;

    SwCompareMode  m_eCmpMode;       //Compare/CompareDocuments;
    bool            m_bUseRsid;       //Compare/Settings/Use RSID
    /// Compare/Settings/Store RSID
    bool            m_bStoreRsid;
    bool            m_bIgnorePieces;  //Compare/Settings/Ignore pieces of length
    sal_uInt16      m_nPieceLen;      //Compare/Settings/Ignore pieces of length

    static const css::uno::Sequence<OUString>& GetPropertyNames();

    virtual void    ImplCommit() override;

public:
    SwCompareConfig();
    virtual ~SwCompareConfig() override;

    virtual void    Notify( const css::uno::Sequence< OUString >& ) override { };
    void            Load();
    using ConfigItem::SetModified;
};

class SwInsertConfig : public utl::ConfigItem
{
    friend class SwModuleOptions;

    std::unique_ptr<InsCaptionOptArr> m_pCapOptions;
    std::unique_ptr<InsCaptionOpt>    m_pOLEMiscOpt;

    SvGlobalName        m_aGlobalNames[5];

    bool            m_bInsWithCaption;       //Insert/Caption/Automatic
    bool            m_bCaptionOrderNumberingFirst; //#i61007# caption order starting with numbering

    SwInsertTableOptions    m_aInsTableOpts;
    bool            m_bIsWeb;

    const css::uno::Sequence<OUString>& GetPropertyNames();

    virtual void    ImplCommit() override;

public:
    SwInsertConfig(bool bWeb);
    virtual ~SwInsertConfig() override;

    virtual void Notify( const css::uno::Sequence< OUString >& aPropertyNames ) override;
    void                    Load();
    using ConfigItem::SetModified;
};

class SwTableConfig : public utl::ConfigItem
{
    friend class SwModuleOptions;

    sal_uInt16      nTableHMove;          //int Table/Shift/Row
    sal_uInt16      nTableVMove;          //int Table/Shift/Column
    sal_uInt16      nTableHInsert;        //int Table/Insert/Row
    sal_uInt16      nTableVInsert;        //int Table/Insert/Column
    TableChgMode  eTableChgMode;        //int Table/Change/Effect

    bool    bInsTableFormatNum;       // Table/Input/NumberRecognition        // Automatic recognition of numbers.
    bool    bInsTableChangeNumFormat; // Table/Input/NumberFormatRecognition  // Automatic recognition of number formats.
    bool    bInsTableAlignNum;        // Table/Input/Alignment                // Align numbers.

    static const css::uno::Sequence<OUString>& GetPropertyNames();

    virtual void    ImplCommit() override;

public:
    SwTableConfig(bool bWeb);
    virtual ~SwTableConfig() override;

    virtual void Notify( const css::uno::Sequence< OUString >& aPropertyNames ) override;
    void                    Load();
    using ConfigItem::SetModified;
};

class SwMiscConfig : public utl::ConfigItem
{
    friend class SwModuleOptions;

    OUString    m_sWordDelimiter;             // Statistics/WordNumber/Delimiter
    bool        m_bDefaultFontsInCurrDocOnly; // DefaultFont/Document
    bool        m_bShowIndexPreview;          // Index/ShowPreview
    bool        m_bGrfToGalleryAsLnk;         // Misc/GraphicToGalleryAsLink
    bool        m_bNumAlignSize;              // Numbering/Graphic/KeepRatio
    bool        m_bSinglePrintJob;            // FormLetter/PrintOutput/SinglePrintJobs
    bool        m_bIsNameFromColumn;          // FormLetter/FileOutput/FileName/Generation
    bool        m_bAskForMailMergeInPrint;    // Ask if documents containing fields should be 'mailmerged'
    MailTextFormats m_nMailingFormats;            // FormLetter/MailingOutput/Formats
    OUString    m_sNameFromColumn;            // FormLetter/FileOutput/FileName/FromDatabaseField (string!)
    OUString    m_sMailingPath;               // FormLetter/FileOutput/Path
    OUString    m_sMailName;                  // FormLetter/FileOutput/FileName/FromManualSetting (string!)

    static const css::uno::Sequence<OUString>& GetPropertyNames();

    virtual void    ImplCommit() override;

public:
    SwMiscConfig();
    virtual ~SwMiscConfig() override;

    virtual void Notify( const css::uno::Sequence< OUString >& aPropertyNames ) override;
    void                    Load();
    using ConfigItem::SetModified;
};

class SW_DLLPUBLIC SwModuleOptions
{
    SwRevisionConfig                aRevisionConfig;
    SwInsertConfig                  aInsertConfig;
    SwInsertConfig                  aWebInsertConfig;

    SwTableConfig                   aTableConfig;
    SwTableConfig                   aWebTableConfig;

    SwMiscConfig                    aMiscConfig;

    SwCompareConfig                 aCompareConfig;

    //fiscus: don't show tips of text fields - it's not part of the configuration!
    bool        bHideFieldTips : 1;

public:
    SwModuleOptions();

    TableChgMode  GetTableMode() const { return aTableConfig.eTableChgMode;}
    void        SetTableMode( TableChgMode  eSet ) { aTableConfig.eTableChgMode = eSet;
                                                aTableConfig.SetModified();}

    sal_uInt16      GetTableHMove() const { return aTableConfig.nTableHMove;}
    void        SetTableHMove( sal_uInt16 nSet ) {    aTableConfig.nTableHMove = nSet;
                                                aTableConfig.SetModified();}

    sal_uInt16      GetTableVMove() const { return aTableConfig.nTableVMove;}
    void        SetTableVMove( sal_uInt16 nSet ) {    aTableConfig.nTableVMove = nSet;
                                                aTableConfig.SetModified();}

    sal_uInt16      GetTableHInsert() const {return aTableConfig.nTableHInsert;}
    void        SetTableHInsert( sal_uInt16 nSet ) {  aTableConfig.nTableHInsert = nSet;
                                                aTableConfig.SetModified();}

    sal_uInt16      GetTableVInsert() const {return aTableConfig.nTableVInsert;}
    void        SetTableVInsert( sal_uInt16 nSet ) {  aTableConfig.nTableVInsert = nSet;
                                                aTableConfig.SetModified();}

    const AuthorCharAttr    &GetInsertAuthorAttr() const { return aRevisionConfig.aInsertAttr; }
    void        SetInsertAuthorAttr( AuthorCharAttr const &rAttr ) {  aRevisionConfig.aInsertAttr = rAttr;
                                                                     aRevisionConfig.SetModified();}

    const AuthorCharAttr    &GetDeletedAuthorAttr() const { return aRevisionConfig.aDeletedAttr; }
    void        SetDeletedAuthorAttr( AuthorCharAttr const &rAttr ) { aRevisionConfig.aDeletedAttr = rAttr;
                                                                      aRevisionConfig.SetModified();}

    const AuthorCharAttr    &GetFormatAuthorAttr() const { return aRevisionConfig.aFormatAttr; }
    void        SetFormatAuthorAttr( AuthorCharAttr const &rAttr ) {  aRevisionConfig.aFormatAttr = rAttr;
                                                                      aRevisionConfig.SetModified();}

    sal_uInt16          GetMarkAlignMode()  const               { return aRevisionConfig.nMarkAlign; }
    void            SetMarkAlignMode(sal_uInt16 nMode)          { aRevisionConfig.nMarkAlign = nMode;
                                                              aRevisionConfig.SetModified();}

    const Color&    GetMarkAlignColor() const               { return aRevisionConfig.aMarkColor; }
    void            SetMarkAlignColor(const Color &rColor)  { aRevisionConfig.aMarkColor = rColor;
                                                              aRevisionConfig.SetModified();}

    bool        IsInsWithCaption(bool bHTML) const
                        { return !bHTML && aInsertConfig.m_bInsWithCaption; }
    void        SetInsWithCaption( bool bHTML, bool b )
                    {   if(!bHTML)
                            aInsertConfig.m_bInsWithCaption = b;
                        aInsertConfig.SetModified();}

    bool    IsCaptionOrderNumberingFirst() const { return aInsertConfig.m_bCaptionOrderNumberingFirst; }
    void        SetCaptionOrderNumberingFirst( bool bSet )
                {
                    if(aInsertConfig.m_bCaptionOrderNumberingFirst != bSet)
                    {
                        aInsertConfig.m_bCaptionOrderNumberingFirst = bSet;
                        aInsertConfig.SetModified();
                    }
                }

    bool        IsInsTableFormatNum(bool bHTML) const
                    { return bHTML ? aWebTableConfig.bInsTableFormatNum : aTableConfig.bInsTableFormatNum; }
    void        SetInsTableFormatNum( bool bHTML, bool b )
                    { bHTML ? (aWebTableConfig.bInsTableFormatNum = b) : (aTableConfig.bInsTableFormatNum = b);
                      bHTML ? aWebTableConfig.SetModified() : aTableConfig.SetModified();}

    bool        IsInsTableChangeNumFormat(bool bHTML) const
                    { return bHTML ? aWebTableConfig.bInsTableChangeNumFormat : aTableConfig.bInsTableChangeNumFormat; }
    void        SetInsTableChangeNumFormat( bool bHTML, bool b )
                    { bHTML ? (aWebTableConfig.bInsTableChangeNumFormat = b) : (aTableConfig.bInsTableChangeNumFormat = b);
                      bHTML ? aWebTableConfig.SetModified() : aTableConfig.SetModified();}

    bool        IsInsTableAlignNum(bool bHTML) const
                    { return bHTML ? aWebTableConfig.bInsTableAlignNum : aTableConfig.bInsTableAlignNum; }
    void        SetInsTableAlignNum( bool bHTML, bool b )
                    { bHTML ? (aWebTableConfig.bInsTableAlignNum = b) : (aTableConfig.bInsTableAlignNum = b);
                        bHTML ? aWebTableConfig.SetModified() : aTableConfig.SetModified();}

    const SwInsertTableOptions& GetInsTableFlags(bool bHTML) const
                    { return bHTML ? aWebInsertConfig.m_aInsTableOpts : aInsertConfig.m_aInsTableOpts;}
    void        SetInsTableFlags( bool bHTML, const SwInsertTableOptions& rOpts ) {
                    bHTML ? (aWebInsertConfig.m_aInsTableOpts = rOpts) : (aInsertConfig.m_aInsTableOpts = rOpts);
                    bHTML ? aWebInsertConfig.SetModified() : aInsertConfig.SetModified();}

    const InsCaptionOpt* GetCapOption(bool bHTML, const SwCapObjType eType, const SvGlobalName *pOleId);
    bool        SetCapOption(bool bHTML, const InsCaptionOpt* pOpt);

    bool        IsGrfToGalleryAsLnk() const     { return aMiscConfig.m_bGrfToGalleryAsLnk; }
    void        SetGrfToGalleryAsLnk( bool b )  { aMiscConfig.m_bGrfToGalleryAsLnk = b;
                                                  aMiscConfig.SetModified();}

    MailTextFormats GetMailingFormats() const               { return aMiscConfig.m_nMailingFormats;}
    void           SetMailingFormats( MailTextFormats nSet ) { aMiscConfig.m_nMailingFormats = nSet;
                                                            aMiscConfig.SetModified();}

    void        SetSinglePrintJob( bool b )     { aMiscConfig.m_bSinglePrintJob = b;
                                                  aMiscConfig.SetModified();}

    bool        IsNameFromColumn() const        { return aMiscConfig.m_bIsNameFromColumn; }
    void        SetIsNameFromColumn( bool bSet )
                        {
                            aMiscConfig.SetModified();
                            aMiscConfig.m_bIsNameFromColumn = bSet;
                        }

    bool        IsAskForMailMerge() const       { return aMiscConfig.m_bAskForMailMergeInPrint;}

    const OUString& GetNameFromColumn() const       { return aMiscConfig.m_sNameFromColumn; }
    void        SetNameFromColumn( const OUString& rSet )       { aMiscConfig.m_sNameFromColumn = rSet;
                                                                  aMiscConfig.SetModified();}

    const OUString& GetMailingPath() const          { return aMiscConfig.m_sMailingPath; }
    void        SetMailingPath(const OUString& sPath) { aMiscConfig.m_sMailingPath = sPath;
                                                      aMiscConfig.SetModified();}

    const OUString& GetWordDelimiter() const        { return aMiscConfig.m_sWordDelimiter; }
    void        SetWordDelimiter(const OUString& sDelim)  { aMiscConfig.m_sWordDelimiter = sDelim;
                                                          aMiscConfig.SetModified();}

    //convert word delimiter from or to user interface
    static OUString ConvertWordDelimiter(const OUString& rDelim, bool bFromUI);

    bool    IsShowIndexPreview() const {return  aMiscConfig.m_bShowIndexPreview;}
    void        SetShowIndexPreview(bool bSet)
                    {aMiscConfig.m_bShowIndexPreview = bSet;
                    aMiscConfig.SetModified();}

    void        SetDefaultFontInCurrDocOnly(bool bSet)
                    {
                        aMiscConfig.m_bDefaultFontsInCurrDocOnly = bSet;
                        aMiscConfig.SetModified();
                    }

    bool    IsHideFieldTips() const {return bHideFieldTips;}
    void        SetHideFieldTips(bool bSet) {bHideFieldTips = bSet;}

    SwCompareMode  GetCompareMode() const { return aCompareConfig.m_eCmpMode; }
    void            SetCompareMode( SwCompareMode eMode ) { aCompareConfig.m_eCmpMode = eMode;
                                                             aCompareConfig.SetModified(); }

    bool    IsUseRsid() const { return aCompareConfig.m_bUseRsid; }
    void        SetUseRsid( bool b ) { aCompareConfig.m_bUseRsid = b;
                                                            aCompareConfig.SetModified(); }

    bool    IsIgnorePieces() const { return aCompareConfig.m_bIgnorePieces; }
    void        SetIgnorePieces( bool b ) { aCompareConfig.m_bIgnorePieces = b;
                                                aCompareConfig.SetModified(); }

    sal_uInt16  GetPieceLen() const { return aCompareConfig.m_nPieceLen; }
    void        SetPieceLen( sal_uInt16 nLen ) { aCompareConfig.m_nPieceLen = nLen;
                                                 aCompareConfig.SetModified(); }

    bool IsStoreRsid() const
    {
        return aCompareConfig.m_bStoreRsid;
    }
    void SetStoreRsid(bool bStoreRsid)
    {
        aCompareConfig.m_bStoreRsid = bStoreRsid;
        aCompareConfig.SetModified();
    }

};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
