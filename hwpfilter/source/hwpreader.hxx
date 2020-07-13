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
#include <com/sun/star/uno/XComponentContext.hpp>

#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XActiveDataControl.hpp>
#include <com/sun/star/io/XStreamListener.hpp>
#include <com/sun/star/document/XExtendedFilterDetection.hpp>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include <memory>

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

#define WRITER_IMPORTER_NAME "com.sun.star.comp.Writer.XMLImporter"

struct HwpReaderPrivate;
/**
 * This class implements the external Parser interface
 */
class HwpReader : public WeakImplHelper<XFilter>
{

public:
    HwpReader();
    virtual ~HwpReader() override;

public:
    /**
     * parseStream does Parser-startup initializations
     */
    virtual sal_Bool SAL_CALL filter(const Sequence< PropertyValue >& aDescriptor) override;
    virtual void SAL_CALL cancel() override {}
    void setDocumentHandler(Reference< XDocumentHandler > const & xHandler)
    {
        m_rxDocumentHandler = xHandler;
    }
private:
    Reference< XDocumentHandler > m_rxDocumentHandler;
    rtl::Reference<AttributeListImpl> mxList;
    HWPFile hwpfile;
    std::unique_ptr<HwpReaderPrivate> d;
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
    void make_text_p0(HWPPara *para, bool bParaStart);
    void make_text_p1(HWPPara *para, bool bParaStart);
    void make_text_p3(HWPPara *para, bool bParaStart);

    /* -------- rDocument->characters(x) --------- */
    void makeChars(hchar_string & rStr);

    /* -------- Special Char Parsing --------- */
    void makeFieldCode(hchar_string const & rStr, FieldCode const *hbox); //6
    void makeBookmark(Bookmark const *hbox);      //6
    void makeDateFormat(DateCode *hbox);    //7
    void makeDateCode(DateCode *hbox);      //8
    void makeTab();            //9
    void makeTable(TxtBox *hbox);
    void makeTextBox(TxtBox *hbox);
    void makeFormula(TxtBox *hbox);
    void makeHyperText(TxtBox *hbox);
    void makePicture(Picture *hbox);
    void makePictureDRAW(HWPDrawingObject *drawobj, Picture *hbox);
    void makeLine();
    void makeHidden(Hidden *hbox);
    void makeFootnote(Footnote *hbox);
    void makeAutoNum(AutoNum const *hbox);
    void makeShowPageNum();
    void makeMailMerge(MailMerge *hbox);
    void makeOutline(Outline const *hbox);

    /* --------- Styles Parsing ------------ */
    void makePageStyle();
    void makeColumns(ColumnDef const *);
    void makeTStyle(CharShape const *);
    void makePStyle(ParaShape const *);
    void makeFStyle(FBoxStyle *);
    void makeCaptionStyle(FBoxStyle *);
    void makeDrawStyle(HWPDrawingObject *,FBoxStyle *);
    void makeTableStyle(Table *);
    void parseCharShape(CharShape const *);
    void parseParaShape(ParaShape const *);
    static char* getTStyleName(int, char *);
    static char* getPStyleName(int, char *);
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
