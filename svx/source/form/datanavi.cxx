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
#include <datanavi.hxx>
#include <fmservs.hxx>

#include <helpids.h>
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
#include <svtools/treelistentry.hxx>
#include <sfx2/app.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/objitem.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/layout.hxx>
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
#include <comphelper/processfactory.hxx>
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
        m_pToolBox->SetItemImage(m_nAddId, Image(BitmapEx(RID_SVXBMP_ADD)));
        m_pToolBox->SetItemImage(m_nAddElementId, Image(BitmapEx(RID_SVXBMP_ADD_ELEMENT)));
        m_pToolBox->SetItemImage(m_nAddAttributeId, Image(BitmapEx(RID_SVXBMP_ADD_ATTRIBUTE)));
        m_pToolBox->SetItemImage(m_nEditId, Image(BitmapEx(RID_SVXBMP_EDIT)));
        m_pToolBox->SetItemImage(m_nRemoveId, Image(BitmapEx(RID_SVXBMP_REMOVE)));

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
        aPos.Y() = aTbxSz.Height();
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
                            aExpImg = aCollImg = Image(BitmapEx(RID_SVXBMP_ATTRIBUTE));
                            break;
                        case css::xml::dom::NodeType_ELEMENT_NODE:
                            aExpImg = aCollImg = Image(BitmapEx(RID_SVXBMP_ELEMENT));
                            break;
                        case css::xml::dom::NodeType_TEXT_NODE:
                            aExpImg = aCollImg = Image(BitmapEx(RID_SVXBMP_TEXT));
                            break;
                        default:
                            aExpImg = aCollImg = Image(BitmapEx(RID_SVXBMP_OTHER));
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
                                aExpImg = aCollImg = Image(BitmapEx(RID_SVXBMP_ATTRIBUTE));
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
            DBG_UNHANDLED_EXCEPTION();
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
                ScopedVclPtrInstance< AddSubmissionDialog > aDlg( this, nullptr, m_xUIHelper );
                if ( aDlg->Execute() == RET_OK && aDlg->GetNewSubmission().is() )
                {
                    try
                    {
                        Reference< css::xforms::XSubmission > xNewSubmission = aDlg->GetNewSubmission();
                        Reference< XSet > xSubmissions( xModel->getSubmissions(), UNO_QUERY );
                        xSubmissions->insert( makeAny( xNewSubmission ) );
                        Reference< XPropertySet > xNewPropSet( xNewSubmission, UNO_QUERY );
                        SvTreeListEntry* pEntry = AddEntry( xNewPropSet );
                        m_pItemList->Select( pEntry );
                        bIsDocModified = true;
                    }
                    catch ( Exception& )
                    {
                        SAL_WARN( "svx.form", "XFormsPage::DoToolBoxAction(): exception while adding submission" );
                    }
                }
            }
            else
            {
                DataItemType eType = DITElement;
                SvTreeListEntry* pEntry = m_pItemList->FirstSelected();
                ItemNode* pNode = nullptr;
                Reference< css::xml::dom::XNode > xParentNode;
                Reference< XPropertySet > xNewBinding;
                const char* pResId = nullptr;
                bool bIsElement = true;
                if ( DGTInstance == m_eGroup )
                {
                    if ( !m_sInstanceURL.isEmpty() )
                    {
                        ScopedVclPtrInstance< LinkedInstanceWarningBox > aMsgBox( this );
                        if ( aMsgBox->Execute() != RET_OK )
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
                        catch ( Exception& )
                        {
                            SAL_WARN( "svx.form", "XFormsPage::DoToolBoxAction(): exception while create element" );
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
                        catch ( Exception& )
                        {
                            SAL_WARN( "svx.form", "XFormsPage::DoToolBoxAction(): exception while create attribute" );
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
                            SAL_WARN( "svx.form", "XFormsPage::DoToolBoxAction(): domexception: size error" );
                        }
                        SAL_WARN( "svx.form", "XFormsPage::DoToolBoxAction(): domexception while append child" );
                    }
                    catch ( Exception& )
                    {
                        SAL_WARN( "svx.form", "XFormsPage::DoToolBoxAction(): exception while append child" );
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
                    catch ( Exception& )
                    {
                        SAL_WARN( "svx.form", "XFormsPage::DoToolboxAction(): exception caught" );
                    }

                    try
                    {
                        m_xUIHelper->getBindingForNode( xNewNode, true );
                    }
                    catch ( Exception& )
                    {
                        SAL_WARN( "svx.form", "XFormsPage::DoToolBoxAction(): exception while get binding for node" );
                    }
                    pNode = new ItemNode( xNewNode );
                }
                else
                {
                    try
                    {
                        pResId = RID_STR_DATANAV_ADD_BINDING;
                        xNewBinding = xModel->createBinding();
                        Reference< XSet > xBindings( xModel->getBindings(), UNO_QUERY );
                        xBindings->insert( makeAny( xNewBinding ) );
                        pNode = new ItemNode( xNewBinding );
                        eType = DITBinding;
                    }
                    catch ( Exception& )
                    {
                        SAL_WARN( "svx.form", "XFormsPage::DoToolBoxAction(): exception while adding binding" );
                    }
                }

                ScopedVclPtrInstance< AddDataItemDialog > aDlg( this, pNode, m_xUIHelper );
                aDlg->SetText( SvxResId( pResId ) );
                aDlg->InitText( eType );
                short nReturn = aDlg->Execute();
                if (  DGTInstance == m_eGroup )
                {
                    if ( RET_OK == nReturn )
                    {
                        SvTreeListEntry* pNewEntry = AddEntry( pNode, bIsElement );
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
                            delete pNode;
                        }
                        catch ( Exception& )
                        {
                            SAL_WARN( "svx.form", "XFormsPage::DoToolboxAction(): exception caught" );
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
                        catch ( Exception& )
                        {
                            SAL_WARN( "svx.form", "XFormsPage::DoToolboxAction(): exception caught" );
                        }
                    }
                    delete pNode;
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
                        ScopedVclPtrInstance< LinkedInstanceWarningBox > aMsgBox( this );
                        if ( aMsgBox->Execute() != RET_OK )
                            return bHandled;
                    }

                    ScopedVclPtrInstance< AddDataItemDialog > aDlg( this, pNode, m_xUIHelper );
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
                        catch ( Exception& )
                        {
                            SAL_WARN( "svx.form", "XFormsPage::DoToolboxAction(): exception caught" );
                        }
                    }
                    else if ( DGTBinding == m_eGroup )
                    {
                        pResId = RID_STR_DATANAV_EDIT_BINDING;
                        eType = DITBinding;
                    }
                    aDlg->SetText( SvxResId( pResId ) );
                    aDlg->InitText( eType );
                    if ( aDlg->Execute() == RET_OK )
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
                            catch ( Exception& )
                            {
                                SAL_WARN( "svx.form", "XFormsPage::DoToolboxAction(): exception caught" );
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
                            catch ( Exception& )
                            {
                                SAL_WARN( "svx.form", "XFormsPage::DoToolboxAction(): exception caught" );
                            }
                        }

                        m_pItemList->SetEntryText( pEntry, sNewName );
                        bIsDocModified = true;
                    }
                }
                else
                {
                    ScopedVclPtrInstance< AddSubmissionDialog > aDlg( this, pNode, m_xUIHelper );
                    aDlg->SetText( SvxResId( RID_STR_DATANAV_EDIT_SUBMISSION ) );
                    if ( aDlg->Execute() == RET_OK )
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
                ScopedVclPtrInstance< LinkedInstanceWarningBox > aMsgBox( this );
                if ( aMsgBox->Execute() != RET_OK )
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

    SvTreeListEntry* XFormsPage::AddEntry( ItemNode* _pNewNode, bool _bIsElement )
    {
        SvTreeListEntry* pParent = m_pItemList->FirstSelected();
        Image aImage(BitmapEx(_bIsElement ? OUString(RID_SVXBMP_ELEMENT) : OUString(RID_SVXBMP_ATTRIBUTE)));
        OUString sName;
        try
        {
            sName = m_xUIHelper->getNodeDisplayName(
                _pNewNode->m_xNode, m_pNaviWin->IsShowDetails() );
        }
        catch ( Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return m_pItemList->InsertEntry(
            sName, aImage, aImage, pParent, false, TREELIST_APPEND, _pNewNode );
    }

    SvTreeListEntry* XFormsPage::AddEntry( const Reference< XPropertySet >& _rEntry )
    {
        SvTreeListEntry* pEntry = nullptr;
        Image aImage(BitmapEx(RID_SVXBMP_ELEMENT));

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
            catch ( Exception& )
            {
                SAL_WARN( "svx.form", "XFormsPage::AddEntry(Ref): exception caught" );
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
            catch ( Exception& )
            {
                SAL_WARN( "svx.form", "XFormsPage::AddEntry(Ref): exception caught" );
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
            catch ( Exception& )
            {
                SAL_WARN( "svx.form", "XFormsPage::EditEntry(): exception caught" );
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
                    ScopedVclPtrInstance< MessageDialog > aQBox(this, SvxResId(pResId), VclMessageType::Question, VclButtonsType::YesNo);
                    OUString sMessText = aQBox->get_primary_text();
                    sMessText = sMessText.replaceFirst(
                        sVar, m_xUIHelper->getNodeDisplayName( pNode->m_xNode, false ) );
                    aQBox->set_primary_text(sMessText);
                    if ( aQBox->Execute() == RET_YES )
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
                catch ( Exception& )
                {
                    SAL_WARN( "svx.form", "XFormsPage::RemoveEntry(): exception caught" );
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
                catch ( Exception& )
                {
                    SAL_WARN( "svx.form", "XFormsPage::RemoveEntry(): exception caught" );
                }
                ScopedVclPtrInstance<MessageDialog> aQBox(this, SvxResId(pResId),
                                                          VclMessageType::Question, VclButtonsType::YesNo);
                OUString sMessText = aQBox->get_primary_text();
                sMessText = sMessText.replaceFirst( sSearch, sName);
                aQBox->set_primary_text(sMessText);
                if ( aQBox->Execute() == RET_YES )
                {
                    try
                    {
                        if ( bSubmission )
                            xModel->getSubmissions()->remove( makeAny( pNode->m_xPropSet ) );
                        else // then Binding Page
                            xModel->getBindings()->remove( makeAny( pNode->m_xPropSet ) );
                        bRet = true;
                    }
                    catch ( Exception& )
                    {
                        SAL_WARN( "svx.form", "XFormsPage::RemoveEntry(): exception caught" );
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
        aTbxSize.Width() = aSize.Width();
        m_pToolBox->SetSizePixel( aTbxSize );
        aSize.Width() -= 4;
        aSize.Height() -= ( 4 + aTbxSize.Height() );
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
                catch( Exception& )
                {
                    SAL_WARN( "svx.form", "XFormsPage::SetModel(): exception caught" );
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
                catch( Exception& )
                {
                    SAL_WARN( "svx.form", "XFormsPage::SetModel(): exception caught" );
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
                            Image aImage(BitmapEx(RID_SVXBMP_ELEMENT));
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
                catch( Exception& )
                {
                    SAL_WARN( "svx.form", "XFormsPage::SetModel(): exception caught" );
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
                    catch ( Exception& )
                    {
                        SAL_WARN( "svx.form", "XFormsPage::LoadInstance(): exception caught" );
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
                    catch ( Exception& )
                    {
                       SAL_WARN( "svx.form", "XFormsPage::EnableMenuItems(): exception caught" );
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
                    catch ( Exception& )
                    {
                       SAL_WARN( "svx.form", "XFormsPage::EnableMenuItems(): exception caught" );
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
        sal_Int32 nPageId = m_pTabCtrl->GetPageId("instance");
        SvtViewOptions aViewOpt( EViewType::TabDialog, CFGNAME_DATANAVIGATOR );
        if ( aViewOpt.Exists() )
        {
            nPageId = aViewOpt.GetPageID();
            aViewOpt.GetUserItem(CFGNAME_SHOWDETAILS) >>= m_bShowDetails;
        }

        Menu* pMenu = m_pInstanceBtn->GetPopupMenu();
        sal_uInt16 nInstancesDetailsId = pMenu->GetItemId("instancesdetails");
        pMenu->SetItemBits(nInstancesDetailsId, MenuItemBits::CHECKABLE );
        pMenu->CheckItem(nInstancesDetailsId, m_bShowDetails );

        m_pTabCtrl->SetCurPageId( static_cast< sal_uInt16 >( nPageId ) );
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
        aViewOpt.SetPageID( static_cast< sal_Int32 >( m_pTabCtrl->GetCurPageId() ) );
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
        catch ( Exception& )
        {
            SAL_WARN( "svx.form", "DataNavigatorWindow::MenuSelectHdl(): exception caught" );
        }
        DBG_ASSERT( xUIHelper.is(), "DataNavigatorWindow::MenuSelectHdl(): no UIHelper" );

        m_bIsNotifyDisabled = true;

        if (m_pModelBtn == pBtn)
        {
            OString sIdent(pBtn->GetCurItemIdent());
            if (sIdent == "modelsadd")
            {
                ScopedVclPtrInstance< AddModelDialog > aDlg( this, false );
                bool bShowDialog = true;
                while ( bShowDialog )
                {
                    bShowDialog = false;
                    if ( aDlg->Execute() == RET_OK )
                    {
                        OUString sNewName = aDlg->GetName();
                        bool bDocumentData = aDlg->GetModifyDoc();

                        if ( m_pModelsBox->GetEntryPos( sNewName ) != LISTBOX_ENTRY_NOTFOUND )
                        {
                            // error: model name already exists
                            ScopedVclPtrInstance< MessageDialog > aErrBox( this, SvxResId( RID_STR_DOUBLE_MODELNAME ) );
                            aErrBox->set_primary_text(aErrBox->get_primary_text().replaceFirst(MSG_VARIABLE, sNewName));
                            aErrBox->Execute();
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
                            catch ( Exception& )
                            {
                                SAL_WARN( "svx.form", "DataNavigatorWindow::MenuSelectHdl(): exception caught" );
                            }
                        }
                    }
                }
            }
            else if (sIdent == "modelsedit")
            {
                ScopedVclPtrInstance< AddModelDialog > aDlg( this, true );
                aDlg->SetName( sSelectedModel );

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
                    DBG_UNHANDLED_EXCEPTION();
                }
                aDlg->SetModifyDoc( bDocumentData );

                if ( aDlg->Execute() == RET_OK )
                {
                    if ( aDlg->GetModifyDoc() != bDocumentData )
                    {
                        bDocumentData = aDlg->GetModifyDoc();
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
                            DBG_UNHANDLED_EXCEPTION();
                        }
                    }

                    OUString sNewName = aDlg->GetName();
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
                        catch ( Exception& )
                        {
                            SAL_WARN( "svx.form", "DataNavigatorWindow::MenuSelectHdl(): exception caught" );
                        }
                    }
                }
            }
            else if (sIdent == "modelsremove")
            {
                ScopedVclPtrInstance<MessageDialog> aQBox(this, SvxResId( RID_STR_QRY_REMOVE_MODEL),
                                    VclMessageType::Question, VclButtonsType::YesNo);
                OUString sText = aQBox->get_primary_text();
                sText = sText.replaceFirst( MODELNAME, sSelectedModel );
                aQBox->set_primary_text(sText);
                if ( aQBox->Execute() == RET_YES )
                {
                    try
                    {
                        xUIHelper->removeModel( m_xFrameModel, sSelectedModel );
                    }
                    catch ( Exception& )
                    {
                        SAL_WARN( "svx.form", "DataNavigatorWindow::MenuSelectHdl(): exception caught" );
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
                ScopedVclPtrInstance< AddInstanceDialog > aDlg( this, false );
                if ( aDlg->Execute() == RET_OK )
                {
                    sal_uInt16 nInst = GetNewPageId();
                    OUString sName = aDlg->GetName();
                    OUString sURL = aDlg->GetURL();
                    bool bLinkOnce = aDlg->IsLinkInstance();
                    try
                    {
                        xUIHelper->newInstance( sName, sURL, !bLinkOnce );
                    }
                    catch ( Exception& )
                    {
                        SAL_WARN( "svx.form", "DataNavigatorWindow::MenuSelectHdl(): exception caught" );
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
                    ScopedVclPtrInstance< AddInstanceDialog > aDlg( this, true );
                    aDlg->SetName( pPage->GetInstanceName() );
                    aDlg->SetURL( pPage->GetInstanceURL() );
                    aDlg->SetLinkInstance( pPage->GetLinkOnce() );
                    OUString sOldName = aDlg->GetName();
                    if ( aDlg->Execute() == RET_OK )
                    {
                        OUString sNewName = aDlg->GetName();
                        OUString sURL = aDlg->GetURL();
                        bool bLinkOnce = aDlg->IsLinkInstance();
                        try
                        {
                            xUIHelper->renameInstance( sOldName,
                                                       sNewName,
                                                       sURL,
                                                       !bLinkOnce );
                        }
                        catch ( Exception& )
                        {
                            SAL_WARN( "svx.form", "DataNavigatorWindow::MenuSelectHdl(): exception caught" );
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
                    ScopedVclPtrInstance<MessageDialog> aQBox(this, SvxResId(RID_STR_QRY_REMOVE_INSTANCE),
                                                              VclMessageType::Question, VclButtonsType::YesNo);
                    OUString sMessText = aQBox->get_primary_text();
                    sMessText = sMessText.replaceFirst( INSTANCENAME, sInstName );
                    aQBox->set_primary_text(sMessText);
                    if ( aQBox->Execute() == RET_YES )
                    {
                        bool bDoRemove = false;
                        if (IsAdditionalPage(nId))
                        {
                            PageList::iterator aPageListEnd = m_aPageList.end();
                            PageList::iterator aFoundPage =
                                std::find( m_aPageList.begin(), aPageListEnd, pPage );
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
                                SAL_WARN( "svx.form", "DataNavigatorWindow::MenuSelectHdl(): exception caught" );
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
                pMenu->CheckItem(pMenu->GetItemId("instancesdetails"), m_bShowDetails );
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
                catch ( Exception& )
                {
                    SAL_WARN( "svx.form", "DataNavigatorWindow::LoadModels(): exception caught" );
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
            catch( Exception& )
            {
                SAL_WARN( "svx.form", "DataNavigatorWindow::LoadModels(): exception caught" );
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
        catch( Exception& )
        {
            SAL_WARN( "svx.form", "DataNavigatorWindow::SetPageModel(): unexpected exception" );
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
        catch( Exception& )
        {
            SAL_WARN( "svx.form", "DataNavigatorWindow::SetPageModel(): unexpected exception" );
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


    void DataNavigatorWindow::AddContainerBroadcaster( const XContainer_ref& xContainer )
    {
        Reference< XContainerListener > xListener(
            static_cast< XContainerListener* >( m_xDataListener.get() ), UNO_QUERY );
        xContainer->addContainerListener( xListener );
        m_aContainerList.push_back( xContainer );
    }


    void DataNavigatorWindow::AddEventBroadcaster( const XEventTarget_ref& xTarget )
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
        aLogExplSize.Width() -= 2;
        aLogExplSize.Height() -= 2;

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

    AddDataItemDialog::AddDataItemDialog(vcl::Window* pParent, ItemNode* _pNode,
        const Reference< css::xforms::XFormsUIHelper1 >& _rUIHelper)
        : ModalDialog(pParent, "AddDataItemDialog" , "svx/ui/adddataitemdialog.ui")
        , m_xUIHelper(_rUIHelper)
        , m_pItemNode(_pNode)
        , m_eItemType(DITNone)
        , m_sFL_Element(SvxResId(RID_STR_ELEMENT))
        , m_sFL_Attribute(SvxResId(RID_STR_ATTRIBUTE))
        , m_sFL_Binding(SvxResId(RID_STR_BINDING))
        , m_sFT_BindingExp(SvxResId(RID_STR_BINDING_EXPR))
    {
        get(m_pItemFrame, "itemframe");
        get(m_pNameFT, "nameft");
        get(m_pNameED, "name");
        get(m_pDefaultFT, "valueft");
        get(m_pDefaultED, "value");
        get(m_pDefaultBtn, "browse");
        get(m_pSettingsFrame, "settingsframe");
        get(m_pDataTypeFT, "datatypeft");
        get(m_pDataTypeLB, "datatype");
        get(m_pRequiredCB, "required");
        get(m_pRequiredBtn, "requiredcond");
        get(m_pRelevantCB, "relevant");
        get(m_pRelevantBtn, "relevantcond");
        get(m_pConstraintCB, "constraint");
        get(m_pConstraintBtn, "constraintcond");
        get(m_pReadonlyCB, "readonly");
        get(m_pReadonlyBtn, "readonlycond");
        get(m_pCalculateCB, "calculate");
        get(m_pCalculateBtn, "calculatecond");
        get(m_pOKBtn, "ok");
        m_pDataTypeLB->SetDropDownLineCount( 10 );

        InitDialog();
        InitFromNode();
        InitDataTypeBox();
        CheckHdl( nullptr );
    }


    AddDataItemDialog::~AddDataItemDialog()
    {
        disposeOnce();
    }

    void AddDataItemDialog::dispose()
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
                    SAL_WARN( "svx.form", "AddDataItemDialog::Dtor(): exception caught" );
                }
            }
        }
        if( m_xUIHelper.is()  &&  m_xBinding.is() )
        {
            // remove binding, if it does not convey 'useful' information
            m_xUIHelper->removeBindingIfUseless( m_xBinding );
        }
        m_pItemFrame.clear();
        m_pNameFT.clear();
        m_pNameED.clear();
        m_pDefaultFT.clear();
        m_pDefaultED.clear();
        m_pDefaultBtn.clear();
        m_pSettingsFrame.clear();
        m_pDataTypeFT.clear();
        m_pDataTypeLB.clear();
        m_pRequiredCB.clear();
        m_pRequiredBtn.clear();
        m_pRelevantCB.clear();
        m_pRelevantBtn.clear();
        m_pConstraintCB.clear();
        m_pConstraintBtn.clear();
        m_pReadonlyCB.clear();
        m_pReadonlyBtn.clear();
        m_pCalculateCB.clear();
        m_pCalculateBtn.clear();
        m_pOKBtn.clear();
        ModalDialog::dispose();
    }


    IMPL_LINK( AddDataItemDialog, CheckHdl, Button *, pButton, void )
    {
        CheckBox* pBox = static_cast<CheckBox*>(pButton);
        // Condition buttons are only enable if their check box is checked
        m_pReadonlyBtn->Enable( m_pReadonlyCB->IsChecked() );
        m_pRequiredBtn->Enable( m_pRequiredCB->IsChecked() );
        m_pRelevantBtn->Enable( m_pRelevantCB->IsChecked() );
        m_pConstraintBtn->Enable( m_pConstraintCB->IsChecked() );
        m_pCalculateBtn->Enable( m_pCalculateCB->IsChecked() );

        if ( pBox && m_xTempBinding.is() )
        {
            OUString sTemp, sPropName;
            if ( m_pRequiredCB == pBox )
                sPropName = PN_REQUIRED_EXPR;
            else if ( m_pRelevantCB == pBox )
                sPropName = PN_RELEVANT_EXPR;
            else if ( m_pConstraintCB == pBox )
                sPropName = PN_CONSTRAINT_EXPR;
            else if ( m_pReadonlyCB == pBox )
                sPropName = PN_READONLY_EXPR;
            else if ( m_pCalculateCB == pBox )
                sPropName = PN_CALCULATE_EXPR;
            bool bIsChecked = pBox->IsChecked();
            m_xTempBinding->getPropertyValue( sPropName ) >>= sTemp;
            if ( bIsChecked && sTemp.isEmpty() )
                sTemp = TRUE_VALUE;
            else if ( !bIsChecked && !sTemp.isEmpty() )
                sTemp.clear();
            m_xTempBinding->setPropertyValue( sPropName, makeAny( sTemp ) );
        }
    }


    IMPL_LINK( AddDataItemDialog, ConditionHdl, Button *, pButton, void )
    {
        PushButton* pBtn = static_cast<PushButton*>(pButton);
        OUString sTemp, sPropName;
        if ( m_pDefaultBtn == pBtn )
            sPropName = PN_BINDING_EXPR;
        else if ( m_pRequiredBtn == pBtn )
            sPropName = PN_REQUIRED_EXPR;
        else if ( m_pRelevantBtn == pBtn )
            sPropName = PN_RELEVANT_EXPR;
        else if ( m_pConstraintBtn == pBtn )
            sPropName = PN_CONSTRAINT_EXPR;
        else if (m_pReadonlyBtn == pBtn)
            sPropName = PN_READONLY_EXPR;
        else if (m_pCalculateBtn == pBtn)
            sPropName = PN_CALCULATE_EXPR;
        ScopedVclPtrInstance< AddConditionDialog > aDlg(this, sPropName, m_xTempBinding);
        bool bIsDefBtn = ( m_pDefaultBtn == pBtn );
        OUString sCondition;
        if ( bIsDefBtn )
            sCondition = m_pDefaultED->GetText();
        else
        {
            m_xTempBinding->getPropertyValue( sPropName ) >>= sTemp;
            if ( sTemp.isEmpty() )
                sTemp = TRUE_VALUE;
            sCondition = sTemp;
        }
        aDlg->SetCondition( sCondition );

        if ( aDlg->Execute() == RET_OK )
        {
            OUString sNewCondition = aDlg->GetCondition();
            if ( bIsDefBtn )
                m_pDefaultED->SetText( sNewCondition );
            else
            {

                m_xTempBinding->setPropertyValue(
                    sPropName, makeAny( sNewCondition ) );
            }
        }
    }

    void copyPropSet( const Reference< XPropertySet >& xFrom, Reference< XPropertySet > const & xTo )
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
        catch ( Exception& )
        {
            SAL_WARN( "svx.form", "copyPropSet(): exception caught" );
        }
    }


    IMPL_LINK_NOARG(AddDataItemDialog, OKHdl, Button*, void)
    {
        bool bIsHandleBinding = ( DITBinding == m_eItemType );
        bool bIsHandleText = ( DITText == m_eItemType );
        OUString sNewName( m_pNameED->GetText() );

        if ( ( !bIsHandleBinding && !bIsHandleText && !m_xUIHelper->isValidXMLName( sNewName ) ) ||
             ( bIsHandleBinding && sNewName.isEmpty() ) )
        {
            // Error and don't close the dialog
            ScopedVclPtrInstance< MessageDialog > aErrBox( this, SvxResId( RID_STR_INVALID_XMLNAME ) );
            aErrBox->set_primary_text(aErrBox->get_primary_text().replaceFirst(MSG_VARIABLE, sNewName));
            aErrBox->Execute();
            return;
        }

        OUString sDataType( m_pDataTypeLB->GetSelectedEntry() );
        m_xTempBinding->setPropertyValue( PN_BINDING_TYPE, makeAny( sDataType ) );

        if ( bIsHandleBinding )
        {
            // copy properties from temp binding to original binding
            copyPropSet( m_xTempBinding, m_pItemNode->m_xPropSet );
            try
            {
                OUString sValue = m_pNameED->GetText();
                m_pItemNode->m_xPropSet->setPropertyValue( PN_BINDING_ID, makeAny( sValue ) );
                sValue = m_pDefaultED->GetText();
                m_pItemNode->m_xPropSet->setPropertyValue( PN_BINDING_EXPR, makeAny( sValue ) );
            }
            catch ( Exception& )
            {
                SAL_WARN( "svx.form", "AddDataDialog::OKHdl(): exception caught" );
            }
        }
        else
        {
            // copy properties from temp binding to original binding
            copyPropSet( m_xTempBinding, m_xBinding );
            try
            {
                if ( bIsHandleText )
                    m_xUIHelper->setNodeValue( m_pItemNode->m_xNode, m_pDefaultED->GetText() );
                else
                {
                    Reference< css::xml::dom::XNode > xNewNode =
                        m_xUIHelper->renameNode( m_pItemNode->m_xNode, m_pNameED->GetText() );
                    m_xUIHelper->setNodeValue( xNewNode, m_pDefaultED->GetText() );
                    m_pItemNode->m_xNode = xNewNode;
                }
            }
            catch ( Exception& )
            {
                SAL_WARN( "svx.form", "AddDataDialog::OKHdl(): exception caught" );
            }
        }
        // then close the dialog
        EndDialog( RET_OK );
    }


    void AddDataItemDialog::InitDialog()
    {
        // set handler
        Link<Button*,void> aLink = LINK( this, AddDataItemDialog, CheckHdl );
        m_pRequiredCB->SetClickHdl( aLink );
        m_pRelevantCB->SetClickHdl( aLink );
        m_pConstraintCB->SetClickHdl( aLink );
        m_pReadonlyCB->SetClickHdl( aLink );
        m_pCalculateCB->SetClickHdl( aLink );

        aLink = LINK( this, AddDataItemDialog, ConditionHdl );
        m_pDefaultBtn->SetClickHdl( aLink );
        m_pRequiredBtn->SetClickHdl( aLink );
        m_pRelevantBtn->SetClickHdl( aLink );
        m_pConstraintBtn->SetClickHdl( aLink );
        m_pReadonlyBtn->SetClickHdl( aLink );
        m_pCalculateBtn->SetClickHdl( aLink );

        m_pOKBtn->SetClickHdl( LINK( this, AddDataItemDialog, OKHdl ) );
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
                        m_pNameED->SetText( sName );
                    }
                    m_pDefaultED->SetText( m_pItemNode->m_xNode->getNodeValue() );
                }
                catch( Exception& )
                {
                    SAL_WARN( "svx.form", "AddDataItemDialog::InitFromNode(): exception caught" );
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
                    catch ( Exception& )
                    {
                        SAL_WARN( "svx.form", "AddDataItemDialog::InitFromNode(): exception caught" );
                    }
                }
                OUString sTemp;
                try
                {
                    Reference< XPropertySetInfo > xInfo = m_pItemNode->m_xPropSet->getPropertySetInfo();
                    if ( xInfo->hasPropertyByName( PN_BINDING_ID ) )
                    {
                        m_pItemNode->m_xPropSet->getPropertyValue( PN_BINDING_ID ) >>= sTemp;
                        m_pNameED->SetText( sTemp );
                        m_pItemNode->m_xPropSet->getPropertyValue( PN_BINDING_EXPR ) >>= sTemp;
                        m_pDefaultED->SetText( sTemp );
                    }
                    else if ( xInfo->hasPropertyByName( PN_SUBMISSION_BIND ) )
                    {
                        m_pItemNode->m_xPropSet->getPropertyValue( PN_SUBMISSION_ID ) >>= sTemp;
                        m_pNameED->SetText( sTemp );
                    }
                }
                catch( Exception& )
                {
                    SAL_WARN( "svx.form", "AddDataItemDialog::InitFromNode(): exception caught" );
                }

                Size a3and1Sz = LogicToPixel( Size(3, 1), MapMode(MapUnit::MapAppFont));
                Size aNewSz = m_pDefaultED->GetSizePixel();
                Point aNewPnt = m_pDefaultED->GetPosPixel();
                aNewPnt.Y() += a3and1Sz.Height();
                aNewSz.Width() -= ( m_pDefaultBtn->GetSizePixel().Width() + a3and1Sz.Width() );
                m_pDefaultED->SetPosSizePixel( aNewPnt, aNewSz );
                m_pDefaultBtn->Show();
            }

            if ( m_xTempBinding.is() )
            {
                OUString sTemp;
                try
                {
                    if ( ( m_xTempBinding->getPropertyValue( PN_REQUIRED_EXPR ) >>= sTemp )
                        && !sTemp.isEmpty() )
                        m_pRequiredCB->Check();
                    if ( ( m_xTempBinding->getPropertyValue( PN_RELEVANT_EXPR ) >>= sTemp )
                        && !sTemp.isEmpty() )
                        m_pRelevantCB->Check();
                    if ( ( m_xTempBinding->getPropertyValue( PN_CONSTRAINT_EXPR ) >>= sTemp )
                        && !sTemp.isEmpty() )
                        m_pConstraintCB->Check();
                    if ( ( m_xTempBinding->getPropertyValue( PN_READONLY_EXPR ) >>= sTemp )
                        && !sTemp.isEmpty() )
                        m_pReadonlyCB->Check();
                    if ( ( m_xTempBinding->getPropertyValue( PN_CALCULATE_EXPR ) >>= sTemp )
                        && !sTemp.isEmpty() )
                        m_pCalculateCB->Check();
                }
                catch (const Exception&)
                {
                    SAL_WARN( "svx.form", "AddDataItemDialog::InitFromNode(): exception caught" );
                }
            }
        }

        if ( DITText == m_eItemType )
        {
            m_pSettingsFrame->Hide();
            m_pNameFT->Disable();
            m_pNameED->Disable();
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
                            m_pDataTypeLB->InsertEntry( pNames[i] );
                    }

                    if ( m_xTempBinding.is() )
                    {
                        OUString sTemp;
                        if ( m_xTempBinding->getPropertyValue( PN_BINDING_TYPE ) >>= sTemp )
                        {
                            sal_Int32 nPos = m_pDataTypeLB->GetEntryPos( sTemp );
                            if ( LISTBOX_ENTRY_NOTFOUND == nPos )
                                nPos = m_pDataTypeLB->InsertEntry( sTemp );
                            m_pDataTypeLB->SelectEntryPos( nPos );
                        }
                    }
                }
                catch ( Exception& )
                {
                    SAL_WARN( "svx.form", "AddDataItemDialog::InitDataTypeBox(): exception caught" );
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
                m_pDefaultFT->SetText( m_sFT_BindingExp );
                break;
            }

            default:
            {
                sText = m_sFL_Element;
            }
        }

        m_pItemFrame->set_label(sText);
    }

    AddConditionDialog::AddConditionDialog(vcl::Window* pParent,
        const OUString& _rPropertyName,
        const Reference< XPropertySet >& _rPropSet)
        : ModalDialog(pParent, "AddConditionDialog", "svx/ui/addconditiondialog.ui")
        , m_sPropertyName(_rPropertyName)
        , m_xBinding(_rPropSet)

    {
        get(m_pConditionED, "condition");
        get(m_pResultWin, "result");
        get(m_pEditNamespacesBtn, "edit");
        get(m_pOKBtn, "ok");
        DBG_ASSERT( m_xBinding.is(), "AddConditionDialog::Ctor(): no Binding" );

        m_pConditionED->set_height_request(m_pConditionED->GetTextHeight() * 4);
        m_pConditionED->set_width_request(m_pConditionED->approximate_char_width() * 62);
        m_pResultWin->set_height_request(m_pResultWin->GetTextHeight() * 4);
        m_pResultWin->set_width_request(m_pResultWin->approximate_char_width() * 62);

        m_pConditionED->SetModifyHdl( LINK( this, AddConditionDialog, ModifyHdl ) );
        m_pEditNamespacesBtn->SetClickHdl( LINK( this, AddConditionDialog, EditHdl ) );
        m_pOKBtn->SetClickHdl( LINK( this, AddConditionDialog, OKHdl ) );
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
                    m_pConditionED->SetText( sTemp );
                }
                else
                {
//!                 m_xBinding->setPropertyValue( m_sPropertyName, makeAny( TRUE_VALUE ) );
                    m_pConditionED->SetText( TRUE_VALUE );
                }

                Reference< css::xforms::XModel > xModel;
                if ( ( m_xBinding->getPropertyValue( PN_BINDING_MODEL ) >>= xModel ) && xModel.is() )
                    m_xUIHelper.set( xModel, UNO_QUERY );
            }
            catch (const Exception&)
            {
                SAL_WARN( "svx.form", "AddConditionDialog::Ctor(): exception caught" );
            }
        }

        DBG_ASSERT( m_xUIHelper.is(), "AddConditionDialog::Ctor(): no UIHelper" );
        ResultHdl( &m_aResultIdle );
    }

    AddConditionDialog::~AddConditionDialog()
    {
        disposeOnce();
    }

    void AddConditionDialog::dispose()
    {
        m_pConditionED.clear();
        m_pResultWin.clear();
        m_pEditNamespacesBtn.clear();
        m_pOKBtn.clear();
        ModalDialog::dispose();
    }

    IMPL_LINK_NOARG(AddConditionDialog, EditHdl, Button*, void)
    {
        Reference< XNameContainer > xNameContnr;
        try
        {
            m_xBinding->getPropertyValue( PN_BINDING_NAMESPACES ) >>= xNameContnr;
        }
        catch ( Exception& )
        {
            SAL_WARN( "svx.form", "AddDataItemDialog::EditHdl(): exception caught" );
        }
        ScopedVclPtrInstance< NamespaceItemDialog > aDlg( this, xNameContnr );
        aDlg->Execute();
        try
        {
            m_xBinding->setPropertyValue( PN_BINDING_NAMESPACES, makeAny( xNameContnr ) );
        }
        catch ( Exception& )
        {
            SAL_WARN( "svx.form", "AddDataItemDialog::EditHdl(): exception caught" );
        }
    }


    IMPL_LINK_NOARG(AddConditionDialog, OKHdl, Button*, void)
    {
/*!!!
        try
        {
            if ( m_xBinding.is() )
                m_xBinding->setPropertyValue( m_sPropertyName, makeAny( OUString( m_pConditionED->GetText() ) ) );
        }
        catch( const Exception& )
        {
            SAL_WARN( "svx.form", "AddConditionDialog, OKHdl: caught an exception!" );
        }
*/
        EndDialog( RET_OK );
    }


    IMPL_LINK_NOARG(AddConditionDialog, ModifyHdl, Edit&, void)
    {
        m_aResultIdle.Start();
    }


    IMPL_LINK_NOARG(AddConditionDialog, ResultHdl, Timer *, void)
    {
        OUString sCondition = comphelper::string::strip(m_pConditionED->GetText(), ' ');
        OUString sResult;
        if ( !sCondition.isEmpty() )
        {
            try
            {
                sResult = m_xUIHelper->getResultForExpression( m_xBinding, ( m_sPropertyName == PN_BINDING_EXPR ), sCondition );
            }
            catch ( Exception& )
            {
                SAL_WARN( "svx.form", "AddConditionDialog::ResultHdl(): exception caught" );
            }
        }
        m_pResultWin->SetText( sResult );
    }

    NamespaceItemDialog::NamespaceItemDialog(
        AddConditionDialog* _pCondDlg,
            Reference< XNameContainer >& _rContainer )
        : ModalDialog( _pCondDlg, "NamespaceDialog",
            "svx/ui/namespacedialog.ui" )
        , m_pConditionDlg(_pCondDlg)
        , m_rNamespaces(_rContainer)
    {
        get(m_pAddNamespaceBtn, "add");
        get(m_pEditNamespaceBtn, "edit");
        get(m_pDeleteNamespaceBtn, "delete");
        get(m_pOKBtn, "ok");

        SvSimpleTableContainer* pNamespacesListContainer =
            get<SvSimpleTableContainer>("namespaces");
        Size aControlSize(175, 72);
        aControlSize = LogicToPixel(aControlSize, MapMode(MapUnit::MapAppFont));
        pNamespacesListContainer->set_width_request(aControlSize.Width());
        pNamespacesListContainer->set_height_request(aControlSize.Height());
        m_pNamespacesList = VclPtr<SvSimpleTable>::Create(*pNamespacesListContainer, 0);

        static long aStaticTabs[]= { 3, 0, 35, 200 };
        m_pNamespacesList->SvSimpleTable::SetTabs( aStaticTabs );
        OUString sHeader = get<FixedText>("prefix")->GetText();
        sHeader += "\t";
        sHeader += get<FixedText>("url")->GetText();
        m_pNamespacesList->InsertHeaderEntry(
            sHeader, HEADERBAR_APPEND, HeaderBarItemBits::LEFT /*| HeaderBarItemBits::FIXEDPOS | HeaderBarItemBits::FIXED*/ );

        m_pNamespacesList->SetSelectHdl( LINK( this, NamespaceItemDialog, SelectHdl ) );
        Link<Button*,void> aLink = LINK( this, NamespaceItemDialog, ClickHdl );
        m_pAddNamespaceBtn->SetClickHdl( aLink );
        m_pEditNamespaceBtn->SetClickHdl( aLink );
        m_pDeleteNamespaceBtn->SetClickHdl( aLink );
        m_pOKBtn->SetClickHdl( LINK( this, NamespaceItemDialog, OKHdl ) );

        LoadNamespaces();
        SelectHdl( m_pNamespacesList );
    }


    NamespaceItemDialog::~NamespaceItemDialog()
    {
        disposeOnce();
    }

    void NamespaceItemDialog::dispose()
    {
        m_pNamespacesList.disposeAndClear();
        m_pAddNamespaceBtn.clear();
        m_pEditNamespaceBtn.clear();
        m_pDeleteNamespaceBtn.clear();
        m_pOKBtn.clear();
        m_pConditionDlg.clear();
        ModalDialog::dispose();
    }


    IMPL_LINK_NOARG( NamespaceItemDialog, SelectHdl, SvTreeListBox *, void)
    {
        bool bEnable = ( m_pNamespacesList->FirstSelected() != nullptr );
        m_pEditNamespaceBtn->Enable( bEnable );
        m_pDeleteNamespaceBtn->Enable( bEnable );
    }


    IMPL_LINK( NamespaceItemDialog, ClickHdl, Button *, pButton, void )
    {
        PushButton* pBtn = static_cast<PushButton*>(pButton);
        if ( m_pAddNamespaceBtn == pBtn )
        {
            ScopedVclPtrInstance< ManageNamespaceDialog > aDlg(this, m_pConditionDlg, false);
            if ( aDlg->Execute() == RET_OK )
            {
                OUString sEntry = aDlg->GetPrefix();
                sEntry += "\t";
                sEntry += aDlg->GetURL();
                m_pNamespacesList->InsertEntry( sEntry );
            }
        }
        else if ( m_pEditNamespaceBtn == pBtn )
        {
            ScopedVclPtrInstance< ManageNamespaceDialog > aDlg( this, m_pConditionDlg, true );
            SvTreeListEntry* pEntry = m_pNamespacesList->FirstSelected();
            DBG_ASSERT( pEntry, "NamespaceItemDialog::ClickHdl(): no entry" );
            OUString sPrefix( SvTabListBox::GetEntryText( pEntry, 0 ) );
            aDlg->SetNamespace(
                sPrefix,
                SvTabListBox::GetEntryText( pEntry, 1 ) );
            if ( aDlg->Execute() == RET_OK )
            {
                // if a prefix was changed, mark the old prefix as 'removed'
                if( sPrefix != aDlg->GetPrefix() )
                    m_aRemovedList.push_back( sPrefix );

                m_pNamespacesList->SetEntryText( aDlg->GetPrefix(), pEntry, 0 );
                m_pNamespacesList->SetEntryText( aDlg->GetURL(), pEntry, 1 );
            }
        }
        else if ( m_pDeleteNamespaceBtn == pBtn )
        {
            SvTreeListEntry* pEntry = m_pNamespacesList->FirstSelected();
            DBG_ASSERT( pEntry, "NamespaceItemDialog::ClickHdl(): no entry" );
            OUString sPrefix( SvTabListBox::GetEntryText( pEntry, 0 ) );
            m_aRemovedList.push_back( sPrefix );
            m_pNamespacesList->GetModel()->Remove( pEntry );
        }
        else
        {
            SAL_WARN( "svx.form", "NamespaceItemDialog::ClickHdl(): invalid button" );
        }

        SelectHdl( m_pNamespacesList );
    }


    IMPL_LINK_NOARG(NamespaceItemDialog, OKHdl, Button*, void)
    {
        try
        {
            // update namespace container
            sal_Int32 i, nRemovedCount = m_aRemovedList.size();
            for( i = 0; i < nRemovedCount; ++i )
                m_rNamespaces->removeByName( m_aRemovedList[i] );

            sal_Int32 nEntryCount = m_pNamespacesList->GetEntryCount();
            for( i = 0; i < nEntryCount; ++i )
            {
                SvTreeListEntry* pEntry = m_pNamespacesList->GetEntry(i);
                OUString sPrefix( SvTabListBox::GetEntryText( pEntry, 0 ) );
                OUString sURL( SvTabListBox::GetEntryText( pEntry, 1 ) );

                if ( m_rNamespaces->hasByName( sPrefix ) )
                    m_rNamespaces->replaceByName( sPrefix, makeAny( sURL ) );
                else
                    m_rNamespaces->insertByName( sPrefix, makeAny( sURL ) );
            }
        }
        catch ( Exception& )
        {
            SAL_WARN( "svx.form", "NamespaceItemDialog::OKHdl(): exception caught" );
        }
        // and close the dialog
        EndDialog( RET_OK );
    }


    void NamespaceItemDialog::LoadNamespaces()
    {
        try
        {
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
                    if ( aAny >>= sURL )
                    {
                        OUString sEntry( sPrefix );
                        sEntry += "\t";
                        sEntry += sURL;

                        m_pNamespacesList->InsertEntry( sEntry );
                    }
                }
            }
        }
        catch ( Exception& )
        {
            SAL_WARN( "svx.form", "NamespaceItemDialog::LoadNamespaces(): exception caught" );
        }
    }

    ManageNamespaceDialog::ManageNamespaceDialog(vcl::Window* pParent, AddConditionDialog* _pCondDlg, bool bIsEdit)
        : ModalDialog(pParent, "AddNamespaceDialog", "svx/ui/addnamespacedialog.ui")
        , m_pConditionDlg ( _pCondDlg )
    {
        get(m_pOKBtn, "ok");
        get(m_pPrefixED, "prefix");
        get(m_pUrlED, "url");

        if (bIsEdit)
            SetText(get<FixedText>("alttitle")->GetText());

        m_pOKBtn->SetClickHdl( LINK( this, ManageNamespaceDialog, OKHdl ) );
    }

    ManageNamespaceDialog::~ManageNamespaceDialog()
    {
        disposeOnce();
    }

    void ManageNamespaceDialog::dispose()
    {
        m_pOKBtn.clear();
        m_pPrefixED.clear();
        m_pUrlED.clear();
        m_pConditionDlg.clear();
        ModalDialog::dispose();
    }

    IMPL_LINK_NOARG(ManageNamespaceDialog, OKHdl, Button*, void)
    {
        OUString sPrefix = m_pPrefixED->GetText();

        try
        {
            if ( !m_pConditionDlg->GetUIHelper()->isValidPrefixName( sPrefix ) )
            {
                ScopedVclPtrInstance< MessageDialog > aErrBox(this, SvxResId( RID_STR_INVALID_XMLPREFIX ) );
                aErrBox->set_primary_text(aErrBox->get_primary_text().replaceFirst(MSG_VARIABLE, sPrefix));
                aErrBox->Execute();
                return;
            }
        }
        catch ( Exception& )
        {
            SAL_WARN( "svx.form", "ManageNamespacesDialog::OKHdl(): exception caught" );
        }

        // no error so close the dialog
        EndDialog( RET_OK );
    }

    AddSubmissionDialog::AddSubmissionDialog(
        vcl::Window* pParent, ItemNode* _pNode,
        const Reference< css::xforms::XFormsUIHelper1 >& _rUIHelper)
        : ModalDialog(pParent, "AddSubmissionDialog",
            "svx/ui/addsubmissiondialog.ui")
        , m_pItemNode(_pNode)
        , m_xUIHelper(_rUIHelper)
    {
        get(m_pNameED, "name");
        get(m_pActionED, "action");
        get(m_pMethodLB, "method");
        get(m_pRefED, "expression");
        get(m_pRefBtn, "browse");
        get(m_pBindLB, "binding");
        get(m_pReplaceLB, "replace");
        get(m_pOKBtn, "ok");
        FillAllBoxes();

        m_pRefBtn->SetClickHdl( LINK( this, AddSubmissionDialog, RefHdl ) );
        m_pOKBtn->SetClickHdl( LINK( this, AddSubmissionDialog, OKHdl ) );
    }


    AddSubmissionDialog::~AddSubmissionDialog()
    {
        disposeOnce();
    }

    void AddSubmissionDialog::dispose()
    {
        // #i38991# if we have added a binding, we need to remove it as well.
        if( m_xCreatedBinding.is() && m_xUIHelper.is() )
            m_xUIHelper->removeBindingIfUseless( m_xCreatedBinding );
        m_pNameED.clear();
        m_pActionED.clear();
        m_pMethodLB.clear();
        m_pRefED.clear();
        m_pRefBtn.clear();
        m_pBindLB.clear();
        m_pReplaceLB.clear();
        m_pOKBtn.clear();
        ModalDialog::dispose();
    }


    IMPL_LINK_NOARG(AddSubmissionDialog, RefHdl, Button*, void)
    {
        ScopedVclPtrInstance< AddConditionDialog > aDlg(this, PN_BINDING_EXPR, m_xTempBinding );
        aDlg->SetCondition( m_pRefED->GetText() );
        if ( aDlg->Execute() == RET_OK )
            m_pRefED->SetText( aDlg->GetCondition() );
    }


    IMPL_LINK_NOARG(AddSubmissionDialog, OKHdl, Button*, void)
    {
        OUString sName(m_pNameED->GetText());
        if(sName.isEmpty()) {

            ScopedVclPtrInstance< MessageDialog > aErrorBox(this,SvxResId(RID_STR_EMPTY_SUBMISSIONNAME));
            aErrorBox->set_primary_text( Application::GetDisplayName() );
            aErrorBox->Execute();
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
                catch ( Exception& )
                {
                    SAL_WARN( "svx.form", "AddSubmissionDialog::OKHdl(): exception caught" );
                }
            }
        }

        if ( m_xSubmission.is() )
        {
            OUString sTemp = m_pNameED->GetText();
            try
            {
                m_xSubmission->setPropertyValue( PN_SUBMISSION_ID, makeAny( sTemp ) );
                sTemp = m_pActionED->GetText();
                m_xSubmission->setPropertyValue( PN_SUBMISSION_ACTION, makeAny( sTemp ) );
                sTemp = m_aMethodString.toAPI( m_pMethodLB->GetSelectedEntry() );
                m_xSubmission->setPropertyValue( PN_SUBMISSION_METHOD, makeAny( sTemp ) );
                sTemp = m_pRefED->GetText();
                m_xSubmission->setPropertyValue( PN_SUBMISSION_REF, makeAny( sTemp ) );
                OUString sEntry = m_pBindLB->GetSelectedEntry();
                sal_Int32 nColonIdx = sEntry.indexOf(':');
                if (nColonIdx != -1)
                    sEntry = sEntry.copy(0, nColonIdx);
                sTemp = sEntry;
                m_xSubmission->setPropertyValue( PN_SUBMISSION_BIND, makeAny( sTemp ) );
                sTemp = m_aReplaceString.toAPI( m_pReplaceLB->GetSelectedEntry() );
                m_xSubmission->setPropertyValue( PN_SUBMISSION_REPLACE, makeAny( sTemp ) );
            }
            catch ( Exception& )
            {
                SAL_WARN( "svx.form", "AddSubmissionDialog::OKHdl(): exception caught" );
            }
        }

        EndDialog( RET_OK );
    }


    void AddSubmissionDialog::FillAllBoxes()
    {
        // method box
        m_pMethodLB->InsertEntry( SvxResId( RID_STR_METHOD_POST   ) );
        m_pMethodLB->InsertEntry( SvxResId( RID_STR_METHOD_PUT ) );
        m_pMethodLB->InsertEntry( SvxResId( RID_STR_METHOD_GET ) );
        m_pMethodLB->SelectEntryPos(0);

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
                                m_pBindLB->InsertEntry( sEntry );

                                if ( !m_xTempBinding.is() )
                                    m_xTempBinding = xPropSet;
                            }
                        }
                    }
                }
            }
            catch ( Exception& )
            {
                SAL_WARN( "svx.form", "AddSubmissionDialog::FillAllBoxes(): exception caught" );
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
        m_pReplaceLB->InsertEntry( SvxResId( RID_STR_REPLACE_NONE ) );
        m_pReplaceLB->InsertEntry( SvxResId( RID_STR_REPLACE_INST ) );
        m_pReplaceLB->InsertEntry( SvxResId( RID_STR_REPLACE_DOC ) );


        // init the controls with the values of the submission
        if ( m_pItemNode && m_pItemNode->m_xPropSet.is() )
        {
            m_xSubmission = m_pItemNode->m_xPropSet;
            OUString sTemp;
            try
            {
                m_xSubmission->getPropertyValue( PN_SUBMISSION_ID ) >>= sTemp;
                m_pNameED->SetText( sTemp );
                m_xSubmission->getPropertyValue( PN_SUBMISSION_ACTION ) >>= sTemp;
                m_pActionED->SetText( sTemp );
                m_xSubmission->getPropertyValue( PN_SUBMISSION_REF ) >>= sTemp;
                m_pRefED->SetText( sTemp );

                m_xSubmission->getPropertyValue( PN_SUBMISSION_METHOD ) >>= sTemp;
                sTemp = m_aMethodString.toUI( sTemp );
                sal_Int32 nPos = m_pMethodLB->GetEntryPos( sTemp );
                if ( LISTBOX_ENTRY_NOTFOUND == nPos )
                    nPos = m_pMethodLB->InsertEntry( sTemp );
                m_pMethodLB->SelectEntryPos( nPos );

                m_xSubmission->getPropertyValue( PN_SUBMISSION_BIND ) >>= sTemp;
                nPos = m_pBindLB->GetEntryPos( sTemp );
                if ( LISTBOX_ENTRY_NOTFOUND == nPos )
                    nPos = m_pBindLB->InsertEntry( sTemp );
                m_pBindLB->SelectEntryPos( nPos );

                m_xSubmission->getPropertyValue( PN_SUBMISSION_REPLACE ) >>= sTemp;
                sTemp = m_aReplaceString.toUI( sTemp );
                if ( sTemp.isEmpty() )
                    sTemp = m_pReplaceLB->GetEntry(0); // first entry == "none"
                nPos = m_pReplaceLB->GetEntryPos( sTemp );
                if ( LISTBOX_ENTRY_NOTFOUND == nPos )
                    nPos = m_pReplaceLB->InsertEntry( sTemp );
                m_pReplaceLB->SelectEntryPos( nPos );
            }
            catch ( Exception& )
            {
                SAL_WARN( "svx.form", "AddSubmissionDialog::FillAllBoxes(): exception caught" );
            }
        }

        m_pRefBtn->Enable( m_xTempBinding.is() );
    }

    AddModelDialog::AddModelDialog(vcl::Window* pParent, bool bIsEdit)
        : ModalDialog(pParent, "AddModelDialog", "svx/ui/addmodeldialog.ui")
    {
        get(m_pNameED, "name");
        get(m_pModifyCB, "modify");

        if (bIsEdit)
            SetText(get<FixedText>("alttitle")->GetText());
    }

    AddModelDialog::~AddModelDialog()
    {
        disposeOnce();
    }

    void AddModelDialog::dispose()
    {
        m_pNameED.clear();
        m_pModifyCB.clear();
        ModalDialog::dispose();
    }

    AddInstanceDialog::AddInstanceDialog(vcl::Window* pParent, bool _bEdit)
        : ModalDialog(pParent, "AddInstanceDialog" , "svx/ui/addinstancedialog.ui")
    {
        get(m_pNameED, "name");
        get(m_pURLFT, "urlft");
        get(m_pURLED, "url");
        get(m_pFilePickerBtn, "browse");
        get(m_pLinkInstanceCB, "link");

        if ( _bEdit )
            SetText(get<FixedText>("alttitle")->GetText());

        m_pURLED->DisableHistory();
        m_pFilePickerBtn->SetClickHdl( LINK( this, AddInstanceDialog, FilePickerHdl ) );

        // load the filter name from fps resource
        m_sAllFilterName = Translate::get(STR_FILTERNAME_ALL, Translate::Create("fps"));
    }

    AddInstanceDialog::~AddInstanceDialog()
    {
        disposeOnce();
    }

    void AddInstanceDialog::dispose()
    {
        m_pNameED.clear();
        m_pURLFT.clear();
        m_pURLED.clear();
        m_pFilePickerBtn.clear();
        m_pLinkInstanceCB.clear();
        ModalDialog::dispose();
    }

    IMPL_LINK_NOARG(AddInstanceDialog, FilePickerHdl, Button*, void)
    {
        ::sfx2::FileDialogHelper aDlg(
            css::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
            FileDialogFlags::NONE, this);
        INetURLObject aFile( SvtPathOptions().GetWorkPath() );

        aDlg.AddFilter( m_sAllFilterName, FILEDIALOG_FILTER_ALL );
        OUString sFilterName( "XML" );
        aDlg.AddFilter( sFilterName, "*.xml" );
        aDlg.SetCurrentFilter( sFilterName );
        aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );

        if( aDlg.Execute() == ERRCODE_NONE )
            m_pURLED->SetText( aDlg.GetPath() );
    }

    LinkedInstanceWarningBox::LinkedInstanceWarningBox( vcl::Window* pParent )
        : MessageDialog(pParent, "FormLinkWarnDialog", "svx/ui/formlinkwarndialog.ui")
    {
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
