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

#include "prodmap.hxx"
#include <tools/geninfo.hxx>
#include <tools/fsys.hxx>
#include "minormk.hxx"

#include <stdio.h>

#define PRODUCT_KEY     "TARGETDESCRIPTION/PRODUCTS"
#define DEPENDS_ON_KEY  "TARGETDESCRIPTION/DEPENDSON"
#define BASED_ON_KEY    "TARGETDESCRIPTION/BASEDON"

//
// class ProductMapper
//

/*****************************************************************************/
ProductMapper::ProductMapper()
/*****************************************************************************/
                : pVersionList( NULL ),
                pProductList( NULL )
{
}

/*****************************************************************************/
ProductMapper::ProductMapper( GenericInformationList *pVerList )
/*****************************************************************************/
                : pVersionList( pVerList ),
                pProductList( NULL )
{
    if ( pVerList )
        CreateProductList( pVerList );
}

/*****************************************************************************/
ProductMapper::~ProductMapper()
/*****************************************************************************/
{
    delete pProductList;
}

/*****************************************************************************/
void ProductMapper::CreateProductList( GenericInformationList *pVerList )
/*****************************************************************************/
{
    /*
    creates a list of the following format:

    ProductName Workspace                // 6.0 Final SRC641
    {                                    // {
        DependsOn                        //     DependsOn
        {                                //     {
            product1                     //
            product2                     //
            ...                          //
        }                                //
        BasedOn                          //
        {                                //
            productX                     //
            productY                     //
            ...                          //
        }                                //
    }                                    //
    */

    delete pProductList;
    pProductList = NULL;

    pVersionList = pVerList;

    if ( pVersionList ) {
        ByteString sProductKey( PRODUCT_KEY );
        ByteString sDependsOnKey( DEPENDS_ON_KEY );
        ByteString sBasedOnKey( BASED_ON_KEY );

        for ( sal_uIntPtr i = 0; i < pVersionList->Count(); i++ ) {
            GenericInformation *pVersion = pVersionList->GetObject( i );

            GenericInformation *pProducts = pVersion->GetSubInfo( sProductKey, sal_True );
            if ( pProducts ) {
                ByteString sProducts = pProducts->GetValue();

                ByteString sDependsOn;
                GenericInformation *pDependsOn = pVersion->GetSubInfo( sDependsOnKey, sal_True );
                if ( pDependsOn )
                    sDependsOn = pDependsOn->GetValue();

                ByteString sBasedOn;
                GenericInformation *pBasedOn = pVersion->GetSubInfo( sBasedOnKey, sal_True );
                if ( pBasedOn )
                    sBasedOn = pBasedOn->GetValue();

                for ( sal_uInt16 x = 0; x < sProducts.GetTokenCount( ';' ); x++ ) {
                    ByteString sProduct( sProducts.GetToken( x, ';' ));
                    if( sProduct.Len()) {
                        if ( !pProductList )
                            pProductList = new GenericInformationList();

                        pProductList->InsertInfo( sProduct, *pVersion, sal_True, sal_True );

                        for ( sal_uInt16 y = 0; y < sDependsOn.GetTokenCount( ';' ); y++ ) {
                            ByteString sDependsOnKey_l = sProduct;
                            sDependsOnKey_l += "/DependsOn/";
                            sDependsOnKey_l += sDependsOn.GetToken( y, ';' );

                            pProductList->InsertInfo( sDependsOnKey_l, "", sal_True, sal_True );
                        }
                        for ( sal_uInt16 z = 0; z < sBasedOn.GetTokenCount( ';' ); z++ ) {
                            ByteString sBasedOnKey_l = sProduct;
                            sBasedOnKey_l += "/BasedOn/";
                            sBasedOnKey_l += sBasedOn.GetToken( z, ';' );

                            pProductList->InsertInfo( sBasedOnKey_l, "", sal_True, sal_True );
                        }
                    }
                }
            }
        }
    }
}

/*****************************************************************************/
sal_uInt16 ProductMapper::GetProductInformation(
    const ByteString &rProduct, GenericInformation *& pProductInfo )
/*****************************************************************************/
{
    pProductInfo = NULL;

    if ( !pVersionList )
        return PRODUCT_MAPPER_NO_VERSION_INFORMATION;

    if ( !pProductList )
        return PRODUCT_MAPPER_NO_PRODUCT;

    ByteString sProductKey( rProduct );
    pProductInfo = pProductList->GetInfo( sProductKey, sal_True );

    if ( !pProductInfo )
        return PRODUCT_MAPPER_NO_PRODUCT;

    return PRODUCT_MAPPER_OK;
}

/*****************************************************************************/
sal_uInt16 ProductMapper::PrintDependentTargets(
    const ByteString &rProduct, sal_uInt16 nLevel )
/*****************************************************************************/
{
    GenericInformation *pProductInfo;

    sal_uInt16 nReturn = GetProductInformation( rProduct, pProductInfo );

    if ( nReturn == PRODUCT_MAPPER_OK ) {
        for ( sal_uInt16 i = 0; i < nLevel; i++ )
            fprintf( stdout, "    " );
        fprintf( stdout, "%s (%s)\n", pProductInfo->GetBuffer(),
            pProductInfo->GetValue().GetBuffer());
        aPrintedList.PutString( new ByteString( *pProductInfo ));

        for ( sal_uIntPtr j = 0; j < pProductList->Count(); j++ ) {
            GenericInformation *pCandidate = pProductList->GetObject( j );
            ByteString sKey( "DEPENDSON/" );
            sKey += rProduct;
            GenericInformation *pDependsOn = pCandidate->GetSubInfo( sKey, sal_True );
            if ( pDependsOn )
                PrintDependentTargets( *pCandidate, nLevel + 1 );
        }
        if ( !nLevel ) {
            ByteString sKey( "BASEDON" );
            GenericInformation *pBasedOn = pProductInfo->GetSubInfo( sKey );
            if ( pBasedOn ) {
                GenericInformationList *pBases = pBasedOn->GetSubList();
                if ( pBases ) {
                    for ( sal_uIntPtr k = 0; k < pBases->Count(); k++ ) {
                        aBaseList.PutString( new ByteString( *pBases->GetObject( k )));
                    }
                }
            }
        }
    }

    return nReturn;
}

/*****************************************************************************/
sal_uInt16 ProductMapper::PrintAndDeleteBaseList()
/*****************************************************************************/
{
    if ( aBaseList.Count()) {
        fprintf( stdout, "\nbased on\n" );
        while ( aBaseList.Count()) {
            ByteString sProduct( *aBaseList.GetObject(( sal_uIntPtr ) 0 ));
            if ( aPrintedList.IsString( aBaseList.GetObject(( sal_uIntPtr ) 0 )) == NOT_THERE ) {
                aPrintedList.PutString( aBaseList.GetObject(( sal_uIntPtr ) 0 ));
                PrintDependentTargets( sProduct );
            }
            else
                delete aBaseList.GetObject(( sal_uIntPtr ) 0 );

            aBaseList.Remove(( sal_uIntPtr ) 0 );
        }
        while ( aPrintedList.Count())
            delete aPrintedList.Remove(( sal_uIntPtr ) 0 );

        fprintf( stdout, "\n" );
    }
    return PRODUCT_MAPPER_OK;
}

/*****************************************************************************/
sal_uInt16 ProductMapper::PrintDependencies( const ByteString &rProduct )
/*****************************************************************************/
{
    sal_uInt16 nResult = PrintDependentTargets( rProduct );
    PrintAndDeleteBaseList();
    return nResult;
}

/*****************************************************************************/
sal_uInt16 ProductMapper::PrintProductList()
/*****************************************************************************/
{
    if ( !pVersionList )
        return PRODUCT_MAPPER_NO_VERSION_INFORMATION;

    if ( !pProductList || !pProductList->Count())
        return PRODUCT_MAPPER_NO_PRODUCT;

    if ( pProductList->Count()) {
        for ( sal_uIntPtr i = 0; i < pProductList->Count(); i++ )
            fprintf( stdout, "%s (%s)\n",
                pProductList->GetObject( i )->GetBuffer(),
                pProductList->GetObject( i )->GetValue().GetBuffer());
        fprintf( stdout, "\n" );
    }

    return PRODUCT_MAPPER_OK;
}

/*****************************************************************************/
SByteStringList *ProductMapper::GetMinorList(
    const ByteString &rVersion, const ByteString &rEnvironment )
/*****************************************************************************/
{
    SByteStringList *pList = NULL;

    if ( pVersionList ) {
        String sRoot( GetVersionRoot( pVersionList, rVersion ));
        if ( sRoot.Len()) {
            DirEntry aEntry( sRoot );
            aEntry += DirEntry( String( rEnvironment, RTL_TEXTENCODING_ASCII_US ));
            String sWildcard( String::CreateFromAscii( "inc.*" ));
            aEntry += DirEntry( sWildcard );

            Dir aDir( aEntry, FSYS_KIND_DIR );
            for ( sal_uInt16 i = 0; i < aDir.Count(); i++ ) {
                ByteString sInc( aDir[ i ].GetName(), RTL_TEXTENCODING_ASCII_US );
                if ( sInc.GetTokenCount( '.' ) > 1 ) {
                    if ( !pList )
                        pList = new SByteStringList();
                    pList->PutString( new ByteString( sInc.GetToken( 1, '.' )));
                }
            }
        }
    }
    return pList;
}

/*****************************************************************************/
String ProductMapper::GetVersionRoot(
    GenericInformationList *pList, const ByteString &rVersion )
/*****************************************************************************/
{
    ByteString sKey( rVersion );
    GenericInformation *pVersion = pList->GetInfo( sKey );
    if ( pVersion ) {
#ifdef UNX
        sKey = "drives/o:/unixvolume";
        GenericInformation *pUnixVolume = pVersion->GetSubInfo( sKey, sal_True );
        ByteString sPath;
        if ( pUnixVolume )
            sPath = pUnixVolume->GetValue();
        sPath += "/";
#else
        ByteString sPath( "o:\\" );
#endif
        sKey = "settings/path";
        GenericInformation *pPath = pVersion->GetSubInfo( sKey, sal_True );
        if ( pPath ) {
            sPath += pPath->GetValue().GetToken( 0, '\\' );
            sPath += "/";
        }
#ifdef UNX
        sPath.SearchAndReplaceAll( "\\", "/" );
        while( sPath.SearchAndReplace( "//", "/" ) != STRING_NOTFOUND ) {};
#else
        sPath.SearchAndReplaceAll( "/", "\\" );
        while( sPath.SearchAndReplace( "\\\\", "\\" ) != STRING_NOTFOUND ) {};
#endif

        return String( sPath, RTL_TEXTENCODING_ASCII_US );
    }
    return String();
}

/*****************************************************************************/
BaseProductList *ProductMapper::GetBases(
    GenericInformation *pProductInfo, sal_uInt16 nLevel,
    BaseProductList *pBases )
/*****************************************************************************/
{
    if ( !pBases )
        pBases = new BaseProductList();

    if ( pProductInfo ) {
        ByteString sCandidate( *pProductInfo );
        sCandidate += " (";
        sCandidate += pProductInfo->GetValue();
        sCandidate += ")";

        ByteString sKey( "BASEDON" );
        GenericInformation *pBasedOn = pProductInfo->GetSubInfo( sKey );
        if ( pBasedOn ) {
            GenericInformationList *pBasesInfo = pBasedOn->GetSubList();
            if ( pBasesInfo ) {
                for ( sal_uIntPtr k = 0; k < pBasesInfo->Count(); k++ ) {
                    GenericInformation *pBaseProduct;
                    if ( GetProductInformation( *pBasesInfo->GetObject( k ), pBaseProduct ) == PRODUCT_MAPPER_OK )
                        GetBases( pBaseProduct, ++ nLevel, pBases );
                }
            }
        }
        sal_Bool bFound = sal_False;
        ByteString sUpperCandidate( sCandidate );
        sUpperCandidate.ToUpperAscii();
        for ( sal_uInt16 i = 0; i < pBases->Count() && !bFound; i++ ) {
            ByteString sTest( *pBases->GetObject( i ));
            if ( sTest.ToUpperAscii() == sUpperCandidate )
                bFound = sal_True;
        }
        if ( !bFound )
            pBases->Insert( new ByteString( sCandidate ), ( sal_uIntPtr ) 0 );
    }
    return pBases;
}

/*****************************************************************************/
sal_uInt16 ProductMapper::PrintMinorList(
    const ByteString rProduct, const ByteString rEnvironment )
/*****************************************************************************/
{
    if ( !pVersionList )
        return PRODUCT_MAPPER_NO_VERSION_INFORMATION;

    if ( !pProductList || !pProductList->Count())
        return PRODUCT_MAPPER_NO_PRODUCT;

    GenericInformation *pProductInfo;
    GetProductInformation( rProduct, pProductInfo );
    if ( !pProductInfo )
        return PRODUCT_MAPPER_NO_PRODUCT;

    BaseProductList *pBases = GetBases( pProductInfo );
    if ( pBases->Count()) {
        if ( pBases->Count() > 1 )
            fprintf( stdout, "Product \"%s\" based on ", pBases->GetObject(( sal_uIntPtr ) 0 )->GetBuffer());
        else
            fprintf( stdout, "Product \"%s\" based on no other products", pBases->GetObject(( sal_uIntPtr ) 0 )->GetBuffer());

        for ( sal_uIntPtr i = 1; i < pBases->Count(); i++ ) {
            fprintf( stdout, "\"%s\"", pBases->GetObject( i )->GetBuffer());
            if ( i < pBases->Count() - 1 )
                fprintf( stdout, ", " );
        }
        fprintf( stdout, "\n\n" );
    }
    sal_uInt16 nResult = PRODUCT_MAPPER_OK;

    if ( rEnvironment.Len())
        nResult = PrintSingleMinorList( pProductInfo, pBases, rEnvironment );
    else {
        ByteString sEnvKey( pProductInfo->GetValue());
        sEnvKey += "/Environments";

        GenericInformation *pEnvironmentInfo = pVersionList->GetInfo( sEnvKey, sal_True );
        if ( pEnvironmentInfo ) {
            GenericInformationList *pEnvironmentList = pEnvironmentInfo->GetSubList();
            if ( pEnvironmentList ) {
                for ( sal_uIntPtr i = 0; i < pEnvironmentList->Count(); i++ ) {
                    sal_uInt16 nTmp = PrintSingleMinorList( pProductInfo, pBases, *pEnvironmentList->GetObject( i ));
                    if ( nTmp != PRODUCT_MAPPER_OK )
                        nResult = nTmp;
                }
            }
        }
    }

    for ( sal_uIntPtr m = 0; m < pBases->Count(); m++ )
        delete pBases->GetObject( m );
    delete pBases;

    return nResult;
}

/*****************************************************************************/
sal_uInt16 ProductMapper::PrintSingleMinorList(
    GenericInformation *pProductInfo, BaseProductList *pBases,
    const ByteString rEnvironment )
/*****************************************************************************/
{
    DirEntry aRoot( GetVersionRoot( pVersionList, pProductInfo->GetValue()));
    aRoot += DirEntry( String( rEnvironment, RTL_TEXTENCODING_ASCII_US ));
    if ( !aRoot.Exists())
        return PRODUCT_MAPPER_OK;

    SByteStringList *pMinors = GetMinorList( pProductInfo->GetValue(), rEnvironment );
    if ( !pMinors )
        pMinors = new SByteStringList();
    pMinors->Insert( new ByteString( "" ), LIST_APPEND );

    SByteStringList aOutputList;
    sal_Bool bUnknownMinor = sal_False;
    for ( sal_uIntPtr i = 0; i < pMinors->Count(); i++ ) {
        ByteString sOutput;
        ByteString sProductVersion;

        for ( sal_uIntPtr j = 0; j < pBases->Count(); j++ ) {
            ByteString sCurProduct( *pBases->GetObject( j ));
            ByteString sVersion( sCurProduct.GetToken( sCurProduct.GetTokenCount( '(' ) - 1, '(' ).GetToken( 0, ')' ));
            if ( !j )
                sProductVersion = sVersion;

            MinorMk *pMinorMk = new MinorMk(
                pVersionList, sProductVersion, sVersion, rEnvironment, *pMinors->GetObject( i ));

            ByteString sMinor( pMinorMk->GetLastMinor().GetBuffer());
            if ( !sMinor.Len()) {
                sMinor = "!";
                bUnknownMinor = sal_True;
            }
            if ( j == 0 ) {
                sOutput += pMinorMk->GetBuildNr();
                sOutput += " ";

                if ( i == pMinors->Count() - 1 )
                    sOutput += "flat: ";
                else
                    sOutput += "      ";
            }
            sOutput += sVersion;
            sOutput += ".";
            sOutput += sMinor;
            sOutput += "(";
            sOutput += pMinorMk->GetBuildNr();
            sOutput += ")  ";
        }
        aOutputList.PutString( new ByteString( sOutput ));
    }
    ByteString sOldMinor;

    if ( aOutputList.Count())
        fprintf( stdout, "Available builds on %s:\n", rEnvironment.GetBuffer());

    for ( sal_uIntPtr o = 0; o < aOutputList.Count(); o++ ) {
        ByteString sOutput( *aOutputList.GetObject( o ));
        sOutput = sOutput.Copy( sOutput.GetToken( 0, ' ' ).Len() + 1 );

        ByteString sCurMinor( sOutput.GetToken( 1, '.' ).GetToken( 0, '(' ));
        if ( sOldMinor.Len() && sCurMinor < sOldMinor ) {
            fprintf( stdout, "      ----------\n" );
        }
        sOldMinor = sCurMinor;

        fprintf( stdout, "%s\n", sOutput.GetBuffer());
        delete aOutputList.GetObject( o );
    }
    if ( bUnknownMinor )
        fprintf( stdout, "Symbol ! indcates that at least one minor could not be found\n\n" );
    else if ( aOutputList.Count())
        fprintf( stdout, "\n" );

    for ( sal_uIntPtr l = 0; l < pMinors->Count(); l++ )
        delete pMinors->GetObject( l );
    delete pMinors;

    return PRODUCT_MAPPER_OK;
}




