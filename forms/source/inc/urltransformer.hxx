/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: urltransformer.hxx,v $
 * $Revision: 1.4 $
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

#ifndef FORMS_SOURCE_INC_URLTRANSFORMER_HXX
#define FORMS_SOURCE_INC_URLTRANSFORMER_HXX

/** === begin UNO includes === **/
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/URL.hpp>
/** === end UNO includes === **/

//........................................................................
namespace frm
{
//........................................................................

    //====================================================================
    //= UrlTransformer
    //====================================================================
    class UrlTransformer
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                        m_xORB;
        mutable ::com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer >
                        m_xTransformer;
        mutable bool    m_bTriedToCreateTransformer;

    public:
        UrlTransformer( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB );

        /** returns an URL object for the given URL string
        */
        ::com::sun::star::util::URL
            getStrictURL( const ::rtl::OUString& _rURL ) const;

        /** returns an URL object for the given URL ASCII string
        */
        ::com::sun::star::util::URL
            getStrictURLFromAscii( const sal_Char* _pAsciiURL ) const;

        /** parses a given URL smartly, with a protocol given by ASCII string
        */
        void
            parseSmartWithAsciiProtocol( ::com::sun::star::util::URL& _rURL, const sal_Char* _pAsciiURL ) const;

    private:
        /** ensures that we have an URLTransformer instance in <member>m_xTransformer</member>

            @return
                <TRUE/> if and only if m_xTransformer is not <NULL/>
        */
        bool    implEnsureTransformer() const;
    };

//........................................................................
} // namespace frm
//........................................................................

#endif // FORMS_SOURCE_INC_URLTRANSFORMER_HXX

