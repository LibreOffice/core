/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SVXSELECTOR_HXX
#define _SVXSELECTOR_HXX

#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <vcl/menubtn.hxx>
#include <svtools/svtreebx.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/script/browse/XBrowseNode.hpp>

#define _SVSTDARR_USHORTS
#define _SVSTDARR_STRINGSDTOR
#include <svl/svstdarr.hxx>     // SvUShorts
#include <sfx2/minarray.hxx>

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

typedef SvxGroupInfo_Impl* SvxGroupInfoPtr;
SV_DECL_PTRARR_DEL(SvxGroupInfoArr_Impl, SvxGroupInfoPtr, 5, 5)

class ImageProvider
{
public:
    virtual ~ImageProvider() {}

    virtual Image GetImage( const rtl::OUString& rCommandURL ) = 0;
};

class SvxConfigFunctionListBox_Impl : public SvTreeListBox
{
friend class SvxConfigGroupListBox_Impl;
    Timer                           aTimer;
    SvLBoxEntry*                    pCurEntry;
    SvxGroupInfoArr_Impl            aArr;
    SvLBoxEntry*                    m_pDraggingEntry;

    DECL_LINK(                      TimerHdl, Timer* );
    virtual void                    MouseMove( const MouseEvent& rMEvt );

public:
                                    SvxConfigFunctionListBox_Impl( Window*, const ResId& );
                                    ~SvxConfigFunctionListBox_Impl();
    void                            ClearAll();
    String                          GetHelpText( SvLBoxEntry *pEntry );
    using Window::GetHelpText;
    SvLBoxEntry*                    GetLastSelectedEntry();
    void                            FunctionSelected();

    // drag n drop methods
    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );

    virtual DragDropMode    NotifyStartDrag(
        TransferDataContainer&, SvLBoxEntry* );

    virtual void        DragFinished( sal_Int8 );
};

class SvxConfigGroupListBox_Impl : public SvTreeListBox
{
    SvxGroupInfoArr_Impl            aArr;
    bool                            m_bShowSlots;

    SvxConfigFunctionListBox_Impl*  pFunctionListBox;
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
        SvLBoxEntry* _pParentEntry,
        bool _bCheapChildsOnDemand
    );

protected:
    virtual void    RequestingChilds( SvLBoxEntry *pEntry);
    virtual sal_Bool    Expand( SvLBoxEntry* pParent );
    using SvListView::Expand;

public:
            SvxConfigGroupListBox_Impl (
                Window* pParent, const ResId&,
                bool _bShowSlots,
                const ::com::sun::star::uno::Reference
                    < ::com::sun::star::frame::XFrame >& xFrame
            );

            ~SvxConfigGroupListBox_Impl();

    void    Init();
    void    Open( SvLBoxEntry*, sal_Bool );
    void    ClearAll();
    void    GroupSelected();

    void    SetFunctionListBox( SvxConfigFunctionListBox_Impl *pBox )
        { pFunctionListBox = pBox; }

    void    SetImageProvider( ImageProvider* provider )
        { m_pImageProvider = provider; }
};

class SvxScriptSelectorDialog : public ModelessDialog
{
    FixedText                       aDialogDescription;
    FixedText                       aGroupText;
    SvxConfigGroupListBox_Impl      aCategories;
    FixedText                       aFunctionText;
    SvxConfigFunctionListBox_Impl   aCommands;
    OKButton                        aOKButton;
    CancelButton                    aCancelButton;
    HelpButton                      aHelpButton;
    FixedLine                       aDescription;
    FixedText                       aDescriptionText;

    sal_Bool                            m_bShowSlots;
    Link                            m_aAddHdl;

    DECL_LINK( ClickHdl, Button * );
    DECL_LINK( SelectHdl, Control* );
    DECL_LINK( FunctionDoubleClickHdl, Control* );

    void                                UpdateUI();
    void                                ResizeControls();

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

    void        SetImageProvider( ImageProvider* provider )
        { aCategories.SetImageProvider( provider ); }

    String      GetScriptURL() const;
    String      GetSelectedDisplayName();
    String      GetSelectedHelpText();
    void        SetRunLabel();
    void        SetDialogDescription(const String& rDescription);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
