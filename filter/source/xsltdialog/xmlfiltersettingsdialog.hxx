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
#ifndef _XMLFILTERSETTINGSDIALOG_HXX_
#define _XMLFILTERSETTINGSDIALOG_HXX_

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XHierarchicalName.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <vcl/wrkwin.hxx>

#include <vcl/button.hxx>
#include <svtools/svtabbx.hxx>
#include <svl/poolitem.hxx>
#include <unotools/moduleoptions.hxx>

#include "xmlfiltercommon.hxx"

// --------------------------------------------------------------------

class SvxPathControl_Impl : public Control
{
private:
    Control*		m_pFocusCtrl;

public:
    SvxPathControl_Impl( Window* pParent, const ResId& rId ) :
        Control( pParent, rId ), m_pFocusCtrl( NULL ) {}

    void			SetFocusControl( Control* pCtrl ) { m_pFocusCtrl = pCtrl; }

    virtual long	Notify( NotifyEvent& rNEvt );
};

// --------------------------------------------------------------------

class HeaderBar;

class XMLFilterListBox : public SvTabListBox
{
private:
    bool		mbFirstPaint;
    HeaderBar* 	mpHeaderBar;

    DECL_LINK( TabBoxScrollHdl_Impl, SvTabListBox* );
    DECL_LINK( HeaderSelect_Impl, HeaderBar* );
    DECL_LINK( HeaderEndDrag_Impl, HeaderBar* );

    String getEntryString( const filter_info_impl* pInfo ) const;

public:
    XMLFilterListBox( SvxPathControl_Impl* pParent );
    ~XMLFilterListBox();

    void Reset();

    /** adds a new filter info entry to the ui filter list */
    void addFilterEntry( const filter_info_impl* pInfo );

    void changeEntry( const filter_info_impl* pInfo );

    virtual void	Paint( const Rectangle& rRect );
};

// --------------------------------------------------------------------

class XMLFilterTestDialog;

class XMLFilterSettingsDialog : public WorkWindow
{
public:
    XMLFilterSettingsDialog( Window* pParent, ResMgr& rResMgr, const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rxMSF  );
    virtual ~XMLFilterSettingsDialog();

    DECL_LINK(ClickHdl_Impl, PushButton * );
    DECL_LINK(SelectionChangedHdl_Impl, void * );
    DECL_LINK(DoubleClickHdl_Impl, void * );

    void ShowWindow();

    void	onNew();
    void	onEdit();
    void	onTest();
    void	onDelete();
    void	onSave();
    void	onOpen();
    void	onClose();

    void	updateStates();

    virtual long	Notify( NotifyEvent& rNEvt );

    bool	isClosable();

    static ResMgr* mpResMgr;

private:
    void	initFilterList();
    void	disposeFilterList();

    bool	insertOrEdit( filter_info_impl* pNewInfo, const filter_info_impl* pOldInfo = NULL );

    rtl::OUString createUniqueFilterName( const rtl::OUString& rUIName );
    rtl::OUString createUniqueTypeName( const rtl::OUString& rTypeName );
    rtl::OUString createUniqueInterfaceName( const rtl::OUString& rInterfaceName );

private:

    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > mxMSF;
    com::sun::star::uno::Reference< com::sun::star::container::XNameContainer > mxFilterContainer;
    com::sun::star::uno::Reference< com::sun::star::container::XNameContainer > mxTypeDetection;
    com::sun::star::uno::Reference< com::sun::star::container::XNameContainer > mxExtendedTypeDetection;

    std::vector< filter_info_impl* > maFilterVector;

    XMLFilterListBox*	mpFilterListBox;
    SvxPathControl_Impl	maCtrlFilterList;
    PushButton	maPBNew;
    PushButton	maPBEdit;
    PushButton	maPBTest;
    PushButton	maPBDelete;
    PushButton	maPBSave;
    PushButton	maPBOpen;
    HelpButton	maPBHelp;
    PushButton	maPBClose;

    bool	mbIsClosable;

    ::rtl::OUString sTemplatePath;
    ::rtl::OUString sDocTypePrefix;

    SvtModuleOptions maModuleOpt;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
