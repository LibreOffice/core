/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_EVOAB_LTABLES_HXX_
#define _CONNECTIVITY_EVOAB_LTABLES_HXX_

#include "file/FTables.hxx"

namespace connectivity
{
    namespace evoab
    {
                //      namespace ::com::sun::star::sdbcx             = ::com::sun::star::sdbcx;
        typedef file::OTables OEvoabTables_BASE;

        class OEvoabTables : public OEvoabTables_BASE
        {
        protected:
            virtual sdbcx::ObjectType createObject(const ::rtl::OUString& _rName);
        public:
            OEvoabTables(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _rMetaData,::cppu::OWeakObject& _rParent, ::osl::Mutex& _rMutex,
                const TStringVector &_rVector) : OEvoabTables_BASE(_rMetaData,_rParent,_rMutex,_rVector)
            {}
        };
    }
}
#endif // _CONNECTIVITY_EVOAB_LTABLES_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
