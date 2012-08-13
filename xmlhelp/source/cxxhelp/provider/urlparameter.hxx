/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _URLPARAMETER_HXX_
#define _URLPARAMETER_HXX_

#include <rtl/ustring.hxx>
#include <rtl/string.hxx>
#include <com/sun/star/ucb/IllegalIdentifierException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/ucb/Command.hpp>

namespace chelp {


    class Databases;


    class DbtToStringConverter
    {
    public:

        DbtToStringConverter( const sal_Char* ptr )
            : m_ptr( ptr )
        {
        }


        rtl::OUString getHash()
        {
            if( m_ptr )
            {
                sal_Int32 sizeOfFile = ( sal_Int32 ) m_ptr[0];
                rtl::OUString Hash( m_ptr+1,sizeOfFile,RTL_TEXTENCODING_UTF8 );
                sal_Int32 idx;
                if( ( idx = Hash.indexOf( sal_Unicode( '#' ) ) ) != -1 )
                    return Hash.copy( 1+idx );
            }
            return rtl::OUString();
        }


        rtl::OUString getFile()
        {
            if( ! m_ptr )
                return rtl::OUString();

            sal_Int32 sizeOfFile = ( sal_Int32 ) m_ptr[0];
            rtl::OUString File( m_ptr+1,sizeOfFile,RTL_TEXTENCODING_UTF8 );
            sal_Int32 idx;
            if( ( idx = File.indexOf( sal_Unicode( '#' ) ) ) != -1 )
                return File.copy( 0,idx );
            else
                return File;
        }


        rtl::OUString getDatabase()
        {
            if( ! m_ptr )
                return rtl::OUString();

            sal_Int32 sizeOfDatabase = ( int ) m_ptr[ 1+ ( sal_Int32 ) m_ptr[0] ];
            return rtl::OUString( m_ptr + 2 + ( sal_Int32 ) m_ptr[0],sizeOfDatabase,RTL_TEXTENCODING_UTF8 );
        }


        rtl::OUString getTitle()
        {
            if( ! m_ptr )
                return rtl::OUString();

            sal_Int32 sizeOfTitle =
                ( sal_Int32 ) m_ptr[  2 + m_ptr[0] +  ( sal_Int32 ) m_ptr[ 1+ ( sal_Int32 ) m_ptr[0] ] ];
            return rtl::OUString( m_ptr + 3 + m_ptr[0] +  ( sal_Int32 ) m_ptr[ 1+ ( sal_Int32 ) m_ptr[0] ],
                                  sizeOfTitle,
                                  RTL_TEXTENCODING_UTF8 );
        }


    private:

        const sal_Char* m_ptr;

    };




    class URLParameter
    {
    public:

        URLParameter( const rtl::OUString& aURL,
                      Databases* pDatabases )
            throw( com::sun::star::ucb::IllegalIdentifierException );

        bool isPicture() const { return m_aModule == "picture"; }
        bool isActive() const { return !m_aActive.isEmpty() && m_aActive == "true"; }
        bool isQuery() const { return m_aId.isEmpty() && !m_aQuery.isEmpty(); }
        bool isEntryForModule() const { return m_aId == "start" || m_bStart; }
        bool isFile() const { return !m_aId.isEmpty(); }
        bool isModule() const { return m_aId.isEmpty() && !m_aModule.isEmpty(); }
        bool isRoot() const { return m_aModule.isEmpty(); }
        bool isErrorDocument();

        rtl::OUString get_url() const { return m_aURL; }

        rtl::OUString get_id();

        rtl::OUString get_tag();

        //  Not called for an directory

        rtl::OUString get_path()   { return get_the_path(); }

        rtl::OUString get_eid() const   { return m_aEid; }

        rtl::OUString get_title();

        rtl::OUString get_jar()      { return get_the_jar(); }      // BerkeleyDb

        rtl::OUString get_ExtensionRegistryPath() const { return m_aExtensionRegistryPath; }

        rtl::OUString get_module() const { return m_aModule; }

        rtl::OUString get_dbpar() const
        {
            if( !m_aDbPar.isEmpty() )
                return m_aDbPar;
            else
                return m_aModule;
        }

        rtl::OUString get_prefix() const { return m_aPrefix; }

        rtl::OUString get_language();

        rtl::OUString get_device() const { return m_aDevice; }

        rtl::OUString get_program();

        rtl::OUString get_query() const { return m_aQuery; }

        rtl::OUString get_scope() const { return m_aScope; }

        rtl::OUString get_system() const { return m_aSystem; }

        sal_Int32     get_hitCount() const { return m_nHitCount; }

        rtl::OString getByName( const char* par );

        void open( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
                   const com::sun::star::ucb::Command& aCommand,
                   sal_Int32 CommandId,
                   const com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment >& Environment,
                   const com::sun::star::uno::Reference< com::sun::star::io::XActiveDataSink >& xDataSink );

        void open( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
                   const com::sun::star::ucb::Command& aCommand,
                   sal_Int32 CommandId,
                   const com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment >& Environment,
                   const com::sun::star::uno::Reference< com::sun::star::io::XOutputStream >& xDataSink );

    private:

        Databases* m_pDatabases;

        bool m_bBerkeleyRead;
        bool m_bStart;
        bool m_bUseDB;

        rtl::OUString  m_aURL;

        rtl::OUString  m_aTag;
        rtl::OUString  m_aId;
        rtl::OUString  m_aPath;
        rtl::OUString  m_aModule;
        rtl::OUString  m_aTitle;
        rtl::OUString  m_aJar;
        rtl::OUString  m_aExtensionRegistryPath;
        rtl::OUString  m_aEid;
        rtl::OUString  m_aDbPar;

        rtl::OUString  m_aDefaultLanguage;
        rtl::OUString  m_aLanguage;

        rtl::OUString  m_aPrefix;
        rtl::OUString  m_aDevice;
        rtl::OUString  m_aProgram;
        rtl::OUString  m_aSystem;
        rtl::OUString  m_aActive;

        rtl::OUString  m_aQuery;
        rtl::OUString  m_aScope;

        rtl::OUString m_aExpr;

        sal_Int32      m_nHitCount;                // The default maximum hitcount


        // private methods

        void init( bool bDefaultLanguageIsInitialized );

        rtl::OUString get_the_tag();

        rtl::OUString get_the_path();

        rtl::OUString get_the_title();

        rtl::OUString get_the_jar();

        void readBerkeley();

        void parse() throw( com::sun::star::ucb::IllegalIdentifierException );

        bool scheme();

        bool module();

        bool name( bool modulePresent );

        bool query();

    };   // end class URLParameter


}  // end namespace chelp

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
