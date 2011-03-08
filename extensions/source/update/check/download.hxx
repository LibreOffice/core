/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
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


#include <com/sun/star/uno/XComponentContext.hpp>

#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <osl/conditn.hxx>
#include <osl/file.h>

struct DownloadInteractionHandler : public rtl::IReference
{
    virtual bool checkDownloadDestination(const rtl::OUString& rFileName) = 0;

    // called if the destination file already exists, but resume is false
    virtual bool downloadTargetExists(const rtl::OUString& rFileName) = 0;

    // called when curl reports an error
    virtual void downloadStalled(const rtl::OUString& rErrorMessage) = 0;

    // progress handler
    virtual void downloadProgressAt(sal_Int8 nPercent) = 0;

    // called on first progress notification
    virtual void downloadStarted(const rtl::OUString& rFileName, sal_Int64 nFileSize) = 0;

    // called when download has been finished
    virtual void downloadFinished(const rtl::OUString& rFileName) = 0;
};


class Download
{
public:
    Download(const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& xContext,
             const rtl::Reference< DownloadInteractionHandler >& rHandler) : m_xContext(xContext), m_aHandler(rHandler) {};

    // returns true when the content of rURL was successfully written to rLocalFile
    bool start(const rtl::OUString& rURL, const rtl::OUString& rFile, const rtl::OUString& rDestinationDir);

    // stops the download after the next write operation
    void stop();

    // returns true if the stop condition is set
    bool isStopped() const
        { return sal_True == const_cast <Download *> (this)->m_aCondition.check(); };

protected:

    // Determines the appropriate proxy settings for the given URL. Returns true if a proxy should be used
    void getProxyForURL(const rtl::OUString& rURL, rtl::OString& rHost, sal_Int32& rPort) const;

private:
    osl::Condition m_aCondition;
    const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& m_xContext;
    const rtl::Reference< DownloadInteractionHandler > m_aHandler;
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
