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

    static const OUString CSSU_PATHSUB(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.util.PathSubstitution"));

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
        static const OUString file_extension(RTL_CONSTASCII_USTRINGPARAM(".zip"));
        return fileurl.match(file_extension, fileurl.getLength()-file_extension.getLength());
    };

    static bool isLogfile(const OUString& fileurl)
    {
        static const OUString file_extension(RTL_CONSTASCII_USTRINGPARAM(".csv"));
        static const OUString current(RTL_CONSTASCII_USTRINGPARAM("Current.csv"));
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
            sf->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ucb.SimpleFileAccess"))),
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
            sf->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ucb.SimpleFileAccess"))),
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
            m_ServiceFactory->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ucb.SimpleFileAccess"))),
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
