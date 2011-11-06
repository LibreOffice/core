/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

