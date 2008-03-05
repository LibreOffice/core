/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ReportSection.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:13:26 $
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

#ifndef REPORT_REPORTSECTION_HXX
#include "ReportSection.hxx"
#endif
#ifndef RPTUI_REPORT_WINDOW_HXX
#include "ReportWindow.hxx"
#endif
#ifndef RPTUI_DESIGNVIEW_HXX
#include "DesignView.hxx"
#endif
#ifndef REPORTDESIGN_SHARED_UISTRINGS_HRC
#include "uistrings.hrc"
#endif
#ifndef _REPORT_RPTUIOBJ_HXX
#include "RptObject.hxx"
#endif
#ifndef REPORT_RPTMODEL_HXX
#include "RptModel.hxx"
#endif
#ifndef _REPORT_SECTIONVIEW_HXX
#include "SectionView.hxx"
#endif
#ifndef _REPORT_RPTUIPAGE_HXX
#include "RptPage.hxx"
#endif
#ifndef RPTUI_REPORTCONTROLLER_HXX
#include "ReportController.hxx"
#endif
#include "UITools.hxx"
#ifndef _SVDPAGV_HXX
#include <svx/svdpagv.hxx>
#endif
#ifndef _EEITEMID_HXX
#include <svx/eeitemid.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX
#include <svx/adjitem.hxx>
#endif
#include <svx/sdrpaintwindow.hxx>
#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARD_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#endif
#include <toolkit/helper/convert.hxx>
#ifndef _REPORT_RPTUIDEF_HXX
#include "RptDef.hxx"
#endif
#ifndef RPTUI_VIEWSWINDOW_HXX
#include "ViewsWindow.hxx"
#endif
#ifndef RTPUI_REPORTDESIGN_HELPID_HRC
#include "helpids.hrc"
#endif
#ifndef _RPTUI_DLGRESID_HRC
#include "RptResId.hrc"
#endif
#ifndef _REPORT_RPTUICLIP_HXX
#include "dlgedclip.hxx"
#endif
#ifndef RPTUI_UNDOACTIONS_HXX
#include "UndoActions.hxx"
#endif
#ifndef _RPTUI_SLOTID_HRC_
#include "rptui_slotid.hrc"
#endif
#ifndef _SBASLTID_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _SVX_DBAEXCHANGE_HXX_
#include <svx/dbaexchange.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _GALLERY_HXX_
#include <svx/gallery.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif
#include <svx/unoshape.hxx>
#include <unotools/confignode.hxx>
#include <svtools/extcolorcfg.hxx>

// =============================================================================
namespace rptui
{
// =============================================================================
using namespace ::com::sun::star;
// -----------------------------------------------------------------------------

sal_Int32 lcl_getOverlappedControlColor(/*const uno::Reference <lang::XMultiServiceFactory> _rxFactory*/)
{
    svtools::ExtendedColorConfig aConfig;
    sal_Int32 nColor = aConfig.GetColorValue(CFG_REPORTDESIGNER, DBOVERLAPPEDCONTROL).getColor();
    return nColor;
}
//------------------------------------------------------------------------------
DBG_NAME( rpt_OReportSection )
OReportSection::OReportSection(OViewsWindow* _pParent,const uno::Reference< report::XSection >& _xSection)
: Window(_pParent,WB_DIALOGCONTROL)
, ::comphelper::OPropertyChangeListener(m_aMutex)
, DropTargetHelper(this)
,m_pPage(NULL)
,m_pView(NULL)
,m_pParent(_pParent)
,m_pFunc(NULL)
,m_pMulti(NULL)
,m_pReportListener(NULL)
,m_xSection(_xSection)
,m_eMode(RPTUI_SELECT)
,m_bDialogModelChanged(FALSE)
,m_bInDrag(sal_False)
{
    DBG_CTOR( rpt_OReportSection,NULL);
    EnableChildTransparentMode();
    SetHelpId(HID_REPORTSECTION);
    SetMapMode( MapMode( MAP_100TH_MM ) );
    try
    {
        fill();
    }
    catch(uno::Exception&)
    {
        OSL_ENSURE(0,"Exception catched!");
    }

    m_pFunc.reset(new DlgEdFuncSelect( this ));
    m_pFunc->setOverlappedControlColor(lcl_getOverlappedControlColor( /* m_pParent->getView()->getReportView()->getController()->getORB() */ ) );

    Show();
}
//------------------------------------------------------------------------------
OReportSection::~OReportSection()
{
    DBG_DTOR( rpt_OReportSection,NULL);
    m_pPage = NULL;
    //m_pModel->GetUndoEnv().RemoveSection(m_xSection.get());
    if ( m_pMulti.is() )
        m_pMulti->dispose();

    if ( m_pReportListener.is() )
        m_pReportListener->dispose();
    m_pFunc = ::std::auto_ptr<DlgEdFunc>();

    {
        ::std::auto_ptr<OSectionView> aTemp( m_pView);
        if ( m_pView )
            m_pView->EndListening( *m_pModel );
        m_pView = NULL;
    }
    /*m_pModel->DeletePage(m_pPage->GetPageNum());*/
}
//------------------------------------------------------------------------------
void OReportSection::Paint( const Rectangle& rRect )
{
    if ( m_pView )
    {
         // repaint, get PageView and prepare Region
        SdrPageView* pPgView = m_pView->GetSdrPageView();
        const Region aPaintRectRegion(rRect);

        // #i74769#
        SdrPaintWindow* pTargetPaintWindow = 0;

        // mark repaint start
        if(pPgView)
        {
            pTargetPaintWindow = pPgView->GetView().BeginDrawLayers(this, aPaintRectRegion);
            OSL_ENSURE(pTargetPaintWindow, "BeginDrawLayers: Got no SdrPaintWindow (!)");
            // draw background self using wallpaper
            OutputDevice& rTargetOutDev = pTargetPaintWindow->GetTargetOutputDevice();
            rTargetOutDev.DrawWallpaper(rRect, Wallpaper(Color(m_xSection->getBackColor())));
        }

        // do paint (unbuffered) and mark repaint end
        if(pPgView)
        {
            pPgView->DrawLayer(0, this);
            pPgView->GetView().EndDrawLayers(*pTargetPaintWindow);
        }


        const Region aReg(rRect);
        m_pView->CompleteRedraw(this,aReg);
    }
}
//------------------------------------------------------------------------------
void OReportSection::Resize()
{
    Window::Resize();
    if ( m_xSection.is() && m_pPage && m_pView )
    {
        uno::Reference<report::XReportDefinition> xReportDefinition = m_xSection->getReportDefinition();
        m_pPage->SetSize( Size( getStyleProperty<awt::Size>(xReportDefinition,PROPERTY_PAPERSIZE).Width,5*m_xSection->getHeight()) );
        const Size aPageSize = m_pPage->GetSize();
        const sal_Int32 nLeftMargin = getStyleProperty<sal_Int32>(xReportDefinition,PROPERTY_LEFTMARGIN);
        const sal_Int32 nRightMargin = getStyleProperty<sal_Int32>(xReportDefinition,PROPERTY_RIGHTMARGIN);
        m_pView->SetWorkArea( Rectangle( Point( nLeftMargin, 0), Size(aPageSize.Width() - nLeftMargin - nRightMargin,aPageSize.Height()) ) );
        // PixelToLogic(Size(0,m_pParent->getTotalHeight(this)) ).Height()
    }
}
//------------------------------------------------------------------------------
void OReportSection::fill()
{
    if ( !m_xSection.is() )
        return;

    m_pMulti = new comphelper::OPropertyChangeMultiplexer(this,m_xSection.get());
    m_pMulti->addProperty(PROPERTY_BACKCOLOR);

    m_pReportListener = addStyleListener(m_xSection->getReportDefinition(),this);

    m_pModel = m_pParent->getView()->getReportView()->getController()->getSdrModel();
    m_pPage = m_pModel->getPage(m_xSection);

    m_pView = new OSectionView( m_pModel.get(), this, m_pParent->getView() );

    // without the following call, no grid is painted
    m_pView->ShowSdrPage( m_pPage );

    m_pView->SetMoveSnapOnlyTopLeft( TRUE );
    ODesignView* pDesignView = m_pParent->getView()->getReportView();

    Size aGridSize = pDesignView->getGridSize();
    m_pView->SetGridCoarse( aGridSize );
    Fraction aX(aGridSize.A());
    Fraction aY(aGridSize.B());
    m_pView->SetSnapGridWidth(aX,aY);
    //m_pView->SetSnapGrid( pDesignView->getGridSize() );
    m_pView->SetGridSnap( pDesignView->isGridSnap() );
    m_pView->SetGridFront( FALSE );
    m_pView->SetDragStripes( TRUE );
    m_pView->SetPageVisible();
    m_pView->SetApplicationDocumentColor(m_xSection->getBackColor());

    const sal_Int32 nLeftMargin = getStyleProperty<sal_Int32>(m_xSection->getReportDefinition(),PROPERTY_LEFTMARGIN);
    const sal_Int32 nRightMargin = getStyleProperty<sal_Int32>(m_xSection->getReportDefinition(),PROPERTY_RIGHTMARGIN);
    m_pPage->SetLftBorder(nLeftMargin);
    m_pPage->SetRgtBorder(nRightMargin);

// LLA: TODO
//  m_pPage->SetUppBorder(-10000);

    m_pView->SetDesignMode( TRUE );

    m_pView->StartListening( *m_pModel  );
    Resize();
}
// -----------------------------------------------------------------------------
void OReportSection::Paste(const uno::Sequence< beans::NamedValue >& _aAllreadyCopiedObjects,bool _bForce)
{
    OSL_ENSURE(m_xSection.is(),"Why is the section here NULL!");
    if ( m_xSection.is() && _aAllreadyCopiedObjects.getLength() )
    {
        // stop all drawing actions
        m_pView->BrkAction();

        // unmark all objects
        m_pView->UnmarkAll();
        const ::rtl::OUString sSectionName = m_xSection->getName();
        const sal_Int32 nLength = _aAllreadyCopiedObjects.getLength();
        const beans::NamedValue* pIter = _aAllreadyCopiedObjects.getConstArray();
        const beans::NamedValue* pEnd  = pIter + nLength;
        for(;pIter != pEnd;++pIter)
        {
            if ( _bForce || pIter->Name == sSectionName)
            {
                try
                {
                    uno::Sequence< uno::Reference<util::XCloneable> > aCopies;
                    pIter->Value >>= aCopies;
                    const uno::Reference<util::XCloneable>* pCopiesIter = aCopies.getConstArray();
                    const uno::Reference<util::XCloneable>* pCopiesEnd = pCopiesIter + aCopies.getLength();
                    for (;pCopiesIter != pCopiesEnd ; ++pCopiesIter)
                    {
                        uno::Reference< report::XReportComponent> xComponent(*pCopiesIter,uno::UNO_QUERY_THROW);
                        uno::Reference< report::XReportComponent> xClone(xComponent->createClone(),uno::UNO_QUERY_THROW);
                        Rectangle aRet(VCLPoint(xClone->getPosition()),VCLSize(xClone->getSize()));
                        aRet.setHeight(aRet.getHeight() + 1);
                        aRet.setWidth(aRet.getWidth() + 1);
                        bool bOverlapping = true;
                        while ( bOverlapping )
                        {
                            bOverlapping = isOver(aRet,*m_pPage,*m_pView,true);
                            if ( bOverlapping )
                            {
                                aRet.Move(0,aRet.getHeight()+1);
                                xClone->setPositionY(aRet.Top());
                            }
                        }
                        if ( !bOverlapping )
                        {
                            m_xSection->add(xClone.get());
                            SvxShape* pShape = SvxShape::getImplementation( xClone );
                            SdrObject* pObject = pShape ? pShape->GetSdrObject() : NULL;
                            OSL_ENSURE( pObject, "OReportSection::Paste: no SdrObject for the shape!" );
                            if ( pObject )
                            {
                                m_pView->AddUndo( m_pView->GetModel()->GetSdrUndoFactory().CreateUndoNewObject( *pObject ) );
                                m_pView->MarkObj( pObject, m_pView->GetSdrPageView() );
                            }
                        }
                    }
                }
                catch(uno::Exception&)
                {
                    OSL_ENSURE(0,"Exception caught while pasting a new object!");
                }
                if ( !_bForce )
                    break;
            }
        }
    }
}
//----------------------------------------------------------------------------
void OReportSection::Delete()
{
    if( !m_pView->AreObjectsMarked() )
        return;

    m_pView->BrkAction();
    m_pView->DeleteMarked();
}
//----------------------------------------------------------------------------
BOOL OReportSection::UnmarkDialog()
{
    SdrObject*      pDlgObj = m_pPage->GetObj(0);
    SdrPageView*    pPgView = m_pView->GetSdrPageView();

    BOOL bWasMarked = m_pView->IsObjMarked( pDlgObj );

    if( bWasMarked )
        m_pView->MarkObj( pDlgObj, pPgView, TRUE );

    return bWasMarked;
}

//----------------------------------------------------------------------------
BOOL OReportSection::RemarkDialog()
{
    SdrObject*      pDlgObj = m_pPage->GetObj(0);
    SdrPageView*    pPgView = m_pView->GetSdrPageView();

    BOOL bWasMarked = m_pView->IsObjMarked( pDlgObj );

    if( !bWasMarked )
        m_pView->MarkObj( pDlgObj, pPgView, FALSE );

    return bWasMarked;
}
//----------------------------------------------------------------------------
void OReportSection::SetMode( DlgEdMode eNewMode )
{
    if ( eNewMode != m_eMode )
    {
        if ( eNewMode == RPTUI_INSERT )
        {
            m_pFunc.reset(new DlgEdFuncInsert( this ));
        }
        else
        {
            m_pFunc.reset(new DlgEdFuncSelect( this ));
        }
        m_pFunc->setOverlappedControlColor(lcl_getOverlappedControlColor( /* m_pParent->getView()->getReportView()->getController()->getORB()  */ ) );
        m_pModel->SetReadOnly(eNewMode == RPTUI_READONLY);
        m_eMode = eNewMode;
    }
}
// -----------------------------------------------------------------------------
void OReportSection::Copy(uno::Sequence< beans::NamedValue >& _rAllreadyCopiedObjects)
{
    Copy(_rAllreadyCopiedObjects,false);
}
//----------------------------------------------------------------------------
void OReportSection::Copy(uno::Sequence< beans::NamedValue >& _rAllreadyCopiedObjects,bool _bEraseAnddNoClone)
{
    OSL_ENSURE(m_xSection.is(),"Why is the section here NULL!");
    if( !m_pView->AreObjectsMarked() || !m_xSection.is() )
        return;

    // stop all drawing actions
    //m_pView->BrkAction();

    // insert control models of marked objects into clipboard dialog model
    const SdrMarkList& rMarkedList = m_pView->GetMarkedObjectList();
    const ULONG nMark = rMarkedList.GetMarkCount();

    ::std::vector< uno::Reference<util::XCloneable> > aCopies;
    aCopies.reserve(nMark);

    SdrUndoFactory& rUndo = m_pView->GetModel()->GetSdrUndoFactory();

    for( ULONG i = nMark; i > 0; )
    {
        --i;
        SdrObject* pSdrObject = rMarkedList.GetMark(i)->GetMarkedSdrObj();
        OObjectBase* pObj = dynamic_cast<OObjectBase*>(pSdrObject);
        if ( pObj  )
        {
            try
            {
                uno::Reference<report::XReportComponent> xComponent = pObj->getReportComponent();
                aCopies.push_back(xComponent->createClone());
                if ( _bEraseAnddNoClone )
                {
                    m_pView->AddUndo( rUndo.CreateUndoDeleteObject( *pSdrObject ) );
                    m_pPage->RemoveObject(pSdrObject->GetOrdNum());
                }

            }
            catch(uno::Exception&)
            {
                OSL_ENSURE(0,"Can't copy report elements!");
            }
        }
    } // for( ULONG i = 0; i < nMark; i++ )

    if ( !aCopies.empty() )
    {
        ::std::reverse(aCopies.begin(),aCopies.end());
        const sal_Int32 nLength = _rAllreadyCopiedObjects.getLength();
        _rAllreadyCopiedObjects.realloc( nLength + 1);
        beans::NamedValue* pNewValue = _rAllreadyCopiedObjects.getArray() + nLength;
        pNewValue->Name = m_xSection->getName();
        pNewValue->Value <<= uno::Sequence< uno::Reference<util::XCloneable> >(&(*aCopies.begin()),aCopies.size());
    }
}
//----------------------------------------------------------------------------
void OReportSection::MouseButtonDown( const MouseEvent& rMEvt )
{
    m_pParent->getView()->setMarked(m_pView,sal_True);
    m_pFunc->MouseButtonDown( rMEvt );
    Window::MouseButtonDown(rMEvt);
}
//----------------------------------------------------------------------------
void OReportSection::GetFocus()
{
    Window::GetFocus();
    m_pParent->SectionHasFocus(this,TRUE);
}
//----------------------------------------------------------------------------
void OReportSection::LoseFocus()
{
    Window::LoseFocus();
    m_pParent->SectionHasFocus(this,FALSE);
}
//----------------------------------------------------------------------------

void OReportSection::MouseButtonUp( const MouseEvent& rMEvt )
{
    if ( !m_pFunc->MouseButtonUp( rMEvt ) )
        getViewsWindow()->getView()->getReportView()->getController()->executeUnChecked(SID_OBJECT_SELECT,uno::Sequence< beans::PropertyValue>());
}

//----------------------------------------------------------------------------

void OReportSection::MouseMove( const MouseEvent& rMEvt )
{
    m_pFunc->MouseMove( rMEvt );
}
//----------------------------------------------------------------------------
bool OReportSection::adjustPageSize()
{
    bool bAdjustedPageSize = false;
    return bAdjustedPageSize;
}
//----------------------------------------------------------------------------
void OReportSection::SetGridVisible(BOOL _bVisible)
{
    m_pView->SetGridVisible( _bVisible );
}
//------------------------------------------------------------------------------
void OReportSection::SelectAll()
{
    if ( m_pView )
        m_pView->MarkAllObj();
}
// -----------------------------------------------------------------------------
void OReportSection::setSectionHeightPixel(sal_uInt32 _nHeight)
{
    OSL_ENSURE(m_xSection.is(),"Why is the UNO section NULL!");
    if ( m_xSection.is() )
        m_xSection->setHeight(PixelToLogic(Size(0,_nHeight),MAP_100TH_MM).Width());
}
//----------------------------------------------------------------------------
void OReportSection::insertObject(const uno::Reference< report::XReportComponent >& _xObject)
{
    OSL_ENSURE(_xObject.is(),"Object is not valid to create a SdrObject!");
    if ( !_xObject.is() || !m_pView || !m_pPage )
        return;
    m_pPage->insertObject(_xObject);
}
//----------------------------------------------------------------------------
void OReportSection::Command( const CommandEvent& _rCEvt )
{
    Window::Command(_rCEvt);
    switch (_rCEvt.GetCommand())
    {
        case COMMAND_CONTEXTMENU:
        {
            OReportController* pController = getViewsWindow()->getView()->getReportView()->getController();
            PopupMenu aContextMenu( ModuleRes( RID_MENU_REPORT ) );
            uno::Reference< report::XReportDefinition> xReportDefinition = getSection()->getReportDefinition();
            const USHORT nCount = aContextMenu.GetItemCount();
            for (USHORT i = 0; i < nCount; ++i)
            {
                if ( MENUITEM_SEPARATOR != aContextMenu.GetItemType(i))
                {
                    const USHORT nId = aContextMenu.GetItemId(i);
                    if ( nId == SID_PAGEHEADERFOOTER )
                    {
                        String sText = String(ModuleRes((xReportDefinition.is() && xReportDefinition->getPageHeaderOn()) ? RID_STR_PAGEHEADERFOOTER_DELETE : RID_STR_PAGEHEADERFOOTER_INSERT));
                        aContextMenu.SetItemText(nId,sText);
                    }
                    else if ( nId == SID_REPORTHEADERFOOTER )
                    {
                        String sText = String(ModuleRes((xReportDefinition.is() && xReportDefinition->getReportHeaderOn()) ? RID_STR_REPORTHEADERFOOTER_DELETE : RID_STR_REPORTHEADERFOOTER_INSERT));
                        aContextMenu.SetItemText(nId,sText);
                    }
                    aContextMenu.CheckItem(nId,pController->isCommandChecked(nId));
                    aContextMenu.EnableItem(nId,pController->isCommandEnabled(nId));
                }
            } // for (USHORT i = 0; i < nCount; ++i)
            Point aPos = _rCEvt.GetMousePosPixel();
            m_pView->EndAction();
            const USHORT nId = aContextMenu.Execute(this, aPos);
            if ( nId )
            {
                uno::Sequence< beans::PropertyValue> aArgs;
                if ( nId == SID_ATTR_CHAR_COLOR_BACKGROUND )
                {
                    aArgs.realloc(1);
                    aArgs[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Selection"));
                    aArgs[0].Value <<= m_xSection;
                }
                pController->executeChecked(nId,aArgs);
            }
        }
        break;
    }
}
// -----------------------------------------------------------------------------
void OReportSection::_propertyChanged(const beans::PropertyChangeEvent& _rEvent) throw( uno::RuntimeException)
{
    if ( m_xSection.is() )
    {
        if ( _rEvent.Source == m_xSection )
        {
            m_pView->SetApplicationDocumentColor(m_xSection->getBackColor());
            Invalidate(INVALIDATE_NOCHILDREN|INVALIDATE_NOERASE);
        }
        else
        {
            uno::Reference<report::XReportDefinition> xReportDefinition = m_xSection->getReportDefinition();
            if ( _rEvent.PropertyName == PROPERTY_LEFTMARGIN )
            {
                const sal_Int32 nLeftMargin = getStyleProperty<sal_Int32>(xReportDefinition,PROPERTY_LEFTMARGIN);
                m_pPage->SetLftBorder(nLeftMargin);
            }
            else if ( _rEvent.PropertyName == PROPERTY_RIGHTMARGIN )
            {
                const sal_Int32 nRightMargin = getStyleProperty<sal_Int32>(xReportDefinition,PROPERTY_RIGHTMARGIN);
                m_pPage->SetRgtBorder(nRightMargin);
            }

            try
            {
                const sal_Int32 nLeftMargin  = getStyleProperty<sal_Int32>(xReportDefinition,PROPERTY_LEFTMARGIN);
                const sal_Int32 nRightMargin = getStyleProperty<sal_Int32>(xReportDefinition,PROPERTY_RIGHTMARGIN);
                const sal_Int32 nPaperWidth  = getStyleProperty<awt::Size>(xReportDefinition,PROPERTY_PAPERSIZE).Width;
                ::boost::shared_ptr<OReportSection> aSection = m_pParent->getReportSection(m_xSection);
                const sal_Int32 nCount = m_xSection->getCount();
                for (sal_Int32 i = 0; i < nCount; ++i)
                {
                    bool bChanged = false;
                    uno::Reference< report::XReportComponent> xReportComponent(m_xSection->getByIndex(i),uno::UNO_QUERY_THROW);
                    awt::Point aPos = xReportComponent->getPosition();
                    awt::Size aSize = xReportComponent->getSize();
                    SvxShape* pShape = SvxShape::getImplementation( xReportComponent );
                    SdrObject* pObject = pShape ? pShape->GetSdrObject() : NULL;
                    if ( pObject )
                    {
                        OObjectBase* pBase = dynamic_cast<OObjectBase*>(pObject);
                        pBase->EndListening(sal_False);
                        if ( aPos.X < nLeftMargin )
                        {
                            aPos.X  = nLeftMargin;
                            bChanged = true;
                        }
                        if ( (aPos.X + aSize.Width) > (nPaperWidth - nRightMargin) )
                        {
                            aPos.X = nPaperWidth - nRightMargin - aSize.Width;
                            if ( aPos.X < nLeftMargin )
                            {
                                aSize.Width += aPos.X - nLeftMargin;
                                aPos.X = nLeftMargin;
                                // add listener around
                                pBase->StartListening();
                                xReportComponent->setSize(aSize);
                                pBase->EndListening(sal_False);
                            }
                            bChanged = true;
                        }
                        if ( aPos.Y < 0 )
                            aPos.Y = 0;
                        if ( bChanged )
                        {
                            xReportComponent->setPosition(aPos);
                            correctOverlapping(pObject,aSection,false);
                        }
                        pBase->StartListening();
                    }
                }
            }
            catch(uno::Exception)
            {
                OSL_ENSURE(0,"Exception caught: OReportSection::_propertyChanged(");
            }

            Resize();
        }
    }
}
//------------------------------------------------------------------------------
sal_Bool OReportSection::handleKeyEvent(const KeyEvent& _rEvent)
{
    return m_pFunc.get() ? m_pFunc->handleKeyEvent(_rEvent) : sal_False;
}
// -----------------------------------------------------------------------------
void OReportSection::createDefault(const ::rtl::OUString& _sType)
{
    SdrObject* pObj = m_pView->GetCreateObj();//rMarkList.GetMark(0)->GetObj();
    if ( !pObj )
        return;
    createDefault(_sType,pObj);
}
// -----------------------------------------------------------------------------
void OReportSection::createDefault(const ::rtl::OUString& _sType,SdrObject* _pObj)
{
    sal_Bool bAttributesAppliedFromGallery = sal_False;

    if ( GalleryExplorer::GetSdrObjCount( GALLERY_THEME_POWERPOINT ) )
    {
        std::vector< rtl::OUString > aObjList;
        if ( GalleryExplorer::FillObjListTitle( GALLERY_THEME_POWERPOINT, aObjList ) )
        {
            std::vector< rtl::OUString >::iterator aIter = aObjList.begin();
            std::vector< rtl::OUString >::iterator aEnd = aObjList.end();
            for (sal_uInt32 i=0 ; aIter != aEnd; ++aIter,++i)
            {
                if ( aIter->equalsIgnoreAsciiCase( _sType ) )
                {
                    OReportModel aReportModel(NULL);
                    SfxItemPool& rPool = aReportModel.GetItemPool();
                    rPool.FreezeIdRanges();
                    if ( GalleryExplorer::GetSdrObj( GALLERY_THEME_POWERPOINT, i, &aReportModel ) )
                    {
                        const SdrObject* pSourceObj = aReportModel.GetPage( 0 )->GetObj( 0 );
                        if( pSourceObj )
                        {
                            const SfxItemSet& rSource = pSourceObj->GetMergedItemSet();
                            SfxItemSet aDest( _pObj->GetModel()->GetItemPool(),                 // ranges from SdrAttrObj
                            SDRATTR_START, SDRATTR_SHADOW_LAST,
                            SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST,
                            SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION,
                            // Graphic Attributes
                            SDRATTR_GRAF_FIRST, SDRATTR_GRAF_LAST,
                            // 3d Properties
                            SDRATTR_3D_FIRST, SDRATTR_3D_LAST,
                            // CustomShape properties
                            SDRATTR_CUSTOMSHAPE_FIRST, SDRATTR_CUSTOMSHAPE_LAST,
                            // range from SdrTextObj
                            EE_ITEMS_START, EE_ITEMS_END,
                            // end
                            0, 0);
                            aDest.Set( rSource );
                            _pObj->SetMergedItemSet( aDest );
                            sal_Int32 nAngle = pSourceObj->GetRotateAngle();
                            if ( nAngle )
                            {
                                double a = nAngle * F_PI18000;
                                _pObj->NbcRotate( _pObj->GetSnapRect().Center(), nAngle, sin( a ), cos( a ) );
                            }
                            bAttributesAppliedFromGallery = sal_True;
                        }
                    }
                    break;
                }
            }
        }
    }
    if ( !bAttributesAppliedFromGallery )
    {
        _pObj->SetMergedItem( SvxAdjustItem( SVX_ADJUST_CENTER ,ITEMID_ADJUST) );
        _pObj->SetMergedItem( SdrTextVertAdjustItem( SDRTEXTVERTADJUST_CENTER ) );
        _pObj->SetMergedItem( SdrTextHorzAdjustItem( SDRTEXTHORZADJUST_BLOCK ) );
        _pObj->SetMergedItem( SdrTextAutoGrowHeightItem( sal_False ) );
        ((SdrObjCustomShape*)_pObj)->MergeDefaultAttributes( &_sType );
    }
}
// -----------------------------------------------------------------------------
uno::Reference< report::XReportComponent > OReportSection::getCurrentControlModel() const
{
    uno::Reference< report::XReportComponent > xModel;
    if ( m_pView )
    {
        const SdrMarkList& rMarkList = m_pView->GetMarkedObjectList();
        sal_uInt32 nMarkCount = rMarkList.GetMarkCount();

        if ( nMarkCount == 1 )
        {
            SdrObject* pDlgEdObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
            OObjectBase* pObj = dynamic_cast<OObjectBase*>(pDlgEdObj);
            if ( pObj )
                xModel = pObj->getReportComponent().get();
        }
    }
    return xModel;
}
// -----------------------------------------------------------------------------
void OReportSection::fillControlModelSelection(::std::vector< uno::Reference< report::XReportComponent > >& _rSelection) const
{
    if ( m_pView )
    {
        const SdrMarkList& rMarkList = m_pView->GetMarkedObjectList();
        const sal_uInt32 nMarkCount = rMarkList.GetMarkCount();

        for (sal_uInt32 i=0; i < nMarkCount; ++i)
        {
            const SdrObject* pDlgEdObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
            const OObjectBase* pObj = dynamic_cast<const OObjectBase*>(pDlgEdObj);
            if ( pObj )
                _rSelection.push_back(pObj->getReportComponent());
        }
    }
}
// -----------------------------------------------------------------------------
sal_Int8 OReportSection::AcceptDrop( const AcceptDropEvent& _rEvt )
{
    OSL_TRACE("AcceptDrop::DropEvent.Action %i\n", _rEvt.mnAction);

    ::Point aDropPos(_rEvt.maPosPixel);
    MouseEvent aMouseEvt(aDropPos);
    if ( m_pFunc->isOverlapping(aMouseEvt) )
        return DND_ACTION_NONE;

    if ( _rEvt.mnAction == DND_ACTION_COPY ||
         _rEvt.mnAction == DND_ACTION_LINK
         )
    {
        if (!m_pParent) return DND_ACTION_NONE;
        USHORT nCurrentPosition = 0;
        nCurrentPosition = m_pParent->getPosition(this);
        if (_rEvt.mnAction == DND_ACTION_COPY )
        {
            // we must assure, we can't drop in the top section
            if (nCurrentPosition < 1)
            {
                return DND_ACTION_NONE;
            }
            return DND_ACTION_LINK;
        }
        if (_rEvt.mnAction == DND_ACTION_LINK)
        {
            // we must assure, we can't drop in the bottom section
            if (m_pParent->getSectionCount() > (nCurrentPosition + 1)  )
            {
                return DND_ACTION_COPY;
            }
            return DND_ACTION_NONE;
        }
    }
    else
    {
        const DataFlavorExVector& rFlavors = GetDataFlavorExVector();
        if ( ::svx::OColumnTransferable::canExtractColumnDescriptor(rFlavors, CTF_FIELD_DESCRIPTOR | CTF_CONTROL_EXCHANGE | CTF_COLUMN_DESCRIPTOR) )
            return _rEvt.mnAction;

        const sal_Int8 nDropOption = ( OReportExchange::canExtract(rFlavors) ) ? DND_ACTION_COPYMOVE : DND_ACTION_NONE;

        return nDropOption;
    }
    return DND_ACTION_NONE;
}

// -----------------------------------------------------------------------------
sal_Int8 OReportSection::ExecuteDrop( const ExecuteDropEvent& _rEvt )
{
    OSL_TRACE("ExecuteDrop::DropEvent.Action %i\n", _rEvt.mnAction);
    ::Point aDropPos(PixelToLogic(_rEvt.maPosPixel));
    MouseEvent aMouseEvt(aDropPos);
    if ( m_pFunc->isOverlapping(aMouseEvt) )
        return DND_ACTION_NONE;

    sal_Int8 nDropOption = DND_ACTION_NONE;
    TransferableDataHelper aDropped(_rEvt.maDropEvent.Transferable);
    DataFlavorExVector& rFlavors = aDropped.GetDataFlavorExVector();
    if ( OReportExchange::canExtract(rFlavors) )
    {
        OReportExchange::TSectionElements aCopies = OReportExchange::extractCopies(aDropped);
        Paste(aCopies,true);
        nDropOption = DND_ACTION_COPYMOVE;
        m_pParent->BrkAction();
        m_pParent->unmarkAllObjects(m_pView);
        //m_pParent->getView()->setMarked(m_pView,sal_True);
    } // if ( OReportExchange::canExtract(rFlavors) )
    else if ( ::svx::OColumnTransferable::canExtractColumnDescriptor(rFlavors, CTF_FIELD_DESCRIPTOR | CTF_CONTROL_EXCHANGE | CTF_COLUMN_DESCRIPTOR) )
    {
        m_pParent->getView()->setMarked(m_pView,sal_True);
        m_pView->UnmarkAll();
        const Rectangle& rRect = m_pView->GetWorkArea();
        if ( aDropPos.X() < rRect.Left() )
            aDropPos.X() = rRect.Left();
        else if ( aDropPos.X() > rRect.Right() )
            aDropPos.X() = rRect.Right();

        if ( aDropPos.Y() > rRect.Bottom() )
            aDropPos.Y() = rRect.Bottom();

        ::svx::ODataAccessDescriptor aDescriptor = ::svx::OColumnTransferable::extractColumnDescriptor(aDropped);
        // we use this way to create undo actions
        OReportController* pController = getViewsWindow()->getView()->getReportView()->getController();
        uno::Sequence<beans::PropertyValue> aValues( aDescriptor.createPropertyValueSequence() );
        sal_Int32 nLength = aValues.getLength();
        aValues.realloc(nLength + 3);
        aValues[nLength].Name = PROPERTY_POSITION;
        aValues[nLength++].Value <<= AWTPoint(aDropPos);
        // give also the DND Action (Shift|Ctrl) Key to really say what we want
        aValues[nLength].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DNDAction"));
        aValues[nLength++].Value <<= _rEvt.mnAction;

        aValues[nLength].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Section"));
        aValues[nLength++].Value <<= getSection();

        pController->executeChecked(SID_ADD_CONTROL_PAIR,aValues);
        nDropOption = DND_ACTION_COPY;
    }
    return nDropOption;
}
// -----------------------------------------------------------------------------
void OReportSection::stopScrollTimer()
{
    m_pFunc->stopScrollTimer();
}
// -----------------------------------------------------------------------------
// =============================================================================
}
// =============================================================================
