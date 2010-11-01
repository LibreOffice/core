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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_XSDDATATYPES_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_XSDDATATYPES_HXX

/** === begin UNO includes === **/
#include <com/sun/star/uno/Reference.hxx>
/** === end UNO includes === **/
#include <rtl/ref.hxx>

namespace com { namespace sun { namespace star {
    namespace xsd {
        class XDataType;
    }
    namespace beans {
        class XPropertySet;
        class XPropertySetInfo;
    }
} } }

//........................................................................
namespace pcr
{
//........................................................................

    //====================================================================
    //= XSDDataType
    //====================================================================
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
        virtual oslInterlockedCount SAL_CALL acquire();
        virtual oslInterlockedCount SAL_CALL release();

        /// retrieves the underlying UNO component
        inline const ::com::sun::star::uno::Reference< ::com::sun::star::xsd::XDataType >&
                getUnoDataType() const { return m_xDataType; }

        /// classifies the data typ
        sal_Int16 classify() const SAL_THROW(());

        // attribute access
        ::rtl::OUString     getName() const SAL_THROW(());
        bool                isBasicType() const SAL_THROW(());

        /// determines whether a given facet exists at the type
        bool                hasFacet( const ::rtl::OUString& _rFacetName ) const SAL_THROW(());
        /// retrieves a facet value
        ::com::sun::star::uno::Any
                            getFacet( const ::rtl::OUString& _rFacetName ) SAL_THROW(());
        /// sets a facet value
        void                setFacet( const ::rtl::OUString& _rFacetName, const ::com::sun::star::uno::Any& _rFacetValue ) SAL_THROW(());

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

//........................................................................
} // namespace pcr
//........................................................................

#endif // EXTENSIONS_SOURCE_PROPCTRLR_XSDDATATYPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
