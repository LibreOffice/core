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

#ifndef INCLUDED_FORMS_SOURCE_INC_FORMCONTROLFONT_HXX
#define INCLUDED_FORMS_SOURCE_INC_FORMCONTROLFONT_HXX

#include <com/sun/star/io/XObjectOutputStream.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

namespace cppu {
    class OPropertySetHelper;
}


namespace frm
{



    //= FontControlModel

    class FontControlModel
    {
    private:
        // <properties>
        ::com::sun::star::awt::FontDescriptor   m_aFont;
        sal_Int16                               m_nFontRelief;
        sal_Int16                               m_nFontEmphasis;
        ::com::sun::star::uno::Any              m_aTextLineColor;
        ::com::sun::star::uno::Any              m_aTextColor;
        // </properties>

        bool                                    m_bToolkitCompatibleDefaults;

    protected:
        const ::com::sun::star::awt::FontDescriptor& getFont() const { return m_aFont; }
        void setFont( const ::com::sun::star::awt::FontDescriptor& _rFont ) { m_aFont = _rFont; }

        void        setTextColor( sal_Int32 _nColor ) { m_aTextColor <<= _nColor;       }
        void        clearTextColor( )                 { m_aTextColor.clear();           }
        bool        hasTextColor( ) const             { return m_aTextColor.hasValue(); }
        sal_Int32   getTextColor( ) const;

        void        setTextLineColor( sal_Int32 _nColor ) { m_aTextLineColor <<= _nColor;       }
        void        clearTextLineColor( )                 { m_aTextLineColor.clear();           }
        bool        hasTextLineColor( ) const             { return m_aTextLineColor.hasValue(); }
        sal_Int32   getTextLineColor( ) const;

    protected:
        FontControlModel( bool _bToolkitCompatibleDefaults );
        FontControlModel( const FontControlModel* _pOriginal );

    protected:
        bool     isFontRelatedProperty( sal_Int32 _nPropertyHandle ) const;
        bool     isFontAggregateProperty( sal_Int32 _nPropertyHandle ) const;

        /// appends (!) the description of all font related properties to the given sequence
        void     describeFontRelatedProperties(
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps ) const;

        void     getFastPropertyValue            ( ::com::sun::star::uno::Any& _rValue, sal_Int32 _nHandle ) const;
        sal_Bool convertFastPropertyValue        ( ::com::sun::star::uno::Any& _rConvertedValue, ::com::sun::star::uno::Any& _rOldValue, sal_Int32 _nHandle, const ::com::sun::star::uno::Any& _rValue ) throw( ::com::sun::star::lang::IllegalArgumentException );
        void     setFastPropertyValue_NoBroadcast_impl(
                ::cppu::OPropertySetHelper & rBase,
                void (::cppu::OPropertySetHelper::*pSet)(
                    sal_Int32, css::uno::Any const&),
                sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue)
            throw ( ::com::sun::star::uno::Exception );
        ::com::sun::star::uno::Any
                 getPropertyDefaultByHandle      ( sal_Int32 _nHandle ) const;

    private:

    private:
        FontControlModel();                                     // never implemented
        FontControlModel( const FontControlModel& );            // never implemented
        FontControlModel& operator=( const FontControlModel& ); // never implemented
    };


}   // namespace frm


#endif // INCLUDED_FORMS_SOURCE_INC_FORMCONTROLFONT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
