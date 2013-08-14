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
#ifndef _MODOPT_HXX
#define _MODOPT_HXX

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
        ~SwRevisionConfig();

    virtual void            Commit();
    virtual void Notify( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames );
    void                    Load();
    void                    SetModified(){ConfigItem::SetModified();}
};

class SwCompareConfig : public utl::ConfigItem
{
    friend class SwModuleOptions;

    sal_uInt16      eCmpMode;		//Compare/CompareDocuments;
    sal_Bool        bUseRsid;		//Compare/Settings/Use RSID
    sal_Bool        bIgnorePieces;	//Compare/Settings/Ignore pieces of length
    sal_uInt16      nPieceLen;		//Compare/Settings/Ignore pieces of length

    const com::sun::star::uno::Sequence<OUString>& GetPropertyNames();
    public:
        SwCompareConfig();
        ~SwCompareConfig();

    virtual void    Commit();
    virtual void    Notify( const ::com::sun::star::uno::Sequence< OUString >& ){ };
    void            Load();
    void            SetModified() {ConfigItem::SetModified(); }
};

class SwInsertConfig : public utl::ConfigItem
{
    friend class SwModuleOptions;

    InsCaptionOptArr*   pCapOptions;
    InsCaptionOpt*      pOLEMiscOpt;

    SvGlobalName        aGlobalNames[5];

    sal_Bool            bInsWithCaption;       //Insert/Caption/Automatic
    sal_Bool            bCaptionOrderNumberingFirst; //#i61007# caption order starting with numbering

    SwInsertTableOptions    aInsTblOpts;
    sal_Bool            bIsWeb;

    const com::sun::star::uno::Sequence<OUString>& GetPropertyNames();
    public:
        SwInsertConfig(sal_Bool bWeb);
        ~SwInsertConfig();

    virtual void            Commit();
    virtual void Notify( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames );
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

    sal_Bool    bInsTblFormatNum;       // Table/Input/NumberRecognition        // Automatic recognition of numbers.
    sal_Bool    bInsTblChangeNumFormat; // Table/Input/NumberFormatRecognition  // Automatic recognition of number formats.
    sal_Bool    bInsTblAlignNum;        // Table/Input/Alignment                // Align numbers.

    const com::sun::star::uno::Sequence<OUString>& GetPropertyNames();
    public:
        SwTableConfig(sal_Bool bWeb);
        ~SwTableConfig();

    virtual void            Commit();
    virtual void Notify( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames );
    void                    Load();
    void                    SetModified(){ConfigItem::SetModified();}
};

class SwMiscConfig : public utl::ConfigItem
{
    friend class SwModuleOptions;

    OUString    sWordDelimiter;             // Statistics/WordNumber/Delimiter
    sal_Bool        bDefaultFontsInCurrDocOnly; // DefaultFont/Document
    sal_Bool        bShowIndexPreview;          // Index/ShowPreview
    sal_Bool        bGrfToGalleryAsLnk;         // Misc/GraphicToGalleryAsLink
    sal_Bool        bNumAlignSize;              // Numbering/Graphic/KeepRatio
    sal_Bool        bSinglePrintJob;            // FormLetter/PrintOutput/SinglePrintJobs
    sal_Bool        bIsNameFromColumn;          // FormLetter/FileOutput/FileName/Generation
    sal_Bool        bAskForMailMergeInPrint;    // Ask if documents containing fields should be 'mailmerged'
    sal_Int16   nMailingFormats;            // FormLetter/MailingOutput/Formats
    OUString    sNameFromColumn;            // FormLetter/FileOutput/FileName/FromDatabaseField (string!)
    OUString    sMailingPath;               // FormLetter/FileOutput/Path
    OUString    sMailName;                  // FormLetter/FileOutput/FileName/FromManualSetting (string!)

    const com::sun::star::uno::Sequence<OUString>& GetPropertyNames();
    public:
        SwMiscConfig();
        ~SwMiscConfig();

    virtual void            Commit();
    virtual void Notify( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames );
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
    sal_Bool        bHideFieldTips : 1;

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

    sal_Bool        IsInsWithCaption(sal_Bool bHTML) const
                        { return bHTML ? sal_False : aInsertConfig.bInsWithCaption; }
    void        SetInsWithCaption( sal_Bool bHTML, sal_Bool b )
                    {   if(!bHTML)
                            aInsertConfig.bInsWithCaption = b;
                        aInsertConfig.SetModified();}

    sal_Bool    IsCaptionOrderNumberingFirst() const { return aInsertConfig.bCaptionOrderNumberingFirst; }
    void        SetCaptionOrderNumberingFirst( sal_Bool bSet )
                {
                    if(aInsertConfig.bCaptionOrderNumberingFirst != bSet)
                    {
                        aInsertConfig.bCaptionOrderNumberingFirst = bSet;
                        aInsertConfig.SetModified();
                    }
                }

    sal_Bool        IsInsTblFormatNum(sal_Bool bHTML) const
                    { return bHTML ? aWebTableConfig.bInsTblFormatNum : aTableConfig.bInsTblFormatNum; }
    void        SetInsTblFormatNum( sal_Bool bHTML, sal_Bool b )
                    { bHTML ? (aWebTableConfig.bInsTblFormatNum = b) : (aTableConfig.bInsTblFormatNum = b);
                      bHTML ? aWebTableConfig.SetModified() : aTableConfig.SetModified();}

    sal_Bool        IsInsTblChangeNumFormat(sal_Bool bHTML) const
                    { return bHTML ? aWebTableConfig.bInsTblChangeNumFormat : aTableConfig.bInsTblChangeNumFormat; }
    void        SetInsTblChangeNumFormat( sal_Bool bHTML, sal_Bool b )
                    { bHTML ? (aWebTableConfig.bInsTblChangeNumFormat = b) : (aTableConfig.bInsTblChangeNumFormat = b);
                      bHTML ? aWebTableConfig.SetModified() : aTableConfig.SetModified();}


    sal_Bool        IsInsTblAlignNum(sal_Bool bHTML) const
                    { return bHTML ? aWebTableConfig.bInsTblAlignNum : aTableConfig.bInsTblAlignNum; }
    void        SetInsTblAlignNum( sal_Bool bHTML, sal_Bool b )
                    { bHTML ? (aWebTableConfig.bInsTblAlignNum = b) : (aTableConfig.bInsTblAlignNum = b);
                        bHTML ? aWebTableConfig.SetModified() : aTableConfig.SetModified();}

    SwInsertTableOptions GetInsTblFlags(sal_Bool bHTML) const
                    { return bHTML ? aWebInsertConfig.aInsTblOpts : aInsertConfig.aInsTblOpts;}
    void        SetInsTblFlags( sal_Bool bHTML, const SwInsertTableOptions& rOpts ) {
                    bHTML ? (aWebInsertConfig.aInsTblOpts = rOpts) : (aInsertConfig.aInsTblOpts = rOpts);
                    bHTML ? aWebInsertConfig.SetModified() : aInsertConfig.SetModified();}

    const InsCaptionOpt* GetCapOption(sal_Bool bHTML, const SwCapObjType eType, const SvGlobalName *pOleId);
    sal_Bool        SetCapOption(sal_Bool bHTML, const InsCaptionOpt* pOpt);


    sal_Bool        IsGrfToGalleryAsLnk() const     { return aMiscConfig.bGrfToGalleryAsLnk; }
    void        SetGrfToGalleryAsLnk( sal_Bool b )  { aMiscConfig.bGrfToGalleryAsLnk = b;
                                                  aMiscConfig.SetModified();}

    sal_Int16   GetMailingFormats() const       { return aMiscConfig.nMailingFormats;}
    void        SetMailingFormats( sal_Int16 nSet ) { aMiscConfig.nMailingFormats = nSet;
                                                  aMiscConfig.SetModified();}

    sal_Bool        IsSinglePrintJob() const        { return aMiscConfig.bSinglePrintJob; }
    void        SetSinglePrintJob( sal_Bool b )     { aMiscConfig.bSinglePrintJob = b;
                                                  aMiscConfig.SetModified();}

    sal_Bool        IsNumAlignSize() const          { return aMiscConfig.bNumAlignSize; }
    void        SetNumAlignSize( sal_Bool b )       { aMiscConfig.bNumAlignSize = b;
                                                  aMiscConfig.SetModified();}

    sal_Bool        IsNameFromColumn() const        { return aMiscConfig.bIsNameFromColumn; }
    void        SetIsNameFromColumn( sal_Bool bSet )
                        {
                            aMiscConfig.SetModified();
                            aMiscConfig.bIsNameFromColumn = bSet;
                        }

    sal_Bool        IsAskForMailMerge() const       { return aMiscConfig.bAskForMailMergeInPrint;}
    void        SetAskForMailMerge(sal_Bool bSet)
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
    static OUString ConvertWordDelimiter(const OUString& rDelim, sal_Bool bFromUI);

    sal_Bool    IsShowIndexPreview() const {return  aMiscConfig.bShowIndexPreview;}
    void        SetShowIndexPreview(sal_Bool bSet)
                    {aMiscConfig.bShowIndexPreview = bSet;
                    aMiscConfig.SetModified();}

    sal_Bool    IsDefaultFontInCurrDocOnly() const { return aMiscConfig.bDefaultFontsInCurrDocOnly;}
    void        SetDefaultFontInCurrDocOnly(sal_Bool bSet)
                    {
                        aMiscConfig.bDefaultFontsInCurrDocOnly = bSet;
                        aMiscConfig.SetModified();
                    }

    sal_Bool    IsHideFieldTips() const {return bHideFieldTips;}
    void        SetHideFieldTips(sal_Bool bSet) {bHideFieldTips = bSet;}

    SvxCompareMode  GetCompareMode() const { return (SvxCompareMode)aCompareConfig.eCmpMode; }
    void            SetCompareMode( SvxCompareMode eMode ) { aCompareConfig.eCmpMode = eMode;
                                                             aCompareConfig.SetModified(); }

    sal_Bool    IsUseRsid() const { return aCompareConfig.bUseRsid; }
    void        SetUseRsid( sal_Bool b ) { aCompareConfig.bUseRsid = b;
                                                            aCompareConfig.SetModified(); }

    sal_Bool    IsIgnorePieces() const { return aCompareConfig.bIgnorePieces; }
    void        SetIgnorePieces( sal_Bool b ) { aCompareConfig.bIgnorePieces = b;
                                                aCompareConfig.SetModified(); }

    sal_uInt16  GetPieceLen() const { return aCompareConfig.nPieceLen; }
    void        SetPieceLen( sal_uInt16 nLen ) { aCompareConfig.nPieceLen = nLen;
                                                 aCompareConfig.SetModified(); }

};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
