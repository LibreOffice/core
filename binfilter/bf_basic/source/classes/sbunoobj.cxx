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

#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _TOOLERR_HXX //autogen
#include <tools/errcode.hxx>
#endif
#ifndef _SFXHINT_HXX //autogen
#include <bf_svtools/hint.hxx>
#endif

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/extract.hxx>

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>

#include <com/sun/star/script/ArrayWrapper.hpp>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/DeploymentException.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyConcept.hpp>
#include <com/sun/star/beans/MethodConcept.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/script/BasicErrorException.hpp>
#include <com/sun/star/script/XAllListener.hpp>
#include <com/sun/star/script/XInvocationAdapterFactory.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <com/sun/star/script/XDefaultProperty.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/reflection/XIdlArray.hpp>
#include <com/sun/star/reflection/XIdlReflection.hpp>
#include <com/sun/star/reflection/XIdlClassProvider.hpp>
#include <com/sun/star/reflection/XTypeDescription.hpp>
#include <com/sun/star/bridge/oleautomation/NamedArgument.hpp>
#include <com/sun/star/bridge/oleautomation/Date.hpp>
#include <com/sun/star/bridge/oleautomation/Decimal.hpp>
#include <com/sun/star/bridge/oleautomation/Currency.hpp>


using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::reflection;
using namespace com::sun::star::beans;
using namespace com::sun::star::script;
using namespace com::sun::star::container;
using namespace com::sun::star::bridge;
using namespace cppu;
using namespace rtl;


#include "sbstar.hxx"
/*?*/ //#include "sbuno.hxx"
#include "sberrors.hxx"
#include "sbunoobj.hxx"
#include "sbjsmod.hxx"
#include "basmgr.hxx"
#include "sbintern.hxx"
/*?*/ //#include "runtime.hxx"

#include<math.h>

namespace binfilter {

TYPEINIT1(SbUnoMethod,SbxMethod)
TYPEINIT1(SbUnoProperty,SbxProperty)
TYPEINIT1(SbUnoObject,SbxObject)
TYPEINIT1(SbUnoClass,SbxObject)

typedef WeakImplHelper1< XAllListener > BasicAllListenerHelper;

// Flag, um immer ueber Invocation zu gehen
//#define INVOCATION_ONLY


// Identifier fuer die dbg_-Properies als Strings anlegen
static String ID_DBG_SUPPORTEDINTERFACES( RTL_CONSTASCII_USTRINGPARAM("Dbg_SupportedInterfaces") );
static String ID_DBG_PROPERTIES( RTL_CONSTASCII_USTRINGPARAM("Dbg_Properties") );
static String ID_DBG_METHODS( RTL_CONSTASCII_USTRINGPARAM("Dbg_Methods") );

static String aIllegalArgumentExceptionName
    ( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.lang.IllegalArgumentException" ) );
static OUString aSeqLevelStr( RTL_CONSTASCII_USTRINGPARAM("[]") );

Reference< XComponentContext > getComponentContext_Impl( void )
{
    static Reference< XComponentContext > xContext;

    // Haben wir schon CoreReflection, sonst besorgen
    if( !xContext.is() )
    {
        Reference< XMultiServiceFactory > xFactory = comphelper::getProcessServiceFactory();
        Reference< XPropertySet > xProps( xFactory, UNO_QUERY );
        OSL_ASSERT( xProps.is() );
        if (xProps.is())
        {
            xProps->getPropertyValue(
                OUString( RTL_CONSTASCII_USTRINGPARAM("DefaultContext") ) ) >>= xContext;
            OSL_ASSERT( xContext.is() );
        }
    }
    return xContext;
}

// CoreReflection statisch speichern
Reference< XIdlReflection > getCoreReflection_Impl( void )
{
    static Reference< XIdlReflection > xCoreReflection;

    // Haben wir schon CoreReflection, sonst besorgen
    if( !xCoreReflection.is() )
    {
        Reference< XComponentContext > xContext = getComponentContext_Impl();
        if( xContext.is() )
        {
            xContext->getValueByName(
                OUString( RTL_CONSTASCII_USTRINGPARAM("/singletons/com.sun.star.reflection.theCoreReflection") ) )
                    >>= xCoreReflection;
            OSL_ENSURE( xCoreReflection.is(), "### CoreReflection singleton not accessable!?" );
        }
        if( !xCoreReflection.is() )
        {
            throw DeploymentException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("/singletons/com.sun.star.reflection.theCoreReflection singleton not accessable") ),
                Reference< XInterface >() );
        }
    }
    return xCoreReflection;
}

// CoreReflection statisch speichern
Reference< XHierarchicalNameAccess > getCoreReflection_HierarchicalNameAccess_Impl( void )
{
    static Reference< XHierarchicalNameAccess > xCoreReflection_HierarchicalNameAccess;

    if( !xCoreReflection_HierarchicalNameAccess.is() )
    {
        Reference< XIdlReflection > xCoreReflection = getCoreReflection_Impl();
        if( xCoreReflection.is() )
        {
            xCoreReflection_HierarchicalNameAccess = 
                Reference< XHierarchicalNameAccess >( xCoreReflection, UNO_QUERY );
        }
    }
    return xCoreReflection_HierarchicalNameAccess;
}

// Hold TypeProvider statically
Reference< XHierarchicalNameAccess > getTypeProvider_Impl( void )
{
    static Reference< XHierarchicalNameAccess > xAccess;

    // Haben wir schon CoreReflection, sonst besorgen
    if( !xAccess.is() )
    {
        Reference< XComponentContext > xContext = getComponentContext_Impl();
        if( xContext.is() )
        {
            xContext->getValueByName(
                OUString( RTL_CONSTASCII_USTRINGPARAM("/singletons/com.sun.star.reflection.theTypeDescriptionManager") ) )
                    >>= xAccess;
            OSL_ENSURE( xAccess.is(), "### TypeDescriptionManager singleton not accessable!?" );
        }
        if( !xAccess.is() )
        {
            throw DeploymentException(
                OUString( RTL_CONSTASCII_USTRINGPARAM
                    ("/singletons/com.sun.star.reflection.theTypeDescriptionManager singleton not accessable") ),
                Reference< XInterface >() );
        }
    }
    return xAccess;
}

// #111851 factory function to create an OLE object
SbUnoObject* createOLEObject_Impl( const String& aType )
{
    static Reference< XMultiServiceFactory > xOLEFactory;
    static bool bNeedsInit = true;

    if( bNeedsInit )
    {
        bNeedsInit = false;

        Reference< XComponentContext > xContext = getComponentContext_Impl();
        if( xContext.is() )
        {
            Reference<XMultiComponentFactory> xSMgr = xContext->getServiceManager();
            xOLEFactory = Reference<XMultiServiceFactory>(
                xSMgr->createInstanceWithContext(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.bridge.OleObjectFactory")),
                        xContext ), UNO_QUERY );
        }
    }

    SbUnoObject* pUnoObj = NULL;
    if( xOLEFactory.is() )
    {
        Reference< XInterface > xOLEObject = xOLEFactory->createInstance( aType );
        if( xOLEObject.is() )
        {
            Any aAny;
            aAny <<= xOLEObject;
            pUnoObj = new SbUnoObject( aType, aAny );
        }
    }
    return pUnoObj;
}


String implGetExceptionMsg( Exception& e, const String& aExceptionType_ );

// #105565 Special Object to wrap a strongly typed Uno Any
TYPEINIT1(SbUnoAnyObject,SbxObject)

// TODO: Spaeter auslagern
Reference<XIdlClass> TypeToIdlClass( const Type& rType )
{
    // void als Default-Klasse eintragen
    Reference<XIdlClass> xRetClass;
    typelib_TypeDescription * pTD = 0;
    rType.getDescription( &pTD );

    if( pTD )
    {
        OUString sOWName( pTD->pTypeName );
        Reference< XIdlReflection > xRefl = getCoreReflection_Impl();
        xRetClass = xRefl->forName( sOWName );
    }
    return xRetClass;
}

// Fehlermeldungs-Message bei Exception zusammenbauen
String implGetExceptionMsg( Exception& e, const String& aExceptionType_ )
{
    String aExceptionType = aExceptionType_;
    if( aExceptionType.Len() == 0 )
        aExceptionType = String( RTL_CONSTASCII_USTRINGPARAM("Unknown" ) );

    String aTypeLine( RTL_CONSTASCII_USTRINGPARAM("\nType: " ) );
    aTypeLine += aExceptionType;

    String aMessageLine( RTL_CONSTASCII_USTRINGPARAM("\nMessage: " ) );
    aMessageLine += String( e.Message );

    String aMsg = aTypeLine;
    aMsg += aMessageLine;
    return aMsg;
}

// Exception type unknown
String implGetExceptionMsg( Exception& e )
{
    String aMsg = implGetExceptionMsg( e, String() );
    return aMsg;
}

// Error-Message fuer WrappedTargetExceptions
String implGetWrappedMsg( WrappedTargetException& e )
{
    String aMsg;
    Any aWrappedAny = e.TargetException;
    Type aExceptionType = aWrappedAny.getValueType();

    // Really an Exception?
    if( aExceptionType.getTypeClass() == TypeClass_EXCEPTION )
    {
        RuntimeException& e_ = *( (RuntimeException*)aWrappedAny.getValue() );
        aMsg = implGetExceptionMsg( e_, String( aExceptionType.getTypeName() ) );
    }
    // Otherwise use WrappedTargetException itself
    else
    {
        aMsg = implGetExceptionMsg( e );
    }

    return aMsg;
}

void implHandleBasicErrorException( BasicErrorException& e )
{
    SbError nError = StarBASIC::GetSfxFromVBError( (USHORT)e.ErrorCode );
    StarBASIC::Error( nError, e.ErrorMessageArgument );
}

void implHandleWrappedTargetException( WrappedTargetException& e )
{
    SbError nError;
    String aMessage;
    Any aWrappedAny = e.TargetException;
    if ( aWrappedAny.getValueType() == ::getCppuType( (BasicErrorException*)NULL ) )
    {
        BasicErrorException& be = *( (BasicErrorException*)aWrappedAny.getValue() );
        nError = StarBASIC::GetSfxFromVBError( (USHORT)be.ErrorCode );
        aMessage = be.ErrorMessageArgument;
    }
    else
    {
        nError = ERRCODE_BASIC_EXCEPTION;
        aMessage = implGetWrappedMsg( e );
    }
    StarBASIC::Error( nError, aMessage );
}

// Von Uno nach Sbx wandeln
SbxDataType unoToSbxType( TypeClass eType )
{
    SbxDataType eRetType = SbxVOID;

    switch( eType )
    {
        case TypeClass_INTERFACE:
        case TypeClass_TYPE:
        case TypeClass_STRUCT:
        case TypeClass_EXCEPTION:		eRetType = SbxOBJECT;	break;

        /* folgende Typen lassen wir erstmal weg
        case TypeClass_SERVICE:			break;
        case TypeClass_CLASS:			break;
        case TypeClass_TYPEDEF:			break;
        case TypeClass_UNION:			break;
        case TypeClass_ARRAY:			break;
        */
        case TypeClass_ENUM:			eRetType = SbxLONG;		break;
        case TypeClass_SEQUENCE:
            eRetType = (SbxDataType) ( SbxOBJECT | SbxARRAY );
            break;

        /*
        case TypeClass_VOID:			break;
        case TypeClass_UNKNOWN:			break;
        */

        case TypeClass_ANY:				eRetType = SbxVARIANT;	break;
        case TypeClass_BOOLEAN:			eRetType = SbxBOOL;		break;
        case TypeClass_CHAR:			eRetType = SbxCHAR;		break;
        case TypeClass_STRING:			eRetType = SbxSTRING;	break;
        case TypeClass_FLOAT:			eRetType = SbxSINGLE;	break;
        case TypeClass_DOUBLE:			eRetType = SbxDOUBLE;	break;
        //case TypeClass_OCTET:									break;
        case TypeClass_BYTE:			eRetType = SbxINTEGER;  break;
        //case TypeClass_INT:				eRetType = SbxINT;	break;
        case TypeClass_SHORT:			eRetType = SbxINTEGER;	break;
        case TypeClass_LONG:			eRetType = SbxLONG;		break;
        case TypeClass_HYPER:			eRetType = SbxSALINT64;	break;
        //case TypeClass_UNSIGNED_OCTET:						break;
        case TypeClass_UNSIGNED_SHORT:	eRetType = SbxUSHORT;	break;
        case TypeClass_UNSIGNED_LONG:	eRetType = SbxULONG;	break;
        case TypeClass_UNSIGNED_HYPER:  eRetType = SbxSALUINT64;break;
        //case TypeClass_UNSIGNED_INT:	eRetType = SbxUINT;		break;
        //case TypeClass_UNSIGNED_BYTE:	eRetType = SbxUSHORT;	break;
        default: break;
    }
    return eRetType;
}

SbxDataType unoToSbxType( const Reference< XIdlClass >& xIdlClass )
{
    SbxDataType eRetType = SbxVOID;
    if( xIdlClass.is() )
    {
        TypeClass eType = xIdlClass->getTypeClass();
        eRetType = unoToSbxType( eType );
    }
    return eRetType;
}
void unoToSbxValue( SbxVariable* pVar, const Any& aValue );
static void implSequenceToMultiDimArray( SbxDimArray*& pArray, Sequence< sal_Int32 >& indices, Sequence< sal_Int32 >& sizes, const Any& aValue, sal_Int32& dimension, sal_Bool bIsZeroIndex )
{
    Type aType = aValue.getValueType();
    TypeClass eTypeClass = aType.getTypeClass();

    sal_Int32 indicesIndex = indices.getLength() -1; 
    sal_Int32 dimCopy = dimension;

    if ( eTypeClass == TypeClass_SEQUENCE )
    {
        Reference< XIdlClass > xIdlTargetClass = TypeToIdlClass( aType );
        Reference< XIdlArray > xIdlArray = xIdlTargetClass->getArray();
        sal_Int32 nLen = xIdlArray->getLen( aValue );
        for ( sal_Int32 index = 0; index < nLen; ++index )
        {
            Any aElementAny = xIdlArray->get( aValue, (UINT32)index );
            // This detects the dimension were currently processing
            if ( dimCopy == dimension )
            {
                ++dimCopy;
                if ( sizes.getLength() < dimCopy )
                {
                    sizes.realloc( sizes.getLength() + 1 );	
                    sizes[ sizes.getLength() - 1 ] = nLen;
                    indices.realloc( indices.getLength() + 1 );
                    indicesIndex = indices.getLength() - 1;
                }
            }

            if ( bIsZeroIndex )
                indices[ dimCopy - 1 ] = index;
            else 
                indices[ dimCopy - 1] = index + 1;

            implSequenceToMultiDimArray( pArray, indices, sizes, aElementAny, dimCopy, bIsZeroIndex );
        }
        
    }
    else
    {
        if ( indices.getLength() < 1 ) 
        {
            // Should never ever get here ( indices.getLength()
            // should equal number of dimensions in the array ) 
            // And that should at least be 1 !
            // #QUESTION is there a better error?
            StarBASIC::Error( SbERR_INVALID_OBJECT );
            return;
        }

        if ( !pArray )
        {
            SbxDataType eSbxElementType = unoToSbxType( aValue.getValueTypeClass() );
            pArray = new SbxDimArray( eSbxElementType );
            sal_Int32 nIndexLen = indices.getLength();
        
            // Dimension the array
            for ( sal_Int32 index = 0; index < nIndexLen; ++index )
            {
                if ( bIsZeroIndex )
                    pArray->unoAddDim32( 0, sizes[ index ] - 1);
                else
                    pArray->unoAddDim32( 1, sizes[ index ] );

            }
        }

        if ( pArray )
        {
            SbxDataType eSbxElementType = unoToSbxType( aValue.getValueTypeClass() );
            SbxVariableRef xVar = new SbxVariable( eSbxElementType );
            unoToSbxValue( (SbxVariable*)xVar, aValue );

            sal_Int32* pIndices = indices.getArray();
            pArray->Put32( 	(SbxVariable*)xVar, pIndices );

        }
    }
}

void unoToSbxValue( SbxVariable* pVar, const Any& aValue )
{
    Type aType = aValue.getValueType();
    TypeClass eTypeClass = aType.getTypeClass();
    switch( eTypeClass )
    {
        case TypeClass_TYPE:
        {
            // Map Type to IdlClass
            Type aType_;
            aValue >>= aType_;
            Reference<XIdlClass> xClass = TypeToIdlClass( aType_ );
            Any aClassAny;
            aClassAny <<= xClass;

            // SbUnoObject instanzieren
            String aName;
            SbUnoObject* pSbUnoObject = new SbUnoObject( aName, aClassAny );
            SbxObjectRef xWrapper = (SbxObject*)pSbUnoObject;

            // #51475 Wenn das Objekt ungueltig ist null liefern
            if( pSbUnoObject->getUnoAny().getValueType().getTypeClass() == TypeClass_VOID )
            {
                pVar->PutObject( NULL );
            }
            else
            {
                pVar->PutObject( xWrapper );
            }
        }
        break;
        // Interfaces und Structs muessen in ein SbUnoObject gewrappt werden
        case TypeClass_INTERFACE:
        case TypeClass_STRUCT:
        case TypeClass_EXCEPTION:
        {
            if( eTypeClass == TypeClass_STRUCT )
            {
                ArrayWrapper aWrap;
                if ( (aValue >>= aWrap) )
                {
                    SbxDimArray* pArray = NULL;
                    Sequence< sal_Int32 > indices;
                    Sequence< sal_Int32 > sizes;
                    sal_Int32 dimension = 0;
                    implSequenceToMultiDimArray( pArray, indices, sizes, aWrap.Array, dimension, aWrap.IsZeroIndex );
                    if ( pArray )
                    {
                        SbxDimArrayRef xArray = pArray;
                        USHORT nFlags = pVar->GetFlags();
                        pVar->ResetFlag( SBX_FIXED );
                        pVar->PutObject( (SbxDimArray*)xArray );
                        pVar->SetFlags( nFlags );
                    }
                    else
                        pVar->PutEmpty();
                    break;
                }
            }
            // SbUnoObject instanzieren
            String aName;
            SbUnoObject* pSbUnoObject = new SbUnoObject( aName, aValue );
            SbxObjectRef xWrapper = (SbxObject*)pSbUnoObject;

            // #51475 Wenn das Objekt ungueltig ist null liefern
            if( pSbUnoObject->getUnoAny().getValueType().getTypeClass() == TypeClass_VOID )
            {
                pVar->PutObject( NULL );
            }
            else
            {
                pVar->PutObject( xWrapper );
            }
        }
        break;

        /* folgende Typen lassen wir erstmal weg
        case TypeClass_SERVICE:			break;
        case TypeClass_CLASS:			break;
        case TypeClass_TYPEDEF:			break;
        case TypeClass_UNION:			break;
        case TypeClass_ENUM:			break;
        case TypeClass_ARRAY:			break;
        */

        case TypeClass_ENUM:
        {
            sal_Int32 nEnum = 0;
            enum2int( nEnum, aValue );
            pVar->PutLong( nEnum );
        }
            break;

        case TypeClass_SEQUENCE:
        {
            Reference< XIdlClass > xIdlTargetClass = TypeToIdlClass( aType );
            Reference< XIdlArray > xIdlArray = xIdlTargetClass->getArray();
            sal_Int32 i, nLen = xIdlArray->getLen( aValue );

            typelib_TypeDescription * pTD = 0;
            aType.getDescription( &pTD );
            OSL_ASSERT( pTD && pTD->eTypeClass == typelib_TypeClass_SEQUENCE );
            Type aElementType( ((typelib_IndirectTypeDescription *)pTD)->pType );
            ::typelib_typedescription_release( pTD );

            // In Basic Array anlegen
            SbxDimArrayRef xArray;
            SbxDataType eSbxElementType = unoToSbxType( aElementType.getTypeClass() );
            xArray = new SbxDimArray( eSbxElementType );
            if( nLen > 0 )
            {
                xArray->unoAddDim32( 0, nLen - 1 );

                // Elemente als Variablen eintragen
                for( i = 0 ; i < nLen ; i++ )
                {
                    // Elemente wandeln
                    Any aElementAny = xIdlArray->get( aValue, (UINT32)i );
                    SbxVariableRef xVar = new SbxVariable( eSbxElementType );
                    unoToSbxValue( (SbxVariable*)xVar, aElementAny );

                    // Ins Array braten
                    xArray->Put32( (SbxVariable*)xVar, &i );
                }
            }
            else
            {
                xArray->unoAddDim( 0, -1 );
            }

            // Array zurueckliefern
            USHORT nFlags = pVar->GetFlags();
            pVar->ResetFlag( SBX_FIXED );
            pVar->PutObject( (SbxDimArray*)xArray );
            pVar->SetFlags( nFlags );

            // #54548, Die Parameter duerfen hier nicht weggehauen werden
            //pVar->SetParameters( NULL );
        }
        break;

        /*
        case TypeClass_VOID:			break;
        case TypeClass_UNKNOWN:			break;

        case TypeClass_ANY:
        {
            // Any rausholen und konvertieren
            //Any* pAny = (Any*)aValue.get();
            //if( pAny )
                //unoToSbxValue( pVar, *pAny );
        }
        break;
        */

        case TypeClass_BOOLEAN:			pVar->PutBool( *(sal_Bool*)aValue.getValue() );	break;
        case TypeClass_CHAR:
        {
            pVar->PutChar( *(sal_Unicode*)aValue.getValue() );
            break;
        }
        case TypeClass_STRING:			{ OUString val; aValue >>= val; pVar->PutString( String( val ) ); }	break;
        case TypeClass_FLOAT:			{ float val = 0; aValue >>= val; pVar->PutSingle( val ); } break;
        case TypeClass_DOUBLE:			{ double val = 0; aValue >>= val; pVar->PutDouble( val ); } break;
        //case TypeClass_OCTET:			break;
        case TypeClass_BYTE:			{ sal_Int8 val = 0; aValue >>= val; pVar->PutInteger( val ); } break;
        //case TypeClass_INT:			break;
        case TypeClass_SHORT:			{ sal_Int16 val = 0; aValue >>= val; pVar->PutInteger( val ); } break;
        case TypeClass_LONG:			{ sal_Int32 val = 0; aValue >>= val; pVar->PutLong( val ); } break;
        case TypeClass_HYPER:			{ sal_Int64 val = 0; aValue >>= val; pVar->PutInt64( val ); } break;
        //case TypeClass_UNSIGNED_OCTET:break;
        case TypeClass_UNSIGNED_SHORT:	{ sal_uInt16 val = 0; aValue >>= val; pVar->PutUShort( val ); } break;
        case TypeClass_UNSIGNED_LONG:	{ sal_uInt32 val = 0; aValue >>= val; pVar->PutULong( val ); } break;
        case TypeClass_UNSIGNED_HYPER:	{ sal_uInt64 val = 0; aValue >>= val; pVar->PutUInt64( val ); } break;
        //case TypeClass_UNSIGNED_INT:	break;
        //case TypeClass_UNSIGNED_BYTE:	break;
        default:						pVar->PutEmpty();						break;
    }
}

// Reflection fuer Sbx-Typen liefern
Type getUnoTypeForSbxBaseType( SbxDataType eType )
{
    Type aRetType = getCppuVoidType();
    switch( eType )
    {
        //case SbxEMPTY:		eRet = TypeClass_VOID; break;
        case SbxNULL:		aRetType = ::getCppuType( (const Reference< XInterface > *)0 ); break;
        case SbxINTEGER:	aRetType = ::getCppuType( (sal_Int16*)0 ); break;
        case SbxLONG:		aRetType = ::getCppuType( (sal_Int32*)0 ); break;
        case SbxSINGLE:		aRetType = ::getCppuType( (float*)0 ); break;
        case SbxDOUBLE:		aRetType = ::getCppuType( (double*)0 ); break;
        case SbxCURRENCY:	aRetType = ::getCppuType( (oleautomation::Currency*)0 ); break;
        case SbxDECIMAL:	aRetType = ::getCppuType( (oleautomation::Decimal*)0 ); break;
        case SbxDATE:		{
/*?*/ //							SbiInstance* pInst = pINST;
/*?*/ //							if( pInst && pInst->IsCompatibility() )
/*?*/ //								aRetType = ::getCppuType( (double*)0 );
/*?*/ //							else
                                aRetType = ::getCppuType( (oleautomation::Date*)0 );
                            }
                            break;
        // case SbxDATE:		aRetType = ::getCppuType( (double*)0 ); break;
        case SbxSTRING:		aRetType = ::getCppuType( (OUString*)0 ); break;
        //case SbxOBJECT:	break;
        //case SbxERROR:	break;
        case SbxBOOL:		aRetType = ::getCppuType( (sal_Bool*)0 ); break;
        case SbxVARIANT:	aRetType = ::getCppuType( (Any*)0 ); break;
        //case SbxDATAOBJECT: break;
        case SbxCHAR:		aRetType = ::getCppuType( (sal_Unicode*)0 ); break;
        case SbxBYTE:		aRetType = ::getCppuType( (sal_Int16*)0 ); break;
        case SbxUSHORT:		aRetType = ::getCppuType( (sal_uInt16*)0 ); break;
        case SbxULONG:		aRetType = ::getCppuType( (sal_uInt32*)0 ); break;
        //case SbxLONG64:	break;
        //case SbxULONG64:	break;
        // Maschinenabhaengige zur Sicherheit auf Hyper abbilden
        case SbxINT:		aRetType = ::getCppuType( (sal_Int32*)0 ); break;
        case SbxUINT:		aRetType = ::getCppuType( (sal_uInt32*)0 ); break;
        //case SbxVOID:		break;
        //case SbxHRESULT:	break;
        //case SbxPOINTER:	break;
        //case SbxDIMARRAY:	break;
        //case SbxCARRAY:	break;
        //case SbxUSERDEF:	break;
        //case SbxLPSTR:	break;
        //case SbxLPWSTR:	break;
        //case SbxCoreSTRING: break;
        default: break;
    }
    return aRetType;
}

// Konvertierung von Sbx nach Uno ohne bekannte Zielklasse fuer TypeClass_ANY
Type getUnoTypeForSbxValue( SbxValue* pVal )
{
    Type aRetType = getCppuVoidType();
    if( !pVal )
        return aRetType;

    // SbxType nach Uno wandeln
    SbxDataType eBaseType = pVal->SbxValue::GetType();
    if( eBaseType == SbxOBJECT )
    {
        SbxBaseRef xObj = (SbxBase*)pVal->GetObject();
        if( !xObj )
        {
            // #109936 No error any more
            // StarBASIC::Error( SbERR_INVALID_OBJECT );
            aRetType = getCppuType( static_cast<Reference<XInterface> *>(0) );
            return aRetType;
        }

        if( xObj->ISA(SbxDimArray) )
        {
            SbxBase* pObj = (SbxBase*)xObj;
            SbxDimArray* pArray = (SbxDimArray*)pObj;

            short nDims = pArray->GetDims();
            Type aElementType = getUnoTypeForSbxBaseType( (SbxDataType)(pArray->GetType() & 0xfff) );
            TypeClass eElementTypeClass = aElementType.getTypeClass();

            // Normal case: One dimensional array
            sal_Int32 nLower, nUpper;
            if( nDims == 1 && pArray->GetDim32( 1, nLower, nUpper ) )
            {
                if( eElementTypeClass == TypeClass_VOID || eElementTypeClass == TypeClass_ANY )
                {
                    // Wenn alle Elemente des Arrays vom gleichen Typ sind, wird
                    // der genommen, sonst wird das ganze als Any-Sequence betrachtet
                    sal_Bool bNeedsInit = sal_True;

                    INT32 nSize = nUpper - nLower + 1;
                    INT32 nIdx = nLower;
                    for( INT32 i = 0 ; i < nSize ; i++,nIdx++ )
                    {
                        SbxVariableRef xVar = pArray->Get32( &nIdx );
                        Type aType = getUnoTypeForSbxValue( (SbxVariable*)xVar );
                        if( bNeedsInit )
                        {
                            if( aType.getTypeClass() == TypeClass_VOID )
                            {
                                // #88522 
                                // if only first element is void: different types  -> []any
                                // if all elements are void: []void is not allowed -> []any
                                aElementType = getCppuType( (Any*)0 );
                                break;
                            }
                            aElementType = aType;
                            bNeedsInit = sal_False;
                        }
                        else if( aElementType != aType )
                        {
                            // Verschiedene Typen -> AnySequence
                            aElementType = getCppuType( (Any*)0 );
                            break;
                        }
                    }
                }

                OUString aSeqTypeName( aSeqLevelStr );
                aSeqTypeName += aElementType.getTypeName();
                aRetType = Type( TypeClass_SEQUENCE, aSeqTypeName );
            }
            // #i33795 Map also multi dimensional arrays to corresponding sequences
            else if( nDims > 1 )
            {
                if( eElementTypeClass == TypeClass_VOID || eElementTypeClass == TypeClass_ANY )
                {
                    // For this check the array's dim structure does not matter
                    UINT32 nFlatArraySize = pArray->Count32();

                    sal_Bool bNeedsInit = sal_True;
                    for( UINT32 i = 0 ; i < nFlatArraySize ; i++ )
                    {
                        SbxVariableRef xVar = pArray->SbxArray::Get32( i );
                        Type aType = getUnoTypeForSbxValue( (SbxVariable*)xVar );
                        if( bNeedsInit )
                        {
                            if( aType.getTypeClass() == TypeClass_VOID )
                            {
                                // if only first element is void: different types  -> []any
                                // if all elements are void: []void is not allowed -> []any
                                aElementType = getCppuType( (Any*)0 );
                                break;
                            }
                            aElementType = aType;
                            bNeedsInit = sal_False;
                        }
                        else if( aElementType != aType )
                        {
                            // Verschiedene Typen -> AnySequence
                            aElementType = getCppuType( (Any*)0 );
                            break;
                        }
                    }
                }

                OUString aSeqTypeName;
                for( short iDim = 0 ; iDim < nDims ; iDim++ )
                    aSeqTypeName += aSeqLevelStr;
                aSeqTypeName += aElementType.getTypeName();
                aRetType = Type( TypeClass_SEQUENCE, aSeqTypeName );
            }
        }
        // Kein Array, sondern...
        else if( xObj->ISA(SbUnoObject) )
        {
            aRetType = ((SbUnoObject*)(SbxBase*)xObj)->getUnoAny().getValueType();
        }
        // SbUnoAnyObject?
        else if( xObj->ISA(SbUnoAnyObject) )
        {
            aRetType = ((SbUnoAnyObject*)(SbxBase*)xObj)->getValue().getValueType();
        }
        // Sonst ist es ein Nicht-Uno-Basic-Objekt -> default==void liefern
    }
    // Kein Objekt, Basistyp konvertieren
    else
    {
        aRetType = getUnoTypeForSbxBaseType( eBaseType );
    }
    return aRetType;
}

// Deklaration Konvertierung von Sbx nach Uno mit bekannter Zielklasse
Any sbxToUnoValue( SbxVariable* pVar, const Type& rType, Property* pUnoProperty = NULL );

// Konvertierung von Sbx nach Uno ohne bekannte Zielklasse fuer TypeClass_ANY
Any sbxToUnoValueImpl( SbxVariable* pVar, bool bBlockConversionToSmallestType = false )
{
    SbxDataType eBaseType = pVar->SbxValue::GetType();
    if( eBaseType == SbxOBJECT )
    {
        SbxBaseRef xObj = (SbxBase*)pVar->GetObject();
        if( xObj.Is() && xObj->ISA(SbUnoAnyObject) )
            return ((SbUnoAnyObject*)(SbxBase*)xObj)->getValue();
    }

    Type aType = getUnoTypeForSbxValue( pVar );
    TypeClass eType = aType.getTypeClass();

    if( !bBlockConversionToSmallestType )
    {
        // #79615 Choose "smallest" represention for int values
        // because up cast is allowed, downcast not
        switch( eType )
        {
            case TypeClass_FLOAT:
            case TypeClass_DOUBLE:
            {
                double d = pVar->GetDouble();
                if( d == floor( d ) )
                {
                    if( d >= -128 && d <= 127 )
                        aType = ::getCppuType( (sal_Int8*)0 );
                    else if( d >= SbxMININT && d <= SbxMAXINT )
                        aType = ::getCppuType( (sal_Int16*)0 );
                    else if( d >= -SbxMAXLNG && d <= SbxMAXLNG )
                        aType = ::getCppuType( (sal_Int32*)0 );
                }
                break;
            }
            case TypeClass_SHORT:
            {
                sal_Int16 n = pVar->GetInteger();
                if( n >= -128 && n <= 127 )
                    aType = ::getCppuType( (sal_Int8*)0 );
                break;
            }
            case TypeClass_LONG:
            {
                sal_Int32 n = pVar->GetLong();
                if( n >= -128 && n <= 127 )
                    aType = ::getCppuType( (sal_Int8*)0 );
                else if( n >= SbxMININT && n <= SbxMAXINT )
                    aType = ::getCppuType( (sal_Int16*)0 );
                break;
            }
            case TypeClass_UNSIGNED_SHORT:
            {
                sal_uInt16 n = pVar->GetUShort();
                if( n <= 255 )
                    aType = ::getCppuType( (sal_uInt8*)0 );
                break;
            }
            case TypeClass_UNSIGNED_LONG:
            {
                sal_uInt32 n = pVar->GetLong();
                if( n <= 255 )
                    aType = ::getCppuType( (sal_uInt8*)0 );
                else if( n <= SbxMAXUINT )
                    aType = ::getCppuType( (sal_uInt16*)0 );
                break;
            }
            default: break;
        }
    }

    return sbxToUnoValue( pVar, aType );
}



// Helper function for StepREDIMP
static Any implRekMultiDimArrayToSequence( SbxDimArray* pArray, 
    const Type& aElemType, short nMaxDimIndex, short nActualDim, 
    sal_Int32* pActualIndices, sal_Int32* pLowerBounds, sal_Int32* pUpperBounds )
{
    sal_Int32 nSeqLevel = nMaxDimIndex - nActualDim + 1;
    OUString aSeqTypeName;
    sal_Int32 i;
    for( i = 0 ; i < nSeqLevel ; i++ )
        aSeqTypeName += aSeqLevelStr;

    aSeqTypeName += aElemType.getTypeName();
    Type aSeqType( TypeClass_SEQUENCE, aSeqTypeName );

    // Create Sequence instance
    Any aRetVal;
    Reference< XIdlClass > xIdlTargetClass = TypeToIdlClass( aSeqType );
    xIdlTargetClass->createObject( aRetVal );

    // Alloc sequence according to array bounds
    sal_Int32 nUpper = pUpperBounds[nActualDim];
    sal_Int32 nLower = pLowerBounds[nActualDim];
    sal_Int32 nSeqSize = nUpper - nLower + 1;
    Reference< XIdlArray > xArray = xIdlTargetClass->getArray();
    xArray->realloc( aRetVal, nSeqSize );

    sal_Int32& ri = pActualIndices[nActualDim];
    
    for( ri = nLower,i = 0 ; ri <= nUpper ; ri++,i++ )
    {
        Any aElementVal;

        if( nActualDim < nMaxDimIndex )
        {
            aElementVal = implRekMultiDimArrayToSequence( pArray, aElemType,
                nMaxDimIndex, nActualDim + 1, pActualIndices, pLowerBounds, pUpperBounds );
        }
        else
        {
            SbxVariable* pSource = pArray->Get32( pActualIndices );
            aElementVal = sbxToUnoValue( pSource, aElemType );
        }

        try
        {
            // In die Sequence uebernehmen
            xArray->set( aRetVal, i, aElementVal );
        }
        catch( IllegalArgumentException& e1 )
        {
            StarBASIC::Error( ERRCODE_BASIC_EXCEPTION, 
                implGetExceptionMsg( e1, aIllegalArgumentExceptionName ) );
        }
        catch (IndexOutOfBoundsException&)
        {
            StarBASIC::Error( SbERR_OUT_OF_RANGE );
        }
    }
    return aRetVal;
}

// Konvertierung von Sbx nach Uno mit bekannter Zielklasse
Any sbxToUnoValue( SbxVariable* pVar, const Type& rType, Property* pUnoProperty )
{
    Any aRetVal;

    // #94560 No conversion of empty/void for MAYBE_VOID properties
    if( pUnoProperty && pUnoProperty->Attributes & PropertyAttribute::MAYBEVOID )
    {
        if( pVar->IsEmpty() )
            return aRetVal;
    }

    SbxDataType eBaseType = pVar->SbxValue::GetType();
    if( eBaseType == SbxOBJECT )
    {
        SbxBaseRef xObj = (SbxBase*)pVar->GetObject();
        if( xObj.Is() && xObj->ISA(SbUnoAnyObject) )
        {
            return ((SbUnoAnyObject*)(SbxBase*)xObj)->getValue();
        }
    }

    TypeClass eType = rType.getTypeClass();
    switch( eType )
    {
        case TypeClass_INTERFACE:
        case TypeClass_STRUCT:
        case TypeClass_EXCEPTION:
        {
            Reference< XIdlClass > xIdlTargetClass = TypeToIdlClass( rType );

            // Null-Referenz?
            if( pVar->IsNull() && eType == TypeClass_INTERFACE )
            {
                Reference< XInterface > xRef;
                OUString aClassName = xIdlTargetClass->getName();
                Type aClassType( xIdlTargetClass->getTypeClass(), aClassName.getStr() );
                aRetVal.setValue( &xRef, aClassType );
            }
        }
        break;

        /* folgende Typen lassen wir erstmal weg
        case TypeClass_SERVICE:			break;
        case TypeClass_CLASS:			break;
        case TypeClass_TYPEDEF:			break;
        case TypeClass_UNION:			break;
        case TypeClass_ENUM:			break;
        case TypeClass_ARRAY:			break;
        */

        // Array -> Sequence
        case TypeClass_ENUM:
        {
            aRetVal = int2enum( pVar->GetLong(), rType );
        }
        break;

        case TypeClass_SEQUENCE:
        {
            SbxBaseRef xObj = (SbxBase*)pVar->GetObject();
            if( xObj && xObj->ISA(SbxDimArray) )
            {
                SbxBase* pObj = (SbxBase*)xObj;
                SbxDimArray* pArray = (SbxDimArray*)pObj;

                short nDims = pArray->GetDims();

                // Normal case: One dimensional array
                sal_Int32 nLower, nUpper;
                if( nDims == 1 && pArray->GetDim32( 1, nLower, nUpper ) )
                {
                    sal_Int32 nSeqSize = nUpper - nLower + 1;

                    // Instanz der geforderten Sequence erzeugen
                    Reference< XIdlClass > xIdlTargetClass = TypeToIdlClass( rType );
                    xIdlTargetClass->createObject( aRetVal );
                    Reference< XIdlArray > xArray = xIdlTargetClass->getArray();
                    xArray->realloc( aRetVal, nSeqSize );

                    // Element-Type
                    OUString aClassName = xIdlTargetClass->getName();
                    typelib_TypeDescription * pSeqTD = 0;
                    typelib_typedescription_getByName( &pSeqTD, aClassName.pData );
                    OSL_ASSERT( pSeqTD );
                    Type aElemType( ((typelib_IndirectTypeDescription *)pSeqTD)->pType );
                    // Reference< XIdlClass > xElementClass = TypeToIdlClass( aElemType );

                    // Alle Array-Member umwandeln und eintragen
                    sal_Int32 nIdx = nLower;
                    for( sal_Int32 i = 0 ; i < nSeqSize ; i++,nIdx++ )
                    {
                        SbxVariableRef xVar = pArray->Get32( &nIdx );

                        // Wert von Sbx nach Uno wandeln
                        Any aAnyValue = sbxToUnoValue( (SbxVariable*)xVar, aElemType );

                        try
                        {
                            // In die Sequence uebernehmen
                            xArray->set( aRetVal, i, aAnyValue );
                        }
                        catch( IllegalArgumentException& e1 )
                        {
                            StarBASIC::Error( ERRCODE_BASIC_EXCEPTION,
                                implGetExceptionMsg( e1, aIllegalArgumentExceptionName ) );
                        }
                        catch (IndexOutOfBoundsException&)
                        {
                            StarBASIC::Error( SbERR_OUT_OF_RANGE );
                        }
                    }
                }
                // #i33795 Map also multi dimensional arrays to corresponding sequences
                else if( nDims > 1 )
                {
                    // Element-Type
                    typelib_TypeDescription * pSeqTD = 0;
                    Type aCurType( rType );
                    sal_Int32 nSeqLevel = 0;
                    Type aElemType;
                    do
                    {
                        OUString aTypeName = aCurType.getTypeName();
                        typelib_typedescription_getByName( &pSeqTD, aTypeName.pData );
                        OSL_ASSERT( pSeqTD );
                        if( pSeqTD->eTypeClass == typelib_TypeClass_SEQUENCE )
                        {
                            aCurType = Type( ((typelib_IndirectTypeDescription *)pSeqTD)->pType );
                            nSeqLevel++;
                        }
                        else
                        {
                            aElemType = aCurType;
                            break;
                        }
                    }
                    while( true );

                    if( nSeqLevel == nDims )
                    {
                        sal_Int32* pLowerBounds = new sal_Int32[nDims];
                        sal_Int32* pUpperBounds = new sal_Int32[nDims];
                        sal_Int32* pActualIndices = new sal_Int32[nDims];
                        for( short i = 1 ; i <= nDims ; i++ )
                        {
                            sal_Int32 lBound, uBound;
                            pArray->GetDim32( i, lBound, uBound );

                            short j = i - 1;
                            pActualIndices[j] = pLowerBounds[j] = lBound;
                            pUpperBounds[j] = uBound;
                        }

                        aRetVal = implRekMultiDimArrayToSequence( pArray, aElemType, 
                            nDims - 1, 0, pActualIndices, pLowerBounds, pUpperBounds );

                        delete[] pUpperBounds;
                        delete[] pLowerBounds;
                        delete[] pActualIndices;
                    }
                }
            }
        }
        break;

        /*
        case TypeClass_VOID:			break;
        case TypeClass_UNKNOWN:			break;
        */

        // Bei Any die Klassen-unabhaengige Konvertierungs-Routine nutzen
        case TypeClass_ANY:
        {
            aRetVal = sbxToUnoValueImpl( pVar );
        }
        break;

        case TypeClass_BOOLEAN:
        {
            sal_Bool b = pVar->GetBool();
            aRetVal.setValue( &b, getBooleanCppuType() );
            break;
        }
        case TypeClass_CHAR:
        {
            sal_Unicode c = pVar->GetChar();
            aRetVal.setValue( &c , getCharCppuType() );
            break;
        }
        case TypeClass_STRING:			aRetVal <<= OUString( pVar->GetString() ); break;
        case TypeClass_FLOAT:			aRetVal <<= pVar->GetSingle(); break;
        case TypeClass_DOUBLE:			aRetVal <<= pVar->GetDouble(); break;
        //case TypeClass_OCTET:			break;

        case TypeClass_BYTE:
        {
            sal_Int16 nVal = pVar->GetInteger();
            sal_Bool bOverflow = sal_False;
            if( nVal < -128 )
            {
                bOverflow = sal_True;
                nVal = -128;
            }
            else if( nVal > 127 )
            {
                bOverflow = sal_True;
                nVal = 127;
            }
            if( bOverflow )
                   StarBASIC::Error( ERRCODE_BASIC_MATH_OVERFLOW );

            sal_Int8 nByteVal = (sal_Int8)nVal;
            aRetVal <<= nByteVal;
            break;
        }
        //case TypeClass_INT:			break;
        case TypeClass_SHORT:			aRetVal <<= (sal_Int16)( pVar->GetInteger() );	break;
        case TypeClass_LONG:			aRetVal <<= (sal_Int32)( pVar->GetLong() );     break;
        case TypeClass_HYPER:			aRetVal <<= (sal_Int64)( pVar->GetInt64() );    break;
        //case TypeClass_UNSIGNED_OCTET:break;
        case TypeClass_UNSIGNED_SHORT:	aRetVal <<= (sal_uInt16)( pVar->GetUShort() );	break;
        case TypeClass_UNSIGNED_LONG:	aRetVal <<= (sal_uInt32)( pVar->GetULong() );	break;
        case TypeClass_UNSIGNED_HYPER:  aRetVal <<= (sal_uInt64)( pVar->GetUInt64() );  break;
        //case TypeClass_UNSIGNED_INT:	break;
        //case TypeClass_UNSIGNED_BYTE:	break;
        default: break;
    }

    return aRetVal;
}

// Dbg-Hilfsmethode zum Auslesen der in einem Object implementierten Interfaces
String Impl_GetInterfaceInfo( const Reference< XInterface >& x, const Reference< XIdlClass >& xClass, USHORT nRekLevel )
{
    Type aIfaceType = ::getCppuType( (const Reference< XInterface > *)0 );
    static Reference< XIdlClass > xIfaceClass = TypeToIdlClass( aIfaceType );

    String aRetStr;
    for( USHORT i = 0 ; i < nRekLevel ; i++ )
        aRetStr.AppendAscii( "    " );
    aRetStr += String( xClass->getName() );
    OUString aClassName = xClass->getName();
    Type aClassType( xClass->getTypeClass(), aClassName.getStr() );

    // Pruefen, ob das Interface wirklich unterstuetzt wird
    if( !x->queryInterface( aClassType ).hasValue() )
    {
        aRetStr.AppendAscii( " (ERROR: Not really supported!)\n" );
    }
    // Gibt es Super-Interfaces
    else
    {
        aRetStr.AppendAscii( "\n" );

        // Super-Interfaces holen
        Sequence< Reference< XIdlClass > > aSuperClassSeq = xClass->getSuperclasses();
        const Reference< XIdlClass >* pClasses = aSuperClassSeq.getConstArray();
        UINT32 nSuperIfaceCount = aSuperClassSeq.getLength();
        for( UINT32 j = 0 ; j < nSuperIfaceCount ; j++ )
        {
            const Reference< XIdlClass >& rxIfaceClass = pClasses[j];
            if( !rxIfaceClass->equals( xIfaceClass ) )
                aRetStr += Impl_GetInterfaceInfo( x, rxIfaceClass, nRekLevel + 1 );
        }
    }
    return aRetStr;
}

String getDbgObjectNameImpl( SbUnoObject* pUnoObj )
{
    String aName;
    if( pUnoObj )
    {
        aName = pUnoObj->GetClassName();
        if( !aName.Len() )
        {
            Any aToInspectObj = pUnoObj->getUnoAny();
            TypeClass eType = aToInspectObj.getValueType().getTypeClass();
            Reference< XInterface > xObj;
            if( eType == TypeClass_INTERFACE )
                xObj = *(Reference< XInterface >*)aToInspectObj.getValue();
            if( xObj.is() )
            {
                Reference< XServiceInfo > xServiceInfo( xObj, UNO_QUERY );
                if( xServiceInfo.is() )
                    aName = xServiceInfo->getImplementationName();
            }
        }
    }
    return aName;
}

String getDbgObjectName( SbUnoObject* pUnoObj )
{
    String aName = getDbgObjectNameImpl( pUnoObj );
    if( !aName.Len() )
        aName.AppendAscii( "Unknown" );

    String aRet;
    if( aName.Len() > 20 )
        aRet.AppendAscii( "\n" );
    aRet.AppendAscii( "\"" );
    aRet += aName;
    aRet.AppendAscii( "\":" );
    return aRet;
}

// Dbg-Hilfsmethode zum Auslesen der in einem Object implementierten Interfaces
String Impl_GetSupportedInterfaces( SbUnoObject* pUnoObj )
{
    Any aToInspectObj = pUnoObj->getUnoAny();

    // #54898: Nur TypeClass Interface zulasssen
    TypeClass eType = aToInspectObj.getValueType().getTypeClass();
    String aRet;
    if( eType != TypeClass_INTERFACE )
    {
        aRet += ID_DBG_SUPPORTEDINTERFACES;
        aRet.AppendAscii( " not available.\n(TypeClass is not TypeClass_INTERFACE)\n" );
    }
    else
    {
        // Interface aus dem Any besorgen
        const Reference< XInterface > x = *(Reference< XInterface >*)aToInspectObj.getValue();

        // XIdlClassProvider-Interface ansprechen
        Reference< XIdlClassProvider > xClassProvider( x, UNO_QUERY );
        Reference< XTypeProvider > xTypeProvider( x, UNO_QUERY );

        aRet.AssignAscii( "Supported interfaces by object " );
        String aObjName = getDbgObjectName( pUnoObj );
        aRet += aObjName;
        aRet.AppendAscii( "\n" );
        if( xTypeProvider.is() )
        {
            // Interfaces der Implementation holen
            Sequence< Type > aTypeSeq = xTypeProvider->getTypes();
            const Type* pTypeArray = aTypeSeq.getConstArray();
            UINT32 nIfaceCount = aTypeSeq.getLength();
            for( UINT32 j = 0 ; j < nIfaceCount ; j++ )
            {
                const Type& rType = pTypeArray[j];

                Reference<XIdlClass> xClass = TypeToIdlClass( rType );
                if( xClass.is() )
                {
                    aRet += Impl_GetInterfaceInfo( x, xClass, 1 );
                }
                else
                {
                    typelib_TypeDescription * pTD = 0;
                    rType.getDescription( &pTD );
                    String TypeName( OUString( pTD->pTypeName ) );

                    aRet.AppendAscii( "*** ERROR: No IdlClass for type \"" );
                    aRet += TypeName;
                    aRet.AppendAscii( "\"\n*** Please check type library\n" );
                }
            }
        }
        else if( xClassProvider.is() )
        {

            DBG_ERROR( "XClassProvider not supported in UNO3" );
        }
    }
    return aRet;
}



// Dbg-Hilfsmethode SbxDataType -> String
String Dbg_SbxDataType2String( SbxDataType eType )
{
    String aRet( RTL_CONSTASCII_USTRINGPARAM("Unknown Sbx-Type!") );
    switch( +eType )
    {
        case SbxEMPTY:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxEMPTY") ); break;
        case SbxNULL:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxNULL") ); break;
        case SbxINTEGER:	aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxINTEGER") ); break;
        case SbxLONG:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxLONG") ); break;
        case SbxSINGLE:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxSINGLE") ); break;
        case SbxDOUBLE:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxDOUBLE") ); break;
        case SbxCURRENCY:	aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxCURRENCY") ); break;
        case SbxDECIMAL:	aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxDECIMAL") ); break;
        case SbxDATE:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxDATE") ); break;
        case SbxSTRING:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxSTRING") ); break;
        case SbxOBJECT:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxOBJECT") ); break;
        case SbxERROR:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxERROR") ); break;
        case SbxBOOL:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxBOOL") ); break;
        case SbxVARIANT:	aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxVARIANT") ); break;
        case SbxDATAOBJECT: aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxDATAOBJECT") ); break;
        case SbxCHAR:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxCHAR") ); break;
        case SbxBYTE:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxBYTE") ); break;
        case SbxUSHORT:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxUSHORT") ); break;
        case SbxULONG:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxULONG") ); break;
        case SbxLONG64:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxLONG64") ); break;
        case SbxULONG64:	aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxULONG64") ); break;
        case SbxSALINT64:	aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxINT64") ); break;
        case SbxSALUINT64:	aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxUINT64") ); break;
        case SbxINT:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxINT") ); break;
        case SbxUINT:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxUINT") ); break;
        case SbxVOID:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxVOID") ); break;
        case SbxHRESULT:	aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxHRESULT") ); break;
        case SbxPOINTER:	aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxPOINTER") ); break;
        case SbxDIMARRAY:	aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxDIMARRAY") ); break;
        case SbxCARRAY:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxCARRAY") ); break;
        case SbxUSERDEF:	aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxUSERDEF") ); break;
        case SbxLPSTR:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxLPSTR") ); break;
        case SbxLPWSTR:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxLPWSTR") ); break;
        case SbxCoreSTRING: aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxCoreSTRING" ) ); break;
        case SbxOBJECT | SbxARRAY: aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxARRAY") ); break;
        default: break;
    }
    return aRet;
}

// Dbg-Hilfsmethode zum Anzeigen der Properties eines SbUnoObjects
String Impl_DumpProperties( SbUnoObject* pUnoObj )
{
    String aRet( RTL_CONSTASCII_USTRINGPARAM("Properties of object ") );
    String aObjName = getDbgObjectName( pUnoObj );
    aRet += aObjName;

    // Uno-Infos auswerten, um Arrays zu erkennen
    Reference< XIntrospectionAccess > xAccess = pUnoObj->getIntrospectionAccess();
    if( !xAccess.is() )
    {
        Reference< XInvocation > xInvok = pUnoObj->getInvocation();
        if( xInvok.is() )
            xAccess = xInvok->getIntrospection();
    }
    if( !xAccess.is() )
    {
        aRet.AppendAscii( "\nUnknown, no introspection available\n" );
        return aRet;
    }

    Sequence<Property> props = xAccess->getProperties( PropertyConcept::ALL - PropertyConcept::DANGEROUS );
    UINT32 nUnoPropCount = props.getLength();
    const Property* pUnoProps = props.getConstArray();

    SbxArray* pProps = pUnoObj->GetProperties();
    USHORT nPropCount = pProps->Count();
    USHORT nPropsPerLine = 1 + nPropCount / 30;
    for( USHORT i = 0; i < nPropCount; i++ )
    {
        SbxVariable* pVar = pProps->Get( i );
        if( pVar )
        {
            String aPropStr;
            if( (i % nPropsPerLine) == 0 )
                aPropStr.AppendAscii( "\n" );

            // Typ und Namen ausgeben
            // Ist es in Uno eine Sequence?
            SbxDataType eType = pVar->GetFullType();

            BOOL bMaybeVoid = FALSE;
            if( i < nUnoPropCount )
            {
                const Property& rProp = pUnoProps[ i ];

                // #63133: Bei MAYBEVOID Typ aus Uno neu konvertieren,
                // damit nicht immer nur SbxEMPTY ausgegben wird.
                if( rProp.Attributes & PropertyAttribute::MAYBEVOID )
                {
                    eType = unoToSbxType( rProp.Type.getTypeClass() );
                    bMaybeVoid = TRUE;
                }
                if( eType == SbxOBJECT )
                {
                    Type aType = rProp.Type;
                    if( aType.getTypeClass() == TypeClass_SEQUENCE )
                        eType = (SbxDataType) ( SbxOBJECT | SbxARRAY );
                }
            }
            aPropStr += Dbg_SbxDataType2String( eType );
            if( bMaybeVoid )
                aPropStr.AppendAscii( "/void" );
            aPropStr.AppendAscii( " " );
            aPropStr += pVar->GetName();

            if( i == nPropCount - 1 )
                aPropStr.AppendAscii( "\n" );
            else
                aPropStr.AppendAscii( "; " );

            aRet += aPropStr;
        }
    }
    return aRet;
}

// Dbg-Hilfsmethode zum Anzeigen der Methoden eines SbUnoObjects
String Impl_DumpMethods( SbUnoObject* pUnoObj )
{
    String aRet( RTL_CONSTASCII_USTRINGPARAM("Methods of object ") );
    String aObjName = getDbgObjectName( pUnoObj );
    aRet += aObjName;

    // XIntrospectionAccess, um die Typen der Parameter auch ausgeben zu koennen
    Reference< XIntrospectionAccess > xAccess = pUnoObj->getIntrospectionAccess();
    if( !xAccess.is() )
    {
        Reference< XInvocation > xInvok = pUnoObj->getInvocation();
        if( xInvok.is() )
            xAccess = xInvok->getIntrospection();
    }
    if( !xAccess.is() )
    {
        aRet.AppendAscii( "\nUnknown, no introspection available\n" );
        return aRet;
    }
    Sequence< Reference< XIdlMethod > > methods = xAccess->getMethods
        ( MethodConcept::ALL - MethodConcept::DANGEROUS );
    const Reference< XIdlMethod >* pUnoMethods = methods.getConstArray();

    SbxArray* pMethods = pUnoObj->GetMethods();
    USHORT nMethodCount = pMethods->Count();
    if( !nMethodCount )
    {
        aRet.AppendAscii( "\nNo methods found\n" );
        return aRet;
    }
    USHORT nPropsPerLine = 1 + nMethodCount / 30;
    for( USHORT i = 0; i < nMethodCount; i++ )
    {
        SbxVariable* pVar = pMethods->Get( i );
        if( pVar )
        {
            String aPropStr;
            if( (i % nPropsPerLine) == 0 )
                aPropStr.AppendAscii( "\n" );

            // Methode ansprechen
            const Reference< XIdlMethod >& rxMethod = pUnoMethods[i];

            // Ist es in Uno eine Sequence?
            SbxDataType eType = pVar->GetFullType();
            if( eType == SbxOBJECT )
            {
                Reference< XIdlClass > xClass = rxMethod->getReturnType();
                if( xClass.is() && xClass->getTypeClass() == TypeClass_SEQUENCE )
                    eType = (SbxDataType) ( SbxOBJECT | SbxARRAY );
            }
            // Name und Typ ausgeben
            aPropStr += Dbg_SbxDataType2String( eType );
            aPropStr.AppendAscii( " " );
            aPropStr += pVar->GetName();
            aPropStr.AppendAscii( " ( " );

            // get-Methode darf keinen Parameter haben
            Sequence< Reference< XIdlClass > > aParamsSeq = rxMethod->getParameterTypes();
            UINT32 nParamCount = aParamsSeq.getLength();
            const Reference< XIdlClass >* pParams = aParamsSeq.getConstArray();

            if( nParamCount > 0 )
            {
                for( USHORT j = 0; j < nParamCount; j++ )
                {
                    String aTypeStr = Dbg_SbxDataType2String( unoToSbxType( pParams[ j ] ) );
                    aPropStr += aTypeStr;

                    if( j < nParamCount - 1 )
                        aPropStr.AppendAscii( ", " );
                }
            }
            else
                aPropStr.AppendAscii( "void" );

            aPropStr.AppendAscii( " ) " );

            if( i == nMethodCount - 1 )
                aPropStr.AppendAscii( "\n" );
            else
                aPropStr.AppendAscii( "; " );

            aRet += aPropStr;
        }
    }
    return aRet;
}

TYPEINIT1(AutomationNamedArgsSbxArray,SbxArray)

// Implementation SbUnoObject
void SbUnoObject::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                           const SfxHint& rHint, const TypeId& rHintType )
{
    if( bNeedIntrospection )
        doIntrospection();

    const SbxHint* pHint = PTR_CAST(SbxHint,&rHint);
    if( pHint )
    {
        SbxVariable* pVar = pHint->GetVar();
        SbxArray* pParams = pVar->GetParameters();
        SbUnoProperty* pProp = PTR_CAST(SbUnoProperty,pVar);
        SbUnoMethod* pMeth = PTR_CAST(SbUnoMethod,pVar);
        if( pProp )
        {
            bool bInvocation = pProp->isInvocationBased();
            if( pHint->GetId() == SBX_HINT_DATAWANTED )
            {
                // Test-Properties
                INT32 nId = pProp->nId;
                if( nId < 0 )
                {
                    // Id == -1: Implementierte Interfaces gemaess ClassProvider anzeigen
                    if( nId == -1 )		// Property ID_DBG_SUPPORTEDINTERFACES"
                    {
                        String aRetStr = Impl_GetSupportedInterfaces( this );
                        pVar->PutString( aRetStr );
                    }
                    // Id == -2: Properties ausgeben
                    else if( nId == -2 )		// Property ID_DBG_PROPERTIES
                    {
                        // Jetzt muessen alle Properties angelegt werden
                        implCreateAll();
                        String aRetStr = Impl_DumpProperties( this );
                        pVar->PutString( aRetStr );
                    }
                    // Id == -3: Methoden ausgeben
                    else if( nId == -3 )		// Property ID_DBG_METHODS
                    {
                        // Jetzt muessen alle Properties angelegt werden
                        implCreateAll();
                        String aRetStr = Impl_DumpMethods( this );
                        pVar->PutString( aRetStr );
                    }
                    return;
                }

                if( !bInvocation && mxUnoAccess.is() )
                {
                    try
                    {
                        // Wert holen
                        Reference< XPropertySet > xPropSet( mxUnoAccess->queryAdapter( ::getCppuType( (const Reference< XPropertySet > *)0 ) ), UNO_QUERY );
                        Any aRetAny = xPropSet->getPropertyValue( pProp->GetName() );
                        // Die Nutzung von getPropertyValue (statt ueber den Index zu gehen) ist
                        // nicht optimal, aber die Umstellung auf XInvocation steht ja ohnehin an
                        // Ansonsten kann auch FastPropertySet genutzt werden

                        // Wert von Uno nach Sbx uebernehmen
                        unoToSbxValue( pVar, aRetAny );
                    }
                    catch( BasicErrorException& e0 )
                    {
                        implHandleBasicErrorException( e0 );
                    }
                    catch( WrappedTargetException& e1 )
                    {
                        implHandleWrappedTargetException( e1 );
                    }
                    catch( RuntimeException& e2 )
                    {
                        StarBASIC::Error( ERRCODE_BASIC_EXCEPTION, implGetExceptionMsg( e2 ) );
                    }
                    catch( Exception& e3 )
                    {
                        StarBASIC::Error( ERRCODE_BASIC_EXCEPTION, implGetExceptionMsg( e3 ) );
                    }
                }
                else if( bInvocation && mxInvocation.is() )
                {
                    try
                    {
                        // Wert holen
                        Any aRetAny = mxInvocation->getValue( pProp->GetName() );

                        // Wert von Uno nach Sbx uebernehmen
                        unoToSbxValue( pVar, aRetAny );
                    }
                    catch( BasicErrorException& e0 )
                    {
                        implHandleBasicErrorException( e0 );
                    }
                    catch( WrappedTargetException& e1 )
                    {
                        implHandleWrappedTargetException( e1 );
                    }
                    catch( RuntimeException& e2 )
                    {
                        StarBASIC::Error( ERRCODE_BASIC_EXCEPTION, implGetExceptionMsg( e2 ) );
                    }
                    catch( Exception& e3 )
                    {
                        StarBASIC::Error( ERRCODE_BASIC_EXCEPTION, implGetExceptionMsg( e3 ) );
                    }
                }
            }
            else if( pHint->GetId() == SBX_HINT_DATACHANGED )
            {
                if( !bInvocation && mxUnoAccess.is() )
                {
                    if( pProp->aUnoProp.Attributes & PropertyAttribute::READONLY )
                    {
                        StarBASIC::Error( SbERR_PROP_READONLY );
                        return;
                    }

                    // Wert von Uno nach Sbx uebernehmen
                    Any aAnyValue = sbxToUnoValue( pVar, pProp->aUnoProp.Type, &pProp->aUnoProp );
                    try
                    {
                        // Wert setzen
                        Reference< XPropertySet > xPropSet( mxUnoAccess->queryAdapter( ::getCppuType( (const Reference< XPropertySet > *)0 ) ), UNO_QUERY );
                        xPropSet->setPropertyValue( pProp->GetName(), aAnyValue );
                        // Die Nutzung von getPropertyValue (statt ueber den Index zu gehen) ist
                        // nicht optimal, aber die Umstellung auf XInvocation steht ja ohnehin an
                        // Ansonsten kann auch FastPropertySet genutzt werden
                    }
                    catch( BasicErrorException& e0 )
                    {
                        implHandleBasicErrorException( e0 );
                    }
                    catch( WrappedTargetException& e1 )
                    {
                        implHandleWrappedTargetException( e1 );
                    }
                    catch( IllegalArgumentException& e2 )
                    {
                        StarBASIC::Error( ERRCODE_BASIC_EXCEPTION,
                            implGetExceptionMsg( e2, aIllegalArgumentExceptionName ) );
                    }
                    catch( RuntimeException& e3 )
                    {
                        StarBASIC::Error( ERRCODE_BASIC_EXCEPTION, implGetExceptionMsg( e3 ) );
                    }
                    catch( Exception& e4 )
                    {
                        StarBASIC::Error( ERRCODE_BASIC_EXCEPTION, implGetExceptionMsg( e4 ) );
                    }
                }
                else if( bInvocation && mxInvocation.is() )
                {
                    // Wert von Uno nach Sbx uebernehmen
                    Any aAnyValue = sbxToUnoValueImpl( pVar );
                    try
                    {
                        // Wert setzen
                        mxInvocation->setValue( pProp->GetName(), aAnyValue );
                    }
                    catch( BasicErrorException& e0 )
                    {
                        implHandleBasicErrorException( e0 );
                    }
                    catch( WrappedTargetException& e1 )
                    {
                        implHandleWrappedTargetException( e1 );
                    }
                    catch( RuntimeException& e2 )
                    {
                        StarBASIC::Error( ERRCODE_BASIC_EXCEPTION, implGetExceptionMsg( e2 ) );
                    }
                    catch( Exception& e3 )
                    {
                        StarBASIC::Error( ERRCODE_BASIC_EXCEPTION, implGetExceptionMsg( e3 ) );
                    }
                }
            }
        }
        else if( pMeth )
        {
            bool bInvocation = pMeth->isInvocationBased();
            if( pHint->GetId() == SBX_HINT_DATAWANTED )
            {
                // Anzahl Parameter -1 wegen Param0 == this
                UINT32 nParamCount = pParams ? ((UINT32)pParams->Count() - 1) : 0;
                Sequence<Any> args;
                BOOL bOutParams = FALSE;
                UINT32 i;

                if( !bInvocation && mxUnoAccess.is() )
                {
                    // Infos holen
                    const Sequence<ParamInfo>& rInfoSeq = pMeth->getParamInfos();
                    const ParamInfo* pParamInfos = rInfoSeq.getConstArray();
                    UINT32 nUnoParamCount = rInfoSeq.getLength();
                    UINT32 nAllocParamCount = nParamCount;

                    // Ueberschuessige Parameter ignorieren, Alternative: Error schmeissen
                    if( nParamCount > nUnoParamCount )
                    {
                        nParamCount = nUnoParamCount;
                        nAllocParamCount = nParamCount;
                    }
                    else if( nParamCount < nUnoParamCount )
                    {
/*?*/ //						SbiInstance* pInst = pINST;
/*?*/ //						if( pInst && pInst->IsCompatibility() )
/*?*/ //						{
/*?*/ //							// Check types
/*?*/ //							bool bError = false;
/*?*/ //							for( i = nParamCount ; i < nUnoParamCount ; i++ )
/*?*/ //							{
/*?*/ //								const ParamInfo& rInfo = pParamInfos[i];
/*?*/ //								const Reference< XIdlClass >& rxClass = rInfo.aType;
/*?*/ //								if( rxClass->getTypeClass() != TypeClass_ANY )
/*?*/ //								{
/*?*/ //									bError = true;
/*?*/ //									StarBASIC::Error( SbERR_NOT_OPTIONAL );
/*?*/ //								}
/*?*/ //							}
/*?*/ //							if( !bError )
/*?*/ //								nAllocParamCount = nUnoParamCount;
/*?*/ //						}
                    }

                    if( nAllocParamCount > 0 )
                    {
                        args.realloc( nAllocParamCount );
                        Any* pAnyArgs = args.getArray();
                        for( i = 0 ; i < nParamCount ; i++ )
                        {
                            const ParamInfo& rInfo = pParamInfos[i];
                            const Reference< XIdlClass >& rxClass = rInfo.aType;
                            //const XIdlClassRef& rxClass = pUnoParams[i];

                            com::sun::star::uno::Type aType( rxClass->getTypeClass(), rxClass->getName() );

                            // ACHTUNG: Bei den Sbx-Parametern den Offset nicht vergessen!
                            pAnyArgs[i] = sbxToUnoValue( pParams->Get( (USHORT)(i+1) ), aType );

                            // Wenn es nicht schon feststeht pruefen, ob Out-Parameter vorliegen
                            if( !bOutParams )
                            {
                                ParamMode aParamMode = rInfo.aMode;
                                if( aParamMode != ParamMode_IN )
                                    bOutParams = TRUE;
                            }
                        }
                    }
                }
                else if( bInvocation && pParams && mxInvocation.is() )
                {
                    bool bOLEAutomation = true;
                    // TODO: bOLEAutomation = xOLEAutomation.is()

                    AutomationNamedArgsSbxArray* pArgNamesArray = NULL;
                    if( bOLEAutomation )
                        pArgNamesArray = PTR_CAST(AutomationNamedArgsSbxArray,pParams);

                    args.realloc( nParamCount );
                    Any* pAnyArgs = args.getArray();
                    bool bBlockConversionToSmallestType( false );
/*?*/ //					bool bBlockConversionToSmallestType = pINST->IsCompatibility();
                    if( pArgNamesArray )
                    {
                        Sequence< OUString >& rNameSeq = pArgNamesArray->getNames();
                        OUString* pNames = rNameSeq.getArray();

                        Any aValAny;
                        for( i = 0 ; i < nParamCount ; i++ )
                        {
                            USHORT iSbx = (USHORT)(i+1);

                            // ACHTUNG: Bei den Sbx-Parametern den Offset nicht vergessen!
                            aValAny = sbxToUnoValueImpl( pParams->Get( iSbx ),
                                                        bBlockConversionToSmallestType );

                            OUString aParamName = pNames[iSbx];
                            if( aParamName.getLength() )
                            {
                                oleautomation::NamedArgument aNamedArgument;
                                aNamedArgument.Name = aParamName;
                                aNamedArgument.Value = aValAny;
                                pAnyArgs[i] <<= aNamedArgument;
                            }
                            else
                            {
                                pAnyArgs[i] = aValAny;
                            }
                        }
                    }
                    else
                    {
                        for( i = 0 ; i < nParamCount ; i++ )
                        {
                            // ACHTUNG: Bei den Sbx-Parametern den Offset nicht vergessen!
                            pAnyArgs[i] = sbxToUnoValueImpl( pParams->Get( (USHORT)(i+1) ),
                                                            bBlockConversionToSmallestType );
                        }
                    }
                }

                // Methode callen
                GetSbData()->bBlockCompilerError = TRUE;  // #106433 Block compiler errors for API calls
                try
                {
                    if( !bInvocation && mxUnoAccess.is() )
                    {
                        Any aRetAny = pMeth->m_xUnoMethod->invoke( getUnoAny(), args );

                        // Wert von Uno nach Sbx uebernehmen
                        unoToSbxValue( pVar, aRetAny );

                        // Muessen wir Out-Parameter zurueckkopieren?
                        if( bOutParams )
                        {
                            const Any* pAnyArgs = args.getConstArray();

                            // Infos holen
                            const Sequence<ParamInfo>& rInfoSeq = pMeth->getParamInfos();
                            const ParamInfo* pParamInfos = rInfoSeq.getConstArray();

                            UINT32 j;
                            for( j = 0 ; j < nParamCount ; j++ )
                            {
                                const ParamInfo& rInfo = pParamInfos[j];
                                ParamMode aParamMode = rInfo.aMode;
                                if( aParamMode != ParamMode_IN )
                                    unoToSbxValue( (SbxVariable*)pParams->Get( (USHORT)(j+1) ), pAnyArgs[ j ] );
                            }
                        }
                    }
                    else if( bInvocation && mxInvocation.is() )
                    {
                        Sequence< INT16 > OutParamIndex;
                        Sequence< Any > OutParam;
                        Any aRetAny = mxInvocation->invoke( pMeth->GetName(), args, OutParamIndex, OutParam );

                        // Wert von Uno nach Sbx uebernehmen
                        unoToSbxValue( pVar, aRetAny );

                        const INT16* pIndices = OutParamIndex.getConstArray();
                        UINT32 nLen = OutParamIndex.getLength();
                        if( nLen )
                        {
                            const Any* pNewValues = OutParam.getConstArray();
                            for( UINT32 j = 0 ; j < nLen ; j++ )
                            {
                                INT16 iTarget = pIndices[ j ];
                                if( iTarget >= (INT16)nParamCount )
                                    break;
                                unoToSbxValue( (SbxVariable*)pParams->Get( (USHORT)(j+1) ), pNewValues[ j ] );
                            }
                        }
                    }

                    // #55460, Parameter hier weghauen, da das in unoToSbxValue()
                    // bei Arrays wegen #54548 nicht mehr gemacht wird
                    if( pParams )
                        pVar->SetParameters( NULL );
                }
                catch( BasicErrorException& e0 )
                {
                    implHandleBasicErrorException( e0 );
                }
                catch( WrappedTargetException& e1 )
                {
                    implHandleWrappedTargetException( e1 );
                }
                catch( RuntimeException& e2 )
                {
                    StarBASIC::Error( ERRCODE_BASIC_EXCEPTION, implGetExceptionMsg( e2 ) );
                }
                catch( IllegalArgumentException& e3)
                {
                    StarBASIC::Error( ERRCODE_BASIC_EXCEPTION,
                        implGetExceptionMsg( e3, aIllegalArgumentExceptionName ) );
                }
                catch( Exception& e4 )
                {
                    StarBASIC::Error( ERRCODE_BASIC_EXCEPTION, implGetExceptionMsg( e4 ) );
                }
                /*
                catch( NullPointerException& e1 )
                {
                }
                catch( InvocationTargetException& e2 )
                {
                }
                */
                GetSbData()->bBlockCompilerError = FALSE;  // #106433 Unblock compiler errors
            }
        }
        else
            SbxObject::SFX_NOTIFY( rBC, rBCType, rHint, rHintType );
    }
}


#ifdef INVOCATION_ONLY
// Aus USR
Reference< XInvocation > createDynamicInvocationFor( const Any& aAny );
#endif

SbUnoObject::SbUnoObject( const String& aName_, const Any& aUnoObj_ )
    : SbxObject( aName_ )
{
    static Reference< XIntrospection > xIntrospection;

    // Default-Properties von Sbx wieder rauspruegeln
    Remove( XubString( RTL_CONSTASCII_USTRINGPARAM("Name") ), SbxCLASS_DONTCARE );
    Remove( XubString( RTL_CONSTASCII_USTRINGPARAM("Parent") ), SbxCLASS_DONTCARE );

    // Typ des Objekts pruefen
    TypeClass eType = aUnoObj_.getValueType().getTypeClass();
    Reference< XInterface > x;
    if( eType == TypeClass_INTERFACE )
    {
        // Interface aus dem Any besorgen
        x = *(Reference< XInterface >*)aUnoObj_.getValue();
        if( !x.is() )
            return;
    }

    Reference< XTypeProvider > xTypeProvider;
#ifdef INVOCATION_ONLY
    // Invocation besorgen
    mxInvocation = createDynamicInvocationFor( aUnoObj_ );
#else
    // Hat das Object selbst eine Invocation?
    mxInvocation = Reference< XInvocation >( x, UNO_QUERY );

    xTypeProvider = Reference< XTypeProvider >( x, UNO_QUERY );
#endif

    if( mxInvocation.is() )
    {
        // #94670: This is WRONG because then the MaterialHolder doesn't refer
        // to the object implementing XInvocation but to the object passed to
        // the invocation service!!!
        // mxMaterialHolder = Reference< XMaterialHolder >::query( mxInvocation );

        // ExactName holen
        mxExactNameInvocation = Reference< XExactName >::query( mxInvocation );

        // Rest bezieht sich nur auf Introspection
        if( !xTypeProvider.is() )
        {
            bNeedIntrospection = FALSE;
            return;
        }
    }

    // Introspection-Flag
    bNeedIntrospection = TRUE;
    maTmpUnoObj = aUnoObj_;


    //*** Namen bestimmen ***
    BOOL bFatalError = TRUE;

    // Ist es ein Interface oder eine struct?
    BOOL bSetClassName = FALSE;
    String aClassName_;
    if( eType == TypeClass_STRUCT || eType == TypeClass_EXCEPTION )
    {
        // Struct ist Ok
        bFatalError = FALSE;

        // #67173 Echten Klassen-Namen eintragen
        if( aName_.Len() == 0 )
        {
            aClassName_ = String( aUnoObj_.getValueType().getTypeName() );
            bSetClassName = TRUE;
        }
    }
    else if( eType == TypeClass_INTERFACE )
    {
        // #70197 Interface geht immer durch Typ im Any
        bFatalError = FALSE;

        // Nach XIdlClassProvider-Interface fragen
        Reference< XIdlClassProvider > xClassProvider( x, UNO_QUERY );
        if( xClassProvider.is() )
        {
            // #67173 Echten Klassen-Namen eintragen
            if( aName_.Len() == 0 )
            {
                Sequence< Reference< XIdlClass > > szClasses = xClassProvider->getIdlClasses();
                UINT32 nLen = szClasses.getLength();
                if( nLen )
                {
                    const Reference< XIdlClass > xImplClass = szClasses.getConstArray()[ 0 ];
                    if( xImplClass.is() )
                    {
                        aClassName_ = String( xImplClass->getName() );
                        bSetClassName = TRUE;
                    }
                }
            }
        }
    }
    if( bSetClassName )
        SetClassName( aClassName_ );

    // Weder Interface noch Struct -> FatalError
    if( bFatalError )
    {
        StarBASIC::FatalError( ERRCODE_BASIC_EXCEPTION );
        return;
    }

    // #67781 Introspection erst on demand durchfuehren
}

SbUnoObject::~SbUnoObject()
{
}


// #76470 Introspection on Demand durchfuehren
void SbUnoObject::doIntrospection( void )
{
    static Reference< XIntrospection > xIntrospection;

    if( !bNeedIntrospection )
        return;
    bNeedIntrospection = FALSE;

    if( !xIntrospection.is() )
    {
        // Introspection-Service holen
        Reference< XMultiServiceFactory > xFactory( comphelper::getProcessServiceFactory() );
        if ( xFactory.is() )
        {
            Reference< XInterface > xI = xFactory->createInstance( rtl::OUString::createFromAscii("com.sun.star.beans.Introspection") );
            if (xI.is())
                xIntrospection = Reference< XIntrospection >::query( xI );
                //xI->queryInterface( ::getCppuType( (const Reference< XIntrospection > *)0 ), xIntrospection );
        }
    }
    if( !xIntrospection.is() )
    {
        StarBASIC::FatalError( ERRCODE_BASIC_EXCEPTION );
        return;
    }

    // Introspection durchfuehren
    try
    {
        mxUnoAccess = xIntrospection->inspect( maTmpUnoObj );
    }
    catch( RuntimeException& e )
    {
        StarBASIC::Error( ERRCODE_BASIC_EXCEPTION, implGetExceptionMsg( e ) );
    }

    if( !mxUnoAccess.is() )
    {
        // #51475 Ungueltiges Objekt kennzeichnen (kein mxMaterialHolder)
        return;
    }

    // MaterialHolder vom Access holen
    mxMaterialHolder = Reference< XMaterialHolder >::query( mxUnoAccess );

    // ExactName vom Access holen
    mxExactName = Reference< XExactName >::query( mxUnoAccess );
}




// #67781 Start einer Liste aller SbUnoMethod-Instanzen
static SbUnoMethod* pFirst = NULL;

SbUnoMethod::SbUnoMethod
(
    const String& aName_,
    SbxDataType eSbxType,
    Reference< XIdlMethod > xUnoMethod_,
    bool bInvocation
)
    : SbxMethod( aName_, eSbxType )
    , mbInvocation( bInvocation )
{
    m_xUnoMethod = xUnoMethod_;
    pParamInfoSeq = NULL;

    // #67781 Methode in Liste eintragen
    pNext = pFirst;
    pPrev = NULL;
    pFirst = this;
    if( pNext )
        pNext->pPrev = this;
}

SbUnoMethod::~SbUnoMethod()
{
    delete pParamInfoSeq;

    if( this == pFirst )
        pFirst = pNext;
    else if( pPrev )
        pPrev->pNext = pNext;
    if( pNext )
        pNext->pPrev = pPrev;
}

SbxInfo* SbUnoMethod::GetInfo()
{
/*?*/ //	if( !pInfo && m_xUnoMethod.is() )
/*?*/ //	{
/*?*/ //		SbiInstance* pInst = pINST;
/*?*/ //		if( pInst && pInst->IsCompatibility() )
/*?*/ //		{
/*?*/ //			pInfo = new SbxInfo();
/*?*/ //
/*?*/ //			const Sequence<ParamInfo>& rInfoSeq = getParamInfos();
/*?*/ //			const ParamInfo* pParamInfos = rInfoSeq.getConstArray();
/*?*/ //			UINT32 nParamCount = rInfoSeq.getLength();
/*?*/ //
/*?*/ //			for( UINT32 i = 0 ; i < nParamCount ; i++ )
/*?*/ //			{
/*?*/ //				const ParamInfo& rInfo = pParamInfos[i];
/*?*/ //				OUString aParamName = rInfo.aName;
/*?*/ //
/*?*/ //				// const Reference< XIdlClass >& rxClass = rInfo.aType;
/*?*/ //				SbxDataType t = SbxVARIANT;
/*?*/ //				USHORT nFlags_ = SBX_READ;
/*?*/ //				pInfo->AddParam( aParamName, t, nFlags_ );
/*?*/ //			}
/*?*/ //		}
/*?*/ //	}
    return pInfo;
}

const Sequence<ParamInfo>& SbUnoMethod::getParamInfos( void )
{
    if( !pParamInfoSeq && m_xUnoMethod.is() )
    {
        Sequence<ParamInfo> aTmp = m_xUnoMethod->getParameterInfos() ;
        pParamInfoSeq = new Sequence<ParamInfo>( aTmp );
    }
    return *pParamInfoSeq;
}

SbUnoProperty::SbUnoProperty
(
    const String& aName_,
    SbxDataType eSbxType,
    const Property& aUnoProp_,
    INT32 nId_,
    bool bInvocation
)
    : SbxProperty( aName_, eSbxType )
    , aUnoProp( aUnoProp_ )
    , nId( nId_ )
    , mbInvocation( bInvocation )
{
    // #54548, bei bedarf Dummy-Array einsetzen, damit SbiRuntime::CheckArray() geht
    static SbxArrayRef xDummyArray = new SbxArray( SbxVARIANT );
    if( eSbxType & SbxARRAY )
        PutObject( xDummyArray );
}

SbUnoProperty::~SbUnoProperty()
{}


/*?*/ // // #72732 Spezielle SbxVariable, die beim put/get prueft,
/*?*/ // // ob der Kontext fuer eine UnoClass sinnvoll ist. Sonst
/*?*/ // // liegt eventuell ein Schreibfehler im Basic-Source vor.
/*?*/ // BOOL UnoClassMemberVariable::Get( SbxValues& rRes ) const
/*?*/ // {
/*?*/ // 	// Zugriff auf den Member einer UnoClass mit Parametern ist unsinnig
/*?*/ // 	if( GetParameters() )
/*?*/ // 	{
/*?*/ // 		if( mpRuntime )
/*?*/ // 			mpRuntime->Error( SbERR_NO_METHOD );
/*?*/ // 	}
/*?*/ // 	return SbxVariable::Get( rRes );
/*?*/ // }
/*?*/ // 
/*?*/ // BOOL UnoClassMemberVariable::Put( const SbxValues& rRes )
/*?*/ // {
/*?*/ // 	if( bInternalUse )
/*?*/ // 	{
/*?*/ // 		return SbxVariable::Put( rRes );
/*?*/ // 	}
/*?*/ // 	// Schreibzugriff auf den Member einer UnoClass ist immer falsch
/*?*/ // 	mpRuntime->Error( SbERR_NO_METHOD );
/*?*/ // 	return FALSE;
/*?*/ // }
/*?*/ // 
/*?*/ // TYPEINIT1(UnoClassMemberVariable,SbxVariable)

SbxVariable* SbUnoObject::Find( const XubString& rName, SbxClassType t )
{
    static Reference< XIdlMethod > xDummyMethod;
    static Property aDummyProp;

    SbxVariable* pRes = SbxObject::Find( rName, t );

    if( bNeedIntrospection )
        doIntrospection();

    // Neu 4.3.1999: Properties on Demand anlegen, daher jetzt perIntrospectionAccess
    // suchen, ob doch eine Property oder Methode des geforderten Namens existiert
    if( !pRes )
    {
        OUString aUName( rName );
        if( mxUnoAccess.is() )
        {
            if( mxExactName.is() )
            {
                OUString aUExactName = mxExactName->getExactName( aUName );
                if( aUExactName.getLength() )
                    aUName = aUExactName;
            }
            if( mxUnoAccess->hasProperty( aUName, PropertyConcept::ALL - PropertyConcept::DANGEROUS ) )
            {
                const Property& rProp = mxUnoAccess->
                    getProperty( aUName, PropertyConcept::ALL - PropertyConcept::DANGEROUS );

                // #58455 Wenn die Property void sein kann, muss als Typ Variant gesetzt werden
                SbxDataType eSbxType;
                if( rProp.Attributes & PropertyAttribute::MAYBEVOID )
                    eSbxType = SbxVARIANT;
                else
                    eSbxType = unoToSbxType( rProp.Type.getTypeClass() );

                // Property anlegen und reinbraten
                SbxVariableRef xVarRef = new SbUnoProperty( rProp.Name, eSbxType, rProp, 0, false );
                QuickInsert( (SbxVariable*)xVarRef );
                pRes = xVarRef;
            }
            else if( mxUnoAccess->hasMethod( aUName,
                MethodConcept::ALL - MethodConcept::DANGEROUS ) )
            {
                // Methode ansprechen
                const Reference< XIdlMethod >& rxMethod = mxUnoAccess->
                    getMethod( aUName, MethodConcept::ALL - MethodConcept::DANGEROUS );

                // SbUnoMethode anlegen und reinbraten
                SbxVariableRef xMethRef = new SbUnoMethod( rxMethod->getName(), 
                    unoToSbxType( rxMethod->getReturnType() ), rxMethod, false );
                QuickInsert( (SbxVariable*)xMethRef );
                pRes = xMethRef;
            }

            // Wenn immer noch nichts gefunden wurde, muss geprueft werden, ob NameAccess vorliegt
            if( !pRes )
            {
                try
                {
                    Reference< XNameAccess > xNameAccess( mxUnoAccess->queryAdapter( ::getCppuType( (const Reference< XPropertySet > *)0 ) ), UNO_QUERY );
                    OUString aUName2( rName );

                    if( xNameAccess.is() && xNameAccess->hasByName( aUName2 ) )
                    {
                        Any aAny = xNameAccess->getByName( aUName2 );

                        // ACHTUNG: Die hier erzeugte Variable darf wegen bei XNameAccess
                        // nicht als feste Property in das Object aufgenommen werden und
                        // wird daher nirgendwo gehalten.
                        // Wenn das Probleme gibt, muss das kuenstlich gemacht werden oder
                        // es muss eine Klasse SbUnoNameAccessProperty geschaffen werden,
                        // bei der die Existenz staendig neu ueberprueft und die ggf. weg-
                        // geworfen wird, wenn der Name nicht mehr gefunden wird.
                        pRes = new SbxVariable( SbxVARIANT );
                        unoToSbxValue( pRes, aAny );
                    }
                }
                catch( NoSuchElementException& e )
                {
                    StarBASIC::Error( ERRCODE_BASIC_EXCEPTION, implGetExceptionMsg( e ) );
                }
                catch( BasicErrorException& e0 )
                {
                    // Anlegen, damit der Exception-Fehler nicht ueberschrieben wird
                    if( !pRes )
                        pRes = new SbxVariable( SbxVARIANT );

                    implHandleBasicErrorException( e0 );
                }
                catch( WrappedTargetException& e1 )
                {
                    // Anlegen, damit der Exception-Fehler nicht ueberschrieben wird
                    if( !pRes )
                        pRes = new SbxVariable( SbxVARIANT );

                    implHandleWrappedTargetException( e1 );
                }
                catch( RuntimeException& e2 )
                {
                    // Anlegen, damit der Exception-Fehler nicht ueberschrieben wird
                    if( !pRes )
                        pRes = new SbxVariable( SbxVARIANT );

                    StarBASIC::Error( ERRCODE_BASIC_EXCEPTION, implGetExceptionMsg( e2 ) );
                }
            }
        }
        if( !pRes && mxInvocation.is() )
        {
            if( mxExactNameInvocation.is() )
            {
                OUString aUExactName = mxExactNameInvocation->getExactName( aUName );
                if( aUExactName.getLength() )
                    aUName = aUExactName;
            }

            try
            {
                if( mxInvocation->hasProperty( aUName ) )
                {
                    // Property anlegen und reinbraten
                    SbxVariableRef xVarRef = new SbUnoProperty( aUName, SbxVARIANT, aDummyProp, 0, true );
                    QuickInsert( (SbxVariable*)xVarRef );
                    pRes = xVarRef;
                }
                else if( mxInvocation->hasMethod( aUName ) )
                {
                    // SbUnoMethode anlegen und reinbraten
                    SbxVariableRef xMethRef = new SbUnoMethod( aUName, SbxVARIANT, xDummyMethod, true );
                    QuickInsert( (SbxVariable*)xMethRef );
                    pRes = xMethRef;
                }
            }
            catch( RuntimeException& e )
            {
                // Anlegen, damit der Exception-Fehler nicht ueberschrieben wird
                if( !pRes )
                    pRes = new SbxVariable( SbxVARIANT );

                StarBASIC::Error( ERRCODE_BASIC_EXCEPTION, implGetExceptionMsg( e ) );
            }
        }
    }

    // Ganz am Schluss noch pruefen, ob die Dbg_-Properties gemeint sind

    if( !pRes )
    {
        if( rName.EqualsIgnoreCaseAscii( ID_DBG_SUPPORTEDINTERFACES ) ||
            rName.EqualsIgnoreCaseAscii( ID_DBG_PROPERTIES ) ||
            rName.EqualsIgnoreCaseAscii( ID_DBG_METHODS ) )
        {
            // Anlegen
            implCreateDbgProperties();

            // Jetzt muessen sie regulaer gefunden werden
            pRes = SbxObject::Find( rName, SbxCLASS_DONTCARE );
        }
    }
    return pRes;
}


// Hilfs-Methode zum Anlegen der dbg_-Properties
void SbUnoObject::implCreateDbgProperties( void )
{
    Property aProp;

    // Id == -1: Implementierte Interfaces gemaess ClassProvider anzeigen
    SbxVariableRef xVarRef = new SbUnoProperty( ID_DBG_SUPPORTEDINTERFACES, SbxSTRING, aProp, -1, false );
    QuickInsert( (SbxVariable*)xVarRef );

    // Id == -2: Properties ausgeben
    xVarRef = new SbUnoProperty( ID_DBG_PROPERTIES, SbxSTRING, aProp, -2, false );
    QuickInsert( (SbxVariable*)xVarRef );

    // Id == -3: Methoden ausgeben
    xVarRef = new SbUnoProperty( ID_DBG_METHODS, SbxSTRING, aProp, -3, false );
    QuickInsert( (SbxVariable*)xVarRef );
}

void SbUnoObject::implCreateAll( void )
{
    // Bestehende Methoden und Properties alle wieder wegwerfen
    pMethods   = new SbxArray;
    pProps     = new SbxArray;

    if( bNeedIntrospection ) doIntrospection();

    // Instrospection besorgen
    Reference< XIntrospectionAccess > xAccess = mxUnoAccess;
    if( !xAccess.is() )
    {
        if( mxInvocation.is() )
            xAccess = mxInvocation->getIntrospection();
    }
    if( !xAccess.is() )
        return;

    // Properties anlegen
    Sequence<Property> props = xAccess->getProperties( PropertyConcept::ALL - PropertyConcept::DANGEROUS );
    UINT32 nPropCount = props.getLength();
    const Property* pProps_ = props.getConstArray();

    UINT32 i;
    for( i = 0 ; i < nPropCount ; i++ )
    {
        const Property& rProp = pProps_[ i ];

        // #58455 Wenn die Property void sein kann, muss als Typ Variant gesetzt werden
        SbxDataType eSbxType;
        if( rProp.Attributes & PropertyAttribute::MAYBEVOID )
            eSbxType = SbxVARIANT;
        else
            eSbxType = unoToSbxType( rProp.Type.getTypeClass() );

        // Property anlegen und reinbraten
        SbxVariableRef xVarRef = new SbUnoProperty( rProp.Name, eSbxType, rProp, i, false );
        QuickInsert( (SbxVariable*)xVarRef );
    }

    // Dbg_-Properties anlegen
    implCreateDbgProperties();

    // Methoden anlegen
    Sequence< Reference< XIdlMethod > > aMethodSeq = xAccess->getMethods
        ( MethodConcept::ALL - MethodConcept::DANGEROUS );
    UINT32 nMethCount = aMethodSeq.getLength();
    const Reference< XIdlMethod >* pMethods_ = aMethodSeq.getConstArray();
    for( i = 0 ; i < nMethCount ; i++ )
    {
        // Methode ansprechen
        const Reference< XIdlMethod >& rxMethod = pMethods_[i];

        // SbUnoMethode anlegen und reinbraten
        SbxVariableRef xMethRef = new SbUnoMethod
            ( rxMethod->getName(), unoToSbxType( rxMethod->getReturnType() ), rxMethod, false );
        QuickInsert( (SbxVariable*)xMethRef );
    }
}


// Wert rausgeben
Any SbUnoObject::getUnoAny( void )
{
    Any aRetAny;
    if( bNeedIntrospection ) doIntrospection();
    if( mxMaterialHolder.is() )
        aRetAny = mxMaterialHolder->getMaterial();
    else if( mxInvocation.is() )
        aRetAny <<= mxInvocation;
    return aRetAny;
}

// Hilfsmethode zum Anlegen einer Uno-Struct per CoreReflection
SbUnoObject* Impl_CreateUnoStruct( const String& aClassName )
{
    // CoreReflection holen
    Reference< XIdlReflection > xCoreReflection = getCoreReflection_Impl();
    if( !xCoreReflection.is() )
        return NULL;

    // Klasse suchen
    Reference< XIdlClass > xClass;
    Reference< XHierarchicalNameAccess > xHarryName =
        getCoreReflection_HierarchicalNameAccess_Impl();
    if( xHarryName.is() && xHarryName->hasByHierarchicalName( aClassName ) )
        xClass = xCoreReflection->forName( aClassName );
    if( !xClass.is() )
        return NULL;

    // Ist es ueberhaupt ein struct?
    TypeClass eType = xClass->getTypeClass();
    if( eType != TypeClass_STRUCT )
        return NULL;

    // Instanz erzeugen
    Any aNewAny;
    xClass->createObject( aNewAny );

    // SbUnoObject daraus basteln
    SbUnoObject* pUnoObj = new SbUnoObject( aClassName, aNewAny );
    return pUnoObj;
}


// Factory-Klasse fuer das Anlegen von Uno-Structs per DIM AS NEW
SbxBase* SbUnoFactory::Create( UINT16, UINT32 )
{
    // Ueber SbxId laeuft in Uno nix
    return NULL;
}

SbxObject* SbUnoFactory::CreateObject( const String& rClassName )
{
    return Impl_CreateUnoStruct( rClassName );
}

// Funktion, um einen globalen Bezeichner im
// UnoScope zu suchen und fuer Sbx zu wrappen
SbxVariable* findUnoClass( const String& rName )
{
    // #105550 Check if module exists
    SbUnoClass* pUnoClass = NULL;

    Reference< XHierarchicalNameAccess > xTypeAccess = getTypeProvider_Impl();
    Reference< XTypeDescription > xTypeDesc;
    if( xTypeAccess->hasByHierarchicalName( rName ) )
    {
        Any aRet = xTypeAccess->getByHierarchicalName( rName );
        aRet >>= xTypeDesc;

        if( xTypeDesc.is() )
        {
            TypeClass eTypeClass = xTypeDesc->getTypeClass();
            if( eTypeClass == TypeClass_MODULE || eTypeClass == TypeClass_CONSTANTS )
                pUnoClass = new SbUnoClass( rName );
        }
    }
    return pUnoClass;
}

SbxVariable* SbUnoClass::Find( const XubString& rName, SbxClassType t )
{
    (void)t;
    
    SbxVariable* pRes = SbxObject::Find( rName, SbxCLASS_VARIABLE );

    // Wenn nichts gefunden wird, ist das Sub-Modul noch nicht bekannt
    if( !pRes )
    {
        // Wenn es schon eine Klasse ist, nach einen Feld fragen
        if( m_xClass.is() )
        {
            // Ist es ein Field
            OUString aUStr( rName );
            Reference< XIdlField > xField = m_xClass->getField( aUStr );
            Reference< XIdlClass > xClass;
            if( xField.is() )
            {
                try
                {
                    Any aAny;
                    aAny = xField->get( aAny );

                    // Nach Sbx wandeln
                    pRes = new SbxVariable( SbxVARIANT );
                    pRes->SetName( rName );
                    unoToSbxValue( pRes, aAny );
                }
                catch( BasicErrorException& e0 )
                {
                    implHandleBasicErrorException( e0 );
                }
                catch( WrappedTargetException& e1 )
                {
                    implHandleWrappedTargetException( e1 );
                }
                catch( RuntimeException& e2 )
                {
                    StarBASIC::Error( ERRCODE_BASIC_EXCEPTION, implGetExceptionMsg( e2 ) );
                }
                catch( IllegalArgumentException& e3 )
                {
                    StarBASIC::Error( ERRCODE_BASIC_EXCEPTION,
                        implGetExceptionMsg( e3, aIllegalArgumentExceptionName ) );
                }
            }
        }
        else
        {
            // Vollqualifizierten Namen erweitern
            String aNewName = GetName();
            aNewName.AppendAscii( "." );
            aNewName += rName;

            // CoreReflection holen
            Reference< XIdlReflection > xCoreReflection = getCoreReflection_Impl();
            if( xCoreReflection.is() )
            {
                // Ist es eine Konstante?
                Reference< XHierarchicalNameAccess > xHarryName( xCoreReflection, UNO_QUERY );
                if( xHarryName.is() )
                {
                    try
                    {
                        Any aValue = xHarryName->getByHierarchicalName( aNewName );
                        TypeClass eType = aValue.getValueType().getTypeClass();

                        // Interface gefunden? Dann ist es eine Klasse
                        if( eType == TypeClass_INTERFACE )
                        {
                            Reference< XInterface > xIface = *(Reference< XInterface >*)aValue.getValue();
                            Reference< XIdlClass > xClass( xIface, UNO_QUERY );
                            if( xClass.is() )
                            {
                                pRes = new SbxVariable( SbxVARIANT );
                                SbxObjectRef xWrapper = (SbxObject*)new SbUnoClass( aNewName, xClass );
                                pRes->PutObject( xWrapper );

                            }
                        }
                        else
                        {
                            pRes = new SbxVariable( SbxVARIANT );
                            unoToSbxValue( pRes, aValue );
                        }
                    }
                    catch( NoSuchElementException& e1 )
                    {
                        String aMsg = implGetExceptionMsg( e1 );
                    }
                }

                // Sonst wieder als Klasse annehmen
                if( !pRes )
                {
                    SbxVariable* pNewClass = findUnoClass( aNewName );
                    if( pNewClass )
                    {
                        Reference< XIdlClass > xClass;
                        pRes = new SbxVariable( SbxVARIANT );
                        SbxObjectRef xWrapper = (SbxObject*)pNewClass;
                        pRes->PutObject( xWrapper );
                    }
                }
            }
        }

        if( pRes )
        {
            pRes->SetName( rName );

            // Variable einfuegen, damit sie spaeter im Find gefunden wird
            QuickInsert( pRes );

            // Uns selbst gleich wieder als Listener rausnehmen,
            // die Werte sind alle konstant
            if( pRes->IsBroadcaster() )
                EndListening( pRes->GetBroadcaster(), TRUE );
        }
    }
    return pRes;
}
}
