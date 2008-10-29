/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: binarywriter.hxx,v $
 * $Revision: 1.7 $
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
#ifndef CONFIGMGR_BINARYWRITER_HXX
#define CONFIGMGR_BINARYWRITER_HXX

#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/io/XDataOutputStream.hpp>


namespace configmgr
{
    // -----------------------------------------------------------------------------
    namespace backend
    {
        namespace css = com::sun::star;

        namespace io   = css::io;
        namespace uno  = css::uno;
        namespace lang = css::lang;
        // -----------------------------------------------------------------------------

        class BinaryWriter
        {
            rtl::OUString       m_aFileURL;

            uno::Reference<lang::XMultiServiceFactory> m_xServiceProvider;
            uno::Reference< io::XDataOutputStream > m_xDataOutputStream;

        public:
            BinaryWriter(rtl::OUString const& m_aFilename, uno::Reference<lang::XMultiServiceFactory> const& _rxServiceProvider);

            ~BinaryWriter();
            bool open()  SAL_THROW( (io::IOException, uno::RuntimeException) );
            void close() SAL_THROW( (io::IOException, uno::RuntimeException) );

            // Type writer
            void write(sal_Bool _bValue)    SAL_THROW( (io::IOException, uno::RuntimeException) );
            void write(sal_Int8 _nValue)    SAL_THROW( (io::IOException, uno::RuntimeException) );
            void write(sal_Int16 _nValue)   SAL_THROW( (io::IOException, uno::RuntimeException) );
            void write(sal_Int32 _nValue)   SAL_THROW( (io::IOException, uno::RuntimeException) );
            void write(sal_Int64 _nValue)   SAL_THROW( (io::IOException, uno::RuntimeException) );
            void write(double _nValue)      SAL_THROW( (io::IOException, uno::RuntimeException) );
            void write(rtl::OUString const& _aStr) SAL_THROW( (io::IOException, uno::RuntimeException) );

        };
    } //namespace backend
} // namespace configmgr

#endif
