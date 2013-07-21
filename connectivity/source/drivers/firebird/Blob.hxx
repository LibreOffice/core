/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CONNECTIVITY_FIREBIRD_BLOB_HXX
#define CONNECTIVITY_FIREBIRD_BLOB_HXX

#include <ibase.h>

#include <cppuhelper/compbase1.hxx>

#include <com/sun/star/sdbc/XBlob.hpp>

namespace connectivity
{
    namespace firebird
    {
        typedef ::cppu::WeakComponentImplHelper1< ::com::sun::star::sdbc::XBlob >
            Blob_BASE;

        class Blob :
            public Blob_BASE
        {
        protected:
            ::osl::Mutex                    m_aMutex;

            isc_db_handle*      m_pDatabaseHandle;
            isc_tr_handle*      m_pTransactionHandle;
            // We store our own copy of the blob id as typically the statement
            // manages it's own blob id, and blobs are independent of a statement
            // in firebird.
            ISC_QUAD            m_blobID;
            isc_blob_handle     m_blobHandle;

            bool                m_bBlobOpened;

            ISC_STATUS_ARRAY    m_statusVector;

            ::com::sun::star::uno::Sequence< sal_Int8 > m_blobData;

            void ensureBlobIsOpened()
                throw(::com::sun::star::sdbc::SQLException);

        public:
            Blob(isc_db_handle* pDatabaseHandle,
                 isc_tr_handle* pTransactionHandle,
                 ISC_QUAD& aBlobID);

            // ---- XBlob ----------------------------------------------------
            virtual sal_Int64 SAL_CALL
                length()
                throw(::com::sun::star::sdbc::SQLException,
                      ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
                getBytes(sal_Int64 aPosition, sal_Int32 aLength)
                throw(::com::sun::star::sdbc::SQLException,
                      ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL
                getBinaryStream()
                throw(::com::sun::star::sdbc::SQLException,
                      ::com::sun::star::uno::RuntimeException);
            virtual sal_Int64 SAL_CALL
                position(const ::com::sun::star::uno::Sequence< sal_Int8 >& rPattern,
                         sal_Int64 aStart)
                throw(::com::sun::star::sdbc::SQLException,
                      ::com::sun::star::uno::RuntimeException);
            virtual sal_Int64 SAL_CALL
                positionOfBlob(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XBlob >& rPattern,
                               sal_Int64 aStart)
                throw(::com::sun::star::sdbc::SQLException,
                      ::com::sun::star::uno::RuntimeException);

            // ---- OComponentHelper ------------------------------------------
            virtual void SAL_CALL disposing();
        };
    }

}

#endif //CONNECTIVITY_FIREBIRD_BLOB_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */