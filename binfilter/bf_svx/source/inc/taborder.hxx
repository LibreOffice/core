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
#ifndef _SVX_TABORDER_HXX
#define _SVX_TABORDER_HXX

#include <bf_svtools/svtreebx.hxx>

#include <com/sun/star/awt/XTabControllerModel.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>

#include <vcl/dialog.hxx>

#include <vcl/button.hxx>


#include <vcl/fixed.hxx>


#include "fmexch.hxx"

class SdrModel;
class ImageList;

#include <comphelper/uno3.hxx>
FORWARD_DECLARE_INTERFACE(beans,XPropertySet)
namespace binfilter {
//STRIP008 FORWARD_DECLARE_INTERFACE(beans,XPropertySet)

//========================================================================
class FmOrderTreeListBox : public SvTreeListBox
{
protected:
    ::binfilter::svxform::OControlExchangeHelper	m_aFieldExchange;//STRIP008 	::svxform::OControlExchangeHelper	m_aFieldExchange;

public:
    FmOrderTreeListBox( Window* pParent );
    FmOrderTreeListBox( Window* pParent, const ResId& rResId  );
    virtual ~FmOrderTreeListBox();


protected:
    virtual sal_Int8	AcceptDrop( const AcceptDropEvent& rEvt );
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt );
    virtual void		StartDrag( sal_Int8 nAction, const Point& rPosPixel );
};


//========================================================================
class FmFormShell;
class FmTabOrderDlg : public ModalDialog
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabControllerModel > 		xTempModel,	// Model zum Bearbeiten der TabReihenfolge
                                xModel;		// Model mit der zu veraendernden TabReihenfolge

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer > 		xControlContainer;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >	m_xORB;
    SdrModel*					pDrawModel;

    FixedText                   aFT_Controls;
    FmOrderTreeListBox			aLB_Controls;

    OKButton					aPB_OK;
    CancelButton				aPB_CANCEL;
    HelpButton					aPB_HELP;

    PushButton                  aPB_MoveUp;
    PushButton					aPB_MoveDown;
    PushButton					aPB_AutoOrder;

    ImageList*					pImageList;

    DECL_LINK( MoveUpClickHdl, Button* );
    DECL_LINK( MoveDownClickHdl, Button* );
    DECL_LINK( AutoOrderClickHdl, Button* );
    DECL_LINK( OKClickHdl, Button* );


public:
    FmTabOrderDlg(	const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&	_xORB,
                    Window* pParent, FmFormShell* pShell );
    virtual ~FmTabOrderDlg();

};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
