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

#include "AppDetailPageHelper.hxx"
#include <tools/diagnose_ex.h>
#include <tabletree.hxx>
#include <dbtreelistbox.hxx>
#include <com/sun/star/awt/XTabController.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/form/XLoadable.hpp>
#include <com/sun/star/frame/thePopupMenuControllerFactory.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/frame/Frame.hpp>
#include <com/sun/star/frame/XFrames.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/sdb/application/XDatabaseDocumentUI.hpp>
#include <com/sun/star/sdb/application/DatabaseObject.hpp>
#include <com/sun/star/sdb/application/DatabaseObjectContainer.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#include <com/sun/star/ucb/Command.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/string.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include "AppView.hxx"
#include <helpids.h>
#include <strings.hxx>
#include <dbaccess_slotid.hrc>
#include <databaseobjectview.hxx>
#include <imageprovider.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/settings.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/event.hxx>
#include <toolkit/awt/vclxmenu.hxx>
#include <tools/stream.hxx>
#include <rtl/ustrbuf.hxx>
#include "AppController.hxx"

#include <com/sun/star/document/XDocumentProperties.hpp>

#include <memory>

using namespace ::dbaui;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdb::application;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star;
using ::com::sun::star::awt::XTabController;

namespace dbaui
{
    namespace DatabaseObject = css::sdb::application::DatabaseObject;
    namespace DatabaseObjectContainer = css::sdb::application::DatabaseObjectContainer;
}

namespace
{
    bool lcl_findEntry_impl(const TreeListBox& rTree, const OUString& rName, weld::TreeIter& rIter)
    {
        bool bReturn = false;
        sal_Int32 nIndex = 0;
        OUString sName( rName.getToken(0,'/',nIndex) );

        const weld::TreeView& rTreeView = rTree.GetWidget();
        bool bEntry = true;
        do
        {
            if (rTreeView.get_text(rIter) == sName)
            {
                if ( nIndex != -1 )
                {
                    sName = rName.getToken(0,'/',nIndex);
                    bEntry = rTreeView.iter_children(rIter);
                }
                else
                {
                    bReturn = true;
                    break;
                }
            }
            else
                bEntry = rTreeView.iter_next_sibling(rIter);
        }
        while (bEntry);

        return bReturn;
    }

    bool lcl_findEntry(const TreeListBox& rTree, const OUString& rName, weld::TreeIter& rIter)
    {
        sal_Int32 nIndex = 0;
        OUString sErase = rName.getToken(0,'/',nIndex); // we don't want to have the "private:forms" part
        return nIndex != -1 && lcl_findEntry_impl(rTree, rName.copy(sErase.getLength() + 1), rIter);
    }

    class OTablePreviewWindow : public vcl::Window
    {
        DECL_LINK(OnDisableInput, void*, void);
        void ImplInitSettings();
    protected:
        virtual void DataChanged(const DataChangedEvent& rDCEvt) override;
    public:
        OTablePreviewWindow( vcl::Window* pParent, WinBits nStyle );
        virtual bool EventNotify( NotifyEvent& rNEvt ) override;
    };
    OTablePreviewWindow::OTablePreviewWindow(vcl::Window* pParent, WinBits nStyle) : Window( pParent, nStyle)
    {
        ImplInitSettings();
    }
    bool OTablePreviewWindow::EventNotify( NotifyEvent& rNEvt )
    {
        bool bRet = Window::EventNotify(rNEvt);
        if ( rNEvt.GetType() == MouseNotifyEvent::INPUTENABLE && IsInputEnabled() )
            PostUserEvent( LINK( this, OTablePreviewWindow, OnDisableInput), nullptr, true );
        return bRet;
    }
    IMPL_LINK_NOARG(OTablePreviewWindow, OnDisableInput, void*, void)
    {
        EnableInput(false);
    }
    void OTablePreviewWindow::DataChanged( const DataChangedEvent& rDCEvt )
    {
        Window::DataChanged( rDCEvt );

        if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
            (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
        {
            ImplInitSettings();
            Invalidate();
        }
    }
    void OTablePreviewWindow::ImplInitSettings()
    {
        //FIXME RenderContext
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        vcl::Font aFont = rStyleSettings.GetFieldFont();
        aFont.SetColor( rStyleSettings.GetWindowTextColor() );
        SetPointFont(*this, aFont);

        SetTextColor( rStyleSettings.GetFieldTextColor() );
        SetTextFillColor();

        SetBackground( rStyleSettings.GetFieldColor() );
    }

}

OAppDetailPageHelper::OAppDetailPageHelper(vcl::Window* pParent, OAppBorderWindow& rBorderWin,PreviewMode ePreviewMode)
    : InterimItemWindow(pParent, "dbaccess/ui/detailwindow.ui", "DetailWindow")
    , m_rBorderWin(rBorderWin)
    , m_xFL(m_xBuilder->weld_widget("separator"))
#if 0
    , m_aTBPreview(VclPtr<ToolBox>::Create(this,WB_TABSTOP) )
    , m_aBorder(VclPtr<Window>::Create(this,WB_BORDER | WB_READONLY))
    , m_aPreview(VclPtr<OPreviewWindow>::Create(m_aBorder.get()))
    , m_aDocumentInfo(VclPtr< ::svtools::ODocumentInfoPreview>::Create(m_aBorder.get(), WB_LEFT | WB_VSCROLL | WB_READONLY) )
#endif
    , m_ePreviewMode(ePreviewMode)
{
#if 0
    m_aBorder->SetBorderStyle(WindowBorderStyle::MONO);

    m_aTBPreview->SetOutStyle(TOOLBOX_STYLE_FLAT);
    auto aProperties = vcl::CommandInfoProvider::GetCommandProperties(".uno:DBDisablePreview",
        "com.sun.star.sdb.OfficeDatabaseDocument");
    m_aTBPreview->InsertItem(SID_DB_APP_DISABLE_PREVIEW,
                             vcl::CommandInfoProvider::GetLabelForCommand(aProperties),
                             ToolBoxItemBits::LEFT|ToolBoxItemBits::DROPDOWNONLY|ToolBoxItemBits::AUTOSIZE|ToolBoxItemBits::RADIOCHECK);
    m_aTBPreview->SetHelpId(HID_APP_VIEW_PREVIEW_CB);
    m_aTBPreview->SetDropdownClickHdl( LINK( this, OAppDetailPageHelper, OnDropdownClickHdl ) );
    m_aTBPreview->Enable();

    m_aPreview->SetHelpId(HID_APP_VIEW_PREVIEW_1);

    m_pTablePreview.set( VclPtr<OTablePreviewWindow>::Create(m_aBorder.get(), WB_READONLY | WB_DIALOGCONTROL ) );
    m_pTablePreview->SetHelpId(HID_APP_VIEW_PREVIEW_2);

    m_aDocumentInfo->SetHelpId(HID_APP_VIEW_PREVIEW_3);

    m_xWindow = VCLUnoHelper::GetInterface( m_pTablePreview );
#endif

    ImplInitSettings();
}

OAppDetailPageHelper::~OAppDetailPageHelper()
{
    disposeOnce();
}

void OAppDetailPageHelper::dispose()
{
    try
    {
        Reference< ::util::XCloseable> xCloseable(m_xFrame,UNO_QUERY);
        if ( xCloseable.is() )
            xCloseable->close(true);
        m_xFrame.clear();
    }
    catch(const Exception&)
    {
        OSL_FAIL("Exception thrown while disposing preview frame!");
    }

    for (auto& rpBox : m_aLists)
    {
        if (!rpBox)
            continue;
//TODO        rpBox->hide();
        rpBox.reset();
    }
#if 0
    m_pTablePreview.disposeAndClear();
    m_aDocumentInfo.disposeAndClear();
    m_aPreview.disposeAndClear();
    m_aBorder.disposeAndClear();
    m_aTBPreview.disposeAndClear();
#endif
    m_xFL.reset();

    InterimItemWindow::dispose();
}

int OAppDetailPageHelper::getVisibleControlIndex() const
{
    int i = 0;
    for (; i < E_ELEMENT_TYPE_COUNT ; ++i)
    {
        if (m_aLists[i] && m_aLists[i]->GetWidget().get_visible())
            break;
    }
    return i;
}

void OAppDetailPageHelper::selectAll()
{
    int nPos = getVisibleControlIndex();
    if (nPos < E_ELEMENT_TYPE_COUNT)
    {
        m_aLists[nPos]->GetWidget().select_all();
    }
}

void OAppDetailPageHelper::sort(int nPos, bool bAscending)
{
    assert(m_aLists[nPos] && "List can not be NULL! ->GPF");
    m_aLists[nPos]->GetWidget().set_sort_order(bAscending);
}

bool OAppDetailPageHelper::isSortUp() const
{
    bool bAscending = false;

    int nPos = getVisibleControlIndex();
    if (nPos < E_ELEMENT_TYPE_COUNT)
        bAscending = m_aLists[nPos]->GetWidget().get_sort_order();

    return bAscending;
}

void OAppDetailPageHelper::sortDown()
{
    int nPos = getVisibleControlIndex();
    if ( nPos < E_ELEMENT_TYPE_COUNT )
        sort(nPos, false);
}

void OAppDetailPageHelper::sortUp()
{
    int nPos = getVisibleControlIndex();
    if ( nPos < E_ELEMENT_TYPE_COUNT )
        sort(nPos, true);
}

void OAppDetailPageHelper::getSelectionElementNames(std::vector<OUString>& rNames) const
{
    int nPos = getVisibleControlIndex();
    if ( nPos >= E_ELEMENT_TYPE_COUNT )
        return;

    DBTreeView& rTree = *m_aLists[nPos];
    weld::TreeView& rTreeView = rTree.GetWidget();
    sal_Int32 nCount = rTreeView.n_children();
    rNames.reserve(nCount);
    ElementType eType = getElementType();

    rTreeView.selected_foreach([this, eType, &rTreeView, &rNames](weld::TreeIter& rEntry){
        if ( eType == E_TABLE )
        {
            if (!rTreeView.iter_has_child(rEntry))
                rNames.push_back(getQualifiedName(&rEntry));
        }
        else
        {
            OUString sName = rTreeView.get_text(rEntry);
            std::unique_ptr<weld::TreeIter> xParent(rTreeView.make_iterator(&rEntry));
            bool bParent = rTreeView.iter_parent(*xParent);
            while (bParent)
            {
                sName = rTreeView.get_text(*xParent) + "/" + sName;
                bParent = rTreeView.iter_parent(*xParent);
            }
            rNames.push_back(sName);
        }

        return false;
    });
}

void OAppDetailPageHelper::describeCurrentSelectionForControl( const Control& _rControl, Sequence< NamedDatabaseObject >& _out_rSelectedObjects )
{
#if 0
    for (size_t i=0; i < E_ELEMENT_TYPE_COUNT; ++i)
    {
        if ( m_aLists[i] == &_rControl )
        {
            describeCurrentSelectionForType(static_cast<ElementType>(i), _out_rSelectedObjects);
            return;
        }
    }
    OSL_FAIL( "OAppDetailPageHelper::describeCurrentSelectionForControl: invalid control!" );
#endif
}

void OAppDetailPageHelper::describeCurrentSelectionForType(const ElementType eType, Sequence< NamedDatabaseObject >& _out_rSelectedObjects)
{
    OSL_ENSURE( eType < E_ELEMENT_TYPE_COUNT, "OAppDetailPageHelper::describeCurrentSelectionForType: invalid type!" );
    DBTreeView* pList = ( eType < E_ELEMENT_TYPE_COUNT ) ? m_aLists[eType].get() : nullptr;
    OSL_ENSURE( pList, "OAppDetailPageHelper::describeCurrentSelectionForType: "
                       "You really should ensure this type has already been viewed before!" );
    if ( !pList )
        return;

    std::vector< NamedDatabaseObject > aSelected;

    weld::TreeView& rTreeView = pList->GetWidget();
    rTreeView.selected_foreach([pList, eType, &rTreeView, &aSelected](weld::TreeIter& rEntry){
        NamedDatabaseObject aObject;
        switch (eType)
        {
            case E_TABLE:
            {
#if 0
                OTableTreeListBox& rTableTree = dynamic_cast<OTableTreeListBox&>(*pList);
                aObject = rTableTree.describeObject(rEntry);
#endif
                break;
            }
            case E_QUERY:
                aObject.Type = DatabaseObject::QUERY;
                aObject.Name = rTreeView.get_text(rEntry);
                break;
            case E_FORM:
            case E_REPORT:
            {
                OUString sName = rTreeView.get_text(rEntry);
                std::unique_ptr<weld::TreeIter> xParent(rTreeView.make_iterator(&rEntry));
                bool bParent = rTreeView.iter_parent(*xParent);
                while (bParent)
                {
                    OUStringBuffer buffer;
                    buffer.append(rTreeView.get_text(*xParent));
                    buffer.append('/');
                    buffer.append(sName);
                    sName = buffer.makeStringAndClear();

                    bParent = rTreeView.iter_parent(*xParent);
                }

                if (isLeaf(rTreeView, rEntry))
                    aObject.Type = (eType == E_FORM) ? DatabaseObject::FORM : DatabaseObject::REPORT;
                else
                    aObject.Type = (eType == E_FORM) ? DatabaseObjectContainer::FORMS_FOLDER : DatabaseObjectContainer::REPORTS_FOLDER;
                aObject.Name = sName;
                break;
            }
            default:
                OSL_FAIL( "OAppDetailPageHelper::describeCurrentSelectionForType: unexpected type!" );
                break;
        }

        if (!aObject.Name.isEmpty())
            aSelected.push_back(aObject);

        return false;
    });

    _out_rSelectedObjects = comphelper::containerToSequence(aSelected);
}

void OAppDetailPageHelper::selectElements(const Sequence< OUString>& _aNames)
{
    int nPos = getVisibleControlIndex();
    if ( nPos >= E_ELEMENT_TYPE_COUNT )
        return;

    DBTreeView& rTree = *m_aLists[nPos];
    weld::TreeView& rTreeView = rTree.GetWidget();
    rTreeView.unselect_all();
    const OUString* pIter = _aNames.getConstArray();
    const OUString* pEnd  = pIter + _aNames.getLength();
    for(;pIter != pEnd;++pIter)
    {
        auto xEntry = rTree.getListBox().GetEntryPosByName(*pIter);
        if (!xEntry)
            continue;
        rTreeView.select(*xEntry);
    }
}

OUString OAppDetailPageHelper::getQualifiedName(weld::TreeIter* _pEntry) const
{
    int nPos = getVisibleControlIndex();
    OUString sComposedName;

    if ( nPos >= E_ELEMENT_TYPE_COUNT )
        return sComposedName;

    OSL_ENSURE(m_aLists[nPos],"Tables tree view is NULL! -> GPF");
    DBTreeView& rTree = *m_aLists[nPos];
    weld::TreeView& rTreeView = rTree.GetWidget();

    std::unique_ptr<weld::TreeIter> xEntry(rTreeView.make_iterator(_pEntry));
    if (!_pEntry)
    {
        if (!rTreeView.get_selected(xEntry.get()))
            xEntry.reset();
    }

    if (!xEntry)
        return sComposedName;

    if ( getElementType() == E_TABLE )
    {
#if 0
        const OTableTreeListBox& rTableTreeListBox = dynamic_cast<const OTableTreeListBox&>(*m_aLists[nPos]);
        sComposedName = rTableTreeListBox.getQualifiedTableName(*xEntry);
#endif
    }
    else
    {
        sComposedName = rTreeView.get_text(*xEntry);
        bool bParent = rTreeView.iter_parent(*xEntry);
        while (bParent)
        {
            sComposedName = rTreeView.get_text(*xEntry) + "/" + sComposedName;
            bParent = rTreeView.iter_parent(*xEntry);
        }
    }

    return sComposedName;
}

ElementType OAppDetailPageHelper::getElementType() const
{
    int nPos = getVisibleControlIndex();
    return static_cast<ElementType>(nPos);
}

sal_Int32 OAppDetailPageHelper::getSelectionCount()
{
    sal_Int32 nCount = 0;
    int nPos = getVisibleControlIndex();
    if ( nPos < E_ELEMENT_TYPE_COUNT )
    {
        DBTreeView& rTree = *m_aLists[nPos];
        weld::TreeView& rTreeView = rTree.GetWidget();
        nCount = rTreeView.count_selected_rows();
    }
    return nCount;
}

sal_Int32 OAppDetailPageHelper::getElementCount() const
{
    sal_Int32 nCount = 0;
    int nPos = getVisibleControlIndex();
    if ( nPos < E_ELEMENT_TYPE_COUNT )
    {
        DBTreeView& rTree = *m_aLists[nPos];
        weld::TreeView& rTreeView = rTree.GetWidget();
        nCount = rTreeView.n_children();
    }
    return nCount;
}

bool OAppDetailPageHelper::isLeaf(const weld::TreeView& rTreeView, const weld::TreeIter& rEntry)
{
    sal_Int32 nEntryType = rTreeView.get_id(rEntry).toInt32();
    return !(   ( nEntryType == DatabaseObjectContainer::TABLES )
             || ( nEntryType == DatabaseObjectContainer::CATALOG )
             || ( nEntryType == DatabaseObjectContainer::SCHEMA )
             || ( nEntryType == DatabaseObjectContainer::FORMS_FOLDER )
             || ( nEntryType == DatabaseObjectContainer::REPORTS_FOLDER ));
}

bool OAppDetailPageHelper::isALeafSelected() const
{
    int nPos = getVisibleControlIndex();
    bool bLeafSelected = false;
    if ( nPos < E_ELEMENT_TYPE_COUNT )
    {
        DBTreeView& rTree = *m_aLists[nPos];
        weld::TreeView& rTreeView = rTree.GetWidget();
        rTreeView.selected_foreach([&rTreeView, &bLeafSelected](weld::TreeIter& rEntry){
            bLeafSelected = isLeaf(rTreeView, rEntry);
            return bLeafSelected;
        });
    }
    return bLeafSelected;
}

std::unique_ptr<weld::TreeIter> OAppDetailPageHelper::getEntry( const Point& _aPosPixel) const
{
    std::unique_ptr<weld::TreeIter> xReturn;
    int nPos = getVisibleControlIndex();
    if ( nPos < E_ELEMENT_TYPE_COUNT )
    {
        DBTreeView& rTree = *m_aLists[nPos];
        weld::TreeView& rTreeView = rTree.GetWidget();
        xReturn = rTreeView.make_iterator();
        if (!rTreeView.get_dest_row_at_pos(_aPosPixel, xReturn.get(), false))
            xReturn.reset();
    }
    return xReturn;
}

void OAppDetailPageHelper::createTablesPage(const Reference< XConnection>& _xConnection)
{
    OSL_ENSURE(_xConnection.is(),"Connection is NULL! -> GPF");

    if ( !m_aLists[E_TABLE] )
    {
        fprintf(stderr, "here ctor\n");
#if 0
        VclPtrInstance<OTableTreeListBox> pTreeView(this, false); // false means: do not show any buttons
        pTreeView->SetHelpId(HID_APP_TABLE_TREE);
        m_aLists[E_TABLE] = createTree(pTreeView);
#endif
#if 0
        m_aBorder->SetZOrder(pTreeView, ZOrderFlags::Behind);
#endif
    }

    weld::TreeView& rTreeView = m_aLists[E_TABLE]->GetWidget();
    if (!rTreeView.n_children())
    {
#if 0
        static_cast<OTableTreeListBox*>(m_aLists[E_TABLE].get())->UpdateTableList(_xConnection);
#endif

        std::unique_ptr<weld::TreeIter> xFirst(rTreeView.make_iterator());
        if (rTreeView.get_iter_first(*xFirst))
            rTreeView.expand_row(*xFirst);
        rTreeView.unselect_all();
    }

    setDetailPage(m_aLists[E_TABLE]->GetWidget());
}

OUString OAppDetailPageHelper::getElementIcons(ElementType _eType)
{
    sal_Int32 nDatabaseObjectType( 0 );
    switch(_eType )
    {
        case E_FORM:    nDatabaseObjectType = DatabaseObject::FORM; break;
        case E_REPORT:  nDatabaseObjectType = DatabaseObject::REPORT; break;
        case E_QUERY:   nDatabaseObjectType = DatabaseObject::QUERY; break;
        default:
            OSL_FAIL( "OAppDetailPageHelper::GetElementIcons: invalid element type!" );
            return OUString();
    }

    return ImageProvider::getDefaultImageResourceID(nDatabaseObjectType);
}

void OAppDetailPageHelper::createPage(ElementType _eType,const Reference< XNameAccess >& _xContainer)
{
    OSL_ENSURE(E_TABLE != _eType,"E_TABLE isn't allowed.");

    OString sHelpId;
    switch( _eType )
    {
        case E_FORM:
            sHelpId = HID_APP_FORM_TREE;
            break;
        case E_REPORT:
            sHelpId = HID_APP_REPORT_TREE;
            break;
        case E_QUERY:
            sHelpId = HID_APP_QUERY_TREE;
            break;
        default:
            OSL_FAIL("Illegal call!");
    }
    OUString sImageId = getElementIcons(_eType);

    if ( !m_aLists[_eType] )
    {
        m_aLists[_eType] = createSimpleTree(sHelpId);
    }

    if ( m_aLists[_eType] )
    {
        weld::TreeView& rTreeView = m_aLists[_eType]->GetWidget();
        if (!rTreeView.n_children() && _xContainer.is())
        {
            rTreeView.make_unsorted();
            fillNames( _xContainer, _eType, sImageId, nullptr );
            rTreeView.make_sorted();

            rTreeView.unselect_all();
        }
        setDetailPage(m_aLists[_eType]->GetWidget());
    }
}

void OAppDetailPageHelper::setDetailPage(weld::Widget& rWidget)
{
    bool bHasFocus = false;

    DBTreeView* pCurrent = getCurrentView();
    if (pCurrent)
    {
        weld::Widget& rCurrent = pCurrent->GetWidget();
        bHasFocus = rCurrent.has_focus(); // TODO
        rCurrent.hide();
    }

    showPreview(nullptr);
    m_xFL->show();
    rWidget.show();
#if 0
    m_aTBPreview->Show();
    m_aBorder->Show();
#endif
    switchPreview(m_ePreviewMode,true);

    if (bHasFocus)
        rWidget.grab_focus();
//TODO    Resize();
}

namespace
{
    namespace DatabaseObjectContainer = ::com::sun::star::sdb::application::DatabaseObjectContainer;

    sal_Int32 lcl_getFolderIndicatorForType( const ElementType _eType )
    {
        const sal_Int32 nFolderIndicator =
                ( _eType == E_FORM ) ? DatabaseObjectContainer::FORMS_FOLDER
            :   ( _eType == E_REPORT ) ? DatabaseObjectContainer::REPORTS_FOLDER : -1;
        return nFolderIndicator;
    }
}

void OAppDetailPageHelper::fillNames( const Reference< XNameAccess >& _xContainer, const ElementType _eType,
                                      const OUString& rImageId, weld::TreeIter* _pParent )
{
    OSL_ENSURE(_xContainer.is(),"Data source is NULL! -> GPF");
    OSL_ENSURE( ( _eType >= E_TABLE ) && ( _eType < E_ELEMENT_TYPE_COUNT ), "OAppDetailPageHelper::fillNames: invalid type!" );

    DBTreeView* pList = m_aLists[_eType].get();
    OSL_ENSURE( pList, "OAppDetailPageHelper::fillNames: you really should create the list before calling this!" );
    if ( !pList )
        return;

    if ( !(_xContainer.is() && _xContainer->hasElements()) )
        return;

    weld::TreeView& rTreeView = pList->GetWidget();

    std::unique_ptr<weld::TreeIter> xRet = rTreeView.make_iterator();
    const sal_Int32 nFolderIndicator = lcl_getFolderIndicatorForType( _eType );

    Sequence< OUString> aSeq = _xContainer->getElementNames();
    const OUString* pIter = aSeq.getConstArray();
    const OUString* pEnd  = pIter + aSeq.getLength();
    for(;pIter != pEnd;++pIter)
    {
        Reference<XNameAccess> xSubElements(_xContainer->getByName(*pIter),UNO_QUERY);
        if ( xSubElements.is() )
        {
            OUString sId(OUString::number(nFolderIndicator));

            rTreeView.insert(_pParent, -1, nullptr, &sId, nullptr, nullptr, false, xRet.get());
            rTreeView.set_text(*xRet, *pIter, 0);
            rTreeView.set_text_emphasis(*xRet, false, 0);
            getBorderWin().getView()->getAppController().containerFound( Reference< XContainer >( xSubElements, UNO_QUERY ) );
            fillNames( xSubElements, _eType, rImageId, xRet.get());
        }
        else
        {
            rTreeView.insert(_pParent, -1, nullptr, nullptr, nullptr, nullptr, false, xRet.get());
            rTreeView.set_text(*xRet, *pIter, 0);
            rTreeView.set_text_emphasis(*xRet, false, 0);
            rTreeView.set_image(*xRet, rImageId);
        }
    }
}

std::unique_ptr<DBTreeView> OAppDetailPageHelper::createSimpleTree(const OString& rHelpId)
{
    std::unique_ptr<DBTreeView> xTreeView(nullptr); // TODO
    xTreeView->GetWidget().set_help_id(rHelpId);
    setupTree(*xTreeView);
    return xTreeView;
}

void OAppDetailPageHelper::setupTree(DBTreeView& rDBTreeView)
{
    weld::WaitObject aWaitCursor(GetFrameWeld());

    rDBTreeView.getListBox().setCopyHandler(LINK(this, OAppDetailPageHelper, OnCopyEntry));
    rDBTreeView.getListBox().setPasteHandler(LINK(this, OAppDetailPageHelper, OnPasteEntry));
    rDBTreeView.getListBox().setDeleteHandler(LINK(this, OAppDetailPageHelper, OnDeleteEntry));

    weld::TreeView& rTreeView = rDBTreeView.GetWidget();
    rTreeView.make_sorted();
    rTreeView.set_selection_mode(SelectionMode::Multiple);

    rTreeView.connect_row_activated(LINK(this, OAppDetailPageHelper, OnEntryDoubleClick));

    rDBTreeView.getListBox().SetSelChangeHdl(LINK(this, OAppDetailPageHelper, OnEntrySelChange));

    rDBTreeView.getListBox().setControlActionListener(&getBorderWin().getView()->getAppController());
    rDBTreeView.getListBox().setContextMenuProvider(&getBorderWin().getView()->getAppController());
}

void OAppDetailPageHelper::clearPages()
{
    showPreview(nullptr);
    for (auto& rpBox : m_aLists)
    {
        if ( rpBox )
            rpBox->GetWidget().clear();
    }
}

bool OAppDetailPageHelper::isFilled() const
{
    size_t i = 0;
    for (; i < E_ELEMENT_TYPE_COUNT && !m_aLists[i]; ++i)
        ;
    return i != E_ELEMENT_TYPE_COUNT;
}

void OAppDetailPageHelper::elementReplaced(ElementType eType,
                                           const OUString& rOldName,
                                           const OUString& rNewName)
{
    DBTreeView* pTreeView = getCurrentView();
    if (!pTreeView)
        return;

    weld::TreeView& rTreeView = pTreeView->GetWidget();
    rTreeView.make_unsorted();

    switch (eType)
    {
        case E_TABLE:
#if 0
            static_cast<OTableTreeListBox*>(pTreeView)->removedTable(rOldName);
            static_cast<OTableTreeListBox*>(pTreeView)->addedTable(rNewName);
#endif
            break;
        case E_QUERY:
        {
            std::unique_ptr<weld::TreeIter> xIter(rTreeView.make_iterator());
            if (rTreeView.get_iter_first(*xIter) && lcl_findEntry_impl(pTreeView->getListBox(), rOldName, *xIter))
                rTreeView.set_text(*xIter, rNewName);
            break;
        }
        case E_FORM:
        case E_REPORT:
        {
            std::unique_ptr<weld::TreeIter> xIter(rTreeView.make_iterator());
            if (rTreeView.get_iter_first(*xIter) && lcl_findEntry(pTreeView->getListBox(), rOldName, *xIter))
                rTreeView.set_text(*xIter, rNewName);
            break;
        }
        default:
            OSL_FAIL("Invalid element type");
    }

    rTreeView.make_sorted();
}

std::unique_ptr<weld::TreeIter> OAppDetailPageHelper::elementAdded(ElementType _eType,const OUString& _rName, const Any& _rObject )
{
    std::unique_ptr<weld::TreeIter> xRet;
    DBTreeView* pTreeView = m_aLists[_eType].get();
    weld::TreeView& rTreeView = pTreeView->GetWidget();
    rTreeView.make_unsorted();
    if( _eType == E_TABLE && pTreeView )
    {
#if 0
        xRet = static_cast<OTableTreeListBox*>(pTreeView)->addedTable( _rName );
#endif
    }
    else if ( pTreeView )
    {
        std::unique_ptr<weld::TreeIter> xEntry;
        Reference<XChild> xChild(_rObject,UNO_QUERY);
        if ( xChild.is() && E_QUERY != _eType )
        {
            Reference<XContent> xContent(xChild->getParent(),UNO_QUERY);
            if ( xContent.is() )
            {
                OUString sName = xContent->getIdentifier()->getContentIdentifier();
                std::unique_ptr<weld::TreeIter> xIter(rTreeView.make_iterator());
                if (rTreeView.get_iter_first(*xIter) && lcl_findEntry(pTreeView->getListBox(), sName, *xIter))
                    xEntry = std::move(xIter);
            }
        }

        OUString sImageId = getElementIcons(_eType);
        Reference<XNameAccess> xContainer(_rObject,UNO_QUERY);
        if ( xContainer.is() )
        {
            const sal_Int32 nFolderIndicator = lcl_getFolderIndicatorForType( _eType );
            OUString sId(OUString::number(nFolderIndicator));

            xRet = rTreeView.make_iterator();
            rTreeView.insert(xEntry.get(), -1, nullptr, &sId, nullptr, nullptr, false, xRet.get());
            rTreeView.set_text(*xRet, _rName, 0);
            rTreeView.set_text_emphasis(*xRet, false, 0);
            fillNames(xContainer, _eType, sImageId, xRet.get());
        }
        else
        {
            xRet = rTreeView.make_iterator();
            rTreeView.insert(xEntry.get(), -1, nullptr, nullptr, nullptr, nullptr, false, xRet.get());
            rTreeView.set_text(*xRet, _rName, 0);
            rTreeView.set_text_emphasis(*xRet, false, 0);
            rTreeView.set_image(*xRet, sImageId);
        }
    }
    rTreeView.make_sorted();
    return xRet;
}

void OAppDetailPageHelper::elementRemoved( ElementType _eType,const OUString& _rName )
{
    DBTreeView* pTreeView = getCurrentView();
    if ( !pTreeView )
        return;

    weld::TreeView& rTreeView = pTreeView->GetWidget();

    switch( _eType )
    {
        case E_TABLE:
#if 0
            // we don't need to clear the table here, it is already done by the dispose listener
            static_cast<OTableTreeListBox*>(pTreeView)->removedTable(_rName);
#endif
            break;
        case E_QUERY:
        {
            std::unique_ptr<weld::TreeIter> xIter(rTreeView.make_iterator());
            if (rTreeView.get_iter_first(*xIter) && lcl_findEntry_impl(pTreeView->getListBox(), _rName, *xIter))
                rTreeView.remove(*xIter);
            break;
        }
        case E_FORM:
        case E_REPORT:
        {
            std::unique_ptr<weld::TreeIter> xIter(rTreeView.make_iterator());
            if (rTreeView.get_iter_first(*xIter) && lcl_findEntry(pTreeView->getListBox(), _rName, *xIter))
                rTreeView.remove(*xIter);
            break;
        }
        default:
            OSL_FAIL("Invalid element type");
    }
    if (!rTreeView.n_children())
        showPreview(nullptr);
}

IMPL_LINK(OAppDetailPageHelper, OnEntryDoubleClick, weld::TreeView&, rTreeView, bool)
{
    return getBorderWin().getView()->getAppController().onEntryDoubleClick(rTreeView);
}

IMPL_LINK_NOARG(OAppDetailPageHelper, OnEntrySelChange, LinkParamNone*, void)
{
    getBorderWin().getView()->getAppController().onSelectionChanged();
}

IMPL_LINK_NOARG( OAppDetailPageHelper, OnCopyEntry, LinkParamNone*, void )
{
    getBorderWin().getView()->getAppController().onCopyEntry();
}

IMPL_LINK_NOARG( OAppDetailPageHelper, OnPasteEntry, LinkParamNone*, void )
{
    getBorderWin().getView()->getAppController().onPasteEntry();
}

IMPL_LINK_NOARG( OAppDetailPageHelper, OnDeleteEntry, LinkParamNone*, void )
{
    getBorderWin().getView()->getAppController().onDeleteEntry();
}

bool OAppDetailPageHelper::isPreviewEnabled() const
{
    return m_ePreviewMode != E_PREVIEWNONE;
}

namespace
{
    OUString stripTrailingDots(const OUString& rStr)
    {
        return comphelper::string::stripEnd(rStr, '.');
    }
}

void OAppDetailPageHelper::switchPreview(PreviewMode _eMode,bool _bForce)
{
#if 0
    if ( !(m_ePreviewMode != _eMode || _bForce) )
        return;

    m_ePreviewMode = _eMode;

    getBorderWin().getView()->getAppController().previewChanged(static_cast<sal_Int32>(m_ePreviewMode));

    OUString aCommand;
    switch ( m_ePreviewMode )
    {
        case E_PREVIEWNONE:
            aCommand = ".uno:DBDisablePreview";
            break;
        case E_DOCUMENT:
            aCommand = ".uno:DBShowDocPreview";
            break;
        case E_DOCUMENTINFO:
            if ( getBorderWin().getView()->getAppController().isCommandEnabled(SID_DB_APP_VIEW_DOCINFO_PREVIEW) )
                aCommand = ".uno:DBShowDocInfoPreview";
            else
            {
                m_ePreviewMode = E_PREVIEWNONE;
                aCommand = ".uno:DBDisablePreview";
            }
            break;
    }

    auto aProperties = vcl::CommandInfoProvider::GetCommandProperties(aCommand, "com.sun.star.sdb.OfficeDatabaseDocument");
    OUString aCommandLabel = vcl::CommandInfoProvider::GetLabelForCommand(aProperties);
    m_aTBPreview->SetItemText(SID_DB_APP_DISABLE_PREVIEW, stripTrailingDots(aCommandLabel));
    Resize();

    // simulate a selectionChanged event at the controller, to force the preview to be updated
    if ( isPreviewEnabled() )
    {
        DBTreeView* pCurrent = getCurrentView();
        if (pCurrent && pCurrent->GetWidget().get_selected(nullptr))
        {
            getBorderWin().getView()->getAppController().onSelectionChanged();
        }
    }
    else
    {
        m_pTablePreview->Hide();
        m_aPreview->Hide();
        m_aDocumentInfo->Hide();
    }
#endif
}

void OAppDetailPageHelper::showPreview(const Reference< XContent >& _xContent)
{
#if 0
    if ( !isPreviewEnabled() )
        return;

    m_pTablePreview->Hide();

    weld::WaitObject aWaitCursor(GetFrameWeld());
    try
    {
        Reference<XCommandProcessor> xContent(_xContent,UNO_QUERY);
        if ( xContent.is() )
        {
            css::ucb::Command aCommand;
            if ( m_ePreviewMode == E_DOCUMENT )
                aCommand.Name = "preview";
            else
                aCommand.Name = "getDocumentInfo";

            Any aPreview = xContent->execute(aCommand,xContent->createCommandIdentifier(),Reference< XCommandEnvironment >());
            if ( m_ePreviewMode == E_DOCUMENT )
            {
                m_aDocumentInfo->Hide();
                m_aPreview->Show();

                Graphic aGraphic;
                Sequence < sal_Int8 > aBmpSequence;
                if ( aPreview >>= aBmpSequence )
                {
                    SvMemoryStream  aData( aBmpSequence.getArray(),
                                        aBmpSequence.getLength(),
                                        StreamMode::READ );

                    GraphicConverter::Import(aData,aGraphic);
                }
                m_aPreview->setGraphic( aGraphic );
                m_aPreview->Invalidate();
            }
            else
            {
                m_aPreview->Hide();
                m_aDocumentInfo->clear();
                m_aDocumentInfo->Show();
                Reference<document::XDocumentProperties> xProp(
                    aPreview, UNO_QUERY);
                if ( xProp.is() )
                    m_aDocumentInfo->fill(xProp);
            }
        }
        else
        {
            m_aPreview->Hide();
            m_aDocumentInfo->Hide();
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
#endif
}

void OAppDetailPageHelper::showPreview( const OUString& _sDataSourceName,
                                        const OUString& _sName,
                                        bool _bTable)
{
#if 0
    if ( !isPreviewEnabled() )
        return;

    weld::WaitObject aWaitCursor(GetFrameWeld());
    m_aPreview->Hide();
    m_aDocumentInfo->Hide();
    m_pTablePreview->Show();
    if ( !m_xFrame.is() )
    {
        try
        {
            m_xFrame = Frame::create( getBorderWin().getView()->getORB() );
            m_xFrame->initialize( m_xWindow );

            // no layout manager (and thus no toolbars) in the preview
            // Must be called after initialize ... but before any other call to this frame.
            // Otherwise frame throws "life time exceptions" as e.g. NON_INITIALIZED
            m_xFrame->setLayoutManager( Reference< XLayoutManager >() );

            Reference<XFramesSupplier> xSup(getBorderWin().getView()->getAppController().getXController()->getFrame(),UNO_QUERY);
            if ( xSup.is() )
            {
                Reference<XFrames> xFrames = xSup->getFrames();
                xFrames->append( Reference<XFrame>(m_xFrame,UNO_QUERY_THROW));
            }
        }
        catch(const Exception&)
        {
        }
    }

    Reference< XDatabaseDocumentUI > xApplication( getBorderWin().getView()->getAppController().getXController(), UNO_QUERY );
    std::unique_ptr< DatabaseObjectView > pDispatcher( new ResultSetBrowser(
        getBorderWin().getView()->getORB(),
        xApplication, nullptr, _bTable
    ) );
    pDispatcher->setTargetFrame( Reference<XFrame>(m_xFrame,UNO_QUERY_THROW) );

    ::comphelper::NamedValueCollection aArgs;
    aArgs.put( "Preview", true );
    aArgs.put( "ReadOnly", true );
    aArgs.put( "AsTemplate", false );
    aArgs.put( OUString(PROPERTY_SHOWMENU), false );

    Reference< XController > xPreview( pDispatcher->openExisting( makeAny( _sDataSourceName ), _sName, aArgs ), UNO_QUERY );
    bool bClearPreview = !xPreview.is();

    // clear the preview when the query or table could not be loaded
    if ( !bClearPreview )
    {
        Reference< XTabController > xTabController( xPreview, UNO_QUERY );
        bClearPreview = !xTabController.is();
        if ( !bClearPreview )
        {
            Reference< XLoadable > xLoadable( xTabController->getModel(), UNO_QUERY );
            bClearPreview = !( xLoadable.is() && xLoadable->isLoaded() );
        }
    }
    if ( bClearPreview )
        showPreview(nullptr);
#endif
}

IMPL_LINK_NOARG(OAppDetailPageHelper, OnDropdownClickHdl, ToolBox*, void)
{
#if 0
    m_aTBPreview->EndSelection();

    // tell the toolbox that the item is pressed down
    m_aTBPreview->SetItemDown( SID_DB_APP_DISABLE_PREVIEW, true );

    // simulate a mouse move (so the "down" state is really painted)
    Point aPoint = m_aTBPreview->GetItemRect( SID_DB_APP_DISABLE_PREVIEW ).TopLeft();
    MouseEvent aMove( aPoint, 0, MouseEventModifiers::SIMPLEMOVE | MouseEventModifiers::SYNTHETIC );
    m_aTBPreview->MouseMove( aMove );

    m_aTBPreview->PaintImmediately();

    // execute the menu
    css::uno::Reference<css::uno::XComponentContext> xContext(getBorderWin().getView()->getORB());
    css::uno::Reference<css::frame::XUIControllerFactory> xPopupMenuFactory(css::frame::thePopupMenuControllerFactory::get(xContext));
    if (!xPopupMenuFactory.is())
        return;

    css::uno::Sequence<css::uno::Any> aArgs {
        css::uno::makeAny(comphelper::makePropertyValue("InToolbar", true)),
        css::uno::makeAny(comphelper::makePropertyValue("ModuleIdentifier", OUString("com.sun.star.sdb.OfficeDatabaseDocument"))),
        css::uno::makeAny(comphelper::makePropertyValue("Frame", getBorderWin().getView()->getAppController().getFrame())) };

    css::uno::Reference<css::frame::XPopupMenuController> xPopupController(
        xPopupMenuFactory->createInstanceWithArgumentsAndContext(".uno:DBPreview", aArgs, xContext), css::uno::UNO_QUERY);

    if (!xPopupController.is())
        return;

    rtl::Reference xPopupMenu(new VCLXPopupMenu);
    xPopupController->setPopupMenu(xPopupMenu.get());
    VclPtr<PopupMenu> aMenu(static_cast<PopupMenu*>(xPopupMenu->GetMenu()));

    sal_uInt16 nSelectedAction = aMenu->Execute(m_aTBPreview.get(), m_aTBPreview->GetItemRect( SID_DB_APP_DISABLE_PREVIEW ));
    // "cleanup" the toolbox state
    MouseEvent aLeave( aPoint, 0, MouseEventModifiers::LEAVEWINDOW | MouseEventModifiers::SYNTHETIC );
    m_aTBPreview->MouseMove( aLeave );
    m_aTBPreview->SetItemDown( SID_DB_APP_DISABLE_PREVIEW, false);
    if ( nSelectedAction )
    {
        m_aTBPreview->SetItemText(SID_DB_APP_DISABLE_PREVIEW, stripTrailingDots(aMenu->GetItemText(nSelectedAction)));
        Resize();
    }

    css::uno::Reference<css::lang::XComponent> xComponent(xPopupController, css::uno::UNO_QUERY);
    if (xComponent.is())
        xComponent->dispose();
#endif
}

void OAppDetailPageHelper::KeyInput( const KeyEvent& rKEvt )
{
    DBTreeView* pCurrentView = getCurrentView();
    OSL_PRECOND( pCurrentView, "OAppDetailPageHelper::KeyInput: how this?" );

    sal_uInt16      nCode = rKEvt.GetKeyCode().GetCode();

    if ( ( KEY_RETURN == nCode ) && pCurrentView )
    {
        getBorderWin().getView()->getAppController().onEntryDoubleClick(pCurrentView->GetWidget());
    }
    else
        Window::KeyInput(rKEvt);
}

void OAppDetailPageHelper::ImplInitSettings()
{
#if 0
    // FIXME RenderContext
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    vcl::Font aFont = rStyleSettings.GetFieldFont();
    aFont.SetColor( rStyleSettings.GetWindowTextColor() );
    SetPointFont(*this, aFont);
    m_aTBPreview->SetPointFont(*m_aTBPreview, aFont);

    SetTextColor( rStyleSettings.GetFieldTextColor() );
    SetTextFillColor();
    m_aBorder->SetTextColor( rStyleSettings.GetFieldTextColor() );
    m_aBorder->SetTextFillColor();
    m_aTBPreview->SetTextColor( rStyleSettings.GetFieldTextColor() );
    m_aTBPreview->SetTextFillColor();
    SetBackground( rStyleSettings.GetFieldColor() );
    m_aBorder->SetBackground( rStyleSettings.GetFieldColor() );
    m_aDocumentInfo->SetBackground( rStyleSettings.GetFieldColor() );
    m_aTBPreview->SetBackground( rStyleSettings.GetFieldColor() );
    m_pTablePreview->SetBackground( rStyleSettings.GetFieldColor() );
#endif
}

OPreviewWindow::OPreviewWindow(vcl::Window* _pParent)
: Window(_pParent)
{
    ImplInitSettings();
}

bool OPreviewWindow::ImplGetGraphicCenterRect( const Graphic& rGraphic, tools::Rectangle& rResultRect ) const
{
    const Size aWinSize( GetOutputSizePixel() );
    Size       aNewSize( LogicToPixel( rGraphic.GetPrefSize(), rGraphic.GetPrefMapMode() ) );
    bool       bRet = false;

    if( aNewSize.Width() && aNewSize.Height() )
    {
        // scale to fit window
        const double fGrfWH = static_cast<double>(aNewSize.Width()) / aNewSize.Height();
        const double fWinWH = static_cast<double>(aWinSize.Width()) / aWinSize.Height();

        if ( fGrfWH < fWinWH )
        {
            aNewSize.setWidth( static_cast<long>( aWinSize.Height() * fGrfWH ) );
            aNewSize.setHeight( aWinSize.Height() );
        }
        else
        {
            aNewSize.setWidth( aWinSize.Width() );
            aNewSize.setHeight( static_cast<long>( aWinSize.Width() / fGrfWH) );
        }

        const Point aNewPos( ( aWinSize.Width()  - aNewSize.Width() ) >> 1,
                             ( aWinSize.Height() - aNewSize.Height() ) >> 1 );

        rResultRect = tools::Rectangle( aNewPos, aNewSize );
        bRet = true;
    }

    return bRet;
}

void OPreviewWindow::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    Window::Paint(rRenderContext, rRect);

    if (ImplGetGraphicCenterRect(m_aGraphicObj.GetGraphic(), m_aPreviewRect))
    {
        const Point aPos(m_aPreviewRect.TopLeft());
        const Size  aSize(m_aPreviewRect.GetSize());

        if (m_aGraphicObj.IsAnimated())
            m_aGraphicObj.StartAnimation(&rRenderContext, aPos, aSize);
        else
            m_aGraphicObj.Draw(&rRenderContext, aPos, aSize);
    }
}

void OPreviewWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
}

void OPreviewWindow::ImplInitSettings()
{
    // FIXME RenderContext
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    vcl::Font aFont = rStyleSettings.GetFieldFont();
    aFont.SetColor( rStyleSettings.GetWindowTextColor() );
    SetPointFont(*this, aFont);

    SetTextColor( rStyleSettings.GetFieldTextColor() );
    SetTextFillColor();

    SetBackground( rStyleSettings.GetFieldColor() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
