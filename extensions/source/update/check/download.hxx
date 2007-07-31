/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: download.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-07-31 15:56:04 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <osl/conditn.hxx>
#include <osl/file.h>

struct DownloadInteractionHandler : public rtl::IReference
{
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


