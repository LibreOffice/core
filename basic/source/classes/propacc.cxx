/*************************************************************************
 *
 *  $RCSfile: propacc.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:09 $
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

#include "propacc.hxx"

#include <tools/urlobj.hxx>
#include <tools/errcode.hxx>
#include <svtools/svarray.hxx>
#include <svtools/sbx.hxx>
#include <sbstar.hxx>
#include <sbunoobj.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace cppu;
using namespace rtl;


//========================================================================

// Deklaration Konvertierung von Sbx nach Uno mit bekannter Zielklasse
Any sbxToUnoValue( SbxVariable* pVar, const Reference< XIdlClass >& xIdlTargetClass );
Reference<XIdlClass> TypeToIdlClass( const Type& rType );

//========================================================================

#ifdef WNT
#define CDECL _cdecl
#endif
#ifdef OS2
#define CDECL _Optlink
#endif
#if defined(UNX)  || defined(MAC)
#define CDECL
#endif

int CDECL SbCompare_PropertyValues_Impl( const void *arg1, const void *arg2 )
{
   return ((PropertyValue*)arg1)->Name.compareTo( ((PropertyValue*)arg2)->Name );
}

int CDECL SbCompare_UString_PropertyValue_Impl( const void *arg1, const void *arg2 )
{
    const OUString *pArg1 = (OUString*) arg1;
    const PropertyValue **pArg2 = (const PropertyValue**) arg2;
    return pArg1->compareTo( (*pArg2)->Name );
}

int CDECL SbCompare_Properties_Impl( const void *arg1, const void *arg2 )
{
   return ((Property*)arg1)->Name.compareTo( ((Property*)arg2)->Name );
}

int CDECL SbCompare_UString_Property_Impl( const void *arg1, const void *arg2 )
{
    const OUString *pArg1 = (OUString*) arg1;
    const Property *pArg2 = (Property*) arg2;
    return pArg1->compareTo( pArg2->Name );
}

//----------------------------------------------------------------------------

SbPropertyValues::SbPropertyValues()
{
}

//----------------------------------------------------------------------------

SbPropertyValues::~SbPropertyValues()
{
    _xInfo = Reference< XPropertySetInfo >();

    for ( USHORT n = 0; n < _aPropVals.Count(); ++n )
        delete _aPropVals.GetObject( n );
}

//----------------------------------------------------------------------------

Reference< XPropertySetInfo > SbPropertyValues::getPropertySetInfo(void) throw( RuntimeException )
{
    // create on demand?
    if ( !_xInfo.is() )
    {
        SbPropertySetInfo *pInfo = new SbPropertySetInfo( _aPropVals );
        ((SbPropertyValues*)this)->_xInfo = (XPropertySetInfo*)pInfo;
    }
    return _xInfo;
}

//-------------------------------------------------------------------------

INT32 SbPropertyValues::GetIndex_Impl( const OUString &rPropName ) const
{
    PropertyValue **ppPV;
    ppPV = (PropertyValue **)
            bsearch( &rPropName, _aPropVals.GetData(), _aPropVals.Count(),
                      sizeof( PropertyValue* ),
                      SbCompare_UString_PropertyValue_Impl );
    return ppPV ? ( (ppPV-_aPropVals.GetData()) / sizeof(ppPV) ) : USHRT_MAX;
}

//----------------------------------------------------------------------------

void SbPropertyValues::setPropertyValue(
                    const OUString& aPropertyName,
                    const Any& aValue)
{
    USHORT nIndex = GetIndex_Impl( aPropertyName );
    PropertyValue *pPropVal = _aPropVals.GetObject(nIndex);
    pPropVal->Value = aValue;
}

//----------------------------------------------------------------------------

Any SbPropertyValues::getPropertyValue(
                    const OUString& aPropertyName) throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    USHORT nIndex = GetIndex_Impl( aPropertyName );
    if ( nIndex != USHRT_MAX )
        return _aPropVals.GetObject(nIndex)->Value;
    return Any();
}

//----------------------------------------------------------------------------

void SbPropertyValues::addPropertyChangeListener(
                    const OUString& aPropertyName,
                    const Reference< XPropertyChangeListener >& )
{
}

//----------------------------------------------------------------------------

void SbPropertyValues::removePropertyChangeListener(
                    const OUString& aPropertyName,
                    const Reference< XPropertyChangeListener >& )
{
}

//----------------------------------------------------------------------------

void SbPropertyValues::addVetoableChangeListener(
                    const OUString& aPropertyName,
                    const Reference< XVetoableChangeListener >& )
{
}

//----------------------------------------------------------------------------

void SbPropertyValues::removeVetoableChangeListener(
                    const OUString& aPropertyName,
                    const Reference< XVetoableChangeListener >& )
{
}

//----------------------------------------------------------------------------

Sequence< PropertyValue > SbPropertyValues::getPropertyValues(void)
{
    Sequence<PropertyValue> aRet( _aPropVals.Count());
    for ( USHORT n = 0; n < _aPropVals.Count(); ++n )
        aRet.getArray()[n] = *_aPropVals.GetObject(n);
    return aRet;
}

//----------------------------------------------------------------------------

void SbPropertyValues::setPropertyValues(const Sequence< PropertyValue >& rPropertyValues )
{
    if ( _aPropVals.Count() )
        throw PropertyExistException();

    const PropertyValue *pPropVals = rPropertyValues.getConstArray();
    for ( sal_Int16 n = 0; n < rPropertyValues.getLength(); ++n )
    {
        PropertyValue *pPropVal = new PropertyValue(pPropVals[n]);
        _aPropVals.Insert( pPropVal, n );
    }
}

//============================================================================
//PropertySetInfoImpl

PropertySetInfoImpl::PropertySetInfoImpl()
{
}

INT32 PropertySetInfoImpl::GetIndex_Impl( const OUString &rPropName ) const
{
    Property *pP;
    pP = (Property*)
            bsearch( &rPropName, _aProps.getConstArray(), _aProps.getLength(),
                      sizeof( Property ),
                      SbCompare_UString_Property_Impl );
    return pP ? ( (pP-_aProps.getConstArray()) / sizeof(pP) ) : -1;
}

Sequence< Property > PropertySetInfoImpl::getProperties(void)
{
    return _aProps;
}

Property PropertySetInfoImpl::getPropertyByName(const OUString& Name) throw( RuntimeException )
{
    USHORT nIndex = GetIndex_Impl( Name );
    if( USHRT_MAX != nIndex )
        return _aProps.getConstArray()[ nIndex ];
    return Property();
}

sal_Bool PropertySetInfoImpl::hasPropertyByName(const OUString& Name) throw( RuntimeException )
{
    USHORT nIndex = GetIndex_Impl( Name );
    return USHRT_MAX != nIndex;
}


//============================================================================

SbPropertySetInfo::SbPropertySetInfo()
{
}

//----------------------------------------------------------------------------

SbPropertySetInfo::SbPropertySetInfo( const SbPropertyValueArr_Impl &rPropVals )
{
    aImpl._aProps.realloc( rPropVals.Count() );
    for ( USHORT n = 0; n < rPropVals.Count(); ++n )
    {
        Property &rProp = aImpl._aProps.getArray()[n];
        const PropertyValue &rPropVal = *rPropVals.GetObject(n);
        rProp.Name = rPropVal.Name;
        rProp.Handle = rPropVal.Handle;
        rProp.Type = getCppuVoidType();
        rProp.Attributes = 0;
    }
}

//----------------------------------------------------------------------------

SbPropertySetInfo::~SbPropertySetInfo()
{
}

//-------------------------------------------------------------------------

Sequence< Property > SbPropertySetInfo::getProperties(void) throw( RuntimeException )
{
    return aImpl.getProperties();
}

Property SbPropertySetInfo::getPropertyByName(const OUString& Name)
    throw( RuntimeException )
{
    return aImpl.getPropertyByName( Name );
}

BOOL SbPropertySetInfo::hasPropertyByName(const OUString& Name)
    throw( RuntimeException )
{
    return aImpl.hasPropertyByName( Name );
}


//----------------------------------------------------------------------------

SbPropertyContainer::SbPropertyContainer()
{
}

//----------------------------------------------------------------------------

SbPropertyContainer::~SbPropertyContainer()
{
}

//----------------------------------------------------------------------------
void SbPropertyContainer::addProperty(const OUString& Name,
                                      INT16 Attributes,
                                      const Any& DefaultValue)
    throw(  PropertyExistException, IllegalTypeException,
            IllegalArgumentException, RuntimeException )
{
}

//----------------------------------------------------------------------------
void SbPropertyContainer::removeProperty(const OUString& Name)
    throw( UnknownPropertyException, RuntimeException )
{
}

//----------------------------------------------------------------------------
// XPropertySetInfo
Sequence< Property > SbPropertyContainer::getProperties(void)
{
    return aImpl.getProperties();
}

Property SbPropertyContainer::getPropertyByName(const OUString& Name)
    throw( RuntimeException )
{
    return aImpl.getPropertyByName( Name );
}

BOOL SbPropertyContainer::hasPropertyByName(const OUString& Name)
    throw( RuntimeException )
{
    return aImpl.hasPropertyByName( Name );
}

//----------------------------------------------------------------------------

Sequence< PropertyValue > SbPropertyContainer::getPropertyValues(void)
{
    return Sequence<PropertyValue>();
}

//----------------------------------------------------------------------------

void SbPropertyContainer::setPropertyValues(const Sequence< PropertyValue >& PropertyValues_)
{
}

//----------------------------------------------------------------------------

void RTL_Impl_CreatePropertySet( StarBASIC* pBasic, SbxArray& rPar, BOOL bWrite )
{
    // Wir brauchen mindestens 1 Parameter
    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    // Klassen-Name der struct holen
    String aServiceName( RTL_CONSTASCII_USTRINGPARAM("stardiv.uno.beans.PropertySet") );

#if 0
    // Service suchen und instanzieren
    Reference< XMultiServiceFactory > xServiceManager = getProcessServiceFactory();
    Reference< XInterface > xInterface;
    if( xProv.is() )
        xInterface = xProv->newInstance();
#else
    Reference< XInterface > xInterface = (OWeakObject*) new SbPropertyValues();
#endif

    SbxVariableRef refVar = rPar.Get(0);
    if( xInterface.is() )
    {
        // PropertyValues setzen
        Any aArgAsAny = sbxToUnoValue( rPar.Get(1),
                TypeToIdlClass( getCppuType( (Sequence<PropertyValue>*)0 ) ) );
        Sequence<PropertyValue> *pArg =
                (Sequence<PropertyValue>*) aArgAsAny.getValue();
        Reference< XPropertyAccess > xPropAcc = Reference< XPropertyAccess >::query( xInterface );
        xPropAcc->setPropertyValues( *pArg );

        // SbUnoObject daraus basteln und zurueckliefern
        Any aAny;
        aAny <<= xInterface;
        SbUnoObjectRef xUnoObj = new SbUnoObject( aServiceName, aAny );
        if( xUnoObj->getUnoAny().getValueType().getTypeClass() != TypeClass_VOID )
        {
            // Objekt zurueckliefern
            refVar->PutObject( (SbUnoObject*)xUnoObj );
            return;
        }
    }

    // Objekt konnte nicht erzeugt werden
    refVar->PutObject( NULL );
}


