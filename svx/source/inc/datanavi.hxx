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

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
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
#include <svx/dialmgr.hxx>
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

    class ReplaceString
    {
        OUString m_sDoc_UI;
        OUString m_sInstance_UI;
        OUString m_sNone_UI;

        OUString m_sDoc_API;
        OUString m_sInstance_API;
        OUString m_sNone_API;

        ReplaceString( const ReplaceString& );

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
        OUString toUI( const OUString& rStr ) const
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

        OUString m_sPost_API;
        OUString m_sPut_API;
        OUString m_sGet_API;

        MethodString( const MethodString& );

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
        OUString toUI( const OUString& rStr ) const
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

    //========================================================================
    class XFormsPage : public TabPage
    {
    private:
        MethodString                m_aMethodString;
        ReplaceString               m_aReplaceString;

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
        OUString                    m_sInstanceName;
        OUString                    m_sInstanceURL;
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
        virtual bool                Notify( NotifyEvent& rNEvt );

    public:
        XFormsPage( Window* pParent, DataNavigatorWindow* _pNaviWin, DataGroupType _eGroup );
        ~XFormsPage();

        virtual void                Resize();

        inline bool                 HasModel() const { return m_bHasModel; }
        OUString                    SetModel( const XModel_ref& _xModel, sal_uInt16 _nPagePos );
        void                        ClearModel();
        OUString                    LoadInstance( const PropertyValue_seq& _xPropSeq,
                                                  const ImageList& _rImgLst );

        bool                        DoMenuAction( sal_uInt16 _nMenuID );
        void                        EnableMenuItems( Menu* _pMenu );

        inline SvTreeListEntry*     GetSelectedItem() const { return m_aItemList.FirstSelected(); }
        inline const OUString&      GetInstanceName() const { return m_sInstanceName; }
        inline const OUString&      GetInstanceURL() const { return m_sInstanceURL; }
        inline bool                 GetLinkOnce() const { return m_bLinkOnce; }
        inline void                 SetInstanceName( const OUString &name ) { m_sInstanceName=name; }
        inline void                 SetInstanceURL( const OUString &url ) { m_sInstanceURL=url; }
        inline void                 SetLinkOnce( bool bLinkOnce ) { m_bLinkOnce=bLinkOnce; }

        typedef com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet> XPropertySet_t;
        typedef com::sun::star::uno::Reference<com::sun::star::xml::dom::XNode> XNode_t;

        inline XPropertySet_t GetBindingForNode( const XNode_t &xNode ) { return m_xUIHelper->getBindingForNode(xNode,true); }
        inline OUString GetServiceNameForNode( const XNode_t &xNode ) { return m_xUIHelper->getDefaultServiceNameForNode(xNode); }
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
        virtual bool            Close();
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

        VclFrame*           m_pItemFrame;
        FixedText*          m_pNameFT;
        Edit*               m_pNameED;
        FixedText*          m_pDefaultFT;
        Edit*               m_pDefaultED;
        PushButton*         m_pDefaultBtn;

        VclFrame*           m_pSettingsFrame;

        FixedText*          m_pDataTypeFT;
        ListBox*            m_pDataTypeLB;

        CheckBox*           m_pRequiredCB;
        PushButton*         m_pRequiredBtn;
        CheckBox*           m_pRelevantCB;
        PushButton*         m_pRelevantBtn;
        CheckBox*           m_pConstraintCB;
        PushButton*         m_pConstraintBtn;
        CheckBox*           m_pReadonlyCB;
        PushButton*         m_pReadonlyBtn;
        CheckBox*           m_pCalculateCB;
        PushButton*         m_pCalculateBtn;

        OKButton*           m_pOKBtn;

        XFormsUIHelper1_ref m_xUIHelper;
        XPropertySet_ref    m_xBinding;
        XPropertySet_ref    m_xTempBinding;

        ItemNode*           m_pItemNode;
        DataItemType        m_eItemType;
        OUString            m_sFL_Element;
        OUString            m_sFL_Attribute;
        OUString            m_sFL_Binding;
        OUString            m_sFT_BindingExp;

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
        VclMultiLineEdit*       m_pConditionED;
        VclMultiLineEdit*       m_pResultWin;
        PushButton*             m_pEditNamespacesBtn;
        OKButton*               m_pOKBtn;

        Timer                   m_aResultTimer;
        OUString         m_sPropertyName;

        XFormsUIHelper1_ref     m_xUIHelper;
        XPropertySet_ref        m_xBinding;

        DECL_LINK(ModifyHdl, void *);
        DECL_LINK(ResultHdl, void *);
        DECL_LINK(EditHdl, void *);
        DECL_LINK(OKHdl, void *);

    public:
        AddConditionDialog(Window* pParent,
            const OUString& _rPropertyName, const XPropertySet_ref& _rBinding);

        XFormsUIHelper1_ref GetUIHelper() const { return m_xUIHelper; }
        OUString GetCondition() const { return m_pConditionED->GetText(); }
        void SetCondition(const OUString& _rCondition)
        {
            m_pConditionED->SetText(_rCondition);
            m_aResultTimer.Start();
        }
    };

    //========================================================================
    class NamespaceItemDialog : public ModalDialog
    {
    private:
        typedef std::vector< OUString > PrefixList;

        SvSimpleTable*      m_pNamespacesList;
        PushButton*         m_pAddNamespaceBtn;
        PushButton*         m_pEditNamespaceBtn;
        PushButton*         m_pDeleteNamespaceBtn;
        OKButton*           m_pOKBtn;

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
        Edit*               m_pPrefixED;
        Edit*               m_pUrlED;
        OKButton*           m_pOKBtn;

        AddConditionDialog* m_pConditionDlg;

        DECL_LINK(OKHdl, void *);

    public:
        ManageNamespaceDialog(Window* pParent, AddConditionDialog* _pCondDlg, bool bIsEdit);

        void SetNamespace(const OUString& _rPrefix, const OUString& _rURL)
        {
            m_pPrefixED->SetText( _rPrefix );
            m_pUrlED->SetText( _rURL );
        }
        OUString GetPrefix() const { return m_pPrefixED->GetText(); }
        OUString GetURL() const { return m_pUrlED->GetText(); }
    };

    //========================================================================
    class AddSubmissionDialog : public ModalDialog
    {
    private:
        MethodString        m_aMethodString;
        ReplaceString       m_aReplaceString;

        Edit*               m_pNameED;
        Edit*               m_pActionED;
        ListBox*            m_pMethodLB;
        Edit*               m_pRefED;
        PushButton*         m_pRefBtn;
        ListBox*            m_pBindLB;
        ListBox*            m_pReplaceLB;

        OKButton*           m_pOKBtn;

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
        Edit* m_pNameED;
        CheckBox* m_pModifyCB;

    public:
        AddModelDialog( Window* pParent, bool _bEdit );

        OUString         GetName() const { return m_pNameED->GetText(); }
        void             SetName( const OUString& _rName ) { m_pNameED->SetText( _rName );}

        bool             GetModifyDoc() const { return bool( m_pModifyCB->IsChecked() ); }
        void             SetModifyDoc( const bool bModify ) { m_pModifyCB->Check( bModify ); }
    };

    //========================================================================
    class AddInstanceDialog : public ModalDialog
    {
    private:
        Edit*                   m_pNameED;
        FixedText*              m_pURLFT;
        SvtURLBox*              m_pURLED;
        PushButton*             m_pFilePickerBtn;
        CheckBox*               m_pLinkInstanceCB;

        OUString                m_sAllFilterName;

        DECL_LINK(FilePickerHdl, void *);

    public:
        AddInstanceDialog( Window* pParent, bool _bEdit );

        void SetRenameMode()
        {
            m_pURLFT->Disable();
            m_pURLED->Disable();
            m_pFilePickerBtn->Disable();
            m_pLinkInstanceCB->Disable();
        }
        OUString         GetName() const { return m_pNameED->GetText(); }
        void             SetName( const OUString& _rName ) { m_pNameED->SetText( _rName );}
        OUString         GetURL() const { return m_pURLED->GetText(); }
        void             SetURL( const OUString& _rURL ) { m_pURLED->SetText( _rURL );}
        bool             IsLinkInstance() const { return m_pLinkInstanceCB->IsChecked(); }
        void             SetLinkInstance( bool _bLink ) { m_pLinkInstanceCB->Check( _bLink != false ); }
    };

    //========================================================================
    class LinkedInstanceWarningBox : public MessBox
    {
    public:
        LinkedInstanceWarningBox( Window* pParent );
    };

//............................................................................
}   // namespace svxform
//............................................................................

#endif // INCLUDED_SVX_SOURCE_INC_DATANAVI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
