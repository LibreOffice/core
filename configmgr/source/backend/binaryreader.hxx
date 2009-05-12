/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: binaryreader.hxx,v $
 * $Revision: 1.8 $
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
#ifndef CONFIGMGR_BINARYREADER_HXX
#define CONFIGMGR_BINARYREADER_HXX

#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <osl/file.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

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
        class BinaryReader_Impl;
        class BinaryReader
        {
            rtl::OUString m_sFileURL;

            rtl::Reference<BinaryReader_Impl> m_xDataInputStream;
        public:
            explicit BinaryReader (rtl::OUString const & _sFileURL);

            ~BinaryReader();

        public:
            bool open()     SAL_THROW( (io::IOException, uno::RuntimeException) );

            void read(sal_Bool &_nValue)    SAL_THROW( (io::IOException, uno::RuntimeException) );
            void read(sal_Int8 &_nValue)    SAL_THROW( (io::IOException, uno::RuntimeException) );
            void read(sal_Int16 &_nValue)   SAL_THROW( (io::IOException, uno::RuntimeException) );
            void read(sal_Int32 &_nValue)   SAL_THROW( (io::IOException, uno::RuntimeException) );
            void read(sal_Int64 &_nValue)   SAL_THROW( (io::IOException, uno::RuntimeException) );
            void read(double &_nValue)      SAL_THROW( (io::IOException, uno::RuntimeException) );
            void read(rtl::OUString& _aStr) SAL_THROW( (io::IOException, uno::RuntimeException) );
            void read(uno::Sequence< sal_Int8 > &_aValue)      SAL_THROW( (io::IOException, uno::RuntimeException) );
            void read(uno::Sequence< rtl::OUString > &_aValue)  SAL_THROW( (io::IOException, uno::RuntimeException) );

        private:
            inline BinaryReader_Impl * getDataInputStream();
        };
        // --------------------------------------------------------------------------

        bool readSequenceValue (
            BinaryReader      & _rReader,
            uno::Any          & _aValue,
            uno::Type const   & _aElementType) SAL_THROW( (io::IOException, uno::RuntimeException) );

        // --------------------------------------------------------------------------
    }
}
#endif
