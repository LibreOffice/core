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

#ifndef __SO_INSTANCE_HXX__
#define __SO_INSTANCE_HXX__

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <osl/diagnose.h>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/bridge/XUnoUrlResolver.hpp>
#include <com/sun/star/uno/XNamingService.hpp>
#include <com/sun/star/connection/ConnectionSetupException.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/io/XSeekable.hpp>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XCloseListener.hpp>
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/awt/WindowDescriptor.hpp>
#include <rtl/ustring.hxx>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatchHelper.hpp>

#include "ns_debug.hxx"

typedef sal_Int32 NSP_HWND;

class PluginDocumentClosePreventer;
class SoPluginInstance
{

private:
    // Service manager of remote Soffice
    static ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mxRemoteMSF;
    // Dir where Soffice is in, ie. /Soffice7/program
    static char sSO_Dir[NPP_PATH_MAX];
    int m_nWidth;
    int m_nHeight;
    int m_nX;
    int m_nY;
    sal_Int16 m_nFlag;            // Set to 12 during initialization
    OUString m_sURL; // URL of the document to be loaded
    sal_Bool m_bInit;       // If the Plugin instance is initilaized.
    NSP_HWND m_hParent;       // Windows handle of parent window
    long m_pParent;       // ID of this instance - get from NPP
    // StarOffice window
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > m_xUnoWin;
    // StarOffice component window
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > m_xComponent;
    // StarOffice frame for this Plugin
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > m_xFrame;
    // Frames of StarOffice
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrames > m_xFrames;
    // Dispatcher of frame
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchHelper > m_xDispatcher;
    // DispatchProvider of frame
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > m_xDispatchProvider;

    // the closelistener that will prevent document closing
    PluginDocumentClosePreventer* m_pCloseListener;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseListener > m_xCloseListener;

    sal_Bool LoadDocument(NSP_HWND hParent);

    long m_dParentStyl;       // Old Windows style of parent window


public:
    SoPluginInstance(long iInstance, ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xMSF);
    virtual ~SoPluginInstance(void);
    virtual sal_Bool SetURL(char* aURL);
    virtual sal_Bool IsInit(void){return m_bInit;};
    virtual sal_Bool SetWindow(NSP_HWND hParent, int x, int y, int w, int h) ;
    virtual sal_Bool Destroy(void) ;
    virtual sal_Bool Print(void) ;

    static sal_Bool ShutDown(void);
    static char* GetSODir(void) {return sSO_Dir;};
    long GetParent(void) {return m_pParent;};
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
