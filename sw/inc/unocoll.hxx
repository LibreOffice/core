/*************************************************************************
 *
 *  $RCSfile: unocoll.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: os $ $Date: 2001-03-23 13:39:12 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _UNOCOLL_HXX
#define _UNOCOLL_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _FLYENUM_HXX //autogen
#include <flyenum.hxx>
#endif
#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif
#ifndef _COM_SUN_STAR_UTIL_XPROTECTABLE_HPP_
#include <com/sun/star/util/XProtectable.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx> // helper for implementations
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx> // helper for implementations
#endif
#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx> // helper for implementations
#endif
/***************************************************
 ***************************************************
 *
 ***************************************************/
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
class SwBookmark;
class SwFmtRefMark;
class SwXReferenceMark;
class SwXBookmark;
/*-----------------11.03.98 11:19-------------------

--------------------------------------------------*/
class SwUnoCollection
{
    sal_Bool    bObjectValid;
    SwDoc*  pDoc;

    public:
        SwUnoCollection(SwDoc* p) :
            pDoc(p),
            bObjectValid(sal_True){}

    virtual void                Invalidate();
    sal_Bool                        IsValid() const {return bObjectValid;}


    SwDoc*          GetDoc() const {return pDoc;}
};

/******************************************************************************
 *
 ******************************************************************************/
#define SW_SERVICE_TYPE_TEXTTABLE                       0   //
#define SW_SERVICE_TYPE_TEXTFRAME                       1   //
#define SW_SERVICE_TYPE_GRAPHIC                         2
#define SW_SERVICE_TYPE_OLE                             3
#define SW_SERVICE_TYPE_BOOKMARK                        4
#define SW_SERVICE_TYPE_FOOTNOTE                        5
#define SW_SERVICE_TYPE_ENDNOTE                         6
#define SW_SERVICE_TYPE_INDEXMARK                       7 // Stichwortverzeichniseintrag
#define SW_SERVICE_TYPE_INDEX                           8 // Stichwortverzeichnis
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
#define SW_SERVICE_FIELDTYPE_TABLEFIELD                 48
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
#define SW_SERVICE_FIELDTYPE_DOCINFO_INFO_0             62
#define SW_SERVICE_FIELDTYPE_DOCINFO_INFO_1             63
#define SW_SERVICE_FIELDTYPE_DOCINFO_INFO_2             64
#define SW_SERVICE_FIELDTYPE_DOCINFO_INFO_3             65
#define SW_SERVICE_FIELDTYPE_DOCINFO_PRINT_AUTHOR       66
#define SW_SERVICE_FIELDTYPE_DOCINFO_PRINT_DATE_TIME    67
#define SW_SERVICE_FIELDTYPE_DOCINFO_KEY_WORDS          68
#define SW_SERVICE_FIELDTYPE_DOCINFO_SUBJECT            69
#define SW_SERVICE_FIELDTYPE_DOCINFO_TITLE              70
#define SW_SERVICE_FIELDTYPE_DOCINFO_REVISION           71
#define SW_SERVICE_FIELDTYPE_BIBLIOGRAPHY               72
#define SW_SERVICE_FIELDTYPE_COMBINED_CHARACTERS        73
#define SW_SERVICE_FIELDTYPE_DUMMY_3                    74
#define SW_SERVICE_FIELDTYPE_DUMMY_4                    75
#define SW_SERVICE_FIELDTYPE_DUMMY_5                    76
#define SW_SERVICE_FIELDTYPE_DUMMY_6                    77
#define SW_SERVICE_FIELDTYPE_DUMMY_7                    78
#define SW_SERVICE_FIELDTYPE_DUMMY_8                    79
#define SW_SERVICE_FIELDMASTER_USER                     80
#define SW_SERVICE_FIELDMASTER_DDE                      81
#define SW_SERVICE_FIELDMASTER_SET_EXP                  82
#define SW_SERVICE_FIELDMASTER_DATABASE                 83
#define SW_SERVICE_FIELDMASTER_BIBLIOGRAPHY             84
#define SW_SERVICE_FIELDMASTER_DUMMY2                   85
#define SW_SERVICE_FIELDMASTER_DUMMY3                   86
#define SW_SERVICE_FIELDMASTER_DUMMY4                   87
#define SW_SERVICE_FIELDMASTER_DUMMY5                   88
#define SW_SERVICE_INDEX_ILLUSTRATIONS                  89
#define SW_SERVICE_INDEX_OBJECTS                        90
#define SW_SERVICE_INDEX_TABLES                         91
#define SW_SERVICE_INDEX_BIBLIOGRAPHY                   92
#define SW_SERVICE_PARAGRAPH                            93
#define SW_SERVICE_FIELDTYPE_INPUT_USER                 94
#define SW_SERVICE_FIELDTYPE_HIDDEN_TEXT                95
#define SW_SERVICE_STYLE_CONDITIONAL_PARAGRAPH_STYLE    96
#define SW_SERVICE_NUMBERING_RULES                      97
#define SW_SERVICE_TEXT_COLUMNS                         98
#define SW_SERVICE_INDEX_HEADER_SECTION                 99
#define SW_SERVICE_DEFAULTS                             100
#define SW_SERVICE_IMAP_RECTANGLE                       101
#define SW_SERVICE_IMAP_CIRCLE                          102
#define SW_SERVICE_IMAP_POLYGON                         103

#define SW_SERVICE_LAST                 SW_SERVICE_IMAP_POLYGON


#define SW_SERVICE_INVALID          USHRT_MAX

class SwXServiceProvider
{
public:
    static rtl::OUString        GetProviderName(sal_uInt16 nObjectType);
    static sal_uInt16           GetProviderType(const rtl::OUString& rServiceName);
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >        MakeInstance(sal_uInt16 nObjectType, SwDoc* pDoc);
    static ::com::sun::star::uno::Sequence<rtl::OUString>   GetAllServiceNames();

};
/*-----------------11.12.97 09:38-------------------

--------------------------------------------------*/
typedef
cppu::WeakImplHelper3
<
    ::com::sun::star::container::XNameAccess,
    ::com::sun::star::container::XIndexAccess,
    ::com::sun::star::lang::XServiceInfo
>
SwCollectionBaseClass;
class SwXTextTables : public SwCollectionBaseClass,
    public SwUnoCollection
{
public:
    SwXTextTables(SwDoc* pDoc);
    virtual ~SwXTextTables();

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
    virtual BOOL SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    static ::com::sun::star::text::XTextTable*          GetObject( SwFrmFmt& rFmt );
};

/*-----------------11.12.97 10:14-------------------

--------------------------------------------------*/
class SwXFrames : public SwCollectionBaseClass,
    public SwUnoCollection
{
    const FlyCntType    eType;
protected:

    SwXFrame*           GetFrmByName( const String& rName );
    SwXFrame*           GetFrmByIndex( sal_Int32 nIndex );

    void                RemoveFrame(const rtl::OUString& rName, FlyCntType eType);

public:
        SwXFrames(SwDoc* pDoc, FlyCntType eSet);
        virtual ~SwXFrames();

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
    virtual BOOL SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    FlyCntType          GetType()const{return eType;}
    static SwXFrame*    GetObject( SwFrmFmt& rFmt, FlyCntType eType );

};

/*-----------------20.02.98 10:51-------------------

--------------------------------------------------*/
class SwXTextFrames : public SwXFrames
{
public:
    SwXTextFrames(SwDoc* pDoc);
    virtual ~SwXTextFrames();

//  SMART_UNO_DECLARATION( SwXTextFrames, UsrObject );

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

};

/*-----------------20.02.98 10:55-------------------

--------------------------------------------------*/
class SwXTextGraphicObjects : public SwXFrames
{
public:
        SwXTextGraphicObjects(SwDoc* pDoc);
        virtual ~SwXTextGraphicObjects();

//  SMART_UNO_DECLARATION( SwXTextGraphicObjects, UsrObject );

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

};

/*-----------------20.02.98 10:57-------------------

--------------------------------------------------*/
class SwXTextEmbeddedObjects :  public SwXFrames
{

public:
    SwXTextEmbeddedObjects(SwDoc* pDoc);
    virtual ~SwXTextEmbeddedObjects();

//  SMART_UNO_DECLARATION( SwXTextEmbeddedObjects, UsrObject );

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

};

/*-----------------12.02.98 07:58-------------------

--------------------------------------------------*/
class SwXTextSections : public cppu::WeakImplHelper4
<
        ::com::sun::star::container::XNameAccess,
        ::com::sun::star::lang::XServiceInfo,
        ::com::sun::star::container::XIndexAccess,
        ::com::sun::star::util::XProtectable
>,
        public SwUnoCollection
{
    Timer   aWrongPasswdTimer;
    sal_uInt16  nWrongPasswd;
    DECL_LINK(WrongPasswordTimerHdl, Timer*);
public:
    SwXTextSections(SwDoc* pDoc);
    virtual ~SwXTextSections();

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

    //XProtectable
    virtual void SAL_CALL protect(const rtl::OUString& aPassword) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL unprotect(const rtl::OUString& aPassword) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL isProtected(void) throw( ::com::sun::star::uno::RuntimeException );

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    static ::com::sun::star::text::XTextSection*        GetObject( SwSectionFmt& rFmt );
};
/*-----------------12.02.98 07:51-------------------

--------------------------------------------------*/
class SwXBookmarks : public SwCollectionBaseClass,
    public SwUnoCollection
{
public:
    SwXBookmarks(SwDoc* pDoc);
    virtual ~SwXBookmarks();

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
    virtual BOOL SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    static SwXBookmark*     GetObject( SwBookmark& rBkm, SwDoc* pDoc );
};

/*-----------------12.02.98 08:01-------------------

--------------------------------------------------*/
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
    sal_Bool                bEndnote;

public:
    SwXFootnotes(sal_Bool bEnd, SwDoc* pDoc);
    virtual ~SwXFootnotes();

    //XIndexAccess
    virtual sal_Int32 SAL_CALL SAL_CALL getCount(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    static ::com::sun::star::uno::Reference< ::com::sun::star::text::XFootnote> GetObject( SwDoc& rDoc, const SwFmtFtn& rFmt );
};

/* -----------------27.08.98 13:10-------------------
 *
 * --------------------------------------------------*/
class SwXReferenceMarks : public SwCollectionBaseClass,
    public SwUnoCollection
{
public:
    SwXReferenceMarks(SwDoc* pDoc);
    virtual ~SwXReferenceMarks();

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
    virtual BOOL SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    static SwXReferenceMark* GetObject( SwDoc* pDoc, const SwFmtRefMark* pMark );
};

#endif






