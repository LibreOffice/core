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
#ifndef INCLUDED_REPORTDESIGN_SOURCE_FILTER_XML_XMLCONTROLPROPERTY_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_FILTER_XML_XMLCONTROLPROPERTY_HXX

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
        css::uno::Reference< css::beans::XPropertySet > m_xControl;
        css::beans::PropertyValue m_aSetting;
        css::uno::Sequence< css::uno::Any> m_aSequence;
        OXMLControlProperty* m_pContainer;
        css::uno::Type m_aPropType;            // the type of the property the instance imports currently
        bool m_bIsList;

        ORptFilter& GetOwnImport();
        static css::uno::Any convertString(const css::uno::Type& _rExpectedType, const OUString& _rReadCharacters);
        OXMLControlProperty(const OXMLControlProperty&) = delete;
        void operator =(const OXMLControlProperty&) = delete;
    public:

        OXMLControlProperty( ORptFilter& rImport
                    ,sal_uInt16 nPrfx
                    ,const OUString& rLName
                    ,const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList
                    ,const css::uno::Reference< css::beans::XPropertySet >& _xControl
                    ,OXMLControlProperty* _pContainer = nullptr);
        virtual ~OXMLControlProperty() override;

        virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                    const OUString& rLocalName,
                    const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList ) override;

        virtual void EndElement() override;

        virtual void Characters( const OUString& rChars ) override;


        /** adds value to property
            @param  _sValue
                The value to add.
        */
        void addValue(const OUString& _sValue);

    private:
        static css::util::Time implGetTime(double _nValue);
        static css::util::Date implGetDate(double _nValue);
    };

} // namespace rptxml


#endif // INCLUDED_REPORTDESIGN_SOURCE_FILTER_XML_XMLCONTROLPROPERTY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
