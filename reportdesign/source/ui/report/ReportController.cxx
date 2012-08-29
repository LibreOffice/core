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


#include "ReportController.hxx"
#include "ReportDefinition.hxx"
#include "CondFormat.hxx"
#include "UITools.hxx"
#include "AddField.hxx"
#include <toolkit/helper/vclunohelper.hxx>
#include "DateTime.hxx"

#include <sfx2/filedlghelper.hxx>
#include <tools/string.hxx>
#include <tools/diagnose_ex.h>
#include "rptui_slotid.hrc"
#include "reportformula.hxx"

#include <comphelper/componentcontext.hxx>
#include <comphelper/documentconstants.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <comphelper/property.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/types.hxx>

#include <connectivity/dbtools.hxx>
#include <com/sun/star/view/PaperFormat.hpp>
#include <com/sun/star/style/GraphicLocation.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/style/PageStyleLayout.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/util/NumberFormatter.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/report/XImageControl.hpp>
#include <com/sun/star/report/XFixedLine.hpp>
#include <com/sun/star/report/Function.hpp>
#include <com/sun/star/report/XShape.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/sdb/XDocumentDataSource.hpp>
#include <com/sun/star/sdb/XParametersSupplier.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/EmbedMapUnits.hpp>
#include <comphelper/streamsection.hxx>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/TextAlign.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/frame/status/FontHeight.hpp>
#include <com/sun/star/report/XFormattedField.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdbc/SQLWarning.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/document/XUndoManagerSupplier.hpp>

#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/waitobj.hxx>

#include <svx/fmview.hxx>
#include <editeng/paperinf.hxx>
#include <editeng/memberids.hrc>
#include <svx/svxids.hrc>
#include <svx/svdobj.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include <editeng/svxenum.hxx>
#include <svx/pageitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/sizeitem.hxx>
#include <sfx2/zoomitem.hxx>
#include <svx/zoomslideritem.hxx>
#include <editeng/brshitem.hxx>
#include <svx/flagsdef.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svxdlg.hxx>
#include <svx/zoom_def.hxx>
#include <svx/dialogs.hrc>

#include "DesignView.hxx"
#include "ModuleHelper.hxx"
#include "RptObject.hxx"
#include "RptUndo.hxx"
#include "uistrings.hrc"
#include "RptDef.hxx"
#include "ReportSection.hxx"
#include "SectionView.hxx"
#include "UndoActions.hxx"
#include "dlgpage.hxx"
#include "RptResId.hrc"

#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <svl/aeitem.hxx>
#include <svtools/cliplistener.hxx>
#include <unotools/syslocale.hxx>
#include <unotools/viewoptions.hxx>

#include <osl/mutex.hxx>
#include "PropertyForward.hxx"
#include "SectionWindow.hxx"

#include <toolkit/helper/convert.hxx>
#include "GroupsSorting.hxx"
#include "PageNumber.hxx"
#include <toolkit/helper/vclunohelper.hxx>
#include "UndoEnv.hxx"
#include "ReportControllerObserver.hxx"

#include <boost/mem_fn.hpp>
#include <boost/bind.hpp>
#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>

#include <cppuhelper/exc_hlp.hxx>
#include <unotools/confignode.hxx>
#include <helpids.hrc>

#include <ReportControllerObserver.hxx>

#define MAX_ROWS_FOR_PREVIEW    20

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
using namespace ::cppu;

// -----------------------------------------------------------------------------
namespace
{
    // comparing two PropertyValue instances
    struct PropertyValueCompare : public ::std::binary_function< beans::PropertyValue, ::rtl::OUString , bool >
    {
        bool operator() (const beans::PropertyValue& x, const ::rtl::OUString& y) const
        {
            return x.Name.equals(y);
        }
        bool operator() (const ::rtl::OUString& x,const beans::PropertyValue& y) const
        {
            return x.equals(y.Name);
        }
    };

    void lcl_setFontWPU_nothrow(const uno::Reference< report::XReportControlFormat>& _xReportControlFormat,const sal_Int32 _nId)
    {
        if ( _xReportControlFormat.is() )
        {
            try
            {
                awt::FontDescriptor aFontDescriptor = _xReportControlFormat->getFontDescriptor();
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
                        OSL_FAIL("Illegal value in default!");
                        break;
                }

                _xReportControlFormat->setFontDescriptor(aFontDescriptor);
            }
            catch(const beans::UnknownPropertyException&)
            {
            }
        }
    }
}

// -----------------------------------------------------------------------------
void lcl_getReportControlFormat(const Sequence< PropertyValue >& aArgs,
                                 ODesignView* _pView,
                                 uno::Reference< awt::XWindow>& _xWindow,
                                 ::std::vector< uno::Reference< uno::XInterface > >& _rControlsFormats)
{
    uno::Reference< report::XReportControlFormat> xReportControlFormat;
    if ( aArgs.getLength() )
    {
        SequenceAsHashMap aMap(aArgs);
        xReportControlFormat = aMap.getUnpackedValueOrDefault(REPORTCONTROLFORMAT,uno::Reference< report::XReportControlFormat>());
        _xWindow = aMap.getUnpackedValueOrDefault(CURRENT_WINDOW,uno::Reference< awt::XWindow>());
    }

    if ( !xReportControlFormat.is() )
    {
        _pView->fillControlModelSelection(_rControlsFormats);
    }
    else
    {
        uno::Reference<uno::XInterface> xInterface(xReportControlFormat);
        _rControlsFormats.push_back(xInterface);
    }

    if ( !_xWindow.is() )
        _xWindow = VCLUnoHelper::GetInterface(_pView);
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

#define PROPERTY_ID_ZOOMVALUE   1

DBG_NAME( rpt_OReportController )
// -----------------------------------------------------------------------------
OReportController::OReportController(Reference< XComponentContext > const & xContext)
    :OReportController_BASE(Reference< XMultiServiceFactory >(xContext->getServiceManager(),UNO_QUERY))
    ,OPropertyStateContainer(OGenericUnoController_Base::rBHelper)
    ,m_aSelectionListeners( getMutex() )
    ,m_pClipbordNotifier(NULL)
    ,m_pGroupsFloater(NULL)
    ,m_xContext(xContext)
    ,m_nSplitPos(-1)
    ,m_nPageNum(-1)
    ,m_nSelectionCount(0)
    ,m_nZoomValue(100)
    ,m_eZoomType(SVX_ZOOM_PERCENT)
    ,m_bShowRuler(sal_True)
    ,m_bGridVisible(sal_True)
    ,m_bGridUse(sal_True)
    ,m_bShowProperties(sal_True)
    ,m_bGroupFloaterWasVisible(sal_False)
    ,m_bHelplinesMove(sal_True)
    ,m_bChartEnabled(false)
    ,m_bChartEnabledAsked(false)
    ,m_bInGeneratePreview(false)
{
    // new Observer
    m_pReportControllerObserver = new OXReportControllerObserver(*this);
    m_pReportControllerObserver->acquire();

    m_sMode =  ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("normal"));
    DBG_CTOR( rpt_OReportController,NULL);
    registerProperty(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ZoomValue")),PROPERTY_ID_ZOOMVALUE,beans::PropertyAttribute::BOUND| beans::PropertyAttribute::TRANSIENT,&m_nZoomValue,::getCppuType(static_cast< sal_Int16*>(0)));

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

    if ( m_pClipbordNotifier )
    {
        m_pClipbordNotifier->ClearCallbackLink();
        m_pClipbordNotifier->AddRemoveListener( getView(), sal_False );
        m_pClipbordNotifier->release();
        m_pClipbordNotifier = NULL;
    }
    if ( m_pGroupsFloater )
    {
        SvtViewOptions aDlgOpt( E_WINDOW, String::CreateFromInt32( RID_GROUPS_SORTING ) );
        aDlgOpt.SetWindowState(::rtl::OStringToOUString(m_pGroupsFloater->GetWindowState(WINDOWSTATE_MASK_ALL), RTL_TEXTENCODING_ASCII_US));
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<FloatingWindow> aTemp(m_pGroupsFloater);
        SAL_WNODEPRECATED_DECLARATIONS_POP
        m_pGroupsFloater = NULL;
    }

    try
    {
        m_xHoldAlive.clear();
        m_xColumns.clear();
        ::comphelper::disposeComponent( m_xRowSet );
        ::comphelper::disposeComponent( m_xRowSetMediator );
        ::comphelper::disposeComponent( m_xFormatter );
    }
    catch(const uno::Exception&)
    {
        OSL_FAIL("Exception caught while disposing row sets.");
    }
    m_xRowSet.clear();
    m_xRowSetMediator.clear();

    if ( m_xReportDefinition.is() )
    {
        try
        {
            ::boost::shared_ptr<OSectionWindow> pSectionWindow;
            if ( getDesignView() )
                pSectionWindow = getDesignView()->getMarkedSection();
            if ( pSectionWindow )
                pSectionWindow->getReportSection().deactivateOle();
            clearUndoManager();
            if ( m_aReportModel )
                listen(false);
            m_pReportControllerObserver->Clear();
            m_pReportControllerObserver->release();
        }
        catch(const uno::Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    {
        EventObject aDisposingEvent( *this );
        m_aSelectionListeners.disposeAndClear( aDisposingEvent );
    }

    OReportController_BASE::disposing();


    try
    {
        m_xReportDefinition.clear();
        m_aReportModel.reset();
        m_xFrameLoader.clear();
        m_xReportEngine.clear();
    }
    catch(const uno::Exception&)
    {
    }
    if ( getDesignView() )
        EndListening( *getDesignView() );
    clearView();
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
            aReturn.bEnabled = isEditable();
            break;
        case SID_REDO:
        case SID_UNDO:
            {
                size_t ( SfxUndoManager::*retrieveCount )( bool const ) const =
                    ( _nId == SID_UNDO ) ? &SfxUndoManager::GetUndoActionCount : &SfxUndoManager::GetRedoActionCount;

                SfxUndoManager& rUndoManager( getUndoManager() );
                aReturn.bEnabled = ( rUndoManager.*retrieveCount )( ::svl::IUndoManager::TopLevel ) > 0;
                if ( aReturn.bEnabled )
                {
                    // TODO: add "Undo/Redo: prefix"
                    rtl::OUString ( SfxUndoManager::*retrieveComment )( size_t, bool const ) const =
                        ( _nId == SID_UNDO ) ? &SfxUndoManager::GetUndoActionComment : &SfxUndoManager::GetRedoActionComment;
                    aReturn.sTitle = (rUndoManager.*retrieveComment)( 0, ::svl::IUndoManager::TopLevel );
                }
            }
            break;
        case SID_OBJECT_RESIZING:
        case SID_OBJECT_SMALLESTWIDTH:
        case SID_OBJECT_SMALLESTHEIGHT:
        case SID_OBJECT_GREATESTWIDTH:
        case SID_OBJECT_GREATESTHEIGHT:
            aReturn.bEnabled = isEditable() && getDesignView()->HasSelection();
            if ( aReturn.bEnabled )
                aReturn.bEnabled = m_nSelectionCount > 1;
            break;

        case SID_DISTRIBUTION:
            aReturn.bEnabled = isEditable() && getDesignView()->HasSelection();
            if ( aReturn.bEnabled )
            {
                OSectionView* pSectionView = getCurrentSectionView();
                aReturn.bEnabled = pSectionView && pSectionView->GetMarkedObjectCount() > 2;
            }
            break;
        case SID_ARRANGEMENU:
        case SID_FRAME_DOWN:
        case SID_FRAME_UP:
        case SID_FRAME_TO_TOP:
        case SID_FRAME_TO_BOTTOM:
        case SID_OBJECT_HEAVEN:
        case SID_OBJECT_HELL:
            aReturn.bEnabled = isEditable() && getDesignView()->HasSelection();
            if ( aReturn.bEnabled )
            {
                OSectionView* pSectionView = getCurrentSectionView();
                aReturn.bEnabled = pSectionView && pSectionView->OnlyShapesMarked();
                if ( aReturn.bEnabled )
                {
                    if ( SID_OBJECT_HEAVEN == _nId )
                        aReturn.bEnabled = pSectionView->GetLayerIdOfMarkedObjects() != RPT_LAYER_FRONT;
                    else if ( SID_OBJECT_HELL == _nId )
                        aReturn.bEnabled = pSectionView->GetLayerIdOfMarkedObjects() != RPT_LAYER_BACK;
                }
            }
            break;

        case SID_SECTION_SHRINK:
        case SID_SECTION_SHRINK_TOP:
        case SID_SECTION_SHRINK_BOTTOM:
            {
                sal_Int32 nCount = 0;
                uno::Reference<report::XSection> xSection = getDesignView()->getCurrentSection();
                if ( xSection.is() )
                {
                    nCount = xSection->getCount();
                }
                aReturn.bEnabled = isEditable() && nCount > 0;
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
            aReturn.bEnabled = isEditable() && getDesignView()->HasSelection();
            break;
        case SID_CUT:
            aReturn.bEnabled = isEditable() && getDesignView()->HasSelection() && !getDesignView()->isHandleEvent(_nId);
            break;
        case SID_COPY:
            aReturn.bEnabled = getDesignView()->HasSelection() && !getDesignView()->isHandleEvent(_nId);
            break;
        case SID_PASTE:
            aReturn.bEnabled = isEditable()  && !getDesignView()->isHandleEvent(_nId) && getDesignView()->IsPasteAllowed();
            break;
        case SID_SELECTALL:
            aReturn.bEnabled = !getDesignView()->isHandleEvent(_nId);
            break;
        case SID_SELECTALL_IN_SECTION:
            aReturn.bEnabled = !getDesignView()->isHandleEvent(_nId);
            if ( aReturn.bEnabled )
                aReturn.bEnabled = getCurrentSectionView() != NULL;
            break;
        case SID_ESCAPE:
            aReturn.bEnabled = getDesignView()->GetMode() == RPTUI_INSERT;
            break;
        case SID_TERMINATE_INPLACEACTIVATION:
            aReturn.bEnabled = sal_True;
            break;
        case SID_SELECT_ALL_EDITS:
        case SID_SELECT_ALL_LABELS:
            aReturn.bEnabled = sal_True;
            break;
        case SID_RPT_NEW_FUNCTION:
            aReturn.bEnabled = isEditable();
            break;
        case SID_COLLAPSE_SECTION:
        case SID_EXPAND_SECTION:
        case SID_NEXT_MARK:
        case SID_PREV_MARK:
            aReturn.bEnabled = isEditable() && !getDesignView()->isHandleEvent(_nId);
            break;
        case SID_SELECT:
        case SID_SELECT_REPORT:
            aReturn.bEnabled = sal_True;
            break;
        case SID_EXECUTE_REPORT:
            aReturn.bEnabled = isConnected() && m_xReportDefinition.is();
            break;
        case SID_DELETE:
            aReturn.bEnabled = isEditable() && getDesignView()->HasSelection() && !getDesignView()->isHandleEvent(_nId);
            if ( aReturn.bEnabled )
            {
                ::boost::shared_ptr<OSectionWindow> pSectionWindow = getDesignView()->getMarkedSection();
                if ( pSectionWindow )
                    aReturn.bEnabled = !pSectionWindow->getReportSection().isUiActive();
            }
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
            aReturn.bChecked = getDesignView()->GetMode() == RPTUI_SELECT;
            break;
        case SID_INSERT_DIAGRAM:
            aReturn.bEnabled = isEditable();
            aReturn.bInvisible = optional< bool >(!m_bChartEnabled);
            aReturn.bChecked = getDesignView()->GetInsertObj() == OBJ_OLE2;
            break;
        case SID_FM_FIXEDTEXT:
            aReturn.bEnabled = isEditable();
            aReturn.bChecked = getDesignView()->GetInsertObj() == OBJ_DLG_FIXEDTEXT;
            break;
        case SID_INSERT_HFIXEDLINE:
            aReturn.bEnabled = isEditable();
            aReturn.bChecked = getDesignView()->GetInsertObj() == OBJ_DLG_HFIXEDLINE;
            break;
        case SID_INSERT_VFIXEDLINE:
            aReturn.bEnabled = isEditable();
            aReturn.bChecked = getDesignView()->GetInsertObj() == OBJ_DLG_VFIXEDLINE;
            break;
        case SID_FM_EDIT:
            aReturn.bEnabled = isEditable();
            aReturn.bChecked = getDesignView()->GetInsertObj() == OBJ_DLG_FORMATTEDFIELD;
            break;
        case SID_FM_IMAGECONTROL:
            aReturn.bEnabled = isEditable();
            aReturn.bChecked = getDesignView()->GetInsertObj() == OBJ_DLG_IMAGECONTROL;
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
            impl_fillCustomShapeState_nothrow("diamond",aReturn);
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
            impl_fillCustomShapeState_nothrow("smiley",aReturn);
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
            impl_fillCustomShapeState_nothrow("left-right-arrow",aReturn);
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
            impl_fillCustomShapeState_nothrow("star5",aReturn);
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
            impl_fillCustomShapeState_nothrow("flowchart-internal-storage",aReturn);
            break;
        case SID_DRAWTBX_CS_CALLOUT:
        case SID_DRAWTBX_CS_CALLOUT1:
        case SID_DRAWTBX_CS_CALLOUT2:
        case SID_DRAWTBX_CS_CALLOUT3:
        case SID_DRAWTBX_CS_CALLOUT4:
        case SID_DRAWTBX_CS_CALLOUT5:
        case SID_DRAWTBX_CS_CALLOUT6:
        case SID_DRAWTBX_CS_CALLOUT7:
            impl_fillCustomShapeState_nothrow("round-rectangular-callout",aReturn);
            break;
        case SID_RPT_SHOWREPORTEXPLORER:
            aReturn.bEnabled = m_xReportDefinition.is();
            aReturn.bChecked = getDesignView() && getDesignView()->isReportExplorerVisible();
            break;
        case SID_FM_ADD_FIELD:
            aReturn.bEnabled = isConnected() && isEditable() && m_xReportDefinition.is()
                && !m_xReportDefinition->getCommand().isEmpty();
            aReturn.bChecked = getDesignView() && getDesignView()->isAddFieldVisible();
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
            aReturn.bEnabled = impl_isModified() && isEditable();
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
            impl_fillState_nothrow(PROPERTY_CONTROLBACKGROUND,aReturn);
            break;
        case SID_ATTR_CHAR_COLOR_BACKGROUND:
            aReturn.bEnabled = isEditable();
            {
                uno::Reference<report::XSection> xSection = getDesignView()->getCurrentSection();
                if ( xSection.is() )
                    try
                    {
                        aReturn.aValue <<= xSection->getBackColor();
                        const uno::Reference< report::XReportControlModel> xControlModel(getDesignView()->getCurrentControlModel(),uno::UNO_QUERY);
                        aReturn.bEnabled = !xControlModel.is();
                    }
                    catch(const beans::UnknownPropertyException&)
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
            impl_fillState_nothrow(PROPERTY_FONTDESCRIPTOR,aReturn);
            if ( aReturn.bEnabled )
            {
                awt::FontDescriptor aFontDescriptor;
                aReturn.aValue >>= aFontDescriptor;
                aReturn.aValue.clear();

                switch(_nId)
                {
                    case SID_ATTR_CHAR_WEIGHT:
                        aReturn.bChecked = awt::FontWeight::BOLD == aFontDescriptor.Weight;
                        break;
                    case SID_ATTR_CHAR_POSTURE:
                        aReturn.bChecked = awt::FontSlant_ITALIC == aFontDescriptor.Slant;
                        break;
                    case SID_ATTR_CHAR_UNDERLINE:
                        aReturn.bChecked = awt::FontUnderline::SINGLE == aFontDescriptor.Underline;
                        break;
                    default:
                        ;
                    }
            }
            break;
        case SID_ATTR_CHAR_COLOR:
        case SID_ATTR_CHAR_COLOR2:
            impl_fillState_nothrow(PROPERTY_CHARCOLOR,aReturn);
            break;
        case SID_ATTR_CHAR_FONT:
            impl_fillState_nothrow(PROPERTY_FONTDESCRIPTOR,aReturn);
            break;
        case SID_ATTR_CHAR_FONTHEIGHT:
            impl_fillState_nothrow(PROPERTY_CHARHEIGHT,aReturn);
            if ( aReturn.aValue.hasValue() )
            {
                frame::status::FontHeight aFontHeight;
                aReturn.aValue >>= aFontHeight.Height;
                aReturn.aValue <<= aFontHeight; // another type is needed here, so
            }
            break;
        case SID_ATTR_PARA_ADJUST_LEFT:
        case SID_ATTR_PARA_ADJUST_CENTER:
        case SID_ATTR_PARA_ADJUST_RIGHT:
        case SID_ATTR_PARA_ADJUST_BLOCK:
            impl_fillState_nothrow(PROPERTY_PARAADJUST,aReturn);
            if ( aReturn.bEnabled )
            {
                ::sal_Int16 nParaAdjust = 0;
                if ( aReturn.aValue >>= nParaAdjust )
                {
                    switch(nParaAdjust)
                    {
                        case style::ParagraphAdjust_LEFT:
                            aReturn.bChecked = _nId == SID_ATTR_PARA_ADJUST_LEFT;
                            break;
                        case style::ParagraphAdjust_RIGHT:
                            aReturn.bChecked = _nId == SID_ATTR_PARA_ADJUST_RIGHT;
                            break;
                        case style::ParagraphAdjust_BLOCK:
                        case style::ParagraphAdjust_STRETCH:
                            aReturn.bChecked = _nId == SID_ATTR_PARA_ADJUST_BLOCK;
                            break;
                        case style::ParagraphAdjust_CENTER:
                            aReturn.bChecked = _nId == SID_ATTR_PARA_ADJUST_CENTER;
                            break;
                    }
                }
                aReturn.aValue.clear();
            }
            break;

        case SID_INSERT_GRAPHIC:
            aReturn.bEnabled = m_xReportDefinition.is() && isEditable() && getDesignView()->getCurrentSection().is();
            break;
        case SID_CHAR_DLG:
        case SID_SETCONTROLDEFAULTS:
            aReturn.bEnabled = m_xReportDefinition.is() && isEditable();
            if ( aReturn.bEnabled )
            {
                ::std::vector< uno::Reference< uno::XInterface > > aSelection;
                getDesignView()->fillControlModelSelection(aSelection);
                ::std::vector< uno::Reference< uno::XInterface > >::iterator aIter = aSelection.begin();
                for(; aIter != aSelection.end()
                    && !uno::Reference< report::XFixedLine >(*aIter,uno::UNO_QUERY).is()
                    && !uno::Reference< report::XImageControl >(*aIter,uno::UNO_QUERY).is()
                    && uno::Reference< report::XReportControlFormat >(*aIter,uno::UNO_QUERY).is() ;++aIter)
                    ;
                aReturn.bEnabled = !aSelection.empty() && aIter == aSelection.end();
            }
            break;
        case SID_CONDITIONALFORMATTING:
            {
                const uno::Reference< report::XFormattedField> xFormattedField(getDesignView()->getCurrentControlModel(),uno::UNO_QUERY);
                aReturn.bEnabled = xFormattedField.is();
            }
            break;
        case SID_INSERT_FLD_PGNUMBER:
        case SID_DATETIME:
            aReturn.bEnabled = m_xReportDefinition.is() && isEditable() && getDesignView()->getCurrentSection().is();
            break;
        case SID_EXPORTDOC:
        case SID_EXPORTDOCASPDF:
            aReturn.bEnabled = m_xReportDefinition.is();
            break;
        case SID_PRINTPREVIEW:
            aReturn.bEnabled = sal_False;
            break;
        case SID_ATTR_ZOOM:
            aReturn.bEnabled = sal_True;
            {
                SvxZoomItem aZoom(m_eZoomType,m_nZoomValue);
                aZoom.SetValueSet(SVX_ZOOM_ENABLE_50|SVX_ZOOM_ENABLE_75|SVX_ZOOM_ENABLE_100|SVX_ZOOM_ENABLE_200);
                aZoom.QueryValue(aReturn.aValue);
            }
            break;
        case SID_ATTR_ZOOMSLIDER:
            aReturn.bEnabled = sal_True;
            {
                SvxZoomSliderItem aZoomSlider(m_nZoomValue,20,400);
                aZoomSlider.AddSnappingPoint(50);
                aZoomSlider.AddSnappingPoint(75);
                aZoomSlider.AddSnappingPoint(100);
                aZoomSlider.AddSnappingPoint(200);
                aZoomSlider.QueryValue(aReturn.aValue);
            }
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
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getMutex() );

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
            const OXUndoEnvironment::OUndoMode aLock( m_aReportModel->GetUndoEnv() );
            sal_Bool ( SfxUndoManager::*doXDo )() =
                ( _nId == SID_UNDO ) ? &SfxUndoManager::Undo : &SfxUndoManager::Redo;

            SfxUndoManager& rUndoManager( getUndoManager() );
            (rUndoManager.*doXDo)();
            InvalidateAll();
            updateFloater();
        }
        break;
        case SID_CUT:
            executeMethodWithUndo(RID_STR_UNDO_REMOVE_SELECTION,::std::mem_fun(&ODesignView::Cut));
            break;
        case SID_COPY:
            getDesignView()->Copy();
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

        case SID_SECTION_SHRINK_BOTTOM:
        case SID_SECTION_SHRINK_TOP:
        case SID_SECTION_SHRINK:
            {
                uno::Reference<report::XSection> xSection = getDesignView()->getCurrentSection();
                shrinkSection(RID_STR_UNDO_SHRINK, xSection, _nId);
            }
            break;

        case SID_SELECTALL:
            getDesignView()->SelectAll(OBJ_NONE);
            break;
        case SID_SELECTALL_IN_SECTION:
            {
                OSectionView* pSectionView = getCurrentSectionView();
                if ( pSectionView )
                    pSectionView->MarkAll();
            }
            break;
        case SID_ESCAPE:
            getDesignView()->SetMode(RPTUI_SELECT);
            InvalidateFeature( SID_OBJECT_SELECT );
            break;
        case SID_SELECT_ALL_EDITS:
            getDesignView()->SelectAll(OBJ_DLG_FORMATTEDFIELD);
            break;
        case SID_SELECT_ALL_LABELS:
            getDesignView()->SelectAll(OBJ_DLG_FIXEDTEXT);
            break;
        case SID_TERMINATE_INPLACEACTIVATION:
            {
                ::boost::shared_ptr<OSectionWindow> pSection = getDesignView()->getMarkedSection();
                if ( pSection )
                    pSection->getReportSection().deactivateOle();
            }
            break;
        case SID_SELECT:
            if ( aArgs.getLength() == 1 )
                select(aArgs[0].Value);
            break;
        case SID_SELECT_REPORT:
            select(uno::makeAny(m_xReportDefinition));
            break;
        case SID_EXECUTE_REPORT:
            getView()->PostUserEvent(LINK(this, OReportController,OnExecuteReport));
            break;
        case SID_RPT_NEW_FUNCTION:
            createNewFunction(aArgs[0].Value);
            break;
        case SID_COLLAPSE_SECTION:
            collapseSection(true);
            break;
        case SID_EXPAND_SECTION:
            collapseSection(false);
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
                    const String sUndoAction = String((ModuleRes(RID_STR_UNDO_REMOVE_FUNCTION)));
                    UndoContext aUndoContext( getUndoManager(), sUndoAction );
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
            getDesignView()->setGridSnap(m_bGridUse = !m_bGridUse);
            break;
        case SID_HELPLINES_MOVE:
            getDesignView()->setDragStripes(m_bHelplinesMove = !m_bHelplinesMove);
            break;
        case SID_GRID_VISIBLE:
            getDesignView()->toggleGrid(m_bGridVisible = !m_bGridVisible);
            break;
        case SID_RULER:
            getDesignView()->showRuler(m_bShowRuler = !m_bShowRuler);
            break;
        case SID_OBJECT_SELECT:
            getDesignView()->SetMode(RPTUI_SELECT);
            InvalidateAll();
            break;
        case SID_INSERT_DIAGRAM:
            getDesignView()->SetMode( RPTUI_INSERT );
            getDesignView()->SetInsertObj( OBJ_OLE2);
            createDefaultControl(aArgs);
            InvalidateAll();
            break;
        case SID_FM_FIXEDTEXT:
            getDesignView()->SetMode( RPTUI_INSERT );
            getDesignView()->SetInsertObj( OBJ_DLG_FIXEDTEXT );
            createDefaultControl(aArgs);
            InvalidateAll();
            break;
        case SID_INSERT_HFIXEDLINE:
            getDesignView()->SetMode( RPTUI_INSERT );
            getDesignView()->SetInsertObj( OBJ_DLG_HFIXEDLINE );
            createDefaultControl(aArgs);
            InvalidateAll();
            break;
        case SID_INSERT_VFIXEDLINE:
            getDesignView()->SetMode( RPTUI_INSERT );
            getDesignView()->SetInsertObj( OBJ_DLG_VFIXEDLINE );
            createDefaultControl(aArgs);
            InvalidateAll();
            break;
        case SID_FM_EDIT:
            getDesignView()->SetMode( RPTUI_INSERT );
            getDesignView()->SetInsertObj( OBJ_DLG_FORMATTEDFIELD );
            createDefaultControl(aArgs);
            InvalidateAll();
            break;
        case SID_FM_IMAGECONTROL:
            getDesignView()->SetMode( RPTUI_INSERT );
            getDesignView()->SetInsertObj( OBJ_DLG_IMAGECONTROL );
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
            getDesignView()->SetMode( RPTUI_INSERT );
            {
                URL aUrl = getURLForId(_nId);
                sal_Int32 nIndex = 1;
                ::rtl::OUString sType = aUrl.Complete.getToken(0,'.',nIndex);
                if ( nIndex == -1 || sType.isEmpty() )
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
                        default:
                            sType = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("diamond"));
                    }
                }
                else
                    sType = aUrl.Complete.getToken(0,'.',nIndex);

                getDesignView()->SetInsertObj( OBJ_CUSTOMSHAPE ,sType);
                createDefaultControl(aArgs);
            }
            InvalidateAll();
            break;
        case SID_RPT_SHOWREPORTEXPLORER:
            if ( isUiVisible() )
                getDesignView()->toggleReportExplorer();
            break;
        case SID_FM_ADD_FIELD:
            if ( isUiVisible() )
                getDesignView()->toggleAddField();
            break;
        case SID_SHOW_PROPERTYBROWSER:
            if ( m_bShowProperties )
                m_sLastActivePage = getDesignView()->getCurrentPage();
            else
                getDesignView()->setCurrentPage(m_sLastActivePage);

            if ( isUiVisible() )
            {
                m_bShowProperties = !m_bShowProperties;
                if ( aArgs.getLength() == 1 )
                    aArgs[0].Value >>= m_bShowProperties;

                getDesignView()->togglePropertyBrowser(m_bShowProperties);
            }
            break;
        case SID_PROPERTYBROWSER_LAST_PAGE: // nothing to do
            m_sLastActivePage = getDesignView()->getCurrentPage();
            break;
        case SID_SPLIT_POSITION:
            getDesignView()->Resize();
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
                const util::Color aColor( lcl_extractBackgroundColor( aArgs ) );
                if ( !impl_setPropertyAtControls_throw(RID_STR_UNDO_CHANGEFONT,PROPERTY_CONTROLBACKGROUND,uno::makeAny(aColor),aArgs) )
                {
                    uno::Reference< report::XSection > xSection = getDesignView()->getCurrentSection();
                    if ( xSection.is() )
                    {
                        xSection->setBackColor( aColor );
                    }
                }
                bForceBroadcast = sal_True;
            }
            break;
        case SID_ATTR_CHAR_WEIGHT:
        case SID_ATTR_CHAR_POSTURE:
        case SID_ATTR_CHAR_UNDERLINE:
            {
                uno::Reference< awt::XWindow> xWindow;
                ::std::vector< uno::Reference< uno::XInterface > > aControlsFormats;
                lcl_getReportControlFormat( aArgs, getDesignView(), xWindow, aControlsFormats );

                const String sUndoAction(ModuleRes(RID_STR_UNDO_CHANGEFONT));
                UndoContext aUndoContext( getUndoManager(), sUndoAction );

                ::std::vector< uno::Reference< uno::XInterface > >::iterator aIter = aControlsFormats.begin();
                for(; aIter != aControlsFormats.end();++aIter)
                {
                    uno::Reference< report::XReportControlFormat> xReportControlFormat(*aIter,uno::UNO_QUERY);
                    lcl_setFontWPU_nothrow(xReportControlFormat,_nId);
                }
            }
            break;
        case SID_ATTR_CHAR_COLOR:
        case SID_ATTR_CHAR_COLOR2:
            {
                const SequenceAsHashMap aMap(aArgs);
                const util::Color aColor = aMap.getUnpackedValueOrDefault(PROPERTY_FONTCOLOR,util::Color());
                impl_setPropertyAtControls_throw(RID_STR_UNDO_CHANGEFONT,PROPERTY_CHARCOLOR,uno::makeAny(aColor),aArgs);
                bForceBroadcast = sal_True;
            }
            break;
        case SID_ATTR_CHAR_FONT:
            if ( aArgs.getLength() == 1 )
            {
                awt::FontDescriptor aFont;
                if ( aArgs[0].Value >>= aFont )
                {
                    impl_setPropertyAtControls_throw(RID_STR_UNDO_CHANGEFONT,PROPERTY_CHARFONTNAME,uno::makeAny(aFont.Name),aArgs);
                }
            }
            break;
        case SID_ATTR_CHAR_FONTHEIGHT:
            if ( aArgs.getLength() == 1 )
            {
                float fSelVal = 0.0;
                if ( aArgs[0].Value >>= fSelVal )
                    impl_setPropertyAtControls_throw(RID_STR_UNDO_CHANGEFONT,PROPERTY_CHARHEIGHT,aArgs[0].Value,aArgs);
            }
            break;
        case SID_ATTR_PARA_ADJUST_LEFT:
        case SID_ATTR_PARA_ADJUST_CENTER:
        case SID_ATTR_PARA_ADJUST_RIGHT:
        case SID_ATTR_PARA_ADJUST_BLOCK:
            {
                sal_Int16 eParagraphAdjust = style::ParagraphAdjust_LEFT;
                switch(_nId)
                {
                    case SID_ATTR_PARA_ADJUST_LEFT:
                        eParagraphAdjust = style::ParagraphAdjust_LEFT;
                        break;
                    case SID_ATTR_PARA_ADJUST_CENTER:
                        eParagraphAdjust = style::ParagraphAdjust_CENTER;
                        break;
                    case SID_ATTR_PARA_ADJUST_RIGHT:
                        eParagraphAdjust = style::ParagraphAdjust_RIGHT;
                        break;
                    case SID_ATTR_PARA_ADJUST_BLOCK:
                        eParagraphAdjust = style::ParagraphAdjust_BLOCK;
                        break;
                }
                impl_setPropertyAtControls_throw(RID_STR_UNDO_ALIGNMENT,PROPERTY_PARAADJUST,uno::makeAny(eParagraphAdjust),aArgs);

                InvalidateFeature(SID_ATTR_PARA_ADJUST_LEFT);
                InvalidateFeature(SID_ATTR_PARA_ADJUST_CENTER);
                InvalidateFeature(SID_ATTR_PARA_ADJUST_RIGHT);
                InvalidateFeature(SID_ATTR_PARA_ADJUST_BLOCK);
            }
            break;
        case SID_CHAR_DLG:
            {
                uno::Sequence< beans::NamedValue > aSettings;
                uno::Reference< awt::XWindow> xWindow;
                ::std::vector< uno::Reference< uno::XInterface > > aControlsFormats;
                lcl_getReportControlFormat( aArgs, getDesignView(), xWindow, aControlsFormats );

                if ( !aControlsFormats.empty() )
                {
                    const String sUndoAction( ModuleRes( RID_STR_UNDO_CHANGEFONT ) );
                    UndoContext aUndoContext( getUndoManager(), sUndoAction );

                    ::std::vector< uno::Reference< uno::XInterface > >::iterator aIter = aControlsFormats.begin();
                    for(; aIter != aControlsFormats.end();++aIter)
                    {
                        uno::Reference< report::XReportControlFormat > xFormat( *aIter, uno::UNO_QUERY );
                        if ( !xFormat.is() )
                            continue;

                        if ( aSettings.getLength() == 0 )
                        {
                            ::rptui::openCharDialog( xFormat, xWindow, aSettings );
                            if ( aSettings.getLength() == 0 )
                                break;
                        }

                        applyCharacterSettings( xFormat, aSettings );
                    }

                    InvalidateAll();
                }
            }
            break;
        case SID_INSERT_GRAPHIC:
            insertGraphic();
            break;
        case SID_SETCONTROLDEFAULTS:
            break;
        case SID_CONDITIONALFORMATTING:
            {
                uno::Reference< report::XFormattedField> xFormattedField(getDesignView()->getCurrentControlModel(),uno::UNO_QUERY);
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
                    ODateTimeDialog aDlg(getView(),getDesignView()->getCurrentSection(),this);
                    aDlg.Execute();
                }
                else
                    createDateTime(aArgs);
            }
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
            }
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
            InvalidateAll();
            return;
        case SID_GROUP:
            break;
        case SID_ATTR_ZOOM:
            if ( aArgs.getLength() == 0 )
            {
                openZoomDialog();
            }
            else if ( aArgs.getLength() == 1 && aArgs[0].Name == "Zoom" )
            {
                SvxZoomItem aZoomItem;
                aZoomItem.PutValue(aArgs[0].Value);
                m_nZoomValue = aZoomItem.GetValue();
                m_eZoomType = aZoomItem.GetType();
                impl_zoom_nothrow();
            }
            break;
        case SID_ATTR_ZOOMSLIDER:
            if ( aArgs.getLength() == 1 && aArgs[0].Name == "ZoomSlider" )
            {
                SvxZoomSliderItem aZoomSlider;
                aZoomSlider.PutValue(aArgs[0].Value);
                m_nZoomValue = aZoomSlider.GetValue();
                m_eZoomType = SVX_ZOOM_PERCENT;
                impl_zoom_nothrow();
            }
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
    if ( m_sName.isEmpty() )
        rArguments.get_ensureType( "DocumentTitle", m_sName );

    try
    {
        if ( m_xReportDefinition.is() )
        {
            getView()->initialize();    // show the windows and fill with our informations

            m_aReportModel = reportdesign::OReportDefinition::getSdrModel(m_xReportDefinition);
            if ( !m_aReportModel )
                throw RuntimeException();
            m_aReportModel->attachController( *this );

            clearUndoManager();
            UndoSuppressor aSuppressUndo( getUndoManager() );

            ::comphelper::NamedValueCollection aArgs(getModel()->getArgs());
            setMode(aArgs.getOrDefault("Mode", ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("normal"))));

            listen(true);
            setEditable( !m_aReportModel->IsReadOnly() );
            m_xFormatter.set(util::NumberFormatter::create(comphelper::ComponentContext(getORB()).getUNOContext()), UNO_QUERY_THROW);
            m_xFormatter->attachNumberFormatsSupplier(Reference< XNumberFormatsSupplier>(m_xReportDefinition,uno::UNO_QUERY));

            ::comphelper::MediaDescriptor aDescriptor( m_xReportDefinition->getArgs() );
            ::rtl::OUString sHierarchicalDocumentName;
            sHierarchicalDocumentName = aDescriptor.getUnpackedValueOrDefault(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HierarchicalDocumentName")),sHierarchicalDocumentName);

            if ( sHierarchicalDocumentName.isEmpty() && getConnection().is() )
            {
                uno::Reference<sdbcx::XTablesSupplier> xTablesSup(getConnection(),uno::UNO_QUERY_THROW);
                uno::Reference<container::XNameAccess> xTables = xTablesSup->getTables();
                const uno::Sequence< ::rtl::OUString > aNames( xTables->getElementNames() );

                if ( aNames.hasElements() )
                {
                    m_xReportDefinition->setCommand(aNames[0]);
                    m_xReportDefinition->setCommandType(sdb::CommandType::TABLE);
                }
            }

            m_aVisualAreaSize = m_xReportDefinition->getVisualAreaSize(0);

        }

        // check if chart is supported by the engine
        checkChartEnabled();
        // restore the view data
        getDesignView()->toggleGrid(m_bGridVisible);
        getDesignView()->showRuler(m_bShowRuler);
        getDesignView()->togglePropertyBrowser(m_bShowProperties);
        getDesignView()->setCurrentPage(m_sLastActivePage);
        getDesignView()->unmarkAllObjects(NULL);

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
        getDesignView()->collapseSections(m_aCollapsedSections);
        impl_zoom_nothrow();
        getDesignView()->Resize();
        getDesignView()->Invalidate();
        InvalidateAll();

        if ( m_bShowProperties && m_nPageNum == -1 )
        {
            m_sLastActivePage = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Data"));
            getDesignView()->setCurrentPage(m_sLastActivePage);
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
        DBG_UNHANDLED_EXCEPTION();
    }
}
// -----------------------------------------------------------------------------
IMPL_LINK( OReportController, OnOpenHelpAgent, void* ,/*_pMemfun*/)
{
    doOpenHelpAgent();
    return 0L;
}
// -----------------------------------------------------------------------------
IMPL_LINK( OReportController, OnCreateHdl, OAddFieldWindow* ,_pAddFieldDlg)
{
    WaitObject aObj( getDesignView() );
    uno::Sequence< beans::PropertyValue > aArgs = _pAddFieldDlg->getSelectedFieldDescriptors();
    // we use this way to create undo actions
    if ( aArgs.getLength() )
    {
        executeChecked(SID_ADD_CONTROL_PAIR,aArgs);
    }
    return 0L;
}
// -----------------------------------------------------------------------------

void OReportController::doOpenHelpAgent()
{
    if (getFrame().is())
    {
        rtl::OUString suURL(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.star.help://shared/text/shared/explorer/database/rep_main.xhp?UseDB=no&DbPAR=swriter"));
        openHelpAgent(suURL);
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
    ODesignView* pMyOwnView = new ODesignView( pParent, getORB(), *this );
    StartListening( *pMyOwnView );
    setView( *pMyOwnView );

    // now that we have a view we can create the clipboard listener
    m_aSystemClipboard = TransferableDataHelper::CreateFromSystemClipboard( getView() );
    m_aSystemClipboard.StartClipboardListening( );
    m_pClipbordNotifier = new TransferableClipboardListener( LINK( this, OReportController, OnClipboardChanged ) );
    m_pClipbordNotifier->acquire();
    m_pClipbordNotifier->AddRemoveListener( getView(), sal_True );

    OReportController_BASE::Construct(pParent);
    return sal_True;
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OReportController::suspend(sal_Bool /*_bSuspend*/) throw( RuntimeException )
{
    if ( getBroadcastHelper().bInDispose || getBroadcastHelper().bDisposed )
        return sal_True;

    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getMutex() );

    if ( getView() && getView()->IsInModalMode() )
        return sal_False;

    // this suspend will be handled in the DBAccess interceptor implementation
    return sal_True;
}
// -----------------------------------------------------------------------------
void OReportController::describeSupportedFeatures()
{
    DBSubComponentController::describeSupportedFeatures();

    implDescribeSupportedFeature( ".uno:TextDocument",              SID_RPT_TEXTDOCUMENT,           CommandGroup::APPLICATION );
    implDescribeSupportedFeature( ".uno:Spreadsheet",               SID_RPT_SPREADSHEET,            CommandGroup::APPLICATION );

    implDescribeSupportedFeature( ".uno:Redo",                      SID_REDO,                       CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:Undo",                      SID_UNDO,                       CommandGroup::EDIT );
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
    implDescribeSupportedFeature( ".uno:DbSortingAndGrouping",      SID_SORTINGANDGROUPING,         CommandGroup::VIEW );
    implDescribeSupportedFeature( ".uno:PageHeaderFooter",          SID_PAGEHEADERFOOTER,           CommandGroup::VIEW );
    implDescribeSupportedFeature( ".uno:ReportHeaderFooter",        SID_REPORTHEADERFOOTER,         CommandGroup::VIEW );
    implDescribeSupportedFeature( ".uno:ZoomSlider",                SID_ATTR_ZOOMSLIDER,            CommandGroup::VIEW );
    implDescribeSupportedFeature( ".uno:Zoom",                      SID_ATTR_ZOOM,                  CommandGroup::VIEW );

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
    implDescribeSupportedFeature( ".uno:JustifyPara",               SID_ATTR_PARA_ADJUST_BLOCK,     CommandGroup::FORMAT );

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
    implDescribeSupportedFeature( ".uno:SectionShrink",             SID_SECTION_SHRINK,             CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:SectionShrinkTop",          SID_SECTION_SHRINK_TOP,         CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:SectionShrinkBottom",       SID_SECTION_SHRINK_BOTTOM,      CommandGroup::FORMAT );

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
    implDescribeSupportedFeature( ".uno:TerminateInplaceActivation",    SID_TERMINATE_INPLACEACTIVATION);
    implDescribeSupportedFeature( ".uno:SelectAllLabels",               SID_SELECT_ALL_LABELS);
    implDescribeSupportedFeature( ".uno:SelectAllEdits",                SID_SELECT_ALL_EDITS);
    implDescribeSupportedFeature( ".uno:CollapseSection",           SID_COLLAPSE_SECTION);
    implDescribeSupportedFeature( ".uno:ExpandSection",             SID_EXPAND_SECTION);
}
// -----------------------------------------------------------------------------
void OReportController::impl_onModifyChanged()
{
    try
    {
        if ( m_xReportDefinition.is() )
            m_xReportDefinition->setModified( impl_isModified() );
        DBSubComponentController::impl_onModifyChanged();
    }
    catch(const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
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
            ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("private:resource/toolbar/sectionshrinkbar"))
        };
        for (size_t i = 0; i< sizeof(s_sMenu)/sizeof(s_sMenu[0]); ++i)
        {
            _xLayoutManager->createElement( s_sMenu[i] );
            _xLayoutManager->requestElement( s_sMenu[i] );
        }
    }
}
// -----------------------------------------------------------------------------
void OReportController::notifyGroupSections(const ContainerEvent& _rEvent,bool _bShow)
{
    uno::Reference< report::XGroup> xGroup(_rEvent.Element,uno::UNO_QUERY);
    if ( xGroup.is() )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getMutex() );
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
            if (_bShow)
            {
                m_pReportControllerObserver->AddSection(xGroup->getHeader());
            }
            else
            {
                m_pReportControllerObserver->RemoveSection(xGroup->getHeader());
            }
        }
        if ( xGroup->getFooterOn() )
        {
            groupChange(xGroup,PROPERTY_FOOTERON,nGroupPos,_bShow);
            if (_bShow)
            {
                m_pReportControllerObserver->AddSection(xGroup->getFooter());
            }
            else
            {
                m_pReportControllerObserver->RemoveSection(xGroup->getFooter());
            }
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
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getMutex() );
    OSL_FAIL("Not yet implemented!");
}
// -----------------------------------------------------------------------------
void SAL_CALL OReportController::propertyChange( const beans::PropertyChangeEvent& evt ) throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getMutex() );
    try
    {
        sal_Bool bShow = sal_False;
        evt.NewValue >>= bShow;
        if ( evt.Source == m_xReportDefinition )
        {
            if ( evt.PropertyName.equals( PROPERTY_REPORTHEADERON ) )
            {
                const sal_uInt16 nPosition = m_xReportDefinition->getPageHeaderOn() ? 1 : 0;
                if ( bShow )
                {
                    getDesignView()->addSection(m_xReportDefinition->getReportHeader(),DBREPORTHEADER,nPosition);
                    m_pReportControllerObserver->AddSection(m_xReportDefinition->getReportHeader());
                }
                else
                {
                    getDesignView()->removeSection(nPosition);
                }
            }
            else if ( evt.PropertyName.equals( PROPERTY_REPORTFOOTERON ) )
            {
                sal_uInt16 nPosition = getDesignView()->getSectionCount();
                if ( m_xReportDefinition->getPageFooterOn() )
                    --nPosition;
                if ( bShow )
                {
                    getDesignView()->addSection(m_xReportDefinition->getReportFooter(),DBREPORTFOOTER,nPosition);
                    m_pReportControllerObserver->AddSection(m_xReportDefinition->getReportFooter());
                }
                else
                {
                    getDesignView()->removeSection(nPosition - 1);
                }
            }
            else if ( evt.PropertyName.equals( PROPERTY_PAGEHEADERON ) )
            {
                if ( bShow )
                {
                    getDesignView()->addSection(m_xReportDefinition->getPageHeader(),DBPAGEHEADER,0);
                    m_pReportControllerObserver->AddSection(m_xReportDefinition->getPageHeader());
                }
                else
                {
                    getDesignView()->removeSection(sal_uInt16(0));
                }
            }
            else if ( evt.PropertyName.equals( PROPERTY_PAGEFOOTERON ) )
            {
                if ( bShow )
                {
                    getDesignView()->addSection(m_xReportDefinition->getPageFooter(),DBPAGEFOOTER);
                    m_pReportControllerObserver->AddSection(m_xReportDefinition->getPageFooter());
                }
                else
                {
                    getDesignView()->removeSection(getDesignView()->getSectionCount() - 1);
                }
            }
            else if (   evt.PropertyName.equals( PROPERTY_COMMAND )
                    ||  evt.PropertyName.equals( PROPERTY_COMMANDTYPE )
                    ||  evt.PropertyName.equals( PROPERTY_ESCAPEPROCESSING )
                    ||  evt.PropertyName.equals( PROPERTY_FILTER )
                    )
            {
                m_xColumns.clear();
                m_xHoldAlive.clear();
                InvalidateFeature(SID_FM_ADD_FIELD);
                if ( !getDesignView()->isAddFieldVisible() && isUiVisible() )
                    getDesignView()->toggleAddField();
            }
            /// TODO: check what we need to notify here TitleHelper
            /*else if (   evt.PropertyName.equals( PROPERTY_CAPTION ) )
                updateTitle();*/
        }
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
    // simply disambiguate
    OReportController_BASE::disposing(Source);
}

// -----------------------------------------------------------------------------
sal_uInt16 lcl_getNonVisbleGroupsBefore( const uno::Reference< report::XGroups>& _xGroups
                          ,sal_Int32 _nGroupPos
                          ,::std::mem_fun_t<sal_Bool,OGroupHelper>&_pGroupMemberFunction)
{
    uno::Reference< report::XGroup> xGroup;
    sal_uInt16 nNonVisibleGroups = 0;
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
    ::std::mem_fun_t<sal_Bool,OGroupHelper> pMemFun = ::std::mem_fun(&OGroupHelper::getHeaderOn);
    ::std::mem_fun_t<uno::Reference<report::XSection> , OGroupHelper> pMemFunSection = ::std::mem_fun(&OGroupHelper::getHeader);
    ::rtl::OUString sColor(DBGROUPHEADER);
    sal_uInt16 nPosition = 0;
    bool bHandle = false;
    if ( _sPropName.equals( PROPERTY_HEADERON ) )
    {
        nPosition = m_xReportDefinition->getPageHeaderOn() ? (m_xReportDefinition->getReportHeaderOn() ? 2 : 1) : (m_xReportDefinition->getReportHeaderOn() ? 1 : 0);
        nPosition += (static_cast<sal_uInt16>(_nGroupPos) - lcl_getNonVisbleGroupsBefore(m_xReportDefinition->getGroups(),_nGroupPos,pMemFun));
        bHandle = true;
    }
    else if ( _sPropName.equals( PROPERTY_FOOTERON ) )
    {
        pMemFun = ::std::mem_fun(&OGroupHelper::getFooterOn);
        pMemFunSection = ::std::mem_fun(&OGroupHelper::getFooter);
        nPosition = getDesignView()->getSectionCount();

        if ( m_xReportDefinition->getPageFooterOn() )
            --nPosition;
        if ( m_xReportDefinition->getReportFooterOn() )
            --nPosition;
        sColor = DBGROUPFOOTER;
        nPosition -= (static_cast<sal_uInt16>(_nGroupPos) - lcl_getNonVisbleGroupsBefore(m_xReportDefinition->getGroups(),_nGroupPos,pMemFun));
        if ( !_bShow )
            --nPosition;
        bHandle = true;
    }
    if ( bHandle )
    {
        if ( _bShow )
        {
            OGroupHelper aGroupHelper(_xGroup);
            getDesignView()->addSection(pMemFunSection(&aGroupHelper),sColor,nPosition);
        }
        else
        {
            getDesignView()->removeSection(nPosition);
        }
    }
}
//------------------------------------------------------------------------------
IMPL_LINK_NOARG(OReportController, OnClipboardChanged)
{
    OnInvalidateClipboard();
    return 0;
}
//------------------------------------------------------------------------------
void OReportController::OnInvalidateClipboard()
{
    InvalidateFeature(SID_CUT);
    InvalidateFeature(SID_COPY);
    InvalidateFeature(SID_PASTE);
}
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
        new SfxAllEnumItem(RPTUI_ID_START,PAPER_A4),
        new SfxAllEnumItem(RPTUI_ID_END,PAPER_E),
        new SvxBrushItem(ITEMID_BRUSH),
        new SfxUInt16Item(RPTUI_ID_METRIC,static_cast<sal_uInt16>(eUserMetric))
    };

    static sal_uInt16 pRanges[] =
    {
        RPTUI_ID_LRSPACE,RPTUI_ID_BRUSH,
        SID_ATTR_METRIC,SID_ATTR_METRIC,
        0
    };
    SfxItemPool* pPool( new SfxItemPool(rtl::OUString("ReportPageProperties"), RPTUI_ID_LRSPACE,RPTUI_ID_METRIC, aItemInfos, pDefaults) );
    pPool->SetDefaultMetric( SFX_MAPUNIT_100TH_MM );    // ripped, don't understand why
    pPool->FreezeIdRanges();                        // the same

    try
    {
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<SfxItemSet> pDescriptor(new SfxItemSet(*pPool, pRanges));
        SAL_WNODEPRECATED_DECLARATIONS_POP
        // fill it
        if ( _xSection.is() )
            pDescriptor->Put(SvxBrushItem(::Color(_xSection->getBackColor()),ITEMID_BRUSH));
        else
        {
            pDescriptor->Put(SvxSizeItem(RPTUI_ID_SIZE,VCLSize(getStyleProperty<awt::Size>(m_xReportDefinition,PROPERTY_PAPERSIZE))));
            pDescriptor->Put(SvxLRSpaceItem(getStyleProperty<sal_Int32>(m_xReportDefinition,PROPERTY_LEFTMARGIN)
                                            ,getStyleProperty<sal_Int32>(m_xReportDefinition,PROPERTY_RIGHTMARGIN),0,0,RPTUI_ID_LRSPACE));
            pDescriptor->Put(SvxULSpaceItem(static_cast<sal_uInt16>(getStyleProperty<sal_Int32>(m_xReportDefinition,PROPERTY_TOPMARGIN))
                                            ,static_cast<sal_uInt16>(getStyleProperty<sal_Int32>(m_xReportDefinition,PROPERTY_BOTTOMMARGIN)),RPTUI_ID_ULSPACE));
            pDescriptor->Put(SfxUInt16Item(SID_ATTR_METRIC,static_cast<sal_uInt16>(eUserMetric)));

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
                    UndoContext aUndoContext( getUndoManager(), sUndoAction );
                    const SfxPoolItem* pItem = NULL;
                    if ( SFX_ITEM_SET == pSet->GetItemState( RPTUI_ID_SIZE,sal_True,&pItem))
                    {
                        uno::Any aValue;
                        static_cast<const SvxSizeItem*>(pItem)->QueryValue(aValue,MID_SIZE_SIZE);
                        xProp->setPropertyValue(PROPERTY_PAPERSIZE,aValue);
                        resetZoomType();
                    }

                    if ( SFX_ITEM_SET == pSet->GetItemState( RPTUI_ID_LRSPACE,sal_True,&pItem))
                    {
                        Any aValue;
                        static_cast<const SvxLRSpaceItem*>(pItem)->QueryValue(aValue,MID_L_MARGIN);
                        xProp->setPropertyValue(PROPERTY_LEFTMARGIN,aValue);
                        static_cast<const SvxLRSpaceItem*>(pItem)->QueryValue(aValue,MID_R_MARGIN);
                        xProp->setPropertyValue(PROPERTY_RIGHTMARGIN,aValue);
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
                        resetZoomType();
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
    catch(const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    SfxItemPool::Free(pPool);

    for (sal_uInt16 i=0; i<sizeof(pDefaults)/sizeof(pDefaults[0]); ++i)
        delete pDefaults[i];

}

// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OReportController::attachModel(const uno::Reference< frame::XModel > & xModel) throw( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( getMutex() );

    uno::Reference< report::XReportDefinition > xReportDefinition( xModel, UNO_QUERY );
    if ( !xReportDefinition.is() )
        return sal_False;

    uno::Reference< document::XUndoManagerSupplier > xTestSuppUndo( xModel, UNO_QUERY );
    if ( !xTestSuppUndo.is() )
        return sal_False;

    m_xReportDefinition = xReportDefinition;
    return sal_True;
}

// -----------------------------------------------------------------------------
void OReportController::openSortingAndGroupingDialog()
{
    if ( !m_xReportDefinition.is() )
        return;
    if ( !m_pGroupsFloater )
    {
        m_pGroupsFloater = new OGroupsSortingDialog(getView(),!isEditable(),this);
        SvtViewOptions aDlgOpt( E_WINDOW, String::CreateFromInt32( RID_GROUPS_SORTING ) );
        if ( aDlgOpt.Exists() )
            m_pGroupsFloater->SetWindowState(::rtl::OUStringToOString(aDlgOpt.GetWindowState(), RTL_TEXTENCODING_ASCII_US));
        m_pGroupsFloater->AddEventListener(LINK(this,OReportController,EventLstHdl));
    }
    else if ( isUiVisible() )
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
void OReportController::Notify(SfxBroadcaster & /* _rBc */, SfxHint const & _rHint)
{
    if (_rHint.ISA(DlgEdHint)
        && (static_cast< DlgEdHint const & >(_rHint).GetKind()
            == RPTUI_HINT_SELECTIONCHANGED))
    {
        const sal_Int32 nSelectionCount = getDesignView()->getMarkedObjectCount();
        if ( m_nSelectionCount != nSelectionCount )
        {
            m_nSelectionCount = nSelectionCount;
            InvalidateAll();
        }
        lang::EventObject aEvent(*this);
        m_aSelectionListeners.forEach<view::XSelectionChangeListener>(
            ::boost::bind(&view::XSelectionChangeListener::selectionChanged,_1,boost::cref(aEvent)));

    }
}
// -----------------------------------------------------------------------------
void OReportController::executeMethodWithUndo(sal_uInt16 _nUndoStrId,const ::std::mem_fun_t<void,ODesignView>& _pMemfun)
{
    const String sUndoAction = String((ModuleRes(_nUndoStrId)));
    UndoContext aUndoContext( getUndoManager(), sUndoAction );
    _pMemfun( getDesignView() );
    InvalidateFeature( SID_SAVEDOC );
    InvalidateFeature( SID_UNDO );
}
// -----------------------------------------------------------------------------
void OReportController::alignControlsWithUndo(sal_uInt16 _nUndoStrId,sal_Int32 _nControlModification,bool _bAlignAtSection)
{
    const String sUndoAction = String((ModuleRes(_nUndoStrId)));
    UndoContext aUndoContext( getUndoManager(), sUndoAction );
    getDesignView()->alignMarkedObjects(_nControlModification,_bAlignAtSection);
    InvalidateFeature( SID_SAVEDOC );
    InvalidateFeature( SID_UNDO );
}
// -----------------------------------------------------------------------------
void OReportController::shrinkSectionBottom(uno::Reference<report::XSection> _xSection)
{
    const sal_Int32 nElements = _xSection->getCount();
    if (nElements == 0)
    {
        // there are no elements
        return;
    }
    const sal_Int32 nSectionHeight = _xSection->getHeight();
    sal_Int32 nMaxPositionY = 0;
    uno::Reference< report::XReportComponent> xReportComponent;

    // for every component get it's Y-position and compare it to the current Y-position
    for (int i=0;i<nElements;i++)
    {
        xReportComponent.set(_xSection->getByIndex(i), uno::UNO_QUERY);
        const sal_Int32 nReportComponentPositionY = xReportComponent->getPositionY();
        const sal_Int32 nReportComponentHeight = xReportComponent->getHeight();
        const sal_Int32 nReportComponentPositionYAndHeight = nReportComponentPositionY + nReportComponentHeight;
        nMaxPositionY = std::max(nReportComponentPositionYAndHeight, nMaxPositionY);
    }
    // now we know the minimal Y-Position and maximal Y-Position

    if (nMaxPositionY > (nSectionHeight - 7) ) // Magic Number, we use a little bit less heights for right positioning
    {
        // the lowest position is already 0
        return;
    }
    _xSection->setHeight(nMaxPositionY);
}

void OReportController::shrinkSectionTop(uno::Reference<report::XSection> _xSection)
{
    const sal_Int32 nElements = _xSection->getCount();
    if (nElements == 0)
    {
        // there are no elements
        return;
    }

    const sal_Int32 nSectionHeight = _xSection->getHeight();
    sal_Int32 nMinPositionY = nSectionHeight;
    uno::Reference< report::XReportComponent> xReportComponent;

    // for every component get it's Y-position and compare it to the current Y-position
    for (int i=0;i<nElements;i++)
    {
        xReportComponent.set(_xSection->getByIndex(i), uno::UNO_QUERY);
        const sal_Int32 nReportComponentPositionY = xReportComponent->getPositionY();
        nMinPositionY = std::min(nReportComponentPositionY, nMinPositionY);
    }
    // now we know the minimal Y-Position and maximal Y-Position
    if (nMinPositionY == 0)
    {
        // the lowest position is already 0
        return;
    }
    for (int i=0;i<nElements;i++)
    {
        xReportComponent.set(_xSection->getByIndex(i), uno::UNO_QUERY);
        const sal_Int32 nReportComponentPositionY = xReportComponent->getPositionY();
        const sal_Int32 nNewPositionY = nReportComponentPositionY - nMinPositionY;
        xReportComponent->setPositionY(nNewPositionY);
    }
    const sal_Int32 nNewSectionHeight = nSectionHeight - nMinPositionY;
    _xSection->setHeight(nNewSectionHeight);
}

void OReportController::shrinkSection(sal_uInt16 _nUndoStrId, uno::Reference<report::XSection> _xSection, sal_Int32 _nSid)
{
    if ( _xSection.is() )
    {
        const String sUndoAction = String((ModuleRes(_nUndoStrId)));
        UndoContext aUndoContext( getUndoManager(), sUndoAction );

        if (_nSid == SID_SECTION_SHRINK)
        {
            shrinkSectionTop(_xSection);
            shrinkSectionBottom(_xSection);
        }
        else if (_nSid == SID_SECTION_SHRINK_TOP)
        {
            shrinkSectionTop(_xSection);
        }
        else if (_nSid == SID_SECTION_SHRINK_BOTTOM)
        {
            shrinkSectionBottom(_xSection);
        }
    }

    InvalidateFeature( SID_SAVEDOC );
    InvalidateFeature( SID_UNDO );
}

// -----------------------------------------------------------------------------
uno::Any SAL_CALL OReportController::getViewData(void) throw( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( getMutex() );

    sal_Int32 nCommandIDs[] =
    {
        SID_GRID_VISIBLE,
        SID_GRID_USE,
        SID_HELPLINES_MOVE,
        SID_RULER,
        SID_SHOW_PROPERTYBROWSER,
        SID_PROPERTYBROWSER_LAST_PAGE,
        SID_SPLIT_POSITION
    };

    ::comphelper::NamedValueCollection aCommandProperties;
    for ( size_t i=0; i < sizeof (nCommandIDs) / sizeof (nCommandIDs[0]); ++i )
    {
        const FeatureState aFeatureState = GetState( nCommandIDs[i] );

        ::rtl::OUString sCommandURL( getURLForId( nCommandIDs[i] ).Main );
        OSL_ENSURE( sCommandURL.indexOfAsciiL( ".uno:", 5 ) == 0, "OReportController::getViewData: illegal command URL!" );
        sCommandURL = sCommandURL.copy( 5 );

        Any aCommandState;
        if ( !!aFeatureState.bChecked )
            aCommandState <<= (*aFeatureState.bChecked) ? sal_True : sal_False;
        else if ( aFeatureState.aValue.hasValue() )
            aCommandState = aFeatureState.aValue;

        aCommandProperties.put( sCommandURL, aCommandState );
    }

    ::comphelper::NamedValueCollection aViewData;
    aViewData.put( "CommandProperties", aCommandProperties.getPropertyValues() );

    if ( getDesignView() )
    {
        ::std::vector<sal_uInt16> aCollapsedPositions;
        getDesignView()->fillCollapsedSections(aCollapsedPositions);
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

            aViewData.put( "CollapsedSections", aCollapsedSections );
        }

        ::boost::shared_ptr<OSectionWindow> pSectionWindow = getDesignView()->getMarkedSection();
        if ( pSectionWindow.get() )
        {
            aViewData.put( "MarkedSection", (sal_Int32)pSectionWindow->getReportSection().getPage()->GetPageNum() );
        }
    }

    aViewData.put( "ZoomFactor", m_nZoomValue );
    return uno::makeAny( aViewData.getPropertyValues() );
}
// -----------------------------------------------------------------------------
void SAL_CALL OReportController::restoreViewData(const uno::Any& i_data) throw( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( getMutex() );

    try
    {
        const ::comphelper::NamedValueCollection aViewData( i_data );

        m_aCollapsedSections = aViewData.getOrDefault( "CollapsedSections", m_aCollapsedSections );
        m_nPageNum = aViewData.getOrDefault( "MarkedSection", m_nPageNum );
        m_nZoomValue = aViewData.getOrDefault( "ZoomFactor", m_nZoomValue );
        // TODO: setting those 3 members is not enough - in theory, restoreViewData can be called when the
        // view is fully alive, so we need to reflect those 3 values in the view.
        // (At the moment, the method is called only during construction phase)


        ::comphelper::NamedValueCollection aCommandProperties( aViewData.get( "CommandProperties" ) );
        const ::std::vector< ::rtl::OUString > aCommandNames( aCommandProperties.getNames() );

        for (   ::std::vector< ::rtl::OUString >::const_iterator commandName = aCommandNames.begin();
                commandName != aCommandNames.end();
                ++commandName
            )
        {
            const Any& rCommandValue = aCommandProperties.get( *commandName );
            if ( !rCommandValue.hasValue() )
                continue;

            if ( getView() )
            {
                util::URL aCommand;
                aCommand.Complete = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:" ) ) + *commandName;

                Sequence< PropertyValue > aCommandArgs(1);
                aCommandArgs[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Value" ) );
                aCommandArgs[0].Value = rCommandValue;

                executeUnChecked( aCommand, aCommandArgs );
            }
            else
            {
                if ( *commandName == "ShowRuler" )
                    OSL_VERIFY( rCommandValue >>= m_bShowRuler );
                else if ( *commandName == "HelplinesMove" )
                    OSL_VERIFY( rCommandValue >>= m_bHelplinesMove );
                else if ( *commandName == "GridVisible" )
                    OSL_VERIFY( rCommandValue >>= m_bGridVisible );
                else if ( *commandName == "GridUse" )
                    OSL_VERIFY( rCommandValue >>= m_bGridUse );
                else if ( *commandName == "ControlProperties" )
                    OSL_VERIFY( rCommandValue >>= m_bShowProperties );
                else if ( *commandName == "LastPropertyBrowserPage" )
                    OSL_VERIFY( rCommandValue >>= m_sLastActivePage );
                else if ( *commandName == "SplitPosition" )
                    OSL_VERIFY( rCommandValue >>= m_nSplitPos );
            }
        }
    }
    catch(const IllegalArgumentException&)
    {
        DBG_UNHANDLED_EXCEPTION();
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
        bool bEnabled = !m_xReportDefinition->getCommand().isEmpty();
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
                    getDesignView()->setCurrentPage(m_sLastActivePage);
                    nCommand = SID_SELECT_REPORT;
                }
                else if ( getDesignView() && !getDesignView()->isAddFieldVisible() )
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
            m_bInGeneratePreview = true;
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
            catch(const sdbc::SQLException&)
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

                if ( !aThirdMessage.Message.isEmpty() )
                    aSecondMessage.NextException <<= aThirdMessage;
                aFirstMessage.NextException <<= aSecondMessage;

                aInfo = aFirstMessage;
            }
            if (aInfo.isValid())
            {
                const String suSQLContext = String( ModuleRes( RID_STR_COULD_NOT_CREATE_REPORT ) );
                aInfo.prepend(suSQLContext);
            }
            m_bInGeneratePreview = false;
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

        ::boost::shared_ptr<AnyConverter> aNoConverter(new AnyConverter());
        TPropertyNamePair aPropertyMediation;
        aPropertyMediation.insert( TPropertyNamePair::value_type( PROPERTY_COMMAND, TPropertyConverter(PROPERTY_COMMAND,aNoConverter) ) );
        aPropertyMediation.insert( TPropertyNamePair::value_type( PROPERTY_COMMANDTYPE, TPropertyConverter(PROPERTY_COMMANDTYPE,aNoConverter) ) );
        aPropertyMediation.insert( TPropertyNamePair::value_type( PROPERTY_ESCAPEPROCESSING, TPropertyConverter(PROPERTY_ESCAPEPROCESSING,aNoConverter) ) );
        aPropertyMediation.insert( TPropertyNamePair::value_type( PROPERTY_FILTER, TPropertyConverter(PROPERTY_FILTER,aNoConverter) ) );

        m_xRowSetMediator = new OPropertyMediator( m_xReportDefinition.get(), xRowSetProp, aPropertyMediation );
        m_xRowSet = xRowSet;
    }
    catch(const uno::Exception&)
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
        uno::Reference< report::XSection> xSection = getDesignView()->getCurrentSection();
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
    catch(const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}
// -----------------------------------------------------------------------------
::boost::shared_ptr<rptui::OReportModel> OReportController::getSdrModel() const
{
    return m_aReportModel;
}
// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL OReportController::select( const Any& aSelection ) throw (IllegalArgumentException, RuntimeException)
{
    ::osl::MutexGuard aGuard( getMutex() );
    ::sal_Bool bRet = sal_True;
    if ( getDesignView() )
    {
        getDesignView()->unmarkAllObjects(NULL);
        getDesignView()->SetMode(RPTUI_SELECT);

        uno::Sequence< uno::Reference<report::XReportComponent> > aElements;
        if ( aSelection >>= aElements )
        {
            if ( aElements.getLength() > 0 )
                getDesignView()->showProperties(uno::Reference<uno::XInterface>(aElements[0],uno::UNO_QUERY));
            getDesignView()->setMarked(aElements,sal_True);
        }
        else
        {
            uno::Reference<uno::XInterface> xObject(aSelection,uno::UNO_QUERY);
            uno::Reference<report::XReportComponent> xProp(xObject,uno::UNO_QUERY);
            if ( xProp.is() )
            {
                getDesignView()->showProperties(xObject);
                aElements.realloc(1);
                aElements[0] = xProp;
                getDesignView()->setMarked(aElements,sal_True);
            }
            else
            {
                uno::Reference<report::XSection> xSection(aSelection,uno::UNO_QUERY);
                if ( !xSection.is() && xObject.is() )
                    getDesignView()->showProperties(xObject);
                getDesignView()->setMarked(xSection,xSection.is());
            }
        }
        InvalidateAll();
    }
    return bRet;
}
// -----------------------------------------------------------------------------
Any SAL_CALL OReportController::getSelection(  ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( getMutex() );
    Any aRet;
    if ( getDesignView() )
    {
        aRet = getDesignView()->getCurrentlyShownProperty();
        if ( !aRet.hasValue() )
            aRet <<= getDesignView()->getCurrentSection();
    }
    return aRet;
}
// -----------------------------------------------------------------------------
void SAL_CALL OReportController::addSelectionChangeListener( const Reference< view::XSelectionChangeListener >& _Listener ) throw (RuntimeException)
{
    m_aSelectionListeners.addInterface( _Listener );
}
// -----------------------------------------------------------------------------
void SAL_CALL OReportController::removeSelectionChangeListener( const Reference< view::XSelectionChangeListener >& _Listener ) throw (RuntimeException)
{
    m_aSelectionListeners.removeInterface( _Listener );
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
    executeReport();
    return 0L;
}
// -----------------------------------------------------------------------------
void OReportController::createControl(const Sequence< PropertyValue >& _aArgs,const uno::Reference< report::XSection>& _xSection,const ::rtl::OUString& _sFunction,sal_uInt16 _nObjectId)
{
    SequenceAsHashMap aMap(_aArgs);
    getDesignView()->setMarked(_xSection ,sal_True);
    ::boost::shared_ptr<OSectionWindow> pSectionWindow = getDesignView()->getMarkedSection();
    if ( !pSectionWindow )
        return;

    OSL_ENSURE(pSectionWindow->getReportSection().getSection() == _xSection,"Invalid section after marking the corrct one.");

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
        pNewControl = SdrObjFactory::MakeNewObject( ReportInventor, _nObjectId, pSectionWindow->getReportSection().getPage(),m_aReportModel.get() );
        xShapeProp.set(pNewControl->getUnoShape(),uno::UNO_QUERY);
        ::rtl::OUString sCustomShapeType = getDesignView()->GetInsertObjString();
        if ( sCustomShapeType.isEmpty() )
            sCustomShapeType = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("diamond"));
        pSectionWindow->getReportSection().createDefault(sCustomShapeType,pNewControl);
        pNewControl->SetLogicRect(Rectangle(3000,500,6000,3500)); // switch height and width
    }
    else if ( _nObjectId == OBJ_OLE2 || OBJ_DLG_SUBREPORT == _nObjectId  )
    {
        pNewControl = SdrObjFactory::MakeNewObject( ReportInventor, _nObjectId, pSectionWindow->getReportSection().getPage(),m_aReportModel.get() );

        pNewControl->SetLogicRect(Rectangle(3000,500,8000,5500)); // switch height and width
        xShapeProp.set(pNewControl->getUnoShape(),uno::UNO_QUERY_THROW);
        OOle2Obj* pObj = dynamic_cast<OOle2Obj*>(pNewControl);
        if ( pObj && !pObj->IsEmpty() )
        {
            pObj->initializeChart(getModel());
        }
    }
    else
    {
        SdrUnoObj* pLabel( NULL );
        SdrUnoObj* pControl( NULL );
        FmFormView::createControlLabelPair( getDesignView()
                            ,nLeftMargin,0
                            ,NULL,NULL,_nObjectId,::rtl::OUString(),ReportInventor,OBJ_DLG_FIXEDTEXT,
                         NULL,pSectionWindow->getReportSection().getPage(),m_aReportModel.get(),
                         pLabel,pControl);
        delete pLabel;

        pNewControl = pControl;
        OUnoObject* pObj = dynamic_cast<OUnoObject*>(pControl);
        uno::Reference<beans::XPropertySet> xUnoProp(pObj->GetUnoControlModel(),uno::UNO_QUERY);
        xShapeProp.set(pObj->getUnoShape(),uno::UNO_QUERY);
        uno::Reference<beans::XPropertySetInfo> xShapeInfo = xShapeProp->getPropertySetInfo();
        uno::Reference<beans::XPropertySetInfo> xInfo = xUnoProp->getPropertySetInfo();

        const ::rtl::OUString sProps[] = {   PROPERTY_NAME
                                            ,PROPERTY_FONTDESCRIPTOR
                                            ,PROPERTY_FONTDESCRIPTORASIAN
                                            ,PROPERTY_FONTDESCRIPTORCOMPLEX
                                            ,PROPERTY_ORIENTATION
                                            ,PROPERTY_BORDER
                                            ,PROPERTY_FORMATSSUPPLIER
                                            ,PROPERTY_BACKGROUNDCOLOR
        };
        for(size_t i = 0; i < sizeof (sProps) / sizeof (sProps[0]); ++i)
        {
            if ( xInfo->hasPropertyByName(sProps[i]) && xShapeInfo->hasPropertyByName(sProps[i]) )
                xUnoProp->setPropertyValue(sProps[i],xShapeProp->getPropertyValue(sProps[i]));
        }

        if ( xInfo->hasPropertyByName(PROPERTY_BORDER) && xShapeInfo->hasPropertyByName(PROPERTY_CONTROLBORDER) )
            xUnoProp->setPropertyValue(PROPERTY_BORDER,xShapeProp->getPropertyValue(PROPERTY_CONTROLBORDER));


        if ( xInfo->hasPropertyByName(PROPERTY_DATAFIELD) && !_sFunction.isEmpty() )
        {
            ReportFormula aFunctionFormula( ReportFormula::Expression, _sFunction );
            xUnoProp->setPropertyValue( PROPERTY_DATAFIELD, uno::makeAny( aFunctionFormula.getCompleteFormula() ) );
        }

        sal_Int32 nFormatKey = aMap.getUnpackedValueOrDefault(PROPERTY_FORMATKEY,sal_Int32(0));
        if ( nFormatKey && xInfo->hasPropertyByName(PROPERTY_FORMATKEY) )
            xUnoProp->setPropertyValue( PROPERTY_FORMATKEY, uno::makeAny( nFormatKey ) );

        ::rtl::OUString sUrl = aMap.getUnpackedValueOrDefault(PROPERTY_IMAGEURL,::rtl::OUString());
        if ( !sUrl.isEmpty() && xInfo->hasPropertyByName(PROPERTY_IMAGEURL) )
            xUnoProp->setPropertyValue( PROPERTY_IMAGEURL, uno::makeAny( sUrl ) );

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

    correctOverlapping(pNewControl,pSectionWindow->getReportSection());
}
// -----------------------------------------------------------------------------
void OReportController::createDateTime(const Sequence< PropertyValue >& _aArgs)
{
    getDesignView()->unmarkAllObjects(NULL);

    const String sUndoAction(ModuleRes(RID_STR_UNDO_INSERT_CONTROL));
    UndoContext aUndoContext( getUndoManager(), sUndoAction );

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
        sFunction = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("TIMEVALUE(NOW())"));
        aMap[PROPERTY_FORMATKEY] <<= aMap.getUnpackedValueOrDefault(PROPERTY_FORMATKEYTIME,sal_Int32(0));
        createControl(aMap.getAsConstPropertyValueList(),xSection,sFunction);
    }
}
// -----------------------------------------------------------------------------
void OReportController::createPageNumber(const Sequence< PropertyValue >& _aArgs)
{
    getDesignView()->unmarkAllObjects(NULL);

    const String sUndoAction(ModuleRes(RID_STR_UNDO_INSERT_CONTROL));
    UndoContext aUndoContext( getUndoManager(), sUndoAction );

    if ( !m_xReportDefinition->getPageHeaderOn() )
    {
        uno::Sequence< beans::PropertyValue > aArgs;
        executeChecked(SID_PAGEHEADERFOOTER,aArgs);
    }

    SequenceAsHashMap aMap(_aArgs);
    sal_Bool bStateOfPage = aMap.getUnpackedValueOrDefault(PROPERTY_STATE,sal_False);

    String sFunction = String(ModuleRes(STR_RPT_PN_PAGE));
    ::rtl::OUString sPageNumber(RTL_CONSTASCII_USTRINGPARAM("PageNumber()"));
    sFunction.SearchAndReplace(rtl::OUString("#PAGENUMBER#"),sPageNumber);

    if ( bStateOfPage )
    {
        ::rtl::OUString sPageCount(RTL_CONSTASCII_USTRINGPARAM("PageCount()"));
        sFunction += String(ModuleRes(STR_RPT_PN_PAGE_OF));
        sFunction.SearchAndReplace(rtl::OUString("#PAGECOUNT#"),sPageCount);
    }

    sal_Bool bInPageHeader = aMap.getUnpackedValueOrDefault(PROPERTY_PAGEHEADERON,sal_True);
    createControl(_aArgs,bInPageHeader ? m_xReportDefinition->getPageHeader() : m_xReportDefinition->getPageFooter(),sFunction);
}

// -----------------------------------------------------------------------------
void OReportController::addPairControls(const Sequence< PropertyValue >& aArgs)
{
    getDesignView()->unmarkAllObjects(NULL);
    //////////////////////////////////////////////////////////////////////
    // Anhand des FormatKeys wird festgestellt, welches Feld benoetigt wird
    ::boost::shared_ptr<OSectionWindow> pSectionWindow[2];
    pSectionWindow[0] = getDesignView()->getMarkedSection();

    if ( !pSectionWindow[0] )
    {
        select(uno::makeAny(m_xReportDefinition->getDetail()));
        pSectionWindow[0] = getDesignView()->getMarkedSection();
        if ( !pSectionWindow[0] )
            return;
    }

    uno::Reference<report::XSection> xCurrentSection = getDesignView()->getCurrentSection();
    UndoContext aUndoContext( getUndoManager(), String( ModuleRes( RID_STR_UNDO_INSERT_CONTROL ) ) );

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

            getDesignView()->setMarked(xSection,sal_True);
            pSectionWindow[0] = getDesignView()->getMarkedSection();

            sal_Int32 nLeftMargin = getStyleProperty<sal_Int32>(m_xReportDefinition,PROPERTY_LEFTMARGIN);
            awt::Point aPos = aMap.getUnpackedValueOrDefault(PROPERTY_POSITION,awt::Point(nLeftMargin,0));
            if ( aPos.X < nLeftMargin )
                aPos.X = nLeftMargin;

            // LLA: new feature, add the Label in dependency of the given DND_ACTION one section up, normal or one section down
            sal_Int8 nDNDAction = aMap.getUnpackedValueOrDefault(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DNDAction")), sal_Int8(0));
            pSectionWindow[1] = pSectionWindow[0];
            sal_Bool bLabelAboveTextField = nDNDAction == DND_ACTION_COPY;
            if ( bLabelAboveTextField || nDNDAction == DND_ACTION_LINK )
            {
                // Add the Label one Section up
                pSectionWindow[1] = getDesignView()->getMarkedSection(bLabelAboveTextField ? PREVIOUS : POST);
                if (!pSectionWindow[1])
                {
                    // maybe out of bounds
                    pSectionWindow[1] = pSectionWindow[0];
                }
            }
            // clear all selections
            getDesignView()->unmarkAllObjects(NULL);

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
                if ( !sCommand.isEmpty() && nCommandType != -1 && !sColumnName.isEmpty() && xConnection.is() )
                {
                    if ( xReportDefinition->getCommand().isEmpty() )
                    {
                        xReportDefinition->setCommand(sCommand);
                        xReportDefinition->setCommandType(nCommandType);
                    }

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
                    catch(const Exception&)
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
            const sal_Int32 nRightMargin = getStyleProperty<sal_Int32>(m_xReportDefinition,PROPERTY_RIGHTMARGIN);
            const sal_Int32 nPaperWidth = getStyleProperty<awt::Size>(m_xReportDefinition,PROPERTY_PAPERSIZE).Width - nRightMargin;
            OSectionView* pSectionViews[2];
            pSectionViews[0] = &pSectionWindow[1]->getReportSection().getSectionView();
            pSectionViews[1] = &pSectionWindow[0]->getReportSection().getSectionView();
            // find this in svx
            FmFormView::createControlLabelPair( getDesignView()
                ,nLeftMargin,0
                ,xField,xNumberFormats,nOBJID,::rtl::OUString(),ReportInventor,OBJ_DLG_FIXEDTEXT,
                pSectionWindow[1]->getReportSection().getPage(),pSectionWindow[0]->getReportSection().getPage(),m_aReportModel.get(),
                pControl[0],pControl[1]);
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

                        uno::Reference<beans::XPropertySetInfo> xShapeInfo = xShapeProp->getPropertySetInfo();
                        uno::Reference<beans::XPropertySetInfo> xInfo = xUnoProp->getPropertySetInfo();
                        const ::rtl::OUString sProps[] = {   PROPERTY_FONTDESCRIPTOR
                                                            ,PROPERTY_FONTDESCRIPTORASIAN
                                                            ,PROPERTY_FONTDESCRIPTORCOMPLEX
                                                            ,PROPERTY_BORDER
                                                            ,PROPERTY_BACKGROUNDCOLOR
                        };
                        for(size_t k = 0; k < sizeof(sProps)/sizeof(sProps[0]);++k)
                        {
                            if ( xInfo->hasPropertyByName(sProps[k]) && xShapeInfo->hasPropertyByName(sProps[k]) )
                                xUnoProp->setPropertyValue(sProps[k],xShapeProp->getPropertyValue(sProps[k]));
                        }
                        if ( xInfo->hasPropertyByName(PROPERTY_DATAFIELD) )
                        {
                            ::rtl::OUString sName;
                            xUnoProp->getPropertyValue(PROPERTY_DATAFIELD) >>= sName;
                            sDefaultName = sName;
                            xUnoProp->setPropertyValue(PROPERTY_NAME,uno::makeAny(sDefaultName));

                            ReportFormula aFormula( ReportFormula::Field, sName );
                            xUnoProp->setPropertyValue( PROPERTY_DATAFIELD, uno::makeAny( aFormula.getCompleteFormula() ) );
                        }

                        if ( xInfo->hasPropertyByName(PROPERTY_BORDER) && xShapeInfo->hasPropertyByName(PROPERTY_CONTROLBORDER) )
                            xUnoProp->setPropertyValue(PROPERTY_BORDER,xShapeProp->getPropertyValue(PROPERTY_CONTROLBORDER));

                        pObjs[i]->CreateMediator(sal_True);

                        const sal_Int32 nShapeWidth = xShapeProp->getWidth();
                        const bool bChangedPos = (aPos.X + nShapeWidth) > nPaperWidth;
                        if ( bChangedPos )
                            aPos.X = nPaperWidth - nShapeWidth;
                        xShapeProp->setPosition(aPos);
                        if ( bChangedPos )
                            aPos.Y += xShapeProp->getHeight();
                        aPos.X += nShapeWidth;
                    }
                    ::rtl::OUString sLabel;
                    if ( xField->getPropertySetInfo()->hasPropertyByName(PROPERTY_LABEL) )
                        xField->getPropertyValue(PROPERTY_LABEL) >>= sLabel;

                    if (pSectionViews[0] != pSectionViews[1] &&
                        nOBJID == OBJ_DLG_FORMATTEDFIELD) // we want this nice feature only at FORMATTEDFIELD
                    {
                        uno::Reference< report::XReportComponent> xShapePropLabel(pObjs[0]->getUnoShape(),uno::UNO_QUERY_THROW);
                        uno::Reference< report::XReportComponent> xShapePropTextField(pObjs[1]->getUnoShape(),uno::UNO_QUERY_THROW);
                        if ( !sLabel.isEmpty() )
                            xShapePropTextField->setName(sLabel);
                        awt::Point aPosLabel = xShapePropLabel->getPosition();
                        awt::Point aPosTextField = xShapePropTextField->getPosition();
                        aPosTextField.X = aPosLabel.X;
                        xShapePropTextField->setPosition(aPosTextField);
                        if (bLabelAboveTextField)
                        {
                            // move the label down near the splitter
                            const uno::Reference<report::XSection> xLabelSection = pSectionWindow[1]->getReportSection().getSection();
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
                    uno::Reference< report::XFixedText> xShapeProp(pObj->getUnoShape(),uno::UNO_QUERY_THROW);
                    xShapeProp->setName(xShapeProp->getName() + sDefaultName );

                    for(i = 0; i < sizeof(pControl)/sizeof(pControl[0]);++i) // insert controls
                    {
                        correctOverlapping(pControl[i],pSectionWindow[1-i]->getReportSection());
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
                                const SdrObject* pOverlappedObj = isOver(aLabelAndTextfield, *pSectionWindow[0]->getReportSection().getPage(), *pSectionViews[0], true, pControl, 2);
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
    catch(const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

// -----------------------------------------------------------------------------
OSectionView* OReportController::getCurrentSectionView() const
{
    OSectionView* pSectionView = NULL;
    ::boost::shared_ptr<OSectionWindow> pSectionWindow = getDesignView()->getMarkedSection();
    if ( pSectionWindow.get() )
        pSectionView = &pSectionWindow->getReportSection().getSectionView();
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
            case SID_FRAME_TO_BOTTOM:
                pSectionView->PutMarkedToBtm();
                break;
            case SID_FRAME_TO_TOP:
                pSectionView->PutMarkedToTop();
                break;
            case SID_FRAME_DOWN:
                pSectionView->MovMarkedToBtm();
                break;
            case SID_FRAME_UP:
                pSectionView->MovMarkedToTop();
                break;

            case SID_OBJECT_HEAVEN:
                pSectionView->SetMarkedToLayer( RPT_LAYER_FRONT );
                break;
            case SID_OBJECT_HELL:
                pSectionView->SetMarkedToLayer( RPT_LAYER_BACK );
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

    // Add Listeners to UndoEnvironment
    void (OXUndoEnvironment::*pElementUndoFunction)( const uno::Reference< uno::XInterface >& ) =
        _bAdd ? &OXUndoEnvironment::AddElement : &OXUndoEnvironment::RemoveElement;

    (rUndoEnv.*pElementUndoFunction)( m_xReportDefinition->getStyleFamilies() );
    (rUndoEnv.*pElementUndoFunction)( m_xReportDefinition->getFunctions() );

    // Add Listeners to ReportControllerObserver
    OXReportControllerObserver& rObserver = *m_pReportControllerObserver;

    if ( m_xReportDefinition->getPageHeaderOn() && _bAdd )
    {
        getDesignView()->addSection(m_xReportDefinition->getPageHeader(),DBPAGEHEADER);
        rObserver.AddSection(m_xReportDefinition->getPageHeader());
    }
    if ( m_xReportDefinition->getReportHeaderOn() && _bAdd )
    {
        getDesignView()->addSection(m_xReportDefinition->getReportHeader(),DBREPORTHEADER);
        rObserver.AddSection(m_xReportDefinition->getReportHeader());
    }

    uno::Reference< report::XGroups > xGroups = m_xReportDefinition->getGroups();
    const sal_Int32 nCount = xGroups->getCount();
    _bAdd ? xGroups->addContainerListener(&rUndoEnv) : xGroups->removeContainerListener(&rUndoEnv);
    _bAdd ? xGroups->addContainerListener(&rObserver) : xGroups->removeContainerListener(&rObserver);

    for (sal_Int32 i=0;i<nCount ; ++i)
    {
        uno::Reference< report::XGroup > xGroup(xGroups->getByIndex(i),uno::UNO_QUERY);
        (xGroup.get()->*pPropertyListenerAction)( PROPERTY_HEADERON, static_cast< XPropertyChangeListener* >( this ) );
        (xGroup.get()->*pPropertyListenerAction)( PROPERTY_FOOTERON, static_cast< XPropertyChangeListener* >( this ) );

        (rUndoEnv.*pElementUndoFunction)( xGroup );
        (rUndoEnv.*pElementUndoFunction)( xGroup->getFunctions() );
        if ( xGroup->getHeaderOn() && _bAdd )
        {
            getDesignView()->addSection(xGroup->getHeader(),DBGROUPHEADER);
            rObserver.AddSection(xGroup->getHeader());
        }
    }

    if ( _bAdd )
    {
        getDesignView()->addSection(m_xReportDefinition->getDetail(),DBDETAIL);
        rObserver.AddSection(m_xReportDefinition->getDetail());

        for (sal_Int32 i=nCount;i > 0 ; --i)
        {
            uno::Reference< report::XGroup > xGroup(xGroups->getByIndex(i-1),uno::UNO_QUERY);
            if ( xGroup->getFooterOn() )
            {
                getDesignView()->addSection(xGroup->getFooter(),DBGROUPFOOTER);
                rObserver.AddSection(xGroup->getFooter());
            }
        }
        if ( m_xReportDefinition->getReportFooterOn() )
        {
            getDesignView()->addSection(m_xReportDefinition->getReportFooter(),DBREPORTFOOTER);
            rObserver.AddSection(m_xReportDefinition->getReportFooter());
        }
        if ( m_xReportDefinition->getPageFooterOn())
        {
            getDesignView()->addSection(m_xReportDefinition->getPageFooter(),DBPAGEFOOTER);
            rObserver.AddSection(m_xReportDefinition->getPageFooter());
        }

        xGroups->addContainerListener(static_cast<XContainerListener*>(this));
        m_xReportDefinition->addModifyListener(static_cast<XModifyListener*>(this));
    }
    else /* ! _bAdd */
    {
        rObserver.RemoveSection(m_xReportDefinition->getDetail());
        xGroups->removeContainerListener(static_cast<XContainerListener*>(this));
        m_xReportDefinition->removeModifyListener(static_cast<XModifyListener*>(this));
        m_aReportModel->detachController();
    }
}
// -----------------------------------------------------------------------------
void OReportController::switchReportSection(const sal_Int16 _nId)
{
    OSL_ENSURE(_nId == SID_REPORTHEADER_WITHOUT_UNDO || _nId == SID_REPORTFOOTER_WITHOUT_UNDO || _nId == SID_REPORTHEADERFOOTER ,"Illegal id given!");

    if ( m_xReportDefinition.is() )
    {
        const OXUndoEnvironment::OUndoEnvLock aLock( m_aReportModel->GetUndoEnv() );
        const bool bSwitchOn = !m_xReportDefinition->getReportHeaderOn();

        ::boost::scoped_ptr< UndoContext > pUndoContext;
        if ( SID_REPORTHEADERFOOTER == _nId )
        {
            const String sUndoAction(ModuleRes(bSwitchOn ? RID_STR_UNDO_ADD_REPORTHEADERFOOTER : RID_STR_UNDO_REMOVE_REPORTHEADERFOOTER));
            pUndoContext.reset( new UndoContext( getUndoManager(), sUndoAction ) );

            addUndoAction(new OReportSectionUndo(*(m_aReportModel),SID_REPORTHEADER_WITHOUT_UNDO
                                                            ,::std::mem_fun(&OReportHelper::getReportHeader)
                                                            ,m_xReportDefinition
                                                            ,bSwitchOn ? Inserted : Removed
                                                            ,0
                                                            ));

            addUndoAction(new OReportSectionUndo(*(m_aReportModel),SID_REPORTFOOTER_WITHOUT_UNDO
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
            pUndoContext.reset();
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

        ::boost::scoped_ptr< UndoContext > pUndoContext;
        if ( SID_PAGEHEADERFOOTER == _nId )
        {
            const String sUndoAction(ModuleRes(bSwitchOn ? RID_STR_UNDO_ADD_REPORTHEADERFOOTER : RID_STR_UNDO_REMOVE_REPORTHEADERFOOTER));
            pUndoContext.reset( new UndoContext( getUndoManager(), sUndoAction ) );

            addUndoAction(new OReportSectionUndo(*m_aReportModel
                                                            ,SID_PAGEHEADER_WITHOUT_UNDO
                                                            ,::std::mem_fun(&OReportHelper::getPageHeader)
                                                            ,m_xReportDefinition
                                                            ,bSwitchOn ? Inserted : Removed
                                                            ,0
                                                            ));

            addUndoAction(new OReportSectionUndo(*m_aReportModel
                                                            ,SID_PAGEFOOTER_WITHOUT_UNDO
                                                            ,::std::mem_fun(&OReportHelper::getPageFooter)
                                                            ,m_xReportDefinition
                                                            ,bSwitchOn ? Inserted : Removed
                                                            ,0
                                                            ));
        }
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
            pUndoContext.reset();
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

        addUndoAction( new OGroupUndo(
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
    catch(const Exception&)
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
                addUndoAction(new OGroupSectionUndo(*m_aReportModel
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
void OReportController::collapseSection(const bool _bCollapse)
{
    ::boost::shared_ptr<OSectionWindow> pSection = getDesignView()->getMarkedSection();
    if ( pSection )
    {
        pSection->setCollapsed(_bCollapse);
    }
}
// -----------------------------------------------------------------------------
void OReportController::markSection(const bool _bNext)
{
    ::boost::shared_ptr<OSectionWindow> pSection = getDesignView()->getMarkedSection();
    if ( pSection )
    {
        ::boost::shared_ptr<OSectionWindow> pPrevSection = getDesignView()->getMarkedSection(_bNext ? POST : PREVIOUS);
        if ( pPrevSection != pSection && pPrevSection )
            select(uno::makeAny(pPrevSection->getReportSection().getSection()));
        else
            select(uno::makeAny(m_xReportDefinition));
    }
    else
    {
        getDesignView()->markSection(_bNext ? 0 : getDesignView()->getSectionCount() - 1);
        pSection = getDesignView()->getMarkedSection();
        if ( pSection )
            select(uno::makeAny(pSection->getReportSection().getSection()));
    }
}
// -----------------------------------------------------------------------------
void OReportController::createDefaultControl(const uno::Sequence< beans::PropertyValue>& _aArgs)
{
    uno::Reference< report::XSection > xSection = getDesignView()->getCurrentSection();
    if ( !xSection.is() )
        xSection = m_xReportDefinition->getDetail();

    if ( xSection.is() )
    {
        const ::rtl::OUString sKeyModifier(RTL_CONSTASCII_USTRINGPARAM("KeyModifier"));
        const beans::PropertyValue* pIter = _aArgs.getConstArray();
        const beans::PropertyValue* pEnd  = pIter + _aArgs.getLength();
        const beans::PropertyValue* pKeyModifier = ::std::find_if(pIter,pEnd,::std::bind2nd(PropertyValueCompare(),boost::cref(sKeyModifier)));
        sal_Int16 nKeyModifier = 0;
        if ( pKeyModifier == pEnd || ((pKeyModifier->Value >>= nKeyModifier) && nKeyModifier == KEY_MOD1) )
        {
            Sequence< PropertyValue > aCreateArgs;
            getDesignView()->unmarkAllObjects(NULL);
            createControl(aCreateArgs,xSection,::rtl::OUString(),getDesignView()->GetInsertObj());
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
// -----------------------------------------------------------------------------

// css.frame.XTitle
::rtl::OUString SAL_CALL OReportController::getTitle()
    throw (uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getMutex() );

    uno::Reference< frame::XTitle> xTitle(m_xReportDefinition,uno::UNO_QUERY_THROW);

    return xTitle->getTitle ();
}
// -----------------------------------------------------------------------------
void OReportController::getPropertyDefaultByHandle( sal_Int32 /*_nHandle*/, Any& _rDefault ) const
{
    _rDefault <<= sal_Int16(100);
}
// -----------------------------------------------------------------------------
// comphelper::OPropertyArrayUsageHelper
::cppu::IPropertyArrayHelper* OReportController::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}
// -------------------------------------------------------------------------

// cppu::OPropertySetHelper
::cppu::IPropertyArrayHelper& SAL_CALL OReportController::getInfoHelper()
{
    typedef ::comphelper::OPropertyArrayUsageHelper<OReportController_BASE> OReportController_PROP;
    return *OReportController_PROP::getArrayHelper();
}
// -----------------------------------------------------------------------------
void SAL_CALL OReportController::setFastPropertyValue_NoBroadcast(sal_Int32 _nHandle,const Any& _aValue) throw (Exception)
{
    if ( _nHandle == PROPERTY_ID_ZOOMVALUE )
    {
        _aValue >>= m_nZoomValue;
        impl_zoom_nothrow();
    }
}
void SAL_CALL OReportController::setMode( const ::rtl::OUString& aMode ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( getMutex() );
    m_sMode = aMode;
}
::rtl::OUString SAL_CALL OReportController::getMode(  ) throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( getMutex() );
    return m_sMode;
}
::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL OReportController::getSupportedModes(  ) throw (::com::sun::star::uno::RuntimeException)
{
    static ::rtl::OUString s_sModes[] = { ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("remote")),
                                          ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("normal")) };
    return uno::Sequence< ::rtl::OUString> (&s_sModes[0],sizeof(s_sModes)/sizeof(s_sModes[0]));
}
::sal_Bool SAL_CALL OReportController::supportsMode( const ::rtl::OUString& aMode ) throw (::com::sun::star::uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString> aModes = getSupportedModes();
    const ::rtl::OUString* pIter = aModes.getConstArray();
    const ::rtl::OUString* pEnd  = pIter + aModes.getLength();
    for(;pIter != pEnd;++pIter)
    {
        if ( pIter->equals(aMode ) )
            break;
    }
    return pIter != pEnd;
}
// -----------------------------------------------------------------------------
bool OReportController::isUiVisible() const
{
    return m_sMode != "remote";
}
// -----------------------------------------------------------------------------
void OReportController::impl_fillState_nothrow(const ::rtl::OUString& _sProperty,dbaui::FeatureState& _rState) const
{
    _rState.bEnabled = isEditable();
    if ( _rState.bEnabled )
    {
        ::std::vector< uno::Reference< uno::XInterface > > aSelection;
        getDesignView()->fillControlModelSelection(aSelection);
        _rState.bEnabled = !aSelection.empty();
        if ( _rState.bEnabled )
        {
            uno::Any aTemp;
            ::std::vector< uno::Reference< uno::XInterface > >::iterator aIter = aSelection.begin();
            for(; aIter != aSelection.end() && _rState.bEnabled ;++aIter)
            {
                uno::Reference< beans::XPropertySet> xProp(*aIter,uno::UNO_QUERY);
                try
                {
                    uno::Any aTemp2 = xProp->getPropertyValue(_sProperty);
                    if ( aIter == aSelection.begin() )
                    {
                        aTemp = aTemp2;
                    }
                    else if ( !comphelper::compare(aTemp,aTemp2) )
                        break;
                }
                catch(const beans::UnknownPropertyException&)
                {
                    _rState.bEnabled = sal_False;
                }
            }
            if ( aIter == aSelection.end() )
                _rState.aValue = aTemp;
        }
    }
}
// -----------------------------------------------------------------------------
void OReportController::impl_zoom_nothrow()
{
    Fraction aZoom(m_nZoomValue,100);
    setZoomFactor( aZoom,*getDesignView() );
    getDesignView()->zoom(aZoom);
    InvalidateFeature(SID_ATTR_ZOOM,Reference< XStatusListener >(),sal_True);
    InvalidateFeature(SID_ATTR_ZOOMSLIDER,Reference< XStatusListener >(),sal_True);
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
            }
        }
        catch(const uno::Exception&)
        {
        }
    }
    return bRet;
}
// -----------------------------------------------------------------------------
bool OReportController::impl_setPropertyAtControls_throw(const sal_uInt16 _nUndoResId,const ::rtl::OUString& _sProperty,const uno::Any& _aValue,const Sequence< PropertyValue >& _aArgs)
{
    ::std::vector< uno::Reference< uno::XInterface > > aSelection;
    uno::Reference< awt::XWindow> xWindow;
    lcl_getReportControlFormat( _aArgs, getDesignView(), xWindow, aSelection );
    ::std::vector< uno::Reference< uno::XInterface > >::iterator aIter = aSelection.begin();

    const String sUndoAction = String( ModuleRes( _nUndoResId ) );
    UndoContext aUndoContext( getUndoManager(), sUndoAction );

    for(;  aIter != aSelection.end();++aIter)
    {
        const uno::Reference< beans::XPropertySet > xControlModel(*aIter,uno::UNO_QUERY);
        if ( xControlModel.is() )
            xControlModel->setPropertyValue(_sProperty,_aValue);
    }

    return !aSelection.empty();
}
// -----------------------------------------------------------------------------
void OReportController::impl_fillCustomShapeState_nothrow(const char* _pCustomShapeType,dbaui::FeatureState& _rState) const
{
    _rState.bEnabled = isEditable();
    _rState.bChecked = getDesignView()->GetInsertObj() == OBJ_CUSTOMSHAPE && getDesignView()->GetInsertObjString().compareToAscii(_pCustomShapeType) == 0;
}

// -----------------------------------------------------------------------------
::boost::shared_ptr<OSectionWindow> OReportController::getSectionWindow(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>& _xSection) const
{
    if ( getDesignView() )
    {
        return  getDesignView()->getSectionWindow(_xSection);
    }

    // throw NullPointerException?
    ::boost::shared_ptr<OSectionWindow> pEmpty;
    return pEmpty;
}


// -----------------------------------------------------------------------------
void OReportController::openZoomDialog()
{
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    if ( pFact )
    {
        static SfxItemInfo aItemInfos[] =
        {
            { SID_ATTR_ZOOM, SFX_ITEM_POOLABLE }
        };
        SfxPoolItem* pDefaults[] =
        {
            new SvxZoomItem()
        };
        static sal_uInt16 pRanges[] =
        {
            SID_ATTR_ZOOM,SID_ATTR_ZOOM,
            0
        };
        SfxItemPool* pPool( new SfxItemPool(rtl::OUString("ZoomProperties"), SID_ATTR_ZOOM,SID_ATTR_ZOOM, aItemInfos, pDefaults) );
        pPool->SetDefaultMetric( SFX_MAPUNIT_100TH_MM );    // ripped, don't understand why
        pPool->FreezeIdRanges();                        // the same
        try
        {
            SAL_WNODEPRECATED_DECLARATIONS_PUSH
            ::std::auto_ptr<SfxItemSet> pDescriptor(new SfxItemSet(*pPool, pRanges));
            SAL_WNODEPRECATED_DECLARATIONS_POP
            // fill it
            SvxZoomItem aZoomItem( m_eZoomType, m_nZoomValue, SID_ATTR_ZOOM );
            aZoomItem.SetValueSet(SVX_ZOOM_ENABLE_100|SVX_ZOOM_ENABLE_WHOLEPAGE|SVX_ZOOM_ENABLE_PAGEWIDTH);
            pDescriptor->Put(aZoomItem);

            SAL_WNODEPRECATED_DECLARATIONS_PUSH
            ::std::auto_ptr<AbstractSvxZoomDialog> pDlg( pFact->CreateSvxZoomDialog(NULL, *pDescriptor.get()) );
            SAL_WNODEPRECATED_DECLARATIONS_POP
            pDlg->SetLimits( 20, 400 );
            bool bCancel = ( RET_CANCEL == pDlg->Execute() );

            if ( !bCancel )
            {
                const SvxZoomItem&  rZoomItem = (const SvxZoomItem&)pDlg->GetOutputItemSet()->Get( SID_ATTR_ZOOM );
                m_eZoomType = rZoomItem.GetType();
                m_nZoomValue = rZoomItem.GetValue();
                if ( m_eZoomType != SVX_ZOOM_PERCENT )
                    m_nZoomValue = getDesignView()->getZoomFactor( m_eZoomType );

                impl_zoom_nothrow();
            }
        }
        catch(const uno::Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        SfxItemPool::Free(pPool);

        for (sal_uInt16 i=0; i<sizeof(pDefaults)/sizeof(pDefaults[0]); ++i)
            delete pDefaults[i];
    }
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// XVisualObject
void SAL_CALL OReportController::setVisualAreaSize( ::sal_Int64 _nAspect, const awt::Size& _aSize ) throw (lang::IllegalArgumentException, embed::WrongStateException, uno::Exception, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( getMutex() );
        bool bChanged =
            (m_aVisualAreaSize.Width != _aSize.Width ||
             m_aVisualAreaSize.Height != _aSize.Height);
        m_aVisualAreaSize = _aSize;
        if( bChanged )
            setModified( sal_True );
    m_nAspect = _nAspect;
}
// -----------------------------------------------------------------------------
awt::Size SAL_CALL OReportController::getVisualAreaSize( ::sal_Int64 /*nAspect*/ ) throw (lang::IllegalArgumentException, embed::WrongStateException, uno::Exception, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( getMutex() );
    return m_aVisualAreaSize;
}
// -----------------------------------------------------------------------------
embed::VisualRepresentation SAL_CALL OReportController::getPreferredVisualRepresentation( ::sal_Int64 _nAspect ) throw (lang::IllegalArgumentException, embed::WrongStateException, uno::Exception, uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getMutex() );
    embed::VisualRepresentation aResult;
    if ( !m_bInGeneratePreview )
    {
        m_bInGeneratePreview = true;
        try
        {
            if ( !m_xReportEngine.is() )
                m_xReportEngine.set(getORB()->createInstance(SERVICE_REPORTENGINE),uno::UNO_QUERY_THROW);
            const sal_Int32 nOldMaxRows = m_xReportEngine->getMaxRows();
            m_xReportEngine->setMaxRows(MAX_ROWS_FOR_PREVIEW);
            m_xReportEngine->setReportDefinition(m_xReportDefinition);
            m_xReportEngine->setActiveConnection(getConnection());
            try
            {
                Reference<embed::XVisualObject> xTransfer(m_xReportEngine->createDocumentModel(),UNO_QUERY);
                if ( xTransfer.is() )
                {
                    xTransfer->setVisualAreaSize(m_nAspect,m_aVisualAreaSize);
                    aResult = xTransfer->getPreferredVisualRepresentation( _nAspect );
                }
            }
            catch(const uno::Exception&)
            {
            }
            m_xReportEngine->setMaxRows(nOldMaxRows);
        }
        catch(const uno::Exception&)
        {
        }
        m_bInGeneratePreview = false;
    }
    return aResult;
}
// -----------------------------------------------------------------------------
::sal_Int32 SAL_CALL OReportController::getMapUnit( ::sal_Int64 /*nAspect*/ ) throw (uno::Exception, uno::RuntimeException)
{
    return embed::EmbedMapUnits::ONE_100TH_MM;
}
// -----------------------------------------------------------------------------
uno::Reference< container::XNameAccess > OReportController::getColumns() const
{
    if ( !m_xColumns.is() && m_xReportDefinition.is() && !m_xReportDefinition->getCommand().isEmpty() )
    {
        m_xColumns = dbtools::getFieldsByCommandDescriptor(getConnection(),m_xReportDefinition->getCommandType(),m_xReportDefinition->getCommand(),m_xHoldAlive);
    }
    return m_xColumns;
}
// -----------------------------------------------------------------------------
::rtl::OUString OReportController::getColumnLabel_throw(const ::rtl::OUString& i_sColumnName) const
{
    ::rtl::OUString sLabel;
    uno::Reference< container::XNameAccess > xColumns = getColumns();
    if ( xColumns.is() && xColumns->hasByName(i_sColumnName) )
    {
        uno::Reference< beans::XPropertySet> xColumn(xColumns->getByName(i_sColumnName),uno::UNO_QUERY_THROW);
        if ( xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_LABEL) )
            xColumn->getPropertyValue(PROPERTY_LABEL) >>= sLabel;
    }
    return sLabel;
}

// -----------------------------------------------------------------------------
SfxUndoManager& OReportController::getUndoManager() const
{
    DBG_TESTSOLARMUTEX();
        // this is expected to be called during UI actions, so the SM is assumed to be locked

    ::boost::shared_ptr< OReportModel > pReportModel( getSdrModel() );
    ENSURE_OR_THROW( !!pReportModel, "no access to our model" );

    SfxUndoManager* pUndoManager( pReportModel->GetSdrUndoManager() );
    ENSURE_OR_THROW( pUndoManager != NULL, "no access to our model's UndoManager" );

    return *pUndoManager;
}

// -----------------------------------------------------------------------------
void OReportController::clearUndoManager() const
{
    getUndoManager().Clear();
}

// -----------------------------------------------------------------------------
void OReportController::addUndoAction( SfxUndoAction* i_pAction )
{
    getUndoManager().AddUndoAction( i_pAction );

    InvalidateFeature( SID_UNDO );
    InvalidateFeature( SID_REDO );
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
