/*************************************************************************
 *
 *  $RCSfile: xsdvalidationhelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 12:14:27 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_XSDVALIDATIONHELPER_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_XSDVALIDATIONHELPER_HXX

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_EFORMSHELPER_HXX
#include "eformshelper.hxx"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_XSDDATATYPES_HXX
#include "xsddatatypes.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_XSD_XDATATYPE_HPP_
#include <com/sun/star/xsd/XDataType.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
/** === end UNO includes === **/

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

//........................................................................
namespace pcr
{
//........................................................................

    class XSDDataType;
    //====================================================================
    //= XSDValidationHelper
    //====================================================================
    class XSDValidationHelper : public EFormsHelper
    {
    public:
        XSDValidationHelper(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxIntrospectee,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _rxContextDocument
        );

        /** retrieves the names of all XForms models in the document the control lives in
        */
        void    getAvailableDataTypeNames( ::std::vector< ::rtl::OUString >& /* [out] */ _rNames ) const SAL_THROW(());

        /** retrieves a particular data type given by name
        */
        ::rtl::Reference< XSDDataType >
                getDataTypeByName( const ::rtl::OUString& _rName ) const SAL_THROW(());

        /** retrieves the DataType instance which the control model is currently validated against

            If there is a binding set at our control model, which at the same time acts as validator,
            and if this validator is bound to an XDataType, then this data type is retrieved here.
        */
        ::rtl::Reference< XSDDataType >
                getValidatingDataType( ) const SAL_THROW(());

        /** retrieves the name of the data type which the control model is currently validated against

            @seealso getValidatingDataType
        */
        ::rtl::OUString
                getValidatingDataTypeName( ) const SAL_THROW(());

        /** binds the validator to a new data type

            To be called with an active binding only.
        */
        void    setValidatingDataTypeByName( const ::rtl::OUString& _rName ) const SAL_THROW(());

        /** removes the data type given by name from the data type repository
        */
        bool    removeDataTypeFromRepository( const ::rtl::OUString& _rName ) const SAL_THROW(());

        /** creates a new data type, which is a clone of an existing data type
        */
        bool    cloneDataType( const ::rtl::Reference< XSDDataType >& _pDataType, const ::rtl::OUString& _rNewName ) const SAL_THROW(());

        /** retrieves the name of the basic data type which has the given class
        */
        ::rtl::OUString
                getBasicTypeNameForClass( sal_Int16 _eClass ) const SAL_THROW(());

        /** copy a data type from one model to another

            If a data type with the given name already exists in the target model, then nothing
            happens. In particular, the facets of the data type are not copied.
        */
        void    copyDataType( const ::rtl::OUString& _rFromModel, const ::rtl::OUString& _rToModel,
                    const ::rtl::OUString& _rDataTypeName ) const SAL_THROW(());
    private:
        /** retrieves the data type repository associated with the current model
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::xforms::XDataTypeRepository >
                getDataTypeRepository() const SAL_THROW((::com::sun::star::uno::Exception));

        /** retrieves the data type repository associated with any model
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::xforms::XDataTypeRepository >
                getDataTypeRepository( const ::rtl::OUString& _rModelName ) const SAL_THROW((::com::sun::star::uno::Exception));

        /** retrieves the data type object for the given name
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::xsd::XDataType >
            getDataType( const ::rtl::OUString& _rName ) const
                SAL_THROW((::com::sun::star::uno::Exception));

        /** retrieves the name of the basic data type which has the given class, in the given repository
        */
        ::rtl::OUString
                getBasicTypeNameForClass(
                    sal_Int16 _nClass,
                    ::com::sun::star::uno::Reference< ::com::sun::star::xforms::XDataTypeRepository > _rxRepository
                ) const SAL_THROW(());
    };

//........................................................................
} // namespace pcr
//........................................................................

#endif // EXTENSIONS_SOURCE_PROPCTRLR_XSDVALIDATIONHELPER_HXX

