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
#ifndef INCLUDED_CUI_SOURCE_INC_SELECTOR_HXX
#define INCLUDED_CUI_SOURCE_INC_SELECTOR_HXX

#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <vcl/menubtn.hxx>
#include <svtools/treelistbox.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/script/browse/XBrowseNode.hpp>

#include <boost/ptr_container/ptr_vector.hpp>

class SaveInData;

#define SVX_CFGGROUP_FUNCTION 1
#define SVX_CFGFUNCTION_SLOT  2
#define SVX_CFGGROUP_SCRIPTCONTAINER  3
#define SVX_CFGFUNCTION_SCRIPT 4

struct SvxGroupInfo_Impl
{
    sal_uInt16          nKind;
    sal_uInt16          nOrd;
    css::uno::Reference< css::script::browse::XBrowseNode >
                        xBrowseNode;
    OUString            sURL;
    OUString            sHelpText;
    bool                bWasOpened;

    SvxGroupInfo_Impl( sal_uInt16 n, sal_uInt16 nr )
        :nKind( n )
        ,nOrd( nr )
        ,xBrowseNode()
        ,sURL()
        ,sHelpText()
        ,bWasOpened(false)
    {
    }

    SvxGroupInfo_Impl( sal_uInt16 n, sal_uInt16 nr, const css::uno::Reference< css::script::browse::XBrowseNode >& _rxNode )
        :nKind( n )
        ,nOrd( nr )
        ,xBrowseNode( _rxNode )
        ,sURL()
        ,sHelpText()
        ,bWasOpened(false)
    {
    }

    SvxGroupInfo_Impl( sal_uInt16 n, sal_uInt16 nr, const OUString& _rURL, const OUString& _rHelpText )
        :nKind( n )
        ,nOrd( nr )
        ,xBrowseNode()
        ,sURL( _rURL )
        ,sHelpText( _rHelpText )
        ,bWasOpened(false)
    {
    }
};

typedef boost::ptr_vector<SvxGroupInfo_Impl> SvxGroupInfoArr_Impl;

class SvxConfigFunctionListBox : public SvTreeListBox
{
friend class SvxConfigGroupListBox;
    Timer                           aTimer;
    SvTreeListEntry*                    pCurEntry;
    SvxGroupInfoArr_Impl            aArr;
    SvTreeListEntry*                    m_pDraggingEntry;

    DECL_LINK_TYPED(TimerHdl, Timer *, void);
    virtual void                    MouseMove( const MouseEvent& rMEvt ) override;

public:
    SvxConfigFunctionListBox(vcl::Window* pParent, WinBits nStyle);
    virtual ~SvxConfigFunctionListBox();
    virtual void                    dispose() override;
    void                            ClearAll();
    OUString                        GetHelpText( SvTreeListEntry *pEntry );
    using Window::GetHelpText;
    SvTreeListEntry*                GetLastSelectedEntry();
    void                            FunctionSelected();

    // drag n drop methods
    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt ) override;

    virtual DragDropMode    NotifyStartDrag(
        TransferDataContainer&, SvTreeListEntry* ) override;

    virtual void        DragFinished( sal_Int8 ) override;
};

class SvxConfigGroupListBox : public SvTreeListBox
{
    SvxGroupInfoArr_Impl            aArr;
    bool                            m_bShowSlots;

    VclPtr<SvxConfigFunctionListBox>  pFunctionListBox;
    SaveInData*                     m_pImageProvider;

    css::uno::Reference
        < css::frame::XFrame > m_xFrame;

    css::uno::Reference
        < css::container::XNameAccess > m_xModuleCommands;

    Image m_hdImage;
    Image m_libImage;
    Image m_macImage;
    Image m_docImage;
    OUString m_sMyMacros;
    OUString m_sProdMacros;
    Image GetImage(
        css::uno::Reference< css::script::browse::XBrowseNode > node,
        css::uno::Reference< css::uno::XComponentContext > xCtx,
        bool bIsRootNode
    );

    static css::uno::Reference< css::uno::XInterface  > getDocumentModel(
        css::uno::Reference< css::uno::XComponentContext >& xCtx,
        OUString& docName
    );

private:
    void    fillScriptList(
        const css::uno::Reference< css::script::browse::XBrowseNode >& _rxRootNode,
        SvTreeListEntry* _pParentEntry,
        bool _bCheapChildrenOnDemand
    );

protected:
    virtual void    RequestingChildren( SvTreeListEntry *pEntry) override;
    virtual bool    Expand( SvTreeListEntry* pParent ) override;

public:
    SvxConfigGroupListBox(vcl::Window* pParent, WinBits nStyle);
    virtual ~SvxConfigGroupListBox();
    virtual void dispose() override;

    void    Init(bool bShowSlots, const css::uno::Reference
                    < css::frame::XFrame >& xFrame);

    void    ClearAll();
    void    GroupSelected();

    void    SetFunctionListBox( SvxConfigFunctionListBox *pBox )
        { pFunctionListBox = pBox; }

    void    SetImageProvider( SaveInData* provider )
        { m_pImageProvider = provider; }
};

class SvxScriptSelectorDialog : public ModelessDialog
{
    VclPtr<FixedText>                      m_pDialogDescription;
    VclPtr<SvxConfigGroupListBox>          m_pCategories;
    VclPtr<SvxConfigFunctionListBox>       m_pCommands;
    VclPtr<PushButton>                     m_pOKButton;
    VclPtr<PushButton>                     m_pCancelButton;
    VclPtr<VclMultiLineEdit>               m_pDescriptionText;
    OUString                               m_sDefaultDesc;
    bool                                   m_bShowSlots;
    Link<SvxScriptSelectorDialog&,void>    m_aAddHdl;

    DECL_LINK_TYPED( ClickHdl, Button *, void );
    DECL_LINK_TYPED( SelectHdl, SvTreeListBox*, void );
    DECL_LINK_TYPED( FunctionDoubleClickHdl, SvTreeListBox*, bool );

    void                            UpdateUI();

public:

    SvxScriptSelectorDialog (
        vcl::Window* pParent = nullptr,
        bool bShowSlots = false,
        const css::uno::Reference< css::frame::XFrame >& xFrame = nullptr
    );
    virtual ~SvxScriptSelectorDialog();
    virtual void dispose() override;

    void        SetAddHdl( const Link<SvxScriptSelectorDialog&,void>& rLink ) { m_aAddHdl = rLink; }
    const Link<SvxScriptSelectorDialog&,void>& GetAddHdl() const { return m_aAddHdl; }

    void        SetImageProvider(SaveInData* provider)
    {
        m_pCategories->SetImageProvider(provider);
    }

    OUString    GetScriptURL() const;
    OUString    GetSelectedDisplayName();
    void        SetRunLabel();
    void        SetDialogDescription(const OUString& rDescription);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
