/*************************************************************************
 *
 *  $RCSfile: unomailmerge.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 14:23:55 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#pragma hdrstop

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _SFX_ITEMPROP_HXX
#include <svtools/itemprop.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _SVX_DATACCESSDESCRIPTOR_HXX_
#include <svx/dataaccessdescriptor.hxx>
#endif
#ifndef _SHL_HXX
#include <tools/shl.hxx>    // GetAppData
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _TOOLS_TEMPFILE_HXX
#include <tools/tempfile.hxx>
#endif
#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX
#include <sfx2/docfilt.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx> // WeakImplHelper1
#endif
#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif

#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_MAILMERGETYPE_HPP_
#include <com/sun/star/text/MailMergeType.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_MAILMERGEEVENT_HPP_
#include <com/sun/star/text/MailMergeEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XMAILMERGELISTENER_HPP_
#include <com/sun/star/text/XMailMergeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XMAILMERGEBROADCASTER_HPP_
#include <com/sun/star/text/XMailMergeBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWSET_HPP_
#include <com/sun/star/sdbc/XRowSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_CloseVetoException_HPP_
#include <com/sun/star/util/CloseVetoException.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XROWLOCATE_HPP_
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif

#ifndef _UNOMAILMERGE_HXX_
#include <unomailmerge.hxx>
#endif
#ifndef SW_SWDLL_HXX
#include <swdll.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _UNOPRNMS_HXX
#include <unoprnms.hxx>
#endif
#ifndef _UNOMAP_HXX
#include <unomap.hxx>
#endif
#ifndef _SWUNOHELPER_HXX
#include <swunohelper.hxx>
#endif
#ifndef _SWDOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _SWVIEW_HXX
#include <view.hxx>
#endif
#ifndef _SWDBMGR_HXX
#include <dbmgr.hxx>
#endif
#ifndef _UNOTXDOC_HXX
#include <unotxdoc.hxx>
#endif
#ifndef _PRTOPT_HXX
#include <prtopt.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _SWDBMGR_HXX
#include <dbmgr.hxx>
#endif

#define C2U(x)         OUString::createFromAscii(x)
#define SN_MAIL_MERGE               "com.sun.star.text.MailMerge"
#define SN_DATA_ACCESS_DESCRIPTOR   "com.sun.star.sdb.DataAccessDescriptor"

using namespace com::sun::star;
using namespace com::sun::star::frame;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::text;
using namespace rtl;
using namespace SWUnoHelper;

////////////////////////////////////////////////////////////

class DisposeHelper
{
    uno::Reference< XComponent > xC;

    // disallow use of copy c-tor and assignment operator
    DisposeHelper( const DisposeHelper & );
    DisposeHelper & operator = ( const DisposeHelper & );

public:
    DisposeHelper() {}
#if OSL_DEBUG_LEVEL > 1
    ~DisposeHelper();
#else
    ~DisposeHelper()    { if (xC.is()) xC->dispose(); }
#endif

    BOOL    SetInterface( uno::Reference< XInterface > &rxRef )  { return (xC = uno::Reference< XComponent >( rxRef, UNO_QUERY )).is(); }
};

#if OSL_DEBUG_LEVEL > 1
DisposeHelper::~DisposeHelper()
{
    if (xC.is())
        xC->dispose();
}
#endif

////////////////////////////////////////////////////////////

osl::Mutex &    GetMailMergeMutex()
{
    static osl::Mutex   aMutex;
    return aMutex;
}

////////////////////////////////////////////////////////////

enum CloseResult
{
    eSuccess,       // successfully closed
    eVetoed,        // vetoed, ownership transfered to the vetoing instance
    eFailed         // failed for some unknown reason
};
static CloseResult CloseModelAndDocSh(
       Reference< frame::XModel > &rxModel,
       SfxObjectShellRef &rxDocSh )
{
    CloseResult eResult = eSuccess;

    rxDocSh = 0;

    //! models/documents should never be disposed (they may still be
    //! used for printing which is called asynchronously for example)
    //! instead call close
    Reference< util::XCloseable > xClose( rxModel, UNO_QUERY );
    if (xClose.is())
    {
        try
        {
            xClose->close( sal_True );
        }
        catch (util::CloseVetoException &)
        {
            //! here we have the problem that the temporary file that is
            //! currently being printed will never be deleted. :-(
            eResult = eVetoed;
        }
        catch ( const uno::RuntimeException& )
        {
            eResult = eFailed;
        }
    }
    return eResult;
}

////////////////////////////////////////////////////////////

static BOOL LoadFromURL_impl(
        Reference< frame::XModel > &rxModel,
        SfxObjectShellRef &rxDocSh,
        const String &rURL,
        BOOL bClose )
    throw (RuntimeException)
{
    // try to open the document readonly and hidden
    Reference< frame::XModel > xTmpModel;
    Sequence < PropertyValue > aArgs( 1 );
    aArgs[0].Name = C2U("Hidden");
    sal_Bool bVal = sal_True;
    aArgs[0].Value <<= bVal;
    try
    {
        Reference < XComponentLoader > xDesktop( ::comphelper::getProcessServiceFactory()->
                createInstance( C2U("com.sun.star.frame.Desktop") ), UNO_QUERY );
        xTmpModel = Reference < XModel >( xDesktop->loadComponentFromURL(
                rURL, C2U("_blank"), 0, aArgs ), UNO_QUERY );
    }
    catch( Exception & )
    {
        return FALSE;
    }

    // try to get the DocShell
    SwDocShell *pTmpDocShell = 0;
    Reference < XUnoTunnel > xTunnel( xTmpModel, UNO_QUERY );
    if (xTunnel.is())
    {
        SwXTextDocument* pTextDoc = (SwXTextDocument *) xTunnel->getSomething( SwXTextDocument::getUnoTunnelId() );
        pTmpDocShell = pTextDoc ? pTextDoc->GetDocShell() : 0;
    }

    BOOL bRes = FALSE;
    if (xTmpModel.is() && pTmpDocShell)    // everything available?
    {
        if (bClose)
            CloseModelAndDocSh( rxModel, rxDocSh );
        // set new stuff
        rxModel = xTmpModel;
        rxDocSh = pTmpDocShell;
        bRes = TRUE;
    }
    else
    {
        SfxObjectShellRef xTmpDocSh = pTmpDocShell;
        CloseModelAndDocSh( xTmpModel, xTmpDocSh );
    }

    return bRes;
}

//==========================================================
namespace
{
    class DelayedFileDeletion : public ::cppu::WeakImplHelper1< ::com::sun::star::util::XCloseListener >
    {
    protected:
        ::osl::Mutex                    m_aMutex;
        Reference< util::XCloseable >   m_xDocument;
        Timer                           m_aDeleteTimer;
        String                          m_sTemporaryFile;
        sal_Int32                       m_nPendingDeleteAttempts;

    public:
        DelayedFileDeletion( const Reference< XModel >& _rxModel,
                             const String& _rTemporaryFile );

    protected:
        ~DelayedFileDeletion( );

        // XCloseListener
        virtual void SAL_CALL queryClosing( const EventObject& _rSource, sal_Bool _bGetsOwnership ) throw (util::CloseVetoException, RuntimeException);
        virtual void SAL_CALL notifyClosing( const EventObject& _rSource ) throw (RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const EventObject& Source ) throw (RuntimeException);

    private:
        void implTakeOwnership( );
        DECL_LINK( OnTryDeleteFile, void* );

    private:
        DelayedFileDeletion( const DelayedFileDeletion& );                  // never implemented
        DelayedFileDeletion& operator=( const DelayedFileDeletion& );       // never implemented
    };

    DBG_NAME( DelayedFileDeletion )
    //------------------------------------------------------
    DelayedFileDeletion::DelayedFileDeletion( const Reference< XModel >& _rxModel, const String& _rTemporaryFile )
        :m_sTemporaryFile( _rTemporaryFile )
        ,m_nPendingDeleteAttempts( 0 )
        ,m_xDocument( _rxModel, UNO_QUERY )
    {
        DBG_CTOR( DelayedFileDeletion, NULL );

        osl_incrementInterlockedCount( &m_refCount );
        try
        {
            if ( m_xDocument.is() )
            {
                m_xDocument->addCloseListener( this );
                // successfully added -> keep ourself alive
                acquire();
            }
            else
                DBG_ERROR( "DelayedFileDeletion::DelayedFileDeletion: model is no component!" );
        }
        catch( const Exception& )
        {
            DBG_ERROR( "DelayedFileDeletion::DelayedFileDeletion: could not register as event listener at the model!" );
        }
        osl_decrementInterlockedCount( &m_refCount );
    }

    //--------------------------------------------------------------------
    IMPL_LINK( DelayedFileDeletion, OnTryDeleteFile, void*, NOTINTERESTEDIN )
    {
        ::osl::ClearableMutexGuard aGuard( m_aMutex );

        sal_Bool bSuccess = sal_False;
        try
        {
            sal_Bool bDeliverOwnership = ( 0 == m_nPendingDeleteAttempts );
                // if this is our last attemt, then anybody which vetoes this has to take the consequences
                // (means take the ownership)
            m_xDocument->close( bDeliverOwnership );
            bSuccess = sal_True;
        }
        catch( const util::CloseVetoException& )
        {
            // somebody vetoed -> next try
            if ( m_nPendingDeleteAttempts )
            {
                // next attempt
                --m_nPendingDeleteAttempts;
                m_aDeleteTimer.Start();
            }
            else
                bSuccess = sal_True;    // can't do anything here ...
        }
        catch( const Exception& )
        {
            DBG_ERROR( "DelayedFileDeletion::OnTryDeleteFile: caught a strange exception!" );
            bSuccess = sal_True;
                // can't do anything here ...
        }

        if ( bSuccess )
        {
            SWUnoHelper::UCB_DeleteFile( m_sTemporaryFile );
            aGuard.clear();
            release();  // this should be our last reference, we should be dead after this
        }
        return 0L;
    }

    //--------------------------------------------------------------------
    void DelayedFileDeletion::implTakeOwnership( )
    {
        // revoke ourself as listener
        try
        {
            m_xDocument->removeCloseListener( this );
        }
        catch( const Exception & )
        {
            DBG_ERROR( "DelayedFileDeletion::implTakeOwnership: could not revoke the listener!" );
        }

        m_aDeleteTimer.SetTimeout( 3000 );  // 3 seconds
        m_aDeleteTimer.SetTimeoutHdl( LINK( this, DelayedFileDeletion, OnTryDeleteFile ) );
        m_nPendingDeleteAttempts = 3;   // try 3 times at most
        m_aDeleteTimer.Start( );
    }

    //--------------------------------------------------------------------
    void SAL_CALL DelayedFileDeletion::queryClosing( const EventObject& _rSource, sal_Bool _bGetsOwnership ) throw (util::CloseVetoException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( _bGetsOwnership )
            implTakeOwnership( );

        // always veto: We want to take the ownership ourself, as this is the only chance to delete
        // the temporary file which the model is based on
        throw util::CloseVetoException( );
    }

    //--------------------------------------------------------------------
    void SAL_CALL DelayedFileDeletion::notifyClosing( const EventObject& _rSource ) throw (RuntimeException)
    {
        DBG_ERROR( "DelayedFileDeletion::notifyClosing: how this?" );
        // this should not happen:
        // Either, a foreign instance closes the document, then we should veto this, and take the ownership
        // Or, we ourself close the document, then we should not be a listener anymore
    }

    //------------------------------------------------------
    void SAL_CALL DelayedFileDeletion::disposing( const EventObject& Source ) throw (RuntimeException)
    {
        DBG_ERROR( "DelayedFileDeletion::disposing: how this?" );
        // this should not happen:
        // Either, a foreign instance closes the document, then we should veto this, and take the ownership
        // Or, we ourself close the document, then we should not be a listener anymore
    }

    //------------------------------------------------------
    DelayedFileDeletion::~DelayedFileDeletion( )
    {
        DBG_DTOR( DelayedFileDeletion, NULL );
    }
}

////////////////////////////////////////////////////////////

static BOOL DeleteTmpFile_Impl(
        Reference< frame::XModel > &rxModel,
        SfxObjectShellRef &rxDocSh,
        const String &rTmpFileURL )
{
    BOOL bRes = FALSE;
    if (rTmpFileURL.Len())
    {
        BOOL bDelete = TRUE;
        if ( eVetoed == CloseModelAndDocSh( rxModel, rxDocSh ) )
        {
            // somebody vetoed the closing, and took the ownership of the document
            // -> ensure that the temporary file is deleted later on
            Reference< XEventListener > xEnsureDelete( new DelayedFileDeletion( rxModel, rTmpFileURL ) );
                // note: as soon as #106931# is fixed, the whole DelayedFileDeletion is to be superseeded by
                // a better solution
            bDelete = FALSE;
        }

        rxModel = 0;
        rxDocSh = 0; // destroy doc shell

        if ( bDelete )
        {
            if ( !SWUnoHelper::UCB_DeleteFile( rTmpFileURL ) )
            {
                Reference< XEventListener > xEnsureDelete( new DelayedFileDeletion( rxModel, rTmpFileURL ) );
                    // same not as above: as soon as #106931#, ...
            }
        }
        else
            bRes = TRUE;    // file will be deleted delayed
    }
    return bRes;
}

////////////////////////////////////////////////////////////

SwXMailMerge::SwXMailMerge() :
    pMap( aSwMapProvider.GetPropertyMap( PROPERTY_MAP_MAILMERGE ) ),
    aEvtListeners   ( GetMailMergeMutex() ),
    aMergeListeners ( GetMailMergeMutex() ),
    aPropListeners  ( GetMailMergeMutex() )
{
    // create empty document
    // like in: SwModule::InsertEnv (appenv.cxx)
    SwDocShell *pDocShell = new SwDocShell( SFX_CREATE_MODE_STANDARD );
    xDocSh = pDocShell;
    xDocSh->DoInitNew( 0 );
    SfxViewFrame *pFrame = SFX_APP()->CreateViewFrame( *xDocSh, 0, TRUE );
    SwView *pView = (SwView*) pFrame->GetViewShell();
    pView->AttrChangedNotify( &pView->GetWrtShell() );//Damit SelectShell gerufen wird.

    xModel = pDocShell->GetModel();

    nDataCommandType    = sdb::CommandType::TABLE;
    nOutputType         = MailMergeType::PRINTER;
    bEscapeProcessing   = sal_True;     //!! allow to process properties like "Filter", "Order", ...
    bSinglePrintJobs    = sal_False;
    bFileNameFromColumn = sal_False;

    bDisposing = sal_False;
}

SwXMailMerge::~SwXMailMerge()
{
    DeleteTmpFile_Impl( xModel, xDocSh, aTmpFileName );
}

uno::Any SAL_CALL SwXMailMerge::execute(
        const uno::Sequence< beans::NamedValue >& rArguments )
    throw (IllegalArgumentException, Exception, RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex() );

    //
    // get property values to be used
    // (use values from the service as default and override them with
    // the values that are provided as arguments)
    //
    uno::Sequence< uno::Any >           aCurSelection   = aSelection;
    uno::Reference< sdbc::XResultSet >  xCurResultSet   = xResultSet;
    uno::Reference< sdbc::XConnection > xCurConnection  = xConnection;
    uno::Reference< frame::XModel >     xCurModel       = xModel;
    OUString   aCurDataSourceName       = aDataSourceName;
    OUString   aCurDataCommand          = aDataCommand;
    OUString   aCurFilter               = aFilter;
    OUString   aCurDocumentURL          = aDocumentURL;
    OUString   aCurOutputURL            = aOutputURL;
    OUString   aCurFileNamePrefix       = aFileNamePrefix;
    sal_Int32  nCurDataCommandType      = nDataCommandType;
    sal_Int16  nCurOutputType           = nOutputType;
    sal_Bool   bCurEscapeProcessing     = bEscapeProcessing;
    sal_Bool   bCurSinglePrintJobs      = bSinglePrintJobs;
    sal_Bool   bCurFileNameFromColumn   = bFileNameFromColumn;
    //
    SfxObjectShellRef xCurDocSh = xDocSh;   // the document
    //
    const beans::NamedValue *pArguments = rArguments.getConstArray();
    sal_Int32 nArgs = rArguments.getLength();
    for (sal_Int32 i = 0;  i < nArgs;  ++i)
    {
        const OUString &rName   = pArguments[i].Name;
        const Any &rValue       = pArguments[i].Value;

        BOOL bOK = TRUE;
        if (rName.equalsAscii( GetPropName( UNO_NAME_SELECTION ) ))
            bOK = rValue >>= aCurSelection;
        else if (rName.equalsAscii( GetPropName( UNO_NAME_RESULT_SET ) ))
            bOK = rValue >>= xCurResultSet;
        else if (rName.equalsAscii( GetPropName( UNO_NAME_CONNECTION ) ))
            bOK = rValue >>= xCurConnection;
        else if (rName.equalsAscii( GetPropName( UNO_NAME_MODEL ) ))
            throw PropertyVetoException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + rName, static_cast < cppu::OWeakObject * > ( this ) );
        else if (rName.equalsAscii( GetPropName( UNO_NAME_DATA_SOURCE_NAME ) ))
            bOK = rValue >>= aCurDataSourceName;
        else if (rName.equalsAscii( GetPropName( UNO_NAME_DAD_COMMAND ) ))
            bOK = rValue >>= aCurDataCommand;
        else if (rName.equalsAscii( GetPropName( UNO_NAME_FILTER ) ))
            bOK = rValue >>= aCurFilter;
        else if (rName.equalsAscii( GetPropName( UNO_NAME_DOCUMENT_URL ) ))
        {
            bOK = rValue >>= aCurDocumentURL;
            if (aCurDocumentURL.getLength()
                && !LoadFromURL_impl( xCurModel, xCurDocSh, aCurDocumentURL, FALSE ))
                throw RuntimeException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Failed to create document from URL: " ) ) + aCurDocumentURL, static_cast < cppu::OWeakObject * > ( this ) );
        }
        else if (rName.equalsAscii( GetPropName( UNO_NAME_OUTPUT_URL ) ))
        {
            bOK = rValue >>= aCurOutputURL;
            if (aCurOutputURL.getLength())
            {
                if (!UCB_IsDirectory(aCurOutputURL))
                    throw IllegalArgumentException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "URL does not point to a directory: " ) ) + aCurOutputURL, static_cast < cppu::OWeakObject * > ( this ), 0 );
                if (UCB_IsReadOnlyFileName(aCurOutputURL))
                    throw IllegalArgumentException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "URL is read-only: " ) ) + aCurOutputURL, static_cast < cppu::OWeakObject * > ( this ), 0 );
            }
        }
        else if (rName.equalsAscii( GetPropName( UNO_NAME_FILE_NAME_PREFIX ) ))
            bOK = rValue >>= aCurFileNamePrefix;
        else if (rName.equalsAscii( GetPropName( UNO_NAME_DAD_COMMAND_TYPE ) ))
            bOK = rValue >>= nCurDataCommandType;
        else if (rName.equalsAscii( GetPropName( UNO_NAME_OUTPUT_TYPE ) ))
            bOK = rValue >>= nCurOutputType;
        else if (rName.equalsAscii( GetPropName( UNO_NAME_ESCAPE_PROCESSING ) ))
            bOK = rValue >>= bCurEscapeProcessing;
        else if (rName.equalsAscii( GetPropName( UNO_NAME_SINGLE_PRINT_JOBS ) ))
            bOK = rValue >>= bCurSinglePrintJobs;
        else if (rName.equalsAscii( GetPropName( UNO_NAME_FILE_NAME_FROM_COLUMN ) ))
            bOK = rValue >>= bCurFileNameFromColumn;
        else
            throw UnknownPropertyException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is unknown: " ) ) + rName, static_cast < cppu::OWeakObject * > ( this ) );

        if (!bOK)
            throw IllegalArgumentException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property type mismatch or property not set: " ) ) + rName, static_cast < cppu::OWeakObject * > ( this ), 0 );
    }

    // need to translate the selection: the API here requires a sequence of bookmarks, but the MergeNew
    // method we will call below requires a sequence of indicies.
    if ( aCurSelection.getLength() )
    {
        Sequence< Any > aTranslated( aCurSelection.getLength() );

        sal_Bool bValid = sal_False;
        Reference< sdbcx::XRowLocate > xRowLocate( xCurResultSet, UNO_QUERY );
        if ( xRowLocate.is() )
        {

            const Any* pBookmarks = aCurSelection.getConstArray();
            const Any* pBookmarksEnd = pBookmarks + aCurSelection.getLength();
            Any* pTranslated = aTranslated.getArray();

            try
            {
                sal_Bool bEverythingsFine = sal_True;
                for ( ; ( pBookmarks != pBookmarksEnd ) && bEverythingsFine; ++pBookmarks )
                {
                    if ( xRowLocate->moveToBookmark( *pBookmarks ) )
                        *pTranslated <<= xCurResultSet->getRow();
                    else
                        bEverythingsFine = sal_False;
                }
                if ( bEverythingsFine )
                    bValid = sal_True;
            }
            catch( const Exception& )
            {
                bValid = sal_False;
            }
        }

        if ( !bValid )
        {
            throw IllegalArgumentException(
                OUString ( RTL_CONSTASCII_USTRINGPARAM ( "The current 'Selection' does not describe a valid array of bookmarks, relative to the current 'ResultSet'." ) ),
                static_cast < cppu::OWeakObject * > ( this ),
                0
            );
        }

        aCurSelection = aTranslated;
    }

    SfxViewFrame*   pFrame = SfxViewFrame::GetFirst( xCurDocSh, 0, FALSE);
    SwView *pView = PTR_CAST( SwView, pFrame->GetViewShell() );
    if (!pView)
        throw RuntimeException();
    SwWrtShell &rSh = *pView->GetWrtShellPtr();

    // avoid assertion in 'Update' from Sfx by supplying a shell
    // and thus avoiding the SelectShell call in Writers GetState function
    // while still in Update of Sfx.
    // (GetSelection in Update is not allowed)
    if (pView && aCurDocumentURL.getLength())
        pView->AttrChangedNotify( &pView->GetWrtShell() );//Damit SelectShell gerufen wird.

    DisposeHelper aRowSetDisposeHelper;
    if (!xCurResultSet.is())
    {
        if (!aCurDataSourceName.getLength() || !aCurDataCommand.getLength() )
        {
            DBG_ERROR("PropertyValues missing or unset")
            throw IllegalArgumentException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Either the ResultSet or DataSourceName and DataCommand must be set." ) ), static_cast < cppu::OWeakObject * > ( this ), 0 );
        }

        //
        // build ResultSet from DataSourceName, DataCommand and DataCommandType
        //
        Reference< XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );
        if (xMgr.is())
        {
            Reference< XInterface > xInstance = xMgr->createInstance(
                    C2U( "com.sun.star.sdb.RowSet" ));
            aRowSetDisposeHelper.SetInterface( xInstance );
            Reference< XPropertySet > xRowSetPropSet( xInstance, UNO_QUERY );
            DBG_ASSERT( xRowSetPropSet.is(), "failed to get XPropertySet interface from RowSet" );
            if (xRowSetPropSet.is())
            {
                if (xCurConnection.is())
                    xRowSetPropSet->setPropertyValue( C2U("ActiveConnection"),  makeAny( xCurConnection ) );
                xRowSetPropSet->setPropertyValue( C2U("DataSourceName"),    makeAny( aCurDataSourceName ) );
                xRowSetPropSet->setPropertyValue( C2U("Command"),           makeAny( aCurDataCommand ) );
                xRowSetPropSet->setPropertyValue( C2U("CommandType"),       makeAny( nCurDataCommandType ) );
                xRowSetPropSet->setPropertyValue( C2U("EscapeProcessing"),  makeAny( bCurEscapeProcessing ) );
                xRowSetPropSet->setPropertyValue( C2U("ApplyFilter"),       makeAny( sal_True ) );
                xRowSetPropSet->setPropertyValue( C2U("Filter"),            makeAny( aCurFilter ) );

                Reference< sdbc::XRowSet > xRowSet( xInstance, UNO_QUERY );
                if (xRowSet.is())
                    xRowSet->execute(); // build ResultSet from properties
                xCurResultSet = Reference< sdbc::XResultSet >( xRowSet, UNO_QUERY );
                DBG_ASSERT( xCurResultSet.is(), "failed to build ResultSet" );
            }
        }
    }

    svx::ODataAccessDescriptor aDescriptor;
    aDescriptor.setDataSource(aCurDataSourceName);
    aDescriptor[ svx::daConnection ]         <<= xCurConnection;
    aDescriptor[ svx::daCommand ]            <<= aCurDataCommand;
    aDescriptor[ svx::daCommandType ]        <<= nCurDataCommandType;
    aDescriptor[ svx::daEscapeProcessing ]   <<= bCurEscapeProcessing;
    aDescriptor[ svx::daCursor ]             <<= xCurResultSet;
    // aDescriptor[ svx::daColumnName ]      not used
    // aDescriptor[ svx::daColumnObject ]    not used
    aDescriptor[ svx::daSelection ]          <<= aCurSelection;

    USHORT nMergeType;
    switch (nCurOutputType)
    {
        case MailMergeType::PRINTER : nMergeType = DBMGR_MERGE_MAILMERGE; break;
        case MailMergeType::FILE    : nMergeType = DBMGR_MERGE_MAILFILES; break;
        default:
            throw IllegalArgumentException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Invalid value of property:" ) ) + C2U("OutputType"), static_cast < cppu::OWeakObject * > ( this ), 0 );
    }

    SwNewDBMgr* pMgr = rSh.GetNewDBMgr();
    //force layout creation
    rSh.CalcLayout();
    DBG_ASSERT( pMgr, "database manager missing" );
    if (MailMergeType::PRINTER == nCurOutputType)
    {
        SwPrintData aPrtData = *SW_MOD()->GetPrtOptions( FALSE );
        SwPrintData* pShellPrintData = rSh.GetPrintData();
        if (pShellPrintData)
            aPrtData = *pShellPrintData;
        aPrtData.SetPrintSingleJobs( bCurSinglePrintJobs );
        rSh.SetPrintData( aPrtData );
    }
    else if (MailMergeType::FILE == nCurOutputType)
    {
        INetURLObject aURLObj;
        aURLObj.SetSmartProtocol( INET_PROT_FILE );

        if (aCurDocumentURL.getLength())
        {
            // if OutputURL or FileNamePrefix are missing get
            // them from DocumentURL
            aURLObj.SetSmartURL( aCurDocumentURL );
            if (!aCurFileNamePrefix.getLength())
                aCurFileNamePrefix = aURLObj.GetBase(); // filename without extension
            if (!aCurOutputURL.getLength())
            {
                //aCurOutputURL = aURLObj.GetURLPath();
                aURLObj.removeSegment();
                aCurOutputURL = aURLObj.GetMainURL( INetURLObject::DECODE_TO_IURI );
            }
        }
        else    // default empty document without URL
        {
            if (!aCurOutputURL.getLength())
                throw RuntimeException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "OutputURL is not set and can not be obtained." ) ), static_cast < cppu::OWeakObject * > ( this ) );
        }

        aURLObj.SetSmartURL( aCurOutputURL );
        String aPath = aURLObj.GetMainURL( INetURLObject::DECODE_TO_IURI );

        String aDelim( INET_PATH_TOKEN );
        if (aPath.Len() >= aDelim.Len() &&
            aPath.Copy( aPath.Len()-aDelim.Len() ).CompareTo( aDelim ) != COMPARE_EQUAL)
            aPath += aDelim;
        if (bCurFileNameFromColumn)
            pMgr->SetEMailColumn( aCurFileNamePrefix );
        else
        {
            aPath += String( aCurFileNamePrefix );
            pMgr->SetEMailColumn( String() );
        }
        pMgr->SetSubject( aPath );
    }

    // save document with temporary filename
    const SfxFilter *pSfxFlt = SwIoSystem::GetFilterOfFormat(
            String::CreateFromAscii( FILTER_XML ),
            SwDocShell::Factory().GetFilterContainer() );
    String aExtension( pSfxFlt->GetDefaultExtension() );
    aExtension.EraseLeadingChars( '*' );
    TempFile aTempFile( C2U("SwMM"), &aExtension );
    aTmpFileName = aTempFile.GetName();

    Reference< XStorable > xStorable( xCurModel, UNO_QUERY );
    sal_Bool bStoredAsTemporary = sal_False;
    if ( xStorable.is() )
    {
        try
        {
            xStorable->storeAsURL( aTmpFileName, Sequence< PropertyValue >() );
            bStoredAsTemporary = sal_True;
        }
        catch( const Exception& )
        {
        }
    }
    if ( !bStoredAsTemporary )
        throw RuntimeException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Failed to save temporary file." ) ), static_cast < cppu::OWeakObject * > ( this ) );

    pMgr->SetMergeSilent( TRUE );       // suppress dialogs, message boxes, etc.
    const SwXMailMerge *pOldSrc = pMgr->GetMailMergeEvtSrc();
    DBG_ASSERT( !pOldSrc || pOldSrc == this, "Ooops... different event source already set." );
    pMgr->SetMailMergeEvtSrc( this );   // launch events for listeners
    BOOL bSucc = pMgr->MergeNew( nMergeType, rSh, aDescriptor );
    pMgr->SetMailMergeEvtSrc( pOldSrc );

    if ( xCurModel.get() != xModel.get() )
    {   // in case it was a temporary model -> close it, and delete the file
        DeleteTmpFile_Impl( xCurModel, xCurDocSh, aTmpFileName );
        aTmpFileName.Erase();
    }
    // (in case it wasn't a temporary model, it will be closed in the dtor, at the latest)

    if (!bSucc)
        throw Exception( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Mail merge failed. Sorry, no further information available." ) ), static_cast < cppu::OWeakObject * > ( this ) );

    return makeAny( sal_True );
}

void SwXMailMerge::LaunchMailMergeEvent( const MailMergeEvent &rEvt ) const
{
    cppu::OInterfaceIteratorHelper aIt( ((SwXMailMerge *) this)->aMergeListeners );
    while (aIt.hasMoreElements())
    {
        Reference< XMailMergeListener > xRef( aIt.next(), UNO_QUERY );
        if (xRef.is())
            xRef->notifyMailMergeEvent( rEvt );
    }
}

void SwXMailMerge::launchEvent( const PropertyChangeEvent &rEvt ) const
{
    cppu::OInterfaceContainerHelper *pContainer =
            aPropListeners.getContainer( rEvt.PropertyHandle );
    if (pContainer)
    {
        cppu::OInterfaceIteratorHelper aIt( *pContainer );
        while (aIt.hasMoreElements())
        {
            Reference< XPropertyChangeListener > xRef( aIt.next(), UNO_QUERY );
            if (xRef.is())
                xRef->propertyChange( rEvt );
        }
    }
}


uno::Reference< beans::XPropertySetInfo > SAL_CALL SwXMailMerge::getPropertySetInfo(  )
    throw (RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex() );
    static Reference< XPropertySetInfo > aRef = new SfxItemPropertySetInfo( pMap );
    return aRef;
}

void SAL_CALL SwXMailMerge::setPropertyValue(
        const OUString& rPropertyName, const uno::Any& rValue )
    throw (UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex() );

    const SfxItemPropertyMap* pCur =
            SfxItemPropertyMap::GetByName( pMap, rPropertyName );
    if (!pCur)
        throw UnknownPropertyException();
    else if (pCur->nFlags & PropertyAttribute::READONLY)
        throw PropertyVetoException();
    else
    {
        void *pData = NULL;
        const com::sun::star::uno::Type* pType = pCur->pType;
        switch (pCur->nWID)
        {
            case WID_SELECTION :                pData = &aSelection;  break;
            case WID_RESULT_SET :               pData = &xResultSet;  break;
            case WID_CONNECTION :               pData = &xConnection;  break;
            case WID_MODEL :                    pData = &xModel;  break;
            case WID_DATA_SOURCE_NAME :         pData = &aDataSourceName;  break;
            case WID_DATA_COMMAND :             pData = &aDataCommand;  break;
            case WID_FILTER :                   pData = &aFilter;  break;
            case WID_DOCUMENT_URL :             pData = &aDocumentURL;  break;
            case WID_OUTPUT_URL :               pData = &aOutputURL;  break;
            case WID_DATA_COMMAND_TYPE :        pData = &nDataCommandType;  break;
            case WID_OUTPUT_TYPE :              pData = &nOutputType;  break;
            case WID_ESCAPE_PROCESSING :        pData = &bEscapeProcessing;  break;
            case WID_SINGLE_PRINT_JOBS :        pData = &bSinglePrintJobs;  break;
            case WID_FILE_NAME_FROM_COLUMN :    pData = &bFileNameFromColumn;  break;
            case WID_FILE_NAME_PREFIX :         pData = &aFileNamePrefix;  break;
            default :
                DBG_ERROR("unknown WID");
        }
        Any aOld( pData, *pType );

        sal_Bool bChanged = sal_False;
        sal_Bool bOK = sal_True;
        if (aOld != rValue)
        {
            if (pData == &aSelection)
                bOK = rValue >>= aSelection;
            else if (pData == &xResultSet)
                bOK = rValue >>= xResultSet;
            else if (pData == &xConnection)
                bOK = rValue >>= xConnection;
            else if (pData == &xModel)
                bOK = rValue >>= xModel;
            else if (pData == &aDataSourceName)
                bOK = rValue >>= aDataSourceName;
            else if (pData == &aDataCommand)
                bOK = rValue >>= aDataCommand;
            else if (pData == &aFilter)
                bOK = rValue >>= aFilter;
            else if (pData == &aDocumentURL)
            {
                OUString aText;
                bOK = rValue >>= aText;
                if (aText.getLength()
                    && !LoadFromURL_impl( xModel, xDocSh, aText, TRUE ))
                    throw RuntimeException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Failed to create document from URL: " ) ) + aText, static_cast < cppu::OWeakObject * > ( this ) );
                aDocumentURL = aText;
            }
            else if (pData == &aOutputURL)
            {
                OUString aText;
                bOK = rValue >>= aText;
                if (aText.getLength())
                {
                    if (!UCB_IsDirectory(aText))
                        throw IllegalArgumentException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "URL does not point to a directory: " ) ) + aText, static_cast < cppu::OWeakObject * > ( this ), 0 );
                    if (UCB_IsReadOnlyFileName(aText))
                        throw IllegalArgumentException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "URL is read-only: " ) ) + aText, static_cast < cppu::OWeakObject * > ( this ), 0 );
                }
                aOutputURL = aText;
            }
            else if (pData == &nDataCommandType)
                bOK = rValue >>= nDataCommandType;
            else if (pData == &nOutputType)
                bOK = rValue >>= nOutputType;
            else if (pData == &bEscapeProcessing)
                bOK = rValue >>= bEscapeProcessing;
            else if (pData == &bSinglePrintJobs)
                bOK = rValue >>= bSinglePrintJobs;
            else if (pData == &bFileNameFromColumn)
                bOK = rValue >>= bFileNameFromColumn;
            else if (pData == &aFileNamePrefix)
                bOK = rValue >>= aFileNamePrefix;
            else
                DBG_ERROR( "invalid pointer" );
            DBG_ASSERT( bOK, "set value failed" );
            bChanged = sal_True;
        }
        if (!bOK)
            throw IllegalArgumentException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property type mismatch or property not set: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ), 0 );

        if (bChanged)
        {
            PropertyChangeEvent aChgEvt( (XPropertySet *) this, rPropertyName,
                    FALSE, pCur->nWID, aOld, rValue );
            launchEvent( aChgEvt );
        }
    }
}

uno::Any SAL_CALL SwXMailMerge::getPropertyValue(
        const OUString& rPropertyName )
    throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex() );

    Any aRet;

    const SfxItemPropertyMap* pCur = SfxItemPropertyMap::GetByName( pMap, rPropertyName );
    if (!pCur)
        throw UnknownPropertyException();
    else
    {
        switch (pCur->nWID)
        {
            case WID_SELECTION :                aRet <<= aSelection;  break;
            case WID_RESULT_SET :               aRet <<= xResultSet;  break;
            case WID_CONNECTION :               aRet <<= xConnection;  break;
            case WID_MODEL :                    aRet <<= xModel;  break;
            case WID_DATA_SOURCE_NAME :         aRet <<= aDataSourceName;  break;
            case WID_DATA_COMMAND :             aRet <<= aDataCommand;  break;
            case WID_FILTER :                   aRet <<= aFilter;  break;
            case WID_DOCUMENT_URL :             aRet <<= aDocumentURL;  break;
            case WID_OUTPUT_URL :               aRet <<= aOutputURL;  break;
            case WID_DATA_COMMAND_TYPE :        aRet <<= nDataCommandType;  break;
            case WID_OUTPUT_TYPE :              aRet <<= nOutputType;  break;
            case WID_ESCAPE_PROCESSING :        aRet <<= bEscapeProcessing;  break;
            case WID_SINGLE_PRINT_JOBS :        aRet <<= bSinglePrintJobs;  break;
            case WID_FILE_NAME_FROM_COLUMN :    aRet <<= bFileNameFromColumn;  break;
            case WID_FILE_NAME_PREFIX :         aRet <<= aFileNamePrefix;  break;
            default :
                DBG_ERROR("unknown WID");
        }
    }

    return aRet;
}

void SAL_CALL SwXMailMerge::addPropertyChangeListener(
        const OUString& rPropertyName,
        const uno::Reference< beans::XPropertyChangeListener >& rListener )
    throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex() );
    if (!bDisposing && rListener.is())
    {
        const SfxItemPropertyMap* pCur =
                SfxItemPropertyMap::GetByName( pMap, rPropertyName );
        if (pCur)
            aPropListeners.addInterface( pCur->nWID, rListener );
        else
            throw UnknownPropertyException();
    }
}

void SAL_CALL SwXMailMerge::removePropertyChangeListener(
        const OUString& rPropertyName,
        const uno::Reference< beans::XPropertyChangeListener >& rListener )
    throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex() );
    if (!bDisposing && rListener.is())
    {
        const SfxItemPropertyMap* pCur =
                SfxItemPropertyMap::GetByName( pMap, rPropertyName );
        if (pCur)
            aPropListeners.removeInterface( pCur->nWID, rListener );
        else
            throw UnknownPropertyException();
    }
}

void SAL_CALL SwXMailMerge::addVetoableChangeListener(
        const OUString& rPropertyName,
        const uno::Reference< beans::XVetoableChangeListener >& rListener )
    throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    // no vetoable property, thus no support for vetoable change listeners
    DBG_WARNING( "not implemented");
}

void SAL_CALL SwXMailMerge::removeVetoableChangeListener(
        const OUString& rPropertyName,
        const uno::Reference< beans::XVetoableChangeListener >& rListener )
    throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    // no vetoable property, thus no support for vetoable change listeners
    DBG_WARNING( "not implemented");
}


void SAL_CALL SwXMailMerge::dispose()
    throw(RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex() );

    if (!bDisposing)
    {
        bDisposing = sal_True;

        EventObject aEvtObj( (XPropertySet *) this );
        aEvtListeners.disposeAndClear( aEvtObj );
        aMergeListeners.disposeAndClear( aEvtObj );
        aPropListeners.disposeAndClear( aEvtObj );
    }
}

void SAL_CALL SwXMailMerge::addEventListener(
        const Reference< XEventListener >& rxListener )
    throw(RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex() );
    if (!bDisposing && rxListener.is())
        aEvtListeners.addInterface( rxListener );
}

void SAL_CALL SwXMailMerge::removeEventListener(
        const Reference< XEventListener >& rxListener )
    throw(RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex() );
    if (!bDisposing && rxListener.is())
        aEvtListeners.removeInterface( rxListener );
}

void SAL_CALL SwXMailMerge::addMailMergeEventListener(
        const uno::Reference< XMailMergeListener >& rxListener )
    throw (RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex() );
    if (!bDisposing && rxListener.is())
        aMergeListeners.addInterface( rxListener );
}

void SAL_CALL SwXMailMerge::removeMailMergeEventListener(
        const uno::Reference< XMailMergeListener >& rxListener )
    throw (RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex() );
    if (!bDisposing && rxListener.is())
        aMergeListeners.removeInterface( rxListener );
}

OUString SAL_CALL SwXMailMerge::getImplementationName()
    throw(RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex() );
    return SwXMailMerge_getImplementationName();
}

sal_Bool SAL_CALL SwXMailMerge::supportsService( const OUString& rServiceName )
    throw(RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex() );
    return C2U( SN_MAIL_MERGE ) == rServiceName ||
           C2U( SN_DATA_ACCESS_DESCRIPTOR ) == rServiceName;
}

uno::Sequence< OUString > SAL_CALL SwXMailMerge::getSupportedServiceNames()
    throw(RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex() );
    return SwXMailMerge_getSupportedServiceNames();
}

////////////////////////////////////////////////////////////

uno::Sequence< OUString > SAL_CALL SwXMailMerge_getSupportedServiceNames()
    throw()
{
    uno::Sequence< OUString > aNames(2);
    OUString *pName = aNames.getArray();
    pName[0] = C2U( SN_MAIL_MERGE );
    pName[1] = C2U( SN_DATA_ACCESS_DESCRIPTOR );
    return aNames;
}

OUString SAL_CALL SwXMailMerge_getImplementationName()
    throw()
{
    return OUString( C2U( "SwXMailMerge" ) );
}

uno::Reference< uno::XInterface > SAL_CALL SwXMailMerge_createInstance(
        const uno::Reference< XMultiServiceFactory > & rSMgr)
    throw( uno::Exception )
{
    vos::OGuard aGuard( Application::GetSolarMutex() );

    //the module may not be loaded
    SwDLL::Init();
    uno::Reference< uno::XInterface > xRef = (cppu::OWeakObject *) new SwXMailMerge();
    return xRef;
}

