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
#ifndef _UNOCOLL_HXX
#define _UNOCOLL_HXX
#include <tools/solar.h>
#include <flyenum.hxx>
#include <vcl/timer.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <cppuhelper/implbase1.hxx> // helper for implementations
#include <cppuhelper/implbase2.hxx> // helper for implementations
#include <cppuhelper/implbase3.hxx> // helper for implementations
#include <cppuhelper/implbase4.hxx> // helper for implementations
#include <IMark.hxx>
#include <unobaseclass.hxx>
#include "swdllapi.h"

class SwDoc;
class SwFrmFmt;
namespace com{namespace sun {namespace star {namespace text
{
    class XTextTable;
    class XTextSection;
    class XFootnote;
}}}}
class SwXFrame;
class SwFrmFmt;
class SwFmtFtn;
class XBookmark;
class SwXReferenceMark;
class SwSectionFmt;
class SwFmtRefMark;
class SwXReferenceMark;
class SwXBookmark;

class SwUnoCollection
{
    SwDoc*      pDoc;
    sal_Bool    bObjectValid;

    public:
        SwUnoCollection(SwDoc* p) :
            pDoc(p),
            bObjectValid(sal_True){}

    virtual ~SwUnoCollection() {}

    virtual void                Invalidate();
    sal_Bool                        IsValid() const {return bObjectValid;}


    SwDoc*          GetDoc() const {return pDoc;}
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
#define SW_SERVICE_FIELDTYPE_DUMMY_O                    62
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

#define SW_SERVICE_LAST                 SW_SERVICE_VBAGLOBALS

#define SW_SERVICE_INVALID          USHRT_MAX

//
// case-corrected versions of service-names (see #i67811)
//
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
    static rtl::OUString        GetProviderName(sal_uInt16 nObjectType);
    static sal_uInt16           GetProviderType(const rtl::OUString& rServiceName);
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >        MakeInstance(sal_uInt16 nObjectType, SwDoc* pDoc);
    static ::com::sun::star::uno::Sequence<rtl::OUString>   GetAllServiceNames();

};

typedef
cppu::WeakImplHelper3
<
    ::com::sun::star::container::XNameAccess,
    ::com::sun::star::container::XIndexAccess,
    ::com::sun::star::lang::XServiceInfo
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
    virtual sal_Int32 SAL_CALL getCount(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

    //XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName(const rtl::OUString& Name)  throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getElementNames(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL hasByName(const rtl::OUString& Name) throw( ::com::sun::star::uno::RuntimeException );

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    static ::com::sun::star::text::XTextTable*          GetObject( SwFrmFmt& rFmt );
};

typedef
cppu::WeakImplHelper4
<
    ::com::sun::star::container::XEnumerationAccess,
    ::com::sun::star::container::XNameAccess,
    ::com::sun::star::container::XIndexAccess,
    ::com::sun::star::lang::XServiceInfo
> SwXFramesBaseClass;

class SwXFrames : public SwXFramesBaseClass,
    public SwUnoCollection
{
    const FlyCntType    eType;
protected:
    virtual ~SwXFrames();
public:
    SwXFrames(SwDoc* pDoc, FlyCntType eSet);

    //XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL createEnumeration(void) throw( ::com::sun::star::uno::RuntimeException );

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

    //XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName(const rtl::OUString& Name) throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getElementNames(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL hasByName(const rtl::OUString& Name) throw( ::com::sun::star::uno::RuntimeException );

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    FlyCntType          GetType()const{return eType;}
    static SwXFrame*    GetObject( SwFrmFmt& rFmt, FlyCntType eType );

};

class SwXTextFrames : public SwXFrames
{
protected:
    virtual ~SwXTextFrames();
public:
    SwXTextFrames(SwDoc* pDoc);


    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

};

class SwXTextGraphicObjects : public SwXFrames
{
protected:
    virtual ~SwXTextGraphicObjects();
public:
        SwXTextGraphicObjects(SwDoc* pDoc);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

};

class SwXTextEmbeddedObjects :  public SwXFrames
{
protected:
    virtual ~SwXTextEmbeddedObjects();
public:
    SwXTextEmbeddedObjects(SwDoc* pDoc);


    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

};

class SwXTextSections : public SwCollectionBaseClass,
                        public SwUnoCollection
{
protected:
    virtual ~SwXTextSections();
public:
    SwXTextSections(SwDoc* pDoc);


    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

    //XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName(const rtl::OUString& Name) throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getElementNames(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL hasByName(const rtl::OUString& Name) throw( ::com::sun::star::uno::RuntimeException );

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    static ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextSection> GetObject( SwSectionFmt& rFmt );
};

class SwXBookmarks : public SwCollectionBaseClass,
    public SwUnoCollection
{
    protected:
        virtual ~SwXBookmarks();
    public:
        SwXBookmarks(SwDoc* pDoc);


        //XIndexAccess
        virtual sal_Int32 SAL_CALL getCount(void) throw( ::com::sun::star::uno::RuntimeException );
        virtual ::com::sun::star::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

        //XNameAccess
        virtual ::com::sun::star::uno::Any SAL_CALL getByName(const rtl::OUString& Name) throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
        virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getElementNames(void) throw( ::com::sun::star::uno::RuntimeException );
        virtual sal_Bool SAL_CALL hasByName(const rtl::OUString& Name) throw( ::com::sun::star::uno::RuntimeException );

        //XElementAccess
        virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

        //XServiceInfo
        virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
        virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
        virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

};

class SwXNumberingRulesCollection : public cppu::WeakImplHelper1
<
    ::com::sun::star::container::XIndexAccess
>,
    public SwUnoCollection
{
protected:
    virtual ~SwXNumberingRulesCollection();

public:
    SwXNumberingRulesCollection( SwDoc* pDoc );

    //XIndexAccess
    virtual sal_Int32 SAL_CALL SAL_CALL getCount(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);
};

typedef
cppu::WeakImplHelper2
<
    ::com::sun::star::container::XIndexAccess,
    ::com::sun::star::lang::XServiceInfo
>
SwSimpleIndexAccessBaseClass;

class SwXFootnotes : public SwSimpleIndexAccessBaseClass,
                     public SwUnoCollection
{
    const bool m_bEndnote;

protected:
    virtual ~SwXFootnotes();
public:
    SwXFootnotes(sal_Bool bEnd, SwDoc* pDoc);

    //XIndexAccess
    virtual sal_Int32 SAL_CALL SAL_CALL getCount(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    static ::com::sun::star::uno::Reference< ::com::sun::star::text::XFootnote> GetObject( SwDoc& rDoc, const SwFmtFtn& rFmt );
};

class SwXReferenceMarks : public SwCollectionBaseClass,
    public SwUnoCollection
{
protected:
    virtual ~SwXReferenceMarks();
public:
    SwXReferenceMarks(SwDoc* pDoc);


    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

    //XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName(const rtl::OUString& Name) throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getElementNames(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL hasByName(const rtl::OUString& Name) throw( ::com::sun::star::uno::RuntimeException );

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    static SwXReferenceMark* GetObject( SwDoc* pDoc, const SwFmtRefMark* pMark );
};

#endif






/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
