/*************************************************************************
 *
 *  $RCSfile: ftpcontentprovider.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: abi $ $Date: 2002-08-28 07:23:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#ifndef _FTP_FTPCONTENTPROVIDER_HXX_
#include "ftpcontentprovider.hxx"
#endif
#ifndef _FTP_FTPCONTENT_HXX_
#include "ftpcontent.hxx"
#endif
#ifndef _FTP_FTPLOADERTHREAD_HXX_
#include "ftploaderthread.hxx"
#endif


using namespace ftp;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;



//=========================================================================
//=========================================================================
//
// ContentProvider Implementation.
//
//=========================================================================
//=========================================================================

FTPContentProvider::FTPContentProvider(const Reference< XMultiServiceFactory >& rSMgr)
    : ::ucb::ContentProviderImplHelper(rSMgr),
             m_ftpLoaderThread(NULL)
{
}

//=========================================================================
// virtual
FTPContentProvider::~FTPContentProvider()
{
    delete m_ftpLoaderThread;
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

XINTERFACE_IMPL_3( FTPContentProvider,
                   XTypeProvider,
                   XServiceInfo,
                   XContentProvider);

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_IMPL_3( FTPContentProvider,
                         XTypeProvider,
                         XServiceInfo,
                         XContentProvider);

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

XSERVICEINFO_IMPL_1(FTPContentProvider,
                    rtl::OUString::createFromAscii("com.sun.star.comp.FTPContentProvider"),
                    rtl::OUString::createFromAscii(MYUCP_CONTENT_PROVIDER_SERVICE_NAME));

//=========================================================================
//
// Service factory implementation.
//
//=========================================================================

ONE_INSTANCE_SERVICE_FACTORY_IMPL(FTPContentProvider);


//=========================================================================
//
// XContentProvider methods.
//
//=========================================================================

// virtual
Reference<XContent> SAL_CALL
FTPContentProvider::queryContent(
    const Reference< XContentIdentifier >& xCanonicId
)
    throw(
        IllegalIdentifierException,
        RuntimeException
    )
{
    // Check, if a content with given id already exists...
    Reference<XContent> xContent
        = queryExistingContent(xCanonicId).getBodyPtr();
    if(xContent.is())
        return xContent;

    // A new content has to be returned:
    {
        // Initialize
        osl::MutexGuard aGuard( m_aMutex );
        if(!m_ftpLoaderThread)
        {
            init();
            if(!m_ftpLoaderThread)
                throw RuntimeException();
        }
    }

    xContent = new FTPContent(m_xSMgr,this,xCanonicId);
    // may throw IllegalIdentifierException
    return xContent;
}


void FTPContentProvider::init() {
    m_ftpLoaderThread = new FTPLoaderThread();
}


CURL* FTPContentProvider::handle() {
    // Cannot be zero if called from here;
    return m_ftpLoaderThread->handle();
}
