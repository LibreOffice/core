/*************************************************************************
 *
 *  $RCSfile: unofield.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:27 $
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

#include <rtl/uuid.h>
#include <rtl/memory.h>

#include "eeitem.hxx"
#define ITEMID_FIELD EE_FEATURE_FIELD
#include "flditem.hxx"
#include "unofield.hxx"
#include "unoprov.hxx"

using namespace ::rtl;
using namespace ::vos;
using namespace ::cppu;
using namespace ::com::sun::star;

#define QUERYINT( xint ) \
    if( rType == ::getCppuType((const uno::Reference< xint >*)0) ) \
        aAny <<= uno::Reference< xint >(this)


#define WID_FCOLOR  0
#define WID_FORMAT  1
#define WID_FIX     2
#define WID_TCOLOR  3
#define WID_PRES    4
#define WID_URL     5
#define WID_TARGET  6

#define ID_DATEFIELD        0
#define ID_URLFIELD         1
#define ID_PAGEFIELD        2
#define ID_PAGESFIELD       3
#define ID_TIMEFIELD        4
#define ID_FILEFIELD        5
#define ID_TABLEFIELD       6
#define ID_EXT_TIMEFIELD    7
#define ID_EXT_FILEFIELD    8
#define ID_AUTHORFIELD      9
#define ID_NOTFOUND         -1

SfxItemPropertyMap* ImplGetFieldItemPropertyMap( USHORT mnId )
{
    static SfxItemPropertyMap aDateFieldPropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN("FieldColor"),   WID_FCOLOR, &::getCppuType((const sal_Int32*)0),    beans::PropertyAttribute::READONLY, 0 },
        { MAP_CHAR_LEN("Format"),       WID_FORMAT, &::getCppuType((const sal_Int32*)0),    0, 0 },
        { MAP_CHAR_LEN("IsFix"),        WID_FIX,    &::getBooleanCppuType(),                0, 0 },
        { MAP_CHAR_LEN("TextColor"),    WID_TCOLOR, &::getCppuType((const sal_Int32*)0),    beans::PropertyAttribute::READONLY, 0 },
        {0,0}
    };

    static SfxItemPropertyMap aUrlFieldPropertyMap_Impl[] =
    {

        { MAP_CHAR_LEN("FieldColor"),       WID_FCOLOR, &::getCppuType((const sal_Int32*)0),    beans::PropertyAttribute::READONLY, 0 },
        { MAP_CHAR_LEN("Format"),           WID_FORMAT, &::getCppuType((const sal_Int32*)0),    0, 0 },
        { MAP_CHAR_LEN("Presentation"),     WID_PRES,   &::getCppuType((const OUString*)0),     0, 0 },
        { MAP_CHAR_LEN("TargetFrame"),      WID_TARGET, &::getCppuType((const OUString*)0),     0, 0 },
        { MAP_CHAR_LEN("TextColor"),        WID_TCOLOR, &::getCppuType((const sal_Int32*)0),    beans::PropertyAttribute::READONLY, 0 },
        { MAP_CHAR_LEN("URL"),              WID_URL,    &::getCppuType((const OUString*)0),     0, 0 },
        {0,0}
    };

    static SfxItemPropertyMap aPageFieldPropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN("FieldColor"),       WID_FCOLOR, &::getCppuType((const sal_Int32*)0),    beans::PropertyAttribute::READONLY, 0 },
        { MAP_CHAR_LEN("TextColor"),        WID_TCOLOR, &::getCppuType((const sal_Int32*)0),    beans::PropertyAttribute::READONLY, 0 },
        {0,0}
    };

    static SfxItemPropertyMap aExtTimeFieldPropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN("FieldColor"),       WID_FCOLOR, &::getCppuType((const sal_Int32*)0),    beans::PropertyAttribute::READONLY, 0 },
        { MAP_CHAR_LEN("Format"),           WID_FORMAT, &::getCppuType((const sal_Int32*)0),    0, 0 },
        { MAP_CHAR_LEN("IsFix"),            WID_FIX,    &::getBooleanCppuType(),                0, 0 },
        { MAP_CHAR_LEN("TextColor"),        WID_TCOLOR, &::getCppuType((const sal_Int32*)0),    beans::PropertyAttribute::READONLY, 0 },
        {0,0}
    };

    static SfxItemPropertyMap aExtFileFieldPropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN("FieldColor"),       WID_FCOLOR, &::getCppuType((const sal_Int32*)0),    beans::PropertyAttribute::READONLY, 0 },
        { MAP_CHAR_LEN("Format"),           WID_FORMAT, &::getCppuType((const sal_Int32*)0),    0,  0 },
        { MAP_CHAR_LEN("IsFix"),            WID_FIX,    &::getBooleanCppuType(),                0,  0 },
        { MAP_CHAR_LEN("TextColor"),        WID_TCOLOR, &::getCppuType((const sal_Int32*)0),    beans::PropertyAttribute::READONLY, 0 },
        {0,0}
    };

    static SfxItemPropertyMap aAuthorFieldPropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN("FieldColor"),       WID_FCOLOR, &::getCppuType((const sal_Int32*)0),    beans::PropertyAttribute::READONLY, 0 },
        { MAP_CHAR_LEN("Format"),           WID_FORMAT, &::getCppuType((const sal_Int32*)0),    0,  0 },
        { MAP_CHAR_LEN("IsFix"),            WID_FIX,    &::getBooleanCppuType(),                0,  0 },
        { MAP_CHAR_LEN("TextColor"),        WID_TCOLOR, &::getCppuType((const sal_Int32*)0),    beans::PropertyAttribute::READONLY, 0 },
        {0,0}
    };

    switch( mnId )
    {
    case ID_DATEFIELD:
        return aDateFieldPropertyMap_Impl;
    case ID_URLFIELD:
        return aUrlFieldPropertyMap_Impl;
    case ID_PAGEFIELD:
    case ID_PAGESFIELD:
    case ID_TIMEFIELD:
    case ID_FILEFIELD:
    case ID_TABLEFIELD:
        return aPageFieldPropertyMap_Impl;
    case ID_EXT_TIMEFIELD:
        return aExtTimeFieldPropertyMap_Impl;
    case ID_EXT_FILEFIELD:
        return aExtFileFieldPropertyMap_Impl;
//  case ID_AUTHORFIELD:
    default:
        return aAuthorFieldPropertyMap_Impl;
    }
}

static SfxItemPropertyMap aEmptyPropertyMap_Impl[] =
{
    {0,0}
};

/*
static SfxItemPropertyMap* aFieldItemPropertyMaps_Impl[] =
{
};
*/

static sal_Char* aFieldItemNameMap_Impl[] =
{
    "Date",
    "Url",
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

SvxUnoTextField::SvxUnoTextField() throw()
:   OComponentHelper( getMutex() ),
    mpDummyText( new SvxDummyTextSource() ),
    SvxUnoTextRangeBase(mpDummyText, aEmptyPropertyMap_Impl),
    mpPropSet(NULL),
    mnId(ID_NOTFOUND)
{
}

SvxUnoTextField::SvxUnoTextField( const SvxUnoText& rText, const ESelection& rSel, const SvxFieldData* pFieldData ) throw()
:   OComponentHelper( getMutex() ),
    SvxUnoTextRangeBase(rText),
    mpPropSet(NULL),
    mnId(ID_NOTFOUND),
    mpDummyText(NULL)
{
    DBG_ASSERT(pFieldData, "pFieldData == NULL! [CL]" );

    SetSelection(rSel);

    mxParentText = ((SvxUnoText*)&rText)->getText();

    if(pFieldData)
    {
        mnId = GetFieldId(pFieldData);
        DBG_ASSERT(mnId != ID_NOTFOUND, "unknown SvxFieldData! [CL]");
        if(mnId != ID_NOTFOUND)
            mpPropSet = new SfxItemPropertySet( ImplGetFieldItemPropertyMap(mnId) );
    }
}

SvxUnoTextField::~SvxUnoTextField() throw()
{
    delete mpPropSet;
    delete mpDummyText;
}

// uno::XInterface
uno::Any SAL_CALL SvxUnoTextField::queryAggregation( const uno::Type & rType )
    throw(uno::RuntimeException)
{
    uno::Any aAny;

    QUERYINT( beans::XPropertySet );
    else QUERYINT( beans::XPropertyState );
    else QUERYINT( text::XTextContent );
    else QUERYINT( text::XTextField );
    else QUERYINT( text::XTextRange );
    else QUERYINT( lang::XServiceInfo );
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

        maTypeSequence.realloc( nOldCount + 5 ); // !DANGER! keep this updated
        uno::Type* pTypes = &maTypeSequence.getArray()[nOldCount];

        *pTypes++ = ::getCppuType(( const uno::Reference< text::XTextField >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< beans::XPropertySet >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< beans::XPropertyState >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< text::XTextRange >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< lang::XServiceInfo >*)0);
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

    SvxTextForwarder* pForwarder = GetEditSource()->GetTextForwarder();
    if(!pForwarder->IsValid())
        throw uno::RuntimeException();

    if(bShowCommand)
    {
        return OUString::createFromAscii( aFieldItemNameMap_Impl[mnId] );
    }
    else
    {
        Color* pTColor = NULL;
        Color* pFColor = NULL;
        const SvxFieldItem* pField = GetField();
        if(pField == NULL)
            throw uno::RuntimeException();

        const ESelection aSel = GetSelection();

        String aStr( pForwarder->CalcFieldValue( *pField, aSel.nStartPara, aSel.nStartPos, pTColor, pFColor ) );

        delete pTColor;
        delete pFColor;
        return aStr;
    }
}

// Interface text::XTextContent
void SAL_CALL SvxUnoTextField::attach( const uno::Reference< text::XTextRange >& xTextRange )
    throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    throw uno::RuntimeException();
}

uno::Reference< text::XTextRange > SAL_CALL SvxUnoTextField::getAnchor()
    throw(uno::RuntimeException)
{
    uno::Reference< text::XTextRange > xRange( mxParentText, uno::UNO_QUERY );
    return xRange;
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

    uno::Reference< beans::XPropertySetInfo > aRet;

    if( mnId != ID_NOTFOUND )
    {
        aRet = new SfxExtItemPropertySetInfo( ImplGetFieldItemPropertyMap(mnId),
                                               SvxUnoTextRangeBase::getPropertySetInfo()->getProperties() );
    }
    else
    {
        aRet = SvxUnoTextRangeBase::getPropertySetInfo();
    }
    return aRet;
}

void SAL_CALL SvxUnoTextField::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
    throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( GetField() == NULL )
        throw uno::RuntimeException();

    const SfxItemPropertyMap* pMap = SfxItemPropertyMap::GetByName(mpPropSet->getPropertyMap(), aPropertyName );
    if ( !pMap )
    {
        SvxUnoTextRangeBase::setPropertyValue( aPropertyName, aValue );
        return;
    }

    SvxTextForwarder* pForwarder = GetEditSource()->GetTextForwarder();
    SvxFieldItem aFieldItem( *GetField() );

    switch( pMap->nWID )
    {
    case WID_FORMAT:
        {
        sal_Int32 nFormat;
        if(!(aValue >>= nFormat))
            throw lang::IllegalArgumentException();

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
}

uno::Any SAL_CALL SvxUnoTextField::getPropertyValue( const OUString& PropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    uno::Any aValue;

    const SfxItemPropertyMap* pMap = SfxItemPropertyMap::GetByName(mpPropSet->getPropertyMap(), PropertyName );
    if ( !pMap )
        return SvxUnoTextRangeBase::getPropertyValue( PropertyName );

    SvxTextForwarder* pForwarder = GetEditSource()->GetTextForwarder();
    const SvxFieldItem* pFieldItem = GetField();

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
}

// beans::XPropertyState
beans::PropertyState SAL_CALL SvxUnoTextField::getPropertyState( const OUString& PropertyName )
    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    const SfxItemPropertyMap* pMap = SfxItemPropertyMap::GetByName(mpPropSet->getPropertyMap(), PropertyName );
    if ( !pMap )
        return SvxUnoTextRangeBase::getPropertyState( PropertyName );

    SvxTextForwarder* pForwarder = GetEditSource()->GetTextForwarder();
    const SvxFieldItem* pFieldItem = GetField();

    sal_Bool bDefault = sal_True;

    switch(pMap->nWID)
    {
    case WID_FORMAT:
        switch( mnId )
        {
        case ID_DATEFIELD:
        {
            SvxDateField* pDate = PTR_CAST( SvxDateField, pFieldItem->GetField() );
            if(pDate)
                bDefault = pDate->GetFormat() == SVXDATEFORMAT_STDSMALL;
            break;
        }
        case ID_URLFIELD:
        {
            SvxURLField* pURL = PTR_CAST( SvxURLField, pFieldItem->GetField() );
            if(pURL)
                bDefault = pURL->GetFormat() == SVXURLFORMAT_URL;
            break;
        }
        case ID_EXT_TIMEFIELD:
        {
            SvxExtTimeField* pTime = PTR_CAST( SvxExtTimeField, pFieldItem->GetField() );
            if(pTime)
                bDefault = pTime->GetFormat() == SVXTIMEFORMAT_STANDARD;
            break;
        }
        case ID_EXT_FILEFIELD:
        {
            SvxExtFileField* pFile = PTR_CAST( SvxExtFileField, pFieldItem->GetField() );
            if(pFile)
                bDefault = pFile->GetFormat() == SVXFILEFORMAT_FULLPATH;
            break;
        }
        case ID_AUTHORFIELD:
        {
            SvxAuthorField* pAuthor = PTR_CAST( SvxAuthorField, pFieldItem->GetField() );
            if(pAuthor)
                bDefault = pAuthor->GetFormat() == SVXAUTHORFORMAT_FULLNAME;
            break;
        }
        default:
            throw beans::UnknownPropertyException();
        }
        break;
    case WID_FIX:
        switch( mnId )
        {
        case ID_EXT_TIMEFIELD:
        {
            SvxExtTimeField* pTime = PTR_CAST( SvxExtTimeField, pFieldItem->GetField() );
            if(pTime)
                bDefault = pTime->GetType() != SVXTIMETYPE_FIX;
            break;
        }
        case ID_DATEFIELD:
        {
            SvxDateField* pDate = PTR_CAST( SvxDateField, pFieldItem->GetField() );
            if(pDate)
                bDefault = pDate->GetType() != SVXDATETYPE_FIX;
            break;
        }
        case ID_EXT_FILEFIELD:
        {
            SvxExtFileField* pFile = PTR_CAST( SvxExtFileField, pFieldItem->GetField() );
            if(pFile)
                bDefault = pFile->GetType() != SVXFILETYPE_FIX;;
            break;
        }
        case ID_AUTHORFIELD:
        {
            SvxAuthorField* pAuthor = PTR_CAST( SvxAuthorField, pFieldItem->GetField() );
            if(pAuthor)
                bDefault = pAuthor->GetType() != SVXAUTHORTYPE_FIX;
            break;
        }
        default:
            throw beans::UnknownPropertyException();
        }
        break;
    case WID_PRES:
    case WID_URL:
    case WID_TARGET:
    {
        SvxURLField* pURL = PTR_CAST( SvxURLField, pFieldItem->GetField() );
        if(pURL)
        {
            String aStr;
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
            bDefault = aStr.Len() == 0;
        }
        break;
    }
    case WID_FCOLOR:
    case WID_TCOLOR:
    {
        bDefault = sal_False;
        break;
    }
    }

    return bDefault?beans::PropertyState_DEFAULT_VALUE:beans::PropertyState_DIRECT_VALUE;
}

void SAL_CALL SvxUnoTextField::setPropertyToDefault( const OUString& PropertyName )
    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    const SfxItemPropertyMap* pMap = SfxItemPropertyMap::GetByName(mpPropSet->getPropertyMap(), PropertyName );
    if ( !pMap )
    {
        SvxUnoTextRangeBase::setPropertyToDefault( PropertyName );
    }
    else
    {
        SvxTextForwarder* pForwarder = GetEditSource()->GetTextForwarder();
        SvxFieldItem aFieldItem( *GetField() );

        switch( pMap->nWID )
        {
        case WID_FORMAT:
            switch( mnId )
            {
            case ID_DATEFIELD:
            {
                SvxDateField* pDate = PTR_CAST( SvxDateField, aFieldItem.GetField() );
                if(pDate)
                    pDate->SetFormat( SVXDATEFORMAT_STDSMALL );
                break;
            }
            case ID_URLFIELD:
            {
                SvxURLField* pURL = PTR_CAST( SvxURLField, aFieldItem.GetField() );
                if(pURL)
                    pURL->SetFormat( SVXURLFORMAT_URL );
                break;
            }
            case ID_EXT_TIMEFIELD:
            {
                SvxExtTimeField* pTime = PTR_CAST( SvxExtTimeField, aFieldItem.GetField() );
                if(pTime)
                    pTime->SetFormat( SVXTIMEFORMAT_STANDARD );
                break;
            }
            case ID_EXT_FILEFIELD:
            {
                SvxExtFileField* pFile = PTR_CAST( SvxExtFileField, aFieldItem.GetField() );
                if(pFile)
                    pFile->SetFormat( SVXFILEFORMAT_FULLPATH );
                break;
            }
            case ID_AUTHORFIELD:
            {
                SvxAuthorField* pAuthor = PTR_CAST( SvxAuthorField, aFieldItem.GetField() );
                if(pAuthor)
                    pAuthor->SetFormat( SVXAUTHORFORMAT_FULLNAME );
                break;
            }
            default:
                throw beans::UnknownPropertyException();
            }
            break;
        case WID_FIX:
            switch( mnId )
            {
            case ID_EXT_TIMEFIELD:
            {
                SvxExtTimeField* pTime = PTR_CAST( SvxExtTimeField, aFieldItem.GetField() );
                if(pTime)
                    pTime->SetType( SVXTIMETYPE_VAR );
                break;
            }
            case ID_DATEFIELD:
            {
                SvxDateField* pDate = PTR_CAST( SvxDateField, aFieldItem.GetField() );
                if(pDate)
                    pDate->SetType( SVXDATETYPE_VAR );
                break;
            }
            case ID_EXT_FILEFIELD:
            {
                SvxExtFileField* pFile = PTR_CAST( SvxExtFileField, aFieldItem.GetField() );
                if(pFile)
                    pFile->SetType( SVXFILETYPE_VAR );
                break;
            }
            case ID_AUTHORFIELD:
            {
                SvxAuthorField* pAuthor = PTR_CAST( SvxAuthorField, aFieldItem.GetField() );
                if(pAuthor)
                    pAuthor->SetType( SVXAUTHORTYPE_VAR );
                break;
            }
            default:
                throw beans::UnknownPropertyException();
            }
            break;
        case WID_PRES:
        case WID_URL:
        case WID_TARGET:
        {
            SvxURLField* pURL = PTR_CAST( SvxURLField, aFieldItem.GetField() );
            if(pURL)
            {
                String aStr;
                switch( pMap->nWID )
                {
                case WID_PRES:
                    pURL->SetRepresentation( aStr );
                    break;
                case WID_URL:
                    pURL->SetURL( aStr );
                    break;
                case WID_TARGET:
                    pURL->SetTargetFrame( aStr );
                    break;
                }
            }
            break;
        }
        }
    }
}

uno::Any SAL_CALL SvxUnoTextField::getPropertyDefault( const OUString& aPropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    uno::Any aValue;

    const SfxItemPropertyMap* pMap = SfxItemPropertyMap::GetByName(mpPropSet->getPropertyMap(), aPropertyName );
    if ( !pMap )
        return SvxUnoTextRangeBase::getPropertyDefault( aPropertyName );

    switch(pMap->nWID)
    {
    case WID_FORMAT:
        {
        sal_Int32 nFormat;
        switch( mnId )
        {
        case ID_DATEFIELD:
        {
            nFormat = SVXDATEFORMAT_STDSMALL;
            break;
        }
        case ID_URLFIELD:
        {
            nFormat = SVXURLFORMAT_URL;
            break;
        }
        case ID_EXT_TIMEFIELD:
        {
            nFormat = SVXTIMEFORMAT_STANDARD;
            break;
        }
        case ID_EXT_FILEFIELD:
        {
            nFormat = SVXFILEFORMAT_FULLPATH;
            break;
        }
        case ID_AUTHORFIELD:
        {
            nFormat = SVXAUTHORFORMAT_FULLNAME;
            break;
        }
        aValue <<= nFormat;
        }
        default:
            throw beans::UnknownPropertyException();
        }
        break;
    case WID_FIX:
        switch( mnId )
        {
        case ID_EXT_TIMEFIELD:
        case ID_DATEFIELD:
        case ID_EXT_FILEFIELD:
        case ID_AUTHORFIELD:
        {
            sal_Bool bFix( sal_False );
            aValue.setValue( &bFix, ::getCppuBooleanType() );
            break;
        }
        default:
            throw beans::UnknownPropertyException();
        }
        break;
    case WID_PRES:
    case WID_URL:
    case WID_TARGET:
    {
        OUString aEmptyURL;
        aValue <<= aEmptyURL;
        break;
    }
    case WID_FCOLOR:
    {
        aValue <<= (sal_Int32)0xffffff;
        break;
    }
    case WID_TCOLOR:
    {
        aValue <<= (sal_Int32)0x000000;
        break;
    }
    }
    return aValue;
}

// OComponentHelper
void SvxUnoTextField::disposing()
{
    // todo
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

const SvxFieldItem* SvxUnoTextField::GetField() const throw()
{
    SvxTextForwarder* pForwarder = GetEditSource()->GetTextForwarder();
    SfxItemSet aSet = pForwarder->GetAttribs( GetSelection() );

    if ( aSet.GetItemState( EE_FEATURE_FIELD ) == SFX_ITEM_ON )
        return (SvxFieldItem*)aSet.GetItem( EE_FEATURE_FIELD );
    else
        return NULL;
}

// Interface text::XTextRange
uno::Reference< text::XText > SAL_CALL SvxUnoTextField::getText()
    throw(uno::RuntimeException)
{
    return mxParentText;
}

// lang::XServiceInfo
OUString SAL_CALL SvxUnoTextField::getImplementationName() throw(uno::RuntimeException)
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM("SvxUnoTextField"));
}

uno::Sequence< OUString > SAL_CALL SvxUnoTextField::getSupportedServiceNames()
    throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSeq( SvxUnoTextRangeBase::getSupportedServiceNames() );
    SvxServiceInfoHelper::addToSequence( aSeq, 3, "com.sun.star.style.ParagraphProperties",
                                                  "com.sun.star.text.TextContent",
                                                  "com.sun.star.text.TextField");
    return aSeq;
}

