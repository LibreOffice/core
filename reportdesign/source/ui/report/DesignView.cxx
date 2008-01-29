/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DesignView.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 13:50:32 $
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

#ifndef RPT_DESIGNVIEW_HXX
#include "DesignView.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef DBUI_TABLECONTROLLER_HXX
#include "ReportController.hxx"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_SYSLOCALE_HXX
#include <svtools/syslocale.hxx>
#endif
#ifndef _REPORT_RPTUIDEF_HXX
#include "RptDef.hxx"
#endif
#include "UITools.hxx"
#include "RptObject.hxx"
#ifndef _REPORT_PROPBRW_HXX
#include "propbrw.hxx"
#endif
#ifndef _TOOLKIT_HELPER_CONVERT_HXX_
#include <toolkit/helper/convert.hxx>
#endif
#ifndef RTPUI_REPORTDESIGN_HELPID_HRC
#include "helpids.hrc"
#endif
#ifndef _REPORT_SECTIONVIEW_HXX
#include "SectionView.hxx"
#endif
#ifndef REPORT_REPORTSECTION_HXX
#include "ReportSection.hxx"
#endif
#ifndef _RPTUI_SLOTID_HRC_
#include "rptui_slotid.hrc"
#endif
#ifndef _SBASLTID_HRC
#include <svx/svxids.hrc>
#endif
#ifndef RPTUI_ADDFIELDWINDOW_HXX
#include "AddField.hxx"
#endif
#ifndef RPTUI_SCROLLHELPER_HXX
#include "ScrollHelper.hxx"
#endif
#ifndef INCLUDED_RPTUI_NAVIGATOR_HXX
#include "Navigator.hxx"
#endif
#include <vcl/svapp.hxx>

namespace rptui
{
using namespace ::dbaui;
using namespace ::utl;
using namespace ::com::sun::star;
using namespace uno;
using namespace lang;
using namespace beans;
using namespace container;

#define LINE_SIZE           50
#define START_SIZE_TASKPANE 30
#define COLSET_ID           1
#define REPORT_ID           2
#define TASKPANE_ID         3

class OTaskWindow : public Window
{
    PropBrw* m_pPropWin;
public:
    OTaskWindow(Window* _pParent) : Window(_pParent),m_pPropWin(NULL){}

    inline void setPropertyBrowser(PropBrw* _pPropWin)
    {
        m_pPropWin = _pPropWin;
    }

    virtual void Resize()
    {
        const Size aSize = GetOutputSizePixel();
        if ( m_pPropWin && aSize.Height() && aSize.Width() )
            m_pPropWin->SetSizePixel(aSize);
    }
    long getMinimumWidth() const
    {
        long nRet = 0;
        if ( m_pPropWin )
            nRet = m_pPropWin->getMinimumSize().Width();
        return nRet;
    }
};
class OwnSplitWindow : public SplitWindow
{
public:
    OwnSplitWindow(Window* pParent) : SplitWindow(pParent,WB_DIALOGCONTROL){SetBackground( );}

    virtual void        Split()
    {
        SplitWindow::Split();
        setItemSizes();
    }
    void setItemSizes()
    {
        const long nOutWidth = GetOutputSizePixel().Width();
        long    nTaskPaneMinSplitSize = static_cast<OTaskWindow*>(GetItemWindow(TASKPANE_ID))->getMinimumWidth();
        nTaskPaneMinSplitSize = static_cast<long>(nTaskPaneMinSplitSize*100/nOutWidth);
        if ( !nTaskPaneMinSplitSize )
            nTaskPaneMinSplitSize = START_SIZE_TASKPANE;

        const long nReportMinSplitSize = static_cast<long>(12000/nOutWidth);

        long nReportSize = GetItemSize( REPORT_ID );
        long nTaskPaneSize = GetItemSize( TASKPANE_ID );

        BOOL        bMod = FALSE;
        if( nReportSize < nReportMinSplitSize )
        {
            nReportSize = nReportMinSplitSize;
            nTaskPaneSize = 99 - nReportMinSplitSize;

            bMod = TRUE;
        }
        else if( nTaskPaneSize < nTaskPaneMinSplitSize )
        {
            nTaskPaneSize = nTaskPaneMinSplitSize;
            nReportSize = 99 - nTaskPaneMinSplitSize;

            bMod = TRUE;
        }

        if( bMod )
        {
            SetItemSize( REPORT_ID, nReportSize );
            SetItemSize( TASKPANE_ID, nTaskPaneSize );
        }
    }
};
//==================================================================
// class ODesignView
//==================================================================
DBG_NAME( rpt_ODesignView )
//------------------------------------------------------------------------------
ODesignView::ODesignView(   Window* pParent,
                            const Reference< XMultiServiceFactory >& _rxOrb,
                            OReportController* _pController) :
    ODataView( pParent,_pController,_rxOrb,WB_DIALOGCONTROL )
    //,m_aSplitter(this,WB_HSCROLL)
    ,m_pReportController( _pController )
    ,m_pPropWin(NULL)
    ,m_pAddField(NULL)
    ,m_pCurrentView(NULL)
    ,m_pReportExplorer(NULL)
    ,m_eMode( RPTUI_SELECT )
    ,m_nCurrentPosition(USHRT_MAX)
    ,m_eActObj( OBJ_NONE )
    ,m_bFirstDraw(FALSE)
    ,m_aGridSize( 250, 250 )    // 100TH_MM
    ,m_bGridVisible(TRUE)
    ,m_bGridSnap(TRUE)
    ,m_bInSplitHandler( FALSE )
{
    DBG_CTOR( rpt_ODesignView,NULL);
    SetHelpId(UID_RPT_RPT_APP_VIEW);
    ImplInitSettings();

    m_pSplitWin = new OwnSplitWindow( this );

    SetMapMode( MapMode( MAP_100TH_MM ) );

    m_pScrollWindow = new OScrollWindowHelper(this);
    //m_pScrollWindow->Show();

    // now create the task pane on the right side :-)
    m_pTaskPane = new OTaskWindow(this);
    //m_pTaskPane->Show();

    m_pSplitWin->InsertItem( COLSET_ID,100,SPLITWINDOW_APPEND, 0, SWIB_PERCENTSIZE | SWIB_COLSET );
    m_pSplitWin->InsertItem( REPORT_ID, m_pScrollWindow, 100/*m_pScrollWindow->getMaxMarkerWidth(sal_False)*/, SPLITWINDOW_APPEND, COLSET_ID, SWIB_PERCENTSIZE  /*SWIB_COLSET*/);
    //m_pSplitWin->InsertItem( TASKPANE_ID, m_pTaskPane, 50, SPLITWINDOW_APPEND, 0, SWIB_PERCENTSIZE );

    // Splitter einrichten
    //m_aSplitter.SetSplitHdl(LINK(this, ODesignView,SplitHdl));
    m_pSplitWin->SetSplitHdl(LINK(this, ODesignView,SplitHdl));
    m_pSplitWin->ShowAutoHideButton();
    m_pSplitWin->SetAlign(WINDOWALIGN_LEFT);
    m_pSplitWin->Show();

    m_aMarkTimer.SetTimeout( 100 );
    m_aMarkTimer.SetTimeoutHdl( LINK( this, ODesignView, MarkTimeout ) );
}

//------------------------------------------------------------------------------
ODesignView::~ODesignView()
{
    DBG_DTOR( rpt_ODesignView,NULL);
    m_aMarkTimer.Stop();
    if ( m_pPropWin )
    {
        notifySystemWindow(this,m_pPropWin,::comphelper::mem_fun(&TaskPaneList::RemoveWindow));
        ::std::auto_ptr<Window> aTemp2(m_pPropWin);
        m_pPropWin = NULL;
    }
    if ( m_pAddField )
    {
        notifySystemWindow(this,m_pAddField,::comphelper::mem_fun(&TaskPaneList::RemoveWindow));
        ::std::auto_ptr<Window> aTemp2(m_pAddField);
        m_pAddField = NULL;
    }
    if ( m_pReportExplorer )
    {
        notifySystemWindow(this,m_pReportExplorer,::comphelper::mem_fun(&TaskPaneList::RemoveWindow));
        ::std::auto_ptr<Window> aTemp2(m_pReportExplorer);
        m_pReportExplorer = NULL;
    }
    {
        ::std::auto_ptr<Window> aT3(m_pScrollWindow);
        m_pScrollWindow = NULL;
    }
    {
        ::std::auto_ptr<Window> aTemp2(m_pTaskPane);
        m_pTaskPane = NULL;
    }
    {
        ::std::auto_ptr<Window> aTemp2(m_pSplitWin);
        m_pSplitWin = NULL;
    }

}
// -----------------------------------------------------------------------------
void ODesignView::initialize()
{
    SetMapMode( MapMode( MAP_100TH_MM ) );
    m_pScrollWindow->initialize();
    m_pScrollWindow->Show();
}
//-----------------------------------------------------------------------------
void ODesignView::DataChanged( const DataChangedEvent& rDCEvt )
{
    ODataView::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
}
//------------------------------------------------------------------------------
long ODesignView::PreNotify( NotifyEvent& rNEvt )
{
    long nRet = ODataView::PreNotify(rNEvt); // 1 := has to be handled here
    switch(rNEvt.GetType())
    {
        case EVENT_KEYINPUT:
            //if ( nRet != 1L )
            {
                const KeyEvent* pKeyEvent = rNEvt.GetKeyEvent();
                if ( handleKeyEvent(*pKeyEvent) )
                    nRet = 1L;
                else if ( nRet == 1L && m_pAccel.get() && m_pController )
                {
                    const KeyCode& rCode = pKeyEvent->GetKeyCode();
                    util::URL aUrl;
                    aUrl.Complete = m_pAccel->findCommand(svt::AcceleratorExecute::st_VCLKey2AWTKey(rCode));
                    if ( !aUrl.Complete.getLength() || !m_pController->isCommandEnabled( aUrl.Complete ) )
                        nRet = 0L;
                }
            }
            break;
        default:
            break;
    }

    return nRet;
}
//------------------------------------------------------------------------------
void ODesignView::resizeDocumentView(Rectangle& _rPlayground)
{
    if ( !_rPlayground.IsEmpty() )
    {
        const Size aPlaygroundSize( _rPlayground.GetSize() );

        // calc the split pos, and forward it to the controller
        sal_Int32 nSplitPos = getController()->getSplitPos();
        if ( 0 != aPlaygroundSize.Width() )
        {
            if  (   ( -1 == nSplitPos )
                ||  ( nSplitPos >= aPlaygroundSize.Width() )
                )
            {
                long nMinWidth = static_cast<long>(0.1*aPlaygroundSize.Width());
                if ( m_pPropWin && m_pPropWin->IsVisible() )
                    nMinWidth = m_pPropWin->GetMinOutputSizePixel().Width();
                nSplitPos = static_cast<sal_Int32>(_rPlayground.Right() - nMinWidth);
                getController()->setSplitPos(nSplitPos);
            }
        } // if ( 0 != _rPlaygroundSize.Width() )

        Size aReportWindowSize(aPlaygroundSize);
        if ( m_pSplitWin->IsItemValid(TASKPANE_ID) )
        {
            // normalize the split pos
            const long nSplitterWidth = GetSettings().GetStyleSettings().GetSplitSize();
            Point aTaskPanePos(nSplitPos + nSplitterWidth, _rPlayground.Top());
            if ( m_pTaskPane && m_pTaskPane->IsVisible() )
            {
                aTaskPanePos.X() = aPlaygroundSize.Width() - m_pTaskPane->GetSizePixel().Width();
                sal_Int32 nMinWidth = m_pPropWin->getMinimumSize().Width();
                if ( nMinWidth > (aPlaygroundSize.Width() - aTaskPanePos.X()) )
                {
                    aTaskPanePos.X() = aPlaygroundSize.Width() - nMinWidth;
                }
                nSplitPos = aTaskPanePos.X() - nSplitterWidth;
                getController()->setSplitPos(nSplitPos);

                const long nTaskPaneSize = static_cast<long>((aPlaygroundSize.Width() - aTaskPanePos.X())*100/aPlaygroundSize.Width());
                if ( m_pSplitWin->GetItemSize( TASKPANE_ID ) != nTaskPaneSize )
                {
                    m_pSplitWin->SetItemSize( REPORT_ID, 99 - nTaskPaneSize );
                    m_pSplitWin->SetItemSize( TASKPANE_ID, nTaskPaneSize );
                }
            }
        }
        // set the size of the report window
        m_pSplitWin->SetPosSizePixel( _rPlayground.TopLeft(),aPlaygroundSize );
    }
        // just for completeness: there is no space left, we occupied it all ...
    _rPlayground.SetPos( _rPlayground.BottomRight() );
    _rPlayground.SetSize( Size( 0, 0 ) );

}
// -----------------------------------------------------------------------------
// set the view readonly or not
void ODesignView::setReadOnly(sal_Bool /*_bReadOnly*/)
{
}
//----------------------------------------------------------------------------
IMPL_LINK( ODesignView, MarkTimeout, Timer *, EMPTYARG )
{
    if ( m_pPropWin && m_pPropWin->IsVisible() )
    {
        m_pPropWin->Update(m_pCurrentView);
        uno::Reference<beans::XPropertySet> xProp(m_xReportComponent,uno::UNO_QUERY);
        if ( xProp.is() )
        {
            m_pPropWin->Update(xProp);
            static_cast<OTaskWindow*>(m_pTaskPane)->Resize();
        }
        Resize();
    }

    return 0;
}

//----------------------------------------------------------------------------
void ODesignView::SetMode( DlgEdMode _eNewMode )
{
    m_eMode = _eNewMode;
    if ( m_eMode == RPTUI_SELECT )
        m_eActObj = OBJ_NONE;
    if ( m_pScrollWindow )
        m_pScrollWindow->SetMode(_eNewMode);
}
//----------------------------------------------------------------------------
void ODesignView::SetInsertObj( USHORT eObj,const ::rtl::OUString& _sShapeType )
{
    m_eActObj = eObj;
    if ( m_pScrollWindow )
        m_pScrollWindow->SetInsertObj( eObj,_sShapeType );
}
//----------------------------------------------------------------------------
rtl::OUString ODesignView::GetInsertObjString() const
{
    if ( m_pScrollWindow )
        return m_pScrollWindow->GetInsertObjString();
    return rtl::OUString();
}
//----------------------------------------------------------------------------

USHORT ODesignView::GetInsertObj() const
{
    return m_eActObj;
}

//----------------------------------------------------------------------------
void ODesignView::Cut()
{
    Copy();
    Delete();
}

//----------------------------------------------------------------------------

void ODesignView::Copy()
{
    if ( m_pScrollWindow )
        m_pScrollWindow->Copy();
}

//----------------------------------------------------------------------------

void ODesignView::Paste()
{
    if ( m_pScrollWindow )
        m_pScrollWindow->Paste();
}
//----------------------------------------------------------------------------
void ODesignView::Delete()
{
    if ( m_pScrollWindow )
        m_pScrollWindow->Delete();
}
//----------------------------------------------------------------------------
BOOL ODesignView::HasSelection()
{
    return m_pScrollWindow && m_pScrollWindow->HasSelection();
}
//----------------------------------------------------------------------------

BOOL ODesignView::IsPasteAllowed()
{
    return m_pScrollWindow && m_pScrollWindow->IsPasteAllowed();
}

//----------------------------------------------------------------------------
void ODesignView::UpdatePropertyBrowserDelayed(OSectionView* _pView)
{
    if ( m_pCurrentView != _pView )
    {
        if ( m_pCurrentView && m_pScrollWindow )
            m_pScrollWindow->setMarked(m_pCurrentView,sal_False);
        m_pCurrentView = _pView;
        if ( m_pCurrentView && m_pScrollWindow )
            m_pScrollWindow->setMarked(m_pCurrentView,sal_True);
        m_xReportComponent.clear();
        DlgEdHint aHint( RPTUI_HINT_SELECTIONCHANGED );
        Broadcast( aHint );
    }
    m_aMarkTimer.Start();
}

//----------------------------------------------------------------------------
void ODesignView::toggleGrid(sal_Bool _bGridVisible)
{
    if ( m_pScrollWindow )
        m_pScrollWindow->toggleGrid(_bGridVisible);
}
//----------------------------------------------------------------------------
USHORT ODesignView::getSectionCount() const
{
    USHORT nRet = 0;
    if ( m_pScrollWindow )
        nRet = m_pScrollWindow->getSectionCount();
    return nRet;
}
//----------------------------------------------------------------------------
void ODesignView::showRuler(sal_Bool _bShow)
{
    if ( m_pScrollWindow )
        m_pScrollWindow->showRuler(_bShow);
}
//----------------------------------------------------------------------------
void ODesignView::removeSection(USHORT _nPosition)
{
    if ( m_pScrollWindow )
        m_pScrollWindow->removeSection(_nPosition);
}
//----------------------------------------------------------------------------
void ODesignView::addSection(const uno::Reference< report::XSection >& _xSection,const ::rtl::OUString& _sColorEntry,USHORT _nPosition)
{
    if ( m_pScrollWindow )
        m_pScrollWindow->addSection(_xSection,_sColorEntry,_nPosition);
}
// -----------------------------------------------------------------------------
void ODesignView::GetFocus()
{
    Window::GetFocus();
    if ( m_pScrollWindow )
    {
        ::boost::shared_ptr<OReportSection> pSection = m_pScrollWindow->getMarkedSection();
        if ( pSection )
            pSection->GrabFocus();
    }
}
// -----------------------------------------------------------------------------
void ODesignView::ImplInitSettings()
{
//#if OSL_DEBUG_LEVEL > 0
//    SetBackground( Wallpaper( COL_RED ));
//#else
    SetBackground( Wallpaper( Application::GetSettings().GetStyleSettings().GetFaceColor() ));
//#endif
    SetFillColor( Application::GetSettings().GetStyleSettings().GetFaceColor() );
    SetTextFillColor( Application::GetSettings().GetStyleSettings().GetFaceColor() );
}
//-----------------------------------------------------------------------------
IMPL_LINK( ODesignView, SplitHdl, void*,  )
{
    m_bInSplitHandler = sal_True;
    //const long nTest = m_aSplitter.GetSplitPosPixel();

    const Size aOutputSize = GetOutputSizePixel();
    const long nTest = aOutputSize.Width() * m_pSplitWin->GetItemSize(TASKPANE_ID) / 100;
    long nMinWidth = static_cast<long>(0.1*aOutputSize.Width());
    if ( m_pPropWin && m_pPropWin->IsVisible() )
        nMinWidth = m_pPropWin->GetMinOutputSizePixel().Width();

    if ( (aOutputSize.Width() - nTest) >= nMinWidth && nTest > m_pScrollWindow->getMaxMarkerWidth(sal_False) )
    {
        long nOldSplitPos = getController()->getSplitPos();
        getController()->setSplitPos(nTest);
        if ( nOldSplitPos != -1 && nOldSplitPos <= nTest )
        {
            Invalidate(INVALIDATE_NOCHILDREN);
        }
    }

    m_bInSplitHandler = sal_False;

    return 0L;
}
//-----------------------------------------------------------------------------
void ODesignView::SelectAll()
{
    if ( m_pScrollWindow )
        m_pScrollWindow->SelectAll();
}
//-----------------------------------------------------------------------------
void ODesignView::unmarkAllObjects(OSectionView* _pSectionView)
{
    if ( m_pScrollWindow )
        m_pScrollWindow->unmarkAllObjects(_pSectionView);
}
//-----------------------------------------------------------------------------
void ODesignView::togglePropertyBrowser(sal_Bool _bToogleOn)
{
    if ( !m_pPropWin && _bToogleOn )
    {
        m_pPropWin = new PropBrw(getController()->getORB(),m_pTaskPane,this);
        m_pPropWin->Invalidate();
        static_cast<OTaskWindow*>(m_pTaskPane)->setPropertyBrowser(m_pPropWin);
        notifySystemWindow(this,m_pPropWin,::comphelper::mem_fun(&TaskPaneList::AddWindow));
    }
    if ( m_pPropWin && _bToogleOn != m_pPropWin->IsVisible() )
    {
        if ( !m_pCurrentView && !m_xReportComponent.is() )
            m_xReportComponent = getController()->getReportDefinition();

        const sal_Bool bWillBeVisible = _bToogleOn;
        m_pPropWin->Show(bWillBeVisible);
        m_pTaskPane->Show(bWillBeVisible);
        m_pTaskPane->Invalidate();

        if ( bWillBeVisible )
            m_pSplitWin->InsertItem( TASKPANE_ID, m_pTaskPane,START_SIZE_TASKPANE, SPLITWINDOW_APPEND, COLSET_ID, SWIB_PERCENTSIZE/*|SWIB_COLSET */);
        else
            m_pSplitWin->RemoveItem(TASKPANE_ID);

        Invalidate(/*INVALIDATE_NOCHILDREN|INVALIDATE_NOERASE*/);
        if ( bWillBeVisible )
            m_aMarkTimer.Start();
    }
}
//-----------------------------------------------------------------------------
void ODesignView::showProperties(const uno::Reference< uno::XInterface>& _xReportComponent)
{
    if ( m_xReportComponent != _xReportComponent )
    {
        m_xReportComponent = _xReportComponent;
        if ( m_pCurrentView )
            m_pScrollWindow->setMarked(m_pCurrentView,sal_False);
        m_pCurrentView = NULL;
        m_aMarkTimer.Start();
    }
}
//-----------------------------------------------------------------------------
BOOL ODesignView::isReportExplorerVisible() const
{
    return m_pReportExplorer && m_pReportExplorer->IsVisible();
}
//-----------------------------------------------------------------------------
void ODesignView::toggleReportExplorer()
{
    if ( !m_pReportExplorer )
    {
        OReportController* pReportController = getController();
        m_pReportExplorer = new ONavigator(this,pReportController);
        m_pReportExplorer->AddEventListener(LINK(pReportController,OReportController,EventLstHdl));
        notifySystemWindow(this,m_pReportExplorer,::comphelper::mem_fun(&TaskPaneList::AddWindow));
    }
    else
        m_pReportExplorer->Show(!m_pReportExplorer->IsVisible());
}
//-----------------------------------------------------------------------------
BOOL ODesignView::isAddFieldVisible() const
{
    return m_pAddField && m_pAddField->IsVisible();
}
//-----------------------------------------------------------------------------
void ODesignView::toggleAddField()
{
    if ( !m_pAddField )
    {
        uno::Reference< report::XReportDefinition > xReport(m_xReportComponent,uno::UNO_QUERY);
        uno::Reference< report::XReportComponent > xReportComponent(m_xReportComponent,uno::UNO_QUERY);
        OReportController* pReportController = getController();
        if ( !m_pCurrentView && !xReport.is() )
        {
            if ( xReportComponent.is() )
                xReport = xReportComponent->getSection()->getReportDefinition();
            else
                xReport = pReportController->getReportDefinition().get();
        }
        else if ( m_pCurrentView )
        {
            uno::Reference< report::XSection > xSection = m_pCurrentView->getSectionWindow()->getSection();
            xReport = xSection->getReportDefinition();
        }
        m_pAddField = new OAddFieldWindow(*pReportController,this);
        m_pAddField->Update();
        m_pAddField->AddEventListener(LINK(pReportController,OReportController,EventLstHdl));
        notifySystemWindow(this,m_pAddField,::comphelper::mem_fun(&TaskPaneList::AddWindow));
    }
    else
        m_pAddField->Show(!m_pAddField->IsVisible());
}
// -------------------------------------------------------------------------
uno::Reference< report::XSection > ODesignView::getCurrentSection() const
{
    uno::Reference< report::XSection > xSection;
    if ( m_pCurrentView )
        xSection = m_pCurrentView->getSectionWindow()->getSection();

    // why do we need the code below?
    //else
 //   {
 //       OReportController* pReportController = getController();
 //       if ( pReportController )
    //      xSection = pReportController->getReportDefinition()->getDetail();
 //   }
    return xSection;
}
// -----------------------------------------------------------------------------
uno::Reference< report::XReportComponent > ODesignView::getCurrentControlModel() const
{
    uno::Reference< report::XReportComponent > xModel;
    if ( m_pCurrentView )
    {
        xModel = m_pCurrentView->getSectionWindow()->getCurrentControlModel();
    }
    return xModel;
}
// -------------------------------------------------------------------------
::boost::shared_ptr<OReportSection> ODesignView::getMarkedSection(NearSectionAccess nsa) const
{
    return  m_pScrollWindow ? m_pScrollWindow->getMarkedSection(nsa) : ::boost::shared_ptr<OReportSection>();
}
// -------------------------------------------------------------------------
void ODesignView::markSection(const sal_uInt16 _nPos)
{
    if ( m_pScrollWindow )
        m_pScrollWindow->markSection(_nPos);
}
// -----------------------------------------------------------------------------
void ODesignView::fillCollapsedSections(::std::vector<sal_uInt16>& _rCollapsedPositions) const
{
    if ( m_pScrollWindow )
        m_pScrollWindow->fillCollapsedSections(_rCollapsedPositions);
}
// -----------------------------------------------------------------------------
void ODesignView::collapseSections(const uno::Sequence< beans::PropertyValue>& _aCollpasedSections)
{
    if ( m_pScrollWindow )
        m_pScrollWindow->collapseSections(_aCollpasedSections);
}
// -----------------------------------------------------------------------------
::rtl::OUString ODesignView::getCurrentPage() const
{
    return m_pPropWin ? m_pPropWin->getCurrentPage() : ::rtl::OUString();
}
// -----------------------------------------------------------------------------
void ODesignView::setCurrentPage(const ::rtl::OUString& _sLastActivePage)
{
    if ( m_pPropWin )
        m_pPropWin->setCurrentPage(_sLastActivePage);
}
// -----------------------------------------------------------------------------
void ODesignView::alignMarkedObjects(sal_Int32 _nControlModification,bool _bAlignAtSection, bool bBoundRects)
{
    if ( m_pScrollWindow )
        m_pScrollWindow->alignMarkedObjects(_nControlModification, _bAlignAtSection,bBoundRects);
}
// -----------------------------------------------------------------------------
sal_Bool ODesignView::isAlignPossible() const
{
    ::boost::shared_ptr<OReportSection> pMarkedSection = getMarkedSection();
    return pMarkedSection.get() && pMarkedSection->getView()->IsAlignPossible();
}
// -------------------------------------------------------------------------
sal_Int32 ODesignView::getMaxMarkerWidth(sal_Bool _bWithEnd) const
{
    return m_pScrollWindow->getMaxMarkerWidth(_bWithEnd);
}
//------------------------------------------------------------------------------
sal_Bool ODesignView::handleKeyEvent(const KeyEvent& _rEvent)
{
    if ( (m_pPropWin && m_pPropWin->HasChildPathFocus()) )
        return sal_False;
    return m_pScrollWindow && m_pScrollWindow->handleKeyEvent(_rEvent);
}
//------------------------------------------------------------------------
void ODesignView::setMarked(OSectionView* _pSectionView,sal_Bool _bMark)
{
    if ( m_pScrollWindow )
        m_pScrollWindow->setMarked(_pSectionView,_bMark);
}
//------------------------------------------------------------------------
void ODesignView::setMarked(const uno::Reference< report::XSection>& _xSection,sal_Bool _bMark)
{
    if ( m_pScrollWindow )
    {
        m_pScrollWindow->setMarked(_xSection,_bMark);
        if ( _bMark )
            UpdatePropertyBrowserDelayed(getMarkedSection()->getView());
        else
            m_pCurrentView = NULL;
    }
}
//------------------------------------------------------------------------
void ODesignView::setMarked(const uno::Sequence< uno::Reference< report::XReportComponent> >& _aShapes,sal_Bool _bMark)
{
    if ( m_pScrollWindow )
    {
        m_pScrollWindow->setMarked(_aShapes,_bMark);
        if ( _aShapes.hasElements() && _bMark )
            showProperties(_aShapes[0]);
        else
            m_xReportComponent.clear();
    }
}
//------------------------------------------------------------------------------
void ODesignView::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() )
    {
        const uno::Sequence< beans::PropertyValue> aArgs;
        getController()->executeChecked(SID_SELECT_REPORT,aArgs);
    }
    ODataView::MouseButtonDown(rMEvt);
}
// -----------------------------------------------------------------------------
uno::Any ODesignView::getCurrentlyShownProperty() const
{
    uno::Any aRet;
    ::boost::shared_ptr<OReportSection> pSection = getMarkedSection();
    if ( pSection )
    {
        ::std::vector< uno::Reference< report::XReportComponent > > aSelection;
        pSection->fillControlModelSelection(aSelection);
        if ( !aSelection.empty() )
            aRet <<= uno::Sequence< uno::Reference< report::XReportComponent > >(&(*aSelection.begin()),aSelection.size());

    }
    return aRet;
}
// -----------------------------------------------------------------------------
void ODesignView::setGridSnap(BOOL bOn)
{
    if ( m_pScrollWindow )
        m_pScrollWindow->setGridSnap(bOn);

}
// -----------------------------------------------------------------------------
void ODesignView::setDragStripes(BOOL bOn)
{
    if ( m_pScrollWindow )
        m_pScrollWindow->setDragStripes(bOn);
}
// -----------------------------------------------------------------------------
BOOL ODesignView::isDragStripes() const
{
    return m_pScrollWindow ? m_pScrollWindow->isDragStripes() : FALSE;
}
// -----------------------------------------------------------------------------
sal_Bool ODesignView::isHandleEvent(sal_uInt16 /*_nId*/) const
{
    return m_pPropWin && m_pPropWin->HasChildPathFocus();
}
// -----------------------------------------------------------------------------
sal_uInt32 ODesignView::getMarkedObjectCount() const
{
    return m_pScrollWindow ? m_pScrollWindow->getMarkedObjectCount() : 0;
}
//============================================================================
} // rptui
//============================================================================
