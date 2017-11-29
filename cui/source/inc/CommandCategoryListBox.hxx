/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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
#ifndef INCLUDED_CUI_SOURCE_INC_COMMANDCATEGORYLISTBOX_HXX
#define INCLUDED_CUI_SOURCE_INC_COMMANDCATEGORYLISTBOX_HXX

#include <vcl/lstbox.hxx>
#include <i18nutil/searchopt.hxx>
#include "cfgutil.hxx"

class CommandCategoryListBox : public ListBox
{
    SfxGroupInfoArr_Impl m_aGroupInfo;
    OUString m_sModuleLongName;
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::frame::XFrame > m_xFrame;
    css::uno::Reference< css::container::XNameAccess > m_xGlobalCategoryInfo;
    css::uno::Reference< css::container::XNameAccess > m_xModuleCategoryInfo;
    css::uno::Reference< css::container::XNameAccess > m_xUICmdDescription;

    // For search
    i18nutil::SearchOptions2 m_searchOptions;

    SfxStylesInfo_Impl* pStylesInfo;
    SfxStylesInfo_Impl m_aStylesInfo;

public:
    CommandCategoryListBox( vcl::Window* pParent );
    virtual ~CommandCategoryListBox() override;
    virtual void dispose() override;
    void ClearAll();

    void        Init(
                    const css::uno::Reference< css::uno::XComponentContext >& xContext,
                    const css::uno::Reference< css::frame::XFrame >& xFrame,
                    const OUString& sModuleLongName);
    void        FillFunctionsList(
                    const css::uno::Sequence< css::frame::DispatchInformation >& xCommands,
                    const VclPtr<SfxConfigFunctionListBox>&  pFunctionListBox,
                    const OUString& filterTerm = OUString() );
    OUString    MapCommand2UIName(const OUString& sCommand);

    /**
        Signals that a command category has been selected.
        And updates the functions list box to include
        the commands in the selected category.
    */
    void categorySelected(  const VclPtr<SfxConfigFunctionListBox>&  pFunctionListBox,
                            const OUString& filterTerm = OUString() );

    void                SetStylesInfo(SfxStylesInfo_Impl* pStyles);

    // Adds children of the given macro group to the functions list
    void addChildren(
        SvTreeListEntry* parentEntry, const css::uno::Reference<com::sun::star::script::browse::XBrowseNode> &parentNode,
        const VclPtr<SfxConfigFunctionListBox> &pFunctionListBox, const OUString &filterTerm);
};

#endif // INCLUDED_CUI_SOURCE_INC_COMMANDCATEGORYLISTBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
