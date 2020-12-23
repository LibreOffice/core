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

#include "ElementSelector.hxx"
#include <ObjectNameProvider.hxx>
#include <ObjectHierarchy.hxx>
#include <servicenames.hxx>
#include <DrawViewWrapper.hxx>
#include <ResId.hxx>
#include <strings.hrc>
#include <ObjectIdentifier.hxx>

#include <cppuhelper/supportsservice.hxx>
#include <o3tl/safeint.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/svapp.hxx>

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

namespace chart { class ExplicitValueProvider; }

namespace chart
{

using namespace com::sun::star;
using namespace com::sun::star::uno;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace
{
const OUStringLiteral lcl_aServiceName
    = u"com.sun.star.comp.chart.ElementSelectorToolbarController";
}

SelectorListBox::SelectorListBox(vcl::Window* pParent)
    : InterimItemWindow(pParent, "modules/schart/ui/combobox.ui", "ComboBox")
    , m_xWidget(m_xBuilder->weld_combo_box("combobox"))
    , m_bReleaseFocus(true)
{
    InitControlBase(m_xWidget.get());

    m_xWidget->connect_key_press(LINK(this, SelectorListBox, KeyInputHdl));
    m_xWidget->connect_changed(LINK(this, SelectorListBox, SelectHdl));
    m_xWidget->connect_focus_out(LINK(this, SelectorListBox, FocusOutHdl));

    ::Size aLogicalSize(75, 0);
    ::Size aPixelSize = LogicToPixel(aLogicalSize, MapMode(MapUnit::MapAppFont));
    m_xWidget->set_size_request(aPixelSize.Width(), -1);
    SetSizePixel(m_xContainer->get_preferred_size());
}

void SelectorListBox::dispose()
{
    m_xWidget.reset();
    InterimItemWindow::dispose();
}

SelectorListBox::~SelectorListBox()
{
    disposeOnce();
}

static void lcl_addObjectsToList( const ObjectHierarchy& rHierarchy, const  ObjectIdentifier & rParent, std::vector< ListBoxEntryData >& rEntries
                          , const sal_Int32 nHierarchyDepth, const Reference< chart2::XChartDocument >& xChartDoc )
{
    ObjectHierarchy::tChildContainer aChildren( rHierarchy.getChildren(rParent) );
    for (auto const& child : aChildren)
    {
        ListBoxEntryData aEntry;
        aEntry.OID = child;
        aEntry.UIName = ObjectNameProvider::getNameForCID( child.getObjectCID(), xChartDoc );
        aEntry.nHierarchyDepth = nHierarchyDepth;
        rEntries.push_back(aEntry);
        lcl_addObjectsToList( rHierarchy, child, rEntries, nHierarchyDepth+1, xChartDoc );
    }
}

void SelectorListBox::SetChartController( const Reference< frame::XController >& xChartController )
{
    m_xChartController = xChartController;
}

void SelectorListBox::UpdateChartElementsListAndSelection()
{
    m_xWidget->clear();
    m_aEntries.clear();

    Reference< frame::XController > xChartController( m_xChartController );
    if( xChartController.is() )
    {
        Reference< view::XSelectionSupplier > xSelectionSupplier( xChartController, uno::UNO_QUERY);
        ObjectIdentifier aSelectedOID;
        OUString aSelectedCID;
        if( xSelectionSupplier.is() )
        {
            aSelectedOID = ObjectIdentifier( xSelectionSupplier->getSelection() );
            aSelectedCID = aSelectedOID.getObjectCID();
        }

        Reference< chart2::XChartDocument > xChartDoc( xChartController->getModel(), uno::UNO_QUERY );
        ObjectType eType( aSelectedOID.getObjectType() );
        bool bAddSelectionToList = false;
        if ( eType == OBJECTTYPE_DATA_POINT || eType == OBJECTTYPE_DATA_LABEL || eType == OBJECTTYPE_SHAPE )
            bAddSelectionToList = true;

        Reference< uno::XInterface > xChartView;
        Reference< lang::XMultiServiceFactory > xFact( xChartController->getModel(), uno::UNO_QUERY );
        if( xFact.is() )
            xChartView = xFact->createInstance( CHART_VIEW_SERVICE_NAME );
        ExplicitValueProvider* pExplicitValueProvider = nullptr; //ExplicitValueProvider::getExplicitValueProvider(xChartView); this creates all visible data points, that's too much
        ObjectHierarchy aHierarchy( xChartDoc, pExplicitValueProvider, true /*bFlattenDiagram*/, true /*bOrderingForElementSelector*/ );
        lcl_addObjectsToList( aHierarchy, ::chart::ObjectHierarchy::getRootNodeOID(), m_aEntries, 0, xChartDoc );

        if( bAddSelectionToList )
        {
            if ( aSelectedOID.isAutoGeneratedObject() )
            {
                OUString aSeriesCID = ObjectIdentifier::createClassifiedIdentifierForParticle( ObjectIdentifier::getSeriesParticleFromCID( aSelectedCID ) );
                std::vector< ListBoxEntryData >::iterator aIt = std::find_if(m_aEntries.begin(), m_aEntries.end(),
                    [&aSeriesCID](const ListBoxEntryData& rEntry) { return rEntry.OID.getObjectCID().match(aSeriesCID); });
                if (aIt != m_aEntries.end())
                {
                    ListBoxEntryData aEntry;
                    aEntry.UIName = ObjectNameProvider::getNameForCID( aSelectedCID, xChartDoc );
                    aEntry.OID = aSelectedOID;
                    ++aIt;
                    if( aIt != m_aEntries.end() )
                        m_aEntries.insert(aIt, aEntry);
                    else
                        m_aEntries.push_back( aEntry );
                }
            }
            else if ( aSelectedOID.isAdditionalShape() )
            {
                ListBoxEntryData aEntry;
                SdrObject* pSelectedObj = DrawViewWrapper::getSdrObject( aSelectedOID.getAdditionalShape() );
                OUString aName = pSelectedObj ? pSelectedObj->GetName() : OUString();
                aEntry.UIName = ( aName.isEmpty() ?  SchResId( STR_OBJECT_SHAPE ) : aName );
                aEntry.OID = aSelectedOID;
                m_aEntries.push_back( aEntry );
            }
        }

        m_xWidget->freeze();
        sal_uInt16 nEntryPosToSelect = 0; bool bSelectionFound = false;
        sal_uInt16 nN=0;
        for (auto const& entry : m_aEntries)
        {
            m_xWidget->append_text(entry.UIName);
            if ( !bSelectionFound && aSelectedOID == entry.OID )
            {
                nEntryPosToSelect = nN;
                bSelectionFound = true;
            }
            ++nN;
        }
        m_xWidget->thaw();

        if( bSelectionFound )
            m_xWidget->set_active(nEntryPosToSelect);
    }
    m_xWidget->save_value(); //remind current selection pos
}

void SelectorListBox::ReleaseFocus_Impl()
{
    if ( !m_bReleaseFocus )
    {
        m_bReleaseFocus = true;
        return;
    }

    Reference< frame::XController > xController( m_xChartController );
    Reference< frame::XFrame > xFrame( xController->getFrame() );
    if ( xFrame.is() && xFrame->getContainerWindow().is() )
        xFrame->getContainerWindow()->setFocus();
}

IMPL_LINK(SelectorListBox, SelectHdl, weld::ComboBox&, rComboBox, void)
{
    if (rComboBox.changed_by_direct_pick())
    {
        const sal_Int32 nPos = rComboBox.get_active();
        if (o3tl::make_unsigned(nPos) < m_aEntries.size())
        {
            ObjectIdentifier aOID = m_aEntries[nPos].OID;
            Reference< view::XSelectionSupplier > xSelectionSupplier( m_xChartController.get(), uno::UNO_QUERY );
            if( xSelectionSupplier.is() )
                xSelectionSupplier->select( aOID.getAny() );
        }
        ReleaseFocus_Impl();
    }
}

IMPL_LINK(SelectorListBox, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    bool bHandled = false;

    sal_uInt16 nCode = rKEvt.GetKeyCode().GetCode();

    switch ( nCode )
    {
        case KEY_RETURN:
        case KEY_TAB:
        {
            if ( nCode == KEY_TAB )
                m_bReleaseFocus = false;
            else
                bHandled = true;
            SelectHdl(*m_xWidget);
            break;
        }

        case KEY_ESCAPE:
            m_xWidget->set_active_text(m_xWidget->get_saved_value()); //restore saved selection
            ReleaseFocus_Impl();
            break;
    }

    return bHandled || ChildKeyInput(rKEvt);
}

IMPL_LINK_NOARG(SelectorListBox, FocusOutHdl, weld::Widget&, void)
{
    if (m_xWidget && !m_xWidget->has_focus()) // comboboxes can be comprised of multiple widgets, ensure all have lost focus
        m_xWidget->set_active_text(m_xWidget->get_saved_value());
}

OUString SAL_CALL ElementSelectorToolbarController::getImplementationName()
{
    return lcl_aServiceName;
}

sal_Bool SAL_CALL ElementSelectorToolbarController::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL ElementSelectorToolbarController::getSupportedServiceNames()
{
    return { "com.sun.star.frame.ToolbarController" };
}
ElementSelectorToolbarController::ElementSelectorToolbarController()
{
}
ElementSelectorToolbarController::~ElementSelectorToolbarController()
{
}
// XInterface
Any SAL_CALL ElementSelectorToolbarController::queryInterface( const Type& _rType )
{
    Any aReturn = ToolboxController::queryInterface(_rType);
    if (!aReturn.hasValue())
        aReturn = ElementSelectorToolbarController_BASE::queryInterface(_rType);
    return aReturn;
}
void SAL_CALL ElementSelectorToolbarController::acquire() throw ()
{
    ToolboxController::acquire();
}
void SAL_CALL ElementSelectorToolbarController::release() throw ()
{
    ToolboxController::release();
}
void SAL_CALL ElementSelectorToolbarController::statusChanged( const frame::FeatureStateEvent& rEvent )
{
    if( m_apSelectorListBox )
    {
        SolarMutexGuard aSolarMutexGuard;
        if ( rEvent.FeatureURL.Path == "ChartElementSelector" )
        {
            Reference< frame::XController > xChartController;
            rEvent.State >>= xChartController;
            m_apSelectorListBox->SetChartController( xChartController );
            m_apSelectorListBox->UpdateChartElementsListAndSelection();
        }
    }
}
uno::Reference< awt::XWindow > SAL_CALL ElementSelectorToolbarController::createItemWindow( const uno::Reference< awt::XWindow >& xParent )
{
    uno::Reference< awt::XWindow > xItemWindow;
    if( !m_apSelectorListBox )
    {
        VclPtr<vcl::Window> pParent = VCLUnoHelper::GetWindow( xParent );
        if( pParent )
        {
            m_apSelectorListBox.reset(VclPtr<SelectorListBox>::Create(pParent));
        }
    }
    if( m_apSelectorListBox )
        xItemWindow = VCLUnoHelper::GetInterface( m_apSelectorListBox.get() );
    return xItemWindow;
}

} // chart2

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart_ElementSelectorToolbarController_get_implementation(css::uno::XComponentContext *,
                                                                            css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new chart::ElementSelectorToolbarController );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
