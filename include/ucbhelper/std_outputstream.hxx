/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _UCBHELPER_STD_OUTPUTSTREAM_HXX_
#define _UCBHELPER_STD_OUTPUTSTREAM_HXX_

#include <boost/shared_ptr.hpp>
#include <ostream>

#include <osl/mutex.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <com/sun/star/io/XOutputStream.hpp>
#include "ucbhelper/ucbhelperdllapi.h"

namespace ucbhelper
{
    /** Implements a OutputStream
     *  working on an std::ostream
     */
    class UCBHELPER_DLLPUBLIC StdOutputStream :
        public cppu::OWeakObject,
        public css::io::XOutputStream
    {
        public:

            StdOutputStream( boost::shared_ptr< std::ostream > pStream );

            ~StdOutputStream( );

            virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& rType )
                throw ( css::uno::RuntimeException );

            virtual void SAL_CALL acquire ( ) throw ( );

            virtual void SAL_CALL release ( ) throw ( );

            virtual void SAL_CALL writeBytes ( const css::uno::Sequence< sal_Int8 >& aData )
                throw ( css::io::NotConnectedException,
                        css::io::BufferSizeExceededException,
                        css::io::IOException,
                        css::uno::RuntimeException );

            virtual void SAL_CALL flush ( )
                throw ( css::io::NotConnectedException,
                        css::io::BufferSizeExceededException,
                        css::io::IOException,
                        css::uno::RuntimeException );

            virtual void SAL_CALL closeOutput ( )
                throw ( css::io::NotConnectedException,
                        css::io::BufferSizeExceededException,
                        css::io::IOException,
                        css::uno::RuntimeException );

        private:

            osl::Mutex m_aMutex;
            boost::shared_ptr< std::ostream > m_pStream;
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
