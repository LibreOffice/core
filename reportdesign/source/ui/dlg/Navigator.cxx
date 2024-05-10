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

#include <Navigator.hxx>

#include <strings.hxx>
#include <bitmaps.hlst>
#include <ReportController.hxx>
#include <UITools.hxx>
#include <reportformula.hxx>
#include <com/sun/star/report/XFixedText.hpp>
#include <com/sun/star/report/XFixedLine.hpp>
#include <com/sun/star/report/XFormattedField.hpp>
#include <com/sun/star/report/XImageControl.hpp>
#include <com/sun/star/report/XShape.hpp>
#include <helpids.h>
#include <strings.hrc>
#include <rptui_slotid.hrc>
#include <comphelper/propmultiplex.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/containermultiplexer.hxx>
#include <cppuhelper/basemutex.hxx>
#include <comphelper/SelectionMultiplex.hxx>
#include <utility>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <vcl/commandevent.hxx>
#include <ReportVisitor.hxx>
#include <core_resource.hxx>
#include <rtl/ref.hxx>
#include <svx/svxids.hrc>

#include <memory>
#include <string_view>

namespace rptui
{
using namespace ::com::sun::star;
using namespace ::comphelper;

static OUString lcl_getImageId(const uno::Reference< report::XReportComponent>& _xElement)
{
    OUString sId;
    uno::Reference< report::XFixedLine> xFixedLine(_xElement,uno::UNO_QUERY);
    if ( uno::Reference< report::XFixedText>(_xElement,uno::UNO_QUERY).is() )
        sId = RID_SVXBMP_FM_FIXEDTEXT;
    else if ( xFixedLine.is() )
        sId = xFixedLine->getOrientation() ? RID_SVXBMP_INSERT_VFIXEDLINE : RID_SVXBMP_INSERT_HFIXEDLINE;
    else if ( uno::Reference< report::XFormattedField>(_xElement,uno::UNO_QUERY).is() )
        sId = RID_SVXBMP_FM_EDIT;
    else if ( uno::Reference< report::XImageControl>(_xElement,uno::UNO_QUERY).is() )
        sId = RID_SVXBMP_FM_IMAGECONTROL;
    else if ( uno::Reference< report::XShape>(_xElement,uno::UNO_QUERY).is() )
        sId = RID_SVXBMP_DRAWTBX_CS_BASIC;
    return sId;
}

static OUString lcl_getName(const uno::Reference< beans::XPropertySet>& _xElement)
{
    OSL_ENSURE(_xElement.is(),"Found report element which is NULL!");
    OUString sTempName;
    _xElement->getPropertyValue(PROPERTY_NAME) >>= sTempName;
    OUStringBuffer sName(sTempName);
    uno::Reference< report::XFixedText> xFixedText(_xElement,uno::UNO_QUERY);
    uno::Reference< report::XReportControlModel> xReportModel(_xElement,uno::UNO_QUERY);
    if ( xFixedText.is() )
    {
        sName.append(" : " + xFixedText->getLabel());
    }
    else if ( xReportModel.is() && _xElement->getPropertySetInfo()->hasPropertyByName(PROPERTY_DATAFIELD) )
    {
        ReportFormula aFormula( xReportModel->getDataField() );
        if ( aFormula.isValid() )
        {
            sName.append(" : " + aFormula.getUndecoratedContent() );
        }
    }
    return sName.makeStringAndClear();
}

class NavigatorTree : public ::cppu::BaseMutex
                    , public reportdesign::ITraverseReport
                    , public comphelper::OSelectionChangeListener
                    , public ::comphelper::OPropertyChangeListener
{
    class UserData;
    friend class UserData;
    class UserData : public ::cppu::BaseMutex
                    ,public ::comphelper::OPropertyChangeListener
                    ,public ::comphelper::OContainerListener
    {
        uno::Reference< uno::XInterface >                           m_xContent;
        ::rtl::Reference< comphelper::OPropertyChangeMultiplexer>   m_pListener;
        ::rtl::Reference< comphelper::OContainerListenerAdapter>    m_pContainerListener;
        NavigatorTree*                                              m_pTree;
    public:
        UserData(NavigatorTree* pTree, uno::Reference<uno::XInterface> xContent);
        virtual ~UserData() override;

        const uno::Reference< uno::XInterface >& getContent() const { return m_xContent; }
        void setContent(const uno::Reference< uno::XInterface >& _xContent) { m_xContent = _xContent; }

    protected:
        // OPropertyChangeListener
        virtual void _propertyChanged(const beans::PropertyChangeEvent& _rEvent) override;

        // OContainerListener
        virtual void _elementInserted( const container::ContainerEvent& _rEvent ) override;
        virtual void _elementRemoved( const container::ContainerEvent& Event ) override;
        virtual void _elementReplaced( const container::ContainerEvent& _rEvent ) override;
        virtual void _disposing(const lang::EventObject& _rSource) override;
    };

    std::unique_ptr<weld::TreeView>                                             m_xTreeView;
    OReportController&                                                          m_rController;
    std::unique_ptr<weld::TreeIter>                                             m_xMasterReport;
    ::rtl::Reference< comphelper::OPropertyChangeMultiplexer>                   m_pReportListener;
    ::rtl::Reference< comphelper::OSelectionChangeMultiplexer>                  m_pSelectionListener;

    void insertEntry(const OUString& rName, const weld::TreeIter* pParent, const OUString& rImageId, int nPosition, const UserData* pData, weld::TreeIter& rRet);

    void traverseSection(const uno::Reference<report::XSection>& xSection, const weld::TreeIter* pParent, const OUString& rImageId, int nPosition = -1);
    void traverseFunctions(const uno::Reference< report::XFunctions>& xFunctions, const weld::TreeIter* pParent);

protected:
    // OSelectionChangeListener
    virtual void _disposing(const lang::EventObject& _rSource) override;

    // OPropertyChangeListener
    virtual void _propertyChanged(const beans::PropertyChangeEvent& _rEvent) override;

    // OContainerListener Helper
    void _elementInserted( const container::ContainerEvent& _rEvent );
    void _elementRemoved( const container::ContainerEvent& Event );
    void _elementReplaced( const container::ContainerEvent& _rEvent );

public:
    NavigatorTree(std::unique_ptr<weld::TreeView>, OReportController& rController);
    virtual ~NavigatorTree() override;

    DECL_LINK(OnEntrySelDesel, weld::TreeView&, void);
    DECL_LINK(CommandHdl, const CommandEvent&, bool);

    virtual void _selectionChanged( const lang::EventObject& aEvent ) override;

    // ITraverseReport
    virtual void traverseReport(const uno::Reference< report::XReportDefinition>& xReport) override;
    virtual void traverseReportFunctions(const uno::Reference< report::XFunctions>& xFunctions) override;
    virtual void traverseReportHeader(const uno::Reference< report::XSection>& xSection) override;
    virtual void traverseReportFooter(const uno::Reference< report::XSection>& xSection) override;
    virtual void traversePageHeader(const uno::Reference< report::XSection>& xSection) override;
    virtual void traversePageFooter(const uno::Reference< report::XSection>& xSection) override;

    virtual void traverseGroups(const uno::Reference< report::XGroups>& xGroups) override;
    virtual void traverseGroup(const uno::Reference< report::XGroup>& xGroup) override;
    virtual void traverseGroupFunctions(const uno::Reference< report::XFunctions>& xFunctions) override;
    virtual void traverseGroupHeader(const uno::Reference< report::XSection>& xSection) override;
    virtual void traverseGroupFooter(const uno::Reference< report::XSection>& xSection) override;

    virtual void traverseDetail(const uno::Reference< report::XSection>& xSection) override;

    bool find(const uno::Reference<uno::XInterface>& xContent, weld::TreeIter& rIter);
    void removeEntry(const weld::TreeIter& rEntry, bool bRemove = true);

    void grab_focus() { m_xTreeView->grab_focus(); }

    void set_text(const weld::TreeIter& rIter, const OUString& rStr)
    {
        m_xTreeView->set_text(rIter, rStr);
    }

    void expand_row(const weld::TreeIter& rIter)
    {
        m_xTreeView->expand_row(rIter);
    }

    std::unique_ptr<weld::TreeIter> make_iterator() const
    {
        return m_xTreeView->make_iterator();
    }

    int iter_n_children(const weld::TreeIter& rIter) const
    {
        return m_xTreeView->iter_n_children(rIter);
    }
};

NavigatorTree::NavigatorTree(std::unique_ptr<weld::TreeView> xTreeView, OReportController& rController)
    : OPropertyChangeListener(m_aMutex)
    , m_xTreeView(std::move(xTreeView))
    , m_rController(rController)
{
    m_xTreeView->set_size_request(m_xTreeView->get_approximate_digit_width() * 25, m_xTreeView->get_height_rows(18));

    m_pReportListener = new OPropertyChangeMultiplexer(this,m_rController.getReportDefinition());
    m_pReportListener->addProperty(PROPERTY_PAGEHEADERON);
    m_pReportListener->addProperty(PROPERTY_PAGEFOOTERON);
    m_pReportListener->addProperty(PROPERTY_REPORTHEADERON);
    m_pReportListener->addProperty(PROPERTY_REPORTFOOTERON);

    m_pSelectionListener = new OSelectionChangeMultiplexer(this,&m_rController);

    m_xTreeView->set_help_id(HID_REPORT_NAVIGATOR_TREE);

    m_xTreeView->set_selection_mode(SelectionMode::Multiple);

    m_xTreeView->connect_changed(LINK(this, NavigatorTree, OnEntrySelDesel));
    m_xTreeView->connect_popup_menu(LINK(this, NavigatorTree, CommandHdl));
}

NavigatorTree::~NavigatorTree()
{
    m_xTreeView->all_foreach([this](weld::TreeIter& rIter) {
        UserData* pData = weld::fromId<UserData*>(m_xTreeView->get_id(rIter));
        delete pData;
        return false;
    });
    m_pSelectionListener->dispose();
    m_pReportListener->dispose();
}

namespace
{
    sal_uInt16 mapIdent(std::u16string_view rIdent)
    {
        if (rIdent == u"sorting")
            return SID_SORTINGANDGROUPING;
        else if (rIdent == u"page")
            return SID_PAGEHEADERFOOTER;
        else if (rIdent == u"report")
            return SID_REPORTHEADERFOOTER;
        else if (rIdent == u"function")
            return SID_RPT_NEW_FUNCTION;
        else if (rIdent == u"properties")
            return SID_SHOW_PROPERTYBROWSER;
        else if (rIdent == u"delete")
            return SID_DELETE;
        return 0;
    }
}

IMPL_LINK(NavigatorTree, CommandHdl, const CommandEvent&, rEvt, bool)
{
    bool bHandled = false;
    switch( rEvt.GetCommand())
    {
    case CommandEventId::ContextMenu:
        {
            UserData* pData = weld::fromId<UserData*>(m_xTreeView->get_selected_id());
            if (!pData)
                break;

            uno::Reference< report::XFunctionsSupplier> xSupplier(pData->getContent(),uno::UNO_QUERY);
            uno::Reference< report::XFunctions> xFunctions(pData->getContent(),uno::UNO_QUERY);
            uno::Reference< report::XGroup> xGroup(pData->getContent(),uno::UNO_QUERY);
            bool bDeleteAllowed = m_rController.isEditable() && (xGroup.is() ||
                                      uno::Reference< report::XFunction>(pData->getContent(),uno::UNO_QUERY).is());

            std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(m_xTreeView.get(), u"modules/dbreport/ui/navigatormenu.ui"_ustr));
            std::unique_ptr<weld::Menu> xContextMenu(xBuilder->weld_menu(u"menu"_ustr));

            const OUString aIds[] = { u"sorting"_ustr, u"page"_ustr, u"report"_ustr, u"function"_ustr, u"properties"_ustr, u"delete"_ustr };
            for (size_t i = 0; i < SAL_N_ELEMENTS(aIds); ++i)
            {
                sal_uInt16 nSId = mapIdent(aIds[i]);

                if (aIds[i] == "page" || aIds[i] == "report" || aIds[i] == "properties")
                    xContextMenu->set_active(aIds[i], m_rController.isCommandChecked(nSId));
                bool bEnabled = m_rController.isCommandEnabled(nSId);
                if (nSId == SID_RPT_NEW_FUNCTION)
                    xContextMenu->set_sensitive(aIds[i], m_rController.isEditable() && (xSupplier.is() || xFunctions.is()));
                // special condition, check for function and group
                else if (nSId == SID_DELETE)
                    xContextMenu->set_sensitive(aIds[i], bDeleteAllowed);
                else
                    xContextMenu->set_sensitive(aIds[i], bEnabled);
            }

            // the point that was clicked on
            ::Point aWhere(rEvt.GetMousePosPixel());
            OUString sCurItemIdent = xContextMenu->popup_at_rect(m_xTreeView.get(), tools::Rectangle(aWhere, Size(1,1)));
            if (!sCurItemIdent.isEmpty())
            {
                sal_uInt16 nId = mapIdent(sCurItemIdent);
                uno::Sequence< beans::PropertyValue> aArgs;
                if ( nId == SID_RPT_NEW_FUNCTION )
                {
                    aArgs.realloc(1);
                    aArgs.getArray()[0].Value <<= (xFunctions.is() ? xFunctions : xSupplier->getFunctions());
                }
                else if ( nId == SID_DELETE )
                {
                    if ( xGroup.is() )
                        nId = SID_GROUP_REMOVE;
                    aArgs = { comphelper::makePropertyValue(PROPERTY_GROUP, pData->getContent()) };
                }
                m_rController.executeUnChecked(nId,aArgs);
            }

            bHandled = true;
        }
        break;
        default: break;
    }

    return bHandled;
}

IMPL_LINK_NOARG(NavigatorTree, OnEntrySelDesel, weld::TreeView&, void)
{
    if ( !m_pSelectionListener->locked() )
    {
        m_pSelectionListener->lock();
        std::unique_ptr<weld::TreeIter> xEntry = m_xTreeView->make_iterator();
        bool bEntry = m_xTreeView->get_cursor(xEntry.get());
        uno::Any aSelection;
        if (bEntry && m_xTreeView->is_selected(*xEntry))
            aSelection <<= weld::fromId<UserData*>(m_xTreeView->get_id(*xEntry))->getContent();
        m_rController.select(aSelection);
        m_pSelectionListener->unlock();
    }
}

void NavigatorTree::_selectionChanged( const lang::EventObject& aEvent )
{
    m_pSelectionListener->lock();
    uno::Reference< view::XSelectionSupplier> xSelectionSupplier(aEvent.Source,uno::UNO_QUERY);
    uno::Any aSec = xSelectionSupplier->getSelection();
    uno::Sequence< uno::Reference< report::XReportComponent > > aSelection;
    aSec >>= aSelection;
    std::unique_ptr<weld::TreeIter> xEntry = m_xTreeView->make_iterator();
    if ( !aSelection.hasElements() )
    {
        uno::Reference< uno::XInterface> xSelection(aSec,uno::UNO_QUERY);
        bool bEntry = find(xSelection, *xEntry);
        if (bEntry && !m_xTreeView->is_selected(*xEntry))
        {
            m_xTreeView->select(*xEntry);
            m_xTreeView->set_cursor(*xEntry);
        }
        else if (!bEntry)
            m_xTreeView->unselect_all();
    }
    else
    {
        for (const uno::Reference<report::XReportComponent>& rElem : aSelection)
        {
            bool bEntry = find(rElem, *xEntry);
            if (bEntry && !m_xTreeView->is_selected(*xEntry))
            {
                m_xTreeView->select(*xEntry);
                m_xTreeView->set_cursor(*xEntry);
            }
        }
    }
    m_pSelectionListener->unlock();
}

void NavigatorTree::insertEntry(const OUString& rName, const weld::TreeIter* pParent, const OUString& rImageId,
                                int nPosition, const UserData* pData, weld::TreeIter& rRet)
{
    OUString sId = pData ? weld::toId(pData) : OUString();
    m_xTreeView->insert(pParent, nPosition, &rName, &sId, nullptr, nullptr, false, &rRet);
    if (!rImageId.isEmpty())
        m_xTreeView->set_image(rRet, rImageId);
}

void NavigatorTree::traverseSection(const uno::Reference<report::XSection>& xSection, const weld::TreeIter* pParent, const OUString& rImageId, int nPosition)
{
    std::unique_ptr<weld::TreeIter> xSectionIter = m_xTreeView->make_iterator();
    std::unique_ptr<weld::TreeIter> xScratch = m_xTreeView->make_iterator();
    insertEntry(xSection->getName(), pParent, rImageId, nPosition, new UserData(this, xSection), *xSectionIter);
    const sal_Int32 nCount = xSection->getCount();
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        uno::Reference< report::XReportComponent> xElement(xSection->getByIndex(i), uno::UNO_QUERY_THROW);
        insertEntry(lcl_getName(xElement), xSectionIter.get(), lcl_getImageId(xElement), -1, new UserData(this, xElement), *xScratch);
        uno::Reference< report::XReportDefinition> xSubReport(xElement,uno::UNO_QUERY);
        if ( xSubReport.is() )
        {
            bool bMasterReport = find(xSection->getReportDefinition(), *xScratch);
            if (!bMasterReport)
                m_xMasterReport.reset();
            else
                m_xMasterReport = m_xTreeView->make_iterator(xScratch.get());
            reportdesign::OReportVisitor aSubVisitor(this);
            aSubVisitor.start(xSubReport);
        }
    }
}

void NavigatorTree::traverseFunctions(const uno::Reference< report::XFunctions>& xFunctions, const weld::TreeIter* pParent)
{
    std::unique_ptr<weld::TreeIter> xFunctionIter = m_xTreeView->make_iterator();
    std::unique_ptr<weld::TreeIter> xScratch = m_xTreeView->make_iterator();
    insertEntry(RptResId(RID_STR_FUNCTIONS), pParent, RID_SVXBMP_RPT_NEW_FUNCTION, -1, new UserData(this, xFunctions), *xFunctionIter);
    const sal_Int32 nCount = xFunctions->getCount();
    for (sal_Int32 i = 0; i< nCount; ++i)
    {
        uno::Reference< report::XFunction> xElement(xFunctions->getByIndex(i),uno::UNO_QUERY);
        insertEntry(xElement->getName(), xFunctionIter.get(), RID_SVXBMP_RPT_NEW_FUNCTION, -1, new UserData(this,xElement), *xScratch);
    }
}

bool NavigatorTree::find(const uno::Reference<uno::XInterface>& xContent, weld::TreeIter& rRet)
{
    bool bRet = false;
    if (xContent.is())
    {
        m_xTreeView->all_foreach([this, &xContent, &bRet, &rRet](weld::TreeIter& rIter) {
            UserData* pData = weld::fromId<UserData*>(m_xTreeView->get_id(rIter));
            if (pData->getContent() == xContent)
            {
                m_xTreeView->copy_iterator(rIter, rRet);
                bRet = true;
                return true;
            }
            return false;
        });
    }
    return bRet;
}

// ITraverseReport

void NavigatorTree::traverseReport(const uno::Reference< report::XReportDefinition>& xReport)
{
    std::unique_ptr<weld::TreeIter> xScratch = m_xTreeView->make_iterator();
    insertEntry(xReport->getName(), m_xMasterReport.get(), RID_SVXBMP_SELECT_REPORT,-1, new UserData(this, xReport), *xScratch);
}

void NavigatorTree::traverseReportFunctions(const uno::Reference< report::XFunctions>& xFunctions)
{
    std::unique_ptr<weld::TreeIter> xReport = m_xTreeView->make_iterator();
    bool bReport = find(xFunctions->getParent(), *xReport);
    if (!bReport)
        xReport.reset();
    traverseFunctions(xFunctions, xReport.get());
}

void NavigatorTree::traverseReportHeader(const uno::Reference< report::XSection>& xSection)
{
    std::unique_ptr<weld::TreeIter> xReport = m_xTreeView->make_iterator();
    bool bReport = find(xSection->getReportDefinition(), *xReport);
    if (!bReport)
        xReport.reset();
    traverseSection(xSection, xReport.get(), RID_SVXBMP_REPORTHEADERFOOTER);
}

void NavigatorTree::traverseReportFooter(const uno::Reference< report::XSection>& xSection)
{
    std::unique_ptr<weld::TreeIter> xReport = m_xTreeView->make_iterator();
    bool bReport = find(xSection->getReportDefinition(), *xReport);
    if (!bReport)
        xReport.reset();
    traverseSection(xSection, xReport.get(), RID_SVXBMP_REPORTHEADERFOOTER);
}

void NavigatorTree::traversePageHeader(const uno::Reference< report::XSection>& xSection)
{
    std::unique_ptr<weld::TreeIter> xReport = m_xTreeView->make_iterator();
    bool bReport = find(xSection->getReportDefinition(), *xReport);
    if (!bReport)
        xReport.reset();
    traverseSection(xSection, xReport.get(), RID_SVXBMP_PAGEHEADERFOOTER);
}

void NavigatorTree::traversePageFooter(const uno::Reference< report::XSection>& xSection)
{
    std::unique_ptr<weld::TreeIter> xReport = m_xTreeView->make_iterator();
    bool bReport = find(xSection->getReportDefinition(), *xReport);
    if (!bReport)
        xReport.reset();
    traverseSection(xSection, xReport.get(), RID_SVXBMP_PAGEHEADERFOOTER);
}

void NavigatorTree::traverseGroups(const uno::Reference< report::XGroups>& xGroups)
{
    std::unique_ptr<weld::TreeIter> xReport = m_xTreeView->make_iterator();
    bool bReport = find(xGroups->getReportDefinition(), *xReport);
    if (!bReport)
        xReport.reset();
    std::unique_ptr<weld::TreeIter> xScratch = m_xTreeView->make_iterator();
    insertEntry(RptResId(RID_STR_GROUPS), xReport.get(), RID_SVXBMP_SORTINGANDGROUPING, -1, new UserData(this, xGroups), *xScratch);
}

void NavigatorTree::traverseGroup(const uno::Reference< report::XGroup>& xGroup)
{
    uno::Reference< report::XGroups> xGroups(xGroup->getParent(),uno::UNO_QUERY);
    std::unique_ptr<weld::TreeIter> xGroupsIter = m_xTreeView->make_iterator();
    bool bGroups = find(xGroups, *xGroupsIter);
    OSL_ENSURE(bGroups, "No Groups inserted so far. Why!");
    if (!bGroups)
        xGroupsIter.reset();
    std::unique_ptr<weld::TreeIter> xScratch = m_xTreeView->make_iterator();
    insertEntry(xGroup->getExpression(), xGroupsIter.get(), RID_SVXBMP_GROUP, rptui::getPositionInIndexAccess(xGroups,xGroup), new UserData(this,xGroup), *xScratch);
}

void NavigatorTree::traverseGroupFunctions(const uno::Reference< report::XFunctions>& xFunctions)
{
    std::unique_ptr<weld::TreeIter> xReport = m_xTreeView->make_iterator();
    bool bReport = find(xFunctions->getParent(), *xReport);
    if (!bReport)
        xReport.reset();
    traverseFunctions(xFunctions, xReport.get());
}

void NavigatorTree::traverseGroupHeader(const uno::Reference< report::XSection>& xSection)
{
    std::unique_ptr<weld::TreeIter> xReport = m_xTreeView->make_iterator();
    bool bReport = find(xSection->getGroup(), *xReport);
    OSL_ENSURE(bReport, "No group found");
    if (!bReport)
        xReport.reset();
    traverseSection(xSection, xReport.get(), RID_SVXBMP_GROUPHEADER, 1);
}

void NavigatorTree::traverseGroupFooter(const uno::Reference< report::XSection>& xSection)
{
    std::unique_ptr<weld::TreeIter> xReport = m_xTreeView->make_iterator();
    bool bReport = find(xSection->getGroup(), *xReport);
    OSL_ENSURE(bReport, "No group found");
    if (!bReport)
        xReport.reset();
    traverseSection(xSection, xReport.get(), RID_SVXBMP_GROUPFOOTER);
}

void NavigatorTree::traverseDetail(const uno::Reference< report::XSection>& xSection)
{
    uno::Reference< report::XReportDefinition> xReport = xSection->getReportDefinition();
    std::unique_ptr<weld::TreeIter> xParent = m_xTreeView->make_iterator();
    bool bParent = find(xReport, *xParent);
    if (!bParent)
        xParent.reset();
    traverseSection(xSection, xParent.get(), RID_SVXBMP_ICON_DETAIL);
}

void NavigatorTree::_propertyChanged(const beans::PropertyChangeEvent& _rEvent)
{
    uno::Reference< report::XReportDefinition> xReport(_rEvent.Source,uno::UNO_QUERY);
    if ( !xReport.is() )
        return;

    bool bEnabled = false;
    _rEvent.NewValue >>= bEnabled;
    if ( !bEnabled )
        return;

    std::unique_ptr<weld::TreeIter> xParent = m_xTreeView->make_iterator();
    bool bParent = find(xReport, *xParent);
    if (!bParent)
        xParent.reset();
    if ( _rEvent.PropertyName == PROPERTY_REPORTHEADERON )
    {
        int nPos = xReport->getReportHeaderOn() ? 2 : 1;
        traverseSection(xReport->getReportHeader(),xParent.get(),RID_SVXBMP_REPORTHEADERFOOTER,nPos);
    }
    else if ( _rEvent.PropertyName == PROPERTY_PAGEHEADERON )
    {
        traverseSection(xReport->getPageHeader(),xParent.get(), RID_SVXBMP_PAGEHEADERFOOTER,1);
    }
    else if ( _rEvent.PropertyName == PROPERTY_PAGEFOOTERON )
        traverseSection(xReport->getPageFooter(),xParent.get(), RID_SVXBMP_PAGEHEADERFOOTER);
    else if ( _rEvent.PropertyName == PROPERTY_REPORTFOOTERON )
    {
        int nPos = -1;
        if (xReport->getPageFooterOn() && xParent)
            nPos = m_xTreeView->iter_n_children(*xParent) - 1;
        traverseSection(xReport->getReportFooter(),xParent.get(),RID_SVXBMP_REPORTHEADERFOOTER,nPos);
    }
}

void NavigatorTree::_elementInserted( const container::ContainerEvent& _rEvent )
{
    std::unique_ptr<weld::TreeIter> xEntry = m_xTreeView->make_iterator();
    bool bEntry = find(_rEvent.Source, *xEntry);
    if (!bEntry)
        xEntry.reset();
    uno::Reference<beans::XPropertySet> xProp(_rEvent.Element,uno::UNO_QUERY_THROW);
    OUString sName;
    uno::Reference< beans::XPropertySetInfo> xInfo = xProp->getPropertySetInfo();
    if ( xInfo.is() )
    {
        if ( xInfo->hasPropertyByName(PROPERTY_NAME) )
            xProp->getPropertyValue(PROPERTY_NAME) >>= sName;
        else if ( xInfo->hasPropertyByName(PROPERTY_EXPRESSION) )
            xProp->getPropertyValue(PROPERTY_EXPRESSION) >>= sName;
    }
    uno::Reference< report::XGroup> xGroup(xProp,uno::UNO_QUERY);
    if ( xGroup.is() )
    {
        reportdesign::OReportVisitor aSubVisitor(this);
        aSubVisitor.start(xGroup);
    }
    else
    {
        uno::Reference< report::XReportComponent> xElement(xProp,uno::UNO_QUERY);
        if ( xProp.is() )
            sName = lcl_getName(xProp);
        std::unique_ptr<weld::TreeIter> xScratch = m_xTreeView->make_iterator();
        insertEntry(sName, xEntry.get(), (!xElement.is() ? RID_SVXBMP_RPT_NEW_FUNCTION : lcl_getImageId(xElement)),
                    -1, new UserData(this,xProp), *xScratch);
    }
    if (bEntry && !m_xTreeView->get_row_expanded(*xEntry))
        m_xTreeView->expand_row(*xEntry);
}

void NavigatorTree::_elementRemoved( const container::ContainerEvent& _rEvent )
{
    uno::Reference<beans::XPropertySet> xProp(_rEvent.Element,uno::UNO_QUERY);
    std::unique_ptr<weld::TreeIter> xEntry = m_xTreeView->make_iterator();
    bool bEntry = find(xProp, *xEntry);
    OSL_ENSURE(bEntry,"NavigatorTree::_elementRemoved: No Entry found!");

    if (bEntry)
    {
        removeEntry(*xEntry);
    }
}

void NavigatorTree::_elementReplaced( const container::ContainerEvent& _rEvent )
{
    uno::Reference<beans::XPropertySet> xProp(_rEvent.ReplacedElement,uno::UNO_QUERY);
    std::unique_ptr<weld::TreeIter> xEntry = m_xTreeView->make_iterator();
    bool bEntry = find(xProp, *xEntry);
    if (bEntry)
    {
        UserData* pData = weld::fromId<UserData*>(m_xTreeView->get_id(*xEntry));
        xProp.set(_rEvent.Element,uno::UNO_QUERY);
        pData->setContent(xProp);
        OUString sName;
        xProp->getPropertyValue(PROPERTY_NAME) >>= sName;
        m_xTreeView->set_text(*xEntry, sName);
    }
}

void NavigatorTree::_disposing(const lang::EventObject& _rSource)
{
    std::unique_ptr<weld::TreeIter> xEntry = m_xTreeView->make_iterator();
    if (find(_rSource.Source, *xEntry))
        removeEntry(*xEntry);
}

void NavigatorTree::removeEntry(const weld::TreeIter& rEntry, bool bRemove)
{
    std::unique_ptr<weld::TreeIter> xChild = m_xTreeView->make_iterator(&rEntry);
    bool bChild = m_xTreeView->iter_children(*xChild);
    while (bChild)
    {
        removeEntry(*xChild, false);
        bChild = m_xTreeView->iter_next_sibling(*xChild);
    }
    delete weld::fromId<UserData*>(m_xTreeView->get_id(rEntry));
    if (bRemove)
        m_xTreeView->remove(rEntry);
}

NavigatorTree::UserData::UserData(NavigatorTree* pTree,uno::Reference<uno::XInterface> xContent)
    : OPropertyChangeListener(m_aMutex)
    , OContainerListener(m_aMutex)
    , m_xContent(std::move(xContent))
    , m_pTree(pTree)
{
    uno::Reference<beans::XPropertySet> xProp(m_xContent,uno::UNO_QUERY);
    if ( xProp.is() )
    {
        uno::Reference< beans::XPropertySetInfo> xInfo = xProp->getPropertySetInfo();
        if ( xInfo.is() )
        {
            m_pListener = new ::comphelper::OPropertyChangeMultiplexer(this,xProp);
            if ( xInfo->hasPropertyByName(PROPERTY_NAME) )
                m_pListener->addProperty(PROPERTY_NAME);
            else if ( xInfo->hasPropertyByName(PROPERTY_EXPRESSION) )
                m_pListener->addProperty(PROPERTY_EXPRESSION);
            if ( xInfo->hasPropertyByName(PROPERTY_DATAFIELD) )
                m_pListener->addProperty(PROPERTY_DATAFIELD);
            if ( xInfo->hasPropertyByName(PROPERTY_LABEL) )
                m_pListener->addProperty(PROPERTY_LABEL);
            if ( xInfo->hasPropertyByName(PROPERTY_HEADERON) )
                m_pListener->addProperty(PROPERTY_HEADERON);
            if ( xInfo->hasPropertyByName(PROPERTY_FOOTERON) )
                m_pListener->addProperty(PROPERTY_FOOTERON);
        }
    }
    uno::Reference< container::XContainer> xContainer(m_xContent,uno::UNO_QUERY);
    if ( xContainer.is() )
    {
        m_pContainerListener = new ::comphelper::OContainerListenerAdapter(this,xContainer);
    }
}

NavigatorTree::UserData::~UserData()
{
    if ( m_pContainerListener.is() )
        m_pContainerListener->dispose();
    if ( m_pListener.is() )
        m_pListener->dispose();
}

// OPropertyChangeListener
void NavigatorTree::UserData::_propertyChanged(const beans::PropertyChangeEvent& _rEvent)
{
    std::unique_ptr<weld::TreeIter> xEntry = m_pTree->make_iterator();
    bool bEntry = m_pTree->find(_rEvent.Source, *xEntry);
    OSL_ENSURE(bEntry,"No entry could be found! Why not!");
    if (!bEntry)
        return;
    const bool bFooterOn = (PROPERTY_FOOTERON == _rEvent.PropertyName);
    try
    {
        if ( bFooterOn || PROPERTY_HEADERON == _rEvent.PropertyName )
        {
            sal_Int32 nPos = 1;
            uno::Reference< report::XGroup> xGroup(_rEvent.Source,uno::UNO_QUERY);
            ::std::function<bool(OGroupHelper *)> pIsOn = ::std::mem_fn(&OGroupHelper::getHeaderOn);
            ::std::function<uno::Reference<report::XSection>(OGroupHelper *)> pMemFunSection = ::std::mem_fn(&OGroupHelper::getHeader);
            if ( bFooterOn )
            {
                pIsOn = ::std::mem_fn(&OGroupHelper::getFooterOn);
                pMemFunSection = ::std::mem_fn(&OGroupHelper::getFooter);
                nPos = m_pTree->iter_n_children(*xEntry) - 1;
            }

            OGroupHelper aGroupHelper(xGroup);
            if ( pIsOn(&aGroupHelper) )
            {
                if ( bFooterOn )
                    ++nPos;
                m_pTree->traverseSection(pMemFunSection(&aGroupHelper),xEntry.get(),bFooterOn ? RID_SVXBMP_GROUPFOOTER : RID_SVXBMP_GROUPHEADER,nPos);
            }
        }
        else if ( PROPERTY_EXPRESSION == _rEvent.PropertyName)
        {
            OUString sNewName;
            _rEvent.NewValue >>= sNewName;
            m_pTree->set_text(*xEntry, sNewName);
        }
        else if ( PROPERTY_DATAFIELD == _rEvent.PropertyName || PROPERTY_LABEL == _rEvent.PropertyName || PROPERTY_NAME == _rEvent.PropertyName )
        {
            uno::Reference<beans::XPropertySet> xProp(_rEvent.Source,uno::UNO_QUERY);
            m_pTree->set_text(*xEntry, lcl_getName(xProp));
        }
    }
    catch(const uno::Exception &)
    {}
}

void NavigatorTree::UserData::_elementInserted( const container::ContainerEvent& _rEvent )
{
    m_pTree->_elementInserted( _rEvent );
}

void NavigatorTree::UserData::_elementRemoved( const container::ContainerEvent& _rEvent )
{
    m_pTree->_elementRemoved( _rEvent );
}

void NavigatorTree::UserData::_elementReplaced( const container::ContainerEvent& _rEvent )
{
    m_pTree->_elementReplaced( _rEvent );
}

void NavigatorTree::UserData::_disposing(const lang::EventObject& _rSource)
{
    m_pTree->_disposing( _rSource );
}

ONavigator::ONavigator(weld::Window* pParent, OReportController& rController)
    : GenericDialogController(pParent, u"modules/dbreport/ui/floatingnavigator.ui"_ustr, u"FloatingNavigator"_ustr)
    , m_xReport(rController.getReportDefinition())
    , m_xNavigatorTree(std::make_unique<NavigatorTree>(m_xBuilder->weld_tree_view(u"treeview"_ustr), rController))
{
    reportdesign::OReportVisitor aVisitor(m_xNavigatorTree.get());
    aVisitor.start(m_xReport);
    std::unique_ptr<weld::TreeIter> xScratch = m_xNavigatorTree->make_iterator();
    if (m_xNavigatorTree->find(m_xReport, *xScratch))
        m_xNavigatorTree->expand_row(*xScratch);
    lang::EventObject aEvent(rController);
    m_xNavigatorTree->_selectionChanged(aEvent);
    m_xNavigatorTree->grab_focus();

    m_xDialog->connect_container_focus_changed(LINK(this, ONavigator, FocusChangeHdl));
}

ONavigator::~ONavigator()
{
}

IMPL_LINK_NOARG(ONavigator, FocusChangeHdl, weld::Container&, void)
{
    if (m_xDialog->has_toplevel_focus())
        m_xNavigatorTree->grab_focus();
}

} // rptui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
