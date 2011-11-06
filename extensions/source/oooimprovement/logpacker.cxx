/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
