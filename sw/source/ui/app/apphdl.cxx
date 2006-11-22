/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: apphdl.cxx,v $
 *
 *  $Revision: 1.60 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-22 10:24:57 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>

#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_UNDOOPT_HXX
#include <svtools/undoopt.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_ACCESSIBILITYOPTIONS_HXX
#include <svtools/accessibilityoptions.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXEVENT_HXX //autogen
#include <sfx2/event.hxx>
#endif
#ifndef _SFX_OBJITEM_HXX
#include <sfx2/objitem.hxx>
#endif
#ifndef _SVX_DATACCESSDESCRIPTOR_HXX_
#include <svx/dataaccessdescriptor.hxx>
#endif
#ifndef _SVX_SRCHITEM_HXX
#include <svx/srchitem.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SFXISETHINT_HXX //autogen
#include <svtools/isethint.hxx>
#endif
#ifndef _SVX_DLG_HYPERLINK_HXX //autogen
#include <svx/hyprlink.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFX_FCONTNR_HXX
#include <sfx2/fcontnr.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SVTOOLS_CTLOPTIONS_HXX
#include <svtools/ctloptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_USEROPTIONS_HXX
#include <svtools/useroptions.hxx>
#endif
#ifndef _VCL_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _VCL_WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SVX_INSCTRL_HXX //autogen
#include <svx/insctrl.hxx>
#endif
#ifndef _SVX_SELCTRL_HXX //autogen
#include <svx/selctrl.hxx>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_UPDATEDOCMODE_HPP_
#include <com/sun/star/document/UpdateDocMode.hpp>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#include <svx/xmlsecctrl.hxx>
#ifndef _NAVICFG_HXX
#include <navicfg.hxx>
#endif

#include <sfx2/objface.hxx>
#include <sfx2/app.hxx>

#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _SWPVIEW_HXX
#include <pview.hxx>
#endif
#ifndef _SRCVIEW_HXX
#include <srcview.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _CMDID_H
#include <cmdid.h>          // Funktion-Ids
#endif
#ifndef _INITUI_HXX
#include <initui.hxx>
#endif
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _WDOCSH_HXX
#include <wdocsh.hxx>
#endif
#ifndef _WVIEW_HXX
#include <wview.hxx>
#endif
#ifndef _USRPREF_HXX
#include <usrpref.hxx>
#endif
#ifndef _GLOSLST_HXX
#include <gloslst.hxx>      // SwGlossaryList
#endif
#ifndef _GLOSDOC_HXX
#include <glosdoc.hxx>      // SwGlossaryList
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _CFGITEMS_HXX
#include <cfgitems.hxx>
#endif
#ifndef _PRTOPT_HXX
#include <prtopt.hxx>
#endif
#ifndef _MODCFG_HXX
#include <modcfg.hxx>
#endif
#ifndef _GLOBALS_H
#include <globals.h>        // globale Konstanten z.B.
#endif
#ifndef _APP_HRC
#include <app.hrc>
#endif
#ifndef _FONTCFG_HXX //autogen
#include <fontcfg.hxx>
#endif
#ifndef _BARCFG_HXX
#include <barcfg.hxx>
#endif
#ifndef _UINUMS_HXX //autogen
#include <uinums.hxx>
#endif
#ifndef _DBCONFIG_HXX
#include <dbconfig.hxx>
#endif
#ifndef _MMCONFIGITEM_HXX
#include <mmconfigitem.hxx>
#endif
#ifndef _SWMAILMERGECHILDWINDOW_HXX
#include <mailmergechildwindow.hxx>
#endif
#ifndef _LINGUISTIC_LNGPROPS_HHX_
#include <linguistic/lngprops.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XMULTIPROPERTYSET_HPP_
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XFASTPROPERTYSET_HPP_
#include <com/sun/star/beans/XFastPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATECHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyStateChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYACCESS_HPP_
#include <com/sun/star/beans/XPropertyAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCONTAINER_HPP_
#include <com/sun/star/beans/XPropertyContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif
#ifndef _SW_ABSTDLG_HXX
#include <swabstdlg.hxx>
#endif


#include <vcl/status.hxx>

#include "salhelper/simplereferenceobject.hxx"
#include "rtl/ref.hxx"

using namespace ::com::sun::star;

#define C2S(cChar) String::CreateFromAscii(cChar)
/*--------------------------------------------------------------------
    Beschreibung: Slotmaps fuer Methoden der Applikation
 --------------------------------------------------------------------*/


// hier werden die SlotID's included
// siehe Idl-File
//
#define SwModule
#define ViewSettings
#define WebViewSettings
#define PrintSettings
#define _ExecAddress ExecOther
#define _StateAddress StateOther
#include "itemdef.hxx"
#include <svx/svxslots.hxx>
#include "swslots.hxx"
#ifndef _CFGID_H
#include <cfgid.h>
#endif

#include <shells.hrc>

SFX_IMPL_INTERFACE( SwModule, SfxModule, SW_RES(RID_SW_NAME) )
{
    SFX_CHILDWINDOW_REGISTRATION(SvxHyperlinkDlgWrapper::GetChildWindowId());
    SFX_STATUSBAR_REGISTRATION(SW_RES(CFG_STATUSBAR));
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_APPLICATION |
            SFX_VISIBILITY_DESKTOP | SFX_VISIBILITY_STANDARD | SFX_VISIBILITY_CLIENT | SFX_VISIBILITY_VIEWER,
            SW_RES(RID_MODULE_TOOLBOX) );
}


/*--------------------------------------------------------------------
    Beschreibung: Andere States
 --------------------------------------------------------------------*/


void SwModule::StateOther(SfxItemSet &rSet)
{
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();

    SwView* pActView = ::GetActiveView();
    sal_Bool bWebView = 0 != PTR_CAST(SwWebView, pActView);

    while(nWhich)
    {
        switch(nWhich)
        {
            case FN_BUSINESS_CARD:
            case FN_LABEL:
            case FN_ENVELOP:
            {
                sal_Bool bDisable = sal_False;
                SfxViewShell* pView = SfxViewShell::Current();
                if( !pView || (pView && !pView->ISA(SwView)) )
                    bDisable = sal_True;
                SwDocShell *pDocSh = (SwDocShell*) SfxObjectShell::Current();
                if ( bDisable ||
                    (pDocSh &&    (pDocSh->IsReadOnly() ||
                                  pDocSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED)) )
                    rSet.DisableItem( nWhich );

            }
            break;
            case FN_XFORMS_INIT:
                // slot is always active!
                break;
            case FN_EDIT_FORMULA:
                {
                    SwWrtShell* pSh = 0;
                    int nSelection = 0;
                    if( pActView )
                        pSh = &pActView->GetWrtShell();
                    if( pSh )
                        nSelection = pSh->GetSelectionType();

                    if( (pSh && pSh->HasSelection()) ||
                        !(nSelection & (SwWrtShell::SEL_TXT | SwWrtShell::SEL_TBL)))
                        rSet.DisableItem(nWhich);
                }
            break;
            case SID_ATTR_METRIC:
                rSet.Put( SfxUInt16Item( SID_ATTR_METRIC, ::GetDfltMetric(bWebView)));
            break;
            case FN_SET_MODOPT_TBLNUMFMT:
                rSet.Put( SfxBoolItem( nWhich, pModuleConfig->
                                            IsInsTblFormatNum( bWebView )));
            break;
            default:
                DBG_ERROR("::StateOther: default");
        }
        nWhich = aIter.NextWhich();
    }
}

/*-- 06.04.2004 15:21:43---------------------------------------------------

  -----------------------------------------------------------------------*/
SwView* lcl_LoadDoc(SwView* pView, const String& rURL)
{
    sal_Bool bRet = sal_False;
    SwView* pNewView = 0;
    if(rURL.Len())
    {
        SfxStringItem aURL(SID_FILE_NAME, rURL);
        SfxStringItem aTargetFrameName( SID_TARGETNAME, String::CreateFromAscii("_blank") );
        SfxBoolItem aHidden( SID_HIDDEN, TRUE );
        SfxStringItem aReferer(SID_REFERER, pView->GetDocShell()->GetTitle());
        SfxObjectItem* pItem = (SfxObjectItem*)pView->GetViewFrame()->GetDispatcher()->
                Execute(SID_OPENDOC, SFX_CALLMODE_SYNCHRON,
                            &aURL, &aHidden, &aReferer, &aTargetFrameName, 0);
        SfxShell* pShell = pItem ? pItem->GetShell() : 0;

        if(pShell)
        {
            SfxViewShell* pViewShell = pShell->GetViewShell();
            if(pViewShell)
            {
                if( pViewShell->ISA(SwView) )
                {
                    pNewView = PTR_CAST(SwView,pViewShell);
                    pNewView->GetViewFrame()->GetFrame()->Appear();
                }
                else
                {
                    pViewShell->GetViewFrame()->DoClose();
                }
            }
        }
    }
    else
    {
        SfxStringItem aFactory(SID_NEWDOCDIRECT, SwDocShell::Factory().GetFilterContainer()->GetName());
        const SfxFrameItem* pItem = (SfxFrameItem*)
                            pView->GetViewFrame()->GetDispatcher()->Execute(SID_NEWDOCDIRECT,
                                SFX_CALLMODE_SYNCHRON, &aFactory, 0L);
        SfxFrame* pFrm = pItem ? pItem->GetFrame() : 0;
        SfxViewFrame* pFrame = pFrm ? pFrm->GetCurrentViewFrame() : 0;
        pNewView = pFrame ? PTR_CAST(SwView, pFrame->GetViewShell()) : 0;
    }

    return pNewView;
}
/*--------------------------------------------------------------------
    Beschreibung:   Felddialog starten
 --------------------------------------------------------------------*/

void NewXForms( SfxRequest& rReq ); // implementation: below

namespace
{

class SwMailMergeWizardExecutor : public salhelper::SimpleReferenceObject
{
    SwView*                  m_pView;     // never owner
    SwMailMergeConfigItem*   m_pMMConfig; // sometimes owner
    AbstractMailMergeWizard* m_pWizard;   // always owner

    DECL_LINK( EndDialogHdl, AbstractMailMergeWizard* );
    DECL_LINK( DestroyDialogHdl, AbstractMailMergeWizard* );
    DECL_LINK( DestroyWizardHdl, AbstractMailMergeWizard* );

    void ExecutionFinished( bool bDeleteConfigItem );
    void ExecuteWizard();

public:
    SwMailMergeWizardExecutor();
    ~SwMailMergeWizardExecutor();

    void ExecuteMailMergeWizard( const SfxItemSet * pArgs );
};

SwMailMergeWizardExecutor::SwMailMergeWizardExecutor()
    : m_pView( 0 ),
      m_pMMConfig( 0 ),
      m_pWizard( 0 )
{
}

SwMailMergeWizardExecutor::~SwMailMergeWizardExecutor()
{
    DBG_ASSERT( m_pWizard == 0, "SwMailMergeWizardExecutor: m_pWizard must be Null!" );
    DBG_ASSERT( m_pMMConfig == 0, "SwMailMergeWizardExecutor: m_pMMConfig must be Null!" );
}

void SwMailMergeWizardExecutor::ExecuteMailMergeWizard( const SfxItemSet * pArgs )
{
    if ( m_pView )
    {
        DBG_ERROR( "SwMailMergeWizardExecutor::ExecuteMailMergeWizard: Already executing the wizard!" );
        return;
    }

    m_pView = ::GetActiveView(); // not owner!
    DBG_ASSERT(m_pView, "no current view?")
    if(m_pView)
    {
        // keep self alive until done.
        acquire();

        // if called from the child window - get the config item and close the ChildWindow, then restore
        // the wizard
        SwMailMergeChildWindow* pChildWin =
            static_cast<SwMailMergeChildWindow*>(m_pView->GetViewFrame()->GetChildWindow(FN_MAILMERGE_CHILDWINDOW));
        bool bRestoreWizard = false;
        sal_uInt16 nRestartPage = 0;
        if(pChildWin && pChildWin->IsVisible())
        {
            m_pMMConfig = m_pView->GetMailMergeConfigItem();
            nRestartPage = m_pView->GetMailMergeRestartPage();
            if(m_pView->IsMailMergeSourceView())
                m_pMMConfig->SetSourceView( m_pView );
            m_pView->SetMailMergeConfigItem(0, 0, sal_True);
            SfxViewFrame* pViewFrame = m_pView->GetViewFrame();
            pViewFrame->ShowChildWindow(FN_MAILMERGE_CHILDWINDOW, FALSE);
            DBG_ASSERT(m_pMMConfig, "no MailMergeConfigItem available");
            bRestoreWizard = true;
        }
        // to make it bullet proof ;-)
        if(!m_pMMConfig)
        {
            m_pMMConfig = new SwMailMergeConfigItem;
            m_pMMConfig->SetSourceView(m_pView);

            //set the first used database as default source on the config item
            const SfxPoolItem* pItem = 0;
            if(pArgs && SFX_ITEM_SET == pArgs->GetItemState(
                   FN_PARAM_DATABASE_PROPERTIES, sal_False, &pItem))
            {
                //mailmerge has been called from the database beamer
                uno::Sequence< beans::PropertyValue> aDBValues;
                if(static_cast<const SfxUsrAnyItem*>(pItem)->GetValue() >>= aDBValues)
                {
                    SwDBData aDBData;
                    svx::ODataAccessDescriptor aDescriptor(aDBValues);
                    aDescriptor[svx::daDataSource]   >>= aDBData.sDataSource;
                    aDescriptor[svx::daCommand]      >>= aDBData.sCommand;
                    aDescriptor[svx::daCommandType]  >>= aDBData.nCommandType;

                    uno::Sequence< uno::Any >                   aSelection;
                    uno::Reference< sdbc::XConnection>          xConnection;
                    uno::Reference< sdbc::XDataSource>          xSource;
                    uno::Reference< sdbcx::XColumnsSupplier>    xColumnsSupplier;
                    if ( aDescriptor.has(svx::daSelection) )
                        aDescriptor[svx::daSelection] >>= aSelection;
                    if ( aDescriptor.has(svx::daConnection) )
                        aDescriptor[svx::daConnection] >>= xConnection;
                    uno::Reference<container::XChild> xChild(xConnection, uno::UNO_QUERY);
                    if(xChild.is())
                        xSource = uno::Reference<sdbc::XDataSource>(
                            xChild->getParent(), uno::UNO_QUERY);
                    m_pMMConfig->SetCurrentConnection(
                        xSource, SharedConnection( xConnection, SharedConnection::NoTakeOwnership ),
                        xColumnsSupplier, aDBData);
                }
            }
            else
            {
                SvStringsDtor aDBNameList(5, 1);
                SvStringsDtor aAllDBNames(5, 5);
                m_pView->GetWrtShell().GetAllUsedDB( aDBNameList, &aAllDBNames );
                if(aDBNameList.Count())
                {
                    String sDBName = *aDBNameList[0];
                    SwDBData aDBData;
                    aDBData.sDataSource = sDBName.GetToken(0, DB_DELIM);
                    aDBData.sCommand = sDBName.GetToken(1, DB_DELIM);
                    aDBData.nCommandType = sDBName.GetToken(2).ToInt32();
                    //set the currently used database for the wizard
                    m_pMMConfig->SetCurrentDBData( aDBData );
                }
            }
        }

        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
        m_pWizard = pFact->CreateMailMergeWizard(*m_pView, *m_pMMConfig);

        if(bRestoreWizard)
        {
            m_pWizard->ShowPage( nRestartPage );
        }

        ExecuteWizard();
    }
}

void SwMailMergeWizardExecutor::ExecutionFinished( bool bDeleteConfigItem )
{
    m_pMMConfig->Commit();
    if ( bDeleteConfigItem ) // owner?
        delete m_pMMConfig;

    m_pMMConfig = 0;

    // release/destroy asynchronously
    Application::PostUserEvent( LINK( this, SwMailMergeWizardExecutor, DestroyDialogHdl ) );
}

void SwMailMergeWizardExecutor::ExecuteWizard()
{
    m_pWizard->StartExecuteModal(
        LINK( this, SwMailMergeWizardExecutor, EndDialogHdl ) );
}

IMPL_LINK( SwMailMergeWizardExecutor, EndDialogHdl, AbstractMailMergeWizard*, pDialog )
{
    DBG_ASSERT( pDialog == m_pWizard, "wrong dialog passed to EndDialogHdl!" );

    long nRet = m_pWizard->GetResult();
    sal_uInt16 nRestartPage = m_pWizard->GetRestartPage();

    switch ( nRet )
    {
    case RET_LOAD_DOC:
        {
            SwView* pNewView = lcl_LoadDoc(m_pView, m_pWizard->GetReloadDocument());

            // destroy wizard asynchronously
            Application::PostUserEvent(
                LINK( this, SwMailMergeWizardExecutor, DestroyWizardHdl ), m_pWizard );

            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            if(pNewView)
            {
                m_pView = pNewView;
                m_pMMConfig->DocumentReloaded();
                //new source view!
                m_pMMConfig->SetSourceView( m_pView );
                m_pWizard = pFact->CreateMailMergeWizard(*m_pView, *m_pMMConfig);
                m_pWizard->ShowPage( nRestartPage );
            }
            else
            {
                m_pWizard = pFact->CreateMailMergeWizard(*m_pView, *m_pMMConfig);
            }

            // execute the wizard again
            ExecuteWizard();
            break;
        }
    case RET_TARGET_CREATED:
        {
            SwView* pTargetView = m_pMMConfig->GetTargetView();
            uno::Reference< frame::XFrame > xFrame =
                m_pView->GetViewFrame()->GetFrame()->GetFrameInterface();
            xFrame->getContainerWindow()->setVisible(sal_False);
            DBG_ASSERT(pTargetView, "No target view has been created");
            if(pTargetView)
            {
                // destroy wizard asynchronously
                Application::PostUserEvent(
                    LINK( this, SwMailMergeWizardExecutor, DestroyWizardHdl ), m_pWizard );

                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                m_pWizard = pFact->CreateMailMergeWizard(*pTargetView, *m_pMMConfig);
                m_pWizard->ShowPage( nRestartPage );

                // execute the wizard again
                ExecuteWizard();
            }
            else
            {
                //should not happen - just in case no target view has been created
                ExecutionFinished( true );
            }
            break;
        }
    case RET_EDIT_DOC:
    case RET_EDIT_RESULT_DOC:
        {
            //create a non-modal dialog that allows to return to the wizard
            //the ConfigItem ownership moves to this dialog
            bool bResult = nRet == RET_EDIT_RESULT_DOC && m_pMMConfig->GetTargetView();
            SwView* pTempView = bResult ? m_pMMConfig->GetTargetView() : m_pMMConfig->GetSourceView();
            pTempView->SetMailMergeConfigItem(m_pMMConfig, m_pWizard->GetRestartPage(), !bResult);
            SfxViewFrame* pViewFrame = pTempView->GetViewFrame();
            pViewFrame->GetDispatcher()->Execute(
                FN_MAILMERGE_CHILDWINDOW, SFX_CALLMODE_SYNCHRON);
            ExecutionFinished( false );
            break;
        }
    case RET_REMOVE_TARGET:
        {
            SwView* pTargetView = m_pMMConfig->GetTargetView();
            SwView* pSourceView = m_pMMConfig->GetSourceView();
            DBG_ASSERT(pTargetView && pSourceView, "source or target view not available" );
            if(pTargetView && pSourceView)
            {
                pTargetView->GetViewFrame()->DoClose();
                pSourceView->GetViewFrame()->GetFrame()->Appear();
                // the current view has be be set when the target is destroyed
                m_pView = pSourceView;
                m_pMMConfig->SetTargetView(0);

                // destroy wizard asynchronously
                Application::PostUserEvent(
                    LINK( this, SwMailMergeWizardExecutor, DestroyWizardHdl ), m_pWizard );

                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                m_pWizard = pFact->CreateMailMergeWizard(*pTargetView, *m_pMMConfig);
                m_pWizard->ShowPage( nRestartPage );

                // execute the wizard again
                ExecuteWizard();
            }
            else
            {
                //should not happen - just in case no target view has been created
                ExecutionFinished( true );
            }
            break;
        }
    case RET_CANCEL:
        {
            //the wizard has been canceled
            if(m_pMMConfig->GetTargetView())
            {
                m_pMMConfig->GetTargetView()->GetViewFrame()->DoClose();
                m_pMMConfig->SetTargetView(0);
            }
            if(m_pMMConfig->GetSourceView())
                m_pMMConfig->GetSourceView()->GetViewFrame()->GetFrame()->Appear();

            ExecutionFinished( true );
            break;
        }
    default: //finish
        {
            SwView* pSourceView = m_pMMConfig->GetSourceView();
            if(pSourceView)
            {
                SwDocShell* pDocShell = pSourceView->GetDocShell();
                if(pDocShell->HasName() && !pDocShell->IsModified())
                    m_pMMConfig->GetSourceView()->GetViewFrame()->DoClose();
                else
                    m_pMMConfig->GetSourceView()->GetViewFrame()->GetFrame()->Appear();
            }
            ExecutionFinished( true );
            break;
        }

    } // switch

    return 0L;
}

IMPL_LINK( SwMailMergeWizardExecutor, DestroyDialogHdl, AbstractMailMergeWizard*, pDialog )
{
    delete m_pWizard;
    m_pWizard = 0;

    release();
    return 0L;
}

IMPL_LINK( SwMailMergeWizardExecutor, DestroyWizardHdl, AbstractMailMergeWizard*, pDialog )
{
    delete pDialog;
    return 0L;
}

} // namespace

void SwModule::ExecOther(SfxRequest& rReq)
{
    const SfxItemSet *pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem = 0;

    sal_uInt16 nWhich = rReq.GetSlot();
    switch (nWhich)
    {
        case FN_ENVELOP:
            InsertEnv( rReq );
            break;

        case FN_BUSINESS_CARD:
        case FN_LABEL:
            InsertLab(rReq, nWhich == FN_LABEL);
            break;

        case FN_XFORMS_INIT:
            NewXForms( rReq );
            break;

        case SID_ATTR_METRIC:
        if(pArgs && SFX_ITEM_SET == pArgs->GetItemState(nWhich, sal_False, &pItem))
        {
            FieldUnit eUnit = (FieldUnit)((const SfxUInt16Item*)pItem)->GetValue();
            switch( eUnit )
            {
                case FUNIT_MM:
                case FUNIT_CM:
                case FUNIT_INCH:
                case FUNIT_PICA:
                case FUNIT_POINT:
                {
                    SwView* pActView = ::GetActiveView();
                    sal_Bool bWebView = 0 != PTR_CAST(SwWebView, pActView);
                    ::SetDfltMetric(eUnit, bWebView);
                }
                break;
            }
        }
        break;

        case FN_SET_MODOPT_TBLNUMFMT:
            {
                sal_Bool bWebView = 0 != PTR_CAST(SwWebView, ::GetActiveView() ),
                     bSet;

                if( pArgs && SFX_ITEM_SET == pArgs->GetItemState(
                        nWhich, sal_False, &pItem ))
                    bSet = ((SfxBoolItem*)pItem)->GetValue();
                else
                    bSet = !pModuleConfig->IsInsTblFormatNum( bWebView );

                pModuleConfig->SetInsTblFormatNum( bWebView, bSet );
            }
            break;
        case FN_MAILMERGE_WIZARD:
        {
            rtl::Reference< SwMailMergeWizardExecutor > xEx( new SwMailMergeWizardExecutor );
            xEx->ExecuteMailMergeWizard( pArgs );
        }
        break;
    }
}

/*--------------------------------------------------------------------
    Beschreibung: Notifies abfangen
 --------------------------------------------------------------------*/


    // Hint abfangen fuer DocInfo
void SwModule::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if( rHint.ISA( SfxEventHint ) )
    {
        SfxEventHint& rEvHint = (SfxEventHint&) rHint;
        SwDocShell* pDocSh = PTR_CAST( SwDocShell, rEvHint.GetObjShell() );
        if( pDocSh )
        {
            SwWrtShell* pWrtSh = pDocSh ? pDocSh->GetWrtShell() : 0;
            switch( rEvHint.GetEventId() )
            {
/*          MA 07. Mar. 96: UpdateInputFlds() nur noch bei Dokument neu.
                                (Und bei Einfuegen Textbaust.)
                case SFX_EVENT_OPENDOC:
                // dann am aktuellen Dokument die Input-Fedler updaten
                if( pWrtSh )
                    pWrtSh->UpdateInputFlds();
                break;
*/
            case SFX_EVENT_CREATEDOC:
                // alle FIX-Date/Time Felder auf akt. setzen
                if( pWrtSh )
                {
                    SFX_ITEMSET_ARG( pDocSh->GetMedium()->GetItemSet(), pUpdateDocItem, SfxUInt16Item, SID_UPDATEDOCMODE, sal_False);
                    sal_Bool bUpdateFields = sal_True;
                    if( pUpdateDocItem &&  pUpdateDocItem->GetValue() == com::sun::star::document::UpdateDocMode::NO_UPDATE)
                        bUpdateFields = sal_False;
                    pWrtSh->SetFixFields();
                    if(bUpdateFields)
                    {
                        pWrtSh->UpdateInputFlds();

                        // Sind Datenbankfelder enthalten?
                        // Erstmal alle verwendeten Datenbanken holen
                        SwDoc *pDoc = pDocSh->GetDoc();
                        SvStringsDtor aDBNameList;
                        pDoc->GetAllUsedDB( aDBNameList );
                        sal_uInt16 nCount = aDBNameList.Count();
                        if (nCount)
                        {   // Datenbankbeamer oeffnen
                            ShowDBObj(pWrtSh->GetView(), pDoc->GetDBData());
                        }
                    }
                }
                break;
            }
        }
    }
    else if(rHint.ISA(SfxItemSetHint))
    {
        if( SFX_ITEM_SET == ((SfxItemSetHint&)rHint).GetItemSet().GetItemState(SID_ATTR_PATHNAME))
        {
            ::GetGlossaries()->UpdateGlosPath( sal_False );
            SwGlossaryList* pList = ::GetGlossaryList();
            if(pList->IsActive())
                pList->Update();
        }
    }
    else if(rHint.ISA(SfxSimpleHint))
    {
        ULONG nHintId = ((SfxSimpleHint&)rHint).GetId();
        if(SFX_HINT_COLORS_CHANGED == nHintId ||
           SFX_HINT_ACCESSIBILITY_CHANGED == nHintId )
        {
            sal_Bool bAccessibility = sal_False;
            if(SFX_HINT_COLORS_CHANGED == nHintId)
                SwViewOption::ApplyColorConfigValues(*pColorConfig);
            else
                bAccessibility = sal_True;

            //invalidate all edit windows
            const TypeId aSwViewTypeId = TYPE(SwView);
            const TypeId aSwPreViewTypeId = TYPE(SwPagePreView);
            const TypeId aSwSrcViewTypeId = TYPE(SwSrcView);
            SfxViewShell* pViewShell = SfxViewShell::GetFirst();
            while(pViewShell)
            {
                if(pViewShell->GetWindow())
                {
                    if((pViewShell->IsA(aSwViewTypeId) ||
                        pViewShell->IsA(aSwPreViewTypeId) ||
                        pViewShell->IsA(aSwSrcViewTypeId)))
                    {
                        if(bAccessibility)
                        {
                            if(pViewShell->IsA(aSwViewTypeId))
                                ((SwView*)pViewShell)->ApplyAccessiblityOptions(*pAccessibilityOptions);
                            else if(pViewShell->IsA(aSwPreViewTypeId))
                                ((SwPagePreView*)pViewShell)->ApplyAccessiblityOptions(*pAccessibilityOptions);
                        }
                        pViewShell->GetWindow()->Invalidate();
                    }
                }
                pViewShell = SfxViewShell::GetNext( *pViewShell );
            }
        }
        else if( SFX_HINT_CTL_SETTINGS_CHANGED == nHintId )
        {
            const SfxObjectShell* pObjSh = SfxObjectShell::GetFirst();
            while( pObjSh )
            {
                if( pObjSh->IsA(TYPE(SwDocShell)) )
                {
                    const SwDoc* pDoc = ((SwDocShell*)pObjSh)->GetDoc();
                    ViewShell* pVSh = 0;
                    pDoc->GetEditShell( &pVSh );
                    if ( pVSh )
                        pVSh->ChgNumberDigits();
                }
                pObjSh = SfxObjectShell::GetNext(*pObjSh);
            }
        }
        else if(SFX_HINT_USER_OPTIONS_CHANGED == nHintId)
        {
            bAuthorInitialised = FALSE;
        }
        else if(SFX_HINT_UNDO_OPTIONS_CHANGED == nHintId)
        {
            const int nNew = GetUndoOptions().GetUndoCount();
            const int nOld = SwEditShell::GetUndoActionCount();
            if(!nNew || !nOld)
            {
                sal_Bool bUndo = nNew != 0;
                //ueber DocShells iterieren und Undo umschalten

                TypeId aType(TYPE(SwDocShell));
                SwDocShell* pDocShell = (SwDocShell*)SfxObjectShell::GetFirst(&aType);
                while( pDocShell )
                {
                    pDocShell->GetDoc()->DoUndo( bUndo );
                    pDocShell = (SwDocShell*)SfxObjectShell::GetNext(*pDocShell, &aType);
                }
            }
            SwEditShell::SetUndoActionCount(nNew);
        }
        else if(SFX_HINT_DEINITIALIZING == nHintId)
        {
            DELETEZ(pWebUsrPref);
            DELETEZ(pUsrPref)   ;
            DELETEZ(pModuleConfig);
            DELETEZ(pPrtOpt)      ;
            DELETEZ(pWebPrtOpt)   ;
            DELETEZ(pChapterNumRules);
            DELETEZ(pStdFontConfig)     ;
            DELETEZ(pNavigationConfig)  ;
            DELETEZ(pToolbarConfig)     ;
            DELETEZ(pWebToolbarConfig)  ;
            DELETEZ(pAuthorNames)       ;
            DELETEZ(pDBConfig);
            EndListening(*pColorConfig);
            DELETEZ(pColorConfig);
            EndListening(*pAccessibilityOptions);
            DELETEZ(pAccessibilityOptions);
            EndListening(*pCTLOptions);
            DELETEZ(pCTLOptions);
            EndListening(*pUserOptions);
            DELETEZ(pUserOptions);
            EndListening(*pUndoOptions);
            DELETEZ(pUndoOptions);
        }
    }
}
void SwModule::FillStatusBar( StatusBar& rStatusBar )
{
    // Hier den StatusBar initialisieren
    // und Elemente reinschieben

    // Anzeige Seite
    String aTmp; aTmp.Fill( 10, 'X' );
    rStatusBar.InsertItem( FN_STAT_PAGE, rStatusBar.GetTextWidth(
                                    aTmp ), SIB_AUTOSIZE | SIB_LEFT);
    rStatusBar.SetHelpId(FN_STAT_PAGE, FN_STAT_PAGE);

    // Seitenvorlage
    aTmp.Fill( 15, 'X' );
    rStatusBar.InsertItem( FN_STAT_TEMPLATE, rStatusBar.GetTextWidth(
                                    aTmp ), SIB_AUTOSIZE | SIB_LEFT );
    rStatusBar.SetHelpId(FN_STAT_TEMPLATE, FN_STAT_TEMPLATE);

    // Zoomeinstellungen
    rStatusBar.InsertItem( SID_ATTR_ZOOM, rStatusBar.GetTextWidth(
                                                            C2S("1000%")) );
    rStatusBar.SetHelpId(SID_ATTR_ZOOM, SID_ATTR_ZOOM);

    // Insert/Overwrite
    rStatusBar.InsertItem( SID_ATTR_INSERT,
        SvxInsertStatusBarControl::GetDefItemWidth(rStatusBar));
    rStatusBar.SetHelpId(SID_ATTR_INSERT, SID_ATTR_INSERT);

    // awt::Selection-Modus
    rStatusBar.InsertItem( FN_STAT_SELMODE,
            SvxSelectionModeControl::GetDefItemWidth(rStatusBar));

    rStatusBar.SetHelpId(FN_STAT_SELMODE, FN_STAT_SELMODE);

    // Hyperlink ausfuehren/bearbeiten
    rStatusBar.InsertItem( FN_STAT_HYPERLINKS, rStatusBar.GetTextWidth(
                                                            C2S("HYP")) );
    rStatusBar.SetHelpId(FN_STAT_HYPERLINKS, FN_STAT_HYPERLINKS);

    // Dokument geaendert
    rStatusBar.InsertItem( SID_DOC_MODIFIED, rStatusBar.GetTextWidth(
                                                                C2S("*")));
    rStatusBar.SetHelpId(SID_DOC_MODIFIED, SID_DOC_MODIFIED);

    // signatures
    rStatusBar.InsertItem( SID_SIGNATURE, XmlSecStatusBarControl::GetDefItemWidth( rStatusBar ), SIB_USERDRAW );
    rStatusBar.SetHelpId(SID_SIGNATURE, SID_SIGNATURE);

    // den aktuellen Context anzeigen Uhrzeit / FrmPos / TabellenInfo
    aTmp.Fill( 25, sal_Unicode('X') );
    rStatusBar.InsertItem( SID_ATTR_SIZE, rStatusBar.GetTextWidth(
                    aTmp ), SIB_AUTOSIZE | SIB_LEFT | SIB_USERDRAW);
    rStatusBar.SetHelpId(SID_ATTR_SIZE, SID_ATTR_SIZE);
}

/* -----------------------------20.02.01 12:43--------------------------------

 ---------------------------------------------------------------------------*/
SwDBConfig* SwModule::GetDBConfig()
{
    if(!pDBConfig)
        pDBConfig = new SwDBConfig;
    return pDBConfig;
}
/* -----------------------------11.04.2002 15:27------------------------------

 ---------------------------------------------------------------------------*/
svtools::ColorConfig& SwModule::GetColorConfig()
{
    if(!pColorConfig)
    {
        pColorConfig = new svtools::ColorConfig;
        SwViewOption::ApplyColorConfigValues(*pColorConfig);
        StartListening(*pColorConfig);
    }
    return *pColorConfig;
}
/* -----------------------------06.05.2002 09:42------------------------------

 ---------------------------------------------------------------------------*/
SvtAccessibilityOptions& SwModule::GetAccessibilityOptions()
{
    if(!pAccessibilityOptions)
    {
        pAccessibilityOptions = new SvtAccessibilityOptions;
        StartListening(*pAccessibilityOptions);
    }
    return *pAccessibilityOptions;
}
/* -----------------06.05.2003 14:52-----------------

 --------------------------------------------------*/
SvtCTLOptions& SwModule::GetCTLOptions()
{
    if(!pCTLOptions)
    {
        pCTLOptions = new SvtCTLOptions;
        StartListening(*pCTLOptions);
    }
    return *pCTLOptions;
}
/* -----------------07.07.2003 09:31-----------------

 --------------------------------------------------*/
SvtUserOptions& SwModule::GetUserOptions()
{
    if(!pUserOptions)
    {
        pUserOptions = new SvtUserOptions;
        StartListening(*pUserOptions);
    }
    return *pUserOptions;
}
/* -----------------18.07.2003 13:31-----------------

 --------------------------------------------------*/
SvtUndoOptions& SwModule::GetUndoOptions()
{
    if(!pUndoOptions)
    {
        pUndoOptions = new SvtUndoOptions;
        StartListening(*pUndoOptions);
    }
    return *pUndoOptions;
}
/*-----------------30.01.97 08.30-------------------

--------------------------------------------------*/
const SwMasterUsrPref *SwModule::GetUsrPref(sal_Bool bWeb) const
{
    SwModule* pNonConstModule = (SwModule*)this;
    if(bWeb && !pWebUsrPref)
    {
        // im Load der SwMasterUsrPref wird der SpellChecker gebraucht, dort darf
        // er aber nicht angelegt werden #58256#
        pNonConstModule->pWebUsrPref = new SwMasterUsrPref(TRUE);
    }
    else if(!bWeb && !pUsrPref)
    {
        pNonConstModule->pUsrPref = new SwMasterUsrPref(FALSE);
    }
    return  bWeb ? pWebUsrPref : pUsrPref;
}



void NewXForms( SfxRequest& rReq )
{
    // copied & excerpted from SwModule::InsertLab(..)

    // create new document
    SfxObjectShellRef xDocSh( new SwDocShell( SFX_CREATE_MODE_STANDARD) );
    xDocSh->DoInitNew( 0 );

    // initialize XForms
    static_cast<SwDocShell*>( &xDocSh )->GetDoc()->initXForms( true );

    // put document into frame
    const SfxItemSet* pArgs = rReq.GetArgs();
    DBG_ASSERT( pArgs, "no arguments in SfxRequest");
    if( pArgs != NULL )
    {
        const SfxPoolItem* pFrameItem = NULL;
        pArgs->GetItemState( SID_DOCFRAME, FALSE, &pFrameItem );
        if( pFrameItem != NULL )
        {
            SfxFrame* pFrame =
                static_cast<const SfxFrameItem*>( pFrameItem )->GetFrame();
            DBG_ASSERT( pFrame != NULL, "no frame?" );
            pFrame->InsertDocument( xDocSh );
        }
    }

    // set return value
    rReq.SetReturnValue( SfxVoidItem( rReq.GetSlot() ) );
}
