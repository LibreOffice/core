/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <ibase.h>

#include <cppuhelper/compbase.hxx>

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/sdbc/XBlob.hpp>

namespace connectivity::firebird
    {
        typedef ::cppu::WeakComponentImplHelper< css::sdbc::XBlob,
                                                 css::io::XInputStream >
            Blob_BASE;

        class Blob :
            public Blob_BASE
        {
        protected:
            ::osl::Mutex                    m_aMutex;

            isc_db_handle*      m_pDatabaseHandle;
            isc_tr_handle*      m_pTransactionHandle;
            // We store our own copy of the blob id as typically the statement
            // manages its own blob id, and blobs are independent of a statement
            // in firebird.
            ISC_QUAD            m_blobID;
            isc_blob_handle     m_blobHandle;

            bool                m_bBlobOpened;
            sal_Int64           m_nBlobLength;
            sal_uInt16          m_nMaxSegmentSize;
            sal_Int64           m_nBlobPosition;

            ISC_STATUS_ARRAY    m_statusVector;

            /// @throws css::sdbc::SQLException
            void ensureBlobIsOpened();
            /**
             * Closes the blob and cleans up resources -- can be used to reset
             * the blob if we e.g. want to read from the beginning again.
             *
             * @throws css::sdbc::SQLException
             */
            void closeBlob();
            sal_uInt16 getMaximumSegmentSize();

        public:
            Blob(isc_db_handle* pDatabaseHandle,
                 isc_tr_handle* pTransactionHandle,
                 ISC_QUAD const & aBlobID);

            bool readOneSegment(css::uno::Sequence< sal_Int8 >& rDataOut);

            // ---- XBlob ----------------------------------------------------
            virtual sal_Int64 SAL_CALL
                length() override;
            virtual css::uno::Sequence< sal_Int8 > SAL_CALL
                getBytes(sal_Int64 aPosition, sal_Int32 aLength) override;
            virtual css::uno::Reference< css::io::XInputStream > SAL_CALL
                getBinaryStream() override;
            virtual sal_Int64 SAL_CALL
                position(const css::uno::Sequence< sal_Int8 >& rPattern,
                         sal_Int64 aStart) override;
            virtual sal_Int64 SAL_CALL
                positionOfBlob(const css::uno::Reference< css::sdbc::XBlob >& rPattern,
                               sal_Int64 aStart) override;

            // ---- XInputStream ----------------------------------------------
            virtual sal_Int32 SAL_CALL
                readBytes(css::uno::Sequence< sal_Int8 >& rDataOut,
                          sal_Int32 nBytes) override;
            virtual sal_Int32 SAL_CALL
                readSomeBytes(css::uno::Sequence< sal_Int8 >& rDataOut,
                              sal_Int32 nMaximumBytes) override;
            virtual void SAL_CALL
                skipBytes(sal_Int32 nBytes) override;
            virtual sal_Int32 SAL_CALL
                available() override;
            virtual void SAL_CALL
                closeInput() override;

            // ---- OComponentHelper ------------------------------------------
            virtual void SAL_CALL disposing() override;
        };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
