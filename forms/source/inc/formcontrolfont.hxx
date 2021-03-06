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

#pragma once

#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <tools/color.hxx>

namespace cppu {
    class OPropertySetHelper;
}


namespace frm
{

    class FontControlModel
    {
    private:
        // <properties>
        css::awt::FontDescriptor                m_aFont;
        sal_Int16                               m_nFontRelief;
        sal_Int16                               m_nFontEmphasis;
        css::uno::Any                           m_aTextLineColor;
        css::uno::Any                           m_aTextColor;
        // </properties>

        bool                                    m_bToolkitCompatibleDefaults;

    protected:
        const css::awt::FontDescriptor& getFont() const { return m_aFont; }
        void setFont( const css::awt::FontDescriptor& _rFont ) { m_aFont = _rFont; }

        void        setTextColor( Color _nColor ) { m_aTextColor <<= _nColor;       }
        void        clearTextColor( )                 { m_aTextColor.clear();           }
        bool        hasTextColor( ) const             { return m_aTextColor.hasValue(); }
        Color       getTextColor( ) const;

        void        setTextLineColor( Color _nColor ) { m_aTextLineColor <<= _nColor;       }
        void        clearTextLineColor( )                 { m_aTextLineColor.clear();           }
        bool        hasTextLineColor( ) const             { return m_aTextLineColor.hasValue(); }
        Color       getTextLineColor( ) const;

    protected:
        FontControlModel( bool _bToolkitCompatibleDefaults );
        FontControlModel( const FontControlModel* _pOriginal );

    protected:
        static bool isFontRelatedProperty( sal_Int32 _nPropertyHandle );
        static bool isFontAggregateProperty( sal_Int32 _nPropertyHandle );

        /// appends (!) the description of all font related properties to the given sequence
        static void describeFontRelatedProperties(
            css::uno::Sequence< css::beans::Property >& /* [out] */ _rProps );

        void     getFastPropertyValue            ( css::uno::Any& _rValue, sal_Int32 _nHandle ) const;
        /// @throws css::lang::IllegalArgumentException
        /// @throws css::uno::RuntimeException
        bool     convertFastPropertyValue        ( css::uno::Any& _rConvertedValue, css::uno::Any& _rOldValue, sal_Int32 _nHandle, const css::uno::Any& _rValue );
        /// @throws css::uno::Exception
        void     setFastPropertyValue_NoBroadcast_impl(
                     ::cppu::OPropertySetHelper & rBase,
                     void (::cppu::OPropertySetHelper::*pSet)( sal_Int32, css::uno::Any const&),
                     sal_Int32 nHandle, const css::uno::Any& rValue);
        css::uno::Any
                 getPropertyDefaultByHandle      ( sal_Int32 _nHandle ) const;

    private:

    private:
        FontControlModel( const FontControlModel& ) = delete;
        FontControlModel& operator=( const FontControlModel& ) = delete;
    };


}   // namespace frm


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
