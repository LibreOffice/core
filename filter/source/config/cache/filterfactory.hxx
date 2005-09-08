/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: filterfactory.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:29:22 $
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

#ifndef __FILTER_CONFIG_FILTERFACTORY_HXX_
#define __FILTER_CONFIG_FILTERFACTORY_HXX_

//_______________________________________________
// includes

#include "basecontainer.hxx"
#include "querytokenizer.hxx"

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

//_______________________________________________
// namespace

namespace filter{
    namespace config{

namespace css = ::com::sun::star;

//_______________________________________________
// definitions

//_______________________________________________

/** @short      implements the service <type scope="com.sun.star.document">FilterFactory</type>.
 */
class FilterFactory : public ::cppu::ImplInheritanceHelper1< BaseContainer                   ,
                                                             css::lang::XMultiServiceFactory >
{
    //-------------------------------------------
    // native interface

    public:

        //---------------------------------------
        // ctor/dtor

        /** @short  standard ctor to connect this interface wrapper to
                    the global filter cache instance ...

            @param  xSMGR
                    reference to the uno service manager, which created this service instance.
         */
        FilterFactory(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR);

        //---------------------------------------

        /** @short  standard dtor.
         */
        virtual ~FilterFactory();

    //-------------------------------------------
    // uno interface

    public:

        //---------------------------------------
        // XMultiServiceFactory

        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance(const ::rtl::OUString& sFilter)
            throw(css::uno::Exception       ,
                  css::uno::RuntimeException);

        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArguments(const ::rtl::OUString&                     sFilter   ,
                                                                                                 const css::uno::Sequence< css::uno::Any >& lArguments)
            throw(css::uno::Exception       ,
                  css::uno::RuntimeException);

        virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getAvailableServiceNames()
            throw(css::uno::RuntimeException);

        //---------------------------------------
        // XContainerQuery

        virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createSubSetEnumerationByQuery(const ::rtl::OUString& sQuery)
            throw (css::uno::RuntimeException);

    //-------------------------------------------
    // internal helper!

    private:

        //---------------------------------------

        /** @short  implement the container string query: "matchByDocumentService=:iflags=:eflags=:..."

            @param  lTokens
                    the list of query tokens and its values.

            @return A string list of internal filter names, including
                    all filters, which match this query.
         */
        OUStringList impl_queryMatchByDocumentService(const QueryTokenizer& lTokens) const;

        //---------------------------------------

        /** TODO document me
         */
        OUStringList impl_getListOfInstalledModules() const;

        //---------------------------------------

        /** @short  implement the container string query:
                    "getSortedFilterList()[:module=<xxx>]:[iflags=<xxx>][:eflags=<xxx>]"

            @param  lTokens
                    the list of query tokens and its values.

            @return A string list of internal filter names, including
                    all filters, which match this query.
         */
        OUStringList impl_getSortedFilterList(const QueryTokenizer& lTokens) const;

        //---------------------------------------

        /** TODO document me
         */
        OUStringList impl_getSortedFilterListForModule(const ::rtl::OUString& sModule,
                                                             sal_Int32        nIFlags,
                                                             sal_Int32        nEFlags) const;

        //---------------------------------------

        /** @short  read a specialized and sorted list of filter names from
                    the configuration (matching the specified module)

            @param  sModule
                    the module for which the sorted list should be retrieved for.

            @return A string list of internal filter names.
                    Can be empty.
         */
        OUStringList impl_readSortedFilterListFromConfig(const ::rtl::OUString& sModule) const;

    //-------------------------------------------
    // static uno helper!

    public:

        //---------------------------------------

        /** @short  return the uno implementation name of this class.

            @descr  Because this information is used at several places
                    (and mostly an object instance of this class is not possible)
                    its implemented as a static function!

            @return The fix uno implementation name of this class.
         */
        static ::rtl::OUString impl_getImplementationName();

        //---------------------------------------

        /** @short  return the list of supported uno services of this class.

            @descr  Because this information is used at several places
                    (and mostly an object instance of this class is not possible)
                    its implemented as a static function!

            @return The fix list of uno services supported by this class.
         */
        static css::uno::Sequence< ::rtl::OUString > impl_getSupportedServiceNames();

        //---------------------------------------

        /** @short  return a new intsnace of this class.

            @descr  This method is used by the uno service manager, to create
                    a new instance of this service if needed.

            @param  xSMGR
                    reference to the uno service manager, which require
                    this new instance. It should be passed to the new object
                    so it can be used internaly to create own needed uno resources.

            @return The new instance of this service as an uno reference.
         */
        static css::uno::Reference< css::uno::XInterface > impl_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR);
};

    } // namespace config
} // namespace filter

#endif // __FILTER_CONFIG_FILTERFACTORY_HXX_
