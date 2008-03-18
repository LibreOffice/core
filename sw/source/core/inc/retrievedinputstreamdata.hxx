/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: retrievedinputstreamdata.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2008-03-18 15:57:48 $
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
#ifndef _RETRIEVEDINPUTSTREAMDATA_HXX
#define _RETRIEVEDINPUTSTREAMDATA_HXX

#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HXX_
#include <com/sun/star/io/XInputStream.hpp>
#endif

#include <map>

#include <boost/weak_ptr.hpp>
class SwAsyncRetrieveInputStreamThreadConsumer;
//#ifndef _RETRIEVEINPUTSTREAMCONSUMER_HXX
//#include <retrieveinputstreamconsumer.hxx>
//#endif

/** Singleton class to manage retrieved input stream data in Writer

    OD 2007-01-29 #i73788#
    The instance of this class provides data container for retrieved input
    stream data. The data container is accessed via a key, which the data
    manager provides on creation of the data container.
    When a certain data container is filled with data, an user event is submitted
    to trigger the processing of with data.

    @author OD
*/
class SwRetrievedInputStreamDataManager
{
    public:

        typedef sal_uInt64 tDataKey;

        struct tData
        {
            boost::weak_ptr< SwAsyncRetrieveInputStreamThreadConsumer > mpThreadConsumer;
            com::sun::star::uno::Reference<com::sun::star::io::XInputStream> mxInputStream;
            sal_Bool mbIsStreamReadOnly;

            tData()
                : mpThreadConsumer(),
                  mbIsStreamReadOnly( sal_False )
            {};

            tData( boost::weak_ptr< SwAsyncRetrieveInputStreamThreadConsumer > pThreadConsumer )
                : mpThreadConsumer( pThreadConsumer ),
                  mbIsStreamReadOnly( sal_False )
            {};
        };

        static SwRetrievedInputStreamDataManager& GetManager();

        ~SwRetrievedInputStreamDataManager()
        {
        };

        tDataKey ReserveData( boost::weak_ptr< SwAsyncRetrieveInputStreamThreadConsumer > pThreadConsumer );

        void PushData( const tDataKey nDataKey,
                       com::sun::star::uno::Reference<com::sun::star::io::XInputStream> xInputStream,
                       const sal_Bool bIsStreamReadOnly );

        bool PopData( const tDataKey nDataKey,
                      tData& rData );

        DECL_LINK( LinkedInputStreamReady, SwRetrievedInputStreamDataManager::tDataKey* );

    private:

        static SwRetrievedInputStreamDataManager* mpManager;
        static tDataKey mnNextKeyValue;
        static osl::Mutex maGetManagerMutex;

        osl::Mutex maMutex;

        std::map< tDataKey, tData > maInputStreamData;

        SwRetrievedInputStreamDataManager()
        {
        };
};
#endif
