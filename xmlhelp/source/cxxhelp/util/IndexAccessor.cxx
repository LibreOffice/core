/*************************************************************************
 *
 *  $RCSfile: IndexAccessor.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: abi $ $Date: 2001-06-19 13:41:05 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _XMLSEARCH_UTIL_INDEXACCESSOR_HXX_
#include <util/IndexAccessor.hxx>
#endif
#ifndef _XMLEARCH_UTIL_RANDOMACCESSSTREAM_HXX_
#include <util/RandomAccessStream.hxx>
#endif


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
        osl::FileBase::RC err = file_.read( (void*)(data), sal_uInt64(num),nbytesread );

        OSL_ENSURE( err == osl::FileBase::E_None, "RandomAccessStreamImpl::readBytes: -> file not open" );

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

    if( !( isOpen_ = ( file_.open( flags ) == osl::FileBase::E_None )) )
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







