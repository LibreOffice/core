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
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XLocalizable.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <cppuhelper/implbase.hxx>
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
    throw ( css::uno::RuntimeException );

css::uno::Sequence < OUString > getSupportedServiceNames()
    throw ( css::uno::RuntimeException );


css::uno::Reference < css::uno::XInterface > SAL_CALL
    CreateInstance( const css::uno::Reference< css::lang::XMultiServiceFactory > &r)
    throw ( css::uno::Exception );



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

class T602ImportFilterDialog : public cppu::WeakImplHelper <
        css::ui::dialogs::XExecutableDialog,
        css::lang::XLocalizable,
        css::lang::XServiceInfo,
        css::beans::XPropertyAccess
>
{
    css::uno::Reference< css::lang::XMultiServiceFactory > mxMSF;
    css::lang::Locale meLocale;
    ResMgr *mpResMgr;
    bool OptionsDlg();
    ResMgr* getResMgr();
    OUString getResStr( sal_Int16 resid );
    void initLocale();

    virtual ~T602ImportFilterDialog();

    // XExecutableDialog
       virtual void SAL_CALL setTitle( const OUString& aTitle )
            throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
       virtual sal_Int16 SAL_CALL execute()
            throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XLocalizable
        virtual void SAL_CALL setLocale( const css::lang::Locale& eLocale )
            throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::lang::Locale SAL_CALL getLocale()
            throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  )
            throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
            throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XPropertyAccess
        virtual css::uno::Sequence< css::beans::PropertyValue >
                            SAL_CALL getPropertyValues() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL   setPropertyValues( const css::uno::Sequence<
                                    css::beans::PropertyValue >& aProps )
                                throw (css::beans::UnknownPropertyException,
                                        css::beans::PropertyVetoException,
                                        css::lang::IllegalArgumentException,
                                        css::lang::WrappedTargetException,
                                        css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

public:
    T602ImportFilterDialog(const css::uno::Reference<css::lang::XMultiServiceFactory > &r );

};


class T602ImportFilter : public cppu::WeakImplHelper <
    css::document::XFilter,
    css::document::XImporter,
    css::document::XExtendedFilterDetection,
    css::lang::XInitialization,
    css::lang::XServiceInfo
>
{
private:
//    css::uno::Reference<css::io::XInputStream> xInputStream;
    css::uno::Reference<css::xml::sax::XDocumentHandler> mxHandler;
    css::uno::Reference< css::lang::XMultiServiceFactory > mxMSF;
    css::uno::Reference< css::lang::XComponent >         mxDoc;
    css::uno::Reference < css::io::XInputStream >        mxInputStream;
    OUString msFilterName;

    SvXMLAttributeList *mpAttrList;

    tnode node;         // START

    struct format602struct
    {
        sal_Int16   mt;       // row for header
        sal_Int16   mb;       // row for footer
        sal_Int16   tb;       // tabs
        sal_Int16   ct;       // encoding (0-kamenik, 1-latin2, 2-koi8)
        sal_Int16   pn;       // from page number
        sal_Int16   lh;       // linespacing 3-2x, 4-1.5x, 6-1x
        sal_Int16   lm;       // left border
        sal_Int16   rm;       // right border
        sal_Int16   pl;       // page length
        format602struct()
            : mt(0)
            , mb(0)
            , tb(0)
            , ct(0)
            , pn(0)
            , lh(0)
            , lm(0)
            , rm(0)
            , pl(0)
        {
        }
    };

    format602struct format602;

    // Initialisation constants - they are not changed during the conversion

    inistruct ini;

    // Font state - changes based on font

    struct fststruct
    {
        fonts nowfnt;     // current font
        fonts oldfnt;     // previous font
        bool uline;       // underlined
        bool olduline;    // previous value of uline (font change)
        fststruct()
            : nowfnt(standard)
            , oldfnt(standard)
            , uline(false)
            , olduline(false)
        {
        }
    };

    fststruct fst;

    // Paragraph state

    struct pststruct
    {
        sal_Int16 pars;            //       the number of line endings times linespacing on the current page
        bool comment;        //       in comments
        sal_Int16 wasspace;        // 0     there was a space - for reformatting
        bool wasfdash;       // 0     formatting dash
        bool ccafterln;      // false
        bool willbeeop;      // false
        bool waspar;         // false
        pststruct()
            : pars(0)
            , comment(false)
            , wasspace(0)
            , wasfdash(false)
            , ccafterln(false)
            , willbeeop(false)
            , waspar(false)
        {
        }
    };

    pststruct pst;

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

    bool SAL_CALL importImpl( const css::uno::Sequence< css::beans::PropertyValue >& aDescriptor )
        throw (css::uno::RuntimeException);

    public:
        T602ImportFilter(const css::uno::Reference<css::lang::XMultiServiceFactory > &r );
        T602ImportFilter(css::uno::Reference<css::io::XInputStream> xInputStream);
        virtual ~T602ImportFilter();

    // XFilter
        virtual sal_Bool SAL_CALL filter( const css::uno::Sequence< css::beans::PropertyValue >& aDescriptor )
            throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL cancel(  )
            throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE {};

    // XImporter
        virtual void SAL_CALL setTargetDocument( const css::uno::Reference< css::lang::XComponent >& xDoc )
            throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XExtendedTypeDetection
        virtual OUString SAL_CALL detect(
            css::uno::Sequence< css::beans::PropertyValue >& Descriptor )
            throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
            throw (css::uno::Exception, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  )
            throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
            throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        bool SAL_CALL test();
};

OUString T602ImportFilter_getImplementationName()
    throw ( css::uno::RuntimeException );

bool SAL_CALL T602ImportFilter_supportsService( const OUString& ServiceName )
    throw ( css::uno::RuntimeException );

css::uno::Sequence< OUString > SAL_CALL T602ImportFilter_getSupportedServiceNames(  )
    throw ( css::uno::RuntimeException );

css::uno::Reference< css::uno::XInterface >
SAL_CALL T602ImportFilter_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr)
    throw ( css::uno::Exception );

OUString T602ImportFilterDialog_getImplementationName()
    throw ( css::uno::RuntimeException );

bool SAL_CALL T602ImportFilterDialog_supportsService( const OUString& ServiceName )
    throw ( css::uno::RuntimeException );

css::uno::Sequence< OUString > SAL_CALL T602ImportFilterDialog_getSupportedServiceNames(  )
    throw ( css::uno::RuntimeException );

css::uno::Reference< css::uno::XInterface >
SAL_CALL T602ImportFilterDialog_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr)
    throw ( css::uno::Exception );

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
