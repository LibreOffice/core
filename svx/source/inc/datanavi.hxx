/*************************************************************************
 *
 *  $RCSfile: datanavi.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 11:28:56 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SVX_DATANAVI_HXX
#define _SVX_DATANAVI_HXX

#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_MENUBTN_HXX
#include <vcl/menubtn.hxx>
#endif
#ifndef _SV_TABCTRL_HXX
#include <vcl/tabctrl.hxx>
#endif
#ifndef _SV_TABPAGE_HXX
#include <vcl/tabpage.hxx>
#endif
#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif
#ifndef _SVTOOLS_INETTBC_HXX
#include <svtools/inettbc.hxx>
#endif
#ifndef _SVEDIT_HXX
#include <svtools/svmedit.hxx>
#endif
#ifndef _SVTREEBOX_HXX
#include <svtools/svtreebx.hxx>
#endif
#ifndef _SFXDOCKWIN_HXX
#include <sfx2/dockwin.hxx>
#endif
#ifndef _SFXCTRLITEM_HXX
#include <sfx2/ctrlitem.hxx>
#endif
#ifndef _SVX_SIMPTABL_HXX
#include "simptabl.hxx"
#endif

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_XFORMS_XFORMSUIHELPER1_HPP_
#include <com/sun/star/xforms/XFormsUIHelper1.hpp>
#endif
#ifndef _COM_SUN_STAR_XFORMS_XMODEL_HPP_
#include <com/sun/star/xforms/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_XFORMS_XSUBMISSION_HPP_
#include <com/sun/star/xforms/XSubmission.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_DOM_XNODE_HPP_
#include <com/sun/star/xml/dom/XNode.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_DOM_EVENTS_XEVENTTARGET_HPP_
#include <com/sun/star/xml/dom/events/XEventTarget.hpp>
#endif

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

    public:
        DataTreeListBox( XFormsPage* pPage, DataGroupType _eGroup, const ResId& rResId );
        ~DataTreeListBox();

        virtual PopupMenu*      CreateContextMenu( void );
        virtual void            ExcecuteContextMenuAction( USHORT _nSelectedPopupEntry );
        virtual sal_Int8        AcceptDrop( const AcceptDropEvent& rEvt );
        virtual sal_Int8        ExecuteDrop( const ExecuteDropEvent& rEvt );
        virtual void            StartDrag( sal_Int8 nAction, const Point& rPosPixel );

        void                    DeleteAndClear();
        void                    RemoveEntry( SvLBoxEntry* _pEntry );
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
        ImageList                   m_TbxHCImageList;
        // these strings are not valid on the Submission and Binding Page
        String                      m_sInstanceName;
        String                      m_sInstanceURL;

        DECL_LINK(                  TbxSelectHdl, ToolBox * );
        DECL_LINK(                  ItemSelectHdl, DataTreeListBox * );

        void                        AddChildren( SvLBoxEntry* _pParent,
                                                 const ImageList& _rImgLst,
                                                 const XNode_ref& _xNode );
        void                        DoToolBoxAction( USHORT _nToolBoxID );
        SvLBoxEntry*                AddEntry( ItemNode* _pNewNode, bool _bIsElement );
        SvLBoxEntry*                AddEntry( const XPropertySet_ref& _rPropSet );
        void                        EditEntry( const XPropertySet_ref& _rPropSet );
        bool                        RemoveEntry();

    protected:
        virtual long                Notify( NotifyEvent& rNEvt );

    public:
        XFormsPage( Window* pParent, DataNavigatorWindow* _pNaviWin, DataGroupType _eGroup );
        ~XFormsPage();

        virtual void                Resize();

        inline bool                 HasModel() const { return m_bHasModel; }
        String                      SetModel( const XModel_ref& _xModel, USHORT _nPagePos );
        void                        ClearModel();
        String                      LoadInstance( const PropertyValue_seq& _xPropSeq,
                                                  const ImageList& _rImgLst );

        void                        DoMenuAction( USHORT _nMenuID );
        void                        EnableMenuItems( Menu* _pMenu );

        inline SvLBoxEntry*         GetSelectedItem() const { return m_aItemList.FirstSelected(); }
        inline const String&        GetInstanceName() const { return m_sInstanceName; }
        inline const String&        GetInstanceURL() const { return m_sInstanceURL; }
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
        USHORT                      m_nLastSelectedPos;
        bool                        m_bShowDetails;
        bool                        m_bIsNotifyDisabled;
        Size                        m_a2Size;
        Size                        m_a3Size;
        ImageList                   m_aItemImageList;
        ImageList                   m_aItemHCImageList;
        PageList                    m_aPageList;
        ContainerList               m_aContainerList;
        EventTargetList             m_aEventTargetList;
        Timer                       m_aUpdateTimer;

        DataListener_ref            m_xDataListener;
        XNameContainer_ref          m_xDataContainer;
        XFrameModel_ref             m_xFrameModel;

        DECL_LINK(                  ModelSelectHdl, ListBox * );
        DECL_LINK(                  MenuSelectHdl, MenuButton * );
        DECL_LINK(                  MenuActivateHdl, MenuButton * );
        DECL_LINK(                  ActivatePageHdl, TabControl* );
        DECL_LINK(                  UpdateHdl, Timer* );

        XFormsPage*                 GetCurrentPage( USHORT& rCurId );
        void                        LoadModels();
        void                        SetPageModel();
        void                        ClearAllPageModels( bool bClearPages );
        void                        InitPages();
        void                        CreateInstancePage( const PropertyValue_seq& _xPropSeq );
        bool                        HasFirstInstancePage() const;
        USHORT                      GetNewPageId() const;

    protected:
        virtual void                Resize();

    public:
        DataNavigatorWindow( Window* pParent );
        ~DataNavigatorWindow();

        void                        SetDocModified();
        void                        NotifyChanges();
        void                        AddContainerBroadcaster( const XContainer_ref& xContainer );
        void                        AddEventBroadcaster( const XEventTarget_ref& xTarget );

        inline const ImageList&     GetItemImageList() const { return m_aItemImageList; }
        inline const ImageList&     GetItemHCImageList() const { return m_aItemHCImageList; }
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

        void                        Update( FmFormShell* pFormShell );
        void                        StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                                  const SfxPoolItem* pState );
    };

    //========================================================================
    class DataNavigatorManager : public SfxChildWindow
    {
    public:
        DataNavigatorManager( Window* pParent, sal_uInt16 nId,
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
        DECL_LINK(          OKHdl, OKButton * );

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
        ::rtl::OUString         m_sPropertyName;

        XFormsUIHelper1_ref     m_xUIHelper;
        XPropertySet_ref        m_xBinding;

        DECL_LINK(              ModifyHdl, MultiLineEdit * );
        DECL_LINK(              ResultHdl, Timer * );
        DECL_LINK(              EditHdl, PushButton * );
        DECL_LINK(              OKHdl, OKButton * );

    public:
        AddConditionDialog( Window* pParent,
            const ::rtl::OUString& _rPropertyName, const XPropertySet_ref& _rBinding );
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
        typedef std::vector< ::rtl::OUString > PrefixList;

        FixedText           m_aNamespacesFT;
        SvxSimpleTable      m_aNamespacesList;
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

        DECL_LINK(          SelectHdl, SvxSimpleTable * );
        DECL_LINK(          ClickHdl, PushButton * );
        DECL_LINK(          OKHdl, OKButton * );

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

        DECL_LINK(          OKHdl, OKButton * );

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

        DECL_LINK(          RefHdl, PushButton * );
        DECL_LINK(          OKHdl, OKButton * );

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
        FixedLine           m_aButtonsFL;
        OKButton            m_aOKBtn;
        CancelButton        m_aEscBtn;
        HelpButton          m_aHelpBtn;

    public:
        AddModelDialog( Window* pParent, bool _bEdit );
        ~AddModelDialog();

        inline String           GetName() const { return m_aNameED.GetText(); }
        inline void             SetName( const String& _rName ) { m_aNameED.SetText( _rName );}
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

        DECL_LINK(              FilePickerHdl, PushButton * );

    public:
        AddInstanceDialog( Window* pParent, bool _bEdit );
        ~AddInstanceDialog();

        inline void             SetRenameMode();
        inline String           GetName() const { return m_aNameED.GetText(); }
        inline void             SetName( const String& _rName ) { m_aNameED.SetText( _rName );}
        inline String           GetURL() const { return m_aURLED.GetText(); }
        inline void             SetURL( const String& _rURL ) { m_aURLED.SetText( _rURL );}
        inline bool             IsLinkInstance() const { return ( m_aLinkInstanceCB.IsChecked() != FALSE ); }
        inline void             SetLinkInstance( bool _bLink ) { m_aLinkInstanceCB.Check( _bLink != false ); }
    };

    inline void AddInstanceDialog::SetRenameMode()
    {
        m_aURLFT.Disable();
        m_aURLED.Disable();
        m_aFilePickerBtn.Disable();
        m_aLinkInstanceCB.Disable();
    }

//............................................................................
}   // namespace svxform
//............................................................................

#endif // _SVX_DATANAVI_HXX

