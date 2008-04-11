/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: IndexAccessor.cxx,v $
 * $Revision: 1.10 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmlhelp.hxx"
#include <osl/diagnose.h>
#include <osl/file.hxx>
#include <util/IndexAccessor.hxx>
#include <util/RandomAccessStream.hxx>


using namespace xmlsearch::excep;
using namespace xmlsearch::util;


class RandomAccessStreamImpl
    : public RandomAccessStream
{
public:

    RandomAccessStreamImpl( const rtl::OUString& aPath,const rtl::OUString& how );

    ~RandomAccessStreamImpl()
    {
        close();
    }

    virtual void seek( sal_Int32 pos )
    {
        file_.setPos( Pos_Absolut,sal_uInt64( pos ) );
    }

    virtual sal_Int32 readBytes( sal_Int8* data,sal_Int32 num )
    {
        sal_uInt64 nbytesread;

#ifdef DBG_UTIL
        osl::FileBase::RC err =
            file_.read( (void*)(data), sal_uInt64(num),nbytesread );

        OSL_ENSURE( err == osl::FileBase::E_None, "RandomAccessStreamImpl::readBytes: -> file not open" );
#else
        file_.read( (void*)(data), sal_uInt64(num),nbytesread );
#endif

        return sal_Int32( nbytesread );
    }

    virtual void writeBytes( sal_Int8*, sal_Int32 )
    {
        // Do nothing by now
    }

    virtual sal_Int32 length()
    {
        osl::DirectoryItem aItem;
        osl::FileStatus aStatus( FileStatusMask_FileSize );
        osl::DirectoryItem::get( path_,aItem );
        aItem.getFileStatus( aStatus );
        return sal_Int32( aStatus.getFileSize() );
    }

    virtual void close()
    {
        file_.close();
    }

    bool isOpen()
    {
        return isOpen_;
    }

private:

    rtl::OUString   path_;
    osl::File       file_;
    bool            isOpen_;
};


RandomAccessStreamImpl::RandomAccessStreamImpl( const rtl::OUString& aPath,const rtl::OUString& how )
    : path_( aPath ),
      file_( aPath )
{
    sal_uInt32 flags = 0;
    const sal_Unicode* bla = how.getStr();

    for( int i = 0; i < how.getLength(); ++i )
    {
        if( bla[i] == sal_Unicode( 'r' ) )
            flags |= Read;
        else if( bla[i] == sal_Unicode( 'w' ) )
            flags |= Write;
        else if( bla[i] == sal_Unicode( 'c' ) )
            flags |= Create;
    }
    isOpen_ = ( file_.open( flags ) == osl::FileBase::E_None );
    if( !isOpen_ )
    {
        file_.close();
        OSL_ENSURE( false,"RandomAccessStreamImpl::RandomAccessStreamImpl -> could not open file" );
    }
}



RandomAccessStream* IndexAccessor::getStream( const rtl::OUString& fileName,const rtl::OUString& how ) const
{
    rtl::OUString qualifiedName;
    int retry = 2;
    RandomAccessStreamImpl *p = 0;

    while( retry-- && ! p )
    {
        if( retry == 1 )
            qualifiedName = dirName_ + fileName;
        else if( retry == 0 )
            qualifiedName = dirName_ + fileName.toAsciiLowerCase();

        p = new RandomAccessStreamImpl( qualifiedName,how );
        if( ! p->isOpen() )
        {
            delete p; p = 0;
        }
    }

    return p;
}


sal_Int32 IndexAccessor::readByteArray( sal_Int8*& out,const rtl::OUString& fileName )
    throw( IOException )
{
    RandomAccessStream* in = getStream( fileName,rtl::OUString::createFromAscii("r") );
    if( ! in )
        throw IOException(
            rtl::OUString::createFromAscii( "IndexAccessor::readByteArray -> cannot open file " ) +
            fileName );

    sal_Int32 n;
    delete[] out;
    out = new sal_Int8[ n = in->length() ];
    in->readBytes( out,n );
    delete in;
    return n;
}







