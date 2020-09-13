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

#pragma once

#include <com/sun/star/container/XNameAccess.hpp>
#include <vcl/weld.hxx>

class SvxHyperlinkTabPageBase;

//#                                                                      #
//# Window-Class                                                         #
//#                                                                      #
class SvxHlinkDlgMarkWnd : public weld::GenericDialogController
{
private:
    friend class SvxHlmarkTreeLBox;

    SvxHyperlinkTabPageBase* mpParent;

    sal_uInt16          mnError;

    std::unique_ptr<weld::Button> mxBtApply;
    std::unique_ptr<weld::Button> mxBtClose;
    std::unique_ptr<weld::TreeView> mxLbTree;
    std::unique_ptr<weld::Label> mxError;

    void ErrorChanged();

protected:
    bool RefreshFromDoc( const OUString& aURL );
    void RestoreLastSelection();

    std::unique_ptr<weld::TreeIter> FindEntry(const OUString& aStrName);
    void ClearTree();
    int FillTree( const css::uno::Reference< css::container::XNameAccess >& xLinks, const weld::TreeIter* pParentEntry =nullptr );

    DECL_LINK( ClickApplyHdl_Impl, weld::Button&, void );
    DECL_LINK( DoubleClickApplyHdl_Impl, weld::TreeView&, bool );
    DECL_LINK( ClickCloseHdl_Impl, weld::Button&, void );

public:
    SvxHlinkDlgMarkWnd(weld::Window* pParentDialog, SvxHyperlinkTabPageBase *pParentPage);
    virtual ~SvxHlinkDlgMarkWnd() override;

    void MoveTo(const Point& rNewPos);
    void RefreshTree(const OUString& aStrURL);
    bool SelectEntry(const OUString& aStrMark);

    sal_uInt16 SetError( sal_uInt16 nError);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
