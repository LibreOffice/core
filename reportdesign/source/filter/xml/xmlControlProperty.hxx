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
#ifndef RPT_XMLCONTROLPROPERTY_HXX
#define RPT_XMLCONTROLPROPERTY_HXX

#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>

namespace rptxml
{
    class ORptFilter;
    class OXMLControlProperty : public SvXMLImportContext
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > m_xControl;
        ::com::sun::star::beans::PropertyValue m_aSetting;
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any> m_aSequence;
        OXMLControlProperty* m_pContainer;
        ::com::sun::star::uno::Type m_aPropType;            // the type of the property the instance imports currently
        sal_Bool m_bIsList;

        ORptFilter& GetOwnImport();
        ::com::sun::star::uno::Any convertString(const ::com::sun::star::uno::Type& _rExpectedType, const ::rtl::OUString& _rReadCharacters);
        OXMLControlProperty(const OXMLControlProperty&);
        void operator =(const OXMLControlProperty&);
    public:

        OXMLControlProperty( ORptFilter& rImport
                    ,sal_uInt16 nPrfx
                    ,const ::rtl::OUString& rLName
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _xControl
                    ,OXMLControlProperty* _pContainer = NULL);
        virtual ~OXMLControlProperty();

        virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                    const ::rtl::OUString& rLocalName,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

        virtual void EndElement();

        virtual void Characters( const ::rtl::OUString& rChars );


        /** adds value to property
            @param  _sValue
                The value to add.
        */
        void addValue(const ::rtl::OUString& _sValue);

    private:
        static ::com::sun::star::util::Time implGetTime(double _nValue);
        static ::com::sun::star::util::Date implGetDate(double _nValue);
    };
// -----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------

#endif // RPT_XMLCONTROLPROPERTY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
