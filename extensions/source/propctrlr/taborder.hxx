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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_TABORDER_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_TABORDER_HXX

#include <svtools/treelistbox.hxx>
#include <com/sun/star/awt/XTabControllerModel.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <vcl/dialog.hxx>

#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>

#include <vcl/fixed.hxx>

#include <comphelper/uno3.hxx>

//............................................................................
namespace pcr
{
//............................................................................

    //========================================================================
    //= TabOrderListBox
    //========================================================================
    class TabOrderListBox : public SvTreeListBox
    {
    public:
        TabOrderListBox( Window* pParent, WinBits nBits  );
        virtual ~TabOrderListBox();

        void            MoveSelection( long nRelPos );

    protected:
        virtual void    ModelHasMoved(SvTreeListEntry* pSource );

    private:
        using SvTreeListBox::MoveSelection;
    };


    //========================================================================
    //= TabOrderDialog
    //========================================================================
    class TabOrderDialog : public ModalDialog
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabControllerModel >
                                    m_xTempModel;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabControllerModel >
                                    m_xModel;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >
                                    m_xControlContainer;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
                                    m_xORB;

        TabOrderListBox* m_pLB_Controls;

        OKButton* m_pPB_OK;

        PushButton* m_pPB_MoveUp;
        PushButton* m_pPB_MoveDown;
        PushButton* m_pPB_AutoOrder;

        ImageList*                  pImageList;

        DECL_LINK( MoveUpClickHdl, Button* );
        DECL_LINK( MoveDownClickHdl, Button* );
        DECL_LINK( AutoOrderClickHdl, Button* );
        DECL_LINK( OKClickHdl, Button* );

        void FillList();
        Image GetImage(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & _rxSet
        ) const;

    public:
        TabOrderDialog(
            Window* _pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabControllerModel >& _rxTabModel,
            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >& _rxControlCont,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxORB
        );

        virtual ~TabOrderDialog();

        void SetModified();
    };

//............................................................................
}  // namespace pcr
//............................................................................

#endif  // EXTENSIONS_SOURCE_PROPCTRLR_TABORDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
