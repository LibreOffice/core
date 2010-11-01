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

#ifndef PRODUCT_MAPPER_HXX
#define PRODUCT_MAPPER_HXX

#include <bootstrp/sstring.hxx>

class GenericInformation;
class GenericInformationList;

DECLARE_LIST( BaseProductList, ByteString * )

//
// class ProductMapper
//

#define PRODUCT_MAPPER_OK                       0x0000
#define PRODUCT_MAPPER_NO_PRODUCT               0x0001
#define PRODUCT_MAPPER_NO_VERSION_INFORMATION   0x0002

class ProductMapper
{
private:
    GenericInformationList *pVersionList;
    GenericInformationList *pProductList;

    SByteStringList aBaseList;
    SByteStringList aPrintedList;

    USHORT PrintDependentTargets( const ByteString &rProduct, USHORT nLevel = 0 );
    USHORT PrintAndDeleteBaseList();

    SByteStringList *GetMinorList( const ByteString &rVersion, const ByteString &rEnvironment );
    BaseProductList *GetBases( GenericInformation *pProductInfo, USHORT nLevel = 0, BaseProductList *pBases = NULL );

    USHORT PrintSingleMinorList( GenericInformation *pProductInfo, BaseProductList *pBases, const ByteString rEnvironment );

public:
    ProductMapper();
    ProductMapper( GenericInformationList *pVerList );

    ~ProductMapper();

    void CreateProductList( GenericInformationList *pVerList );

    USHORT GetProductInformation( const ByteString &rProduct, GenericInformation *& pProductInfo );
    USHORT PrintDependencies( const ByteString &rProduct );
    USHORT PrintProductList();

    USHORT PrintMinorList( const ByteString rProduct, const ByteString rEnvironment );

    static String GetVersionRoot( GenericInformationList *pList, const ByteString &rVersion );

    GenericInformationList *GetProductList() { return pProductList; }
};


#endif // PRODUCT_MAPPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
