/*************************************************************************
 *
 *  $RCSfile: bufferedfile.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-10-19 14:44:46 $
 *
 *  last change: $Author: rt $ $Date: 2005-10-19 14:44:46 $
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

#ifndef CONFIGMGR_BUFFEREDFILE_HXX
#define CONFIGMGR_BUFFEREDFILE_HXX

#include "utility.hxx"

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

#ifndef _COM_SUN_STAR_IO_IOEXCEPTION_HPP_
#include <com/sun/star/io/IOException.hpp>
#endif

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

namespace configmgr
{

    namespace io = com::sun::star::io;

    class BufferedInputFile : public osl::FileBase, Noncopyable
    {
        rtl::OUString m_aFileURL;
        sal_Int8  *m_pBuffer;
        sal_uInt64 m_nPointer;
        sal_uInt64 m_nSize;

    public:
        BufferedInputFile( rtl::OUString const& aFileURL );
        ~BufferedInputFile ();

        RC open( sal_uInt32 uFlags );
        RC close();
        RC setPos( sal_uInt32 uHow, sal_uInt64 uPos );

        RC getPos( sal_uInt64& uPos );
        RC read( void *pBuffer, sal_uInt64 uBytesRequested, sal_uInt64& rBytesRead );

        // extra api for buffered file
        RC available( sal_uInt64& nAvail) const;
    };

    class BufferedOutputFile : public osl::FileBase, Noncopyable
    {
        typedef std::vector<sal_uInt8> Buffer;

        osl::File * m_pFile;
        Buffer      m_buffer;
    public:
        BufferedOutputFile( rtl::OUString const& aFileURL, sal_uInt32 nBufferSizeHint = 0 );
        ~BufferedOutputFile ();

        RC open( sal_uInt32 uFlags );
        RC close();

        //RC getPos( sal_uInt64& uPos )
        RC write(const void *pBuffer, sal_uInt64 uBytesToWrite, sal_uInt64& rBytesWritten);

        // as opposed to osl::File, this method is not const here
        RC sync();
    };
} // namespace

#endif
