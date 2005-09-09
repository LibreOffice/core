/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: collatorwrapper.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:28:09 $
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
#ifndef INCLUDED_UNOTOOLSDLLAPI_H
#include "unotools/unotoolsdllapi.h"
#endif

#ifndef _UNOTOOLS_COLLATORWRAPPER_HXX
#define _UNOTOOLS_COLLATORWRAPPER_HXX

#ifndef _COM_SUN_STAR_UTIL_XCOLLATOR_HPP_
#include <com/sun/star/i18n/XCollator.hpp>
#endif

namespace com { namespace sun { namespace star { namespace lang {
        class XMultiServiceFactory;
}}}}

class UNOTOOLS_DLLPUBLIC CollatorWrapper
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
                const ::rtl::OUString& s2, sal_Int32 off2, sal_Int32 len2) const;

           sal_Int32
        compareString (
                const ::rtl::OUString& s1, const ::rtl::OUString& s2) const;

        ::com::sun::star::uno::Sequence< ::rtl::OUString >
        listCollatorAlgorithms (
                const ::com::sun::star::lang::Locale& rLocale) const;

        ::com::sun::star::uno::Sequence< sal_Int32 >
        listCollatorOptions (
                const ::rtl::OUString& rAlgorithm) const;

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
