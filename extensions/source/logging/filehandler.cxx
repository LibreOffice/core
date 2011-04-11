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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#include "log_module.hxx"
#include "methodguard.hxx"
#include "loghandler.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/logging/XLogHandler.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ucb/AlreadyInitializedException.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/util/XStringSubstitution.hpp>
/** === end UNO includes === **/

#include <tools/diagnose_ex.h>

#include <comphelper/componentcontext.hxx>

#include <cppuhelper/compbase3.hxx>
#include <cppuhelper/basemutex.hxx>

#include <osl/thread.h>
#include <osl/file.hxx>

#include <rtl/strbuf.hxx>

#include <memory>

//........................................................................
namespace logging
{
//........................................................................

    /** === begin UNO using === **/
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
    using ::com::sun::star::util::XStringSubstitution;
    using ::com::sun::star::beans::NamedValue;
    /** === end UNO using === **/

    //====================================================================
    //= FileHandler - declaration
    //====================================================================
    typedef ::cppu::WeakComponentImplHelper3    <   XLogHandler
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
        ::comphelper::ComponentContext  m_aContext;
        LogHandlerHelper                m_aHandlerHelper;
        ::rtl::OUString                 m_sFileURL;
        ::std::auto_ptr< ::osl::File >  m_pFile;
        FileValidity                    m_eFileValidity;

    protected:
        FileHandler( const Reference< XComponentContext >& _rxContext );
        virtual ~FileHandler();

        // XLogHandler
        virtual ::rtl::OUString SAL_CALL getEncoding() throw (RuntimeException);
        virtual void SAL_CALL setEncoding( const ::rtl::OUString& _encoding ) throw (RuntimeException);
        virtual Reference< XLogFormatter > SAL_CALL getFormatter() throw (RuntimeException);
        virtual void SAL_CALL setFormatter( const Reference< XLogFormatter >& _formatter ) throw (RuntimeException);
        virtual ::sal_Int32 SAL_CALL getLevel() throw (RuntimeException);
        virtual void SAL_CALL setLevel( ::sal_Int32 _level ) throw (RuntimeException);
        virtual void SAL_CALL flush(  ) throw (RuntimeException);
        virtual ::sal_Bool SAL_CALL publish( const LogRecord& Record ) throw (RuntimeException);

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw(RuntimeException);
        virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& _rServiceName ) throw(RuntimeException);
        virtual Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(RuntimeException);

        // OComponentHelper
        virtual void SAL_CALL disposing();

    public:
        // XServiceInfo - static version
        static ::rtl::OUString SAL_CALL getImplementationName_static();
        static Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_static();
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
        void    impl_writeString_nothrow( const ::rtl::OString& _rEntry );

        /** does string substitution on a (usually externally provided) file url
        */
        void    impl_doStringsubstitution_nothrow( ::rtl::OUString& _inout_rURL );
    };

    //====================================================================
    //= FileHandler - implementation
    //====================================================================
    //--------------------------------------------------------------------
    FileHandler::FileHandler( const Reference< XComponentContext >& _rxContext )
        :FileHandler_Base( m_aMutex )
        ,m_aContext( _rxContext )
        ,m_aHandlerHelper( _rxContext, m_aMutex, rBHelper )
        ,m_sFileURL( )
        ,m_pFile( )
        ,m_eFileValidity( eUnknown )
    {
    }

    //--------------------------------------------------------------------
    FileHandler::~FileHandler()
    {
        if ( !rBHelper.bDisposed )
        {
            acquire();
            dispose();
        }
    }

    //--------------------------------------------------------------------
    bool FileHandler::impl_prepareFile_nothrow()
    {
        if ( m_eFileValidity == eUnknown )
        {
            m_pFile.reset( new ::osl::File( m_sFileURL ) );
            // check whether the log file already exists
            ::osl::DirectoryItem aFileItem;
            ::osl::DirectoryItem::get( m_sFileURL, aFileItem );
            ::osl::FileStatus aStatus( FileStatusMask_Validate );
            if ( ::osl::FileBase::E_None == aFileItem.getFileStatus( aStatus ) )
                ::osl::File::remove( m_sFileURL );

            ::osl::FileBase::RC res = m_pFile->open( osl_File_OpenFlag_Write | osl_File_OpenFlag_Create );
            m_eFileValidity =   res == ::osl::FileBase::E_None
                            ?   eValid
                            :   eInvalid;
        #if OSL_DEBUG_LEVEL > 0
            if ( m_eFileValidity == eInvalid )
            {
                ::rtl::OStringBuffer sMessage;
                sMessage.append( "FileHandler::impl_prepareFile_nothrow: could not open the designated log file:" );
                sMessage.append( "\nURL: " );
                sMessage.append( ::rtl::OString( m_sFileURL.getStr(), m_sFileURL.getLength(), osl_getThreadTextEncoding() ) );
                sMessage.append( "\nerror code: " );
                sMessage.append( (sal_Int32)res );
                OSL_FAIL( sMessage.makeStringAndClear() );
            }
        #endif
            if ( m_eFileValidity == eValid )
            {
                ::rtl::OString sHead;
                if ( m_aHandlerHelper.getEncodedHead( sHead ) )
                    impl_writeString_nothrow( sHead );
            }
        }

        return m_eFileValidity == eValid;
    }

    //--------------------------------------------------------------------
    void FileHandler::impl_writeString_nothrow( const ::rtl::OString& _rEntry )
    {
        OSL_PRECOND( m_pFile.get(), "FileHandler::impl_writeString_nothrow: no file!" );

        sal_uInt64 nBytesToWrite( _rEntry.getLength() );
        sal_uInt64 nBytesWritten( 0 );
    #if OSL_DEBUG_LEVEL > 0
        ::osl::FileBase::RC res =
    #endif
        m_pFile->write( _rEntry.getStr(), nBytesToWrite, nBytesWritten );
        OSL_ENSURE( ( res == ::osl::FileBase::E_None ) && ( nBytesWritten == nBytesToWrite ),
            "FileHandler::impl_writeString_nothrow: could not write the log entry!" );
    }

    //--------------------------------------------------------------------
    void FileHandler::impl_doStringsubstitution_nothrow( ::rtl::OUString& _inout_rURL )
    {
        try
        {
            Reference< XStringSubstitution > xStringSubst;
            if ( m_aContext.createComponent( "com.sun.star.util.PathSubstitution", xStringSubst ) )
                _inout_rURL = xStringSubst->substituteVariables( _inout_rURL, true );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL FileHandler::disposing()
    {
        if ( m_eFileValidity == eValid )
        {
            ::rtl::OString sTail;
            if ( m_aHandlerHelper.getEncodedTail( sTail ) )
                impl_writeString_nothrow( sTail );
        }

        m_pFile.reset( NULL );
        m_aHandlerHelper.setFormatter( NULL );
    }

    //--------------------------------------------------------------------
    void FileHandler::enterMethod( MethodGuard::Access )
    {
        m_aHandlerHelper.enterMethod();
    }

    //--------------------------------------------------------------------
    void FileHandler::leaveMethod( MethodGuard::Access )
    {
        m_aMutex.release();
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL FileHandler::getEncoding() throw (RuntimeException)
    {
        MethodGuard aGuard( *this );
        ::rtl::OUString sEncoding;
        OSL_VERIFY( m_aHandlerHelper.getEncoding( sEncoding ) );
        return sEncoding;
    }

    //--------------------------------------------------------------------
    void SAL_CALL FileHandler::setEncoding( const ::rtl::OUString& _rEncoding ) throw (RuntimeException)
    {
        MethodGuard aGuard( *this );
        OSL_VERIFY( m_aHandlerHelper.setEncoding( _rEncoding ) );
    }

    //--------------------------------------------------------------------
    Reference< XLogFormatter > SAL_CALL FileHandler::getFormatter() throw (RuntimeException)
    {
        MethodGuard aGuard( *this );
        return m_aHandlerHelper.getFormatter();
    }

    //--------------------------------------------------------------------
    void SAL_CALL FileHandler::setFormatter( const Reference< XLogFormatter >& _rxFormatter ) throw (RuntimeException)
    {
        MethodGuard aGuard( *this );
        m_aHandlerHelper.setFormatter( _rxFormatter );
    }

    //--------------------------------------------------------------------
    ::sal_Int32 SAL_CALL FileHandler::getLevel() throw (RuntimeException)
    {
        MethodGuard aGuard( *this );
        return m_aHandlerHelper.getLevel();
    }

    //--------------------------------------------------------------------
    void SAL_CALL FileHandler::setLevel( ::sal_Int32 _nLevel ) throw (RuntimeException)
    {
        MethodGuard aGuard( *this );
        m_aHandlerHelper.setLevel( _nLevel );
    }

    //--------------------------------------------------------------------
    void SAL_CALL FileHandler::flush(  ) throw (RuntimeException)
    {
        MethodGuard aGuard( *this );
        if(!m_pFile.get())
        {
            OSL_PRECOND(false, "FileHandler::flush: no file!");
            return;
        }
        #if OSL_DEBUG_LEVEL > 0
            ::osl::FileBase::RC res =
        #endif
                m_pFile->sync();
        OSL_ENSURE(res == ::osl::FileBase::E_None, "FileHandler::flush: Could not sync logfile to filesystem.");
    }

    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL FileHandler::publish( const LogRecord& _rRecord ) throw (RuntimeException)
    {
        MethodGuard aGuard( *this );

        if ( !impl_prepareFile_nothrow() )
            return sal_False;

        ::rtl::OString sEntry;
        if ( !m_aHandlerHelper.formatForPublishing( _rRecord, sEntry ) )
            return sal_False;

        impl_writeString_nothrow( sEntry );
        return sal_True;
    }

    //--------------------------------------------------------------------
    void SAL_CALL FileHandler::initialize( const Sequence< Any >& _rArguments ) throw (Exception, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( m_aHandlerHelper.getIsInitialized() )
            throw AlreadyInitializedException();

        if ( _rArguments.getLength() != 1 )
            throw IllegalArgumentException( ::rtl::OUString(), *this, 1 );

        Sequence< NamedValue > aSettings;
        if ( _rArguments[0] >>= m_sFileURL )
        {
            // create( [in] string URL );
            impl_doStringsubstitution_nothrow( m_sFileURL );
        }
        else if ( _rArguments[0] >>= aSettings )
        {
            // createWithSettings( [in] sequence< ::com::sun::star::beans::NamedValue > Settings )
            ::comphelper::NamedValueCollection aTypedSettings( aSettings );
            m_aHandlerHelper.initFromSettings( aTypedSettings );

            if ( aTypedSettings.get_ensureType( "FileURL", m_sFileURL ) )
                impl_doStringsubstitution_nothrow( m_sFileURL );
        }
        else
            throw IllegalArgumentException( ::rtl::OUString(), *this, 1 );

        m_aHandlerHelper.setIsInitialized();
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL FileHandler::getImplementationName() throw(RuntimeException)
    {
        return getImplementationName_static();
    }

    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL FileHandler::supportsService( const ::rtl::OUString& _rServiceName ) throw(RuntimeException)
    {
        const Sequence< ::rtl::OUString > aServiceNames( getSupportedServiceNames() );
        for (   const ::rtl::OUString* pServiceNames = aServiceNames.getConstArray();
                pServiceNames != aServiceNames.getConstArray() + aServiceNames.getLength();
                ++pServiceNames
            )
            if ( _rServiceName == *pServiceNames )
                return sal_True;
        return sal_False;
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL FileHandler::getSupportedServiceNames() throw(RuntimeException)
    {
        return getSupportedServiceNames_static();
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL FileHandler::getImplementationName_static()
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.extensions.FileHandler" ) );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL FileHandler::getSupportedServiceNames_static()
    {
        Sequence< ::rtl::OUString > aServiceNames(1);
        aServiceNames[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.logging.FileHandler" ) );
        return aServiceNames;
    }

    //--------------------------------------------------------------------
    Reference< XInterface > FileHandler::Create( const Reference< XComponentContext >& _rxContext )
    {
        return *( new FileHandler( _rxContext ) );
    }

    //--------------------------------------------------------------------
    void createRegistryInfo_FileHandler()
    {
        static OAutoRegistration< FileHandler > aAutoRegistration;
    }

//........................................................................
} // namespace logging
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
