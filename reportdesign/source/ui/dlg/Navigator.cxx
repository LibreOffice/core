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

#include "Navigator.hxx"

#include "uistrings.hrc"
#include "ReportController.hxx"
#include "UITools.hxx"
#include "RptUndo.hxx"
#include "reportformula.hxx"
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/report/XReportDefinition.hpp>
#include <com/sun/star/report/XFixedText.hpp>
#include <com/sun/star/report/XFixedLine.hpp>
#include <com/sun/star/report/XFormattedField.hpp>
#include <com/sun/star/report/XImageControl.hpp>
#include <com/sun/star/report/XShape.hpp>
#include "helpids.hrc"
#include "RptResId.hrc"
#include "rptui_slotid.hrc"
#include <tools/debug.hxx>
#include <comphelper/propmultiplex.hxx>
#include <comphelper/containermultiplexer.hxx>
#include <comphelper/types.hxx>
#include "cppuhelper/basemutex.hxx"
#include "comphelper/SelectionMultiplex.hxx"
#include <svtools/treelistbox.hxx>
#include <svl/solar.hrc>
#include "ReportVisitor.hxx"
#include "ModuleHelper.hxx"
#include <rtl/ref.hxx>

#include <memory>
#include <algorithm>

#define RID_SVXIMG_COLLAPSEDNODE            (RID_FORMS_START + 2)
#define RID_SVXIMG_EXPANDEDNODE             (RID_FORMS_START + 3)
#define DROP_ACTION_TIMER_INITIAL_TICKS     10
#define DROP_ACTION_TIMER_SCROLL_TICKS      3
#define DROP_ACTION_TIMER_TICK_BASE         10

namespace rptui
{
using namespace ::com::sun::star;
using namespace utl;
using namespace ::comphelper;

sal_uInt16 lcl_getImageId(const uno::Reference< report::XReportComponent>& _xElement)
{
    sal_uInt16 nId = 0;
    uno::Reference< report::XFixedLine> xFixedLine(_xElement,uno::UNO_QUERY);
    if ( uno::Reference< report::XFixedText>(_xElement,uno::UNO_QUERY).is() )
        nId = SID_FM_FIXEDTEXT;
    else if ( xFixedLine.is() )
        nId = xFixedLine->getOrientation() ? SID_INSERT_VFIXEDLINE : SID_INSERT_HFIXEDLINE;
    else if ( uno::Reference< report::XFormattedField>(_xElement,uno::UNO_QUERY).is() )
        nId = SID_FM_EDIT;
    else if ( uno::Reference< report::XImageControl>(_xElement,uno::UNO_QUERY).is() )
        nId = SID_FM_IMAGECONTROL;
    else if ( uno::Reference< report::XShape>(_xElement,uno::UNO_QUERY).is() )
        nId = SID_DRAWTBX_CS_BASIC;
    return nId;
}
// -----------------------------------------------------------------------------
::rtl::OUString lcl_getName(const uno::Reference< beans::XPropertySet>& _xElement)
{
    OSL_ENSURE(_xElement.is(),"Found report element which is NULL!");
    ::rtl::OUString sTempName;
    _xElement->getPropertyValue(PROPERTY_NAME) >>= sTempName;
    ::rtl::OUStringBuffer sName = sTempName;
    uno::Reference< report::XFixedText> xFixedText(_xElement,uno::UNO_QUERY);
    uno::Reference< report::XReportControlModel> xReportModel(_xElement,uno::UNO_QUERY);
    if ( xFixedText.is() )
    {
        sName.append(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" : ")));
        sName.append(xFixedText->getLabel());
    }
    else if ( xReportModel.is() && _xElement->getPropertySetInfo()->hasPropertyByName(PROPERTY_DATAFIELD) )
    {
        ReportFormula aFormula( xReportModel->getDataField() );
        if ( aFormula.isValid() )
        {
            sName.append(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" : ")));
            sName.append( aFormula.getUndecoratedContent() );
        }
    }
    return sName.makeStringAndClear();
}
// -----------------------------------------------------------------------------

class NavigatorTree :   public ::cppu::BaseMutex
                    ,   public SvTreeListBox
                    ,   public reportdesign::ITraverseReport
                    ,   public comphelper::OSelectionChangeListener
                    ,   public ::comphelper::OPropertyChangeListener
{
    class UserData;
    friend class UserData;
    class UserData : public ::cppu::BaseMutex
                    ,public ::comphelper::OPropertyChangeListener
                    ,public ::comphelper::OContainerListener
    {
        uno::Reference< uno::XInterface >                           m_xContent;
        ::rtl::Reference< comphelper::OPropertyChangeMultiplexer>   m_pListener;
        ::rtl::Reference< comphelper::OContainerListenerAdapter>    m_pContainerListener;
        NavigatorTree*                                              m_pTree;
    public:
        UserData(NavigatorTree* _pTree,const uno::Reference<uno::XInterface>& _xContent);
        ~UserData();

        inline uno::Reference< uno::XInterface > getContent() const { return m_xContent; }
        inline void setContent(const uno::Reference< uno::XInterface >& _xContent) { m_xContent = _xContent; }
    protected:
        // OPropertyChangeListener
        virtual void _propertyChanged(const beans::PropertyChangeEvent& _rEvent) throw( uno::RuntimeException);

        // OContainerListener
        virtual void _elementInserted( const container::ContainerEvent& _rEvent ) throw(uno::RuntimeException);
        virtual void _elementRemoved( const container::ContainerEvent& _Event ) throw(uno::RuntimeException);
        virtual void _elementReplaced( const container::ContainerEvent& _rEvent ) throw(uno::RuntimeException);
        virtual void _disposing(const lang::EventObject& _rSource) throw( uno::RuntimeException);
    };

    enum DROP_ACTION        { DA_SCROLLUP, DA_SCROLLDOWN, DA_EXPANDNODE };
    AutoTimer                                                                   m_aDropActionTimer;
    Timer                                                                       m_aSynchronizeTimer;
    ImageList                                                                   m_aNavigatorImages;
    Point                                                                       m_aTimerTriggered;      // die Position, an der der DropTimer angeschaltet wurde
    DROP_ACTION                                                                 m_aDropActionType;
    OReportController&                                                          m_rController;
    SvTreeListEntry*                                                                m_pMasterReport;
    SvTreeListEntry*                                                                m_pDragedEntry;
    ::rtl::Reference< comphelper::OPropertyChangeMultiplexer>                   m_pReportListener;
    ::rtl::Reference< comphelper::OSelectionChangeMultiplexer>                  m_pSelectionListener;
    unsigned short                                                              m_nTimerCounter;

    SvTreeListEntry* insertEntry(const ::rtl::OUString& _sName,SvTreeListEntry* _pParent,sal_uInt16 _nImageId,sal_uLong _nPosition,UserData* _pData);
    void traverseSection(const uno::Reference< report::XSection>& _xSection,SvTreeListEntry* _pParent,sal_uInt16 _nImageId,sal_uLong _nPosition = LIST_APPEND);
    void traverseFunctions(const uno::Reference< report::XFunctions>& _xFunctions,SvTreeListEntry* _pParent);

    NavigatorTree(const NavigatorTree&);
    void operator =(const NavigatorTree&);
protected:
    virtual void        Command( const CommandEvent& rEvt );
    // DragSourceHelper overridables
    virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel );
    // DropTargetHelper overridables
    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& _rEvt );
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& _rEvt );

    // OSelectionChangeListener
    virtual void _disposing(const lang::EventObject& _rSource) throw( uno::RuntimeException);

    // OPropertyChangeListener
    virtual void _propertyChanged(const beans::PropertyChangeEvent& _rEvent) throw( uno::RuntimeException);

    // OContainerListener Helper
    void _elementInserted( const container::ContainerEvent& _rEvent );
    void _elementRemoved( const container::ContainerEvent& _Event );
    void _elementReplaced( const container::ContainerEvent& _rEvent );

public:
    NavigatorTree(Window* pParent,OReportController& _rController );
    virtual ~NavigatorTree();

    DECL_LINK(OnEntrySelDesel, NavigatorTree*);
    DECL_LINK( OnDropActionTimer, void* );

    virtual void _selectionChanged( const lang::EventObject& aEvent ) throw (uno::RuntimeException);

    // ITraverseReport
    virtual void traverseReport(const uno::Reference< report::XReportDefinition>& _xReport);
    virtual void traverseReportFunctions(const uno::Reference< report::XFunctions>& _xFunctions);
    virtual void traverseReportHeader(const uno::Reference< report::XSection>& _xSection);
    virtual void traverseReportFooter(const uno::Reference< report::XSection>& _xSection);
    virtual void traversePageHeader(const uno::Reference< report::XSection>& _xSection);
    virtual void traversePageFooter(const uno::Reference< report::XSection>& _xSection);

    virtual void traverseGroups(const uno::Reference< report::XGroups>& _xGroups);
    virtual void traverseGroup(const uno::Reference< report::XGroup>& _xGroup);
    virtual void traverseGroupFunctions(const uno::Reference< report::XFunctions>& _xFunctions);
    virtual void traverseGroupHeader(const uno::Reference< report::XSection>& _xSection);
    virtual void traverseGroupFooter(const uno::Reference< report::XSection>& _xSection);

    virtual void traverseDetail(const uno::Reference< report::XSection>& _xSection);

    SvTreeListEntry* find(const uno::Reference< uno::XInterface >& _xContent);
    void removeEntry(SvTreeListEntry* _pEntry,bool _bRemove = true);
private:
    using SvTreeListBox::ExecuteDrop;
};
DBG_NAME(rpt_NavigatorTree)
// -----------------------------------------------------------------------------
NavigatorTree::NavigatorTree( Window* pParent,OReportController& _rController )
        :SvTreeListBox( pParent, WB_TABSTOP| WB_HASBUTTONS|WB_HASLINES|WB_BORDER|WB_HSCROLL|WB_HASBUTTONSATROOT )
        ,comphelper::OSelectionChangeListener(m_aMutex)
        ,OPropertyChangeListener(m_aMutex)
        ,m_aTimerTriggered(-1,-1)
        ,m_aDropActionType( DA_SCROLLUP )
        ,m_rController(_rController)
        ,m_pMasterReport(NULL)
        ,m_pDragedEntry(NULL)
        ,m_nTimerCounter( DROP_ACTION_TIMER_INITIAL_TICKS )
{
    DBG_CTOR(rpt_NavigatorTree,NULL);
    m_pReportListener = new OPropertyChangeMultiplexer(this,m_rController.getReportDefinition().get());
    m_pReportListener->addProperty(PROPERTY_PAGEHEADERON);
    m_pReportListener->addProperty(PROPERTY_PAGEFOOTERON);
    m_pReportListener->addProperty(PROPERTY_REPORTHEADERON);
    m_pReportListener->addProperty(PROPERTY_REPORTFOOTERON);

    m_pSelectionListener = new OSelectionChangeMultiplexer(this,&m_rController);

    SetHelpId( HID_REPORT_NAVIGATOR_TREE );

    m_aNavigatorImages = ImageList( ModuleRes( RID_SVXIMGLIST_RPTEXPL ) );

    SetNodeBitmaps(
        m_aNavigatorImages.GetImage( RID_SVXIMG_COLLAPSEDNODE ),
        m_aNavigatorImages.GetImage( RID_SVXIMG_EXPANDEDNODE )
    );

    SetDragDropMode(0xFFFF);
    EnableInplaceEditing( sal_False );
    SetSelectionMode(MULTIPLE_SELECTION);
    Clear();

    m_aDropActionTimer.SetTimeoutHdl(LINK(this, NavigatorTree, OnDropActionTimer));
    SetSelectHdl(LINK(this, NavigatorTree, OnEntrySelDesel));
    SetDeselectHdl(LINK(this, NavigatorTree, OnEntrySelDesel));
}
// -----------------------------------------------------------------------------
NavigatorTree::~NavigatorTree()
{
    SvTreeListEntry* pCurrent = First();
    while ( pCurrent )
    {
        delete static_cast<UserData*>(pCurrent->GetUserData());
        pCurrent = Next(pCurrent);
    }
    m_pReportListener->dispose();
    m_pSelectionListener->dispose();
    DBG_DTOR(rpt_NavigatorTree,NULL);
}
//------------------------------------------------------------------------------
void NavigatorTree::Command( const CommandEvent& rEvt )
{
    sal_Bool bHandled = sal_False;
    switch( rEvt.GetCommand() )
    {
        case COMMAND_CONTEXTMENU:
        {
            // die Stelle, an der geklickt wurde
            SvTreeListEntry* ptClickedOn = NULL;
            ::Point aWhere;
            if (rEvt.IsMouseEvent())
            {
                aWhere = rEvt.GetMousePosPixel();
                ptClickedOn = GetEntry(aWhere);
                if (ptClickedOn == NULL)
                    break;
                if ( !IsSelected(ptClickedOn) )
                {
                    SelectAll(sal_False);
                    Select(ptClickedOn, sal_True);
                    SetCurEntry(ptClickedOn);
                }
            }
            else
            {
                ptClickedOn = GetCurEntry();
                if ( !ptClickedOn )
                    break;
                aWhere = GetEntryPosition(ptClickedOn);
            }
            UserData* pData = static_cast<UserData*>(ptClickedOn->GetUserData());
            uno::Reference< report::XFunctionsSupplier> xSupplier(pData->getContent(),uno::UNO_QUERY);
            uno::Reference< report::XFunctions> xFunctions(pData->getContent(),uno::UNO_QUERY);
            uno::Reference< report::XGroup> xGroup(pData->getContent(),uno::UNO_QUERY);
            sal_Bool bDeleteAllowed = m_rController.isEditable() && (xGroup.is() ||
                                      uno::Reference< report::XFunction>(pData->getContent(),uno::UNO_QUERY).is());
            PopupMenu aContextMenu( ModuleRes( RID_MENU_NAVIGATOR ) );

            sal_uInt16 nCount = aContextMenu.GetItemCount();
            for (sal_uInt16 i = 0; i < nCount; ++i)
            {
                if ( MENUITEM_SEPARATOR != aContextMenu.GetItemType(i))
                {
                    sal_uInt16 nId = aContextMenu.GetItemId(i);

                    aContextMenu.CheckItem(nId,m_rController.isCommandChecked(nId));
                    sal_Bool bEnabled = m_rController.isCommandEnabled(nId);
                    if ( nId == SID_RPT_NEW_FUNCTION )
                        aContextMenu.EnableItem(nId,m_rController.isEditable() && (xSupplier.is() || xFunctions.is()) );
                    // special condition, check for function and group
                    else if ( nId == SID_DELETE )
                        aContextMenu.EnableItem(SID_DELETE,bDeleteAllowed);
                    else
                        aContextMenu.EnableItem(nId,bEnabled);
                }
            }
            sal_uInt16 nId = aContextMenu.Execute(this, aWhere);
            if ( nId )
            {
                uno::Sequence< beans::PropertyValue> aArgs;
                if ( nId == SID_RPT_NEW_FUNCTION )
                {
                    aArgs.realloc(1);
                    aArgs[0].Value <<= (xFunctions.is() ? xFunctions : xSupplier->getFunctions());
                }
                else if ( nId == SID_DELETE )
                {
                    if ( xGroup.is() )
                        nId = SID_GROUP_REMOVE;
                    aArgs.realloc(1);
                    aArgs[0].Name = PROPERTY_GROUP;
                    aArgs[0].Value <<= pData->getContent();
                }
                m_rController.executeUnChecked(nId,aArgs);
            }

            bHandled = sal_True;
        } break;
    }

    if (!bHandled)
        SvTreeListBox::Command( rEvt );
}
// -----------------------------------------------------------------------------
sal_Int8 NavigatorTree::AcceptDrop( const AcceptDropEvent& _rEvt )
{
    sal_Int8 nDropOption = DND_ACTION_NONE;
    ::Point aDropPos = _rEvt.maPosPixel;
    if (_rEvt.mbLeaving)
    {
        if (m_aDropActionTimer.IsActive())
            m_aDropActionTimer.Stop();
    }
    else
    {
        bool bNeedTrigger = false;
        // auf dem ersten Eintrag ?
        if ((aDropPos.Y() >= 0) && (aDropPos.Y() < GetEntryHeight()))
        {
            m_aDropActionType = DA_SCROLLUP;
            bNeedTrigger = true;
        }
        else if ((aDropPos.Y() < GetSizePixel().Height()) && (aDropPos.Y() >= GetSizePixel().Height() - GetEntryHeight()))
        {
            m_aDropActionType = DA_SCROLLDOWN;
            bNeedTrigger = true;
        }
        else
        {
            SvTreeListEntry* pDropppedOn = GetEntry(aDropPos);
            if (pDropppedOn && (GetChildCount(pDropppedOn) > 0) && !IsExpanded(pDropppedOn))
            {
                m_aDropActionType = DA_EXPANDNODE;
                bNeedTrigger = true;
            }
        }

        if (bNeedTrigger && (m_aTimerTriggered != aDropPos))
        {
            // neu anfangen zu zaehlen
            m_nTimerCounter = DROP_ACTION_TIMER_INITIAL_TICKS;
            // die Pos merken, da ich auch AcceptDrops bekomme, wenn sich die Maus gar nicht bewegt hat
            m_aTimerTriggered = aDropPos;
            // und den Timer los
            if (!m_aDropActionTimer.IsActive()) // gibt es den Timer schon ?
            {
                m_aDropActionTimer.SetTimeout(DROP_ACTION_TIMER_TICK_BASE);
                m_aDropActionTimer.Start();
            }
        }
        else if (!bNeedTrigger)
            m_aDropActionTimer.Stop();
    }

    return nDropOption;
}
// -------------------------------------------------------------------------
sal_Int8 NavigatorTree::ExecuteDrop( const ExecuteDropEvent& /*_rEvt*/ )
{
    return DND_ACTION_NONE;
}
// -------------------------------------------------------------------------
void NavigatorTree::StartDrag( sal_Int8 /*_nAction*/, const Point& _rPosPixel )
{
    m_pDragedEntry = GetEntry(_rPosPixel);
    if ( m_pDragedEntry )
    {
        EndSelection();
    }
}
//------------------------------------------------------------------------
IMPL_LINK_NOARG(NavigatorTree, OnDropActionTimer)
{
    if (--m_nTimerCounter > 0)
        return 0L;

    switch ( m_aDropActionType )
    {
    case DA_EXPANDNODE:
    {
        SvTreeListEntry* pToExpand = GetEntry(m_aTimerTriggered);
        if (pToExpand && (GetChildCount(pToExpand) > 0) &&  !IsExpanded(pToExpand))
            // tja, eigentlich muesste ich noch testen, ob die Node nicht schon expandiert ist, aber ich
            // habe dazu weder in den Basisklassen noch im Model eine Methode gefunden ...
            // aber ich denke, die BK sollte es auch so vertragen
            Expand(pToExpand);

        // nach dem Expand habe ich im Gegensatz zum Scrollen natuerlich nix mehr zu tun
        m_aDropActionTimer.Stop();
    }
    break;

    case DA_SCROLLUP :
        ScrollOutputArea( 1 );
        m_nTimerCounter = DROP_ACTION_TIMER_SCROLL_TICKS;
        break;

    case DA_SCROLLDOWN :
        ScrollOutputArea( -1 );
        m_nTimerCounter = DROP_ACTION_TIMER_SCROLL_TICKS;
        break;

    }

    return 0L;
}

// -----------------------------------------------------------------------------
IMPL_LINK(NavigatorTree, OnEntrySelDesel, NavigatorTree*, /*pThis*/)
{
    if ( !m_pSelectionListener->locked() )
    {
        m_pSelectionListener->lock();
        SvTreeListEntry* pEntry = GetCurEntry();
        uno::Any aSelection;
        if ( IsSelected(pEntry) )
            aSelection <<= static_cast<UserData*>(pEntry->GetUserData())->getContent();
        m_rController.select(aSelection);
        m_pSelectionListener->unlock();
    }

    return 0L;
}
// -----------------------------------------------------------------------------
void NavigatorTree::_selectionChanged( const lang::EventObject& aEvent ) throw (uno::RuntimeException)
{
    m_pSelectionListener->lock();
    uno::Reference< view::XSelectionSupplier> xSelectionSupplier(aEvent.Source,uno::UNO_QUERY);
    uno::Any aSec = xSelectionSupplier->getSelection();
    uno::Sequence< uno::Reference< report::XReportComponent > > aSelection;
    aSec >>= aSelection;
    if ( !aSelection.getLength() )
    {
        uno::Reference< uno::XInterface> xSelection(aSec,uno::UNO_QUERY);
        SvTreeListEntry* pEntry = find(xSelection);
        if ( pEntry && !IsSelected(pEntry) )
        {
            Select(pEntry, sal_True);
            SetCurEntry(pEntry);
        }
        else if ( !pEntry )
            SelectAll(sal_False,sal_False);
    }
    else
    {
        const uno::Reference< report::XReportComponent >* pIter = aSelection.getConstArray();
        const uno::Reference< report::XReportComponent >* pEnd  = pIter + aSelection.getLength();
        for (; pIter != pEnd; ++pIter)
        {
            SvTreeListEntry* pEntry = find(*pIter);
            if ( pEntry && !IsSelected(pEntry) )
            {
                Select(pEntry, sal_True);
                SetCurEntry(pEntry);
            }
        }
    }
    m_pSelectionListener->unlock();
}
// -----------------------------------------------------------------------------
SvTreeListEntry* NavigatorTree::insertEntry(const ::rtl::OUString& _sName,SvTreeListEntry* _pParent,sal_uInt16 _nImageId,sal_uLong _nPosition,UserData* _pData)
{
    SvTreeListEntry* pEntry = NULL;
    if ( _nImageId )
    {
        const Image aImage( m_aNavigatorImages.GetImage( _nImageId ) );
        pEntry = InsertEntry(_sName,aImage,aImage,_pParent,sal_False,_nPosition,_pData);
    }
    else
        pEntry = InsertEntry(_sName,_pParent,sal_False,_nPosition,_pData);
    return pEntry;
}
// -----------------------------------------------------------------------------
void NavigatorTree::traverseSection(const uno::Reference< report::XSection>& _xSection,SvTreeListEntry* _pParent,sal_uInt16 _nImageId,sal_uLong _nPosition)
{
    SvTreeListEntry* pSection = insertEntry(_xSection->getName(),_pParent,_nImageId,_nPosition,new UserData(this,_xSection));
    const sal_Int32 nCount = _xSection->getCount();
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        uno::Reference< report::XReportComponent> xElement(_xSection->getByIndex(i),uno::UNO_QUERY_THROW);
        OSL_ENSURE(xElement.is(),"Found report element which is NULL!");
        insertEntry(lcl_getName(xElement.get()),pSection,lcl_getImageId(xElement),LIST_APPEND,new UserData(this,xElement));
        uno::Reference< report::XReportDefinition> xSubReport(xElement,uno::UNO_QUERY);
        if ( xSubReport.is() )
        {
            m_pMasterReport = find(_xSection->getReportDefinition());
            reportdesign::OReportVisitor aSubVisitor(this);
            aSubVisitor.start(xSubReport);
        }
    }
}
// -----------------------------------------------------------------------------
void NavigatorTree::traverseFunctions(const uno::Reference< report::XFunctions>& _xFunctions,SvTreeListEntry* _pParent)
{
    SvTreeListEntry* pFunctions = insertEntry(String(ModuleRes(RID_STR_FUNCTIONS)),_pParent,SID_RPT_NEW_FUNCTION,LIST_APPEND,new UserData(this,_xFunctions));
    const sal_Int32 nCount = _xFunctions->getCount();
    for (sal_Int32 i = 0; i< nCount; ++i)
    {
        uno::Reference< report::XFunction> xElement(_xFunctions->getByIndex(i),uno::UNO_QUERY);
        insertEntry(xElement->getName(),pFunctions,SID_RPT_NEW_FUNCTION,LIST_APPEND,new UserData(this,xElement));
    }
}
// -----------------------------------------------------------------------------
SvTreeListEntry* NavigatorTree::find(const uno::Reference< uno::XInterface >& _xContent)
{
    SvTreeListEntry* pRet = NULL;
    if ( _xContent.is() )
    {
        SvTreeListEntry* pCurrent = First();
        while ( pCurrent )
        {
            UserData* pData = static_cast<UserData*>(pCurrent->GetUserData());
            OSL_ENSURE(pData,"No UserData set an entry!");
            if ( pData->getContent() == _xContent )
            {
                pRet = pCurrent;
                break;
            }
            pCurrent = Next(pCurrent);
        }
    }
    return pRet;
}
// -----------------------------------------------------------------------------
// ITraverseReport
// -----------------------------------------------------------------------------
void NavigatorTree::traverseReport(const uno::Reference< report::XReportDefinition>& _xReport)
{
    insertEntry(_xReport->getName(),m_pMasterReport,SID_SELECT_REPORT,LIST_APPEND,new UserData(this,_xReport));
}
// -----------------------------------------------------------------------------
void NavigatorTree::traverseReportFunctions(const uno::Reference< report::XFunctions>& _xFunctions)
{
    SvTreeListEntry* pReport = find(_xFunctions->getParent());
    traverseFunctions(_xFunctions,pReport);
}
// -----------------------------------------------------------------------------
void NavigatorTree::traverseReportHeader(const uno::Reference< report::XSection>& _xSection)
{
    SvTreeListEntry* pReport = find(_xSection->getReportDefinition());
    traverseSection(_xSection,pReport,SID_REPORTHEADERFOOTER);
}
// -----------------------------------------------------------------------------
void NavigatorTree::traverseReportFooter(const uno::Reference< report::XSection>& _xSection)
{
    SvTreeListEntry* pReport = find(_xSection->getReportDefinition());
    traverseSection(_xSection,pReport,SID_REPORTHEADERFOOTER);
}
// -----------------------------------------------------------------------------
void NavigatorTree::traversePageHeader(const uno::Reference< report::XSection>& _xSection)
{
    SvTreeListEntry* pReport = find(_xSection->getReportDefinition());
    traverseSection(_xSection,pReport,SID_PAGEHEADERFOOTER);
}
// -----------------------------------------------------------------------------
void NavigatorTree::traversePageFooter(const uno::Reference< report::XSection>& _xSection)
{
    SvTreeListEntry* pReport = find(_xSection->getReportDefinition());
    traverseSection(_xSection,pReport,SID_PAGEHEADERFOOTER);
}
// -----------------------------------------------------------------------------
void NavigatorTree::traverseGroups(const uno::Reference< report::XGroups>& _xGroups)
{
    SvTreeListEntry* pReport = find(_xGroups->getReportDefinition());
    insertEntry(String(ModuleRes(RID_STR_GROUPS)),pReport,SID_SORTINGANDGROUPING,LIST_APPEND,new UserData(this,_xGroups));
}
// -----------------------------------------------------------------------------
void NavigatorTree::traverseGroup(const uno::Reference< report::XGroup>& _xGroup)
{
    uno::Reference< report::XGroups> xGroups(_xGroup->getParent(),uno::UNO_QUERY);
    SvTreeListEntry* pGroups = find(xGroups);
    OSL_ENSURE(pGroups,"No Groups inserted so far. Why!");
    insertEntry(_xGroup->getExpression(),pGroups,SID_GROUP,rptui::getPositionInIndexAccess(xGroups.get(),_xGroup),new UserData(this,_xGroup));
}
// -----------------------------------------------------------------------------
void NavigatorTree::traverseGroupFunctions(const uno::Reference< report::XFunctions>& _xFunctions)
{
    SvTreeListEntry* pGroup = find(_xFunctions->getParent());
    traverseFunctions(_xFunctions,pGroup);
}
// -----------------------------------------------------------------------------
void NavigatorTree::traverseGroupHeader(const uno::Reference< report::XSection>& _xSection)
{
    SvTreeListEntry* pGroup = find(_xSection->getGroup());
    OSL_ENSURE(pGroup,"No group found");
    traverseSection(_xSection,pGroup,SID_GROUPHEADER,1);
}
// -----------------------------------------------------------------------------
void NavigatorTree::traverseGroupFooter(const uno::Reference< report::XSection>& _xSection)
{
    SvTreeListEntry* pGroup = find(_xSection->getGroup());
    OSL_ENSURE(pGroup,"No group found");
    traverseSection(_xSection,pGroup,SID_GROUPFOOTER);
}
// -----------------------------------------------------------------------------
void NavigatorTree::traverseDetail(const uno::Reference< report::XSection>& _xSection)
{
    uno::Reference< report::XReportDefinition> xReport = _xSection->getReportDefinition();
    SvTreeListEntry* pParent = find(xReport);
    traverseSection(_xSection,pParent,SID_ICON_DETAIL);
}
// -----------------------------------------------------------------------------
void NavigatorTree::_propertyChanged(const beans::PropertyChangeEvent& _rEvent) throw( uno::RuntimeException)
{
    uno::Reference< report::XReportDefinition> xReport(_rEvent.Source,uno::UNO_QUERY);
    if ( xReport.is() )
    {
        sal_Bool bEnabled = sal_False;
        _rEvent.NewValue >>= bEnabled;
        if ( bEnabled )
        {
            SvTreeListEntry* pParent = find(xReport);
            if ( _rEvent.PropertyName == PROPERTY_REPORTHEADERON )
            {
                sal_uLong nPos = xReport->getReportHeaderOn() ? 2 : 1;
                traverseSection(xReport->getReportHeader(),pParent,SID_REPORTHEADERFOOTER,nPos);
            }
            else if ( _rEvent.PropertyName == PROPERTY_PAGEHEADERON )
            {
                traverseSection(xReport->getPageHeader(),pParent, SID_PAGEHEADERFOOTER,1);
            }
            else if ( _rEvent.PropertyName == PROPERTY_PAGEFOOTERON )
                traverseSection(xReport->getPageFooter(),pParent, SID_PAGEHEADERFOOTER);
            else if ( _rEvent.PropertyName == PROPERTY_REPORTFOOTERON )
            {
                sal_uLong nPos = xReport->getPageFooterOn() ? (GetLevelChildCount(pParent) - 1) : LIST_APPEND;
                traverseSection(xReport->getReportFooter(),pParent,SID_REPORTHEADERFOOTER,nPos);
            }
        }
    }
}
// -----------------------------------------------------------------------------
void NavigatorTree::_elementInserted( const container::ContainerEvent& _rEvent )
{
    SvTreeListEntry* pEntry = find(_rEvent.Source);
    uno::Reference<beans::XPropertySet> xProp(_rEvent.Element,uno::UNO_QUERY_THROW);
    ::rtl::OUString sName;
    uno::Reference< beans::XPropertySetInfo> xInfo = xProp->getPropertySetInfo();
    if ( xInfo.is() )
    {
        if ( xInfo->hasPropertyByName(PROPERTY_NAME) )
            xProp->getPropertyValue(PROPERTY_NAME) >>= sName;
        else if ( xInfo->hasPropertyByName(PROPERTY_EXPRESSION) )
            xProp->getPropertyValue(PROPERTY_EXPRESSION) >>= sName;
    }
    uno::Reference< report::XGroup> xGroup(xProp,uno::UNO_QUERY);
    if ( xGroup.is() )
    {
        reportdesign::OReportVisitor aSubVisitor(this);
        aSubVisitor.start(xGroup);
    }
    else
    {
        uno::Reference< report::XReportComponent> xElement(xProp,uno::UNO_QUERY);
        if ( xProp.is() )
            sName = lcl_getName(xProp);
        insertEntry(sName,pEntry,(!xElement.is() ? sal_uInt16(SID_RPT_NEW_FUNCTION) : lcl_getImageId(xElement)),LIST_APPEND,new UserData(this,xProp));
    }
    if ( !IsExpanded(pEntry) )
        Expand(pEntry);
}
// -----------------------------------------------------------------------------
void NavigatorTree::_elementRemoved( const container::ContainerEvent& _rEvent )
{
    uno::Reference<beans::XPropertySet> xProp(_rEvent.Element,uno::UNO_QUERY);
    SvTreeListEntry* pEntry = find(xProp);
    OSL_ENSURE(pEntry,"NavigatorTree::_elementRemoved: No Entry found!");

    if ( pEntry )
    {
        SvTreeListEntry* pParent = GetParent(pEntry);
        removeEntry(pEntry);
        PaintEntry(pParent);
    }
}
// -----------------------------------------------------------------------------
void NavigatorTree::_elementReplaced( const container::ContainerEvent& _rEvent )
{
    uno::Reference<beans::XPropertySet> xProp(_rEvent.ReplacedElement,uno::UNO_QUERY);
    SvTreeListEntry* pEntry = find(xProp);
    if ( pEntry )
    {
        UserData* pData = static_cast<UserData*>(pEntry->GetUserData());
        xProp.set(_rEvent.Element,uno::UNO_QUERY);
        pData->setContent(xProp);
        ::rtl::OUString sName;
        xProp->getPropertyValue(PROPERTY_NAME) >>= sName;
        SetEntryText(pEntry,sName);
    }
}
// -----------------------------------------------------------------------------
void NavigatorTree::_disposing(const lang::EventObject& _rSource)throw( uno::RuntimeException)
{
    removeEntry(find(_rSource.Source));
}
// -----------------------------------------------------------------------------
void NavigatorTree::removeEntry(SvTreeListEntry* _pEntry,bool _bRemove)
{
    if ( _pEntry )
    {
        SvTreeListEntry* pChild = FirstChild(_pEntry);
        while( pChild )
        {
            removeEntry(pChild,false);
            pChild = NextSibling(pChild);
        }
        delete static_cast<UserData*>(_pEntry->GetUserData());
        if ( _bRemove )
            GetModel()->Remove(_pEntry);
    }
}
DBG_NAME(rpt_NavigatorTree_UserData)
// -----------------------------------------------------------------------------
NavigatorTree::UserData::UserData(NavigatorTree* _pTree,const uno::Reference<uno::XInterface>& _xContent)
    : OPropertyChangeListener(m_aMutex)
    , OContainerListener(m_aMutex)
    , m_xContent(_xContent)
    , m_pTree(_pTree)
{
    DBG_CTOR(rpt_NavigatorTree_UserData,NULL);
    uno::Reference<beans::XPropertySet> xProp(m_xContent,uno::UNO_QUERY);
    if ( xProp.is() )
    {
        uno::Reference< beans::XPropertySetInfo> xInfo = xProp->getPropertySetInfo();
        if ( xInfo.is() )
        {
            m_pListener = new ::comphelper::OPropertyChangeMultiplexer(this,xProp);
            if ( xInfo->hasPropertyByName(PROPERTY_NAME) )
                m_pListener->addProperty(PROPERTY_NAME);
            else if ( xInfo->hasPropertyByName(PROPERTY_EXPRESSION) )
                m_pListener->addProperty(PROPERTY_EXPRESSION);
            if ( xInfo->hasPropertyByName(PROPERTY_DATAFIELD) )
                m_pListener->addProperty(PROPERTY_DATAFIELD);
            if ( xInfo->hasPropertyByName(PROPERTY_LABEL) )
                m_pListener->addProperty(PROPERTY_LABEL);
            if ( xInfo->hasPropertyByName(PROPERTY_HEADERON) )
                m_pListener->addProperty(PROPERTY_HEADERON);
            if ( xInfo->hasPropertyByName(PROPERTY_FOOTERON) )
                m_pListener->addProperty(PROPERTY_FOOTERON);
        }
    }
    uno::Reference< container::XContainer> xContainer(m_xContent,uno::UNO_QUERY);
    if ( xContainer.is() )
    {
        m_pContainerListener = new ::comphelper::OContainerListenerAdapter(this,xContainer);
    }
}
// -----------------------------------------------------------------------------
NavigatorTree::UserData::~UserData()
{
    DBG_DTOR(rpt_NavigatorTree_UserData,NULL);
    if ( m_pContainerListener.is() )
        m_pContainerListener->dispose();
    if ( m_pListener.is() )
        m_pListener->dispose();
}
// -----------------------------------------------------------------------------
// OPropertyChangeListener
void NavigatorTree::UserData::_propertyChanged(const beans::PropertyChangeEvent& _rEvent) throw( uno::RuntimeException)
{
    SvTreeListEntry* pEntry = m_pTree->find(_rEvent.Source);
    OSL_ENSURE(pEntry,"No entry could be found! Why not!");
    const bool bFooterOn = (PROPERTY_FOOTERON == _rEvent.PropertyName);
    try
    {
        if ( bFooterOn || PROPERTY_HEADERON == _rEvent.PropertyName )
        {
            sal_Int32 nPos = 1;
            uno::Reference< report::XGroup> xGroup(_rEvent.Source,uno::UNO_QUERY);
            ::std::mem_fun_t< sal_Bool,OGroupHelper> pIsOn = ::std::mem_fun(&OGroupHelper::getHeaderOn);
            ::std::mem_fun_t< uno::Reference<report::XSection> ,OGroupHelper> pMemFunSection = ::std::mem_fun(&OGroupHelper::getHeader);
            if ( bFooterOn )
            {
                pIsOn = ::std::mem_fun(&OGroupHelper::getFooterOn);
                pMemFunSection = ::std::mem_fun(&OGroupHelper::getFooter);
                nPos = m_pTree->GetChildCount(pEntry) - 1;
            }

            OGroupHelper aGroupHelper(xGroup);
            if ( pIsOn(&aGroupHelper) )
            {
                if ( bFooterOn )
                    ++nPos;
                m_pTree->traverseSection(pMemFunSection(&aGroupHelper),pEntry,bFooterOn ? SID_GROUPFOOTER : SID_GROUPHEADER,nPos);
            }
        }
        else if ( PROPERTY_EXPRESSION == _rEvent.PropertyName)
        {
            ::rtl::OUString sNewName;
            _rEvent.NewValue >>= sNewName;
            m_pTree->SetEntryText(pEntry,sNewName);
        }
        else if ( PROPERTY_DATAFIELD == _rEvent.PropertyName || PROPERTY_LABEL == _rEvent.PropertyName || PROPERTY_NAME == _rEvent.PropertyName )
        {
            uno::Reference<beans::XPropertySet> xProp(_rEvent.Source,uno::UNO_QUERY);
            m_pTree->SetEntryText(pEntry,lcl_getName(xProp));
        }
    }
    catch(const uno::Exception &)
    {}
}
// -----------------------------------------------------------------------------
void NavigatorTree::UserData::_elementInserted( const container::ContainerEvent& _rEvent ) throw(uno::RuntimeException)
{
    m_pTree->_elementInserted( _rEvent );
}
// -----------------------------------------------------------------------------
void NavigatorTree::UserData::_elementRemoved( const container::ContainerEvent& _rEvent ) throw(uno::RuntimeException)
{
    m_pTree->_elementRemoved( _rEvent );
}
// -----------------------------------------------------------------------------
void NavigatorTree::UserData::_elementReplaced( const container::ContainerEvent& _rEvent ) throw(uno::RuntimeException)
{
    m_pTree->_elementReplaced( _rEvent );
}
// -----------------------------------------------------------------------------
void NavigatorTree::UserData::_disposing(const lang::EventObject& _rSource) throw( uno::RuntimeException)
{
    m_pTree->_disposing( _rSource );
}
// -----------------------------------------------------------------------------
// class ONavigatorImpl
// -----------------------------------------------------------------------------
class ONavigatorImpl
{
    ONavigatorImpl(const ONavigatorImpl&);
    void operator =(const ONavigatorImpl&);
public:
    ONavigatorImpl(OReportController& _rController,ONavigator* _pParent);
    virtual ~ONavigatorImpl();

    uno::Reference< report::XReportDefinition>  m_xReport;
    ::rptui::OReportController&                 m_rController;
    ::std::auto_ptr<NavigatorTree>              m_pNavigatorTree;
};

ONavigatorImpl::ONavigatorImpl(OReportController& _rController,ONavigator* _pParent)
    :m_xReport(_rController.getReportDefinition())
    ,m_rController(_rController)
    ,m_pNavigatorTree(new NavigatorTree(_pParent,_rController))
{
    reportdesign::OReportVisitor aVisitor(m_pNavigatorTree.get());
    aVisitor.start(m_xReport);
    m_pNavigatorTree->Expand(m_pNavigatorTree->find(m_xReport));
    lang::EventObject aEvent(m_rController);
    m_pNavigatorTree->_selectionChanged(aEvent);
}
//------------------------------------------------------------------------
ONavigatorImpl::~ONavigatorImpl()
{
}
// -----------------------------------------------------------------------------
DBG_NAME( rpt_ONavigator )
const long STD_WIN_SIZE_X = 210;
const long STD_WIN_SIZE_Y = 280;
const long LISTBOX_BORDER = 2;
//========================================================================
// class ONavigator
//========================================================================
ONavigator::ONavigator( Window* _pParent
                        ,OReportController& _rController)
    : FloatingWindow( _pParent, ModuleRes(RID_NAVIGATOR) )
{
    DBG_CTOR( rpt_ONavigator,NULL);

    m_pImpl.reset(new ONavigatorImpl(_rController,this));

    FreeResource();
    m_pImpl->m_pNavigatorTree->Show();
    m_pImpl->m_pNavigatorTree->GrabFocus();
    SetSizePixel(Size(STD_WIN_SIZE_X,STD_WIN_SIZE_Y));
    Show();

}
// -----------------------------------------------------------------------------

//------------------------------------------------------------------------
ONavigator::~ONavigator()
{
    DBG_DTOR( rpt_ONavigator,NULL);
}
//------------------------------------------------------------------------------
void ONavigator::Resize()
{
    FloatingWindow::Resize();

    Size aSize( GetOutputSizePixel() );

    //////////////////////////////////////////////////////////////////////

    // Groesse der form::ListBox anpassen
    Point aLBPos( LISTBOX_BORDER, LISTBOX_BORDER );
    Size aLBSize( aSize );
    aLBSize.Width() -= (2*LISTBOX_BORDER);
    aLBSize.Height() -= (2*LISTBOX_BORDER);

    m_pImpl->m_pNavigatorTree->SetPosSizePixel( aLBPos, aLBSize );
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void ONavigator::GetFocus()
{
    Window::GetFocus();
    if ( m_pImpl->m_pNavigatorTree.get() )
        m_pImpl->m_pNavigatorTree->GrabFocus();
}
// =============================================================================
} // rptui
// =============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
