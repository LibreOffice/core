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
#ifndef CONNECTIVITY_ADO_WRAP_CATALOG_HXX
#define CONNECTIVITY_ADO_WRAP_CATALOG_HXX

#include "ado/WrapTypeDefs.hxx"

namespace connectivity
{
    namespace ado
    {
        class WpADOCatalog : public WpOLEBase<_ADOCatalog>
        {
        public:
            WpADOCatalog(_ADOCatalog* pInt = NULL)  :   WpOLEBase<_ADOCatalog>(pInt){}
            WpADOCatalog(const WpADOCatalog& rhs){operator=(rhs);}

            inline WpADOCatalog& operator=(const WpADOCatalog& rhs)
                {WpOLEBase<_ADOCatalog>::operator=(rhs); return *this;}

            ::rtl::OUString GetObjectOwner(const ::rtl::OUString& _rName, ObjectTypeEnum _eNum);

            void putref_ActiveConnection(IDispatch* pCon);
            WpADOTables     get_Tables();
            WpADOViews      get_Views();
            WpADOGroups     get_Groups();
            WpADOUsers      get_Users();
            ADOProcedures*  get_Procedures();
            void Create();
        };
    }
}

#endif //CONNECTIVITY_ADO_WRAP_CATALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
