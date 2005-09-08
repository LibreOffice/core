/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: binaryreader.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:26:23 $
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
#ifndef CONFIGMGR_BINARYREADER_HXX
#define CONFIGMGR_BINARYREADER_HXX

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _COM_SUN_STAR_IO_IOEXCEPTION_HPP_
#include <com/sun/star/io/IOException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XDATAINPUTSTREAM_HPP_
#include <com/sun/star/io/XDataInputStream.hpp>
#endif

namespace configmgr
{
    // -----------------------------------------------------------------------------
    namespace backend
    {
        namespace css = com::sun::star;

        namespace io   = css::io;
        namespace uno  = css::uno;
        namespace lang = css::lang;

        typedef uno::Reference<lang::XMultiServiceFactory> MultiServiceFactory;
        // -----------------------------------------------------------------------------
        class BinaryReader
        {
            rtl::OUString m_sFileURL;

            uno::Reference<io::XDataInputStream> m_xDataInputStream;
        public:
            explicit BinaryReader (rtl::OUString const & _sFileURL)
            : m_sFileURL(_sFileURL)
            {}

            ~BinaryReader()
            {}

        public:
            bool open()     SAL_THROW( (io::IOException, uno::RuntimeException) );
            void reopen()   SAL_THROW( (io::IOException, uno::RuntimeException) );
            void close()    SAL_THROW( (io::IOException, uno::RuntimeException) );

            typedef uno::Sequence< sal_Int8 >       Binary;
            typedef uno::Sequence< rtl::OUString >  StringList;

            void read(sal_Bool &_nValue)    SAL_THROW( (io::IOException, uno::RuntimeException) );
            void read(sal_Int8 &_nValue)    SAL_THROW( (io::IOException, uno::RuntimeException) );
            void read(sal_Int16 &_nValue)   SAL_THROW( (io::IOException, uno::RuntimeException) );
            void read(sal_Int32 &_nValue)   SAL_THROW( (io::IOException, uno::RuntimeException) );
            void read(sal_Int64 &_nValue)   SAL_THROW( (io::IOException, uno::RuntimeException) );
            void read(double &_nValue)      SAL_THROW( (io::IOException, uno::RuntimeException) );
            void read(rtl::OUString& _aStr) SAL_THROW( (io::IOException, uno::RuntimeException) );
            void read(Binary &_aValue)      SAL_THROW( (io::IOException, uno::RuntimeException) );
            void read(StringList &_aValue)  SAL_THROW( (io::IOException, uno::RuntimeException) );

        private:
            inline uno::Reference<io::XDataInputStream> getDataInputStream();
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
