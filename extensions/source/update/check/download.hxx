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

#ifndef INCLUDED_EXTENSIONS_SOURCE_UPDATE_CHECK_DOWNLOAD_HXX
#define INCLUDED_EXTENSIONS_SOURCE_UPDATE_CHECK_DOWNLOAD_HXX

#include <com/sun/star/uno/XComponentContext.hpp>

#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <osl/conditn.hxx>
#include <osl/file.h>
#include <salhelper/simplereferenceobject.hxx>

struct DownloadInteractionHandler : public virtual salhelper::SimpleReferenceObject
{
    virtual bool checkDownloadDestination(const OUString& rFileName) = 0;

    // called if the destination file already exists, but resume is false
    virtual bool downloadTargetExists(const OUString& rFileName) = 0;

    // called when curl reports an error
    virtual void downloadStalled(const OUString& rErrorMessage) = 0;

    // progress handler
    virtual void downloadProgressAt(sal_Int8 nPercent) = 0;

    // called on first progress notification
    virtual void downloadStarted(const OUString& rFileName, sal_Int64 nFileSize) = 0;

    // called when download has been finished
    virtual void downloadFinished(const OUString& rFileName) = 0;

protected:
    virtual ~DownloadInteractionHandler() {}
};


class Download
{
public:
    Download(const css::uno::Reference< css::uno::XComponentContext >& xContext,
             const rtl::Reference< DownloadInteractionHandler >& rHandler) : m_xContext(xContext), m_aHandler(rHandler) {};

    // returns true when the content of rURL was successfully written to rLocalFile
    bool start(const OUString& rURL, const OUString& rFile, const OUString& rDestinationDir);

    // stops the download after the next write operation
    void stop();

protected:

    // Determines the appropriate proxy settings for the given URL. Returns true if a proxy should be used
    void getProxyForURL(const OUString& rURL, OString& rHost, sal_Int32& rPort) const;

private:
    osl::Condition m_aCondition;
    const css::uno::Reference< css::uno::XComponentContext >& m_xContext;
    const rtl::Reference< DownloadInteractionHandler > m_aHandler;
};

#endif // INCLUDED_EXTENSIONS_SOURCE_UPDATE_CHECK_DOWNLOAD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
