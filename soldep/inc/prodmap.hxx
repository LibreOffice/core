 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: prodmap.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 16:35:04 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef PRODUCT_MAPPER_HXX
#define PRODUCT_MAPPER_HXX

#ifndef _SSTRING_HXX
#include <bootstrp/sstring.hxx>
#endif

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

