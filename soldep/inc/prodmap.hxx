/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef PRODUCT_MAPPER_HXX
#define PRODUCT_MAPPER_HXX

#include <soldep/sstring.hxx>

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

    sal_uInt16 PrintDependentTargets( const ByteString &rProduct, sal_uInt16 nLevel = 0 );
    sal_uInt16 PrintAndDeleteBaseList();

    SByteStringList *GetMinorList( const ByteString &rVersion, const ByteString &rEnvironment );
    BaseProductList *GetBases( GenericInformation *pProductInfo, sal_uInt16 nLevel = 0, BaseProductList *pBases = NULL );

    sal_uInt16 PrintSingleMinorList( GenericInformation *pProductInfo, BaseProductList *pBases, const ByteString rEnvironment );

public:
    ProductMapper();
    ProductMapper( GenericInformationList *pVerList );

    ~ProductMapper();

    void CreateProductList( GenericInformationList *pVerList );

    sal_uInt16 GetProductInformation( const ByteString &rProduct, GenericInformation *& pProductInfo );
    sal_uInt16 PrintDependencies( const ByteString &rProduct );
    sal_uInt16 PrintProductList();

    sal_uInt16 PrintMinorList( const ByteString rProduct, const ByteString rEnvironment );

    static String GetVersionRoot( GenericInformationList *pList, const ByteString &rVersion );

    GenericInformationList *GetProductList() { return pProductList; }
};


#endif // PRODUCT_MAPPER_HXX

