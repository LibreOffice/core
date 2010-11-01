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
            maFile.open(osl_File_OpenFlag_Create|OpenFlag_Write);
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
