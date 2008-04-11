/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: formcontrolfont.hxx,v $
 * $Revision: 1.5 $
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

#ifndef FORMS_SOURCE_COMPONENT_FORMCONTROLFONT_HXX
#define FORMS_SOURCE_COMPONENT_FORMCONTROLFONT_HXX

#include <com/sun/star/io/XObjectOutputStream.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

//.........................................................................
namespace frm
{
//.........................................................................

    //=====================================================================
    //= FontControlModel
    //=====================================================================
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
        void     setFastPropertyValue_NoBroadcast( sal_Int32 _nHandle, const ::com::sun::star::uno::Any& _rValue ) throw ( ::com::sun::star::uno::Exception );
        ::com::sun::star::uno::Any
                 getPropertyDefaultByHandle      ( sal_Int32 _nHandle ) const;

    private:

    private:
        FontControlModel();                                     // never implemented
        FontControlModel( const FontControlModel& );            // never implemented
        FontControlModel& operator=( const FontControlModel& ); // never implemented
    };

//.........................................................................
}   // namespace frm
//.........................................................................

#endif // FORMS_SOURCE_COMPONENT_FORMCONTROLFONT_HXX
