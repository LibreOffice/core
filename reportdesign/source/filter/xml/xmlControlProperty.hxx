#ifndef RPT_XMLCONTROLPROPERTY_HXX
#define RPT_XMLCONTROLPROPERTY_HXX
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlControlProperty.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:17 $
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

#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATE_HPP_
#include <com/sun/star/util/Date.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_TIME_HPP_
#include <com/sun/star/util/Time.hpp>
#endif

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
