/*************************************************************************
 *
 *  $RCSfile: unofield.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: cl $ $Date: 2000-10-18 16:10:16 $
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

#ifndef _COM_SUN_STAR_LANG_NOSUPPORTEXCEPTION_HPP_
#include <com/sun/star/lang/NoSupportException.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _SFX_INIMGR_HXX
#include <sfx2/inimgr.hxx>
#endif

#include <rtl/uuid.h>
#include <rtl/memory.h>

#include "eeitem.hxx"
#define ITEMID_FIELD EE_FEATURE_FIELD
#include "flditem.hxx"
#include "unofield.hxx"
#include "unoprov.hxx"
#include "unotext.hxx"
#include "adritem.hxx"

using namespace ::rtl;
using namespace ::vos;
using namespace ::cppu;
using namespace ::com::sun::star;

#define QUERYINT( xint ) \
    if( rType == ::getCppuType((const uno::Reference< xint >*)0) ) \
        aAny <<= uno::Reference< xint >(this)


#define WID_DOUBLE  0
#define WID_BOOL1   1
#define WID_BOOL2   2
#define WID_INTEGER 3
#define WID_STRING1 4
#define WID_STRING2 5
#define WID_STRING3 6

class SvxUnoFieldData_Impl
{
public:
    Double      mdDouble;
    sal_Bool    mbBoolean1;
    sal_Bool    mbBoolean2;
    sal_Int32   mnInteger;
    OUString    msString1;
    OUString    msString2;
    OUString    msString3;

    OUString    msPresentation;
};

SfxItemPropertyMap* ImplGetFieldItemPropertyMap( USHORT mnId )
{
    static SfxItemPropertyMap aExDateTimeFieldPropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN("DateTimeValue"),    WID_DOUBLE,     &::getCppuType((const Double*)0),       0, 0 },
        { MAP_CHAR_LEN("IsFixed"),          WID_BOOL1,      &::getBooleanCppuType(),                0, 0 },
        { MAP_CHAR_LEN("IsDate"),           WID_BOOL2,      &::getBooleanCppuType(),                0, 0 },
        { MAP_CHAR_LEN("NumberFormat"),     WID_INTEGER,    &::getCppuType((const sal_Int32*)0),    0, 0 },
        {0,0}
    };

    static SfxItemPropertyMap aDateTimeFieldPropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN("IsDate"),           WID_BOOL2,      &::getBooleanCppuType(),                0, 0 },
        {0,0}
    };

    static SfxItemPropertyMap aUrlFieldPropertyMap_Impl[] =
    {

        { MAP_CHAR_LEN("Format"),           WID_INTEGER,    &::getCppuType((const sal_Int32*)0),    0, 0 },
        { MAP_CHAR_LEN("Presentation"),     WID_STRING1,    &::getCppuType((const OUString*)0),     0, 0 },
        { MAP_CHAR_LEN("TargetFrame"),      WID_STRING2,    &::getCppuType((const OUString*)0),     0, 0 },
        { MAP_CHAR_LEN("URL"),              WID_STRING3,    &::getCppuType((const OUString*)0),     0, 0 },
        {0,0}
    };

    static SfxItemPropertyMap aEmptyPropertyMap_Impl[] =
    {
        {0,0}
    };

    static SfxItemPropertyMap aExtFileFieldPropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN("IsFixed"),              WID_BOOL1,  &::getBooleanCppuType(),                0, 0 },
        { MAP_CHAR_LEN("FileFormat"),           WID_INTEGER,&::getCppuType((const sal_Int32*)0),    0, 0 },
        { MAP_CHAR_LEN("CurrentPresentation"),  WID_STRING1,&::getCppuType((const OUString*)0),     0, 0 },
        {0,0}
    };

    static SfxItemPropertyMap aAuthorFieldPropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN("IsFixed"),              WID_BOOL1,  &::getBooleanCppuType(),                0, 0 },
        { MAP_CHAR_LEN("CurrentPresentation"),  WID_STRING1,&::getCppuType((const OUString*)0),     0, 0 },
        { MAP_CHAR_LEN("Content"),              WID_STRING2,&::getCppuType((const OUString*)0),     0, 0 },
        { MAP_CHAR_LEN("AuthorFormat"),         WID_INTEGER,&::getCppuType((const sal_Int32*)0),    0, 0 },
        {0,0}
    };

    switch( mnId )
    {
    case ID_DATEFIELD:
        return aExDateTimeFieldPropertyMap_Impl;
    case ID_URLFIELD:
        return aUrlFieldPropertyMap_Impl;
    case ID_TIMEFIELD:
        return aDateTimeFieldPropertyMap_Impl;
    case ID_EXT_TIMEFIELD:
        return aExDateTimeFieldPropertyMap_Impl;
    case ID_EXT_FILEFIELD:
        return aExtFileFieldPropertyMap_Impl;
    case ID_AUTHORFIELD:
        return aAuthorFieldPropertyMap_Impl;
//  case ID_PAGEFIELD:
//  case ID_PAGESFIELD:
//  case ID_FILEFIELD:
//  case ID_TABLEFIELD:
    default:
        return aEmptyPropertyMap_Impl;
    }
}

static SfxItemPropertyMap aEmptyPropertyMap_Impl[] =
{
    {0,0}
};

static sal_Char* aFieldItemNameMap_Impl[] =
{
    "Date",
    "URL",
    "Page",
    "Pages",
    "Time",
    "File",
    "Table",
    "ExtTime",
    "ExtFile",
    "Author"
};

// ====================================================================
// class SvxUnoTextField
// ====================================================================
UNO3_GETIMPLEMENTATION_IMPL( SvxUnoTextField );

SvxUnoTextField::SvxUnoTextField( sal_Int32 nServiceId ) throw()
:   OComponentHelper( getMutex() ),
    mpPropSet(NULL),
    mpImpl( new SvxUnoFieldData_Impl ),
    mnServiceId(nServiceId)
{
    mpPropSet = new SfxItemPropertySet( ImplGetFieldItemPropertyMap(mnServiceId) );

    mpImpl->mdDouble = 0.0;
    mpImpl->mbBoolean1 = sal_False;
    mpImpl->mbBoolean2 = sal_False;
    mpImpl->mnInteger = 0;
}

SvxUnoTextField::SvxUnoTextField( uno::Reference< text::XTextRange > xAnchor, const OUString& rPresentation, const SvxFieldData* pData ) throw()
:   OComponentHelper( getMutex() ),
    mxAnchor( xAnchor ),
    mpPropSet(NULL),
    mpImpl( new SvxUnoFieldData_Impl ),
    mnServiceId(ID_NOTFOUND)
{
    DBG_ASSERT(pData, "pFieldData == NULL! [CL]" );

    mpImpl->msPresentation = rPresentation;

    if(pData)
    {
        mnServiceId = GetFieldId(pData);
        DBG_ASSERT(mnServiceId != ID_NOTFOUND, "unknown SvxFieldData! [CL]");
        if(mnServiceId != ID_NOTFOUND)
        {
            mpPropSet = new SfxItemPropertySet( ImplGetFieldItemPropertyMap(mnServiceId) );

            // extract field properties from data class
            switch( mnServiceId )
            {
            case ID_DATEFIELD:
                mpImpl->mbBoolean2 = sal_True;
                mpImpl->mdDouble = (double) ((SvxDateField*)pData)->GetFixDate();
                mpImpl->mnInteger = ((SvxDateField*)pData)->GetFormat();
                mpImpl->mbBoolean1 = ((SvxDateField*)pData)->GetType() == SVXDATETYPE_FIX;
                break;

            case ID_EXT_TIMEFIELD:
                mpImpl->mbBoolean2 = sal_False;
                mpImpl->mdDouble = (double) ((SvxExtTimeField*)pData)->GetFixTime();
                mpImpl->mbBoolean1 = ((SvxExtTimeField*)pData)->GetType() == SVXTIMETYPE_FIX;
                mpImpl->mnInteger = ((SvxExtTimeField*)pData)->GetFormat();
                break;

            case ID_TIMEFIELD:
                mpImpl->mbBoolean2 = sal_False;
                break;

            case ID_URLFIELD:
                mpImpl->msString1 = ((SvxURLField*)pData)->GetRepresentation();
                mpImpl->msString2 = ((SvxURLField*)pData)->GetTargetFrame();
                mpImpl->msString3 = ((SvxURLField*)pData)->GetURL();
                mpImpl->mnInteger = ((SvxURLField*)pData)->GetFormat();
                break;

            case ID_EXT_FILEFIELD:
                mpImpl->msString1 = ((SvxExtFileField*)pData)->GetFile();
                mpImpl->mbBoolean1 = ((SvxExtFileField*)pData)->GetType() == SVXFILETYPE_FIX;
                mpImpl->mnInteger = ((SvxExtFileField*)pData)->GetFormat();
                break;

            case ID_AUTHORFIELD:
                mpImpl->msString1 = ((SvxAuthorField*)pData)->GetFormatted();
                mpImpl->msString2 = ((SvxAuthorField*)pData)->GetFormatted();
                mpImpl->mnInteger = ((SvxAuthorField*)pData)->GetFormat();
                mpImpl->mbBoolean1 = ((SvxAuthorField*)pData)->GetType() == SVXAUTHORTYPE_FIX;
                break;
            }
        }
    }
}

SvxUnoTextField::~SvxUnoTextField() throw()
{
    delete mpPropSet;
    delete mpImpl;
}

SvxFieldData* SvxUnoTextField::CreateFieldData() const throw()
{
    SvxFieldData* pData = NULL;

    switch( mnServiceId )
    {
    case ID_TIMEFIELD:
    case ID_EXT_TIMEFIELD:
    case ID_DATEFIELD:
    {
        if( mpImpl->mbBoolean2 ) // IsDate?
        {
            Date aDate;
            if( mpImpl->mdDouble != 0.0 )
                aDate.SetDate( (ULONG) mpImpl->mdDouble );

            pData = new SvxDateField( aDate, mpImpl->mbBoolean1?SVXDATETYPE_FIX:SVXDATETYPE_VAR );
            if( mpImpl->mnInteger >= SVXDATEFORMAT_APPDEFAULT && mpImpl->mnInteger <= SVXDATEFORMAT_F )
                ((SvxDateField*)pData)->SetFormat( (SvxDateFormat)mpImpl->mnInteger );
        }
        else
        {
            Time aTime;

            if( mnServiceId != ID_TIMEFIELD )
            {
                if( mpImpl->mdDouble != 0.0 )
                    aTime.SetTime( (ULONG) mpImpl->mdDouble );

                pData = new SvxExtTimeField( aTime, mpImpl->mbBoolean1?SVXTIMETYPE_FIX:SVXTIMETYPE_VAR );

                if( mpImpl->mnInteger >= SVXTIMEFORMAT_APPDEFAULT && mpImpl->mnInteger <= SVXTIMEFORMAT_AM_HMSH )
                    ((SvxExtTimeField*)pData)->SetFormat( (SvxTimeFormat)mpImpl->mnInteger );
            }
            else
            {
                pData = new SvxTimeField();
            }
        }

    }
        break;

    case ID_URLFIELD:
        pData = new SvxURLField( mpImpl->msString3, mpImpl->msString1 );
        ((SvxURLField*)pData)->SetTargetFrame( mpImpl->msString2 );
        if( mpImpl->mnInteger >= SVXURLFORMAT_APPDEFAULT && mpImpl->mnInteger <= SVXURLFORMAT_REPR )
            ((SvxURLField*)pData)->SetFormat( (SvxURLFormat)mpImpl->mnInteger );
        break;

    case ID_PAGEFIELD:
        pData = new SvxPageField();
        break;

    case ID_PAGESFIELD:
        pData = new SvxPagesField();
        break;

    case ID_FILEFIELD:
        pData = new SvxFileField();
        break;

    case ID_TABLEFIELD:
        pData = new SvxTableField();
        break;

    case ID_EXT_FILEFIELD:
    {
        pData = new SvxExtFileField( mpImpl->msString1 );
        if( mpImpl->mnInteger >= SVXFILEFORMAT_NAME_EXT && mpImpl->mnInteger <= SVXFILEFORMAT_NAME )
            ((SvxExtFileField*)pData)->SetFormat( (SvxFileFormat)mpImpl->mnInteger );
        break;
    }

    case ID_AUTHORFIELD:
    {
        String aFirstName;
        String aLastName;
        String aEmpty;

        sal_Int32 nPos = mpImpl->msString1.lastIndexOf( sal_Char(' '), 0 );
        if( nPos > 0 )
        {
            aFirstName = mpImpl->msString1.copy( 0, nPos );
            aLastName = mpImpl->msString1.copy( nPos + 1 );
        }
        else
        {
            aLastName = mpImpl->msString1;
        }

        pData = new SvxAuthorField( SvxAddressItem( aEmpty, aEmpty, aFirstName, aLastName ) );
        break;
    }

    };

    return pData;
}

// uno::XInterface
uno::Any SAL_CALL SvxUnoTextField::queryAggregation( const uno::Type & rType )
    throw(uno::RuntimeException)
{
    uno::Any aAny;

    QUERYINT( beans::XPropertySet );
    else QUERYINT( text::XTextContent );
    else QUERYINT( text::XTextField );
    else QUERYINT( lang::XServiceInfo );
    else QUERYINT( lang::XUnoTunnel );
    else
        return OComponentHelper::queryAggregation( rType );

    return aAny;
}

// XTypeProvider

uno::Sequence< uno::Type > SAL_CALL SvxUnoTextField::getTypes()
    throw (uno::RuntimeException)
{
    if( maTypeSequence.getLength() == 0 )
    {
        maTypeSequence == OComponentHelper::getTypes();
        sal_Int32 nOldCount = maTypeSequence.getLength();

        maTypeSequence.realloc( nOldCount + 4 ); // !DANGER! keep this updated
        uno::Type* pTypes = &maTypeSequence.getArray()[nOldCount];

        *pTypes++ = ::getCppuType(( const uno::Reference< text::XTextField >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< beans::XPropertySet >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< lang::XServiceInfo >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< lang::XUnoTunnel >*)0);
    }
    return maTypeSequence;
}

uno::Sequence< sal_Int8 > SAL_CALL SvxUnoTextField::getImplementationId()
    throw (uno::RuntimeException)
{
    static uno::Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;
}

uno::Any SAL_CALL SvxUnoTextField::queryInterface( const uno::Type & rType )
    throw(uno::RuntimeException)
{
    return OComponentHelper::queryInterface(rType);
}

void SAL_CALL SvxUnoTextField::acquire() throw(uno::RuntimeException)
{
    OComponentHelper::acquire();
}

void SAL_CALL SvxUnoTextField::release() throw(uno::RuntimeException)
{
    OComponentHelper::release();
}

// Interface text::XTextField
OUString SAL_CALL SvxUnoTextField::getPresentation( sal_Bool bShowCommand )
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(bShowCommand)
    {
        return OUString::createFromAscii( aFieldItemNameMap_Impl[mnServiceId] );
    }
    else
    {
        return mpImpl->msPresentation;
    }
}

// Interface text::XTextContent
void SAL_CALL SvxUnoTextField::attach( const uno::Reference< text::XTextRange >& xTextRange )
    throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    SvxUnoTextRangeBase* pRange = SvxUnoTextRange::getImplementation( xTextRange );
    if(pRange == NULL)
        throw lang::IllegalArgumentException();

    SvxFieldData* pData = CreateFieldData();
    if( pData )
        pRange->attachField( pData );

    delete pData;
}

uno::Reference< text::XTextRange > SAL_CALL SvxUnoTextField::getAnchor()
    throw(uno::RuntimeException)
{
    return mxAnchor;
}

// lang::XComponent
void SAL_CALL SvxUnoTextField::dispose()
    throw(uno::RuntimeException)
{
    OComponentHelper::dispose();
}

void SAL_CALL SvxUnoTextField::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
    throw(uno::RuntimeException)
{
    OComponentHelper::addEventListener(xListener);
}

void SAL_CALL SvxUnoTextField::removeEventListener( const uno::Reference< lang::XEventListener >& aListener )
    throw(uno::RuntimeException)
{
    OComponentHelper::removeEventListener(aListener);
}


// Interface beans::XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL SvxUnoTextField::getPropertySetInfo(  )
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );
    return mpPropSet->getPropertySetInfo();
}

void SAL_CALL SvxUnoTextField::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
    throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( mpImpl == NULL )
        throw uno::RuntimeException();

    const SfxItemPropertyMap* pMap = SfxItemPropertyMap::GetByName(mpPropSet->getPropertyMap(), aPropertyName );
    if ( !pMap )
        throw beans::UnknownPropertyException();

    switch( pMap->nWID )
    {
    case WID_DOUBLE:
        if(aValue >>= mpImpl->mdDouble)
            return;
        break;
    case WID_BOOL1:
        if(aValue >>= mpImpl->mbBoolean1)
            return;
        break;
    case WID_BOOL2:
        if(aValue >>= mpImpl->mbBoolean2)
            return;
        break;
    case WID_INTEGER:
        if(aValue >>= mpImpl->mnInteger)
            return;
        break;
    case WID_STRING1:
        if(aValue >>= mpImpl->msString1)
            return;
        break;
    case WID_STRING2:
        if(aValue >>= mpImpl->msString2)
            return;
        break;
    case WID_STRING3:
        if(aValue >>= mpImpl->msString3)
            return;
        break;
    }

    throw lang::IllegalArgumentException();

/*
    case WID_FORMAT:
        {
        sal_Int32 nFormat;

        switch( mnId )
        {
        case ID_DATEFIELD:
        {
            SvxDateField* pDate = PTR_CAST( SvxDateField, aFieldItem.GetField() );
            if(pDate)
                pDate->SetFormat( (SvxDateFormat)nFormat );
            break;
        }
        case ID_URLFIELD:
        {
            SvxURLField* pURL = PTR_CAST( SvxURLField, aFieldItem.GetField() );
            if(pURL)
                pURL->SetFormat( (SvxURLFormat)nFormat );
            break;
        }
        case ID_EXT_TIMEFIELD:
        {
            SvxExtTimeField* pTime = PTR_CAST( SvxExtTimeField, aFieldItem.GetField() );
            if(pTime)
                pTime->SetFormat( (SvxTimeFormat)nFormat );
            break;
        }
        case ID_EXT_FILEFIELD:
        {
            SvxExtFileField* pFile = PTR_CAST( SvxExtFileField, aFieldItem.GetField() );
            if(pFile)
                pFile->SetFormat( (SvxFileFormat)nFormat );
            break;
        }
        case ID_AUTHORFIELD:
        {
            SvxAuthorField* pAuthor = PTR_CAST( SvxAuthorField, aFieldItem.GetField() );
            if(pAuthor)
                pAuthor->SetFormat( (SvxAuthorFormat)nFormat );
            break;
        }
        default:
            throw beans::UnknownPropertyException();
        }
        }
        break;
    case WID_FIX:
        {
        if( aValue.hasValue() || aValue.getValueType() != ::getCppuBooleanType() )
            throw lang::IllegalArgumentException();
        sal_Bool bFix( *(sal_Bool*)aValue.getValue() );
        switch( mnId )
        {
        case ID_EXT_TIMEFIELD:
        {
            SvxExtTimeField* pTime = PTR_CAST( SvxExtTimeField, aFieldItem.GetField() );
            if(pTime)
                pTime->SetType( (SvxTimeType)bFix?SVXTIMETYPE_FIX:SVXTIMETYPE_VAR );
            break;
        }
        case ID_DATEFIELD:
        {
            SvxDateField* pDate = PTR_CAST( SvxDateField, aFieldItem.GetField() );
            if(pDate)
                pDate->SetType( (SvxDateType)bFix?SVXDATETYPE_FIX:SVXDATETYPE_VAR );
            break;
        }
        case ID_EXT_FILEFIELD:
        {
            SvxExtFileField* pFile = PTR_CAST( SvxExtFileField, aFieldItem.GetField() );
            if(pFile)
                pFile->SetType( (SvxFileType)bFix?SVXFILETYPE_FIX:SVXFILETYPE_VAR );
            break;
        }
        case ID_AUTHORFIELD:
        {
            SvxAuthorField* pAuthor = PTR_CAST( SvxAuthorField, aFieldItem.GetField() );
            if(pAuthor)
                pAuthor->SetType( (SvxAuthorType)bFix?SVXAUTHORTYPE_FIX:SVXAUTHORTYPE_VAR );
            break;
        }
        default:
            throw beans::UnknownPropertyException();
        }
        }
        break;
    case WID_PRES:
    case WID_URL:
    case WID_TARGET:
    {
        SvxURLField* pURL = PTR_CAST( SvxURLField, aFieldItem.GetField() );
        if(pURL)
        {
            OUString aUnoStr;
            if(!(aValue >>= aUnoStr))
                throw lang::IllegalArgumentException();

            switch( pMap->nWID )
            {
            case WID_PRES:
                pURL->SetRepresentation( aUnoStr );
                break;
            case WID_URL:
                pURL->SetURL( aUnoStr );
                break;
            case WID_TARGET:
                pURL->SetTargetFrame( aUnoStr );
                break;
            }
        }
        break;
    }
    }

    SfxItemSet aSet = pForwarder->GetAttribs( GetSelection() );
    aSet.Put( aFieldItem );
*/
}

uno::Any SAL_CALL SvxUnoTextField::getPropertyValue( const OUString& PropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    uno::Any aValue;

    const SfxItemPropertyMap* pMap = SfxItemPropertyMap::GetByName(mpPropSet->getPropertyMap(), PropertyName );
    if ( !pMap )
        throw beans::UnknownPropertyException();

    switch( pMap->nWID )
    {
    case WID_DOUBLE:
        aValue <<= mpImpl->mdDouble;
        break;
    case WID_BOOL1:
        aValue <<= mpImpl->mbBoolean1;
        break;
    case WID_BOOL2:
        aValue <<= mpImpl->mbBoolean2;
        break;
    case WID_INTEGER:
        aValue <<= mpImpl->mnInteger;
        break;
    case WID_STRING1:
        aValue <<= mpImpl->msString1;
        break;
    case WID_STRING2:
        aValue <<= mpImpl->msString2;
        break;
    case WID_STRING3:
        aValue <<= mpImpl->msString3;
        break;
    }

    return aValue;

/*
    switch(pMap->nWID)
    {
    case WID_FORMAT:
        switch( mnId )
        {
        case ID_DATEFIELD:
        {
            SvxDateField* pDate = PTR_CAST( SvxDateField, pFieldItem->GetField() );
            if(pDate)
                aValue <<= (sal_Int32)pDate->GetFormat();
            break;
        }
        case ID_URLFIELD:
        {
            SvxURLField* pURL = PTR_CAST( SvxURLField, pFieldItem->GetField() );
            if(pURL)
                aValue <<= (sal_Int32)pURL->GetFormat();
            break;
        }
        case ID_EXT_TIMEFIELD:
        {
            SvxExtTimeField* pTime = PTR_CAST( SvxExtTimeField, pFieldItem->GetField() );
            if(pTime)
                aValue <<= (sal_Int32)pTime->GetFormat();
            break;
        }
        case ID_EXT_FILEFIELD:
        {
            SvxExtFileField* pFile = PTR_CAST( SvxExtFileField, pFieldItem->GetField() );
            if(pFile)
                aValue <<= (sal_Int32)pFile->GetFormat();
            break;
        }
        case ID_AUTHORFIELD:
        {
            SvxAuthorField* pAuthor = PTR_CAST( SvxAuthorField, pFieldItem->GetField() );
            if(pAuthor)
                aValue <<= (sal_Int32)pAuthor->GetFormat();
            break;
        }
        default:
            throw beans::UnknownPropertyException();
        }
        break;
    case WID_FIX:
        {
            sal_Bool bFix = sal_False;
        switch( mnId )
        {
        case ID_EXT_TIMEFIELD:
        {
            SvxExtTimeField* pTime = PTR_CAST( SvxExtTimeField, pFieldItem->GetField() );
            if(pTime)
                bFix = pTime->GetType() == SVXTIMETYPE_FIX;
            break;
        }
        case ID_DATEFIELD:
        {
            SvxDateField* pDate = PTR_CAST( SvxDateField, pFieldItem->GetField() );
            if(pDate)
                bFix = pDate->GetType() == SVXDATETYPE_FIX;
            break;
        }
        case ID_EXT_FILEFIELD:
        {
            SvxExtFileField* pFile = PTR_CAST( SvxExtFileField, pFieldItem->GetField() );
            if(pFile)
                bFix = pFile->GetType() == SVXFILETYPE_FIX;
            break;
        }
        case ID_AUTHORFIELD:
        {
            SvxAuthorField* pAuthor = PTR_CAST( SvxAuthorField, pFieldItem->GetField() );
            if(pAuthor)
                bFix = pAuthor->GetType() == SVXAUTHORTYPE_FIX;
            break;
        }
        default:
            throw beans::UnknownPropertyException();
        }
        aValue.setValue( &bFix, ::getCppuBooleanType() );
        }
        break;
    case WID_PRES:
    case WID_URL:
    case WID_TARGET:
    {
        SvxURLField* pURL = PTR_CAST( SvxURLField, pFieldItem->GetField() );
        if(pURL)
        {
            OUString aStr;
            switch( pMap->nWID )
            {
            case WID_PRES:
                aStr = pURL->GetRepresentation();
                break;
            case WID_URL:
                aStr = pURL->GetURL();
                break;
            case WID_TARGET:
                aStr = pURL->GetTargetFrame();
                break;
            }
            aValue <<= aStr;
        }
        break;
    }
    case WID_FCOLOR:
    case WID_TCOLOR:
    {
        Color* pFColor = NULL;
        Color* pTColor = NULL;
        const ESelection aSel = GetSelection();

        pForwarder->CalcFieldValue( *pFieldItem, aSel.nStartPara, aSel.nStartPos, pTColor, pFColor );

        if( pMap->nWID == WID_FCOLOR )
            aValue <<= (sal_Int32)pFColor->GetColor();
        else
            aValue <<= (sal_Int32)pTColor->GetColor();
        break;

        delete pTColor;
        delete pFColor;
    }
    }
    return aValue;
*/
}

void SAL_CALL SvxUnoTextField::addPropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}
void SAL_CALL SvxUnoTextField::removePropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}
void SAL_CALL SvxUnoTextField::addVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}
void SAL_CALL SvxUnoTextField::removeVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}

// OComponentHelper
void SvxUnoTextField::disposing()
{
    // nothing to do
}

sal_Int32 SvxUnoTextField::GetFieldId( const SvxFieldData* pFieldData ) const throw()
{
    if( pFieldData->ISA( SvxURLField ) )
        return ID_URLFIELD;
    else if( pFieldData->ISA( SvxPageField ) )
        return ID_PAGEFIELD;
    else if( pFieldData->ISA( SvxPagesField ) )
        return ID_PAGESFIELD;
    else if( pFieldData->ISA( SvxTimeField )    )
        return ID_TIMEFIELD;
    else if( pFieldData->ISA( SvxFileField )    )
        return ID_FILEFIELD;
    else if( pFieldData->ISA( SvxTableField ) )
        return ID_TABLEFIELD;
    else if( pFieldData->ISA( SvxExtTimeField ) )
        return ID_EXT_TIMEFIELD;
    else if( pFieldData->ISA( SvxExtFileField ) )
        return ID_EXT_FILEFIELD;
    else if( pFieldData->ISA( SvxAuthorField ) )
        return ID_AUTHORFIELD;
    else if( pFieldData->ISA( SvxDateField )    )
        return ID_DATEFIELD;

    return ID_NOTFOUND;
}

// lang::XServiceInfo
OUString SAL_CALL SvxUnoTextField::getImplementationName() throw(uno::RuntimeException)
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM("SvxUnoTextField2"));
}

static const sal_Char* pServiceNames[] =
{
    "com.sun.star.text.TextField.DateTime",
    "com.sun.star.text.TextField.URL",
    "com.sun.star.text.TextField.PageNumber",
    "com.sun.star.text.TextField.PageCount",
    "com.sun.star.text.TextField.DateTime",
    "com.sun.star.text.TextField.FileName",
    "com.sun.star.text.TextField.SheetName",
    "com.sun.star.text.TextField.DateTime",
    "com.sun.star.text.TextField.FileName",
    "com.sun.star.text.TextField.Author"
};

uno::Sequence< OUString > SAL_CALL SvxUnoTextField::getSupportedServiceNames()
    throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSeq( 3 );
    OUString* pServices = aSeq.getArray();
    pServices[0] = OUString::createFromAscii( pServiceNames[mnServiceId] );
    pServices[1] = OUString::createFromAscii( "com.sun.star.text.TextContent" ),
    pServices[2] = OUString::createFromAscii( "com.sun.star.text.TextField" );

    return aSeq;
}

sal_Bool SAL_CALL SvxUnoTextField::supportsService( const OUString& ServiceName ) throw( uno::RuntimeException )
{
    return SvxServiceInfoHelper::supportsService( ServiceName, getSupportedServiceNames() );
}


