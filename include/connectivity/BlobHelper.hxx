/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_CONNECTIVITY_BLOBHELPER_HXX
#define INCLUDED_CONNECTIVITY_BLOBHELPER_HXX

#include <connectivity/dbtoolsdllapi.hxx>
#include <com/sun/star/sdbc/XBlob.hpp>
#include <cppuhelper/implbase.hxx>

namespace connectivity
{
    class OOO_DLLPUBLIC_DBTOOLS BlobHelper : public ::cppu::WeakImplHelper< com::sun::star::sdbc::XBlob >
    {
        ::com::sun::star::uno::Sequence< sal_Int8 > m_aValue;
    public:
        BlobHelper(const ::com::sun::star::uno::Sequence< sal_Int8 >& _val);
    private:
        virtual ::sal_Int64 SAL_CALL length(  ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Sequence< ::sal_Int8 > SAL_CALL getBytes( ::sal_Int64 pos, ::sal_Int32 length ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getBinaryStream(  ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int64 SAL_CALL position( const ::com::sun::star::uno::Sequence< ::sal_Int8 >& pattern, ::sal_Int64 start ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int64 SAL_CALL positionOfBlob( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XBlob >& pattern, ::sal_Int64 start ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    };
}

#endif // INCLUDED_CONNECTIVITY_BLOBHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
