/*************************************************************************
 *
 *  $RCSfile: transfer2.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ka $ $Date: 2001-02-27 11:06:47 $
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

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif
#ifndef _SOT_EXCHANGE_HXX
#include <sot/exchange.hxx>
#endif
#ifndef _SOT_STORAGE_HXX
#include <sot/storage.hxx>
#endif
#ifndef _SV_BITMAP_HXX
#include <vcl/bitmap.hxx>
#endif
#ifndef _SV_GDIMTF_HXX
#include <vcl/gdimtf.hxx>
#endif
#ifndef _SV_GRAPH_HXX
#include <vcl/graph.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#include "transfer.hxx"

// --------------
// - Namespaces -
// --------------

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::clipboard;
using namespace ::com::sun::star::datatransfer::dnd;

// ----------------------------------------
// - DropTargetHelper::DropTargetListener -
// ----------------------------------------

DropTargetHelper::DropTargetListener::DropTargetListener( DropTargetHelper& rDropTargetHelper ) :
    mrParent( rDropTargetHelper )
{
}

// -----------------------------------------------------------------------------

DropTargetHelper::DropTargetListener::~DropTargetListener()
{
}

// -----------------------------------------------------------------------------

void SAL_CALL DropTargetHelper::DropTargetListener::disposing( const EventObject& rSource ) throw( RuntimeException )
{
}

// -----------------------------------------------------------------------------

void SAL_CALL DropTargetHelper::DropTargetListener::drop( const DropTargetDropEvent& rDTDE ) throw( RuntimeException )
{
    Application::GetSolarMutex().acquire();
    const sal_Int8 nRet = mrParent.ExecuteDrop( rDTDE.DropAction, Point( rDTDE.LocationX, rDTDE.LocationY ),
                                                rDTDE.SourceActions, rDTDE.Transferable );

    if( DNDConstants::ACTION_NONE == nRet )
        rDTDE.Context->rejectDrop();
    else
        rDTDE.Context->acceptDrop( nRet );

    rDTDE.Context->dropComplete( DNDConstants::ACTION_NONE != nRet );
    Application::GetSolarMutex().release();
}

// -----------------------------------------------------------------------------

void SAL_CALL DropTargetHelper::DropTargetListener::dragEnter( const DropTargetDragEnterEvent& rDTDEE ) throw( RuntimeException )
{
    Application::GetSolarMutex().acquire();
    mrParent.ImplBeginDrag( rDTDEE.SupportedDataFlavors );
    Application::GetSolarMutex().release();
}

// -----------------------------------------------------------------------------

void SAL_CALL DropTargetHelper::DropTargetListener::dragExit( const DropTargetEvent& dte ) throw( RuntimeException )
{
    Application::GetSolarMutex().acquire();
    mrParent.ImplEndDrag();
    Application::GetSolarMutex().release();
}

// -----------------------------------------------------------------------------

void SAL_CALL DropTargetHelper::DropTargetListener::dragOver( const DropTargetDragEvent& rDTDE ) throw( RuntimeException )
{
    Application::GetSolarMutex().acquire();
    const sal_Int8 nRet = mrParent.AcceptDrop( rDTDE.DropAction, Point( rDTDE.LocationX, rDTDE.LocationY ),
                                               rDTDE.SourceActions );

    if( DNDConstants::ACTION_NONE == nRet )
        rDTDE.Context->rejectDrag();
    else
        rDTDE.Context->acceptDrag( nRet );
    Application::GetSolarMutex().release();
}

// -----------------------------------------------------------------------------

void SAL_CALL DropTargetHelper::DropTargetListener::dropActionChanged( const DropTargetDragEvent& dtde ) throw( RuntimeException )
{
}

// --------------------
// - DropTargetHelper -
// --------------------

DropTargetHelper::DropTargetHelper( Window& rWindow ) :
    mxDropTarget( rWindow.GetDropTarget() )
{
    ImplConstruct();
}

// -----------------------------------------------------------------------------

DropTargetHelper::DropTargetHelper( const Reference< XDropTarget >& rxDropTarget ) :
    mxDropTarget( rxDropTarget )
{
    ImplConstruct();
}

// -----------------------------------------------------------------------------

DropTargetHelper::~DropTargetHelper()
{
    if( mxDropTarget.is() )
        mxDropTarget->removeDropTargetListener( mxDropTargetListener );
}

// -----------------------------------------------------------------------------

void DropTargetHelper::ImplConstruct()
{
    if( mxDropTarget.is() )
    {
        mxDropTargetListener = new DropTargetHelper::DropTargetListener( *this );
        mxDropTarget->addDropTargetListener( mxDropTargetListener );
        mxDropTarget->setActive( sal_True );
    }
}

// -----------------------------------------------------------------------------

void DropTargetHelper::ImplBeginDrag( const Sequence< DataFlavor >& rSupportedDataFlavors )
{
    DataFlavorEx        aFlavorEx;
    const DataFlavor*   pFlavor = rSupportedDataFlavors.getConstArray();

    maDragFormats.clear();

    for( sal_uInt32 i = 0, nCount = rSupportedDataFlavors.getLength(); i < nCount; i++, pFlavor++ )
    {
        aFlavorEx.MimeType = pFlavor->MimeType;
        aFlavorEx.HumanPresentableName = pFlavor->HumanPresentableName;
        aFlavorEx.DataType = pFlavor->DataType;
        aFlavorEx.mnSotId = SotExchange::RegisterFormat( *pFlavor );

        maDragFormats.push_back( aFlavorEx );
    }
}

// -----------------------------------------------------------------------------

void DropTargetHelper::ImplEndDrag()
{
    maDragFormats.clear();
}

// -----------------------------------------------------------------------------

sal_Int8 DropTargetHelper::AcceptDrop( sal_Int8 nDropAction, const Point& rPointerPos, sal_Int8 nDnDSourceActions )
{
    return( DNDConstants::ACTION_NONE );
}

// -----------------------------------------------------------------------------

sal_Int8 DropTargetHelper::ExecuteDrop( sal_Int8 nDropAction, const Point& rPointerPos, sal_Int8 nDnDSourceActions,
                                        const Reference< XTransferable >& rxTransferable )
{
    return( DNDConstants::ACTION_NONE );
}

// -----------------------------------------------------------------------------

sal_Bool DropTargetHelper::IsDropFormatSupported( SotFormatStringId nFormat )
{
    DataFlavorExList::iterator  aIter( maDragFormats.begin() ), aEnd( maDragFormats.end() );
    sal_Bool                    bRet = sal_False;

    while( aIter != aEnd )
    {
        if( nFormat == (*aIter++).mnSotId )
        {
            bRet = sal_True;
            aIter = aEnd;
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool DropTargetHelper::IsDropFormatSupported( const DataFlavor& rFlavor )
{
    DataFlavorExList::iterator  aIter( maDragFormats.begin() ), aEnd( maDragFormats.end() );
    sal_Bool                    bRet = sal_False;

    while( aIter != aEnd )
    {
        if( TransferableDataHelper::IsEqual( rFlavor, *aIter++ ) )
        {
            bRet = sal_True;
            aIter = aEnd;
        }
    }

    return bRet;
}
