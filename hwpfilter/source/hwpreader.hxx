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

#ifndef INCLUDED_HWPFILTER_SOURCE_HWPREADER_HXX
#define INCLUDED_HWPFILTER_SOURCE_HWPREADER_HXX
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sal/alloca.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>

#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XActiveDataControl.hpp>
#include <com/sun/star/io/XStreamListener.hpp>
#include <com/sun/star/document/XExtendedFilterDetection.hpp>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>

using namespace ::cppu;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::xml::sax;

#include <assert.h>

#include <unotools/mediadescriptor.hxx>

#include "hwpfile.h"
#include "hcode.h"
#include "hbox.h"
#include "htags.h"
#include "hstream.hxx"
#include "drawdef.h"
#include "attributes.hxx"

#define IMPLEMENTATION_NAME "com.sun.comp.hwpimport.HwpImportFilter"
#define SERVICE_NAME1 "com.sun.star.document.ImportFilter"
#define SERVICE_NAME2 "com.sun.star.document.ExtendedTypeDetection"
#define WRITER_IMPORTER_NAME "com.sun.star.comp.Writer.XMLImporter"

struct HwpReaderPrivate;
/**
 * This class implements the external Parser interface
 */
class HwpReader : public WeakImplHelper<XFilter>
{

public:
    HwpReader();
    virtual ~HwpReader();

public:
    /**
     * parseStream does Parser-startup initializations
     */
    virtual sal_Bool SAL_CALL filter(const Sequence< PropertyValue >& aDescriptor) throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL cancel() throw(RuntimeException, std::exception) override {}
    void SAL_CALL setDocumentHandler(Reference< XDocumentHandler > xHandler)
    {
        m_rxDocumentHandler = xHandler;
    }
private:
    Reference< XDocumentHandler > m_rxDocumentHandler;
    Reference< XAttributeList > rList;
    AttributeListImpl *pList;
    HWPFile hwpfile;
    HwpReaderPrivate *d;
private:
    /* -------- Document Parsing --------- */
    void makeMeta();
    void makeStyles();
    void makeDrawMiscStyle(HWPDrawingObject *);
    void makeAutoStyles();
    void makeMasterStyles();
    void makeBody();

    void makeTextDecls();

    /* -------- Paragraph Parsing --------- */
    void parsePara(HWPPara *para);
    void make_text_p0(HWPPara *para, bool bParaStart = false);
    void make_text_p1(HWPPara *para, bool bParaStart = false);
    void make_text_p3(HWPPara *para, bool bParaStart = false);

    /* -------- rDocument->characters(x) --------- */
    void makeChars(hchar_string & rStr);

    /* -------- Special Char Parsing --------- */
    void makeFieldCode(hchar_string & rStr, FieldCode *hbox); //6
    void makeBookmark(Bookmark *hbox);      //6
    void makeDateFormat(DateCode *hbox);    //7
    void makeDateCode(DateCode *hbox);      //8
    void makeTab(Tab *hbox);            //9
    void makeTable(TxtBox *hbox);
    void makeTextBox(TxtBox *hbox);
    void makeFormula(TxtBox *hbox);
    void makeHyperText(TxtBox *hbox);
    void makePicture(Picture *hbox);
    void makePictureDRAW(HWPDrawingObject *drawobj, Picture *hbox);
    void makeLine(Line *hbox);
    void makeHidden(Hidden *hbox);
    void makeFootnote(Footnote *hbox);
    void makeAutoNum(AutoNum *hbox);
    void makeShowPageNum();
    void makeMailMerge(MailMerge *hbox);
    void makeOutline(Outline *hbox);

    /* --------- Styles Parsing ------------ */
    void makePageStyle();
    void makeColumns(ColumnDef *);
    void makeTStyle(CharShape *);
    void makePStyle(ParaShape *);
    void makeFStyle(FBoxStyle *);
    void makeCaptionStyle(FBoxStyle *);
    void makeDrawStyle(HWPDrawingObject *,FBoxStyle *);
    void makeTableStyle(Table *);
    void parseCharShape(CharShape *);
    void parseParaShape(ParaShape *);
    static char* getTStyleName(int, char *);
    static char* getPStyleName(int, char *);
};

class HwpImportFilter : public WeakImplHelper< XFilter, XImporter, XServiceInfo, XExtendedFilterDetection >
{
public:
    explicit HwpImportFilter(const Reference< XMultiServiceFactory >& rFact);
    virtual ~HwpImportFilter();

public:
    static Sequence< OUString > getSupportedServiceNames_Static() throw();
    static OUString getImplementationName_Static() throw();

public:
    // XFilter
    virtual sal_Bool SAL_CALL filter( const Sequence< PropertyValue >& aDescriptor )
        throw( RuntimeException, std::exception ) override;
    virtual void SAL_CALL cancel() throw(RuntimeException, std::exception) override;

    // XImporter
    virtual void SAL_CALL setTargetDocument( const Reference< XComponent >& xDoc)
        throw( IllegalArgumentException, RuntimeException, std::exception ) override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName() throw (RuntimeException, std::exception) override;
    Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (css::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw (css::uno::RuntimeException, std::exception) override;

    //XExtendedFilterDetection
    virtual OUString SAL_CALL detect( css::uno::Sequence< css::beans::PropertyValue >& rDescriptor ) throw (css::uno::RuntimeException, std::exception) override;

public:
    Reference< XFilter > rFilter;
    Reference< XImporter > rImporter;
};

Reference< XInterface > HwpImportFilter_CreateInstance(
    const Reference< XMultiServiceFactory >& rSMgr ) throw( Exception )
{
    HwpImportFilter *p = new HwpImportFilter( rSMgr );

    return Reference< XInterface > ( static_cast<OWeakObject*>(p) );
}

Sequence< OUString > HwpImportFilter::getSupportedServiceNames_Static() throw ()
{
    Sequence< OUString > aRet { HwpImportFilter::getImplementationName_Static() };
    return aRet;
}

HwpImportFilter::HwpImportFilter(const Reference< XMultiServiceFactory >& rFact)
{
    OUString sService( WRITER_IMPORTER_NAME );
    try {
        Reference< XDocumentHandler > xHandler( rFact->createInstance( sService ), UNO_QUERY );

        HwpReader *p = new HwpReader;
        p->setDocumentHandler( xHandler );

        Reference< XImporter > xImporter( xHandler, UNO_QUERY );
        rImporter = xImporter;
        Reference< XFilter > xFilter( p );
        rFilter = xFilter;
    }
    catch( Exception & )
    {
        printf(" fail to instanciate %s\n", WRITER_IMPORTER_NAME );
        exit( 1 );
    }
}

HwpImportFilter::~HwpImportFilter()
{
}

sal_Bool HwpImportFilter::filter( const Sequence< PropertyValue >& aDescriptor )
    throw( RuntimeException, std::exception )
{
    // delegate to IchitaroImpoter
    return rFilter->filter( aDescriptor );
}

void HwpImportFilter::cancel() throw(css::uno::RuntimeException, std::exception)
{
    rFilter->cancel();
}

void HwpImportFilter::setTargetDocument( const Reference< XComponent >& xDoc )
    throw( IllegalArgumentException, RuntimeException, std::exception )
{
        // delegate
    rImporter->setTargetDocument( xDoc );
}

OUString HwpImportFilter::getImplementationName_Static() throw()
{
    return OUString( IMPLEMENTATION_NAME );
}

OUString HwpImportFilter::getImplementationName() throw(css::uno::RuntimeException, std::exception)
{
    return OUString( IMPLEMENTATION_NAME );
}

sal_Bool HwpImportFilter::supportsService( const OUString& ServiceName ) throw(css::uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

//XExtendedFilterDetection
OUString HwpImportFilter::detect( css::uno::Sequence< css::beans::PropertyValue >& rDescriptor ) throw (css::uno::RuntimeException, std::exception)
{
    OUString sTypeName;

    utl::MediaDescriptor aDescriptor(rDescriptor);
    aDescriptor.addInputStream();

    Reference< XInputStream > xInputStream(
        aDescriptor[utl::MediaDescriptor::PROP_INPUTSTREAM()], UNO_QUERY);

    if (xInputStream.is())
    {
        Sequence< sal_Int8 > aData;
        sal_Int32 nLen = HWPIDLen;
        if (
             nLen == xInputStream->readBytes(aData, nLen) &&
             detect_hwp_version(reinterpret_cast<const char*>(aData.getConstArray()))
           )
        {
            sTypeName = "writer_MIZI_Hwp_97";
        }
    }

    return sTypeName;
}

Sequence< OUString> HwpImportFilter::getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception)
{
    Sequence < OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = SERVICE_NAME1;
    pArray[1] = SERVICE_NAME2;
    return aRet;
}

extern "C"
{
    SAL_DLLPUBLIC_EXPORT void * SAL_CALL hwp_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void *  )
    {
        void * pRet = nullptr;

        if (pServiceManager )
        {
            Reference< XSingleServiceFactory > xRet;
            Reference< XMultiServiceFactory > xSMgr = static_cast< XMultiServiceFactory * > ( pServiceManager );

            OUString aImplementationName = OUString::createFromAscii( pImplName );

            if (aImplementationName == IMPLEMENTATION_NAME )
            {
                xRet = createSingleFactory( xSMgr, aImplementationName,
                                            HwpImportFilter_CreateInstance,
                                            HwpImportFilter::getSupportedServiceNames_Static() );
            }
            if (xRet.is())
            {
                xRet->acquire();
                pRet = xRet.get();
            }
        }

        return pRet;
    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
