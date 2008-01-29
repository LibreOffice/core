/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $::osl::FileBase::RCSfile: file.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 14:14:04 $
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
 *    ME::osl::FileBase::RCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _FILREC_HXX_
#define _FILREC_HXX_

#include <osl/file.hxx>

namespace fileaccess {

class ReconnectingFile
{
    ::osl::File     m_aFile;

    sal_uInt32      m_nFlags;
    sal_Bool        m_bFlagsSet;

    sal_Bool        m_bDisconnect;

    ReconnectingFile( ReconnectingFile& );

    ReconnectingFile& operator=( ReconnectingFile& );

public:

    ReconnectingFile( const ::rtl::OUString& aFileURL )
        : m_aFile( aFileURL )
        , m_nFlags( 0 )
        , m_bFlagsSet( sal_False )
        , m_bDisconnect( sal_False )
    {}

    ~ReconnectingFile()
    {
        close();
    }

    void disconnect();
    sal_Bool reconnect();

    ::osl::FileBase::RC open( sal_uInt32 uFlags );

    ::osl::FileBase::RC close();

    ::osl::FileBase::RC setPos( sal_uInt32 uHow, sal_Int64 uPos );

    ::osl::FileBase::RC getPos( sal_uInt64& uPos );

    ::osl::FileBase::RC isEndOfFile( sal_Bool *pIsEOF );

    ::osl::FileBase::RC setSize( sal_uInt64 uSize );

    ::osl::FileBase::RC getSize( sal_uInt64 &rSize );

    ::osl::FileBase::RC read( void *pBuffer, sal_uInt64 uBytesRequested, sal_uInt64& rBytesRead );

    ::osl::FileBase::RC write(const void *pBuffer, sal_uInt64 uBytesToWrite, sal_uInt64& rBytesWritten);

    ::osl::FileBase::RC readLine( ::rtl::ByteSequence& aSeq );

    ::osl::FileBase::RC sync() const;
};

} // namespace fileaccess
#endif  // _FILREC_HXX_

