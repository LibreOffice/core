 /*************************************************************************
 *
 *  $RCSfile: prodmap.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: obo $ $Date: 2006-04-24 15:20:03 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

