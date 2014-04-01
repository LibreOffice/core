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
#ifndef INCLUDED_FILTER_SOURCE_XSLTDIALOG_XMLFILTERSETTINGSDIALOG_HXX
#define INCLUDED_FILTER_SOURCE_XSLTDIALOG_XMLFILTERSETTINGSDIALOG_HXX

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XHierarchicalName.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/layout.hxx>
#include <svtools/svtabbx.hxx>
#include <svl/poolitem.hxx>
#include <unotools/moduleoptions.hxx>

#include "xmlfiltercommon.hxx"



class HeaderBar;
class XMLFilterListBox;

class SvxPathControl : public VclVBox
{
private:
    bool bHasBeenShown;
    HeaderBar* m_pHeaderBar;
    XMLFilterListBox* m_pFocusCtrl;
protected:
    virtual void setAllocation(const Size &rAllocation) SAL_OVERRIDE;
public:
    SvxPathControl(Window* pParent);
    HeaderBar* getHeaderBar() { return m_pHeaderBar; }
    XMLFilterListBox* getListBox() { return m_pFocusCtrl; }
    virtual ~SvxPathControl();

    virtual bool Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
};



class HeaderBar;

class XMLFilterListBox : public SvTabListBox
{
private:
    bool        mbFirstPaint;
    HeaderBar*  m_pHeaderBar;

    DECL_LINK( TabBoxScrollHdl_Impl, SvTabListBox* );
    DECL_LINK( HeaderEndDrag_Impl, HeaderBar* );

    OUString getEntryString( const filter_info_impl* pInfo ) const;

public:
    XMLFilterListBox(SvxPathControl* pParent);

    /** adds a new filter info entry to the ui filter list */
    void addFilterEntry( const filter_info_impl* pInfo );

    void changeEntry( const filter_info_impl* pInfo );

    virtual void    Paint( const Rectangle& rRect ) SAL_OVERRIDE;
};



class XMLFilterSettingsDialog : public ModelessDialog
{
public:
    XMLFilterSettingsDialog(Window* pParent,
        const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext);

    DECL_LINK(ClickHdl_Impl, PushButton * );
    DECL_LINK(SelectionChangedHdl_Impl, void * );
    DECL_LINK(DoubleClickHdl_Impl, void * );

    virtual short Execute() SAL_OVERRIDE;

    void    onNew();
    void    onEdit();
    void    onTest();
    void    onDelete();
    void    onSave();
    void    onOpen();
    void    onClose();

    void    updateStates();

    virtual bool Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;

    bool    isClosable();

private:
    void    initFilterList();
    void    disposeFilterList();

    bool    insertOrEdit( filter_info_impl* pNewInfo, const filter_info_impl* pOldInfo = NULL );

    OUString createUniqueFilterName( const OUString& rUIName );
    OUString createUniqueTypeName( const OUString& rTypeName );
    OUString createUniqueInterfaceName( const OUString& rInterfaceName );

private:

    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > mxContext;
    com::sun::star::uno::Reference< com::sun::star::container::XNameContainer > mxFilterContainer;
    com::sun::star::uno::Reference< com::sun::star::container::XNameContainer > mxTypeDetection;
    com::sun::star::uno::Reference< com::sun::star::container::XNameContainer > mxExtendedTypeDetection;

    std::vector< filter_info_impl* > maFilterVector;

    XMLFilterListBox*   m_pFilterListBox;
    SvxPathControl* m_pCtrlFilterList;
    PushButton* m_pPBNew;
    PushButton* m_pPBEdit;
    PushButton* m_pPBTest;
    PushButton* m_pPBDelete;
    PushButton* m_pPBSave;
    PushButton* m_pPBOpen;
    CloseButton* m_pPBClose;

    bool m_bIsClosable;

    OUString m_sTemplatePath;
    OUString m_sDocTypePrefix;

    SvtModuleOptions maModuleOpt;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
