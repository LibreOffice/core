/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/text/FilenameDisplayFormat.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <vcl/svapp.hxx>
#include <tools/debug.hxx>

#include <editeng/eeitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/CustomPropertyField.hxx>
#include <editeng/measfld.hxx>
#include <editeng/unofield.hxx>
#include <editeng/unotext.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <sal/log.hxx>

#include <editeng/unonames.hxx>

using namespace ::cppu;
using namespace ::com::sun::star;

#define QUERYINT( xint ) \
    if( rType == cppu::UnoType<xint>::get() ) \
        aAny <<= uno::Reference< xint >(this)


#define WID_DATE    0
#define WID_BOOL1   1
#define WID_BOOL2   2
#define WID_INT32   3
#define WID_INT16   4
#define WID_STRING1 5
#define WID_STRING2 6
#define WID_STRING3 7

class SvxUnoFieldData_Impl
{
public:
    bool    mbBoolean1;
    bool    mbBoolean2;
    sal_Int32   mnInt32;
    sal_Int16   mnInt16;
    OUString    msString1;
    OUString    msString2;
    OUString    msString3;
    util::DateTime maDateTime;

    OUString    msPresentation;
};

static const SfxItemPropertySet* ImplGetFieldItemPropertySet( sal_Int32 mnId )
{
    static const SfxItemPropertyMapEntry aExDateTimeFieldPropertyMap_Impl[] =
    {
        { OUString(UNO_TC_PROP_DATE_TIME), WID_DATE,  ::cppu::UnoType<util::DateTime>::get(), 0, 0 },
        { OUString(UNO_TC_PROP_IS_FIXED),  WID_BOOL1, cppu::UnoType<bool>::get(),                  0, 0 },
        { OUString(UNO_TC_PROP_IS_DATE),   WID_BOOL2, cppu::UnoType<bool>::get(),                  0, 0 },
        { OUString(UNO_TC_PROP_NUMFORMAT), WID_INT32, ::cppu::UnoType<sal_Int32>::get(),      0, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    static const SfxItemPropertySet aExDateTimeFieldPropertySet_Impl(aExDateTimeFieldPropertyMap_Impl);

    static const SfxItemPropertyMapEntry aDateTimeFieldPropertyMap_Impl[] =
    {
        { OUString(UNO_TC_PROP_IS_DATE), WID_BOOL2, cppu::UnoType<bool>::get(), 0, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    static const SfxItemPropertySet aDateTimeFieldPropertySet_Impl(aDateTimeFieldPropertyMap_Impl);

    static const SfxItemPropertyMapEntry aUrlFieldPropertyMap_Impl[] =
    {

        { OUString(UNO_TC_PROP_URL_FORMAT),         WID_INT16,   ::cppu::UnoType<sal_Int16>::get(), 0, 0 },
        { OUString(UNO_TC_PROP_URL_REPRESENTATION), WID_STRING1, ::cppu::UnoType<OUString>::get(),  0, 0 },
        { OUString(UNO_TC_PROP_URL_TARGET),         WID_STRING2, ::cppu::UnoType<OUString>::get(),  0, 0 },
        { OUString(UNO_TC_PROP_URL),                WID_STRING3, ::cppu::UnoType<OUString>::get(),  0, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    static const SfxItemPropertySet aUrlFieldPropertySet_Impl(aUrlFieldPropertyMap_Impl);

    static const SfxItemPropertyMapEntry aEmptyPropertyMap_Impl[] =
    {
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    static const SfxItemPropertySet aEmptyPropertySet_Impl(aEmptyPropertyMap_Impl);

    static const SfxItemPropertyMapEntry aExtFileFieldPropertyMap_Impl[] =
    {
        { OUString(UNO_TC_PROP_IS_FIXED),             WID_BOOL1,   cppu::UnoType<bool>::get(),             0, 0 },
        { OUString(UNO_TC_PROP_FILE_FORMAT),          WID_INT16,   ::cppu::UnoType<sal_Int16>::get(), 0, 0 },
        { OUString(UNO_TC_PROP_CURRENT_PRESENTATION), WID_STRING1, ::cppu::UnoType<OUString>::get(),  0, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    static const SfxItemPropertySet aExtFileFieldPropertySet_Impl(aExtFileFieldPropertyMap_Impl);

    static const SfxItemPropertyMapEntry aAuthorFieldPropertyMap_Impl[] =
    {
        { OUString(UNO_TC_PROP_IS_FIXED),             WID_BOOL1,  cppu::UnoType<bool>::get(),             0, 0 },
        { OUString(UNO_TC_PROP_CURRENT_PRESENTATION), WID_STRING1,::cppu::UnoType<OUString>::get(),  0, 0 },
        { OUString(UNO_TC_PROP_AUTHOR_CONTENT),       WID_STRING2,::cppu::UnoType<OUString>::get(),  0, 0 },
        { OUString(UNO_TC_PROP_AUTHOR_FORMAT),        WID_INT16,  ::cppu::UnoType<sal_Int16>::get(), 0, 0 },
        { OUString(UNO_TC_PROP_AUTHOR_FULLNAME),      WID_BOOL2,  cppu::UnoType<bool>::get(),             0, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    static const SfxItemPropertySet aAuthorFieldPropertySet_Impl(aAuthorFieldPropertyMap_Impl);

    static const SfxItemPropertyMapEntry aMeasureFieldPropertyMap_Impl[] =
    {
        { OUString(UNO_TC_PROP_MEASURE_KIND), WID_INT16,  ::cppu::UnoType<sal_Int16>::get(), 0, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    static const SfxItemPropertySet aMeasureFieldPropertySet_Impl(aMeasureFieldPropertyMap_Impl);

    static const SfxItemPropertyMapEntry aDocInfoCustomFieldPropertyMap_Impl[] =
    {
        { OUString(UNO_TC_PROP_NAME),                 WID_STRING1, cppu::UnoType<OUString>::get(),   0, 0 },
        { OUString(UNO_TC_PROP_CURRENT_PRESENTATION), WID_STRING2, cppu::UnoType<OUString>::get(),   0, 0 },
        { OUString(UNO_TC_PROP_IS_FIXED),             WID_BOOL1,   cppu::UnoType<bool>::get(),       0, 0 },
        { OUString(UNO_TC_PROP_NUMFORMAT),            WID_INT32,   cppu::UnoType<sal_Int32>::get(),  0, 0 },
        { OUString(UNO_TC_PROP_IS_FIXED_LANGUAGE),    WID_BOOL2,   cppu::UnoType<bool>::get(),       0, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    static const SfxItemPropertySet aDocInfoCustomFieldPropertySet_Impl(aDocInfoCustomFieldPropertyMap_Impl);

    switch( mnId )
    {
    case text::textfield::Type::EXTENDED_TIME:
    case text::textfield::Type::DATE:
        return &aExDateTimeFieldPropertySet_Impl;
    case text::textfield::Type::URL:
        return &aUrlFieldPropertySet_Impl;
    case text::textfield::Type::TIME:
        return &aDateTimeFieldPropertySet_Impl;
    case text::textfield::Type::EXTENDED_FILE:
        return &aExtFileFieldPropertySet_Impl;
    case text::textfield::Type::AUTHOR:
        return &aAuthorFieldPropertySet_Impl;
    case text::textfield::Type::MEASURE:
        return &aMeasureFieldPropertySet_Impl;
    case text::textfield::Type::DOCINFO_CUSTOM:
        return &aDocInfoCustomFieldPropertySet_Impl;
    default:
        return &aEmptyPropertySet_Impl;
    }
}

/* conversion routines */

static sal_Int16 getFileNameDisplayFormat( SvxFileFormat nFormat )
{
    switch( nFormat )
    {
    case SvxFileFormat::NameAndExt:    return text::FilenameDisplayFormat::NAME_AND_EXT;
    case SvxFileFormat::PathFull:    return text::FilenameDisplayFormat::FULL;
    case SvxFileFormat::PathOnly:    return text::FilenameDisplayFormat::PATH;
//  case SvxFileFormat::NameOnly:
    default: return text::FilenameDisplayFormat::NAME;
    }
}

static SvxFileFormat setFileNameDisplayFormat( sal_Int16 nFormat )
{
    switch( nFormat )
    {
    case text::FilenameDisplayFormat::FULL: return SvxFileFormat::PathFull;
    case text::FilenameDisplayFormat::PATH: return SvxFileFormat::PathOnly;
    case text::FilenameDisplayFormat::NAME: return SvxFileFormat::NameOnly;
//  case text::FilenameDisplayFormat::NAME_AND_EXT:
    default:
        return SvxFileFormat::NameAndExt;
    }
}

static util::DateTime getDate( sal_Int32 nDate )
{
    util::DateTime aDate;

    Date aTempDate( nDate );

    aDate.Day = aTempDate.GetDay();
    aDate.Month = aTempDate.GetMonth();
    aDate.Year = aTempDate.GetYear();

    return aDate;
}

static Date setDate( util::DateTime const & rDate )
{
    return Date( rDate.Day, rDate.Month, rDate.Year );
}

static util::DateTime getTime(sal_Int64 const nTime)
{
    util::DateTime aTime;

    tools::Time aTempTime( nTime );

    aTime.NanoSeconds = aTempTime.GetNanoSec();
    aTime.Seconds = aTempTime.GetSec();
    aTime.Minutes = aTempTime.GetMin();
    aTime.Hours = aTempTime.GetHour();

    return aTime;
}

static tools::Time setTime( util::DateTime const & rDate )
{
    return tools::Time( rDate  );
}


// class SvxUnoTextField

namespace
{
    class theSvxUnoTextFieldUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSvxUnoTextFieldUnoTunnelId> {};
}

const css::uno::Sequence< sal_Int8 > & SvxUnoTextField::getUnoTunnelId() throw()
{
    return theSvxUnoTextFieldUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL SvxUnoTextField::getSomething( const css::uno::Sequence< sal_Int8 >& rId )
{
    if( rId.getLength() == 16 && 0 == memcmp( getUnoTunnelId().getConstArray(),
                                                         rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this));
    }
    return 0;
}

SvxUnoTextField::SvxUnoTextField( sal_Int32 nServiceId ) throw()
:   OComponentHelper( getMutex() )
,   mpPropSet(nullptr)
,   mnServiceId(nServiceId)
,   mpImpl( new SvxUnoFieldData_Impl )
{
    mpPropSet = ImplGetFieldItemPropertySet(mnServiceId);

    mpImpl->maDateTime.NanoSeconds = 0;
    mpImpl->maDateTime.Seconds = 0;
    mpImpl->maDateTime.Minutes = 0;
    mpImpl->maDateTime.Hours = 0;
    mpImpl->maDateTime.Day = 0;
    mpImpl->maDateTime.Month = 0;
    mpImpl->maDateTime.Year = 0;
    mpImpl->maDateTime.IsUTC = false;

    switch( nServiceId )
    {
    case text::textfield::Type::DATE:
        mpImpl->mbBoolean2 = true;
        mpImpl->mnInt32 = static_cast<sal_Int32>(SvxDateFormat::StdSmall);
        mpImpl->mbBoolean1 = false;
        break;

    case text::textfield::Type::EXTENDED_TIME:
    case text::textfield::Type::TIME:
        mpImpl->mbBoolean2 = false;
        mpImpl->mbBoolean1 = false;
        mpImpl->mnInt32 = static_cast<sal_Int32>(SvxTimeFormat::Standard);
        break;

    case text::textfield::Type::URL:
        mpImpl->mnInt16 = static_cast<sal_uInt16>(SvxURLFormat::Repr);
        break;

    case text::textfield::Type::EXTENDED_FILE:
        mpImpl->mbBoolean1 = false;
        mpImpl->mnInt16 = text::FilenameDisplayFormat::FULL;
        break;

    case text::textfield::Type::AUTHOR:
        mpImpl->mnInt16 = static_cast<sal_uInt16>(SvxAuthorFormat::FullName);
        mpImpl->mbBoolean1 = false;
        mpImpl->mbBoolean2 = true;
        break;

    case text::textfield::Type::MEASURE:
        mpImpl->mnInt16 = static_cast<sal_uInt16>(SdrMeasureFieldKind::Value);
        break;

    case text::textfield::Type::DOCINFO_CUSTOM:
        mpImpl->mbBoolean1 = true;
        mpImpl->mbBoolean2 = true;
        mpImpl->mnInt32 = 0;
        break;

    default:
        mpImpl->mbBoolean1 = false;
        mpImpl->mbBoolean2 = false;
        mpImpl->mnInt32 = 0;
        mpImpl->mnInt16 = 0;

    }
}

SvxUnoTextField::SvxUnoTextField( uno::Reference< text::XTextRange > const & xAnchor, const OUString& rPresentation, const SvxFieldData* pData ) throw()
:   OComponentHelper( getMutex() )
,   mxAnchor( xAnchor )
,   mpPropSet(nullptr)
,   mnServiceId(text::textfield::Type::UNSPECIFIED)
,   mpImpl( new SvxUnoFieldData_Impl )
{
    DBG_ASSERT(pData, "pFieldData == NULL! [CL]" );

    mpImpl->msPresentation = rPresentation;

    if(pData)
    {
        mnServiceId = pData->GetClassId();
        DBG_ASSERT(mnServiceId != text::textfield::Type::UNSPECIFIED, "unknown SvxFieldData! [CL]");
        if (mnServiceId != text::textfield::Type::UNSPECIFIED)
        {
            // extract field properties from data class
            switch( mnServiceId )
            {
            case text::textfield::Type::DATE:
                {
                    mpImpl->mbBoolean2 = true;
                    // #i35416# for variable date field, don't use invalid "0000-00-00" date,
                    // use current date instead
                    bool bFixed = static_cast<const SvxDateField*>(pData)->GetType() == SvxDateType::Fix;
                    mpImpl->maDateTime = getDate( bFixed ?
                                            static_cast<const SvxDateField*>(pData)->GetFixDate() :
                                            Date( Date::SYSTEM ).GetDate() );
                    mpImpl->mnInt32 = static_cast<sal_Int32>(static_cast<const SvxDateField*>(pData)->GetFormat());
                    mpImpl->mbBoolean1 = bFixed;
                }
                break;

            case text::textfield::Type::TIME:
                mpImpl->mbBoolean2 = false;
                mpImpl->mbBoolean1 = false;
                mpImpl->mnInt32 = static_cast<sal_Int32>(SvxTimeFormat::Standard);
                break;

            case text::textfield::Type::EXTENDED_TIME:
                mpImpl->mbBoolean2 = false;
                mpImpl->maDateTime = getTime( static_cast<const SvxExtTimeField*>(pData)->GetFixTime() );
                mpImpl->mbBoolean1 = static_cast<const SvxExtTimeField*>(pData)->GetType() == SvxTimeType::Fix;
                mpImpl->mnInt32 = static_cast<sal_Int32>(static_cast<const SvxExtTimeField*>(pData)->GetFormat());
                break;

            case text::textfield::Type::URL:
                mpImpl->msString1 = static_cast<const SvxURLField*>(pData)->GetRepresentation();
                mpImpl->msString2 = static_cast<const SvxURLField*>(pData)->GetTargetFrame();
                mpImpl->msString3 = static_cast<const SvxURLField*>(pData)->GetURL();
                mpImpl->mnInt16 = sal::static_int_cast< sal_Int16 >(
                    static_cast<const SvxURLField*>(pData)->GetFormat());
                break;

            case text::textfield::Type::EXTENDED_FILE:
                mpImpl->msString1 = static_cast<const SvxExtFileField*>(pData)->GetFile();
                mpImpl->mbBoolean1 = static_cast<const SvxExtFileField*>(pData)->GetType() == SvxFileType::Fix;
                mpImpl->mnInt16 = getFileNameDisplayFormat(static_cast<const SvxExtFileField*>(pData)->GetFormat());
                break;

            case text::textfield::Type::AUTHOR:
                mpImpl->msString1  = static_cast<const SvxAuthorField*>(pData)->GetFormatted();
                mpImpl->msString2  = static_cast<const SvxAuthorField*>(pData)->GetFormatted();
                mpImpl->mnInt16    = sal::static_int_cast< sal_Int16 >(
                    static_cast<const SvxAuthorField*>(pData)->GetFormat());
                mpImpl->mbBoolean1 = static_cast<const SvxAuthorField*>(pData)->GetType() == SvxAuthorType::Fix;
                mpImpl->mbBoolean2 = static_cast<const SvxAuthorField*>(pData)->GetFormat() != SvxAuthorFormat::ShortName;
                break;

            case text::textfield::Type::MEASURE:
                mpImpl->mnInt16     = sal::static_int_cast< sal_Int16 >(static_cast<const SdrMeasureField*>(pData)->GetMeasureFieldKind());
                break;

            case text::textfield::Type::DOCINFO_CUSTOM:
                mpImpl->msString1 = static_cast<const editeng::CustomPropertyField*>(pData)->GetName();
                mpImpl->msString2 = static_cast<const editeng::CustomPropertyField*>(pData)->GetCurrentPresentation();
                mpImpl->mbBoolean1 = false;
                mpImpl->mbBoolean2 = false;
                mpImpl->mnInt32 = 0;
                break;

            default:
                SAL_WARN("editeng", "Id service unknown: " << mnServiceId);
                break;
            }
        }
    }

    mpPropSet = ImplGetFieldItemPropertySet(mnServiceId);
}

SvxUnoTextField::~SvxUnoTextField() throw()
{
}

std::unique_ptr<SvxFieldData> SvxUnoTextField::CreateFieldData() const throw()
{
    std::unique_ptr<SvxFieldData> pData;

    switch( mnServiceId )
    {
    case text::textfield::Type::TIME:
    case text::textfield::Type::EXTENDED_TIME:
    case text::textfield::Type::DATE:
    {
        if( mpImpl->mbBoolean2 ) // IsDate?
        {
            Date aDate( setDate( mpImpl->maDateTime ) );
            pData.reset( new SvxDateField( aDate, mpImpl->mbBoolean1?SvxDateType::Fix:SvxDateType::Var ) );
            if( mpImpl->mnInt32 >= static_cast<sal_Int32>(SvxDateFormat::AppDefault) &&
                mpImpl->mnInt32 <= static_cast<sal_Int32>(SvxDateFormat::F) )
                static_cast<SvxDateField*>(pData.get())->SetFormat( static_cast<SvxDateFormat>(mpImpl->mnInt32) );
        }
        else
        {
            if( mnServiceId != text::textfield::Type::TIME && mnServiceId != text::textfield::Type::DATE )
            {
                tools::Time aTime( setTime( mpImpl->maDateTime ) );
                pData.reset( new SvxExtTimeField( aTime, mpImpl->mbBoolean1?SvxTimeType::Fix:SvxTimeType::Var ) );

                if( static_cast<SvxTimeFormat>(mpImpl->mnInt32) >= SvxTimeFormat::AppDefault &&
                    static_cast<SvxTimeFormat>(mpImpl->mnInt32) <= SvxTimeFormat::HH12_MM_SS_00_AMPM )
                    static_cast<SvxExtTimeField*>(pData.get())->SetFormat( static_cast<SvxTimeFormat>(mpImpl->mnInt32) );
            }
            else
            {
                pData.reset( new SvxTimeField() );
            }
        }

    }
        break;

    case text::textfield::Type::URL:
        pData.reset( new SvxURLField( mpImpl->msString3, mpImpl->msString1, !mpImpl->msString1.isEmpty() ? SvxURLFormat::Repr : SvxURLFormat::Url ) );
        static_cast<SvxURLField*>(pData.get())->SetTargetFrame( mpImpl->msString2 );
        if( static_cast<SvxURLFormat>(mpImpl->mnInt16) >= SvxURLFormat::AppDefault &&
            static_cast<SvxURLFormat>(mpImpl->mnInt16) <= SvxURLFormat::Repr )
            static_cast<SvxURLField*>(pData.get())->SetFormat( static_cast<SvxURLFormat>(mpImpl->mnInt16) );
        break;

    case text::textfield::Type::PAGE:
        pData.reset( new SvxPageField() );
        break;

    case text::textfield::Type::PAGES:
        pData.reset( new SvxPagesField() );
        break;

    case text::textfield::Type::DOCINFO_TITLE:
        pData.reset( new SvxFileField() );
        break;

    case text::textfield::Type::TABLE:
        pData.reset( new SvxTableField() );
        break;

    case text::textfield::Type::EXTENDED_FILE:
    {
        // #92009# pass fixed attribute to constructor
        pData.reset( new SvxExtFileField( mpImpl->msString1,
                                     mpImpl->mbBoolean1 ? SvxFileType::Fix : SvxFileType::Var,
                                     setFileNameDisplayFormat(mpImpl->mnInt16 ) ) );
        break;
    }

    case text::textfield::Type::AUTHOR:
    {
        OUString aContent;
        OUString aFirstName;
        OUString aLastName;

        // do we have CurrentPresentation given?
        // mimic behaviour of writer, which means:
        // prefer CurrentPresentation over Content
        // if both are given.
        if( !mpImpl->msString1.isEmpty() )
            aContent = mpImpl->msString1;
        else
            aContent = mpImpl->msString2;

        sal_Int32 nPos = aContent.lastIndexOf( sal_Char(' '), 0 );
        if( nPos > 0 )
        {
            aFirstName = aContent.copy( 0, nPos );
            aLastName = aContent.copy( nPos + 1 );
        }
        else
        {
            aLastName = aContent;
        }

        // #92009# pass fixed attribute to constructor
        pData.reset( new SvxAuthorField( aFirstName, aLastName, "",
                                    mpImpl->mbBoolean1 ? SvxAuthorType::Fix : SvxAuthorType::Var ) );

        if( !mpImpl->mbBoolean2 )
        {
            static_cast<SvxAuthorField*>(pData.get())->SetFormat( SvxAuthorFormat::ShortName );
        }
        else if( static_cast<SvxAuthorFormat>(mpImpl->mnInt16) >= SvxAuthorFormat::FullName &&
                 static_cast<SvxAuthorFormat>(mpImpl->mnInt16) <= SvxAuthorFormat::ShortName )
        {
            static_cast<SvxAuthorField*>(pData.get())->SetFormat( static_cast<SvxAuthorFormat>(mpImpl->mnInt16) );
        }

        break;
    }

    case text::textfield::Type::MEASURE:
    {
        SdrMeasureFieldKind eKind = SdrMeasureFieldKind::Value;
        if( mpImpl->mnInt16 == sal_Int16(SdrMeasureFieldKind::Unit) || mpImpl->mnInt16 == sal_Int16(SdrMeasureFieldKind::Rotate90Blanks) )
            eKind = static_cast<SdrMeasureFieldKind>(mpImpl->mnInt16);
        pData.reset( new SdrMeasureField( eKind) );
        break;
    }
    case text::textfield::Type::PRESENTATION_HEADER:
        pData.reset( new SvxHeaderField() );
        break;
    case text::textfield::Type::PRESENTATION_FOOTER:
        pData.reset( new SvxFooterField() );
        break;
    case text::textfield::Type::PRESENTATION_DATE_TIME:
        pData.reset( new SvxDateTimeField() );
        break;
    case text::textfield::Type::PAGE_NAME:
        pData.reset( new SvxPageTitleField() );
        break;
    case text::textfield::Type::DOCINFO_CUSTOM:
        pData.reset( new editeng::CustomPropertyField(mpImpl->msString1, mpImpl->msString2) );
        break;
    };

    return pData;
}

// uno::XInterface
uno::Any SAL_CALL SvxUnoTextField::queryAggregation( const uno::Type & rType )
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
{
    if( maTypeSequence.getLength() == 0 )
    {
        maTypeSequence = comphelper::concatSequences(
            OComponentHelper::getTypes(),
            uno::Sequence {
                cppu::UnoType<text::XTextField>::get(),
                cppu::UnoType<beans::XPropertySet>::get(),
                cppu::UnoType<lang::XServiceInfo>::get(),
                cppu::UnoType<lang::XUnoTunnel>::get() });
    }
    return maTypeSequence;
}

uno::Sequence< sal_Int8 > SAL_CALL SvxUnoTextField::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

uno::Any SAL_CALL SvxUnoTextField::queryInterface( const uno::Type & rType )
{
    return OComponentHelper::queryInterface(rType);
}

void SAL_CALL SvxUnoTextField::acquire() throw( )
{
    OComponentHelper::acquire();
}

void SAL_CALL SvxUnoTextField::release() throw( )
{
    OComponentHelper::release();
}

// Interface text::XTextField
OUString SAL_CALL SvxUnoTextField::getPresentation( sal_Bool bShowCommand )
{
    SolarMutexGuard aGuard;
    if (bShowCommand)
    {
        switch (mnServiceId)
        {
            case text::textfield::Type::DATE:
                return OUString("Date");
            case text::textfield::Type::URL:
                return OUString("URL");
            case text::textfield::Type::PAGE:
                return OUString("Page");
            case text::textfield::Type::PAGES:
                return OUString("Pages");
            case text::textfield::Type::TIME:
                return OUString("Time");
            case text::textfield::Type::DOCINFO_TITLE:
                return OUString("File");
            case text::textfield::Type::TABLE:
                return OUString("Table");
            case text::textfield::Type::EXTENDED_TIME:
                return OUString("ExtTime");
            case text::textfield::Type::EXTENDED_FILE:
                return OUString("ExtFile");
            case text::textfield::Type::AUTHOR:
                return OUString("Author");
            case text::textfield::Type::MEASURE:
                return OUString("Measure");
            case text::textfield::Type::PRESENTATION_HEADER:
                return OUString("Header");
            case text::textfield::Type::PRESENTATION_FOOTER:
                return OUString("Footer");
            case text::textfield::Type::PRESENTATION_DATE_TIME:
                return OUString("DateTime");
            case text::textfield::Type::PAGE_NAME:
                return OUString("PageName");
            case text::textfield::Type::DOCINFO_CUSTOM:
                return OUString("Custom");
            default:
                return OUString("Unknown");
        }
    }
    else
    {
        return mpImpl->msPresentation;
    }
}

// Interface text::XTextContent
void SAL_CALL SvxUnoTextField::attach( const uno::Reference< text::XTextRange >& xTextRange )
{
    SvxUnoTextRangeBase* pRange = SvxUnoTextRange::getImplementation( xTextRange );
    if(pRange == nullptr)
        throw lang::IllegalArgumentException();

    std::unique_ptr<SvxFieldData> pData = CreateFieldData();
    if( pData )
        pRange->attachField( std::move(pData) );
}

uno::Reference< text::XTextRange > SAL_CALL SvxUnoTextField::getAnchor()
{
    return mxAnchor;
}

// lang::XComponent
void SAL_CALL SvxUnoTextField::dispose()
{
    OComponentHelper::dispose();
}

void SAL_CALL SvxUnoTextField::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
{
    OComponentHelper::addEventListener(xListener);
}

void SAL_CALL SvxUnoTextField::removeEventListener( const uno::Reference< lang::XEventListener >& aListener )
{
    OComponentHelper::removeEventListener(aListener);
}


// Interface beans::XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL SvxUnoTextField::getPropertySetInfo(  )
{
    SolarMutexGuard aGuard;
    return mpPropSet->getPropertySetInfo();
}

void SAL_CALL SvxUnoTextField::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
{
    SolarMutexGuard aGuard;

    if( mpImpl == nullptr )
        throw uno::RuntimeException();

    if (aPropertyName == UNO_TC_PROP_ANCHOR)
    {
        aValue >>= mxAnchor;
        return;
    }

    const SfxItemPropertySimpleEntry* pMap = mpPropSet->getPropertyMap().getByName( aPropertyName );
    if ( !pMap )
        throw beans::UnknownPropertyException();

    switch( pMap->nWID )
    {
    case WID_DATE:
        if(aValue >>= mpImpl->maDateTime)
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
    case WID_INT16:
        if(aValue >>= mpImpl->mnInt16)
            return;
        break;
    case WID_INT32:
        if(aValue >>= mpImpl->mnInt32)
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
}

uno::Any SAL_CALL SvxUnoTextField::getPropertyValue( const OUString& PropertyName )
{
    SolarMutexGuard aGuard;

    if (PropertyName == UNO_TC_PROP_ANCHOR)
        return uno::makeAny(mxAnchor);

    if (PropertyName == UNO_TC_PROP_TEXTFIELD_TYPE)
        return uno::makeAny(mnServiceId);

    uno::Any aValue;

    const SfxItemPropertySimpleEntry* pMap = mpPropSet->getPropertyMap().getByName( PropertyName );
    if ( !pMap )
        throw beans::UnknownPropertyException();

    switch( pMap->nWID )
    {
    case WID_DATE:
        aValue <<= mpImpl->maDateTime;
        break;
    case WID_BOOL1:
        aValue <<= mpImpl->mbBoolean1;
        break;
    case WID_BOOL2:
        aValue <<= mpImpl->mbBoolean2;
        break;
    case WID_INT16:
        aValue <<= mpImpl->mnInt16;
        break;
    case WID_INT32:
        aValue <<= mpImpl->mnInt32;
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
}

void SAL_CALL SvxUnoTextField::addPropertyChangeListener( const OUString&, const uno::Reference< beans::XPropertyChangeListener >& ) {}
void SAL_CALL SvxUnoTextField::removePropertyChangeListener( const OUString&, const uno::Reference< beans::XPropertyChangeListener >& ) {}
void SAL_CALL SvxUnoTextField::addVetoableChangeListener( const OUString&, const uno::Reference< beans::XVetoableChangeListener >& ) {}
void SAL_CALL SvxUnoTextField::removeVetoableChangeListener( const OUString&, const uno::Reference< beans::XVetoableChangeListener >& ) {}

// OComponentHelper
void SvxUnoTextField::disposing()
{
    // nothing to do
}

// lang::XServiceInfo
OUString SAL_CALL SvxUnoTextField::getImplementationName()
{
    return OUString("SvxUnoTextField");
}

uno::Sequence< OUString > SAL_CALL SvxUnoTextField::getSupportedServiceNames()
{
    uno::Sequence<OUString> aSeq(4);
    OUString* pServices = aSeq.getArray();
    pServices[0] = "com.sun.star.text.TextContent";
    pServices[1] = "com.sun.star.text.TextField";

    switch (mnServiceId)
    {
        case text::textfield::Type::DATE:
            pServices[2] = "com.sun.star.text.TextField.DateTime";
            pServices[3] = "com.sun.star.text.textfield.DateTime";
        break;
        case text::textfield::Type::URL:
            pServices[2] = "com.sun.star.text.TextField.URL";
            pServices[3] = "com.sun.star.text.textfield.URL";
        break;
        case text::textfield::Type::PAGE:
            pServices[2] = "com.sun.star.text.TextField.PageNumber";
            pServices[3] = "com.sun.star.text.textfield.PageNumber";
        break;
        case text::textfield::Type::PAGES:
            pServices[2] = "com.sun.star.text.TextField.PageCount";
            pServices[3] = "com.sun.star.text.textfield.PageCount";
        break;
        case text::textfield::Type::TIME:
            pServices[2] = "com.sun.star.text.TextField.DateTime";
            pServices[3] = "com.sun.star.text.textfield.DateTime";
        break;
        case text::textfield::Type::DOCINFO_TITLE:
            pServices[2] = "com.sun.star.text.TextField.docinfo.Title";
            pServices[3] = "com.sun.star.text.textfield.docinfo.Title";
        break;
        case text::textfield::Type::TABLE:
            pServices[2] = "com.sun.star.text.TextField.SheetName";
            pServices[3] = "com.sun.star.text.textfield.SheetName";
        break;
        case text::textfield::Type::EXTENDED_TIME:
            pServices[2] = "com.sun.star.text.TextField.DateTime";
            pServices[3] = "com.sun.star.text.textfield.DateTime";
        break;
        case text::textfield::Type::EXTENDED_FILE:
            pServices[2] = "com.sun.star.text.TextField.FileName";
            pServices[3] = "com.sun.star.text.textfield.FileName";
        break;
        case text::textfield::Type::AUTHOR:
            pServices[2] = "com.sun.star.text.TextField.Author";
            pServices[3] = "com.sun.star.text.textfield.Author";
        break;
        case text::textfield::Type::MEASURE:
            pServices[2] = "com.sun.star.text.TextField.Measure";
            pServices[3] = "com.sun.star.text.textfield.Measure";
        break;
        case text::textfield::Type::PRESENTATION_HEADER:
            pServices[2] = "com.sun.star.presentation.TextField.Header";
            pServices[3] = "com.sun.star.presentation.textfield.Header";
        break;
        case text::textfield::Type::PRESENTATION_FOOTER:
            pServices[2] = "com.sun.star.presentation.TextField.Footer";
            pServices[3] = "com.sun.star.presentation.textfield.Footer";
        break;
        case text::textfield::Type::PRESENTATION_DATE_TIME:
            pServices[2] = "com.sun.star.presentation.TextField.DateTime";
            pServices[3] = "com.sun.star.presentation.textfield.DateTime";
        break;
        case text::textfield::Type::PAGE_NAME:
            pServices[2] = "com.sun.star.text.TextField.PageName";
            pServices[3] = "com.sun.star.text.textfield.PageName";
        break;
        case text::textfield::Type::DOCINFO_CUSTOM:
            pServices[2] = "com.sun.star.text.TextField.DocInfo.Custom";
            pServices[3] = "com.sun.star.text.textfield.DocInfo.Custom";
        break;
        default:
            aSeq.realloc(0);
    }

    return aSeq;
}

sal_Bool SAL_CALL SvxUnoTextField::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

uno::Reference< uno::XInterface > SvxUnoTextCreateTextField( const OUString& ServiceSpecifier )
{
    uno::Reference< uno::XInterface > xRet;

    const OUString aTextFieldPrexit( "com.sun.star.text.textfield." );

    // #i93308# up to OOo 3.2 we used this wrong namespace name with the capital T & F. This is
    // fixed since OOo 3.2 but for compatibility we will still provide support for the wrong notation.

    if( (ServiceSpecifier.startsWith( aTextFieldPrexit )) ||
        (ServiceSpecifier.startsWith( "com.sun.star.text.TextField." )) )
    {
        OUString aFieldType( ServiceSpecifier.copy( aTextFieldPrexit.getLength() ) );

        sal_Int32 nId = text::textfield::Type::UNSPECIFIED;

        if ( aFieldType == "DateTime" )
        {
            nId = text::textfield::Type::DATE;
        }
        else if ( aFieldType == "URL" )
        {
            nId = text::textfield::Type::URL;
        }
        else if ( aFieldType == "PageNumber" )
        {
            nId = text::textfield::Type::PAGE;
        }
        else if ( aFieldType == "PageCount" )
        {
            nId = text::textfield::Type::PAGES;
        }
        else if ( aFieldType == "SheetName" )
        {
            nId = text::textfield::Type::TABLE;
        }
        else if ( aFieldType == "FileName" )
        {
            nId = text::textfield::Type::EXTENDED_FILE;
        }
        else if (aFieldType == "docinfo.Title" ||
                 aFieldType == "DocInfo.Title" )
        {
            nId = text::textfield::Type::DOCINFO_TITLE;
        }
        else if ( aFieldType == "Author" )
        {
            nId = text::textfield::Type::AUTHOR;
        }
        else if ( aFieldType == "Measure" )
        {
            nId = text::textfield::Type::MEASURE;
        }
        else if (aFieldType == "DocInfo.Custom")
        {
            nId = text::textfield::Type::DOCINFO_CUSTOM;
        }

        if (nId != text::textfield::Type::UNSPECIFIED)
            xRet = static_cast<cppu::OWeakObject *>(new SvxUnoTextField( nId ));
    }

    return xRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
