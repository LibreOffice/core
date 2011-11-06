/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_forms.hxx"
#include "urltransformer.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/
#include <tools/debug.hxx>

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
