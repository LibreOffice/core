/*************************************************************************
 *
 *  $RCSfile: CollectionView.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:39:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef DBAUI_COLLECTIONVIEW_HXX
#include "CollectionView.hxx"
#endif
#ifndef DBAUI_COLLECTIONVIEW_HRC
#include "CollectionView.hrc"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
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
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef INCLUDED_SVTOOLS_VIEWOPTIONS_HXX
#include <svtools/viewoptions.hxx>
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
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
using namespace comphelper;
// -----------------------------------------------------------------------------
DBG_NAME(OCollectionView)
OCollectionView::OCollectionView( Window * pParent
                                 ,const Reference< XContent>& _xContent
                                 ,const ::rtl::OUString& _sDefaultName
                                 ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xORB)
    : ModalDialog( pParent, ModuleRes(DLG_COLLECTION_VIEW))
    , m_aView(          this, ResId( CTRL_VIEW ) ,FILEVIEW_SHOW_TITLE )
    , m_aPB_OK(         this, ResId( BTN_EXPLORERFILE_SAVE ) )
    , m_aPB_CANCEL(     this, ResId( PB_CANCEL ) )
    , m_aPB_HELP(       this, ResId( PB_HELP ) )
    , m_aName(          this, ResId( ED_EXPLORERFILE_FILENAME ) )
    , m_aFTCurrentPath( this, ResId( FT_EXPLORERFILE_CURRENTPATH ) )
    , m_aFTName(        this, ResId( FT_EXPLORERFILE_FILENAME ) )
    , m_aNewFolder(     this, ResId( BTN_EXPLORERFILE_NEWFOLDER ) )
    , m_aFL(            this, ResId( FL_1 ) )
    , m_aUp(            this, ResId( BTN_EXPLORERFILE_UP ) )
    , m_sPath(          ResId( STR_PATHNAME ) )
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
                xNameContainer->removeByName(sName);
            }
            m_aName.SetText(sName);
            EndDialog( TRUE );
        }
    }
    catch(Exception)
    {
        OSL_ENSURE(0,"Exception caught!");
    }
    return 0;
}
// -----------------------------------------------------------------------------
IMPL_LINK( OCollectionView, NewFolder_Click, PushButton*, EMPTYARG )
{
    try
    {
        Reference<XHierarchicalNameContainer> xNameContainer(m_xContent,UNO_QUERY);
        if ( dbaui::insertHierachyElement(this,xNameContainer,String(),m_bCreateForm) )
            m_aView.Initialize(m_xContent,String());
    }
    catch(Exception)
    {
        OSL_ENSURE(0,"Exception caught!");
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
    catch(Exception)
    {
        OSL_ENSURE(0,"Exception caught!");
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
    catch(Exception)
    {
        OSL_ENSURE(0,"Exception caught!");
    }
    return 0;
}
// -----------------------------------------------------------------------------
void OCollectionView::initCurrentPath()
{
    BOOL bEnable = FALSE;
    try
    {
        if ( m_xContent.is() )
        {
            ::rtl::OUString sCID = m_xContent->getIdentifier()->getContentIdentifier();
            const static ::rtl::OUString sFormsCID(RTL_CONSTASCII_USTRINGPARAM("private:forms"));
            m_bCreateForm = sFormsCID.compareTo(sCID) == 0;
            m_aFTCurrentPath.SetText(sCID.copy(8));
            Reference<XChild> xChild(m_xContent,UNO_QUERY);
            bEnable = xChild.is() && Reference<XNameAccess>(xChild->getParent(),UNO_QUERY).is();
        }
    }
    catch(Exception)
    {
        OSL_ENSURE(0,"Exception caught!");
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


