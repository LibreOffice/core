/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: retrieveinputstream.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-18 13:34:20 $
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
#ifndef _RETRIEVEINPUTSTREAM_HXX
#define _RETRIEVEINPUTSTREAM_HXX

#ifndef _OBSERVABLETHREAD_HXX
#include <observablethread.hxx>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
//#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
//#include <com/sun/star/uno/Reference.hxx>
//#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HXX_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _RETRIEVEDINPUTSTREAMDATA_HXX
#include <retrievedinputstreamdata.hxx>
#endif

/** class for a thread to retrieve an input stream given by an URL

    OD 2007-01-29 #i73788#

    @author OD
*/
class SwAsyncRetrieveInputStreamThread : public ObservableThread
{
    public:

        static ::rtl::Reference< ObservableThread > createThread(
                    const SwRetrievedInputStreamDataManager::tDataKey nDataKey,
                    const String& rLinkedURL );

        virtual ~SwAsyncRetrieveInputStreamThread();

    protected:

        virtual void threadFunction();

    private:

        SwAsyncRetrieveInputStreamThread( const SwRetrievedInputStreamDataManager::tDataKey nDataKey,
                                          const String& rLinkedURL );

        const SwRetrievedInputStreamDataManager::tDataKey mnDataKey;
        const String mrLinkedURL;
};
#endif
