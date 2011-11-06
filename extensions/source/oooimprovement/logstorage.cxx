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

#include "logstorage.hxx"
#include "config.hxx"
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/util/XStringSubstitution.hpp>


using namespace com::sun::star::io;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::util;
using ::com::sun::star::ucb::XSimpleFileAccess;
using ::rtl::OUString;
using ::std::vector;


namespace
{
    using namespace oooimprovement;

    static const OUString CSSU_PATHSUB = OUString::createFromAscii("com.sun.star.util.PathSubstitution");

    static OUString getLogPathFromCfg(const Reference<XMultiServiceFactory>& sf)
    {
        Config config(sf);
        OUString result=config.getLogPath();
        Reference<XStringSubstitution> path_sub(
            sf->createInstance(CSSU_PATHSUB),
            UNO_QUERY);
        if(path_sub.is())
            result = path_sub->substituteVariables(result, sal_False);
        return result;
    }

    static bool isZipfile(const OUString& fileurl)
    {
        static const OUString file_extension = OUString::createFromAscii(".zip");
        return fileurl.match(file_extension, fileurl.getLength()-file_extension.getLength());
    };

    static bool isLogfile(const OUString& fileurl)
    {
        static const OUString file_extension = OUString::createFromAscii(".csv");
        static const OUString current = OUString::createFromAscii("Current.csv");
        return
            fileurl.match(file_extension, fileurl.getLength()-file_extension.getLength())
            && !fileurl.match(current, fileurl.getLength()-current.getLength());
    };

    static bool isZipOrLogFile(const OUString& fileurl)
    {
        return isZipfile(fileurl) || isLogfile(fileurl);
    }

    static Sequence<OUString> getAllLogStoragefiles(const Reference<XMultiServiceFactory>& sf)
    {
        Reference<XSimpleFileAccess> file_access(
            sf->createInstance(OUString::createFromAscii("com.sun.star.ucb.SimpleFileAccess")),
            UNO_QUERY_THROW);
        return file_access->getFolderContents(
            getLogPathFromCfg(sf),
            false);
    };

    static vector<OUString> getLogStoragefiles(
        const Reference<XMultiServiceFactory>& sf,
        bool (*condition)(const OUString& path))
    {
        Sequence<OUString> candidates = getAllLogStoragefiles(sf);
        vector<OUString> result;
        result.reserve(candidates.getLength());
        for(sal_Int32 idx=0; idx<candidates.getLength(); ++idx)
            if(condition(candidates[idx]))
                result.push_back(candidates[idx]);
        return result;
    };

    static void assureLogStorageExists(const Reference<XMultiServiceFactory>& sf)
    {
        Reference<XSimpleFileAccess> file_access(
            sf->createInstance(OUString::createFromAscii("com.sun.star.ucb.SimpleFileAccess")),
            UNO_QUERY_THROW);
        OUString log_path(getLogPathFromCfg(sf));
        if(!file_access->isFolder(log_path))
            file_access->createFolder(log_path);
    };
}

namespace oooimprovement
{

    LogStorage::LogStorage(const Reference<XMultiServiceFactory>& sf)
        : m_ServiceFactory(sf)
    {}

    void LogStorage::assureExists()
    {
        assureLogStorageExists(m_ServiceFactory);
    }

    void LogStorage::clear()
    {
        Reference<XSimpleFileAccess> file_access(
            m_ServiceFactory->createInstance(OUString::createFromAscii("com.sun.star.ucb.SimpleFileAccess")),
            UNO_QUERY_THROW);
        vector<OUString> files_to_kill = getLogStoragefiles(m_ServiceFactory, &isZipOrLogFile);
        for(vector<OUString>::iterator item = files_to_kill.begin();
            item != files_to_kill.end();
            item++)
            file_access->kill(*item);
    }

    const vector<OUString> LogStorage::getUnzippedLogFiles() const
    { return getLogStoragefiles(m_ServiceFactory, &isLogfile); }

    const vector<OUString> LogStorage::getZippedLogFiles() const
    { return getLogStoragefiles(m_ServiceFactory, &isZipfile); }
}
