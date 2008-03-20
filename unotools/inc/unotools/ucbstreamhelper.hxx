/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ucbstreamhelper.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-20 15:29:52 $
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
#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#define _UNTOOLS_UCBSTREAMHELPER_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSTREAM_HPP_
#include <com/sun/star/io/XStream.hpp>
#endif
#ifndef INCLUDED_UNOTOOLSDLLAPI_H
#include "unotools/unotoolsdllapi.h"
#endif

#include <tools/stream.hxx>

namespace com
{
    namespace sun
    {
        namespace star
        {
            namespace task
            {
                class XInteractionHandler;
            }
            namespace io
            {
                class XStream;
                class XInputStream;
            }
        }
    }
}

#define NS_UNO ::com::sun::star::uno
#define NS_IO ::com::sun::star::io
#define NS_TASK ::com::sun::star::task

class String;
namespace utl
{
    class UcbLockBytesHandler;

    class UNOTOOLS_DLLPUBLIC UcbStreamHelper : public SvStream
    {
    public:
        static SvStream*    CreateStream( const String& rFileName, StreamMode eOpenMode,
                                UcbLockBytesHandler* pHandler=0, sal_Bool bForceSynchron=sal_True );
        static SvStream*    CreateStream( const String& rFileName, StreamMode eOpenMode,
                                NS_UNO::Reference < NS_TASK::XInteractionHandler >,
                                UcbLockBytesHandler* pHandler=0, sal_Bool bForceSynchron=sal_True );
        static SvStream*    CreateStream( const String& rFileName, StreamMode eOpenMode,
                                sal_Bool bFileExists,
                                UcbLockBytesHandler* pHandler=0, sal_Bool bForceSynchron=sal_True );
        static SvStream*    CreateStream( NS_UNO::Reference < NS_IO::XInputStream > xStream );
        static SvStream*    CreateStream( NS_UNO::Reference < NS_IO::XStream > xStream );
        static SvStream*    CreateStream( NS_UNO::Reference < NS_IO::XInputStream > xStream, sal_Bool bCloseStream );
        static SvStream*    CreateStream( NS_UNO::Reference < NS_IO::XStream > xStream, sal_Bool bCloseStream );
    };
}

#endif
