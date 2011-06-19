/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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


// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#include "soaprequest.hxx"
#include "errormail.hxx"
#include "config.hxx"
#include <boost/shared_ptr.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <rtl/strbuf.hxx>
#include <rtl/ustring.hxx>


using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::io;
using boost::shared_ptr;
using com::sun::star::io::XOutputStream;
using com::sun::star::ucb::XSimpleFileAccess;
using rtl::OUString;
using rtl::OString;
using rtl::OStringBuffer;


namespace
{
    static unsigned long asUlong(sal_Int8 input)
    {
        return *reinterpret_cast<unsigned char *>(&input);
    };

    static Sequence<sal_Int8> base64_encode(const Sequence<sal_Int8>& input)
    {
        static const char base64_tab[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        Sequence<sal_Int8> result(4);
        unsigned long value = asUlong(input[0]) << 16;
        if(input.getLength() > 1) value |= asUlong(input[1]) << 8;
        if(input.getLength() > 2) value |= asUlong(input[2]);

        result[0] = static_cast<sal_Int8>(base64_tab[(value >> 18) & 0x3F]);
        result[1] = static_cast<sal_Int8>(base64_tab[(value >> 12) & 0x3F]);
        result[2] = static_cast<sal_Int8>('=');
        result[3] = static_cast<sal_Int8>('=');

        if (input.getLength() > 1)
        {
            result[2] = base64_tab[(value >> 6) & 0x3F];
            if (input.getLength() > 2)
                 result[3] = base64_tab[(value >> 0) & 0x3F];
        }
        return result;
    };

    static OString replaceAll(const OString& str, sal_Char old, const OString& replacement)
    {
        OStringBuffer result;
        sal_Int32 idx = 0;
        do {
            result.append(str.getToken(0, old, idx));
            if(idx>=0) result.append(replacement);
        } while(idx >= 0);
        return result.makeStringAndClear();
    };

    static OString xmlEncode(const OString& input)
    {
        OString result = replaceAll(input, '&', OString("&amp;"));
        result = replaceAll(result, '<', OString("&lt;"));
        return replaceAll(result, '>', OString("&gt;"));
    }

    static shared_ptr<Sequence<sal_Int8> > createSequenceFromString(const OString& str)
    {
        const sal_Int32 size = str.getLength();
        shared_ptr<Sequence<sal_Int8> > result(new Sequence<sal_Int8>(size));
        for(sal_Int32 idx=0; idx < size; idx++)
            (*result)[idx] = str[idx];
        return result;
    };

    static void writeString(const Reference<XOutputStream>& target, const OString& str)
    {
        shared_ptr<Sequence<sal_Int8> > seq = createSequenceFromString(str);
        target->writeBytes(*seq);
    };

    static void writeFile(const Reference<XMultiServiceFactory>& sf, const Reference<XOutputStream>& target, const OUString& fileurl)
    {
        Reference<XSimpleFileAccess> file_access(
            sf->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ucb.SimpleFileAccess"))),
            UNO_QUERY);
        Reference<XInputStream> file = file_access->openFileRead(fileurl);
        const sal_Int32 bufsize = 3;
        sal_Int32 bytes_read;
        Sequence<sal_Int8> buf(bufsize);
        do
        {
            bytes_read = file->readBytes(buf, bufsize);
            if(bytes_read < buf.getLength()) buf.realloc(bytes_read);
            if(bytes_read) target->writeBytes(base64_encode(buf));
        } while(bytes_read == bufsize);
    };

    static const OString SOAP_START(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\"\n"
        "xmlns:SOAP-ENC=\"http://schemas.xmlsoap.org/soap/encoding/\"\n"
        "xmlns:xsi=\"http://www.w3.org/1999/XMLSchema-instance\"\n"
        "xmlns:xsd=\"http://www.w3.org/1999/XMLSchema\"\n"
        "xmlns:rds=\"urn:ReportDataService\"\n"
        "xmlns:apache=\"http://xml.apache.org/xml-soap\"\n"
        "SOAP-ENV:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\n"
        "<SOAP-ENV:Body>\n"
        "<rds:submitReport>\n");
    static const OString SOAP_ITEMS_START("<hash xsi:type=\"apache:Map\">\n");
    static const OString SOAP_ITEMS_END("</hash>\n");
    static const OString SOAP_END(
        "</rds:submitReport>\n"
        "</SOAP-ENV:Body>\n"
        "</SOAP-ENV:Envelope>\n");
    static const OString SOAP_ITEM_END("]]></value></item>\n");

    static const OString getSoapOfficeversion(const Reference<XMultiServiceFactory>& sf)
    {
        return ::rtl::OUStringToOString(oooimprovement::Config(sf).getCompleteProductname(), RTL_TEXTENCODING_ASCII_US);
    };

    static const OString getSoapSoapId(const Reference<XMultiServiceFactory>& sf, const OString& soap_id)
    {
        OStringBuffer buf;
        buf.append("<body xsi:type=\"xsd:string\">");
        buf.append(xmlEncode(soap_id)).append("\n");
        buf.append(xmlEncode(getSoapOfficeversion(sf))).append("\n");
        buf.append("</body>\n");
        return buf.makeStringAndClear();
    };

    static const OString getSoapItemStart(const OString& key)
    {
        OStringBuffer buf =
            "<item>\n"
            "<key xsi:type=\"xsd:string\">" + key + "</key>\n"
            "<value xsi:type=\"xsd:string\"><![CDATA[";
        return buf.makeStringAndClear();
    };
}

namespace oooimprovement
{
    SoapRequest::SoapRequest(const Reference<XMultiServiceFactory>& sf, const OUString& soap_id, const OUString& logfile)
        : m_ServiceFactory(sf)
        , m_SoapId(soap_id)
        , m_Logfile(logfile)
    {}

     void SoapRequest::writeTo(const Reference<XOutputStream>& target) const
    {
        writeString(target, SOAP_START);
            writeString(
                target,
                getSoapSoapId(m_ServiceFactory, rtl::OUStringToOString(m_SoapId, RTL_TEXTENCODING_ASCII_US)));
            writeString(target, SOAP_ITEMS_START);
                writeString(target, getSoapItemStart("reportmail.xml"));
                    writeString(target, Errormail(m_ServiceFactory).getXml());
                writeString(target, SOAP_ITEM_END);
                writeString(target, getSoapItemStart("data.zip"));
                    writeFile(m_ServiceFactory, target, m_Logfile);
                writeString(target, SOAP_ITEM_END);
            writeString(target, SOAP_ITEMS_END);
        writeString(target, SOAP_END);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
