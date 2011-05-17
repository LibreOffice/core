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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_TABORDER_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_TABORDER_HXX

#include <svtools/svtreebx.hxx>
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
        TabOrderListBox( Window* pParent, const ResId& rResId  );
        virtual ~TabOrderListBox();

        void            MoveSelection( long nRelPos );

    protected:
        virtual void    ModelHasMoved(SvListEntry* pSource );

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
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                                    m_xORB;

        FixedText                   aFT_Controls;
        TabOrderListBox         aLB_Controls;

        OKButton                    aPB_OK;
        CancelButton                aPB_CANCEL;
        HelpButton                  aPB_HELP;

        PushButton                  aPB_MoveUp;
        PushButton                  aPB_MoveDown;
        PushButton                  aPB_AutoOrder;

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
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
        );

        virtual ~TabOrderDialog();

        void SetModified();
    };

//............................................................................
}  // namespace pcr
//............................................................................

#endif  // EXTENSIONS_SOURCE_PROPCTRLR_TABORDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
