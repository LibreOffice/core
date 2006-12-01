/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlfiltersettingsdialog.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-01 14:34:56 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _XMLFILTERSETTINGSDIALOG_HXX_
#define _XMLFILTERSETTINGSDIALOG_HXX_

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAME_HPP_
#include <com/sun/star/container/XHierarchicalName.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif

#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif

#ifndef _SVTABBX_HXX
#include <svtools/svtabbx.hxx>
#endif

#ifndef _SFXPOOLITEM_HXX
#include <svtools/poolitem.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <svtools/moduleoptions.hxx>
#endif

#include "xmlfiltercommon.hxx"

// --------------------------------------------------------------------

class SvxPathControl_Impl : public Control
{
private:
    Control*        m_pFocusCtrl;

public:
    SvxPathControl_Impl( Window* pParent, const ResId& rId ) :
        Control( pParent, rId ), m_pFocusCtrl( NULL ) {}

    void            SetFocusControl( Control* pCtrl ) { m_pFocusCtrl = pCtrl; }

    virtual long    Notify( NotifyEvent& rNEvt );
};

// --------------------------------------------------------------------

class HeaderBar;

class XMLFilterListBox : public SvTabListBox
{
private:
    bool        mbFirstPaint;
    HeaderBar*  mpHeaderBar;

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

    virtual void    Paint( const Rectangle& rRect );
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

    void    onNew();
    void    onEdit();
    void    onTest();
    void    onDelete();
    void    onSave();
    void    onOpen();
    void    onClose();

    void    updateStates();

    virtual long    Notify( NotifyEvent& rNEvt );

    bool    isClosable();

    static ResMgr* mpResMgr;

private:
    void    initFilterList();
    void    disposeFilterList();

    bool    insertOrEdit( filter_info_impl* pNewInfo, const filter_info_impl* pOldInfo = NULL );

    rtl::OUString createUniqueFilterName( const rtl::OUString& rUIName );
    rtl::OUString createUniqueTypeName( const rtl::OUString& rTypeName );
    rtl::OUString createUniqueInterfaceName( const rtl::OUString& rInterfaceName );

private:

    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > mxMSF;
    com::sun::star::uno::Reference< com::sun::star::container::XNameContainer > mxFilterContainer;
    com::sun::star::uno::Reference< com::sun::star::container::XNameContainer > mxTypeDetection;
    com::sun::star::uno::Reference< com::sun::star::container::XNameContainer > mxExtendedTypeDetection;

    std::vector< filter_info_impl* > maFilterVector;

    XMLFilterListBox*   mpFilterListBox;
    SvxPathControl_Impl maCtrlFilterList;
    PushButton  maPBNew;
    PushButton  maPBEdit;
    PushButton  maPBTest;
    PushButton  maPBDelete;
    PushButton  maPBSave;
    PushButton  maPBOpen;
    HelpButton  maPBHelp;
    PushButton  maPBClose;

    bool    mbIsClosable;

    ::rtl::OUString sTemplatePath;
    ::rtl::OUString sDocTypePrefix;

    SvtModuleOptions maModuleOpt;
};

#endif
