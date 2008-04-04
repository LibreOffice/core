/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ReportController.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-04 15:11:51 $
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
#include "precompiled_reportdesign.hxx"

#define RPTUI_ID_LRSPACE    1
#define RPTUI_ID_ULSPACE    2
#define RPTUI_ID_PAGE       3
#define RPTUI_ID_SIZE       4
#define RPTUI_ID_PAGE_MODE  5
#define RPTUI_ID_START      6
#define RPTUI_ID_END        7
#define RPTUI_ID_BRUSH      8
#define RPTUI_ID_METRIC     9

#define ITEMID_LRSPACE      RPTUI_ID_LRSPACE
#define ITEMID_ULSPACE      RPTUI_ID_ULSPACE
#define ITEMID_SIZE         RPTUI_ID_SIZE
#define ITEMID_PAGE         RPTUI_ID_PAGE
#define ITEMID_BRUSH        RPTUI_ID_BRUSH


#ifndef RPTUI_REPORTCONTROLLER_HXX
#include "ReportController.hxx"
#endif
#ifndef REPORTDESIGN_API_REPORTDEFINITION_HXX
#include "ReportDefinition.hxx"
#endif
#ifndef RPTUI_CONDFORMAT_HXX
#include "CondFormat.hxx"
#endif
#ifndef RPTUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif
#ifndef RPTUI_DATETIME_HXX
#include "DateTime.hxx"
#endif
#ifndef INCLUDED_SVTOOLS_SYSLOCALE_HXX
#include <svtools/syslocale.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _FILEDLGHELPER_HXX
#include <sfx2/filedlghelper.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif
#ifndef _RPTUI_SLOTID_HRC_
#include "rptui_slotid.hrc"
#endif
#include "reportformula.hxx"
#ifndef _COMPHELPER_DOCUMENTCONSTANTS_HXX
#include <comphelper/documentconstants.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _SVX_FMVIEW_HXX
#include <svx/fmview.hxx>
#endif
#ifndef _SVX_PAPERINF_HXX
#include <svx/paperinf.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCEASHASHMAP_HXX_
#include <comphelper/sequenceashashmap.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _COM_SUN_STAR_VIEW_PAPERFORMAT_HPP_
#include <com/sun/star/view/PaperFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_GRAPHICLOCATION_HPP_
#include <com/sun/star/style/GraphicLocation.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLE_HPP_
#include <com/sun/star/style/XStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_PAGESTYLELAYOUT_HPP_
#include <com/sun/star/style/PageStyleLayout.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XFILEPICKERCONTROLACCESS_HPP_
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_TEMPLATEDESCRIPTION_HPP_
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_EXTENDEDFILEPICKERELEMENTIDS_HPP_
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_REPORT_XImageControl_HPP_
#include <com/sun/star/report/XImageControl.hpp>
#endif
#include <com/sun/star/report/XFixedLine.hpp>
#ifndef _COM_SUN_STAR_REPORT_FUNCTION_HPP_
#include <com/sun/star/report/Function.hpp>
#endif
#ifndef _COM_SUN_STAR_REPORT_XSHAPE_HPP_
#include <com/sun/star/report/XShape.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTDESCRIPTOR_HPP_
#include <com/sun/star/awt/FontDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XDOCUMENTDATASOURCE_HPP_
#include <com/sun/star/sdb/XDocumentDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XPARAMETERSSUPPLIER_HPP_
#include <com/sun/star/sdb/XParametersSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif
#ifndef _COMPHELPER_STREAMSECTION_HXX_
#include <comphelper/streamsection.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTWEIGHT_HPP_
#include <com/sun/star/awt/FontWeight.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTUNDERLINE_HPP_
#include <com/sun/star/awt/FontUnderline.hpp>
#endif
#include <com/sun/star/awt/TextAlign.hpp>
#ifndef _COM_SUN_STAR_AWT_FONTSLANT_HPP_
#include <com/sun/star/awt/FontSlant.hpp>
#endif
#include <com/sun/star/frame/status/FontHeight.hpp>
#include <com/sun/star/report/XFormattedField.hpp>

#include "DesignView.hxx"
#ifndef _RPTUI_MODULE_HELPER_RPT_HXX_
#include "ModuleHelper.hxx"
#endif
#ifndef _REPORT_RPTUIOBJ_HXX
#include "RptObject.hxx"
#endif
#ifndef RPTUI_UNDO_HXX
#include "Undo.hxx"
#endif
#ifndef _SVX_DATACCESSDESCRIPTOR_HXX_
#include <svx/dataaccessdescriptor.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif
#ifndef _SBASLTID_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif
#ifndef REPORTDESIGN_SHARED_UISTRINGS_HRC
#include "uistrings.hrc"
#endif
#ifndef _CLIPLISTENER_HXX
#include <svtools/cliplistener.hxx>
#endif
#ifndef _REPORT_RPTUIDEF_HXX
#include "RptDef.hxx"
#endif
#ifndef REPORT_REPORTSECTION_HXX
#include "ReportSection.hxx"
#endif
#ifndef _REPORT_SECTIONVIEW_HXX
#include "SectionView.hxx"
#endif
#ifndef RPTUI_UNDOACTIONS_HXX
#include "UndoActions.hxx"
#endif
#ifndef RPTUI_DLGPAGE_HXX
#include "dlgpage.hxx"
#endif
#ifndef _RPTUI_DLGRESID_HRC
#include "RptResId.hrc"
#endif
#ifndef _SVX_SVXENUM_HXX
#include <svx/svxenum.hxx>
#endif
#ifndef _SVX_PAGEITEM_HXX
#include <svx/pageitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_SIZEITEM_HXX
#include <svx/sizeitem.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif
#ifndef _SFXITEMSET_HXX //autogen wg. SfxItemSet
#include <svtools/itemset.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef RPTUI_PROPERTYSETFORWARD_HXX
#include "PropertyForward.hxx"
#endif
#ifndef _AEITEM_HXX //CHINA001
#include <svtools/aeitem.hxx> //CHINA001
#endif //CHINA001
#ifndef _SVX_BRSHITEM_HXX
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_FLAGSDEF_HXX
#include <svx/flagsdef.hxx> //CHINA001
#endif
#ifndef _TOOLKIT_HELPER_CONVERT_HXX_
#include <toolkit/helper/convert.hxx>
#endif
#ifndef RPTUI_GROUPS_SORTING_HXX
#include "GroupsSorting.hxx"
#endif
#ifndef RPTUI_PAGENUMBER_HXX
#include "PageNumber.hxx"
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif
#include "UndoEnv.hxx"
#include "InsertFunctions.hxx"
#include <svx/svdpagv.hxx>
#include <boost/mem_fn.hpp>
#include <boost/bind.hpp>
#include <boost/utility.hpp>

#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdbc/SQLWarning.hpp>

#include <cppuhelper/exc_hlp.hxx>
#include <unotools/confignode.hxx>
#include <helpids.hrc>

using namespace ::com::sun::star;
using namespace uno;
using namespace io;
using namespace beans;
using namespace frame;
using namespace util;
using namespace lang;
using namespace container;
using namespace sdbcx;
using namespace sdbc;
using namespace sdb;
using namespace ui;
using namespace ui::dialogs;
using namespace util;
using namespace ::dbtools;
using namespace ::rptui;
using namespace ::dbaui;
using namespace ::comphelper;

// -----------------------------------------------------------------------------
namespace
{
    // comparing two PropertyValue instances
    struct PropertyValueCompare : public ::std::binary_function< beans::PropertyValue, ::rtl::OUString , bool >
    {
        bool operator() (const beans::PropertyValue& x, const ::rtl::OUString& y) const
        {
            return x.Name.equals(y);// ? true : false;
        }
        bool operator() (const ::rtl::OUString& x,const beans::PropertyValue& y) const
        {
            return x.equals(y.Name);// ? true : false;
        }
    };
}

// -----------------------------------------------------------------------------
uno::Reference< report::XReportControlFormat> lcl_getReportControlFormat(const Sequence< PropertyValue >& aArgs,ODesignView* _pView,uno::Reference< awt::XWindow>& _xWindow)
{
    uno::Reference< report::XReportControlFormat> xReportControlFormat;
    if ( aArgs.getLength() )
    {
        SequenceAsHashMap aMap(aArgs);
        xReportControlFormat = aMap.getUnpackedValueOrDefault(REPORTCONTROLFORMAT,uno::Reference< report::XReportControlFormat>());
        _xWindow = aMap.getUnpackedValueOrDefault(CURRENT_WINDOW,uno::Reference< awt::XWindow>());
    } // if ( aArgs.getLength() )
    if ( !xReportControlFormat.is() )
        xReportControlFormat.set( _pView->getCurrentControlModel(),uno::UNO_QUERY);
    if ( !_xWindow.is() )
        _xWindow = VCLUnoHelper::GetInterface(_pView);
    return xReportControlFormat;
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OReportController::getImplementationName() throw( RuntimeException )
{
    return getImplementationName_Static();
}

//------------------------------------------------------------------------------
::rtl::OUString OReportController::getImplementationName_Static() throw( RuntimeException )
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.report.comp.ReportDesign"));
}
//------------------------------------------------------------------------------
Sequence< ::rtl::OUString> OReportController::getSupportedServiceNames_Static(void) throw( RuntimeException )
{
    Sequence< ::rtl::OUString> aSupported(1);
    aSupported.getArray()[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.ReportDesign"));
    return aSupported;
}
//-------------------------------------------------------------------------
Sequence< ::rtl::OUString> SAL_CALL OReportController::getSupportedServiceNames() throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}
// -------------------------------------------------------------------------
Reference< XInterface > OReportController::create(Reference< XComponentContext > const & xContext)
{
    return *(new OReportController(xContext));
}
DBG_NAME( rpt_OReportController )
// -----------------------------------------------------------------------------
OReportController::OReportController(Reference< XComponentContext > const & xContext)
: OReportController_BASE(Reference< XMultiServiceFactory >(xContext->getServiceManager(),UNO_QUERY))
,m_pMyOwnView(NULL)
,m_pClipbordNotifier(NULL)
,m_pGroupsFloater(NULL)
,m_xContext(xContext)
,m_nSplitPos(-1)
,m_nPageNum(-1)
//,m_nExecuteReportEvent(0)
,m_bShowRuler(sal_True)
,m_bGridVisible(sal_True)
,m_bGridUse(sal_True)
,m_bShowProperties(sal_True)
,m_bGroupFloaterWasVisible(sal_False)
,m_bHelplinesMove(sal_True)
,m_bChartEnabled(false)
,m_bChartEnabledAsked(false)
{
    DBG_CTOR( rpt_OReportController,NULL);
}
// -----------------------------------------------------------------------------
OReportController::~OReportController()
{
    DBG_DTOR( rpt_OReportController,NULL);
}
// -----------------------------------------------------------------------------
IMPLEMENT_FORWARD_XTYPEPROVIDER2(OReportController,OReportController_BASE,OReportController_Listener)
IMPLEMENT_FORWARD_XINTERFACE2(OReportController,OReportController_BASE,OReportController_Listener)
// -----------------------------------------------------------------------------
void OReportController::disposing()
{

    if ( getView() && m_pClipbordNotifier )
    {
        m_pClipbordNotifier->ClearCallbackLink();
        m_pClipbordNotifier->AddRemoveListener( getView(), sal_False );
        m_pClipbordNotifier->release();
        m_pClipbordNotifier = NULL;
    }
    {
        ::std::auto_ptr<FloatingWindow> aTemp(m_pGroupsFloater);
        m_pGroupsFloater = NULL;
    }

    try
    {
        ::comphelper::disposeComponent( m_xRowSet );
        ::comphelper::disposeComponent( m_xRowSetMediator );
        ::comphelper::disposeComponent( m_xFormatter );
    }
    catch(uno::Exception&)
    {
        OSL_ENSURE(0,"Exception caught while disposing row sets.");
    }
    m_xRowSet.clear();
    m_xRowSetMediator.clear();

    if ( m_xReportDefinition.is() )
    {
        try
        {
            getUndoMgr()->Clear();      // clear all undo redo things
            listen(false);
        }
        catch(uno::Exception&)
        {
        }
    }
    OReportController_BASE::disposing();
    // disconnect();
    try
    {
        m_xReportDefinition.clear();
        m_aReportModel.reset();
        m_xFrameLoader.clear();
        m_xReportEngine.clear();
    }
    catch(uno::Exception&)
    {
    }
    if ( m_pMyOwnView )
        EndListening(*(m_pMyOwnView));
    m_pView     = NULL;
    m_pMyOwnView = NULL;
}
// -----------------------------------------------------------------------------
FeatureState OReportController::GetState(sal_uInt16 _nId) const
{
    FeatureState aReturn;
    // (disabled automatically)
    aReturn.bEnabled = sal_False;
    // check this first
    if ( !getView() )
        return aReturn;

    switch (_nId)
    {
        case SID_RPT_TEXTDOCUMENT:
            aReturn.bEnabled = isEditable();
            aReturn.bChecked = (m_xReportDefinition.is() && m_xReportDefinition->getMimeType() == MIMETYPE_OASIS_OPENDOCUMENT_TEXT);
            break;
        case SID_RPT_SPREADSHEET:
            aReturn.bEnabled = isEditable();
            aReturn.bChecked = (m_xReportDefinition.is() && m_xReportDefinition->getMimeType() == MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET);
            break;
        case SID_REPORTHEADER_WITHOUT_UNDO:
        case SID_REPORTFOOTER_WITHOUT_UNDO:
        case SID_REPORTHEADERFOOTER:
            {
                aReturn.bEnabled = isEditable();
                ::rtl::OUString sText = String(ModuleRes((m_xReportDefinition.is() && m_xReportDefinition->getReportHeaderOn()) ? RID_STR_REPORTHEADERFOOTER_DELETE : RID_STR_REPORTHEADERFOOTER_INSERT));
                aReturn.sTitle = sText;
            }
            break;
        case SID_PAGEHEADER_WITHOUT_UNDO:
        case SID_PAGEFOOTER_WITHOUT_UNDO:
        case SID_PAGEHEADERFOOTER:
            {
                aReturn.bEnabled = isEditable();
                ::rtl::OUString sText = String(ModuleRes((m_xReportDefinition.is() && m_xReportDefinition->getPageHeaderOn()) ? RID_STR_PAGEHEADERFOOTER_DELETE : RID_STR_PAGEHEADERFOOTER_INSERT));
                aReturn.sTitle = sText;
            }
            break;
        case SID_GROUP_APPEND:
        case SID_GROUP_REMOVE:
        case SID_GROUPHEADER_WITHOUT_UNDO:
        case SID_GROUPHEADER:
        case SID_GROUPFOOTER_WITHOUT_UNDO:
        case SID_GROUPFOOTER:
            aReturn.bEnabled = isEditable();
            break;
        case SID_ADD_CONTROL_PAIR:
            aReturn.bEnabled = isEditable() && m_pMyOwnView->getMarkedSection().get() != NULL;
            break;
        case SID_REDO:
        case SID_UNDO:
            aReturn = OReportController_BASE::GetState(_nId);
            aReturn.bEnabled = aReturn.bEnabled;
            break;
        case SID_OBJECT_RESIZING:
        case SID_OBJECT_SMALLESTWIDTH:
        case SID_OBJECT_SMALLESTHEIGHT:
        case SID_OBJECT_GREATESTWIDTH:
        case SID_OBJECT_GREATESTHEIGHT:
            aReturn.bEnabled = isEditable() && m_pMyOwnView->HasSelection() && m_pMyOwnView->isAlignPossible();
            if ( aReturn.bEnabled )
                aReturn.bEnabled = m_pMyOwnView->getMarkedObjectCount() > 1;
            break;

        case SID_DISTRIBUTION:
            aReturn.bEnabled = isEditable() && m_pMyOwnView->HasSelection();
            if ( aReturn.bEnabled )
            {
                OSectionView* pSectionView = getCurrentSectionView();
                aReturn.bEnabled = pSectionView && pSectionView->GetMarkedObjectCount() > 2;
            }
            break;
        case SID_FRAME_DOWN:
        case SID_FRAME_UP:
            aReturn.bEnabled = sal_False;
            break;
        case SID_ARRANGEMENU:
        case SID_FRAME_TO_TOP:
        case SID_FRAME_TO_BOTTOM:
        case SID_OBJECT_HEAVEN:
        case SID_OBJECT_HELL:
            aReturn.bEnabled = isEditable() && m_pMyOwnView->HasSelection();
            if ( aReturn.bEnabled )
            {
                uno::Reference< report::XShape> xShape(m_pMyOwnView->getCurrentControlModel(),uno::UNO_QUERY);
                aReturn.bEnabled = xShape.is();
                if ( aReturn.bEnabled )
                {
                    OSectionView* pSectionView = getCurrentSectionView();
                    if ( pSectionView )
                    {
                        switch(_nId)
                        {
                            case SID_OBJECT_HEAVEN:
                                aReturn.bEnabled = pSectionView->IsToTopPossible();
                                break;
                            case SID_OBJECT_HELL:
                                aReturn.bEnabled = pSectionView->IsToBtmPossible();
                                break;
                            default:
                                break;
                        }
                    }
                }
            }
            break;

        case SID_OBJECT_ALIGN:
        case SID_OBJECT_ALIGN_LEFT:
        case SID_OBJECT_ALIGN_CENTER:
        case SID_OBJECT_ALIGN_RIGHT:
        case SID_OBJECT_ALIGN_UP:
        case SID_OBJECT_ALIGN_MIDDLE:
        case SID_OBJECT_ALIGN_DOWN:
        case SID_SECTION_ALIGN:
        case SID_SECTION_ALIGN_LEFT:
        case SID_SECTION_ALIGN_CENTER:
        case SID_SECTION_ALIGN_RIGHT:
        case SID_SECTION_ALIGN_UP:
        case SID_SECTION_ALIGN_MIDDLE:
        case SID_SECTION_ALIGN_DOWN:
            aReturn.bEnabled = isEditable() && m_pMyOwnView->HasSelection() && m_pMyOwnView->isAlignPossible();
            break;
        case SID_CUT:
            aReturn.bEnabled = isEditable() && m_pMyOwnView->HasSelection() && !m_pMyOwnView->isHandleEvent(_nId);
            break;
        case SID_COPY:
            aReturn.bEnabled = m_pMyOwnView->HasSelection() && !m_pMyOwnView->isHandleEvent(_nId);
            break;
        case SID_PASTE:
            aReturn.bEnabled = isEditable()  && !m_pMyOwnView->isHandleEvent(_nId) && m_pMyOwnView->IsPasteAllowed();
            break;
        case SID_SELECTALL:
            aReturn.bEnabled = !m_pMyOwnView->isHandleEvent(_nId);
            break;
        case SID_SELECTALL_IN_SECTION:
            aReturn.bEnabled = !m_pMyOwnView->isHandleEvent(_nId);
            if ( aReturn.bEnabled )
                aReturn.bEnabled = getCurrentSectionView() != NULL;
            break;
        case SID_ESCAPE:
            aReturn.bEnabled = m_pMyOwnView->GetMode() == RPTUI_INSERT;
            break;
        case SID_RPT_NEW_FUNCTION:
            aReturn.bEnabled = isEditable();
            break;
        case SID_NEXT_MARK:
        case SID_PREV_MARK:
            aReturn.bEnabled = isEditable();
            break;
        case SID_SELECT:
        case SID_SELECT_REPORT:
            aReturn.bEnabled = sal_True;
            break;
        case SID_EXECUTE_REPORT:
            aReturn.bEnabled = isConnected() && m_xReportDefinition.is();
            break;
        case SID_DELETE:
            aReturn.bEnabled = isEditable() && m_pMyOwnView->HasSelection() && !m_pMyOwnView->isHandleEvent(_nId);
            {
                ::rtl::OUString sText = String(ModuleRes(RID_STR_DELETE));
                aReturn.sTitle = sText;
            }
            break;
        case SID_GRID_VISIBLE:
            aReturn.bEnabled = isEditable();
            aReturn.bChecked = m_bGridVisible;
            break;
        case SID_GRID_USE:
            aReturn.bEnabled = isEditable();
            aReturn.bChecked = m_bGridUse;
            break;
        case SID_HELPLINES_MOVE:
            aReturn.bEnabled = isEditable();
            aReturn.bChecked = m_bHelplinesMove;
            break;
        case SID_RULER:
            aReturn.bEnabled = isEditable();
            aReturn.bChecked = m_bShowRuler;
            break;
        case SID_OBJECT_SELECT:
            aReturn.bEnabled = sal_True;
            aReturn.bChecked = m_pMyOwnView->GetMode() == RPTUI_SELECT;
            break;
        case SID_INSERT_DIAGRAM:
            aReturn.bEnabled = isEditable();
            aReturn.bInvisible = optional< bool >(!m_bChartEnabled);
            aReturn.bChecked = m_pMyOwnView->GetInsertObj() == OBJ_OLE2;
            break;
        case SID_FM_FIXEDTEXT:
            aReturn.bEnabled = isEditable();
            aReturn.bChecked = m_pMyOwnView->GetInsertObj() == OBJ_DLG_FIXEDTEXT;
            break;
        case SID_INSERT_HFIXEDLINE:
            aReturn.bEnabled = isEditable();
            aReturn.bChecked = m_pMyOwnView->GetInsertObj() == OBJ_DLG_HFIXEDLINE;
            break;
        case SID_INSERT_VFIXEDLINE:
            aReturn.bEnabled = isEditable();
            aReturn.bChecked = m_pMyOwnView->GetInsertObj() == OBJ_DLG_VFIXEDLINE;
            break;
        case SID_FM_EDIT:
            aReturn.bEnabled = isEditable();
            aReturn.bChecked = m_pMyOwnView->GetInsertObj() == OBJ_DLG_FORMATTEDFIELD;
            break;
        case SID_FM_IMAGECONTROL:
            aReturn.bEnabled = isEditable();
            aReturn.bChecked = m_pMyOwnView->GetInsertObj() == OBJ_DLG_IMAGECONTROL;
            break;
        case SID_DRAWTBX_CS_BASIC:
        case SID_DRAWTBX_CS_BASIC1:
        case SID_DRAWTBX_CS_BASIC2:
        case SID_DRAWTBX_CS_BASIC3:
        case SID_DRAWTBX_CS_BASIC4:
        case SID_DRAWTBX_CS_BASIC5:
        case SID_DRAWTBX_CS_BASIC6:
        case SID_DRAWTBX_CS_BASIC7:
        case SID_DRAWTBX_CS_BASIC8:
        case SID_DRAWTBX_CS_BASIC9:
        case SID_DRAWTBX_CS_BASIC10:
        case SID_DRAWTBX_CS_BASIC11:
        case SID_DRAWTBX_CS_BASIC12:
        case SID_DRAWTBX_CS_BASIC13:
        case SID_DRAWTBX_CS_BASIC14:
        case SID_DRAWTBX_CS_BASIC15:
        case SID_DRAWTBX_CS_BASIC16:
        case SID_DRAWTBX_CS_BASIC17:
        case SID_DRAWTBX_CS_BASIC18:
        case SID_DRAWTBX_CS_BASIC19:
        case SID_DRAWTBX_CS_BASIC20:
        case SID_DRAWTBX_CS_BASIC21:
        case SID_DRAWTBX_CS_BASIC22:
            {
                aReturn.bEnabled = isEditable();
                rtl::OUString sShapeType = m_pMyOwnView->GetInsertObjString();
                aReturn.bChecked = m_pMyOwnView->GetInsertObj() == OBJ_CUSTOMSHAPE &&
                                   sShapeType.compareToAscii("diamond") == 0;
            }
            break;
        case SID_DRAWTBX_CS_SYMBOL:
        case SID_DRAWTBX_CS_SYMBOL1:
        case SID_DRAWTBX_CS_SYMBOL2:
        case SID_DRAWTBX_CS_SYMBOL3:
        case SID_DRAWTBX_CS_SYMBOL4:
        case SID_DRAWTBX_CS_SYMBOL5:
        case SID_DRAWTBX_CS_SYMBOL6:
        case SID_DRAWTBX_CS_SYMBOL7:
        case SID_DRAWTBX_CS_SYMBOL8:
        case SID_DRAWTBX_CS_SYMBOL9:
        case SID_DRAWTBX_CS_SYMBOL10:
        case SID_DRAWTBX_CS_SYMBOL11:
        case SID_DRAWTBX_CS_SYMBOL12:
        case SID_DRAWTBX_CS_SYMBOL13:
        case SID_DRAWTBX_CS_SYMBOL14:
        case SID_DRAWTBX_CS_SYMBOL15:
        case SID_DRAWTBX_CS_SYMBOL16:
        case SID_DRAWTBX_CS_SYMBOL17:
        case SID_DRAWTBX_CS_SYMBOL18:
            {
                aReturn.bEnabled = isEditable();
                rtl::OUString sShapeType = m_pMyOwnView->GetInsertObjString();
                aReturn.bChecked = m_pMyOwnView->GetInsertObj() == OBJ_CUSTOMSHAPE &&
                                   sShapeType.compareToAscii("smiley") == 0;
            }
            break;
        case SID_DRAWTBX_CS_ARROW:
        case SID_DRAWTBX_CS_ARROW1:
        case SID_DRAWTBX_CS_ARROW2:
        case SID_DRAWTBX_CS_ARROW3:
        case SID_DRAWTBX_CS_ARROW4:
        case SID_DRAWTBX_CS_ARROW5:
        case SID_DRAWTBX_CS_ARROW6:
        case SID_DRAWTBX_CS_ARROW7:
        case SID_DRAWTBX_CS_ARROW8:
        case SID_DRAWTBX_CS_ARROW9:
        case SID_DRAWTBX_CS_ARROW10:
        case SID_DRAWTBX_CS_ARROW11:
        case SID_DRAWTBX_CS_ARROW12:
        case SID_DRAWTBX_CS_ARROW13:
        case SID_DRAWTBX_CS_ARROW14:
        case SID_DRAWTBX_CS_ARROW15:
        case SID_DRAWTBX_CS_ARROW16:
        case SID_DRAWTBX_CS_ARROW17:
        case SID_DRAWTBX_CS_ARROW18:
        case SID_DRAWTBX_CS_ARROW19:
        case SID_DRAWTBX_CS_ARROW20:
        case SID_DRAWTBX_CS_ARROW21:
        case SID_DRAWTBX_CS_ARROW22:
        case SID_DRAWTBX_CS_ARROW23:
        case SID_DRAWTBX_CS_ARROW24:
        case SID_DRAWTBX_CS_ARROW25:
        case SID_DRAWTBX_CS_ARROW26:
            {
                aReturn.bEnabled = isEditable();
                rtl::OUString sShapeType = m_pMyOwnView->GetInsertObjString();
                aReturn.bChecked = m_pMyOwnView->GetInsertObj() == OBJ_CUSTOMSHAPE &&
                                   sShapeType.compareToAscii("left-right-arrow") == 0;
            }
            break;
        case SID_DRAWTBX_CS_STAR:
        case SID_DRAWTBX_CS_STAR1:
        case SID_DRAWTBX_CS_STAR2:
        case SID_DRAWTBX_CS_STAR3:
        case SID_DRAWTBX_CS_STAR4:
        case SID_DRAWTBX_CS_STAR5:
        case SID_DRAWTBX_CS_STAR6:
        case SID_DRAWTBX_CS_STAR7:
        case SID_DRAWTBX_CS_STAR8:
        case SID_DRAWTBX_CS_STAR9:
        case SID_DRAWTBX_CS_STAR10:
        case SID_DRAWTBX_CS_STAR11:
        case SID_DRAWTBX_CS_STAR12:
            {
                aReturn.bEnabled = isEditable();
                rtl::OUString sShapeType = m_pMyOwnView->GetInsertObjString();
                aReturn.bChecked = m_pMyOwnView->GetInsertObj() == OBJ_CUSTOMSHAPE &&
                                   sShapeType.compareToAscii("star5") == 0;
            }
            break;
        case SID_DRAWTBX_CS_FLOWCHART:
        case SID_DRAWTBX_CS_FLOWCHART1:
        case SID_DRAWTBX_CS_FLOWCHART2:
        case SID_DRAWTBX_CS_FLOWCHART3:
        case SID_DRAWTBX_CS_FLOWCHART4:
        case SID_DRAWTBX_CS_FLOWCHART5:
        case SID_DRAWTBX_CS_FLOWCHART6:
        case SID_DRAWTBX_CS_FLOWCHART7:
        case SID_DRAWTBX_CS_FLOWCHART8:
        case SID_DRAWTBX_CS_FLOWCHART9:
        case SID_DRAWTBX_CS_FLOWCHART10:
        case SID_DRAWTBX_CS_FLOWCHART11:
        case SID_DRAWTBX_CS_FLOWCHART12:
        case SID_DRAWTBX_CS_FLOWCHART13:
        case SID_DRAWTBX_CS_FLOWCHART14:
        case SID_DRAWTBX_CS_FLOWCHART15:
        case SID_DRAWTBX_CS_FLOWCHART16:
        case SID_DRAWTBX_CS_FLOWCHART17:
        case SID_DRAWTBX_CS_FLOWCHART18:
        case SID_DRAWTBX_CS_FLOWCHART19:
        case SID_DRAWTBX_CS_FLOWCHART20:
        case SID_DRAWTBX_CS_FLOWCHART21:
        case SID_DRAWTBX_CS_FLOWCHART22:
        case SID_DRAWTBX_CS_FLOWCHART23:
        case SID_DRAWTBX_CS_FLOWCHART24:
        case SID_DRAWTBX_CS_FLOWCHART25:
        case SID_DRAWTBX_CS_FLOWCHART26:
        case SID_DRAWTBX_CS_FLOWCHART27:
        case SID_DRAWTBX_CS_FLOWCHART28:
            {
                aReturn.bEnabled = isEditable();
                rtl::OUString sShapeType = m_pMyOwnView->GetInsertObjString();
                aReturn.bChecked = m_pMyOwnView->GetInsertObj() == OBJ_CUSTOMSHAPE &&
                                   sShapeType.compareToAscii("flowchart-internal-storage") == 0;
            }
            break;
        case SID_DRAWTBX_CS_CALLOUT:
        case SID_DRAWTBX_CS_CALLOUT1:
        case SID_DRAWTBX_CS_CALLOUT2:
        case SID_DRAWTBX_CS_CALLOUT3:
        case SID_DRAWTBX_CS_CALLOUT4:
        case SID_DRAWTBX_CS_CALLOUT5:
        case SID_DRAWTBX_CS_CALLOUT6:
        case SID_DRAWTBX_CS_CALLOUT7:
            {
            aReturn.bEnabled = isEditable();
                rtl::OUString sShapeType = m_pMyOwnView->GetInsertObjString();
                aReturn.bChecked = m_pMyOwnView->GetInsertObj() == OBJ_CUSTOMSHAPE &&
                                   sShapeType.compareToAscii("round-rectangular-callout") == 0;
            }
            break;
        case SID_RPT_SHOWREPORTEXPLORER:
            aReturn.bEnabled = m_xReportDefinition.is();
            aReturn.bChecked = m_pMyOwnView && m_pMyOwnView->isReportExplorerVisible();
            break;
        case SID_FM_ADD_FIELD:
            aReturn.bEnabled = isConnected() && isEditable() && m_xReportDefinition.is()
                && m_xReportDefinition->getCommand().getLength();
            aReturn.bChecked = m_pMyOwnView && m_pMyOwnView->isAddFieldVisible();
            break;
        case SID_SHOW_PROPERTYBROWSER:
            aReturn.bEnabled = sal_True;
            aReturn.bChecked = m_bShowProperties;
            break;
        case SID_PROPERTYBROWSER_LAST_PAGE:
            aReturn.bEnabled = sal_True;
            aReturn.aValue <<= m_sLastActivePage;
            break;
        case SID_SPLIT_POSITION:
            aReturn.bEnabled = sal_True;
            aReturn.aValue <<= getSplitPos();
            break;
        case SID_SAVEDOC:
            aReturn.bEnabled = isModified() && isEditable();
            break;
        case SID_SAVEASDOC:
            aReturn.bEnabled = isConnected() && isEditable();
            break;
        case SID_EDITDOC:
            aReturn.bChecked = isEditable();
            break;
        case SID_PAGEDIALOG:
            aReturn.bEnabled = isEditable();
            break;
        case SID_BACKGROUND_COLOR:
            aReturn.bEnabled = isEditable();
            {
                uno::Reference< report::XReportComponent > xReportComponent(m_pMyOwnView->getCurrentControlModel());
                uno::Reference< report::XReportControlFormat> xReportControlFormat(xReportComponent,uno::UNO_QUERY);
                if ( xReportControlFormat.is() )
                {
                    aReturn.bEnabled = xReportComponent->getPropertySetInfo()->hasPropertyByName(PROPERTY_CONTROLBACKGROUND);
                    if ( aReturn.bEnabled )
                        aReturn.aValue <<= xReportControlFormat->getControlBackground();
                }
            }
            break;
        case SID_ATTR_CHAR_COLOR_BACKGROUND:
            aReturn.bEnabled = isEditable();
            {
                uno::Reference<report::XSection> xSection = m_pMyOwnView->getCurrentSection();
                if ( xSection.is() )
                    try
                    {
                        aReturn.aValue <<= xSection->getBackColor();
                        uno::Reference< report::XReportControlModel> xControlModel(m_pMyOwnView->getCurrentControlModel(),uno::UNO_QUERY);
                        aReturn.bEnabled = !xControlModel.is();
                    }
                    catch(beans::UnknownPropertyException&)
                    {
                    }
                else
                    aReturn.bEnabled = sal_False;
            }
            break;
        case SID_SORTINGANDGROUPING:
            aReturn.bEnabled = sal_True;
            aReturn.bChecked = m_pGroupsFloater && m_pGroupsFloater->IsVisible();
            break;
        case SID_ATTR_CHAR_WEIGHT:
        case SID_ATTR_CHAR_POSTURE:
        case SID_ATTR_CHAR_UNDERLINE:
            aReturn.bEnabled = m_xReportDefinition.is() && isEditable() && m_pMyOwnView->getCurrentControlModel().is();
            aReturn.bChecked = isFormatCommandEnabled(_nId,uno::Reference< report::XReportControlFormat>(m_pMyOwnView->getCurrentControlModel(),uno::UNO_QUERY));
            break;
        case SID_ATTR_CHAR_COLOR:
        case SID_ATTR_CHAR_COLOR2:
            {
                uno::Reference< report::XReportControlFormat> xReportControlFormat(m_pMyOwnView->getCurrentControlModel(),uno::UNO_QUERY);
                aReturn.bEnabled = isEditable() && xReportControlFormat.is() && !uno::Reference< report::XFixedLine>(xReportControlFormat,uno::UNO_QUERY).is();
                if ( aReturn.bEnabled )
                {
                    try
                    {
                        aReturn.aValue <<= xReportControlFormat->getCharColor();
                    }
                    catch(beans::UnknownPropertyException&)
                    {
                        aReturn.bEnabled = sal_False;
                    }
                }
            }
            break;
        case SID_ATTR_CHAR_FONT:
            aReturn.bEnabled = m_xReportDefinition.is() && isEditable();
            if ( aReturn.bEnabled )
            {
                uno::Reference< report::XReportControlModel> xControlModel(m_pMyOwnView->getCurrentControlModel(),uno::UNO_QUERY);
                aReturn.bEnabled = xControlModel.is() && !uno::Reference< report::XFixedLine>(xControlModel,uno::UNO_QUERY).is();
                if ( aReturn.bEnabled )
                {
                    try
                    {
                        aReturn.aValue <<= xControlModel->getFontDescriptor();
                    }
                    catch(beans::UnknownPropertyException&)
                    {
                        aReturn.bEnabled = sal_False;
                    }
                }
            }
            break;
        case SID_ATTR_CHAR_FONTHEIGHT:
            aReturn.bEnabled = m_xReportDefinition.is() && isEditable();
            if ( aReturn.bEnabled )
            {
                uno::Reference< report::XReportControlModel> xControlModel(m_pMyOwnView->getCurrentControlModel(),uno::UNO_QUERY);
                aReturn.bEnabled = xControlModel.is() && !uno::Reference< report::XFixedLine>(xControlModel,uno::UNO_QUERY).is();
                if ( aReturn.bEnabled )
                {
                    try
                    {
                        frame::status::FontHeight aFontHeight;
                        aFontHeight.Height = xControlModel->getCharHeight();
                        aReturn.aValue <<= aFontHeight;
                    }
                    catch(beans::UnknownPropertyException&)
                    {
                        aReturn.bEnabled = sal_False;
                    }
                }
            }
            break;
        case SID_ATTR_PARA_ADJUST_LEFT:
        case SID_ATTR_PARA_ADJUST_CENTER:
        case SID_ATTR_PARA_ADJUST_RIGHT:
            aReturn.bEnabled = m_xReportDefinition.is() && isEditable();
            if ( aReturn.bEnabled )
            {
                uno::Reference< report::XReportControlModel> xControlModel(m_pMyOwnView->getCurrentControlModel(),uno::UNO_QUERY);
                aReturn.bEnabled = xControlModel.is();
                if ( aReturn.bEnabled )
                {
                    ::sal_Int16 nParaAdjust = xControlModel->getParaAdjust();
                    switch(nParaAdjust)
                    {
                        case awt::TextAlign::LEFT:
                            aReturn.bChecked = _nId == SID_ATTR_PARA_ADJUST_LEFT;
                            break;
                        case awt::TextAlign::CENTER:
                            aReturn.bChecked = _nId == SID_ATTR_PARA_ADJUST_CENTER;
                            break;
                        case awt::TextAlign::RIGHT:
                            aReturn.bChecked = _nId == SID_ATTR_PARA_ADJUST_RIGHT;
                            break;
                    }
                }
            }
            break;

        case SID_INSERT_GRAPHIC:
            aReturn.bEnabled = m_xReportDefinition.is() && isEditable() && m_pMyOwnView->getCurrentSection().is();
            break;
        case SID_CHAR_DLG:
        case SID_SETCONTROLDEFAULTS:
            aReturn.bEnabled = m_xReportDefinition.is() && isEditable() && m_pMyOwnView->getCurrentControlModel().is();
            break;
        case SID_CONDITIONALFORMATTING:
            {
                uno::Reference< report::XFormattedField> xFormattedField(m_pMyOwnView->getCurrentControlModel(),uno::UNO_QUERY);
                aReturn.bEnabled = xFormattedField.is();
            }
            break;
        case SID_INSERT_FLD_PGNUMBER:
        case SID_DATETIME:
            aReturn.bEnabled = m_xReportDefinition.is() && isEditable() && m_pMyOwnView->getCurrentSection().is();
            break;
        case SID_EXPORTDOC:
        case SID_EXPORTDOCASPDF:
            aReturn.bEnabled = m_xReportDefinition.is();
            break;
        case SID_PRINTPREVIEW:
            aReturn.bEnabled = sal_False;
            break;
        default:
            aReturn = OReportController_BASE::GetState(_nId);
    }
    return aReturn;
}

// -----------------------------------------------------------------------------
namespace
{
    /** extracts a background color from a dispatched SID_BACKGROUND_COLOR call

        The dispatch might originate from either the toolbar, or the conditional
        formatting dialog. In both cases, argument formats are different.
    */
    util::Color lcl_extractBackgroundColor( const Sequence< PropertyValue >& _rDispatchArgs )
    {
        util::Color aColor( COL_TRANSPARENT );
        if ( _rDispatchArgs.getLength() == 1 )
        {
            OSL_VERIFY( _rDispatchArgs[0].Value >>= aColor );
        }
        else
        {
            SequenceAsHashMap aMap( _rDispatchArgs );
            aColor = aMap.getUnpackedValueOrDefault( PROPERTY_FONTCOLOR, aColor );
        }
        return aColor;
    }
}

// -----------------------------------------------------------------------------
void OReportController::Execute(sal_uInt16 _nId, const Sequence< PropertyValue >& aArgs)
{
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard aGuard(m_aMutex);
    if ( !getView() )
    {
        switch(_nId)
        {
            case SID_RULER:
                OSL_ENSURE(aArgs.getLength() == 1,"Invalid length!");
                aArgs[0].Value >>= m_bShowRuler;
                break;
            case SID_HELPLINES_MOVE:
                OSL_ENSURE(aArgs.getLength() == 1,"Invalid length!");
                aArgs[0].Value >>= m_bHelplinesMove;
                break;
            case SID_GRID_VISIBLE:
                OSL_ENSURE(aArgs.getLength() == 1,"Invalid length!");
                aArgs[0].Value >>= m_bGridVisible;
                break;
            case SID_SHOW_PROPERTYBROWSER:
                OSL_ENSURE(aArgs.getLength() == 1,"Invalid length!");
                aArgs[0].Value >>= m_bShowProperties;
                break;
            case SID_PROPERTYBROWSER_LAST_PAGE:
                OSL_ENSURE(aArgs.getLength() == 1,"Invalid length!");
                aArgs[0].Value >>= m_sLastActivePage;
                break;
            case SID_SPLIT_POSITION:
                OSL_ENSURE(aArgs.getLength() == 1,"Invalid length!");
                aArgs[0].Value >>= m_nSplitPos;
                break;
        }
        return; // return without execution
    }
    sal_Bool bForceBroadcast = sal_False;
    switch(_nId)
    {
        case SID_RPT_TEXTDOCUMENT:
            if ( m_xReportDefinition.is() )
                m_xReportDefinition->setMimeType( MIMETYPE_OASIS_OPENDOCUMENT_TEXT );
            break;
        case SID_RPT_SPREADSHEET:
            if (m_xReportDefinition.is() )
                m_xReportDefinition->setMimeType( MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET );
            break;
        case SID_REPORTHEADER_WITHOUT_UNDO:
        case SID_REPORTFOOTER_WITHOUT_UNDO:
        case SID_REPORTHEADERFOOTER:
            switchReportSection(_nId);
            break;
        case SID_PAGEHEADER_WITHOUT_UNDO:
        case SID_PAGEFOOTER_WITHOUT_UNDO:
        case SID_PAGEHEADERFOOTER:
            switchPageSection(_nId);
            break;
        case SID_GROUP_APPEND:
        case SID_GROUP_REMOVE:
            modifyGroup(_nId == SID_GROUP_APPEND,aArgs);
            break;
        case SID_GROUPHEADER_WITHOUT_UNDO:
        case SID_GROUPHEADER:
            createGroupSection(SID_GROUPHEADER == _nId,true,aArgs);
            break;
        case SID_GROUPFOOTER_WITHOUT_UNDO:
        case SID_GROUPFOOTER:
            createGroupSection(SID_GROUPFOOTER == _nId,false,aArgs);
            break;
        case SID_ADD_CONTROL_PAIR:
            addPairControls(aArgs);
            break;
        case SID_REDO:
        case SID_UNDO:
        {
            const OXUndoEnvironment::OUndoEnvLock aLock( m_aReportModel->GetUndoEnv() );
            OReportController_BASE::Execute( _nId, aArgs );
            InvalidateFeature( SID_UNDO );
            updateFloater();
        }
        break;
        case SID_CUT:
            executeMethodWithUndo(RID_STR_UNDO_REMOVE_SELECTION,::std::mem_fun(&ODesignView::Cut));
            break;
        case SID_COPY:
            m_pMyOwnView->Copy();
            break;
        case SID_PASTE:
            executeMethodWithUndo(RID_STR_UNDO_PASTE,::std::mem_fun(&ODesignView::Paste));
            break;

        case SID_FRAME_TO_TOP:
        case SID_FRAME_DOWN:
        case SID_FRAME_UP:
        case SID_FRAME_TO_BOTTOM:
        case SID_OBJECT_HEAVEN:
        case SID_OBJECT_HELL:
            changeZOrder(_nId);
            break;
        case SID_DISTRIBUTION:
            {
                OSectionView* pSectionView = getCurrentSectionView();
                if ( pSectionView )
                    pSectionView->DistributeMarkedObjects();
            }
            break;
        case SID_OBJECT_SMALLESTWIDTH:
            alignControlsWithUndo(RID_STR_UNDO_ALIGNMENT,ControlModification::WIDTH_SMALLEST);
            break;
        case SID_OBJECT_SMALLESTHEIGHT:
            alignControlsWithUndo(RID_STR_UNDO_ALIGNMENT,ControlModification::HEIGHT_SMALLEST);
            break;
        case SID_OBJECT_GREATESTWIDTH:
            alignControlsWithUndo(RID_STR_UNDO_ALIGNMENT,ControlModification::WIDTH_GREATEST);
            break;
        case SID_OBJECT_GREATESTHEIGHT:
            alignControlsWithUndo(RID_STR_UNDO_ALIGNMENT,ControlModification::HEIGHT_GREATEST);
            break;
        case SID_SECTION_ALIGN_LEFT:
        case SID_OBJECT_ALIGN_LEFT:
            alignControlsWithUndo(RID_STR_UNDO_ALIGNMENT,ControlModification::LEFT,SID_SECTION_ALIGN_LEFT == _nId);
            break;
        case SID_SECTION_ALIGN_CENTER:
        case SID_OBJECT_ALIGN_CENTER:
            alignControlsWithUndo(RID_STR_UNDO_ALIGNMENT,ControlModification::CENTER_HORIZONTAL,SID_SECTION_ALIGN_CENTER == _nId);
            break;
        case SID_SECTION_ALIGN_RIGHT:
        case SID_OBJECT_ALIGN_RIGHT:
            alignControlsWithUndo(RID_STR_UNDO_ALIGNMENT,ControlModification::RIGHT,SID_SECTION_ALIGN_RIGHT == _nId);
            break;
        case SID_SECTION_ALIGN_UP:
        case SID_OBJECT_ALIGN_UP:
            alignControlsWithUndo(RID_STR_UNDO_ALIGNMENT,ControlModification::TOP,SID_SECTION_ALIGN_UP == _nId);
            break;
        case SID_SECTION_ALIGN_MIDDLE:
        case SID_OBJECT_ALIGN_MIDDLE:
            alignControlsWithUndo(RID_STR_UNDO_ALIGNMENT,ControlModification::CENTER_VERTICAL,SID_SECTION_ALIGN_MIDDLE == _nId);
            break;
        case SID_SECTION_ALIGN_DOWN:
        case SID_OBJECT_ALIGN_DOWN:
            alignControlsWithUndo(RID_STR_UNDO_ALIGNMENT,ControlModification::BOTTOM,SID_SECTION_ALIGN_DOWN == _nId);
            break;
        case SID_SELECTALL:
            m_pMyOwnView->SelectAll();
            break;
        case SID_SELECTALL_IN_SECTION:
            {
                OSectionView* pSectionView = getCurrentSectionView();
                if ( pSectionView )
                    pSectionView->MarkAll();
            }
            break;
        case SID_ESCAPE:
            m_pMyOwnView->SetMode(RPTUI_SELECT);
            InvalidateFeature( SID_OBJECT_SELECT );
            break;
        case SID_SELECT:
            if ( aArgs.getLength() == 1 )
                select(aArgs[0].Value);
            break;
        case SID_SELECT_REPORT:
            select(uno::makeAny(m_xReportDefinition));
            break;
        case SID_EXECUTE_REPORT:
            /*m_nExecuteReportEvent = */getView()->PostUserEvent(LINK(this, OReportController,OnExecuteReport));
            break;
        case SID_RPT_NEW_FUNCTION:
            createNewFunction(aArgs[0].Value);
            break;
        case SID_NEXT_MARK:
            markSection(true);
            break;
        case SID_PREV_MARK:
            markSection(false);
            break;
        case SID_DELETE:
            if ( aArgs.getLength() == 1 )
            {
                uno::Reference< report::XFunction> xFunction;
                aArgs[0].Value >>= xFunction;
                if ( xFunction.is() )
                {
                    uno::Reference< report::XFunctions> xFunctions(xFunction->getParent(),uno::UNO_QUERY_THROW);
                    sal_Int32 nIndex = getPositionInIndexAccess(xFunctions.get(),xFunction);
                    String sUndoAction = String((ModuleRes(RID_STR_UNDO_REMOVE_FUNCTION)));
                    UndoManagerListAction aListAction(m_aUndoManager,sUndoAction);
                    xFunctions->removeByIndex(nIndex);
                    select(uno::makeAny(xFunctions->getParent()));
                    InvalidateFeature( SID_SAVEDOC );
                    InvalidateFeature( SID_UNDO );
                }
            }
            else
                executeMethodWithUndo(RID_STR_UNDO_REMOVE_SELECTION,::std::mem_fun(&ODesignView::Delete));
            break;
        case SID_GRID_USE:
            m_pMyOwnView->setGridSnap(m_bGridUse = !m_bGridUse);
            break;
        case SID_HELPLINES_MOVE:
            m_pMyOwnView->setDragStripes(m_bHelplinesMove = !m_bHelplinesMove);
            break;
        case SID_GRID_VISIBLE:
            m_pMyOwnView->toggleGrid(m_bGridVisible = !m_bGridVisible);
            break;
        case SID_RULER:
            m_pMyOwnView->showRuler(m_bShowRuler = !m_bShowRuler);
            break;
        case SID_OBJECT_SELECT:
            m_pMyOwnView->SetMode(RPTUI_SELECT);
            InvalidateAll();
            break;
        case SID_INSERT_DIAGRAM:
            /*{
                OSectionView* pView = getCurrentSectionView();
                if ( pView )
                {
                    Reference< awt::XWindow> xWindow = VCLUnoHelper::GetInterface(getView()->Window::GetParent());
                    InsertChart(m_xContext,m_xReportDefinition.get(),xWindow,pView,getSdrModel().get());
                }
            }*/
            m_pMyOwnView->SetMode( RPTUI_INSERT );
            m_pMyOwnView->SetInsertObj( OBJ_OLE2);
            createDefaultControl(aArgs);
            InvalidateAll();
            break;
        case SID_FM_FIXEDTEXT:
            m_pMyOwnView->SetMode( RPTUI_INSERT );
            m_pMyOwnView->SetInsertObj( OBJ_DLG_FIXEDTEXT );
            createDefaultControl(aArgs);
            InvalidateAll();
            break;
        case SID_INSERT_HFIXEDLINE:
            m_pMyOwnView->SetMode( RPTUI_INSERT );
            m_pMyOwnView->SetInsertObj( OBJ_DLG_HFIXEDLINE );
            createDefaultControl(aArgs);
            InvalidateAll();
            break;
        case SID_INSERT_VFIXEDLINE:
            m_pMyOwnView->SetMode( RPTUI_INSERT );
            m_pMyOwnView->SetInsertObj( OBJ_DLG_VFIXEDLINE );
            createDefaultControl(aArgs);
            InvalidateAll();
            break;
        case SID_FM_EDIT:
            m_pMyOwnView->SetMode( RPTUI_INSERT );
            m_pMyOwnView->SetInsertObj( OBJ_DLG_FORMATTEDFIELD );
            createDefaultControl(aArgs);
            InvalidateAll();
            break;
        case SID_FM_IMAGECONTROL:
            m_pMyOwnView->SetMode( RPTUI_INSERT );
            m_pMyOwnView->SetInsertObj( OBJ_DLG_IMAGECONTROL );
            createDefaultControl(aArgs);
            InvalidateAll();
            break;
        case SID_DRAWTBX_CS_BASIC:
        case SID_DRAWTBX_CS_BASIC1:
        case SID_DRAWTBX_CS_BASIC2:
        case SID_DRAWTBX_CS_BASIC3:
        case SID_DRAWTBX_CS_BASIC4:
        case SID_DRAWTBX_CS_BASIC5:
        case SID_DRAWTBX_CS_BASIC6:
        case SID_DRAWTBX_CS_BASIC7:
        case SID_DRAWTBX_CS_BASIC8:
        case SID_DRAWTBX_CS_BASIC9:
        case SID_DRAWTBX_CS_BASIC10:
        case SID_DRAWTBX_CS_BASIC11:
        case SID_DRAWTBX_CS_BASIC12:
        case SID_DRAWTBX_CS_BASIC13:
        case SID_DRAWTBX_CS_BASIC14:
        case SID_DRAWTBX_CS_BASIC15:
        case SID_DRAWTBX_CS_BASIC16:
        case SID_DRAWTBX_CS_BASIC17:
        case SID_DRAWTBX_CS_BASIC18:
        case SID_DRAWTBX_CS_BASIC19:
        case SID_DRAWTBX_CS_BASIC20:
        case SID_DRAWTBX_CS_BASIC21:
        case SID_DRAWTBX_CS_BASIC22:
        case SID_DRAWTBX_CS_SYMBOL1:
        case SID_DRAWTBX_CS_SYMBOL2:
        case SID_DRAWTBX_CS_SYMBOL3:
        case SID_DRAWTBX_CS_SYMBOL4:
        case SID_DRAWTBX_CS_SYMBOL5:
        case SID_DRAWTBX_CS_SYMBOL6:
        case SID_DRAWTBX_CS_SYMBOL7:
        case SID_DRAWTBX_CS_SYMBOL8:
        case SID_DRAWTBX_CS_SYMBOL9:
        case SID_DRAWTBX_CS_SYMBOL10:
        case SID_DRAWTBX_CS_SYMBOL11:
        case SID_DRAWTBX_CS_SYMBOL12:
        case SID_DRAWTBX_CS_SYMBOL13:
        case SID_DRAWTBX_CS_SYMBOL14:
        case SID_DRAWTBX_CS_SYMBOL15:
        case SID_DRAWTBX_CS_SYMBOL16:
        case SID_DRAWTBX_CS_SYMBOL17:
        case SID_DRAWTBX_CS_SYMBOL18:
        case SID_DRAWTBX_CS_ARROW1:
        case SID_DRAWTBX_CS_ARROW2:
        case SID_DRAWTBX_CS_ARROW3:
        case SID_DRAWTBX_CS_ARROW4:
        case SID_DRAWTBX_CS_ARROW5:
        case SID_DRAWTBX_CS_ARROW6:
        case SID_DRAWTBX_CS_ARROW7:
        case SID_DRAWTBX_CS_ARROW8:
        case SID_DRAWTBX_CS_ARROW9:
        case SID_DRAWTBX_CS_ARROW10:
        case SID_DRAWTBX_CS_ARROW11:
        case SID_DRAWTBX_CS_ARROW12:
        case SID_DRAWTBX_CS_ARROW13:
        case SID_DRAWTBX_CS_ARROW14:
        case SID_DRAWTBX_CS_ARROW15:
        case SID_DRAWTBX_CS_ARROW16:
        case SID_DRAWTBX_CS_ARROW17:
        case SID_DRAWTBX_CS_ARROW18:
        case SID_DRAWTBX_CS_ARROW19:
        case SID_DRAWTBX_CS_ARROW20:
        case SID_DRAWTBX_CS_ARROW21:
        case SID_DRAWTBX_CS_ARROW22:
        case SID_DRAWTBX_CS_ARROW23:
        case SID_DRAWTBX_CS_ARROW24:
        case SID_DRAWTBX_CS_ARROW25:
        case SID_DRAWTBX_CS_ARROW26:
        case SID_DRAWTBX_CS_STAR1:
        case SID_DRAWTBX_CS_STAR2:
        case SID_DRAWTBX_CS_STAR3:
        case SID_DRAWTBX_CS_STAR4:
        case SID_DRAWTBX_CS_STAR5:
        case SID_DRAWTBX_CS_STAR6:
        case SID_DRAWTBX_CS_STAR7:
        case SID_DRAWTBX_CS_STAR8:
        case SID_DRAWTBX_CS_STAR9:
        case SID_DRAWTBX_CS_STAR10:
        case SID_DRAWTBX_CS_STAR11:
        case SID_DRAWTBX_CS_STAR12:
        case SID_DRAWTBX_CS_FLOWCHART1:
        case SID_DRAWTBX_CS_FLOWCHART2:
        case SID_DRAWTBX_CS_FLOWCHART3:
        case SID_DRAWTBX_CS_FLOWCHART4:
        case SID_DRAWTBX_CS_FLOWCHART5:
        case SID_DRAWTBX_CS_FLOWCHART6:
        case SID_DRAWTBX_CS_FLOWCHART7:
        case SID_DRAWTBX_CS_FLOWCHART8:
        case SID_DRAWTBX_CS_FLOWCHART9:
        case SID_DRAWTBX_CS_FLOWCHART10:
        case SID_DRAWTBX_CS_FLOWCHART11:
        case SID_DRAWTBX_CS_FLOWCHART12:
        case SID_DRAWTBX_CS_FLOWCHART13:
        case SID_DRAWTBX_CS_FLOWCHART14:
        case SID_DRAWTBX_CS_FLOWCHART15:
        case SID_DRAWTBX_CS_FLOWCHART16:
        case SID_DRAWTBX_CS_FLOWCHART17:
        case SID_DRAWTBX_CS_FLOWCHART18:
        case SID_DRAWTBX_CS_FLOWCHART19:
        case SID_DRAWTBX_CS_FLOWCHART20:
        case SID_DRAWTBX_CS_FLOWCHART21:
        case SID_DRAWTBX_CS_FLOWCHART22:
        case SID_DRAWTBX_CS_FLOWCHART23:
        case SID_DRAWTBX_CS_FLOWCHART24:
        case SID_DRAWTBX_CS_FLOWCHART25:
        case SID_DRAWTBX_CS_FLOWCHART26:
        case SID_DRAWTBX_CS_FLOWCHART27:
        case SID_DRAWTBX_CS_FLOWCHART28:
        case SID_DRAWTBX_CS_CALLOUT1:
        case SID_DRAWTBX_CS_CALLOUT2:
        case SID_DRAWTBX_CS_CALLOUT3:
        case SID_DRAWTBX_CS_CALLOUT4:
        case SID_DRAWTBX_CS_CALLOUT5:
        case SID_DRAWTBX_CS_CALLOUT6:
        case SID_DRAWTBX_CS_CALLOUT7:
        case SID_DRAWTBX_CS_SYMBOL:
        case SID_DRAWTBX_CS_ARROW:
        case SID_DRAWTBX_CS_FLOWCHART:
        case SID_DRAWTBX_CS_CALLOUT:
        case SID_DRAWTBX_CS_STAR:
            m_pMyOwnView->SetMode( RPTUI_INSERT );
            {
                URL aUrl = getURLForId(_nId);
                sal_Int32 nIndex = 1;
                ::rtl::OUString sType = aUrl.Complete.getToken(0,'.',nIndex);
                if ( nIndex == -1 || !sType.getLength() )
                {
                    switch(_nId)
                    {
                        case SID_DRAWTBX_CS_SYMBOL:
                            sType = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("smiley"));
                            break;
                        case SID_DRAWTBX_CS_ARROW:
                            sType = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("left-right-arrow"));
                            break;
                        case SID_DRAWTBX_CS_FLOWCHART:
                            sType = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("flowchart-internal-storage"));
                            break;
                        case SID_DRAWTBX_CS_CALLOUT:
                            sType = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("round-rectangular-callout"));
                            break;
                        case SID_DRAWTBX_CS_STAR:
                            sType = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("star5"));
                            break;
                        case SID_DRAWTBX_CS_BASIC:
                        default:
                            sType = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("diamond"));
                    }
                }
                else
                    sType = aUrl.Complete.getToken(0,'.',nIndex);

                m_pMyOwnView->SetInsertObj( OBJ_CUSTOMSHAPE ,sType);
                createDefaultControl(aArgs);
            }
            InvalidateAll();
            break;
        case SID_RPT_SHOWREPORTEXPLORER:
            m_pMyOwnView->toggleReportExplorer();
            break;
        case SID_FM_ADD_FIELD:
            m_pMyOwnView->toggleAddField();
            break;
        case SID_SHOW_PROPERTYBROWSER:
            if ( m_bShowProperties )
                m_sLastActivePage = m_pMyOwnView->getCurrentPage();
            else
                m_pMyOwnView->setCurrentPage(m_sLastActivePage);

            m_bShowProperties = !m_bShowProperties;
            if ( aArgs.getLength() == 1 )
                aArgs[0].Value >>= m_bShowProperties;

            m_pMyOwnView->togglePropertyBrowser(m_bShowProperties);
            break;
        case SID_PROPERTYBROWSER_LAST_PAGE: // nothing to do
            m_sLastActivePage = m_pMyOwnView->getCurrentPage();
            break;
        case SID_SPLIT_POSITION:
            m_pMyOwnView->Resize();
            break;
        case SID_PAGEDIALOG:
        case SID_ATTR_CHAR_COLOR_BACKGROUND:
            {
                uno::Reference<report::XSection> xSection;
                if (aArgs.getLength() == 1 )
                    aArgs[0].Value >>= xSection;
                openPageDialog(xSection);
                bForceBroadcast = sal_True;
            }
            break;
        case SID_SORTINGANDGROUPING:
            openSortingAndGroupingDialog();
            m_bGroupFloaterWasVisible = m_pGroupsFloater && m_pGroupsFloater->IsVisible();
            break;
        case SID_BACKGROUND_COLOR:
            {
                uno::Reference< awt::XWindow> xWindow;
                uno::Reference< report::XReportControlFormat> xReportControlFormat = lcl_getReportControlFormat(aArgs,m_pMyOwnView,xWindow);
                uno::Reference< report::XSection > xSection = m_pMyOwnView->getCurrentSection();
                try
                {
                    if ( xReportControlFormat.is() )
                    {
                        util::Color aColor( lcl_extractBackgroundColor( aArgs ) );
                        xReportControlFormat->setControlBackground( aColor );
                    }
                    else if ( xSection.is() )
                    {
                        util::Color aColor( lcl_extractBackgroundColor( aArgs ) );
                        xSection->setBackColor( aColor );
                    }
                    else
                        OSL_ENSURE( false, "OReportController::Execute: no object to apply this background color to!" );
                }
                catch( const Exception& )
                {
                    DBG_UNHANDLED_EXCEPTION();
                }

                bForceBroadcast = sal_True;
            }
            break;
        case SID_ATTR_CHAR_WEIGHT:
        case SID_ATTR_CHAR_POSTURE:
        case SID_ATTR_CHAR_UNDERLINE:
            {
                uno::Reference< awt::XWindow> xWindow;
                uno::Reference< report::XReportControlFormat> xReportControlFormat = lcl_getReportControlFormat(aArgs,m_pMyOwnView,xWindow);
                if ( xReportControlFormat.is() )
                {
                    try
                    {
                        awt::FontDescriptor aFontDescriptor = xReportControlFormat->getFontDescriptor();
                        switch(_nId)
                        {
                            case SID_ATTR_CHAR_WEIGHT:
                                aFontDescriptor.Weight = (awt::FontWeight::NORMAL + awt::FontWeight::BOLD) - aFontDescriptor.Weight;
                                break;
                            case SID_ATTR_CHAR_POSTURE:
                                aFontDescriptor.Slant = static_cast<awt::FontSlant>(static_cast<sal_Int16>(awt::FontSlant_ITALIC) - static_cast<sal_Int16>(aFontDescriptor.Slant));
                                break;
                            case SID_ATTR_CHAR_UNDERLINE:
                                aFontDescriptor.Underline = awt::FontUnderline::SINGLE - aFontDescriptor.Underline;
                                break;
                            default:
                                OSL_ENSURE(0,"Illegal value in default!");
                                break;
                        }

                        xReportControlFormat->setFontDescriptor(aFontDescriptor);
                    }
                    catch(beans::UnknownPropertyException&)
                    {
                    }
                }
            }
            break;
        case SID_ATTR_CHAR_COLOR:
        case SID_ATTR_CHAR_COLOR2:
            {
                uno::Reference< awt::XWindow> xWindow;
                uno::Reference< report::XReportControlFormat> xReportControlFormat = lcl_getReportControlFormat(aArgs,m_pMyOwnView,xWindow);
                if ( xReportControlFormat.is() )
                {
                    SequenceAsHashMap aMap(aArgs);
                    util::Color aColor = aMap.getUnpackedValueOrDefault(PROPERTY_FONTCOLOR,util::Color());
                    try
                    {
                        xReportControlFormat->setCharColor(aColor);
                        bForceBroadcast = sal_True;
                    }
                    catch(beans::UnknownPropertyException&)
                    {
                    }
                }
            }
            break;
        case SID_ATTR_CHAR_FONT:
            if ( aArgs.getLength() == 1 )
            {
                uno::Reference< report::XReportControlModel> xControlModel(m_pMyOwnView->getCurrentControlModel(),uno::UNO_QUERY);
                awt::FontDescriptor aFont;
                if ( xControlModel.is() && ( aArgs[0].Value >>= aFont ) )
                    xControlModel->setCharFontName(aFont.Name);
                    //xControlModel->setFontDescriptor(aFont);
            }
            break;
        case SID_ATTR_CHAR_FONTHEIGHT:
            if ( aArgs.getLength() == 1 )
            {
                uno::Reference< report::XReportControlModel> xControlModel(m_pMyOwnView->getCurrentControlModel(),uno::UNO_QUERY);
                float fSelVal = 0.0;
                if ( xControlModel.is() && ( aArgs[0].Value >>= fSelVal) )
                {
                    xControlModel->setCharHeight(fSelVal);
                }
            }
            break;
        case SID_ATTR_PARA_ADJUST_LEFT:
        case SID_ATTR_PARA_ADJUST_CENTER:
        case SID_ATTR_PARA_ADJUST_RIGHT:
            {
                uno::Reference< report::XReportControlModel> xControlModel(m_pMyOwnView->getCurrentControlModel(),uno::UNO_QUERY);
                if ( xControlModel.is() )
                {
                    switch(_nId)
                    {
                        case SID_ATTR_PARA_ADJUST_LEFT:
                            xControlModel->setParaAdjust(awt::TextAlign::LEFT);
                            break;
                        case SID_ATTR_PARA_ADJUST_CENTER:
                            xControlModel->setParaAdjust(awt::TextAlign::CENTER);
                            break;
                        case SID_ATTR_PARA_ADJUST_RIGHT:
                            xControlModel->setParaAdjust(awt::TextAlign::RIGHT);
                            break;
                    }
                }
                InvalidateFeature(SID_ATTR_PARA_ADJUST_LEFT);
                InvalidateFeature(SID_ATTR_PARA_ADJUST_CENTER);
                InvalidateFeature(SID_ATTR_PARA_ADJUST_RIGHT);
            }
            break;
        case SID_CHAR_DLG:
            {
                uno::Reference< awt::XWindow> xWindow;
                uno::Reference< report::XReportControlFormat> xReportControlFormat = lcl_getReportControlFormat(aArgs,m_pMyOwnView,xWindow);
                if ( xReportControlFormat.is() )
                {
                    {
                        const String sUndoAction(ModuleRes(RID_STR_UNDO_CHANGEFONT));
                        UndoManagerListAction aListAction(m_aUndoManager,sUndoAction);
                        rptui::openCharDialog(xReportControlFormat,xWindow);
                    }
                    InvalidateAll();
                }
            }
            break;
        //case SID_FM_DESIGN_MODE:
  //          break;
        case SID_INSERT_GRAPHIC:
            insertGraphic();
            break;
        case SID_SETCONTROLDEFAULTS:
            break;
        case SID_CONDITIONALFORMATTING:
            {
                uno::Reference< report::XFormattedField> xFormattedField(m_pMyOwnView->getCurrentControlModel(),uno::UNO_QUERY);
                if ( xFormattedField.is() )
                {
                    ConditionalFormattingDialog aDlg( getView(), xFormattedField.get(), *this );
                    aDlg.Execute();
                }
            }
            break;
        case SID_DATETIME:
            if ( m_xReportDefinition.is() )
            {
                if ( !aArgs.getLength() )
                {
                    ODateTimeDialog aDlg(getView(),m_pMyOwnView->getCurrentSection(),this);
                    aDlg.Execute();
                }
                else
                    createDateTime(aArgs);
            } // if ( m_xReportDefinition.is() )
            break;
        case SID_INSERT_FLD_PGNUMBER:
            if ( m_xReportDefinition.is() )
            {
                if ( !aArgs.getLength() )
                {
                    OPageNumberDialog aDlg(getView(),m_xReportDefinition,this);
                    aDlg.Execute();
                }
                else
                    createPageNumber(aArgs);
            } // if ( m_xReportDefinition.is() )
            break;
        case SID_EXPORTDOC:
        case SID_EXPORTDOCASPDF:
            break;
        case SID_PRINTPREVIEW:
            if ( m_xReportDefinition.is() )
            {
            }
            break;
        case SID_EDITDOC:
            if(isEditable())
            { // the state should be changed to not editable
                switch (saveModified())
                {
                    case RET_CANCEL:
                        // don't change anything here so return
                        return;
                    case RET_NO:
                        setModified(sal_False);     // and we are not modified yet
                        break;
                    default:
                        break;
                }
            }
            setEditable(!isEditable());
            //getJoinView()->setReadOnly(!isEditable());
            InvalidateAll();
            return;
        case SID_GROUP:
    //        {
    //            ::boost::shared_ptr<OReportSection> pReportSection = m_pMyOwnView->getMarkedSection();
                //if ( pReportSection.get() )
                //{
    //                ::std::vector< uno::Reference< report::XReportComponent > > aSelection;
    //                uno::Reference<report::XSection> xSection = pReportSection->getSection();
    //                const String sUndoAction(ModuleRes(RID_STR_UNDO_GROUP));
    //                getUndoMgr()->EnterListAction( sUndoAction, String() );

    //                getUndoMgr()->LeaveListAction();
    //            }
    //        }
            break;
        default:
            OReportController_BASE::Execute(_nId,aArgs);
    }
    InvalidateFeature(_nId,Reference< XStatusListener >(),bForceBroadcast);
}
// -----------------------------------------------------------------------------
short OReportController::saveModified()
{
    return RET_NO;
}
// -----------------------------------------------------------------------------
void OReportController::impl_initialize( )
{
    OReportController_BASE::impl_initialize();

    const ::comphelper::NamedValueCollection& rArguments( getInitParams() );

    rArguments.get_ensureType( (::rtl::OUString)PROPERTY_REPORTNAME, m_sName );
    if ( !m_sName.getLength() )
        rArguments.get_ensureType( "DocumentTitle", m_sName );

    try
    {
        if ( m_xReportDefinition.is() )
        {
            //m_sName = m_xReportDefinition->getName();
            getView()->initialize();    // show the windows and fill with our informations
            getUndoMgr()->Clear();      // clear all undo redo things
            getSdrModel();
            if ( !m_aReportModel )
                throw Exception();

            listen(true);
            setEditable( !m_aReportModel->IsReadOnly() );
            m_xFormatter.set(getORB()->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.util.NumberFormatter"))), UNO_QUERY);
            m_xFormatter->attachNumberFormatsSupplier(Reference< XNumberFormatsSupplier>(m_xReportDefinition,uno::UNO_QUERY));


        } // if ( m_xReportDefinition.is() )

        // check if chart is supported by the engine
        checkChartEnabled();
        // restore the view data
        m_pMyOwnView->toggleGrid(m_bGridVisible);
        m_pMyOwnView->showRuler(m_bShowRuler);
        m_pMyOwnView->togglePropertyBrowser(m_bShowProperties);
        m_pMyOwnView->setCurrentPage(m_sLastActivePage);
        m_pMyOwnView->unmarkAllObjects(NULL);

        if ( m_nPageNum != -1 )
        {
            if ( m_nPageNum < m_aReportModel->GetPageCount() )
            {
                const OReportPage* pPage = dynamic_cast<OReportPage*>(m_aReportModel->GetPage(static_cast<sal_uInt16>(m_nPageNum)));
                if ( pPage )
                {
                    uno::Sequence< beans::PropertyValue> aArgs(1);
                    aArgs[0].Value <<= pPage->getSection();
                    executeUnChecked(SID_SELECT,aArgs);
                }
            }
            else
                m_nPageNum = -1;
        }
        m_pMyOwnView->collapseSections(m_aCollapsedSections);
        m_pMyOwnView->Invalidate();

        if ( m_bShowProperties && m_nPageNum == -1 )
        {
            m_sLastActivePage = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Data"));
            m_pMyOwnView->setCurrentPage(m_sLastActivePage);
            uno::Sequence< beans::PropertyValue> aArgs;
            executeUnChecked(SID_SELECT_REPORT,aArgs);
        }

        setModified(sal_False);     // and we are not modified yet

        // open the global help agent
        // we need a Frame but at this time there is no frame, therefore we send a UserEvent
        getView()->PostUserEvent(LINK(this, OReportController, OnOpenHelpAgent));
    }
    catch(const SQLException&)
    {
        OSL_ENSURE(sal_False, "OReportController::initialize: caught an exception!");
    }
}
// -----------------------------------------------------------------------------
IMPL_LINK( OReportController, OnOpenHelpAgent, void* ,/*_pMemfun*/)
{
    //m_nExecuteReportEvent = 0;
    doOpenHelpAgent();
    return 0L;
}
// -----------------------------------------------------------------------------

void OReportController::doOpenHelpAgent()
{
    if (getFrame().is())
    {
        rtl::OUString suURL = rtl::OUString::createFromAscii("vnd.sun.star.help://shared/text/shared/explorer/database/rep_main.xhp?UseDB=no&DbPAR=swriter");
        openHelpAgent(suURL);
        // openHelpAgent(68245 /* HID_REPORT_DESIGN... UNKNOWN */ );
        // HID_APP_REPORT_TREE
    }
    else
    {
        // as long as we don't get a Frame, we send the user event again.
        getView()->PostUserEvent(LINK(this, OReportController, OnOpenHelpAgent));
    }
}
// -----------------------------------------------------------------------------
sal_Bool OReportController::Construct(Window* pParent)
{
    m_pMyOwnView = new ODesignView(pParent,getORB(),this);
    StartListening(*(m_pMyOwnView));
    m_pView = m_pMyOwnView;

    // now that we have a view we can create the clipboard listener
    m_aSystemClipboard = TransferableDataHelper::CreateFromSystemClipboard( getView() );
    m_aSystemClipboard.StartClipboardListening( );
    m_pClipbordNotifier = new TransferableClipboardListener( LINK( this, OReportController, OnClipboardChanged ) );
    m_pClipbordNotifier->acquire();
    m_pClipbordNotifier->AddRemoveListener( getView(), sal_True );

    OReportController_BASE::Construct(pParent);
    //getView()->Show();
    return sal_True;
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OReportController::suspend(sal_Bool /*_bSuspend*/) throw( RuntimeException )
{
    if ( getBroadcastHelper().bInDispose || getBroadcastHelper().bDisposed )
        return sal_True;

    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard aGuard(m_aMutex);

    if ( getView() && getView()->IsInModalMode() )
        return sal_False;

    // this suspend will be handled in the DBAccess interceptor implementation
    return sal_True;
}
// -----------------------------------------------------------------------------
void OReportController::describeSupportedFeatures()
{
    OSingleDocumentController::describeSupportedFeatures();

    implDescribeSupportedFeature( ".uno:TextDocument",              SID_RPT_TEXTDOCUMENT,           CommandGroup::APPLICATION );
    implDescribeSupportedFeature( ".uno:Spreadsheet",               SID_RPT_SPREADSHEET,            CommandGroup::APPLICATION );

    implDescribeSupportedFeature( ".uno:Redo",                      SID_REDO,                       CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:Undo",                      SID_UNDO,                       CommandGroup::EDIT );
    //implDescribeSupportedFeature( ".uno:PasteSpecial",              SID_PASTE,                    CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:SelectAll",                 SID_SELECTALL,                  CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:SelectAllInSection",        SID_SELECTALL_IN_SECTION,       CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:Delete",                    SID_DELETE,                     CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:SelectReport",              SID_SELECT_REPORT,              CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:ExecuteReport",             SID_EXECUTE_REPORT,             CommandGroup::EDIT );

    implDescribeSupportedFeature( ".uno:GridVisible",               SID_GRID_VISIBLE,               CommandGroup::VIEW );
    implDescribeSupportedFeature( ".uno:GridUse",                   SID_GRID_USE,                   CommandGroup::VIEW );
    implDescribeSupportedFeature( ".uno:HelplinesMove",             SID_HELPLINES_MOVE,             CommandGroup::VIEW );
    implDescribeSupportedFeature( ".uno:ShowRuler",                 SID_RULER,                      CommandGroup::VIEW );
    implDescribeSupportedFeature( ".uno:AddField",                  SID_FM_ADD_FIELD,               CommandGroup::VIEW );
    implDescribeSupportedFeature( ".uno:ReportNavigator",           SID_RPT_SHOWREPORTEXPLORER,     CommandGroup::VIEW );
    implDescribeSupportedFeature( ".uno:ControlProperties",         SID_SHOW_PROPERTYBROWSER,       CommandGroup::VIEW );
    //implDescribeSupportedFeature( ".uno:SwitchControlDesignMode", SID_FM_DESIGN_MODE,             CommandGroup::VIEW );
    implDescribeSupportedFeature( ".uno:DbSortingAndGrouping",      SID_SORTINGANDGROUPING,         CommandGroup::VIEW );
    implDescribeSupportedFeature( ".uno:PageHeaderFooter",          SID_PAGEHEADERFOOTER,           CommandGroup::VIEW );
    implDescribeSupportedFeature( ".uno:ReportHeaderFooter",        SID_REPORTHEADERFOOTER,         CommandGroup::VIEW );
    //implDescribeSupportedFeature( ".uno:SwitchControlDesignMode", SID_FM_DESIGN_MODE,             CommandGroup::VIEW );

    implDescribeSupportedFeature( ".uno:ConditionalFormatting",     SID_CONDITIONALFORMATTING,      CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:PageDialog",                SID_PAGEDIALOG,                 CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:ResetAttributes",           SID_SETCONTROLDEFAULTS,         CommandGroup::FORMAT );

    implDescribeSupportedFeature( ".uno:Bold",                      SID_ATTR_CHAR_WEIGHT,           CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:Italic",                    SID_ATTR_CHAR_POSTURE,          CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:Underline",                 SID_ATTR_CHAR_UNDERLINE,        CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:BackColor",                 SID_ATTR_CHAR_COLOR_BACKGROUND, CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:BackgroundColor",           SID_BACKGROUND_COLOR,           CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:Color",                     SID_ATTR_CHAR_COLOR);
    implDescribeSupportedFeature( ".uno:FontColor",                 SID_ATTR_CHAR_COLOR2,           CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:FontDialog",                SID_CHAR_DLG,                   CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:LeftPara",                  SID_ATTR_PARA_ADJUST_LEFT,      CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:CenterPara",                SID_ATTR_PARA_ADJUST_CENTER,    CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:RightPara",                 SID_ATTR_PARA_ADJUST_RIGHT,     CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:FontHeight",                SID_ATTR_CHAR_FONTHEIGHT,       CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:CharFontName",              SID_ATTR_CHAR_FONT,             CommandGroup::FORMAT );

    implDescribeSupportedFeature( ".uno:ArrangeMenu",               SID_ARRANGEMENU,                CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:BringToFront",              SID_FRAME_TO_TOP,               CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:ObjectBackOne",             SID_FRAME_DOWN,                 CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:ObjectForwardOne",          SID_FRAME_UP,                   CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:SendToBack",                SID_FRAME_TO_BOTTOM,            CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:SetObjectToForeground",     SID_OBJECT_HEAVEN,              CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:SetObjectToBackground",     SID_OBJECT_HELL,                CommandGroup::FORMAT );

    implDescribeSupportedFeature( ".uno:ObjectAlign",               SID_OBJECT_ALIGN,               CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:ObjectAlignLeft",           SID_OBJECT_ALIGN_LEFT,          CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:AlignCenter",               SID_OBJECT_ALIGN_CENTER,        CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:ObjectAlignRight",          SID_OBJECT_ALIGN_RIGHT,         CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:AlignUp",                   SID_OBJECT_ALIGN_UP,            CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:AlignMiddle",               SID_OBJECT_ALIGN_MIDDLE,        CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:AlignDown",                 SID_OBJECT_ALIGN_DOWN,          CommandGroup::FORMAT );

    implDescribeSupportedFeature( ".uno:SectionAlign",              SID_SECTION_ALIGN,              CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:SectionAlignLeft",          SID_SECTION_ALIGN_LEFT,         CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:SectionAlignCenter",        SID_SECTION_ALIGN_CENTER,       CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:SectionAlignRight",         SID_SECTION_ALIGN_RIGHT,        CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:SectionAlignTop",           SID_SECTION_ALIGN_UP,           CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:SectionAlignMiddle",        SID_SECTION_ALIGN_MIDDLE,       CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:SectionAlignBottom",        SID_SECTION_ALIGN_DOWN,         CommandGroup::FORMAT );

    implDescribeSupportedFeature( ".uno:ObjectResize",              SID_OBJECT_RESIZING,            CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:SmallestWidth",             SID_OBJECT_SMALLESTWIDTH,       CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:SmallestHeight",            SID_OBJECT_SMALLESTHEIGHT,      CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:GreatestWidth",             SID_OBJECT_GREATESTWIDTH,       CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:GreatestHeight",            SID_OBJECT_GREATESTHEIGHT,      CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:Distribution",              SID_DISTRIBUTION,               CommandGroup::FORMAT );

    implDescribeSupportedFeature( ".uno:HelpMenu",                  SID_HELPMENU,                   CommandGroup::APPLICATION );
    implDescribeSupportedFeature( ".uno:ExportTo",                  SID_EXPORTDOC,                  CommandGroup::APPLICATION );
    implDescribeSupportedFeature( ".uno:ExportToPDF",               SID_EXPORTDOCASPDF,             CommandGroup::APPLICATION );
    implDescribeSupportedFeature( ".uno:PrintPreview",              SID_PRINTPREVIEW,               CommandGroup::APPLICATION );

    implDescribeSupportedFeature( ".uno:NewDoc",                    SID_NEWDOC,                     CommandGroup::DOCUMENT );
    implDescribeSupportedFeature( ".uno:Save",                      SID_SAVEDOC,                    CommandGroup::DOCUMENT );
    implDescribeSupportedFeature( ".uno:SaveAs",                    SID_SAVEASDOC,                  CommandGroup::DOCUMENT );

    implDescribeSupportedFeature( ".uno:InsertPageNumberField",     SID_INSERT_FLD_PGNUMBER,        CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:InsertDateTimeField",       SID_DATETIME,                   CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:InsertObjectChart",         SID_INSERT_DIAGRAM,             CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:InsertGraphic",             SID_INSERT_GRAPHIC,             CommandGroup::INSERT );
    // controls
    implDescribeSupportedFeature( ".uno:SelectObject",              SID_OBJECT_SELECT,              CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:Label",                     SID_FM_FIXEDTEXT,               CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:Edit",                      SID_FM_EDIT,                    CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:ImageControl",              SID_FM_IMAGECONTROL,            CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:HFixedLine",                SID_INSERT_HFIXEDLINE,          CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:VFixedLine",                SID_INSERT_VFIXEDLINE,          CommandGroup::INSERT );

    // shapes
    implDescribeSupportedFeature( ".uno:BasicShapes",               SID_DRAWTBX_CS_BASIC,           CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:BasicShapes.rectangle",     SID_DRAWTBX_CS_BASIC1,          CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:BasicShapes.round-rectangle",SID_DRAWTBX_CS_BASIC2,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:BasicShapes.quadrat",       SID_DRAWTBX_CS_BASIC3,          CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:BasicShapes.round-quadrat", SID_DRAWTBX_CS_BASIC4,          CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:BasicShapes.circle",        SID_DRAWTBX_CS_BASIC5,          CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:BasicShapes.ellipse",       SID_DRAWTBX_CS_BASIC6,          CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:BasicShapes.circle-pie",    SID_DRAWTBX_CS_BASIC7,          CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:BasicShapes.isosceles-triangle",SID_DRAWTBX_CS_BASIC8,      CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:BasicShapes.right-triangle",SID_DRAWTBX_CS_BASIC9,          CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:BasicShapes.trapezoid",     SID_DRAWTBX_CS_BASIC10,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:BasicShapes.diamond",       SID_DRAWTBX_CS_BASIC11,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:BasicShapes.parallelogram", SID_DRAWTBX_CS_BASIC12,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:BasicShapes.pentagon",      SID_DRAWTBX_CS_BASIC13,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:BasicShapes.hexagon",       SID_DRAWTBX_CS_BASIC14,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:BasicShapes.octagon",       SID_DRAWTBX_CS_BASIC15,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:BasicShapes.cross",         SID_DRAWTBX_CS_BASIC16,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:BasicShapes.ring",          SID_DRAWTBX_CS_BASIC17,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:BasicShapes.block-arc",     SID_DRAWTBX_CS_BASIC18,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:BasicShapes.can",           SID_DRAWTBX_CS_BASIC19,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:BasicShapes.cube",          SID_DRAWTBX_CS_BASIC20,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:BasicShapes.paper",         SID_DRAWTBX_CS_BASIC21,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:BasicShapes.frame",         SID_DRAWTBX_CS_BASIC22,         CommandGroup::INSERT );

    implDescribeSupportedFeature( ".uno:SymbolShapes",              SID_DRAWTBX_CS_SYMBOL,          CommandGroup::INSERT );

    implDescribeSupportedFeature( ".uno:SymbolShapes.smiley" ,      SID_DRAWTBX_CS_SYMBOL1,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:SymbolShapes.sun" ,         SID_DRAWTBX_CS_SYMBOL2,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:SymbolShapes.moon" ,        SID_DRAWTBX_CS_SYMBOL3,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:SymbolShapes.lightning" ,   SID_DRAWTBX_CS_SYMBOL4,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:SymbolShapes.heart" ,       SID_DRAWTBX_CS_SYMBOL5,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:SymbolShapes.flower" ,      SID_DRAWTBX_CS_SYMBOL6,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:SymbolShapes.cloud" ,       SID_DRAWTBX_CS_SYMBOL7,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:SymbolShapes.forbidden" ,   SID_DRAWTBX_CS_SYMBOL8,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:SymbolShapes.puzzle" ,      SID_DRAWTBX_CS_SYMBOL9,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:SymbolShapes.bracket-pair" ,SID_DRAWTBX_CS_SYMBOL10,        CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:SymbolShapes.left-bracket" ,SID_DRAWTBX_CS_SYMBOL11,        CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:SymbolShapes.right-bracket",SID_DRAWTBX_CS_SYMBOL12,        CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:SymbolShapes.brace-pair" ,  SID_DRAWTBX_CS_SYMBOL13,        CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:SymbolShapes.left-brace" ,  SID_DRAWTBX_CS_SYMBOL14,        CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:SymbolShapes.right-brace" , SID_DRAWTBX_CS_SYMBOL15,        CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:SymbolShapes.quad-bevel" ,  SID_DRAWTBX_CS_SYMBOL16,        CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:SymbolShapes.octagon-bevel",SID_DRAWTBX_CS_SYMBOL17,        CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:SymbolShapes.diamond-bevel",SID_DRAWTBX_CS_SYMBOL18,        CommandGroup::INSERT );

    implDescribeSupportedFeature( ".uno:ArrowShapes.left-arrow" ,           SID_DRAWTBX_CS_ARROW1,  CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:ArrowShapes.right-arrow" ,          SID_DRAWTBX_CS_ARROW2,  CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:ArrowShapes.up-arrow" ,             SID_DRAWTBX_CS_ARROW3,  CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:ArrowShapes.down-arrow" ,           SID_DRAWTBX_CS_ARROW4,  CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:ArrowShapes.left-right-arrow" ,     SID_DRAWTBX_CS_ARROW5,  CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:ArrowShapes.up-down-arrow" ,        SID_DRAWTBX_CS_ARROW6,  CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:ArrowShapes.up-right-arrow" ,       SID_DRAWTBX_CS_ARROW7,  CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:ArrowShapes.up-right-down-arrow" ,  SID_DRAWTBX_CS_ARROW8,  CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:ArrowShapes.quad-arrow" ,           SID_DRAWTBX_CS_ARROW9,  CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:ArrowShapes.corner-right-arrow" ,   SID_DRAWTBX_CS_ARROW10, CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:ArrowShapes.split-arrow" ,          SID_DRAWTBX_CS_ARROW11, CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:ArrowShapes.striped-right-arrow" ,  SID_DRAWTBX_CS_ARROW12, CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:ArrowShapes.notched-right-arrow" ,  SID_DRAWTBX_CS_ARROW13, CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:ArrowShapes.pentagon-right" ,       SID_DRAWTBX_CS_ARROW14, CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:ArrowShapes.chevron" ,              SID_DRAWTBX_CS_ARROW15, CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:ArrowShapes.right-arrow-callout" ,  SID_DRAWTBX_CS_ARROW16, CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:ArrowShapes.left-arrow-callout" ,   SID_DRAWTBX_CS_ARROW17, CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:ArrowShapes.up-arrow-callout" ,     SID_DRAWTBX_CS_ARROW18, CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:ArrowShapes.down-arrow-callout" ,   SID_DRAWTBX_CS_ARROW19, CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:ArrowShapes.left-right-arrow-callout",SID_DRAWTBX_CS_ARROW20,       CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:ArrowShapes.up-down-arrow-callout" ,SID_DRAWTBX_CS_ARROW21, CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:ArrowShapes.up-right-arrow-callout",SID_DRAWTBX_CS_ARROW22, CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:ArrowShapes.quad-arrow-callout" ,   SID_DRAWTBX_CS_ARROW23, CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:ArrowShapes.circular-arrow" ,       SID_DRAWTBX_CS_ARROW24, CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:ArrowShapes.split-round-arrow" ,    SID_DRAWTBX_CS_ARROW25, CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:ArrowShapes.s-sharped-arrow" ,      SID_DRAWTBX_CS_ARROW26, CommandGroup::INSERT );

    implDescribeSupportedFeature( ".uno:StarShapes.bang" ,                  SID_DRAWTBX_CS_STAR1,           CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:StarShapes.star4" ,                 SID_DRAWTBX_CS_STAR2,           CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:StarShapes.star5" ,                 SID_DRAWTBX_CS_STAR3,           CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:StarShapes.star6" ,                 SID_DRAWTBX_CS_STAR4,           CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:StarShapes.star8" ,                 SID_DRAWTBX_CS_STAR5,           CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:StarShapes.star12" ,                SID_DRAWTBX_CS_STAR6,           CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:StarShapes.star24" ,                SID_DRAWTBX_CS_STAR7,           CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:StarShapes.concave-star6" ,         SID_DRAWTBX_CS_STAR8,           CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:StarShapes.vertical-scroll" ,       SID_DRAWTBX_CS_STAR9,           CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:StarShapes.horizontal-scroll" ,     SID_DRAWTBX_CS_STAR10,          CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:StarShapes.signet" ,                SID_DRAWTBX_CS_STAR11,          CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:StarShapes.doorplate" ,             SID_DRAWTBX_CS_STAR12,          CommandGroup::INSERT );

    implDescribeSupportedFeature( ".uno:FlowChartShapes.flowchart-process" ,            SID_DRAWTBX_CS_FLOWCHART1,          CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:FlowChartShapes.flowchart-alternate-process" ,  SID_DRAWTBX_CS_FLOWCHART2,          CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:FlowChartShapes.flowchart-decision" ,           SID_DRAWTBX_CS_FLOWCHART3,          CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:FlowChartShapes.flowchart-data" ,               SID_DRAWTBX_CS_FLOWCHART4,          CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:FlowChartShapes.flowchart-predefined-process" , SID_DRAWTBX_CS_FLOWCHART5,          CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:FlowChartShapes.flowchart-internal-storage" ,   SID_DRAWTBX_CS_FLOWCHART6,          CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:FlowChartShapes.flowchart-document" ,           SID_DRAWTBX_CS_FLOWCHART7,          CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:FlowChartShapes.flowchart-multidocument" ,      SID_DRAWTBX_CS_FLOWCHART8,          CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:FlowChartShapes.flowchart-terminator" ,         SID_DRAWTBX_CS_FLOWCHART9,          CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:FlowChartShapes.flowchart-preparation" ,        SID_DRAWTBX_CS_FLOWCHART10,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:FlowChartShapes.flowchart-manual-input" ,       SID_DRAWTBX_CS_FLOWCHART11,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:FlowChartShapes.flowchart-manual-operation" ,   SID_DRAWTBX_CS_FLOWCHART12,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:FlowChartShapes.flowchart-connector" ,          SID_DRAWTBX_CS_FLOWCHART13,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:FlowChartShapes.flowchart-off-page-connector" , SID_DRAWTBX_CS_FLOWCHART14,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:FlowChartShapes.flowchart-card" ,               SID_DRAWTBX_CS_FLOWCHART15,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:FlowChartShapes.flowchart-punched-tape" ,       SID_DRAWTBX_CS_FLOWCHART16,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:FlowChartShapes.flowchart-summing-junction" ,   SID_DRAWTBX_CS_FLOWCHART17,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:FlowChartShapes.flowchart-or" ,                 SID_DRAWTBX_CS_FLOWCHART18,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:FlowChartShapes.flowchart-collate" ,            SID_DRAWTBX_CS_FLOWCHART19,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:FlowChartShapes.flowchart-sort" ,               SID_DRAWTBX_CS_FLOWCHART20,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:FlowChartShapes.flowchart-extract" ,            SID_DRAWTBX_CS_FLOWCHART21,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:FlowChartShapes.flowchart-merge" ,              SID_DRAWTBX_CS_FLOWCHART22,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:FlowChartShapes.flowchart-stored-data" ,        SID_DRAWTBX_CS_FLOWCHART23,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:FlowChartShapes.flowchart-delay" ,              SID_DRAWTBX_CS_FLOWCHART24,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:FlowChartShapes.flowchart-sequential-access" ,  SID_DRAWTBX_CS_FLOWCHART25,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:FlowChartShapes.flowchart-magnetic-disk" ,      SID_DRAWTBX_CS_FLOWCHART26,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:FlowChartShapes.flowchart-direct-access-storage",SID_DRAWTBX_CS_FLOWCHART27,        CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:FlowChartShapes.flowchart-display" ,            SID_DRAWTBX_CS_FLOWCHART28,         CommandGroup::INSERT );

    implDescribeSupportedFeature( ".uno:CalloutShapes.rectangular-callout" ,        SID_DRAWTBX_CS_CALLOUT1,            CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:CalloutShapes.round-rectangular-callout" ,  SID_DRAWTBX_CS_CALLOUT2,            CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:CalloutShapes.round-callout" ,              SID_DRAWTBX_CS_CALLOUT3,            CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:CalloutShapes.cloud-callout" ,              SID_DRAWTBX_CS_CALLOUT4,            CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:CalloutShapes.line-callout-1" ,             SID_DRAWTBX_CS_CALLOUT5,            CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:CalloutShapes.line-callout-2" ,             SID_DRAWTBX_CS_CALLOUT6,            CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:CalloutShapes.line-callout-3" ,             SID_DRAWTBX_CS_CALLOUT7,            CommandGroup::INSERT );

    implDescribeSupportedFeature( ".uno:ArrowShapes",               SID_DRAWTBX_CS_ARROW,           CommandGroup::INSERT );

    implDescribeSupportedFeature( ".uno:FlowChartShapes",           SID_DRAWTBX_CS_FLOWCHART,       CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:CalloutShapes",             SID_DRAWTBX_CS_CALLOUT,         CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:StarShapes",                SID_DRAWTBX_CS_STAR,            CommandGroup::INSERT );


    // keys
    implDescribeSupportedFeature( ".uno:Escape",                    SID_ESCAPE,                     CommandGroup::CONTROLS);


    // internal one
    implDescribeSupportedFeature( ".uno:RPT_RPTHEADER_UNDO",            SID_REPORTHEADER_WITHOUT_UNDO);
    implDescribeSupportedFeature( ".uno:RPT_RPTFOOTER_UNDO",            SID_REPORTFOOTER_WITHOUT_UNDO);
    implDescribeSupportedFeature( ".uno:RPT_PGHEADER_UNDO",             SID_PAGEHEADER_WITHOUT_UNDO);
    implDescribeSupportedFeature( ".uno:RPT_PGFOOTER_UNDO",             SID_PAGEFOOTER_WITHOUT_UNDO);
    implDescribeSupportedFeature( ".uno:DBBackgroundColor",             SID_ATTR_CHAR_COLOR_BACKGROUND);
    implDescribeSupportedFeature( ".uno:SID_GROUPHEADER",               SID_GROUPHEADER);
    implDescribeSupportedFeature( ".uno:SID_GROUPHEADER_WITHOUT_UNDO",  SID_GROUPHEADER_WITHOUT_UNDO);
    implDescribeSupportedFeature( ".uno:SID_GROUPFOOTER",               SID_GROUPFOOTER);
    implDescribeSupportedFeature( ".uno:SID_GROUPFOOTER_WITHOUT_UNDO",  SID_GROUPFOOTER_WITHOUT_UNDO);
    implDescribeSupportedFeature( ".uno:SID_GROUP_REMOVE",              SID_GROUP_REMOVE);
    implDescribeSupportedFeature( ".uno:SID_GROUP_APPEND",              SID_GROUP_APPEND);
    implDescribeSupportedFeature( ".uno:SID_ADD_CONTROL_PAIR",          SID_ADD_CONTROL_PAIR);
    implDescribeSupportedFeature( ".uno:SplitPosition",                 SID_SPLIT_POSITION);
    implDescribeSupportedFeature( ".uno:LastPropertyBrowserPage",       SID_PROPERTYBROWSER_LAST_PAGE);
    implDescribeSupportedFeature( ".uno:Select",                        SID_SELECT);
    implDescribeSupportedFeature( ".uno:InsertFunction",                SID_RPT_NEW_FUNCTION);
    implDescribeSupportedFeature( ".uno:NextMark",                      SID_NEXT_MARK);
    implDescribeSupportedFeature( ".uno:PrevMark",                      SID_PREV_MARK);
}
// -----------------------------------------------------------------------------
SfxUndoManager* OReportController::getUndoMgr()
{
    return &m_aUndoManager;
}
// -----------------------------------------------------------------------------
void OReportController::setModified(sal_Bool _bModified)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    try
    {
        if ( m_xReportDefinition.is() )
            m_xReportDefinition->setModified(_bModified);
        OSingleDocumentController::setModified(_bModified);
    }
    catch(uno::Exception)
    {}
}
// -----------------------------------------------------------------------------
void OReportController::losingConnection( )
{
    // let the base class do it's reconnect
    OReportController_BASE::losingConnection( );

    InvalidateAll();
}
// -----------------------------------------------------------------------------
void OReportController::onLoadedMenu(const Reference< frame::XLayoutManager >& _xLayoutManager)
{
    if ( _xLayoutManager.is() )
    {
        static const ::rtl::OUString s_sMenu[] = {
             ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("private:resource/statusbar/statusbar"))
            ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("private:resource/toolbar/reportcontrols"))
            ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("private:resource/toolbar/drawbar"))
            ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("private:resource/toolbar/Formatting"))
            ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("private:resource/toolbar/alignmentbar"))
            ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("private:resource/toolbar/sectionalignmentbar"))
            ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("private:resource/toolbar/resizebar"))
        };
        for (size_t i = 0; i< sizeof(s_sMenu)/sizeof(s_sMenu[0]); ++i)
        {
            _xLayoutManager->createElement( s_sMenu[i] );
            _xLayoutManager->requestElement( s_sMenu[i] );
        }
    } // if ( _xLayoutManager.is() )
}
// -----------------------------------------------------------------------------
void OReportController::notifyGroupSections(const ContainerEvent& _rEvent,bool _bShow)
{
    uno::Reference< report::XGroup> xGroup(_rEvent.Element,uno::UNO_QUERY);
    if ( xGroup.is() )
    {
        ::vos::OGuard aSolarGuard(Application::GetSolarMutex());
        ::osl::MutexGuard aGuard(m_aMutex);
        sal_Int32 nGroupPos = 0;
        _rEvent.Accessor >>= nGroupPos;

        if ( _bShow )
        {
            xGroup->addPropertyChangeListener(PROPERTY_HEADERON, static_cast<XPropertyChangeListener*>(this));
            xGroup->addPropertyChangeListener(PROPERTY_FOOTERON, static_cast<XPropertyChangeListener*>(this));
        }
        else
        {
            xGroup->removePropertyChangeListener(PROPERTY_HEADERON, static_cast<XPropertyChangeListener*>(this));
            xGroup->removePropertyChangeListener(PROPERTY_FOOTERON, static_cast<XPropertyChangeListener*>(this));
        }

        if ( xGroup->getHeaderOn() )
        {
            groupChange(xGroup,PROPERTY_HEADERON,nGroupPos,_bShow);
        }
        if ( xGroup->getFooterOn() )
        {
            groupChange(xGroup,PROPERTY_FOOTERON,nGroupPos,_bShow);
        }
    }
}
// -----------------------------------------------------------------------------
// ::container::XContainerListener
void SAL_CALL OReportController::elementInserted( const ContainerEvent& _rEvent ) throw(RuntimeException)
{
    notifyGroupSections(_rEvent,true);
}
// -----------------------------------------------------------------------------
void SAL_CALL OReportController::elementRemoved( const ContainerEvent& _rEvent ) throw(RuntimeException)
{
    notifyGroupSections(_rEvent,false);
}
// -----------------------------------------------------------------------------
void SAL_CALL OReportController::elementReplaced( const ContainerEvent& /*_rEvent*/ ) throw(RuntimeException)
{
    ::vos::OGuard aSolarGuard(Application::GetSolarMutex());
    ::osl::MutexGuard aGuard(m_aMutex);
    OSL_ENSURE(0,"Not yet implemented!");
}
// -----------------------------------------------------------------------------
void SAL_CALL OReportController::propertyChange( const beans::PropertyChangeEvent& evt ) throw (RuntimeException)
{
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard aGuard(m_aMutex);
    try
    {
        sal_Bool bShow = sal_False;
        evt.NewValue >>= bShow;
        if ( evt.Source == m_xReportDefinition )
        {
            if ( evt.PropertyName.equals( PROPERTY_REPORTHEADERON ) )
            {
                const USHORT nPosition = m_xReportDefinition->getPageHeaderOn() ? 1 : 0;
                if ( bShow )
                    m_pMyOwnView->addSection(m_xReportDefinition->getReportHeader(),DBREPORTHEADER,nPosition);
                else
                    m_pMyOwnView->removeSection(nPosition);
            }
            else if ( evt.PropertyName.equals( PROPERTY_REPORTFOOTERON ) )
            {
                USHORT nPosition = m_pMyOwnView->getSectionCount();
                if ( m_xReportDefinition->getPageFooterOn() )
                    --nPosition;
                if ( bShow )
                    m_pMyOwnView->addSection(m_xReportDefinition->getReportFooter(),DBREPORTFOOTER,nPosition);
                else
                    m_pMyOwnView->removeSection(nPosition - 1);
            }
            else if ( evt.PropertyName.equals( PROPERTY_PAGEHEADERON ) )
            {
                if ( bShow )
                    m_pMyOwnView->addSection(m_xReportDefinition->getPageHeader(),DBPAGEHEADER,0);
                else
                    m_pMyOwnView->removeSection(USHORT(0));
            }
            else if ( evt.PropertyName.equals( PROPERTY_PAGEFOOTERON ) )
            {
                if ( bShow )
                    m_pMyOwnView->addSection(m_xReportDefinition->getPageFooter(),DBPAGEFOOTER);
                else
                    m_pMyOwnView->removeSection(m_pMyOwnView->getSectionCount() - 1);
            }
            else if (   evt.PropertyName.equals( PROPERTY_COMMAND )
                    ||  evt.PropertyName.equals( PROPERTY_COMMANDTYPE )
                    ||  evt.PropertyName.equals( PROPERTY_ESCAPEPROCESSING )
                    ||  evt.PropertyName.equals( PROPERTY_FILTER )
                    )
            {
                InvalidateFeature(SID_FM_ADD_FIELD);
                if ( !m_pMyOwnView->isAddFieldVisible() )
                    m_pMyOwnView->toggleAddField();
            }
            /// TODO: check what we need to notify here TitleHelper
            /*else if (   evt.PropertyName.equals( PROPERTY_CAPTION ) )
                updateTitle();*/
        } // if ( evt.Source == m_xReportDefinition )
        else
        {
            uno::Reference< report::XGroup> xGroup(evt.Source,uno::UNO_QUERY);
            if ( xGroup.is() )
            {
                sal_Int32 nGroupPos = getGroupPosition(xGroup);

                groupChange(xGroup,evt.PropertyName,nGroupPos,bShow);
            }
        }
    }
    catch(const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

// -----------------------------------------------------------------------------
void SAL_CALL OReportController::disposing( const lang::EventObject& Source ) throw(uno::RuntimeException)
{
    OReportController_BASE::disposing(Source);
}

// -----------------------------------------------------------------------------
USHORT lcl_getNonVisbleGroupsBefore( const uno::Reference< report::XGroups>& _xGroups
                          ,sal_Int32 _nGroupPos
                          ,::std::mem_fun_t<sal_Bool,OGroupHelper>&_pGroupMemberFunction)
{
    uno::Reference< report::XGroup> xGroup;
    USHORT nNonVisibleGroups = 0;
    sal_Int32 nCount = _xGroups->getCount();
    for( sal_Int32 i = 0; i < _nGroupPos && i < nCount; ++i)
    {
        xGroup.set(_xGroups->getByIndex(i),uno::UNO_QUERY);
        OSL_ENSURE(xGroup.is(),"Group is NULL! -> GPF");
        OGroupHelper aGroupHelper(xGroup);
        if ( !_pGroupMemberFunction(&aGroupHelper) )
            ++nNonVisibleGroups;
    }
    return nNonVisibleGroups;
}
// -----------------------------------------------------------------------------
void OReportController::groupChange( const uno::Reference< report::XGroup>& _xGroup,const ::rtl::OUString& _sPropName,sal_Int32 _nGroupPos,bool _bShow)
{
    //adjustSectionName(_xGroup,_nGroupPos);
    ::std::mem_fun_t<sal_Bool,OGroupHelper> pMemFun = ::std::mem_fun(&OGroupHelper::getHeaderOn);
    ::std::mem_fun_t<uno::Reference<report::XSection> , OGroupHelper> pMemFunSection = ::std::mem_fun(&OGroupHelper::getHeader);
    ::rtl::OUString sColor(DBGROUPHEADER);
    USHORT nPosition = 0;
    bool bHandle = false;
    if ( _sPropName.equals( PROPERTY_HEADERON ) )
    {
        nPosition = m_xReportDefinition->getPageHeaderOn() ? (m_xReportDefinition->getReportHeaderOn() ? 2 : 1) : (m_xReportDefinition->getReportHeaderOn() ? 1 : 0);
        nPosition += (static_cast<USHORT>(_nGroupPos) - lcl_getNonVisbleGroupsBefore(m_xReportDefinition->getGroups(),_nGroupPos,pMemFun));
        bHandle = true;
    }
    else if ( _sPropName.equals( PROPERTY_FOOTERON ) )
    {
        pMemFun = ::std::mem_fun(&OGroupHelper::getFooterOn);
        pMemFunSection = ::std::mem_fun(&OGroupHelper::getFooter);
        nPosition = m_pMyOwnView->getSectionCount();

        if ( m_xReportDefinition->getPageFooterOn() )
            --nPosition;
        if ( m_xReportDefinition->getReportFooterOn() )
            --nPosition;
        sColor = DBGROUPFOOTER;
        nPosition -= (static_cast<USHORT>(_nGroupPos) - lcl_getNonVisbleGroupsBefore(m_xReportDefinition->getGroups(),_nGroupPos,pMemFun));
        if ( !_bShow )
            --nPosition;
        bHandle = true;
    }
    if ( bHandle )
    {
        if ( _bShow )
        {
            OGroupHelper aGroupHelper(_xGroup);
            m_pMyOwnView->addSection(pMemFunSection(&aGroupHelper),sColor,nPosition);
        }
        else
            m_pMyOwnView->removeSection(nPosition);
    }
}
//------------------------------------------------------------------------------
IMPL_LINK( OReportController, OnClipboardChanged, void*, EMPTYARG )
{
    return OnInvalidateClipboard( NULL );
}
//------------------------------------------------------------------------------
IMPL_LINK( OReportController, NotifyUndoActionHdl, SfxUndoAction*, _pUndoAction )
{
    OSL_ENSURE(_pUndoAction,"UndoAction is NULL!");
    addUndoActionAndInvalidate(_pUndoAction);
    return 0L;
}
//------------------------------------------------------------------------------
IMPL_LINK(OReportController, OnInvalidateClipboard, void*, EMPTYARG)
{
    InvalidateFeature(SID_CUT);
    InvalidateFeature(SID_COPY);
    InvalidateFeature(SID_PASTE);
    return 0L;
}

namespace
{
// -----------------------------------------------------------------------------
view::PaperFormat lcl_convertPaperFormat(SvxPaper _ePaperFormat)
{
    view::PaperFormat eUnoPaperFormat;
    switch(_ePaperFormat)
    {
        case SVX_PAPER_A3:
            eUnoPaperFormat = view::PaperFormat_A3;
            break;
        case SVX_PAPER_A4:
            eUnoPaperFormat = view::PaperFormat_A4;
            break;
        case SVX_PAPER_A5:
            eUnoPaperFormat = view::PaperFormat_A5;
            break;
        case SVX_PAPER_B4:
            eUnoPaperFormat = view::PaperFormat_B4;
            break;
        case SVX_PAPER_B5:
            eUnoPaperFormat = view::PaperFormat_B5;
            break;
        case SVX_PAPER_LETTER:
            eUnoPaperFormat = view::PaperFormat_LETTER;
            break;
        case SVX_PAPER_LEGAL:
            eUnoPaperFormat = view::PaperFormat_LEGAL;
            break;
        case SVX_PAPER_TABLOID:
            eUnoPaperFormat = view::PaperFormat_TABLOID;
            break;
        default:
            eUnoPaperFormat = view::PaperFormat_USER;
    } // switch(_ePaperFormat)
    return eUnoPaperFormat;
}
// -----------------------------------------------------------------------------
SvxPaper lcl_convertPaperFormat(view::PaperFormat _eUnoPaperFormat)
{
    SvxPaper ePaperFormat;
    switch(_eUnoPaperFormat)
    {
        case view::PaperFormat_A3:
            ePaperFormat = SVX_PAPER_A3;
            break;
        case view::PaperFormat_A4:
            ePaperFormat = SVX_PAPER_A4;
            break;
        case view::PaperFormat_A5:
            ePaperFormat = SVX_PAPER_A5;
            break;
        case view::PaperFormat_B4:
            ePaperFormat = SVX_PAPER_B4;
            break;
        case view::PaperFormat_B5:
            ePaperFormat = SVX_PAPER_B5;
            break;
        case view::PaperFormat_LETTER:
            ePaperFormat = SVX_PAPER_LETTER;
            break;
        case view::PaperFormat_LEGAL:
            ePaperFormat = SVX_PAPER_LEGAL;
            break;
        case view::PaperFormat_TABLOID:
            ePaperFormat = SVX_PAPER_TABLOID;
            break;
        default:
            ePaperFormat = SVX_PAPER_USER;
    } // switch(_eUnoPaperFormat)
    return ePaperFormat;
}
// -----------------------------------------------------------------------------
SvxGraphicPosition lcl_convertGraphicPosition(style::GraphicLocation eUnoGraphicPos)
{
    SvxGraphicPosition eRet = GPOS_NONE;
    switch(eUnoGraphicPos)
    {
        case style::GraphicLocation_NONE: eRet = GPOS_NONE; break;
        case style::GraphicLocation_LEFT_TOP: eRet = GPOS_LT; break;
        case style::GraphicLocation_MIDDLE_TOP: eRet = GPOS_MT; break;
        case style::GraphicLocation_RIGHT_TOP: eRet = GPOS_RT; break;
        case style::GraphicLocation_LEFT_MIDDLE: eRet = GPOS_LM; break;
        case style::GraphicLocation_MIDDLE_MIDDLE: eRet = GPOS_MM; break;
        case style::GraphicLocation_RIGHT_MIDDLE: eRet = GPOS_RM; break;
        case style::GraphicLocation_LEFT_BOTTOM: eRet = GPOS_LB; break;
        case style::GraphicLocation_MIDDLE_BOTTOM: eRet = GPOS_MB; break;
        case style::GraphicLocation_RIGHT_BOTTOM: eRet = GPOS_RB; break;
        case style::GraphicLocation_AREA: eRet = GPOS_AREA; break;
        case style::GraphicLocation_TILED: eRet = GPOS_TILED; break;
        default:
            break;
    } // switch(eUnoGraphicPos)
    return eRet;
}
// -----------------------------------------------------------------------------
style::GraphicLocation lcl_convertGraphicPosition(SvxGraphicPosition eGraphicPos)
{
    style::GraphicLocation eRet = style::GraphicLocation_NONE;
    switch(eGraphicPos)
    {
        case GPOS_NONE: eRet = style::GraphicLocation_NONE; break;
        case GPOS_LT: eRet = style::GraphicLocation_LEFT_TOP; break;
        case GPOS_MT: eRet = style::GraphicLocation_MIDDLE_TOP; break;
        case GPOS_RT: eRet = style::GraphicLocation_RIGHT_TOP; break;
        case GPOS_LM: eRet = style::GraphicLocation_LEFT_MIDDLE; break;
        case GPOS_MM: eRet = style::GraphicLocation_MIDDLE_MIDDLE; break;
        case GPOS_RM: eRet = style::GraphicLocation_RIGHT_MIDDLE; break;
        case GPOS_LB: eRet = style::GraphicLocation_LEFT_BOTTOM; break;
        case GPOS_MB: eRet = style::GraphicLocation_MIDDLE_BOTTOM; break;
        case GPOS_RB: eRet = style::GraphicLocation_RIGHT_BOTTOM; break;
        case GPOS_AREA: eRet = style::GraphicLocation_AREA; break;
        case GPOS_TILED: eRet = style::GraphicLocation_TILED; break;
        default:
            break;
    } // switch(eUnoGraphicPos)
    return eRet;
}
// -----------------------------------------------------------------------------
// =============================================================================
}
// =============================================================================
// -----------------------------------------------------------------------------
void OReportController::openPageDialog(const uno::Reference<report::XSection>& _xSection)
{
    if ( !m_xReportDefinition.is() )
        return;
    // ------------
    // UNO->ItemSet
    static SfxItemInfo aItemInfos[] =
    {
        { SID_ATTR_LRSPACE,     SFX_ITEM_POOLABLE },
        { SID_ATTR_ULSPACE,     SFX_ITEM_POOLABLE },
        { SID_ATTR_PAGE,        SFX_ITEM_POOLABLE },
        { SID_ATTR_PAGE_SIZE,   SFX_ITEM_POOLABLE },
        { SID_ENUM_PAGE_MODE,   SFX_ITEM_POOLABLE },
        { SID_PAPER_START,      SFX_ITEM_POOLABLE },
        { SID_PAPER_END,        SFX_ITEM_POOLABLE },
        { SID_ATTR_BRUSH,       SFX_ITEM_POOLABLE },
        { SID_FLAG_TYPE,        SFX_ITEM_POOLABLE },
        { SID_ATTR_METRIC,      SFX_ITEM_POOLABLE }
    };

    MeasurementSystem eSystem = SvtSysLocale().GetLocaleData().getMeasurementSystemEnum();
    FieldUnit eUserMetric = MEASURE_METRIC == eSystem ? FUNIT_CM : FUNIT_INCH;
    SfxPoolItem* pDefaults[] =
    {
        new SvxLRSpaceItem(RPTUI_ID_LRSPACE),
        new SvxULSpaceItem(RPTUI_ID_ULSPACE),
        new SvxPageItem(RPTUI_ID_PAGE),
        new SvxSizeItem(RPTUI_ID_SIZE),
        new SfxAllEnumItem(RPTUI_ID_PAGE_MODE,SVX_PAGE_MODE_STANDARD),
        new SfxAllEnumItem(RPTUI_ID_START,SVX_PAPER_A4),
        new SfxAllEnumItem(RPTUI_ID_END,SVX_PAPER_E),
        new SvxBrushItem(ITEMID_BRUSH),
        new SfxUInt16Item(RPTUI_ID_METRIC,static_cast<UINT16>(eUserMetric))
    };

    static USHORT pRanges[] =
    {
        RPTUI_ID_LRSPACE,RPTUI_ID_BRUSH,
        SID_ATTR_METRIC,SID_ATTR_METRIC,
        0
    };

    try
    {
        ::std::auto_ptr<SfxItemPool> pPool( new SfxItemPool(String::CreateFromAscii("ReportPageProperties"), RPTUI_ID_LRSPACE,RPTUI_ID_METRIC, aItemInfos, pDefaults) );
        pPool->SetDefaultMetric( SFX_MAPUNIT_100TH_MM );    // ripped, don't understand why
        pPool->FreezeIdRanges();                        // the same

        ::std::auto_ptr<SfxItemSet> pDescriptor(new SfxItemSet(*pPool, pRanges));
        // fill it
        if ( _xSection.is() )
            pDescriptor->Put(SvxBrushItem(::Color(_xSection->getBackColor()),ITEMID_BRUSH));
        else
        {
            //view::PaperFormat eUnoPaperFormat = m_xReportDefinition->getPaperFormat();
            //pDescriptor->Put(SfxAllEnumItem(RPTUI_ID_START,lcl_convertPaperFormat(eUnoPaperFormat)));
            pDescriptor->Put(SvxSizeItem(RPTUI_ID_SIZE,VCLSize(getStyleProperty<awt::Size>(m_xReportDefinition,PROPERTY_PAPERSIZE))));
            pDescriptor->Put(SvxLRSpaceItem(getStyleProperty<sal_Int32>(m_xReportDefinition,PROPERTY_LEFTMARGIN)
                                            ,getStyleProperty<sal_Int32>(m_xReportDefinition,PROPERTY_RIGHTMARGIN),0,0,RPTUI_ID_LRSPACE));
            pDescriptor->Put(SvxULSpaceItem(static_cast<USHORT>(getStyleProperty<sal_Int32>(m_xReportDefinition,PROPERTY_TOPMARGIN))
                                            ,static_cast<USHORT>(getStyleProperty<sal_Int32>(m_xReportDefinition,PROPERTY_BOTTOMMARGIN)),RPTUI_ID_ULSPACE));
            pDescriptor->Put(SfxUInt16Item(SID_ATTR_METRIC,static_cast<UINT16>(eUserMetric)));

            uno::Reference< style::XStyle> xPageStyle(getUsedStyle(m_xReportDefinition));
            if ( xPageStyle.is() )
            {
                SvxPageItem aPageItem(RPTUI_ID_PAGE);
                aPageItem.SetDescName(xPageStyle->getName());
                uno::Reference<beans::XPropertySet> xProp(xPageStyle,uno::UNO_QUERY_THROW);
                aPageItem.PutValue(xProp->getPropertyValue(PROPERTY_PAGESTYLELAYOUT),MID_PAGE_LAYOUT);
                aPageItem.SetLandscape(getStyleProperty<sal_Bool>(m_xReportDefinition,PROPERTY_ISLANDSCAPE));
                aPageItem.SetNumType((SvxNumType)getStyleProperty<sal_Int16>(m_xReportDefinition,PROPERTY_NUMBERINGTYPE));
                pDescriptor->Put(aPageItem);
                pDescriptor->Put(SvxBrushItem(::Color(getStyleProperty<sal_Int32>(m_xReportDefinition,PROPERTY_BACKCOLOR)),RPTUI_ID_BRUSH));
            }
        }

        {   // want the dialog to be destroyed before our set
            ORptPageDialog aDlg(getView(), pDescriptor.get(),_xSection.is() ? RID_PAGEDIALOG_BACKGROUND : RID_PAGEDIALOG_PAGE);
            if (RET_OK == aDlg.Execute())
            {
                // ------------
                // ItemSet->UNO
                // UNO-properties
                const SfxItemSet* pSet = aDlg.GetOutputItemSet();
                if ( _xSection.is() )
                {
                    const SfxPoolItem* pItem;
                    if ( SFX_ITEM_SET == pSet->GetItemState( RPTUI_ID_BRUSH,sal_True,&pItem))
                        _xSection->setBackColor(static_cast<const SvxBrushItem*>(pItem)->GetColor().GetColor());
                }
                else
                {
                    uno::Reference< beans::XPropertySet> xProp(getUsedStyle(m_xReportDefinition),uno::UNO_QUERY_THROW);
                    const String sUndoAction(ModuleRes(RID_STR_UNDO_CHANGEPAGE));
                    UndoManagerListAction aListAction(m_aUndoManager,sUndoAction);
                    const SfxPoolItem* pItem = NULL;
                    if ( SFX_ITEM_SET == pSet->GetItemState( RPTUI_ID_SIZE,sal_True,&pItem))
                    {
                        const Size aPaperSize = static_cast<const SvxSizeItem*>(pItem)->GetSize();
                        //view::PaperFormat eUnoPaperFormat = lcl_convertPaperFormat(SvxPaperInfo::GetSvxPaper(aPaperSize,MAP_100TH_MM,TRUE));
                        //m_xReportDefinition->setPaperFormat(eUnoPaperFormat);
                        uno::Any aValue;
                        static_cast<const SvxSizeItem*>(pItem)->QueryValue(aValue,MID_SIZE_SIZE);
                        xProp->setPropertyValue(PROPERTY_PAPERSIZE,aValue);
                    }

                    if ( SFX_ITEM_SET == pSet->GetItemState( RPTUI_ID_LRSPACE,sal_True,&pItem))
                    {
                        xProp->setPropertyValue(PROPERTY_LEFTMARGIN,uno::makeAny(static_cast<const SvxLRSpaceItem*>(pItem)->GetLeft()));
                        xProp->setPropertyValue(PROPERTY_RIGHTMARGIN,uno::makeAny(static_cast<const SvxLRSpaceItem*>(pItem)->GetRight()));
                    }
                    if ( SFX_ITEM_SET == pSet->GetItemState( RPTUI_ID_ULSPACE,sal_True,&pItem))
                    {
                        xProp->setPropertyValue(PROPERTY_TOPMARGIN,uno::makeAny(static_cast<const SvxULSpaceItem*>(pItem)->GetUpper()));
                        xProp->setPropertyValue(PROPERTY_BOTTOMMARGIN,uno::makeAny(static_cast<const SvxULSpaceItem*>(pItem)->GetLower()));
                    }
                    if ( SFX_ITEM_SET == pSet->GetItemState( RPTUI_ID_PAGE,sal_True,&pItem))
                    {
                        const SvxPageItem* pPageItem = static_cast<const SvxPageItem*>(pItem);
                        xProp->setPropertyValue(PROPERTY_ISLANDSCAPE,uno::makeAny(static_cast<sal_Bool>(pPageItem->IsLandscape())));
                        xProp->setPropertyValue(PROPERTY_NUMBERINGTYPE,uno::makeAny(static_cast<sal_Int16>(pPageItem->GetNumType())));
                        uno::Any aValue;
                        pPageItem->QueryValue(aValue,MID_PAGE_LAYOUT);
                        xProp->setPropertyValue(PROPERTY_PAGESTYLELAYOUT,aValue);
                    }
                    if ( SFX_ITEM_SET == pSet->GetItemState( RPTUI_ID_BRUSH,sal_True,&pItem))
                    {
                        ::Color aBackColor = static_cast<const SvxBrushItem*>(pItem)->GetColor();
                        xProp->setPropertyValue(PROPERTY_BACKTRANSPARENT,uno::makeAny(aBackColor == COL_TRANSPARENT));
                        xProp->setPropertyValue(PROPERTY_BACKCOLOR,uno::makeAny(aBackColor.GetColor()));
                    }
                }
            }
        }
    }
    catch(Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    for (sal_uInt16 i=0; i<sizeof(pDefaults)/sizeof(pDefaults[0]); ++i)
        delete pDefaults[i];

}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OReportController::attachModel(const uno::Reference< frame::XModel > & xModel) throw( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    m_xReportDefinition.set(xModel,uno::UNO_QUERY);
    return m_xReportDefinition.is();
}
// -----------------------------------------------------------------------------
void OReportController::openSortingAndGroupingDialog()
{
    if ( !m_xReportDefinition.is() )
        return;
    if ( !m_pGroupsFloater )
    {
        m_pGroupsFloater = new OGroupsSortingDialog(getView(),!isEditable(),this);
        m_pGroupsFloater->AddEventListener(LINK(this,OReportController,EventLstHdl));
    }
    else
        m_pGroupsFloater->Show(!m_pGroupsFloater->IsVisible());
}
// -----------------------------------------------------------------------------
sal_Int32 OReportController::getGroupPosition(const uno::Reference< report::XGroup >& _xGroup)
{
    return rptui::getPositionInIndexAccess(m_xReportDefinition->getGroups().get(),_xGroup);
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
IMPL_LINK( OReportController, EventLstHdl, VclWindowEvent*, _pEvent )
{
    if ( _pEvent && _pEvent->GetId() == VCLEVENT_WINDOW_CLOSE )
    {
        InvalidateFeature(SID_SORTINGANDGROUPING);
        InvalidateFeature(SID_FM_ADD_FIELD);
        InvalidateFeature(SID_RPT_SHOWREPORTEXPLORER);
    }
    return 1L;
}
// -----------------------------------------------------------------------------
sal_Bool OReportController::isFormatCommandEnabled(sal_uInt16 _nCommand,const uno::Reference< report::XReportControlFormat>& _xReportControlFormat) const
{
    sal_Bool bRet = sal_False;
    if ( _xReportControlFormat.is() && !uno::Reference< report::XFixedLine>(_xReportControlFormat,uno::UNO_QUERY).is() ) // this command is really often called so we nedd a short cut here
    {
        try
        {
            const awt::FontDescriptor aFontDescriptor = _xReportControlFormat->getFontDescriptor();

            switch(_nCommand)
            {
                case SID_ATTR_CHAR_WEIGHT:
                    bRet = awt::FontWeight::BOLD == aFontDescriptor.Weight;
                    break;
                case SID_ATTR_CHAR_POSTURE:
                    bRet = awt::FontSlant_ITALIC == aFontDescriptor.Slant;
                    break;
                case SID_ATTR_CHAR_UNDERLINE:
                    bRet = awt::FontUnderline::SINGLE == aFontDescriptor.Underline;
                    break;
                default:
                    ;
            } // switch(_nCommand)
        }
        catch(uno::Exception&)
        {
        }
    }
    return bRet;
}
// -----------------------------------------------------------------------------
void OReportController::Notify(SfxBroadcaster & /*rBc*/, SfxHint const & rHint)
{
    if (rHint.ISA(DlgEdHint)
        && (static_cast< DlgEdHint const & >(rHint).GetKind()
            == RPTUI_HINT_SELECTIONCHANGED))
    {
        InvalidateAll();
        lang::EventObject aEvent(*this);
        m_aSelectionListeners.forEach<view::XSelectionChangeListener>(
            ::boost::bind(&view::XSelectionChangeListener::selectionChanged,_1,boost::cref(aEvent)));

    }
}
// -----------------------------------------------------------------------------
void OReportController::executeMethodWithUndo(USHORT _nUndoStrId,const ::std::mem_fun_t<void,ODesignView>& _pMemfun)
{
    const String sUndoAction = String((ModuleRes(_nUndoStrId)));
    UndoManagerListAction aListAction(m_aUndoManager,sUndoAction);
    _pMemfun(m_pMyOwnView);
    InvalidateFeature( SID_SAVEDOC );
    InvalidateFeature( SID_UNDO );
}
// -----------------------------------------------------------------------------
void OReportController::alignControlsWithUndo(USHORT _nUndoStrId,sal_Int32 _nControlModification,bool _bAlignAtSection)
{
    const String sUndoAction = String((ModuleRes(_nUndoStrId)));
    UndoManagerListAction aListAction(m_aUndoManager,sUndoAction);
    m_pMyOwnView->alignMarkedObjects(_nControlModification,_bAlignAtSection);
    InvalidateFeature( SID_SAVEDOC );
    InvalidateFeature( SID_UNDO );
}
// -----------------------------------------------------------------------------
uno::Any SAL_CALL OReportController::getViewData(void) throw( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    typedef ::std::pair< ::rtl::OUString,sal_uInt16> TStringIntPair;
    const TStringIntPair pViewDataList[] =
    {
         TStringIntPair(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("GridVisible")),            SID_GRID_VISIBLE)
        ,TStringIntPair(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("GridUse")),                SID_GRID_USE)
        ,TStringIntPair(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HelplinesMove")),          SID_HELPLINES_MOVE)
        ,TStringIntPair(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowRuler")),              SID_RULER)
        ,TStringIntPair(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ControlProperties")),      SID_SHOW_PROPERTYBROWSER)
        ,TStringIntPair(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LastPropertyBrowserPage")),SID_PROPERTYBROWSER_LAST_PAGE)
        ,TStringIntPair(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SplitPosition")),          SID_SPLIT_POSITION)
    };

    uno::Sequence<beans::PropertyValue> aCommandProps(sizeof(pViewDataList)/sizeof(pViewDataList[0]));
    beans::PropertyValue* pIter = aCommandProps.getArray();
    beans::PropertyValue* pEnd = pIter + aCommandProps.getLength();
    for (sal_Int32 i = 0; pIter != pEnd; ++pIter,++i)
    {
        FeatureState aFeatureState = GetState(pViewDataList[i].second);
        pIter->Name = pViewDataList[i].first;
        if ( !!aFeatureState.bChecked )
            pIter->Value <<= (*aFeatureState.bChecked) ? sal_True : sal_False;
        else if ( aFeatureState.aValue.hasValue() )
            pIter->Value = aFeatureState.aValue;

    } // for (; pIter != pEnd; ++pIter)

    uno::Sequence<beans::PropertyValue> aProps(1);
    aProps[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CommandProperties"));
    aProps[0].Value <<= aCommandProps;

    if ( m_pMyOwnView )
    {
        ::std::vector<sal_uInt16> aCollapsedPositions;
        m_pMyOwnView->fillCollapsedSections(aCollapsedPositions);
        if ( !aCollapsedPositions.empty() )
        {
            uno::Sequence<beans::PropertyValue> aCollapsedSections(aCollapsedPositions.size());
            beans::PropertyValue* pCollapsedIter = aCollapsedSections.getArray();
            ::std::vector<sal_uInt16>::iterator aIter = aCollapsedPositions.begin();
            ::std::vector<sal_uInt16>::iterator aEnd = aCollapsedPositions.end();
            for (sal_Int32 i = 1; aIter != aEnd ; ++aIter,++pCollapsedIter,++i)
            {
                pCollapsedIter->Name = PROPERTY_SECTION + ::rtl::OUString::valueOf(i);
                pCollapsedIter->Value <<= static_cast<sal_Int32>(*aIter);
            }
            const sal_Int32 nCount = aProps.getLength();
            aProps.realloc( nCount + 1 );
            aProps[nCount].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CollapsedSections"));
            aProps[nCount].Value <<= aCollapsedSections;
        }

        ::boost::shared_ptr<OReportSection> pSection = m_pMyOwnView->getMarkedSection();
        if ( pSection.get() )
        {
            const sal_Int32 nCount = aProps.getLength();
            aProps.realloc( nCount + 1 );
            aProps[nCount].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MarkedSection"));
            aProps[nCount].Value <<= (sal_Int32)pSection->getPage()->GetPageNum();
        }
    }
    return uno::makeAny(aProps);
}
// -----------------------------------------------------------------------------
void SAL_CALL OReportController::restoreViewData(const uno::Any& Data) throw( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    uno::Sequence<beans::PropertyValue> aProps;
    if ( Data >>= aProps )
    {
        const beans::PropertyValue* pPropsIter = aProps.getConstArray();
        const beans::PropertyValue* pPropsEnd = pPropsIter + aProps.getLength();
        for (sal_Int32 i = 0; pPropsIter != pPropsEnd; ++pPropsIter,++i)
        {
            if ( pPropsIter->Name.equalsAscii("CommandProperties") )
            {
                util::URL aCommand;
                uno::Sequence< beans::PropertyValue> aArgs(1);
                beans::PropertyValue* pArg = aArgs.getArray();
                pArg->Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Value"));
                uno::Sequence< beans::PropertyValue> aCommandProps;
                if ( pPropsIter->Value >>= aCommandProps )
                {
                    const beans::PropertyValue* pIter = aCommandProps.getConstArray();
                    const beans::PropertyValue* pEnd = pIter + aCommandProps.getLength();
                    for (; pIter != pEnd; ++pIter)
                    {
                        pArg->Value = pIter->Value;
                        if ( pArg->Value.hasValue() )
                        {
                            aCommand.Complete = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:")) + pIter->Name;
                            executeUnChecked(aCommand,aArgs);
                        }
                    }
                }
            }
            else if ( pPropsIter->Name.equalsAscii("CollapsedSections") )
                pPropsIter->Value >>= m_aCollapsedSections;
            else if ( pPropsIter->Name.equalsAscii("MarkedSection") )
                pPropsIter->Value >>= m_nPageNum;
        }
    }
}
// -----------------------------------------------------------------------------
void OReportController::updateFloater()
{
       if ( m_pGroupsFloater && m_pGroupsFloater->IsVisible() )
        m_pGroupsFloater->UpdateData();
}

// -----------------------------------------------------------------------------
Reference<XFrame> OReportController::getXFrame()
{
    if ( !m_xFrameLoader.is() )
    {
        m_xFrameLoader.set(getORB()->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop"))),uno::UNO_QUERY_THROW);
    }
    const sal_Int32 nFrameSearchFlag = frame::FrameSearchFlag::TASKS | frame::FrameSearchFlag::CREATE;
    const ::rtl::OUString sTarget(RTL_CONSTASCII_USTRINGPARAM("_blank"));
    Reference<XFrame> xFrame = Reference<XFrame>(m_xFrameLoader,uno::UNO_QUERY_THROW)->findFrame(sTarget,nFrameSearchFlag);
    return xFrame;
}

// -----------------------------------------------------------------------------
uno::Reference<frame::XModel> OReportController::executeReport()
{
    OSL_ENSURE(m_xReportDefinition.is(),"Where is my report?");

    uno::Reference<frame::XModel> xModel;
    if ( m_xReportDefinition.is() )
    {
        sal_uInt16 nErrorId = RID_ERR_NO_COMMAND;
        bool bEnabled = m_xReportDefinition->getCommand().getLength() != 0;
        if ( bEnabled )
        {
            bEnabled = false;
            const sal_uInt16 nCount = m_aReportModel->GetPageCount();
            sal_uInt16 i = 0;
            for (; i < nCount && !bEnabled ; ++i)
            {
                const SdrPage* pPage = m_aReportModel->GetPage(i);
                bEnabled = pPage->GetObjCount() != 0;
            }
            if ( !bEnabled )
                nErrorId = RID_ERR_NO_OBJECTS;
        }

        dbtools::SQLExceptionInfo aInfo;
        if ( !bEnabled )
        {
            sdb::SQLContext aFirstMessage;
            String sInfo = String( ModuleRes( nErrorId ) );
            aFirstMessage.Message = sInfo;
            aInfo = aFirstMessage;
            if ( isEditable() )
            {
                sal_uInt16 nCommand = 0;
                if ( nErrorId == RID_ERR_NO_COMMAND )
                {
                    if ( !m_bShowProperties )
                        executeUnChecked(SID_SHOW_PROPERTYBROWSER,uno::Sequence< beans::PropertyValue>());

                    m_sLastActivePage = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Data"));
                    m_pMyOwnView->setCurrentPage(m_sLastActivePage);
                    nCommand = SID_SELECT_REPORT;
                }
                else if ( m_pMyOwnView && !m_pMyOwnView->isAddFieldVisible() )
                {
                    nCommand = SID_FM_ADD_FIELD;
                }
                if ( nCommand )
                {
                    uno::Sequence< beans::PropertyValue> aArgs;
                    executeUnChecked(nCommand,aArgs);
                }
            }
        }
        else
        {
            try
            {
                WaitObject aWait(getView()); // cursor
                if ( !m_xReportEngine.is() )
                    m_xReportEngine.set(getORB()->createInstance(SERVICE_REPORTENGINE),uno::UNO_QUERY_THROW);
                m_xReportEngine->setReportDefinition(m_xReportDefinition);
                m_xReportEngine->setActiveConnection(getConnection());
                Reference<XFrame> xFrame = getXFrame();
                xModel = m_xReportEngine->createDocumentAlive(xFrame);
            }
            catch( const sdbc::SQLException& /*e*/ )
            {   // SQLExceptions and derived exceptions must not be translated
                aInfo = ::cppu::getCaughtException();
            }
            catch(const uno::Exception& e)
            {
                uno::Any aCaughtException( ::cppu::getCaughtException() );

                // our first message says: we caught an exception
                sdb::SQLContext aFirstMessage;
                String sInfo = String( ModuleRes( RID_STR_CAUGHT_FOREIGN_EXCEPTION ) );
                sInfo.SearchAndReplaceAllAscii( "$type$", aCaughtException.getValueTypeName() );
                aFirstMessage.Message = sInfo;

                // our second message: the message of the exception we caught
                sdbc::SQLException aSecondMessage;
                aSecondMessage.Message = e.Message;
                aSecondMessage.Context = e.Context;

                // maybe our third message: the message which is wrapped in the exception we caught
                sdbc::SQLException aThirdMessage;
                lang::WrappedTargetException aWrapped;
                if ( aCaughtException >>= aWrapped )
                {
                    aThirdMessage.Message = aWrapped.Message;
                    aThirdMessage.Context = aWrapped.Context;
                }

                if ( aThirdMessage.Message.getLength() )
                    aSecondMessage.NextException <<= aThirdMessage;
                aFirstMessage.NextException <<= aSecondMessage;

                aInfo = aFirstMessage;
            }
            if (aInfo.isValid())
            {
                const String suSQLContext = String( ModuleRes( RID_STR_COULD_NOT_CREATE_REPORT ) );
                aInfo.prepend(suSQLContext);
            }
        }

        if (aInfo.isValid())
        {
            showError(aInfo);
        }
    }
    return xModel;
}
// -----------------------------------------------------------------------------
uno::Reference< frame::XModel >  SAL_CALL OReportController::getModel(void) throw( uno::RuntimeException )
{
    return m_xReportDefinition.get();
}
// -----------------------------------------------------------------------------
uno::Reference< sdbc::XRowSet > OReportController::getRowSet()
{
    OSL_PRECOND( m_xReportDefinition.is(), "OReportController::getRowSet: no report definition?!" );

    if ( m_xRowSet.is() || !m_xReportDefinition.is() )
        return m_xRowSet;

    try
    {
        uno::Reference< sdbc::XRowSet > xRowSet( getORB()->createInstance(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sdb.RowSet" ) ) ), uno::UNO_QUERY );
        uno::Reference< beans::XPropertySet> xRowSetProp( xRowSet, uno::UNO_QUERY_THROW );

        xRowSetProp->setPropertyValue( PROPERTY_ACTIVECONNECTION, uno::makeAny( getConnection() ) );
        xRowSetProp->setPropertyValue( PROPERTY_APPLYFILTER, uno::makeAny( sal_True ) );

        TPropertyNamePair aPropertyMediation;
        aPropertyMediation.insert( TPropertyNamePair::value_type( PROPERTY_COMMAND, PROPERTY_COMMAND ) );
        aPropertyMediation.insert( TPropertyNamePair::value_type( PROPERTY_COMMANDTYPE, PROPERTY_COMMANDTYPE ) );
        aPropertyMediation.insert( TPropertyNamePair::value_type( PROPERTY_ESCAPEPROCESSING, PROPERTY_ESCAPEPROCESSING ) );
        aPropertyMediation.insert( TPropertyNamePair::value_type( PROPERTY_FILTER, PROPERTY_FILTER ) );

        m_xRowSetMediator = new OPropertyMediator( m_xReportDefinition.get(), xRowSetProp, aPropertyMediation );
        m_xRowSet = xRowSet;
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return m_xRowSet;
}
// -----------------------------------------------------------------------------
void OReportController::insertGraphic()
{
    const String sTitle(ModuleRes(RID_STR_IMPORT_GRAPHIC));
    // build some arguments for the upcoming dialog
    try
    {
        uno::Reference< report::XSection> xSection = m_pMyOwnView->getCurrentSection();
        ::sfx2::FileDialogHelper aDialog( ui::dialogs::TemplateDescription::FILEOPEN_LINK_PREVIEW, SFXWB_GRAPHIC );
        aDialog.SetTitle( sTitle );

        uno::Reference< ui::dialogs::XFilePickerControlAccess > xController(aDialog.GetFilePicker(), UNO_QUERY_THROW);
        xController->setValue(ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_PREVIEW, 0, ::cppu::bool2any(sal_True));
        xController->enableControl(ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_LINK, sal_False/*sal_True*/);
        sal_Bool bLink = sal_True;
        xController->setValue( ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_LINK, 0, ::cppu::bool2any( bLink ) );

        if ( ERRCODE_NONE == aDialog.Execute() )
        {
            xController->getValue( ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_LINK, 0) >>= bLink;
            uno::Sequence<beans::PropertyValue> aArgs(2);
            aArgs[0].Name = PROPERTY_IMAGEURL;
            aArgs[0].Value <<= ::rtl::OUString(aDialog.GetPath());
            aArgs[1].Name = PROPERTY_PRESERVEIRI;
            aArgs[1].Value <<= bLink;
            createControl(aArgs,xSection,::rtl::OUString(),OBJ_DLG_IMAGECONTROL);
        }
    }
    catch(Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}
// -----------------------------------------------------------------------------
void OReportController::displayDesignFloater(sal_Bool _bShow)
{
    if ( m_pGroupsFloater )
    {
        m_pGroupsFloater->Show( m_bGroupFloaterWasVisible && _bShow);
    }
}
// -----------------------------------------------------------------------------
::boost::shared_ptr<rptui::OReportModel> OReportController::getSdrModel()
{
    if ( !m_aReportModel )
    {
        m_aReportModel = reportdesign::OReportDefinition::getSdrModel(m_xReportDefinition);
        if ( m_aReportModel )
        {
            m_aReportModel->attachController( *this );
            m_aReportModel->SetNotifyUndoActionHdl(LINK( this, OReportController, NotifyUndoActionHdl ));
        }
    }
    return m_aReportModel;
}
// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL OReportController::select( const Any& aSelection ) throw (IllegalArgumentException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::sal_Bool bRet = sal_True;
    if ( m_pMyOwnView )
    {
        m_pMyOwnView->unmarkAllObjects(NULL);
        m_pMyOwnView->SetMode(RPTUI_SELECT);

        uno::Sequence< uno::Reference<report::XReportComponent> > aElements;
        if ( aSelection >>= aElements )
        {
            if ( aElements.getLength() > 0 )
                m_pMyOwnView->showProperties(uno::Reference<uno::XInterface>(aElements[0],uno::UNO_QUERY));
            m_pMyOwnView->setMarked(aElements,sal_True);
        }
        else
        {
            uno::Reference<uno::XInterface> xObject(aSelection,uno::UNO_QUERY);
            uno::Reference<report::XReportComponent> xProp(xObject,uno::UNO_QUERY);
            if ( xProp.is() )
            {
                m_pMyOwnView->showProperties(xObject);
                aElements.realloc(1);
                aElements[0] = xProp;
                m_pMyOwnView->setMarked(aElements,sal_True);
            }
            else
            {
                uno::Reference<report::XSection> xSection(aSelection,uno::UNO_QUERY);
                if ( !xSection.is() && xObject.is() )
                    m_pMyOwnView->showProperties(xObject);
                m_pMyOwnView->setMarked(xSection,xSection.is());
            }
        }
        InvalidateAll();
    }
    return bRet;
}
// -----------------------------------------------------------------------------
Any SAL_CALL OReportController::getSelection(  ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    Any aRet;
    if ( m_pMyOwnView )
    {
        aRet = m_pMyOwnView->getCurrentlyShownProperty();
        if ( !aRet.hasValue() )
            aRet <<= m_pMyOwnView->getCurrentSection();
    }
    return aRet;
}
// -----------------------------------------------------------------------------
void OReportController::createNewFunction(const uno::Any& _aValue)
{
    uno::Reference< container::XIndexContainer> xFunctions(_aValue,uno::UNO_QUERY_THROW);
    const ::rtl::OUString sNewName = String(ModuleRes(RID_STR_FUNCTION));
    uno::Reference< report::XFunction> xFunction(report::Function::create(m_xContext));
    xFunction->setName(sNewName);
    // the call below will also create an undo action -> listener
    xFunctions->insertByIndex(xFunctions->getCount(),uno::makeAny(xFunction));
}
// -----------------------------------------------------------------------------
IMPL_LINK( OReportController, OnExecuteReport, void* ,/*_pMemfun*/)
{
    //m_nExecuteReportEvent = 0;
    executeReport();
    return 0L;
}
// -----------------------------------------------------------------------------
void OReportController::createControl(const Sequence< PropertyValue >& _aArgs,const uno::Reference< report::XSection>& _xSection,const ::rtl::OUString& _sFunction,sal_uInt16 _nObjectId)
{
    SequenceAsHashMap aMap(_aArgs);
    m_pMyOwnView->setMarked(_xSection ,sal_True);
    ::boost::shared_ptr<OReportSection> pReportSection = m_pMyOwnView->getMarkedSection();
    if ( !pReportSection )
        return;

    OSL_ENSURE(pReportSection->getSection() == _xSection,"Invalid section after marking the corrct one.");

    sal_Int32 nLeftMargin = getStyleProperty<sal_Int32>(m_xReportDefinition,PROPERTY_LEFTMARGIN);
    const sal_Int32 nRightMargin = getStyleProperty<sal_Int32>(m_xReportDefinition,PROPERTY_RIGHTMARGIN);
    const sal_Int32 nPaperWidth = getStyleProperty<awt::Size>(m_xReportDefinition,PROPERTY_PAPERSIZE).Width - nRightMargin;
    awt::Point aPos = aMap.getUnpackedValueOrDefault(PROPERTY_POSITION,awt::Point(nLeftMargin,0));
    if ( aPos.X < nLeftMargin )
        aPos.X = nLeftMargin;

    SdrObject* pNewControl = NULL;
    uno::Reference< report::XReportComponent> xShapeProp;
    if ( _nObjectId == OBJ_CUSTOMSHAPE )
    {
        pNewControl = SdrObjFactory::MakeNewObject( ReportInventor, _nObjectId, pReportSection->getPage(),m_aReportModel.get() );
        xShapeProp.set(pNewControl->getUnoShape(),uno::UNO_QUERY);
        pReportSection->createDefault(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("diamond")),pNewControl); // TODO: use real custom shape type
    }
    else
    {
        SdrUnoObj* pLabel,*pControl;
        FmFormView::createControlLabelPair(NULL,m_pMyOwnView
                            ,nLeftMargin,0
                            ,NULL,NULL,_nObjectId,::rtl::OUString(),ReportInventor,OBJ_DLG_FIXEDTEXT,
                         NULL,pReportSection->getPage(),m_aReportModel.get(),
                         pLabel,pControl);

        delete pLabel;

        pNewControl = pControl;
        OUnoObject* pObj = dynamic_cast<OUnoObject*>(pControl);
        uno::Reference<beans::XPropertySet> xUnoProp(pObj->GetUnoControlModel(),uno::UNO_QUERY);
        xShapeProp.set(pObj->getUnoShape(),uno::UNO_QUERY);
        uno::Reference<beans::XPropertySetInfo> xShapeInfo = xShapeProp->getPropertySetInfo();
        uno::Reference<beans::XPropertySetInfo> xInfo = xUnoProp->getPropertySetInfo();

        if ( xInfo->hasPropertyByName(PROPERTY_NAME) )
            xUnoProp->setPropertyValue(PROPERTY_NAME,xShapeProp->getPropertyValue(PROPERTY_NAME));

        if ( xInfo->hasPropertyByName(PROPERTY_FONTDESCRIPTOR) && xShapeInfo->hasPropertyByName(PROPERTY_FONTDESCRIPTOR) )
            xUnoProp->setPropertyValue(PROPERTY_FONTDESCRIPTOR,xShapeProp->getPropertyValue(PROPERTY_FONTDESCRIPTOR));
        if ( xInfo->hasPropertyByName(PROPERTY_ORIENTATION) && xShapeInfo->hasPropertyByName(PROPERTY_ORIENTATION) )
            xUnoProp->setPropertyValue(PROPERTY_ORIENTATION,xShapeProp->getPropertyValue(PROPERTY_ORIENTATION));
        if ( xInfo->hasPropertyByName(PROPERTY_BORDER) && xShapeInfo->hasPropertyByName(PROPERTY_CONTROLBORDER) )
            xUnoProp->setPropertyValue(PROPERTY_BORDER,xShapeProp->getPropertyValue(PROPERTY_CONTROLBORDER));
        if ( xInfo->hasPropertyByName(PROPERTY_DATAFIELD) && _sFunction.getLength() )
        {
            ReportFormula aFunctionFormula( ReportFormula::Expression, _sFunction );
            xUnoProp->setPropertyValue( PROPERTY_DATAFIELD, uno::makeAny( aFunctionFormula.getCompleteFormula() ) );
        }

        if ( xInfo->hasPropertyByName(PROPERTY_FORMATSSUPPLIER) && xShapeInfo->hasPropertyByName(PROPERTY_FORMATSSUPPLIER)  )
            xUnoProp->setPropertyValue( PROPERTY_FORMATSSUPPLIER, xShapeProp->getPropertyValue( PROPERTY_FORMATSSUPPLIER) );

        sal_Int32 nFormatKey = aMap.getUnpackedValueOrDefault(PROPERTY_FORMATKEY,sal_Int32(0));
        if ( nFormatKey && xInfo->hasPropertyByName(PROPERTY_FORMATKEY) )
            xUnoProp->setPropertyValue( PROPERTY_FORMATKEY, uno::makeAny( nFormatKey ) );

        ::rtl::OUString sUrl = aMap.getUnpackedValueOrDefault(PROPERTY_IMAGEURL,::rtl::OUString());
        if ( sUrl.getLength() && xInfo->hasPropertyByName(PROPERTY_IMAGEURL) )
            xUnoProp->setPropertyValue( PROPERTY_IMAGEURL, uno::makeAny( sUrl ) );
        if ( xInfo->hasPropertyByName(PROPERTY_BACKGROUNDCOLOR) )
            xUnoProp->setPropertyValue(PROPERTY_BACKGROUNDCOLOR,xShapeProp->getPropertyValue(PROPERTY_CONTROLBACKGROUND));

        pObj->CreateMediator(sal_True);

        if ( _nObjectId == OBJ_DLG_FIXEDTEXT ) // special case for fixed text
            xUnoProp->setPropertyValue(PROPERTY_LABEL,uno::makeAny(OUnoObject::GetDefaultName(pObj)));
        else if ( _nObjectId == OBJ_DLG_VFIXEDLINE )
        {
            awt::Size aOlSize = xShapeProp->getSize();
            xShapeProp->setSize(awt::Size(aOlSize.Height,aOlSize.Width)); // switch height and width
        }
    }

    const sal_Int32 nShapeWidth = aMap.getUnpackedValueOrDefault(PROPERTY_WIDTH,xShapeProp->getWidth());
    if ( nShapeWidth != xShapeProp->getWidth() )
        xShapeProp->setWidth( nShapeWidth );

    const bool bChangedPos = (aPos.X + nShapeWidth) > nPaperWidth;
    if ( bChangedPos )
        aPos.X = nPaperWidth - nShapeWidth;
    xShapeProp->setPosition(aPos);

    correctOverlapping(pNewControl,pReportSection);
}
// -----------------------------------------------------------------------------
void OReportController::createDateTime(const Sequence< PropertyValue >& _aArgs)
{
    m_pMyOwnView->unmarkAllObjects(NULL);

    const String sUndoAction(ModuleRes(RID_STR_UNDO_INSERT_CONTROL));
    UndoManagerListAction aListAction(m_aUndoManager,sUndoAction);

    SequenceAsHashMap aMap(_aArgs);
    aMap.createItemIfMissing(PROPERTY_FORMATKEY,aMap.getUnpackedValueOrDefault(PROPERTY_FORMATKEYDATE,sal_Int32(0)));

    uno::Reference< report::XSection> xSection = aMap.getUnpackedValueOrDefault(PROPERTY_SECTION,uno::Reference< report::XSection>());
    ::rtl::OUString sFunction;

    sal_Bool bDate = aMap.getUnpackedValueOrDefault(PROPERTY_DATE_STATE,sal_False);
    if ( bDate )
    {
        sFunction = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("TODAY()"));
        createControl(aMap.getAsConstPropertyValueList(),xSection,sFunction);
    }
    sal_Bool bTime = aMap.getUnpackedValueOrDefault(PROPERTY_TIME_STATE,sal_False);
    if ( bTime )
    {
        sFunction = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("NOW()"));
        aMap[PROPERTY_FORMATKEY] <<= aMap.getUnpackedValueOrDefault(PROPERTY_FORMATKEYTIME,sal_Int32(0));
        createControl(aMap.getAsConstPropertyValueList(),xSection,sFunction);
    }
}
// -----------------------------------------------------------------------------
void OReportController::createPageNumber(const Sequence< PropertyValue >& _aArgs)
{
    m_pMyOwnView->unmarkAllObjects(NULL);

    const String sUndoAction(ModuleRes(RID_STR_UNDO_INSERT_CONTROL));
    UndoManagerListAction aListAction(m_aUndoManager,sUndoAction);

    if ( !m_xReportDefinition->getPageHeaderOn() )
    {
        uno::Sequence< beans::PropertyValue > aArgs;
        executeChecked(SID_PAGEHEADERFOOTER,aArgs);
    } // if ( !m_xHoldAlive->getPageHeaderOn() )

    SequenceAsHashMap aMap(_aArgs);
    sal_Bool bStateOfPage = aMap.getUnpackedValueOrDefault(PROPERTY_STATE,sal_False);

    String sFunction = String(ModuleRes(STR_RPT_PN_PAGE));
    ::rtl::OUString sPageNumber(RTL_CONSTASCII_USTRINGPARAM("PageNumber()"));
    sFunction.SearchAndReplace(String::CreateFromAscii("#PAGENUMBER#"),sPageNumber);

    if ( bStateOfPage )
    {
        ::rtl::OUString sPageCount(RTL_CONSTASCII_USTRINGPARAM("PageCount()"));
        sFunction += String(ModuleRes(STR_RPT_PN_PAGE_OF));
        sFunction.SearchAndReplace(String::CreateFromAscii("#PAGECOUNT#"),sPageCount);
    }

    sal_Bool bInPageHeader = aMap.getUnpackedValueOrDefault(PROPERTY_PAGEHEADERON,sal_True);
    createControl(_aArgs,bInPageHeader ? m_xReportDefinition->getPageHeader() : m_xReportDefinition->getPageFooter(),sFunction);
}

// -----------------------------------------------------------------------------
void OReportController::addPairControls(const Sequence< PropertyValue >& aArgs)
{
    m_pMyOwnView->unmarkAllObjects(NULL);
    //////////////////////////////////////////////////////////////////////
    // Anhand des FormatKeys wird festgestellt, welches Feld benoetigt wird
    ::boost::shared_ptr<OReportSection> pReportSection[2];
    pReportSection[0] = m_pMyOwnView->getMarkedSection();

    if ( !pReportSection[0].get() )
        return;

    uno::Reference<report::XSection> xCurrentSection = m_pMyOwnView->getCurrentSection();
    UndoManagerListAction aUndo( *getUndoMgr(), String( ModuleRes( RID_STR_UNDO_INSERT_CONTROL ) ) );

    try
    {
        bool bHandleOnlyOne = false;
        const PropertyValue* pIter = aArgs.getConstArray();
        const PropertyValue* pEnd  = pIter + aArgs.getLength();
        for(;pIter != pEnd && !bHandleOnlyOne;++pIter)
        {
            Sequence< PropertyValue > aValue;
            if ( !(pIter->Value >>= aValue) )
            {   // the sequence has only one element which already contains the descriptor
                bHandleOnlyOne = true;
                aValue = aArgs;
            }
            ::svx::ODataAccessDescriptor aDescriptor(aValue);
            SequenceAsHashMap aMap(aValue);
            uno::Reference<report::XSection> xSection = aMap.getUnpackedValueOrDefault(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Section")),xCurrentSection);
            uno::Reference<report::XReportDefinition> xReportDefinition = xSection->getReportDefinition();

            m_pMyOwnView->setMarked(xSection,sal_True);
            pReportSection[0] = m_pMyOwnView->getMarkedSection();

            sal_Int32 nLeftMargin = getStyleProperty<sal_Int32>(m_xReportDefinition,PROPERTY_LEFTMARGIN);
            awt::Point aPos = aMap.getUnpackedValueOrDefault(PROPERTY_POSITION,awt::Point(nLeftMargin,0));
            if ( aPos.X < nLeftMargin )
                aPos.X = nLeftMargin;

            // LLA: new feature, add the Label in dependency of the given DND_ACTION one section up, normal or one section down
            sal_Int8 nDNDAction = aMap.getUnpackedValueOrDefault(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DNDAction")), sal_Int8(0));
            pReportSection[1] = pReportSection[0];
            // ::boost::shared_ptr<OReportSection> pReportSectionPost;
            sal_Bool bLabelAboveTextField = nDNDAction == DND_ACTION_COPY;
            if ( bLabelAboveTextField || nDNDAction == DND_ACTION_LINK )
            {
                // Add the Label one Section up
                pReportSection[1] = m_pMyOwnView->getMarkedSection(bLabelAboveTextField ? PREVIOUS : POST);
                if (!pReportSection[1].get())
                {
                    // maybe out of bounds
                    pReportSection[1] = pReportSection[0];
                }
            }
            // clear all selections
            m_pMyOwnView->unmarkAllObjects(NULL);

            uno::Reference< beans::XPropertySet > xField( aDescriptor[ ::svx::daColumnObject ], uno::UNO_QUERY );
            uno::Reference< lang::XComponent > xHoldAlive;
            if ( !xField.is() )
            {
                ::rtl::OUString sCommand;
                ::rtl::OUString sColumnName;
                sal_Int32 nCommandType( -1 );
                OSL_VERIFY( aDescriptor[ ::svx::daCommand ] >>= sCommand );
                OSL_VERIFY( aDescriptor[ ::svx::daColumnName ] >>= sColumnName );
                OSL_VERIFY( aDescriptor[ ::svx::daCommandType ] >>= nCommandType );

                uno::Reference< container::XNameAccess > xColumns;
                uno::Reference< sdbc::XConnection > xConnection( getConnection() );
                if ( sCommand.getLength() && nCommandType != -1 && sColumnName.getLength() && xConnection.is() )
                {
                    if ( !xReportDefinition->getCommand().getLength() )
                    {
                        xReportDefinition->setCommand(sCommand);
                        xReportDefinition->setCommandType(nCommandType);
                    } // if ( !xReportDefinition->getCommand().getLength() )

                    xColumns = dbtools::getFieldsByCommandDescriptor(xConnection,nCommandType,sCommand,xHoldAlive);
                    if ( xColumns.is() && xColumns->hasByName(sColumnName) )
                        xField.set( xColumns->getByName( sColumnName ), uno::UNO_QUERY );
                }

                if ( !xField.is() )
                {
                #if OSL_DEBUG_LEVEL > 0
                    try
                    {
                        uno::Reference< beans::XPropertySet > xRowSetProps( getRowSet(), UNO_QUERY_THROW );
                        ::rtl::OUString sRowSetCommand;
                        sal_Int32 nRowSetCommandType( -1 );
                        OSL_VERIFY( xRowSetProps->getPropertyValue( PROPERTY_COMMAND ) >>= sRowSetCommand );
                        OSL_VERIFY( xRowSetProps->getPropertyValue( PROPERTY_COMMANDTYPE ) >>= nRowSetCommandType );
                        OSL_ENSURE( ( sRowSetCommand == sCommand ) && ( nCommandType == nRowSetCommandType ),
                            "OReportController::addPairControls: this only works for a data source which equals our current settings!" );
                        // if this asserts, then either our row set and our report definition are not in sync, or somebody
                        // requested the creation of a control/pair for another data source than what our report
                        // definition is bound to - which is not supported for the parameters case, since we
                        // can retrieve parameters from the RowSet only.
                    }
                    catch( const Exception& )
                    {
                        DBG_UNHANDLED_EXCEPTION();
                    }
                #endif

                    // no column name - perhaps a parameter name?
                    uno::Reference< sdb::XParametersSupplier > xSuppParam( getRowSet(), uno::UNO_QUERY_THROW );
                    uno::Reference< container::XIndexAccess > xParams( xSuppParam->getParameters(), uno::UNO_QUERY_THROW );
                    sal_Int32 nParamCount( xParams->getCount() );
                    for ( sal_Int32 i=0; i<nParamCount; ++i)
                    {
                        uno::Reference< beans::XPropertySet > xParamCol( xParams->getByIndex(i), uno::UNO_QUERY_THROW );
                        ::rtl::OUString sParamName;
                        OSL_VERIFY( xParamCol->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Name" ) ) ) >>= sParamName );
                        if ( sParamName == sColumnName )
                        {
                            xField = xParamCol;
                            break;
                        }
                    }
                }
            }
            if ( !xField.is() )
                continue;

            sal_uInt16 nOBJID = 0;
            sal_Int32 nDataType = sdbc::DataType::BINARY;
            xField->getPropertyValue(PROPERTY_TYPE) >>= nDataType;
            switch ( nDataType )
            {
                case sdbc::DataType::BINARY:
                case sdbc::DataType::VARBINARY:
                case sdbc::DataType::LONGVARBINARY:
                    nOBJID = OBJ_DLG_IMAGECONTROL;
                    break;
                default:
                    nOBJID = OBJ_DLG_FORMATTEDFIELD;
                    break;
            }

            if ( !nOBJID )
                continue;

            Reference< util::XNumberFormatsSupplier >  xSupplier = getReportNumberFormatter()->getNumberFormatsSupplier();
            if ( !xSupplier.is() )
                continue;

            Reference< XNumberFormats >  xNumberFormats(xSupplier->getNumberFormats());
            SdrUnoObj* pControl[2];
            pControl[0] = NULL;
            pControl[1] = NULL;
            //m_pMyOwnView->GetModel()->GetUndoEnv().Lock();
            const sal_Int32 nRightMargin = getStyleProperty<sal_Int32>(m_xReportDefinition,PROPERTY_RIGHTMARGIN);
            const sal_Int32 nPaperWidth = getStyleProperty<awt::Size>(m_xReportDefinition,PROPERTY_PAPERSIZE).Width - nRightMargin;
            OSectionView* pSectionViews[2];
            pSectionViews[0] = pReportSection[1]->getView();
            pSectionViews[1] = pReportSection[0]->getView();
            // find this in svx
            FmFormView::createControlLabelPair(pSectionViews[0],m_pMyOwnView
                ,nLeftMargin,0
                ,xField,xNumberFormats,nOBJID,::rtl::OUString(),ReportInventor,OBJ_DLG_FIXEDTEXT,
                pReportSection[1]->getPage(),pReportSection[0]->getPage(),m_aReportModel.get(),
                pControl[0],pControl[1]);
            //m_pMyOwnView->GetModel()->GetUndoEnv().UnLock();
            if ( pControl[0] && pControl[1] )
            {
                SdrPageView* pPgViews[2];
                pPgViews[0] = pSectionViews[0]->GetSdrPageView();
                pPgViews[1] = pSectionViews[1]->GetSdrPageView();
                if ( pPgViews[0] && pPgViews[1] )
                {
                    ::rtl::OUString sDefaultName;
                    size_t i = 0;
                    OUnoObject* pObjs[2];
                    for(i = 0; i < sizeof(pControl)/sizeof(pControl[0]);++i)
                    {
                        pObjs[i] = dynamic_cast<OUnoObject*>(pControl[i]);
                        uno::Reference<beans::XPropertySet> xUnoProp(pObjs[i]->GetUnoControlModel(),uno::UNO_QUERY_THROW);
                        uno::Reference< report::XReportComponent> xShapeProp(pObjs[i]->getUnoShape(),uno::UNO_QUERY_THROW);
                        xUnoProp->setPropertyValue(PROPERTY_NAME,xShapeProp->getPropertyValue(PROPERTY_NAME));

                        uno::Reference<beans::XPropertySetInfo> xInfo = xUnoProp->getPropertySetInfo();
                        if ( xInfo->hasPropertyByName(PROPERTY_FONTDESCRIPTOR) )
                            xUnoProp->setPropertyValue(PROPERTY_FONTDESCRIPTOR,xShapeProp->getPropertyValue(PROPERTY_FONTDESCRIPTOR));
                        if ( xInfo->hasPropertyByName(PROPERTY_BORDER) )
                            xUnoProp->setPropertyValue(PROPERTY_BORDER,xShapeProp->getPropertyValue(PROPERTY_CONTROLBORDER));
                        if ( xInfo->hasPropertyByName(PROPERTY_DATAFIELD) )
                        {
                            ::rtl::OUString sName;
                            xUnoProp->getPropertyValue(PROPERTY_DATAFIELD) >>= sName;
                            sDefaultName = sName;
                            xUnoProp->setPropertyValue(PROPERTY_NAME,uno::makeAny(sDefaultName));

                            ReportFormula aFormula( ReportFormula::Field, sName );
                            xUnoProp->setPropertyValue( PROPERTY_DATAFIELD, uno::makeAny( aFormula.getCompleteFormula() ) );
                        }
                        if ( xInfo->hasPropertyByName(PROPERTY_BACKGROUNDCOLOR) )
                            xUnoProp->setPropertyValue(PROPERTY_BACKGROUNDCOLOR,xShapeProp->getPropertyValue(PROPERTY_CONTROLBACKGROUND));

                        pObjs[i]->CreateMediator(sal_True);
                        // need SectionView from the above or follow Section
                        // (getMarkedSection) returns the current Section
                        //pSectionViews[i]->InsertObjectAtView(pControl[i],*pPgViews[i],SDRINSERT_ADDMARK);

                        const sal_Int32 nShapeWidth = xShapeProp->getWidth();
                        const bool bChangedPos = (aPos.X + nShapeWidth) > nPaperWidth;
                        if ( bChangedPos )
                            aPos.X = nPaperWidth - nShapeWidth;
                        xShapeProp->setPosition(aPos);
                        if ( bChangedPos )
                            aPos.Y += xShapeProp->getHeight();
                        aPos.X += nShapeWidth;
                    }
                    if (pSectionViews[0] != pSectionViews[1] &&
                        nOBJID == OBJ_DLG_FORMATTEDFIELD) // we want this nice feature only at FORMATTEDFIELD
                    {
                        // we have two different Views, so set the position x new.
                        // pSectionViews[1].position.x = pSectionViews[0].position.x
                        uno::Reference< report::XReportComponent> xShapePropLabel(pObjs[0]->getUnoShape(),uno::UNO_QUERY_THROW);
                        uno::Reference< report::XReportComponent> xShapePropTextField(pObjs[1]->getUnoShape(),uno::UNO_QUERY_THROW);
                        awt::Point aPosLabel = xShapePropLabel->getPosition();
                        awt::Point aPosTextField = xShapePropTextField->getPosition();
                        aPosTextField.X = aPosLabel.X;
                        xShapePropTextField->setPosition(aPosTextField);
                        if (bLabelAboveTextField)
                        {
                            // move the label down near the splitter
                            const uno::Reference<report::XSection> xLabelSection = pReportSection[1]->getSection();
                            aPosLabel.Y = xLabelSection->getHeight() - xShapePropLabel->getHeight();
                        }
                        else
                        {
                            // move the label up to the splitter
                            aPosLabel.Y = 0;
                        }
                        xShapePropLabel->setPosition(aPosLabel);
                    }
                    OUnoObject* pObj = dynamic_cast<OUnoObject*>(pControl[0]);
                    uno::Reference< report::XReportComponent> xShapeProp(pObj->getUnoShape(),uno::UNO_QUERY_THROW);
                    xShapeProp->setName(xShapeProp->getName() + sDefaultName );

                    for(i = 0; i < sizeof(pControl)/sizeof(pControl[0]);++i) // insert controls
                    {
                        correctOverlapping(pControl[i],pReportSection[1-i]);
                    }

                    if (!bLabelAboveTextField )
                    {
                        if ( pSectionViews[0] == pSectionViews[1] )
                        {
                            Rectangle aLabel = getRectangleFromControl(pControl[0]);
                            Rectangle aTextfield = getRectangleFromControl(pControl[1]);

                            // create a Union of the given Label and Textfield
                            Rectangle aLabelAndTextfield( aLabel );
                            aLabelAndTextfield.Union(aTextfield);

                            // check if there exists other fields and if yes, move down
                            bool bOverlapping = true;
                            bool bHasToMove = false;
                            while ( bOverlapping )
                            {
                                const SdrObject* pOverlappedObj = isOver(aLabelAndTextfield, *pReportSection[0]->getPage(), *pSectionViews[0], true, pControl, 2);
                                bOverlapping = pOverlappedObj != NULL;
                                if ( bOverlapping )
                                {
                                    const Rectangle& aLogicRect = pOverlappedObj->GetLogicRect();
                                    aLabelAndTextfield.Move(0,aLogicRect.Top() + aLogicRect.getHeight() - aLabelAndTextfield.Top());
                                    bHasToMove = true;
                                }
                            }

                            if (bHasToMove)
                            {
                                // There was a move down, we need to move the Label and the Textfield down
                                aLabel.Move(0, aLabelAndTextfield.Top() - aLabel.Top());
                                aTextfield.Move(0, aLabelAndTextfield.Top() - aTextfield.Top());

                                uno::Reference< report::XReportComponent> xLabel(pControl[0]->getUnoShape(),uno::UNO_QUERY_THROW);
                                xLabel->setPositionY(aLabel.Top());

                                uno::Reference< report::XReportComponent> xTextfield(pControl[1]->getUnoShape(),uno::UNO_QUERY_THROW);
                                xTextfield->setPositionY(aTextfield.Top());
                        }
                    }
                        // this should never happen.
                        // else
                        // {
                        //  DBG_ERROR("unhandled case.");
                        // }
                    }
                }
            }
            else
            {
                for(size_t i = 0; i < sizeof(pControl)/sizeof(pControl[0]);++i)
                    delete pControl[i];
            }
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

// -----------------------------------------------------------------------------
OSectionView* OReportController::getCurrentSectionView() const
{
    OSectionView* pSectionView = NULL;
    ::boost::shared_ptr<OReportSection> pReportSection = m_pMyOwnView->getMarkedSection();
    if ( pReportSection.get() )
        pSectionView = pReportSection->getView();
    return pSectionView;
}
// -----------------------------------------------------------------------------
void OReportController::changeZOrder(sal_Int32 _nId)
{
    OSectionView* pSectionView = getCurrentSectionView();
    if ( pSectionView )
    {
        switch(_nId)
        {
            case SID_FRAME_TO_TOP:
                pSectionView->MovMarkedToTop();
                break;
            case SID_FRAME_DOWN:
                break;
            case SID_FRAME_UP:
                break;
            case SID_FRAME_TO_BOTTOM:
                pSectionView->MovMarkedToBtm();
                break;
            case SID_OBJECT_HEAVEN:
                pSectionView->PutMarkedToTop();
                break;
            case SID_OBJECT_HELL:
                pSectionView->PutMarkedToBtm();
                break;
        }
    }
}
// -----------------------------------------------------------------------------
void OReportController::listen(const bool _bAdd)
{
    const ::rtl::OUString aProps [] = {    PROPERTY_REPORTHEADERON,PROPERTY_REPORTFOOTERON
                                            ,PROPERTY_PAGEHEADERON,PROPERTY_PAGEFOOTERON
                                            ,PROPERTY_COMMAND, PROPERTY_COMMANDTYPE,PROPERTY_CAPTION
    };

    void (SAL_CALL XPropertySet::*pPropertyListenerAction)( const ::rtl::OUString&, const uno::Reference< XPropertyChangeListener >& ) =
        _bAdd ? &XPropertySet::addPropertyChangeListener : &XPropertySet::removePropertyChangeListener;

    for (size_t i = 0; i < sizeof(aProps)/sizeof(aProps[0]); ++i)
        (m_xReportDefinition.get()->*pPropertyListenerAction)( aProps[i], static_cast< XPropertyChangeListener* >( this ) );

    OXUndoEnvironment& rUndoEnv = m_aReportModel->GetUndoEnv();
    uno::Reference< XPropertyChangeListener > xUndo = &rUndoEnv;
    uno::Sequence< beans::Property> aSeq = m_xReportDefinition->getPropertySetInfo()->getProperties();
    const beans::Property* pIter = aSeq.getConstArray();
    const beans::Property* pEnd   = pIter + aSeq.getLength();
    const ::rtl::OUString* pPropsBegin = &aProps[0];
    const ::rtl::OUString* pPropsEnd   = pPropsBegin + (sizeof(aProps)/sizeof(aProps[0])) - 3;
    for(;pIter != pEnd;++pIter)
    {
        if ( ::std::find(pPropsBegin,pPropsEnd,pIter->Name) == pPropsEnd )
            (m_xReportDefinition.get()->*pPropertyListenerAction)( pIter->Name, xUndo );
    }

    void (OXUndoEnvironment::*pElementUndoFunction)( const uno::Reference< uno::XInterface >& ) =
        _bAdd ? &OXUndoEnvironment::AddElement : &OXUndoEnvironment::RemoveElement;

    (rUndoEnv.*pElementUndoFunction)( m_xReportDefinition->getStyleFamilies() );
    (rUndoEnv.*pElementUndoFunction)( m_xReportDefinition->getFunctions() );

    if ( m_xReportDefinition->getPageHeaderOn() && _bAdd )
        m_pMyOwnView->addSection(m_xReportDefinition->getPageHeader(),DBPAGEHEADER);
    if ( m_xReportDefinition->getReportHeaderOn() && _bAdd )
        m_pMyOwnView->addSection(m_xReportDefinition->getReportHeader(),DBREPORTHEADER);

    uno::Reference< report::XGroups > xGroups = m_xReportDefinition->getGroups();
    const sal_Int32 nCount = xGroups->getCount();
    _bAdd ? xGroups->addContainerListener(&rUndoEnv) : xGroups->removeContainerListener(&rUndoEnv);

    for (sal_Int32 i=0;i<nCount ; ++i)
    {
        uno::Reference< report::XGroup > xGroup(xGroups->getByIndex(i),uno::UNO_QUERY);
        (xGroup.get()->*pPropertyListenerAction)( PROPERTY_HEADERON, static_cast< XPropertyChangeListener* >( this ) );
        (xGroup.get()->*pPropertyListenerAction)( PROPERTY_FOOTERON, static_cast< XPropertyChangeListener* >( this ) );

        (rUndoEnv.*pElementUndoFunction)( xGroup );
        (rUndoEnv.*pElementUndoFunction)( xGroup->getFunctions() );
        if ( xGroup->getHeaderOn() && _bAdd )
            m_pMyOwnView->addSection(xGroup->getHeader(),DBGROUPHEADER);
    } // for (sal_Int32 i=0;i<nCount ; ++i)

    if ( _bAdd )
    {
        m_pMyOwnView->addSection(m_xReportDefinition->getDetail(),DBDETAIL);

        for (sal_Int32 i=nCount;i > 0 ; --i)
        {
            uno::Reference< report::XGroup > xGroup(xGroups->getByIndex(i-1),uno::UNO_QUERY);
            if ( xGroup->getFooterOn() )
                m_pMyOwnView->addSection(xGroup->getFooter(),DBGROUPFOOTER);
        }
        if ( m_xReportDefinition->getReportFooterOn() )
            m_pMyOwnView->addSection(m_xReportDefinition->getReportFooter(),DBREPORTFOOTER);
        if ( m_xReportDefinition->getPageFooterOn())
            m_pMyOwnView->addSection(m_xReportDefinition->getPageFooter(),DBPAGEFOOTER);
    }



    _bAdd ? xGroups->addContainerListener(static_cast<XContainerListener*>(this))
        : xGroups->removeContainerListener(static_cast<XContainerListener*>(this));
    _bAdd ? m_xReportDefinition->addModifyListener(static_cast<XModifyListener*>(this))
        : m_xReportDefinition->removeModifyListener(static_cast<XModifyListener*>(this));

    if ( !_bAdd )
        m_aReportModel->detachController();
}
// -----------------------------------------------------------------------------
void OReportController::switchReportSection(const sal_Int16 _nId)
{
    OSL_ENSURE(_nId == SID_REPORTHEADER_WITHOUT_UNDO || _nId == SID_REPORTFOOTER_WITHOUT_UNDO || _nId == SID_REPORTHEADERFOOTER ,"Illegal id given!");

    if ( m_xReportDefinition.is() )
    {
        const OXUndoEnvironment::OUndoEnvLock aLock( m_aReportModel->GetUndoEnv() );
        const bool bSwitchOn = !m_xReportDefinition->getReportHeaderOn();
        if ( SID_REPORTHEADERFOOTER == _nId )
        {
            const String sUndoAction(ModuleRes(bSwitchOn ? RID_STR_UNDO_ADD_REPORTHEADERFOOTER : RID_STR_UNDO_REMOVE_REPORTHEADERFOOTER));
            getUndoMgr()->EnterListAction( sUndoAction, String() );

            addUndoActionAndInvalidate(new OReportSectionUndo(*(m_aReportModel),SID_REPORTHEADER_WITHOUT_UNDO
                                                            ,::std::mem_fun(&OReportHelper::getReportHeader)
                                                            ,m_xReportDefinition
                                                            ,bSwitchOn ? Inserted : Removed
                                                            ,0
                                                            ));

            addUndoActionAndInvalidate(new OReportSectionUndo(*(m_aReportModel),SID_REPORTFOOTER_WITHOUT_UNDO
                                                            ,::std::mem_fun(&OReportHelper::getReportFooter)
                                                            ,m_xReportDefinition
                                                            ,bSwitchOn ? Inserted : Removed
                                                            ,0
                                                            ));
        }

        switch( _nId )
        {
            case SID_REPORTHEADER_WITHOUT_UNDO:
                m_xReportDefinition->setReportHeaderOn( bSwitchOn );
                break;
            case SID_REPORTFOOTER_WITHOUT_UNDO:
                m_xReportDefinition->setReportFooterOn( !m_xReportDefinition->getReportFooterOn() );
                break;
            case SID_REPORTHEADERFOOTER:
                m_xReportDefinition->setReportHeaderOn( bSwitchOn );
                m_xReportDefinition->setReportFooterOn( bSwitchOn );
                break;
        }

        if ( SID_REPORTHEADERFOOTER == _nId )
            getUndoMgr()->LeaveListAction();
        getView()->Resize();
    }
}
// -----------------------------------------------------------------------------
void OReportController::switchPageSection(const sal_Int16 _nId)
{
    OSL_ENSURE(_nId == SID_PAGEHEADERFOOTER || _nId == SID_PAGEHEADER_WITHOUT_UNDO || _nId == SID_PAGEFOOTER_WITHOUT_UNDO ,"Illegal id given!");
    if ( m_xReportDefinition.is() )
    {
        const OXUndoEnvironment::OUndoEnvLock aLock( m_aReportModel->GetUndoEnv() );
        const bool bSwitchOn = !m_xReportDefinition->getPageHeaderOn();

        if ( SID_PAGEHEADERFOOTER == _nId )
        {
            const String sUndoAction(ModuleRes(bSwitchOn ? RID_STR_UNDO_ADD_REPORTHEADERFOOTER : RID_STR_UNDO_REMOVE_REPORTHEADERFOOTER));
            getUndoMgr()->EnterListAction( sUndoAction, String() );

            addUndoActionAndInvalidate(new OReportSectionUndo(*m_aReportModel
                                                            ,SID_PAGEHEADER_WITHOUT_UNDO
                                                            ,::std::mem_fun(&OReportHelper::getPageHeader)
                                                            ,m_xReportDefinition
                                                            ,bSwitchOn ? Inserted : Removed
                                                            ,0
                                                            ));

            addUndoActionAndInvalidate(new OReportSectionUndo(*m_aReportModel
                                                            ,SID_PAGEFOOTER_WITHOUT_UNDO
                                                            ,::std::mem_fun(&OReportHelper::getPageFooter)
                                                            ,m_xReportDefinition
                                                            ,bSwitchOn ? Inserted : Removed
                                                            ,0
                                                            ));
        } // if ( SID_PAGEHEADERFOOTER == _nId )
        switch( _nId )
        {
            case SID_PAGEHEADER_WITHOUT_UNDO:
                m_xReportDefinition->setPageHeaderOn( bSwitchOn );
                break;
            case SID_PAGEFOOTER_WITHOUT_UNDO:
                m_xReportDefinition->setPageFooterOn( !m_xReportDefinition->getPageFooterOn() );
                break;
            case SID_PAGEHEADERFOOTER:
                m_xReportDefinition->setPageHeaderOn( bSwitchOn );
                m_xReportDefinition->setPageFooterOn( bSwitchOn );
                break;
        }
        if ( SID_PAGEHEADERFOOTER == _nId )
            getUndoMgr()->LeaveListAction();
        getView()->Resize();
    }
}
// -----------------------------------------------------------------------------
void OReportController::modifyGroup(const bool _bAppend, const Sequence< PropertyValue >& _aArgs)
{
    if ( !m_xReportDefinition.is() )
        return;

    try
    {
        const SequenceAsHashMap aMap( _aArgs );
        uno::Reference< report::XGroup > xGroup = aMap.getUnpackedValueOrDefault( PROPERTY_GROUP, uno::Reference< report::XGroup >() );
        if ( !xGroup.is() )
            return;

        OXUndoEnvironment& rUndoEnv = m_aReportModel->GetUndoEnv();
        uno::Reference< report::XGroups > xGroups = m_xReportDefinition->getGroups();
        if ( _bAppend )
        {
            const sal_Int32 nPos = aMap.getUnpackedValueOrDefault( PROPERTY_POSITIONY, xGroups->getCount() );
            xGroups->insertByIndex( nPos, uno::makeAny( xGroup ) );
            rUndoEnv.AddElement( xGroup->getFunctions() );
        }

        addUndoActionAndInvalidate( new OGroupUndo(
            *m_aReportModel,
            _bAppend ? RID_STR_UNDO_APPEND_GROUP : RID_STR_UNDO_REMOVE_GROUP,
            _bAppend ? Inserted : Removed,
            xGroup,
            m_xReportDefinition
        ) );

        if ( !_bAppend )
        {
            rUndoEnv.RemoveElement( xGroup->getFunctions() );
            const sal_Int32 nPos = getGroupPosition( xGroup );
            const OXUndoEnvironment::OUndoEnvLock aLock( m_aReportModel->GetUndoEnv() );
            xGroups->removeByIndex( nPos );
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

// -----------------------------------------------------------------------------
void OReportController::createGroupSection(const bool _bUndo,const bool _bHeader, const Sequence< PropertyValue >& _aArgs)
{
    if ( m_xReportDefinition.is() )
    {
        const SequenceAsHashMap aMap(_aArgs);
        const sal_Bool bSwitchOn = aMap.getUnpackedValueOrDefault(_bHeader ? PROPERTY_HEADERON : PROPERTY_FOOTERON,sal_False);
        uno::Reference< report::XGroup> xGroup = aMap.getUnpackedValueOrDefault(PROPERTY_GROUP,uno::Reference< report::XGroup>());
        if ( xGroup.is() )
        {
            const OXUndoEnvironment::OUndoEnvLock aLock(m_aReportModel->GetUndoEnv());
            if ( _bUndo )
                addUndoActionAndInvalidate(new OGroupSectionUndo(*m_aReportModel
                                                                ,_bHeader ? SID_GROUPHEADER_WITHOUT_UNDO : SID_GROUPFOOTER_WITHOUT_UNDO
                                                                ,_bHeader ? ::std::mem_fun(&OGroupHelper::getHeader) : ::std::mem_fun(&OGroupHelper::getFooter)
                                                                ,xGroup
                                                                ,bSwitchOn ? Inserted : Removed
                                                                , ( _bHeader ?
                                                                        (bSwitchOn ? RID_STR_UNDO_ADD_GROUP_HEADER : RID_STR_UNDO_REMOVE_GROUP_HEADER)
                                                                       :(bSwitchOn ? RID_STR_UNDO_ADD_GROUP_FOOTER : RID_STR_UNDO_REMOVE_GROUP_FOOTER)
                                                                  )
                                                                ));

            if ( _bHeader )
                xGroup->setHeaderOn( bSwitchOn );
            else
                xGroup->setFooterOn( bSwitchOn );
        }
    }
}
// -----------------------------------------------------------------------------
void OReportController::markSection(const bool _bNext)
{
    ::boost::shared_ptr<OReportSection> pSection = m_pMyOwnView->getMarkedSection();
    if ( pSection )
    {
        ::boost::shared_ptr<OReportSection> pPrevSection = m_pMyOwnView->getMarkedSection(_bNext ? POST : PREVIOUS);
        if ( pPrevSection != pSection && pPrevSection )
            select(uno::makeAny(pPrevSection->getSection()));
        else
            select(uno::makeAny(m_xReportDefinition));
    }
    else
    {
        m_pMyOwnView->markSection(_bNext ? 0 : m_pMyOwnView->getSectionCount() - 1);
        pSection = m_pMyOwnView->getMarkedSection();
        if ( pSection )
            select(uno::makeAny(pSection->getSection()));
    }
}
// -----------------------------------------------------------------------------
void OReportController::createDefaultControl(const uno::Sequence< beans::PropertyValue>& _aArgs)
{
    uno::Reference< report::XSection > xSection = m_pMyOwnView->getCurrentSection();
    if ( xSection.is() )
    {
        const ::rtl::OUString sKeyModifier(RTL_CONSTASCII_USTRINGPARAM("KeyModifier"));
        const beans::PropertyValue* pIter = _aArgs.getConstArray();
        const beans::PropertyValue* pEnd  = pIter + _aArgs.getLength();
        const beans::PropertyValue* pFind = ::std::find_if(pIter,pEnd,::std::bind2nd(PropertyValueCompare(),boost::cref(sKeyModifier)));
        sal_Int16 nKeyModifier = 0;
        if ( pFind != pEnd && (pFind->Value >>= nKeyModifier) && nKeyModifier == KEY_MOD1 )
        {
            Sequence< PropertyValue > aCreateArgs;
            m_pMyOwnView->unmarkAllObjects(NULL);
            createControl(aCreateArgs,xSection,::rtl::OUString(),m_pMyOwnView->GetInsertObj());
        }
    }
}
// -----------------------------------------------------------------------------
uno::Reference< util::XNumberFormatter > OReportController::getReportNumberFormatter() const
{
    return m_xFormatter;
}
// -----------------------------------------------------------------------------
void OReportController::checkChartEnabled()
{
    if ( !m_bChartEnabledAsked )
    {
        m_bChartEnabledAsked = true;
        const ::rtl::OUString sConfigName( RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.ReportDesign" ) );
        const ::rtl::OUString sPropertyName( RTL_CONSTASCII_USTRINGPARAM( "UserData/Chart" ) );

        try
        {
            ::utl::OConfigurationTreeRoot aConfiguration(
                ::utl::OConfigurationTreeRoot::createWithServiceFactory( m_xServiceFactory, sConfigName ) );

            sal_Bool bChartEnabled = sal_False;
            if ( aConfiguration.hasByHierarchicalName(sPropertyName) )
                aConfiguration.getNodeValue( sPropertyName ) >>= bChartEnabled;
            m_bChartEnabled = bChartEnabled;
        }
        catch(const Exception&)
        {
        }
    }
}

// css.frame.XTitle
::rtl::OUString SAL_CALL OReportController::getTitle()
    throw (uno::RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard aGuard(m_aMutex);

    uno::Reference< frame::XTitle> xTitle(m_xReportDefinition,uno::UNO_QUERY_THROW);

    return xTitle->getTitle ();
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

