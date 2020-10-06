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
#ifndef INCLUDED_SVX_SOURCE_INC_DATANAVI_HXX
#define INCLUDED_SVX_SOURCE_INC_DATANAVI_HXX

#include <config_options.h>
#include <vcl/builderpage.hxx>
#include <vcl/idle.hxx>
#include <vcl/transfer.hxx>
#include <vcl/weld.hxx>
#include <svtools/inettbc.hxx>
#include <sfx2/dockwin.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/ctrlitem.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <svx/svxdllapi.h>
#include <rtl/ref.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/xforms/XFormsUIHelper1.hpp>
#include <com/sun/star/xforms/XModel.hpp>
#include <com/sun/star/xforms/XSubmission.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/events/XEventTarget.hpp>

#include "datalistener.hxx"

#include <memory>
#include <vector>

class FmFormShell;


namespace svxform
{


    enum DataGroupType
    {
        DGTUnknown = 0,
        DGTInstance,
        DGTSubmission,
        DGTBinding
    };

    enum DataItemType
    {
        DITNone = 0,
        DITText,
        DITAttribute,
        DITElement,
        DITBinding
    };

    struct ItemNode;
    class XFormsPage;
    class DataNavigatorWindow;
    class AddInstanceDialog;

    class ReplaceString
    {
        OUString m_sDoc_UI;
        OUString m_sInstance_UI;
        OUString m_sNone_UI;

        static constexpr OUStringLiteral m_sDoc_API = u"all";
        static constexpr OUStringLiteral m_sInstance_API = u"instance";
        static constexpr OUStringLiteral m_sNone_API = u"none";

        ReplaceString( const ReplaceString& ) = delete;

    public:
        ReplaceString()
        {
            m_sDoc_UI = SvxResId(RID_STR_REPLACE_DOC);
            m_sInstance_UI = SvxResId(RID_STR_REPLACE_INST);
            m_sNone_UI = SvxResId(RID_STR_REPLACE_NONE);
        }

        /** convert submission replace string from API value to UI value.
            Use 'none' as default. */
        OUString const & toUI( const OUString& rStr ) const
        {
            if( rStr == m_sDoc_API )
                return m_sDoc_UI;
            else if( rStr == m_sInstance_API )
                return m_sInstance_UI;
            else
                return m_sNone_UI;
        }

        /** convert submission replace string from UI to API.
            Use 'none' as default. */
        OUString toAPI( const OUString& rStr ) const
        {
            if( rStr == m_sDoc_UI )
                return m_sDoc_API;
            else if( rStr == m_sInstance_UI )
                return m_sInstance_API;
            else
                return m_sNone_API;
        }
    };

    class MethodString
    {
        OUString m_sPost_UI;
        OUString m_sPut_UI;
        OUString m_sGet_UI;

        static constexpr OUStringLiteral m_sPost_API = u"post";
        static constexpr OUStringLiteral m_sPut_API = u"put";
        static constexpr OUStringLiteral m_sGet_API = u"get";

        MethodString( const MethodString& ) = delete;

    public:

        MethodString()
        {
            m_sPost_UI = SvxResId(RID_STR_METHOD_POST);
            m_sPut_UI  = SvxResId(RID_STR_METHOD_PUT);
            m_sGet_UI  = SvxResId(RID_STR_METHOD_GET);
        }

        /** convert from API to UI; put is default. */
        OUString const & toUI( const OUString& rStr ) const
        {
            if( rStr == m_sGet_API )
                return m_sGet_UI;
            else if( rStr == m_sPost_API )
                return m_sPost_UI;
            else
                return m_sPut_UI;
        }

        /** convert from UI to API; put is default */
        OUString toAPI( const OUString& rStr ) const
        {
            if( rStr == m_sGet_UI )
                return m_sGet_API;
            else if( rStr == m_sPost_UI )
                return m_sPost_API;
            else
                return m_sPut_API;
        }
    };

    class DataTreeDropTarget : public DropTargetHelper
    {
    private:
        virtual sal_Int8 AcceptDrop(const AcceptDropEvent& rEvt) override;
        virtual sal_Int8 ExecuteDrop(const ExecuteDropEvent& rEvt) override;

    public:
        DataTreeDropTarget(weld::TreeView& rWidget);
    };

    class XFormsPage : public BuilderPage
    {
    private:
        MethodString                m_aMethodString;
        ReplaceString               m_aReplaceString;

        weld::Container* m_pParent;
        std::unique_ptr<weld::Toolbar> m_xToolBox;
        std::unique_ptr<weld::TreeView> m_xItemList;
        std::unique_ptr<weld::TreeIter> m_xScratchIter;

        DataTreeDropTarget m_aDropHelper;

        css::uno::Reference< css::xforms::XFormsUIHelper1 >
                                    m_xUIHelper;

        DataNavigatorWindow*        m_pNaviWin;
        bool                        m_bHasModel;
        DataGroupType               m_eGroup;
        // these strings are not valid on the Submission and Binding Page
        // mb: furthermore these are properties of an instance, thus
        // it would be much better to get/set them through the UIHelper
        // interface.
        OUString                    m_sInstanceName;
        OUString                    m_sInstanceURL;
        bool                        m_bLinkOnce;

        DECL_LINK(TbxSelectHdl, const OString&, void);
        DECL_LINK(ItemSelectHdl, weld::TreeView&, void);
        DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
        DECL_LINK(PopupMenuHdl, const CommandEvent&, bool);

        void                        AddChildren(const weld::TreeIter* _pParent,
                                                const css::uno::Reference< css::xml::dom::XNode >& _xNode);
        bool                        DoToolBoxAction(const OString& rToolBoxID);
        void                        AddEntry(std::unique_ptr<ItemNode> _pNewNode, bool _bIsElement, weld::TreeIter* pRet = nullptr);
        void                        AddEntry(const css::uno::Reference< css::beans::XPropertySet >& _rPropSet, weld::TreeIter* pRet = nullptr);
        void                        EditEntry( const css::uno::Reference< css::beans::XPropertySet >& _rPropSet );
        bool                        RemoveEntry();

        void                        PrepDnD();

        void                        DeleteAndClearTree();

    public:
        XFormsPage(weld::Container* pParent, DataNavigatorWindow* _pNaviWin, DataGroupType _eGroup);
        virtual ~XFormsPage() override;

        bool                 HasModel() const { return m_bHasModel; }
        OUString             SetModel( const css::uno::Reference< css::xforms::XModel > & _xModel, int _nPagePos );
        void                 ClearModel();
        OUString             LoadInstance(const css::uno::Sequence< css::beans::PropertyValue >& _xPropSeq);

        bool                 DoMenuAction(const OString& rMenuID);
        void                 EnableMenuItems(weld::Menu* pMenu);

        const OUString&      GetInstanceName() const { return m_sInstanceName; }
        const OUString&      GetInstanceURL() const { return m_sInstanceURL; }
        bool                 GetLinkOnce() const { return m_bLinkOnce; }
        void                 SetInstanceName( const OUString &name ) { m_sInstanceName=name; }
        void                 SetInstanceURL( const OUString &url ) { m_sInstanceURL=url; }
        void                 SetLinkOnce( bool bLinkOnce ) { m_bLinkOnce=bLinkOnce; }

        css::uno::Reference<css::beans::XPropertySet>
                             GetBindingForNode( const css::uno::Reference<css::xml::dom::XNode> &xNode ) { return m_xUIHelper->getBindingForNode(xNode,true); }
        OUString             GetServiceNameForNode( const css::uno::Reference<css::xml::dom::XNode> &xNode ) { return m_xUIHelper->getDefaultServiceNameForNode(xNode); }
        const css::uno::Reference< css::xforms::XFormsUIHelper1 >&
                             GetXFormsHelper() const { return m_xUIHelper; }
    };

    class DataNavigatorWindow final
    {
    private:
        VclPtr<vcl::Window> m_xParent;
        std::unique_ptr<weld::ComboBox> m_xModelsBox;
        std::unique_ptr<weld::MenuButton> m_xModelBtn;
        std::unique_ptr<weld::Notebook> m_xTabCtrl;
        std::unique_ptr<weld::MenuButton> m_xInstanceBtn;

        std::unique_ptr<XFormsPage> m_xInstPage;
        std::unique_ptr<XFormsPage> m_xSubmissionPage;
        std::unique_ptr<XFormsPage> m_xBindingPage;

        sal_Int32                   m_nLastSelectedPos;
        bool                        m_bShowDetails;
        bool                        m_bIsNotifyDisabled;
        std::vector< std::unique_ptr<XFormsPage> >
                                    m_aPageList;
        std::vector< css::uno::Reference< css::container::XContainer >  >
                                    m_aContainerList;
        std::vector< css::uno::Reference< css::xml::dom::events::XEventTarget > >
                                    m_aEventTargetList;
        Timer                       m_aUpdateTimer;

        ::rtl::Reference < DataListener >
                                    m_xDataListener;
        css::uno::Reference< css::container::XNameContainer >
                                    m_xDataContainer;
        css::uno::Reference< css::frame::XFrame >
                                    m_xFrame;
        css::uno::Reference< css::frame::XModel >
                                    m_xFrameModel;

        DECL_LINK(            ModelSelectListBoxHdl, weld::ComboBox&, void );
        DECL_LINK(            MenuSelectHdl, const OString&, void );
        DECL_LINK(            MenuActivateHdl, weld::ToggleButton&, void );
        DECL_LINK(            ActivatePageHdl, const OString&, void);
        DECL_LINK(            UpdateHdl, Timer *, void);
        void ModelSelectHdl(const weld::ComboBox*);
        OString                     GetCurrentPage() const;
        XFormsPage*                 GetPage(const OString& rCurId);
        void                        LoadModels();
        void                        SetPageModel(const OString& rCurId);
        void                        ClearAllPageModels( bool bClearPages );
        void                        InitPages();
        void                        CreateInstancePage( const css::uno::Sequence< css::beans::PropertyValue >& _xPropSeq );
        bool                        HasFirstInstancePage() const;
        OString                     GetNewPageId() const;

        static bool                 IsAdditionalPage(const OString& rIdent);

    public:
        DataNavigatorWindow(vcl::Window* pParent, weld::Builder& rBuilder, SfxBindings const * pBindings);
        ~DataNavigatorWindow();

        static void                 SetDocModified();
        void                        NotifyChanges( bool _bLoadAll = false );
        void                        AddContainerBroadcaster( const css::uno::Reference< css::container::XContainer > & xContainer );
        void                        AddEventBroadcaster( const css::uno::Reference< css::xml::dom::events::XEventTarget >& xTarget );
        void                        RemoveBroadcaster();

        weld::Window*               GetFrameWeld() const { return m_xParent->GetFrameWeld(); }

        bool                        IsShowDetails() const { return m_bShowDetails; }
        void                        DisableNotify( bool _bDisable ) { m_bIsNotifyDisabled = _bDisable; }
    };

    class DataNavigator : public SfxDockingWindow, public SfxControllerItem
    {
    private:
        std::unique_ptr<DataNavigatorWindow> m_xDataWin;

    protected:
        virtual Size                CalcDockingSize( SfxChildAlignment ) override;
        virtual SfxChildAlignment   CheckAlignment( SfxChildAlignment, SfxChildAlignment ) override;

    public:
        DataNavigator( SfxBindings* pBindings, SfxChildWindow* pMgr, vcl::Window* pParent );
        virtual ~DataNavigator() override;
        virtual void dispose() override;

        using                       SfxDockingWindow::StateChanged;

        void                        StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                                  const SfxPoolItem* pState ) override;
    };

    class UNLESS_MERGELIBS(SVXCORE_DLLPUBLIC) DataNavigatorManager : public SfxChildWindow
    {
    public:
        SVX_DLLPRIVATE DataNavigatorManager( vcl::Window* pParent, sal_uInt16 nId,
                              SfxBindings* pBindings, SfxChildWinInfo* pInfo );
        SFX_DECL_CHILDWINDOW( DataNavigatorManager );
    };

    class AddDataItemDialog : public weld::GenericDialogController
    {
    private:
        css::uno::Reference< css::xforms::XFormsUIHelper1 >
                            m_xUIHelper;
        css::uno::Reference< css::beans::XPropertySet >
                            m_xBinding;
        css::uno::Reference< css::beans::XPropertySet >
                            m_xTempBinding;

        ItemNode*           m_pItemNode;
        DataItemType        m_eItemType;
        OUString            m_sFL_Element;
        OUString            m_sFL_Attribute;
        OUString            m_sFL_Binding;
        OUString            m_sFT_BindingExp;

        std::unique_ptr<weld::Frame> m_xItemFrame;
        std::unique_ptr<weld::Label> m_xNameFT;
        std::unique_ptr<weld::Entry> m_xNameED;
        std::unique_ptr<weld::Label> m_xDefaultFT;
        std::unique_ptr<weld::Entry> m_xDefaultED;
        std::unique_ptr<weld::Button> m_xDefaultBtn;
        std::unique_ptr<weld::Widget> m_xSettingsFrame;
        std::unique_ptr<weld::Label> m_xDataTypeFT;
        std::unique_ptr<weld::ComboBox> m_xDataTypeLB;
        std::unique_ptr<weld::CheckButton> m_xRequiredCB;
        std::unique_ptr<weld::Button> m_xRequiredBtn;
        std::unique_ptr<weld::CheckButton> m_xRelevantCB;
        std::unique_ptr<weld::Button> m_xRelevantBtn;
        std::unique_ptr<weld::CheckButton> m_xConstraintCB;
        std::unique_ptr<weld::Button> m_xConstraintBtn;
        std::unique_ptr<weld::CheckButton> m_xReadonlyCB;
        std::unique_ptr<weld::Button> m_xReadonlyBtn;
        std::unique_ptr<weld::CheckButton> m_xCalculateCB;
        std::unique_ptr<weld::Button> m_xCalculateBtn;
        std::unique_ptr<weld::Button> m_xOKBtn;

        void Check(const weld::ToggleButton* pBox);
        DECL_LINK(CheckHdl, weld::ToggleButton&, void);
        DECL_LINK(ConditionHdl, weld::Button&, void);
        DECL_LINK(OKHdl, weld::Button&, void);

        void                InitDialog();
        void                InitFromNode();
        void                InitDataTypeBox();

    public:
        AddDataItemDialog(
            weld::Window* pParent, ItemNode* _pNode,
            const css::uno::Reference< css::xforms::XFormsUIHelper1 >& _rUIHelper );
        virtual ~AddDataItemDialog() override;

        void                InitText( DataItemType _eType );
    };

    class AddConditionDialog : public weld::GenericDialogController
    {
    private:
        Idle                           m_aResultIdle;
        OUString                       m_sPropertyName;

        css::uno::Reference< css::xforms::XFormsUIHelper1 >
                                       m_xUIHelper;
        css::uno::Reference< css::beans::XPropertySet >
                                       m_xBinding;

        std::unique_ptr<weld::TextView> m_xConditionED;
        std::unique_ptr<weld::TextView> m_xResultWin;
        std::unique_ptr<weld::Button> m_xEditNamespacesBtn;
        std::unique_ptr<weld::Button> m_xOKBtn;

        DECL_LINK(ModifyHdl, weld::TextView&, void);
        DECL_LINK(ResultHdl, Timer *, void);
        DECL_LINK(EditHdl, weld::Button&, void);
        DECL_LINK(OKHdl, weld::Button&, void);

    public:
        AddConditionDialog(weld::Window* pParent,
            const OUString& _rPropertyName, const css::uno::Reference< css::beans::XPropertySet >& _rBinding);
        virtual ~AddConditionDialog() override;

        const css::uno::Reference< css::xforms::XFormsUIHelper1 >& GetUIHelper() const { return m_xUIHelper; }
        OUString GetCondition() const { return m_xConditionED->get_text(); }
        void SetCondition(const OUString& _rCondition)
        {
            m_xConditionED->set_text(_rCondition);
            m_aResultIdle.Start();
        }
    };

    class NamespaceItemDialog : public weld::GenericDialogController
    {
    private:
        AddConditionDialog* m_pConditionDlg;
        std::vector< OUString >    m_aRemovedList;

        css::uno::Reference< css::container::XNameContainer >&
                                   m_rNamespaces;

        std::unique_ptr<weld::TreeView> m_xNamespacesList;
        std::unique_ptr<weld::Button> m_xAddNamespaceBtn;
        std::unique_ptr<weld::Button> m_xEditNamespaceBtn;
        std::unique_ptr<weld::Button> m_xDeleteNamespaceBtn;
        std::unique_ptr<weld::Button> m_xOKBtn;

        DECL_LINK(SelectHdl, weld::TreeView&, void);
        DECL_LINK(ClickHdl, weld::Button&, void);
        DECL_LINK(OKHdl, weld::Button&, void);

        void                LoadNamespaces();

    public:
        NamespaceItemDialog(AddConditionDialog* pParent, css::uno::Reference< css::container::XNameContainer >& _rContainer);
        virtual ~NamespaceItemDialog() override;
    };

    class ManageNamespaceDialog : public weld::GenericDialogController
    {
    private:
        AddConditionDialog* m_pConditionDlg;

        std::unique_ptr<weld::Entry> m_xPrefixED;
        std::unique_ptr<weld::Entry> m_xUrlED;
        std::unique_ptr<weld::Button> m_xOKBtn;
        std::unique_ptr<weld::Label> m_xAltTitle;

        DECL_LINK(OKHdl, weld::Button&, void);

    public:
        ManageNamespaceDialog(weld::Window* pParent, AddConditionDialog* _pCondDlg, bool bIsEdit);
        virtual ~ManageNamespaceDialog() override;

        void SetNamespace(const OUString& _rPrefix, const OUString& _rURL)
        {
            m_xPrefixED->set_text(_rPrefix);
            m_xUrlED->set_text(_rURL);
        }
        OUString GetPrefix() const { return m_xPrefixED->get_text(); }
        OUString GetURL() const { return m_xUrlED->get_text(); }
    };

    class AddSubmissionDialog : public weld::GenericDialogController
    {
    private:
        MethodString         m_aMethodString;
        ReplaceString        m_aReplaceString;

        ItemNode*            m_pItemNode;

        css::uno::Reference< css::xforms::XFormsUIHelper1 >
                            m_xUIHelper;
        css::uno::Reference< css::xforms::XSubmission >
                            m_xNewSubmission;
        css::uno::Reference< css::beans::XPropertySet >
                            m_xSubmission;
        css::uno::Reference< css::beans::XPropertySet >
                            m_xTempBinding;
        css::uno::Reference< css::beans::XPropertySet >
                            m_xCreatedBinding;

        std::unique_ptr<weld::Entry> m_xNameED;
        std::unique_ptr<weld::Entry> m_xActionED;
        std::unique_ptr<weld::ComboBox> m_xMethodLB;
        std::unique_ptr<weld::Entry> m_xRefED;
        std::unique_ptr<weld::Button> m_xRefBtn;
        std::unique_ptr<weld::ComboBox> m_xBindLB;
        std::unique_ptr<weld::ComboBox> m_xReplaceLB;
        std::unique_ptr<weld::Button> m_xOKBtn;

        DECL_LINK(RefHdl, weld::Button&, void);
        DECL_LINK(OKHdl, weld::Button&, void);

        void                FillAllBoxes();

    public:
        AddSubmissionDialog(weld::Window* pParent, ItemNode* pNode,
            const css::uno::Reference< css::xforms::XFormsUIHelper1 >& rUIHelper);
        virtual ~AddSubmissionDialog() override;

        const css::uno::Reference< css::xforms::XSubmission >& GetNewSubmission() const { return m_xNewSubmission; }
    };

    class AddModelDialog : public weld::GenericDialogController
    {
    private:
        std::unique_ptr<weld::Entry> m_xNameED;
        std::unique_ptr<weld::CheckButton> m_xModifyCB;
        std::unique_ptr<weld::Label> m_xAltTitle;

    public:
        AddModelDialog(weld::Window* pParent, bool _bEdit);
        virtual ~AddModelDialog() override;

        OUString         GetName() const { return m_xNameED->get_text(); }
        void             SetName( const OUString& _rName ) { m_xNameED->set_text( _rName );}

        bool             GetModifyDoc() const { return m_xModifyCB->get_active(); }
        void             SetModifyDoc( const bool bModify ) { m_xModifyCB->set_active(bModify); }
    };

    class AddInstanceDialog : public weld::GenericDialogController
    {
    private:
        OUString                m_sAllFilterName;

        std::unique_ptr<weld::Entry> m_xNameED;
        std::unique_ptr<weld::Label> m_xURLFT;
        std::unique_ptr<SvtURLBox> m_xURLED;
        std::unique_ptr<weld::Button> m_xFilePickerBtn;
        std::unique_ptr<weld::CheckButton> m_xLinkInstanceCB;
        std::unique_ptr<weld::Label> m_xAltTitle;

        DECL_LINK(FilePickerHdl, weld::Button&, void);

    public:
        AddInstanceDialog(weld::Window* pParent, bool _bEdit);
        virtual ~AddInstanceDialog() override;

        OUString         GetName() const { return m_xNameED->get_text(); }
        void             SetName( const OUString& rName ) { m_xNameED->set_text( rName );}
        OUString         GetURL() const { return m_xURLED->get_active_text(); }
        void             SetURL( const OUString& rURL ) { m_xURLED->set_entry_text( rURL );}
        bool             IsLinkInstance() const { return m_xLinkInstanceCB->get_active(); }
        void             SetLinkInstance( bool bLink ) { m_xLinkInstanceCB->set_active(bLink); }
    };

    class LinkedInstanceWarningBox : public weld::MessageDialogController
    {
    public:
        LinkedInstanceWarningBox(weld::Widget* pParent);
    };
}


#endif // INCLUDED_SVX_SOURCE_INC_DATANAVI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
