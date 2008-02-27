/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: log.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lla $ $Date: 2008-02-27 16:25:31 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cppunit.hxx"

#ifndef __QADEV_REGSCAN_LOG_HXX__
#include "testshl/log.hxx"
#endif

// #include <hash_map>
// #include <list>
// #include <set>

using namespace std;

::osl::FileBase::RC Log::open( sal_Bool append ) {

    ::osl::FileBase::RC ret;

    if ( ! append ) {
        ret = ::osl::File::remove( m_logurl );
    }

    if( m_logfile->open( OpenFlag_Write ) == ::osl::FileBase::E_NOENT ) {
        ret = m_logfile->open( OpenFlag_Write | OpenFlag_Create );
    }
    else  {
        ret = m_logfile->setPos( Pos_End, 0 );
    }
    return ret;
}

::osl::FileBase::RC Log::write( const sal_Char* buf, sal_Bool v ) {
    sal_uInt64 uBytes=0;
    sal_uInt32 len = 0;
    const sal_Char* ptr = buf;

    if( ptr ) {
        while( *ptr++ ) len++;
    }

    if ( v ) {
        // cout << buf << flush;
        printf("%s", buf);
    }
    return m_logfile->write( buf, len , uBytes );
}

::osl::FileBase::RC Log::write( const rtl::OString& buf, sal_Bool v  ) {
    sal_uInt64 uBytes=0;
    if ( v ) {
        // cout << buf.getStr() << flush;
        printf("%s", buf.getStr());
    }
    return m_logfile->write( buf.getStr(), buf.getLength(), uBytes );
}
/*
// LLA: due to a problem with sal/OUStingBuffer getLength() which isn't const, this
// is so not compilable.
::osl::FileBase::RC Log::write( rtl::OStringBuffer const& buf, sal_Bool v  ) {
    sal_uInt64 uBytes=0;
    if ( v ) {
        cout << buf.getStr() << flush;
    }
    return m_logfile->write( buf.getStr(), buf.getLength(), uBytes );
}
*/
::osl::FileBase::RC Log::write( const rtl::OUString& buf,
                                    rtl_TextEncoding tenc, sal_Bool v  ) {
    sal_uInt64 uBytes=0;
    if ( ! tenc ) {
        tenc = RTL_TEXTENCODING_ASCII_US;
    }
    rtl::OStringBuffer osbuf(
                            rtl::OUStringToOString( buf, tenc ).getStr() );

    if ( v ) {
        // cout << osbuf.getStr() << flush;
        printf("%s", osbuf.getStr());
    }
    return m_logfile->write( osbuf.getStr(), osbuf.getLength(), uBytes );
}

Log &operator <<( Log &_aLog, const sal_Char * _sValue )
{
    _aLog.write(_sValue); return _aLog;
}
Log &operator <<( Log &_aLog, rtl::OString const& _sValue )
{
    _aLog.write(_sValue); return _aLog;
}
Log &operator <<( Log &_aLog, rtl::OUString const& _sValue )
{
    _aLog.write(_sValue); return _aLog;
}
//! Log &operator <<( Log &_aLog, rtl::OStringBuffer const& _sValue )
//! {
//!     _aLog.write(_sValue); return _aLog;
//! }
