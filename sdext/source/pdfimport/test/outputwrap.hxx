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


#ifndef INCLUDED_PDFIMPORT_OUTPUTWRAP_HXX
#define INCLUDED_PDFIMPORT_OUTPUTWRAP_HXX

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase1.hxx>
#include <com/sun/star/io/XOutputStream.hpp>
#include <osl/file.hxx>

namespace pdfi
{

typedef ::cppu::WeakComponentImplHelper1<
        com::sun::star::io::XOutputStream > OutputWrapBase;

    class OutputWrap : private cppu::BaseMutex, public OutputWrapBase
    {
        osl::File maFile;

    public:

        explicit OutputWrap( const rtl::OUString& rURL ) : OutputWrapBase(m_aMutex), maFile(rURL)
        {
            maFile.open(osl_File_OpenFlag_Create|osl_File_OpenFlag_Write);
        }

        virtual void SAL_CALL writeBytes( const com::sun::star::uno::Sequence< ::sal_Int8 >& aData ) throw (com::sun::star::io::NotConnectedException,com::sun::star::io::BufferSizeExceededException, com::sun::star::io::IOException, com::sun::star::uno::RuntimeException)

        {
            sal_uInt64 nBytesWritten(0);
            maFile.write(aData.getConstArray(),aData.getLength(),nBytesWritten);
        }

        virtual void SAL_CALL flush() throw (com::sun::star::io::NotConnectedException, com::sun::star::io::BufferSizeExceededException, com::sun::star::io::IOException, com::sun::star::uno::RuntimeException)
        {
        }

        virtual void SAL_CALL closeOutput() throw (com::sun::star::io::NotConnectedException, com::sun::star::io::BufferSizeExceededException, com::sun::star::io::IOException, com::sun::star::uno::RuntimeException)
        {
            maFile.close();
        }
    };
}
#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
