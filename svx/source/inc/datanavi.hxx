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

#include <vcl/builder.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/idle.hxx>
#include <svtools/inettbc.hxx>
#include <svtools/simptabl.hxx>
#include <svtools/svmedit.hxx>
#include <svtools/treelistbox.hxx>
#include <sfx2/dockwin.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/ctrlitem.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>
#include <svx/fmresids.hrc>
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

    typedef css::uno::Reference< css::xforms::XFormsUIHelper1 >         XFormsUIHelper1_ref;
    typedef css::uno::Reference< css::xml::dom::XNode >                 XNode_ref;
    typedef css::uno::Reference< css::beans::XPropertySet >             XPropertySet_ref;
    typedef css::uno::Reference< css::xforms::XModel >                  XModel_ref;
    typedef css::uno::Reference< css::container::XContainer >           XContainer_ref;
    typedef css::uno::Reference< css::container::XNameContainer >       XNameContainer_ref;
    typedef css::uno::Reference< css::frame::XFrame >                   XFrame_ref;
    typedef css::uno::Reference< css::frame::XModel >                   XFrameModel_ref;
    typedef css::uno::Reference< css::xml::dom::events::XEventTarget >  XEventTarget_ref;
    typedef css::uno::Reference< css::xforms::XSubmission >             XSubmission_ref;
    typedef css::uno::Sequence< css::beans::PropertyValue >             PropertyValue_seq;
    typedef std::vector< XContainer_ref >                               ContainerList;
    typedef std::vector< XEventTarget_ref >                             EventTargetList;


    class DataTreeListBox : public SvTreeListBox
    {
    private:
        VclPtr<XFormsPage>      m_pXFormsPage;
        DataGroupType           m_eGroup;
        sal_uInt16              m_nAddId;
        sal_uInt16              m_nAddElementId;
        sal_uInt16              m_nAddAttributeId;
        sal_uInt16              m_nEditId;
        sal_uInt16              m_nRemoveId;

    protected:
        using SvTreeListBox::ExecuteDrop;

    public:
        DataTreeListBox( vcl::Window* pParent, WinBits nBits );
        virtual ~DataTreeListBox();
        virtual void dispose() override;

        virtual VclPtr<PopupMenu> CreateContextMenu() override;
        virtual void            ExecuteContextMenuAction( sal_uInt16 _nSelectedPopupEntry ) override;
        virtual sal_Int8        AcceptDrop( const AcceptDropEvent& rEvt ) override;
        virtual sal_Int8        ExecuteDrop( const ExecuteDropEvent& rEvt ) override;
        virtual void            StartDrag( sal_Int8 nAction, const Point& rPosPixel ) override;

        void                    SetGroup(DataGroupType _eGroup);
        void                    SetXFormsPage(XFormsPage* _pPage);
        void                    SetToolBoxItemIds(sal_uInt16 _nAddId,
                                                  sal_uInt16 _nAddElementId,
                                                  sal_uInt16 _nAddAttributeId,
                                                  sal_uInt16 _nEditId,
                                                  sal_uInt16 _nRemoveId);
        void                    DeleteAndClear();
        void                    RemoveEntry( SvTreeListEntry* _pEntry );
    };

    class ReplaceString
    {
        OUString m_sDoc_UI;
        OUString m_sInstance_UI;
        OUString m_sNone_UI;

        OUString m_sDoc_API;
        OUString m_sInstance_API;
        OUString m_sNone_API;

        ReplaceString( const ReplaceString& ) = delete;

    public:
        ReplaceString() :
            m_sDoc_API(      "all" ),
            m_sInstance_API( "instance" ),
            m_sNone_API(     "none" )
        {
            m_sDoc_UI = SVX_RESSTR(RID_STR_REPLACE_DOC);
            m_sInstance_UI = SVX_RESSTR(RID_STR_REPLACE_INST);
            m_sNone_UI = SVX_RESSTR(RID_STR_REPLACE_NONE);
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
        OUString const & toAPI( const OUString& rStr ) const
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

        OUString m_sPost_API;
        OUString m_sPut_API;
        OUString m_sGet_API;

        MethodString( const MethodString& ) = delete;

    public:

        MethodString() :
            m_sPost_API( "post" ),
            m_sPut_API(  "put" ),
            m_sGet_API(  "get" )
        {
            m_sPost_UI = SVX_RESSTR(RID_STR_METHOD_POST);
            m_sPut_UI  = SVX_RESSTR(RID_STR_METHOD_PUT);
            m_sGet_UI  = SVX_RESSTR(RID_STR_METHOD_GET);
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
        OUString const & toAPI( const OUString& rStr ) const
        {
            if( rStr == m_sGet_UI )
                return m_sGet_API;
            else if( rStr == m_sPost_UI )
                return m_sPost_API;
            else
                return m_sPut_API;
        }
    };


    class XFormsPage : public TabPage
    {
    private:
        MethodString                m_aMethodString;
        ReplaceString               m_aReplaceString;

        VclPtr<ToolBox>             m_pToolBox;
        VclPtr<DataTreeListBox>     m_pItemList;
        sal_uInt16                  m_nAddId;
        sal_uInt16                  m_nAddElementId;
        sal_uInt16                  m_nAddAttributeId;
        sal_uInt16                  m_nEditId;
        sal_uInt16                  m_nRemoveId;

        XFormsUIHelper1_ref         m_xUIHelper;

        VclPtr<DataNavigatorWindow> m_pNaviWin;
        bool                        m_bHasModel;
        DataGroupType               m_eGroup;
        ImageList                   m_TbxImageList;
        // these strings are not valid on the Submission and Binding Page
        // mb: furthermore these are properties of an instance, thus
        // it would be much better to get/set them through the UIHelper
        // interface.
        OUString                    m_sInstanceName;
        OUString                    m_sInstanceURL;
        bool                        m_bLinkOnce;

        DECL_LINK_TYPED(TbxSelectHdl, ToolBox *, void);
        DECL_LINK_TYPED(ItemSelectHdl, SvTreeListBox*, void);

        void                        AddChildren( SvTreeListEntry* _pParent,
                                                 const ImageList& _rImgLst,
                                                 const XNode_ref& _xNode );
        bool                        DoToolBoxAction( sal_uInt16 _nToolBoxID );
        SvTreeListEntry*            AddEntry( ItemNode* _pNewNode, bool _bIsElement );
        SvTreeListEntry*            AddEntry( const XPropertySet_ref& _rPropSet );
        void                        EditEntry( const XPropertySet_ref& _rPropSet );
        bool                        RemoveEntry();

    protected:
        virtual bool                Notify( NotifyEvent& rNEvt ) override;

    public:
        XFormsPage( vcl::Window* pParent, DataNavigatorWindow* _pNaviWin, DataGroupType _eGroup );
        virtual ~XFormsPage();
        virtual void dispose() override;

        virtual void                Resize() override;

        inline bool                 HasModel() const { return m_bHasModel; }
        OUString                    SetModel( const XModel_ref& _xModel, sal_uInt16 _nPagePos );
        void                        ClearModel();
        OUString                    LoadInstance( const PropertyValue_seq& _xPropSeq,
                                                  const ImageList& _rImgLst );

        bool                        DoMenuAction( sal_uInt16 _nMenuID );
        void                        EnableMenuItems( Menu* _pMenu );

        inline const OUString&      GetInstanceName() const { return m_sInstanceName; }
        inline const OUString&      GetInstanceURL() const { return m_sInstanceURL; }
        inline bool                 GetLinkOnce() const { return m_bLinkOnce; }
        inline void                 SetInstanceName( const OUString &name ) { m_sInstanceName=name; }
        inline void                 SetInstanceURL( const OUString &url ) { m_sInstanceURL=url; }
        inline void                 SetLinkOnce( bool bLinkOnce ) { m_bLinkOnce=bLinkOnce; }

        inline css::uno::Reference<css::beans::XPropertySet>
                                    GetBindingForNode( const css::uno::Reference<css::xml::dom::XNode> &xNode ) { return m_xUIHelper->getBindingForNode(xNode,true); }
        inline OUString             GetServiceNameForNode( const css::uno::Reference<css::xml::dom::XNode> &xNode ) { return m_xUIHelper->getDefaultServiceNameForNode(xNode); }
        const XFormsUIHelper1_ref&  GetXFormsHelper() const { return m_xUIHelper; }
    };


    typedef std::vector< VclPtr<XFormsPage> >   PageList;
    typedef ::rtl::Reference < DataListener >   DataListener_ref;

    class DataNavigatorWindow : public vcl::Window, public VclBuilderContainer
    {
    private:
        VclPtr<ListBox>             m_pModelsBox;
        VclPtr<MenuButton>          m_pModelBtn;
        VclPtr<TabControl>          m_pTabCtrl;
        VclPtr<MenuButton>          m_pInstanceBtn;

        VclPtr<XFormsPage>          m_pInstPage;
        VclPtr<XFormsPage>          m_pSubmissionPage;
        VclPtr<XFormsPage>          m_pBindingPage;

        sal_Int32                   m_nLastSelectedPos;
        bool                        m_bShowDetails;
        bool                        m_bIsNotifyDisabled;
        ImageList                   m_aItemImageList;
        PageList                    m_aPageList;
        ContainerList               m_aContainerList;
        EventTargetList             m_aEventTargetList;
        Timer                       m_aUpdateTimer;

        DataListener_ref            m_xDataListener;
        XNameContainer_ref          m_xDataContainer;
        XFrame_ref                  m_xFrame;
        XFrameModel_ref             m_xFrameModel;

        DECL_LINK_TYPED(            ModelSelectListBoxHdl, ListBox&, void );
        DECL_LINK_TYPED(            MenuSelectHdl, MenuButton *, void );
        DECL_LINK_TYPED(            MenuActivateHdl, MenuButton *, void );
        DECL_LINK_TYPED(            ActivatePageHdl, TabControl*, void);
        DECL_LINK_TYPED(            UpdateHdl, Timer *, void);
        void ModelSelectHdl(ListBox*);
        XFormsPage*                 GetCurrentPage( sal_uInt16& rCurId );
        void                        LoadModels();
        void                        SetPageModel();
        void                        ClearAllPageModels( bool bClearPages );
        void                        InitPages();
        void                        CreateInstancePage( const PropertyValue_seq& _xPropSeq );
        bool                        HasFirstInstancePage() const;
        sal_uInt16                  GetNewPageId() const;

        bool                        IsAdditionalPage(sal_uInt16 nPageId) const;

    protected:
        virtual void                Resize() override;
        virtual Size                GetOptimalSize() const override;

    public:
        DataNavigatorWindow( vcl::Window* pParent, SfxBindings* pBindings );
        virtual ~DataNavigatorWindow();
        virtual void dispose() override;

        static void                 SetDocModified();
        void                        NotifyChanges( bool _bLoadAll = false );
        void                        AddContainerBroadcaster( const XContainer_ref& xContainer );
        void                        AddEventBroadcaster( const XEventTarget_ref& xTarget );
        void                        RemoveBroadcaster();

        inline const ImageList&     GetItemImageList() const { return m_aItemImageList; }
        inline bool                 IsShowDetails() const { return m_bShowDetails; }
        inline void                 DisableNotify( bool _bDisable ) { m_bIsNotifyDisabled = _bDisable; }
    };


    class DataNavigator : public SfxDockingWindow, public SfxControllerItem
    {
    private:
        VclPtr<DataNavigatorWindow> m_aDataWin;

    protected:
        virtual void                Resize() override;
        virtual void                GetFocus() override;
        virtual Size                CalcDockingSize( SfxChildAlignment ) override;
        virtual SfxChildAlignment   CheckAlignment( SfxChildAlignment, SfxChildAlignment ) override;

    public:
        DataNavigator( SfxBindings* pBindings, SfxChildWindow* pMgr, vcl::Window* pParent );
        virtual ~DataNavigator();
        virtual void dispose() override;

        using Window::Update;
        using                       SfxDockingWindow::StateChanged;

        void                        StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                                  const SfxPoolItem* pState ) override;
    };


    class SVX_DLLPUBLIC DataNavigatorManager : public SfxChildWindow
    {
    public:
        SVX_DLLPRIVATE DataNavigatorManager( vcl::Window* pParent, sal_uInt16 nId,
                              SfxBindings* pBindings, SfxChildWinInfo* pInfo );
        SFX_DECL_CHILDWINDOW( DataNavigatorManager );
    };


    class AddDataItemDialog : public ModalDialog
    {
    private:

        VclPtr<VclFrame>    m_pItemFrame;
        VclPtr<FixedText>   m_pNameFT;
        VclPtr<Edit>        m_pNameED;
        VclPtr<FixedText>   m_pDefaultFT;
        VclPtr<Edit>        m_pDefaultED;
        VclPtr<PushButton>  m_pDefaultBtn;

        VclPtr<VclFrame>    m_pSettingsFrame;

        VclPtr<FixedText>   m_pDataTypeFT;
        VclPtr<ListBox>     m_pDataTypeLB;

        VclPtr<CheckBox>    m_pRequiredCB;
        VclPtr<PushButton>  m_pRequiredBtn;
        VclPtr<CheckBox>    m_pRelevantCB;
        VclPtr<PushButton>  m_pRelevantBtn;
        VclPtr<CheckBox>    m_pConstraintCB;
        VclPtr<PushButton>  m_pConstraintBtn;
        VclPtr<CheckBox>    m_pReadonlyCB;
        VclPtr<PushButton>  m_pReadonlyBtn;
        VclPtr<CheckBox>    m_pCalculateCB;
        VclPtr<PushButton>  m_pCalculateBtn;

        VclPtr<OKButton>    m_pOKBtn;

        XFormsUIHelper1_ref m_xUIHelper;
        XPropertySet_ref    m_xBinding;
        XPropertySet_ref    m_xTempBinding;

        ItemNode*           m_pItemNode;
        DataItemType        m_eItemType;
        OUString            m_sFL_Element;
        OUString            m_sFL_Attribute;
        OUString            m_sFL_Binding;
        OUString            m_sFT_BindingExp;

        DECL_LINK_TYPED(    CheckHdl, Button*, void );
        DECL_LINK_TYPED(    ConditionHdl, Button*, void );
        DECL_LINK_TYPED(    OKHdl, Button*, void);

        void                InitDialog();
        void                InitFromNode();
        void                InitDataTypeBox();

    public:
        AddDataItemDialog(
            vcl::Window* pParent, ItemNode* _pNode, const XFormsUIHelper1_ref& _rUIHelper );
        virtual ~AddDataItemDialog();
        virtual void dispose() override;

        void                InitText( DataItemType _eType );
    };


    class AddConditionDialog : public ModalDialog
    {
    private:
        VclPtr<VclMultiLineEdit>       m_pConditionED;
        VclPtr<VclMultiLineEdit>       m_pResultWin;
        VclPtr<PushButton>             m_pEditNamespacesBtn;
        VclPtr<OKButton>               m_pOKBtn;

        Idle                    m_aResultIdle;
        OUString                m_sPropertyName;

        XFormsUIHelper1_ref     m_xUIHelper;
        XPropertySet_ref        m_xBinding;

        DECL_LINK_TYPED(ModifyHdl, Edit&, void);
        DECL_LINK_TYPED(ResultHdl, Idle *, void);
        DECL_LINK_TYPED(EditHdl, Button*, void);
        DECL_LINK_TYPED(OKHdl, Button*, void);

    public:
        AddConditionDialog(vcl::Window* pParent,
            const OUString& _rPropertyName, const XPropertySet_ref& _rBinding);
        virtual ~AddConditionDialog();
        virtual void dispose() override;

        const XFormsUIHelper1_ref& GetUIHelper() const { return m_xUIHelper; }
        OUString GetCondition() const { return m_pConditionED->GetText(); }
        void SetCondition(const OUString& _rCondition)
        {
            m_pConditionED->SetText(_rCondition);
            m_aResultIdle.Start();
        }
    };


    class NamespaceItemDialog : public ModalDialog
    {
    private:
        typedef std::vector< OUString > PrefixList;

        VclPtr<SvSimpleTable>      m_pNamespacesList;
        VclPtr<PushButton>         m_pAddNamespaceBtn;
        VclPtr<PushButton>         m_pEditNamespaceBtn;
        VclPtr<PushButton>         m_pDeleteNamespaceBtn;
        VclPtr<OKButton>           m_pOKBtn;

        VclPtr<AddConditionDialog> m_pConditionDlg;
        PrefixList          m_aRemovedList;

        XNameContainer_ref& m_rNamespaces;

        DECL_LINK_TYPED(    SelectHdl, SvTreeListBox*, void );
        DECL_LINK_TYPED(    ClickHdl, Button*, void );
        DECL_LINK_TYPED(    OKHdl, Button*, void);

        void                LoadNamespaces();

    public:
        NamespaceItemDialog( AddConditionDialog* pParent, XNameContainer_ref& _rContainer );
        virtual ~NamespaceItemDialog();
        virtual void dispose() override;
    };


    class ManageNamespaceDialog : public ModalDialog
    {
    private:
        VclPtr<Edit>               m_pPrefixED;
        VclPtr<Edit>               m_pUrlED;
        VclPtr<OKButton>           m_pOKBtn;

        VclPtr<AddConditionDialog> m_pConditionDlg;

        DECL_LINK_TYPED(OKHdl, Button*, void);

    public:
        ManageNamespaceDialog(vcl::Window* pParent, AddConditionDialog* _pCondDlg, bool bIsEdit);
        virtual ~ManageNamespaceDialog();
        virtual void dispose() override;

        void SetNamespace(const OUString& _rPrefix, const OUString& _rURL)
        {
            m_pPrefixED->SetText( _rPrefix );
            m_pUrlED->SetText( _rURL );
        }
        OUString GetPrefix() const { return m_pPrefixED->GetText(); }
        OUString GetURL() const { return m_pUrlED->GetText(); }
    };


    class AddSubmissionDialog : public ModalDialog
    {
    private:
        MethodString        m_aMethodString;
        ReplaceString       m_aReplaceString;

        VclPtr<Edit>        m_pNameED;
        VclPtr<Edit>        m_pActionED;
        VclPtr<ListBox>     m_pMethodLB;
        VclPtr<Edit>        m_pRefED;
        VclPtr<PushButton>  m_pRefBtn;
        VclPtr<ListBox>     m_pBindLB;
        VclPtr<ListBox>     m_pReplaceLB;

        VclPtr<OKButton>    m_pOKBtn;

        ItemNode*           m_pItemNode;

        XFormsUIHelper1_ref m_xUIHelper;
        XSubmission_ref     m_xNewSubmission;
        XPropertySet_ref    m_xSubmission;
        XPropertySet_ref    m_xTempBinding;
        XPropertySet_ref    m_xCreatedBinding;

        DECL_LINK_TYPED(RefHdl, Button*, void);
        DECL_LINK_TYPED(OKHdl, Button*, void);

        void                FillAllBoxes();

    public:
        AddSubmissionDialog( vcl::Window* pParent, ItemNode* _pNode,
            const XFormsUIHelper1_ref& _rUIHelper );
        virtual ~AddSubmissionDialog();
        virtual void dispose() override;

        inline const XSubmission_ref& GetNewSubmission() const { return m_xNewSubmission; }
    };


    class AddModelDialog : public ModalDialog
    {
    private:
        VclPtr<Edit>     m_pNameED;
        VclPtr<CheckBox> m_pModifyCB;

    public:
        AddModelDialog( vcl::Window* pParent, bool _bEdit );
        virtual ~AddModelDialog();
        virtual void dispose() override;

        OUString         GetName() const { return m_pNameED->GetText(); }
        void             SetName( const OUString& _rName ) { m_pNameED->SetText( _rName );}

        bool             GetModifyDoc() const { return bool( m_pModifyCB->IsChecked() ); }
        void             SetModifyDoc( const bool bModify ) { m_pModifyCB->Check( bModify ); }
    };


    class AddInstanceDialog : public ModalDialog
    {
    private:
        VclPtr<Edit>                   m_pNameED;
        VclPtr<FixedText>              m_pURLFT;
        VclPtr<SvtURLBox>              m_pURLED;
        VclPtr<PushButton>             m_pFilePickerBtn;
        VclPtr<CheckBox>               m_pLinkInstanceCB;

        OUString                m_sAllFilterName;

        DECL_LINK_TYPED(FilePickerHdl, Button*, void);

    public:
        AddInstanceDialog( vcl::Window* pParent, bool _bEdit );
        virtual ~AddInstanceDialog();
        virtual void dispose() override;

        OUString         GetName() const { return m_pNameED->GetText(); }
        void             SetName( const OUString& _rName ) { m_pNameED->SetText( _rName );}
        OUString         GetURL() const { return m_pURLED->GetText(); }
        void             SetURL( const OUString& _rURL ) { m_pURLED->SetText( _rURL );}
        bool             IsLinkInstance() const { return m_pLinkInstanceCB->IsChecked(); }
        void             SetLinkInstance( bool _bLink ) { m_pLinkInstanceCB->Check(_bLink); }
    };


    class LinkedInstanceWarningBox : public MessageDialog
    {
    public:
        LinkedInstanceWarningBox( vcl::Window* pParent );
    };


}


#endif // INCLUDED_SVX_SOURCE_INC_DATANAVI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
