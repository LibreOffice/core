/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_FIREBIRD_BLOB_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_FIREBIRD_BLOB_HXX

#include <ibase.h>

#include <cppuhelper/compbase.hxx>

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/sdbc/XBlob.hpp>

namespace connectivity
{
    namespace firebird
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
            sal_Int64           m_nBlobPosition;

            ISC_STATUS_ARRAY    m_statusVector;

            void ensureBlobIsOpened()
                throw(css::sdbc::SQLException);
            /**
             * Closes the blob and cleans up resources -- can be used to reset
             * the blob if we e.g. want to read from the beginning again.
             */
            void closeBlob()
                throw(css::sdbc::SQLException);

        public:
            Blob(isc_db_handle* pDatabaseHandle,
                 isc_tr_handle* pTransactionHandle,
                 ISC_QUAD& aBlobID);

            // ---- XBlob ----------------------------------------------------
            virtual sal_Int64 SAL_CALL
                length()
                throw(css::sdbc::SQLException,
                      css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Sequence< sal_Int8 > SAL_CALL
                getBytes(sal_Int64 aPosition, sal_Int32 aLength)
                throw(css::sdbc::SQLException,
                      css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Reference< css::io::XInputStream > SAL_CALL
                getBinaryStream()
                throw(css::sdbc::SQLException,
                      css::uno::RuntimeException, std::exception) override;
            virtual sal_Int64 SAL_CALL
                position(const css::uno::Sequence< sal_Int8 >& rPattern,
                         sal_Int64 aStart)
                throw(css::sdbc::SQLException,
                      css::uno::RuntimeException, std::exception) override;
            virtual sal_Int64 SAL_CALL
                positionOfBlob(const css::uno::Reference< css::sdbc::XBlob >& rPattern,
                               sal_Int64 aStart)
                throw(css::sdbc::SQLException,
                      css::uno::RuntimeException, std::exception) override;

            // ---- XInputStream ----------------------------------------------
            virtual sal_Int32 SAL_CALL
                readBytes(css::uno::Sequence< sal_Int8 >& rDataOut,
                          sal_Int32 nBytes)
                throw(css::io::NotConnectedException,
                      css::io::BufferSizeExceededException,
                      css::io::IOException,
                      css::uno::RuntimeException, std::exception) override;
            virtual sal_Int32 SAL_CALL
                readSomeBytes(css::uno::Sequence< sal_Int8 >& rDataOut,
                              sal_Int32 nMaximumBytes)
                throw(css::io::NotConnectedException,
                      css::io::BufferSizeExceededException,
                        css::io::IOException,
                      css::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL
                skipBytes(sal_Int32 nBytes)
                throw(css::io::NotConnectedException,
                      css::io::BufferSizeExceededException,
                      css::io::IOException,
                      css::uno::RuntimeException, std::exception) override;
            virtual sal_Int32 SAL_CALL
                available()
                throw(css::io::NotConnectedException,
                      css::io::IOException,
                      css::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL
                closeInput()
                throw(css::io::NotConnectedException,
                      css::io::IOException,
                      css::uno::RuntimeException, std::exception) override;

            // ---- OComponentHelper ------------------------------------------
            virtual void SAL_CALL disposing() override;
        };
    }

}

#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_FIREBIRD_BLOB_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
