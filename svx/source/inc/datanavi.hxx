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
#ifndef _SVX_DATANAVI_HXX
#define _SVX_DATANAVI_HXX

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/toolbox.hxx>
#include <svtools/inettbc.hxx>
#include <svtools/simptabl.hxx>
#include <svtools/svmedit.hxx>
#include <svtools/treelistbox.hxx>
#include <sfx2/dockwin.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/ctrlitem.hxx>
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
#include <vector>

// forward ---------------------------------------------------------------

class FmFormShell;

//========================================================================
//............................................................................
namespace svxform
{
//............................................................................

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

    typedef ::com::sun::star::uno::Reference<
                ::com::sun::star::xforms::XFormsUIHelper1 >         XFormsUIHelper1_ref;
    typedef ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::dom::XNode >                 XNode_ref;
    typedef ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet >             XPropertySet_ref;
    typedef ::com::sun::star::uno::Reference<
                ::com::sun::star::xforms::XModel >                  XModel_ref;
    typedef ::com::sun::star::uno::Reference<
                ::com::sun::star::container::XContainer >           XContainer_ref;
    typedef ::com::sun::star::uno::Reference<
                ::com::sun::star::container::XNameContainer >       XNameContainer_ref;
    typedef ::com::sun::star::uno::Reference<
                ::com::sun::star::frame::XFrame >                   XFrame_ref;
    typedef ::com::sun::star::uno::Reference<
                ::com::sun::star::frame::XModel >                   XFrameModel_ref;
    typedef ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::dom::events::XEventTarget >  XEventTarget_ref;
    typedef ::com::sun::star::uno::Reference<
                ::com::sun::star::xforms::XSubmission >             XSubmission_ref;

    typedef ::com::sun::star::uno::Sequence<
                ::com::sun::star::beans::PropertyValue >            PropertyValue_seq;

    typedef std::vector< XContainer_ref >                           ContainerList;
    typedef std::vector< XEventTarget_ref >                         EventTargetList;

    //========================================================================
    class DataTreeListBox : public SvTreeListBox
    {
    private:
        XFormsPage*             m_pXFormsPage;
        DataGroupType           m_eGroup;

    protected:
        using SvTreeListBox::ExecuteDrop;

    public:
        DataTreeListBox( XFormsPage* pPage, DataGroupType _eGroup, const ResId& rResId );
        ~DataTreeListBox();

        virtual PopupMenu*      CreateContextMenu( void );
        virtual void            ExcecuteContextMenuAction( sal_uInt16 _nSelectedPopupEntry );
        virtual sal_Int8        AcceptDrop( const AcceptDropEvent& rEvt );
        virtual sal_Int8        ExecuteDrop( const ExecuteDropEvent& rEvt );
        virtual void            StartDrag( sal_Int8 nAction, const Point& rPosPixel );

        void                    DeleteAndClear();
        void                    RemoveEntry( SvTreeListEntry* _pEntry );
    };

    //========================================================================
    class XFormsPage : public TabPage
    {
    private:
        ToolBox                     m_aToolBox;
        DataTreeListBox             m_aItemList;

        XFormsUIHelper1_ref         m_xUIHelper;

        DataNavigatorWindow*        m_pNaviWin;
        bool                        m_bHasModel;
        DataGroupType               m_eGroup;
        ImageList                   m_TbxImageList;
        // these strings are not valid on the Submission and Binding Page
        // mb: furthermore these are properties of an instance, thus
        // it would be much better to get/set them through the UIHelper
        // interface.
        String                      m_sInstanceName;
        String                      m_sInstanceURL;
        bool                        m_bLinkOnce;

        DECL_LINK(TbxSelectHdl, void *);
        DECL_LINK(ItemSelectHdl, void *);

        void                        AddChildren( SvTreeListEntry* _pParent,
                                                 const ImageList& _rImgLst,
                                                 const XNode_ref& _xNode );
        bool                        DoToolBoxAction( sal_uInt16 _nToolBoxID );
        SvTreeListEntry*                AddEntry( ItemNode* _pNewNode, bool _bIsElement );
        SvTreeListEntry*                AddEntry( const XPropertySet_ref& _rPropSet );
        void                        EditEntry( const XPropertySet_ref& _rPropSet );
        bool                        RemoveEntry();

    protected:
        virtual long                Notify( NotifyEvent& rNEvt );

    public:
        XFormsPage( Window* pParent, DataNavigatorWindow* _pNaviWin, DataGroupType _eGroup );
        ~XFormsPage();

        virtual void                Resize();

        inline bool                 HasModel() const { return m_bHasModel; }
        String                      SetModel( const XModel_ref& _xModel, sal_uInt16 _nPagePos );
        void                        ClearModel();
        String                      LoadInstance( const PropertyValue_seq& _xPropSeq,
                                                  const ImageList& _rImgLst );

        bool                        DoMenuAction( sal_uInt16 _nMenuID );
        void                        EnableMenuItems( Menu* _pMenu );

        inline SvTreeListEntry*         GetSelectedItem() const { return m_aItemList.FirstSelected(); }
        inline const String&        GetInstanceName() const { return m_sInstanceName; }
        inline const String&        GetInstanceURL() const { return m_sInstanceURL; }
        inline bool                 GetLinkOnce() const { return m_bLinkOnce; }
        inline void                 SetInstanceName( const String &name ) { m_sInstanceName=name; }
        inline void                 SetInstanceURL( const String &url ) { m_sInstanceURL=url; }
        inline void                 SetLinkOnce( bool bLinkOnce ) { m_bLinkOnce=bLinkOnce; }

        typedef com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet> XPropertySet_t;
        typedef com::sun::star::uno::Reference<com::sun::star::xml::dom::XNode> XNode_t;

        inline XPropertySet_t GetBindingForNode( const XNode_t &xNode ) { return m_xUIHelper->getBindingForNode(xNode,true); }
        inline String GetServiceNameForNode( const XNode_t &xNode ) { return m_xUIHelper->getDefaultServiceNameForNode(xNode); }
        inline XFormsUIHelper1_ref GetXFormsHelper( void ) const { return m_xUIHelper; }
    };

    //========================================================================

    typedef std::vector< XFormsPage* >          PageList;
    typedef ::rtl::Reference < DataListener >   DataListener_ref;

    class DataNavigatorWindow : public Window
    {
    private:

        ListBox                     m_aModelsBox;
        MenuButton                  m_aModelBtn;
        TabControl                  m_aTabCtrl;
        MenuButton                  m_aInstanceBtn;

        XFormsPage*                 m_pInstPage;
        XFormsPage*                 m_pSubmissionPage;
        XFormsPage*                 m_pBindingPage;

        long                        m_nMinWidth;
        long                        m_nMinHeight;
        long                        m_nBorderHeight;
        sal_uInt16                      m_nLastSelectedPos;
        bool                        m_bShowDetails;
        bool                        m_bIsNotifyDisabled;
        Size                        m_a2Size;
        Size                        m_a3Size;
        ImageList                   m_aItemImageList;
        PageList                    m_aPageList;
        ContainerList               m_aContainerList;
        EventTargetList             m_aEventTargetList;
        Timer                       m_aUpdateTimer;

        DataListener_ref            m_xDataListener;
        XNameContainer_ref          m_xDataContainer;
        XFrame_ref                  m_xFrame;
        XFrameModel_ref             m_xFrameModel;

        DECL_LINK(                  ModelSelectHdl, ListBox * );
        DECL_LINK(                  MenuSelectHdl, MenuButton * );
        DECL_LINK(                  MenuActivateHdl, MenuButton * );
        DECL_LINK(ActivatePageHdl, void *);
        DECL_LINK(UpdateHdl, void *);

        XFormsPage*                 GetCurrentPage( sal_uInt16& rCurId );
        void                        LoadModels();
        void                        SetPageModel();
        void                        ClearAllPageModels( bool bClearPages );
        void                        InitPages();
        void                        CreateInstancePage( const PropertyValue_seq& _xPropSeq );
        bool                        HasFirstInstancePage() const;
        sal_uInt16                      GetNewPageId() const;

    protected:
        virtual void                Resize();

    public:
        DataNavigatorWindow( Window* pParent, SfxBindings* pBindings );
        ~DataNavigatorWindow();

        void                        SetDocModified();
        void                        NotifyChanges( bool _bLoadAll = false );
        void                        AddContainerBroadcaster( const XContainer_ref& xContainer );
        void                        AddEventBroadcaster( const XEventTarget_ref& xTarget );
        void                        RemoveBroadcaster();

        inline const ImageList&     GetItemImageList() const { return m_aItemImageList; }
        inline bool                 IsShowDetails() const { return m_bShowDetails; }
        inline void                 DisableNotify( bool _bDisable ) { m_bIsNotifyDisabled = _bDisable; }
    };

    //========================================================================
    class DataNavigator : public SfxDockingWindow, public SfxControllerItem
    {
    private:
        DataNavigatorWindow         m_aDataWin;

    protected:
        virtual void                Resize();
        virtual sal_Bool            Close();
        virtual void                GetFocus();
        virtual Size                CalcDockingSize( SfxChildAlignment );
        virtual SfxChildAlignment   CheckAlignment( SfxChildAlignment, SfxChildAlignment );

    public:
        DataNavigator( SfxBindings* pBindings, SfxChildWindow* pMgr, Window* pParent );
        virtual ~DataNavigator();

        using Window::Update;
        using                       SfxDockingWindow::StateChanged;

        void                        Update( FmFormShell* pFormShell );
        void                        StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                                  const SfxPoolItem* pState );
    };

    //========================================================================
    class SVX_DLLPUBLIC DataNavigatorManager : public SfxChildWindow
    {
    public:
        SVX_DLLPRIVATE DataNavigatorManager( Window* pParent, sal_uInt16 nId,
                              SfxBindings* pBindings, SfxChildWinInfo* pInfo );
        SFX_DECL_CHILDWINDOW( DataNavigatorManager );
    };

    //========================================================================
    class AddDataItemDialog : public ModalDialog
    {
    private:

        FixedLine           m_aItemFL;
        FixedText           m_aNameFT;
        Edit                m_aNameED;
        FixedText           m_aDefaultFT;
        Edit                m_aDefaultED;
        PushButton          m_aDefaultBtn;

        FixedLine           m_aSettingsFL;
        FixedText           m_aDataTypeFT;
        ListBox             m_aDataTypeLB;

        CheckBox            m_aRequiredCB;
        PushButton          m_aRequiredBtn;
        CheckBox            m_aRelevantCB;
        PushButton          m_aRelevantBtn;
        CheckBox            m_aConstraintCB;
        PushButton          m_aConstraintBtn;
        CheckBox            m_aReadonlyCB;
        PushButton          m_aReadonlyBtn;
        CheckBox            m_aCalculateCB;
        PushButton          m_aCalculateBtn;

        FixedLine           m_aButtonsFL;
        OKButton            m_aOKBtn;
        CancelButton        m_aEscBtn;
        HelpButton          m_aHelpBtn;

        XFormsUIHelper1_ref m_xUIHelper;
        XPropertySet_ref    m_xBinding;
        XPropertySet_ref    m_xTempBinding;

        ItemNode*           m_pItemNode;
        DataItemType        m_eItemType;
        String              m_sFL_Element;
        String              m_sFL_Attribute;
        String              m_sFL_Binding;
        String              m_sFT_BindingExp;

        DECL_LINK(          CheckHdl, CheckBox * );
        DECL_LINK(          ConditionHdl, PushButton * );
        DECL_LINK(OKHdl, void *);

        void                InitDialog();
        void                InitFromNode();
        void                InitDataTypeBox();

    public:
        AddDataItemDialog(
            Window* pParent, ItemNode* _pNode, const XFormsUIHelper1_ref& _rUIHelper );
        ~AddDataItemDialog();

        void                InitText( DataItemType _eType );
    };

    //========================================================================
    class AddConditionDialog : public ModalDialog
    {
    private:
        FixedText               m_aConditionFT;
        MultiLineEdit           m_aConditionED;
        FixedText               m_aResultFT;
        FixedText               m_aResultWin;
        PushButton              m_aEditNamespacesBtn;
        FixedLine               m_aButtonsFL;
        OKButton                m_aOKBtn;
        CancelButton            m_aEscBtn;
        HelpButton              m_aHelpBtn;

        Timer                   m_aResultTimer;
        OUString         m_sPropertyName;

        XFormsUIHelper1_ref     m_xUIHelper;
        XPropertySet_ref        m_xBinding;

        DECL_LINK(ModifyHdl, void *);
        DECL_LINK(ResultHdl, void *);
        DECL_LINK(EditHdl, void *);
        DECL_LINK(OKHdl, void *);

    public:
        AddConditionDialog( Window* pParent,
            const OUString& _rPropertyName, const XPropertySet_ref& _rBinding );
        ~AddConditionDialog();

        inline XFormsUIHelper1_ref  GetUIHelper() const { return m_xUIHelper; }
        inline String               GetCondition() const { return m_aConditionED.GetText(); }
        inline void                 SetCondition( const String& _rCondition );
    };

    inline void AddConditionDialog::SetCondition( const String& _rCondition )
    {
        m_aConditionED.SetText( _rCondition );
        m_aResultTimer.Start();
    }

    //========================================================================
    class NamespaceItemDialog : public ModalDialog
    {
    private:
        typedef std::vector< OUString > PrefixList;

        FixedText           m_aNamespacesFT;
        SvSimpleTableContainer m_aNamespacesListContainer;
        SvSimpleTable      m_aNamespacesList;
        PushButton          m_aAddNamespaceBtn;
        PushButton          m_aEditNamespaceBtn;
        PushButton          m_aDeleteNamespaceBtn;
        FixedLine           m_aButtonsFL;
        OKButton            m_aOKBtn;
        CancelButton        m_aEscBtn;
        HelpButton          m_aHelpBtn;

        AddConditionDialog* m_pConditionDlg;
        PrefixList          m_aRemovedList;

        XNameContainer_ref& m_rNamespaces;

        DECL_LINK(          SelectHdl, SvSimpleTable * );
        DECL_LINK(          ClickHdl, PushButton * );
        DECL_LINK(OKHdl, void *);

        void                LoadNamespaces();

    public:
        NamespaceItemDialog( AddConditionDialog* pParent, XNameContainer_ref& _rContainer );
        ~NamespaceItemDialog();
    };

    //========================================================================
    class ManageNamespaceDialog : public ModalDialog
    {
    private:
        FixedText           m_aPrefixFT;
        Edit                m_aPrefixED;
        FixedText           m_aUrlFT;
        Edit                m_aUrlED;
        FixedLine           m_aButtonsFL;
        OKButton            m_aOKBtn;
        CancelButton        m_aEscBtn;
        HelpButton          m_aHelpBtn;

        AddConditionDialog* m_pConditionDlg;

        DECL_LINK(OKHdl, void *);

    public:
        ManageNamespaceDialog( Window* pParent, AddConditionDialog* _pCondDlg, bool _bIsEdit );
        ~ManageNamespaceDialog();

        inline void         SetNamespace( const String& _rPrefix, const String& _rURL );
        inline String       GetPrefix() const { return m_aPrefixED.GetText(); }
        inline String       GetURL() const { return m_aUrlED.GetText(); }
    };

    void ManageNamespaceDialog::SetNamespace( const String& _rPrefix, const String& _rURL )
    {
        m_aPrefixED.SetText( _rPrefix );
        m_aUrlED.SetText( _rURL );
    }

    //========================================================================
    class AddSubmissionDialog : public ModalDialog
    {
    private:
        FixedLine           m_aSubmissionFL;
        FixedText           m_aNameFT;
        Edit                m_aNameED;
        FixedText           m_aActionFT;
        Edit                m_aActionED;
        FixedText           m_aMethodFT;
        ListBox             m_aMethodLB;
        FixedText           m_aRefFT;
        Edit                m_aRefED;
        PushButton          m_aRefBtn;
        FixedText           m_aBindFT;
        ListBox             m_aBindLB;
        FixedText           m_aReplaceFT;
        ListBox             m_aReplaceLB;

        FixedLine           m_aButtonsFL;
        OKButton            m_aOKBtn;
        CancelButton        m_aEscBtn;
        HelpButton          m_aHelpBtn;

        ItemNode*           m_pItemNode;

        XFormsUIHelper1_ref m_xUIHelper;
        XSubmission_ref     m_xNewSubmission;
        XPropertySet_ref    m_xSubmission;
        XPropertySet_ref    m_xTempBinding;
        XPropertySet_ref    m_xCreatedBinding;

        DECL_LINK(RefHdl, void *);
        DECL_LINK(OKHdl, void *);

        void                FillAllBoxes();

    public:
        AddSubmissionDialog( Window* pParent, ItemNode* _pNode,
            const XFormsUIHelper1_ref& _rUIHelper );
        ~AddSubmissionDialog();

        inline const XSubmission_ref& GetNewSubmission() const { return m_xNewSubmission; }
    };

    //========================================================================
    class AddModelDialog : public ModalDialog
    {
    private:
        FixedLine           m_aModelFL;
        FixedText           m_aNameFT;
        Edit                m_aNameED;
        CheckBox            m_aModifyCB;
        FixedLine           m_aButtonsFL;
        OKButton            m_aOKBtn;
        CancelButton        m_aEscBtn;
        HelpButton          m_aHelpBtn;

    public:
        AddModelDialog( Window* pParent, bool _bEdit );
        ~AddModelDialog();

        inline String           GetName() const { return m_aNameED.GetText(); }
        inline void             SetName( const String& _rName ) { m_aNameED.SetText( _rName );}

        inline bool             GetModifyDoc() const { return bool( m_aModifyCB.IsChecked() ); }
        inline void             SetModifyDoc( const bool _bModify ) { m_aModifyCB.Check( _bModify ); }
    };

    //========================================================================
    class AddInstanceDialog : public ModalDialog
    {
    private:
        FixedLine               m_aInstanceFL;
        FixedText               m_aNameFT;
        Edit                    m_aNameED;
        FixedText               m_aURLFT;
        SvtURLBox               m_aURLED;
        PushButton              m_aFilePickerBtn;
        CheckBox                m_aLinkInstanceCB;
        FixedLine               m_aButtonsFL;
        OKButton                m_aOKBtn;
        CancelButton            m_aEscBtn;
        HelpButton              m_aHelpBtn;

        String                  m_sAllFilterName;

        DECL_LINK(FilePickerHdl, void *);

    public:
        AddInstanceDialog( Window* pParent, bool _bEdit );
        ~AddInstanceDialog();

        inline void             SetRenameMode();
        inline String           GetName() const { return m_aNameED.GetText(); }
        inline void             SetName( const String& _rName ) { m_aNameED.SetText( _rName );}
        inline String           GetURL() const { return m_aURLED.GetText(); }
        inline void             SetURL( const String& _rURL ) { m_aURLED.SetText( _rURL );}
        inline bool             IsLinkInstance() const { return ( m_aLinkInstanceCB.IsChecked() != sal_False ); }
        inline void             SetLinkInstance( bool _bLink ) { m_aLinkInstanceCB.Check( _bLink != false ); }
    };

    inline void AddInstanceDialog::SetRenameMode()
    {
        m_aURLFT.Disable();
        m_aURLED.Disable();
        m_aFilePickerBtn.Disable();
        m_aLinkInstanceCB.Disable();
    }

    //========================================================================
    class LinkedInstanceWarningBox : public MessBox
    {
    public:
        LinkedInstanceWarningBox( Window* pParent );
    };

//............................................................................
}   // namespace svxform
//............................................................................

#endif // _SVX_DATANAVI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
