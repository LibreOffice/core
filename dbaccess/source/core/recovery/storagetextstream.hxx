/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* This file is part of OpenOffice.org.
*
* OpenOffice.org is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License version 3
* only, as published by the Free Software Foundation.
*
* OpenOffice.org is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License version 3 for more details
* (a copy is included in the LICENSE file that accompanied this code).
*
* You should have received a copy of the GNU Lesser General Public License
* version 3 along with OpenOffice.org.  If not, see
* <http://www.openoffice.org/license.html>
* for a copy of the LGPLv3 License.
************************************************************************/

#ifndef STORAGETEXTSTREAM_HXX
#define STORAGETEXTSTREAM_HXX

#include "storagestream.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <memory>

//......................................................................................................................
namespace dbaccess
{
//......................................................................................................................

    //==================================================================================================================
    //= StorageTextStream
    //==================================================================================================================
    struct StorageTextOutputStream_Data;
    class DBACCESS_DLLPRIVATE StorageTextOutputStream : public StorageOutputStream
    {
    public:
        StorageTextOutputStream(
            const ::comphelper::ComponentContext& i_rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& i_rParentStorage,
            const ::rtl::OUString& i_rStreamName
        );
        ~StorageTextOutputStream();

        void    writeLine( const ::rtl::OUString& i_rLine );
        void    writeLine();

    private:
        ::std::auto_ptr< StorageTextOutputStream_Data > m_pData;
    };

//......................................................................................................................
} // namespace dbaccess
//......................................................................................................................

#endif // STORAGETEXTSTREAM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
