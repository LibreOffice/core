/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: officeresourcebundle.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-21 16:52:30 $
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

#ifndef COMPHELPER_OFFICE_RESOURCE_BUNDLE_HXX
#define COMPHELPER_OFFICE_RESOURCE_BUNDLE_HXX

#ifndef INCLUDED_COMPHELPERDLLAPI_H
#include <comphelper/comphelperdllapi.h>
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
/** === end UNO includes === **/

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#include <memory>

//........................................................................
namespace comphelper
{
//........................................................................

    //====================================================================
    //= OfficeResourceBundle
    //====================================================================
    class ResourceBundle_Impl;
    /** wraps the <type scope="com::sun::star::resource">OfficeResourceAccess</type> service
    */
    class COMPHELPER_DLLPUBLIC OfficeResourceBundle
    {
    private:
        ::std::auto_ptr< ResourceBundle_Impl >  m_pImpl;

    public:
        /** constructs a resource bundle
            @param  _context
                the component context to operate in
            @param  _bundleBaseName
                the base name of the resource file which should be accessed (*without* the SUPD!)
            @raises ::com::sun::star::lang::NullPointerException
                if the given component context is <NULL/>
        */
        OfficeResourceBundle(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _context,
            const ::rtl::OUString& _bundleBaseName
        );

        /** constructs a resource bundle with the resource bundle given as 8-bit ASCII name

            This is a convenience constructor only, it does nothing different than the constructor
            taking an unicode string.

            @param  _context
                the component context to operate in
            @param  _bundleBaseName
                the base name of the resource file which should be accessed (*without* the SUPD!)
            @raises ::com::sun::star::lang::NullPointerException
                if the given component context is <NULL/>
        */
        OfficeResourceBundle(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _context,
            const sal_Char* _bundleBaseAsciiName
        );

        /** destroys the instance
        */
        ~OfficeResourceBundle();

        /** loads the string with the given resource id from the resource bundle
            @param  _resourceId
                the id of the string to load
            @return
                the requested resource string. If no string with the given id exists in the resource bundle,
                an empty string is returned. In a non-product version, an OSL_ENSURE will notify you of this
                then.
        */
        ::rtl::OUString loadString( sal_Int32 _resourceId ) const;

        /** determines whether the resource bundle has a string with the given id
            @param  _resourceId
                the id of the string whose existence is to be checked
            @return
                <TRUE/> if and only if a string with the given ID exists in the resource
                bundle.
        */
        bool            hasString( sal_Int32 _resourceId ) const;
    };

//........................................................................
} // namespace comphelper
//........................................................................

#endif // COMPHELPER_OFFICE_RESOURCE_BUNDLE_HXX

