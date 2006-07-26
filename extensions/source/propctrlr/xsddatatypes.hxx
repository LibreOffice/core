/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xsddatatypes.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-26 08:02:08 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_XSDDATATYPES_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_XSDDATATYPES_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
/** === end UNO includes === **/

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

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
        /// determines the UNO type of a facet
        ::com::sun::star::uno::Type
                            getFacetType( const ::rtl::OUString& _rFacetName ) const SAL_THROW(());
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

