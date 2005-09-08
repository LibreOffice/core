/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: urltransformer.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 22:59:46 $
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

#ifndef FORMS_SOURCE_INC_URLTRANSFORMER_HXX
#define FORMS_SOURCE_INC_URLTRANSFORMER_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_URL_HPP_
#include <com/sun/star/util/URL.hpp>
#endif
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

