/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _MODOPT_HXX
#define _MODOPT_HXX


#include <tools/string.hxx>
#include <vcl/wintypes.hxx>
#include <vcl/field.hxx>
#include <bf_svtools/svarray.hxx>

#include "authratr.hxx"
#include "caption.hxx"
#include "tblenum.hxx"

#include <unotools/configitem.hxx>
namespace binfilter {

//-----------------------------------------------------------------------------

typedef InsCaptionOpt* InsCaptionOptPtr;
SV_DECL_PTRARR_SORT_DEL(InsCapOptArr, InsCaptionOptPtr, 0, 5)

class InsCaptionOptArr : public InsCapOptArr
{
    friend class SwModuleOptions;
    friend class SwInsertConfig;
protected:
    InsCaptionOpt* Find(const SwCapObjType eType, const SvGlobalName *pOleId = 0) const;
};

/* -----------------------------10.10.00 16:14--------------------------------

 ---------------------------------------------------------------------------*/
class SwModuleOptions;
class SwRevisionConfig : public ::utl::ConfigItem
{
    friend class SwModuleOptions;

    AuthorCharAttr	aInsertAttr;	//Revision/TextDisplay/Insert/Attribute  // Redlining: Author-Zeichenattribute
                                    //Revision/TextDisplay/Insert/Color
    AuthorCharAttr	aDeletedAttr;	//Revision/TextDisplay/Delete/Attribute
                                    //Revision/TextDisplay/Delete/Color
    AuthorCharAttr	aFormatAttr;    //Revision/TextDisplay/ChangeAttribute/Attribute
                                    //Revision/TextDisplay/ChangeAttribute/Color
    USHORT			nMarkAlign;		//Revision/LinesChanged/Mark
    Color			aMarkColor;     //Revision/LinesChanged/Color

    const ::com::sun::star::uno::Sequence<rtl::OUString>& GetPropertyNames();
    public:
        SwRevisionConfig();
        ~SwRevisionConfig();

    void					Load();
    void 					SetModified(){ConfigItem::SetModified();}
    virtual void Commit();
    virtual void Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
};
/* -----------------------------11.10.00 09:00--------------------------------

 ---------------------------------------------------------------------------*/
class SwInsertConfig : public ::utl::ConfigItem
{
    friend class SwModuleOptions;

    InsCaptionOptArr* 	pCapOptions;
    InsCaptionOpt* 		pOLEMiscOpt;

    SvGlobalName		aGlobalNames[5];

    sal_Bool 			bInsWithCaption;	   //Insert/Caption/Automatic	// Objekte beschriftet einfuegen
    sal_uInt16			nInsTblFlags;			//Insert/Table/Header	 	// Flags fuer Tabellen einfuegen
                                                //Insert/Table/RepeatHeader
                                                //Insert/Table/Split
                                                //Insert/Table/Border
    sal_Bool 			bIsWeb;

    const ::com::sun::star::uno::Sequence<rtl::OUString>& GetPropertyNames();
    public:
        SwInsertConfig(sal_Bool bWeb);
        ~SwInsertConfig();

    void					Load();
    void 					SetModified(){ConfigItem::SetModified();}
    virtual void Commit();
    virtual void Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
};
/* -----------------------------11.10.00 09:00--------------------------------

 ---------------------------------------------------------------------------*/
class SwTableConfig : public ::utl::ConfigItem
{
    friend class SwModuleOptions;

    USHORT		nTblHMove;  		//int Table/Shift/Row
    USHORT 		nTblVMove;          //int Table/Shift/Column
    USHORT 		nTblHInsert;        //int Table/Insert/Row
    USHORT		nTblVInsert;        //int Table/Insert/Column
    TblChgMode	eTblChgMode;    	//int Table/Change/Effect

    sal_Bool	bInsTblFormatNum;		// Table/Input/NumberRecognition 		// Automatische Zahlenerkennung
    sal_Bool	bInsTblChangeNumFormat; // Table/Input/NumberFormatRecognition 	// Automatische Zahlenformaterkennung
    sal_Bool	bInsTblAlignNum;		// Table/Input/Alignment				// Zahlen ausrichten

    const ::com::sun::star::uno::Sequence<rtl::OUString>& GetPropertyNames();
    public:
        SwTableConfig(sal_Bool bWeb);
        ~SwTableConfig();

    void					Load();
    void 					SetModified(){ConfigItem::SetModified();}
    virtual void Commit();
    virtual void Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
};
/* -----------------------------18.01.01 16:57--------------------------------

 ---------------------------------------------------------------------------*/
class SwMiscConfig : public ::utl::ConfigItem
{
    friend class SwModuleOptions;

    String		sWordDelimiter;				// Statistics/WordNumber/Delimiter
    BOOL		bDefaultFontsInCurrDocOnly; // DefaultFont/Document
    BOOL 		bShowIndexPreview ;  		// Index/ShowPreview
    BOOL		bGrfToGalleryAsLnk; 		// Misc/GraphicToGalleryAsLink
    BOOL		bNumAlignSize;				// Numbering/Graphic/KeepRatio
    BOOL		bSinglePrintJob;			// FormLetter/PrintOutput/SinglePrintJobs
    BOOL		bIsNameFromColumn;          // FormLetter/FileOutput/FileName/Generation
    sal_Int16	nMailingFormats;			// FormLetter/MailingOutput/Formats
    String		sNameFromColumn;			// FormLetter/FileOutput/FileName/FromDatabaseField (string!)
    String		sMailingPath;				// FormLetter/FileOutput/Path
    String		sMailName;					// FormLetter/FileOutput/FileName/FromManualSetting (string!)

    const ::com::sun::star::uno::Sequence<rtl::OUString>& GetPropertyNames();
    public:
        SwMiscConfig();
        ~SwMiscConfig();

    void					Load();
    void 					SetModified(){ConfigItem::SetModified();}
    virtual void Commit();
    virtual void Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
};

/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
class SwModuleOptions
{
    SwRevisionConfig 				aRevisionConfig;
    SwInsertConfig					aInsertConfig;
    SwInsertConfig					aWebInsertConfig;

    SwTableConfig					aTableConfig;
    SwTableConfig					aWebTableConfig;

    SwMiscConfig					aMiscConfig;

    //fiscus: don't show tips of text fields - it's not part of the configuration!
    BOOL 		bHideFieldTips : 1;

public:
    SwModuleOptions();

    TblChgMode	GetTblMode() const { return aTableConfig.eTblChgMode;}
    void		SetTblMode( TblChgMode  eSet ) { aTableConfig.eTblChgMode = eSet;
                                                aTableConfig.SetModified();}

    USHORT		GetTblHMove() const { return aTableConfig.nTblHMove;}
    void		SetTblHMove( USHORT nSet ) { 	aTableConfig.nTblHMove = nSet;
                                                aTableConfig.SetModified();}

    USHORT 		GetTblVMove() const { return aTableConfig.nTblVMove;}
    void 		SetTblVMove( USHORT nSet ) { 	aTableConfig.nTblVMove = nSet;
                                                aTableConfig.SetModified();}

    USHORT 		GetTblHInsert() const {return aTableConfig.nTblHInsert;}
    void 		SetTblHInsert( USHORT nSet ) {	aTableConfig.nTblHInsert = nSet;
                                                aTableConfig.SetModified();}

    USHORT		GetTblVInsert() const {return aTableConfig.nTblVInsert;}
    void 		SetTblVInsert( USHORT nSet ) {	aTableConfig.nTblVInsert = nSet;
                                                aTableConfig.SetModified();}

    const AuthorCharAttr	&GetInsertAuthorAttr() const { return aRevisionConfig.aInsertAttr; }
    void 		SetInsertAuthorAttr( AuthorCharAttr &rAttr ) {	aRevisionConfig.aInsertAttr = rAttr;
                                                                aRevisionConfig.SetModified();}

    const AuthorCharAttr	&GetDeletedAuthorAttr() const { return aRevisionConfig.aDeletedAttr; }
    void 		SetDeletedAuthorAttr( AuthorCharAttr &rAttr ) {	aRevisionConfig.aDeletedAttr = rAttr;
                                                                aRevisionConfig.SetModified();}

    const AuthorCharAttr	&GetFormatAuthorAttr() const { return aRevisionConfig.aFormatAttr; }
    void 		SetFormatAuthorAttr( AuthorCharAttr &rAttr ) {	aRevisionConfig.aFormatAttr = rAttr;
                                                                aRevisionConfig.SetModified();}

    USHORT			GetMarkAlignMode()	const				{ return aRevisionConfig.nMarkAlign; }
    void			SetMarkAlignMode(USHORT nMode)			{ aRevisionConfig.nMarkAlign = nMode;
                                                              aRevisionConfig.SetModified();}

    const Color&	GetMarkAlignColor()	const				{ return aRevisionConfig.aMarkColor; }
    void 			SetMarkAlignColor(const Color &rColor)	{ aRevisionConfig.aMarkColor = rColor;
                                                              aRevisionConfig.SetModified();}

    BOOL		IsInsWithCaption(BOOL bHTML) const
                        { return bHTML ? FALSE : aInsertConfig.bInsWithCaption; }
    void		SetInsWithCaption( BOOL bHTML, BOOL b )
                    { 	if(!bHTML)
                            aInsertConfig.bInsWithCaption = b;
                        aInsertConfig.SetModified();}

    BOOL		IsInsTblFormatNum(BOOL bHTML) const
                    { return bHTML ? aWebTableConfig.bInsTblFormatNum : aTableConfig.bInsTblFormatNum; }
    void		SetInsTblFormatNum( BOOL bHTML, BOOL b )
                    { bHTML ? (aWebTableConfig.bInsTblFormatNum = b) : (aTableConfig.bInsTblFormatNum = b);
                      bHTML ? aWebTableConfig.SetModified() : aTableConfig.SetModified();}

    BOOL		IsInsTblChangeNumFormat(BOOL bHTML) const
                    { return bHTML ? aWebTableConfig.bInsTblChangeNumFormat : aTableConfig.bInsTblChangeNumFormat; }
    void		SetInsTblChangeNumFormat( BOOL bHTML, BOOL b )
                    { bHTML ? (aWebTableConfig.bInsTblChangeNumFormat = b) : (aTableConfig.bInsTblChangeNumFormat = b);
                      bHTML ? aWebTableConfig.SetModified() : aTableConfig.SetModified();}


    BOOL		IsInsTblAlignNum(BOOL bHTML) const
                    { return bHTML ? aWebTableConfig.bInsTblAlignNum : aTableConfig.bInsTblAlignNum; }
    void		SetInsTblAlignNum( BOOL bHTML, BOOL b )
                    { bHTML ? (aWebTableConfig.bInsTblAlignNum = b) : (aTableConfig.bInsTblAlignNum = b);
                        bHTML ? aWebTableConfig.SetModified() : aTableConfig.SetModified();;}

    USHORT		GetInsTblFlags(BOOL bHTML) const
                    { return bHTML ? aWebInsertConfig.nInsTblFlags : aInsertConfig.nInsTblFlags;}
    void		SetInsTblFlags( BOOL bHTML, USHORT nSet )	{
                    bHTML ? (aWebInsertConfig.nInsTblFlags = nSet) : (aInsertConfig.nInsTblFlags = nSet);
                    bHTML ? aWebInsertConfig.SetModified() : aInsertConfig.SetModified();}



    BOOL		IsGrfToGalleryAsLnk() const 	{ return aMiscConfig.bGrfToGalleryAsLnk; }
    void		SetGrfToGalleryAsLnk( BOOL b )  { aMiscConfig.bGrfToGalleryAsLnk = b;
                                                  aMiscConfig.SetModified();}

    sal_Int16	GetMailingFormats() const 		{ return aMiscConfig.nMailingFormats;}
    void 		SetMailingFormats( sal_Int16 nSet )	{ aMiscConfig.nMailingFormats = nSet;
                                                  aMiscConfig.SetModified();}

    BOOL		IsSinglePrintJob() const 		{ return aMiscConfig.bSinglePrintJob; }
    void		SetSinglePrintJob( BOOL b )  	{ aMiscConfig.bSinglePrintJob = b;
                                                  aMiscConfig.SetModified();}

    BOOL		IsNumAlignSize() const 			{ return aMiscConfig.bNumAlignSize; }
    void		SetNumAlignSize( BOOL b )  		{ aMiscConfig.bNumAlignSize = b;
                                                  aMiscConfig.SetModified();}

    BOOL		IsNameFromColumn() const		{ return aMiscConfig.bIsNameFromColumn; }
    void		SetIsNameFromColumn( BOOL bSet )  	{ aMiscConfig.bIsNameFromColumn = bSet;}

    const String&	GetNameFromColumn() const		{ return aMiscConfig.sNameFromColumn; }
    void			SetNameFromColumn( const String& rSet )  	{ aMiscConfig.sNameFromColumn = rSet;
                                                                  aMiscConfig.SetModified();}

    String		GetMailingPath() const			{ return aMiscConfig.sMailingPath; }
    void		SetMailingPath(const String& sPath)	{ aMiscConfig.sMailingPath = sPath;
                                                      aMiscConfig.SetModified();}

    String		GetMailName() const				{ return aMiscConfig.sMailName; }
    void		SetMailName(const String& sName){ aMiscConfig.sMailName = sName;
                                                  aMiscConfig.SetModified();}

    const String	&GetWordDelimiter() const			{ return aMiscConfig.sWordDelimiter; }
    void		SetWordDelimiter(const String& sDelim)	{ aMiscConfig.sWordDelimiter = sDelim;
                                                          aMiscConfig.SetModified();}

    //convert word delimiter from or to user interface
    static String ConvertWordDelimiter(const String& rDelim, BOOL bFromUI);

    BOOL 		IsShowIndexPreview() const {return  aMiscConfig.bShowIndexPreview;}
    void		SetShowIndexPreview(BOOL bSet)
                    {aMiscConfig.bShowIndexPreview = bSet;
                    aMiscConfig.SetModified();}

    BOOL		IsDefaultFontInCurrDocOnly() const { return aMiscConfig.bDefaultFontsInCurrDocOnly;}
    void 		SetDefaultFontInCurrDocOnly(BOOL bSet)
                    {
                        aMiscConfig.bDefaultFontsInCurrDocOnly = bSet;
                        aMiscConfig.SetModified();
                    }

    BOOL 		IsHideFieldTips() const {return bHideFieldTips;}
    void		SetHideFieldTips(BOOL bSet) {bHideFieldTips = bSet;}
};
} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
