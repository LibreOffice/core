/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "Blob.hxx"

#include <cppuhelper/compbase.hxx>

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/sdbc/XClob.hpp>
#include <rtl/ref.hxx>

namespace connectivity::firebird
    {
        typedef ::cppu::WeakComponentImplHelper< css::sdbc::XClob >
            Clob_BASE;

        class Clob :
            public Clob_BASE
        {
        protected:
            ::osl::Mutex                    m_aMutex;

            /*
             * In Firebird Clob (textual Blob) is a subtype of blob,
             * hence we store the data in a Blob, and the Clob class is
             * a wrapper around that.
             */
            rtl::Reference<connectivity::firebird::Blob> m_aBlob;

            sal_Int64 m_nCharCount;

        public:
            Clob(isc_db_handle* pDatabaseHandle,
                 isc_tr_handle* pTransactionHandle,
                 ISC_QUAD const & aBlobID);

            // ---- XClob ----------------------------------------------------
            virtual sal_Int64 SAL_CALL
                length() override;
            virtual OUString SAL_CALL
                getSubString(sal_Int64 aPosition, sal_Int32 aLength) override;
            virtual css::uno::Reference< css::io::XInputStream > SAL_CALL
                getCharacterStream() override;
            virtual sal_Int64 SAL_CALL
                position(const OUString& rPattern,
                         sal_Int32 aStart) override;
            virtual sal_Int64 SAL_CALL
                positionOfClob(const ::css::uno::Reference< ::css::sdbc::XClob >& rPattern,
                               sal_Int64 aStart) override;
            // ---- OComponentHelper ------------------------------------------
            virtual void SAL_CALL disposing() override;
        };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
