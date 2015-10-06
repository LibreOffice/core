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

#include "cellvalueconversion.hxx"

#include <com/sun/star/util/NumberFormatter.hpp>
#include <com/sun/star/util/NumberFormatsSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <rtl/strbuf.hxx>
#include <rtl/math.hxx>
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/syslocale.hxx>
#include <comphelper/processfactory.hxx>

#include <memory>
#include <unordered_map>

namespace svt
{


    using namespace ::com::sun::star::uno;
    using ::com::sun::star::util::XNumberFormatter;
    using ::com::sun::star::util::XNumberFormatter2;
    using ::com::sun::star::util::NumberFormatter;
    using ::com::sun::star::util::XNumberFormatsSupplier;
    using ::com::sun::star::util::NumberFormatsSupplier;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::lang::Locale;
    using ::com::sun::star::util::DateTime;
    using ::com::sun::star::util::XNumberFormatTypes;

    namespace NumberFormat = ::com::sun::star::util::NumberFormat;

    typedef ::com::sun::star::util::Time UnoTime;
    typedef ::com::sun::star::util::Date UnoDate;


    //= helper

    namespace
    {

        double lcl_convertDateToDays( long const i_day, long const i_month, long const i_year )
        {
            long const nNullDateDays = ::Date::DateToDays( 1, 1, 1900 );
            long const nValueDateDays = ::Date::DateToDays( i_day, i_month, i_year );

            return nValueDateDays - nNullDateDays;
        }


        double lcl_convertTimeToDays( long const i_hours, long const i_minutes, long const i_seconds, long const i_100thSeconds )
        {
            return tools::Time( i_hours, i_minutes, i_seconds, i_100thSeconds ).GetTimeInDays();
        }
    }


    //= CellValueConversion_Data
    class StandardFormatNormalizer;
    struct CellValueConversion_Data
    {
        typedef std::unordered_map< OUString, std::shared_ptr< StandardFormatNormalizer >, OUStringHash >    NormalizerCache;

        Reference< XNumberFormatter >           xNumberFormatter;
        bool                                    bAttemptedFormatterCreation;
        NormalizerCache                         aNormalizers;

        CellValueConversion_Data()
            :xNumberFormatter()
            ,bAttemptedFormatterCreation( false )
            ,aNormalizers()
        {
        }
    };


    //= StandardFormatNormalizer

    class StandardFormatNormalizer
    {
    public:
        /** converts the given <code>Any</code> into a <code>double</code> value to be fed into a number formatter
        */
        virtual double convertToDouble( Any const & i_value ) const = 0;

        /** returns the format key to be used for formatting values
        */
        sal_Int32 getFormatKey() const
        {
            return m_nFormatKey;
        }

    protected:
        StandardFormatNormalizer( Reference< XNumberFormatter > const & i_formatter, ::sal_Int32 const i_numberFormatType )
            :m_nFormatKey( 0 )
        {
            try
            {
                ENSURE_OR_THROW( i_formatter.is(), "StandardFormatNormalizer: no formatter!" );
                Reference< XNumberFormatsSupplier > const xSupplier( i_formatter->getNumberFormatsSupplier(), UNO_SET_THROW );
                Reference< XNumberFormatTypes > const xTypes( xSupplier->getNumberFormats(), UNO_QUERY_THROW );
                m_nFormatKey = xTypes->getStandardFormat( i_numberFormatType, SvtSysLocale().GetLanguageTag().getLocale() );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }

        virtual ~StandardFormatNormalizer() {}

    private:
        ::sal_Int32 m_nFormatKey;
    };


    //= DoubleNormalization

    class DoubleNormalization : public StandardFormatNormalizer
    {
    public:
        explicit DoubleNormalization( Reference< XNumberFormatter > const & i_formatter )
            :StandardFormatNormalizer( i_formatter, NumberFormat::NUMBER )
        {
        }

        virtual double convertToDouble( Any const & i_value ) const SAL_OVERRIDE
        {
            double returnValue(0);
            ::rtl::math::setNan( &returnValue );
            OSL_VERIFY( i_value >>= returnValue );
            return returnValue;
        }

        virtual ~DoubleNormalization() { }
    };


    //= IntegerNormalization

    class IntegerNormalization : public StandardFormatNormalizer
    {
    public:
        explicit IntegerNormalization( Reference< XNumberFormatter > const & i_formatter )
            :StandardFormatNormalizer( i_formatter, NumberFormat::NUMBER )
        {
        }

        virtual ~IntegerNormalization() {}

        virtual double convertToDouble( Any const & i_value ) const SAL_OVERRIDE
        {
            sal_Int64 value( 0 );
            OSL_VERIFY( i_value >>= value );
            return value;
        }
    };


    //= BooleanNormalization

    class BooleanNormalization : public StandardFormatNormalizer
    {
    public:
        explicit BooleanNormalization( Reference< XNumberFormatter > const & i_formatter )
            :StandardFormatNormalizer( i_formatter, NumberFormat::LOGICAL )
        {
        }

        virtual ~BooleanNormalization() {}

        virtual double convertToDouble( Any const & i_value ) const SAL_OVERRIDE
        {
            bool value( false );
            OSL_VERIFY( i_value >>= value );
            return value ? 1 : 0;
        }
    };


    //= DateTimeNormalization

    class DateTimeNormalization : public StandardFormatNormalizer
    {
    public:
        explicit DateTimeNormalization( Reference< XNumberFormatter > const & i_formatter )
            :StandardFormatNormalizer( i_formatter, NumberFormat::DATETIME )
        {
        }

        virtual ~DateTimeNormalization() {}

        virtual double convertToDouble( Any const & i_value ) const SAL_OVERRIDE
        {
            double returnValue(0);
            ::rtl::math::setNan( &returnValue );

            // extract actual UNO value
            DateTime aDateTimeValue;
            ENSURE_OR_RETURN( i_value >>= aDateTimeValue, "allowed for DateTime values only", returnValue );

            // date part
            returnValue = lcl_convertDateToDays( aDateTimeValue.Day, aDateTimeValue.Month, aDateTimeValue.Year );

            // time part
            returnValue += lcl_convertTimeToDays(
                aDateTimeValue.Hours, aDateTimeValue.Minutes, aDateTimeValue.Seconds, aDateTimeValue.NanoSeconds );

            // done
            return returnValue;
        }
    };


    //= DateNormalization

    class DateNormalization : public StandardFormatNormalizer
    {
    public:
        explicit DateNormalization( Reference< XNumberFormatter > const & i_formatter )
            :StandardFormatNormalizer( i_formatter, NumberFormat::DATE )
        {
        }

        virtual ~DateNormalization() {}

        virtual double convertToDouble( Any const & i_value ) const SAL_OVERRIDE
        {
            double returnValue(0);
            ::rtl::math::setNan( &returnValue );

            // extract
            UnoDate aDateValue;
            ENSURE_OR_RETURN( i_value >>= aDateValue, "allowed for Date values only", returnValue );

            // convert
            returnValue = lcl_convertDateToDays( aDateValue.Day, aDateValue.Month, aDateValue.Year );

            // done
            return returnValue;
        }
    };


    //= TimeNormalization

    class TimeNormalization : public StandardFormatNormalizer
    {
    public:
        explicit TimeNormalization( Reference< XNumberFormatter > const & i_formatter )
            :StandardFormatNormalizer( i_formatter, NumberFormat::TIME )
        {
        }

        virtual ~TimeNormalization() {}

        virtual double convertToDouble( Any const & i_value ) const SAL_OVERRIDE
        {
            double returnValue(0);
            ::rtl::math::setNan( &returnValue );

            // extract
            UnoTime aTimeValue;
            ENSURE_OR_RETURN( i_value >>= aTimeValue, "allowed for tools::Time values only", returnValue );

            // convert
            returnValue += lcl_convertTimeToDays(
                aTimeValue.Hours, aTimeValue.Minutes, aTimeValue.Seconds, aTimeValue.NanoSeconds );

            // done
            return returnValue;
        }
    };


    //= operations

    namespace
    {

        bool lcl_ensureNumberFormatter( CellValueConversion_Data & io_data )
        {
            if ( io_data.bAttemptedFormatterCreation )
                return io_data.xNumberFormatter.is();
            io_data.bAttemptedFormatterCreation = true;

            try
            {
                Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
                // a number formatter
                Reference< XNumberFormatter > const xFormatter( NumberFormatter::create( xContext ), UNO_QUERY_THROW );

                // a supplier of number formats
                Locale aLocale = SvtSysLocale().GetLanguageTag().getLocale();

                Reference< XNumberFormatsSupplier > const xSupplier =
                    NumberFormatsSupplier::createWithLocale( xContext, aLocale );

                // ensure a NullDate we will assume later on
                UnoDate const aNullDate( 1, 1, 1900 );
                Reference< XPropertySet > const xFormatSettings( xSupplier->getNumberFormatSettings(), UNO_SET_THROW );
                xFormatSettings->setPropertyValue( "NullDate", makeAny( aNullDate ) );

                // knit
                xFormatter->attachNumberFormatsSupplier( xSupplier );

                // done
                io_data.xNumberFormatter = xFormatter;
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }

            return io_data.xNumberFormatter.is();
        }


        bool lcl_getValueNormalizer( CellValueConversion_Data & io_data, Type const & i_valueType,
            std::shared_ptr< StandardFormatNormalizer > & o_formatter )
        {
            CellValueConversion_Data::NormalizerCache::const_iterator pos = io_data.aNormalizers.find( i_valueType.getTypeName() );
            if ( pos == io_data.aNormalizers.end() )
            {
                // never encountered this type before
                o_formatter.reset();

                OUString const sTypeName( i_valueType.getTypeName() );
                TypeClass const eTypeClass = i_valueType.getTypeClass();

                if ( sTypeName.equals( ::cppu::UnoType< DateTime >::get().getTypeName() ) )
                {
                    o_formatter.reset( new DateTimeNormalization( io_data.xNumberFormatter ) );
                }
                else if ( sTypeName.equals( ::cppu::UnoType< UnoDate >::get().getTypeName() ) )
                {
                    o_formatter.reset( new DateNormalization( io_data.xNumberFormatter ) );
                }
                else if ( sTypeName.equals( ::cppu::UnoType< UnoTime >::get().getTypeName() ) )
                {
                    o_formatter.reset( new TimeNormalization( io_data.xNumberFormatter ) );
                }
                else if ( sTypeName.equals( ::cppu::UnoType< sal_Bool >::get().getTypeName() ) )
                {
                    o_formatter.reset( new BooleanNormalization( io_data.xNumberFormatter ) );
                }
                else if (   sTypeName.equals( ::cppu::UnoType< double >::get().getTypeName() )
                        ||  sTypeName.equals( ::cppu::UnoType< float >::get().getTypeName() )
                        )
                {
                    o_formatter.reset( new DoubleNormalization( io_data.xNumberFormatter ) );
                }
                else if (   ( eTypeClass == TypeClass_BYTE )
                        ||  ( eTypeClass == TypeClass_SHORT )
                        ||  ( eTypeClass == TypeClass_UNSIGNED_SHORT )
                        ||  ( eTypeClass == TypeClass_LONG )
                        ||  ( eTypeClass == TypeClass_UNSIGNED_LONG )
                        ||  ( eTypeClass == TypeClass_HYPER )
                        )
                {
                    o_formatter.reset( new IntegerNormalization( io_data.xNumberFormatter ) );
                }
                else
                {
                    SAL_WARN( "svtools.table", "unsupported type '" << sTypeName << "'!" );
                }
                io_data.aNormalizers[ sTypeName ] = o_formatter;
            }
            else
                o_formatter = pos->second;

            return !!o_formatter;
        }
    }


    //= CellValueConversion


    CellValueConversion::CellValueConversion()
        :m_pData( new CellValueConversion_Data )
    {
    }


    CellValueConversion::~CellValueConversion()
    {
    }


    OUString CellValueConversion::convertToString( const Any& i_value )
    {
        OUString sStringValue;
        if ( !i_value.hasValue() )
            return sStringValue;

        if ( ! ( i_value >>= sStringValue ) )
        {
            if ( lcl_ensureNumberFormatter( *m_pData ) )
            {
                std::shared_ptr< StandardFormatNormalizer > pNormalizer;
                if ( lcl_getValueNormalizer( *m_pData, i_value.getValueType(), pNormalizer ) )
                {
                    try
                    {
                        double const formatterCompliantValue = pNormalizer->convertToDouble( i_value );
                        sal_Int32 const formatKey = pNormalizer->getFormatKey();
                        sStringValue = m_pData->xNumberFormatter->convertNumberToString(
                            formatKey, formatterCompliantValue );
                    }
                    catch( const Exception& )
                    {
                        DBG_UNHANDLED_EXCEPTION();
                    }
                }
            }
        }

        return sStringValue;
    }


} // namespace svt


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
