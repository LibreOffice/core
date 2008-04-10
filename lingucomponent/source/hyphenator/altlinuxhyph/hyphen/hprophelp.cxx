/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: hprophelp.cxx,v $
 * $Revision: 1.8 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_lingucomponent.hxx"


#include <linguistic/misc.hxx>

#include "hprophelp.hxx"
#include <linguistic/lngprops.hxx>
#include <tools/debug.hxx>

#include <com/sun/star/linguistic2/LinguServiceEvent.hpp>
#include <com/sun/star/linguistic2/LinguServiceEventFlags.hpp>
#include <com/sun/star/linguistic2/XLinguServiceEventListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <osl/mutex.hxx>


//using namespace utl;
using namespace osl;
using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;


#define A2OU(x) ::rtl::OUString::createFromAscii( x )

///////////////////////////////////////////////////////////////////////////


PropertyChgHelper::PropertyChgHelper(
        const Reference< XInterface > & rxSource,
        Reference< XPropertySet > &rxPropSet,
        const char *pPropNames[], USHORT nPropCount ) :
    aPropNames( nPropCount),
    xMyEvtObj           (rxSource),
    aLngSvcEvtListeners (GetLinguMutex()),
    xPropSet(rxPropSet)
{
    OUString *pName = aPropNames.getArray();
    for (INT32 i = 0;  i < nPropCount;  ++i)
    {
        pName[i] = A2OU( pPropNames[i] );
    }
}


/*PropertyChgHelper::PropertyChgHelper( const PropertyChgHelper &rHelper ) :
    aLngSvcEvtListeners (GetLinguMutex())
{
    xPropSet    = rHelper.xPropSet;
    aPropNames  = rHelper.aPropNames;
    AddAsPropListener();

    xMyEvtObj   = rHelper.xMyEvtObj;
} */


PropertyChgHelper::~PropertyChgHelper()
{
}


void PropertyChgHelper::AddAsPropListener()
{
    if (xPropSet.is())
    {
        INT32 nLen = aPropNames.getLength();
        const OUString *pPropName = aPropNames.getConstArray();
        for (INT32 i = 0;  i < nLen;  ++i)
        {
            if (pPropName[i].getLength())
                xPropSet->addPropertyChangeListener( pPropName[i], this );
        }
    }
}

void PropertyChgHelper::RemoveAsPropListener()
{
    if (xPropSet.is())
    {
        INT32 nLen = aPropNames.getLength();
        const OUString *pPropName = aPropNames.getConstArray();
        for (INT32 i = 0;  i < nLen;  ++i)
        {
            if (pPropName[i].getLength())
                xPropSet->removePropertyChangeListener( pPropName[i], this );
        }
    }
}


void PropertyChgHelper::LaunchEvent( const LinguServiceEvent &rEvt )
{
    cppu::OInterfaceIteratorHelper aIt( aLngSvcEvtListeners );
    while (aIt.hasMoreElements())
    {
        Reference< XLinguServiceEventListener > xRef( aIt.next(), UNO_QUERY );
        if (xRef.is())
            xRef->processLinguServiceEvent( rEvt );
    }
}


void SAL_CALL PropertyChgHelper::disposing( const EventObject& rSource )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    if (rSource.Source == xPropSet)
    {
        RemoveAsPropListener();
        xPropSet = NULL;
        aPropNames.realloc( 0 );
    }
}


sal_Bool SAL_CALL
    PropertyChgHelper::addLinguServiceEventListener(
            const Reference< XLinguServiceEventListener >& rxListener )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    BOOL bRes = FALSE;
    if (rxListener.is())
    {
        INT32   nCount = aLngSvcEvtListeners.getLength();
        bRes = aLngSvcEvtListeners.addInterface( rxListener ) != nCount;
    }
    return bRes;
}


sal_Bool SAL_CALL
    PropertyChgHelper::removeLinguServiceEventListener(
            const Reference< XLinguServiceEventListener >& rxListener )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    BOOL bRes = FALSE;
    if (rxListener.is())
    {
        INT32   nCount = aLngSvcEvtListeners.getLength();
        bRes = aLngSvcEvtListeners.removeInterface( rxListener ) != nCount;
    }
    return bRes;
}


///////////////////////////////////////////////////////////////////////////

static const char *aHP[] =
{
    UPN_HYPH_MIN_LEADING,
    UPN_HYPH_MIN_TRAILING,
    UPN_HYPH_MIN_WORD_LENGTH
};


PropertyHelper_Hyphen::PropertyHelper_Hyphen(
        const Reference< XInterface > & rxSource,
        Reference< XPropertySet > &rxPropSet ) :
    PropertyChgHelper   ( rxSource, rxPropSet, aHP, sizeof(aHP) / sizeof(aHP[0]) )
{
    SetDefault();
    INT32 nLen = GetPropNames().getLength();
    if (rxPropSet.is() && nLen)
    {
        const OUString *pPropName = GetPropNames().getConstArray();
        for (INT32 i = 0;  i < nLen;  ++i)
        {
                INT16  *pnVal    = NULL;
            INT16  *pnResVal = NULL;

            if (A2OU( UPN_HYPH_MIN_LEADING ) == pPropName[i])
            {
                pnVal    = &nHyphMinLeading;
                pnResVal = &nResHyphMinLeading;
            }
            else if (A2OU( UPN_HYPH_MIN_TRAILING ) == pPropName[i])
            {
                pnVal    = &nHyphMinTrailing;
                pnResVal = &nResHyphMinTrailing;
            }
            else if (A2OU( UPN_HYPH_MIN_WORD_LENGTH ) == pPropName[i])
            {
                pnVal    = &nHyphMinWordLength;
                pnResVal = &nResHyphMinWordLength;
            }

            if (pnVal && pnResVal)
            {
                GetPropSet()->getPropertyValue( pPropName[i] ) >>= *pnVal;
                *pnResVal = *pnVal;
            }
        }
    }

}


PropertyHelper_Hyphen::~PropertyHelper_Hyphen()
{
}


void PropertyHelper_Hyphen::SetDefault()
{
    nResHyphMinLeading      = nHyphMinLeading           = 2;
    nResHyphMinTrailing     = nHyphMinTrailing      = 2;
    nResHyphMinWordLength           = nHyphMinWordLength            = 0;
}


void SAL_CALL
        PropertyHelper_Hyphen::propertyChange( const PropertyChangeEvent& rEvt )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (GetPropSet().is()  &&  rEvt.Source == GetPropSet())
    {
        INT16 nLngSvcFlags = LinguServiceEventFlags::HYPHENATE_AGAIN;

        INT16   *pnVal = NULL;
        switch (rEvt.PropertyHandle)
        {
            case UPH_HYPH_MIN_LEADING     : pnVal = &nHyphMinLeading;    break;
            case UPH_HYPH_MIN_TRAILING    : pnVal = &nHyphMinTrailing;   break;
            case UPH_HYPH_MIN_WORD_LENGTH     : pnVal = &nHyphMinWordLength; break;
            default:
                   DBG_ERROR( "unknown property" );
        }
        if (pnVal)
            rEvt.NewValue >>= *pnVal;

        if (pnVal)
        {
            if (nLngSvcFlags)
            {
                LinguServiceEvent aEvt( GetEvtObj(), nLngSvcFlags );
                LaunchEvent( aEvt );
            }
        }
    }
}



void PropertyHelper_Hyphen::SetTmpPropVals( const PropertyValues &rPropVals )
{

        nResHyphMinLeading      = nHyphMinLeading;
    nResHyphMinTrailing     = nHyphMinTrailing;
    nResHyphMinWordLength           = nHyphMinWordLength;

    INT32 nLen = rPropVals.getLength();

    if (nLen)
    {
        const PropertyValue *pVal = rPropVals.getConstArray();
        for (INT32 i = 0;  i < nLen;  ++i)
        {
            INT16   *pnResVal = NULL;
            switch (pVal[i].Handle)
            {
                case UPH_HYPH_MIN_LEADING     : pnResVal = &nResHyphMinLeading;    break;
                case UPH_HYPH_MIN_TRAILING    : pnResVal = &nResHyphMinTrailing;   break;
                case UPH_HYPH_MIN_WORD_LENGTH     : pnResVal = &nResHyphMinWordLength; break;
                default:
                  DBG_ERROR( "unknown property" );
            }
            if (pnResVal)
                pVal[i].Value >>= *pnResVal;
        }
    }
}

///////////////////////////////////////////////////////////////////////////

