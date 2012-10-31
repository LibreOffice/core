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
#ifndef _SVXSELECTOR_HXX
#define _SVXSELECTOR_HXX

#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <vcl/menubtn.hxx>
#include <svtools/treelistbox.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/script/browse/XBrowseNode.hpp>

#include <sfx2/minarray.hxx>
#include <boost/ptr_container/ptr_vector.hpp>

#define SVX_CFGGROUP_FUNCTION 1
#define SVX_CFGFUNCTION_SLOT  2
#define SVX_CFGGROUP_SCRIPTCONTAINER  3
#define SVX_CFGFUNCTION_SCRIPT 4

struct SvxGroupInfo_Impl
{
    sal_uInt16          nKind;
    sal_uInt16          nOrd;
    ::com::sun::star::uno::Reference< ::com::sun::star::script::browse::XBrowseNode >
                    xBrowseNode;
    ::rtl::OUString sURL;
    ::rtl::OUString sHelpText;
    sal_Bool            bWasOpened;

    SvxGroupInfo_Impl( sal_uInt16 n, sal_uInt16 nr )
        :nKind( n )
        ,nOrd( nr )
        ,xBrowseNode()
        ,sURL()
        ,sHelpText()
        ,bWasOpened(sal_False)
    {
    }

    SvxGroupInfo_Impl( sal_uInt16 n, sal_uInt16 nr, const ::com::sun::star::uno::Reference< ::com::sun::star::script::browse::XBrowseNode >& _rxNode )
        :nKind( n )
        ,nOrd( nr )
        ,xBrowseNode( _rxNode )
        ,sURL()
        ,sHelpText()
        ,bWasOpened(sal_False)
    {
    }

    SvxGroupInfo_Impl( sal_uInt16 n, sal_uInt16 nr, const ::rtl::OUString& _rURL, const ::rtl::OUString& _rHelpText )
        :nKind( n )
        ,nOrd( nr )
        ,xBrowseNode()
        ,sURL( _rURL )
        ,sHelpText( _rHelpText )
        ,bWasOpened(sal_False)
    {
    }
};

typedef boost::ptr_vector<SvxGroupInfo_Impl> SvxGroupInfoArr_Impl;

class ImageProvider
{
public:
    virtual ~ImageProvider() {}

    virtual Image GetImage( const rtl::OUString& rCommandURL ) = 0;
};

class SvxConfigFunctionListBox : public SvTreeListBox
{
friend class SvxConfigGroupListBox;
    Timer                           aTimer;
    SvTreeListEntry*                    pCurEntry;
    SvxGroupInfoArr_Impl            aArr;
    SvTreeListEntry*                    m_pDraggingEntry;

    DECL_LINK(TimerHdl, void *);
    virtual void                    MouseMove( const MouseEvent& rMEvt );

    void Init();

public:
                                    SvxConfigFunctionListBox(Window*, const ResId&);
                                    SvxConfigFunctionListBox(Window* pParent);
                                    ~SvxConfigFunctionListBox();
    void                            ClearAll();
    String                          GetHelpText( SvTreeListEntry *pEntry );
    using Window::GetHelpText;
    SvTreeListEntry*                GetLastSelectedEntry();
    void                            FunctionSelected();

    // drag n drop methods
    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );

    virtual DragDropMode    NotifyStartDrag(
        TransferDataContainer&, SvTreeListEntry* );

    virtual void        DragFinished( sal_Int8 );
};

class SvxConfigGroupListBox : public SvTreeListBox
{
    SvxGroupInfoArr_Impl            aArr;
    bool                            m_bShowSlots;

    SvxConfigFunctionListBox*  pFunctionListBox;
    ImageProvider*                  m_pImageProvider;

    ::com::sun::star::uno::Reference
        < ::com::sun::star::frame::XFrame > m_xFrame;

    ::com::sun::star::uno::Reference
        < ::com::sun::star::container::XNameAccess > m_xModuleCommands;

    Image m_hdImage;
    Image m_libImage;
    Image m_macImage;
    Image m_docImage;
    ::rtl::OUString m_sMyMacros;
    ::rtl::OUString m_sProdMacros;
    Image GetImage(
        ::com::sun::star::uno::Reference< ::com::sun::star::script::browse::XBrowseNode > node,
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > xCtx,
        bool bIsRootNode
    );

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface  > getDocumentModel(
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xCtx,
        ::rtl::OUString& docName
    );

private:
    void    fillScriptList(
        const ::com::sun::star::uno::Reference< ::com::sun::star::script::browse::XBrowseNode >& _rxRootNode,
        SvTreeListEntry* _pParentEntry,
        bool _bCheapChildrenOnDemand
    );

protected:
    virtual void    RequestingChildren( SvTreeListEntry *pEntry);
    virtual sal_Bool    Expand( SvTreeListEntry* pParent );
    using SvListView::Expand;

public:
            SvxConfigGroupListBox (
                Window* pParent, const ResId&,
                bool _bShowSlots,
                const ::com::sun::star::uno::Reference
                    < ::com::sun::star::frame::XFrame >& xFrame
            );
            SvxConfigGroupListBox(Window* pParent);
            ~SvxConfigGroupListBox();

    void    Init(bool bShowSlots, const ::com::sun::star::uno::Reference
                    < ::com::sun::star::frame::XFrame >& xFrame);

    void    Open( SvTreeListEntry*, sal_Bool );
    void    ClearAll();
    void    GroupSelected();

    void    SetFunctionListBox( SvxConfigFunctionListBox *pBox )
        { pFunctionListBox = pBox; }

    void    SetImageProvider( ImageProvider* provider )
        { m_pImageProvider = provider; }
};

class SvxScriptSelectorDialog : public ModelessDialog
{
    FixedText*                      m_pDialogDescription;
    SvxConfigGroupListBox*          m_pCategories;
    SvxConfigFunctionListBox*       m_pCommands;
    PushButton*                     m_pOKButton;
    PushButton*                     m_pCancelButton;
    VclMultiLineEdit*               m_pDescriptionText;
    OUString                        m_sDefaultDesc;
    sal_Bool                        m_bShowSlots;
    Link                            m_aAddHdl;

    DECL_LINK( ClickHdl, Button * );
    DECL_LINK( SelectHdl, Control* );
    DECL_LINK( FunctionDoubleClickHdl, Control* );

    void                            UpdateUI();

public:

    SvxScriptSelectorDialog (
        Window* pParent = NULL,
        sal_Bool bShowSlots = sal_False,
        const ::com::sun::star::uno::Reference
            < ::com::sun::star::frame::XFrame >& xFrame = 0
    );

    ~SvxScriptSelectorDialog ( );

    void        SetAddHdl( const Link& rLink ) { m_aAddHdl = rLink; }
    const Link& GetAddHdl() const { return m_aAddHdl; }

    void        SetImageProvider(ImageProvider* provider)
    {
        m_pCategories->SetImageProvider(provider);
    }

    String      GetScriptURL() const;
    String      GetSelectedDisplayName();
    String      GetSelectedHelpText();
    void        SetRunLabel();
    void        SetDialogDescription(const String& rDescription);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
