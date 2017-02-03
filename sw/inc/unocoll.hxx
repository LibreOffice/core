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
#ifndef INCLUDED_SW_INC_UNOCOLL_HXX
#define INCLUDED_SW_INC_UNOCOLL_HXX
#include <tools/solar.h>
#include <flyenum.hxx>
#include <vcl/timer.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <cppuhelper/implbase.hxx>
#include <IMark.hxx>
#include <unobaseclass.hxx>
#include "swdllapi.h"

class SwDoc;
class SwFrameFormat;
namespace com{namespace sun {namespace star {namespace text
{
    class XTextTable;
    class XTextSection;
    class XFootnote;
}}}}
class SwXFrame;
class SwFrameFormat;
class SwFormatFootnote;
class XBookmark;
class SwXReferenceMark;
class SwSectionFormat;
class SwFormatRefMark;
class SwXReferenceMark;
class SwXBookmark;
struct SvEventDescription;

const SvEventDescription* sw_GetSupportedMacroItems();

class SwUnoCollection
{
    SwDoc*      m_pDoc;
    bool    m_bObjectValid;

    public:
        SwUnoCollection(SwDoc* p) :
            m_pDoc(p),
            m_bObjectValid(true){}

    virtual ~SwUnoCollection() {}

    virtual void                Invalidate();
    bool                        IsValid() const {return m_bObjectValid;}

    SwDoc*          GetDoc() const {return m_pDoc;}
};

/** entries in this enum are mapped in an array in unocoll.cxx */
enum class SwServiceType {
    TypeTextTable                   =   0,
    TypeTextFrame                   =   1,
    TypeGraphic                     =   2,
    TypeOLE                         =   3,
    TypeBookmark                    =   4,
    TypeFootnote                    =   5,
    TypeEndnote                     =   6,
    TypeIndexMark                   =   7,
    TypeIndex                       =   8,
    ReferenceMark                   =   9,
    StyleCharacter                  =  10,
    StyleParagraph                  =  11,
    StyleFrame                      =  12,
    StylePage                       =  13,
    StyleNumbering                  =  14,
    ContentIndexMark                =  15,
    ContentIndex                    =  16,
    UserIndexMark                   =  17,
    UserIndex                       =  18,
    TextSection                     =  19,
    FieldTypeDateTime               =  20,
    FieldTypeUser                   =  21,
    FieldTypeSetExp                 =  22,
    FieldTypeGetExp                 =  23,
    FieldTypeFileName               =  24,
    FieldTypePageNum                =  25,
    FieldTypeAuthor                 =  26,
    FieldTypeChapter                =  27,
    FieldTypeDummy0                 =  28,
    FieldTypeGetReference           =  29,
    FieldTypeConditionedText        =  30,
    FieldTypeAnnotation             =  31,
    FieldTypeInput                  =  32,
    FieldTypeMacro                  =  33,
    FieldTypeDDE                    =  34,
    FieldTypeHiddenPara             =  35,
    FieldTypeDocInfo                =  36,
    FieldTypeTemplateName           =  37,
    FieldTypeUserExt                =  38,
    FieldTypeRefPageSet             =  39,
    FieldTypeRefPageGet             =  40,
    FieldTypeJumpEdit               =  41,
    FieldTypeScript                 =  42,
    FieldTypeDatabaseNextSet        =  43,
    FieldTypeDatabaseNumSet         =  44,
    FieldTypeDatabaseSetNum         =  45,
    FieldTypeDatabase               =  46,
    FieldTypeDatabaseName           =  47,
    FieldTypeTableFormula           =  48,
    FieldTypePageCount              =  49,
    FieldTypeParagraphCount         =  50,
    FieldTypeWordCount              =  51,
    FieldTypeCharacterCount         =  52,
    FieldTypeTableCount             =  53,
    FieldTypeGraphicObjectCount     =  54,
    FieldTypeEmbeddedObjectCount    =  55,
    FieldTypeDocInfoChangeAuthor    =  56,
    FieldTypeDocInfoChangeDateTime  =  57,
    FieldTypeDocInfoEditTime        =  58,
    FieldTypeDocInfoDescription     =  59,
    FieldTypeDocInfoCreateAuthor    =  60,
    FieldTypeDocInfoCreateDateTime  =  61,
    FieldTypeDummy1                 =  63,
    FieldTypeDummy2                 =  64,
    FieldTypeDummy3                 =  65,
    FieldTypeDocInfoCustom          =  66,
    FieldTypeDocInfoPrintAuthor     =  67,
    FieldTypeDocInfoPrintDateTime   =  68,
    FieldTypeDocInfoKeywords        =  69,
    FieldTypeDocInfoSubject         =  70,
    FieldTypeDocInfoTitle           =  71,
    FieldTypeDocInfoRevision        =  72,
    FieldTypeBibliography           =  73,
    FieldTypeCombinedCharacters     =  74,
    FieldTypeDropdown               =  75,
    FieldTypeMetafield              =  76,
    FieldTypeDummy4                 =  77,
    FieldTypeDummy5                 =  78,
    FieldTypeDummy6                 =  79,
    FieldTypeDummy7                 =  80,
    FieldMasterUser                 =  81,
    FieldMasterDDE                  =  82,
    FieldMasterSetExp               =  83,
    FieldMasterDatabase             =  84,
    FieldMasterBibliography         =  85,
    FieldMasterDummy2               =  86,
    FieldMasterDummy3               =  87,
    FieldMasterDummy4               =  88,
    FieldMasterDummy5               =  89,
    IndexIllustrations              =  90,
    IndexObjects                    =  91,
    IndexTables                     =  92,
    IndexBibliography               =  93,
    Paragraph                       =  94,
    FieldTypeInputUser              =  95,
    FieldTypeHiddenText             =  96,
    StyleConditionalParagraph       =  97,
    NumberingRules                  =  98,
    TextColumns                     =  99,
    IndexHeaderSection              = 100,
    Defaults                        = 101,
    IMapRectangle                   = 102,
    IMapCircle                      = 103,
    IMapPolygon                     = 104,
    TypeTextGraphic                 = 105,
    Chart2DataProvider              = 106,
    TypeFieldMark                   = 107,
    TypeFormFieldMark               = 108,
    TypeMeta                        = 109,
    VbaObjectProvider               = 110,
    VbaCodeNameProvider             = 111,
    VbaProjectNameProvider          = 112,
    VbaGlobals                      = 113,
    StyleTable                      = 114,
    StyleCell                       = 115,

    Invalid                         = USHRT_MAX
};

// case-corrected versions of service-names (see #i67811)

#define CSS_TEXT_TEXTFIELD_DATE_TIME                    "com.sun.star.text.textfield.DateTime"
#define CSS_TEXT_TEXTFIELD_USER                         "com.sun.star.text.textfield.User"
#define CSS_TEXT_TEXTFIELD_SET_EXPRESSION               "com.sun.star.text.textfield.SetExpression"
#define CSS_TEXT_TEXTFIELD_GET_EXPRESSION               "com.sun.star.text.textfield.GetExpression"
#define CSS_TEXT_TEXTFIELD_FILE_NAME                    "com.sun.star.text.textfield.FileName"
#define CSS_TEXT_TEXTFIELD_PAGE_NUMBER                  "com.sun.star.text.textfield.PageNumber"
#define CSS_TEXT_TEXTFIELD_AUTHOR                       "com.sun.star.text.textfield.Author"
#define CSS_TEXT_TEXTFIELD_CHAPTER                      "com.sun.star.text.textfield.Chapter"
#define CSS_TEXT_TEXTFIELD_GET_REFERENCE                "com.sun.star.text.textfield.GetReference"
#define CSS_TEXT_TEXTFIELD_CONDITIONAL_TEXT             "com.sun.star.text.textfield.ConditionalText"
#define CSS_TEXT_TEXTFIELD_ANNOTATION                   "com.sun.star.text.textfield.Annotation"
#define CSS_TEXT_TEXTFIELD_INPUT                        "com.sun.star.text.textfield.Input"
#define CSS_TEXT_TEXTFIELD_MACRO                        "com.sun.star.text.textfield.Macro"
#define CSS_TEXT_TEXTFIELD_DDE                          "com.sun.star.text.textfield.DDE"
#define CSS_TEXT_TEXTFIELD_HIDDEN_PARAGRAPH             "com.sun.star.text.textfield.HiddenParagraph"
#define CSS_TEXT_TEXTFIELD_TEMPLATE_NAME                "com.sun.star.text.textfield.TemplateName"
#define CSS_TEXT_TEXTFIELD_EXTENDED_USER                "com.sun.star.text.textfield.ExtendedUser"
#define CSS_TEXT_TEXTFIELD_REFERENCE_PAGE_SET           "com.sun.star.text.textfield.ReferencePageSet"
#define CSS_TEXT_TEXTFIELD_REFERENCE_PAGE_GET           "com.sun.star.text.textfield.ReferencePageGet"
#define CSS_TEXT_TEXTFIELD_JUMP_EDIT                    "com.sun.star.text.textfield.JumpEdit"
#define CSS_TEXT_TEXTFIELD_SCRIPT                       "com.sun.star.text.textfield.Script"
#define CSS_TEXT_TEXTFIELD_DATABASE_NEXT_SET            "com.sun.star.text.textfield.DatabaseNextSet"
#define CSS_TEXT_TEXTFIELD_DATABASE_NUMBER_OF_SET       "com.sun.star.text.textfield.DatabaseNumberOfSet"
#define CSS_TEXT_TEXTFIELD_DATABASE_SET_NUMBER          "com.sun.star.text.textfield.DatabaseSetNumber"
#define CSS_TEXT_TEXTFIELD_DATABASE                     "com.sun.star.text.textfield.Database"
#define CSS_TEXT_TEXTFIELD_DATABASE_NAME                "com.sun.star.text.textfield.DatabaseName"
#define CSS_TEXT_TEXTFIELD_TABLE_FORMULA                "com.sun.star.text.textfield.TableFormula"
#define CSS_TEXT_TEXTFIELD_PAGE_COUNT                   "com.sun.star.text.textfield.PageCount"
#define CSS_TEXT_TEXTFIELD_PARAGRAPH_COUNT              "com.sun.star.text.textfield.ParagraphCount"
#define CSS_TEXT_TEXTFIELD_WORD_COUNT                   "com.sun.star.text.textfield.WordCount"
#define CSS_TEXT_TEXTFIELD_CHARACTER_COUNT              "com.sun.star.text.textfield.CharacterCount"
#define CSS_TEXT_TEXTFIELD_TABLE_COUNT                  "com.sun.star.text.textfield.TableCount"
#define CSS_TEXT_TEXTFIELD_GRAPHIC_OBJECT_COUNT         "com.sun.star.text.textfield.GraphicObjectCount"
#define CSS_TEXT_TEXTFIELD_EMBEDDED_OBJECT_COUNT        "com.sun.star.text.textfield.EmbeddedObjectCount"
#define CSS_TEXT_TEXTFIELD_DOCINFO_CHANGE_AUTHOR        "com.sun.star.text.textfield.docinfo.ChangeAuthor"
#define CSS_TEXT_TEXTFIELD_DOCINFO_CHANGE_DATE_TIME     "com.sun.star.text.textfield.docinfo.ChangeDateTime"
#define CSS_TEXT_TEXTFIELD_DOCINFO_EDIT_TIME            "com.sun.star.text.textfield.docinfo.EditTime"
#define CSS_TEXT_TEXTFIELD_DOCINFO_DESCRIPTION          "com.sun.star.text.textfield.docinfo.Description"
#define CSS_TEXT_TEXTFIELD_DOCINFO_CREATE_AUTHOR        "com.sun.star.text.textfield.docinfo.CreateAuthor"
#define CSS_TEXT_TEXTFIELD_DOCINFO_CREATE_DATE_TIME     "com.sun.star.text.textfield.docinfo.CreateDateTime"
#define CSS_TEXT_TEXTFIELD_DOCINFO_PRINT_AUTHOR         "com.sun.star.text.textfield.docinfo.PrintAuthor"
#define CSS_TEXT_TEXTFIELD_DOCINFO_PRINT_DATE_TIME      "com.sun.star.text.textfield.docinfo.PrintDateTime"
#define CSS_TEXT_TEXTFIELD_DOCINFO_KEY_WORDS            "com.sun.star.text.textfield.docinfo.KeyWords"
#define CSS_TEXT_TEXTFIELD_DOCINFO_SUBJECT              "com.sun.star.text.textfield.docinfo.Subject"
#define CSS_TEXT_TEXTFIELD_DOCINFO_TITLE                "com.sun.star.text.textfield.docinfo.Title"
#define CSS_TEXT_TEXTFIELD_DOCINFO_REVISION             "com.sun.star.text.textfield.docinfo.Revision"
#define CSS_TEXT_TEXTFIELD_DOCINFO_CUSTOM               "com.sun.star.text.textfield.docinfo.Custom"
#define CSS_TEXT_TEXTFIELD_BIBLIOGRAPHY                 "com.sun.star.text.textfield.Bibliography"
#define CSS_TEXT_TEXTFIELD_COMBINED_CHARACTERS          "com.sun.star.text.textfield.CombinedCharacters"
#define CSS_TEXT_TEXTFIELD_DROP_DOWN                    "com.sun.star.text.textfield.DropDown"
#define CSS_TEXT_TEXTFIELD_INPUT_USER                   "com.sun.star.text.textfield.InputUser"
#define CSS_TEXT_TEXTFIELD_HIDDEN_TEXT                  "com.sun.star.text.textfield.HiddenText"
#define CSS_TEXT_FIELDMASTER_USER                       "com.sun.star.text.fieldmaster.User"
#define CSS_TEXT_FIELDMASTER_DDE                        "com.sun.star.text.fieldmaster.DDE"
#define CSS_TEXT_FIELDMASTER_SET_EXPRESSION             "com.sun.star.text.fieldmaster.SetExpression"
#define CSS_TEXT_FIELDMASTER_DATABASE                   "com.sun.star.text.fieldmaster.Database"
#define CSS_TEXT_FIELDMASTER_BIBLIOGRAPHY               "com.sun.star.text.fieldmaster.Bibliography"

class SwXServiceProvider
{
public:
    static OUString             GetProviderName(SwServiceType nObjectType);
    static SwServiceType        GetProviderType(const OUString& rServiceName);
    static css::uno::Reference< css::uno::XInterface >  MakeInstance(SwServiceType nObjectType, SwDoc & rDoc);
    static css::uno::Sequence<OUString>   GetAllServiceNames();

};

typedef
cppu::WeakImplHelper
<
    css::container::XNameAccess,
    css::container::XIndexAccess,
    css::lang::XServiceInfo
>
SwCollectionBaseClass;
class SW_DLLPUBLIC SwXTextTables : public SwCollectionBaseClass,
    public SwUnoCollection
{
protected:
    virtual ~SwXTextTables() override;
public:
    SwXTextTables(SwDoc* pDoc);

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) override;
    virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) override;

    //XNameAccess
    virtual css::uno::Any SAL_CALL getByName(const OUString& Name) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName(const OUString& Name) override;

    //XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    static css::uno::Reference<css::text::XTextTable> GetObject(SwFrameFormat& rFormat);
};

typedef
cppu::WeakImplHelper
<
    css::container::XEnumerationAccess,
    css::container::XNameAccess,
    css::container::XIndexAccess,
    css::lang::XServiceInfo
> SwXFramesBaseClass;

class SW_DLLPUBLIC SwXFrames : public SwXFramesBaseClass,
    public SwUnoCollection
{
    const FlyCntType    m_eType;
protected:
    virtual ~SwXFrames() override;
public:
    SwXFrames(SwDoc* pDoc, FlyCntType eSet);

    //XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() override;

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) override;

    //XNameAccess
    virtual css::uno::Any SAL_CALL getByName(const OUString& Name) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName(const OUString& Name) override;

    //XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

class SwXTextFrames : public SwXFrames
{
protected:
    virtual ~SwXTextFrames() override;
public:
    SwXTextFrames(SwDoc* pDoc);

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

};

class SwXTextGraphicObjects : public SwXFrames
{
protected:
    virtual ~SwXTextGraphicObjects() override;
public:
        SwXTextGraphicObjects(SwDoc* pDoc);

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

};

class SwXTextEmbeddedObjects :  public SwXFrames
{
protected:
    virtual ~SwXTextEmbeddedObjects() override;
public:
    SwXTextEmbeddedObjects(SwDoc* pDoc);

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

};

class SwXTextSections : public SwCollectionBaseClass,
                        public SwUnoCollection
{
protected:
    virtual ~SwXTextSections() override;
public:
    SwXTextSections(SwDoc* pDoc);

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) override;

    //XNameAccess
    virtual css::uno::Any SAL_CALL getByName(const OUString& Name) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName(const OUString& Name) override;

    //XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    static css::uno::Reference< css::text::XTextSection> GetObject( SwSectionFormat& rFormat );
};

class SwXBookmarks : public SwCollectionBaseClass,
    public SwUnoCollection
{
    protected:
        virtual ~SwXBookmarks() override;
    public:
        SwXBookmarks(SwDoc* pDoc);

        //XIndexAccess
        virtual sal_Int32 SAL_CALL getCount() override;
        virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) override;

        //XNameAccess
        virtual css::uno::Any SAL_CALL getByName(const OUString& Name) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
        virtual sal_Bool SAL_CALL hasByName(const OUString& Name) override;

        //XElementAccess
        virtual css::uno::Type SAL_CALL getElementType(  ) override;
        virtual sal_Bool SAL_CALL hasElements(  ) override;

        //XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override;
        virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

};

class SwXNumberingRulesCollection : public cppu::WeakImplHelper
<
    css::container::XIndexAccess
>,
    public SwUnoCollection
{
protected:
    virtual ~SwXNumberingRulesCollection() override;

public:
    SwXNumberingRulesCollection( SwDoc* pDoc );

    //XIndexAccess
    virtual sal_Int32 SAL_CALL SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) override;

    //XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;
};

typedef
cppu::WeakImplHelper
<
    css::container::XIndexAccess,
    css::lang::XServiceInfo
>
SwSimpleIndexAccessBaseClass;

class SwXFootnotes : public SwSimpleIndexAccessBaseClass,
                     public SwUnoCollection
{
    const bool m_bEndnote;

protected:
    virtual ~SwXFootnotes() override;
public:
    SwXFootnotes(bool bEnd, SwDoc* pDoc);

    //XIndexAccess
    virtual sal_Int32 SAL_CALL SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) override;

    //XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    static css::uno::Reference< css::text::XFootnote> GetObject( SwDoc& rDoc, const SwFormatFootnote& rFormat );
};

class SwXReferenceMarks : public SwCollectionBaseClass,
    public SwUnoCollection
{
protected:
    virtual ~SwXReferenceMarks() override;
public:
    SwXReferenceMarks(SwDoc* pDoc);

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) override;

    //XNameAccess
    virtual css::uno::Any SAL_CALL getByName(const OUString& Name) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName(const OUString& Name) override;

    //XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
