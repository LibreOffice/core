/*************************************************************************
 *
 *  $RCSfile: sbunoobj.cxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-23 10:12:58 $
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
//#include <stl_queue.h>

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
#include <svtools/hint.hxx>
#endif
#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
#endif

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/extract.hxx>

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>


#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/DeploymentException.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyConcept.hpp>
#include <com/sun/star/beans/MethodConcept.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/script/XAllListener.hpp>
#include <com/sun/star/script/XInvocationAdapterFactory.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
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


#include<sbstar.hxx>
#include<sbuno.hxx>
#include<sberrors.hxx>
#include<sbunoobj.hxx>
#include"sbjsmod.hxx"
#include<basmgr.hxx>
#include<sbintern.hxx>
#include<runtime.hxx>

#include<math.h>

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

// Hold TypeConverter statically
Reference< XTypeConverter > getTypeConverter_Impl( void )
{
    static Reference< XTypeConverter > xTypeConverter;

    // Haben wir schon CoreReflection, sonst besorgen
    if( !xTypeConverter.is() )
    {
        Reference< XComponentContext > xContext = getComponentContext_Impl();
        if( xContext.is() )
        {
            Reference<XMultiComponentFactory> xSMgr = xContext->getServiceManager();
            xTypeConverter = Reference<XTypeConverter>(
                xSMgr->createInstanceWithContext(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.script.Converter")),
                        xContext ), UNO_QUERY );
        }
        if( !xTypeConverter.is() )
        {
            throw DeploymentException(
                OUString( RTL_CONSTASCII_USTRINGPARAM
                    ("com.sun.star.script.Converter service not accessable") ),
                Reference< XInterface >() );
        }
    }
    return xTypeConverter;
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
        Any aAny;
        aAny <<= xOLEObject;
        pUnoObj = new SbUnoObject( aType, aAny );
    }
    return pUnoObj;
}


String implGetExceptionMsg( Exception& e, const String& aExceptionType_ );

Any convertAny( const Any& rVal, const Type& aDestType )
{
    Any aConvertedVal;
    Reference< XTypeConverter > xConverter = getTypeConverter_Impl();
    try
    {
        aConvertedVal = xConverter->convertTo( rVal, aDestType );
    }
    catch( IllegalArgumentException& e1 )
    {
        StarBASIC::Error( ERRCODE_BASIC_EXCEPTION,
            implGetExceptionMsg( e1, aIllegalArgumentExceptionName ) );
        return aConvertedVal;
    }
    catch( CannotConvertException& e2 )
    {
        String aCannotConvertExceptionName
            ( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.lang.IllegalArgumentException" ) );
        StarBASIC::Error( ERRCODE_BASIC_EXCEPTION,
            implGetExceptionMsg( e2, aCannotConvertExceptionName ) );
        return aConvertedVal;
    }
    return aConvertedVal;
}


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
        RuntimeException& e = *( (RuntimeException*)aWrappedAny.getValue() );
        aMsg = implGetExceptionMsg( e, String( aExceptionType.getTypeName() ) );
    }
    // Otherwise use WrappedTargetException itself
    else
    {
        aMsg = implGetExceptionMsg( e );
    }

    return aMsg;
}

// Von Uno nach Sbx wandeln
SbxDataType unoToSbxType( TypeClass eType )
{
    SbxDataType eRetType = SbxVOID;

    switch( eType )
    {
        case TypeClass_INTERFACE:
        case TypeClass_TYPE:
        case TypeClass_STRUCT:          eRetType = SbxOBJECT;   break;

        /* folgende Typen lassen wir erstmal weg
        case TypeClass_SERVICE:         break;
        case TypeClass_CLASS:           break;
        case TypeClass_TYPEDEF:         break;
        case TypeClass_UNION:           break;
        case TypeClass_EXCEPTION:       break;
        case TypeClass_ARRAY:           break;
        */
        case TypeClass_ENUM:            eRetType = SbxLONG;     break;
        case TypeClass_SEQUENCE:
            eRetType = (SbxDataType) ( SbxOBJECT | SbxARRAY );
            break;

        /*
        case TypeClass_VOID:            break;
        case TypeClass_UNKNOWN:         break;
        */

        case TypeClass_ANY:             eRetType = SbxVARIANT;  break;
        case TypeClass_BOOLEAN:         eRetType = SbxBOOL;     break;
        case TypeClass_CHAR:            eRetType = SbxCHAR;     break;
        case TypeClass_STRING:          eRetType = SbxSTRING;   break;
        case TypeClass_FLOAT:           eRetType = SbxSINGLE;   break;
        case TypeClass_DOUBLE:          eRetType = SbxDOUBLE;   break;
        //case TypeClass_OCTET:                                 break;
        case TypeClass_BYTE:            eRetType = SbxINTEGER;  break;
        //case TypeClass_INT:               eRetType = SbxINT;  break;
        case TypeClass_SHORT:           eRetType = SbxINTEGER;  break;
        case TypeClass_LONG:            eRetType = SbxLONG;     break;
        case TypeClass_HYPER:           eRetType = SbxSALINT64; break;
        //case TypeClass_UNSIGNED_OCTET:                        break;
        case TypeClass_UNSIGNED_SHORT:  eRetType = SbxUSHORT;   break;
        case TypeClass_UNSIGNED_LONG:   eRetType = SbxULONG;    break;
        case TypeClass_UNSIGNED_HYPER:  eRetType = SbxSALUINT64;break;
        //case TypeClass_UNSIGNED_INT:  eRetType = SbxUINT;     break;
        //case TypeClass_UNSIGNED_BYTE: eRetType = SbxUSHORT;   break;
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

void unoToSbxValue( SbxVariable* pVar, const Any& aValue )
{
    Type aType = aValue.getValueType();
    TypeClass eTypeClass = aType.getTypeClass();
    switch( eTypeClass )
    {
        case TypeClass_TYPE:
        {
            // Map Type to IdlClass
            Type aType;
            aValue >>= aType;
            Reference<XIdlClass> xClass = TypeToIdlClass( aType );
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
        {
            if( eTypeClass == TypeClass_STRUCT )
            {
                SbiInstance* pInst = pINST;
                if( pInst && pInst->IsCompatibility() )
                {
                    oleautomation::Date aDate;
                    if( (aValue >>= aDate) )
                    {
                        pVar->PutDate( aDate.Value );
                        break;
                    }
                    else
                    {
                        oleautomation::Decimal aDecimal;
                        if( (aValue >>= aDecimal) )
                        {
                            pVar->PutDecimal( aDecimal );
                            break;
                        }
                        else
                        {
                            oleautomation::Currency aCurrency;
                            if( (aValue >>= aCurrency) )
                            {
                                sal_Int64 nValue64 = aCurrency.Value;
                                SbxINT64 aInt64;
                                aInt64.nHigh = nValue64 >> 32;
                                aInt64.nLow = (UINT32)( nValue64 & 0xffffffff );
                                pVar->PutCurrency( aInt64 );
                                break;
                            }
                        }
                    }
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
        case TypeClass_SERVICE:         break;
        case TypeClass_CLASS:           break;
        case TypeClass_TYPEDEF:         break;
        case TypeClass_UNION:           break;
        case TypeClass_ENUM:            break;
        case TypeClass_EXCEPTION:       break;
        case TypeClass_ARRAY:           break;
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
        case TypeClass_VOID:            break;
        case TypeClass_UNKNOWN:         break;

        case TypeClass_ANY:
        {
            // Any rausholen und konvertieren
            //Any* pAny = (Any*)aValue.get();
            //if( pAny )
                //unoToSbxValue( pVar, *pAny );
        }
        break;
        */

        case TypeClass_BOOLEAN:         pVar->PutBool( *(sal_Bool*)aValue.getValue() ); break;
        case TypeClass_CHAR:
        {
            pVar->PutChar( *(sal_Unicode*)aValue.getValue() );
            break;
        }
        case TypeClass_STRING:          { OUString val; aValue >>= val; pVar->PutString( String( val ) ); } break;
        case TypeClass_FLOAT:           { float val; aValue >>= val; pVar->PutSingle( val ); } break;
        case TypeClass_DOUBLE:          { double val; aValue >>= val; pVar->PutDouble( val ); } break;
        //case TypeClass_OCTET:         break;
        case TypeClass_BYTE:            { sal_Int8 val; aValue >>= val; pVar->PutInteger( val ); } break;
        //case TypeClass_INT:           break;
        case TypeClass_SHORT:           { sal_Int16 val; aValue >>= val; pVar->PutInteger( val ); } break;
        case TypeClass_LONG:            { sal_Int32 val; aValue >>= val; pVar->PutLong( val ); } break;
        case TypeClass_HYPER:           { sal_Int64 val; aValue >>= val; pVar->PutInt64( val ); } break;
        //case TypeClass_UNSIGNED_OCTET:break;
        case TypeClass_UNSIGNED_SHORT:  { sal_uInt16 val; aValue >>= val; pVar->PutUShort( val ); } break;
        case TypeClass_UNSIGNED_LONG:   { sal_uInt32 val; aValue >>= val; pVar->PutULong( val ); } break;
        case TypeClass_UNSIGNED_HYPER:  { sal_uInt64 val; aValue >>= val; pVar->PutUInt64( val ); } break;
        //case TypeClass_UNSIGNED_INT:  break;
        //case TypeClass_UNSIGNED_BYTE: break;
        default:                        pVar->PutEmpty();                       break;
    }
}

// Reflection fuer Sbx-Typen liefern
Type getUnoTypeForSbxBaseType( SbxDataType eType )
{
    Type aRetType = getCppuVoidType();
    switch( eType )
    {
        //case SbxEMPTY:        eRet = TypeClass_VOID; break;
        case SbxNULL:       aRetType = ::getCppuType( (const Reference< XInterface > *)0 ); break;
        case SbxINTEGER:    aRetType = ::getCppuType( (sal_Int16*)0 ); break;
        case SbxLONG:       aRetType = ::getCppuType( (sal_Int32*)0 ); break;
        case SbxSINGLE:     aRetType = ::getCppuType( (float*)0 ); break;
        case SbxDOUBLE:     aRetType = ::getCppuType( (double*)0 ); break;
        case SbxCURRENCY:   aRetType = ::getCppuType( (oleautomation::Currency*)0 ); break;
        case SbxDECIMAL:    aRetType = ::getCppuType( (oleautomation::Decimal*)0 ); break;
        case SbxDATE:       {
                            SbiInstance* pInst = pINST;
                            if( pInst && pInst->IsCompatibility() )
                                aRetType = ::getCppuType( (double*)0 );
                            else
                                aRetType = ::getCppuType( (oleautomation::Date*)0 );
                            }
                            break;
        // case SbxDATE:        aRetType = ::getCppuType( (double*)0 ); break;
        case SbxSTRING:     aRetType = ::getCppuType( (OUString*)0 ); break;
        //case SbxOBJECT:   break;
        //case SbxERROR:    break;
        case SbxBOOL:       aRetType = ::getCppuType( (sal_Bool*)0 ); break;
        case SbxVARIANT:    aRetType = ::getCppuType( (Any*)0 ); break;
        //case SbxDATAOBJECT: break;
        case SbxCHAR:       aRetType = ::getCppuType( (sal_Unicode*)0 ); break;
        case SbxBYTE:       aRetType = ::getCppuType( (sal_Int16*)0 ); break;
        case SbxUSHORT:     aRetType = ::getCppuType( (sal_uInt16*)0 ); break;
        case SbxULONG:      aRetType = ::getCppuType( (sal_uInt32*)0 ); break;
        //case SbxLONG64:   break;
        //case SbxULONG64:  break;
        // Maschinenabhaengige zur Sicherheit auf Hyper abbilden
        case SbxINT:        aRetType = ::getCppuType( (sal_Int32*)0 ); break;
        case SbxUINT:       aRetType = ::getCppuType( (sal_uInt32*)0 ); break;
        //case SbxVOID:     break;
        //case SbxHRESULT:  break;
        //case SbxPOINTER:  break;
        //case SbxDIMARRAY: break;
        //case SbxCARRAY:   break;
        //case SbxUSERDEF:  break;
        //case SbxLPSTR:    break;
        //case SbxLPWSTR:   break;
        //case SbxCoreSTRING: break;
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

            // es muss ein eindimensionales Array sein
            sal_Int32 nLower, nUpper;
            if( pArray->GetDims() == 1 && pArray->GetDim32( 1, nLower, nUpper ) )
            {
                Type aElementType = getUnoTypeForSbxBaseType( (SbxDataType)(pArray->GetType() & 0xfff) );

                if( aElementType.getTypeClass() == TypeClass_VOID )
                {
                    // Wenn alle Elemente des Arrays vom gleichen Typ sind, wird
                    // der genommen, sonst wird das ganze als Any-Sequence betrachtet
                    sal_Bool bNeedsInit = sal_True;

                    INT32 nSize = nUpper - nLower + 1;
                    INT32 nIdx = nLower;
                    for( sal_uInt32 i = 0 ; i < nSize ; i++,nIdx++ )
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

                OUString aSeqTypeName( RTL_CONSTASCII_USTRINGPARAM("[]") );
                aSeqTypeName += aElementType.getTypeName();
                aRetType = Type( TypeClass_SEQUENCE, aSeqTypeName );
            }
            // Ein Array mit Dim > 1 wird nicht konvertiert -> default==void liefern
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
        }
    }

    return sbxToUnoValue( pVar, aType );
}

// Map old interface
Any sbxToUnoValue( SbxVariable* pVar )
{
    return sbxToUnoValueImpl( pVar );
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
            else
            {
                // #112368 Special conversion for Decimal, Currency and Date
                if( eType == TypeClass_STRUCT )
                {
                    SbiInstance* pInst = pINST;
                    if( pInst && pInst->IsCompatibility() )
                    {
                        if( rType == ::getCppuType( (oleautomation::Decimal*)0 ) )
                        {
                            oleautomation::Decimal aDecimal;
                            pVar->fillAutomationDecimal( aDecimal );
                            aRetVal <<= aDecimal;
                            break;
                        }
                        else if( rType == ::getCppuType( (oleautomation::Currency*)0 ) )
                        {
                            SbxINT64 aInt64 = pVar->GetCurrency();
                            oleautomation::Currency aCurrency;
                            sal_Int64& rnValue64 = aCurrency.Value;
                            rnValue64 = aInt64.nHigh;
                            rnValue64 <<= 32;
                            rnValue64 |= aInt64.nLow;
                            aRetVal <<= aCurrency;
                            break;
                        }
                        else if( rType == ::getCppuType( (oleautomation::Date*)0 ) )
                        {
                            oleautomation::Date aDate;
                            aDate.Value = pVar->GetDate();
                            aRetVal <<= aDate;
                            break;
                        }
                    }
                }

                SbxBaseRef pObj = (SbxBase*)pVar->GetObject();
                if( pObj && pObj->ISA(SbUnoObject) )
                {
                    aRetVal = ((SbUnoObject*)(SbxBase*)pObj)->getUnoAny();
                }
                else
                {
                    // #109936 NULL object -> NULL XInterface
                    Reference<XInterface> xInt;
                    aRetVal <<= xInt;
                }
            }
        }
        break;

        /* folgende Typen lassen wir erstmal weg
        case TypeClass_SERVICE:         break;
        case TypeClass_CLASS:           break;
        case TypeClass_TYPEDEF:         break;
        case TypeClass_UNION:           break;
        case TypeClass_ENUM:            break;
        case TypeClass_EXCEPTION:       break;
        case TypeClass_ARRAY:           break;
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

                // Instanz der geforderten Sequence erzeugen
                Reference< XIdlClass > xIdlTargetClass = TypeToIdlClass( rType );
                xIdlTargetClass->createObject( aRetVal );

                // es muss ein eindimensionales Array sein
                sal_Int32 nLower, nUpper;
                if( pArray->GetDims() == 1 && pArray->GetDim32( 1, nLower, nUpper ) )
                {
                    sal_Int32 nSeqSize = nUpper - nLower + 1;

                    Reference< XIdlArray > xArray = xIdlTargetClass->getArray();
                    xArray->realloc( aRetVal, nSeqSize );

                    // Element-Type
                    OUString aClassName = xIdlTargetClass->getName();
                    typelib_TypeDescription * pSeqTD = 0;
                    typelib_typedescription_getByName( &pSeqTD, aClassName.pData );
                    OSL_ASSERT( pSeqTD );
#if SUPD > 600
                    Type aElemType( ((typelib_IndirectTypeDescription *)pSeqTD)->pType );
#else
                    typelib_TypeDescription * pElementTD =
                        ((typelib_IndirectTypeDescription *)pSeqTD)->pType;
                    Type aElemType( pElementTD->pWeakRef );
#endif
                    Reference< XIdlClass > xElementClass = TypeToIdlClass( aElemType );

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
                        catch (IndexOutOfBoundsException& e2)
                        {
                            StarBASIC::Error( SbERR_OUT_OF_RANGE );
                        }
                    }
                }
            }
        }
        break;

        /*
        case TypeClass_VOID:            break;
        case TypeClass_UNKNOWN:         break;
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
        case TypeClass_STRING:          aRetVal <<= OUString( pVar->GetString() ); break;
        case TypeClass_FLOAT:           aRetVal <<= pVar->GetSingle(); break;
        case TypeClass_DOUBLE:          aRetVal <<= pVar->GetDouble(); break;
        //case TypeClass_OCTET:         break;

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
        //case TypeClass_INT:           break;
        case TypeClass_SHORT:           aRetVal <<= (sal_Int16)( pVar->GetInteger() );  break;
        case TypeClass_LONG:            aRetVal <<= (sal_Int32)( pVar->GetLong() );     break;
        case TypeClass_HYPER:           aRetVal <<= (sal_Int64)( pVar->GetInt64() );    break;
        //case TypeClass_UNSIGNED_OCTET:break;
        case TypeClass_UNSIGNED_SHORT:  aRetVal <<= (sal_uInt16)( pVar->GetUShort() );  break;
        case TypeClass_UNSIGNED_LONG:   aRetVal <<= (sal_uInt32)( pVar->GetULong() );   break;
        case TypeClass_UNSIGNED_HYPER:  aRetVal <<= (sal_uInt64)( pVar->GetUInt64() );  break;
        //case TypeClass_UNSIGNED_INT:  break;
        //case TypeClass_UNSIGNED_BYTE: break;
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

String getBasicObjectTypeName( SbxObject* pObj )
{
    String aName;
    if( pObj )
    {
        SbUnoObject* pUnoObj = PTR_CAST(SbUnoObject,pObj);
        if( pUnoObj )
            aName = getDbgObjectNameImpl( pUnoObj );
    }
    return aName;
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
    switch( eType )
    {
        case SbxEMPTY:      aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxEMPTY") ); break;
        case SbxNULL:       aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxNULL") ); break;
        case SbxINTEGER:    aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxINTEGER") ); break;
        case SbxLONG:       aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxLONG") ); break;
        case SbxSINGLE:     aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxSINGLE") ); break;
        case SbxDOUBLE:     aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxDOUBLE") ); break;
        case SbxCURRENCY:   aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxCURRENCY") ); break;
        case SbxDECIMAL:    aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxDECIMAL") ); break;
        case SbxDATE:       aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxDATE") ); break;
        case SbxSTRING:     aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxSTRING") ); break;
        case SbxOBJECT:     aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxOBJECT") ); break;
        case SbxERROR:      aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxERROR") ); break;
        case SbxBOOL:       aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxBOOL") ); break;
        case SbxVARIANT:    aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxVARIANT") ); break;
        case SbxDATAOBJECT: aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxDATAOBJECT") ); break;
        case SbxCHAR:       aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxCHAR") ); break;
        case SbxBYTE:       aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxBYTE") ); break;
        case SbxUSHORT:     aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxUSHORT") ); break;
        case SbxULONG:      aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxULONG") ); break;
        case SbxLONG64:     aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxLONG64") ); break;
        case SbxULONG64:    aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxULONG64") ); break;
        case SbxINT:        aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxINT") ); break;
        case SbxUINT:       aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxUINT") ); break;
        case SbxVOID:       aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxVOID") ); break;
        case SbxHRESULT:    aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxHRESULT") ); break;
        case SbxPOINTER:    aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxPOINTER") ); break;
        case SbxDIMARRAY:   aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxDIMARRAY") ); break;
        case SbxCARRAY:     aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxCARRAY") ); break;
        case SbxUSERDEF:    aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxUSERDEF") ); break;
        case SbxLPSTR:      aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxLPSTR") ); break;
        case SbxLPWSTR:     aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxLPWSTR") ); break;
        case SbxCoreSTRING: aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxCoreSTRING" ) ); break;
        case SbxOBJECT | SbxARRAY: aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxARRAY") ); break;
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
    if( bNeedIntrospection ) doIntrospection();

    const SbxHint* pHint = PTR_CAST(SbxHint,&rHint);
    if( pHint )
    {
        SbxVariable* pVar = pHint->GetVar();
        SbxArray* pParams = pVar->GetParameters();
        SbUnoProperty* pProp = PTR_CAST(SbUnoProperty,pVar);
        SbUnoMethod* pMeth = PTR_CAST(SbUnoMethod,pVar);
        if( pProp )
        {
            if( pHint->GetId() == SBX_HINT_DATAWANTED )
            {
                // Test-Properties
                INT32 nId = (INT32)pProp->nId;
                if( nId < 0 )
                {
                    // Id == -1: Implementierte Interfaces gemaess ClassProvider anzeigen
                    if( nId == -1 )     // Property ID_DBG_SUPPORTEDINTERFACES"
                    {
                        String aRetStr = Impl_GetSupportedInterfaces( this );
                        pVar->PutString( aRetStr );
                    }
                    // Id == -2: Properties ausgeben
                    else if( nId == -2 )        // Property ID_DBG_PROPERTIES
                    {
                        // Jetzt muessen alle Properties angelegt werden
                        implCreateAll();
                        String aRetStr = Impl_DumpProperties( this );
                        pVar->PutString( aRetStr );
                    }
                    // Id == -3: Methoden ausgeben
                    else if( nId == -3 )        // Property ID_DBG_METHODS
                    {
                        // Jetzt muessen alle Properties angelegt werden
                        implCreateAll();
                        String aRetStr = Impl_DumpMethods( this );
                        pVar->PutString( aRetStr );
                    }
                    return;
                }

                if( mxUnoAccess.is() )
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
                    catch( WrappedTargetException& e1 )
                    {
                        StarBASIC::Error( ERRCODE_BASIC_EXCEPTION, implGetWrappedMsg( e1 ) );
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
                else if( mxInvocation.is() )
                {
                    try
                    {
                        // Wert holen
                        Any aRetAny = mxInvocation->getValue( pProp->GetName() );

                        // Wert von Uno nach Sbx uebernehmen
                        unoToSbxValue( pVar, aRetAny );
                    }
                    catch( WrappedTargetException& e1 )
                    {
                        StarBASIC::Error( ERRCODE_BASIC_EXCEPTION, implGetWrappedMsg( e1 ) );
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
                if( mxUnoAccess.is() )
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
                    catch( WrappedTargetException& e1 )
                    {
                        StarBASIC::Error( ERRCODE_BASIC_EXCEPTION, implGetWrappedMsg( e1 ) );
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
                else if( mxInvocation.is() )
                {
                    // Wert von Uno nach Sbx uebernehmen
                    Any aAnyValue = sbxToUnoValueImpl( pVar );
                    try
                    {
                        // Wert setzen
                        mxInvocation->setValue( pProp->GetName(), aAnyValue );
                    }
                    catch( WrappedTargetException& e1 )
                    {
                        StarBASIC::Error( ERRCODE_BASIC_EXCEPTION, implGetWrappedMsg( e1 ) );
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
            if( pHint->GetId() == SBX_HINT_DATAWANTED )
            {
                // Anzahl Parameter -1 wegen Param0 == this
                UINT32 nParamCount = pParams ? ((UINT32)pParams->Count() - 1) : 0;
                Sequence<Any> args;
                BOOL bOutParams = FALSE;
                UINT32 i;

                if( mxUnoAccess.is() )
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
                        SbiInstance* pInst = pINST;
                        if( pInst && pInst->IsCompatibility() )
                        {
                            // Check types
                            bool bError = false;
                            for( i = nParamCount ; i < nUnoParamCount ; i++ )
                            {
                                const ParamInfo& rInfo = pParamInfos[i];
                                const Reference< XIdlClass >& rxClass = rInfo.aType;
                                if( rxClass->getTypeClass() != TypeClass_ANY )
                                {
                                    bError = true;
                                    StarBASIC::Error( SbERR_NOT_OPTIONAL );
                                }
                            }
                            if( !bError )
                                nAllocParamCount = nUnoParamCount;
                        }
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
                else if( pParams && mxInvocation.is() )
                {
                    bool bOLEAutomation = true;
                    // TODO: bOLEAutomation = xOLEAutomation.is()

                    AutomationNamedArgsSbxArray* pArgNamesArray = NULL;
                    if( bOLEAutomation )
                        pArgNamesArray = PTR_CAST(AutomationNamedArgsSbxArray,pParams);

                    args.realloc( nParamCount );
                    Any* pAnyArgs = args.getArray();
                    bool bBlockConversionToSmallestType = pINST->IsCompatibility();
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
                    if( mxUnoAccess.is() )
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

                            UINT32 i;
                            for( i = 0 ; i < nParamCount ; i++ )
                            {
                                const ParamInfo& rInfo = pParamInfos[i];
                                ParamMode aParamMode = rInfo.aMode;
                                if( aParamMode != ParamMode_IN )
                                    unoToSbxValue( (SbxVariable*)pParams->Get( (USHORT)(i+1) ), pAnyArgs[ i ] );
                            }
                        }
                    }
                    else if( mxInvocation.is() )
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
                            for( UINT32 i = 0 ; i < nLen ; i++ )
                            {
                                INT16 iTarget = pIndices[ i ];
                                if( iTarget >= nParamCount )
                                    break;
                                unoToSbxValue( (SbxVariable*)pParams->Get( (USHORT)(i+1) ), pNewValues[ i ] );
                            }
                        }
                    }

                    // #55460, Parameter hier weghauen, da das in unoToSbxValue()
                    // bei Arrays wegen #54548 nicht mehr gemacht wird
                    if( pParams )
                        pVar->SetParameters( NULL );
                }
                catch( WrappedTargetException& e1 )
                {
                    StarBASIC::Error( ERRCODE_BASIC_EXCEPTION, implGetWrappedMsg( e1 ) );
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

SbUnoObject::SbUnoObject( const String& aName, const Any& aUnoObj_ )
    : SbxObject( aName )
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

#ifdef INVOCATION_ONLY
    // Invocation besorgen
    mxInvocation = createDynamicInvocationFor( aUnoObj_ );
#else
    // Hat das Object selbst eine Invocation?
    mxInvocation = Reference< XInvocation >( x, UNO_QUERY );
#endif

    if( mxInvocation.is() )
    {
        // #94670: This is WRONG because then the MaterialHolder doesn't refer
        // to the object implementing XInvocation but to the object passed to
        // the invocation service!!!
        // mxMaterialHolder = Reference< XMaterialHolder >::query( mxInvocation );

        // ExactName holen
        mxExactName = Reference< XExactName >::query( mxInvocation );

        // Rest bezieht sich nur auf Introspection
        bNeedIntrospection = FALSE;
        return;
    }

    // Introspection-Flag
    bNeedIntrospection = TRUE;
    maTmpUnoObj = aUnoObj_;


    //*** Namen bestimmen ***
    BOOL bFatalError = TRUE;

    // Ist es ein Interface oder eine struct?
    BOOL bSetClassName = FALSE;
    String aClassName;
    if( eType == TypeClass_STRUCT )
    {
        // Struct ist Ok
        bFatalError = FALSE;

        // #67173 Echten Klassen-Namen eintragen
        if( aName.Len() == 0 )
        {
            aClassName = String( aUnoObj_.getValueType().getTypeName() );
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
            if( aName.Len() == 0 )
            {
                Sequence< Reference< XIdlClass > > szClasses = xClassProvider->getIdlClasses();
                UINT32 nLen = szClasses.getLength();
                if( nLen )
                {
                    const Reference< XIdlClass > xImplClass = szClasses.getConstArray()[ 0 ];
                    if( xImplClass.is() )
                    {
                        aClassName = String( xImplClass->getName() );
                        bSetClassName = TRUE;
                    }
                }
            }
        }
    }
    if( bSetClassName )
        SetClassName( aClassName );

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

void clearUnoMethods( void )
{
    SbUnoMethod* pMeth = pFirst;
    while( pMeth )
    {
        pMeth->SbxValue::Clear();
        pMeth = pMeth->pNext;
    }
}


SbUnoMethod::SbUnoMethod
(
    const String& aName,
    SbxDataType eSbxType,
    Reference< XIdlMethod > xUnoMethod_
)
    : SbxMethod( aName, eSbxType )
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
    if( !pInfo && m_xUnoMethod.is() )
    {
        SbiInstance* pInst = pINST;
        if( pInst && pInst->IsCompatibility() )
        {
            pInfo = new SbxInfo();

            const Sequence<ParamInfo>& rInfoSeq = getParamInfos();
            const ParamInfo* pParamInfos = rInfoSeq.getConstArray();
            UINT32 nParamCount = rInfoSeq.getLength();

            for( int i = 0 ; i < nParamCount ; i++ )
            {
                const ParamInfo& rInfo = pParamInfos[i];
                OUString aParamName = rInfo.aName;

                // const Reference< XIdlClass >& rxClass = rInfo.aType;
                SbxDataType t = SbxVARIANT;
                USHORT nFlags = SBX_READ;
                pInfo->AddParam( aParamName, t, nFlags );
            }
        }
    }
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
    const String& aName,
    SbxDataType eSbxType,
    const Property& aUnoProp_,
    UINT32 nId_
)
    : SbxProperty( aName, eSbxType )
{
    aUnoProp = aUnoProp_;
    nId = nId_;

    // #54548, bei bedarf Dummy-Array einsetzen, damit SbiRuntime::CheckArray() geht
    static SbxArrayRef xDummyArray = new SbxArray( SbxVARIANT );
    if( eSbxType & SbxARRAY )
        PutObject( xDummyArray );
}

SbUnoProperty::~SbUnoProperty()
{}


// #72732 Spezielle SbxVariable, die beim put/get prueft,
// ob der Kontext fuer eine UnoClass sinnvoll ist. Sonst
// liegt eventuell ein Schreibfehler im Basic-Source vor.
BOOL UnoClassMemberVariable::Get( SbxValues& rRes ) const
{
    // Zugriff auf den Member einer UnoClass mit Parametern ist unsinnig
    if( GetParameters() )
    {
        if( mpRuntime )
            mpRuntime->Error( SbERR_NO_METHOD );
    }
    return SbxVariable::Get( rRes );
}

BOOL UnoClassMemberVariable::Put( const SbxValues& rRes )
{
    if( bInternalUse )
    {
        return SbxVariable::Put( rRes );
    }
    // Schreibzugriff auf den Member einer UnoClass ist immer falsch
    mpRuntime->Error( SbERR_NO_METHOD );
    return FALSE;
}

TYPEINIT1(UnoClassMemberVariable,SbxVariable)


SbxVariable* SbUnoObject::Find( const XubString& rName, SbxClassType t )
{
    static Reference< XIdlMethod > xDummyMethod;
    static Property aDummyProp;

    SbxVariable* pRes = SbxObject::Find( rName, t );

    if( bNeedIntrospection ) doIntrospection();

    // Neu 4.3.1999: Properties on Demand anlegen, daher jetzt perIntrospectionAccess
    // suchen, ob doch eine Property oder Methode des geforderten Namens existiert
    if( !pRes )
    {
        OUString aUName( rName );
        if( mxExactName.is() )
        {
            OUString aUExactName = mxExactName->getExactName( aUName );
            if( aUExactName.getLength() )
                aUName = aUExactName;
        }
        if( mxUnoAccess.is() )
        {
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
                SbxVariableRef xVarRef = new SbUnoProperty( rProp.Name, eSbxType, rProp, 0 );
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
                SbxVariableRef xMethRef = new SbUnoMethod
                    ( rxMethod->getName(), unoToSbxType( rxMethod->getReturnType() ), rxMethod );
                QuickInsert( (SbxVariable*)xMethRef );
                pRes = xMethRef;
            }

            // Wenn immer noch nichts gefunden wurde, muss geprueft werden, ob NameAccess vorliegt
            if( !pRes )
            {
                try
                {
                    Reference< XNameAccess > xNameAccess( mxUnoAccess->queryAdapter( ::getCppuType( (const Reference< XPropertySet > *)0 ) ), UNO_QUERY );
                    OUString aUName( rName );

                    if( xNameAccess.is() && xNameAccess->hasByName( aUName ) )
                    {
                        Any aAny = xNameAccess->getByName( aUName );

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
                catch( WrappedTargetException& e1 )
                {
                    // Anlegen, damit der Exception-Fehler nicht ueberschrieben wird
                    if( !pRes )
                        pRes = new SbxVariable( SbxVARIANT );

                    StarBASIC::Error( ERRCODE_BASIC_EXCEPTION, implGetWrappedMsg( e1 ) );
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
        else if( mxInvocation.is() )
        {
            try
            {
                if( mxInvocation->hasProperty( aUName ) )
                {
                    // Property anlegen und reinbraten
                    SbxVariableRef xVarRef = new SbUnoProperty( aUName, SbxVARIANT, aDummyProp, 0 );
                    QuickInsert( (SbxVariable*)xVarRef );
                    pRes = xVarRef;
                }
                else if( mxInvocation->hasMethod( aUName ) )
                {
                    // SbUnoMethode anlegen und reinbraten
                    SbxVariableRef xMethRef = new SbUnoMethod( aUName, SbxVARIANT, xDummyMethod );
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
    SbxVariableRef xVarRef = new SbUnoProperty( ID_DBG_SUPPORTEDINTERFACES, SbxSTRING, aProp, -1 );
    QuickInsert( (SbxVariable*)xVarRef );

    // Id == -2: Properties ausgeben
    xVarRef = new SbUnoProperty( ID_DBG_PROPERTIES, SbxSTRING, aProp, -2 );
    QuickInsert( (SbxVariable*)xVarRef );

    // Id == -3: Methoden ausgeben
    xVarRef = new SbUnoProperty( ID_DBG_METHODS, SbxSTRING, aProp, -3 );
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
    const Property* pProps = props.getConstArray();

    UINT32 i;
    for( i = 0 ; i < nPropCount ; i++ )
    {
        const Property& rProp = pProps[ i ];

        // #58455 Wenn die Property void sein kann, muss als Typ Variant gesetzt werden
        SbxDataType eSbxType;
        if( rProp.Attributes & PropertyAttribute::MAYBEVOID )
            eSbxType = SbxVARIANT;
        else
            eSbxType = unoToSbxType( rProp.Type.getTypeClass() );

        // Property anlegen und reinbraten
        SbxVariableRef xVarRef = new SbUnoProperty( rProp.Name, eSbxType, rProp, i );
        QuickInsert( (SbxVariable*)xVarRef );
    }

    // Dbg_-Properties anlegen
    implCreateDbgProperties();

    // Methoden anlegen
    Sequence< Reference< XIdlMethod > > aMethodSeq = xAccess->getMethods
        ( MethodConcept::ALL - MethodConcept::DANGEROUS );
    UINT32 nMethCount = aMethodSeq.getLength();
    const Reference< XIdlMethod >* pMethods = aMethodSeq.getConstArray();
    for( i = 0 ; i < nMethCount ; i++ )
    {
        // Methode ansprechen
        const Reference< XIdlMethod >& rxMethod = pMethods[i];

        // SbUnoMethode anlegen und reinbraten
        SbxVariableRef xMethRef = new SbUnoMethod
            ( rxMethod->getName(), unoToSbxType( rxMethod->getReturnType() ), rxMethod );
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
    Reference< XIdlClass > xClass = xCoreReflection->forName( aClassName );
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
SbxBase* SbUnoFactory::Create( UINT16 nSbxId, UINT32 )
{
    // Ueber SbxId laeuft in Uno nix
    return NULL;
}

SbxObject* SbUnoFactory::CreateObject( const String& rClassName )
{
    return Impl_CreateUnoStruct( rClassName );
}


// Provisorische Schnittstelle fuer UNO-Anbindung
// Liefert ein SbxObject, das ein Uno-Interface wrappt
SbxObjectRef GetSbUnoObject( const String& aName, const Any& aUnoObj_ )
{
    return new SbUnoObject( aName, aUnoObj_ );
}

// Force creation of all properties for debugging
void createAllObjectProperties( SbxObject* pObj )
{
    if( !pObj )
        return;

    SbUnoObject* pUnoObj = PTR_CAST(SbUnoObject,pObj);
    if( pUnoObj )
        pUnoObj->createAllProperties();
    else
        pObj->GetAll( SbxCLASS_DONTCARE );
}


void RTL_Impl_CreateUnoStruct( StarBASIC* pBasic, SbxArray& rPar, BOOL bWrite )
{
    // Wir brauchen mindestens 1 Parameter
    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    // Klassen-Name der struct holen
    String aClassName = rPar.Get(1)->GetString();

    // Versuchen, gleichnamige Struct zu erzeugen
    SbUnoObjectRef xUnoObj = Impl_CreateUnoStruct( aClassName );
    if( !xUnoObj )
        return;

    // Objekt zurueckliefern
    SbxVariableRef refVar = rPar.Get(0);
    refVar->PutObject( (SbUnoObject*)xUnoObj );
}

void RTL_Impl_CreateUnoService( StarBASIC* pBasic, SbxArray& rPar, BOOL bWrite )
{
    // Wir brauchen mindestens 1 Parameter
    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    // Klassen-Name der struct holen
    String aServiceName = rPar.Get(1)->GetString();

    // Service suchen und instanzieren
    Reference< XMultiServiceFactory > xFactory( comphelper::getProcessServiceFactory() );
    Reference< XInterface > xInterface;
    if ( xFactory.is() )
    {
        try
        {
            xInterface = xFactory->createInstance( aServiceName );
        }
        catch( WrappedTargetException& e1 )
        {
            StarBASIC::Error( ERRCODE_BASIC_EXCEPTION, implGetWrappedMsg( e1 ) );
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

    SbxVariableRef refVar = rPar.Get(0);
    if( xInterface.is() )
    {
        Any aAny;
        aAny <<= xInterface;

        // SbUnoObject daraus basteln und zurueckliefern
        SbUnoObjectRef xUnoObj = new SbUnoObject( aServiceName, aAny );
        if( xUnoObj->getUnoAny().getValueType().getTypeClass() != TypeClass_VOID )
        {
            // Objekt zurueckliefern
            refVar->PutObject( (SbUnoObject*)xUnoObj );
        }
        else
        {
            refVar->PutObject( NULL );
        }
    }
    else
    {
        refVar->PutObject( NULL );
    }
}

void RTL_Impl_GetProcessServiceManager( StarBASIC* pBasic, SbxArray& rPar, BOOL bWrite )
{
    SbxVariableRef refVar = rPar.Get(0);

    // Globalen Service-Manager holen
    Reference< XMultiServiceFactory > xFactory( comphelper::getProcessServiceFactory() );
    if( xFactory.is() )
    {
        Any aAny;
        aAny <<= xFactory;

        // SbUnoObject daraus basteln und zurueckliefern
        SbUnoObjectRef xUnoObj = new SbUnoObject( String( RTL_CONSTASCII_USTRINGPARAM("ProcessServiceManager") ), aAny );
        refVar->PutObject( (SbUnoObject*)xUnoObj );
    }
    else
    {
        refVar->PutObject( NULL );
    }
}

void RTL_Impl_HasInterfaces( StarBASIC* pBasic, SbxArray& rPar, BOOL bWrite )
{
    // Wir brauchen mindestens 2 Parameter
    USHORT nParCount = rPar.Count();
    if( nParCount < 3 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    // Variable fuer Rueckgabewert
    SbxVariableRef refVar = rPar.Get(0);
    refVar->PutBool( FALSE );

    // Uno-Objekt holen
    SbxBaseRef pObj = (SbxBase*)rPar.Get( 1 )->GetObject();
    if( !(pObj && pObj->ISA(SbUnoObject)) )
        return;
    Any aAny = ((SbUnoObject*)(SbxBase*)pObj)->getUnoAny();
    TypeClass eType = aAny.getValueType().getTypeClass();
    if( eType != TypeClass_INTERFACE )
        return;

    // Interface aus dem Any besorgen
    Reference< XInterface > x = *(Reference< XInterface >*)aAny.getValue();

    // CoreReflection holen
    Reference< XIdlReflection > xCoreReflection = getCoreReflection_Impl();
    if( !xCoreReflection.is() )
        return;

    for( USHORT i = 2 ; i < nParCount ; i++ )
    {
        // Interface-Name der struct holen
        String aIfaceName = rPar.Get( i )->GetString();

        // Klasse suchen
        Reference< XIdlClass > xClass = xCoreReflection->forName( aIfaceName );
        if( !xClass.is() )
            return;

        // Pruefen, ob das Interface unterstuetzt wird
        OUString aClassName = xClass->getName();
        Type aClassType( xClass->getTypeClass(), aClassName.getStr() );
        if( !x->queryInterface( aClassType ).hasValue() )
            return;
    }

    // Alles hat geklappt, dann TRUE liefern
    refVar->PutBool( TRUE );
}

void RTL_Impl_IsUnoStruct( StarBASIC* pBasic, SbxArray& rPar, BOOL bWrite )
{
    // Wir brauchen mindestens 1 Parameter
    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    // Variable fuer Rueckgabewert
    SbxVariableRef refVar = rPar.Get(0);
    refVar->PutBool( FALSE );

    // Uno-Objekt holen
    SbxVariableRef xParam = rPar.Get( 1 );
    if( !xParam->IsObject() )
        return;
    SbxBaseRef pObj = (SbxBase*)rPar.Get( 1 )->GetObject();
    if( !(pObj && pObj->ISA(SbUnoObject)) )
        return;
    Any aAny = ((SbUnoObject*)(SbxBase*)pObj)->getUnoAny();
    TypeClass eType = aAny.getValueType().getTypeClass();
    if( eType == TypeClass_STRUCT )
        refVar->PutBool( TRUE );
}


void RTL_Impl_EqualUnoObjects( StarBASIC* pBasic, SbxArray& rPar, BOOL bWrite )
{
    if ( rPar.Count() < 3 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    // Variable fuer Rueckgabewert
    SbxVariableRef refVar = rPar.Get(0);
    refVar->PutBool( FALSE );

    // Uno-Objekte holen
    SbxVariableRef xParam1 = rPar.Get( 1 );
    if( !xParam1->IsObject() )
        return;
    SbxBaseRef pObj1 = (SbxBase*)xParam1->GetObject();
    if( !(pObj1 && pObj1->ISA(SbUnoObject)) )
        return;
    Any aAny1 = ((SbUnoObject*)(SbxBase*)pObj1)->getUnoAny();
    TypeClass eType1 = aAny1.getValueType().getTypeClass();
    if( eType1 != TypeClass_INTERFACE )
        return;
    Reference< XInterface > x1;
    aAny1 >>= x1;
    //XInterfaceRef x1 = *(XInterfaceRef*)aAny1.get();

    SbxVariableRef xParam2 = rPar.Get( 2 );
    if( !xParam2->IsObject() )
        return;
    SbxBaseRef pObj2 = (SbxBase*)xParam2->GetObject();
    if( !(pObj2 && pObj2->ISA(SbUnoObject)) )
        return;
    Any aAny2 = ((SbUnoObject*)(SbxBase*)pObj2)->getUnoAny();
    TypeClass eType2 = aAny2.getValueType().getTypeClass();
    if( eType2 != TypeClass_INTERFACE )
        return;
    Reference< XInterface > x2;
    aAny2 >>= x2;
    //XInterfaceRef x2 = *(XInterfaceRef*)aAny2.get();

    if( x1 == x2 )
        refVar->PutBool( TRUE );
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
                catch( WrappedTargetException& e1 )
                {
                    StarBASIC::Error( ERRCODE_BASIC_EXCEPTION, implGetWrappedMsg( e1 ) );
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


//========================================================================
//========================================================================
//========================================================================

// Implementation eines EventAttacher-bezogenen AllListeners, der
// nur einzelne Events an einen allgemeinen AllListener weiterleitet
class BasicAllListener_Impl : public BasicAllListenerHelper
{
    virtual void firing_impl(const AllEventObject& Event, Any* pRet);

public:
    SbxObjectRef    xSbxObj;
    OUString        aPrefixName;

    BasicAllListener_Impl( const OUString& aPrefixName );
    ~BasicAllListener_Impl();

    // Methoden von XInterface
    //virtual BOOL queryInterface( Uik aUik, Reference< XInterface > & rOut );

    // Methoden von XAllListener
    virtual void SAL_CALL firing(const AllEventObject& Event) throw ( RuntimeException );
    virtual Any SAL_CALL approveFiring(const AllEventObject& Event) throw ( RuntimeException );

    // Methoden von XEventListener
    virtual void SAL_CALL disposing(const EventObject& Source) throw ( RuntimeException );
};


//========================================================================
BasicAllListener_Impl::BasicAllListener_Impl
(
    const OUString  & aPrefixName_
)
    : aPrefixName( aPrefixName_ )
{
}

//========================================================================
BasicAllListener_Impl::~BasicAllListener_Impl()
{
}

//========================================================================

void BasicAllListener_Impl::firing_impl( const AllEventObject& Event, Any* pRet )
{
    NAMESPACE_VOS(OGuard) guard( Application::GetSolarMutex() );

    if( xSbxObj.Is() )
    {
        OUString aMethodName = aPrefixName;
        aMethodName = aMethodName + Event.MethodName;

        SbxVariable * pP = xSbxObj;
        while( pP->GetParent() )
        {
            pP = pP->GetParent();
            StarBASIC * pLib = PTR_CAST(StarBASIC,pP);
            if( pLib )
            {
                // In Basic Array anlegen
                SbxArrayRef xSbxArray = new SbxArray( SbxVARIANT );
                const Any * pArgs = Event.Arguments.getConstArray();
                INT32 nCount = Event.Arguments.getLength();
                for( INT32 i = 0; i < nCount; i++ )
                {
                    // Elemente wandeln
                    SbxVariableRef xVar = new SbxVariable( SbxVARIANT );
                    unoToSbxValue( (SbxVariable*)xVar, pArgs[i] );
                    xSbxArray->Put( xVar, i +1 );
                }

                pLib->Call( aMethodName, xSbxArray );

                // Return-Wert aus dem Param-Array holen, wenn verlangt
                if( pRet )
                {
                    SbxVariable* pVar = xSbxArray->Get( 0 );
                    if( pVar )
                    {
                        // #95792 Avoid a second call
                        USHORT nFlags = pVar->GetFlags();
                        pVar->SetFlag( SBX_NO_BROADCAST );
                        *pRet = sbxToUnoValueImpl( pVar );
                        pVar->SetFlags( nFlags );
                    }
                }
                break;
            }
        }
    }
}


// Methoden von XAllListener
void BasicAllListener_Impl::firing( const AllEventObject& Event ) throw ( RuntimeException )
{
    firing_impl( Event, NULL );
}

Any BasicAllListener_Impl::approveFiring( const AllEventObject& Event ) throw ( RuntimeException )
{
    Any aRetAny;
    firing_impl( Event, &aRetAny );
    return aRetAny;
}

//========================================================================
// Methoden von XEventListener
void BasicAllListener_Impl ::disposing(const EventObject& ) throw ( RuntimeException )
{
    NAMESPACE_VOS(OGuard) guard( Application::GetSolarMutex() );

    xSbxObj.Clear();
}



//*************************************************************************
//  class InvocationToAllListenerMapper
//  helper class to map XInvocation to XAllListener (also in project eventattacher!)
//*************************************************************************
class InvocationToAllListenerMapper : public WeakImplHelper1< XInvocation >
{
public:
    InvocationToAllListenerMapper( const Reference< XIdlClass >& ListenerType,
        const Reference< XAllListener >& AllListener, const Any& Helper );

    // XInvocation
    virtual Reference< XIntrospectionAccess > SAL_CALL getIntrospection(void) throw( RuntimeException );
    virtual Any SAL_CALL invoke(const OUString& FunctionName, const Sequence< Any >& Params, Sequence< sal_Int16 >& OutParamIndex, Sequence< Any >& OutParam)
        throw( IllegalArgumentException, CannotConvertException, InvocationTargetException, RuntimeException );
    virtual void SAL_CALL setValue(const OUString& PropertyName, const Any& Value)
        throw( UnknownPropertyException, CannotConvertException, InvocationTargetException, RuntimeException );
    virtual Any SAL_CALL getValue(const OUString& PropertyName) throw( UnknownPropertyException, RuntimeException );
    virtual sal_Bool SAL_CALL hasMethod(const OUString& Name) throw( RuntimeException );
    virtual sal_Bool SAL_CALL hasProperty(const OUString& Name) throw( RuntimeException );

private:
    Reference< XIdlReflection >  m_xCoreReflection;
    Reference< XAllListener >    m_xAllListener;
    Reference< XIdlClass >       m_xListenerType;
    Any                          m_Helper;
};


// Function to replace AllListenerAdapterService::createAllListerAdapter
Reference< XInterface > createAllListenerAdapter
(
    const Reference< XInvocationAdapterFactory >& xInvocationAdapterFactory,
    const Reference< XIdlClass >& xListenerType,
    const Reference< XAllListener >& xListener,
    const Any& Helper
)
{
    Reference< XInterface > xAdapter;
    if( xInvocationAdapterFactory.is() && xListenerType.is() && xListener.is() )
    {
       Reference< XInvocation > xInvocationToAllListenerMapper =
            (XInvocation*)new InvocationToAllListenerMapper( xListenerType, xListener, Helper );
        Type aListenerType( xListenerType->getTypeClass(), xListenerType->getName() );
        xAdapter = xInvocationAdapterFactory->createAdapter( xInvocationToAllListenerMapper, aListenerType );
    }
    return xAdapter;
}


//--------------------------------------------------------------------------------------------------
// InvocationToAllListenerMapper
InvocationToAllListenerMapper::InvocationToAllListenerMapper
    ( const Reference< XIdlClass >& ListenerType, const Reference< XAllListener >& AllListener, const Any& Helper )
        : m_xAllListener( AllListener )
        , m_Helper( Helper )
        , m_xListenerType( ListenerType )
{
}

//*************************************************************************
Reference< XIntrospectionAccess > SAL_CALL InvocationToAllListenerMapper::getIntrospection(void)
    throw( RuntimeException )
{
    return Reference< XIntrospectionAccess >();
}

//*************************************************************************
Any SAL_CALL InvocationToAllListenerMapper::invoke(const OUString& FunctionName, const Sequence< Any >& Params,
    Sequence< sal_Int16 >& OutParamIndex, Sequence< Any >& OutParam)
        throw( IllegalArgumentException, CannotConvertException,
        InvocationTargetException, RuntimeException )
{
    Any aRet;

    // Check if to firing or approveFiring has to be called
    Reference< XIdlMethod > xMethod = m_xListenerType->getMethod( FunctionName );
    sal_Bool bApproveFiring = sal_False;
    if( !xMethod.is() )
        return aRet;
    Reference< XIdlClass > xReturnType = xMethod->getReturnType();
    Sequence< Reference< XIdlClass > > aExceptionSeq = xMethod->getExceptionTypes();
    if( ( xReturnType.is() && xReturnType->getTypeClass() != TypeClass_VOID ) ||
        aExceptionSeq.getLength() > 0 )
    {
        bApproveFiring = sal_True;
    }
    else
    {
        Sequence< ParamInfo > aParamSeq = xMethod->getParameterInfos();
        sal_uInt32 nParamCount = aParamSeq.getLength();
        if( nParamCount > 1 )
        {
            const ParamInfo* pInfos = aParamSeq.getConstArray();
            for( sal_uInt32 i = 0 ; i < nParamCount ; i++ )
            {
                if( pInfos[ i ].aMode != ParamMode_IN )
                {
                    bApproveFiring = sal_True;
                    break;
                }
            }
        }
    }

    AllEventObject aAllEvent;
    aAllEvent.Source = (OWeakObject*) this;
    aAllEvent.Helper = m_Helper;
    aAllEvent.ListenerType = Type(m_xListenerType->getTypeClass(), m_xListenerType->getName() );
    aAllEvent.MethodName = FunctionName;
    aAllEvent.Arguments = Params;
    if( bApproveFiring )
        aRet = m_xAllListener->approveFiring( aAllEvent );
    else
        m_xAllListener->firing( aAllEvent );
    return aRet;
}

//*************************************************************************
void SAL_CALL InvocationToAllListenerMapper::setValue(const OUString& PropertyName, const Any& Value)
    throw( UnknownPropertyException, CannotConvertException,
           InvocationTargetException, RuntimeException )
{
}

//*************************************************************************
Any SAL_CALL InvocationToAllListenerMapper::getValue(const OUString& PropertyName)
    throw( UnknownPropertyException, RuntimeException )
{
    return Any();
}

//*************************************************************************
sal_Bool SAL_CALL InvocationToAllListenerMapper::hasMethod(const OUString& Name)
    throw( RuntimeException )
{
    Reference< XIdlMethod > xMethod = m_xListenerType->getMethod( Name );
    return xMethod.is();
}

//*************************************************************************
sal_Bool SAL_CALL InvocationToAllListenerMapper::hasProperty(const OUString& Name)
    throw( RuntimeException )
{
    Reference< XIdlField > xField = m_xListenerType->getField( Name );
    return xField.is();
}

//========================================================================
// Uno-Service erzeugen
// 1. Parameter == Prefix-Name der Makros
// 2. Parameter == voll qualifizierter Name des Listeners
void SbRtl_CreateUnoListener( StarBASIC* pBasic, SbxArray& rPar, BOOL bWrite )
//RTLFUNC(CreateUnoListener)
{
    // Wir brauchen 2 Parameter
    if ( rPar.Count() != 3 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    // Klassen-Name der struct holen
    String aPrefixName = rPar.Get(1)->GetString();
    String aListenerClassName = rPar.Get(2)->GetString();

    // CoreReflection holen
    Reference< XIdlReflection > xCoreReflection = getCoreReflection_Impl();
    if( !xCoreReflection.is() )
        return;

    // AllListenerAdapterService holen
    Reference< XMultiServiceFactory > xFactory( comphelper::getProcessServiceFactory() );
    if( !xFactory.is() )
        return;

    // Klasse suchen
    Reference< XIdlClass > xClass = xCoreReflection->forName( aListenerClassName );
    if( !xClass.is() )
        return;

    // AB, 30.11.1999 InvocationAdapterFactory holen
    Reference< XInvocationAdapterFactory > xInvocationAdapterFactory = Reference< XInvocationAdapterFactory >(
        xFactory->createInstance( rtl::OUString::createFromAscii("com.sun.star.script.InvocationAdapterFactory") ), UNO_QUERY );

    BasicAllListener_Impl * p;
    Reference< XAllListener > xAllLst = p = new BasicAllListener_Impl( aPrefixName );
    Any aTmp;
    Reference< XInterface > xLst = createAllListenerAdapter( xInvocationAdapterFactory, xClass, xAllLst, aTmp );
    if( !xLst.is() )
        return;

    OUString aClassName = xClass->getName();
    Type aClassType( xClass->getTypeClass(), aClassName.getStr() );
    aTmp = xLst->queryInterface( aClassType );
    if( !aTmp.hasValue() )
        return;

    SbUnoObject* pUnoObj = new SbUnoObject( aListenerClassName, aTmp );
    p->xSbxObj = pUnoObj;
    p->xSbxObj->SetParent( pBasic );

    // #100326 Register listener object to set Parent NULL in Dtor
    SbxArrayRef xBasicUnoListeners = pBasic->getUnoListeners();
    xBasicUnoListeners->Insert( pUnoObj, xBasicUnoListeners->Count() );

    // Objekt zurueckliefern
    SbxVariableRef refVar = rPar.Get(0);
    refVar->PutObject( p->xSbxObj );
}

//========================================================================
// Represents the DefaultContext property of the ProcessServiceManager
// in the Basic runtime system.
void RTL_Impl_GetDefaultContext( StarBASIC* pBasic, SbxArray& rPar, BOOL bWrite )
{
    SbxVariableRef refVar = rPar.Get(0);

    Reference< XMultiServiceFactory > xFactory = comphelper::getProcessServiceFactory();
    Reference< XPropertySet> xPSMPropertySet( xFactory, UNO_QUERY );
    if( xPSMPropertySet.is() )
    {
        Any aContextAny = xPSMPropertySet->getPropertyValue(
            String( RTL_CONSTASCII_USTRINGPARAM("DefaultContext") ) );

        SbUnoObjectRef xUnoObj = new SbUnoObject
            ( String( RTL_CONSTASCII_USTRINGPARAM("DefaultContext") ),
              aContextAny );
        refVar->PutObject( (SbUnoObject*)xUnoObj );
    }
    else
    {
        refVar->PutObject( NULL );
    }
}

//========================================================================
// Creates a Basic wrapper object for a strongly typed Uno value
// 1. parameter: Uno type as full qualified type name, e.g. "byte[]"
// void RTL_Impl_GetDefaultContext( StarBASIC* pBasic, SbxArray& rPar, BOOL bWrite )
void RTL_Impl_CreateUnoValue( StarBASIC* pBasic, SbxArray& rPar, BOOL bWrite )
{
    // 2 parameters needed
    if ( rPar.Count() != 3 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    // Klassen-Name der struct holen
    String aTypeName = rPar.Get(1)->GetString();
    SbxVariable* pVal = rPar.Get(2);

    // Check the type
    Reference< XHierarchicalNameAccess > xTypeAccess = getTypeProvider_Impl();
    Any aRet;
    try
    {
        aRet = xTypeAccess->getByHierarchicalName( aTypeName );
    }
    catch( NoSuchElementException& e1 )
    {
        String aNoSuchElementExceptionName
            ( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.container.NoSuchElementException" ) );
        StarBASIC::Error( ERRCODE_BASIC_EXCEPTION,
            implGetExceptionMsg( e1, aNoSuchElementExceptionName ) );
        return;
    }
    Reference< XTypeDescription > xTypeDesc;
    aRet >>= xTypeDesc;
    TypeClass eTypeClass = xTypeDesc->getTypeClass();
    Type aDestType( eTypeClass, aTypeName );


    // Preconvert value
    Any aVal = sbxToUnoValueImpl( pVal );
    Any aConvertedVal = convertAny( aVal, aDestType );

    /*
    // Convert
    Reference< XTypeConverter > xConverter = getTypeConverter_Impl();
    try
    {
        aConvertedVal = xConverter->convertTo( aVal, aDestType );
    }
    catch( IllegalArgumentException& e1 )
    {
        StarBASIC::Error( ERRCODE_BASIC_EXCEPTION,
            implGetExceptionMsg( e1, aIllegalArgumentExceptionName ) );
        return;
    }
    catch( CannotConvertException& e2 )
    {
        String aCannotConvertExceptionName
            ( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.lang.IllegalArgumentException" ) );
        StarBASIC::Error( ERRCODE_BASIC_EXCEPTION,
            implGetExceptionMsg( e2, aCannotConvertExceptionName ) );
        return;
    }
    */

    SbxVariableRef refVar = rPar.Get(0);
    SbxObjectRef xUnoAnyObject = new SbUnoAnyObject( aConvertedVal );
    refVar->PutObject( xUnoAnyObject );
}

