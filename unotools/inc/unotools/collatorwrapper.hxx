/*************************************************************************
 *
 *  $RCSfile: collatorwrapper.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: cp $ $Date: 2001-03-02 17:28:58 $
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

#ifndef _UNOTOOLS_COLLATORWRAPPER_HXX
#define _UNOTOOLS_COLLATORWRAPPER_HXX

#ifndef _COM_SUN_STAR_UTIL_XCOLLATOR_HPP_
#include <com/sun/star/i18n/XCollator.hpp>
#endif

namespace com { namespace sun { namespace star { namespace lang {
        class XMultiServiceFactory;
}}}}

class CollatorWrapper
{
    private:

        ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XMultiServiceFactory
            > mxServiceFactory;

        ::com::sun::star::uno::Reference<
                ::com::sun::star::i18n::XCollator
            > mxInternationalCollator;

    public:

        CollatorWrapper (
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::lang::XMultiServiceFactory > &xServiceFactoryIn);

        ~CollatorWrapper();

        sal_Int32
        compareSubstring (
                const ::rtl::OUString& s1, sal_Int32 off1, sal_Int32 len1,
                const ::rtl::OUString& s2, sal_Int32 off2, sal_Int32 len2);

           sal_Int32
        compareString (
                const ::rtl::OUString& s1, const ::rtl::OUString& s2);

        ::com::sun::star::uno::Sequence< ::rtl::OUString >
        listCollatorAlgorithms (
                const ::com::sun::star::lang::Locale& rLocale);

        ::com::sun::star::uno::Sequence< sal_Int32 >
        listCollatorOptions (
                const ::rtl::OUString& rAlgorithm);

        sal_Int32
        loadDefaultCollator (
                const ::com::sun::star::lang::Locale& rLocale, sal_Int32 nOption);

        sal_Int32
        loadCollatorAlgorithm (
                const ::rtl::OUString& rAlgorithm,
                const ::com::sun::star::lang::Locale& rLocale, sal_Int32 nOption);

        void
        loadCollatorAlgorithmWithEndUserOption (
                const ::rtl::OUString& rAlgorithm,
                const ::com::sun::star::lang::Locale& rLocale,
                const ::com::sun::star::uno::Sequence< sal_Int32 >& rOption);

    protected:

        CollatorWrapper();
};

#endif /* _UNOTOOLS_COLLATORWRAPPER_HXX */
