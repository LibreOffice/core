/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: migrationengine.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:04:57 $
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
#include "precompiled_dbaccess.hxx"

#include "dbmm_global.hrc"
#include "dbmm_module.hxx"
#include "dbmm_types.hxx"
#include "docerrorhandling.hxx"
#include "migrationengine.hxx"
#include "migrationprogress.hxx"
#include "migrationlog.hxx"
#include "progresscapture.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/sdb/XFormDocumentsSupplier.hpp>
#include <com/sun/star/sdb/XReportDocumentsSupplier.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#include <com/sun/star/embed/XComponentSupplier.hpp>
/** === end UNO includes === **/

#include <comphelper/string.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <tools/string.hxx>
#include <tools/diagnose_ex.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/ref.hxx>

#include <vector>

#define DEFAULT_DOC_PROGRESS_RANGE  100000

//........................................................................
namespace dbmm
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::sdb::XOfficeDatabaseDocument;
    using ::com::sun::star::sdb::XFormDocumentsSupplier;
    using ::com::sun::star::sdb::XReportDocumentsSupplier;
    using ::com::sun::star::container::XNameAccess;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::util::XCloseable;
    using ::com::sun::star::util::CloseVetoException;
    using ::com::sun::star::lang::XComponent;
    using ::com::sun::star::frame::XModel;
    using ::com::sun::star::frame::XComponentLoader;
    using ::com::sun::star::ucb::XCommandProcessor;
    using ::com::sun::star::ucb::Command;
    using ::com::sun::star::embed::XComponentSupplier;
    using ::com::sun::star::task::XStatusIndicator;
    /** === end UNO using === **/

    //====================================================================
    //= SubDocument
    //====================================================================
    struct SubDocument
    {
        const Reference< XCommandProcessor >    xCommandProcessor;
        const ::rtl::OUString                   sHierarchicalName;
        const SubDocumentType                   eType;

        SubDocument( const Reference< XCommandProcessor >& _rxCommandProcessor, const ::rtl::OUString& _rName, const SubDocumentType _eType )
            :xCommandProcessor( _rxCommandProcessor )
            ,sHierarchicalName( _rName )
            ,eType( _eType )
        {
        }
    };

    typedef ::std::vector< SubDocument >    SubDocuments;

    //====================================================================
    //= MigrationEngine_Impl - declaration
    //====================================================================
    class MigrationEngine_Impl
    {
    public:
        MigrationEngine_Impl(
            const ::comphelper::ComponentContext& _rContext,
            const Reference< XOfficeDatabaseDocument >& _rxDocument,
            IMigrationProgress& _rProgress,
            MigrationLog& _rLogger
        );
        ~MigrationEngine_Impl();

        inline  sal_Int32   getFormCount() const    { return m_nFormCount; }
        inline  sal_Int32   getReportCount()const   { return m_nReportCount; }
        bool    migrateAll();

    private:
        ::comphelper::ComponentContext              m_aContext;
        const Reference< XOfficeDatabaseDocument >  m_xDocument;
        IMigrationProgress&                         m_rProgress;
        MigrationLog&                               m_rLogger;
        SubDocuments                                m_aSubDocs;
        size_t                                      m_nFormCount;
        size_t                                      m_nReportCount;

    private:
        /** collects a description of all sub documents of our database document

            @return
                <TRUE/> if and only if collecting the documents was successful
        */
        bool    impl_collectSubDocuments_nothrow();

        /** reports the given error (usually an exception caught on the caller's side)
            to the user, using the document's interaction handler, if any.
        */
        void    impl_reportError_nothrow( const Any& _rError ) const;

        /** migrates the macros/scripts of the given sub document
        */
        bool    impl_handleDocument_nothrow( const SubDocument& _rDocument ) const;
    };

    //====================================================================
    //= MigrationEngine_Impl - implementation
    //====================================================================
    //--------------------------------------------------------------------
    MigrationEngine_Impl::MigrationEngine_Impl( const ::comphelper::ComponentContext& _rContext,
            const Reference< XOfficeDatabaseDocument >& _rxDocument, IMigrationProgress& _rProgress, MigrationLog& _rLogger )
        :m_aContext( _rContext )
        ,m_xDocument( _rxDocument )
        ,m_rProgress( _rProgress )
        ,m_rLogger( _rLogger )
        ,m_aSubDocs()
        ,m_nFormCount( 0 )
        ,m_nReportCount( 0 )
    {
        OSL_VERIFY( impl_collectSubDocuments_nothrow() );
    }

    //--------------------------------------------------------------------
    MigrationEngine_Impl::~MigrationEngine_Impl()
    {
    }

    //--------------------------------------------------------------------
    bool MigrationEngine_Impl::migrateAll()
    {
        if  ( m_aSubDocs.empty() )
        {
            OSL_ENSURE( false, "MigrationEngine_Impl::migrateAll: no forms/reports found!" );
            // The whole migration wizard is not expected to be called when there are no forms/reports
            // with macros, not to mention when there are no forms/reports at all.
            return false;
        }

        // initialize global progress
        sal_Int32 nOverallRange( m_aSubDocs.size() );
        String sProgressSkeleton = String( MacroMigrationResId( STR_OVERALL_PROGRESS ) );
        sProgressSkeleton.SearchAndReplaceAscii( "$overall$", String::CreateFromInt32( nOverallRange ) );

        m_rProgress.start( nOverallRange );

        for (   SubDocuments::const_iterator doc = m_aSubDocs.begin();
                doc != m_aSubDocs.end();
                ++doc
            )
        {
            sal_Int32 nOverallProgressValue( doc - m_aSubDocs.begin() + 1 );
            // update overall progress text
            ::rtl::OUString sOverallProgress( sProgressSkeleton );
            ::comphelper::string::searchAndReplaceAsciiI( sOverallProgress, "$current$", ::rtl::OUString::valueOf( nOverallProgressValue ) );
            m_rProgress.setOverallProgressText( sOverallProgress );

            // migrate document
            if ( !impl_handleDocument_nothrow( *doc ) )
                return false;

            // update overall progress vallue
            m_rProgress.setOverallProgressValue( nOverallProgressValue );
        }

        return true;
    }

    //--------------------------------------------------------------------
    namespace
    {
        size_t lcl_collectHierarchicalElementNames_throw(
            const Reference< XNameAccess >& _rxContainer, const ::rtl::OUString& _rContainerLoc,
            SubDocuments& _out_rDocs, const SubDocumentType _eType )
        {
            size_t nAddedElements = 0;

            const ::rtl::OUString sHierarhicalBase(
                _rContainerLoc.getLength()  ?   ::rtl::OUStringBuffer( _rContainerLoc ).appendAscii( "/" ).makeStringAndClear()
                                            :   ::rtl::OUString() );

            Sequence< ::rtl::OUString > aElementNames( _rxContainer->getElementNames() );
            for (   const ::rtl::OUString* elementName = aElementNames.getConstArray();
                    elementName != aElementNames.getConstArray() + aElementNames.getLength();
                    ++elementName
                )
            {
                Any aElement( _rxContainer->getByName( *elementName ) );
                ::rtl::OUString sElementName( ::rtl::OUStringBuffer( sHierarhicalBase ).append( *elementName ) );

                Reference< XNameAccess > xSubContainer( aElement, UNO_QUERY );
                if ( xSubContainer.is() )
                {
                    nAddedElements += lcl_collectHierarchicalElementNames_throw( xSubContainer, sElementName, _out_rDocs, _eType );
                }
                else
                {
                    Reference< XCommandProcessor > xCommandProcessor( aElement, UNO_QUERY );
                    OSL_ENSURE( xCommandProcessor.is(), "lcl_collectHierarchicalElementNames_throw: no container, and no comand processor? What *is* it, then?!" );
                    if ( xCommandProcessor.is() )
                    {
                        _out_rDocs.push_back( SubDocument( xCommandProcessor, sElementName, _eType ) );
                        ++nAddedElements;
                    }
                }
            }
            return nAddedElements;
        }
    }

    //--------------------------------------------------------------------
    bool MigrationEngine_Impl::impl_collectSubDocuments_nothrow()
    {
        try
        {
            ::rtl::OUString sRootLocation;

            Reference< XFormDocumentsSupplier > xSuppForms( m_xDocument, UNO_QUERY_THROW );
            Reference< XNameAccess > xDocContainer( xSuppForms->getFormDocuments(), UNO_SET_THROW );
            m_nFormCount = lcl_collectHierarchicalElementNames_throw( xDocContainer, sRootLocation, m_aSubDocs, eForm );

            Reference< XReportDocumentsSupplier > xSuppReports( m_xDocument, UNO_QUERY_THROW );
            xDocContainer.set( xSuppReports->getReportDocuments(), UNO_SET_THROW );
            m_nReportCount = lcl_collectHierarchicalElementNames_throw( xDocContainer, sRootLocation, m_aSubDocs, eReport );
        }
        catch( const Exception& )
        {
            // TODO: check whether we can handle this error
            DBG_UNHANDLED_EXCEPTION();
            return false;
        }

        return true;
    }

    //--------------------------------------------------------------------
    namespace
    {
        //................................................................
        static void lcl_disposeComponent_nothrow( const Reference< XCommandProcessor >& _rxCommandProc )
        {
            OSL_PRECOND( _rxCommandProc.is(), "lcl_disposeComponent_nothrow: illegal object!" );
            if ( !_rxCommandProc.is() )
                return;

            bool bCouldClose = false;
            try
            {
                Command aCommand;
                aCommand.Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "shutdown" ) );
                OSL_VERIFY( _rxCommandProc->execute(
                    aCommand, _rxCommandProc->createCommandIdentifier(), NULL ) >>= bCouldClose );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            if ( !bCouldClose )
            {
                ;
                // TODO: can we handle this somehow?
            }
        }

        //................................................................
        static Reference< XModel > lcl_loadSubDocument_nothrow( const SubDocument& _rDocument,
            const Reference< XStatusIndicator >& _rxProgress )
        {
            Reference< XModel > xDocument;

            try
            {
                ::comphelper::NamedValueCollection aLoadArgs;
                aLoadArgs.put( "Hidden", (sal_Bool)sal_True );
                aLoadArgs.put( "StatusIndicator", _rxProgress );

                Reference< XCommandProcessor > xCommandProcessor( _rDocument.xCommandProcessor, UNO_SET_THROW );
                Command aCommand;
                aCommand.Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "openDesign" ) );
                aCommand.Argument <<= aLoadArgs.getPropertyValues();
                Reference< XComponent > xDocComponent(
                    xCommandProcessor->execute(
                        aCommand, xCommandProcessor->createCommandIdentifier(), NULL
                    ),
                    UNO_QUERY
                );
                OSL_ENSURE( xDocComponent.is(), "lcl_loadSubDocument_nothrow: no component loaded!" );

                xDocument.set( xDocComponent, UNO_QUERY_THROW );
            }
            catch( const Exception& )
            {
                // TODO: how to proceed?
                DBG_UNHANDLED_EXCEPTION();
            }

            return xDocument;
        }
    }

    //--------------------------------------------------------------------
    bool MigrationEngine_Impl::impl_handleDocument_nothrow( const SubDocument& _rDocument ) const
    {
        DocumentID nDocID = m_rLogger.startedDocument( _rDocument.eType, _rDocument.sHierarchicalName );

        // start the progress
        ::rtl::OUString aProgress;
        aProgress = String( MacroMigrationResId( _rDocument.eType == eForm ? STR_FORM : STR_REPORT ) );
        ::comphelper::string::searchAndReplaceAsciiI( aProgress, "$name$", _rDocument.sHierarchicalName );
        m_rProgress.startObject( aProgress, ::rtl::OUString(), DEFAULT_DOC_PROGRESS_RANGE );

        // load the document
        ::rtl::Reference< ProgressCapture > pStatusIndicator( new ProgressCapture( aProgress, m_rProgress ) );
        Reference< XModel > xDocument( lcl_loadSubDocument_nothrow( _rDocument, pStatusIndicator.get() ) );
        if ( !xDocument.is() )
        {
            pStatusIndicator->dispose();
            m_rProgress.endObject();
            m_rLogger.finishedDocument( nDocID, false );
                // TODO: log the *reason* for the failure
            return false;
        }

        // TODO

        // clean up
        lcl_disposeComponent_nothrow( _rDocument.xCommandProcessor );
        pStatusIndicator->dispose();

        // end the progress, just in case the ProgressCapture didn't receive the XStatusIndicator::end event
        m_rProgress.endObject();

        m_rLogger.finishedDocument( nDocID, true );
        return true;
    }

    //--------------------------------------------------------------------
    void MigrationEngine_Impl::impl_reportError_nothrow( const Any& _rError ) const
    {
        DocumentErrorHandling::reportError( m_aContext, m_xDocument, _rError );
    }

    //====================================================================
    //= MigrationEngine
    //====================================================================
    //--------------------------------------------------------------------
    MigrationEngine::MigrationEngine( const ::comphelper::ComponentContext& _rContext, const Reference< XOfficeDatabaseDocument >& _rxDocument,
            IMigrationProgress& _rProgress, MigrationLog& _rLogger )
        :m_pImpl( new MigrationEngine_Impl( _rContext, _rxDocument, _rProgress, _rLogger ) )
    {
    }

    //--------------------------------------------------------------------
    MigrationEngine::~MigrationEngine()
    {
    }

    //--------------------------------------------------------------------
    sal_Int32 MigrationEngine::getFormCount() const
    {
        return m_pImpl->getFormCount();
    }

    //--------------------------------------------------------------------
    sal_Int32 MigrationEngine::getReportCount() const
    {
        return m_pImpl->getReportCount();
    }

    //--------------------------------------------------------------------
    bool MigrationEngine::migrateAll()
    {
        return m_pImpl->migrateAll();
    }

//........................................................................
} // namespace dbmm
//........................................................................
