/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CommandDispatchContainer.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:06:17 $
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
#include "precompiled_chart2.hxx"

#include "CommandDispatchContainer.hxx"
#include "UndoCommandDispatch.hxx"
#include "StatusBarCommandDispatch.hxx"
#include "DisposeHelper.hxx"
#include "macros.hxx"

#include <comphelper/InlineContainer.hxx>

#include <com/sun/star/frame/XDispatchProvider.hpp>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace chart
{

CommandDispatchContainer::CommandDispatchContainer(
    const Reference< uno::XComponentContext > & xContext ) :
        m_xContext( xContext )
{
    m_aContainerDocumentCommands =
        ::comphelper::MakeSet< OUString >
        ( C2U("AddDirect"))    ( C2U("NewDoc"))             ( C2U("Open"))
        ( C2U("Save"))         ( C2U("SaveAs"))             ( C2U("SendMail"))
        ( C2U("EditDoc"))      ( C2U("ExportDirectToPDF"))  ( C2U("PrintDefault"))
        ;
}

void CommandDispatchContainer::setModel(
    const Reference< frame::XModel > & xModel )
{
    // remove all existing dispatcher that base on the old model
    DisposeHelper::DisposeAllMapElements( m_aCachedDispatches );
    m_aCachedDispatches.clear();
    m_xModel.set( xModel );
}

void CommandDispatchContainer::setUndoManager( UndoManager * pUndoManager )
{
    m_pUndoManager = pUndoManager;
}

void CommandDispatchContainer::setFallbackDispatch(
    const Reference< frame::XDispatch > xFallbackDispatch,
    const ::std::set< OUString > & rFallbackCommands )
{
    m_xFallbackDispatcher.set( xFallbackDispatch );
    m_aFallbackCommands = rFallbackCommands;
}

Reference< frame::XDispatch > CommandDispatchContainer::getDispatchForURL(
    const util::URL & rURL )
{
    Reference< frame::XDispatch > xResult;

    tDispatchMap::const_iterator aIt( m_aCachedDispatches.find( rURL.Complete ));
    if( aIt != m_aCachedDispatches.end())
    {
        xResult.set( (*aIt).second );
    }
    else
    {
        if( rURL.Path.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Undo" ))
            || rURL.Path.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Redo" )))
        {
            CommandDispatch * pDispatch = new UndoCommandDispatch( m_xContext, m_pUndoManager, m_xModel );
            xResult.set( pDispatch );
            pDispatch->initialize();
            m_aCachedDispatches[ C2U(".uno:Undo") ].set( xResult );
            m_aCachedDispatches[ C2U(".uno:Redo") ].set( xResult );
        }
        else if( rURL.Path.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Context" ))
                 || rURL.Path.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ModifiedStatus" )))
        {
            Reference< view::XSelectionSupplier > xSelSupp;
            if( m_xModel.is())
                xSelSupp.set( m_xModel->getCurrentController(), uno::UNO_QUERY );
            CommandDispatch * pDispatch = new StatusBarCommandDispatch( m_xContext, m_xModel, xSelSupp );
            xResult.set( pDispatch );
            pDispatch->initialize();
            m_aCachedDispatches[ C2U(".uno:Context") ].set( xResult );
            m_aCachedDispatches[ C2U(".uno:ModifiedStatus") ].set( xResult );
        }
        else if( m_xModel.is() &&
                 (m_aContainerDocumentCommands.find( rURL.Path ) != m_aContainerDocumentCommands.end()) )
        {
            xResult.set( getContainerDispatchForURL( m_xModel->getCurrentController(), rURL ));
            // ToDo: can those dispatches be cached?
            m_aCachedDispatches[ rURL.Complete ].set( xResult );
        }
        else if( m_xFallbackDispatcher.is() &&
                 (m_aFallbackCommands.find( rURL.Path ) != m_aFallbackCommands.end()) )
        {
            xResult.set( m_xFallbackDispatcher );
            m_aCachedDispatches[ rURL.Complete ].set( xResult );
        }
    }

    return xResult;
}

Sequence< Reference< frame::XDispatch > > CommandDispatchContainer::getDispatchesForURLs(
    const Sequence< frame::DispatchDescriptor > & aDescriptors )
{
    sal_Int32 nCount = aDescriptors.getLength();
    uno::Sequence< uno::Reference< frame::XDispatch > > aRet( nCount );

    for( sal_Int32 nPos = 0; nPos < nCount; ++nPos )
    {
        if( aDescriptors[ nPos ].FrameName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("_self")))
            aRet[ nPos ] = getDispatchForURL( aDescriptors[ nPos ].FeatureURL );
    }
    return aRet;
}

void CommandDispatchContainer::DisposeAndClear()
{
    DisposeHelper::DisposeAllMapElements( m_aCachedDispatches );
    m_aCachedDispatches.clear();
    m_xFallbackDispatcher.clear();
    m_aFallbackCommands.clear();
}

Reference< frame::XDispatch > CommandDispatchContainer::getContainerDispatchForURL(
    const Reference< frame::XController > & xChartController,
    const util::URL & rURL )
{
    Reference< frame::XDispatch > xResult;
    if( xChartController.is())
    {
        Reference< frame::XFrame > xFrame( xChartController->getFrame());
        if( xFrame.is())
        {
            Reference< frame::XDispatchProvider > xDispProv( xFrame->getCreator(), uno::UNO_QUERY );
            if( xDispProv.is())
                xResult.set( xDispProv->queryDispatch( rURL, C2U("_self"), 0 ));
        }
    }
    return xResult;
}

} //  namespace chart
