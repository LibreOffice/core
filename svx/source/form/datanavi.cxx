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

#include <sal/config.h>

#include <memory>

#include <sal/log.hxx>
#include <datanavi.hxx>
#include <fmservs.hxx>

#include <bitmaps.hlst>
#include <fpicker/strings.hrc>
#include <o3tl/safeint.hxx>
#include <o3tl/string_view.hxx>
#include <svx/svxids.hrc>
#include <comphelper/diagnose_ex.hxx>
#include <unotools/resmgr.hxx>
#include <svx/xmlexchg.hxx>
#include <unotools/viewoptions.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <utility>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <vcl/weld.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/xforms/XFormsSupplier.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <comphelper/string.hxx>

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::dom::events;
using namespace ::svx;

constexpr OUString CFGNAME_DATANAVIGATOR = u"DataNavigator"_ustr;
constexpr OUString CFGNAME_SHOWDETAILS = u"ShowDetails"_ustr;
constexpr OUString MSG_VARIABLE = u"%1"_ustr;
constexpr OUStringLiteral MODELNAME = u"$MODELNAME";
constexpr OUStringLiteral INSTANCENAME = u"$INSTANCENAME";
constexpr OUStringLiteral ELEMENTNAME = u"$ELEMENTNAME";
constexpr OUStringLiteral ATTRIBUTENAME = u"$ATTRIBUTENAME";
constexpr OUStringLiteral SUBMISSIONNAME = u"$SUBMISSIONNAME";
constexpr OUStringLiteral BINDINGNAME = u"$BINDINGNAME";


namespace svxform
{

    // properties of instance
    constexpr OUStringLiteral PN_INSTANCE_MODEL = u"Instance";
    constexpr OUString PN_INSTANCE_ID = u"ID"_ustr;
    constexpr OUStringLiteral PN_INSTANCE_URL = u"URL";

    // properties of binding
    constexpr OUString PN_BINDING_ID = u"BindingID"_ustr;
    constexpr OUString PN_BINDING_EXPR = u"BindingExpression"_ustr;
    constexpr OUStringLiteral PN_BINDING_MODEL = u"Model";
    constexpr OUString PN_BINDING_NAMESPACES = u"ModelNamespaces"_ustr;
    constexpr OUString PN_READONLY_EXPR = u"ReadonlyExpression"_ustr;
    constexpr OUString PN_RELEVANT_EXPR = u"RelevantExpression"_ustr;
    constexpr OUString PN_REQUIRED_EXPR = u"RequiredExpression"_ustr;
    constexpr OUString PN_CONSTRAINT_EXPR = u"ConstraintExpression"_ustr;
    constexpr OUString PN_CALCULATE_EXPR = u"CalculateExpression"_ustr;
    constexpr OUString PN_BINDING_TYPE = u"Type"_ustr;

    // properties of submission
    constexpr OUString PN_SUBMISSION_ID = u"ID"_ustr;
    constexpr OUString PN_SUBMISSION_BIND = u"Bind"_ustr;
    constexpr OUString PN_SUBMISSION_REF = u"Ref"_ustr;
    constexpr OUString PN_SUBMISSION_ACTION = u"Action"_ustr;
    constexpr OUString PN_SUBMISSION_METHOD = u"Method"_ustr;
    constexpr OUString PN_SUBMISSION_REPLACE = u"Replace"_ustr;

    // other const strings
    constexpr OUString TRUE_VALUE = u"true()"_ustr;
    constexpr OUStringLiteral NEW_ELEMENT = u"newElement";
    constexpr OUStringLiteral NEW_ATTRIBUTE = u"newAttribute";
    constexpr OUString EVENTTYPE_CHARDATA = u"DOMCharacterDataModified"_ustr;
    constexpr OUString EVENTTYPE_ATTR = u"DOMAttrModified"_ustr;

    #define MIN_PAGE_COUNT          3 // at least one instance, one submission and one binding page

    struct ItemNode
    {
        Reference< css::xml::dom::XNode >   m_xNode;
        Reference< XPropertySet >           m_xPropSet;

        explicit ItemNode( const Reference< css::xml::dom::XNode >& _rxNode ) :
            m_xNode( _rxNode ) {}
        explicit ItemNode( const Reference< XPropertySet >& _rxSet ) :
            m_xPropSet( _rxSet ) {}
    };

    DataTreeDropTarget::DataTreeDropTarget(weld::TreeView& rWidget)
        : DropTargetHelper(rWidget.get_drop_target())
    {
    }

    sal_Int8 DataTreeDropTarget::AcceptDrop( const AcceptDropEvent& /*rEvt*/ )
    {
        return DND_ACTION_NONE;
    }

    sal_Int8 DataTreeDropTarget::ExecuteDrop( const ExecuteDropEvent& /*rEvt*/ )
    {
        return DND_ACTION_NONE;
    }

    IMPL_LINK(XFormsPage, PopupMenuHdl, const CommandEvent&, rCEvt, bool)
    {
        if (rCEvt.GetCommand() != CommandEventId::ContextMenu)
            return false;

        Point aPos(rCEvt.GetMousePosPixel());

        if (m_xItemList->get_dest_row_at_pos(aPos, m_xScratchIter.get(), false) && !m_xItemList->is_selected(*m_xScratchIter))
        {
            m_xItemList->select(*m_xScratchIter);
            ItemSelectHdl(*m_xItemList);
        }

        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(m_xItemList.get(), u"svx/ui/formdatamenu.ui"_ustr));
        m_xMenu = xBuilder->weld_menu(u"menu"_ustr);
        m_aRemovedMenuEntries.clear();

        if (DGTInstance == m_eGroup)
            m_aRemovedMenuEntries.insert(u"additem"_ustr);
        else
        {
            m_aRemovedMenuEntries.insert(u"addelement"_ustr);
            m_aRemovedMenuEntries.insert(u"addattribute"_ustr);

            if (DGTSubmission == m_eGroup)
            {
                m_xMenu->set_label(u"additem"_ustr, SvxResId(RID_STR_DATANAV_ADD_SUBMISSION));
                m_xMenu->set_label(u"edit"_ustr, SvxResId(RID_STR_DATANAV_EDIT_SUBMISSION));
                m_xMenu->set_label(u"delete"_ustr, SvxResId(RID_STR_DATANAV_REMOVE_SUBMISSION));
            }
            else
            {
                m_xMenu->set_label(u"additem"_ustr, SvxResId(RID_STR_DATANAV_ADD_BINDING));
                m_xMenu->set_label(u"edit"_ustr, SvxResId(RID_STR_DATANAV_EDIT_BINDING));
                m_xMenu->set_label(u"delete"_ustr, SvxResId(RID_STR_DATANAV_REMOVE_BINDING));
            }
        }
        for (const auto& rRemove : m_aRemovedMenuEntries)
            m_xMenu->remove(rRemove);
        EnableMenuItems();
        OUString sCommand = m_xMenu->popup_at_rect(m_xItemList.get(), tools::Rectangle(aPos, Size(1,1)));
        if (!sCommand.isEmpty())
            DoMenuAction(sCommand);
        m_xMenu.reset();
        return true;
    }

    void XFormsPage::DeleteAndClearTree()
    {
        m_xItemList->all_foreach([this](weld::TreeIter& rEntry) {
            delete weld::fromId<ItemNode*>(m_xItemList->get_id(rEntry));
            return false;
        });
        m_xItemList->clear();
    }

    void XFormsPage::SelectFirstEntry()
    {
        if (m_xItemList->get_iter_first(*m_xScratchIter))
        {
            m_xItemList->select(*m_xScratchIter);
            ItemSelectHdl(*m_xItemList);
        }
    }

    XFormsPage::XFormsPage(weld::Container* pPage, DataNavigatorWindow* _pNaviWin, DataGroupType _eGroup)
        : BuilderPage(pPage, nullptr, u"svx/ui/xformspage.ui"_ustr, u"XFormsPage"_ustr)
        , m_pParent(pPage)
        , m_xToolBox(m_xBuilder->weld_toolbar(u"toolbar"_ustr))
        , m_xItemList(m_xBuilder->weld_tree_view(u"items"_ustr))
        , m_xScratchIter(m_xItemList->make_iterator())
        , m_aDropHelper(*m_xItemList)
        , m_pNaviWin(_pNaviWin)
        , m_bHasModel(false)
        , m_eGroup(_eGroup)
        , m_bLinkOnce(false)
    {
        m_xItemList->set_show_expanders(DGTInstance == m_eGroup || DGTSubmission == m_eGroup);

        if ( DGTInstance == m_eGroup )
            m_xToolBox->set_item_visible(u"additem"_ustr, false);
        else
        {
            m_xToolBox->set_item_visible(u"addelement"_ustr, false);
            m_xToolBox->set_item_visible(u"addattribute"_ustr, false);

            if ( DGTSubmission == m_eGroup )
            {
                m_xToolBox->set_item_label(u"additem"_ustr, SvxResId(RID_STR_DATANAV_ADD_SUBMISSION));
                m_xToolBox->set_item_label(u"edit"_ustr, SvxResId(RID_STR_DATANAV_EDIT_SUBMISSION));
                m_xToolBox->set_item_label(u"delete"_ustr, SvxResId(RID_STR_DATANAV_REMOVE_SUBMISSION));
            }
            else
            {
                m_xToolBox->set_item_label(u"additem"_ustr, SvxResId(RID_STR_DATANAV_ADD_BINDING));
                m_xToolBox->set_item_label(u"edit"_ustr, SvxResId(RID_STR_DATANAV_EDIT_BINDING));
                m_xToolBox->set_item_label(u"delete"_ustr, SvxResId(RID_STR_DATANAV_REMOVE_BINDING));
            }
        }

        m_xToolBox->connect_clicked(LINK(this, XFormsPage, TbxSelectHdl));

        m_xItemList->connect_changed(LINK(this, XFormsPage, ItemSelectHdl));
        m_xItemList->connect_key_press(LINK(this, XFormsPage, KeyInputHdl));
        m_xItemList->connect_popup_menu(LINK(this, XFormsPage, PopupMenuHdl));
        ItemSelectHdl(*m_xItemList);
    }

    XFormsPage::~XFormsPage()
    {
        DeleteAndClearTree();
        m_pNaviWin = nullptr;
        m_pParent->move(m_xContainer.get(), nullptr);
    }

    IMPL_LINK(XFormsPage, TbxSelectHdl, const OUString&, rIdent, void)
    {
        DoToolBoxAction(rIdent);
    }

    IMPL_LINK_NOARG(XFormsPage, ItemSelectHdl, weld::TreeView&, void)
    {
        EnableMenuItems();
        PrepDnD();
    }

    void XFormsPage::PrepDnD()
    {
        rtl::Reference<TransferDataContainer> xTransferable(new TransferDataContainer);
        m_xItemList->enable_drag_source(xTransferable, DND_ACTION_NONE);

        if (!m_xItemList->get_selected(m_xScratchIter.get()))
        {
            // no drag without an entry
            return;
        }

        if ( m_eGroup == DGTBinding )
        {
            // for the moment, bindings cannot be dragged.
            // #i59395# / 2005-12-15 / frank.schoenheit@sun.com
            return;
        }

        // GetServiceNameForNode() requires a datatype repository which
        // will be automatically build if requested???
        Reference< css::xforms::XModel > xModel( GetXFormsHelper(), UNO_QUERY );
        Reference< css::xforms::XDataTypeRepository > xDataTypes =
            xModel->getDataTypeRepository();
        if(!xDataTypes.is())
            return;

        ItemNode *pItemNode = weld::fromId<ItemNode*>(m_xItemList->get_id(*m_xScratchIter));
        if (!pItemNode)
        {
            // the only known (and allowed?) case where this happens are sub-entries of a submission
            // entry
            DBG_ASSERT( DGTSubmission == m_eGroup, "DataTreeListBox::StartDrag: how this?" );
            bool bSelected = m_xItemList->iter_parent(*m_xScratchIter);
            DBG_ASSERT(bSelected && !m_xItemList->get_iter_depth(*m_xScratchIter), "DataTreeListBox::StartDrag: what kind of entry *is* this?");
                // on the submission page, we have only top-level entries (the submission themself)
                // plus direct children of those (facets of a submission)
            pItemNode = bSelected ? weld::fromId<ItemNode*>(m_xItemList->get_id(*m_xScratchIter)) : nullptr;
            if (!pItemNode)
                return;
        }

        OUString szName = m_xItemList->get_text(*m_xScratchIter);
        Reference<css::xml::dom::XNode> xNode(pItemNode->m_xNode);
        Reference<XPropertySet> xPropSet(pItemNode->m_xPropSet);

        // tdf#154535 create the OXFormsDescriptor on-demand so we don't cause an unwanted
        // Binding to be created unless we are forced to.
        auto fnCreateFormsDescriptor = [this, szName=std::move(szName), xNode=std::move(xNode), xPropSet=std::move(xPropSet)](){
            OXFormsDescriptor desc;
            desc.szName = szName;
            if (xNode) {
                // a valid node interface tells us that we need to create a control from a binding
                desc.szServiceName = GetServiceNameForNode(xNode);
                desc.xPropSet = GetBindingForNode(xNode);
                DBG_ASSERT( desc.xPropSet.is(), "DataTreeListBox::StartDrag(): invalid node binding" );
            }
            else {
                desc.szServiceName = FM_COMPONENT_COMMANDBUTTON;
                desc.xPropSet = xPropSet;
            }
            return desc;
        };

        xTransferable = rtl::Reference<TransferDataContainer>(new OXFormsTransferable(fnCreateFormsDescriptor));
        m_xItemList->enable_drag_source(xTransferable, DND_ACTION_COPY);
    }

    void XFormsPage::AddChildren(const weld::TreeIter* _pParent,
        const Reference< css::xml::dom::XNode >& _xNode)
    {
        DBG_ASSERT( m_xUIHelper.is(), "XFormsPage::AddChildren(): invalid UIHelper" );

        try
        {
            Reference< css::xml::dom::XNodeList > xNodeList = _xNode->getChildNodes();
            if ( xNodeList.is() )
            {
                bool bShowDetails = m_pNaviWin->IsShowDetails();
                sal_Int32 i, nNodeCount = xNodeList->getLength();
                for ( i = 0; i < nNodeCount; ++i )
                {
                    Reference< css::xml::dom::XNode > xChild = xNodeList->item(i);
                    css::xml::dom::NodeType eChildType = xChild->getNodeType();
                    OUString aExpImg;
                    switch ( eChildType )
                    {
                        case css::xml::dom::NodeType_ATTRIBUTE_NODE:
                            aExpImg = RID_SVXBMP_ATTRIBUTE;
                            break;
                        case css::xml::dom::NodeType_ELEMENT_NODE:
                            aExpImg = RID_SVXBMP_ELEMENT;
                            break;
                        case css::xml::dom::NodeType_TEXT_NODE:
                            aExpImg = RID_SVXBMP_TEXT;
                            break;
                        default:
                            aExpImg = RID_SVXBMP_OTHER;
                    }

                    OUString sName = m_xUIHelper->getNodeDisplayName( xChild, bShowDetails );
                    if ( !sName.isEmpty() )
                    {
                        ItemNode* pNode = new ItemNode( xChild );
                        OUString sId(weld::toId(pNode));
                        std::unique_ptr<weld::TreeIter> xEntry = m_xItemList->make_iterator();
                        m_xItemList->insert(_pParent, -1, &sName, &sId, nullptr, nullptr, false, xEntry.get());
                        m_xItemList->set_image(*xEntry, aExpImg);

                        if ( xChild->hasAttributes() )
                        {
                            Reference< css::xml::dom::XNamedNodeMap > xMap = xChild->getAttributes();
                            if ( xMap.is() )
                            {
                                aExpImg = RID_SVXBMP_ATTRIBUTE;
                                sal_Int32 j, nMapLen = xMap->getLength();
                                for ( j = 0; j < nMapLen; ++j )
                                {
                                    Reference< css::xml::dom::XNode > xAttr = xMap->item(j);
                                    pNode = new ItemNode( xAttr );
                                    OUString sSubId(weld::toId(pNode));
                                    OUString sAttrName = m_xUIHelper->getNodeDisplayName( xAttr, bShowDetails );
                                    m_xItemList->insert(xEntry.get(), -1, &sAttrName, &sSubId, nullptr, nullptr, false, m_xScratchIter.get());
                                    m_xItemList->set_image(*m_xScratchIter, aExpImg);
                                }
                            }
                        }
                        if ( xChild->hasChildNodes() )
                            AddChildren(xEntry.get(), xChild);
                    }
                }
            }
        }
        catch( Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("svx");
        }
    }

    bool XFormsPage::DoToolBoxAction(std::u16string_view rToolBoxID)
    {
        bool bHandled = false;
        bool bIsDocModified = false;
        m_pNaviWin->DisableNotify( true );

        if (rToolBoxID == u"additem" || rToolBoxID == u"addelement" || rToolBoxID == u"addattribute")
        {
            bHandled = true;
            Reference< css::xforms::XModel > xModel( m_xUIHelper, UNO_QUERY );
            DBG_ASSERT( xModel.is(), "XFormsPage::DoToolBoxAction(): Action without model" );
            if ( DGTSubmission == m_eGroup )
            {
                AddSubmissionDialog aDlg(m_pNaviWin->GetFrameWeld(), nullptr, m_xUIHelper);
                if ( aDlg.run() == RET_OK && aDlg.GetNewSubmission().is() )
                {
                    try
                    {
                        Reference< css::xforms::XSubmission > xNewSubmission = aDlg.GetNewSubmission();
                        Reference< XSet > xSubmissions = xModel->getSubmissions();
                        xSubmissions->insert( Any( xNewSubmission ) );
                        AddEntry(xNewSubmission, m_xScratchIter.get());
                        m_xItemList->select(*m_xScratchIter);
                        bIsDocModified = true;
                    }
                    catch ( Exception const & )
                    {
                        TOOLS_WARN_EXCEPTION( "svx.form", "XFormsPage::DoToolBoxAction()" );
                    }
                }
            }
            else
            {
                DataItemType eType = DITElement;

                std::unique_ptr<weld::TreeIter> xEntry(m_xItemList->make_iterator());
                bool bEntry = m_xItemList->get_selected(xEntry.get());

                std::unique_ptr<ItemNode> pNode;
                Reference< css::xml::dom::XNode > xParentNode;
                Reference< XPropertySet > xNewBinding;
                TranslateId pResId;
                bool bIsElement = true;
                if ( DGTInstance == m_eGroup )
                {
                    if ( !m_sInstanceURL.isEmpty() )
                    {
                        LinkedInstanceWarningBox aMsgBox(m_pNaviWin->GetFrameWeld());
                        if (aMsgBox.run() != RET_OK)
                            return bHandled;
                    }

                    DBG_ASSERT( bEntry, "XFormsPage::DoToolBoxAction(): no entry" );
                    ItemNode* pParentNode = weld::fromId<ItemNode*>(m_xItemList->get_id(*xEntry));
                    assert(pParentNode && "XFormsPage::DoToolBoxAction(): no parent node");
                    xParentNode = pParentNode->m_xNode;
                    Reference< css::xml::dom::XNode > xNewNode;
                    if (rToolBoxID == u"addelement")
                    {
                        try
                        {
                            pResId = RID_STR_DATANAV_ADD_ELEMENT;
                            xNewNode = m_xUIHelper->createElement( xParentNode, NEW_ELEMENT );
                        }
                        catch ( Exception const & )
                        {
                            TOOLS_WARN_EXCEPTION( "svx.form", "XFormsPage::DoToolBoxAction(): exception while create element" );
                        }
                    }
                    else
                    {
                        pResId = RID_STR_DATANAV_ADD_ATTRIBUTE;
                        bIsElement = false;
                        eType = DITAttribute;
                        try
                        {
                            xNewNode = m_xUIHelper->createAttribute( xParentNode, NEW_ATTRIBUTE );
                        }
                        catch ( Exception const & )
                        {
                            TOOLS_WARN_EXCEPTION( "svx.form", "XFormsPage::DoToolBoxAction(): exception while create attribute" );
                        }
                    }

                    try
                    {
                        xNewNode = xParentNode->appendChild( xNewNode );
                    }
                    catch ( Exception const & )
                    {
                        TOOLS_WARN_EXCEPTION( "svx.form", "XFormsPage::DoToolBoxAction(): exception while append child" );
                    }

                    try
                    {
                        Reference< css::xml::dom::XNode > xPNode;
                        if ( xNewNode.is() )
                             xPNode = xNewNode->getParentNode();
                        // attributes don't have parents in the DOM model
                        DBG_ASSERT( rToolBoxID  == u"addattribute"
                                    || xPNode.is(), "XFormsPage::DoToolboxAction(): node not added" );
                    }
                    catch ( Exception const & )
                    {
                        TOOLS_WARN_EXCEPTION( "svx.form", "XFormsPage::DoToolboxAction()" );
                    }

                    try
                    {
                        m_xUIHelper->getBindingForNode( xNewNode, true );
                    }
                    catch ( Exception const & )
                    {
                        TOOLS_WARN_EXCEPTION( "svx.form", "XFormsPage::DoToolBoxAction(): exception while get binding for node" );
                    }
                    pNode.reset(new ItemNode( xNewNode ));
                }
                else
                {
                    try
                    {
                        pResId = RID_STR_DATANAV_ADD_BINDING;
                        xNewBinding = xModel->createBinding();
                        Reference< XSet > xBindings = xModel->getBindings();
                        xBindings->insert( Any( xNewBinding ) );
                        pNode.reset(new ItemNode( xNewBinding ));
                        eType = DITBinding;
                    }
                    catch ( Exception const & )
                    {
                        TOOLS_WARN_EXCEPTION( "svx.form", "XFormsPage::DoToolBoxAction(): exception while adding binding" );
                    }
                }

                AddDataItemDialog aDlg(m_pNaviWin->GetFrameWeld(), pNode.get(), m_xUIHelper);
                aDlg.set_title(SvxResId(pResId));
                aDlg.InitText( eType );
                short nReturn = aDlg.run();
                if (  DGTInstance == m_eGroup )
                {
                    if ( RET_OK == nReturn )
                    {
                        AddEntry( std::move(pNode), bIsElement, m_xScratchIter.get());
                        m_xItemList->scroll_to_row(*m_xScratchIter);
                        m_xItemList->select(*m_xScratchIter);
                        bIsDocModified = true;
                    }
                    else
                    {
                        try
                        {
                            Reference< css::xml::dom::XNode > xPNode;
                            Reference< css::xml::dom::XNode > xNode =
                                xParentNode->removeChild( pNode->m_xNode );
                            if ( xNode.is() )
                                xPNode = xNode->getParentNode();
                            DBG_ASSERT( !xPNode.is(), "XFormsPage::RemoveEntry(): node not removed" );
                        }
                        catch ( Exception const & )
                        {
                            TOOLS_WARN_EXCEPTION( "svx.form", "XFormsPage::DoToolboxAction()" );
                        }
                    }
                }
                else
                {
                    if ( RET_OK == nReturn )
                    {
                        AddEntry(xNewBinding, m_xScratchIter.get());
                        m_xItemList->select(*m_xScratchIter);
                        bIsDocModified = true;
                    }
                    else
                    {
                        try
                        {
                            Reference< XSet > xBindings = xModel->getBindings();
                            xBindings->remove( Any( xNewBinding ) );
                        }
                        catch ( Exception const & )
                        {
                            TOOLS_WARN_EXCEPTION( "svx.form", "XFormsPage::DoToolboxAction()" );
                        }
                    }
                }
            }
        }
        else if (rToolBoxID == u"edit")
        {
            bHandled = true;

            std::unique_ptr<weld::TreeIter> xEntry(m_xItemList->make_iterator());
            bool bEntry = m_xItemList->get_selected(xEntry.get());
            if ( bEntry )
            {
                if ( DGTSubmission == m_eGroup && m_xItemList->get_iter_depth(*xEntry) )
                {
                    m_xItemList->iter_parent(*xEntry);
                }
                ItemNode* pNode = weld::fromId<ItemNode*>(m_xItemList->get_id(*xEntry));
                if ( DGTInstance == m_eGroup || DGTBinding == m_eGroup )
                {
                    if ( DGTInstance == m_eGroup && !m_sInstanceURL.isEmpty() )
                    {
                        LinkedInstanceWarningBox aMsgBox(m_pNaviWin->GetFrameWeld());
                        if (aMsgBox.run() != RET_OK)
                            return bHandled;
                    }

                    AddDataItemDialog aDlg(m_pNaviWin->GetFrameWeld(), pNode, m_xUIHelper);
                    DataItemType eType = DITElement;
                    TranslateId pResId = RID_STR_DATANAV_EDIT_ELEMENT;
                    if ( pNode && pNode->m_xNode.is() )
                    {
                        try
                        {
                            css::xml::dom::NodeType eChildType = pNode->m_xNode->getNodeType();
                            if ( eChildType == css::xml::dom::NodeType_ATTRIBUTE_NODE )
                            {
                                pResId = RID_STR_DATANAV_EDIT_ATTRIBUTE;
                                eType = DITAttribute;
                            }
                        }
                        catch ( Exception const & )
                        {
                            TOOLS_WARN_EXCEPTION( "svx.form", "XFormsPage::DoToolboxAction()" );
                        }
                    }
                    else if ( DGTBinding == m_eGroup )
                    {
                        pResId = RID_STR_DATANAV_EDIT_BINDING;
                        eType = DITBinding;
                    }
                    aDlg.set_title(SvxResId(pResId));
                    aDlg.InitText( eType );
                    if (aDlg.run() == RET_OK)
                    {
                        // Set the new name
                        OUString sNewName;
                        if (pNode)
                        {
                            if ( DGTInstance == m_eGroup )
                            {
                                try
                                {
                                    sNewName = m_xUIHelper->getNodeDisplayName(
                                        pNode->m_xNode, m_pNaviWin->IsShowDetails() );
                                }
                                catch ( Exception const & )
                                {
                                    TOOLS_WARN_EXCEPTION( "svx.form", "XFormsPage::DoToolboxAction()" );
                                }
                            }
                            else
                            {
                                try
                                {
                                    OUString sTemp;
                                    pNode->m_xPropSet->getPropertyValue( PN_BINDING_ID ) >>= sTemp;
                                    sNewName += sTemp + ": ";
                                    pNode->m_xPropSet->getPropertyValue( PN_BINDING_EXPR ) >>= sTemp;
                                    sNewName += sTemp;
                                }
                                catch ( Exception const & )
                                {
                                    TOOLS_WARN_EXCEPTION( "svx.form", "XFormsPage::DoToolboxAction()" );
                                }
                            }
                        }

                        m_xItemList->set_text(*xEntry, sNewName);
                        bIsDocModified = true;
                    }
                }
                else
                {
                    AddSubmissionDialog aDlg(m_pNaviWin->GetFrameWeld(), pNode, m_xUIHelper);
                    aDlg.set_title(SvxResId(RID_STR_DATANAV_EDIT_SUBMISSION));
                    if (aDlg.run() == RET_OK)
                    {
                        EditEntry( pNode->m_xPropSet );
                        bIsDocModified = true;
                    }
                }
            }
        }
        else if (rToolBoxID == u"delete")
        {
            bHandled = true;
            if ( DGTInstance == m_eGroup && !m_sInstanceURL.isEmpty() )
            {
                LinkedInstanceWarningBox aMsgBox(m_pNaviWin->GetFrameWeld());
                if (aMsgBox.run() != RET_OK)
                    return bHandled;
            }
            bIsDocModified = RemoveEntry();
        }
        else
        {
            OSL_FAIL( "XFormsPage::DoToolboxAction: unknown ID!" );
        }

        m_pNaviWin->DisableNotify( false );
        EnableMenuItems();
        if ( bIsDocModified )
            svxform::DataNavigatorWindow::SetDocModified();
        return bHandled;
    }

    void XFormsPage::AddEntry(std::unique_ptr<ItemNode> _pNewNode, bool _bIsElement, weld::TreeIter* pRet)
    {
        if (!pRet)
            pRet = m_xScratchIter.get();

        std::unique_ptr<weld::TreeIter> xParent(m_xItemList->make_iterator());
        if (!m_xItemList->get_selected(xParent.get()))
            xParent.reset();
        OUString aImage(_bIsElement ? RID_SVXBMP_ELEMENT : RID_SVXBMP_ATTRIBUTE);
        OUString sName;
        try
        {
            sName = m_xUIHelper->getNodeDisplayName(
                _pNewNode->m_xNode, m_pNaviWin->IsShowDetails() );
        }
        catch ( Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("svx");
        }
        OUString sId(weld::toId(_pNewNode.release()));
        m_xItemList->insert(xParent.get(), -1, &sName, &sId, nullptr, nullptr, false, pRet);
        m_xItemList->set_image(*pRet, aImage);
        if (xParent && !m_xItemList->get_row_expanded(*xParent) && m_xItemList->iter_has_child(*xParent))
            m_xItemList->expand_row(*xParent);
    }

    void XFormsPage::AddEntry(const Reference< XPropertySet >& _rEntry, weld::TreeIter* pRet)
    {
        if (!pRet)
            pRet = m_xScratchIter.get();

        OUString aImage(RID_SVXBMP_ELEMENT);

        ItemNode* pNode = new ItemNode( _rEntry );
        OUString sTemp;

        if ( DGTSubmission == m_eGroup )
        {
            try
            {
                // ID
                _rEntry->getPropertyValue( PN_SUBMISSION_ID ) >>= sTemp;
                OUString sId(weld::toId(pNode));
                m_xItemList->insert(nullptr, -1, &sTemp, &sId, nullptr, nullptr, false, pRet);
                m_xItemList->set_image(*pRet, aImage);
                std::unique_ptr<weld::TreeIter> xRes(m_xItemList->make_iterator());
                // Action
                _rEntry->getPropertyValue( PN_SUBMISSION_ACTION ) >>= sTemp;
                OUString sEntry = SvxResId( RID_STR_DATANAV_SUBM_ACTION ) + sTemp;
                m_xItemList->insert(pRet, -1, &sEntry, nullptr, nullptr, nullptr, false, xRes.get());
                m_xItemList->set_image(*xRes, aImage);
                // Method
                _rEntry->getPropertyValue( PN_SUBMISSION_METHOD ) >>= sTemp;
                sEntry = SvxResId( RID_STR_DATANAV_SUBM_METHOD ) +
                    m_aMethodString.toUI( sTemp );
                m_xItemList->insert(pRet, -1, &sEntry, nullptr, nullptr, nullptr, false, xRes.get());
                m_xItemList->set_image(*xRes, aImage);
                // Ref
                _rEntry->getPropertyValue( PN_SUBMISSION_REF ) >>= sTemp;
                sEntry = SvxResId( RID_STR_DATANAV_SUBM_REF ) + sTemp;
                m_xItemList->insert(pRet, -1, &sEntry, nullptr, nullptr, nullptr, false, xRes.get());
                m_xItemList->set_image(*xRes, aImage);
                // Bind
                _rEntry->getPropertyValue( PN_SUBMISSION_BIND ) >>= sTemp;
                sEntry = SvxResId( RID_STR_DATANAV_SUBM_BIND ) + sTemp;
                m_xItemList->insert(pRet, -1, &sEntry, nullptr, nullptr, nullptr, false, xRes.get());
                m_xItemList->set_image(*xRes, aImage);
                // Replace
                _rEntry->getPropertyValue( PN_SUBMISSION_REPLACE ) >>= sTemp;
                sEntry = SvxResId( RID_STR_DATANAV_SUBM_REPLACE ) +
                    m_aReplaceString.toUI( sTemp );
                m_xItemList->insert(pRet, -1, &sEntry, nullptr, nullptr, nullptr, false, xRes.get());
                m_xItemList->set_image(*xRes, aImage);
            }
            catch ( Exception const & )
            {
                TOOLS_WARN_EXCEPTION( "svx.form", "XFormsPage::AddEntry(Ref)" );
            }
        }
        else // then Binding Page
        {
            try
            {
                OUString sName;
                _rEntry->getPropertyValue( PN_BINDING_ID ) >>= sTemp;
                sName += sTemp + ": ";
                _rEntry->getPropertyValue( PN_BINDING_EXPR ) >>= sTemp;
                sName += sTemp;

                OUString sId(weld::toId(pNode));
                m_xItemList->insert(nullptr, -1, &sName, &sId, nullptr, nullptr, false, pRet);
                m_xItemList->set_image(*pRet, aImage);
            }
            catch ( Exception const & )
            {
                TOOLS_WARN_EXCEPTION( "svx.form", "XFormsPage::AddEntry(Ref)" );
            }
        }
    }

    void XFormsPage::EditEntry( const Reference< XPropertySet >& _rEntry )
    {
        if ( DGTSubmission != m_eGroup )
            return;

        try
        {
            std::unique_ptr<weld::TreeIter> xEntry(m_xItemList->make_iterator());
            if (!m_xItemList->get_selected(xEntry.get()))
            {
                SAL_WARN( "svx.form", "corrupt tree" );
                return;
            }

            // #i36262# may be called for submission entry *or* for
            // submission children. If we don't have any children, we
            // assume the latter case and use the parent
            if (!m_xItemList->iter_has_child(*xEntry))
                m_xItemList->iter_parent(*xEntry);

            OUString sTemp;
            _rEntry->getPropertyValue( PN_SUBMISSION_ID ) >>= sTemp;
            m_xItemList->set_text(*xEntry, sTemp);

            _rEntry->getPropertyValue( PN_SUBMISSION_BIND ) >>= sTemp;
            OUString sEntry = SvxResId( RID_STR_DATANAV_SUBM_BIND ) + sTemp;
            if (!m_xItemList->iter_children(*xEntry))
            {
                SAL_WARN( "svx.form", "corrupt tree" );
                return;
            }
            m_xItemList->set_text(*xEntry, sEntry);
            _rEntry->getPropertyValue( PN_SUBMISSION_REF ) >>= sTemp;
            sEntry = SvxResId( RID_STR_DATANAV_SUBM_REF ) + sTemp;
            if (!m_xItemList->iter_next_sibling(*xEntry))
            {
                SAL_WARN( "svx.form", "corrupt tree" );
                return;
            }
            m_xItemList->set_text(*xEntry, sEntry);
            _rEntry->getPropertyValue( PN_SUBMISSION_ACTION ) >>= sTemp;
            sEntry = SvxResId( RID_STR_DATANAV_SUBM_ACTION ) + sTemp;
            if (!m_xItemList->iter_next_sibling(*xEntry))
            {
                SAL_WARN( "svx.form", "corrupt tree" );
                return;
            }
            _rEntry->getPropertyValue( PN_SUBMISSION_METHOD ) >>= sTemp;
            sEntry = SvxResId( RID_STR_DATANAV_SUBM_METHOD ) +
                m_aMethodString.toUI( sTemp );
            if (!m_xItemList->iter_next_sibling(*xEntry))
            {
                SAL_WARN( "svx.form", "corrupt tree" );
                return;
            }
            m_xItemList->set_text(*xEntry, sEntry);
            _rEntry->getPropertyValue( PN_SUBMISSION_REPLACE ) >>= sTemp;
            sEntry = SvxResId( RID_STR_DATANAV_SUBM_REPLACE ) +
                m_aReplaceString.toUI( sTemp );
            if (!m_xItemList->iter_next_sibling(*xEntry))
            {
                SAL_WARN( "svx.form", "corrupt tree" );
                return;
            }
            m_xItemList->set_text(*xEntry, sEntry);
        }
        catch ( Exception const & )
        {
            TOOLS_WARN_EXCEPTION( "svx.form", "XFormsPage::EditEntry()" );
        }
    }

    bool XFormsPage::RemoveEntry()
    {
        bool bRet = false;

        std::unique_ptr<weld::TreeIter> xEntry(m_xItemList->make_iterator());
        bool bEntry = m_xItemList->get_selected(xEntry.get());
        if ( bEntry &&
             ( DGTInstance != m_eGroup || m_xItemList->get_iter_depth(*xEntry) ) )
        {
            Reference< css::xforms::XModel > xModel( m_xUIHelper, UNO_QUERY );
            DBG_ASSERT( xModel.is(), "XFormsPage::RemoveEntry(): no model" );
            ItemNode* pNode = weld::fromId<ItemNode*>(m_xItemList->get_id(*xEntry));
            assert(pNode && "XFormsPage::RemoveEntry(): no node");

            if ( DGTInstance == m_eGroup )
            {
                try
                {
                    DBG_ASSERT( pNode->m_xNode.is(), "XFormsPage::RemoveEntry(): no XNode" );
                    css::xml::dom::NodeType eChildType = pNode->m_xNode->getNodeType();
                    bool bIsElement = ( eChildType == css::xml::dom::NodeType_ELEMENT_NODE );
                    TranslateId pResId = bIsElement ? RID_STR_QRY_REMOVE_ELEMENT : RID_STR_QRY_REMOVE_ATTRIBUTE;
                    OUString sVar = bIsElement ? OUString(ELEMENTNAME) : OUString(ATTRIBUTENAME);
                    std::unique_ptr<weld::MessageDialog> xQBox(Application::CreateMessageDialog(m_pNaviWin->GetFrameWeld(),
                                                                             VclMessageType::Question, VclButtonsType::YesNo,
                                                                             SvxResId(pResId)));
                    OUString sMessText = xQBox->get_primary_text();
                    sMessText = sMessText.replaceFirst(
                        sVar, m_xUIHelper->getNodeDisplayName( pNode->m_xNode, false ) );
                    xQBox->set_primary_text(sMessText);
                    if (xQBox->run() == RET_YES)
                    {
                        std::unique_ptr<weld::TreeIter> xParent(m_xItemList->make_iterator(xEntry.get()));
                        bool bParent = m_xItemList->iter_parent(*xParent); (void)bParent;
                        assert(bParent && "XFormsPage::RemoveEntry(): no parent entry");
                        ItemNode* pParentNode = weld::fromId<ItemNode*>(m_xItemList->get_id(*xParent));
                        assert(pParentNode && pParentNode->m_xNode.is() && "XFormsPage::RemoveEntry(): no parent XNode");

                        Reference< css::xml::dom::XNode > xPNode;
                        Reference< css::xml::dom::XNode > xNode =
                            pParentNode->m_xNode->removeChild( pNode->m_xNode );
                        if ( xNode.is() )
                            xPNode = xNode->getParentNode();
                        DBG_ASSERT( !xPNode.is(), "XFormsPage::RemoveEntry(): node not removed" );
                        bRet = true;
                    }
                }
                catch ( Exception const & )
                {
                    TOOLS_WARN_EXCEPTION( "svx.form", "XFormsPage::RemoveEntry()" );
                }
            }
            else
            {
                DBG_ASSERT( pNode->m_xPropSet.is(), "XFormsPage::RemoveEntry(): no propset" );
                bool bSubmission = ( DGTSubmission == m_eGroup );
                TranslateId pResId = bSubmission ? RID_STR_QRY_REMOVE_SUBMISSION : RID_STR_QRY_REMOVE_BINDING;
                OUString sProperty = bSubmission ? PN_SUBMISSION_ID : PN_BINDING_ID;
                OUString sSearch = bSubmission ? OUString(SUBMISSIONNAME) : OUString(BINDINGNAME);
                OUString sName;
                try
                {
                    pNode->m_xPropSet->getPropertyValue( sProperty ) >>= sName;
                }
                catch ( Exception const & )
                {
                    TOOLS_WARN_EXCEPTION( "svx.form", "XFormsPage::RemoveEntry()" );
                }
                std::unique_ptr<weld::MessageDialog> xQBox(Application::CreateMessageDialog(m_pNaviWin->GetFrameWeld(),
                                                                         VclMessageType::Question, VclButtonsType::YesNo,
                                                                         SvxResId(pResId)));
                OUString sMessText = xQBox->get_primary_text();
                sMessText = sMessText.replaceFirst( sSearch, sName);
                xQBox->set_primary_text(sMessText);
                if (xQBox->run() == RET_YES)
                {
                    try
                    {
                        if ( bSubmission )
                            xModel->getSubmissions()->remove( Any( pNode->m_xPropSet ) );
                        else // then Binding Page
                            xModel->getBindings()->remove( Any( pNode->m_xPropSet ) );
                        bRet = true;
                    }
                    catch ( Exception const & )
                    {
                        TOOLS_WARN_EXCEPTION( "svx.form", "XFormsPage::RemoveEntry()" );
                    }
                }
            }

            if (bRet)
            {
                m_xItemList->remove(*xEntry);
                delete pNode;
            }
        }

        return bRet;
    }

    IMPL_LINK(XFormsPage, KeyInputHdl, const KeyEvent&, rKEvt, bool)
    {
        bool bHandled = false;

        sal_uInt16 nCode = rKEvt.GetKeyCode().GetCode();
        if (nCode == KEY_DELETE)
            bHandled = DoMenuAction(u"delete");

        return bHandled;
    }

    OUString XFormsPage::SetModel( const Reference< css::xforms::XModel >& _xModel, int _nPagePos )
    {
        DBG_ASSERT( _xModel.is(), "XFormsPage::SetModel(): invalid model" );

        m_xUIHelper.set( _xModel, UNO_QUERY );
        OUString sRet;
        m_bHasModel = true;

        switch ( m_eGroup )
        {
            case DGTInstance :
            {
                DBG_ASSERT( _nPagePos != -1, "XFormsPage::SetModel(): invalid page position" );
                try
                {
                    Reference< XContainer > xContainer( _xModel->getInstances(), UNO_QUERY );
                    if ( xContainer.is() )
                        m_pNaviWin->AddContainerBroadcaster( xContainer );

                    Reference< XEnumerationAccess > xNumAccess = _xModel->getInstances();
                    if ( xNumAccess.is() )
                    {
                        Reference < XEnumeration > xNum = xNumAccess->createEnumeration();
                        if ( xNum.is() && xNum->hasMoreElements() )
                        {
                            int nIter = 0;
                            while ( xNum->hasMoreElements() )
                            {
                                if ( nIter == _nPagePos )
                                {
                                    Sequence< PropertyValue > xPropSeq;
                                    Any aAny = xNum->nextElement();
                                    if ( aAny >>= xPropSeq )
                                        sRet = LoadInstance(xPropSeq);
                                    else
                                    {
                                        SAL_WARN( "svx.form", "XFormsPage::SetModel(): invalid instance" );
                                    }
                                    break;
                                }
                                else
                                {
                                    xNum->nextElement();
                                    ++nIter;
                                }
                            }
                        }
                    }
                }
                catch( Exception const & )
                {
                    TOOLS_WARN_EXCEPTION( "svx.form", "XFormsPage::SetModel()" );
                }
                break;
            }

            case DGTSubmission :
            {
                DBG_ASSERT( _nPagePos == -1, "XFormsPage::SetModel(): invalid page position" );
                try
                {
                    Reference< XContainer > xContainer( _xModel->getSubmissions(), UNO_QUERY );
                    if ( xContainer.is() )
                        m_pNaviWin->AddContainerBroadcaster( xContainer );

                    Reference< XEnumerationAccess > xNumAccess = _xModel->getSubmissions();
                    if ( xNumAccess.is() )
                    {
                        Reference < XEnumeration > xNum = xNumAccess->createEnumeration();
                        if ( xNum.is() && xNum->hasMoreElements() )
                        {
                            while ( xNum->hasMoreElements() )
                            {
                                Reference< XPropertySet > xPropSet;
                                Any aAny = xNum->nextElement();
                                if ( aAny >>= xPropSet )
                                    AddEntry( xPropSet );
                            }
                        }
                    }
                }
                catch( Exception const & )
                {
                    TOOLS_WARN_EXCEPTION( "svx.form", "XFormsPage::SetModel()" );
                }
                break;
            }

            case DGTBinding :
            {
                DBG_ASSERT( _nPagePos == -1, "XFormsPage::SetModel(): invalid page position" );
                try
                {
                    Reference< XContainer > xContainer( _xModel->getBindings(), UNO_QUERY );
                    if ( xContainer.is() )
                        m_pNaviWin->AddContainerBroadcaster( xContainer );

                    Reference< XEnumerationAccess > xNumAccess = _xModel->getBindings();
                    if ( xNumAccess.is() )
                    {
                        Reference < XEnumeration > xNum = xNumAccess->createEnumeration();
                        if ( xNum.is() && xNum->hasMoreElements() )
                        {
                            OUString aImage(RID_SVXBMP_ELEMENT);
                            std::unique_ptr<weld::TreeIter> xRes(m_xItemList->make_iterator());
                            while ( xNum->hasMoreElements() )
                            {
                                Reference< XPropertySet > xPropSet;
                                Any aAny = xNum->nextElement();
                                if ( aAny >>= xPropSet )
                                {
                                    OUString sEntry;
                                    OUString sTemp;
                                    xPropSet->getPropertyValue( PN_BINDING_ID ) >>= sTemp;
                                    sEntry += sTemp + ": ";
                                    xPropSet->getPropertyValue( PN_BINDING_EXPR ) >>= sTemp;
                                    sEntry += sTemp;

                                    ItemNode* pNode = new ItemNode( xPropSet );

                                    OUString sId(weld::toId(pNode));
                                    m_xItemList->insert(nullptr, -1, &sEntry, &sId, nullptr, nullptr, false, xRes.get());
                                    m_xItemList->set_image(*xRes, aImage);
                                }
                            }
                        }
                    }
                }
                catch( Exception const & )
                {
                    TOOLS_WARN_EXCEPTION( "svx.form", "XFormsPage::SetModel()" );
                }
                break;
            }
            default:
                OSL_FAIL( "XFormsPage::SetModel: unknown group!" );
                break;
        }

        EnableMenuItems();

        return sRet;
    }

    void XFormsPage::ClearModel()
    {
        m_bHasModel = false;
        DeleteAndClearTree();
    }

    OUString XFormsPage::LoadInstance(const Sequence< PropertyValue >& _xPropSeq)
    {
        OUString sRet;
        OUString sTemp;
        OUString sInstModel = PN_INSTANCE_MODEL;
        OUString sInstName = PN_INSTANCE_ID;
        OUString sInstURL = PN_INSTANCE_URL;
        for ( const PropertyValue& rProp : _xPropSeq )
        {
            if ( sInstModel == rProp.Name )
            {
                Reference< css::xml::dom::XNode > xRoot;
                if ( rProp.Value >>= xRoot )
                {
                    try
                    {
                        Reference< XEventTarget > xTarget( xRoot, UNO_QUERY );
                        if ( xTarget.is() )
                            m_pNaviWin->AddEventBroadcaster( xTarget );

                        OUString sNodeName =
                            m_xUIHelper->getNodeDisplayName( xRoot, m_pNaviWin->IsShowDetails() );
                        if ( sNodeName.isEmpty() )
                            sNodeName = xRoot->getNodeName();
                        if ( xRoot->hasChildNodes() )
                            AddChildren(nullptr, xRoot);
                    }
                    catch ( Exception const & )
                    {
                        TOOLS_WARN_EXCEPTION( "svx.form", "XFormsPage::LoadInstance()" );
                    }
                }
            }
            else if ( sInstName == rProp.Name && ( rProp.Value >>= sTemp ) )
                m_sInstanceName = sRet = sTemp;
            else if ( sInstURL == rProp.Name && ( rProp.Value >>= sTemp ) )
                m_sInstanceURL = sTemp;
        }

        return sRet;
    }

    bool XFormsPage::DoMenuAction(std::u16string_view rMenuID)
    {
        return DoToolBoxAction(rMenuID);
    }

    void XFormsPage::SetMenuEntrySensitive(const OUString& rIdent, bool bSensitive)
    {
        if (m_aRemovedMenuEntries.find(rIdent) != m_aRemovedMenuEntries.end())
            return;
        m_xMenu->set_sensitive(rIdent, bSensitive);
    }

    void XFormsPage::EnableMenuItems()
    {
        bool bEnableAdd = false;
        bool bEnableEdit = false;
        bool bEnableRemove = false;

        std::unique_ptr<weld::TreeIter> xEntry(m_xItemList->make_iterator());
        bool bEntry = m_xItemList->get_selected(xEntry.get());
        if (bEntry)
        {
            bEnableAdd = true;
            bool bSubmitChild = false;
            if (DGTSubmission == m_eGroup && m_xItemList->get_iter_depth(*xEntry))
            {
                m_xItemList->iter_parent(*xEntry);
                bSubmitChild = true;
            }
            ItemNode* pNode = weld::fromId<ItemNode*>(m_xItemList->get_id(*xEntry));
            if ( pNode && ( pNode->m_xNode.is() || pNode->m_xPropSet.is() ) )
            {
                bEnableEdit = true;
                bEnableRemove = !bSubmitChild;
                if ( DGTInstance == m_eGroup && !m_xItemList->get_iter_depth(*xEntry) )
                    bEnableRemove = false;
                if ( pNode->m_xNode.is() )
                {
                    try
                    {
                        css::xml::dom::NodeType eChildType = pNode->m_xNode->getNodeType();
                        if ( eChildType != css::xml::dom::NodeType_ELEMENT_NODE
                            && eChildType != css::xml::dom::NodeType_DOCUMENT_NODE )
                        {
                            bEnableAdd = false;
                        }
                    }
                    catch ( Exception const & )
                    {
                       TOOLS_WARN_EXCEPTION( "svx.form", "XFormsPage::EnableMenuItems()" );
                    }
                }
            }
        }
        else if ( m_eGroup != DGTInstance )
            bEnableAdd = true;

        m_xToolBox->set_item_sensitive(u"additem"_ustr, bEnableAdd);
        m_xToolBox->set_item_sensitive(u"addelement"_ustr, bEnableAdd);
        m_xToolBox->set_item_sensitive(u"addattribute"_ustr, bEnableAdd);
        m_xToolBox->set_item_sensitive(u"edit"_ustr, bEnableEdit);
        m_xToolBox->set_item_sensitive(u"delete"_ustr, bEnableRemove);

        if (m_xMenu)
        {
            SetMenuEntrySensitive(u"additem"_ustr, bEnableAdd);
            SetMenuEntrySensitive(u"addelement"_ustr, bEnableAdd);
            SetMenuEntrySensitive(u"addattribute"_ustr, bEnableAdd);
            SetMenuEntrySensitive(u"edit"_ustr, bEnableEdit);
            SetMenuEntrySensitive(u"delete"_ustr, bEnableRemove);
        }
        if ( DGTInstance != m_eGroup )
            return;

        TranslateId pResId1 = RID_STR_DATANAV_EDIT_ELEMENT;
        TranslateId pResId2 = RID_STR_DATANAV_REMOVE_ELEMENT;
        if (bEntry)
        {
            ItemNode* pNode = weld::fromId<ItemNode*>(m_xItemList->get_id(*xEntry));
            if ( pNode && pNode->m_xNode.is() )
            {
                try
                {
                    css::xml::dom::NodeType eChildType = pNode->m_xNode->getNodeType();
                    if ( eChildType == css::xml::dom::NodeType_ATTRIBUTE_NODE )
                    {
                        pResId1 = RID_STR_DATANAV_EDIT_ATTRIBUTE;
                        pResId2 = RID_STR_DATANAV_REMOVE_ATTRIBUTE;
                    }
                }
                catch ( Exception const & )
                {
                   TOOLS_WARN_EXCEPTION( "svx.form", "XFormsPage::EnableMenuItems()" );
                }
            }
        }
        m_xToolBox->set_item_label(u"edit"_ustr, SvxResId(pResId1));
        m_xToolBox->set_item_label(u"delete"_ustr, SvxResId(pResId2));
        if (m_xMenu)
        {
            m_xMenu->set_label(u"edit"_ustr, SvxResId( pResId1 ) );
            m_xMenu->set_label(u"delete"_ustr, SvxResId( pResId2 ) );
        }
    }

    DataNavigatorWindow::DataNavigatorWindow(vcl::Window* pParent, weld::Builder& rBuilder, SfxBindings const * pBindings)
        : m_xParent(pParent)
        , m_xModelsBox(rBuilder.weld_combo_box(u"modelslist"_ustr))
        , m_xModelBtn(rBuilder.weld_menu_button(u"modelsbutton"_ustr))
        , m_xTabCtrl(rBuilder.weld_notebook(u"tabcontrol"_ustr))
        , m_xInstanceBtn(rBuilder.weld_menu_button(u"instances"_ustr))
        , m_nLastSelectedPos(-1)
        , m_bShowDetails(false)
        , m_bIsNotifyDisabled(false)
        , m_aUpdateTimer("svx DataNavigatorWindow m_aUpdateTimer")
        , m_xDataListener(new DataListener(this))
    {
        // handler
        m_xModelsBox->connect_changed( LINK( this, DataNavigatorWindow, ModelSelectListBoxHdl ) );
        Link<const OUString&, void> aLink1 = LINK( this, DataNavigatorWindow, MenuSelectHdl );
        m_xModelBtn->connect_selected(aLink1);
        m_xInstanceBtn->connect_selected(aLink1);
        Link<weld::Toggleable&,void> aLink2 = LINK( this, DataNavigatorWindow, MenuActivateHdl );
        m_xModelBtn->connect_toggled( aLink2 );
        m_xInstanceBtn->connect_toggled( aLink2 );
        m_xTabCtrl->connect_enter_page( LINK( this, DataNavigatorWindow, ActivatePageHdl ) );
        m_aUpdateTimer.SetTimeout( 2000 );
        m_aUpdateTimer.SetInvokeHandler( LINK( this, DataNavigatorWindow, UpdateHdl ) );

        // init tabcontrol
        OUString sPageId(u"instance"_ustr);
        SvtViewOptions aViewOpt( EViewType::TabDialog, CFGNAME_DATANAVIGATOR );
        if ( aViewOpt.Exists() )
        {
            OUString sNewPageId = aViewOpt.GetPageID();
            if (m_xTabCtrl->get_page_index(sNewPageId) != -1)
                sPageId = sNewPageId;
            aViewOpt.GetUserItem(CFGNAME_SHOWDETAILS) >>= m_bShowDetails;
        }

        m_xInstanceBtn->set_item_active(u"instancesdetails"_ustr, m_bShowDetails);

        m_xTabCtrl->set_current_page(sPageId);
        ActivatePageHdl(sPageId);

        // get our frame
        assert(pBindings != nullptr &&
                    "DataNavigatorWindow::LoadModels(): no SfxBindings; can't get frame");
        m_xFrame = pBindings->GetDispatcher()->GetFrame()->GetFrame().GetFrameInterface();
        DBG_ASSERT( m_xFrame.is(), "DataNavigatorWindow::LoadModels(): no frame" );
        // add frameaction listener
        Reference< XFrameActionListener > xListener = m_xDataListener;
        m_xFrame->addFrameActionListener( xListener );

        // load xforms models of the current document
        LoadModels();

        // tdf#154322 select the first entry of the current page by default
        if (XFormsPage* pPage = GetPage(sPageId))
            pPage->SelectFirstEntry();
    }

    DataNavigatorWindow::~DataNavigatorWindow()
    {
        Reference< XFrameActionListener > xListener = m_xDataListener;
        m_xFrame->removeFrameActionListener( xListener );

        SvtViewOptions aViewOpt( EViewType::TabDialog, CFGNAME_DATANAVIGATOR );
        aViewOpt.SetPageID(m_xTabCtrl->get_current_page_ident());
        aViewOpt.SetUserItem(CFGNAME_SHOWDETAILS, Any(m_bShowDetails));

        m_xInstPage.reset();
        m_xSubmissionPage.reset();
        m_xBindingPage.reset();

        sal_Int32 i, nCount = m_aPageList.size();
        for ( i = 0; i < nCount; ++i )
            m_aPageList[i].reset();
        m_aPageList.clear();

        RemoveBroadcaster();
        m_xDataListener.clear();
    }

    IMPL_LINK( DataNavigatorWindow, ModelSelectListBoxHdl, weld::ComboBox&, rBox, void )
    {
        ModelSelectHdl(&rBox);
    }

    void DataNavigatorWindow::ModelSelectHdl(const weld::ComboBox* pBox)
    {
        sal_Int32 nPos = m_xModelsBox->get_active();
        // pBox == NULL, if you want to force a new fill.
        if ( nPos != m_nLastSelectedPos || !pBox )
        {
            m_nLastSelectedPos = nPos;
            ClearAllPageModels( pBox != nullptr );
            InitPages();
            SetPageModel(GetCurrentPage());
        }
    }

    IMPL_LINK(DataNavigatorWindow, MenuSelectHdl, const OUString&, rIdent, void)
    {
        bool bIsDocModified = false;
        Reference< css::xforms::XFormsUIHelper1 > xUIHelper;
        sal_Int32 nSelectedPos = m_xModelsBox->get_active();
        OUString sSelectedModel(m_xModelsBox->get_text(nSelectedPos));
        Reference< css::xforms::XModel > xModel;
        try
        {
            Any aAny = m_xDataContainer->getByName( sSelectedModel );
            if ( aAny >>= xModel )
                xUIHelper.set( xModel, UNO_QUERY );
        }
        catch ( Exception const & )
        {
            TOOLS_WARN_EXCEPTION( "svx.form", "DataNavigatorWindow::MenuSelectHdl()" );
        }
        DBG_ASSERT( xUIHelper.is(), "DataNavigatorWindow::MenuSelectHdl(): no UIHelper" );

        m_bIsNotifyDisabled = true;

        if (rIdent == "modelsadd")
        {
            AddModelDialog aDlg(GetFrameWeld(), false);
            bool bShowDialog = true;
            while ( bShowDialog )
            {
                bShowDialog = false;
                if (aDlg.run() == RET_OK)
                {
                    OUString sNewName = aDlg.GetName();
                    bool bDocumentData = aDlg.GetModifyDoc();

                    if (m_xModelsBox->find_text(sNewName) != -1)
                    {
                        // error: model name already exists
                        std::unique_ptr<weld::MessageDialog> xErrBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                                                 VclMessageType::Warning, VclButtonsType::Ok,
                                                                                 SvxResId(RID_STR_DOUBLE_MODELNAME)));
                        xErrBox->set_primary_text(xErrBox->get_primary_text().replaceFirst(MSG_VARIABLE, sNewName));
                        xErrBox->run();
                        bShowDialog = true;
                    }
                    else
                    {
                        try
                        {
                            // add new model to frame model
                            Reference< css::xforms::XModel > xNewModel(
                                xUIHelper->newModel( m_xFrameModel, sNewName ), UNO_SET_THROW );

                            Reference< XPropertySet > xModelProps( xNewModel, UNO_QUERY_THROW );
                            xModelProps->setPropertyValue(u"ExternalData"_ustr, Any( !bDocumentData ) );

                            m_xModelsBox->append_text(sNewName);
                            m_xModelsBox->set_active(m_xModelsBox->get_count() - 1);
                            ModelSelectHdl(m_xModelsBox.get());
                            bIsDocModified = true;
                        }
                        catch ( Exception const & )
                        {
                            TOOLS_WARN_EXCEPTION( "svx.form", "DataNavigatorWindow::MenuSelectHdl()" );
                        }
                    }
                }
            }
        }
        else if (rIdent == "modelsedit")
        {
            AddModelDialog aDlg(GetFrameWeld(), true);
            aDlg.SetName( sSelectedModel );

            bool bDocumentData( false );
            try
            {
                Reference< css::xforms::XFormsSupplier > xFormsSupp( m_xFrameModel, UNO_QUERY_THROW );
                Reference< XNameContainer > xXForms( xFormsSupp->getXForms(), UNO_SET_THROW );
                Reference< XPropertySet > xModelProps( xXForms->getByName( sSelectedModel ), UNO_QUERY_THROW );
                bool bExternalData = false;
                OSL_VERIFY( xModelProps->getPropertyValue( u"ExternalData"_ustr ) >>= bExternalData );
                bDocumentData = !bExternalData;
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("svx");
            }
            aDlg.SetModifyDoc( bDocumentData );

            if (aDlg.run() == RET_OK)
            {
                if ( aDlg.GetModifyDoc() != bDocumentData )
                {
                    bDocumentData = aDlg.GetModifyDoc();
                    try
                    {
                        Reference< css::xforms::XFormsSupplier > xFormsSupp( m_xFrameModel, UNO_QUERY_THROW );
                        Reference< XNameContainer > xXForms( xFormsSupp->getXForms(), UNO_SET_THROW );
                        Reference< XPropertySet > xModelProps( xXForms->getByName( sSelectedModel ), UNO_QUERY_THROW );
                        xModelProps->setPropertyValue( u"ExternalData"_ustr, Any( !bDocumentData ) );
                        bIsDocModified = true;
                    }
                    catch( const Exception& )
                    {
                        DBG_UNHANDLED_EXCEPTION("svx");
                    }
                }

                OUString sNewName = aDlg.GetName();
                if ( !sNewName.isEmpty() && ( sNewName != sSelectedModel ) )
                {
                    try
                    {
                        xUIHelper->renameModel( m_xFrameModel, sSelectedModel, sNewName );

                        m_xModelsBox->remove(nSelectedPos);
                        m_xModelsBox->append_text(sNewName);
                        m_xModelsBox->set_active(m_xModelsBox->get_count() - 1);
                        bIsDocModified = true;
                    }
                    catch ( Exception const & )
                    {
                        TOOLS_WARN_EXCEPTION( "svx.form", "DataNavigatorWindow::MenuSelectHdl()" );
                    }
                }
            }
        }
        else if (rIdent == "modelsremove")
        {
            std::unique_ptr<weld::MessageDialog> xQBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                                     VclMessageType::Question, VclButtonsType::YesNo,
                                                                     SvxResId( RID_STR_QRY_REMOVE_MODEL)));
            OUString sText = xQBox->get_primary_text();
            sText = sText.replaceFirst( MODELNAME, sSelectedModel );
            xQBox->set_primary_text(sText);
            if (xQBox->run() == RET_YES)
            {
                try
                {
                    xUIHelper->removeModel( m_xFrameModel, sSelectedModel );
                }
                catch ( Exception const & )
                {
                    TOOLS_WARN_EXCEPTION( "svx.form", "DataNavigatorWindow::MenuSelectHdl()" );
                }
                m_xModelsBox->remove(nSelectedPos);
                if (m_xModelsBox->get_count() <= nSelectedPos)
                    nSelectedPos = m_xModelsBox->get_count() - 1;
                m_xModelsBox->set_active(nSelectedPos);
                ModelSelectHdl(m_xModelsBox.get());
                bIsDocModified = true;
            }
        }
        else if (rIdent == "instancesadd")
        {
            AddInstanceDialog aDlg(GetFrameWeld(), false);
            if (aDlg.run() == RET_OK)
            {
                OUString sPageId = GetNewPageId(); // ModelSelectHdl will cause a page of this id to be created

                OUString sName = aDlg.GetName();
                if (sName.isEmpty())
                {
                    SAL_WARN( "svx.form", "DataNavigatorWindow::CreateInstancePage(): instance without name" );
                    sName = "untitled";
                }

                OUString sURL = aDlg.GetURL();
                bool bLinkOnce = aDlg.IsLinkInstance();
                try
                {
                    xUIHelper->newInstance( sName, sURL, !bLinkOnce );
                }
                catch ( Exception const & )
                {
                    TOOLS_WARN_EXCEPTION( "svx.form", "DataNavigatorWindow::MenuSelectHdl()" );
                }
                ModelSelectHdl( nullptr );

                XFormsPage* pPage = GetPage(sPageId);
                pPage->SetInstanceName(sName);
                pPage->SetInstanceURL(sURL);
                pPage->SetLinkOnce(bLinkOnce);
                ActivatePageHdl(sPageId);

                bIsDocModified = true;
            }
        }
        else if (rIdent == "instancesedit")
        {
            OUString sIdent = GetCurrentPage();
            XFormsPage* pPage = GetPage(sIdent);
            if ( pPage )
            {
                AddInstanceDialog aDlg(GetFrameWeld(), true);
                aDlg.SetName( pPage->GetInstanceName() );
                aDlg.SetURL( pPage->GetInstanceURL() );
                aDlg.SetLinkInstance( pPage->GetLinkOnce() );
                OUString sOldName = aDlg.GetName();
                if (aDlg.run() == RET_OK)
                {
                    OUString sNewName = aDlg.GetName();
                    OUString sURL = aDlg.GetURL();
                    bool bLinkOnce = aDlg.IsLinkInstance();
                    try
                    {
                        xUIHelper->renameInstance( sOldName,
                                                   sNewName,
                                                   sURL,
                                                   !bLinkOnce );
                    }
                    catch ( Exception const & )
                    {
                        TOOLS_WARN_EXCEPTION( "svx.form", "DataNavigatorWindow::MenuSelectHdl()" );
                    }
                    pPage->SetInstanceName(sNewName);
                    pPage->SetInstanceURL(sURL);
                    pPage->SetLinkOnce(bLinkOnce);
                    m_xTabCtrl->set_tab_label_text(sIdent, sNewName);
                    bIsDocModified = true;
                }
            }
        }
        else if (rIdent == "instancesremove")
        {
            OUString sIdent = GetCurrentPage();
            XFormsPage* pPage = GetPage(sIdent);
            if (pPage)
            {
                OUString sInstName = pPage->GetInstanceName();
                std::unique_ptr<weld::MessageDialog> xQBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                                         VclMessageType::Question, VclButtonsType::YesNo,
                                                                         SvxResId(RID_STR_QRY_REMOVE_INSTANCE)));
                OUString sMessText = xQBox->get_primary_text();
                sMessText = sMessText.replaceFirst( INSTANCENAME, sInstName );
                xQBox->set_primary_text(sMessText);
                if (xQBox->run() == RET_YES)
                {
                    bool bDoRemove = false;
                    if (IsAdditionalPage(sIdent))
                    {
                        auto aPageListEnd = m_aPageList.end();
                        auto aFoundPage = std::find_if(m_aPageList.begin(), aPageListEnd,
                                                       [pPage](const auto&elem) { return elem.get() == pPage; });
                        if ( aFoundPage != aPageListEnd )
                        {
                            m_aPageList.erase( aFoundPage );
                            bDoRemove = true;
                        }
                    }
                    else
                    {
                        m_xInstPage.reset();
                        bDoRemove = true;
                    }

                    if ( bDoRemove )
                    {
                        try
                        {
                            xUIHelper->removeInstance( sInstName );
                        }
                        catch (const Exception&)
                        {
                            TOOLS_WARN_EXCEPTION( "svx.form", "DataNavigatorWindow::MenuSelectHdl()" );
                        }
                        m_xTabCtrl->remove_page(sIdent);
                        m_xTabCtrl->set_current_page(u"instance"_ustr);
                        ModelSelectHdl( nullptr );
                        bIsDocModified = true;
                    }
                }
            }
        }
        else if (rIdent == "instancesdetails")
        {
            m_bShowDetails = !m_bShowDetails;
            m_xInstanceBtn->set_item_active(u"instancesdetails"_ustr, m_bShowDetails);
            ModelSelectHdl(m_xModelsBox.get());
        }
        else
        {
            SAL_WARN( "svx.form", "DataNavigatorWindow::MenuSelectHdl(): wrong menu item" );
        }

        m_bIsNotifyDisabled = false;

        if ( bIsDocModified )
            SetDocModified();
    }

    bool DataNavigatorWindow::IsAdditionalPage(std::u16string_view rIdent)
    {
        return o3tl::starts_with(rIdent, u"additional");
    }

    IMPL_LINK( DataNavigatorWindow, MenuActivateHdl, weld::Toggleable&, rBtn, void )
    {
        if (m_xInstanceBtn.get() == &rBtn)
        {
            OUString sIdent(m_xTabCtrl->get_current_page_ident());
            bool bIsInstPage = (IsAdditionalPage(sIdent) || sIdent == "instance");
            m_xInstanceBtn->set_item_sensitive( u"instancesedit"_ustr, bIsInstPage );
            m_xInstanceBtn->set_item_sensitive( u"instancesremove"_ustr,
                bIsInstPage && m_xTabCtrl->get_n_pages() > MIN_PAGE_COUNT );
            m_xInstanceBtn->set_item_sensitive( u"instancesdetails"_ustr, bIsInstPage );
        }
        else if (m_xModelBtn.get() == &rBtn)
        {
            // we need at least one model!
            m_xModelBtn->set_item_sensitive(u"modelsremove"_ustr, m_xModelsBox->get_count() > 1 );
        }
        else
        {
            SAL_WARN( "svx.form", "DataNavigatorWindow::MenuActivateHdl(): wrong button" );
        }
    }

    IMPL_LINK(DataNavigatorWindow, ActivatePageHdl, const OUString&, rIdent, void)
    {
        XFormsPage* pPage = GetPage(rIdent);
        if (!pPage)
            return;
        if (m_xDataContainer.is() && !pPage->HasModel())
            SetPageModel(rIdent);
    }

    IMPL_LINK_NOARG(DataNavigatorWindow, UpdateHdl, Timer *, void)
    {
        ModelSelectHdl( nullptr );
    }

    XFormsPage* DataNavigatorWindow::GetPage(const OUString& rCurId)
    {
        XFormsPage* pPage = nullptr;
        if (rCurId == "submissions")
        {
            if (!m_xSubmissionPage)
                m_xSubmissionPage.reset(new XFormsPage(m_xTabCtrl->get_page(rCurId), this, DGTSubmission));
            pPage = m_xSubmissionPage.get();
        }
        else if (rCurId == "bindings")
        {
            if (!m_xBindingPage)
                m_xBindingPage.reset(new XFormsPage(m_xTabCtrl->get_page(rCurId), this, DGTBinding));
            pPage = m_xBindingPage.get();
        }
        else if (rCurId == "instance")
        {
            if (!m_xInstPage)
                m_xInstPage.reset(new XFormsPage(m_xTabCtrl->get_page(rCurId), this, DGTInstance));
            pPage = m_xInstPage.get();
        }
        else
        {
            sal_uInt16 nPos = m_xTabCtrl->get_page_index(rCurId);
            if (HasFirstInstancePage() && nPos > 0)
                nPos--;
            if (m_aPageList.size() > nPos)
                pPage = m_aPageList[nPos].get();
            else
            {
                m_aPageList.emplace_back(std::make_unique<XFormsPage>(m_xTabCtrl->get_page(rCurId), this, DGTInstance));
                pPage = m_aPageList.back().get();
            }
        }
        return pPage;
    }

    OUString DataNavigatorWindow::GetCurrentPage() const
    {
        return m_xTabCtrl->get_current_page_ident();
    }

    void DataNavigatorWindow::LoadModels()
    {
        if ( !m_xFrameModel.is() )
        {
            // get model of active frame
            Reference< XController > xCtrl = m_xFrame->getController();
            if ( xCtrl.is() )
            {
                try
                {
                    m_xFrameModel = xCtrl->getModel();
                }
                catch ( Exception const & )
                {
                    TOOLS_WARN_EXCEPTION( "svx.form", "DataNavigatorWindow::LoadModels()" );
                }
            }
        }

        if ( m_xFrameModel.is() )
        {
            try
            {
                Reference< css::xforms::XFormsSupplier > xFormsSupp( m_xFrameModel, UNO_QUERY );
                if ( xFormsSupp.is() )
                {
                    Reference< XNameContainer > xContainer = xFormsSupp->getXForms();
                    if ( xContainer.is() )
                    {
                        m_xDataContainer = xContainer;
                        const Sequence< OUString > aNameList = m_xDataContainer->getElementNames();
                        for ( const OUString& rName : aNameList )
                        {
                            Any aAny = m_xDataContainer->getByName( rName );
                            Reference< css::xforms::XModel > xFormsModel;
                            if ( aAny >>= xFormsModel )
                                m_xModelsBox->append_text(xFormsModel->getID());
                        }
                    }
                }
            }
            catch( Exception const & )
            {
                TOOLS_WARN_EXCEPTION( "svx.form", "DataNavigatorWindow::LoadModels()" );
            }
        }

        if (m_xModelsBox->get_count() > 0)
        {
            m_xModelsBox->set_active(0);
            ModelSelectHdl(m_xModelsBox.get());
        }
    }

    void DataNavigatorWindow::SetPageModel(const OUString& rIdent)
    {
        OUString sModel(m_xModelsBox->get_active_text());
        try
        {
            Any aAny = m_xDataContainer->getByName( sModel );
            Reference< css::xforms::XModel > xFormsModel;
            if ( aAny >>= xFormsModel )
            {
                int nPagePos = -1;
                XFormsPage* pPage = GetPage(rIdent);
                DBG_ASSERT( pPage, "DataNavigatorWindow::SetPageModel(): no page" );
                if (IsAdditionalPage(rIdent) || rIdent == "instance")
                {
                    // instance page
                    nPagePos = m_xTabCtrl->get_page_index(rIdent);
                }
                m_bIsNotifyDisabled = true;
                OUString sText = pPage->SetModel( xFormsModel, nPagePos );
                m_bIsNotifyDisabled = false;
                if (!sText.isEmpty())
                    m_xTabCtrl->set_tab_label_text(rIdent, sText);
            }
        }
        catch (const NoSuchElementException& )
        {
            SAL_WARN( "svx.form", "DataNavigatorWindow::SetPageModel(): no such element" );
        }
        catch( Exception const & )
        {
            TOOLS_WARN_EXCEPTION( "svx.form", "DataNavigatorWindow::SetPageModel()" );
        }
    }

    void DataNavigatorWindow::InitPages()
    {
        OUString sModel(m_xModelsBox->get_active_text());
        try
        {
            Any aAny = m_xDataContainer->getByName( sModel );
            Reference< css::xforms::XModel > xModel;
            if ( aAny >>= xModel )
            {
                Reference< XEnumerationAccess > xNumAccess = xModel->getInstances();
                if ( xNumAccess.is() )
                {
                    Reference < XEnumeration > xNum = xNumAccess->createEnumeration();
                    if ( xNum.is() && xNum->hasMoreElements() )
                    {
                        sal_Int32 nAlreadyLoadedCount = m_aPageList.size();
                        if ( !HasFirstInstancePage() && nAlreadyLoadedCount > 0 )
                            nAlreadyLoadedCount--;
                        sal_Int32 nIdx = 0;
                        while ( xNum->hasMoreElements() )
                        {
                            if ( nIdx > nAlreadyLoadedCount )
                            {
                                Sequence< PropertyValue > xPropSeq;
                                if ( xNum->nextElement() >>= xPropSeq )
                                    CreateInstancePage( xPropSeq );
                                else
                                {
                                    SAL_WARN( "svx.form", "DataNavigator::InitPages(): invalid instance" );
                                }
                            }
                            else
                                xNum->nextElement();
                            nIdx++;
                        }
                    }
                }
            }
        }
        catch ( NoSuchElementException& )
        {
            SAL_WARN( "svx.form", "DataNavigatorWindow::SetPageModel(): no such element" );
        }
        catch( Exception const & )
        {
            TOOLS_WARN_EXCEPTION( "svx.form", "DataNavigatorWindow::SetPageModel()" );
        }
    }

    void DataNavigatorWindow::ClearAllPageModels( bool bClearPages )
    {
        if ( m_xInstPage )
            m_xInstPage->ClearModel();
        if ( m_xSubmissionPage )
            m_xSubmissionPage->ClearModel();
        if ( m_xBindingPage )
            m_xBindingPage->ClearModel();

        sal_Int32 nCount = m_aPageList.size();
        for (sal_Int32 i = 0; i < nCount; ++i)
        {
            XFormsPage* pPage = m_aPageList[i].get();
            pPage->ClearModel();
        }

        if ( bClearPages )
        {
            m_aPageList.clear();
            while ( m_xTabCtrl->get_n_pages() > MIN_PAGE_COUNT )
                m_xTabCtrl->remove_page(m_xTabCtrl->get_page_ident(1));
        }
    }

    void DataNavigatorWindow::CreateInstancePage( const Sequence< PropertyValue >& _xPropSeq )
    {
        OUString sInstName;
        auto pProp = std::find_if(_xPropSeq.begin(), _xPropSeq.end(),
            [](const PropertyValue& rProp) { return PN_INSTANCE_ID == rProp.Name; });
        if (pProp != _xPropSeq.end())
            pProp->Value >>= sInstName;

        OUString sPageId = GetNewPageId();
        if ( sInstName.isEmpty() )
        {
            SAL_WARN( "svx.form", "DataNavigatorWindow::CreateInstancePage(): instance without name" );
            sInstName = "untitled";
        }
        m_xTabCtrl->insert_page(sPageId, sInstName, m_xTabCtrl->get_n_pages() - 2);
    }

    bool DataNavigatorWindow::HasFirstInstancePage() const
    {
        return m_xTabCtrl->get_page_ident(0) == "instance";
    }

    OUString DataNavigatorWindow::GetNewPageId() const
    {
        int nMax = 0;

        int nCount = m_xTabCtrl->get_n_pages();
        for (int i = 0; i < nCount; ++i)
        {
            OUString sIdent = m_xTabCtrl->get_page_ident(i);
            OUString sNumber;
            if (!sIdent.startsWith("additional", &sNumber))
                continue;
            int nPageId = sNumber.toInt32();
            if (nMax < nPageId)
                nMax = nPageId;
        }

        return "additional" + OUString::number(nMax + 1);
    }

    void DataNavigatorWindow::SetDocModified()
    {
        SfxObjectShell* pCurrentDoc = SfxObjectShell::Current();
        DBG_ASSERT( pCurrentDoc, "DataNavigatorWindow::SetDocModified(): no objectshell" );
        if (pCurrentDoc && !pCurrentDoc->IsModified() && pCurrentDoc->IsEnableSetModified())
            pCurrentDoc->SetModified();
    }

    void DataNavigatorWindow::NotifyChanges( bool _bLoadAll )
    {
        if ( m_bIsNotifyDisabled )
            return;

        if ( _bLoadAll )
        {
            // reset all members
            RemoveBroadcaster();
            m_xDataContainer.clear();
            m_xFrameModel.clear();
            m_xModelsBox->clear();
            m_nLastSelectedPos = -1;
            // for a reload
            LoadModels();
        }
        else
            m_aUpdateTimer.Start();
    }

    void DataNavigatorWindow::AddContainerBroadcaster( const css::uno::Reference< css::container::XContainer >& xContainer )
    {
        Reference< XContainerListener > xListener = m_xDataListener;
        xContainer->addContainerListener( xListener );
        m_aContainerList.push_back( xContainer );
    }


    void DataNavigatorWindow::AddEventBroadcaster( const css::uno::Reference< css::xml::dom::events::XEventTarget >& xTarget )
    {
        Reference< XEventListener > xListener = m_xDataListener;
        xTarget->addEventListener( EVENTTYPE_CHARDATA, xListener, true );
        xTarget->addEventListener( EVENTTYPE_CHARDATA, xListener, false );
        xTarget->addEventListener( EVENTTYPE_ATTR, xListener, true );
        xTarget->addEventListener( EVENTTYPE_ATTR, xListener, false );
        m_aEventTargetList.push_back( xTarget );
    }

    void DataNavigatorWindow::RemoveBroadcaster()
    {
        Reference< XContainerListener > xContainerListener = m_xDataListener;
        sal_Int32 i, nCount = m_aContainerList.size();
        for ( i = 0; i < nCount; ++i )
            m_aContainerList[i]->removeContainerListener( xContainerListener );
        Reference< XEventListener > xEventListener = m_xDataListener;
        nCount = m_aEventTargetList.size();
        for ( i = 0; i < nCount; ++i )
        {
            m_aEventTargetList[i]->removeEventListener( EVENTTYPE_CHARDATA, xEventListener, true );
            m_aEventTargetList[i]->removeEventListener( EVENTTYPE_CHARDATA, xEventListener, false );
            m_aEventTargetList[i]->removeEventListener( EVENTTYPE_ATTR, xEventListener, true );
            m_aEventTargetList[i]->removeEventListener( EVENTTYPE_ATTR, xEventListener, false );
        }
    }

    DataNavigator::DataNavigator(SfxBindings* _pBindings, SfxChildWindow* _pMgr, vcl::Window* _pParent)
        : SfxDockingWindow(_pBindings, _pMgr, _pParent, u"DataNavigator"_ustr, u"svx/ui/datanavigator.ui"_ustr)
        , SfxControllerItem(SID_FM_DATANAVIGATOR_CONTROL, *_pBindings)
        , m_xDataWin(new DataNavigatorWindow(this, *m_xBuilder, _pBindings))
    {
        SetText( SvxResId( RID_STR_DATANAVIGATOR ) );

        Size aSize = GetOptimalSize();
        Size aLogSize = PixelToLogic(aSize, MapMode(MapUnit::MapAppFont));
        SfxDockingWindow::SetFloatingSize( aLogSize );
    }

    DataNavigator::~DataNavigator()
    {
        disposeOnce();
    }

    void DataNavigator::dispose()
    {
        m_xDataWin.reset();
        ::SfxControllerItem::dispose();
        SfxDockingWindow::dispose();
    }

    void DataNavigator::StateChangedAtToolBoxControl( sal_uInt16 , SfxItemState , const SfxPoolItem*  )
    {
    }

    Size DataNavigator::CalcDockingSize( SfxChildAlignment eAlign )
    {
        if ( ( eAlign == SfxChildAlignment::TOP ) || ( eAlign == SfxChildAlignment::BOTTOM ) )
            return Size();

        return SfxDockingWindow::CalcDockingSize( eAlign );
    }

    SfxChildAlignment DataNavigator::CheckAlignment( SfxChildAlignment eActAlign, SfxChildAlignment eAlign )
    {
        switch ( eAlign )
        {
            case SfxChildAlignment::LEFT:
            case SfxChildAlignment::RIGHT:
            case SfxChildAlignment::NOALIGNMENT:
                return eAlign;
            default:
                break;
        }
        return eActAlign;
    }

    SFX_IMPL_DOCKINGWINDOW( DataNavigatorManager, SID_FM_SHOW_DATANAVIGATOR )

    DataNavigatorManager::DataNavigatorManager(
        vcl::Window* _pParent, sal_uInt16 _nId, SfxBindings* _pBindings, SfxChildWinInfo* _pInfo ) :

        SfxChildWindow( _pParent, _nId )

    {
        SetWindow( VclPtr<DataNavigator>::Create( _pBindings, this, _pParent ) );
        SetAlignment(SfxChildAlignment::RIGHT);
        GetWindow()->SetSizePixel( Size( 250, 400 ) );
        static_cast<SfxDockingWindow*>(GetWindow())->Initialize( _pInfo );
    }

    AddDataItemDialog::AddDataItemDialog(weld::Window* pParent, ItemNode* _pNode,
        const Reference< css::xforms::XFormsUIHelper1 >& _rUIHelper)
        : GenericDialogController(pParent, u"svx/ui/adddataitemdialog.ui"_ustr, u"AddDataItemDialog"_ustr)
        , m_xUIHelper(_rUIHelper)
        , m_pItemNode(_pNode)
        , m_eItemType(DITNone)
        , m_sFL_Element(SvxResId(RID_STR_ELEMENT))
        , m_sFL_Attribute(SvxResId(RID_STR_ATTRIBUTE))
        , m_sFL_Binding(SvxResId(RID_STR_BINDING))
        , m_sFT_BindingExp(SvxResId(RID_STR_BINDING_EXPR))
        , m_xItemFrame(m_xBuilder->weld_frame(u"itemframe"_ustr))
        , m_xNameFT(m_xBuilder->weld_label(u"nameft"_ustr))
        , m_xNameED(m_xBuilder->weld_entry(u"name"_ustr))
        , m_xDefaultFT(m_xBuilder->weld_label(u"valueft"_ustr))
        , m_xDefaultED(m_xBuilder->weld_entry(u"value"_ustr))
        , m_xDefaultBtn(m_xBuilder->weld_button(u"browse"_ustr))
        , m_xSettingsFrame(m_xBuilder->weld_widget(u"settingsframe"_ustr))
        , m_xDataTypeLB(m_xBuilder->weld_combo_box(u"datatype"_ustr))
        , m_xRequiredCB(m_xBuilder->weld_check_button(u"required"_ustr))
        , m_xRequiredBtn(m_xBuilder->weld_button(u"requiredcond"_ustr))
        , m_xRelevantCB(m_xBuilder->weld_check_button(u"relevant"_ustr))
        , m_xRelevantBtn(m_xBuilder->weld_button(u"relevantcond"_ustr))
        , m_xConstraintCB(m_xBuilder->weld_check_button(u"constraint"_ustr))
        , m_xConstraintBtn(m_xBuilder->weld_button(u"constraintcond"_ustr))
        , m_xReadonlyCB(m_xBuilder->weld_check_button(u"readonly"_ustr))
        , m_xReadonlyBtn(m_xBuilder->weld_button(u"readonlycond"_ustr))
        , m_xCalculateCB(m_xBuilder->weld_check_button(u"calculate"_ustr))
        , m_xCalculateBtn(m_xBuilder->weld_button(u"calculatecond"_ustr))
        , m_xOKBtn(m_xBuilder->weld_button(u"ok"_ustr))
    {
        InitDialog();
        InitFromNode();
        InitDataTypeBox();
        Check(nullptr);
    }

    AddDataItemDialog::~AddDataItemDialog()
    {
        if ( m_xTempBinding.is() )
        {
            Reference< css::xforms::XModel > xModel( m_xUIHelper, UNO_QUERY );
            if ( xModel.is() )
            {
                try
                {
                    Reference < XSet > xBindings = xModel->getBindings();
                    if ( xBindings.is() )
                        xBindings->remove( Any( m_xTempBinding ) );
                }
                catch (const Exception&)
                {
                    TOOLS_WARN_EXCEPTION( "svx.form", "AddDataItemDialog::Dtor()" );
                }
            }
        }
        if( m_xUIHelper.is()  &&  m_xBinding.is() )
        {
            // remove binding, if it does not convey 'useful' information
            m_xUIHelper->removeBindingIfUseless( m_xBinding );
        }
    }

    IMPL_LINK(AddDataItemDialog, CheckHdl, weld::Toggleable&, rBox, void)
    {
        Check(&rBox);
    }

    void AddDataItemDialog::Check(const weld::Toggleable* pBox)
    {
        // Condition buttons are only enable if their check box is checked
        m_xReadonlyBtn->set_sensitive( m_xReadonlyCB->get_active() );
        m_xRequiredBtn->set_sensitive( m_xRequiredCB->get_active() );
        m_xRelevantBtn->set_sensitive( m_xRelevantCB->get_active() );
        m_xConstraintBtn->set_sensitive( m_xConstraintCB->get_active() );
        m_xCalculateBtn->set_sensitive( m_xCalculateCB->get_active() );

        if ( !(pBox && m_xTempBinding.is()) )
            return;

        OUString sTemp, sPropName;
        if ( m_xRequiredCB.get() == pBox )
            sPropName = PN_REQUIRED_EXPR;
        else if ( m_xRelevantCB.get() == pBox )
            sPropName = PN_RELEVANT_EXPR;
        else if ( m_xConstraintCB.get() == pBox )
            sPropName = PN_CONSTRAINT_EXPR;
        else if ( m_xReadonlyCB.get() == pBox )
            sPropName = PN_READONLY_EXPR;
        else if ( m_xCalculateCB.get() == pBox )
            sPropName = PN_CALCULATE_EXPR;
        bool bIsChecked = pBox->get_active();
        m_xTempBinding->getPropertyValue( sPropName ) >>= sTemp;
        if ( bIsChecked && sTemp.isEmpty() )
            sTemp = TRUE_VALUE;
        else if ( !bIsChecked && !sTemp.isEmpty() )
            sTemp.clear();
        m_xTempBinding->setPropertyValue( sPropName, Any( sTemp ) );
    }

    IMPL_LINK(AddDataItemDialog, ConditionHdl, weld::Button&, rBtn, void)
    {
        OUString sPropName;
        if ( m_xDefaultBtn.get() == &rBtn )
            sPropName = PN_BINDING_EXPR;
        else if ( m_xRequiredBtn.get() == &rBtn )
            sPropName = PN_REQUIRED_EXPR;
        else if ( m_xRelevantBtn.get() == &rBtn )
            sPropName = PN_RELEVANT_EXPR;
        else if ( m_xConstraintBtn.get() == &rBtn )
            sPropName = PN_CONSTRAINT_EXPR;
        else if (m_xReadonlyBtn.get() == &rBtn)
            sPropName = PN_READONLY_EXPR;
        else if (m_xCalculateBtn.get() == &rBtn)
            sPropName = PN_CALCULATE_EXPR;
        AddConditionDialog aDlg(m_xDialog.get(), sPropName, m_xTempBinding);
        bool bIsDefBtn = ( m_xDefaultBtn.get() == &rBtn );
        OUString sCondition;
        if ( bIsDefBtn )
            sCondition = m_xDefaultED->get_text();
        else
        {
            OUString sTemp;
            m_xTempBinding->getPropertyValue( sPropName ) >>= sTemp;
            if ( sTemp.isEmpty() )
                sTemp = TRUE_VALUE;
            sCondition = sTemp;
        }
        aDlg.SetCondition( sCondition );

        if (aDlg.run() == RET_OK)
        {
            OUString sNewCondition = aDlg.GetCondition();
            if ( bIsDefBtn )
                m_xDefaultED->set_text(sNewCondition);
            else
            {

                m_xTempBinding->setPropertyValue(
                    sPropName, Any( sNewCondition ) );
            }
        }
    }

    static void copyPropSet( const Reference< XPropertySet >& xFrom, Reference< XPropertySet > const & xTo )
    {
        DBG_ASSERT( xFrom.is(), "copyPropSet(): no source" );
        DBG_ASSERT( xTo.is(), "copyPropSet(): no target" );

        try
        {
            // get property names & infos, and iterate over target properties
            const Sequence< Property > aProperties = xTo->getPropertySetInfo()->getProperties();
            Reference< XPropertySetInfo > xFromInfo = xFrom->getPropertySetInfo();
            for ( const Property& rProperty : aProperties )
            {
                const OUString& rName = rProperty.Name;

                // if both set have the property, copy the value
                // (catch and ignore exceptions, if any)
                if ( xFromInfo->hasPropertyByName( rName ) )
                {
                    // don't set readonly properties
                    Property aProperty = xFromInfo->getPropertyByName( rName );
                    if ( ( aProperty.Attributes & PropertyAttribute::READONLY ) == 0 )
                        xTo->setPropertyValue(rName, xFrom->getPropertyValue( rName ));
                }
                // else: no property? then ignore.
            }
        }
        catch ( Exception const & )
        {
            TOOLS_WARN_EXCEPTION( "svx.form", "copyPropSet()" );
        }
    }

    IMPL_LINK_NOARG(AddDataItemDialog, OKHdl, weld::Button&, void)
    {
        bool bIsHandleBinding = ( DITBinding == m_eItemType );
        bool bIsHandleText = ( DITText == m_eItemType );
        OUString sNewName( m_xNameED->get_text() );

        if ( ( !bIsHandleBinding && !bIsHandleText && !m_xUIHelper->isValidXMLName( sNewName ) ) ||
             ( bIsHandleBinding && sNewName.isEmpty() ) )
        {
            // Error and don't close the dialog
            std::unique_ptr<weld::MessageDialog> xErrBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                                     VclMessageType::Warning, VclButtonsType::Ok,
                                                                     SvxResId(RID_STR_INVALID_XMLNAME)));
            xErrBox->set_primary_text(xErrBox->get_primary_text().replaceFirst(MSG_VARIABLE, sNewName));
            xErrBox->run();
            return;
        }

        OUString sDataType( m_xDataTypeLB->get_active_text() );
        m_xTempBinding->setPropertyValue( PN_BINDING_TYPE, Any( sDataType ) );

        if ( bIsHandleBinding )
        {
            // copy properties from temp binding to original binding
            copyPropSet( m_xTempBinding, m_pItemNode->m_xPropSet );
            try
            {
                OUString sValue = m_xNameED->get_text();
                m_pItemNode->m_xPropSet->setPropertyValue( PN_BINDING_ID, Any( sValue ) );
                sValue = m_xDefaultED->get_text();
                m_pItemNode->m_xPropSet->setPropertyValue( PN_BINDING_EXPR, Any( sValue ) );
            }
            catch ( Exception const & )
            {
                TOOLS_WARN_EXCEPTION( "svx.form", "AddDataDialog::OKHdl()" );
            }
        }
        else
        {
            // copy properties from temp binding to original binding
            copyPropSet( m_xTempBinding, m_xBinding );
            try
            {
                if ( bIsHandleText )
                    m_xUIHelper->setNodeValue( m_pItemNode->m_xNode, m_xDefaultED->get_text() );
                else
                {
                    Reference< css::xml::dom::XNode > xNewNode =
                        m_xUIHelper->renameNode( m_pItemNode->m_xNode, m_xNameED->get_text() );
                    m_xUIHelper->setNodeValue( xNewNode, m_xDefaultED->get_text() );
                    m_pItemNode->m_xNode = xNewNode;
                }
            }
            catch ( Exception const & )
            {
                TOOLS_WARN_EXCEPTION( "svx.form", "AddDataDialog::OKHdl()" );
            }
        }
        // then close the dialog
        m_xDialog->response(RET_OK);
    }

    void AddDataItemDialog::InitDialog()
    {
        // set handler
        Link<weld::Toggleable&,void> aLink = LINK( this, AddDataItemDialog, CheckHdl );
        m_xRequiredCB->connect_toggled( aLink );
        m_xRelevantCB->connect_toggled( aLink );
        m_xConstraintCB->connect_toggled( aLink );
        m_xReadonlyCB->connect_toggled( aLink );
        m_xCalculateCB->connect_toggled( aLink );

        Link<weld::Button&,void> aLink2 = LINK( this, AddDataItemDialog, ConditionHdl );
        m_xDefaultBtn->connect_clicked( aLink2 );
        m_xRequiredBtn->connect_clicked( aLink2 );
        m_xRelevantBtn->connect_clicked( aLink2 );
        m_xConstraintBtn->connect_clicked( aLink2 );
        m_xReadonlyBtn->connect_clicked( aLink2 );
        m_xCalculateBtn->connect_clicked( aLink2 );

        m_xOKBtn->connect_clicked( LINK( this, AddDataItemDialog, OKHdl ) );
    }

    void AddDataItemDialog::InitFromNode()
    {
        if ( m_pItemNode )
        {
            if ( m_pItemNode->m_xNode.is() )
            {
                try
                {
                    // detect type of the node
                    css::xml::dom::NodeType eChildType = m_pItemNode->m_xNode->getNodeType();
                    switch ( eChildType )
                    {
                        case css::xml::dom::NodeType_ATTRIBUTE_NODE:
                            m_eItemType = DITAttribute;
                            break;
                        case css::xml::dom::NodeType_ELEMENT_NODE:
                            m_eItemType = DITElement;
                            break;
                        case css::xml::dom::NodeType_TEXT_NODE:
                            m_eItemType = DITText;
                            break;
                        default:
                            OSL_FAIL( "AddDataItemDialog::InitFronNode: cannot handle this node type!" );
                            break;
                    }

                    /** Get binding of the node and clone it
                        Then use this temporary binding in the dialog.
                        When the user click OK the temporary binding will be copied
                        into the original binding.
                     */

                    Reference< css::xml::dom::XNode > xNode = m_pItemNode->m_xNode;
                    m_xBinding = m_xUIHelper->getBindingForNode( xNode, true );
                    if ( m_xBinding.is() )
                    {
                        Reference< css::xforms::XModel > xModel( m_xUIHelper, UNO_QUERY );
                        if ( xModel.is() )
                        {
                            m_xTempBinding = m_xUIHelper->cloneBindingAsGhost( m_xBinding );
                            Reference < XSet > xBindings = xModel->getBindings();
                            if ( xBindings.is() )
                                xBindings->insert( Any( m_xTempBinding ) );
                        }
                    }

                    if ( m_eItemType != DITText )
                    {
                        OUString sName( m_xUIHelper->getNodeName( m_pItemNode->m_xNode ) );
                        m_xNameED->set_text( sName );
                    }
                    m_xDefaultED->set_text( m_pItemNode->m_xNode->getNodeValue() );
                }
                catch( Exception const & )
                {
                    TOOLS_WARN_EXCEPTION( "svx.form", "AddDataItemDialog::InitFromNode()" );
                }
            }
            else if ( m_pItemNode->m_xPropSet.is() )
            {
                m_eItemType = DITBinding;
                Reference< css::xforms::XModel > xModel( m_xUIHelper, UNO_QUERY );
                if ( xModel.is() )
                {
                    try
                    {
                        m_xTempBinding = m_xUIHelper->cloneBindingAsGhost( m_pItemNode->m_xPropSet );
                        Reference < XSet > xBindings = xModel->getBindings();
                        if ( xBindings.is() )
                            xBindings->insert( Any( m_xTempBinding ) );
                    }
                    catch ( Exception const & )
                    {
                        TOOLS_WARN_EXCEPTION( "svx.form", "AddDataItemDialog::InitFromNode()" );
                    }
                }
                try
                {
                    Reference< XPropertySetInfo > xInfo = m_pItemNode->m_xPropSet->getPropertySetInfo();
                    OUString sTemp;
                    if ( xInfo->hasPropertyByName( PN_BINDING_ID ) )
                    {
                        m_pItemNode->m_xPropSet->getPropertyValue( PN_BINDING_ID ) >>= sTemp;
                        m_xNameED->set_text( sTemp );
                        m_pItemNode->m_xPropSet->getPropertyValue( PN_BINDING_EXPR ) >>= sTemp;
                        m_xDefaultED->set_text( sTemp );
                    }
                    else if ( xInfo->hasPropertyByName( PN_SUBMISSION_BIND ) )
                    {
                        m_pItemNode->m_xPropSet->getPropertyValue( PN_SUBMISSION_ID ) >>= sTemp;
                        m_xNameED->set_text( sTemp );
                    }
                }
                catch( Exception const & )
                {
                    TOOLS_WARN_EXCEPTION( "svx.form", "AddDataItemDialog::InitFromNode()" );
                }

                m_xDefaultBtn->show();
            }

            if ( m_xTempBinding.is() )
            {
                try
                {
                    OUString sTemp;
                    if ( ( m_xTempBinding->getPropertyValue( PN_REQUIRED_EXPR ) >>= sTemp )
                        && !sTemp.isEmpty() )
                        m_xRequiredCB->set_active(true);
                    if ( ( m_xTempBinding->getPropertyValue( PN_RELEVANT_EXPR ) >>= sTemp )
                        && !sTemp.isEmpty() )
                        m_xRelevantCB->set_active(true);
                    if ( ( m_xTempBinding->getPropertyValue( PN_CONSTRAINT_EXPR ) >>= sTemp )
                        && !sTemp.isEmpty() )
                        m_xConstraintCB->set_active(true);
                    if ( ( m_xTempBinding->getPropertyValue( PN_READONLY_EXPR ) >>= sTemp )
                        && !sTemp.isEmpty() )
                        m_xReadonlyCB->set_active(true);
                    if ( ( m_xTempBinding->getPropertyValue( PN_CALCULATE_EXPR ) >>= sTemp )
                        && !sTemp.isEmpty() )
                        m_xCalculateCB->set_active(true);
                }
                catch (const Exception&)
                {
                    TOOLS_WARN_EXCEPTION( "svx.form", "AddDataItemDialog::InitFromNode()" );
                }
            }
        }

        if ( DITText == m_eItemType )
        {
            m_xSettingsFrame->hide();
            m_xNameFT->set_sensitive(false);
            m_xNameED->set_sensitive(false);
        }
    }

    void AddDataItemDialog::InitDataTypeBox()
    {
        if ( m_eItemType == DITText )
            return;

        Reference< css::xforms::XModel > xModel( m_xUIHelper, UNO_QUERY );
        if ( !xModel.is() )
            return;

        try
        {
            Reference< css::xforms::XDataTypeRepository > xDataTypes =
                xModel->getDataTypeRepository();
            if ( xDataTypes.is() )
            {
                const Sequence< OUString > aNameList = xDataTypes->getElementNames();
                for ( const OUString& rName : aNameList )
                    m_xDataTypeLB->append_text(rName);
            }

            if ( m_xTempBinding.is() )
            {
                OUString sTemp;
                if ( m_xTempBinding->getPropertyValue( PN_BINDING_TYPE ) >>= sTemp )
                {
                    int nPos = m_xDataTypeLB->find_text(sTemp);
                    if (nPos == -1)
                    {
                        m_xDataTypeLB->append_text(sTemp);
                        nPos = m_xDataTypeLB->get_count() - 1;
                    }
                    m_xDataTypeLB->set_active(nPos);
                }
            }
        }
        catch ( Exception const & )
        {
            TOOLS_WARN_EXCEPTION( "svx.form", "AddDataItemDialog::InitDataTypeBox()" );
        }
    }

    void AddDataItemDialog::InitText( DataItemType _eType )
    {
        OUString sText;

        switch ( _eType )
        {
            case DITAttribute :
            {
                sText = m_sFL_Attribute;
                break;
            }

            case DITBinding :
            {
                sText = m_sFL_Binding;
                m_xDefaultFT->set_label(m_sFT_BindingExp);
                break;
            }

            default:
            {
                sText = m_sFL_Element;
            }
        }

        m_xItemFrame->set_label(sText);
    }

    AddConditionDialog::AddConditionDialog(weld::Window* pParent,
        OUString _aPropertyName,
        const Reference< XPropertySet >& _rPropSet)
        : GenericDialogController(pParent, u"svx/ui/addconditiondialog.ui"_ustr, u"AddConditionDialog"_ustr)
        , m_aResultIdle("svx AddConditionDialog m_aResultIdle")
        , m_sPropertyName(std::move(_aPropertyName))
        , m_xBinding(_rPropSet)
        , m_xConditionED(m_xBuilder->weld_text_view(u"condition"_ustr))
        , m_xResultWin(m_xBuilder->weld_text_view(u"result"_ustr))
        , m_xEditNamespacesBtn(m_xBuilder->weld_button(u"edit"_ustr))
        , m_xOKBtn(m_xBuilder->weld_button(u"ok"_ustr))
    {
        DBG_ASSERT( m_xBinding.is(), "AddConditionDialog::Ctor(): no Binding" );

        m_xConditionED->set_size_request(m_xConditionED->get_approximate_digit_width() * 52,
                                         m_xConditionED->get_height_rows(4));
        m_xResultWin->set_size_request(m_xResultWin->get_approximate_digit_width() * 52,
                                       m_xResultWin->get_height_rows(4));

        m_xConditionED->connect_changed( LINK( this, AddConditionDialog, ModifyHdl ) );
        m_xEditNamespacesBtn->connect_clicked( LINK( this, AddConditionDialog, EditHdl ) );
        m_xOKBtn->connect_clicked( LINK( this, AddConditionDialog, OKHdl ) );
        m_aResultIdle.SetPriority( TaskPriority::LOWEST );
        m_aResultIdle.SetInvokeHandler( LINK( this, AddConditionDialog, ResultHdl ) );

        if ( !m_sPropertyName.isEmpty() )
        {
            try
            {
                OUString sTemp;
                if ( ( m_xBinding->getPropertyValue( m_sPropertyName ) >>= sTemp )
                    && !sTemp.isEmpty() )
                {
                    m_xConditionED->set_text( sTemp );
                }
                else
                {
//!                 m_xBinding->setPropertyValue( m_sPropertyName, makeAny( TRUE_VALUE ) );
                    m_xConditionED->set_text( TRUE_VALUE );
                }

                Reference< css::xforms::XModel > xModel;
                if ( ( m_xBinding->getPropertyValue( PN_BINDING_MODEL ) >>= xModel ) && xModel.is() )
                    m_xUIHelper.set( xModel, UNO_QUERY );
            }
            catch (const Exception&)
            {
                TOOLS_WARN_EXCEPTION( "svx.form", "AddConditionDialog::Ctor()" );
            }
        }

        DBG_ASSERT( m_xUIHelper.is(), "AddConditionDialog::Ctor(): no UIHelper" );
        ResultHdl( &m_aResultIdle );
    }

    AddConditionDialog::~AddConditionDialog()
    {
    }

    IMPL_LINK_NOARG(AddConditionDialog, EditHdl, weld::Button&, void)
    {
        Reference< XNameContainer > xNameContnr;
        try
        {
            m_xBinding->getPropertyValue( PN_BINDING_NAMESPACES ) >>= xNameContnr;
        }
        catch ( Exception const & )
        {
            TOOLS_WARN_EXCEPTION( "svx.form", "AddDataItemDialog::EditHdl()" );
        }
        NamespaceItemDialog aDlg(this, xNameContnr);
        aDlg.run();
        try
        {
            m_xBinding->setPropertyValue( PN_BINDING_NAMESPACES, Any( xNameContnr ) );
        }
        catch ( Exception const & )
        {
            TOOLS_WARN_EXCEPTION( "svx.form", "AddDataItemDialog::EditHdl()" );
        }
    }

    IMPL_LINK_NOARG(AddConditionDialog, OKHdl, weld::Button&, void)
    {
        m_xDialog->response(RET_OK);
    }

    IMPL_LINK_NOARG(AddConditionDialog, ModifyHdl, weld::TextView&, void)
    {
        m_aResultIdle.Start();
    }

    IMPL_LINK_NOARG(AddConditionDialog, ResultHdl, Timer *, void)
    {
        OUString sCondition = comphelper::string::strip(m_xConditionED->get_text(), ' ');
        OUString sResult;
        if ( !sCondition.isEmpty() )
        {
            try
            {
                sResult = m_xUIHelper->getResultForExpression( m_xBinding, ( m_sPropertyName == PN_BINDING_EXPR ), sCondition );
            }
            catch ( Exception const & )
            {
                TOOLS_WARN_EXCEPTION( "svx.form", "AddConditionDialog::ResultHdl()" );
            }
        }
        m_xResultWin->set_text(sResult);
    }

    NamespaceItemDialog::NamespaceItemDialog(AddConditionDialog* pCondDlg, Reference<XNameContainer>& rContainer)
        : GenericDialogController(pCondDlg->getDialog(), u"svx/ui/namespacedialog.ui"_ustr, u"NamespaceDialog"_ustr)
        , m_pConditionDlg(pCondDlg)
        , m_rNamespaces(rContainer)
        , m_xNamespacesList(m_xBuilder->weld_tree_view(u"namespaces"_ustr))
        , m_xAddNamespaceBtn(m_xBuilder->weld_button(u"add"_ustr))
        , m_xEditNamespaceBtn(m_xBuilder->weld_button(u"edit"_ustr))
        , m_xDeleteNamespaceBtn(m_xBuilder->weld_button(u"delete"_ustr))
        , m_xOKBtn(m_xBuilder->weld_button(u"ok"_ustr))
    {
        m_xNamespacesList->set_size_request(m_xNamespacesList->get_approximate_digit_width() * 80,
                                            m_xNamespacesList->get_height_rows(8));

        std::vector<int> aWidths
        {
            o3tl::narrowing<int>(m_xNamespacesList->get_approximate_digit_width() * 20)
        };
        m_xNamespacesList->set_column_fixed_widths(aWidths);

        m_xNamespacesList->connect_changed( LINK( this, NamespaceItemDialog, SelectHdl ) );
        Link<weld::Button&,void> aLink = LINK( this, NamespaceItemDialog, ClickHdl );
        m_xAddNamespaceBtn->connect_clicked( aLink );
        m_xEditNamespaceBtn->connect_clicked( aLink );
        m_xDeleteNamespaceBtn->connect_clicked( aLink );
        m_xOKBtn->connect_clicked( LINK( this, NamespaceItemDialog, OKHdl ) );

        LoadNamespaces();
        SelectHdl(*m_xNamespacesList);
    }

    NamespaceItemDialog::~NamespaceItemDialog()
    {
    }

    IMPL_LINK_NOARG( NamespaceItemDialog, SelectHdl, weld::TreeView&, void)
    {
        bool bEnable = m_xNamespacesList->get_selected_index() != -1;
        m_xEditNamespaceBtn->set_sensitive( bEnable );
        m_xDeleteNamespaceBtn->set_sensitive( bEnable );
    }

    IMPL_LINK( NamespaceItemDialog, ClickHdl, weld::Button&, rButton, void )
    {
        if (m_xAddNamespaceBtn.get() == &rButton)
        {
            ManageNamespaceDialog aDlg(m_xDialog.get(), m_pConditionDlg, false);
            if (aDlg.run() == RET_OK)
            {
                m_xNamespacesList->append_text(aDlg.GetPrefix());
                int nRow = m_xNamespacesList->n_children();
                m_xNamespacesList->set_text(nRow - 1, aDlg.GetURL(), 1);
            }
        }
        else if (m_xEditNamespaceBtn.get() == &rButton)
        {
            ManageNamespaceDialog aDlg(m_xDialog.get(), m_pConditionDlg, true);
            int nEntry = m_xNamespacesList->get_selected_index();
            DBG_ASSERT( nEntry != -1, "NamespaceItemDialog::ClickHdl(): no entry" );
            OUString sPrefix(m_xNamespacesList->get_text(nEntry, 0));
            aDlg.SetNamespace(sPrefix, m_xNamespacesList->get_text(nEntry, 1));
            if (aDlg.run() == RET_OK)
            {
                // if a prefix was changed, mark the old prefix as 'removed'
                if( sPrefix != aDlg.GetPrefix() )
                    m_aRemovedList.push_back( sPrefix );

                m_xNamespacesList->set_text(nEntry, aDlg.GetPrefix(), 0);
                m_xNamespacesList->set_text(nEntry, aDlg.GetURL(), 1);
            }
        }
        else if (m_xDeleteNamespaceBtn.get() == &rButton)
        {
            int nEntry = m_xNamespacesList->get_selected_index();
            DBG_ASSERT( nEntry != -1, "NamespaceItemDialog::ClickHdl(): no entry" );
            OUString sPrefix(m_xNamespacesList->get_text(nEntry, 0));
            m_aRemovedList.push_back( sPrefix );
            m_xNamespacesList->remove(nEntry);
        }
        else
        {
            SAL_WARN( "svx.form", "NamespaceItemDialog::ClickHdl(): invalid button" );
        }

        SelectHdl(*m_xNamespacesList);
    }

    IMPL_LINK_NOARG(NamespaceItemDialog, OKHdl, weld::Button&, void)
    {
        try
        {
            // update namespace container
            sal_Int32 i, nRemovedCount = m_aRemovedList.size();
            for( i = 0; i < nRemovedCount; ++i )
                m_rNamespaces->removeByName( m_aRemovedList[i] );

            sal_Int32 nEntryCount = m_xNamespacesList->n_children();
            for( i = 0; i < nEntryCount; ++i )
            {
                OUString sPrefix(m_xNamespacesList->get_text(i, 0));
                OUString sURL(m_xNamespacesList->get_text(i, 1));

                if ( m_rNamespaces->hasByName( sPrefix ) )
                    m_rNamespaces->replaceByName( sPrefix, Any( sURL ) );
                else
                    m_rNamespaces->insertByName( sPrefix, Any( sURL ) );
            }
        }
        catch ( Exception const & )
        {
            TOOLS_WARN_EXCEPTION( "svx.form", "NamespaceItemDialog::OKHdl()" );
        }
        // and close the dialog
        m_xDialog->response(RET_OK);
    }

    void NamespaceItemDialog::LoadNamespaces()
    {
        try
        {
            int nRow = 0;
            const Sequence< OUString > aAllNames = m_rNamespaces->getElementNames();
            for ( const OUString& sPrefix : aAllNames )
            {
                if ( m_rNamespaces->hasByName( sPrefix ) )
                {
                    OUString sURL;
                    Any aAny = m_rNamespaces->getByName( sPrefix );
                    if (aAny >>= sURL)
                    {
                        m_xNamespacesList->append_text(sPrefix);
                        m_xNamespacesList->set_text(nRow, sURL, 1);
                        ++nRow;
                    }
                }
            }
        }
        catch ( Exception const & )
        {
            TOOLS_WARN_EXCEPTION( "svx.form", "NamespaceItemDialog::LoadNamespaces()" );
        }
    }

    ManageNamespaceDialog::ManageNamespaceDialog(weld::Window* pParent, AddConditionDialog* pCondDlg, bool bIsEdit)
        : GenericDialogController(pParent, u"svx/ui/addnamespacedialog.ui"_ustr, u"AddNamespaceDialog"_ustr)
        , m_pConditionDlg(pCondDlg)
        , m_xPrefixED(m_xBuilder->weld_entry(u"prefix"_ustr))
        , m_xUrlED(m_xBuilder->weld_entry(u"url"_ustr))
        , m_xOKBtn(m_xBuilder->weld_button(u"ok"_ustr))
        , m_xAltTitle(m_xBuilder->weld_label(u"alttitle"_ustr))
    {
        if (bIsEdit)
            m_xDialog->set_title(m_xAltTitle->get_label());

        m_xOKBtn->connect_clicked(LINK(this, ManageNamespaceDialog, OKHdl));
    }

    ManageNamespaceDialog::~ManageNamespaceDialog()
    {
    }

    IMPL_LINK_NOARG(ManageNamespaceDialog, OKHdl, weld::Button&, void)
    {
        OUString sPrefix = m_xPrefixED->get_text();

        try
        {
            if (!m_pConditionDlg->GetUIHelper()->isValidPrefixName(sPrefix))
            {
                std::unique_ptr<weld::MessageDialog> xErrBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                                         VclMessageType::Warning, VclButtonsType::Ok,
                                                                         SvxResId(RID_STR_INVALID_XMLPREFIX)));
                xErrBox->set_primary_text(xErrBox->get_primary_text().replaceFirst(MSG_VARIABLE, sPrefix));
                xErrBox->run();
                return;
            }
        }
        catch ( Exception const & )
        {
            TOOLS_WARN_EXCEPTION( "svx.form", "ManageNamespacesDialog::OKHdl()" );
        }

        // no error so close the dialog
        m_xDialog->response(RET_OK);
    }

    AddSubmissionDialog::AddSubmissionDialog(
        weld::Window* pParent, ItemNode* _pNode,
        const Reference< css::xforms::XFormsUIHelper1 >& _rUIHelper)
        : GenericDialogController(pParent, u"svx/ui/addsubmissiondialog.ui"_ustr, u"AddSubmissionDialog"_ustr)
        , m_pItemNode(_pNode)
        , m_xUIHelper(_rUIHelper)
        , m_xNameED(m_xBuilder->weld_entry(u"name"_ustr))
        , m_xActionED(m_xBuilder->weld_entry(u"action"_ustr))
        , m_xMethodLB(m_xBuilder->weld_combo_box(u"method"_ustr))
        , m_xRefED(m_xBuilder->weld_entry(u"expression"_ustr))
        , m_xRefBtn(m_xBuilder->weld_button(u"browse"_ustr))
        , m_xBindLB(m_xBuilder->weld_combo_box(u"binding"_ustr))
        , m_xReplaceLB(m_xBuilder->weld_combo_box(u"replace"_ustr))
        , m_xOKBtn(m_xBuilder->weld_button(u"ok"_ustr))
    {
        FillAllBoxes();

        m_xRefBtn->connect_clicked( LINK( this, AddSubmissionDialog, RefHdl ) );
        m_xOKBtn->connect_clicked( LINK( this, AddSubmissionDialog, OKHdl ) );
    }

    AddSubmissionDialog::~AddSubmissionDialog()
    {
        // #i38991# if we have added a binding, we need to remove it as well.
        if( m_xCreatedBinding.is() && m_xUIHelper.is() )
            m_xUIHelper->removeBindingIfUseless( m_xCreatedBinding );
    }

    IMPL_LINK_NOARG(AddSubmissionDialog, RefHdl, weld::Button&, void)
    {
        AddConditionDialog aDlg(m_xDialog.get(), PN_BINDING_EXPR, m_xTempBinding );
        aDlg.SetCondition( m_xRefED->get_text() );
        if ( aDlg.run() == RET_OK )
            m_xRefED->set_text(aDlg.GetCondition());
    }

    IMPL_LINK_NOARG(AddSubmissionDialog, OKHdl, weld::Button&, void)
    {
        OUString sName(m_xNameED->get_text());
        if(sName.isEmpty())
        {
            std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                                     VclMessageType::Warning, VclButtonsType::Ok,
                                                                     SvxResId(RID_STR_EMPTY_SUBMISSIONNAME)));
            xErrorBox->run();
            return;
        }

        if ( !m_xSubmission.is() )
        {
            DBG_ASSERT( !m_xNewSubmission.is(),
                "AddSubmissionDialog::OKHdl(): new submission already exists" );

            // add a new submission
            Reference< css::xforms::XModel > xModel( m_xUIHelper, UNO_QUERY );
            if ( xModel.is() )
            {
                try
                {
                    m_xNewSubmission = xModel->createSubmission();
                    m_xSubmission = m_xNewSubmission;
                }
                catch ( Exception const & )
                {
                    TOOLS_WARN_EXCEPTION( "svx.form", "AddSubmissionDialog::OKHdl()" );
                }
            }
        }

        if ( m_xSubmission.is() )
        {
            OUString sTemp = m_xNameED->get_text();
            try
            {
                m_xSubmission->setPropertyValue( PN_SUBMISSION_ID, Any( sTemp ) );
                sTemp = m_xActionED->get_text();
                m_xSubmission->setPropertyValue( PN_SUBMISSION_ACTION, Any( sTemp ) );
                sTemp = m_aMethodString.toAPI( m_xMethodLB->get_active_text() );
                m_xSubmission->setPropertyValue( PN_SUBMISSION_METHOD, Any( sTemp ) );
                sTemp = m_xRefED->get_text();
                m_xSubmission->setPropertyValue( PN_SUBMISSION_REF, Any( sTemp ) );
                OUString sEntry = m_xBindLB->get_active_text();
                sal_Int32 nColonIdx = sEntry.indexOf(':');
                if (nColonIdx != -1)
                    sEntry = sEntry.copy(0, nColonIdx);
                sTemp = sEntry;
                m_xSubmission->setPropertyValue( PN_SUBMISSION_BIND, Any( sTemp ) );
                sTemp = m_aReplaceString.toAPI( m_xReplaceLB->get_active_text() );
                m_xSubmission->setPropertyValue( PN_SUBMISSION_REPLACE, Any( sTemp ) );
            }
            catch ( Exception const & )
            {
                TOOLS_WARN_EXCEPTION( "svx.form", "AddSubmissionDialog::OKHdl()" );
            }
        }

        m_xDialog->response(RET_OK);
    }

    void AddSubmissionDialog::FillAllBoxes()
    {
        // method box
        m_xMethodLB->append_text(SvxResId(RID_STR_METHOD_POST));
        m_xMethodLB->append_text(SvxResId(RID_STR_METHOD_PUT));
        m_xMethodLB->append_text(SvxResId(RID_STR_METHOD_GET));
        m_xMethodLB->set_active(0);

        // binding box
        Reference< css::xforms::XModel > xModel( m_xUIHelper, UNO_QUERY );
        if ( xModel.is() )
        {
            try
            {
                Reference< XEnumerationAccess > xNumAccess = xModel->getBindings();
                if ( xNumAccess.is() )
                {
                    Reference < XEnumeration > xNum = xNumAccess->createEnumeration();
                    if ( xNum.is() && xNum->hasMoreElements() )
                    {
                        while ( xNum->hasMoreElements() )
                        {
                            Reference< XPropertySet > xPropSet;
                            Any aAny = xNum->nextElement();
                            if ( aAny >>= xPropSet )
                            {
                                OUString sEntry;
                                OUString sTemp;
                                xPropSet->getPropertyValue( PN_BINDING_ID ) >>= sTemp;
                                sEntry += sTemp + ": ";
                                xPropSet->getPropertyValue( PN_BINDING_EXPR ) >>= sTemp;
                                sEntry += sTemp;
                                m_xBindLB->append_text(sEntry);

                                if ( !m_xTempBinding.is() )
                                    m_xTempBinding = xPropSet;
                            }
                        }
                    }
                }
            }
            catch ( Exception const & )
            {
                TOOLS_WARN_EXCEPTION( "svx.form", "AddSubmissionDialog::FillAllBoxes()" );
            }
        }

        // #i36342# we need a temporary binding; create one if no existing binding
        // is found
        if( !m_xTempBinding.is() )
        {
            m_xCreatedBinding = m_xUIHelper->getBindingForNode(
                Reference<css::xml::dom::XNode>(
                    xModel->getDefaultInstance()->getDocumentElement(),
                    UNO_QUERY_THROW ),
                true );
            m_xTempBinding = m_xCreatedBinding;
        }

        // replace box
        m_xReplaceLB->append_text(SvxResId(RID_STR_REPLACE_NONE));
        m_xReplaceLB->append_text(SvxResId(RID_STR_REPLACE_INST));
        m_xReplaceLB->append_text(SvxResId(RID_STR_REPLACE_DOC));


        // init the controls with the values of the submission
        if ( m_pItemNode && m_pItemNode->m_xPropSet.is() )
        {
            m_xSubmission = m_pItemNode->m_xPropSet;
            try
            {
                OUString sTemp;
                m_xSubmission->getPropertyValue( PN_SUBMISSION_ID ) >>= sTemp;
                m_xNameED->set_text( sTemp );
                m_xSubmission->getPropertyValue( PN_SUBMISSION_ACTION ) >>= sTemp;
                m_xActionED->set_text( sTemp );
                m_xSubmission->getPropertyValue( PN_SUBMISSION_REF ) >>= sTemp;
                m_xRefED->set_text(sTemp);

                m_xSubmission->getPropertyValue( PN_SUBMISSION_METHOD ) >>= sTemp;
                sTemp = m_aMethodString.toUI( sTemp );
                sal_Int32 nPos = m_xMethodLB->find_text( sTemp );
                if (nPos == -1)
                {
                    m_xMethodLB->append_text( sTemp );
                    nPos = m_xMethodLB->get_count() - 1;
                }
                m_xMethodLB->set_active( nPos );

                m_xSubmission->getPropertyValue( PN_SUBMISSION_BIND ) >>= sTemp;
                nPos = m_xBindLB->find_text(sTemp);
                if (nPos == -1)
                {
                    m_xBindLB->append_text(sTemp);
                    nPos = m_xBindLB->get_count() - 1;
                }
                m_xBindLB->set_active(nPos);

                m_xSubmission->getPropertyValue( PN_SUBMISSION_REPLACE ) >>= sTemp;
                sTemp = m_aReplaceString.toUI( sTemp );
                if ( sTemp.isEmpty() )
                    sTemp = m_xReplaceLB->get_text(0); // first entry == "none"
                nPos = m_xReplaceLB->find_text(sTemp);
                if (nPos == -1)
                {
                    m_xReplaceLB->append_text(sTemp);
                    nPos = m_xReplaceLB->get_count() - 1;
                }
                m_xReplaceLB->set_active(nPos);
            }
            catch ( Exception const & )
            {
                TOOLS_WARN_EXCEPTION( "svx.form", "AddSubmissionDialog::FillAllBoxes()" );
            }
        }

        m_xRefBtn->set_sensitive(m_xTempBinding.is());
    }

    AddModelDialog::AddModelDialog(weld::Window* pParent, bool bIsEdit)
        : GenericDialogController(pParent, u"svx/ui/addmodeldialog.ui"_ustr, u"AddModelDialog"_ustr)
        , m_xNameED(m_xBuilder->weld_entry(u"name"_ustr))
        , m_xModifyCB(m_xBuilder->weld_check_button(u"modify"_ustr))
        , m_xAltTitle(m_xBuilder->weld_label(u"alttitle"_ustr))
    {
        if (bIsEdit)
            m_xDialog->set_title(m_xAltTitle->get_label());
    }

    AddModelDialog::~AddModelDialog()
    {
    }

    AddInstanceDialog::AddInstanceDialog(weld::Window* pParent, bool _bEdit)
        : GenericDialogController(pParent, u"svx/ui/addinstancedialog.ui"_ustr, u"AddInstanceDialog"_ustr)
        , m_xNameED(m_xBuilder->weld_entry(u"name"_ustr))
        , m_xURLED(new SvtURLBox(m_xBuilder->weld_combo_box(u"url"_ustr)))
        , m_xFilePickerBtn(m_xBuilder->weld_button(u"browse"_ustr))
        , m_xLinkInstanceCB(m_xBuilder->weld_check_button(u"link"_ustr))
        , m_xAltTitle(m_xBuilder->weld_label(u"alttitle"_ustr))
    {
        if (_bEdit)
            m_xDialog->set_title(m_xAltTitle->get_label());

        m_xURLED->DisableHistory();
        m_xFilePickerBtn->connect_clicked(LINK(this, AddInstanceDialog, FilePickerHdl));

        // load the filter name from fps resource
        m_sAllFilterName = Translate::get(STR_FILTERNAME_ALL, Translate::Create("fps"));
    }

    AddInstanceDialog::~AddInstanceDialog()
    {
    }

    IMPL_LINK_NOARG(AddInstanceDialog, FilePickerHdl, weld::Button&, void)
    {
        ::sfx2::FileDialogHelper aDlg(
            css::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
            FileDialogFlags::NONE, m_xDialog.get());
        aDlg.SetContext(sfx2::FileDialogHelper::FormsAddInstance);

        aDlg.AddFilter( m_sAllFilterName, FILEDIALOG_FILTER_ALL );
        OUString sFilterName( u"XML"_ustr );
        aDlg.AddFilter( sFilterName, u"*.xml"_ustr );
        aDlg.SetCurrentFilter( sFilterName );

        if (aDlg.Execute() == ERRCODE_NONE)
            m_xURLED->set_entry_text(aDlg.GetPath());
    }

    LinkedInstanceWarningBox::LinkedInstanceWarningBox(weld::Widget* pParent)
        : MessageDialogController(pParent, u"svx/ui/formlinkwarndialog.ui"_ustr,
                u"FormLinkWarnDialog"_ustr)
    {
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
