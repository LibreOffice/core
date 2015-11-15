/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include "log_module.hxx"
#include "log_services.hxx"
#include "methodguard.hxx"
#include "loghandler.hxx"

#include <com/sun/star/logging/XLogHandler.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ucb/AlreadyInitializedException.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/util/PathSubstitution.hpp>
#include <com/sun/star/util/XStringSubstitution.hpp>

#include <tools/diagnose_ex.h>

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <osl/thread.h>
#include <osl/file.hxx>

#include <rtl/strbuf.hxx>

#include <memory>


namespace logging
{


    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::logging::LogRecord;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::logging::XLogFormatter;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::logging::XLogHandler;
    using ::com::sun::star::lang::XServiceInfo;
    using ::com::sun::star::ucb::AlreadyInitializedException;
    using ::com::sun::star::lang::XInitialization;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::util::PathSubstitution;
    using ::com::sun::star::util::XStringSubstitution;
    using ::com::sun::star::beans::NamedValue;

    typedef ::cppu::WeakComponentImplHelper    <   XLogHandler
                                                ,   XServiceInfo
                                                ,   XInitialization
                                                >   FileHandler_Base;
    class FileHandler   :public ::cppu::BaseMutex
                        ,public FileHandler_Base
    {
    private:
        enum FileValidity
        {
            /// never attempted to open the file
            eUnknown,
            /// file is valid
            eValid,
            /// file is invalid
            eInvalid
        };

    private:
        Reference<XComponentContext>    m_xContext;
        LogHandlerHelper                m_aHandlerHelper;
        OUString                 m_sFileURL;
        ::std::unique_ptr< ::osl::File >  m_pFile;
        FileValidity                    m_eFileValidity;

    protected:
        explicit FileHandler( const Reference< XComponentContext >& _rxContext );
        virtual ~FileHandler();

        // XLogHandler
        virtual OUString SAL_CALL getEncoding() throw (RuntimeException, std::exception) override;
        virtual void SAL_CALL setEncoding( const OUString& _encoding ) throw (RuntimeException, std::exception) override;
        virtual Reference< XLogFormatter > SAL_CALL getFormatter() throw (RuntimeException, std::exception) override;
        virtual void SAL_CALL setFormatter( const Reference< XLogFormatter >& _formatter ) throw (RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getLevel() throw (RuntimeException, std::exception) override;
        virtual void SAL_CALL setLevel( ::sal_Int32 _level ) throw (RuntimeException, std::exception) override;
        virtual void SAL_CALL flush(  ) throw (RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL publish( const LogRecord& Record ) throw (RuntimeException, std::exception) override;

        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& _rServiceName ) throw(RuntimeException, std::exception) override;
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(RuntimeException, std::exception) override;

        // OComponentHelper
        virtual void SAL_CALL disposing() override;

    public:
        // XServiceInfo - static version
        static OUString SAL_CALL getImplementationName_static();
        static Sequence< OUString > SAL_CALL getSupportedServiceNames_static();
        static Reference< XInterface > Create( const Reference< XComponentContext >& _rxContext );

    public:
        typedef ComponentMethodGuard< FileHandler > MethodGuard;
        void    enterMethod( MethodGuard::Access );
        void    leaveMethod( MethodGuard::Access );

    private:
        /** prepares our output file for writing
        */
        bool    impl_prepareFile_nothrow();

        /// writes the given string to our file
        void    impl_writeString_nothrow( const OString& _rEntry );

        /** does string substitution on a (usually externally provided) file url
        */
        void    impl_doStringsubstitution_nothrow( OUString& _inout_rURL );
    };

    FileHandler::FileHandler( const Reference< XComponentContext >& _rxContext )
        :FileHandler_Base( m_aMutex )
        ,m_xContext( _rxContext )
        ,m_aHandlerHelper( _rxContext, m_aMutex, rBHelper )
        ,m_sFileURL( )
        ,m_pFile( )
        ,m_eFileValidity( eUnknown )
    {
    }


    FileHandler::~FileHandler()
    {
        if ( !rBHelper.bDisposed )
        {
            acquire();
            dispose();
        }
    }


    bool FileHandler::impl_prepareFile_nothrow()
    {
        if ( m_eFileValidity == eUnknown )
        {
            m_pFile.reset( new ::osl::File( m_sFileURL ) );
            // check whether the log file already exists
            ::osl::DirectoryItem aFileItem;
            ::osl::DirectoryItem::get( m_sFileURL, aFileItem );
            ::osl::FileStatus aStatus( osl_FileStatus_Mask_Validate );
            if ( ::osl::FileBase::E_None == aFileItem.getFileStatus( aStatus ) )
                ::osl::File::remove( m_sFileURL );

            ::osl::FileBase::RC res = m_pFile->open( osl_File_OpenFlag_Write | osl_File_OpenFlag_Create );
            m_eFileValidity =   res == ::osl::FileBase::E_None
                            ?   eValid
                            :   eInvalid;
        #if OSL_DEBUG_LEVEL > 0
            if ( m_eFileValidity == eInvalid )
            {
                OStringBuffer sMessage;
                sMessage.append( "FileHandler::impl_prepareFile_nothrow: could not open the designated log file:" );
                sMessage.append( "\nURL: " );
                sMessage.append( OString( m_sFileURL.getStr(), m_sFileURL.getLength(), osl_getThreadTextEncoding() ) );
                sMessage.append( "\nerror code: " );
                sMessage.append( (sal_Int32)res );
                OSL_FAIL( sMessage.makeStringAndClear().getStr() );
            }
        #endif
            if ( m_eFileValidity == eValid )
            {
                OString sHead;
                if ( m_aHandlerHelper.getEncodedHead( sHead ) )
                    impl_writeString_nothrow( sHead );
            }
        }

        return m_eFileValidity == eValid;
    }


    void FileHandler::impl_writeString_nothrow( const OString& _rEntry )
    {
        OSL_PRECOND( m_pFile.get(), "FileHandler::impl_writeString_nothrow: no file!" );

        sal_uInt64 nBytesToWrite( _rEntry.getLength() );
        sal_uInt64 nBytesWritten( 0 );
        ::osl::FileBase::RC res =
              m_pFile->write( _rEntry.getStr(), nBytesToWrite, nBytesWritten );
        OSL_ENSURE( ( res == ::osl::FileBase::E_None ) && ( nBytesWritten == nBytesToWrite ),
            "FileHandler::impl_writeString_nothrow: could not write the log entry!" );
    }


    void FileHandler::impl_doStringsubstitution_nothrow( OUString& _inout_rURL )
    {
        try
        {
            Reference< XStringSubstitution > xStringSubst(PathSubstitution::create(m_xContext));
            _inout_rURL = xStringSubst->substituteVariables( _inout_rURL, true );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }


    void SAL_CALL FileHandler::disposing()
    {
        if ( m_eFileValidity == eValid )
        {
            OString sTail;
            if ( m_aHandlerHelper.getEncodedTail( sTail ) )
                impl_writeString_nothrow( sTail );
        }

        m_pFile.reset();
        m_aHandlerHelper.setFormatter( nullptr );
    }


    void FileHandler::enterMethod( MethodGuard::Access )
    {
        m_aHandlerHelper.enterMethod();
    }


    void FileHandler::leaveMethod( MethodGuard::Access )
    {
        m_aMutex.release();
    }


    OUString SAL_CALL FileHandler::getEncoding() throw (RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this );
        OUString sEncoding;
        OSL_VERIFY( m_aHandlerHelper.getEncoding( sEncoding ) );
        return sEncoding;
    }


    void SAL_CALL FileHandler::setEncoding( const OUString& _rEncoding ) throw (RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this );
        OSL_VERIFY( m_aHandlerHelper.setEncoding( _rEncoding ) );
    }


    Reference< XLogFormatter > SAL_CALL FileHandler::getFormatter() throw (RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this );
        return m_aHandlerHelper.getFormatter();
    }


    void SAL_CALL FileHandler::setFormatter( const Reference< XLogFormatter >& _rxFormatter ) throw (RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this );
        m_aHandlerHelper.setFormatter( _rxFormatter );
    }


    ::sal_Int32 SAL_CALL FileHandler::getLevel() throw (RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this );
        return m_aHandlerHelper.getLevel();
    }


    void SAL_CALL FileHandler::setLevel( ::sal_Int32 _nLevel ) throw (RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this );
        m_aHandlerHelper.setLevel( _nLevel );
    }


    void SAL_CALL FileHandler::flush(  ) throw (RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this );
        if(!m_pFile.get())
        {
            OSL_PRECOND(false, "FileHandler::flush: no file!");
            return;
        }
        ::osl::FileBase::RC res = m_pFile->sync();
        OSL_ENSURE(res == ::osl::FileBase::E_None, "FileHandler::flush: Could not sync logfile to filesystem.");
    }


    sal_Bool SAL_CALL FileHandler::publish( const LogRecord& _rRecord ) throw (RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this );

        if ( !impl_prepareFile_nothrow() )
            return sal_False;

        OString sEntry;
        if ( !m_aHandlerHelper.formatForPublishing( _rRecord, sEntry ) )
            return sal_False;

        impl_writeString_nothrow( sEntry );
        return sal_True;
    }


    void SAL_CALL FileHandler::initialize( const Sequence< Any >& _rArguments ) throw (Exception, RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( m_aHandlerHelper.getIsInitialized() )
            throw AlreadyInitializedException();

        if ( _rArguments.getLength() != 1 )
            throw IllegalArgumentException( OUString(), *this, 1 );

        Sequence< NamedValue > aSettings;
        if ( _rArguments[0] >>= m_sFileURL )
        {
            // create( [in] string URL );
            impl_doStringsubstitution_nothrow( m_sFileURL );
        }
        else if ( _rArguments[0] >>= aSettings )
        {
            // createWithSettings( [in] sequence< css::beans::NamedValue > Settings )
            ::comphelper::NamedValueCollection aTypedSettings( aSettings );
            m_aHandlerHelper.initFromSettings( aTypedSettings );

            if ( aTypedSettings.get_ensureType( "FileURL", m_sFileURL ) )
                impl_doStringsubstitution_nothrow( m_sFileURL );
        }
        else
            throw IllegalArgumentException( OUString(), *this, 1 );

        m_aHandlerHelper.setIsInitialized();
    }


    OUString SAL_CALL FileHandler::getImplementationName() throw(RuntimeException, std::exception)
    {
        return getImplementationName_static();
    }

    sal_Bool SAL_CALL FileHandler::supportsService( const OUString& _rServiceName ) throw(RuntimeException, std::exception)
    {
        return cppu::supportsService(this, _rServiceName);
    }


    Sequence< OUString > SAL_CALL FileHandler::getSupportedServiceNames() throw(RuntimeException, std::exception)
    {
        return getSupportedServiceNames_static();
    }


    OUString SAL_CALL FileHandler::getImplementationName_static()
    {
        return OUString( "com.sun.star.comp.extensions.FileHandler" );
    }


    Sequence< OUString > SAL_CALL FileHandler::getSupportedServiceNames_static()
    {
        Sequence< OUString > aServiceNames { "com.sun.star.logging.FileHandler" };
        return aServiceNames;
    }


    Reference< XInterface > FileHandler::Create( const Reference< XComponentContext >& _rxContext )
    {
        return *( new FileHandler( _rxContext ) );
    }


    void createRegistryInfo_FileHandler()
    {
        static OAutoRegistration< FileHandler > aAutoRegistration;
    }


} // namespace logging


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
