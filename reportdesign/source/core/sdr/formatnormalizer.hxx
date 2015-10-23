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

#ifndef INCLUDED_REPORTDESIGN_SOURCE_CORE_SDR_FORMATNORMALIZER_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_CORE_SDR_FORMATNORMALIZER_HXX

#include <com/sun/star/report/XReportDefinition.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/report/XFormattedField.hpp>

#include <boost/noncopyable.hpp>

#include <vector>


namespace rptui
{


    class OReportModel;

    //= FormatNormalizer

    class FormatNormalizer : public ::boost::noncopyable
    {
    public:
        struct Field
        {
            OUString sName;
            sal_Int32       nDataType;
            sal_Int32       nScale;
            bool        bIsCurrency;

            Field() : sName(), nDataType( 0 ), nScale( 0 ), bIsCurrency( false ) { }
        };
        typedef ::std::vector< Field >  FieldList;

    private:
        const OReportModel&                                       m_rModel;
        css::uno::Reference< css::report::XReportDefinition >     m_xReportDefinition;

        /// is our field list dirty?
        FieldList                       m_aFields;
        bool                            m_bFieldListDirty;

    public:
        explicit FormatNormalizer( const OReportModel& _rModel );
        ~FormatNormalizer();

        void    notifyPropertyChange( const css::beans::PropertyChangeEvent& _rEvent );
        void    notifyElementInserted( const css::uno::Reference< css::uno::XInterface >& _rxElement );

    private:
        bool    impl_lateInit();

        void    impl_onDefinitionPropertyChange( const OUString& _rChangedPropName );
        void    impl_onFormattedProperttyChange( const css::uno::Reference< css::report::XFormattedField >& _rxFormatted, const OUString& _rChangedPropName );

        bool    impl_ensureUpToDateFieldList_nothrow();

        void    impl_adjustFormatToDataFieldType_nothrow( const css::uno::Reference< css::report::XFormattedField >& _rxFormatted );
    };


} // namespace rptui


#endif // INCLUDED_REPORTDESIGN_SOURCE_CORE_SDR_FORMATNORMALIZER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
