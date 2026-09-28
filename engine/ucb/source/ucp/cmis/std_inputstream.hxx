/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <boost/shared_ptr.hpp>
#include <istream>

#include <mutex>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>

namespace cmis
{
    /** Implements a seekable InputStream
     *  working on an std::istream
     */
    class StdInputStream
        : public cppu::OWeakObject,
          public css::io::XInputStream,
          public css::io::XSeekable
    {
        public:

            StdInputStream( boost::shared_ptr< std::istream > pStream );

            virtual ~StdInputStream() override;

            virtual cpo::uno::Any queryInterface ( const cpo::uno::Type& rType ) override;

            virtual void acquire( ) noexcept override;

            virtual void release( ) noexcept override;

            virtual sal_Int32
            readBytes ( cpo::uno::Sequence< sal_Int8 >& aData,
                        sal_Int32 nBytesToRead ) override;

            virtual sal_Int32
            readSomeBytes ( cpo::uno::Sequence< sal_Int8 >& aData,
                           sal_Int32 nMaxBytesToRead ) override;

            virtual void
            skipBytes ( sal_Int32 nBytesToSkip ) override;

            virtual sal_Int32
            available ( ) override;

            virtual void
            closeInput ( ) override;


            /** XSeekable
             */

            virtual void
            seek ( sal_Int64 location ) override;


            virtual sal_Int64
            getPosition ( ) override;


            virtual sal_Int64
            getLength ( ) override;

        private:

            std::mutex m_aMutex;
            boost::shared_ptr< std::istream > m_pStream;
            sal_Int64 m_nLength;
    };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
