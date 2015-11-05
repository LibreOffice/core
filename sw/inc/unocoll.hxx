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

#define SW_SERVICE_TYPE_TEXTTABLE                       0
#define SW_SERVICE_TYPE_TEXTFRAME                       1
#define SW_SERVICE_TYPE_GRAPHIC                         2
#define SW_SERVICE_TYPE_OLE                             3
#define SW_SERVICE_TYPE_BOOKMARK                        4
#define SW_SERVICE_TYPE_FOOTNOTE                        5
#define SW_SERVICE_TYPE_ENDNOTE                         6
#define SW_SERVICE_TYPE_INDEXMARK                       7
#define SW_SERVICE_TYPE_INDEX                           8
#define SW_SERVICE_REFERENCE_MARK                       9
#define SW_SERVICE_STYLE_CHARACTER_STYLE                10
#define SW_SERVICE_STYLE_PARAGRAPH_STYLE                11
#define SW_SERVICE_STYLE_FRAME_STYLE                    12
#define SW_SERVICE_STYLE_PAGE_STYLE                     13
#define SW_SERVICE_STYLE_NUMBERING_STYLE                14
#define SW_SERVICE_CONTENT_INDEX_MARK                   15
#define SW_SERVICE_CONTENT_INDEX                        16
#define SW_SERVICE_USER_INDEX_MARK                      17
#define SW_SERVICE_USER_INDEX                           18
#define SW_SERVICE_TEXT_SECTION                         19
#define SW_SERVICE_FIELDTYPE_DATETIME                   20
#define SW_SERVICE_FIELDTYPE_USER                       21
#define SW_SERVICE_FIELDTYPE_SET_EXP                    22
#define SW_SERVICE_FIELDTYPE_GET_EXP                    23
#define SW_SERVICE_FIELDTYPE_FILE_NAME                  24
#define SW_SERVICE_FIELDTYPE_PAGE_NUM                   25
#define SW_SERVICE_FIELDTYPE_AUTHOR                     26
#define SW_SERVICE_FIELDTYPE_CHAPTER                    27
#define SW_SERVICE_FIELDTYPE_DUMMY_0                    28
#define SW_SERVICE_FIELDTYPE_GET_REFERENCE              29
#define SW_SERVICE_FIELDTYPE_CONDITIONED_TEXT           30
#define SW_SERVICE_FIELDTYPE_ANNOTATION                 31
#define SW_SERVICE_FIELDTYPE_INPUT                      32
#define SW_SERVICE_FIELDTYPE_MACRO                      33
#define SW_SERVICE_FIELDTYPE_DDE                        34
#define SW_SERVICE_FIELDTYPE_HIDDEN_PARA                35
#define SW_SERVICE_FIELDTYPE_DOC_INFO                   36
#define SW_SERVICE_FIELDTYPE_TEMPLATE_NAME              37
#define SW_SERVICE_FIELDTYPE_USER_EXT                   38
#define SW_SERVICE_FIELDTYPE_REF_PAGE_SET               39
#define SW_SERVICE_FIELDTYPE_REF_PAGE_GET               40
#define SW_SERVICE_FIELDTYPE_JUMP_EDIT                  41
#define SW_SERVICE_FIELDTYPE_SCRIPT                     42
#define SW_SERVICE_FIELDTYPE_DATABASE_NEXT_SET          43
#define SW_SERVICE_FIELDTYPE_DATABASE_NUM_SET           44
#define SW_SERVICE_FIELDTYPE_DATABASE_SET_NUM           45
#define SW_SERVICE_FIELDTYPE_DATABASE                   46
#define SW_SERVICE_FIELDTYPE_DATABASE_NAME              47
#define SW_SERVICE_FIELDTYPE_TABLE_FORMULA              48
#define SW_SERVICE_FIELDTYPE_PAGE_COUNT                 49
#define SW_SERVICE_FIELDTYPE_PARAGRAPH_COUNT            50
#define SW_SERVICE_FIELDTYPE_WORD_COUNT                 51
#define SW_SERVICE_FIELDTYPE_CHARACTER_COUNT            52
#define SW_SERVICE_FIELDTYPE_TABLE_COUNT                53
#define SW_SERVICE_FIELDTYPE_GRAPHIC_OBJECT_COUNT       54
#define SW_SERVICE_FIELDTYPE_EMBEDDED_OBJECT_COUNT      55
#define SW_SERVICE_FIELDTYPE_DOCINFO_CHANGE_AUTHOR      56
#define SW_SERVICE_FIELDTYPE_DOCINFO_CHANGE_DATE_TIME   57
#define SW_SERVICE_FIELDTYPE_DOCINFO_EDIT_TIME          58
#define SW_SERVICE_FIELDTYPE_DOCINFO_DESCRIPTION        59
#define SW_SERVICE_FIELDTYPE_DOCINFO_CREATE_AUTHOR      60
#define SW_SERVICE_FIELDTYPE_DOCINFO_CREATE_DATE_TIME   61
#define SW_SERVICE_FIELDTYPE_DUMMY_1                    63
#define SW_SERVICE_FIELDTYPE_DUMMY_2                    64
#define SW_SERVICE_FIELDTYPE_DUMMY_3                    65
#define SW_SERVICE_FIELDTYPE_DOCINFO_CUSTOM             66
#define SW_SERVICE_FIELDTYPE_DOCINFO_PRINT_AUTHOR       67
#define SW_SERVICE_FIELDTYPE_DOCINFO_PRINT_DATE_TIME    68
#define SW_SERVICE_FIELDTYPE_DOCINFO_KEY_WORDS          69
#define SW_SERVICE_FIELDTYPE_DOCINFO_SUBJECT            70
#define SW_SERVICE_FIELDTYPE_DOCINFO_TITLE              71
#define SW_SERVICE_FIELDTYPE_DOCINFO_REVISION           72
#define SW_SERVICE_FIELDTYPE_BIBLIOGRAPHY               73
#define SW_SERVICE_FIELDTYPE_COMBINED_CHARACTERS        74
#define SW_SERVICE_FIELDTYPE_DROPDOWN                   75
#define SW_SERVICE_FIELDTYPE_METAFIELD                  76
#define SW_SERVICE_FIELDTYPE_DUMMY_4                    77
#define SW_SERVICE_FIELDTYPE_DUMMY_5                    78
#define SW_SERVICE_FIELDTYPE_DUMMY_6                    79
#define SW_SERVICE_FIELDTYPE_DUMMY_7                    80
#define SW_SERVICE_FIELDMASTER_USER                     81
#define SW_SERVICE_FIELDMASTER_DDE                      82
#define SW_SERVICE_FIELDMASTER_SET_EXP                  83
#define SW_SERVICE_FIELDMASTER_DATABASE                 84
#define SW_SERVICE_FIELDMASTER_BIBLIOGRAPHY             85
#define SW_SERVICE_FIELDMASTER_DUMMY2                   86
#define SW_SERVICE_FIELDMASTER_DUMMY3                   87
#define SW_SERVICE_FIELDMASTER_DUMMY4                   88
#define SW_SERVICE_FIELDMASTER_DUMMY5                   89
#define SW_SERVICE_INDEX_ILLUSTRATIONS                  90
#define SW_SERVICE_INDEX_OBJECTS                        91
#define SW_SERVICE_INDEX_TABLES                         92
#define SW_SERVICE_INDEX_BIBLIOGRAPHY                   93
#define SW_SERVICE_PARAGRAPH                            94
#define SW_SERVICE_FIELDTYPE_INPUT_USER                 95
#define SW_SERVICE_FIELDTYPE_HIDDEN_TEXT                96
#define SW_SERVICE_STYLE_CONDITIONAL_PARAGRAPH_STYLE    97
#define SW_SERVICE_NUMBERING_RULES                      98
#define SW_SERVICE_TEXT_COLUMNS                         99
#define SW_SERVICE_INDEX_HEADER_SECTION                 100
#define SW_SERVICE_DEFAULTS                             101
#define SW_SERVICE_IMAP_RECTANGLE                       102
#define SW_SERVICE_IMAP_CIRCLE                          103
#define SW_SERVICE_IMAP_POLYGON                         104
#define SW_SERVICE_TYPE_TEXT_GRAPHIC                    105
#define SW_SERVICE_CHART2_DATA_PROVIDER                 106
#define SW_SERVICE_TYPE_FIELDMARK                       107
#define SW_SERVICE_TYPE_FORMFIELDMARK                   108
#define SW_SERVICE_TYPE_META                            109
#define SW_SERVICE_VBAOBJECTPROVIDER                    110
#define SW_SERVICE_VBACODENAMEPROVIDER                  111
#define SW_SERVICE_VBAPROJECTNAMEPROVIDER               112
#define SW_SERVICE_VBAGLOBALS                           113


#define SW_SERVICE_INVALID          USHRT_MAX

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
    static OUString             GetProviderName(sal_uInt16 nObjectType);
    static sal_uInt16           GetProviderType(const OUString& rServiceName);
    static css::uno::Reference< css::uno::XInterface >        MakeInstance(sal_uInt16 nObjectType, SwDoc & rDoc);
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
    virtual ~SwXTextTables();
public:
    SwXTextTables(SwDoc* pDoc);

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw( css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception ) override;

    //XNameAccess
    virtual css::uno::Any SAL_CALL getByName(const OUString& Name)  throw( css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL hasByName(const OUString& Name) throw( css::uno::RuntimeException, std::exception ) override;

    //XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements(  ) throw(css::uno::RuntimeException, std::exception) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

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
    virtual ~SwXFrames();
public:
    SwXFrames(SwDoc* pDoc, FlyCntType eSet);

    //XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw( css::uno::RuntimeException, std::exception ) override;

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw( css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception ) override;

    //XNameAccess
    virtual css::uno::Any SAL_CALL getByName(const OUString& Name) throw( css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL hasByName(const OUString& Name) throw( css::uno::RuntimeException, std::exception ) override;

    //XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements(  ) throw(css::uno::RuntimeException, std::exception) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;
};

class SwXTextFrames : public SwXFrames
{
protected:
    virtual ~SwXTextFrames();
public:
    SwXTextFrames(SwDoc* pDoc);

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

};

class SwXTextGraphicObjects : public SwXFrames
{
protected:
    virtual ~SwXTextGraphicObjects();
public:
        SwXTextGraphicObjects(SwDoc* pDoc);

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

};

class SwXTextEmbeddedObjects :  public SwXFrames
{
protected:
    virtual ~SwXTextEmbeddedObjects();
public:
    SwXTextEmbeddedObjects(SwDoc* pDoc);

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

};

class SwXTextSections : public SwCollectionBaseClass,
                        public SwUnoCollection
{
protected:
    virtual ~SwXTextSections();
public:
    SwXTextSections(SwDoc* pDoc);

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw( css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception ) override;

    //XNameAccess
    virtual css::uno::Any SAL_CALL getByName(const OUString& Name) throw( css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL hasByName(const OUString& Name) throw( css::uno::RuntimeException, std::exception ) override;

    //XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements(  ) throw(css::uno::RuntimeException, std::exception) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

    static css::uno::Reference< css::text::XTextSection> GetObject( SwSectionFormat& rFormat );
};

class SwXBookmarks : public SwCollectionBaseClass,
    public SwUnoCollection
{
    protected:
        virtual ~SwXBookmarks();
    public:
        SwXBookmarks(SwDoc* pDoc);

        //XIndexAccess
        virtual sal_Int32 SAL_CALL getCount() throw( css::uno::RuntimeException, std::exception ) override;
        virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw( css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception ) override;

        //XNameAccess
        virtual css::uno::Any SAL_CALL getByName(const OUString& Name) throw( css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() throw( css::uno::RuntimeException, std::exception ) override;
        virtual sal_Bool SAL_CALL hasByName(const OUString& Name) throw( css::uno::RuntimeException, std::exception ) override;

        //XElementAccess
        virtual css::uno::Type SAL_CALL getElementType(  ) throw(css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL hasElements(  ) throw(css::uno::RuntimeException, std::exception) override;

        //XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
        virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( css::uno::RuntimeException, std::exception ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

};

class SwXNumberingRulesCollection : public cppu::WeakImplHelper
<
    css::container::XIndexAccess
>,
    public SwUnoCollection
{
protected:
    virtual ~SwXNumberingRulesCollection();

public:
    SwXNumberingRulesCollection( SwDoc* pDoc );

    //XIndexAccess
    virtual sal_Int32 SAL_CALL SAL_CALL getCount() throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw( css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception ) override;

    //XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements(  ) throw(css::uno::RuntimeException, std::exception) override;
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
    virtual ~SwXFootnotes();
public:
    SwXFootnotes(bool bEnd, SwDoc* pDoc);

    //XIndexAccess
    virtual sal_Int32 SAL_CALL SAL_CALL getCount() throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw( css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception ) override;

    //XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements(  ) throw(css::uno::RuntimeException, std::exception) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

    static css::uno::Reference< css::text::XFootnote> GetObject( SwDoc& rDoc, const SwFormatFootnote& rFormat );
};

class SwXReferenceMarks : public SwCollectionBaseClass,
    public SwUnoCollection
{
protected:
    virtual ~SwXReferenceMarks();
public:
    SwXReferenceMarks(SwDoc* pDoc);

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw( css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception ) override;

    //XNameAccess
    virtual css::uno::Any SAL_CALL getByName(const OUString& Name) throw( css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL hasByName(const OUString& Name) throw( css::uno::RuntimeException, std::exception ) override;

    //XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements(  ) throw(css::uno::RuntimeException, std::exception) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
