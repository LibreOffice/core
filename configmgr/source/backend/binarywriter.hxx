/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: binarywriter.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-10-19 12:16:02 $
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
#ifndef CONFIGMGR_BINARYWRITER_HXX
#define CONFIGMGR_BINARYWRITER_HXX

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
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
#ifndef _COM_SUN_STAR_IO_XDATAOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XDataOutputStream.hpp>
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

        class BinaryWriter
        {
            rtl::OUString       m_aFileURL;

            uno::Reference<lang::XMultiServiceFactory> m_xServiceProvider;
            uno::Reference< io::XDataOutputStream > m_xDataOutputStream;

        public:
            BinaryWriter(rtl::OUString const& m_aFilename, MultiServiceFactory const& _rxServiceProvider);

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
