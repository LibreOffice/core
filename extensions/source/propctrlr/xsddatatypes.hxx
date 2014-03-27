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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_XSDDATATYPES_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_XSDDATATYPES_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <rtl/ref.hxx>

namespace com { namespace sun { namespace star {
    namespace xsd {
        class XDataType;
    }
    namespace beans {
        class XPropertySetInfo;
    }
} } }


namespace pcr
{



    //= XSDDataType

    class XSDDataType : public ::rtl::IReference
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::xsd::XDataType >
                            m_xDataType;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            m_xFacetInfo;

    protected:
        oslInterlockedCount m_refCount;

    protected:
        inline ::com::sun::star::xsd::XDataType* getDataTypeInterface() const { return m_xDataType.get(); }

    public:
        XSDDataType(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xsd::XDataType >& _rxDataType
        );

        // IReference
        virtual oslInterlockedCount SAL_CALL acquire() SAL_OVERRIDE;
        virtual oslInterlockedCount SAL_CALL release() SAL_OVERRIDE;

        /// retrieves the underlying UNO component
        inline const ::com::sun::star::uno::Reference< ::com::sun::star::xsd::XDataType >&
                getUnoDataType() const { return m_xDataType; }

        /// classifies the data typ
        sal_Int16 classify() const SAL_THROW(());

        // attribute access
        OUString     getName() const SAL_THROW(());
        bool                isBasicType() const SAL_THROW(());

        /// determines whether a given facet exists at the type
        bool                hasFacet( const OUString& _rFacetName ) const SAL_THROW(());
        /// retrieves a facet value
        ::com::sun::star::uno::Any
                            getFacet( const OUString& _rFacetName ) SAL_THROW(());
        /// sets a facet value
        void                setFacet( const OUString& _rFacetName, const ::com::sun::star::uno::Any& _rFacetValue ) SAL_THROW(());

       /** copies as much facets (values, respectively) from a give data type instance
       */
       void             copyFacetsFrom( const ::rtl::Reference< XSDDataType >& _pSourceType );

    protected:
        virtual ~XSDDataType();

    private:
        XSDDataType();                                  // never implemented
        XSDDataType( const XSDDataType& );              // never implemented
        XSDDataType& operator=( const XSDDataType& );   // never implemented
    };


} // namespace pcr


#endif // EXTENSIONS_SOURCE_PROPCTRLR_XSDDATATYPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
