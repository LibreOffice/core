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


#ifndef INCLUDED_SDEXT_SOURCE_PDFIMPORT_TEST_OUTPUTWRAP_HXX
#define INCLUDED_SDEXT_SOURCE_PDFIMPORT_TEST_OUTPUTWRAP_HXX

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <com/sun/star/io/XOutputStream.hpp>
#include <osl/file.hxx>

namespace pdfi
{

typedef ::cppu::WeakComponentImplHelper<
        css::io::XOutputStream > OutputWrapBase;

    class OutputWrap : private cppu::BaseMutex, public OutputWrapBase
    {
        osl::File maFile;

    public:

        explicit OutputWrap( const OUString& rURL ) : OutputWrapBase(m_aMutex), maFile(rURL)
        {
            maFile.open(osl_File_OpenFlag_Create|osl_File_OpenFlag_Write);
        }

        virtual void SAL_CALL writeBytes( const css::uno::Sequence< ::sal_Int8 >& aData ) throw (css::io::NotConnectedException,css::io::BufferSizeExceededException, css::io::IOException, css::uno::RuntimeException, std::exception) override

        {
            sal_uInt64 nBytesWritten(0);
            maFile.write(aData.getConstArray(),aData.getLength(),nBytesWritten);
        }

        virtual void SAL_CALL flush() throw (css::io::NotConnectedException, css::io::BufferSizeExceededException, css::io::IOException, css::uno::RuntimeException, std::exception) override
        {
        }

        virtual void SAL_CALL closeOutput() throw (css::io::NotConnectedException, css::io::BufferSizeExceededException, css::io::IOException, css::uno::RuntimeException, std::exception) override
        {
            maFile.close();
        }
    };
}
#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
