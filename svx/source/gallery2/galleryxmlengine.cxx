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

#include <svx/galleryxmlengine.hxx>

#include <tools/urlobj.hxx>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <comphelper/processfactory.hxx>
#include <unotools/ucbstreamhelper.hxx>

#include <tools/XmlWalker.hxx>
#include <tools/XmlWriter.hxx>

#include <sal/log.hxx>

using namespace ::com::sun::star;

/*void GalleryXMLEngine::readZIP()
{
    SAL_WARN("svx", "call to XMLEngine function createZipFile() triggered");
    //OUString aURL = "C:/cygwin/home/adith/lode/dev/core/package/qa/cppunit/data/a2z.zip";
    OUString aURL = "file:///C:/Users/adith/Desktop/zip testing/sendEmails.zip";
    uno::Reference<uno::XComponentContext> xContext = comphelper::getProcessComponentContext();
    uno::Reference<packages::zip::XZipFileAccess2> const xZip(
        packages::zip::ZipFileAccess::createWithURL(xContext, aURL));
    SAL_WARN_IF(!xZip.is(), "svx", "Failed to read zip file");

    uno::Sequence<OUString> elementList = xZip->getElementNames();
    sal_uInt32 len = elementList.getLength();
    len;
    // to read meta.xml
    uno::Reference<io::XInputStream> const xInputStream(xZip->getByName("meta.xml"),
                                                        uno::UNO_QUERY);
    std::unique_ptr<SvStream> const pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    pStream.get();
}

void GalleryXMLEngine::readXML()
{
    //OUString sFilePath = "file:///C:/Users/adith/Desktop/zip testing/test.xml";
    OUString sFilePath = "file:///C:/cygwin/home/adith/lode/dev/core/tools/qa/data/test.xml";
    SvFileStream aFileStream(sFilePath, StreamMode::READ);
    tools::XmlWalker aWalker;
    if (!aWalker.open(&aFileStream))
        return;
    if ((aWalker.name() != "root") || (aWalker.attribute("root-attr") != "Hello World"))
        return;
    OString tmpAttr, tmpAttr2, tmpContent, tmpName;
    aWalker.children();
    while (aWalker.isValid())
    {
        if (aWalker.name() == "text")
        {
            aWalker.next();
            if (aWalker.name() == "child")
            {
                if (aWalker.attribute("attribute") == "123")
                {
                    aWalker.children();
                    if (aWalker.name() == "text")
                    {
                        aWalker.next();
                        if (aWalker.isValid())
                        {
                            //tmpContent = aWalker.content();
                            tmpName = aWalker.name();
                            tmpAttr = aWalker.attribute("attribute1");
                            tmpAttr2 = aWalker.attribute("attribute2");
                            tmpContent = aWalker.content();
                            break;
                        }
                    }
                }
            }
        }
    }
}

void GalleryXMLEngine::writeXML()
{
    OUString rStreamName = "file:///C:/Users/adith/Desktop/zip testing/writexmltest.xml";
    std::unique_ptr<SvStream> pStream;
    pStream.reset(new SvFileStream(rStreamName, StreamMode::STD_READWRITE | StreamMode::TRUNC));
    tools::XmlWriter aXmlWriter(pStream.get());

    if (!aXmlWriter.startDocument())
    {
        SAL_WARN("svx", "failed to start document");
        return;
    }

    aXmlWriter.startElement("theme");
    aXmlWriter.startElement("name");
    aXmlWriter.content(OString("Arrows"));
    aXmlWriter.endElement();
    aXmlWriter.endElement();
    aXmlWriter.endDocument();
}*/

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
