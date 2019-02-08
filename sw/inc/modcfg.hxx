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

    AuthorCharAttr  m_aInsertAttr;        //Revision/TextDisplay/Insert/Attribute  // Redlining: author character attributes
                                        //Revision/TextDisplay/Insert/Color
    AuthorCharAttr  m_aDeletedAttr;       //Revision/TextDisplay/Delete/Attribute
                                        //Revision/TextDisplay/Delete/Color
    AuthorCharAttr  m_aFormatAttr;        //Revision/TextDisplay/ChangeAttribute/Attribute
                                        //Revision/TextDisplay/ChangeAttribute/Color
    sal_uInt16      m_nMarkAlign;         //Revision/LinesChanged/Mark
    Color           m_aMarkColor;         //Revision/LinesChanged/Color

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
    bool const            m_bIsWeb;

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

    sal_uInt16      m_nTableHMove;          //int Table/Shift/Row
    sal_uInt16      m_nTableVMove;          //int Table/Shift/Column
    sal_uInt16      m_nTableHInsert;        //int Table/Insert/Row
    sal_uInt16      m_nTableVInsert;        //int Table/Insert/Column
    TableChgMode  m_eTableChgMode;        //int Table/Change/Effect

    bool    m_bInsTableFormatNum;       // Table/Input/NumberRecognition        // Automatic recognition of numbers.
    bool    m_bInsTableChangeNumFormat; // Table/Input/NumberFormatRecognition  // Automatic recognition of number formats.
    bool    m_bInsTableAlignNum;        // Table/Input/Alignment                // Align numbers.

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
    SwRevisionConfig                m_aRevisionConfig;
    SwInsertConfig                  m_aInsertConfig;
    SwInsertConfig                  m_aWebInsertConfig;

    SwTableConfig                   m_aTableConfig;
    SwTableConfig                   m_aWebTableConfig;

    SwMiscConfig                    m_aMiscConfig;

    SwCompareConfig                 m_aCompareConfig;

    //fiscus: don't show tips of text fields - it's not part of the configuration!
    bool        m_bHideFieldTips : 1;

public:
    SwModuleOptions();

    TableChgMode  GetTableMode() const { return m_aTableConfig.m_eTableChgMode;}
    void        SetTableMode( TableChgMode  eSet ) { m_aTableConfig.m_eTableChgMode = eSet;
                                                     m_aTableConfig.SetModified();}

    sal_uInt16      GetTableHMove() const { return m_aTableConfig.m_nTableHMove;}
    void        SetTableHMove( sal_uInt16 nSet ) {    m_aTableConfig.m_nTableHMove = nSet;
                                                      m_aTableConfig.SetModified();}

    sal_uInt16      GetTableVMove() const { return m_aTableConfig.m_nTableVMove;}
    void        SetTableVMove( sal_uInt16 nSet ) {    m_aTableConfig.m_nTableVMove = nSet;
                                                      m_aTableConfig.SetModified();}

    sal_uInt16      GetTableHInsert() const {return m_aTableConfig.m_nTableHInsert;}
    void        SetTableHInsert( sal_uInt16 nSet ) {  m_aTableConfig.m_nTableHInsert = nSet;
                                                      m_aTableConfig.SetModified();}

    sal_uInt16      GetTableVInsert() const {return m_aTableConfig.m_nTableVInsert;}
    void        SetTableVInsert( sal_uInt16 nSet ) {  m_aTableConfig.m_nTableVInsert = nSet;
                                                      m_aTableConfig.SetModified();}

    const AuthorCharAttr    &GetInsertAuthorAttr() const { return m_aRevisionConfig.m_aInsertAttr; }
    void        SetInsertAuthorAttr( AuthorCharAttr const &rAttr ) {  m_aRevisionConfig.m_aInsertAttr = rAttr;
                                                                      m_aRevisionConfig.SetModified();}

    const AuthorCharAttr    &GetDeletedAuthorAttr() const { return m_aRevisionConfig.m_aDeletedAttr; }
    void        SetDeletedAuthorAttr( AuthorCharAttr const &rAttr ) { m_aRevisionConfig.m_aDeletedAttr = rAttr;
                                                                      m_aRevisionConfig.SetModified();}

    const AuthorCharAttr    &GetFormatAuthorAttr() const { return m_aRevisionConfig.m_aFormatAttr; }
    void        SetFormatAuthorAttr( AuthorCharAttr const &rAttr ) {  m_aRevisionConfig.m_aFormatAttr = rAttr;
                                                                      m_aRevisionConfig.SetModified();}

    sal_uInt16          GetMarkAlignMode()  const               { return m_aRevisionConfig.m_nMarkAlign; }
    void            SetMarkAlignMode(sal_uInt16 nMode)          { m_aRevisionConfig.m_nMarkAlign = nMode;
                                                                  m_aRevisionConfig.SetModified();}

    const Color&    GetMarkAlignColor() const               { return m_aRevisionConfig.m_aMarkColor; }
    void            SetMarkAlignColor(const Color &rColor)  { m_aRevisionConfig.m_aMarkColor = rColor;
                                                              m_aRevisionConfig.SetModified();}

    bool        IsInsWithCaption(bool bHTML) const
                        { return !bHTML && m_aInsertConfig.m_bInsWithCaption; }
    void        SetInsWithCaption( bool bHTML, bool b )
                    {   if(!bHTML)
                            m_aInsertConfig.m_bInsWithCaption = b;
                        m_aInsertConfig.SetModified();}

    bool    IsCaptionOrderNumberingFirst() const { return m_aInsertConfig.m_bCaptionOrderNumberingFirst; }
    void        SetCaptionOrderNumberingFirst( bool bSet )
                {
                    if(m_aInsertConfig.m_bCaptionOrderNumberingFirst != bSet)
                    {
                        m_aInsertConfig.m_bCaptionOrderNumberingFirst = bSet;
                        m_aInsertConfig.SetModified();
                    }
                }

    bool        IsInsTableFormatNum(bool bHTML) const
                    { return bHTML ? m_aWebTableConfig.m_bInsTableFormatNum : m_aTableConfig.m_bInsTableFormatNum; }
    void        SetInsTableFormatNum( bool bHTML, bool b )
                    { auto & config = bHTML ? m_aWebTableConfig : m_aTableConfig;
                      config.m_bInsTableFormatNum = b;
                      config.SetModified();}

    bool        IsInsTableChangeNumFormat(bool bHTML) const
                    { return bHTML ? m_aWebTableConfig.m_bInsTableChangeNumFormat : m_aTableConfig.m_bInsTableChangeNumFormat; }
    void        SetInsTableChangeNumFormat( bool bHTML, bool b )
                    { auto & config = bHTML ? m_aWebTableConfig : m_aTableConfig;
                      config.m_bInsTableChangeNumFormat = b;
                      config.SetModified();}

    bool        IsInsTableAlignNum(bool bHTML) const
                    { return bHTML ? m_aWebTableConfig.m_bInsTableAlignNum : m_aTableConfig.m_bInsTableAlignNum; }
    void        SetInsTableAlignNum( bool bHTML, bool b )
                    { auto & config = bHTML ? m_aWebTableConfig : m_aTableConfig;
                      config.m_bInsTableAlignNum = b;
                      config.SetModified();}

    const SwInsertTableOptions& GetInsTableFlags(bool bHTML) const
                    { return bHTML ? m_aWebInsertConfig.m_aInsTableOpts : m_aInsertConfig.m_aInsTableOpts;}
    void        SetInsTableFlags( bool bHTML, const SwInsertTableOptions& rOpts ) {
                    auto & config = bHTML ? m_aWebInsertConfig : m_aInsertConfig;
                    config.m_aInsTableOpts = rOpts;
                    config.SetModified();}

    const InsCaptionOpt* GetCapOption(bool bHTML, const SwCapObjType eType, const SvGlobalName *pOleId);
    bool        SetCapOption(bool bHTML, const InsCaptionOpt* pOpt);

    bool        IsGrfToGalleryAsLnk() const     { return m_aMiscConfig.m_bGrfToGalleryAsLnk; }
    void        SetGrfToGalleryAsLnk( bool b )  { m_aMiscConfig.m_bGrfToGalleryAsLnk = b;
                                                  m_aMiscConfig.SetModified();}

    MailTextFormats GetMailingFormats() const               { return m_aMiscConfig.m_nMailingFormats;}
    void           SetMailingFormats( MailTextFormats nSet ) { m_aMiscConfig.m_nMailingFormats = nSet;
                                                               m_aMiscConfig.SetModified();}

    void        SetSinglePrintJob( bool b )     { m_aMiscConfig.m_bSinglePrintJob = b;
                                                  m_aMiscConfig.SetModified();}

    bool        IsNameFromColumn() const        { return m_aMiscConfig.m_bIsNameFromColumn; }
    void        SetIsNameFromColumn( bool bSet )
                        {
                            m_aMiscConfig.SetModified();
                            m_aMiscConfig.m_bIsNameFromColumn = bSet;
                        }

    bool        IsAskForMailMerge() const       { return m_aMiscConfig.m_bAskForMailMergeInPrint;}

    const OUString& GetNameFromColumn() const       { return m_aMiscConfig.m_sNameFromColumn; }
    void        SetNameFromColumn( const OUString& rSet )       { m_aMiscConfig.m_sNameFromColumn = rSet;
                                                                  m_aMiscConfig.SetModified();}

    const OUString& GetMailingPath() const          { return m_aMiscConfig.m_sMailingPath; }
    void        SetMailingPath(const OUString& sPath) { m_aMiscConfig.m_sMailingPath = sPath;
                                                        m_aMiscConfig.SetModified();}

    const OUString& GetWordDelimiter() const        { return m_aMiscConfig.m_sWordDelimiter; }
    void        SetWordDelimiter(const OUString& sDelim)  { m_aMiscConfig.m_sWordDelimiter = sDelim;
                                                            m_aMiscConfig.SetModified();}

    //convert word delimiter from or to user interface
    static OUString ConvertWordDelimiter(const OUString& rDelim, bool bFromUI);

    bool    IsShowIndexPreview() const {return  m_aMiscConfig.m_bShowIndexPreview;}
    void        SetShowIndexPreview(bool bSet)
                    {m_aMiscConfig.m_bShowIndexPreview = bSet;
                     m_aMiscConfig.SetModified();}

    void        SetDefaultFontInCurrDocOnly(bool bSet)
                    {
                        m_aMiscConfig.m_bDefaultFontsInCurrDocOnly = bSet;
                        m_aMiscConfig.SetModified();
                    }

    bool    IsHideFieldTips() const {return m_bHideFieldTips;}
    void        SetHideFieldTips(bool bSet) {m_bHideFieldTips = bSet;}

    SwCompareMode  GetCompareMode() const { return m_aCompareConfig.m_eCmpMode; }
    void            SetCompareMode( SwCompareMode eMode ) { m_aCompareConfig.m_eCmpMode = eMode;
                                                            m_aCompareConfig.SetModified(); }

    bool    IsUseRsid() const { return m_aCompareConfig.m_bUseRsid; }
    void        SetUseRsid( bool b ) { m_aCompareConfig.m_bUseRsid = b;
                                       m_aCompareConfig.SetModified(); }

    bool    IsIgnorePieces() const { return m_aCompareConfig.m_bIgnorePieces; }
    void        SetIgnorePieces( bool b ) { m_aCompareConfig.m_bIgnorePieces = b;
                                            m_aCompareConfig.SetModified(); }

    sal_uInt16  GetPieceLen() const { return m_aCompareConfig.m_nPieceLen; }
    void        SetPieceLen( sal_uInt16 nLen ) { m_aCompareConfig.m_nPieceLen = nLen;
                                                 m_aCompareConfig.SetModified(); }

    bool IsStoreRsid() const
    {
        return m_aCompareConfig.m_bStoreRsid;
    }
    void SetStoreRsid(bool bStoreRsid)
    {
        m_aCompareConfig.m_bStoreRsid = bStoreRsid;
        m_aCompareConfig.SetModified();
    }

};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
