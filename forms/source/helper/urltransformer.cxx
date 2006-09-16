/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: urltransformer.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 23:56:33 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_forms.hxx"
#ifndef FORMS_SOURCE_INC_URLTRANSFORMER_HXX
#include "urltransformer.hxx"
#endif

/** === begin UNO includes === **/
/** === end UNO includes === **/

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

//........................................................................
namespace frm
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::lang;

    //====================================================================
    //= UrlTransformer
    //====================================================================
    //--------------------------------------------------------------------
    UrlTransformer::UrlTransformer( const Reference< XMultiServiceFactory >& _rxORB )
        :m_xORB( _rxORB )
        ,m_bTriedToCreateTransformer( false )
    {
        DBG_ASSERT( _rxORB.is(), "UrlTransformer::UrlTransformer: invalid service factory!" );
    }

    //--------------------------------------------------------------------
    bool UrlTransformer::implEnsureTransformer() const
    {
        // create the transformer, if not already attempted to do so
        if ( !m_xTransformer.is() && !m_bTriedToCreateTransformer )
        {
            if ( m_xORB.is() )
            {
                m_xTransformer = m_xTransformer.query(
                    m_xORB->createInstance(
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ) )
                    )
                );
                DBG_ASSERT( m_xTransformer.is(), "UrlTransformer::getStrictURL: couldn't get an URL transformer!" );
            }

            m_bTriedToCreateTransformer = true;
        }
        return m_xTransformer.is();
    }

    //--------------------------------------------------------------------
    URL UrlTransformer::getStrictURL( const ::rtl::OUString& _rURL ) const
    {
        URL aReturn;
        aReturn.Complete = _rURL;
        if ( implEnsureTransformer() )
            m_xTransformer->parseStrict( aReturn );
        return aReturn;
    }

    //--------------------------------------------------------------------
    URL UrlTransformer::getStrictURLFromAscii( const sal_Char* _pAsciiURL ) const
    {
        return getStrictURL( ::rtl::OUString::createFromAscii( _pAsciiURL ) );
    }

    //--------------------------------------------------------------------
    void UrlTransformer::parseSmartWithAsciiProtocol( ::com::sun::star::util::URL& _rURL, const sal_Char* _pAsciiURL ) const
    {
        if ( implEnsureTransformer() )
            m_xTransformer->parseSmart( _rURL, ::rtl::OUString::createFromAscii( _pAsciiURL ) );
    }

//........................................................................
} // namespace frm
//........................................................................
