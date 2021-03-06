/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <boost/shared_ptr.hpp>
#include <ostream>

#include <osl/mutex.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/io/XOutputStream.hpp>

namespace cmis
{
    /** Implements a OutputStream
     *  working on an std::ostream
     */
    class StdOutputStream :
        public cppu::OWeakObject,
        public css::io::XOutputStream
    {
        public:

            StdOutputStream( boost::shared_ptr< std::ostream > const & pStream );

            virtual ~StdOutputStream( ) override;

            virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& rType ) override;

            virtual void SAL_CALL acquire ( ) throw ( ) override;

            virtual void SAL_CALL release ( ) throw ( ) override;

            virtual void SAL_CALL writeBytes ( const css::uno::Sequence< sal_Int8 >& aData ) override;

            virtual void SAL_CALL flush ( ) override;

            virtual void SAL_CALL closeOutput ( ) override;

        private:

            osl::Mutex m_aMutex;
            boost::shared_ptr< std::ostream > m_pStream;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
