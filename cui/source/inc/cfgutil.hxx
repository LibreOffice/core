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
#ifndef _CFGUTIL_HXX
#define _CFGUTIL_HXX

#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>
#include <rtl/ustring.hxx>
#include <tools/string.hxx>

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/script/browse/XBrowseNode.hpp>
#include <vcl/timer.hxx>
#include <svtools/svtabbx.hxx>
#include <svtools/treelistbox.hxx>
#include <vcl/image.hxx>

class SfxMacroInfoItem;

namespace css = ::com::sun::star;

struct SfxStyleInfo_Impl
{
    ::rtl::OUString sFamily;
    ::rtl::OUString sStyle;
    ::rtl::OUString sCommand;
    ::rtl::OUString sLabel;

    SfxStyleInfo_Impl()
    {}

    SfxStyleInfo_Impl(const SfxStyleInfo_Impl& rCopy)
    {
        sFamily  = rCopy.sFamily;
        sStyle   = rCopy.sStyle;
        sCommand = rCopy.sCommand;
        sLabel   = rCopy.sLabel;
    }
};

struct SfxStylesInfo_Impl
{
    private:

        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > m_xDoc;

    public:

        SfxStylesInfo_Impl();
        void setModel(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModel);

        sal_Bool parseStyleCommand(SfxStyleInfo_Impl& aStyle);
        void getLabel4Style(SfxStyleInfo_Impl& aStyle);

        ::std::vector< SfxStyleInfo_Impl > getStyleFamilies();
        ::std::vector< SfxStyleInfo_Impl > getStyles(const ::rtl::OUString& sFamily);

        static ::rtl::OUString generateCommand(const ::rtl::OUString& sFamily, const ::rtl::OUString& sStyle);
};

#define SFX_CFGGROUP_FUNCTION           1
#define SFX_CFGFUNCTION_SLOT            2
#define SFX_CFGGROUP_SCRIPTCONTAINER    3
#define SFX_CFGFUNCTION_SCRIPT          4
#define SFX_CFGGROUP_STYLES             5

struct SfxGroupInfo_Impl
{
    sal_uInt16  nKind;
    sal_uInt16  nUniqueID;
    void*       pObject;
    sal_Bool        bWasOpened;
    String      sCommand;
    String      sLabel;

                SfxGroupInfo_Impl( sal_uInt16 n, sal_uInt16 nr, void* pObj = 0 ) :
                    nKind( n ), nUniqueID( nr ), pObject( pObj ), bWasOpened(sal_False) {}
};

struct CuiMacroInfo
{
};

typedef boost::ptr_vector<SfxGroupInfo_Impl> SfxGroupInfoArr_Impl;

class SfxConfigFunctionListBox_Impl : public SvTreeListBox
{
    friend class SfxConfigGroupListBox_Impl;
    Timer                aTimer;
    SvLBoxEntry*         pCurEntry;
    SfxGroupInfoArr_Impl aArr;
    SfxStylesInfo_Impl*  pStylesInfo;

    DECL_LINK( TimerHdl, Timer* );
    virtual void  MouseMove( const MouseEvent& rMEvt );

public:
                  SfxConfigFunctionListBox_Impl( Window*, const ResId& );
                  ~SfxConfigFunctionListBox_Impl();

    void          ClearAll();
    using Window::GetHelpText;
    String        GetHelpText( SvLBoxEntry *pEntry );
    String        GetCurCommand();
    String        GetCurLabel();
    String        GetSelectedScriptURI();
    void          FunctionSelected();
    void          SetStylesInfo(SfxStylesInfo_Impl* pStyles);
};

struct SvxConfigGroupBoxResource_Impl;
class SfxConfigGroupListBox_Impl : public SvTreeListBox
{
    SvxConfigGroupBoxResource_Impl* pImp;
    SfxConfigFunctionListBox_Impl*  pFunctionListBox;
    SfxGroupInfoArr_Impl            aArr;
    sal_uLong                       nMode;

    ::rtl::OUString m_sModuleLongName;
    css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;
    css::uno::Reference< css::frame::XFrame > m_xFrame;
    css::uno::Reference< css::container::XNameAccess > m_xGlobalCategoryInfo;
    css::uno::Reference< css::container::XNameAccess > m_xModuleCategoryInfo;
    css::uno::Reference< css::container::XNameAccess > m_xUICmdDescription;

    Image GetImage(
        ::com::sun::star::uno::Reference< ::com::sun::star::script::browse::XBrowseNode > node,
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > xCtx,
        bool bIsRootNode
    );

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface  > getDocumentModel(
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xCtx,
        ::rtl::OUString& docName
    );


    void InitModule();
    void InitBasic();
    void InitStyles();

    ::rtl::OUString MapCommand2UIName(const ::rtl::OUString& sCommand);

    SfxStylesInfo_Impl* pStylesInfo;

protected:
    virtual void        RequestingChildren( SvLBoxEntry *pEntry);
    using SvListView::Expand;
    virtual sal_Bool        Expand( SvLBoxEntry* pParent );

public:
    SfxConfigGroupListBox_Impl ( Window* pParent,
                                 const ResId&,
                                 sal_uLong nConfigMode = 0 );
    ~SfxConfigGroupListBox_Impl();
    void                ClearAll();

    void                Init(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR          ,
                             const css::uno::Reference< css::frame::XFrame >&              xFrame         ,
                             const ::rtl::OUString&                                        sModuleLongName);
    void                SetFunctionListBox( SfxConfigFunctionListBox_Impl *pBox )
                        { pFunctionListBox = pBox; }
    void                Open( SvLBoxEntry*, sal_Bool );
    void                GroupSelected();
    void                SelectMacro( const SfxMacroInfoItem* );
    void                SelectMacro( const String&, const String& );
    String              GetGroup();
    void                SetStylesInfo(SfxStylesInfo_Impl* pStyles);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
