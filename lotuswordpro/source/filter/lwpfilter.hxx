/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/*************************************************************************
 * @file
 * Circle object.
 ************************************************************************/
#ifndef _LWPFILTER_HXX
#define _LWPFILTER_HXX

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/factory.hxx>

#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <tools/stream.hxx>

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star;

/**
 * @brief
 * Implements the XFilter interface.
 * This is not the entry for the filter, but a proto of LwpFilterImportFilter.
 */
class LWPFilterReader : public WeakImplHelper1< XFilter >
{
public:
    LWPFilterReader();
    ~LWPFilterReader();

public:
    /**
     * @descr   loading the file. It's call be SfxObejctShell::ImportFrom.
     * @param   aDescriptor the parameters include file URL or XInputStream obejct, from which the filter can
     *          get which file to import.
     */
    virtual sal_Bool SAL_CALL filter( const Sequence< PropertyValue >& aDescriptor )
        throw( RuntimeException );

    /**
     * @descr   stop loading the file.
     */
    virtual void SAL_CALL cancel() throw (com::sun::star::uno::RuntimeException);

    /**
     * @descr   get the XDocumentHandler interface.
     */
    void setDocumentHandler( uno::Reference< XDocumentHandler >& xHandler )
    {
        m_DocumentHandler = xHandler;
    }

private:
    uno::Reference< XDocumentHandler > m_DocumentHandler;
};

/**
 * @brief
 * Main entry for the xml filter framework.
 * It's called by SfxObjectShell::ImportFrom.
 */
class LWPFilterImportFilter : public WeakImplHelper4< XFilter, XImporter, XServiceInfo, XExtendedFilterDetection >
{
public:
    LWPFilterImportFilter( const uno::Reference< XMultiServiceFactory >& xFact );
    ~LWPFilterImportFilter();

public:
    /**
     * @descr   see LWPFilterReader::filter.
     */
    virtual sal_Bool SAL_CALL filter( const Sequence< PropertyValue >& aDescriptor )
        throw( RuntimeException );

    /**
     * @descr   see LWPFilterReader::cancel.
     */
    virtual void SAL_CALL cancel() throw (com::sun::star::uno::RuntimeException);

    // XImporter
    virtual void SAL_CALL setTargetDocument( const uno::Reference< XComponent >& xDoc)
        throw( IllegalArgumentException, RuntimeException );

    // XServiceInfo
    OUString SAL_CALL getImplementationName() throw ();

    Sequence< OUString > SAL_CALL getSupportedServiceNames(void) throw ();

    sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw ();

    /**
     * @descr   function of interface XExtendedFilterDetection. If this interface is registered, it will be called whenever
     *          a file is to be loaded.
     */
    virtual OUString SAL_CALL detect( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& Descriptor ) throw (::com::sun::star::uno::RuntimeException);

public:
    uno::Reference< XFilter > rFilter;
    uno::Reference< XImporter > rImporter;
};

//test code
int ReadWordproFile(SvStream &rStream, uno::Reference<XDocumentHandler>& XDoc);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
