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

#include "logpacker.hxx"
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <rtl/ustrbuf.hxx>


using namespace com::sun::star::embed;
using namespace com::sun::star::io;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using ::com::sun::star::ucb::XSimpleFileAccess;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;


namespace
{
    static const OUString getZipfileurl(const OUString& csvfileurl)
    {
        OUStringBuffer buf(csvfileurl);
        buf.setLength(csvfileurl.getLength()-3);
        buf.appendAscii("zip");
        return buf.makeStringAndClear();
    };

    static sal_Int32 countLines(const Sequence<sal_Int8>& data)
    {
        sal_Int32 result = 0;
        for(sal_Int32 idx = data.getLength()-1; idx>=0; --idx)
            if(data[idx]==0x0a) result++;
        return result;
    };
}

namespace oooimprovement
{
    LogPacker::LogPacker(const Reference<XMultiServiceFactory>& sf)
        : m_ServiceFactory(sf)
    {}

    sal_Int32 LogPacker::pack(const OUString& fileurl)
    {
        Reference<XSimpleFileAccess> file_access(
            m_ServiceFactory->createInstance(OUString::createFromAscii("com.sun.star.ucb.SimpleFileAccess")),
            UNO_QUERY_THROW);

        Reference<XStorage> storage;
        {
            Reference<XSingleServiceFactory> storage_factory(
                m_ServiceFactory->createInstance(OUString::createFromAscii("com.sun.star.embed.StorageFactory")),
                UNO_QUERY_THROW);
            Sequence<Any> storage_init_args(2);
            storage_init_args[0] = Any(getZipfileurl(fileurl));
            storage_init_args[1] = Any(ElementModes::WRITE);
            storage = Reference<XStorage>(
                storage_factory->createInstanceWithArguments(storage_init_args),
                UNO_QUERY_THROW);
        }

        Reference<XOutputStream> zipped_stream = storage->openStreamElement(
            OUString::createFromAscii("logdata.csv"),
            ElementModes::WRITE)->getOutputStream();
        Reference<XInputStream> unzipped_stream = file_access->openFileRead(fileurl);
        const sal_Int32 bufsize = 1024;
        sal_Int32 read_bytes;
        sal_Int32 logged_events = -1; // ignore header row
        Sequence<sal_Int8> buf(bufsize);
        do
        {
            read_bytes = unzipped_stream->readBytes(buf, bufsize);
            buf.realloc(read_bytes);
            logged_events += countLines(buf);
            zipped_stream->writeBytes(buf);
        } while(read_bytes == bufsize);
        unzipped_stream->closeInput();
        zipped_stream->flush();
        zipped_stream->closeOutput();
        Reference<XTransactedObject>(storage, UNO_QUERY_THROW)->commit();
        file_access->kill(fileurl);
        return logged_events;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
