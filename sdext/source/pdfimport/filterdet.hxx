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
    virtual OUString SAL_CALL detect( com::sun::star::uno::Sequence<
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
   getAdditionalStream( const OUString&                                                            rPDFFile,
                        OUString&                                                                  o_rOutMimetype,
                        OUString&                                                                  io_rOutPwd,
                        const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& xContext,
                        const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >&    rFilterData,
                        bool                                                                            bMayUseUI );


bool checkDocChecksum( const OUString& rInPDFFileURL,
                       sal_uInt32           nBytes,
                       const OUString& rChkSum );

bool getPassword( const com::sun::star::uno::Reference< com::sun::star::task::XInteractionHandler >& xHandler,
                  OUString&                                                                     o_rPwd,
                  bool                                                                               bFirstTry,
                  const OUString&                                                               i_rDocName
                  );

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
