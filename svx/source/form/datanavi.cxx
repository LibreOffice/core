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

#include <sal/macros.h>
#include <sal/log.hxx>
#include <datanavi.hxx>
#include <fmservs.hxx>

#include <bitmaps.hlst>
#include <fpicker/strings.hrc>
#include <svx/svxids.hrc>
#include <tools/diagnose_ex.h>
#include <unotools/resmgr.hxx>
#include <svx/xmlexchg.hxx>
#include <svx/fmshell.hxx>
#include <svtools/miscopt.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/viewoptions.hxx>
#include <vcl/treelistentry.hxx>
#include <sfx2/app.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/objitem.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/weld.hxx>
#include <vcl/builderfactory.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/xforms/XFormsSupplier.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/dom/DOMException.hpp>
#include <com/sun/star/form/binding/XValueBinding.hpp>
#include <comphelper/string.hxx>

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::dom::events;
using namespace ::svx;

#define CFGNAME_DATANAVIGATOR       "DataNavigator"
#define CFGNAME_SHOWDETAILS         "ShowDetails"
#define MSG_VARIABLE                "%1"
#define MODELNAME                   "$MODELNAME"
#define INSTANCENAME                "$INSTANCENAME"
#define ELEMENTNAME                 "$ELEMENTNAME"
#define ATTRIBUTENAME               "$ATTRIBUTENAME"
#define SUBMISSIONNAME              "$SUBMISSIONNAME"
#define BINDINGNAME                 "$BINDINGNAME"


namespace svxform
{

    // properties of instance
    #define PN_INSTANCE_MODEL       "Instance"
    #define PN_INSTANCE_ID          "ID"
    #define PN_INSTANCE_URL         "URL"

    // properties of binding
    #define PN_BINDING_ID           "BindingID"
    #define PN_BINDING_EXPR         "BindingExpression"
    #define PN_BINDING_MODEL        "Model"
    #define PN_BINDING_NAMESPACES   "ModelNamespaces"
    #define PN_READONLY_EXPR        "ReadonlyExpression"
    #define PN_RELEVANT_EXPR        "RelevantExpression"
    #define PN_REQUIRED_EXPR        "RequiredExpression"
    #define PN_CONSTRAINT_EXPR      "ConstraintExpression"
    #define PN_CALCULATE_EXPR       "CalculateExpression"
    #define PN_BINDING_TYPE         "Type"

    // properties of submission
    #define PN_SUBMISSION_ID        "ID"
    #define PN_SUBMISSION_BIND      "Bind"
    #define PN_SUBMISSION_REF       "Ref"
    #define PN_SUBMISSION_ACTION    "Action"
    #define PN_SUBMISSION_METHOD    "Method"
    #define PN_SUBMISSION_REPLACE   "Replace"

    // other const strings
    #define TRUE_VALUE              "true()"
    #define NEW_ELEMENT             "newElement"
    #define NEW_ATTRIBUTE           "newAttribute"
    #define EVENTTYPE_CHARDATA      "DOMCharacterDataModified"
    #define EVENTTYPE_ATTR          "DOMAttrModified"

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

    DataTreeListBox::DataTreeListBox(vcl::Window* pParent, WinBits nBits)
        : SvTreeListBox(pParent, nBits)
        , m_pXFormsPage(nullptr)
        , m_eGroup(DGTUnknown)
        , m_nAddId(0)
        , m_nAddElementId(0)
        , m_nAddAttributeId(0)
        , m_nEditId(0)
        , m_nRemoveId(0)
    {
        EnableContextMenuHandling();

        if ( DGTInstance == m_eGroup )
            SetDragDropMode( DragDropMode::CTRL_MOVE |DragDropMode::CTRL_COPY | DragDropMode::APP_COPY );
    }

    DataTreeListBox::~DataTreeListBox()
    {
        disposeOnce();
    }

    void DataTreeListBox::dispose()
    {
        DeleteAndClear();
        m_xMenu.clear();
        m_xBuilder.reset();
        m_pXFormsPage.clear();
        SvTreeListBox::dispose();
    }

    sal_Int8 DataTreeListBox::AcceptDrop( const AcceptDropEvent& /*rEvt*/ )
    {
        return DND_ACTION_NONE;
    }

    sal_Int8 DataTreeListBox::ExecuteDrop( const ExecuteDropEvent& /*rEvt*/ )
    {
        return DND_ACTION_NONE;
    }

    void DataTreeListBox::StartDrag( sal_Int8 /*_nAction*/, const Point& /*_rPosPixel*/ )
    {
        SvTreeListEntry* pSelected = FirstSelected();
        if ( !pSelected )
            // no drag without an entry
            return;

        if ( m_eGroup == DGTBinding )
            // for the moment, bindings cannot be dragged.
            // #i59395# / 2005-12-15 / frank.schoenheit@sun.com
            return;

        // GetServiceNameForNode() requires a datatype repository which
        // will be automatically build if requested???
        Reference< css::xforms::XModel > xModel( m_pXFormsPage->GetXFormsHelper(), UNO_QUERY );
        Reference< css::xforms::XDataTypeRepository > xDataTypes =
            xModel->getDataTypeRepository();
        if(!xDataTypes.is())
            return;

        ItemNode *pItemNode = static_cast<ItemNode*>(pSelected->GetUserData());

        if ( !pItemNode )
        {
            // the only known (and allowed?) case where this happens are sub-entries of a submission
            // entry
            DBG_ASSERT( DGTSubmission == m_eGroup, "DataTreeListBox::StartDrag: how this?" );
            pSelected = GetParent( pSelected );
            DBG_ASSERT( pSelected && !GetParent( pSelected ), "DataTreeListBox::StartDrag: what kind of entry *is* this?" );
                // on the submission page, we have only top-level entries (the submission themself)
                // plus direct children of those (facets of a submission)
            pItemNode = pSelected ? static_cast< ItemNode* >( pSelected->GetUserData() ) : nullptr;
            if ( !pItemNode )
                return;
        }

        OXFormsDescriptor desc;
        desc.szName = GetEntryText(pSelected);
        if(pItemNode->m_xNode.is()) {
            // a valid node interface tells us that we need to create a control from a binding
            desc.szServiceName = m_pXFormsPage->GetServiceNameForNode(pItemNode->m_xNode);
            desc.xPropSet = m_pXFormsPage->GetBindingForNode(pItemNode->m_xNode);
            DBG_ASSERT( desc.xPropSet.is(), "DataTreeListBox::StartDrag(): invalid node binding" );
        }
        else {
            desc.szServiceName = FM_COMPONENT_COMMANDBUTTON;
            desc.xPropSet = pItemNode->m_xPropSet;
        }
        rtl::Reference<OXFormsTransferable> pTransferable = new OXFormsTransferable(desc);
        EndSelection();
        pTransferable->StartDrag( this, DND_ACTION_COPY );
    }

    VclPtr<PopupMenu> DataTreeListBox::CreateContextMenu()
    {
        m_xMenu.disposeAndClear();
        m_xBuilder.reset(new VclBuilder(nullptr, VclBuilderContainer::getUIRootDir(), "svx/ui/formdatamenu.ui", ""));
        m_xMenu.set(m_xBuilder->get_menu("menu"));

        if ( DGTInstance == m_eGroup )
            m_xMenu->RemoveItem(m_xMenu->GetItemPos(m_xMenu->GetItemId("additem")));
        else
        {
            m_xMenu->RemoveItem(m_xMenu->GetItemPos(m_xMenu->GetItemId("addelement")));
            m_xMenu->RemoveItem(m_xMenu->GetItemPos(m_xMenu->GetItemId("addattribute")));

            if ( DGTSubmission == m_eGroup )
            {
                m_xMenu->SetItemText(m_xMenu->GetItemId("additem"), SvxResId(RID_STR_DATANAV_ADD_SUBMISSION));
                m_xMenu->SetItemText(m_xMenu->GetItemId("edit"), SvxResId(RID_STR_DATANAV_EDIT_SUBMISSION));
                m_xMenu->SetItemText(m_xMenu->GetItemId("delete"), SvxResId(RID_STR_DATANAV_REMOVE_SUBMISSION));
            }
            else
            {
                m_xMenu->SetItemText(m_xMenu->GetItemId("additem"), SvxResId(RID_STR_DATANAV_ADD_BINDING));
                m_xMenu->SetItemText(m_xMenu->GetItemId("edit"), SvxResId(RID_STR_DATANAV_EDIT_BINDING));
                m_xMenu->SetItemText(m_xMenu->GetItemId("delete"), SvxResId(RID_STR_DATANAV_REMOVE_BINDING));
            }
        }
        m_pXFormsPage->EnableMenuItems(m_xMenu.get());
        return m_xMenu;
    }

    void DataTreeListBox::ExecuteContextMenuAction( sal_uInt16 _nSelectedPopupEntry )
    {
        if (m_xMenu->GetItemId("additem") == _nSelectedPopupEntry)
            m_pXFormsPage->DoMenuAction(m_nAddId);
        else if (m_xMenu->GetItemId("addelement") == _nSelectedPopupEntry)
            m_pXFormsPage->DoMenuAction(m_nAddElementId);
        else if (m_xMenu->GetItemId("addattribute") == _nSelectedPopupEntry)
            m_pXFormsPage->DoMenuAction(m_nAddAttributeId);
        else if (m_xMenu->GetItemId("edit") == _nSelectedPopupEntry)
            m_pXFormsPage->DoMenuAction(m_nEditId);
        else if (m_xMenu->GetItemId("delete") == _nSelectedPopupEntry)
            m_pXFormsPage->DoMenuAction(m_nRemoveId);
    }

    void DataTreeListBox::RemoveEntry( SvTreeListEntry const * _pEntry )
    {
        if ( _pEntry )
        {
            delete static_cast< ItemNode* >( _pEntry->GetUserData() );
            SvTreeListBox::GetModel()->Remove( _pEntry );
        }
    }

    void DataTreeListBox::SetGroup(DataGroupType _eGroup)
    {
        m_eGroup = _eGroup;
    }

    void DataTreeListBox::SetXFormsPage(XFormsPage* _pPage)
    {
        m_pXFormsPage = _pPage;
    }

    void DataTreeListBox::SetToolBoxItemIds(sal_uInt16 _nAddId,
                           sal_uInt16 _nAddElementId,
                           sal_uInt16 _nAddAttributeId,
                           sal_uInt16 _nEditId,
                           sal_uInt16 _nRemoveId)
    {
        m_nAddId = _nAddId;
        m_nAddElementId = _nAddElementId;
        m_nAddAttributeId = _nAddAttributeId;
        m_nEditId = _nEditId;
        m_nRemoveId = _nRemoveId;
    }

    void DataTreeListBox::DeleteAndClear()
    {
        sal_uIntPtr i, nCount = GetEntryCount();
        for ( i = 0; i < nCount; ++i )
        {
            SvTreeListEntry* pEntry = GetEntry(i);
            if ( pEntry )
                delete static_cast< ItemNode* >( pEntry->GetUserData() );
        }

        Clear();
    }


    // class XFormsPage

    VCL_BUILDER_FACTORY_ARGS(DataTreeListBox, WB_BORDER)

    XFormsPage::XFormsPage( vcl::Window* pParent, DataNavigatorWindow* _pNaviWin, DataGroupType _eGroup ) :

        TabPage( pParent, "XFormsPage", "svx/ui/xformspage.ui" ),
        m_pNaviWin      ( _pNaviWin ),
        m_bHasModel     ( false ),
        m_eGroup        ( _eGroup ),
        m_bLinkOnce     ( false )

    {
        get(m_pToolBox, "toolbar");
        get(m_pItemList, "items");
        Size aSize(LogicToPixel(Size(63, 100), MapMode(MapUnit::MapAppFont)));
        m_pItemList->set_width_request(aSize.Width());
        m_pItemList->set_height_request(aSize.Height());

        m_pItemList->SetGroup(_eGroup);
        m_pItemList->SetXFormsPage( this );

        m_nAddId = m_pToolBox->GetItemId("TBI_ITEM_ADD");
        m_nAddElementId = m_pToolBox->GetItemId("TBI_ITEM_ADD_ELEMENT");
        m_nAddAttributeId = m_pToolBox->GetItemId("TBI_ITEM_ADD_ATTRIBUTE");
        m_nEditId = m_pToolBox->GetItemId("TBI_ITEM_EDIT");
        m_nRemoveId = m_pToolBox->GetItemId("TBI_ITEM_REMOVE");

        m_pItemList->SetToolBoxItemIds(m_nAddId, m_nAddElementId, m_nAddAttributeId, m_nEditId, m_nRemoveId);

        m_pToolBox->InsertSeparator(4,5);
        m_pToolBox->SetItemImage(m_nAddId, Image(StockImage::Yes, RID_SVXBMP_ADD));
        m_pToolBox->SetItemImage(m_nAddElementId, Image(StockImage::Yes, RID_SVXBMP_ADD_ELEMENT));
        m_pToolBox->SetItemImage(m_nAddAttributeId, Image(StockImage::Yes, RID_SVXBMP_ADD_ATTRIBUTE));
        m_pToolBox->SetItemImage(m_nEditId, Image(StockImage::Yes, RID_SVXBMP_EDIT));
        m_pToolBox->SetItemImage(m_nRemoveId, Image(StockImage::Yes, RID_SVXBMP_REMOVE));

        if ( DGTInstance == m_eGroup )
            m_pToolBox->RemoveItem( m_pToolBox->GetItemPos( m_nAddId ) );
        else
        {
            m_pToolBox->RemoveItem( m_pToolBox->GetItemPos( m_nAddElementId ) );
            m_pToolBox->RemoveItem( m_pToolBox->GetItemPos( m_nAddAttributeId ) );

            if ( DGTSubmission == m_eGroup )
            {
                m_pToolBox->SetItemText( m_nAddId, SvxResId( RID_STR_DATANAV_ADD_SUBMISSION ) );
                m_pToolBox->SetItemText( m_nEditId, SvxResId( RID_STR_DATANAV_EDIT_SUBMISSION ) );
                m_pToolBox->SetItemText( m_nRemoveId, SvxResId( RID_STR_DATANAV_REMOVE_SUBMISSION ) );
            }
            else
            {
                m_pToolBox->SetItemText( m_nAddId, SvxResId( RID_STR_DATANAV_ADD_BINDING ) );
                m_pToolBox->SetItemText( m_nEditId, SvxResId( RID_STR_DATANAV_EDIT_BINDING ) );
                m_pToolBox->SetItemText( m_nRemoveId, SvxResId( RID_STR_DATANAV_REMOVE_BINDING ) );
            }
        }

        const Size aTbxSz( m_pToolBox->CalcWindowSizePixel() );
        m_pToolBox->SetSizePixel( aTbxSz );
        m_pToolBox->SetOutStyle( SvtMiscOptions().GetToolboxStyle() );
        m_pToolBox->SetSelectHdl( LINK( this, XFormsPage, TbxSelectHdl ) );
        Point aPos = m_pItemList->GetPosPixel();
        aPos.setY( aTbxSz.Height() );
        m_pItemList->SetPosPixel( aPos );

        m_pItemList->SetSelectHdl( LINK( this, XFormsPage, ItemSelectHdl ) );
        m_pItemList->SetNodeDefaultImages();
        WinBits nBits = WB_BORDER | WB_TABSTOP | WB_HIDESELECTION | WB_NOINITIALSELECTION;
        if ( DGTInstance == m_eGroup || DGTSubmission == m_eGroup )
            nBits |= WB_HASBUTTONS | WB_HASLINES | WB_HASLINESATROOT | WB_HASBUTTONSATROOT;
        m_pItemList->SetStyle( m_pItemList->GetStyle() | nBits  );
        m_pItemList->Show();
        ItemSelectHdl( m_pItemList );
    }

    XFormsPage::~XFormsPage()
    {
        disposeOnce();
    }

    void XFormsPage::dispose()
    {
        m_pToolBox.clear();
        m_pItemList.clear();
        m_pNaviWin.clear();
        TabPage::dispose();
    }

    IMPL_LINK_NOARG(XFormsPage, TbxSelectHdl, ToolBox *, void)
    {
        DoToolBoxAction( m_pToolBox->GetCurItemId() );
    }

    IMPL_LINK_NOARG(XFormsPage, ItemSelectHdl, SvTreeListBox*, void)
    {
        EnableMenuItems( nullptr );
    }

    void XFormsPage::AddChildren(SvTreeListEntry* _pParent,
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
                    Image aExpImg, aCollImg;
                    switch ( eChildType )
                    {
                        case css::xml::dom::NodeType_ATTRIBUTE_NODE:
                            aExpImg = aCollImg = Image(StockImage::Yes, RID_SVXBMP_ATTRIBUTE);
                            break;
                        case css::xml::dom::NodeType_ELEMENT_NODE:
                            aExpImg = aCollImg = Image(StockImage::Yes, RID_SVXBMP_ELEMENT);
                            break;
                        case css::xml::dom::NodeType_TEXT_NODE:
                            aExpImg = aCollImg = Image(StockImage::Yes, RID_SVXBMP_TEXT);
                            break;
                        default:
                            aExpImg = aCollImg = Image(StockImage::Yes, RID_SVXBMP_OTHER);
                    }

                    OUString sName = m_xUIHelper->getNodeDisplayName( xChild, bShowDetails );
                    if ( !sName.isEmpty() )
                    {
                        ItemNode* pNode = new ItemNode( xChild );
                        SvTreeListEntry* pEntry = m_pItemList->InsertEntry(
                            sName, aExpImg, aCollImg, _pParent, false, TREELIST_APPEND, pNode );
                        if ( xChild->hasAttributes() )
                        {
                            Reference< css::xml::dom::XNamedNodeMap > xMap = xChild->getAttributes();
                            if ( xMap.is() )
                            {
                                aExpImg = aCollImg = Image(StockImage::Yes, RID_SVXBMP_ATTRIBUTE);
                                sal_Int32 j, nMapLen = xMap->getLength();
                                for ( j = 0; j < nMapLen; ++j )
                                {
                                    Reference< css::xml::dom::XNode > xAttr = xMap->item(j);
                                    pNode = new ItemNode( xAttr );
                                    OUString sAttrName =
                                        m_xUIHelper->getNodeDisplayName( xAttr, bShowDetails );
                                    m_pItemList->InsertEntry(
                                        sAttrName, aExpImg, aCollImg,
                                        pEntry, false, TREELIST_APPEND, pNode );
                                }
                            }
                        }
                        if ( xChild->hasChildNodes() )
                            AddChildren(pEntry, xChild);
                    }
                }
            }
        }
        catch( Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("svx");
        }
    }

    bool XFormsPage::DoToolBoxAction( sal_uInt16 _nToolBoxID ) {

        bool bHandled = false;
        bool bIsDocModified = false;
        m_pNaviWin->DisableNotify( true );

        if(_nToolBoxID == m_nAddId || _nToolBoxID == m_nAddElementId || _nToolBoxID == m_nAddAttributeId)
        {
            bHandled = true;
            Reference< css::xforms::XModel > xModel( m_xUIHelper, UNO_QUERY );
            DBG_ASSERT( xModel.is(), "XFormsPage::DoToolBoxAction(): Action without model" );
            if ( DGTSubmission == m_eGroup )
            {
                AddSubmissionDialog aDlg(GetFrameWeld(), nullptr, m_xUIHelper);
                if ( aDlg.run() == RET_OK && aDlg.GetNewSubmission().is() )
                {
                    try
                    {
                        Reference< css::xforms::XSubmission > xNewSubmission = aDlg.GetNewSubmission();
                        Reference< XSet > xSubmissions( xModel->getSubmissions(), UNO_QUERY );
                        xSubmissions->insert( makeAny( xNewSubmission ) );
                        Reference< XPropertySet > xNewPropSet( xNewSubmission, UNO_QUERY );
                        SvTreeListEntry* pEntry = AddEntry( xNewPropSet );
                        m_pItemList->Select( pEntry );
                        bIsDocModified = true;
                    }
                    catch ( Exception const & )
                    {
                        css::uno::Any ex( cppu::getCaughtException() );
                        SAL_WARN( "svx.form", "XFormsPage::DoToolBoxAction(): exception while adding submission " << exceptionToString(ex) );
                    }
                }
            }
            else
            {
                DataItemType eType = DITElement;
                SvTreeListEntry* pEntry = m_pItemList->FirstSelected();
                std::unique_ptr<ItemNode> pNode;
                Reference< css::xml::dom::XNode > xParentNode;
                Reference< XPropertySet > xNewBinding;
                const char* pResId = nullptr;
                bool bIsElement = true;
                if ( DGTInstance == m_eGroup )
                {
                    if ( !m_sInstanceURL.isEmpty() )
                    {
                        LinkedInstanceWarningBox aMsgBox(GetFrameWeld());
                        if (aMsgBox.run() != RET_OK)
                            return bHandled;
                    }

                    DBG_ASSERT( pEntry, "XFormsPage::DoToolBoxAction(): no entry" );
                    ItemNode* pParentNode = static_cast< ItemNode* >( pEntry->GetUserData() );
                    DBG_ASSERT( pParentNode, "XFormsPage::DoToolBoxAction(): no parent node" );
                    xParentNode = pParentNode->m_xNode;
                    Reference< css::xml::dom::XNode > xNewNode;
                    if ( m_nAddElementId == _nToolBoxID )
                    {
                        try
                        {
                            pResId = RID_STR_DATANAV_ADD_ELEMENT;
                            xNewNode = m_xUIHelper->createElement( xParentNode, NEW_ELEMENT );
                        }
                        catch ( Exception const & )
                        {
                            css::uno::Any ex( cppu::getCaughtException() );
                            SAL_WARN( "svx.form", "XFormsPage::DoToolBoxAction(): exception while create element " << exceptionToString(ex) );
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
                            css::uno::Any ex( cppu::getCaughtException() );
                            SAL_WARN( "svx.form", "XFormsPage::DoToolBoxAction(): exception while create attribute " << exceptionToString(ex) );
                        }
                    }

                    try
                    {
                        xNewNode = xParentNode->appendChild( xNewNode );
                    }
                    catch ( css::xml::dom::DOMException& e )
                    {
                        if ( e.Code == css::xml::dom::DOMExceptionType_DOMSTRING_SIZE_ERR )
                        {
                            css::uno::Any ex( cppu::getCaughtException() );
                            SAL_WARN( "svx.form", "XFormsPage::DoToolBoxAction(): domexception: size error " << exceptionToString(ex) );
                        }
                        SAL_WARN( "svx.form", "XFormsPage::DoToolBoxAction(): domexception while append child" );
                    }
                    catch ( Exception const & )
                    {
                        css::uno::Any ex( cppu::getCaughtException() );
                        SAL_WARN( "svx.form", "XFormsPage::DoToolBoxAction(): exception while append child " << exceptionToString(ex) );
                    }

                    try
                    {
                        Reference< css::xml::dom::XNode > xPNode;
                        if ( xNewNode.is() )
                             xPNode = xNewNode->getParentNode();
                        // attributes don't have parents in the DOM model
                        DBG_ASSERT( m_nAddAttributeId == _nToolBoxID
                                    || xPNode.is(), "XFormsPage::DoToolboxAction(): node not added" );
                    }
                    catch ( Exception const & )
                    {
                        css::uno::Any ex( cppu::getCaughtException() );
                        SAL_WARN( "svx.form", "XFormsPage::DoToolboxAction(): exception caught " << exceptionToString(ex) );
                    }

                    try
                    {
                        m_xUIHelper->getBindingForNode( xNewNode, true );
                    }
                    catch ( Exception const & )
                    {
                        css::uno::Any ex( cppu::getCaughtException() );
                        SAL_WARN( "svx.form", "XFormsPage::DoToolBoxAction(): exception while get binding for node " << exceptionToString(ex) );
                    }
                    pNode.reset(new ItemNode( xNewNode ));
                }
                else
                {
                    try
                    {
                        pResId = RID_STR_DATANAV_ADD_BINDING;
                        xNewBinding = xModel->createBinding();
                        Reference< XSet > xBindings( xModel->getBindings(), UNO_QUERY );
                        xBindings->insert( makeAny( xNewBinding ) );
                        pNode.reset(new ItemNode( xNewBinding ));
                        eType = DITBinding;
                    }
                    catch ( Exception const & )
                    {
                        css::uno::Any ex( cppu::getCaughtException() );
                        SAL_WARN( "svx.form", "XFormsPage::DoToolBoxAction(): exception while adding binding " << exceptionToString(ex) );
                    }
                }

                AddDataItemDialog aDlg(GetFrameWeld(), pNode.get(), m_xUIHelper);
                aDlg.set_title(SvxResId(pResId));
                aDlg.InitText( eType );
                short nReturn = aDlg.run();
                if (  DGTInstance == m_eGroup )
                {
                    if ( RET_OK == nReturn )
                    {
                        SvTreeListEntry* pNewEntry = AddEntry( std::move(pNode), bIsElement );
                        m_pItemList->MakeVisible( pNewEntry );
                        m_pItemList->Select( pNewEntry );
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
                            css::uno::Any ex( cppu::getCaughtException() );
                            SAL_WARN( "svx.form", "XFormsPage::DoToolboxAction(): exception caught " << exceptionToString(ex) );
                        }
                    }
                }
                else
                {
                    if ( RET_OK == nReturn )
                    {
                        SvTreeListEntry* pNewEntry = AddEntry( xNewBinding );
                        m_pItemList->Select( pNewEntry );
                        bIsDocModified = true;
                    }
                    else
                    {
                        try
                        {
                            Reference< XSet > xBindings( xModel->getBindings(), UNO_QUERY );
                            xBindings->remove( makeAny( xNewBinding ) );
                        }
                        catch ( Exception const & )
                        {
                            css::uno::Any ex( cppu::getCaughtException() );
                            SAL_WARN( "svx.form", "XFormsPage::DoToolboxAction(): exception caught " << exceptionToString(ex) );
                        }
                    }
                }
            }
        }
        else if(_nToolBoxID == m_nEditId)
        {
            bHandled = true;
            SvTreeListEntry* pEntry = m_pItemList->FirstSelected();
            if ( pEntry )
            {
                if ( DGTSubmission == m_eGroup && m_pItemList->GetParent( pEntry ) )
                    pEntry = m_pItemList->GetParent( pEntry );
                ItemNode* pNode = static_cast< ItemNode* >( pEntry->GetUserData() );
                if ( DGTInstance == m_eGroup || DGTBinding == m_eGroup )
                {
                    if ( DGTInstance == m_eGroup && !m_sInstanceURL.isEmpty() )
                    {
                        LinkedInstanceWarningBox aMsgBox(GetFrameWeld());
                        if (aMsgBox.run() != RET_OK)
                            return bHandled;
                    }

                    AddDataItemDialog aDlg(GetFrameWeld(), pNode, m_xUIHelper);
                    DataItemType eType = DITElement;
                    const char* pResId = RID_STR_DATANAV_EDIT_ELEMENT;
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
                            css::uno::Any ex( cppu::getCaughtException() );
                            SAL_WARN( "svx.form", "XFormsPage::DoToolboxAction(): exception caught " << exceptionToString(ex) );
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
                        if ( DGTInstance == m_eGroup )
                        {
                            try
                            {
                                sNewName = m_xUIHelper->getNodeDisplayName(
                                    pNode->m_xNode, m_pNaviWin->IsShowDetails() );
                            }
                            catch ( Exception const & )
                            {
                                css::uno::Any ex( cppu::getCaughtException() );
                                SAL_WARN( "svx.form", "XFormsPage::DoToolboxAction(): exception caught " << exceptionToString(ex) );
                            }
                        }
                        else if (pNode)
                        {
                            try
                            {
                                OUString sTemp;
                                pNode->m_xPropSet->getPropertyValue( PN_BINDING_ID ) >>= sTemp;
                                sNewName += sTemp;
                                sNewName += ": ";
                                pNode->m_xPropSet->getPropertyValue( PN_BINDING_EXPR ) >>= sTemp;
                                sNewName += sTemp;
                            }
                            catch ( Exception const & )
                            {
                                css::uno::Any ex( cppu::getCaughtException() );
                                SAL_WARN( "svx.form", "XFormsPage::DoToolboxAction(): exception caught " << exceptionToString(ex) );
                            }
                        }

                        m_pItemList->SetEntryText( pEntry, sNewName );
                        bIsDocModified = true;
                    }
                }
                else
                {
                    AddSubmissionDialog aDlg(GetFrameWeld(), pNode, m_xUIHelper);
                    aDlg.set_title(SvxResId(RID_STR_DATANAV_EDIT_SUBMISSION));
                    if (aDlg.run() == RET_OK)
                    {
                        EditEntry( pNode->m_xPropSet );
                        bIsDocModified = true;
                    }
                }
            }
        }
        else if(_nToolBoxID == m_nRemoveId)
        {
            bHandled = true;
            if ( DGTInstance == m_eGroup && !m_sInstanceURL.isEmpty() )
            {
                LinkedInstanceWarningBox aMsgBox(GetFrameWeld());
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
        EnableMenuItems( nullptr );
        if ( bIsDocModified )
            svxform::DataNavigatorWindow::SetDocModified();
        return bHandled;
    }

    SvTreeListEntry* XFormsPage::AddEntry( std::unique_ptr<ItemNode> _pNewNode, bool _bIsElement )
    {
        SvTreeListEntry* pParent = m_pItemList->FirstSelected();
        Image aImage(StockImage::Yes, _bIsElement ? OUString(RID_SVXBMP_ELEMENT) : OUString(RID_SVXBMP_ATTRIBUTE));
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
        return m_pItemList->InsertEntry(
            sName, aImage, aImage, pParent, false, TREELIST_APPEND, _pNewNode.release() );
    }

    SvTreeListEntry* XFormsPage::AddEntry( const Reference< XPropertySet >& _rEntry )
    {
        SvTreeListEntry* pEntry = nullptr;
        Image aImage(StockImage::Yes, RID_SVXBMP_ELEMENT);

        ItemNode* pNode = new ItemNode( _rEntry );
        OUString sTemp;

        if ( DGTSubmission == m_eGroup )
        {
            try
            {
                // ID
                _rEntry->getPropertyValue( PN_SUBMISSION_ID ) >>= sTemp;
                pEntry = m_pItemList->InsertEntry( sTemp, aImage, aImage, nullptr, false, TREELIST_APPEND, pNode );
                // Action
                _rEntry->getPropertyValue( PN_SUBMISSION_ACTION ) >>= sTemp;
                OUString sEntry = SvxResId( RID_STR_DATANAV_SUBM_ACTION );
                sEntry += sTemp;
                m_pItemList->InsertEntry( sEntry, aImage, aImage, pEntry );
                // Method
                _rEntry->getPropertyValue( PN_SUBMISSION_METHOD ) >>= sTemp;
                sEntry = SvxResId( RID_STR_DATANAV_SUBM_METHOD );
                sEntry +=  m_aMethodString.toUI( sTemp );
                m_pItemList->InsertEntry( sEntry, aImage, aImage, pEntry );
                // Ref
                _rEntry->getPropertyValue( PN_SUBMISSION_REF ) >>= sTemp;
                sEntry = SvxResId( RID_STR_DATANAV_SUBM_REF );
                sEntry += sTemp;
                m_pItemList->InsertEntry( sEntry, aImage, aImage, pEntry );
                // Bind
                _rEntry->getPropertyValue( PN_SUBMISSION_BIND ) >>= sTemp;
                sEntry = SvxResId( RID_STR_DATANAV_SUBM_BIND );
                sEntry += sTemp;
                m_pItemList->InsertEntry( sEntry, aImage, aImage, pEntry );
                // Replace
                _rEntry->getPropertyValue( PN_SUBMISSION_REPLACE ) >>= sTemp;
                sEntry = SvxResId( RID_STR_DATANAV_SUBM_REPLACE );
                sEntry += m_aReplaceString.toUI( sTemp );
                m_pItemList->InsertEntry( sEntry, aImage, aImage, pEntry );
            }
            catch ( Exception const & )
            {
                css::uno::Any ex( cppu::getCaughtException() );
                SAL_WARN( "svx.form", "XFormsPage::AddEntry(Ref): exception caught " << exceptionToString(ex) );
            }
        }
        else // then Binding Page
        {
            try
            {
                OUString sName;
                _rEntry->getPropertyValue( PN_BINDING_ID ) >>= sTemp;
                sName += sTemp;
                sName += ": ";
                _rEntry->getPropertyValue( PN_BINDING_EXPR ) >>= sTemp;
                sName += sTemp;
                pEntry = m_pItemList->InsertEntry(
                    sName, aImage, aImage, nullptr, false, TREELIST_APPEND, pNode );
            }
            catch ( Exception const & )
            {
                css::uno::Any ex( cppu::getCaughtException() );
                SAL_WARN( "svx.form", "XFormsPage::AddEntry(Ref): exception caught " << exceptionToString(ex) );
            }
        }

        return pEntry;
    }


    void XFormsPage::EditEntry( const Reference< XPropertySet >& _rEntry )
    {
        OUString sTemp;

        if ( DGTSubmission == m_eGroup )
        {
            try
            {
                SvTreeListEntry* pEntry = m_pItemList->FirstSelected();

                // #i36262# may be called for submission entry *or* for
                // submission children. If we don't have any children, we
                // assume the latter case and use the parent
                if( m_pItemList->GetEntry( pEntry, 0 ) == nullptr )
                {
                    pEntry = m_pItemList->GetModel()->GetParent( pEntry );
                }

                _rEntry->getPropertyValue( PN_SUBMISSION_ID ) >>= sTemp;
                m_pItemList->SetEntryText( pEntry, sTemp );

                _rEntry->getPropertyValue( PN_SUBMISSION_BIND ) >>= sTemp;
                OUString sEntry = SvxResId( RID_STR_DATANAV_SUBM_BIND );
                sEntry += sTemp;
                sal_uIntPtr nPos = 0;
                SvTreeListEntry* pChild = m_pItemList->GetEntry( pEntry, nPos++ );
                m_pItemList->SetEntryText( pChild, sEntry );
                _rEntry->getPropertyValue( PN_SUBMISSION_REF ) >>= sTemp;
                sEntry = SvxResId( RID_STR_DATANAV_SUBM_REF );
                sEntry += sTemp;
                pChild = m_pItemList->GetEntry( pEntry, nPos++ );
                m_pItemList->SetEntryText( pChild, sEntry );
                _rEntry->getPropertyValue( PN_SUBMISSION_ACTION ) >>= sTemp;
                sEntry = SvxResId( RID_STR_DATANAV_SUBM_ACTION );
                sEntry += sTemp;
                pChild = m_pItemList->GetEntry( pEntry, nPos++ );
                m_pItemList->SetEntryText( pChild, sEntry );
                _rEntry->getPropertyValue( PN_SUBMISSION_METHOD ) >>= sTemp;
                sEntry = SvxResId( RID_STR_DATANAV_SUBM_METHOD );
                sEntry += m_aMethodString.toUI( sTemp );
                pChild = m_pItemList->GetEntry( pEntry, nPos++ );
                m_pItemList->SetEntryText( pChild, sEntry );
                _rEntry->getPropertyValue( PN_SUBMISSION_REPLACE ) >>= sTemp;
                sEntry = SvxResId( RID_STR_DATANAV_SUBM_REPLACE );
                sEntry += m_aReplaceString.toUI( sTemp );
                pChild = m_pItemList->GetEntry( pEntry, nPos++ );
                m_pItemList->SetEntryText( pChild, sEntry );
            }
            catch ( Exception const & )
            {
                css::uno::Any ex( cppu::getCaughtException() );
                SAL_WARN( "svx.form", "XFormsPage::EditEntry(): exception caught " << exceptionToString(ex) );
            }
        }
    }


    bool XFormsPage::RemoveEntry()
    {
        bool bRet = false;
        SvTreeListEntry* pEntry = m_pItemList->FirstSelected();
        if ( pEntry &&
             ( DGTInstance != m_eGroup || m_pItemList->GetParent( pEntry ) ) )
        {
            Reference< css::xforms::XModel > xModel( m_xUIHelper, UNO_QUERY );
            DBG_ASSERT( xModel.is(), "XFormsPage::RemoveEntry(): no model" );
            ItemNode* pNode = static_cast< ItemNode* >( pEntry->GetUserData() );
            DBG_ASSERT( pNode, "XFormsPage::RemoveEntry(): no node" );

            if ( DGTInstance == m_eGroup )
            {
                try
                {
                    DBG_ASSERT( pNode->m_xNode.is(), "XFormsPage::RemoveEntry(): no XNode" );
                    css::xml::dom::NodeType eChildType = pNode->m_xNode->getNodeType();
                    bool bIsElement = ( eChildType == css::xml::dom::NodeType_ELEMENT_NODE );
                    const char* pResId = bIsElement ? RID_STR_QRY_REMOVE_ELEMENT : RID_STR_QRY_REMOVE_ATTRIBUTE;
                    OUString sVar = bIsElement ? OUString(ELEMENTNAME) : OUString(ATTRIBUTENAME);
                    std::unique_ptr<weld::MessageDialog> xQBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                                             VclMessageType::Question, VclButtonsType::YesNo,
                                                                             SvxResId(pResId)));
                    OUString sMessText = xQBox->get_primary_text();
                    sMessText = sMessText.replaceFirst(
                        sVar, m_xUIHelper->getNodeDisplayName( pNode->m_xNode, false ) );
                    xQBox->set_primary_text(sMessText);
                    if (xQBox->run() == RET_YES)
                    {
                        SvTreeListEntry* pParent = m_pItemList->GetParent( pEntry );
                        DBG_ASSERT( pParent, "XFormsPage::RemoveEntry(): no parent entry" );
                        ItemNode* pParentNode = static_cast< ItemNode* >( pParent->GetUserData() );
                        DBG_ASSERT( pParentNode && pParentNode->m_xNode.is(), "XFormsPage::RemoveEntry(): no parent XNode" );

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
                    css::uno::Any ex( cppu::getCaughtException() );
                    SAL_WARN( "svx.form", "XFormsPage::RemoveEntry(): exception caught " << exceptionToString(ex) );
                }
            }
            else
            {
                DBG_ASSERT( pNode->m_xPropSet.is(), "XFormsPage::RemoveEntry(): no propset" );
                bool bSubmission = ( DGTSubmission == m_eGroup );
                const char* pResId = bSubmission ? RID_STR_QRY_REMOVE_SUBMISSION : RID_STR_QRY_REMOVE_BINDING;
                OUString sProperty = bSubmission ? OUString(PN_SUBMISSION_ID) : OUString(PN_BINDING_ID);
                OUString sSearch = bSubmission ? OUString(SUBMISSIONNAME) : OUString(BINDINGNAME);
                OUString sName;
                try
                {
                    pNode->m_xPropSet->getPropertyValue( sProperty ) >>= sName;
                }
                catch ( Exception const & )
                {
                    css::uno::Any ex( cppu::getCaughtException() );
                    SAL_WARN( "svx.form", "XFormsPage::RemoveEntry(): exception caught " << exceptionToString(ex) );
                }
                std::unique_ptr<weld::MessageDialog> xQBox(Application::CreateMessageDialog(GetFrameWeld(),
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
                            xModel->getSubmissions()->remove( makeAny( pNode->m_xPropSet ) );
                        else // then Binding Page
                            xModel->getBindings()->remove( makeAny( pNode->m_xPropSet ) );
                        bRet = true;
                    }
                    catch ( Exception const & )
                    {
                        css::uno::Any ex( cppu::getCaughtException() );
                        SAL_WARN( "svx.form", "XFormsPage::RemoveEntry(): exception caught " << exceptionToString(ex) );
                    }
                }
            }

            if ( bRet )
                m_pItemList->RemoveEntry( pEntry );
        }

        return bRet;
    }


    bool XFormsPage::EventNotify( NotifyEvent& rNEvt )
    {
        bool bHandled = false;

        if ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
        {
            sal_uInt16 nCode = rNEvt.GetKeyEvent()->GetKeyCode().GetCode();

            switch ( nCode )
            {
                case KEY_DELETE:
                    bHandled = DoMenuAction( m_nRemoveId );
                    break;
            }
        }

        return bHandled || Window::EventNotify( rNEvt );
    }

    void XFormsPage::Resize()
    {
        Size aSize = GetOutputSizePixel();
        Size aTbxSize = m_pToolBox->GetSizePixel();
        aTbxSize.setWidth( aSize.Width() );
        m_pToolBox->SetSizePixel( aTbxSize );
        aSize.AdjustWidth( -4 );
        aSize.AdjustHeight( -( 4 + aTbxSize.Height() ) );
        m_pItemList->SetPosSizePixel( Point( 2, 2 + aTbxSize.Height() ), aSize );
    }

    OUString XFormsPage::SetModel( const Reference< css::xforms::XModel >& _xModel, sal_uInt16 _nPagePos )
    {
        DBG_ASSERT( _xModel.is(), "XFormsPage::SetModel(): invalid model" );

        m_xUIHelper.set( _xModel, UNO_QUERY );
        OUString sRet;
        m_bHasModel = true;

        switch ( m_eGroup )
        {
            case DGTInstance :
            {
                DBG_ASSERT( _nPagePos != TAB_PAGE_NOTFOUND, "XFormsPage::SetModel(): invalid page position" );
                try
                {
                    Reference< XContainer > xContainer( _xModel->getInstances(), UNO_QUERY );
                    if ( xContainer.is() )
                        m_pNaviWin->AddContainerBroadcaster( xContainer );

                    Reference< XEnumerationAccess > xNumAccess( _xModel->getInstances(), UNO_QUERY );
                    if ( xNumAccess.is() )
                    {
                        Reference < XEnumeration > xNum = xNumAccess->createEnumeration();
                        if ( xNum.is() && xNum->hasMoreElements() )
                        {
                            sal_uInt16 nIter = 0;
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
                    css::uno::Any ex( cppu::getCaughtException() );
                    SAL_WARN( "svx.form", "XFormsPage::SetModel(): exception caught " << exceptionToString(ex) );
                }
                break;
            }

            case DGTSubmission :
            {
                DBG_ASSERT( TAB_PAGE_NOTFOUND == _nPagePos, "XFormsPage::SetModel(): invalid page position" );
                try
                {
                    Reference< XContainer > xContainer( _xModel->getSubmissions(), UNO_QUERY );
                    if ( xContainer.is() )
                        m_pNaviWin->AddContainerBroadcaster( xContainer );

                    Reference< XEnumerationAccess > xNumAccess( _xModel->getSubmissions(), UNO_QUERY );
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
                    css::uno::Any ex( cppu::getCaughtException() );
                    SAL_WARN( "svx.form", "XFormsPage::SetModel(): exception caught " << exceptionToString(ex) );
                }
                break;
            }

            case DGTBinding :
            {
                DBG_ASSERT( TAB_PAGE_NOTFOUND == _nPagePos, "XFormsPage::SetModel(): invalid page position" );
                try
                {
                    Reference< XContainer > xContainer( _xModel->getBindings(), UNO_QUERY );
                    if ( xContainer.is() )
                        m_pNaviWin->AddContainerBroadcaster( xContainer );

                    Reference< XEnumerationAccess > xNumAccess( _xModel->getBindings(), UNO_QUERY );
                    if ( xNumAccess.is() )
                    {
                        Reference < XEnumeration > xNum = xNumAccess->createEnumeration();
                        if ( xNum.is() && xNum->hasMoreElements() )
                        {
                            Image aImage(StockImage::Yes, RID_SVXBMP_ELEMENT);
                            while ( xNum->hasMoreElements() )
                            {
                                Reference< XPropertySet > xPropSet;
                                Any aAny = xNum->nextElement();
                                if ( aAny >>= xPropSet )
                                {
                                    OUString sEntry;
                                    OUString sTemp;
                                    xPropSet->getPropertyValue( PN_BINDING_ID ) >>= sTemp;
                                    sEntry += sTemp;
                                    sEntry += ": ";
                                    xPropSet->getPropertyValue( PN_BINDING_EXPR ) >>= sTemp;
                                    sEntry += sTemp;

                                    ItemNode* pNode = new ItemNode( xPropSet );
                                    m_pItemList->InsertEntry(
                                        sEntry, aImage, aImage, nullptr, false, TREELIST_APPEND, pNode );
                                }
                            }
                        }
                    }
                }
                catch( Exception const & )
                {
                    css::uno::Any ex( cppu::getCaughtException() );
                    SAL_WARN( "svx.form", "XFormsPage::SetModel(): exception caught " << exceptionToString(ex) );
                }
                break;
            }
            default:
                OSL_FAIL( "XFormsPage::SetModel: unknown group!" );
                break;
        }

        EnableMenuItems( nullptr );

        return sRet;
    }

    void XFormsPage::ClearModel()
    {
        m_bHasModel = false;
        m_pItemList->DeleteAndClear();
    }

    OUString XFormsPage::LoadInstance(const Sequence< PropertyValue >& _xPropSeq)
    {
        OUString sRet;
        OUString sTemp;
        OUString sInstModel = PN_INSTANCE_MODEL;
        OUString sInstName = PN_INSTANCE_ID;
        OUString sInstURL = PN_INSTANCE_URL;
        const PropertyValue* pProps = _xPropSeq.getConstArray();
        const PropertyValue* pPropsEnd = pProps + _xPropSeq.getLength();
        for ( ; pProps != pPropsEnd; ++pProps )
        {
            if ( sInstModel == pProps->Name )
            {
                Reference< css::xml::dom::XNode > xRoot;
                if ( pProps->Value >>= xRoot )
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
                        css::uno::Any ex( cppu::getCaughtException() );
                        SAL_WARN( "svx.form", "XFormsPage::LoadInstance(): exception caught " << exceptionToString(ex) );
                    }
                }
            }
            else if ( sInstName == pProps->Name && ( pProps->Value >>= sTemp ) )
                m_sInstanceName = sRet = sTemp;
            else if ( sInstURL == pProps->Name && ( pProps->Value >>= sTemp ) )
                m_sInstanceURL = sTemp;
        }

        return sRet;
    }


    bool XFormsPage::DoMenuAction( sal_uInt16 _nMenuID )
    {
        return DoToolBoxAction( _nMenuID );
    }


    void XFormsPage::EnableMenuItems( Menu* _pMenu )
    {
        bool bEnableAdd = false;
        bool bEnableEdit = false;
        bool bEnableRemove = false;

        SvTreeListEntry* pEntry = m_pItemList->FirstSelected();
        if ( pEntry )
        {
            bEnableAdd = true;
            bool bSubmitChild = false;
            if ( DGTSubmission == m_eGroup && m_pItemList->GetParent( pEntry ) )
            {
                pEntry = m_pItemList->GetParent( pEntry );
                bSubmitChild = true;
            }
            ItemNode* pNode = static_cast< ItemNode* >( pEntry->GetUserData() );
            if ( pNode && ( pNode->m_xNode.is() || pNode->m_xPropSet.is() ) )
            {
                bEnableEdit = true;
                bEnableRemove = !bSubmitChild;
                if ( DGTInstance == m_eGroup && !m_pItemList->GetParent( pEntry ) )
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
                       css::uno::Any ex( cppu::getCaughtException() );
                       SAL_WARN( "svx.form", "XFormsPage::EnableMenuItems(): exception caught " << exceptionToString(ex) );
                    }
                }
            }
        }
        else if ( m_eGroup != DGTInstance )
            bEnableAdd = true;

        m_pToolBox->EnableItem( m_nAddId, bEnableAdd );
        m_pToolBox->EnableItem( m_nAddElementId, bEnableAdd );
        m_pToolBox->EnableItem( m_nAddAttributeId, bEnableAdd );
        m_pToolBox->EnableItem( m_nEditId, bEnableEdit );
        m_pToolBox->EnableItem( m_nRemoveId, bEnableRemove );

        if ( _pMenu )
        {
            _pMenu->EnableItem(_pMenu->GetItemId("additem"), bEnableAdd);
            _pMenu->EnableItem(_pMenu->GetItemId("addelement"), bEnableAdd);
            _pMenu->EnableItem(_pMenu->GetItemId("addattribute"), bEnableAdd);
            _pMenu->EnableItem(_pMenu->GetItemId("edit"), bEnableEdit);
            _pMenu->EnableItem(_pMenu->GetItemId("delete"), bEnableRemove);
        }
        if ( DGTInstance == m_eGroup )
        {
            const char* pResId1 = RID_STR_DATANAV_EDIT_ELEMENT;
            const char* pResId2 = RID_STR_DATANAV_REMOVE_ELEMENT;
            if ( pEntry )
            {
                ItemNode* pNode = static_cast< ItemNode* >( pEntry->GetUserData() );
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
                       css::uno::Any ex( cppu::getCaughtException() );
                       SAL_WARN( "svx.form", "XFormsPage::EnableMenuItems(): exception caught " << exceptionToString(ex) );
                    }
                }
            }
            m_pToolBox->SetItemText( m_nEditId, SvxResId( pResId1 ) );
            m_pToolBox->SetItemText( m_nRemoveId, SvxResId( pResId2 ) );
            if ( _pMenu )
            {
                _pMenu->SetItemText(_pMenu->GetItemId("edit"), SvxResId( pResId1 ) );
                _pMenu->SetItemText(_pMenu->GetItemId("delete"), SvxResId( pResId2 ) );
            }
        }
    }

    DataNavigatorWindow::DataNavigatorWindow(vcl::Window* pParent, SfxBindings const * pBindings)
        : Window(pParent)
        , m_pInstPage(nullptr)
        , m_pSubmissionPage(nullptr)
        , m_pBindingPage(nullptr)
        , m_nLastSelectedPos(LISTBOX_ENTRY_NOTFOUND)
        , m_bShowDetails(false)
        , m_bIsNotifyDisabled(false)
        , m_xDataListener(new DataListener(this))
    {
        m_pUIBuilder.reset(new VclBuilder(this, getUIRootDir(), "svx/ui/datanavigator.ui", "DataNavigator"));
        get(m_pModelsBox, "modelslist");
        get(m_pModelBtn, "modelsbutton");
        get(m_pTabCtrl, "tabcontrol");
        get(m_pInstanceBtn, "instances");

        // handler
        m_pModelsBox->SetSelectHdl( LINK( this, DataNavigatorWindow, ModelSelectListBoxHdl ) );
        Link<MenuButton *, void> aLink1 = LINK( this, DataNavigatorWindow, MenuSelectHdl );
        m_pModelBtn->SetSelectHdl( aLink1 );
        m_pInstanceBtn->SetSelectHdl( aLink1 );
        Link<MenuButton*,void> aLink2 = LINK( this, DataNavigatorWindow, MenuActivateHdl );
        m_pModelBtn->SetActivateHdl( aLink2 );
        m_pInstanceBtn->SetActivateHdl( aLink2 );
        m_pTabCtrl->SetActivatePageHdl( LINK( this, DataNavigatorWindow, ActivatePageHdl ) );
        m_aUpdateTimer.SetTimeout( 2000 );
        m_aUpdateTimer.SetInvokeHandler( LINK( this, DataNavigatorWindow, UpdateHdl ) );

        // init tabcontrol
        m_pTabCtrl->Show();
        OString sPageId("instance");
        SvtViewOptions aViewOpt( EViewType::TabDialog, CFGNAME_DATANAVIGATOR );
        if ( aViewOpt.Exists() )
        {
            sPageId = aViewOpt.GetPageID();
            aViewOpt.GetUserItem(CFGNAME_SHOWDETAILS) >>= m_bShowDetails;
        }

        Menu* pMenu = m_pInstanceBtn->GetPopupMenu();
        sal_uInt16 nInstancesDetailsId = pMenu->GetItemId("instancesdetails");
        pMenu->SetItemBits(nInstancesDetailsId, MenuItemBits::CHECKABLE );
        pMenu->CheckItem(nInstancesDetailsId, m_bShowDetails );

        m_pTabCtrl->SetCurPageId(m_pTabCtrl->GetPageId(sPageId));
        ActivatePageHdl(m_pTabCtrl);

        // get our frame
        DBG_ASSERT( pBindings != nullptr,
                    "DataNavigatorWindow::LoadModels(): no SfxBindings; can't get frame" );
        m_xFrame.set( pBindings->GetDispatcher()->GetFrame()->GetFrame().GetFrameInterface(),
                      UNO_QUERY );
        DBG_ASSERT( m_xFrame.is(), "DataNavigatorWindow::LoadModels(): no frame" );
        // add frameaction listener
        Reference< XFrameActionListener > xListener(
            static_cast< XFrameActionListener* >( m_xDataListener.get() ), UNO_QUERY );
        m_xFrame->addFrameActionListener( xListener );

        // load xforms models of the current document
        LoadModels();
    }

    DataNavigatorWindow::~DataNavigatorWindow()
    {
        disposeOnce();
    }

    void DataNavigatorWindow::dispose()
    {
        SvtViewOptions aViewOpt( EViewType::TabDialog, CFGNAME_DATANAVIGATOR );
        aViewOpt.SetPageID(m_pTabCtrl->GetPageName(m_pTabCtrl->GetCurPageId()));
        aViewOpt.SetUserItem(CFGNAME_SHOWDETAILS, Any(m_bShowDetails));

        m_pInstPage.disposeAndClear();
        m_pSubmissionPage.disposeAndClear();
        m_pBindingPage.disposeAndClear();

        sal_Int32 i, nCount = m_aPageList.size();
        for ( i = 0; i < nCount; ++i )
            m_aPageList[i].disposeAndClear();
        m_aPageList.clear();

        Reference< XFrameActionListener > xListener(
            static_cast< XFrameActionListener* >( m_xDataListener.get() ), UNO_QUERY );
        m_xFrame->removeFrameActionListener( xListener );
        RemoveBroadcaster();
        m_xDataListener.clear();
        disposeBuilder();
        m_pModelsBox.clear();
        m_pModelBtn.clear();
        m_pTabCtrl.clear();
        m_pInstanceBtn.clear();
        vcl::Window::dispose();
    }


    IMPL_LINK( DataNavigatorWindow, ModelSelectListBoxHdl, ListBox&, rBox, void )
    {
        ModelSelectHdl(&rBox);
    }
    void DataNavigatorWindow::ModelSelectHdl(ListBox const * pBox)
    {
        sal_Int32 nPos = m_pModelsBox->GetSelectedEntryPos();
        // pBox == NULL, if you want to force a new fill.
        if ( nPos != m_nLastSelectedPos || !pBox )
        {
            m_nLastSelectedPos = nPos;
            ClearAllPageModels( pBox != nullptr );
            InitPages();
            SetPageModel();
        }
    }

    IMPL_LINK( DataNavigatorWindow, MenuSelectHdl, MenuButton *, pBtn, void )
    {
        bool bIsDocModified = false;
        Reference< css::xforms::XFormsUIHelper1 > xUIHelper;
        sal_Int32 nSelectedPos = m_pModelsBox->GetSelectedEntryPos();
        OUString sSelectedModel( m_pModelsBox->GetEntry( nSelectedPos ) );
        Reference< css::xforms::XModel > xModel;
        try
        {
            Any aAny = m_xDataContainer->getByName( sSelectedModel );
            if ( aAny >>= xModel )
                xUIHelper.set( xModel, UNO_QUERY );
        }
        catch ( Exception const & )
        {
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN( "svx.form", "DataNavigatorWindow::MenuSelectHdl(): exception caught " << exceptionToString(ex) );
        }
        DBG_ASSERT( xUIHelper.is(), "DataNavigatorWindow::MenuSelectHdl(): no UIHelper" );

        m_bIsNotifyDisabled = true;

        if (m_pModelBtn == pBtn)
        {
            OString sIdent(pBtn->GetCurItemIdent());
            if (sIdent == "modelsadd")
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

                        if ( m_pModelsBox->GetEntryPos( sNewName ) != LISTBOX_ENTRY_NOTFOUND )
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
                                xModelProps->setPropertyValue("ExternalData", makeAny( !bDocumentData ) );

                                sal_Int32 nNewPos = m_pModelsBox->InsertEntry( sNewName );
                                m_pModelsBox->SelectEntryPos( nNewPos );
                                ModelSelectHdl(m_pModelsBox);
                                bIsDocModified = true;
                            }
                            catch ( Exception const & )
                            {
                                css::uno::Any ex( cppu::getCaughtException() );
                                SAL_WARN( "svx.form", "DataNavigatorWindow::MenuSelectHdl(): exception caught " << exceptionToString(ex) );
                            }
                        }
                    }
                }
            }
            else if (sIdent == "modelsedit")
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
                    OSL_VERIFY( xModelProps->getPropertyValue( "ExternalData" ) >>= bExternalData );
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
                            xModelProps->setPropertyValue( "ExternalData", makeAny( !bDocumentData ) );
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

                            m_pModelsBox->RemoveEntry( nSelectedPos );
                            nSelectedPos = m_pModelsBox->InsertEntry( sNewName );
                            m_pModelsBox->SelectEntryPos( nSelectedPos );
                            bIsDocModified = true;
                        }
                        catch ( Exception const & )
                        {
                            css::uno::Any ex( cppu::getCaughtException() );
                            SAL_WARN( "svx.form", "DataNavigatorWindow::MenuSelectHdl(): exception caught " << exceptionToString(ex) );
                        }
                    }
                }
            }
            else if (sIdent == "modelsremove")
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
                        css::uno::Any ex( cppu::getCaughtException() );
                        SAL_WARN( "svx.form", "DataNavigatorWindow::MenuSelectHdl(): exception caught " << exceptionToString(ex) );
                    }
                    m_pModelsBox->RemoveEntry( nSelectedPos );
                    if ( m_pModelsBox->GetEntryCount() <= nSelectedPos )
                        nSelectedPos = m_pModelsBox->GetEntryCount() - 1;
                    m_pModelsBox->SelectEntryPos( nSelectedPos );
                    ModelSelectHdl(m_pModelsBox);
                    bIsDocModified = true;
                }
            }
            else
            {
                SAL_WARN( "svx.form", "DataNavigatorWindow::MenuSelectHdl(): wrong menu item" );
            }
        }
        else if (m_pInstanceBtn == pBtn)
        {
            OString sIdent(pBtn->GetCurItemIdent());
            if (sIdent == "instancesadd")
            {
                AddInstanceDialog aDlg(GetFrameWeld(), false);
                if (aDlg.run() == RET_OK)
                {
                    sal_uInt16 nInst = GetNewPageId();
                    OUString sName = aDlg.GetName();
                    OUString sURL = aDlg.GetURL();
                    bool bLinkOnce = aDlg.IsLinkInstance();
                    try
                    {
                        xUIHelper->newInstance( sName, sURL, !bLinkOnce );
                    }
                    catch ( Exception const & )
                    {
                        css::uno::Any ex( cppu::getCaughtException() );
                        SAL_WARN( "svx.form", "DataNavigatorWindow::MenuSelectHdl(): exception caught " << exceptionToString(ex) );
                    }
                    ModelSelectHdl( nullptr );
                    m_pTabCtrl->SetCurPageId( nInst );
                    XFormsPage* pPage = GetCurrentPage( nInst );
                    pPage->SetInstanceName(sName);
                    pPage->SetInstanceURL(sURL);
                    pPage->SetLinkOnce(bLinkOnce);
                    ActivatePageHdl(m_pTabCtrl);
                    bIsDocModified = true;
                }
            }
            else if (sIdent == "instancesedit")
            {
                sal_uInt16 nId = 0;
                XFormsPage* pPage = GetCurrentPage( nId );
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
                            css::uno::Any ex( cppu::getCaughtException() );
                            SAL_WARN( "svx.form", "DataNavigatorWindow::MenuSelectHdl(): exception caught " << exceptionToString(ex) );
                        }
                        pPage->SetInstanceName(sNewName);
                        pPage->SetInstanceURL(sURL);
                        pPage->SetLinkOnce(bLinkOnce);
                        m_pTabCtrl->SetPageText( nId, sNewName );
                        bIsDocModified = true;
                    }
                }
            }
            else if (sIdent == "instancesremove")
            {
                sal_uInt16 nId = 0;
                VclPtr<XFormsPage> pPage = GetCurrentPage( nId );
                if ( pPage )
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
                        if (IsAdditionalPage(nId))
                        {
                            auto aPageListEnd = m_aPageList.end();
                            auto aFoundPage = std::find( m_aPageList.begin(), aPageListEnd, pPage );
                            if ( aFoundPage != aPageListEnd )
                            {
                                m_aPageList.erase( aFoundPage );
                                pPage.disposeAndClear() ;
                                bDoRemove = true;
                            }
                        }
                        else
                        {
                            m_pInstPage.disposeAndClear();
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
                                css::uno::Any ex( cppu::getCaughtException() );
                                SAL_WARN( "svx.form", "DataNavigatorWindow::MenuSelectHdl(): exception caught " << exceptionToString(ex) );
                            }
                            m_pTabCtrl->RemovePage( nId );
                            m_pTabCtrl->SetCurPageId(m_pTabCtrl->GetPageId("instance"));
                            ModelSelectHdl( nullptr );
                            bIsDocModified = true;
                        }
                    }
                }
            }
            else if (sIdent == "instancesdetails")
            {
                m_bShowDetails = !m_bShowDetails;
                PopupMenu* pMenu = m_pInstanceBtn->GetPopupMenu();
                pMenu->CheckItem("instancesdetails", m_bShowDetails );
                ModelSelectHdl(m_pModelsBox);
            }
            else
            {
                SAL_WARN( "svx.form", "DataNavigatorWindow::MenuSelectHdl(): wrong menu item" );
            }
        }
        else
        {
            SAL_WARN( "svx.form", "DataNavigatorWindow::MenuSelectHdl(): wrong button" );
        }

        m_bIsNotifyDisabled = false;

        if ( bIsDocModified )
            SetDocModified();
    }

    bool DataNavigatorWindow::IsAdditionalPage(sal_uInt16 nId) const
    {
        return m_pTabCtrl->GetPageName(nId).isEmpty();
    }

    IMPL_LINK( DataNavigatorWindow, MenuActivateHdl, MenuButton *, pBtn, void )
    {
        Menu* pMenu = pBtn->GetPopupMenu();

        if (m_pInstanceBtn == pBtn)
        {
            sal_uInt16 nId(m_pTabCtrl->GetCurPageId());
            bool bIsInstPage = (IsAdditionalPage(nId) || m_pTabCtrl->GetPageName(nId) == "instance");
            pMenu->EnableItem( "instancesedit", bIsInstPage );
            pMenu->EnableItem( "instancesremove",
                bIsInstPage && m_pTabCtrl->GetPageCount() > MIN_PAGE_COUNT );
            pMenu->EnableItem( "instancesdetails", bIsInstPage );
        }
        else if (m_pModelBtn == pBtn)
        {
            // we need at least one model!
            pMenu->EnableItem("modelsremove", m_pModelsBox->GetEntryCount() > 1 );
        }
        else
        {
            SAL_WARN( "svx.form", "DataNavigatorWindow::MenuActivateHdl(): wrong button" );
        }
    }

    IMPL_LINK_NOARG(DataNavigatorWindow, ActivatePageHdl, TabControl*, void)
    {
        sal_uInt16 nId = 0;
        XFormsPage* pPage = GetCurrentPage( nId );
        if ( pPage )
        {
            m_pTabCtrl->SetTabPage( nId, pPage );
            if ( m_xDataContainer.is() && !pPage->HasModel() )
                SetPageModel();
        }
    }

    IMPL_LINK_NOARG(DataNavigatorWindow, UpdateHdl, Timer *, void)
    {
        ModelSelectHdl( nullptr );
    }

    XFormsPage* DataNavigatorWindow::GetCurrentPage( sal_uInt16& rCurId )
    {
        rCurId = m_pTabCtrl->GetCurPageId();
        VclPtr<XFormsPage> pPage;
        OString sName(m_pTabCtrl->GetPageName(rCurId));
        if (sName == "submissions")
        {
            if ( !m_pSubmissionPage )
                m_pSubmissionPage = VclPtr<XFormsPage>::Create(m_pTabCtrl, this, DGTSubmission);
            pPage = m_pSubmissionPage;
        }
        else if (sName == "bindings")
        {
            if ( !m_pBindingPage )
                m_pBindingPage = VclPtr<XFormsPage>::Create(m_pTabCtrl, this, DGTBinding);
            pPage = m_pBindingPage;
        }
        else if (sName == "instance")
        {
            if ( !m_pInstPage )
                m_pInstPage = VclPtr<XFormsPage>::Create(m_pTabCtrl, this, DGTInstance);
            pPage = m_pInstPage;
        }
        else
        {
            sal_uInt16 nPos = m_pTabCtrl->GetPagePos( rCurId );
            if ( HasFirstInstancePage() && nPos > 0 )
                nPos--;
            if ( m_aPageList.size() > nPos )
                pPage = m_aPageList[nPos];
            else
            {
                pPage = VclPtr<XFormsPage>::Create(m_pTabCtrl, this, DGTInstance);
                m_aPageList.push_back( pPage );
            }
        }

        return pPage;
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
                    css::uno::Any ex( cppu::getCaughtException() );
                    SAL_WARN( "svx.form", "DataNavigatorWindow::LoadModels(): exception caught " << exceptionToString(ex) );
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
                        Sequence< OUString > aNameList = m_xDataContainer->getElementNames();
                        sal_Int32 i, nCount = aNameList.getLength();
                        OUString* pNames = aNameList.getArray();
                        for ( i = 0; i < nCount; ++i )
                        {
                            Any aAny = m_xDataContainer->getByName( pNames[i] );
                            Reference< css::xforms::XModel > xFormsModel;
                            if ( aAny >>= xFormsModel )
                                m_pModelsBox->InsertEntry( xFormsModel->getID() );
                        }
                    }
                }
            }
            catch( Exception const & )
            {
                css::uno::Any ex( cppu::getCaughtException() );
                SAL_WARN( "svx.form", "DataNavigatorWindow::LoadModels(): exception caught " << exceptionToString(ex) );
            }
        }

        if ( m_pModelsBox->GetEntryCount() > 0 )
        {
            m_pModelsBox->SelectEntryPos(0);
            ModelSelectHdl(m_pModelsBox);
        }
    }

    void DataNavigatorWindow::SetPageModel()
    {
        OUString sModel( m_pModelsBox->GetSelectedEntry() );
        try
        {
            Any aAny = m_xDataContainer->getByName( sModel );
            Reference< css::xforms::XModel > xFormsModel;
            if ( aAny >>= xFormsModel )
            {
                sal_uInt16 nPagePos = TAB_PAGE_NOTFOUND;
                sal_uInt16 nId = 0;
                XFormsPage* pPage = GetCurrentPage( nId );
                DBG_ASSERT( pPage, "DataNavigatorWindow::SetPageModel(): no page" );
                if (IsAdditionalPage(nId) || m_pTabCtrl->GetPageName(nId) == "instance")
                {
                    // instance page
                    nPagePos = m_pTabCtrl->GetPagePos( nId );
                }
                m_bIsNotifyDisabled = true;
                OUString sText = pPage->SetModel( xFormsModel, nPagePos );
                m_bIsNotifyDisabled = false;
                if ( !sText.isEmpty() )
                    m_pTabCtrl->SetPageText( nId, sText );
            }
        }
        catch (const NoSuchElementException& )
        {
            SAL_WARN( "svx.form", "DataNavigatorWindow::SetPageModel(): no such element" );
        }
        catch( Exception const & )
        {
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN( "svx.form", "DataNavigatorWindow::SetPageModel(): unexpected exception " << exceptionToString(ex) );
        }
    }

    void DataNavigatorWindow::InitPages()
    {
        OUString sModel( m_pModelsBox->GetSelectedEntry() );
        try
        {
            Any aAny = m_xDataContainer->getByName( sModel );
            Reference< css::xforms::XModel > xModel;
            if ( aAny >>= xModel )
            {
                Reference< XEnumerationAccess > xNumAccess( xModel->getInstances(), UNO_QUERY );
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
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN( "svx.form", "DataNavigatorWindow::SetPageModel(): unexpected exception " << exceptionToString(ex) );
        }
    }

    void DataNavigatorWindow::ClearAllPageModels( bool bClearPages )
    {
        if ( m_pInstPage )
            m_pInstPage->ClearModel();
        if ( m_pSubmissionPage )
            m_pSubmissionPage->ClearModel();
        if ( m_pBindingPage )
            m_pBindingPage->ClearModel();

        sal_Int32 i, nCount = m_aPageList.size();
        for ( i = 0; i < nCount; ++i )
        {
            VclPtr<XFormsPage> pPage = m_aPageList[i];
            pPage->ClearModel();
            if ( bClearPages )
                pPage.disposeAndClear();
        }

        if ( bClearPages )
        {
            m_aPageList.clear();
            while ( m_pTabCtrl->GetPageCount() > MIN_PAGE_COUNT )
                m_pTabCtrl->RemovePage( m_pTabCtrl->GetPageId( 1 ) );
        }
    }

    void DataNavigatorWindow::CreateInstancePage( const Sequence< PropertyValue >& _xPropSeq )
    {
        OUString sInstName;
        const PropertyValue* pProps = _xPropSeq.getConstArray();
        const PropertyValue* pPropsEnd = pProps + _xPropSeq.getLength();
        for ( ; pProps != pPropsEnd; ++pProps )
        {
            if ( PN_INSTANCE_ID == pProps->Name )
            {
                pProps->Value >>= sInstName;
                break;
            }
        }

        sal_uInt16 nPageId = GetNewPageId();
        if ( sInstName.isEmpty() )
        {
            SAL_WARN( "svx.form", "DataNavigatorWindow::CreateInstancePage(): instance without name" );
            OUString sTemp("untitled");
            sTemp += OUString::number( nPageId );
            sInstName = sTemp;
        }
        m_pTabCtrl->InsertPage( nPageId, sInstName, m_pTabCtrl->GetPageCount() - 2 );
    }

    bool DataNavigatorWindow::HasFirstInstancePage() const
    {
        return (m_pTabCtrl->GetPageName(m_pTabCtrl->GetPageId(0)) == "instance");
    }

    sal_uInt16 DataNavigatorWindow::GetNewPageId() const
    {
        sal_uInt16 i, nMax = 0, nCount = m_pTabCtrl->GetPageCount();
        for ( i = 0; i < nCount; ++i )
        {
            if ( nMax < m_pTabCtrl->GetPageId(i) )
                nMax = m_pTabCtrl->GetPageId(i);
        }
        return ( nMax + 1 );
    }

    void DataNavigatorWindow::Resize()
    {
        vcl::Window *pChild = GetWindow(GetWindowType::FirstChild);
        if (!pChild)
            return;
        VclContainer::setLayoutAllocation(*pChild, Point(0,0), GetSizePixel());
    }

    Size DataNavigatorWindow::GetOptimalSize() const
    {
        const vcl::Window *pChild = GetWindow(GetWindowType::FirstChild);
        if (!pChild)
            return Window::GetOptimalSize();
        return VclContainer::getLayoutRequisition(*pChild);
    }

    void DataNavigatorWindow::SetDocModified()
    {
        SfxObjectShell* pCurrentDoc = SfxObjectShell::Current();
        DBG_ASSERT( pCurrentDoc, "DataNavigatorWindow::SetDocModified(): no objectshell" );
        if ( !pCurrentDoc->IsModified() && pCurrentDoc->IsEnableSetModified() )
            pCurrentDoc->SetModified();
    }

    void DataNavigatorWindow::NotifyChanges( bool _bLoadAll )
    {
        if ( !m_bIsNotifyDisabled )
        {
            if ( _bLoadAll )
            {
                // reset all members
                RemoveBroadcaster();
                m_xDataContainer.clear();
                m_xFrameModel.clear();
                m_pModelsBox->Clear();
                m_nLastSelectedPos = LISTBOX_ENTRY_NOTFOUND;
                // for a reload
                LoadModels();
            }
            else
                m_aUpdateTimer.Start();
        }
    }


    void DataNavigatorWindow::AddContainerBroadcaster( const css::uno::Reference< css::container::XContainer >& xContainer )
    {
        Reference< XContainerListener > xListener(
            static_cast< XContainerListener* >( m_xDataListener.get() ), UNO_QUERY );
        xContainer->addContainerListener( xListener );
        m_aContainerList.push_back( xContainer );
    }


    void DataNavigatorWindow::AddEventBroadcaster( const css::uno::Reference< css::xml::dom::events::XEventTarget >& xTarget )
    {
        Reference< XEventListener > xListener(
            static_cast< XEventListener* >( m_xDataListener.get() ), UNO_QUERY );
        xTarget->addEventListener( EVENTTYPE_CHARDATA, xListener, true );
        xTarget->addEventListener( EVENTTYPE_CHARDATA, xListener, false );
        xTarget->addEventListener( EVENTTYPE_ATTR, xListener, true );
        xTarget->addEventListener( EVENTTYPE_ATTR, xListener, false );
        m_aEventTargetList.push_back( xTarget );
    }


    void DataNavigatorWindow::RemoveBroadcaster()
    {
        Reference< XContainerListener > xContainerListener(
            static_cast< XContainerListener* >( m_xDataListener.get() ), UNO_QUERY );
        sal_Int32 i, nCount = m_aContainerList.size();
        for ( i = 0; i < nCount; ++i )
            m_aContainerList[i]->removeContainerListener( xContainerListener );
        Reference< XEventListener > xEventListener(
            static_cast< XEventListener* >( m_xDataListener.get() ), UNO_QUERY );
        nCount = m_aEventTargetList.size();
        for ( i = 0; i < nCount; ++i )
        {
            m_aEventTargetList[i]->removeEventListener( EVENTTYPE_CHARDATA, xEventListener, true );
            m_aEventTargetList[i]->removeEventListener( EVENTTYPE_CHARDATA, xEventListener, false );
            m_aEventTargetList[i]->removeEventListener( EVENTTYPE_ATTR, xEventListener, true );
            m_aEventTargetList[i]->removeEventListener( EVENTTYPE_ATTR, xEventListener, false );
        }
    }

    DataNavigator::DataNavigator( SfxBindings* _pBindings, SfxChildWindow* _pMgr, vcl::Window* _pParent ) :

        SfxDockingWindow( _pBindings, _pMgr, _pParent,
                          WinBits(WB_STDMODELESS|WB_SIZEABLE|WB_ROLLABLE|WB_3DLOOK|WB_DOCKABLE) ),
        SfxControllerItem( SID_FM_DATANAVIGATOR_CONTROL, *_pBindings ),

        m_aDataWin( VclPtr<DataNavigatorWindow>::Create(this, _pBindings) )

    {

        SetText( SvxResId( RID_STR_DATANAVIGATOR ) );

        Size aSize = m_aDataWin->GetOutputSizePixel();
        Size aLogSize = PixelToLogic(aSize, MapMode(MapUnit::MapAppFont));
        SfxDockingWindow::SetFloatingSize( aLogSize );

        m_aDataWin->Show();
    }


    DataNavigator::~DataNavigator()
    {
        disposeOnce();
    }

    void DataNavigator::dispose()
    {
        m_aDataWin.disposeAndClear();
        ::SfxControllerItem::dispose();
        SfxDockingWindow::dispose();
    }

    void DataNavigator::StateChanged( sal_uInt16 , SfxItemState , const SfxPoolItem*  )
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


    void DataNavigator::Resize()
    {
        SfxDockingWindow::Resize();

        Size aLogOutputSize = PixelToLogic(GetOutputSizePixel(), MapMode(MapUnit::MapAppFont));
        Size aLogExplSize = aLogOutputSize;
        aLogExplSize.AdjustWidth( -2 );
        aLogExplSize.AdjustHeight( -2 );

        Point aExplPos = LogicToPixel(Point(1,1), MapMode(MapUnit::MapAppFont));
        Size aExplSize = LogicToPixel(aLogExplSize, MapMode(MapUnit::MapAppFont));

        m_aDataWin->SetPosSizePixel( aExplPos, aExplSize );
    }


    // class NavigatorFrameManager


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
        : GenericDialogController(pParent, "svx/ui/adddataitemdialog.ui", "AddDataItemDialog")
        , m_xUIHelper(_rUIHelper)
        , m_pItemNode(_pNode)
        , m_eItemType(DITNone)
        , m_sFL_Element(SvxResId(RID_STR_ELEMENT))
        , m_sFL_Attribute(SvxResId(RID_STR_ATTRIBUTE))
        , m_sFL_Binding(SvxResId(RID_STR_BINDING))
        , m_sFT_BindingExp(SvxResId(RID_STR_BINDING_EXPR))
        , m_xItemFrame(m_xBuilder->weld_frame("itemframe"))
        , m_xNameFT(m_xBuilder->weld_label("nameft"))
        , m_xNameED(m_xBuilder->weld_entry("name"))
        , m_xDefaultFT(m_xBuilder->weld_label("valueft"))
        , m_xDefaultED(m_xBuilder->weld_entry("value"))
        , m_xDefaultBtn(m_xBuilder->weld_button("browse"))
        , m_xSettingsFrame(m_xBuilder->weld_widget("settingsframe"))
        , m_xDataTypeFT(m_xBuilder->weld_label("datatypeft"))
        , m_xDataTypeLB(m_xBuilder->weld_combo_box("datatype"))
        , m_xRequiredCB(m_xBuilder->weld_check_button("required"))
        , m_xRequiredBtn(m_xBuilder->weld_button("requiredcond"))
        , m_xRelevantCB(m_xBuilder->weld_check_button("relevant"))
        , m_xRelevantBtn(m_xBuilder->weld_button("relevantcond"))
        , m_xConstraintCB(m_xBuilder->weld_check_button("constraint"))
        , m_xConstraintBtn(m_xBuilder->weld_button("constraintcond"))
        , m_xReadonlyCB(m_xBuilder->weld_check_button("readonly"))
        , m_xReadonlyBtn(m_xBuilder->weld_button("readonlycond"))
        , m_xCalculateCB(m_xBuilder->weld_check_button("calculate"))
        , m_xCalculateBtn(m_xBuilder->weld_button("calculatecond"))
        , m_xOKBtn(m_xBuilder->weld_button("ok"))
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
                        xBindings->remove( makeAny( m_xTempBinding ) );
                }
                catch (const Exception&)
                {
                    css::uno::Any ex( cppu::getCaughtException() );
                    SAL_WARN( "svx.form", "AddDataItemDialog::Dtor(): exception caught " << exceptionToString(ex) );
                }
            }
        }
        if( m_xUIHelper.is()  &&  m_xBinding.is() )
        {
            // remove binding, if it does not convey 'useful' information
            m_xUIHelper->removeBindingIfUseless( m_xBinding );
        }
    }

    IMPL_LINK(AddDataItemDialog, CheckHdl, weld::ToggleButton&, rBox, void)
    {
        Check(&rBox);
    }

    void AddDataItemDialog::Check(weld::ToggleButton* pBox)
    {
        // Condition buttons are only enable if their check box is checked
        m_xReadonlyBtn->set_sensitive( m_xReadonlyCB->get_active() );
        m_xRequiredBtn->set_sensitive( m_xRequiredCB->get_active() );
        m_xRelevantBtn->set_sensitive( m_xRelevantCB->get_active() );
        m_xConstraintBtn->set_sensitive( m_xConstraintCB->get_active() );
        m_xCalculateBtn->set_sensitive( m_xCalculateCB->get_active() );

        if ( pBox && m_xTempBinding.is() )
        {
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
            m_xTempBinding->setPropertyValue( sPropName, makeAny( sTemp ) );
        }
    }

    IMPL_LINK(AddDataItemDialog, ConditionHdl, weld::Button&, rBtn, void)
    {
        OUString sTemp, sPropName;
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
                    sPropName, makeAny( sNewCondition ) );
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
            Sequence< Property > aProperties = xTo->getPropertySetInfo()->getProperties();
            sal_Int32 nProperties = aProperties.getLength();
            const Property* pProperties = aProperties.getConstArray();
            Reference< XPropertySetInfo > xFromInfo = xFrom->getPropertySetInfo();
            for ( sal_Int32 i = 0; i < nProperties; ++i )
            {
                const OUString& rName = pProperties[i].Name;

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
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN( "svx.form", "copyPropSet(): exception caught " << exceptionToString(ex) );
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
        m_xTempBinding->setPropertyValue( PN_BINDING_TYPE, makeAny( sDataType ) );

        if ( bIsHandleBinding )
        {
            // copy properties from temp binding to original binding
            copyPropSet( m_xTempBinding, m_pItemNode->m_xPropSet );
            try
            {
                OUString sValue = m_xNameED->get_text();
                m_pItemNode->m_xPropSet->setPropertyValue( PN_BINDING_ID, makeAny( sValue ) );
                sValue = m_xDefaultED->get_text();
                m_pItemNode->m_xPropSet->setPropertyValue( PN_BINDING_EXPR, makeAny( sValue ) );
            }
            catch ( Exception const & )
            {
                css::uno::Any ex( cppu::getCaughtException() );
                SAL_WARN( "svx.form", "AddDataDialog::OKHdl(): exception caught " << exceptionToString(ex) );
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
                css::uno::Any ex( cppu::getCaughtException() );
                SAL_WARN( "svx.form", "AddDataDialog::OKHdl(): exception caught " << exceptionToString(ex) );
            }
        }
        // then close the dialog
        m_xDialog->response(RET_OK);
    }

    void AddDataItemDialog::InitDialog()
    {
        // set handler
        Link<weld::ToggleButton&,void> aLink = LINK( this, AddDataItemDialog, CheckHdl );
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
                                xBindings->insert( makeAny( m_xTempBinding ) );
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
                    css::uno::Any ex( cppu::getCaughtException() );
                    SAL_WARN( "svx.form", "AddDataItemDialog::InitFromNode(): exception caught " << exceptionToString(ex) );
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
                            xBindings->insert( makeAny( m_xTempBinding ) );
                    }
                    catch ( Exception const & )
                    {
                        css::uno::Any ex( cppu::getCaughtException() );
                        SAL_WARN( "svx.form", "AddDataItemDialog::InitFromNode(): exception caught " << exceptionToString(ex) );
                    }
                }
                OUString sTemp;
                try
                {
                    Reference< XPropertySetInfo > xInfo = m_pItemNode->m_xPropSet->getPropertySetInfo();
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
                    css::uno::Any ex( cppu::getCaughtException() );
                    SAL_WARN( "svx.form", "AddDataItemDialog::InitFromNode(): exception caught " << exceptionToString(ex) );
                }

                m_xDefaultBtn->show();
            }

            if ( m_xTempBinding.is() )
            {
                OUString sTemp;
                try
                {
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
                    css::uno::Any ex( cppu::getCaughtException() );
                    SAL_WARN( "svx.form", "AddDataItemDialog::InitFromNode(): exception caught " << exceptionToString(ex) );
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
        if ( m_eItemType != DITText )
        {
            Reference< css::xforms::XModel > xModel( m_xUIHelper, UNO_QUERY );
            if ( xModel.is() )
            {
                try
                {
                    Reference< css::xforms::XDataTypeRepository > xDataTypes =
                        xModel->getDataTypeRepository();
                    if ( xDataTypes.is() )
                    {
                        Sequence< OUString > aNameList = xDataTypes->getElementNames();
                        sal_Int32 i, nCount = aNameList.getLength();
                        OUString* pNames = aNameList.getArray();
                        for ( i = 0; i < nCount; ++i )
                            m_xDataTypeLB->append_text(pNames[i]);
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
                    css::uno::Any ex( cppu::getCaughtException() );
                    SAL_WARN( "svx.form", "AddDataItemDialog::InitDataTypeBox(): exception caught " << exceptionToString(ex) );
                }
            }
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
        const OUString& _rPropertyName,
        const Reference< XPropertySet >& _rPropSet)
        : GenericDialogController(pParent, "svx/ui/addconditiondialog.ui", "AddConditionDialog")
        , m_sPropertyName(_rPropertyName)
        , m_xBinding(_rPropSet)
        , m_xConditionED(m_xBuilder->weld_text_view("condition"))
        , m_xResultWin(m_xBuilder->weld_text_view("result"))
        , m_xEditNamespacesBtn(m_xBuilder->weld_button("edit"))
        , m_xOKBtn(m_xBuilder->weld_button("ok"))
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
                css::uno::Any ex( cppu::getCaughtException() );
                SAL_WARN( "svx.form", "AddConditionDialog::Ctor(): exception caught " << exceptionToString(ex) );
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
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN( "svx.form", "AddDataItemDialog::EditHdl(): exception caught " << exceptionToString(ex) );
        }
        NamespaceItemDialog aDlg(this, xNameContnr);
        aDlg.run();
        try
        {
            m_xBinding->setPropertyValue( PN_BINDING_NAMESPACES, makeAny( xNameContnr ) );
        }
        catch ( Exception const & )
        {
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN( "svx.form", "AddDataItemDialog::EditHdl(): exception caught " << exceptionToString(ex) );
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
                css::uno::Any ex( cppu::getCaughtException() );
                SAL_WARN( "svx.form", "AddConditionDialog::ResultHdl(): exception caught " << exceptionToString(ex) );
            }
        }
        m_xResultWin->set_text(sResult);
    }

    NamespaceItemDialog::NamespaceItemDialog(AddConditionDialog* pCondDlg, Reference<XNameContainer>& rContainer)
        : GenericDialogController(pCondDlg->getDialog(), "svx/ui/namespacedialog.ui", "NamespaceDialog")
        , m_pConditionDlg(pCondDlg)
        , m_rNamespaces(rContainer)
        , m_xNamespacesList(m_xBuilder->weld_tree_view("namespaces"))
        , m_xAddNamespaceBtn(m_xBuilder->weld_button("add"))
        , m_xEditNamespaceBtn(m_xBuilder->weld_button("edit"))
        , m_xDeleteNamespaceBtn(m_xBuilder->weld_button("delete"))
        , m_xOKBtn(m_xBuilder->weld_button("ok"))
    {
        m_xNamespacesList->set_size_request(m_xNamespacesList->get_approximate_digit_width() * 80,
                                            m_xNamespacesList->get_height_rows(8));

        std::vector<int> aWidths;
        aWidths.push_back(m_xNamespacesList->get_approximate_digit_width() * 20);
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
                    m_rNamespaces->replaceByName( sPrefix, makeAny( sURL ) );
                else
                    m_rNamespaces->insertByName( sPrefix, makeAny( sURL ) );
            }
        }
        catch ( Exception const & )
        {
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN( "svx.form", "NamespaceItemDialog::OKHdl(): exception caught " << exceptionToString(ex) );
        }
        // and close the dialog
        m_xDialog->response(RET_OK);
    }

    void NamespaceItemDialog::LoadNamespaces()
    {
        try
        {
            int nRow = 0;
            Sequence< OUString > aAllNames = m_rNamespaces->getElementNames();
            const OUString* pAllNames = aAllNames.getConstArray();
            const OUString* pAllNamesEnd = pAllNames + aAllNames.getLength();
            for ( ; pAllNames != pAllNamesEnd; ++pAllNames )
            {
                OUString sURL;
                OUString sPrefix = *pAllNames;
                if ( m_rNamespaces->hasByName( sPrefix ) )
                {
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
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN( "svx.form", "NamespaceItemDialog::LoadNamespaces(): exception caught " << exceptionToString(ex) );
        }
    }

    ManageNamespaceDialog::ManageNamespaceDialog(weld::Window* pParent, AddConditionDialog* pCondDlg, bool bIsEdit)
        : GenericDialogController(pParent, "svx/ui/addnamespacedialog.ui", "AddNamespaceDialog")
        , m_pConditionDlg(pCondDlg)
        , m_xPrefixED(m_xBuilder->weld_entry("prefix"))
        , m_xUrlED(m_xBuilder->weld_entry("url"))
        , m_xOKBtn(m_xBuilder->weld_button("ok"))
        , m_xAltTitle(m_xBuilder->weld_label("alttitle"))
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
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN( "svx.form", "ManageNamespacesDialog::OKHdl(): exception caught " << exceptionToString(ex) );
        }

        // no error so close the dialog
        m_xDialog->response(RET_OK);
    }

    AddSubmissionDialog::AddSubmissionDialog(
        weld::Window* pParent, ItemNode* _pNode,
        const Reference< css::xforms::XFormsUIHelper1 >& _rUIHelper)
        : GenericDialogController(pParent, "svx/ui/addsubmissiondialog.ui", "AddSubmissionDialog")
        , m_pItemNode(_pNode)
        , m_xUIHelper(_rUIHelper)
        , m_xNameED(m_xBuilder->weld_entry("name"))
        , m_xActionED(m_xBuilder->weld_entry("action"))
        , m_xMethodLB(m_xBuilder->weld_combo_box("method"))
        , m_xRefED(m_xBuilder->weld_entry("expression"))
        , m_xRefBtn(m_xBuilder->weld_button("browse"))
        , m_xBindLB(m_xBuilder->weld_combo_box("binding"))
        , m_xReplaceLB(m_xBuilder->weld_combo_box("replace"))
        , m_xOKBtn(m_xBuilder->weld_button("ok"))
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
            xErrorBox->set_primary_text(Application::GetDisplayName());
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
                    m_xSubmission.set( m_xNewSubmission, UNO_QUERY );
                }
                catch ( Exception const & )
                {
                    css::uno::Any ex( cppu::getCaughtException() );
                    SAL_WARN( "svx.form", "AddSubmissionDialog::OKHdl(): exception caught " << exceptionToString(ex) );
                }
            }
        }

        if ( m_xSubmission.is() )
        {
            OUString sTemp = m_xNameED->get_text();
            try
            {
                m_xSubmission->setPropertyValue( PN_SUBMISSION_ID, makeAny( sTemp ) );
                sTemp = m_xActionED->get_text();
                m_xSubmission->setPropertyValue( PN_SUBMISSION_ACTION, makeAny( sTemp ) );
                sTemp = m_aMethodString.toAPI( m_xMethodLB->get_active_text() );
                m_xSubmission->setPropertyValue( PN_SUBMISSION_METHOD, makeAny( sTemp ) );
                sTemp = m_xRefED->get_text();
                m_xSubmission->setPropertyValue( PN_SUBMISSION_REF, makeAny( sTemp ) );
                OUString sEntry = m_xBindLB->get_active_text();
                sal_Int32 nColonIdx = sEntry.indexOf(':');
                if (nColonIdx != -1)
                    sEntry = sEntry.copy(0, nColonIdx);
                sTemp = sEntry;
                m_xSubmission->setPropertyValue( PN_SUBMISSION_BIND, makeAny( sTemp ) );
                sTemp = m_aReplaceString.toAPI( m_xReplaceLB->get_active_text() );
                m_xSubmission->setPropertyValue( PN_SUBMISSION_REPLACE, makeAny( sTemp ) );
            }
            catch ( Exception const & )
            {
                css::uno::Any ex( cppu::getCaughtException() );
                SAL_WARN( "svx.form", "AddSubmissionDialog::OKHdl(): exception caught " << exceptionToString(ex) );
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
                Reference< XEnumerationAccess > xNumAccess( xModel->getBindings(), UNO_QUERY );
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
                                sEntry += sTemp;
                                sEntry += ": ";
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
                css::uno::Any ex( cppu::getCaughtException() );
                SAL_WARN( "svx.form", "AddSubmissionDialog::FillAllBoxes(): exception caught " << exceptionToString(ex) );
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
            OUString sTemp;
            try
            {
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
                css::uno::Any ex( cppu::getCaughtException() );
                SAL_WARN( "svx.form", "AddSubmissionDialog::FillAllBoxes(): exception caught " << exceptionToString(ex) );
            }
        }

        m_xRefBtn->set_sensitive(m_xTempBinding.is());
    }

    AddModelDialog::AddModelDialog(weld::Window* pParent, bool bIsEdit)
        : GenericDialogController(pParent, "svx/ui/addmodeldialog.ui", "AddModelDialog")
        , m_xNameED(m_xBuilder->weld_entry("name"))
        , m_xModifyCB(m_xBuilder->weld_check_button("modify"))
        , m_xAltTitle(m_xBuilder->weld_label("alttitle"))
    {
        if (bIsEdit)
            m_xDialog->set_title(m_xAltTitle->get_label());
    }

    AddModelDialog::~AddModelDialog()
    {
    }

    AddInstanceDialog::AddInstanceDialog(weld::Window* pParent, bool _bEdit)
        : GenericDialogController(pParent, "svx/ui/addinstancedialog.ui", "AddInstanceDialog")
        , m_xNameED(m_xBuilder->weld_entry("name"))
        , m_xURLFT(m_xBuilder->weld_label("urlft"))
        , m_xURLED(new URLBox(m_xBuilder->weld_combo_box("url")))
        , m_xFilePickerBtn(m_xBuilder->weld_button("browse"))
        , m_xLinkInstanceCB(m_xBuilder->weld_check_button("link"))
        , m_xAltTitle(m_xBuilder->weld_label("alttitle"))
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
        INetURLObject aFile( SvtPathOptions().GetWorkPath() );

        aDlg.AddFilter( m_sAllFilterName, FILEDIALOG_FILTER_ALL );
        OUString sFilterName( "XML" );
        aDlg.AddFilter( sFilterName, "*.xml" );
        aDlg.SetCurrentFilter( sFilterName );
        aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );

        if (aDlg.Execute() == ERRCODE_NONE)
            m_xURLED->set_entry_text(aDlg.GetPath());
    }

    LinkedInstanceWarningBox::LinkedInstanceWarningBox(weld::Widget* pParent)
        : MessageDialogController(pParent, "svx/ui/formlinkwarndialog.ui",
                "FormLinkWarnDialog")
    {
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
