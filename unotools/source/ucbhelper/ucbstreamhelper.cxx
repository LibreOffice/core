/*************************************************************************
 *
 *  $RCSfile: ucbstreamhelper.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mba $ $Date: 2000-11-02 10:24:49 $
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

#include <unotools/ucblockbytes.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <comphelper/processfactory.hxx>

#ifndef _COM_SUN_STAR_UCB_XCOMMANDENVIRONMENT_HDL_
#include <com/sun/star/ucb/XCommandEnvironment.hdl>
#endif

#ifndef _COM_SUN_STAR_UCB_INSERTCOMMANDARGUMENT_HPP_
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASTREAMER_HPP_
#include <com/sun/star/io/XActiveDataStreamer.hpp>
#endif

#include <ucbhelper/contentbroker.hxx>
#include <ucbhelper/content.hxx>
#include <tools/debug.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

namespace utl
{

SvStream* UcbStreamHelper::CreateStream( const String& rFileName, StreamMode eOpenMode, UcbLockBytesHandler* pHandler, sal_Bool bForceSynchron )
{
    SvStream* pStream = NULL;
    ::ucb::ContentBroker* pBroker = ::ucb::ContentBroker::get();
    if ( pBroker )
    {
        try
        {
            UcbLockBytesRef xLockBytes;
            ::ucb::Content aContent( rFileName, Reference < XCommandEnvironment >() );
            if ( eOpenMode & STREAM_WRITE )
            {
                sal_Bool bTruncate = ( eOpenMode & STREAM_TRUNC );
                if ( bTruncate )
                    // truncate is implemented with deleting the original file
                    aContent.executeCommand( ::rtl::OUString::createFromAscii( "delete" ), makeAny( sal_Bool( sal_True ) ) );

                // make sure that the desired file exists before trying to open
                InsertCommandArgument aInsertArg;
                aInsertArg.Data = Reference< XInputStream >();
                aInsertArg.ReplaceExisting = sal_False;
                Any aCmdArg;
                aCmdArg <<= aInsertArg;
                aContent.executeCommand( ::rtl::OUString::createFromAscii( "insert" ), aCmdArg );
            }

            // create LockBytes using UCB
            xLockBytes = UcbLockBytes::CreateLockBytes( aContent.get(), eOpenMode, pHandler );
            if ( xLockBytes.Is() )
                pStream = new SvStream( xLockBytes );
        }
        catch ( Exception e )
        {
            DBG_ERROR( "Any other exception!" );
        }
    }
    else
        // if no UCB is present at least conventional file io is supported
        pStream = new SvFileStream( rFileName, eOpenMode );

    return pStream;
}

SvStream* UcbStreamHelper::CreateStream( Reference < XInputStream > xStream )
{
    UcbLockBytesRef xLockBytes = UcbLockBytes::CreateInputLockBytes( xStream );
    if ( xLockBytes.Is() )
        return new SvStream( xLockBytes );
    else
        return NULL;
};

};
