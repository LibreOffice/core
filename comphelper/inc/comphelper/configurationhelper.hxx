/*************************************************************************
 *
 *  $RCSfile: configurationhelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-17 13:39:33 $
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

#ifndef _COMPHELPER_CONFIGURATIONHELPER_HXX_
#define _COMPHELPER_CONFIGURATIONHELPER_HXX_

//_______________________________________________
// includes

#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XCHANGESBATCH_HPP_
#include <com/sun/star/util/XChangesBatch.hpp>
#endif

#ifndef _COMPHELPER_SEQUENCEASVECTOR_HXX_
#include <comphelper/sequenceasvector.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

//_______________________________________________
// namespaces

#ifdef css
#error "Who use css? I need it as namespace alias."
#else
#define css ::com::sun::star
#endif

namespace comphelper{

//_______________________________________________
// definitions

//-----------------------------------------------
class ConfigurationHelper
{
    public:

    enum EConfigurationModes
    {
        E_STANDARD = 0,
        E_READONLY = 1,
        E_ALL_LOCALES = 2,
        E_LAZY_WRITE = 4
    };

    //-----------------------------------------------
    static css::uno::Reference< css::uno::XInterface > openConfig(const css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR   ,
                                                                  const ::rtl::OUString&                                       sPackage,
                                                                        sal_Int32                                              eMode   );

    //-----------------------------------------------
    static css::uno::Any readRelativeKey(const css::uno::Reference< css::uno::XInterface > xCFG    ,
                                         const ::rtl::OUString&                            sRelPath,
                                         const ::rtl::OUString&                            sKey    );

    //-----------------------------------------------
    static void writeRelativeKey(const css::uno::Reference< css::uno::XInterface > xCFG    ,
                                 const ::rtl::OUString&                            sRelPath,
                                 const ::rtl::OUString&                            sKey    ,
                                 const css::uno::Any&                              aValue  );

    //-----------------------------------------------
    static css::uno::Any readDirectKey(const css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR   ,
                                       const ::rtl::OUString&                                       sPackage,
                                       const ::rtl::OUString&                                       sRelPath,
                                       const ::rtl::OUString&                                       sKey    ,
                                             sal_Int32                                              eMode   );

    //-----------------------------------------------
    static void writeDirectKey(const css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR   ,
                               const ::rtl::OUString&                                       sPackage,
                               const ::rtl::OUString&                                       sRelPath,
                               const ::rtl::OUString&                                       sKey    ,
                               const css::uno::Any&                                         aValue  ,
                                     sal_Int32                                              eMode   );

    //-----------------------------------------------
    static void flush(const css::uno::Reference< css::uno::XInterface >& xCFG);
};

} // namespace comphelper

#undef css // important!

#endif // _COMPHELPER_CONFIGURATIONHELPER_HXX_
