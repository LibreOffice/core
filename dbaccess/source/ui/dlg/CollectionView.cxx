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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#ifndef DBAUI_COLLECTIONVIEW_HXX
#include "CollectionView.hxx"
#endif
#ifndef DBAUI_COLLECTIONVIEW_HRC
#include "CollectionView.hrc"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _DBU_DLG_HRC_
#include "dbu_dlg.hrc"
#endif
#ifndef _COMPHELPER_INTERACTION_HXX_
#include <comphelper/interaction.hxx>
#endif
#ifndef _CPPUHELPER_EXC_HLP_HXX_
#include <cppuhelper/exc_hlp.hxx>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif
#ifndef SVT_QUERYFOLDER_HXX
#include <svtools/QueryFolderName.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMECONTAINER_HPP_
#include <com/sun/star/container/XHierarchicalNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVEAUGMENTEDIOEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_IOERRORCODE_HPP_
#include <com/sun/star/ucb/IOErrorCode.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_INTERACTIONCLASSIFICATION_HPP_
#include <com/sun/star/task/InteractionClassification.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_SQLEXCEPTION_HPP_
#include <com/sun/star/sdbc/SQLException.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef INCLUDED_SVTOOLS_VIEWOPTIONS_HXX
#include <unotools/viewoptions.hxx>
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif

#define FILEDIALOG_DEF_IMAGEBORDER  10
//.........................................................................
namespace dbaui
{
//.........................................................................

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::sdbc;
using namespace comphelper;
// -----------------------------------------------------------------------------
DBG_NAME(OCollectionView)
OCollectionView::OCollectionView( Window * pParent
                                 ,const Reference< XContent>& _xContent
                                 ,const ::rtl::OUString& _sDefaultName
                                 ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xORB)
    : ModalDialog( pParent, ModuleRes(DLG_COLLECTION_VIEW))
    , m_aFTCurrentPath( this, ModuleRes( FT_EXPLORERFILE_CURRENTPATH ) )
    , m_aNewFolder(     this, ModuleRes( BTN_EXPLORERFILE_NEWFOLDER ) )
    , m_aUp(            this, ModuleRes( BTN_EXPLORERFILE_UP ) )
    , m_aView(          this, ModuleRes( CTRL_VIEW ) ,FILEVIEW_SHOW_TITLE )
    , m_aFTName(        this, ModuleRes( FT_EXPLORERFILE_FILENAME ) )
    , m_aName(          this, ModuleRes( ED_EXPLORERFILE_FILENAME ) )
    , m_aFL(            this, ModuleRes( FL_1 ) )
    , m_aPB_OK(         this, ModuleRes( BTN_EXPLORERFILE_SAVE ) )
    , m_aPB_CANCEL(     this, ModuleRes( PB_CANCEL ) )
    , m_aPB_HELP(       this, ModuleRes( PB_HELP ) )
    , m_sPath(          ModuleRes( STR_PATHNAME ) )
    , m_xContent(_xContent)
    , m_xORB(_xORB)
    , m_bCreateForm(sal_True)
{
    DBG_CTOR(OCollectionView,NULL);
    FreeResource();

    OSL_ENSURE(m_xContent.is(),"No valid content!");
    m_aView.Initialize(m_xContent,String());
    m_aFTCurrentPath.SetStyle( m_aFTCurrentPath.GetStyle() | WB_PATHELLIPSIS );
    initCurrentPath();

    m_aName.SetText(_sDefaultName);
    m_aName.GrabFocus();

    m_aNewFolder.SetStyle( m_aNewFolder.GetStyle() | WB_NOPOINTERFOCUS );
    m_aUp.SetModeImage(ModuleRes(IMG_NAVIGATION_BTN_UP_SC));
    m_aUp.SetModeImage(ModuleRes(IMG_NAVIGATION_BTN_UP_SCH),BMP_COLOR_HIGHCONTRAST);
    m_aNewFolder.SetModeImage(ModuleRes(IMG_NAVIGATION_CREATEFOLDER_SC));
    m_aNewFolder.SetModeImage(ModuleRes(IMG_NAVIGATION_CREATEFOLDER_SCH),BMP_COLOR_HIGHCONTRAST);

    m_aView.SetDoubleClickHdl( LINK( this, OCollectionView, Dbl_Click_FileView ) );
    m_aView.EnableAutoResize();
    m_aUp.SetClickHdl( LINK( this, OCollectionView, Up_Click ) );
    m_aNewFolder.SetClickHdl( LINK( this, OCollectionView, NewFolder_Click ) );
    m_aPB_OK.SetClickHdl( LINK( this, OCollectionView, Save_Click ) );
}
// -----------------------------------------------------------------------------
OCollectionView::~OCollectionView( )
{
    DBG_DTOR(OCollectionView,NULL);
}
// -----------------------------------------------------------------------------
Reference< XContent> OCollectionView::getSelectedFolder() const
{
    return m_xContent;
}
// -----------------------------------------------------------------------------
IMPL_LINK( OCollectionView, Save_Click, PushButton*, EMPTYARG )
{
    ::rtl::OUString sName = m_aName.GetText();
    if ( !sName.getLength() )
        return 0;
    try
    {
        ::rtl::OUString sSubFolder = m_aView.GetCurrentURL();
        sal_Int32 nIndex = sName.lastIndexOf('/') + 1;
        if ( nIndex )
        {
            if ( nIndex == 1 ) // special handling for root
            {
                Reference<XChild> xChild(m_xContent,UNO_QUERY);
                Reference<XNameAccess> xNameAccess(xChild,UNO_QUERY);
                while( xNameAccess.is() )
                {
                    xNameAccess.set(xChild->getParent(),UNO_QUERY);
                    if ( xNameAccess.is() )
                    {
                        m_xContent.set(xNameAccess,UNO_QUERY);
                        xChild.set(m_xContent,UNO_QUERY);
                    }
                }
                m_aView.Initialize(m_xContent,String());
                initCurrentPath();
            }
            sSubFolder = sName.copy(0,nIndex-1);
            sName = sName.copy(nIndex);
            Reference<XHierarchicalNameContainer> xHier(m_xContent,UNO_QUERY);
            OSL_ENSURE(xHier.is(),"XHierarchicalNameContainer not supported!");
            if ( sSubFolder.getLength() && xHier.is() )
            {
                if ( xHier->hasByHierarchicalName(sSubFolder) )
                {
                    m_xContent.set(xHier->getByHierarchicalName(sSubFolder),UNO_QUERY);
                }
                else // sub folder doesn't exist
                {
                    Sequence< Any > aValues(2);
                    PropertyValue aValue;
                    aValue.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ResourceName"));
                    aValue.Value <<= sSubFolder;
                    aValues[0] <<= aValue;

                    aValue.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ResourceType"));
                    aValue.Value <<= ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("folder"));
                    aValues[1] <<= aValue;

                    InteractionClassification eClass = InteractionClassification_ERROR;
                    ::com::sun::star::ucb::IOErrorCode eError = IOErrorCode_NOT_EXISTING_PATH;
                    ::rtl::OUString sTemp;
                    InteractiveAugmentedIOException aException(sTemp,Reference<XInterface>(),eClass,eError,aValues);


                    Reference<XInitialization> xIni(m_xORB->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.task.InteractionHandler"))),UNO_QUERY);
                    if ( xIni.is() )
                    {
                        aValue.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Parent"));
                        aValue.Value <<= VCLUnoHelper::GetInterface( this );
                        Sequence< Any > aArgs(1);
                        aArgs[0] <<= makeAny(aValue);
                        xIni->initialize(aArgs);
                        OInteractionRequest* pRequest = new OInteractionRequest(makeAny(aException));
                        Reference< XInteractionRequest > xRequest(pRequest);

                        OInteractionApprove* pApprove = new OInteractionApprove;
                        pRequest->addContinuation(pApprove);

                        Reference< XInteractionHandler > xHandler(xIni,UNO_QUERY);
                        xHandler->handle(xRequest);
                    }
                    return 0;
                }
            }
        }
        Reference<XNameContainer> xNameContainer(m_xContent,UNO_QUERY);
        if ( xNameContainer.is() )
        {
            Reference< XContent> xContent;
            if ( xNameContainer->hasByName(sName) )
            {
                QueryBox aBox( this, WB_YES_NO, ModuleRes( STR_ALREADYEXISTOVERWRITE ) );
                if ( aBox.Execute() != RET_YES )
                    return 0;
                // xNameContainer->removeByName(sName);
            }
            m_aName.SetText(sName);
            EndDialog( sal_True );
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return 0;
}
// -----------------------------------------------------------------------------
IMPL_LINK( OCollectionView, NewFolder_Click, PushButton*, EMPTYARG )
{
    try
    {
        Reference<XHierarchicalNameContainer> xNameContainer(m_xContent,UNO_QUERY);
        if ( dbaui::insertHierachyElement(this,m_xORB,xNameContainer,String(),m_bCreateForm) )
            m_aView.Initialize(m_xContent,String());
    }
    catch( const SQLException& )
    {
        showError( ::dbtools::SQLExceptionInfo( ::cppu::getCaughtException() ), this, m_xORB );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return 0;
}
// -----------------------------------------------------------------------------
IMPL_LINK( OCollectionView, Up_Click, PushButton*, EMPTYARG )
{
    try
    {
        Reference<XChild> xChild(m_xContent,UNO_QUERY);
        if ( xChild.is() )
        {
            Reference<XNameAccess> xNameAccess(xChild->getParent(),UNO_QUERY);
            if ( xNameAccess.is() )
            {
                m_xContent.set(xNameAccess,UNO_QUERY);
                m_aView.Initialize(m_xContent,String());
                initCurrentPath();
            }
            else
                m_aUp.Disable();
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return 0;
}
// -----------------------------------------------------------------------------
IMPL_LINK( OCollectionView, Dbl_Click_FileView, SvtFileView*, EMPTYARG )
{
    try
    {
        Reference<XNameAccess> xNameAccess(m_xContent,UNO_QUERY);
        if ( xNameAccess.is() )
        {
            ::rtl::OUString sSubFolder = m_aView.GetCurrentURL();
            sal_Int32 nIndex = sSubFolder.lastIndexOf('/') + 1;
            sSubFolder = sSubFolder.getToken(0,'/',nIndex);
            if ( sSubFolder.getLength() )
            {
                Reference< XContent> xContent;
                if ( xNameAccess->hasByName(sSubFolder) )
                    xContent.set(xNameAccess->getByName(sSubFolder),UNO_QUERY);
                if ( xContent.is() )
                {
                    m_xContent = xContent;
                    m_aView.Initialize(m_xContent,String());
                    initCurrentPath();
                }
            }
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return 0;
}
// -----------------------------------------------------------------------------
void OCollectionView::initCurrentPath()
{
    sal_Bool bEnable = sal_False;
    try
    {
        if ( m_xContent.is() )
        {
            const ::rtl::OUString sCID = m_xContent->getIdentifier()->getContentIdentifier();
            const static ::rtl::OUString s_sFormsCID(RTL_CONSTASCII_USTRINGPARAM("private:forms"));
            const static ::rtl::OUString s_sReportsCID(RTL_CONSTASCII_USTRINGPARAM("private:reports"));
            m_bCreateForm = s_sFormsCID.compareTo(sCID) == 0;
            ::rtl::OUString sPath(RTL_CONSTASCII_USTRINGPARAM("/"));
            if ( m_bCreateForm && sCID.getLength() != s_sFormsCID.getLength())
                sPath = sCID.copy(s_sFormsCID.getLength());
            else if ( !m_bCreateForm && sCID.getLength() != s_sReportsCID.getLength() )
                sPath = sCID.copy(s_sReportsCID.getLength());

            m_aFTCurrentPath.SetText(sPath);
            Reference<XChild> xChild(m_xContent,UNO_QUERY);
            bEnable = xChild.is() && Reference<XNameAccess>(xChild->getParent(),UNO_QUERY).is();
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    m_aUp.Enable(bEnable);
}
// -----------------------------------------------------------------------------
::rtl::OUString OCollectionView::getName() const
{
    return m_aName.GetText();
}
// -----------------------------------------------------------------------------
//.........................................................................
}   // namespace dbaui
//.........................................................................


