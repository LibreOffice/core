/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: urlparameter.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 15:24:29 $
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

#ifndef _URLPARAMETER_HXX_
#define _URLPARAMETER_HXX_

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif
#ifndef _COM_SUN_STAR_UCB_ILLEGALIDENTIFIEREXCEPTION_HPP_
#include <com/sun/star/ucb/IllegalIdentifierException.hpp>
#endif
#ifndef  _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDENVIRONMENT_HPP_
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_COMMAND_HPP_
#include <com/sun/star/ucb/Command.hpp>
#endif

namespace chelp {


    class Databases;


    class DbtToStringConverter
    {
    public:

        DbtToStringConverter( const sal_Char* ptr,sal_Int32 len )
            : m_len( len ),
              m_ptr( ptr )
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
            if( ! m_ptr ) // || getHash().getLength() )
                return rtl::OUString();

            sal_Int32 sizeOfTitle =
                ( sal_Int32 ) m_ptr[  2 + m_ptr[0] +  ( sal_Int32 ) m_ptr[ 1+ ( sal_Int32 ) m_ptr[0] ] ];
            return rtl::OUString( m_ptr + 3 + m_ptr[0] +  ( sal_Int32 ) m_ptr[ 1+ ( sal_Int32 ) m_ptr[0] ],
                                  sizeOfTitle,
                                  RTL_TEXTENCODING_UTF8 );
        }


    private:

        sal_Int32 m_len;

        const sal_Char* m_ptr;

    };




    class URLParameter
    {
    public:

        URLParameter( const rtl::OUString& aURL,
                      Databases* pDatabases )
            throw( com::sun::star::ucb::IllegalIdentifierException );

        URLParameter( const rtl::OUString&  aURL,
                      const rtl::OUString& aDefaultLanguage,
                      Databases* pDatabases )
            throw( com::sun::star::ucb::IllegalIdentifierException );


        bool isPicture()        { return m_aModule.compareToAscii("picture") == 0; }
        bool isActive()         { return m_aActive.getLength() > 0 && m_aActive.compareToAscii( "true" ) == 0; }
        bool isQuery()          { return m_aId.compareToAscii("") == 0 && m_aQuery.compareToAscii("") != 0; }
        bool isEntryForModule() { return m_aId.compareToAscii("start") == 0 || m_bStart; }
        bool isFile()           { return m_aId.compareToAscii( "" ) != 0; }
        bool isModule()         { return m_aId.compareToAscii("") == 0 && m_aModule.compareToAscii("") != 0; }
        bool isRoot()           { return m_aModule.compareToAscii("") == 0; }
        bool isErrorDocument();

        rtl::OUString get_url() { return m_aURL; }

        rtl::OUString get_id();

        rtl::OUString get_tag();

        //  Not called for an directory

        rtl::OUString get_path()   { return get_the_path(); }

        rtl::OUString get_eid()    { return m_aEid; }

        rtl::OUString get_title();

        rtl::OUString get_jar()      { return get_the_jar(); }      // BerkeleyDb

        rtl::OUString get_module()   { return m_aModule; }

        rtl::OUString get_dbpar()    {
            if( m_aDbPar.getLength() ) return m_aDbPar;
            else return m_aModule;
        }

        rtl::OUString get_prefix()   { return m_aPrefix; }

        rtl::OUString get_language();

        rtl::OUString get_device()   { return m_aDevice; }

        rtl::OUString get_program();

        rtl::OUString get_query()    { return m_aQuery; }

        rtl::OUString get_scope()    { return m_aScope; }

        rtl::OUString get_system()   { return m_aSystem; }

        sal_Int32     get_hitCount() { return m_nHitCount; }

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
