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

#ifndef INCLUDED_FILTER_SOURCE_T602_T602FILTER_HXX
#define INCLUDED_FILTER_SOURCE_T602_T602FILTER_HXX

#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XLocalizable.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <cppuhelper/implbase5.hxx>
#include <cppuhelper/implbase4.hxx>
#include <xmloff/attrlist.hxx>
#include <tools/resmgr.hxx>

namespace T602ImportFilter {

typedef enum {L2,KAM,KOI} tcode;

typedef enum {
        standard,   // default
        fat,        // bold
        cursive,    // italic
        bold,       // wide
        tall,       // high
        big,        // big
        lindex,     // lower index
        hindex,     // upper index
        err,        // not set yet
        chngul      // change underline
} fonts;

typedef enum {START,READCH,EOL,POCMD,EXPCMD,SETCMD,SETCH,WRITE,EEND,QUIT} tnode;

OUString getImplementationName()
    throw ( ::com::sun::star::uno::RuntimeException );

::com::sun::star::uno::Sequence < OUString > getSupportedServiceNames()
    throw ( ::com::sun::star::uno::RuntimeException );


::com::sun::star::uno::Reference < ::com::sun::star::uno::XInterface > SAL_CALL
    CreateInstance( const ::com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > &r)
    throw ( ::com::sun::star::uno::Exception );



// class T602ImportFilter


struct inistruct
{
        bool showcomm;      // true   show comment lines
        bool forcecode;     // false  the user has changed the encoding with something else than @CT
        tcode xcode;        // KAM    encoding set - forced
        bool ruscode;       // false  Russian tables turned on
        bool reformatpars;  // false  Reformat paragraphs (whitespaces and line breaks)
        sal_Int16 fontsize;       // font size in points

        inistruct()
            : showcomm( true )
            , forcecode( false )
            , xcode ( KAM )
            , ruscode ( false )
            , reformatpars ( false )
            , fontsize (10)
        {
        };
};

class T602ImportFilterDialog : public cppu::WeakImplHelper4 <
        com::sun::star::ui::dialogs::XExecutableDialog,
        com::sun::star::lang::XLocalizable,
        com::sun::star::lang::XServiceInfo,
        com::sun::star::beans::XPropertyAccess
>
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mxMSF;
    com::sun::star::lang::Locale meLocale;
    ResMgr *mpResMgr;
    sal_Bool OptionsDlg();
    ResMgr* getResMgr();
    OUString getResStr( sal_Int16 resid );
    void initLocale();

    virtual ~T602ImportFilterDialog();

    // XExecutableDialog
       virtual void SAL_CALL setTitle( const OUString& aTitle )
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
       virtual sal_Int16 SAL_CALL execute()
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XLocalizable
        virtual void SAL_CALL setLocale( const com::sun::star::lang::Locale& eLocale )
            throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual com::sun::star::lang::Locale SAL_CALL getLocale()
            throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  )
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XPropertyAccess
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >
                            SAL_CALL getPropertyValues() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL   setPropertyValues( const ::com::sun::star::uno::Sequence<
                                    ::com::sun::star::beans::PropertyValue >& aProps )
                                throw (::com::sun::star::beans::UnknownPropertyException,
                                        ::com::sun::star::beans::PropertyVetoException,
                                        ::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

public:
    T602ImportFilterDialog(const ::com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory > &r );

};


class T602ImportFilter : public cppu::WeakImplHelper5 <
    com::sun::star::document::XFilter,
    com::sun::star::document::XImporter,
    com::sun::star::document::XExtendedFilterDetection,
    com::sun::star::lang::XInitialization,
    com::sun::star::lang::XServiceInfo
>
{
private:
//    ::com::sun::star::uno::Reference<com::sun::star::io::XInputStream> xInputStream;
    ::com::sun::star::uno::Reference<com::sun::star::xml::sax::XDocumentHandler> mxHandler;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mxMSF;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > mxDoc;
    ::com::sun::star::uno::Reference < ::com::sun::star::io::XInputStream > mxInputStream;
    OUString msFilterName;

    SvXMLAttributeList *mpAttrList;

    tnode node;         // START

    struct {
        sal_Int16   mt;       // row for header
        sal_Int16   mb;       // row for footer
        sal_Int16   tb;       // tabs
        sal_Int16   ct;       // encoding (0-kamenik, 1-latin2, 2-koi8)
        sal_Int16   pn;       // from page number
        sal_Int16   lh;       // linespacing 3-2x, 4-1.5x, 6-1x
        sal_Int16   lm;       // left border
        sal_Int16   rm;       // right border
        sal_Int16   pl;       // page length
    } format602;

    // Initialisation constants - they are not changed during the conversion

    inistruct ini;

    // Font state - changes based on font

    struct {
        fonts nowfnt;     // current font
        fonts oldfnt;     // previous font
        bool uline;       // underlined
        bool olduline;    // previous value of uline (font change)
    } fst;

    // Paragraph state

    struct {
        sal_Int16 pars;            //       the number of line endings times linespacing on the current page
        bool comment;        //       in comments
        sal_Int16 wasspace;        // 0     there was a space - for reformatting
        bool wasfdash;       // 0     formatting dash
        bool ccafterln;      // false
        bool willbeeop;      // false
        bool waspar;         // false
    } pst;

    void Reset602();
    unsigned char Readchar602();
    void Read602();
    void par602(bool endofpage);
    void inschr(unsigned char ch);
    void inschrdef(unsigned char ch);
    unsigned char Setformat602(char *cmd);
    sal_Int16 readnum(unsigned char *ch, bool show);
    tnode PointCmd602(unsigned char *ch);
    void setfnt(fonts fnt,bool mustwrite);
    void wrtfnt();

    sal_Bool SAL_CALL importImpl( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
        throw (::com::sun::star::uno::RuntimeException);

    public:
        T602ImportFilter(const ::com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory > &r );
        virtual ~T602ImportFilter();

    // XFilter
        virtual sal_Bool SAL_CALL filter( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL cancel(  )
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE {};

    // XImporter
        virtual void SAL_CALL setTargetDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc )
            throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XExtendedTypeDetection
        virtual OUString SAL_CALL detect(
            com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& Descriptor )
            throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
            throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  )
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

OUString T602ImportFilter_getImplementationName()
    throw ( ::com::sun::star::uno::RuntimeException );

sal_Bool SAL_CALL T602ImportFilter_supportsService( const OUString& ServiceName )
    throw ( ::com::sun::star::uno::RuntimeException );

::com::sun::star::uno::Sequence< OUString > SAL_CALL T602ImportFilter_getSupportedServiceNames(  )
    throw ( ::com::sun::star::uno::RuntimeException );

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
SAL_CALL T602ImportFilter_createInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & rSMgr)
    throw ( ::com::sun::star::uno::Exception );

OUString T602ImportFilterDialog_getImplementationName()
    throw ( ::com::sun::star::uno::RuntimeException );

sal_Bool SAL_CALL T602ImportFilterDialog_supportsService( const OUString& ServiceName )
    throw ( ::com::sun::star::uno::RuntimeException );

::com::sun::star::uno::Sequence< OUString > SAL_CALL T602ImportFilterDialog_getSupportedServiceNames(  )
    throw ( ::com::sun::star::uno::RuntimeException );

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
SAL_CALL T602ImportFilterDialog_createInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & rSMgr)
    throw ( ::com::sun::star::uno::Exception );

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
