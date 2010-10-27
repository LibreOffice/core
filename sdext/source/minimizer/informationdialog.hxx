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

#ifndef INFORMATIONDIALOG_HXX
#define INFORMATIONDIALOG_HXX
#include <vector>
#include "unodialog.hxx"
#include "configurationaccess.hxx"
#include "pppoptimizertoken.hxx"
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/awt/XItemListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/awt/XItemEventBroadcaster.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/awt/PushButtonType.hpp>
#include <com/sun/star/io/XStream.hpp>

// ---------------------
// - InformationDialog -
// ---------------------
class InformationDialog : public UnoDialog, public ConfigurationAccess
{
public :

    InformationDialog( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxMSF,
            com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rxFrame, const rtl::OUString& rSaveAsURL,
                sal_Bool& bOpenNewDocument, const sal_Int64& nSourceSize, const sal_Int64& nDestSize, const sal_Int64& nApproxDest );
    ~InformationDialog();

    sal_Bool                execute();

private :

    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >mxMSF;
    com::sun::star::uno::Reference< com::sun::star::frame::XFrame >         mxFrame;
    com::sun::star::uno::Reference< com::sun::star::io::XStream >           mxTempFile;

    com::sun::star::uno::Reference< com::sun::star::awt::XActionListener >  mxActionListener;

    rtl::OUString ImpGetStandardImage( const rtl::OUString& rPrivateURL );
    void InitDialog();

    sal_Int64 mnSourceSize;
    sal_Int64 mnDestSize;
    sal_Int64 mnApproxSize;
    sal_Bool& mrbOpenNewDocument;
    const rtl::OUString& maSaveAsURL;

public :

    com::sun::star::uno::Reference< com::sun::star::frame::XFrame>& GetFrame() { return mxFrame; };
    const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& GetComponentContext() { return mxMSF; };
};

class OKActionListener : public ::cppu::WeakImplHelper1< com::sun::star::awt::XActionListener >
{
public:
    OKActionListener( InformationDialog& rInformationDialog ) : mrInformationDialog( rInformationDialog ){};

    virtual void SAL_CALL actionPerformed( const ::com::sun::star::awt::ActionEvent& Event ) throw ( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw ( com::sun::star::uno::RuntimeException);
private:

    InformationDialog& mrInformationDialog;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
