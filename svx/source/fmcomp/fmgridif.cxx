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


#include "svx/fmgridif.hxx"
#include "fmprop.hrc"
#include "fmservs.hxx"
#include "svx/fmtools.hxx"
#include "fmurl.hxx"
#include "formcontrolfactory.hxx"
#include "gridcell.hxx"
#include "sdbdatacolumn.hxx"
#include "svx/fmgridcl.hxx"
#include "svx/svxids.hrc"
#include <tools/urlobj.hxx>

#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/form/XFormComponent.hpp>
#include <com/sun/star/form/XLoadable.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/sdbcx/XRowLocate.hpp>

#include <comphelper/container.hxx>
#include <comphelper/enumhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/property.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/types.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/diagnose_ex.h>
#include <sal/macros.h>

using namespace ::svxform;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star;

using ::com::sun::star::sdbcx::XColumnsSupplier;
using ::com::sun::star::frame::XDispatchProviderInterceptor;
using ::com::sun::star::frame::XDispatchProvider;
using ::com::sun::star::accessibility::XAccessible;
using ::com::sun::star::accessibility::XAccessibleContext;
using ::com::sun::star::sdb::XRowSetSupplier;
using ::com::sun::star::awt::XVclWindowPeer;


css::awt::FontDescriptor ImplCreateFontDescriptor( const vcl::Font& rFont )
{
    css::awt::FontDescriptor aFD;
    aFD.Name = rFont.GetFamilyName();
    aFD.StyleName = rFont.GetStyleName();
    aFD.Height = (sal_Int16)rFont.GetFontSize().Height();
    aFD.Width = (sal_Int16)rFont.GetFontSize().Width();
    aFD.Family = (sal_Int16)rFont.GetFamilyType();
    aFD.CharSet = rFont.GetCharSet();
    aFD.Pitch = (sal_Int16)rFont.GetPitch();
    aFD.CharacterWidth = VCLUnoHelper::ConvertFontWidth( rFont.GetWidthType() );
    aFD.Weight= VCLUnoHelper::ConvertFontWeight( rFont.GetWeight() );
    aFD.Slant = VCLUnoHelper::ConvertFontSlant( rFont.GetItalic() );
    aFD.Underline = (sal_Int16)rFont.GetUnderline();
    aFD.Strikeout = (sal_Int16)rFont.GetStrikeout();
    aFD.Orientation = rFont.GetOrientation();
    aFD.Kerning = rFont.IsKerning();
    aFD.WordLineMode = rFont.IsWordLineMode();
    aFD.Type = 0;   // ??? => Nur an Metric...
    return aFD;
}


vcl::Font ImplCreateFont( const css::awt::FontDescriptor& rDescr )
{
    vcl::Font aFont;
    aFont.SetFamilyName( rDescr.Name );
    aFont.SetStyleName( rDescr.StyleName );
    aFont.SetFontSize( ::Size( rDescr.Width, rDescr.Height ) );
    aFont.SetFamily( (FontFamily)rDescr.Family );
    aFont.SetCharSet( (rtl_TextEncoding)rDescr.CharSet );
    aFont.SetPitch( (FontPitch)rDescr.Pitch );
    aFont.SetWidthType( VCLUnoHelper::ConvertFontWidth( rDescr.CharacterWidth ) );
    aFont.SetWeight( VCLUnoHelper::ConvertFontWeight( rDescr.Weight ) );
    aFont.SetItalic( (FontItalic)rDescr.Slant );
    aFont.SetUnderline( (::FontLineStyle)rDescr.Underline );
    aFont.SetStrikeout( (::FontStrikeout)rDescr.Strikeout );
    aFont.SetOrientation( (sal_Int16)rDescr.Orientation );
    aFont.SetKerning( static_cast<FontKerning>(rDescr.Kerning) );
    aFont.SetWordLineMode( rDescr.WordLineMode );
    return aFont;
}

FmXModifyMultiplexer::FmXModifyMultiplexer( ::cppu::OWeakObject& rSource, ::osl::Mutex& _rMutex )
                    :OWeakSubObject( rSource )
                    ,OInterfaceContainerHelper2( _rMutex )
{
}


Any SAL_CALL FmXModifyMultiplexer::queryInterface(const Type& _rType)
{
    Any aReturn;
    aReturn = ::cppu::queryInterface(_rType,
        static_cast< css::util::XModifyListener*>(this),
        static_cast< XEventListener*>(this)
    );

    if (!aReturn.hasValue())
        aReturn = OWeakSubObject::queryInterface( _rType );

    return aReturn;
}


void FmXModifyMultiplexer::disposing(const EventObject& )
{
}


void FmXModifyMultiplexer::modified(const EventObject& e)
{
    EventObject aMulti( e);
    aMulti.Source = &m_rParent;
    notifyEach( &XModifyListener::modified, aMulti );
}

FmXUpdateMultiplexer::FmXUpdateMultiplexer( ::cppu::OWeakObject& rSource, ::osl::Mutex& _rMutex )
                    :OWeakSubObject( rSource )
                    ,OInterfaceContainerHelper2( _rMutex )
{
}


Any SAL_CALL FmXUpdateMultiplexer::queryInterface(const Type& _rType)
{
    Any aReturn;
    aReturn = ::cppu::queryInterface(_rType,
        static_cast< XUpdateListener*>(this),
        static_cast< XEventListener*>(this)
    );

    if (!aReturn.hasValue())
        aReturn = OWeakSubObject::queryInterface( _rType );

    return aReturn;
}


void FmXUpdateMultiplexer::disposing(const EventObject& )
{
}


sal_Bool FmXUpdateMultiplexer::approveUpdate(const EventObject &e)
{
    EventObject aMulti( e );
    aMulti.Source = &m_rParent;

    bool bResult = true;
    if (getLength())
    {
        ::comphelper::OInterfaceIteratorHelper2 aIter(*this);
        while ( bResult && aIter.hasMoreElements() )
            bResult = static_cast< XUpdateListener* >( aIter.next() )->approveUpdate( aMulti );
    }

    return bResult;
}


void FmXUpdateMultiplexer::updated(const EventObject &e)
{
    EventObject aMulti( e );
    aMulti.Source = &m_rParent;
    notifyEach( &XUpdateListener::updated, aMulti );
}

FmXSelectionMultiplexer::FmXSelectionMultiplexer( ::cppu::OWeakObject& rSource, ::osl::Mutex& _rMutex )
    :OWeakSubObject( rSource )
    ,OInterfaceContainerHelper2( _rMutex )
{
}


Any SAL_CALL FmXSelectionMultiplexer::queryInterface(const Type& _rType)
{
    Any aReturn;
    aReturn = ::cppu::queryInterface(_rType,
        static_cast< XSelectionChangeListener*>(this),
        static_cast< XEventListener*>(this)
    );

    if (!aReturn.hasValue())
        aReturn = OWeakSubObject::queryInterface( _rType );

    return aReturn;
}


void FmXSelectionMultiplexer::disposing(const EventObject& )
{
}


void SAL_CALL FmXSelectionMultiplexer::selectionChanged( const EventObject& _rEvent )
{
    EventObject aMulti(_rEvent);
    aMulti.Source = &m_rParent;
    notifyEach( &XSelectionChangeListener::selectionChanged, aMulti );
}

FmXContainerMultiplexer::FmXContainerMultiplexer( ::cppu::OWeakObject& rSource, ::osl::Mutex& _rMutex )
                        :OWeakSubObject( rSource )
                        ,OInterfaceContainerHelper2( _rMutex )
{
}


Any SAL_CALL FmXContainerMultiplexer::queryInterface(const Type& _rType)
{
    Any aReturn;
    aReturn = ::cppu::queryInterface(_rType,
        static_cast< XContainerListener*>(this),
        static_cast< XEventListener*>(this)
    );

    if (!aReturn.hasValue())
        aReturn = OWeakSubObject::queryInterface( _rType );

    return aReturn;
}


void FmXContainerMultiplexer::disposing(const EventObject& )
{
}

void FmXContainerMultiplexer::elementInserted(const ContainerEvent& e)
{
    ContainerEvent aMulti( e );
    aMulti.Source = &m_rParent;
    notifyEach( &XContainerListener::elementInserted, aMulti );
}


void FmXContainerMultiplexer::elementRemoved(const ContainerEvent& e)
{
    ContainerEvent aMulti( e );
    aMulti.Source = &m_rParent;
    notifyEach( &XContainerListener::elementRemoved, aMulti );
}


void FmXContainerMultiplexer::elementReplaced(const ContainerEvent& e)
{
    ContainerEvent aMulti( e );
    aMulti.Source = &m_rParent;
    notifyEach( &XContainerListener::elementReplaced, aMulti );
}

FmXGridControlMultiplexer::FmXGridControlMultiplexer( ::cppu::OWeakObject& rSource, ::osl::Mutex& _rMutex )
    :OWeakSubObject( rSource )
    ,OInterfaceContainerHelper2( _rMutex )
{
}


Any SAL_CALL FmXGridControlMultiplexer::queryInterface(const Type& _rType)
{
    Any aReturn;
    aReturn = ::cppu::queryInterface( _rType,
        static_cast< XGridControlListener*>(this)
    );

    if (!aReturn.hasValue())
        aReturn = OWeakSubObject::queryInterface( _rType );

    return aReturn;
}


void FmXGridControlMultiplexer::disposing( const EventObject& )
{
}


void SAL_CALL FmXGridControlMultiplexer::columnChanged( const EventObject& _event )
{
    EventObject aForwardedEvent( _event );
    aForwardedEvent.Source = &m_rParent;
    notifyEach( &XGridControlListener::columnChanged, aForwardedEvent );
}


//= FmXGridControl


Reference< XInterface > SAL_CALL FmXGridControl_NewInstance_Impl(const Reference< XMultiServiceFactory>& _rxFactory)
{
    return *(new FmXGridControl( comphelper::getComponentContext(_rxFactory) ));
}

FmXGridControl::FmXGridControl(const Reference< XComponentContext >& _rxContext)
               :UnoControl()
               ,m_aModifyListeners(*this, GetMutex())
               ,m_aUpdateListeners(*this, GetMutex())
               ,m_aContainerListeners(*this, GetMutex())
               ,m_aSelectionListeners(*this, GetMutex())
               ,m_aGridControlListeners(*this, GetMutex())
               ,m_bInDraw(false)
               ,m_xContext(_rxContext)
{
}


FmXGridControl::~FmXGridControl()
{
}


Any SAL_CALL FmXGridControl::queryAggregation(const Type& _rType)
{
    Any aReturn = FmXGridControl_BASE::queryInterface(_rType);

    if (!aReturn.hasValue())
        aReturn = UnoControl::queryAggregation( _rType );
    return aReturn;
}


Sequence< Type> SAL_CALL FmXGridControl::getTypes(  )
{
    return comphelper::concatSequences(UnoControl::getTypes(),FmXGridControl_BASE::getTypes());
}


Sequence<sal_Int8> SAL_CALL FmXGridControl::getImplementationId(  )
{
    return css::uno::Sequence<sal_Int8>();
}

// XServiceInfo
sal_Bool SAL_CALL FmXGridControl::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

OUString SAL_CALL FmXGridControl::getImplementationName()
{
    return OUString("com.sun.star.form.FmXGridControl");
}

css::uno::Sequence<OUString> SAL_CALL FmXGridControl::getSupportedServiceNames()
{
    Sequence< OUString > aServiceNames(2);
    aServiceNames[0] = FM_SUN_CONTROL_GRIDCONTROL;
    aServiceNames[1] = "com.sun.star.awt.UnoControl";
    return aServiceNames;
}


void SAL_CALL FmXGridControl::dispose()
{
    SolarMutexGuard aGuard;

    EventObject aEvt;
    aEvt.Source = static_cast< ::cppu::OWeakObject* >(this);
    m_aModifyListeners.disposeAndClear(aEvt);
    m_aUpdateListeners.disposeAndClear(aEvt);
    m_aContainerListeners.disposeAndClear(aEvt);

    UnoControl::dispose();
}


OUString FmXGridControl::GetComponentServiceName()
{
    OUString aName("DBGrid");
    return aName;
}


sal_Bool SAL_CALL FmXGridControl::setModel(const Reference< css::awt::XControlModel >& rModel)
{
    SolarMutexGuard aGuard;

    if (!UnoControl::setModel(rModel))
        return false;

    Reference< XGridPeer > xGridPeer(getPeer(), UNO_QUERY);
    if (xGridPeer.is())
    {
        Reference< XIndexContainer > xCols(mxModel, UNO_QUERY);
        xGridPeer->setColumns(xCols);
    }
    return true;
}


FmXGridPeer* FmXGridControl::imp_CreatePeer(vcl::Window* pParent)
{
    FmXGridPeer* pReturn = new FmXGridPeer(m_xContext);

    // translate properties into WinBits
    WinBits nStyle = WB_TABSTOP;
    Reference< XPropertySet >  xModelSet(getModel(), UNO_QUERY);
    if (xModelSet.is())
    {
        try
        {
            if (::comphelper::getINT16(xModelSet->getPropertyValue(FM_PROP_BORDER)))
                nStyle |= WB_BORDER;
        }
        catch(const Exception&)
        {
            OSL_FAIL("Can not get style");
        }
    }

    pReturn->Create(pParent, nStyle);
    return pReturn;
}


void SAL_CALL FmXGridControl::createPeer(const Reference< css::awt::XToolkit >& /*rToolkit*/, const Reference< css::awt::XWindowPeer >& rParentPeer)
{
    if ( !mxModel.is() )
        throw DisposedException( OUString(), *this );

    DBG_ASSERT(/*(0 == m_nPeerCreationLevel) && */!mbCreatingPeer, "FmXGridControl::createPeer : recursion!");
        // I think this should never assert, now that we're using the base class' mbCreatingPeer in addition to
        // our own m_nPeerCreationLevel
        // But I'm not sure as I don't _fully_ understand the underlying toolkit implementations ....
        // (if this asserts, we still need m_nPeerCreationLevel. If not, we could omit it ....)
        // 14.05.2001 - 86836 - frank.schoenheit@germany.sun.com

    // TODO: why the hell this whole class does not use any mutex?

    if (!getPeer().is())
    {
        mbCreatingPeer = true;
        // mbCreatingPeer is virtually the same as m_nPeerCreationLevel, but it's the base class' method
        // to prevent recursion.

        vcl::Window* pParentWin = nullptr;
        if (rParentPeer.is())
        {
            VCLXWindow* pParent = VCLXWindow::GetImplementation(rParentPeer);
            if (pParent)
                pParentWin = pParent->GetWindow().get();
        }

        FmXGridPeer* pPeer = imp_CreatePeer(pParentWin);
        DBG_ASSERT(pPeer != nullptr, "FmXGridControl::createPeer : imp_CreatePeer didn't return a peer !");
        setPeer( pPeer );

        // reading the properties from the model
//      ++m_nPeerCreationLevel;
        updateFromModel();

        // consider the following ugly scenario: updateFromModel leads to a propertiesChanges on the Control,
        // which determines, dat a "critical" property has changed (e.g. "Border") and therefore starts a new
        // Peer, which lands again here in createPeer we also start a second FmXGridPeer and initialise it.
        // Then we exit from the first incarnation's updateFromModel and continue working with the pPeer,
        // that is in fact now already obsolete (as another peer is being started in the second incarnation).
        // Therefore the effort with the PeerCreationLevel, which ensures that we really use the Peer
        // created at the deepest level, but first initialise it in the top-level.
//      if (--m_nPeerCreationLevel == 0)
        {
            DBG_ASSERT(getPeer().is(), "FmXGridControl::createPeer : something went wrong ... no top level peer !");
            pPeer = FmXGridPeer::getImplementation(getPeer());

            setPosSize( maComponentInfos.nX, maComponentInfos.nY, maComponentInfos.nWidth, maComponentInfos.nHeight, css::awt::PosSize::POSSIZE );

            Reference< XIndexContainer >  xColumns(getModel(), UNO_QUERY);
            if (xColumns.is())
                pPeer->setColumns(xColumns);

            if (maComponentInfos.bVisible)
                pPeer->setVisible(true);

            if (!maComponentInfos.bEnable)
                pPeer->setEnable(false);

            if (maWindowListeners.getLength())
                pPeer->addWindowListener( &maWindowListeners );

            if (maFocusListeners.getLength())
                pPeer->addFocusListener( &maFocusListeners );

            if (maKeyListeners.getLength())
                pPeer->addKeyListener( &maKeyListeners );

            if (maMouseListeners.getLength())
                pPeer->addMouseListener( &maMouseListeners );

            if (maMouseMotionListeners.getLength())
                pPeer->addMouseMotionListener( &maMouseMotionListeners );

            if (maPaintListeners.getLength())
                pPeer->addPaintListener( &maPaintListeners );

            if (m_aModifyListeners.getLength())
                pPeer->addModifyListener( &m_aModifyListeners );

            if (m_aUpdateListeners.getLength())
                pPeer->addUpdateListener( &m_aUpdateListeners );

            if (m_aContainerListeners.getLength())
                pPeer->addContainerListener( &m_aContainerListeners );

            // forward the design mode
            bool bForceAlivePeer = m_bInDraw && !maComponentInfos.bVisible;
            // (we force an alive-mode peer if we're in "draw", cause in this case the peer will be used for drawing in
            // foreign devices. We ensure this with the visibility check as an living peer is assumed to be noncritical
            // only if invisible)
            Any aOldCursorBookmark;
            if (!mbDesignMode || bForceAlivePeer)
            {
                Reference< XFormComponent >  xComp(getModel(), UNO_QUERY);
                if (xComp.is())
                {
                    Reference< XRowSet >  xForm(xComp->getParent(), UNO_QUERY);
                    // is the form alive?
                    // we can see that if the form contains columns
                    Reference< css::sdbcx::XColumnsSupplier >  xColumnsSupplier(xForm, UNO_QUERY);
                    if (xColumnsSupplier.is())
                    {
                        if (Reference< XIndexAccess > (xColumnsSupplier->getColumns(),UNO_QUERY)->getCount())
                        {
                            // we get only a new bookmark if the resultset is not forwardonly
                            if (::comphelper::getINT32(Reference< XPropertySet > (xForm, UNO_QUERY)->getPropertyValue(FM_PROP_RESULTSET_TYPE)) != ResultSetType::FORWARD_ONLY)
                            {
                                // as the FmGridControl touches the data source it is connected to we have to remember the current
                                // cursor position (and restore afterwards)
                                // OJ: but only when we stand on a valid row
                                Reference< XResultSet > xResultSet(xForm, UNO_QUERY);
                                if ( !xResultSet->isBeforeFirst() && !xResultSet->isAfterLast() )
                                {
                                    try
                                    {
                                        aOldCursorBookmark = Reference< css::sdbcx::XRowLocate > (xForm, UNO_QUERY)->getBookmark();
                                    }
                                    catch( const Exception& e )
                                    {
                                        DBG_UNHANDLED_EXCEPTION();
                                        (void)e;
                                    }
                                }
                            }
                        }
                    }
                    pPeer->setRowSet(xForm);
                }
            }
            pPeer->setDesignMode(mbDesignMode && !bForceAlivePeer);

            try
            {
                if (aOldCursorBookmark.hasValue())
                {   // we have a valid bookmark, so we have to restore the cursor's position
                    Reference< XFormComponent >  xComp(getModel(), UNO_QUERY);
                    Reference< css::sdbcx::XRowLocate >  xLocate(xComp->getParent(), UNO_QUERY);
                    xLocate->moveToBookmark(aOldCursorBookmark);
                }
            }
            catch( const Exception& e )
            {
                DBG_UNHANDLED_EXCEPTION();
                (void)e;
            }

            Reference< css::awt::XView >  xPeerView(getPeer(), UNO_QUERY);
            xPeerView->setZoom( maComponentInfos.nZoomX, maComponentInfos.nZoomY );
            xPeerView->setGraphics( mxGraphics );
        }
        mbCreatingPeer = false;
    }
}


void FmXGridControl::addModifyListener(const Reference< css::util::XModifyListener >& l)
{
    m_aModifyListeners.addInterface( l );
    if( getPeer().is() && m_aModifyListeners.getLength() == 1 )
    {
        Reference< css::util::XModifyBroadcaster >  xGrid(getPeer(), UNO_QUERY);
        xGrid->addModifyListener( &m_aModifyListeners);
    }
}


sal_Bool SAL_CALL FmXGridControl::select( const Any& _rSelection )
{
    SolarMutexGuard aGuard;
    Reference< XSelectionSupplier > xPeer(getPeer(), UNO_QUERY);
    return xPeer->select(_rSelection);
}


Any SAL_CALL FmXGridControl::getSelection(  )
{
    SolarMutexGuard aGuard;
    Reference< XSelectionSupplier > xPeer(getPeer(), UNO_QUERY);
    return xPeer->getSelection();
}


void SAL_CALL FmXGridControl::addSelectionChangeListener( const Reference< XSelectionChangeListener >& _rxListener )
{
    m_aSelectionListeners.addInterface( _rxListener );
    if( getPeer().is() && 1 == m_aSelectionListeners.getLength() )
    {
        Reference< XSelectionSupplier > xGrid(getPeer(), UNO_QUERY);
        xGrid->addSelectionChangeListener( &m_aSelectionListeners);
    }
}


void SAL_CALL FmXGridControl::removeSelectionChangeListener( const Reference< XSelectionChangeListener >& _rxListener )
{
    if( getPeer().is() && 1 == m_aSelectionListeners.getLength() )
    {
        Reference< XSelectionSupplier > xGrid(getPeer(), UNO_QUERY);
        xGrid->removeSelectionChangeListener( &m_aSelectionListeners);
    }
    m_aSelectionListeners.removeInterface( _rxListener );
}


Sequence< sal_Bool > SAL_CALL FmXGridControl::queryFieldDataType( const Type& xType )
{
    if (getPeer().is())
    {
        Reference< XGridFieldDataSupplier >  xPeerSupplier(getPeer(), UNO_QUERY);
        if (xPeerSupplier.is())
            return xPeerSupplier->queryFieldDataType(xType);
    }

    return Sequence<sal_Bool>();
}


Sequence< Any > SAL_CALL FmXGridControl::queryFieldData( sal_Int32 nRow, const Type& xType )
{
    if (getPeer().is())
    {
        Reference< XGridFieldDataSupplier >  xPeerSupplier(getPeer(), UNO_QUERY);
        if (xPeerSupplier.is())
            return xPeerSupplier->queryFieldData(nRow, xType);
    }

    return Sequence< Any>();
}


void SAL_CALL FmXGridControl::removeModifyListener(const Reference< css::util::XModifyListener >& l)
{
    if( getPeer().is() && m_aModifyListeners.getLength() == 1 )
    {
        Reference< css::util::XModifyBroadcaster >  xGrid(getPeer(), UNO_QUERY);
        xGrid->removeModifyListener( &m_aModifyListeners);
    }
    m_aModifyListeners.removeInterface( l );
}


void SAL_CALL FmXGridControl::draw( sal_Int32 x, sal_Int32 y )
{
    SolarMutexGuard aGuard;
    m_bInDraw = true;
    UnoControl::draw(x, y);
    m_bInDraw = false;
}


void SAL_CALL FmXGridControl::setDesignMode(sal_Bool bOn)
{
    css::util::ModeChangeEvent aModeChangeEvent;

    // --- <mutex_lock> ---
    {
        SolarMutexGuard aGuard;

        Reference< XRowSetSupplier >  xGrid(getPeer(), UNO_QUERY);

        if (xGrid.is() && (bool(bOn) != mbDesignMode || (!bOn && !xGrid->getRowSet().is())))
        {
            if (bOn)
            {
                xGrid->setRowSet(Reference< XRowSet > ());
            }
            else
            {
                Reference< XFormComponent >  xComp(getModel(), UNO_QUERY);
                if (xComp.is())
                {
                    Reference< XRowSet >  xForm(xComp->getParent(), UNO_QUERY);
                    xGrid->setRowSet(xForm);
                }
            }

            mbDesignMode = bOn;

            Reference< XVclWindowPeer >  xVclWindowPeer( getPeer(), UNO_QUERY );
            if (xVclWindowPeer.is())
                xVclWindowPeer->setDesignMode(bOn);
        }
        mbDesignMode = bOn;

        // dispose our current AccessibleContext, if we have one
        // (changing the design mode implies having a new implementation for this context,
        // so the old one must be declared DEFUNC)
        DisposeAccessibleContext(
                Reference<XComponent>(maAccessibleContext, UNO_QUERY));
        maAccessibleContext.clear();

        // prepare firing an event
        aModeChangeEvent.Source = *this;
        aModeChangeEvent.NewMode = mbDesignMode ? OUString( "design" ) : OUString( "alive" );
    }

    // --- </mutex_lock> ---
    maModeChangeListeners.notifyEach( &XModeChangeListener::modeChanged, aModeChangeEvent );
}

// XBoundComponent

void SAL_CALL FmXGridControl::addUpdateListener(const Reference< XUpdateListener >& l)
{
    m_aUpdateListeners.addInterface( l );
    if( getPeer().is() && m_aUpdateListeners.getLength() == 1 )
    {
        Reference< XBoundComponent >  xBound(getPeer(), UNO_QUERY);
        xBound->addUpdateListener( &m_aUpdateListeners);
    }
}


void SAL_CALL FmXGridControl::removeUpdateListener(const Reference< XUpdateListener >& l)
{
    if( getPeer().is() && m_aUpdateListeners.getLength() == 1 )
    {
        Reference< XBoundComponent >  xBound(getPeer(), UNO_QUERY);
        xBound->removeUpdateListener( &m_aUpdateListeners);
    }
    m_aUpdateListeners.removeInterface( l );
}


sal_Bool SAL_CALL FmXGridControl::commit()
{
    Reference< XBoundComponent >  xBound(getPeer(), UNO_QUERY);
    if (xBound.is())
        return xBound->commit();
    else
        return true;
}

// XContainer

void SAL_CALL FmXGridControl::addContainerListener(const Reference< XContainerListener >& l)
{
    m_aContainerListeners.addInterface( l );
    if( getPeer().is() && m_aContainerListeners.getLength() == 1 )
    {
        Reference< XContainer >  xContainer(getPeer(), UNO_QUERY);
        xContainer->addContainerListener( &m_aContainerListeners);
    }
}


void SAL_CALL FmXGridControl::removeContainerListener(const Reference< XContainerListener >& l)
{
    if( getPeer().is() && m_aContainerListeners.getLength() == 1 )
    {
        Reference< XContainer >  xContainer(getPeer(), UNO_QUERY);
        xContainer->removeContainerListener( &m_aContainerListeners);
    }
    m_aContainerListeners.removeInterface( l );
}


Reference< css::frame::XDispatch >  SAL_CALL FmXGridControl::queryDispatch(const css::util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags)
{
    Reference< css::frame::XDispatchProvider >  xPeerProvider(getPeer(), UNO_QUERY);
    if (xPeerProvider.is())
        return xPeerProvider->queryDispatch(aURL, aTargetFrameName, nSearchFlags);
    else
        return Reference< css::frame::XDispatch > ();
}


Sequence< Reference< css::frame::XDispatch > > SAL_CALL FmXGridControl::queryDispatches(const Sequence< css::frame::DispatchDescriptor>& aDescripts)
{
    Reference< css::frame::XDispatchProvider >  xPeerProvider(getPeer(), UNO_QUERY);
    if (xPeerProvider.is())
        return xPeerProvider->queryDispatches(aDescripts);
    else
        return Sequence< Reference< css::frame::XDispatch > >();
}


void SAL_CALL FmXGridControl::registerDispatchProviderInterceptor(const Reference< css::frame::XDispatchProviderInterceptor >& _xInterceptor)
{
    Reference< css::frame::XDispatchProviderInterception >  xPeerInterception(getPeer(), UNO_QUERY);
    if (xPeerInterception.is())
        xPeerInterception->registerDispatchProviderInterceptor(_xInterceptor);
}


void SAL_CALL FmXGridControl::releaseDispatchProviderInterceptor(const Reference< css::frame::XDispatchProviderInterceptor >& _xInterceptor)
{
    Reference< css::frame::XDispatchProviderInterception >  xPeerInterception(getPeer(), UNO_QUERY);
    if (xPeerInterception.is())
        xPeerInterception->releaseDispatchProviderInterceptor(_xInterceptor);
}


void SAL_CALL FmXGridControl::addGridControlListener( const Reference< XGridControlListener >& _listener )
{
    ::osl::MutexGuard aGuard( GetMutex() );

    m_aGridControlListeners.addInterface( _listener );
    if ( getPeer().is() && 1 == m_aGridControlListeners.getLength() )
    {
        Reference< XGridControl > xPeerGrid( getPeer(), UNO_QUERY );
        if ( xPeerGrid.is() )
            xPeerGrid->addGridControlListener( &m_aGridControlListeners );
    }
}


void SAL_CALL FmXGridControl::removeGridControlListener( const Reference< XGridControlListener >& _listener )
{
    ::osl::MutexGuard aGuard( GetMutex() );

    if( getPeer().is() && 1 == m_aGridControlListeners.getLength() )
    {
        Reference< XGridControl > xPeerGrid( getPeer(), UNO_QUERY );
        if ( xPeerGrid.is() )
            xPeerGrid->removeGridControlListener( &m_aGridControlListeners );
    }

    m_aGridControlListeners.removeInterface( _listener );
}


sal_Int16 SAL_CALL FmXGridControl::getCurrentColumnPosition()
{
    Reference< XGridControl > xGrid( getPeer(), UNO_QUERY );
    return xGrid.is() ? xGrid->getCurrentColumnPosition() : -1;
}


void SAL_CALL FmXGridControl::setCurrentColumnPosition(sal_Int16 nPos)
{
    Reference< XGridControl > xGrid( getPeer(), UNO_QUERY );
    if ( xGrid.is() )
    {
        SolarMutexGuard aGuard;
        xGrid->setCurrentColumnPosition( nPos );
    }
}

// XElementAccess

sal_Bool SAL_CALL FmXGridControl::hasElements()
{
    Reference< XElementAccess >  xPeer(getPeer(), UNO_QUERY);
    return xPeer.is() && xPeer->hasElements();
}


Type SAL_CALL FmXGridControl::getElementType(  )
{
    return cppu::UnoType<css::awt::XTextComponent>::get();
}

// XEnumerationAccess

Reference< XEnumeration >  SAL_CALL FmXGridControl::createEnumeration()
{
    Reference< XEnumerationAccess >  xPeer(getPeer(), UNO_QUERY);
    if (xPeer.is())
        return xPeer->createEnumeration();
    else
        return new ::comphelper::OEnumerationByIndex(this);
}

// XIndexAccess

sal_Int32 SAL_CALL FmXGridControl::getCount()
{
    Reference< XIndexAccess >  xPeer(getPeer(), UNO_QUERY);
    return xPeer.is() ? xPeer->getCount() : 0;
}


Any SAL_CALL FmXGridControl::getByIndex(sal_Int32 _nIndex)
{
    Reference< XIndexAccess >  xPeer(getPeer(), UNO_QUERY);
    if (!xPeer.is())
        throw IndexOutOfBoundsException();

    return xPeer->getByIndex(_nIndex);
}

// css::util::XModeSelector

void SAL_CALL FmXGridControl::setMode(const OUString& Mode)
{
    Reference< css::util::XModeSelector >  xPeer(getPeer(), UNO_QUERY);
    if (!xPeer.is())
        throw NoSupportException();

    xPeer->setMode(Mode);
}


OUString SAL_CALL FmXGridControl::getMode()
{
    Reference< css::util::XModeSelector >  xPeer(getPeer(), UNO_QUERY);
    return xPeer.is() ? xPeer->getMode() : OUString();
}


css::uno::Sequence<OUString> SAL_CALL FmXGridControl::getSupportedModes()
{
    Reference< css::util::XModeSelector >  xPeer(getPeer(), UNO_QUERY);
    return xPeer.is() ? xPeer->getSupportedModes() : css::uno::Sequence<OUString>();
}


sal_Bool SAL_CALL FmXGridControl::supportsMode(const OUString& Mode)
{
    Reference< css::util::XModeSelector >  xPeer(getPeer(), UNO_QUERY);
    return xPeer.is() && xPeer->supportsMode(Mode);
}

// helper class which prevents that in the peer's header the FmGridListener must be known
class FmXGridPeer::GridListenerDelegator : public FmGridListener
{
protected:
    FmXGridPeer*        m_pPeer;

public:
    explicit GridListenerDelegator( FmXGridPeer* _pPeer );
    virtual ~GridListenerDelegator();

protected:
    virtual void selectionChanged() override;
    virtual void columnChanged() override;
};


FmXGridPeer::GridListenerDelegator::GridListenerDelegator(FmXGridPeer* _pPeer)
    :m_pPeer(_pPeer)
{
    DBG_ASSERT(m_pPeer, "GridListenerDelegator::GridListenerDelegator");
}

FmXGridPeer::GridListenerDelegator::~GridListenerDelegator()
{
}


void FmXGridPeer::GridListenerDelegator::selectionChanged()
{
    m_pPeer->selectionChanged();
}


void FmXGridPeer::GridListenerDelegator::columnChanged()
{
    m_pPeer->columnChanged();
}

void FmXGridPeer::selectionChanged()
{
    EventObject aSource;
    aSource.Source = static_cast< ::cppu::OWeakObject* >(this);
    m_aSelectionListeners.notifyEach( &XSelectionChangeListener::selectionChanged, aSource);
}


void FmXGridPeer::columnChanged()
{
    EventObject aEvent( *this );
    m_aGridControlListeners.notifyEach( &XGridControlListener::columnChanged, aEvent );
}


namespace fmgridif
{
    const OUString getDataModeIdentifier()
    {
        return OUString("DataMode");
    }
}
using namespace fmgridif;


FmXGridPeer::FmXGridPeer(const Reference< XComponentContext >& _rxContext)
            :m_aModifyListeners(m_aMutex)
            ,m_aUpdateListeners(m_aMutex)
            ,m_aContainerListeners(m_aMutex)
            ,m_aSelectionListeners(m_aMutex)
            ,m_aGridControlListeners(m_aMutex)
            ,m_aMode( getDataModeIdentifier() )
            ,m_nCursorListening(0)
            ,m_bInterceptingDispatch(false)
            ,m_pStateCache(nullptr)
            ,m_pDispatchers(nullptr)
            ,m_pGridListener(nullptr)
            ,m_xContext(_rxContext)
{
    // Create must be called after this constructure
    m_pGridListener = new GridListenerDelegator( this );
}


VclPtr<FmGridControl> FmXGridPeer::imp_CreateControl(vcl::Window* pParent, WinBits nStyle)
{
    return VclPtr<FmGridControl>::Create(m_xContext, pParent, this, nStyle);
}


void FmXGridPeer::Create(vcl::Window* pParent, WinBits nStyle)
{
    VclPtr<FmGridControl> pWin = imp_CreateControl(pParent, nStyle);
    DBG_ASSERT(pWin != nullptr, "FmXGridPeer::Create : imp_CreateControl didn't return a control !");

    pWin->SetStateProvider(LINK(this, FmXGridPeer, OnQueryGridSlotState));
    pWin->SetSlotExecutor(LINK(this, FmXGridPeer, OnExecuteGridSlot));

    // want to hear about row selections
    pWin->setGridListener( m_pGridListener );

    // Init must always be called
    pWin->Init();
    pWin->SetComponentInterface(this);

    getSupportedURLs();
}


FmXGridPeer::~FmXGridPeer()
{
    setRowSet(Reference< XRowSet > ());
    setColumns(Reference< XIndexContainer > ());

    delete m_pGridListener;
}

namespace
{
    class theFmXGridPeerImplementationId : public rtl::Static< UnoTunnelIdInit, theFmXGridPeerImplementationId > {};
}

const Sequence< sal_Int8 >& FmXGridPeer::getUnoTunnelImplementationId() throw()
{
    return theFmXGridPeerImplementationId::get().getSeq();
}


FmXGridPeer* FmXGridPeer::getImplementation( const Reference< XInterface >& _rxIFace ) throw()
{
    FmXGridPeer* pReturn = nullptr;
    Reference< XUnoTunnel >  xTunnel(_rxIFace, UNO_QUERY);
    if (xTunnel.is())
        pReturn = reinterpret_cast<FmXGridPeer*>(xTunnel->getSomething(getUnoTunnelImplementationId()));

    return pReturn;
}


sal_Int64 SAL_CALL FmXGridPeer::getSomething( const Sequence< sal_Int8 >& _rIdentifier )
{
    sal_Int64 nReturn(0);

    if  (   (_rIdentifier.getLength() == 16)
        &&  (0 == memcmp( getUnoTunnelImplementationId().getConstArray(), _rIdentifier.getConstArray(), 16 ))
        )
    {
        nReturn = reinterpret_cast<sal_Int64>(this);
    }
    else
        nReturn = VCLXWindow::getSomething(_rIdentifier);

    return nReturn;
}

// XEventListener

void FmXGridPeer::disposing(const EventObject& e)
{
using namespace ::com::sun::star::util;
    bool bKnownSender = false;

    Reference< XIndexContainer >  xCols( e.Source, UNO_QUERY );
    if ( xCols.is() )
    {
        setColumns(Reference< XIndexContainer > ());
        bKnownSender = true;
    }

    Reference< XRowSet >  xCursor(e.Source, UNO_QUERY);
    if (xCursor.is())
    {
        setRowSet( m_xCursor );
        m_xCursor = nullptr;
        bKnownSender = true;
    }


    if ( !bKnownSender && m_pDispatchers )
    {
        const Sequence< URL>& aSupportedURLs = getSupportedURLs();
        const URL* pSupportedURLs = aSupportedURLs.getConstArray();
        for ( sal_uInt16 i=0; i < ( aSupportedURLs.getLength() ) && !bKnownSender; ++i, ++pSupportedURLs )
        {
            if ( m_pDispatchers[i] == e.Source )
            {
                m_pDispatchers[i]->removeStatusListener( static_cast< css::frame::XStatusListener* >( this ), *pSupportedURLs );
                m_pDispatchers[i] = nullptr;
                m_pStateCache[i] = false;
                bKnownSender = true;
            }
        }
    }

    if ( !bKnownSender )
        VCLXWindow::disposing(e);
}


void FmXGridPeer::addModifyListener(const Reference< css::util::XModifyListener >& l)
{
    m_aModifyListeners.addInterface( l );
}


void FmXGridPeer::removeModifyListener(const Reference< css::util::XModifyListener >& l)
{
    m_aModifyListeners.removeInterface( l );
}


#define LAST_KNOWN_TYPE     FormComponentType::PATTERNFIELD
Sequence< sal_Bool > SAL_CALL FmXGridPeer::queryFieldDataType( const Type& xType )
{
    // eine 'Konvertierungstabelle'
    static const bool bCanConvert[LAST_KNOWN_TYPE][4] =
    {
        { false, false, false, false }, //  FormComponentType::CONTROL
        { false, false, false, false }, //  FormComponentType::COMMANDBUTTON
        { false, false, false, false }, //  FormComponentType::RADIOBUTTON
        { false, false, false, false }, //  FormComponentType::IMAGEBUTTON
        { false, false, false, true  }, //  FormComponentType::CHECKBOX
        { false, false, false, false }, //  FormComponentType::LISTBOX
        { false, false, false, false }, //  FormComponentType::COMBOBOX
        { false, false, false, false }, //  FormComponentType::GROUPBOX
        { true , false, false, false }, //  FormComponentType::TEXTFIELD
        { false, false, false, false }, //  FormComponentType::FIXEDTEXT
        { false, false, false, false }, //  FormComponentType::GRIDCONTROL
        { false, false, false, false }, //  FormComponentType::FILECONTROL
        { false, false, false, false }, //  FormComponentType::HIDDENCONTROL
        { false, false, false, false }, //  FormComponentType::IMAGECONTROL
        { true , true , true , false }, //  FormComponentType::DATEFIELD
        { true , true , false, false }, //  FormComponentType::TIMEFIELD
        { true , true , false, false }, //  FormComponentType::NUMERICFIELD
        { true , true , false, false }, //  FormComponentType::CURRENCYFIELD
        { true , false, false, false }  //  FormComponentType::PATTERNFIELD
    };


    sal_Int16 nMapColumn = -1;
    switch (xType.getTypeClass())
    {
        case TypeClass_STRING           : nMapColumn = 0; break;
        case TypeClass_FLOAT:
        case TypeClass_DOUBLE           : nMapColumn = 1; break;
        case TypeClass_SHORT:
        case TypeClass_LONG:
        case TypeClass_UNSIGNED_LONG:
        case TypeClass_UNSIGNED_SHORT   : nMapColumn = 2; break;
        case TypeClass_BOOLEAN          : nMapColumn = 3; break;
        default:
            break;
    }

    Reference< XIndexContainer >  xColumns = getColumns();

    VclPtr< FmGridControl > pGrid = GetAs< FmGridControl >();
    sal_Int32 nColumns = pGrid->GetViewColCount();

    DbGridColumns aColumns = pGrid->GetColumns();

    Sequence<sal_Bool> aReturnSequence(nColumns);
    sal_Bool* pReturnArray = aReturnSequence.getArray();

    bool bRequestedAsAny = (xType.getTypeClass() == TypeClass_ANY);

    DbGridColumn* pCol;
    Reference< css::sdb::XColumn >  xFieldContent;
    Reference< XPropertySet >  xCurrentColumn;
    for (sal_Int32 i=0; i<nColumns; ++i)
    {
        if (bRequestedAsAny)
        {
            pReturnArray[i] = true;
            continue;
        }

        pReturnArray[i] = false;

        sal_uInt16 nModelPos = pGrid->GetModelColumnPos(pGrid->GetColumnIdFromViewPos((sal_uInt16)i));
        DBG_ASSERT(nModelPos != (sal_uInt16)-1, "FmXGridPeer::queryFieldDataType : no model pos !");

        pCol = aColumns[ nModelPos ];
        const DbGridRowRef xRow = pGrid->GetSeekRow();
        xFieldContent = (xRow.is() && xRow->HasField(pCol->GetFieldPos())) ? xRow->GetField(pCol->GetFieldPos()).getColumn() : Reference< css::sdb::XColumn > ();
        if (!xFieldContent.is())
            // can't supply anything without a field content
            // FS - 07.12.99 - 54391
            continue;

        xColumns->getByIndex(nModelPos) >>= xCurrentColumn;
        if (!::comphelper::hasProperty(FM_PROP_CLASSID, xCurrentColumn))
            continue;

        sal_Int16 nClassId = sal_Int16();
        xCurrentColumn->getPropertyValue(FM_PROP_CLASSID) >>= nClassId;
        if (nClassId>LAST_KNOWN_TYPE)
            continue;
        DBG_ASSERT(nClassId>0, "FmXGridPeer::queryFieldDataType : somebody changed the definition of the FormComponentType enum !");

        if (nMapColumn != -1)
            pReturnArray[i] = bCanConvert[nClassId-1][nMapColumn];
    }

    return aReturnSequence;
}


Sequence< Any > SAL_CALL FmXGridPeer::queryFieldData( sal_Int32 nRow, const Type& xType )
{
    VclPtr< FmGridControl > pGrid = GetAs< FmGridControl >();
    DBG_ASSERT(pGrid && pGrid->IsOpen(), "FmXGridPeer::queryFieldData : have no valid grid window !");
    if (!pGrid || !pGrid->IsOpen())
        return Sequence< Any>();

    // das Control zur angegebenen Row fahren
    if (!pGrid->SeekRow(nRow))
    {
        throw IllegalArgumentException();
    }

    // don't use GetCurrentRow as this isn't affected by the above SeekRow
    // FS - 30.09.99 - 68644
    DbGridRowRef xPaintRow = pGrid->GetPaintRow();
    ENSURE_OR_THROW( xPaintRow.is(), "invalid paint row" );

    // I need the columns of the control for GetFieldText
    DbGridColumns aColumns = pGrid->GetColumns();

    // und durch alle Spalten durch
    sal_Int32 nColumnCount = pGrid->GetViewColCount();

    Sequence< Any> aReturnSequence(nColumnCount);
    Any* pReturnArray = aReturnSequence.getArray();

    bool bRequestedAsAny = (xType.getTypeClass() == TypeClass_ANY);
    Reference< css::sdb::XColumn >  xFieldContent;
    for (sal_Int32 i=0; i < nColumnCount; ++i)
    {
        sal_uInt16 nModelPos = pGrid->GetModelColumnPos(pGrid->GetColumnIdFromViewPos((sal_uInt16)i));
        DBG_ASSERT(nModelPos != (sal_uInt16)-1, "FmXGridPeer::queryFieldData : invalid model pos !");

        // don't use GetCurrentFieldValue to determine the field content as this isn't affected by the above SeekRow
        // FS - 30.09.99 - 68644
        DbGridColumn* pCol = aColumns[ nModelPos ];
        xFieldContent = xPaintRow->HasField( pCol->GetFieldPos() )
                    ?   xPaintRow->GetField( pCol->GetFieldPos() ).getColumn()
                    :   Reference< XColumn > ();

        if ( !xFieldContent.is() )
            continue;

        if (bRequestedAsAny)
        {
            Reference< XPropertySet >  xFieldSet(xFieldContent, UNO_QUERY);
            pReturnArray[i] = xFieldSet->getPropertyValue(FM_PROP_VALUE);
        }
        else
        {
            switch (xType.getTypeClass())
            {
                // Strings are dealt with directly by the GetFieldText
                case TypeClass_STRING           :
                {
                    OUString sText = aColumns[ nModelPos ]->GetCellText( xPaintRow.get(), pGrid->getNumberFormatter() );
                    pReturnArray[i] <<= sText;
                }
                break;
                // everything else is requested in the DatabaseVariant
                case TypeClass_FLOAT            : pReturnArray[i] <<= xFieldContent->getFloat(); break;
                case TypeClass_DOUBLE           : pReturnArray[i] <<= xFieldContent->getDouble(); break;
                case TypeClass_SHORT            : pReturnArray[i] <<= (sal_Int16)xFieldContent->getShort(); break;
                case TypeClass_LONG             : pReturnArray[i] <<= (sal_Int32)xFieldContent->getLong(); break;
                case TypeClass_UNSIGNED_SHORT   : pReturnArray[i] <<= (sal_uInt16)xFieldContent->getShort(); break;
                case TypeClass_UNSIGNED_LONG    : pReturnArray[i] <<= (sal_uInt32)xFieldContent->getLong(); break;
                case TypeClass_BOOLEAN          : pReturnArray[i] <<= xFieldContent->getBoolean(); break;
                default:
                {
                    throw IllegalArgumentException();
                }
            }
        }
    }
    return aReturnSequence;
}


void FmXGridPeer::CellModified()
{
    EventObject aEvt;
    aEvt.Source = static_cast< ::cppu::OWeakObject* >(this);
    m_aModifyListeners.notifyEach( &XModifyListener::modified, aEvt );
}

// XPropertyChangeListener

void FmXGridPeer::propertyChange(const PropertyChangeEvent& evt)
{
    SolarMutexGuard aGuard;
        // want to do a lot of VCL stuff here ...
        // this should not be (deadlock) critical, as by definition, every component should release
        // any own mutexes before notifying

    VclPtr< FmGridControl > pGrid = GetAs< FmGridControl >();
    if (!pGrid)
        return;

    // Database event
    Reference< XRowSet >  xCursor(evt.Source, UNO_QUERY);
    if (evt.PropertyName == FM_PROP_VALUE || m_xCursor == evt.Source)
        pGrid->propertyChange(evt);
    else if (pGrid && m_xColumns.is() && m_xColumns->hasElements())
    {
        // next find which column has changed
        css::uno::Reference<css::uno::XInterface> xCurrent;
        sal_Int32 i;

        for ( i = 0; i < m_xColumns->getCount(); i++)
        {
            xCurrent.set(m_xColumns->getByIndex(i), css::uno::UNO_QUERY);
            if (evt.Source == xCurrent)
                break;
        }

        if (i >= m_xColumns->getCount())
            // this is valid because we are listening at the cursor, too (RecordCount, -status, edit mode)
            return;

        sal_uInt16 nId = pGrid->GetColumnIdFromModelPos((sal_uInt16)i);
        bool bInvalidateColumn = false;

        if (evt.PropertyName == FM_PROP_LABEL)
        {
            OUString aName = ::comphelper::getString(evt.NewValue);
            if (aName != pGrid->GetColumnTitle(nId))
                pGrid->SetColumnTitle(nId, aName);
        }
        else if (evt.PropertyName == FM_PROP_WIDTH)
        {
            sal_Int32 nWidth = 0;
            if (evt.NewValue.getValueType().getTypeClass() == TypeClass_VOID)
                nWidth = pGrid->GetDefaultColumnWidth(pGrid->GetColumnTitle(nId));
                // GetDefaultColumnWidth already considered the zoom factor
            else
            {
                sal_Int32 nTest = 0;
                if (evt.NewValue >>= nTest)
                {
                    nWidth = pGrid->LogicToPixel(Point(nTest,0),MapUnit::Map10thMM).X();
                    // take the zoom factor into account
                    nWidth = pGrid->CalcZoom(nWidth);
                }
            }
            if (nWidth != (sal_Int32(pGrid->GetColumnWidth(nId))))
            {
                if (pGrid->IsEditing())
                {
                    pGrid->DeactivateCell();
                    pGrid->ActivateCell();
                }
                pGrid->SetColumnWidth(nId, nWidth);
            }
        }
        else if (evt.PropertyName == FM_PROP_HIDDEN)
        {
            DBG_ASSERT(evt.NewValue.getValueType().getTypeClass() == TypeClass_BOOLEAN,
                "FmXGridPeer::propertyChange : the property 'hidden' should be of type boolean !");
            if (::comphelper::getBOOL(evt.NewValue))
                pGrid->HideColumn(nId);
            else
                pGrid->ShowColumn(nId);
        }
        else if (evt.PropertyName == FM_PROP_ALIGN)
        {
            // in design mode it doesn't matter
            if (!isDesignMode())
            {
                DbGridColumn* pCol = pGrid->GetColumns().at( i );

                pCol->SetAlignmentFromModel(-1);
                bInvalidateColumn = true;
            }
        }
        else if (evt.PropertyName == FM_PROP_FORMATKEY)
        {
            if (!isDesignMode())
                bInvalidateColumn = true;
        }

        // need to invalidate the affected column ?
        if (bInvalidateColumn)
        {
            bool bWasEditing = pGrid->IsEditing();
            if (bWasEditing)
                pGrid->DeactivateCell();

            ::Rectangle aColRect = pGrid->GetFieldRect(nId);
            aColRect.Top() = 0;
            aColRect.Bottom() = pGrid->GetSizePixel().Height();
            pGrid->Invalidate(aColRect);

            if (bWasEditing)
                pGrid->ActivateCell();
        }
    }
}

// XBoundComponent

void FmXGridPeer::addUpdateListener(const Reference< XUpdateListener >& l)
{
    m_aUpdateListeners.addInterface(l);
}


void FmXGridPeer::removeUpdateListener(const Reference< XUpdateListener >& l)
{
    m_aUpdateListeners.removeInterface(l);
}


sal_Bool FmXGridPeer::commit()
{
    VclPtr< FmGridControl > pGrid = GetAs< FmGridControl >();
    if (!m_xCursor.is() || !pGrid)
        return true;

    EventObject aEvt(static_cast< ::cppu::OWeakObject* >(this));
    ::comphelper::OInterfaceIteratorHelper2 aIter(m_aUpdateListeners);
    bool bCancel = false;
    while (aIter.hasMoreElements() && !bCancel)
        if ( !static_cast< XUpdateListener* >( aIter.next() )->approveUpdate( aEvt ) )
            bCancel = true;

    if (!bCancel)
        bCancel = !pGrid->commit();

    if (!bCancel)
        m_aUpdateListeners.notifyEach( &XUpdateListener::updated, aEvt );
    return !bCancel;
}


void FmXGridPeer::cursorMoved(const EventObject& _rEvent)
{
    VclPtr< FmGridControl > pGrid = GetAs< FmGridControl >();
    // we are not interested in move to insert row only in the resetted event
    // which is fired after positioning an the insert row
    if (pGrid && pGrid->IsOpen() && !::comphelper::getBOOL(Reference< XPropertySet > (_rEvent.Source, UNO_QUERY)->getPropertyValue(FM_PROP_ISNEW)))
        pGrid->positioned(_rEvent);
}


void FmXGridPeer::rowChanged(const EventObject& _rEvent)
{
    VclPtr< FmGridControl > pGrid = GetAs< FmGridControl >();
    if (pGrid && pGrid->IsOpen())
    {
        if (m_xCursor->rowUpdated() && !pGrid->IsCurrentAppending())
            pGrid->RowModified(pGrid->GetCurrentPos());
        else if (m_xCursor->rowInserted())
            pGrid->inserted(_rEvent);
    }
}


void FmXGridPeer::rowSetChanged(const EventObject& /*event*/)
{
    // not interested in ...
    // (our parent is a form which means we get a loaded or reloaded after this rowSetChanged)
}

// XLoadListener

void FmXGridPeer::loaded(const EventObject& /*rEvent*/)
{
    updateGrid(m_xCursor);
}


void FmXGridPeer::unloaded(const EventObject& /*rEvent*/)
{
    updateGrid( Reference< XRowSet > (nullptr) );
}


void FmXGridPeer::reloading(const EventObject& /*aEvent*/)
{
    // empty the grid
    updateGrid( Reference< XRowSet > (nullptr) );
}


void FmXGridPeer::unloading(const EventObject& /*aEvent*/)
{
    // empty the grid
    updateGrid( Reference< XRowSet > (nullptr) );
}


void FmXGridPeer::reloaded(const EventObject& aEvent)
{
    {
        const sal_Int32 cnt = m_xColumns->getCount();
        for(sal_Int32 i=0; i<cnt; ++i)
        {
            Reference< XLoadListener> xll(m_xColumns->getByIndex(i), UNO_QUERY);
            if(xll.is())
            {
                xll->reloaded(aEvent);
            }
        }
    }
    updateGrid(m_xCursor);
}

// XGridPeer

Reference< XIndexContainer >  FmXGridPeer::getColumns()
{
    return m_xColumns;
}


void FmXGridPeer::addColumnListeners(const Reference< XPropertySet >& xCol)
{
    static const OUStringLiteral aPropsListenedTo[] =
    {
        FM_PROP_LABEL, FM_PROP_WIDTH, FM_PROP_HIDDEN, FM_PROP_ALIGN,
        FM_PROP_FORMATKEY
    };

    // as not all properties have to be supported by all columns we have to check this
    // before adding a listener
    Reference< XPropertySetInfo > xInfo = xCol->getPropertySetInfo();
    for (unsigned i=0; i<SAL_N_ELEMENTS(aPropsListenedTo); ++i)
    {
        if ( xInfo->hasPropertyByName( aPropsListenedTo[i] ) )
        {
            Property aPropDesc = xInfo->getPropertyByName( aPropsListenedTo[i] );
            if ( 0 != ( aPropDesc.Attributes & PropertyAttribute::BOUND ) )
                xCol->addPropertyChangeListener( aPropsListenedTo[i], this );
        }
    }
}


void FmXGridPeer::removeColumnListeners(const Reference< XPropertySet >& xCol)
{
    // the same props as in addColumnListeners ... linux has problems with global static UStrings, so
    // we have to do it this way ....
    static const OUStringLiteral aPropsListenedTo[] =
    {
        FM_PROP_LABEL, FM_PROP_WIDTH, FM_PROP_HIDDEN, FM_PROP_ALIGN,
        FM_PROP_FORMATKEY
    };

    Reference< XPropertySetInfo >  xInfo = xCol->getPropertySetInfo();
    for (const auto & i : aPropsListenedTo)
        if (xInfo->hasPropertyByName(i))
            xCol->removePropertyChangeListener(i, this);
}


void FmXGridPeer::setColumns(const Reference< XIndexContainer >& Columns)
{
    SolarMutexGuard aGuard;

    VclPtr< FmGridControl > pGrid = GetAs< FmGridControl >();

    if (m_xColumns.is())
    {
        Reference< XPropertySet > xCol;
        for (sal_Int32 i = 0; i < m_xColumns->getCount(); i++)
        {
            xCol.set(m_xColumns->getByIndex(i), css::uno::UNO_QUERY);
            removeColumnListeners(xCol);
        }
        Reference< XContainer >  xContainer(m_xColumns, UNO_QUERY);
        xContainer->removeContainerListener(this);

        Reference< XSelectionSupplier >  xSelSupplier(m_xColumns, UNO_QUERY);
        xSelSupplier->removeSelectionChangeListener(this);

        Reference< XReset >  xColumnReset(m_xColumns, UNO_QUERY);
        if (xColumnReset.is())
            xColumnReset->removeResetListener(static_cast<XResetListener*>(this));
    }
    if (Columns.is())
    {
        Reference< XContainer >  xContainer(Columns, UNO_QUERY);
        xContainer->addContainerListener(this);

        Reference< XSelectionSupplier >  xSelSupplier(Columns, UNO_QUERY);
        xSelSupplier->addSelectionChangeListener(this);

        Reference< XPropertySet >  xCol;
        for (sal_Int32 i = 0; i < Columns->getCount(); i++)
        {
            xCol.set(Columns->getByIndex(i), css::uno::UNO_QUERY);
            addColumnListeners(xCol);
        }

        Reference< XReset >  xColumnReset(Columns, UNO_QUERY);
        if (xColumnReset.is())
            xColumnReset->addResetListener(static_cast<XResetListener*>(this));
    }
    m_xColumns = Columns;
    if (pGrid)
    {
        pGrid->InitColumnsByModels(m_xColumns);

        if (m_xColumns.is())
        {
            EventObject aEvt(m_xColumns);
            selectionChanged(aEvt);
        }
    }
}


void FmXGridPeer::setDesignMode(sal_Bool bOn)
{
    if (bOn != isDesignMode())
    {
        VclPtr<vcl::Window> pWin = GetWindow();
        if (pWin)
            static_cast<FmGridControl*>(pWin.get())->SetDesignMode(bOn);
    }

    if (bOn)
        DisConnectFromDispatcher();
    else
        UpdateDispatches(); // will connect if not already connected and just update else
}


sal_Bool FmXGridPeer::isDesignMode()
{
    VclPtr<vcl::Window> pWin = GetWindow();
    if (pWin)
        return static_cast<FmGridControl*>(pWin.get())->IsDesignMode();
    else
        return false;
}


void FmXGridPeer::elementInserted(const ContainerEvent& evt)
{
    SolarMutexGuard aGuard;

    VclPtr< FmGridControl > pGrid = GetAs< FmGridControl >();
    // Handle Column beruecksichtigen
    if (!pGrid || !m_xColumns.is() || pGrid->IsInColumnMove() || m_xColumns->getCount() == ((sal_Int32)pGrid->GetModelColCount()))
        return;

    Reference< XPropertySet >  xNewColumn(evt.Element, css::uno::UNO_QUERY);
    addColumnListeners(xNewColumn);

    OUString aName = ::comphelper::getString(xNewColumn->getPropertyValue(FM_PROP_LABEL));
    Any aWidth = xNewColumn->getPropertyValue(FM_PROP_WIDTH);
    sal_Int32 nWidth = 0;
    if (aWidth >>= nWidth)
        nWidth = pGrid->LogicToPixel(Point(nWidth,0),MapUnit::Map10thMM).X();

    pGrid->AppendColumn(aName, (sal_uInt16)nWidth, (sal_Int16)::comphelper::getINT32(evt.Accessor));

    // jetzt die Spalte setzen
    DbGridColumn* pCol = pGrid->GetColumns().at( ::comphelper::getINT32(evt.Accessor) );
    pCol->setModel(xNewColumn);

    Any aHidden = xNewColumn->getPropertyValue(FM_PROP_HIDDEN);
    if (::comphelper::getBOOL(aHidden))
        pGrid->HideColumn(pCol->GetId());

    FormControlFactory( m_xContext ).initializeTextFieldLineEnds( xNewColumn );
}


void FmXGridPeer::elementReplaced(const ContainerEvent& evt)
{
    SolarMutexGuard aGuard;

    VclPtr< FmGridControl > pGrid = GetAs< FmGridControl >();

    // Handle Column beruecksichtigen
    if (!pGrid || !m_xColumns.is() || pGrid->IsInColumnMove())
        return;

    Reference< XPropertySet > xNewColumn(evt.Element, css::uno::UNO_QUERY);
    Reference< XPropertySet > xOldColumn(
        evt.ReplacedElement, css::uno::UNO_QUERY);

    bool bWasEditing = pGrid->IsEditing();
    if (bWasEditing)
        pGrid->DeactivateCell();

    pGrid->RemoveColumn(pGrid->GetColumnIdFromModelPos((sal_uInt16)::comphelper::getINT32(evt.Accessor)));

    removeColumnListeners(xOldColumn);
    addColumnListeners(xNewColumn);

    OUString aName = ::comphelper::getString(xNewColumn->getPropertyValue(FM_PROP_LABEL));
    Any aWidth = xNewColumn->getPropertyValue(FM_PROP_WIDTH);
    sal_Int32 nWidth = 0;
    if (aWidth >>= nWidth)
        nWidth = pGrid->LogicToPixel(Point(nWidth,0),MapUnit::Map10thMM).X();
    sal_uInt16 nNewId = pGrid->AppendColumn(aName, (sal_uInt16)nWidth, (sal_Int16)::comphelper::getINT32(evt.Accessor));
    sal_uInt16 nNewPos = pGrid->GetModelColumnPos(nNewId);

    // set the model of the new column
    DbGridColumn* pCol = pGrid->GetColumns().at( nNewPos );

    // for initializing this grid column, we need the fields of the grid's data source
    Reference< XColumnsSupplier > xSuppColumns;
    CursorWrapper* pGridDataSource = pGrid->getDataSource();
    if ( pGridDataSource )
        xSuppColumns.set(Reference< XInterface >( *pGridDataSource ), css::uno::UNO_QUERY);
    Reference< XNameAccess > xColumnsByName;
    if ( xSuppColumns.is() )
        xColumnsByName = xSuppColumns->getColumns();
    Reference< XIndexAccess > xColumnsByIndex( xColumnsByName, UNO_QUERY );

    if ( xColumnsByIndex.is() )
        FmGridControl::InitColumnByField( pCol, xNewColumn, xColumnsByName, xColumnsByIndex );
    else
        // the simple version, applies when the grid is not yet connected to a data source
        pCol->setModel(xNewColumn);

    if (bWasEditing)
        pGrid->ActivateCell();
}


void FmXGridPeer::elementRemoved(const ContainerEvent& evt)
{
    SolarMutexGuard aGuard;

    VclPtr< FmGridControl > pGrid = GetAs< FmGridControl >();

    // Handle Column beruecksichtigen
    if (!pGrid || !m_xColumns.is() || pGrid->IsInColumnMove() || m_xColumns->getCount() == ((sal_Int32)pGrid->GetModelColCount()))
        return;

    pGrid->RemoveColumn(pGrid->GetColumnIdFromModelPos((sal_uInt16)::comphelper::getINT32(evt.Accessor)));

    Reference< XPropertySet > xOldColumn(evt.Element, css::uno::UNO_QUERY);
    removeColumnListeners(xOldColumn);
}


void FmXGridPeer::setProperty( const OUString& PropertyName, const Any& Value)
{
    SolarMutexGuard aGuard;

    VclPtr< FmGridControl > pGrid = GetAs< FmGridControl >();

    bool bVoid = !Value.hasValue();

    if ( PropertyName == FM_PROP_TEXTLINECOLOR )
    {
        ::Color aTextLineColor( bVoid ? COL_TRANSPARENT : ::comphelper::getINT32( Value ) );
        if (bVoid)
        {
            pGrid->SetTextLineColor();
            pGrid->GetDataWindow().SetTextLineColor();
        }
        else
        {
            pGrid->SetTextLineColor(aTextLineColor);
            pGrid->GetDataWindow().SetTextLineColor(aTextLineColor);
        }

        // need to forward this to the columns
        DbGridColumns& rColumns = const_cast<DbGridColumns&>(pGrid->GetColumns());
        for (DbGridColumn* pLoop : rColumns)
        {
            FmXGridCell* pXCell = pLoop->GetCell();
            if (pXCell)
            {
                if (bVoid)
                    pXCell->SetTextLineColor();
                else
                    pXCell->SetTextLineColor(aTextLineColor);
            }
        }

        if (isDesignMode())
            pGrid->Invalidate();
    }
    else if ( PropertyName == FM_PROP_FONTEMPHASISMARK )
    {
        vcl::Font aGridFont = pGrid->GetControlFont();
        sal_Int16 nValue = ::comphelper::getINT16(Value);
        aGridFont.SetEmphasisMark( (FontEmphasisMark)nValue );
        pGrid->SetControlFont( aGridFont );
    }
    else if ( PropertyName == FM_PROP_FONTRELIEF )
    {
        vcl::Font aGridFont = pGrid->GetControlFont();
        sal_Int16 nValue = ::comphelper::getINT16(Value);
        aGridFont.SetRelief( (FontRelief)nValue );
        pGrid->SetControlFont( aGridFont );
    }
    else if ( PropertyName == FM_PROP_HELPURL )
    {
        OUString sHelpURL;
        OSL_VERIFY( Value >>= sHelpURL );
        INetURLObject aHID( sHelpURL );
        if ( aHID.GetProtocol() == INetProtocol::Hid )
            sHelpURL = aHID.GetURLPath();
        pGrid->SetHelpId( OUStringToOString( sHelpURL, RTL_TEXTENCODING_UTF8 ) );
    }
    else if ( PropertyName == FM_PROP_DISPLAYSYNCHRON )
    {
        pGrid->setDisplaySynchron(::comphelper::getBOOL(Value));
    }
    else if ( PropertyName == FM_PROP_CURSORCOLOR )
    {
        if (bVoid)
            pGrid->SetCursorColor(COL_TRANSPARENT);
        else
            pGrid->SetCursorColor( ::Color(::comphelper::getINT32(Value)));
        if (isDesignMode())
            pGrid->Invalidate();
    }
    else if ( PropertyName == FM_PROP_ALWAYSSHOWCURSOR )
    {
        pGrid->EnablePermanentCursor(::comphelper::getBOOL(Value));
        if (isDesignMode())
            pGrid->Invalidate();
    }
    else if ( PropertyName == FM_PROP_FONT )
    {
        if ( bVoid )
            pGrid->SetControlFont( vcl::Font() );
        else
        {
            css::awt::FontDescriptor aFont;
            if (Value >>= aFont)
            {
                vcl::Font aNewVclFont;
                if (aFont != ::comphelper::getDefaultFont())    // ist das der Default
                    aNewVclFont = ImplCreateFont( aFont );

                // need to add relief and emphasis (they're stored in a VCL-Font, but not in a FontDescriptor
                vcl::Font aOldVclFont = pGrid->GetControlFont();
                aNewVclFont.SetRelief( aOldVclFont.GetRelief() );
                aNewVclFont.SetEmphasisMark( aOldVclFont.GetEmphasisMark() );

                // now set it ...
                pGrid->SetControlFont( aNewVclFont );

                // if our row-height property is void (which means "calculate it font-dependent") we have
                // to adjust the control's row height
                Reference< XPropertySet >  xModelSet(getColumns(), UNO_QUERY);
                if (xModelSet.is() && ::comphelper::hasProperty(FM_PROP_ROWHEIGHT, xModelSet))
                {
                    Any aHeight = xModelSet->getPropertyValue(FM_PROP_ROWHEIGHT);
                    if (!aHeight.hasValue())
                        pGrid->SetDataRowHeight(0);
                }

            }
        }
    }
    else if ( PropertyName == FM_PROP_BACKGROUNDCOLOR )
    {
        if ( bVoid )
        {
            pGrid->SetControlBackground();
        }
        else
        {
            ::Color aColor( ::comphelper::getINT32(Value) );
            pGrid->SetBackground( aColor );
            pGrid->SetControlBackground( aColor );
        }
    }
    else if ( PropertyName == FM_PROP_TEXTCOLOR )
    {
        if ( bVoid )
        {
            pGrid->SetControlForeground();
        }
        else
        {
            ::Color aColor( ::comphelper::getINT32(Value) );
            pGrid->SetTextColor( aColor );
            pGrid->SetControlForeground( aColor );
        }
    }
    else if ( PropertyName == FM_PROP_ROWHEIGHT )
    {
        sal_Int32 nLogHeight(0);
        if (Value >>= nLogHeight)
        {
            sal_Int32 nHeight = pGrid->LogicToPixel(Point(0,nLogHeight),MapUnit::Map10thMM).Y();
            // take the zoom factor into account
            nHeight = pGrid->CalcZoom(nHeight);
            pGrid->SetDataRowHeight(nHeight);
        }
        else if (bVoid)
            pGrid->SetDataRowHeight(0);
    }
    else if ( PropertyName == FM_PROP_HASNAVIGATION )
    {
        bool bValue( true );
        OSL_VERIFY( Value >>= bValue );
        pGrid->EnableNavigationBar( bValue );
    }
    else if ( PropertyName == FM_PROP_RECORDMARKER )
    {
        bool bValue( true );
        OSL_VERIFY( Value >>= bValue );
        pGrid->EnableHandle( bValue );
    }
    else if ( PropertyName == FM_PROP_ENABLED )
    {
        bool bValue( true );
        OSL_VERIFY( Value >>= bValue );

        // In design mode, disable only the data window.
        // Else the control cannot be configured anymore.
        if (isDesignMode())
            pGrid->GetDataWindow().Enable( bValue );
        else
            pGrid->Enable( bValue );
    }
    else
        VCLXWindow::setProperty( PropertyName, Value );
}


Reference< XAccessibleContext > FmXGridPeer::CreateAccessibleContext()
{
    Reference< XAccessibleContext > xContext;

    // use the AccessibleContext provided by the VCL window
    VclPtr<vcl::Window> pGrid = GetWindow();
    if ( pGrid )
    {
        Reference< XAccessible > xAcc( pGrid->GetAccessible() );
        if ( xAcc.is() )
            xContext = xAcc->getAccessibleContext();
        // TODO: this has a slight conceptual problem:

        // We know that the XAccessible and XAccessibleContext implementation of the browse
        // box is the same (the class implements both interfaces), which, speaking strictly,
        // is bad here (means when a browse box acts as UnoControl): We (the FmXGridPeer) are
        // the XAccessible here, and the browse box should be able to provide us an XAccessibleContext,
        // but it should _not_ be the XAccessible itself.
        // However, as long as no client implementation uses dirty hacks such as querying an
        // XAccessibleContext for XAccessible, this should not be a problem.
    }

    if ( !xContext.is() )
        xContext = VCLXWindow::CreateAccessibleContext( );

    return xContext;
}


Any FmXGridPeer::getProperty( const OUString& _rPropertyName )
{
    Any aProp;
    if (GetWindow())
    {
        VclPtr< FmGridControl > pGrid = GetAs< FmGridControl >();
        vcl::Window* pDataWindow  = &pGrid->GetDataWindow();

        if ( _rPropertyName == FM_PROP_NAME )
        {
            vcl::Font aFont = pDataWindow->GetControlFont();
            aProp <<= ImplCreateFontDescriptor( aFont );
        }
        else if ( _rPropertyName == FM_PROP_TEXTCOLOR )
        {
            aProp <<= (sal_Int32)pDataWindow->GetControlForeground().GetColor();
        }
        else if ( _rPropertyName == FM_PROP_BACKGROUNDCOLOR )
        {
            aProp <<= (sal_Int32)pDataWindow->GetControlBackground().GetColor();
        }
        else if ( _rPropertyName == FM_PROP_ROWHEIGHT )
        {
            sal_Int32 nPixelHeight = pGrid->GetDataRowHeight();
            // take the zoom factor into account
            nPixelHeight = pGrid->CalcReverseZoom(nPixelHeight);
            aProp <<= (sal_Int32)pGrid->PixelToLogic(Point(0,nPixelHeight),MapUnit::Map10thMM).Y();
        }
        else if ( _rPropertyName == FM_PROP_HASNAVIGATION )
        {
            bool bHasNavBar = pGrid->HasNavigationBar();
            aProp <<= bHasNavBar;
        }
        else if ( _rPropertyName == FM_PROP_RECORDMARKER )
        {
            bool bHasHandle = pGrid->HasHandle();
            aProp <<= bHasHandle;
        }
        else if ( _rPropertyName == FM_PROP_ENABLED )
        {
            aProp <<= pDataWindow->IsEnabled();
        }
        else
            aProp = VCLXWindow::getProperty( _rPropertyName );
    }
    return aProp;
}


void FmXGridPeer::dispose()
{
    EventObject aEvt;
    aEvt.Source = static_cast< ::cppu::OWeakObject* >(this);
    m_aModifyListeners.disposeAndClear(aEvt);
    m_aUpdateListeners.disposeAndClear(aEvt);
    m_aContainerListeners.disposeAndClear(aEvt);
    VCLXWindow::dispose();

    // release all interceptors
    Reference< XDispatchProviderInterceptor > xInterceptor( m_xFirstDispatchInterceptor );
    m_xFirstDispatchInterceptor.clear();
    while ( xInterceptor.is() )
    {
        // tell the interceptor it has a new (means no) predecessor
        xInterceptor->setMasterDispatchProvider( nullptr );

        // ask for its successor
        Reference< XDispatchProvider > xSlave = xInterceptor->getSlaveDispatchProvider();
        // and give it the new (means no) successoert
        xInterceptor->setSlaveDispatchProvider( nullptr );

        // start over with the next chain element
        xInterceptor.set(xSlave, css::uno::UNO_QUERY);
    }

    DisConnectFromDispatcher();
    setRowSet(Reference< XRowSet > ());
}

// XContainer

void FmXGridPeer::addContainerListener(const Reference< XContainerListener >& l)
{
    m_aContainerListeners.addInterface( l );
}

void FmXGridPeer::removeContainerListener(const Reference< XContainerListener >& l)
{
    m_aContainerListeners.removeInterface( l );
}

// css::data::XDatabaseCursorSupplier

void FmXGridPeer::startCursorListening()
{
    if (!m_nCursorListening)
    {
        Reference< XRowSet >  xRowSet(m_xCursor, UNO_QUERY);
        if (xRowSet.is())
            xRowSet->addRowSetListener(this);

        Reference< XReset >  xReset(m_xCursor, UNO_QUERY);
        if (xReset.is())
            xReset->addResetListener(this);

        // register all listeners
        Reference< XPropertySet >  xSet(m_xCursor, UNO_QUERY);
        if (xSet.is())
        {
            xSet->addPropertyChangeListener(FM_PROP_ISMODIFIED, this);
            xSet->addPropertyChangeListener(FM_PROP_ROWCOUNT, this);
        }
    }
    m_nCursorListening++;
}


void FmXGridPeer::stopCursorListening()
{
    if (!--m_nCursorListening)
    {
        Reference< XRowSet >  xRowSet(m_xCursor, UNO_QUERY);
        if (xRowSet.is())
            xRowSet->removeRowSetListener(this);

        Reference< XReset >  xReset(m_xCursor, UNO_QUERY);
        if (xReset.is())
            xReset->removeResetListener(this);

        Reference< XPropertySet >  xSet(m_xCursor, UNO_QUERY);
        if (xSet.is())
        {
            xSet->removePropertyChangeListener(FM_PROP_ISMODIFIED, this);
            xSet->removePropertyChangeListener(FM_PROP_ROWCOUNT, this);
        }
    }
}


void FmXGridPeer::updateGrid(const Reference< XRowSet >& _rxCursor)
{
    VclPtr< FmGridControl > pGrid = GetAs< FmGridControl >();
    if (pGrid)
        pGrid->setDataSource(_rxCursor);
}


Reference< XRowSet >  FmXGridPeer::getRowSet()
{
    return m_xCursor;
}


void FmXGridPeer::setRowSet(const Reference< XRowSet >& _rDatabaseCursor)
{
    VclPtr< FmGridControl > pGrid = GetAs< FmGridControl >();
    if (!pGrid || !m_xColumns.is() || !m_xColumns->getCount())
        return;
    // unregister all listeners
    if (m_xCursor.is())
    {
        Reference< XLoadable >  xLoadable(m_xCursor, UNO_QUERY);
        // only if the form is loaded we set the rowset
        if (xLoadable.is())
        {
            stopCursorListening();
            xLoadable->removeLoadListener(this);
        }
    }

    m_xCursor = _rDatabaseCursor;

    if (pGrid)
    {
        Reference< XLoadable >  xLoadable(m_xCursor, UNO_QUERY);
        // only if the form is loaded we set the rowset
        if (xLoadable.is() && xLoadable->isLoaded())
            pGrid->setDataSource(m_xCursor);
        else
            pGrid->setDataSource(Reference< XRowSet > ());

        if (xLoadable.is())
        {
            startCursorListening();
            xLoadable->addLoadListener(this);
        }
    }
}


void SAL_CALL FmXGridPeer::addGridControlListener( const Reference< XGridControlListener >& _listener )
{
    m_aGridControlListeners.addInterface( _listener );
}


void SAL_CALL FmXGridPeer::removeGridControlListener( const Reference< XGridControlListener >& _listener )
{
    m_aGridControlListeners.removeInterface( _listener );
}


sal_Int16 FmXGridPeer::getCurrentColumnPosition()
{
    VclPtr< FmGridControl > pGrid = GetAs< FmGridControl >();
    return pGrid ? pGrid->GetViewColumnPos(pGrid->GetCurColumnId()) : -1;
}


void FmXGridPeer::setCurrentColumnPosition(sal_Int16 nPos)
{
    VclPtr< FmGridControl > pGrid = GetAs< FmGridControl >();
    if (pGrid)
        pGrid->GoToColumnId(pGrid->GetColumnIdFromViewPos(nPos));
}


void FmXGridPeer::selectionChanged(const EventObject& evt)
{
    SolarMutexGuard aGuard;

    VclPtr< FmGridControl > pGrid = GetAs< FmGridControl >();
    if (pGrid)
    {
        Reference< css::view::XSelectionSupplier >  xSelSupplier(evt.Source, UNO_QUERY);
        Any aSelection = xSelSupplier->getSelection();
        DBG_ASSERT(aSelection.getValueType().getTypeClass() == TypeClass_INTERFACE, "FmXGridPeer::selectionChanged : invalid selection !");
        Reference< XPropertySet >  xSelection;
         aSelection >>= xSelection;
        if (xSelection.is())
        {
            Reference< XPropertySet > xCol;
            sal_Int32 i = 0;
            sal_Int32 nColCount = m_xColumns->getCount();

            for (; i < nColCount; ++i)
            {
                m_xColumns->getByIndex(i) >>= xCol;
                if ( xCol == xSelection )
                {
                    pGrid->markColumn(pGrid->GetColumnIdFromModelPos((sal_uInt16)i));
                    break;
                }
            }
            // The columns have to be 1-based for the VCL control.
            // If necessary, pass on the selection to the VCL control
            if ( i != pGrid->GetSelectedColumn() )
            {   // (wenn das nicht greift, wurde das selectionChanged implizit von dem Control selber ausgeloest
                // if this does not ?hold?catch?, the selectionChanged is cleared by the Control itself
                if ( i < nColCount )
                {
                    pGrid->SelectColumnPos(pGrid->GetViewColumnPos(pGrid->GetColumnIdFromModelPos( (sal_uInt16)i )) + 1);
                    // SelectColumnPos has led to an implicit ActivateCell again
                    if (pGrid->IsEditing())
                        pGrid->DeactivateCell();
                }
                else
                    pGrid->SetNoSelection();
            }
        }
        else
            pGrid->markColumn(USHRT_MAX);
    }
}

// XElementAccess

sal_Bool FmXGridPeer::hasElements()
{
    return getCount() != 0;
}


Type SAL_CALL FmXGridPeer::getElementType(  )
{
    return cppu::UnoType<css::awt::XControl>::get();
}

// XEnumerationAccess

Reference< XEnumeration >  FmXGridPeer::createEnumeration()
{
    return new ::comphelper::OEnumerationByIndex(this);
}

// XIndexAccess

sal_Int32 FmXGridPeer::getCount()
{
    VclPtr< FmGridControl > pGrid = GetAs< FmGridControl >();
    if (pGrid)
        return pGrid->GetViewColCount();
    else
        return 0;
}


Any FmXGridPeer::getByIndex(sal_Int32 _nIndex)
{
    VclPtr< FmGridControl > pGrid = GetAs< FmGridControl >();
    if (_nIndex < 0 ||
        _nIndex >= getCount() || !pGrid)
        throw IndexOutOfBoundsException();

    Any aElement;
    // get the columnid
    sal_uInt16 nId = pGrid->GetColumnIdFromViewPos((sal_uInt16)_nIndex);
    // get the list position
    sal_uInt16 nPos = pGrid->GetModelColumnPos(nId);

    if ( nPos == GRID_COLUMN_NOT_FOUND )
        return aElement;

    DbGridColumn* pCol = pGrid->GetColumns().at( nPos );
    Reference< css::awt::XControl >  xControl(pCol->GetCell());
    aElement <<= xControl;

    return aElement;
}

// css::util::XModeSelector

void FmXGridPeer::setMode(const OUString& Mode)
{
    if (!supportsMode(Mode))
        throw NoSupportException();

    if (Mode == m_aMode)
        return;

    m_aMode = Mode;

    VclPtr< FmGridControl > pGrid = GetAs< FmGridControl >();
    if ( Mode == "FilterMode" )
        pGrid->SetFilterMode(true);
    else
    {
        pGrid->SetFilterMode(false);
        pGrid->setDataSource(m_xCursor);
    }
}


OUString FmXGridPeer::getMode()
{
    return m_aMode;
}


css::uno::Sequence<OUString> FmXGridPeer::getSupportedModes()
{
    static css::uno::Sequence<OUString> aModes;
    if (!aModes.getLength())
    {
        aModes.realloc(2);
        OUString* pModes = aModes.getArray();
        pModes[0] = "DataMode";
        pModes[1] = "FilterMode";
    }
    return aModes;
}


sal_Bool FmXGridPeer::supportsMode(const OUString& Mode)
{
    css::uno::Sequence<OUString> aModes(getSupportedModes());
    const OUString* pModes = aModes.getConstArray();
    for (sal_Int32 i = aModes.getLength(); i > 0; )
    {
        if (pModes[--i] == Mode)
            return true;
    }
    return false;
}


void FmXGridPeer::columnVisible(DbGridColumn* pColumn)
{
    VclPtr< FmGridControl > pGrid = GetAs< FmGridControl >();

    sal_Int32 _nIndex = pGrid->GetModelColumnPos(pColumn->GetId());
    Reference< css::awt::XControl >  xControl(pColumn->GetCell());
    ContainerEvent aEvt;
    aEvt.Source   = static_cast<XContainer*>(this);
    aEvt.Accessor <<= _nIndex;
    aEvt.Element  <<= xControl;

    m_aContainerListeners.notifyEach( &XContainerListener::elementInserted, aEvt );
}


void FmXGridPeer::columnHidden(DbGridColumn* pColumn)
{
    VclPtr< FmGridControl > pGrid = GetAs< FmGridControl >();

    sal_Int32 _nIndex = pGrid->GetModelColumnPos(pColumn->GetId());
    Reference< css::awt::XControl >  xControl(pColumn->GetCell());
    ContainerEvent aEvt;
    aEvt.Source   = static_cast<XContainer*>(this);
    aEvt.Accessor <<= _nIndex;
    aEvt.Element  <<= xControl;

    m_aContainerListeners.notifyEach( &XContainerListener::elementRemoved, aEvt );
}


void FmXGridPeer::draw( sal_Int32 x, sal_Int32 y )
{
    VclPtr< FmGridControl > pGrid = GetAs< FmGridControl >();
    EditBrowseBoxFlags nOldFlags = pGrid->GetBrowserFlags();
    pGrid->SetBrowserFlags(nOldFlags | EditBrowseBoxFlags::NO_HANDLE_COLUMN_CONTENT);

    VCLXWindow::draw(x, y);

    pGrid->SetBrowserFlags(nOldFlags);
}


Reference< css::frame::XDispatch >  FmXGridPeer::queryDispatch(const css::util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags)
{
    Reference< css::frame::XDispatch >  xResult;

    // first ask our interceptor chain
    if (m_xFirstDispatchInterceptor.is() && !m_bInterceptingDispatch)
    {
        m_bInterceptingDispatch = true;
        // safety against recursion : as we are master of the first chain element and slave of the last one we would
        // have an infinite loop without this if no dispatcher can fulfill the request
        xResult = m_xFirstDispatchInterceptor->queryDispatch(aURL, aTargetFrameName, nSearchFlags);
        m_bInterceptingDispatch = false;
    }

    // then ask ourself : we don't have any dispatches
    return xResult;
}


Sequence< Reference< css::frame::XDispatch > > FmXGridPeer::queryDispatches(const Sequence< css::frame::DispatchDescriptor>& aDescripts)
{
    if (m_xFirstDispatchInterceptor.is())
        return m_xFirstDispatchInterceptor->queryDispatches(aDescripts);

    // then ask ourself : we don't have any dispatches
    return Sequence< Reference< css::frame::XDispatch > >();
}


void FmXGridPeer::registerDispatchProviderInterceptor(const Reference< css::frame::XDispatchProviderInterceptor >& _xInterceptor)
{
    if (_xInterceptor.is())
    {
        if (m_xFirstDispatchInterceptor.is())
        {
            Reference< css::frame::XDispatchProvider > xFirstProvider(m_xFirstDispatchInterceptor, UNO_QUERY);
            // there is already an interceptor; the new one will become its master
            _xInterceptor->setSlaveDispatchProvider(xFirstProvider);
            m_xFirstDispatchInterceptor->setMasterDispatchProvider(xFirstProvider);
        }
        else
        {
            // it is the first interceptor; set ourself as slave
            _xInterceptor->setSlaveDispatchProvider(static_cast<css::frame::XDispatchProvider*>(this));
        }

        // we are the master of the chain's first interceptor
        m_xFirstDispatchInterceptor = _xInterceptor;
        m_xFirstDispatchInterceptor->setMasterDispatchProvider(static_cast<css::frame::XDispatchProvider*>(this));

        // we have a new interceptor and we're alive ?
        if (!isDesignMode())
            // -> check for new dispatchers
            UpdateDispatches();
    }
}


void FmXGridPeer::releaseDispatchProviderInterceptor(const Reference< css::frame::XDispatchProviderInterceptor >& _xInterceptor)
{
    if (!_xInterceptor.is())
        return;

    Reference< css::frame::XDispatchProviderInterceptor >  xChainWalk(m_xFirstDispatchInterceptor);

    if (m_xFirstDispatchInterceptor == _xInterceptor)
    {   // our chain will have a new first element
        Reference< css::frame::XDispatchProviderInterceptor >  xSlave(m_xFirstDispatchInterceptor->getSlaveDispatchProvider(), UNO_QUERY);
        m_xFirstDispatchInterceptor = xSlave;
    }
    // do this before removing the interceptor from the chain as we won't know it's slave afterwards)

    while (xChainWalk.is())
    {
        // walk along the chain of interceptors and look for the interceptor that has to be removed
        Reference< css::frame::XDispatchProviderInterceptor >  xSlave(xChainWalk->getSlaveDispatchProvider(), UNO_QUERY);

        if (xChainWalk == _xInterceptor)
        {
            // old master may be an interceptor too
            Reference< css::frame::XDispatchProviderInterceptor >  xMaster(xChainWalk->getMasterDispatchProvider(), UNO_QUERY);

            // unchain the interceptor that has to be removed
            xChainWalk->setSlaveDispatchProvider(Reference< css::frame::XDispatchProvider > ());
            xChainWalk->setMasterDispatchProvider(Reference< css::frame::XDispatchProvider > ());

            // reconnect the chain
            if (xMaster.is())
            {
                if (xSlave.is())
                    xMaster->setSlaveDispatchProvider(Reference< css::frame::XDispatchProvider >::query(xSlave));
                else
                    // it's the first interceptor of the chain, set ourself as slave
                    xMaster->setSlaveDispatchProvider(static_cast<css::frame::XDispatchProvider*>(this));
            }
            else
            {
                // the chain's first element was removed, set ourself as new master of the second one
                if (xSlave.is())
                    xSlave->setMasterDispatchProvider(static_cast<css::frame::XDispatchProvider*>(this));
            }
        }

        xChainWalk = xSlave;
    }
    // our interceptor chain has changed and we're alive ?
    if (!isDesignMode())
        // -> check the dispatchers
        UpdateDispatches();
}


void FmXGridPeer::statusChanged(const css::frame::FeatureStateEvent& Event)
{
    DBG_ASSERT(m_pStateCache, "FmXGridPeer::statusChanged : invalid call !");
    DBG_ASSERT(m_pDispatchers, "FmXGridPeer::statusChanged : invalid call !");

    Sequence< css::util::URL>& aUrls = getSupportedURLs();
    const css::util::URL* pUrls = aUrls.getConstArray();

    const std::vector<DbGridControlNavigationBarState>& aSlots = getSupportedGridSlots();

    sal_uInt16 i;
    for (i=0; i<aUrls.getLength(); ++i, ++pUrls)
    {
        if (pUrls->Main == Event.FeatureURL.Main)
        {
            DBG_ASSERT(m_pDispatchers[i] == Event.Source, "FmXGridPeer::statusChanged : the event source is a little bit suspect !");
            m_pStateCache[i] = Event.IsEnabled;
            VclPtr< FmGridControl > pGrid = GetAs< FmGridControl >();
            if (aSlots[i] != DbGridControlNavigationBarState::Undo)
                pGrid->GetNavigationBar().InvalidateState(aSlots[i]);
            break;
        }
    }
    DBG_ASSERT(i<aUrls.getLength(), "FmXGridPeer::statusChanged : got a call for an unknown url !");
}


sal_Bool FmXGridPeer::approveReset(const EventObject& /*rEvent*/)
{
    return true;
}


sal_Bool SAL_CALL FmXGridPeer::select( const Any& _rSelection )
{
    Sequence< Any > aBookmarks;
    if ( !( _rSelection >>= aBookmarks ) )
        throw IllegalArgumentException();

    return GetAs< FmGridControl >()->selectBookmarks(aBookmarks);

    // TODO:
    // speaking strictly, we would have to adjust our model, as our ColumnSelection may have changed.
    // Our model is a XSelectionSupplier, too, it handles the selection of single columns.
    // This is somewhat strange, as selection should be a view (not a model) aspect.
    // So for a clean solution, we should handle column selection ourself, and the model shouldn't
    // deal with selection at all.
}


Any SAL_CALL FmXGridPeer::getSelection(  )
{
    VclPtr< FmGridControl > pVclControl = GetAs< FmGridControl >();
    Sequence< Any > aSelectionBookmarks = pVclControl->getSelectionBookmarks();
    return makeAny(aSelectionBookmarks);
}


void SAL_CALL FmXGridPeer::addSelectionChangeListener( const Reference< XSelectionChangeListener >& _rxListener )
{
    m_aSelectionListeners.addInterface( _rxListener );
}


void SAL_CALL FmXGridPeer::removeSelectionChangeListener( const Reference< XSelectionChangeListener >& _rxListener )
{
    m_aSelectionListeners.removeInterface( _rxListener );
}


void FmXGridPeer::resetted(const EventObject& rEvent)
{
    if (m_xColumns == rEvent.Source)
    {   // my model was reset -> refresh the grid content
        SolarMutexGuard aGuard;
        VclPtr< FmGridControl > pGrid = GetAs< FmGridControl >();
        if (!pGrid)
            return;
        pGrid->resetCurrentRow();
    }
    // if the cursor fired a reset event we seem to be on the insert row
    else if (m_xCursor == rEvent.Source)
    {
        SolarMutexGuard aGuard;
        VclPtr< FmGridControl > pGrid = GetAs< FmGridControl >();
        if (pGrid && pGrid->IsOpen())
            pGrid->positioned(rEvent);
    }
}


const std::vector<DbGridControlNavigationBarState>& FmXGridPeer::getSupportedGridSlots()
{
    static const std::vector<DbGridControlNavigationBarState> aSupported {
        DbGridControlNavigationBarState::First,
        DbGridControlNavigationBarState::Prev,
        DbGridControlNavigationBarState::Next,
        DbGridControlNavigationBarState::Last,
        DbGridControlNavigationBarState::New,
        DbGridControlNavigationBarState::Undo
    };
    return aSupported;
}


Sequence< css::util::URL>& FmXGridPeer::getSupportedURLs()
{
    static Sequence< css::util::URL> aSupported;
    if (aSupported.getLength() == 0)
    {
        static const char* sSupported[] = {
            FMURL_RECORD_MOVEFIRST,
            FMURL_RECORD_MOVEPREV,
            FMURL_RECORD_MOVENEXT,
            FMURL_RECORD_MOVELAST,
            FMURL_RECORD_MOVETONEW,
            FMURL_RECORD_UNDO
        };
        aSupported.realloc(SAL_N_ELEMENTS(sSupported));
        css::util::URL* pSupported = aSupported.getArray();
        sal_uInt16 i;

        for ( i = 0; i < aSupported.getLength(); ++i, ++pSupported)
            pSupported->Complete = OUString::createFromAscii(sSupported[i]);

        // let an css::util::URL-transformer normalize the URLs
        Reference< css::util::XURLTransformer >  xTransformer(
            util::URLTransformer::create(::comphelper::getProcessComponentContext()) );
        pSupported = aSupported.getArray();
        for (i=0; i<aSupported.getLength(); ++i)
            xTransformer->parseStrict(pSupported[i]);
    }

    return aSupported;
}


void FmXGridPeer::UpdateDispatches()
{
    if (!m_pStateCache)
    {   // we don't have any dispatchers yet -> do the initial connect
        ConnectToDispatcher();
        return;
    }

    sal_uInt16 nDispatchersGot = 0;
    const Sequence< css::util::URL>& aSupportedURLs = getSupportedURLs();
    const css::util::URL* pSupportedURLs = aSupportedURLs.getConstArray();
    Reference< css::frame::XDispatch >  xNewDispatch;
    for (sal_Int32 i=0; i<aSupportedURLs.getLength(); ++i, ++pSupportedURLs)
    {
        xNewDispatch = queryDispatch(*pSupportedURLs, OUString(), 0);
        if (xNewDispatch != m_pDispatchers[i])
        {
            if (m_pDispatchers[i].is())
                m_pDispatchers[i]->removeStatusListener(static_cast<css::frame::XStatusListener*>(this), *pSupportedURLs);
            m_pDispatchers[i] = xNewDispatch;
            if (m_pDispatchers[i].is())
                m_pDispatchers[i]->addStatusListener(static_cast<css::frame::XStatusListener*>(this), *pSupportedURLs);
        }
        if (m_pDispatchers[i].is())
            ++nDispatchersGot;
    }

    if (!nDispatchersGot)
    {
        delete[] m_pStateCache;
        delete[] m_pDispatchers;
        m_pStateCache = nullptr;
        m_pDispatchers = nullptr;
    }
}


void FmXGridPeer::ConnectToDispatcher()
{
    DBG_ASSERT((m_pStateCache != nullptr) == (m_pDispatchers != nullptr), "FmXGridPeer::ConnectToDispatcher : inconsistent !");
    if (m_pStateCache)
    {   // already connected -> just do an update
        UpdateDispatches();
        return;
    }

    const Sequence< css::util::URL>& aSupportedURLs = getSupportedURLs();

    // _before_ adding the status listeners (as the add should result in a statusChanged-call) !
    m_pStateCache = new bool[aSupportedURLs.getLength()];
    m_pDispatchers = new Reference< css::frame::XDispatch > [aSupportedURLs.getLength()];

    sal_uInt16 nDispatchersGot = 0;
    const css::util::URL* pSupportedURLs = aSupportedURLs.getConstArray();
    for (sal_Int32 i=0; i<aSupportedURLs.getLength(); ++i, ++pSupportedURLs)
    {
        m_pStateCache[i] = false;
        m_pDispatchers[i] = queryDispatch(*pSupportedURLs, OUString(), 0);
        if (m_pDispatchers[i].is())
        {
            m_pDispatchers[i]->addStatusListener(static_cast<css::frame::XStatusListener*>(this), *pSupportedURLs);
            ++nDispatchersGot;
        }
    }

    if (!nDispatchersGot)
    {
        delete[] m_pStateCache;
        delete[] m_pDispatchers;
        m_pStateCache = nullptr;
        m_pDispatchers = nullptr;
    }
}


void FmXGridPeer::DisConnectFromDispatcher()
{
    if (!m_pStateCache || !m_pDispatchers)
        return;
    // we're not connected

    const Sequence< css::util::URL>& aSupportedURLs = getSupportedURLs();
    const css::util::URL* pSupportedURLs = aSupportedURLs.getConstArray();
    for (sal_Int32 i=0; i<aSupportedURLs.getLength(); ++i, ++pSupportedURLs)
    {
        if (m_pDispatchers[i].is())
            m_pDispatchers[i]->removeStatusListener(static_cast<css::frame::XStatusListener*>(this), *pSupportedURLs);
    }

    delete[] m_pStateCache;
    delete[] m_pDispatchers;
    m_pStateCache = nullptr;
    m_pDispatchers = nullptr;
}


IMPL_LINK(FmXGridPeer, OnQueryGridSlotState, DbGridControlNavigationBarState, nSlot, int)
{
    if (!m_pStateCache)
        return -1;  // unspecified

    // search the given slot with our supported sequence
    const std::vector<DbGridControlNavigationBarState>& aSupported = getSupportedGridSlots();
    for (size_t i=0; i<aSupported.size(); ++i)
    {
        if (aSupported[i] == nSlot)
        {
            if (!m_pDispatchers[i].is())
                return -1;  // nothing known about this slot
            else
                return m_pStateCache[i] ? 1 : 0;
        }
    }

    return  -1;
}


IMPL_LINK(FmXGridPeer, OnExecuteGridSlot, DbGridControlNavigationBarState, nSlot, bool)
{
    if (!m_pDispatchers)
        return false;   // not handled

    Sequence< css::util::URL>& aUrls = getSupportedURLs();
    const css::util::URL* pUrls = aUrls.getConstArray();

    const std::vector<DbGridControlNavigationBarState>& aSlots = getSupportedGridSlots();

    DBG_ASSERT((sal_Int32)aSlots.size() == aUrls.getLength(), "FmXGridPeer::OnExecuteGridSlot : inconstent data returned by getSupportedURLs/getSupportedGridSlots !");

    for (size_t i=0; i<aSlots.size(); ++i, ++pUrls)
    {
        if (aSlots[i] == nSlot)
        {
            if (m_pDispatchers[i].is())
            {
                // commit any changes done so far, if it's not the undoRecord URL
                if ( pUrls->Complete == FMURL_RECORD_UNDO || commit() )
                    m_pDispatchers[i]->dispatch(*pUrls, Sequence< PropertyValue>());

                return true;   // handled
            }
        }
    }

    return false;   // not handled
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
