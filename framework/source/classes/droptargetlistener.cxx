/*************************************************************************
 *
 *  $RCSfile: droptargetlistener.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 18:21:29 $
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

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_CLASSES_DROPTARGETLISTENER_HXX_
#include <classes/droptargetlistener.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_READGUARD_HXX_
#include <threadhelp/readguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_
#include <threadhelp/writeguard.hxx>
#endif

#ifndef __FRAMEWORK_TARGETS_H_
#include <targets.h>
#endif

#ifndef __FRAMEWORK_SERVICES_H_
#include <services.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_DNDCONSTANTS_HPP_
#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif

#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif

#ifndef _FILELIST_HXX
#include <sot/filelist.hxx>
#endif

#include <osl/file.hxx>

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework
{

DropTargetListener::DropTargetListener( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory,
                                        const css::uno::Reference< css::frame::XFrame >&              xFrame  )
        : ThreadHelpBase  ( &Application::GetSolarMutex() )
        , m_xFactory      ( xFactory                      )
        , m_xTargetFrame  ( xFrame                        )
        , m_pFormats      ( new DataFlavorExVector        )
{
}

// -----------------------------------------------------------------------------

DropTargetListener::~DropTargetListener()
{
    m_xTargetFrame = css::uno::WeakReference< css::frame::XFrame >();
    m_xFactory     = css::uno::Reference< css::lang::XMultiServiceFactory >();
    delete m_pFormats;
    m_pFormats = NULL;
}

// -----------------------------------------------------------------------------

void SAL_CALL DropTargetListener::disposing( const css::lang::EventObject& rSource ) throw( css::uno::RuntimeException )
{
    m_xTargetFrame = css::uno::WeakReference< css::frame::XFrame >();
    m_xFactory     = css::uno::Reference< css::lang::XMultiServiceFactory >();
}

// -----------------------------------------------------------------------------

void SAL_CALL DropTargetListener::drop( const css::datatransfer::dnd::DropTargetDropEvent& dtde ) throw( css::uno::RuntimeException )
{
    const sal_Int8 nAction = dtde.DropAction;

    try
    {
        if ( css::datatransfer::dnd::DNDConstants::ACTION_NONE != nAction )
        {
            TransferableDataHelper aHelper( dtde.Transferable );
            sal_uInt32 nFormatCount = aHelper.GetFormatCount();
            sal_Bool bFormatFound = sal_False;
            FileList aFileList;

            // at first check filelist format
            if ( aHelper.GetFileList( SOT_FORMAT_FILE_LIST, aFileList ) )
            {
                ULONG i, nCount = aFileList.Count();
                for ( i = 0; i < nCount; ++i )
                    implts_OpenFile( aFileList.GetFile(i) );
                bFormatFound = sal_True;
            }

            // then, if necessary, the file format
            String aFilePath;
            if ( !bFormatFound && aHelper.GetString( SOT_FORMAT_FILE, aFilePath ) )
                implts_OpenFile( aFilePath );
        }
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
    }
}

// -----------------------------------------------------------------------------

void SAL_CALL DropTargetListener::dragEnter( const css::datatransfer::dnd::DropTargetDragEnterEvent& dtdee ) throw( css::uno::RuntimeException )
{
    try
    {
        implts_BeginDrag( dtdee.SupportedDataFlavors );
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
    }

    dragOver( dtdee );
}

// -----------------------------------------------------------------------------

void SAL_CALL DropTargetListener::dragExit( const css::datatransfer::dnd::DropTargetEvent& dte ) throw( css::uno::RuntimeException )
{
    try
    {
        implts_EndDrag();
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
    }
}

// -----------------------------------------------------------------------------

void SAL_CALL DropTargetListener::dragOver( const css::datatransfer::dnd::DropTargetDragEvent& dtde ) throw( css::uno::RuntimeException )
{
    try
    {
        sal_Bool bAccept = ( implts_IsDropFormatSupported( SOT_FORMAT_FILE ) ||
                             implts_IsDropFormatSupported( SOT_FORMAT_FILE_LIST ) );

        if ( !bAccept )
            dtde.Context->rejectDrag();
        else
            dtde.Context->acceptDrag( css::datatransfer::dnd::DNDConstants::ACTION_COPY );
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
    }
}

// -----------------------------------------------------------------------------

void SAL_CALL DropTargetListener::dropActionChanged( const css::datatransfer::dnd::DropTargetDragEvent& dtde ) throw( css::uno::RuntimeException )
{
}

void DropTargetListener::implts_BeginDrag( const css::uno::Sequence< css::datatransfer::DataFlavor >& rSupportedDataFlavors )
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);
    m_pFormats->clear();
    TransferableDataHelper::FillDataFlavorExVector(rSupportedDataFlavors,*m_pFormats);
    aWriteLock.unlock();
    /* } SAFE */
}

void DropTargetListener::implts_EndDrag()
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);
    m_pFormats->clear();
    aWriteLock.unlock();
    /* } SAFE */
}

sal_Bool DropTargetListener::implts_IsDropFormatSupported( SotFormatStringId nFormat )
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);
    DataFlavorExVector::iterator aIter( m_pFormats->begin() ), aEnd( m_pFormats->end() );
    sal_Bool bRet = sal_False;

    while ( aIter != aEnd )
    {
        if ( nFormat == (*aIter++).mnSotId )
        {
            bRet = sal_True;
            aIter = aEnd;
        }
    }
    aReadLock.unlock();
    /* } SAFE */

    return bRet;
}

void DropTargetListener::implts_OpenFile( const String& rFilePath )
{
    String aFileURL;
    if ( !::utl::LocalFileHelper::ConvertPhysicalNameToURL( rFilePath, aFileURL ) )
        aFileURL = rFilePath;

    ::osl::FileStatus aStatus( FileStatusMask_FileURL );
    ::osl::DirectoryItem aItem;
    if( ::osl::FileBase::E_None == ::osl::DirectoryItem::get( aFileURL, aItem ) &&
        ::osl::FileBase::E_None == aItem.getFileStatus( aStatus ) )
            aFileURL = aStatus.getFileURL();

    // open file
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::frame::XFrame >         xTargetFrame( m_xTargetFrame.get()                                  , css::uno::UNO_QUERY );
    css::uno::Reference< css::util::XURLTransformer > xParser     ( m_xFactory->createInstance(SERVICENAME_URLTRANSFORMER), css::uno::UNO_QUERY );
    aReadLock.unlock();
    /* } SAFE */
    if (xTargetFrame.is() && xParser.is())
    {
        css::util::URL aURL;
        aURL.Complete = aFileURL;
        xParser->parseStrict(aURL);

        css::uno::Reference < css::frame::XDispatchProvider > xProvider( xTargetFrame, css::uno::UNO_QUERY );
        css::uno::Reference< css::frame::XDispatch > xDispatcher = xProvider->queryDispatch( aURL, SPECIALTARGET_DEFAULT, 0 );
        if ( xDispatcher.is() )
            xDispatcher->dispatch( aURL, css::uno::Sequence < css::beans::PropertyValue >() );
    }
}

}

