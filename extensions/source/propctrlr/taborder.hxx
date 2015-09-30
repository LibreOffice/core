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

#ifndef INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_TABORDER_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_TABORDER_HXX

#include <svtools/treelistbox.hxx>
#include <com/sun/star/awt/XTabControllerModel.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <vcl/dialog.hxx>

#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>

#include <vcl/fixed.hxx>

#include <comphelper/uno3.hxx>


namespace pcr
{



    //= TabOrderListBox

    class TabOrderListBox : public SvTreeListBox
    {
    public:
        TabOrderListBox( vcl::Window* pParent, WinBits nBits  );
        virtual ~TabOrderListBox();

        void            MoveSelection( long nRelPos );

    protected:
        virtual void    ModelHasMoved(SvTreeListEntry* pSource ) SAL_OVERRIDE;

    private:
        using SvTreeListBox::MoveSelection;
    };



    //= TabOrderDialog

    class TabOrderDialog : public ModalDialog
    {
        css::uno::Reference< css::awt::XTabControllerModel >
                                    m_xTempModel;
        css::uno::Reference< css::awt::XTabControllerModel >
                                    m_xModel;
        css::uno::Reference< css::awt::XControlContainer >
                                    m_xControlContainer;
        css::uno::Reference< css::uno::XComponentContext >
                                    m_xORB;

        VclPtr<TabOrderListBox>     m_pLB_Controls;

        VclPtr<OKButton>            m_pPB_OK;

        VclPtr<PushButton>          m_pPB_MoveUp;
        VclPtr<PushButton>          m_pPB_MoveDown;
        VclPtr<PushButton>          m_pPB_AutoOrder;

        ImageList*                  pImageList;

        DECL_LINK_TYPED( MoveUpClickHdl, Button*, void );
        DECL_LINK_TYPED( MoveDownClickHdl, Button*, void );
        DECL_LINK_TYPED( AutoOrderClickHdl, Button*, void );
        DECL_LINK_TYPED( OKClickHdl, Button*, void );

        void FillList();
        Image GetImage(
            const css::uno::Reference< css::beans::XPropertySet > & _rxSet
        ) const;

    public:
        TabOrderDialog(
            vcl::Window* _pParent,
            const css::uno::Reference< css::awt::XTabControllerModel >& _rxTabModel,
            const css::uno::Reference< css::awt::XControlContainer >& _rxControlCont,
            const css::uno::Reference< css::uno::XComponentContext >& _rxORB
        );

        virtual ~TabOrderDialog();
        virtual void dispose() SAL_OVERRIDE;

        void SetModified();
    };


}  // namespace pcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_TABORDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
