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

#ifndef INCLUDED_PDFI_FILTERDET_HXX
#define INCLUDED_PDFI_FILTERDET_HXX

#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>

#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/basemutex.hxx>


namespace pdfi
{

typedef ::cppu::WeakComponentImplHelper1<
    com::sun::star::document::XExtendedFilterDetection > PDFDetectorBase;

class PDFDetector : private cppu::BaseMutex,
                    public PDFDetectorBase
{
private:
    com::sun::star::uno::Reference<
        com::sun::star::uno::XComponentContext > m_xContext;

public:
    explicit PDFDetector( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::uno::XComponentContext >& xContext );

    // XExtendedFilterDetection
    virtual rtl::OUString SAL_CALL detect( com::sun::star::uno::Sequence<
                                               com::sun::star::beans::PropertyValue >& io_rDescriptor )
        throw (com::sun::star::uno::RuntimeException);
};


/** Retrieve embedded substream from PDF file

    Useful e.g. for hybrid PDF

    @param rPDFFile
    URI of the pdf file

    @param o_rOutMimetype
    Output parameter. Receives the mime type of the
    substream. Used to distinguish between
    draw/impress/writer/calc during import

    @param o_rOutPwd
    In/Out parameter. If given password is wrong, user is queried
    for another (if bMayUseUI is true)

    @param xContext
    Component context

    @param rFilterData
    Basically used to pass on XFilter::filter properties (function
    uses it to retrieve interaction handler)

    @param bMayUseUI
    When false, no dialog is opened to query user for alternate
    password
 */
com::sun::star::uno::Reference< com::sun::star::io::XStream >
   getAdditionalStream( const rtl::OUString&                                                            rPDFFile,
                        rtl::OUString&                                                                  o_rOutMimetype,
                        rtl::OUString&                                                                  io_rOutPwd,
                        const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& xContext,
                        const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >&    rFilterData,
                        bool                                                                            bMayUseUI );


bool checkDocChecksum( const rtl::OUString& rInPDFFileURL,
                       sal_uInt32           nBytes,
                       const rtl::OUString& rChkSum );

bool getPassword( const com::sun::star::uno::Reference< com::sun::star::task::XInteractionHandler >& xHandler,
                  rtl::OUString&                                                                     o_rPwd,
                  bool                                                                               bFirstTry,
                  const rtl::OUString&                                                               i_rDocName
                  );

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
