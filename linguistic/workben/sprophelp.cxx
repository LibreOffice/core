/*************************************************************************
 *
 *  $RCSfile: sprophelp.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-11-17 12:37:44 $
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

#include "misc.hxx"

#include "sprophelp.hxx"
#include "lngprops.hxx"

#ifndef _TOOLS_DEBUG_HXX //autogen wg. DBG_ASSERT
#include <tools/debug.hxx>
#endif

#include <com/sun/star/linguistic2/LinguServiceEvent.hpp>
#include <com/sun/star/linguistic2/LinguServiceEventFlags.hpp>
#include <com/sun/star/linguistic2/XLinguServiceEventListener.hpp>

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

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
    xMyEvtObj           (rxSource),
    xPropSet            (rxPropSet),
    aPropNames          (nPropCount),
    aLngSvcEvtListeners (GetLinguMutex())
{
    OUString *pName = aPropNames.getArray();
    for (INT32 i = 0;  i < nPropCount;  ++i)
    {
        pName[i] = A2OU( pPropNames[i] );
    }
}


PropertyChgHelper::PropertyChgHelper( const PropertyChgHelper &rHelper ) :
    aLngSvcEvtListeners (GetLinguMutex())
{
    xPropSet    = rHelper.xPropSet;
    aPropNames  = rHelper.aPropNames;
    AddAsPropListener();

    xMyEvtObj   = rHelper.xMyEvtObj;
}


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

static const char *aSP[] =
{
    UPN_IS_GERMAN_PRE_REFORM,
    UPN_IS_IGNORE_CONTROL_CHARACTERS,
    UPN_IS_USE_DICTIONARY_LIST,
    UPN_IS_SPELL_UPPER_CASE,
    UPN_IS_SPELL_WITH_DIGITS,
    UPN_IS_SPELL_CAPITALIZATION
};


PropertyHelper_Spell::PropertyHelper_Spell(
        const Reference< XInterface > & rxSource,
        Reference< XPropertySet > &rxPropSet ) :
    PropertyChgHelper   ( rxSource, rxPropSet, aSP, sizeof(aSP) / sizeof(aSP[0]) )
{
    SetDefault();
    INT32 nLen = GetPropNames().getLength();
    if (rxPropSet.is() && nLen)
    {
        const OUString *pPropName = GetPropNames().getConstArray();
        for (INT32 i = 0;  i < nLen;  ++i)
        {
            BOOL *pbVal     = NULL,
                 *pbResVal  = NULL;

            if (A2OU( UPN_IS_GERMAN_PRE_REFORM ) == pPropName[i])
            {
                pbVal    = &bIsGermanPreReform;
                pbResVal = &bResIsGermanPreReform;
            }
            else if (A2OU( UPN_IS_IGNORE_CONTROL_CHARACTERS ) == pPropName[i])
            {
                pbVal    = &bIsIgnoreControlCharacters;
                pbResVal = &bResIsIgnoreControlCharacters;
            }
            else if (A2OU( UPN_IS_USE_DICTIONARY_LIST ) == pPropName[i])
            {
                pbVal    = &bIsUseDictionaryList;
                pbResVal = &bResIsUseDictionaryList;
            }
            else if (A2OU( UPN_IS_SPELL_UPPER_CASE ) == pPropName[i])
            {
                pbVal    = &bIsSpellUpperCase;
                pbResVal = &bResIsSpellUpperCase;
            }
            else if (A2OU( UPN_IS_SPELL_WITH_DIGITS ) == pPropName[i])
            {
                pbVal    = &bIsSpellWithDigits;
                pbResVal = &bResIsSpellWithDigits;
            }
            else if (A2OU( UPN_IS_SPELL_CAPITALIZATION ) == pPropName[i])
            {
                pbVal    = &bIsSpellCapitalization;
                pbResVal = &bResIsSpellCapitalization;
            }

            if (pbVal && pbResVal)
            {
                rxPropSet->getPropertyValue( pPropName[i] ) >>= *pbVal;
                *pbResVal = *pbVal;
            }
        }
    }
}


PropertyHelper_Spell::~PropertyHelper_Spell()
{
}


void PropertyHelper_Spell::SetDefault()
{
    bResIsGermanPreReform           = bIsGermanPreReform            = FALSE;
    bResIsIgnoreControlCharacters   = bIsIgnoreControlCharacters    = TRUE;
    bResIsUseDictionaryList         = bIsUseDictionaryList          = TRUE;
    bResIsSpellUpperCase            = bIsSpellUpperCase             = FALSE;
    bResIsSpellWithDigits           = bIsSpellWithDigits            = FALSE;
    bResIsSpellCapitalization       = bIsSpellCapitalization        = TRUE;
}


void SAL_CALL
    PropertyHelper_Spell::propertyChange( const PropertyChangeEvent& rEvt )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (GetPropSet().is()  &&  rEvt.Source == GetPropSet())
    {
        INT16 nLngSvcFlags = 0;
        BOOL bSCWA = FALSE, // SPELL_CORRECT_WORDS_AGAIN ?
             bSWWA = FALSE; // SPELL_WRONG_WORDS_AGAIN ?

        BOOL *pbVal = NULL;
        switch (rEvt.PropertyHandle)
        {
            case UPH_IS_IGNORE_CONTROL_CHARACTERS :
            {
                pbVal = &bIsIgnoreControlCharacters;
                break;
            }
            case UPH_IS_GERMAN_PRE_REFORM         :
            {
                pbVal = &bIsGermanPreReform;
                bSCWA = bSWWA = TRUE;
                break;
            }
            case UPH_IS_USE_DICTIONARY_LIST       :
            {
                pbVal = &bIsUseDictionaryList;
                bSCWA = bSWWA = TRUE;
                break;
            }
            case UPH_IS_SPELL_UPPER_CASE          :
            {
                pbVal = &bIsSpellUpperCase;
                bSCWA = FALSE == *pbVal;    // FALSE->TRUE change?
                bSWWA = !bSCWA;             // TRUE->FALSE change?
                break;
            }
            case UPH_IS_SPELL_WITH_DIGITS         :
            {
                pbVal = &bIsSpellWithDigits;
                bSCWA = FALSE == *pbVal;    // FALSE->TRUE change?
                bSWWA = !bSCWA;             // TRUE->FALSE change?
                break;
            }
            case UPH_IS_SPELL_CAPITALIZATION      :
            {
                pbVal = &bIsSpellCapitalization;
                bSCWA = FALSE == *pbVal;    // FALSE->TRUE change?
                bSWWA = !bSCWA;             // TRUE->FALSE change?
                break;
            }
            default:
                DBG_ERROR( "unknown property" );
        }
        if (pbVal)
            rEvt.NewValue >>= *pbVal;

        if (bSCWA)
            nLngSvcFlags |= LinguServiceEventFlags::SPELL_CORRECT_WORDS_AGAIN;
        if (bSWWA)
            nLngSvcFlags |= LinguServiceEventFlags::SPELL_WRONG_WORDS_AGAIN;
        if (nLngSvcFlags)
        {
            LinguServiceEvent aEvt( GetEvtObj(), nLngSvcFlags );
            LaunchEvent( aEvt );
        }
    }
}


void PropertyHelper_Spell::SetTmpPropVals( const PropertyValues &rPropVals )
{
    // set return value to default value unless there is an
    // explicitly supplied temporary value
    bResIsGermanPreReform           = bIsGermanPreReform;
    bResIsIgnoreControlCharacters   = bIsIgnoreControlCharacters;
    bResIsUseDictionaryList         = bIsUseDictionaryList;
    bResIsSpellUpperCase            = bIsSpellUpperCase;
    bResIsSpellWithDigits           = bIsSpellWithDigits;
    bResIsSpellCapitalization       = bIsSpellCapitalization;
    //
    INT32 nLen = rPropVals.getLength();
    if (nLen)
    {
        const PropertyValue *pVal = rPropVals.getConstArray();
        for (INT32 i = 0;  i < nLen;  ++i)
        {
            BOOL *pbResVal = NULL;
            switch (pVal[i].Handle)
            {
                case UPH_IS_GERMAN_PRE_REFORM         : pbResVal = &bResIsGermanPreReform; break;
                case UPH_IS_IGNORE_CONTROL_CHARACTERS : pbResVal = &bResIsIgnoreControlCharacters; break;
                case UPH_IS_USE_DICTIONARY_LIST       : pbResVal = &bResIsUseDictionaryList; break;
                case UPH_IS_SPELL_UPPER_CASE          : pbResVal = &bResIsSpellUpperCase; break;
                case UPH_IS_SPELL_WITH_DIGITS         : pbResVal = &bResIsSpellWithDigits; break;
                case UPH_IS_SPELL_CAPITALIZATION      : pbResVal = &bResIsSpellCapitalization; break;
                default:
                    DBG_ERROR( "unknown property" );
            }
            if (pbResVal)
                pVal[i].Value >>= *pbResVal;
        }
    }
}

///////////////////////////////////////////////////////////////////////////

