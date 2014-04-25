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

#include <boost/ptr_container/ptr_vector.hpp>
#include <tools/wintypes.hxx>
#include <vcl/field.hxx>
#include <unotools/configitem.hxx>
#include "swdllapi.h"
#include "authratr.hxx"
#include <SwCapObjType.hxx>
#include "tblenum.hxx"
#include "itabenum.hxx"
#include <tools/globname.hxx>
#include <editeng/svxenum.hxx>
class SwModuleOptions;

class InsCaptionOpt;

class InsCaptionOptArr
{
private:
    typedef boost::ptr_vector<InsCaptionOpt> InsCapOptArr;
    InsCapOptArr m_aInsCapOptArr;
public:
    InsCaptionOpt* Find(const SwCapObjType eType, const SvGlobalName *pOleId = 0);
    void Insert(InsCaptionOpt* pObj);
};

class SwRevisionConfig : public utl::ConfigItem
{
    friend class SwModuleOptions;

    AuthorCharAttr  aInsertAttr;    //Revision/TextDisplay/Insert/Attribute  // Redlining: Author-Zeichenattribute
                                    //Revision/TextDisplay/Insert/Color
    AuthorCharAttr  aDeletedAttr;   //Revision/TextDisplay/Delete/Attribute
                                    //Revision/TextDisplay/Delete/Color
    AuthorCharAttr  aFormatAttr;    //Revision/TextDisplay/ChangeAttribute/Attribute
                                    //Revision/TextDisplay/ChangeAttribute/Color
    sal_uInt16          nMarkAlign;     //Revision/LinesChanged/Mark
    Color           aMarkColor;     //Revision/LinesChanged/Color

    const com::sun::star::uno::Sequence<OUString>& GetPropertyNames();
    public:
        SwRevisionConfig();
        virtual ~SwRevisionConfig();

    virtual void            Commit() SAL_OVERRIDE;
    virtual void Notify( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) SAL_OVERRIDE;
    void                    Load();
    void                    SetModified(){ConfigItem::SetModified();}
};

class SwCompareConfig : public utl::ConfigItem
{
    friend class SwModuleOptions;

    sal_uInt16      eCmpMode;		//Compare/CompareDocuments;
    bool            bUseRsid;       //Compare/Settings/Use RSID
    bool            bIgnorePieces;  //Compare/Settings/Ignore pieces of length
    sal_uInt16      nPieceLen;		//Compare/Settings/Ignore pieces of length

    const com::sun::star::uno::Sequence<OUString>& GetPropertyNames();
    public:
        SwCompareConfig();
        virtual ~SwCompareConfig();

    virtual void    Commit() SAL_OVERRIDE;
    virtual void    Notify( const ::com::sun::star::uno::Sequence< OUString >& ) SAL_OVERRIDE { };
    void            Load();
    void            SetModified() {ConfigItem::SetModified(); }
};

class SwInsertConfig : public utl::ConfigItem
{
    friend class SwModuleOptions;

    InsCaptionOptArr*   pCapOptions;
    InsCaptionOpt*      pOLEMiscOpt;

    SvGlobalName        aGlobalNames[5];

    bool            bInsWithCaption;       //Insert/Caption/Automatic
    bool            bCaptionOrderNumberingFirst; //#i61007# caption order starting with numbering

    SwInsertTableOptions    aInsTblOpts;
    bool            bIsWeb;

    const com::sun::star::uno::Sequence<OUString>& GetPropertyNames();
    public:
        SwInsertConfig(bool bWeb);
        virtual ~SwInsertConfig();

    virtual void            Commit() SAL_OVERRIDE;
    virtual void Notify( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) SAL_OVERRIDE;
    void                    Load();
    void                    SetModified(){ConfigItem::SetModified();}
};

class SwTableConfig : public utl::ConfigItem
{
    friend class SwModuleOptions;

    sal_uInt16      nTblHMove;          //int Table/Shift/Row
    sal_uInt16      nTblVMove;          //int Table/Shift/Column
    sal_uInt16      nTblHInsert;        //int Table/Insert/Row
    sal_uInt16      nTblVInsert;        //int Table/Insert/Column
    TblChgMode  eTblChgMode;        //int Table/Change/Effect

    bool    bInsTblFormatNum;       // Table/Input/NumberRecognition        // Automatic recognition of numbers.
    bool    bInsTblChangeNumFormat; // Table/Input/NumberFormatRecognition  // Automatic recognition of number formats.
    bool    bInsTblAlignNum;        // Table/Input/Alignment                // Align numbers.

    const com::sun::star::uno::Sequence<OUString>& GetPropertyNames();
    public:
        SwTableConfig(bool bWeb);
        virtual ~SwTableConfig();

    virtual void            Commit() SAL_OVERRIDE;
    virtual void Notify( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) SAL_OVERRIDE;
    void                    Load();
    void                    SetModified(){ConfigItem::SetModified();}
};

class SwMiscConfig : public utl::ConfigItem
{
    friend class SwModuleOptions;

    OUString    sWordDelimiter;             // Statistics/WordNumber/Delimiter
    bool        bDefaultFontsInCurrDocOnly; // DefaultFont/Document
    bool        bShowIndexPreview;          // Index/ShowPreview
    bool        bGrfToGalleryAsLnk;         // Misc/GraphicToGalleryAsLink
    bool        bNumAlignSize;              // Numbering/Graphic/KeepRatio
    bool        bSinglePrintJob;            // FormLetter/PrintOutput/SinglePrintJobs
    bool        bIsNameFromColumn;          // FormLetter/FileOutput/FileName/Generation
    bool        bAskForMailMergeInPrint;    // Ask if documents containing fields should be 'mailmerged'
    sal_Int16   nMailingFormats;            // FormLetter/MailingOutput/Formats
    OUString    sNameFromColumn;            // FormLetter/FileOutput/FileName/FromDatabaseField (string!)
    OUString    sMailingPath;               // FormLetter/FileOutput/Path
    OUString    sMailName;                  // FormLetter/FileOutput/FileName/FromManualSetting (string!)

    const com::sun::star::uno::Sequence<OUString>& GetPropertyNames();
    public:
        SwMiscConfig();
        virtual ~SwMiscConfig();

    virtual void            Commit() SAL_OVERRIDE;
    virtual void Notify( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) SAL_OVERRIDE;
    void                    Load();
    void                    SetModified(){ConfigItem::SetModified();}
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

    TblChgMode  GetTblMode() const { return aTableConfig.eTblChgMode;}
    void        SetTblMode( TblChgMode  eSet ) { aTableConfig.eTblChgMode = eSet;
                                                aTableConfig.SetModified();}

    sal_uInt16      GetTblHMove() const { return aTableConfig.nTblHMove;}
    void        SetTblHMove( sal_uInt16 nSet ) {    aTableConfig.nTblHMove = nSet;
                                                aTableConfig.SetModified();}

    sal_uInt16      GetTblVMove() const { return aTableConfig.nTblVMove;}
    void        SetTblVMove( sal_uInt16 nSet ) {    aTableConfig.nTblVMove = nSet;
                                                aTableConfig.SetModified();}

    sal_uInt16      GetTblHInsert() const {return aTableConfig.nTblHInsert;}
    void        SetTblHInsert( sal_uInt16 nSet ) {  aTableConfig.nTblHInsert = nSet;
                                                aTableConfig.SetModified();}

    sal_uInt16      GetTblVInsert() const {return aTableConfig.nTblVInsert;}
    void        SetTblVInsert( sal_uInt16 nSet ) {  aTableConfig.nTblVInsert = nSet;
                                                aTableConfig.SetModified();}

    const AuthorCharAttr    &GetInsertAuthorAttr() const { return aRevisionConfig.aInsertAttr; }
    void        SetInsertAuthorAttr( AuthorCharAttr &rAttr ) {  aRevisionConfig.aInsertAttr = rAttr;
                                                                aRevisionConfig.SetModified();}

    const AuthorCharAttr    &GetDeletedAuthorAttr() const { return aRevisionConfig.aDeletedAttr; }
    void        SetDeletedAuthorAttr( AuthorCharAttr &rAttr ) { aRevisionConfig.aDeletedAttr = rAttr;
                                                                aRevisionConfig.SetModified();}

    const AuthorCharAttr    &GetFormatAuthorAttr() const { return aRevisionConfig.aFormatAttr; }
    void        SetFormatAuthorAttr( AuthorCharAttr &rAttr ) {  aRevisionConfig.aFormatAttr = rAttr;
                                                                aRevisionConfig.SetModified();}

    sal_uInt16          GetMarkAlignMode()  const               { return aRevisionConfig.nMarkAlign; }
    void            SetMarkAlignMode(sal_uInt16 nMode)          { aRevisionConfig.nMarkAlign = nMode;
                                                              aRevisionConfig.SetModified();}

    const Color&    GetMarkAlignColor() const               { return aRevisionConfig.aMarkColor; }
    void            SetMarkAlignColor(const Color &rColor)  { aRevisionConfig.aMarkColor = rColor;
                                                              aRevisionConfig.SetModified();}

    bool        IsInsWithCaption(bool bHTML) const
                        { return bHTML ? sal_False : aInsertConfig.bInsWithCaption; }
    void        SetInsWithCaption( bool bHTML, bool b )
                    {   if(!bHTML)
                            aInsertConfig.bInsWithCaption = b;
                        aInsertConfig.SetModified();}

    bool    IsCaptionOrderNumberingFirst() const { return aInsertConfig.bCaptionOrderNumberingFirst; }
    void        SetCaptionOrderNumberingFirst( bool bSet )
                {
                    if(aInsertConfig.bCaptionOrderNumberingFirst != bSet)
                    {
                        aInsertConfig.bCaptionOrderNumberingFirst = bSet;
                        aInsertConfig.SetModified();
                    }
                }

    bool        IsInsTblFormatNum(bool bHTML) const
                    { return bHTML ? aWebTableConfig.bInsTblFormatNum : aTableConfig.bInsTblFormatNum; }
    void        SetInsTblFormatNum( bool bHTML, bool b )
                    { bHTML ? (aWebTableConfig.bInsTblFormatNum = b) : (aTableConfig.bInsTblFormatNum = b);
                      bHTML ? aWebTableConfig.SetModified() : aTableConfig.SetModified();}

    bool        IsInsTblChangeNumFormat(bool bHTML) const
                    { return bHTML ? aWebTableConfig.bInsTblChangeNumFormat : aTableConfig.bInsTblChangeNumFormat; }
    void        SetInsTblChangeNumFormat( bool bHTML, bool b )
                    { bHTML ? (aWebTableConfig.bInsTblChangeNumFormat = b) : (aTableConfig.bInsTblChangeNumFormat = b);
                      bHTML ? aWebTableConfig.SetModified() : aTableConfig.SetModified();}

    bool        IsInsTblAlignNum(bool bHTML) const
                    { return bHTML ? aWebTableConfig.bInsTblAlignNum : aTableConfig.bInsTblAlignNum; }
    void        SetInsTblAlignNum( bool bHTML, bool b )
                    { bHTML ? (aWebTableConfig.bInsTblAlignNum = b) : (aTableConfig.bInsTblAlignNum = b);
                        bHTML ? aWebTableConfig.SetModified() : aTableConfig.SetModified();}

    SwInsertTableOptions GetInsTblFlags(bool bHTML) const
                    { return bHTML ? aWebInsertConfig.aInsTblOpts : aInsertConfig.aInsTblOpts;}
    void        SetInsTblFlags( bool bHTML, const SwInsertTableOptions& rOpts ) {
                    bHTML ? (aWebInsertConfig.aInsTblOpts = rOpts) : (aInsertConfig.aInsTblOpts = rOpts);
                    bHTML ? aWebInsertConfig.SetModified() : aInsertConfig.SetModified();}

    const InsCaptionOpt* GetCapOption(bool bHTML, const SwCapObjType eType, const SvGlobalName *pOleId);
    bool        SetCapOption(bool bHTML, const InsCaptionOpt* pOpt);

    bool        IsGrfToGalleryAsLnk() const     { return aMiscConfig.bGrfToGalleryAsLnk; }
    void        SetGrfToGalleryAsLnk( bool b )  { aMiscConfig.bGrfToGalleryAsLnk = b;
                                                  aMiscConfig.SetModified();}

    sal_Int16   GetMailingFormats() const       { return aMiscConfig.nMailingFormats;}
    void        SetMailingFormats( sal_Int16 nSet ) { aMiscConfig.nMailingFormats = nSet;
                                                  aMiscConfig.SetModified();}

    bool        IsSinglePrintJob() const        { return aMiscConfig.bSinglePrintJob; }
    void        SetSinglePrintJob( bool b )     { aMiscConfig.bSinglePrintJob = b;
                                                  aMiscConfig.SetModified();}

    bool        IsNumAlignSize() const          { return aMiscConfig.bNumAlignSize; }
    void        SetNumAlignSize( bool b )       { aMiscConfig.bNumAlignSize = b;
                                                  aMiscConfig.SetModified();}

    bool        IsNameFromColumn() const        { return aMiscConfig.bIsNameFromColumn; }
    void        SetIsNameFromColumn( bool bSet )
                        {
                            aMiscConfig.SetModified();
                            aMiscConfig.bIsNameFromColumn = bSet;
                        }

    bool        IsAskForMailMerge() const       { return aMiscConfig.bAskForMailMergeInPrint;}
    void        SetAskForMailMerge(bool bSet)
                    {
                        aMiscConfig.SetModified();
                        aMiscConfig.bAskForMailMergeInPrint = bSet;
                    }

    OUString    GetNameFromColumn() const       { return aMiscConfig.sNameFromColumn; }
    void        SetNameFromColumn( const OUString& rSet )       { aMiscConfig.sNameFromColumn = rSet;
                                                                  aMiscConfig.SetModified();}

    OUString    GetMailingPath() const          { return aMiscConfig.sMailingPath; }
    void        SetMailingPath(const OUString& sPath) { aMiscConfig.sMailingPath = sPath;
                                                      aMiscConfig.SetModified();}

    OUString    GetMailName() const             { return aMiscConfig.sMailName; }
    void        SetMailName(const OUString& sName){ aMiscConfig.sMailName = sName;
                                                  aMiscConfig.SetModified();}

    OUString    GetWordDelimiter() const        { return aMiscConfig.sWordDelimiter; }
    void        SetWordDelimiter(const OUString& sDelim)  { aMiscConfig.sWordDelimiter = sDelim;
                                                          aMiscConfig.SetModified();}

    //convert word delimiter from or to user interface
    static OUString ConvertWordDelimiter(const OUString& rDelim, bool bFromUI);

    bool    IsShowIndexPreview() const {return  aMiscConfig.bShowIndexPreview;}
    void        SetShowIndexPreview(bool bSet)
                    {aMiscConfig.bShowIndexPreview = bSet;
                    aMiscConfig.SetModified();}

    bool    IsDefaultFontInCurrDocOnly() const { return aMiscConfig.bDefaultFontsInCurrDocOnly;}
    void        SetDefaultFontInCurrDocOnly(bool bSet)
                    {
                        aMiscConfig.bDefaultFontsInCurrDocOnly = bSet;
                        aMiscConfig.SetModified();
                    }

    bool    IsHideFieldTips() const {return bHideFieldTips;}
    void        SetHideFieldTips(bool bSet) {bHideFieldTips = bSet;}

    SvxCompareMode  GetCompareMode() const { return (SvxCompareMode)aCompareConfig.eCmpMode; }
    void            SetCompareMode( SvxCompareMode eMode ) { aCompareConfig.eCmpMode = eMode;
                                                             aCompareConfig.SetModified(); }

    bool    IsUseRsid() const { return aCompareConfig.bUseRsid; }
    void        SetUseRsid( bool b ) { aCompareConfig.bUseRsid = b;
                                                            aCompareConfig.SetModified(); }

    bool    IsIgnorePieces() const { return aCompareConfig.bIgnorePieces; }
    void        SetIgnorePieces( bool b ) { aCompareConfig.bIgnorePieces = b;
                                                aCompareConfig.SetModified(); }

    sal_uInt16  GetPieceLen() const { return aCompareConfig.nPieceLen; }
    void        SetPieceLen( sal_uInt16 nLen ) { aCompareConfig.nPieceLen = nLen;
                                                 aCompareConfig.SetModified(); }

};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
