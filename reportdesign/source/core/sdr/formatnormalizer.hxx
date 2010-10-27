/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef REPORTDESIGN_FORMATNORMALIZER_HXX
#define REPORTDESIGN_FORMATNORMALIZER_HXX

/** === begin UNO includes === **/
#include <com/sun/star/report/XReportDefinition.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/report/XFormattedField.hpp>
/** === end UNO includes === **/

#include <boost/noncopyable.hpp>

#include <vector>

//........................................................................
namespace rptui
{
//........................................................................

    class OReportModel;
    //====================================================================
    //= FormatNormalizer
    //====================================================================
    class FormatNormalizer : public ::boost::noncopyable
    {
    public:
        struct Field
        {
            ::rtl::OUString sName;
            sal_Int32       nDataType;
            sal_Int32       nScale;
            sal_Bool        bIsCurrency;

            Field() : sName(), nDataType( 0 ), nScale( 0 ), bIsCurrency( 0 ) { }
        };
        typedef ::std::vector< Field >  FieldList;

    private:
        const OReportModel&                                                                 m_rModel;
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition >     m_xReportDefinition;

        /// is our field list dirty?
        FieldList                       m_aFields;
        bool                            m_bFieldListDirty;

    public:
        FormatNormalizer( const OReportModel& _rModel );
        ~FormatNormalizer();

        void    notifyPropertyChange( const ::com::sun::star::beans::PropertyChangeEvent& _rEvent );
        void    notifyElementInserted( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxElement );

    private:
        bool    impl_lateInit();

        void    impl_onDefinitionPropertyChange( const ::rtl::OUString& _rChangedPropName );
        void    impl_onFormattedProperttyChange( const ::com::sun::star::uno::Reference< ::com::sun::star::report::XFormattedField >& _rxFormatted, const ::rtl::OUString& _rChangedPropName );

        bool    impl_ensureUpToDateFieldList_nothrow();

        void    impl_adjustFormatToDataFieldType_nothrow( const ::com::sun::star::uno::Reference< ::com::sun::star::report::XFormattedField >& _rxFormatted );
    };

//........................................................................
} // namespace rptui
//........................................................................

#endif // REPORTDESIGN_FORMATNORMALIZER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
