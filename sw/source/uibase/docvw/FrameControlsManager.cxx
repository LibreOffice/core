/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <edtwin.hxx>
#include <cntfrm.hxx>
#include <FrameControlsManager.hxx>
#include <HeaderFooterWin.hxx>
#include <PageBreakWin.hxx>
#include <UnfloatTableButton.hxx>
#include <pagefrm.hxx>
#include <flyfrm.hxx>
#include <viewopt.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <OutlineContentVisibilityWin.hxx>
#include <ndtxt.hxx>
#include <IDocumentOutlineNodes.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>

using namespace std;

SwFrameControlsManager::SwFrameControlsManager( SwEditWin* pEditWin ) :
    m_pEditWin( pEditWin ),
    m_aControls( )
{
}

SwFrameControlsManager::~SwFrameControlsManager()
{
}

void SwFrameControlsManager::dispose()
{
    m_aControls.clear();
    m_aTextNodeContentFrameMap.clear();
}

SwFrameControlPtr SwFrameControlsManager::GetControl( FrameControlType eType, const SwFrame* pFrame )
{
    SwFrameControlPtrMap& rControls = m_aControls[eType];

    SwFrameControlPtrMap::iterator aIt = rControls.find(pFrame);

    if (aIt != rControls.end())
        return aIt->second;

    return SwFrameControlPtr();
}

void SwFrameControlsManager::RemoveControls( const SwFrame* pFrame )
{
    for ( auto& rEntry : m_aControls )
    {
        SwFrameControlPtrMap& rMap = rEntry.second;
        rMap.erase(pFrame);
    }
}

void SwFrameControlsManager::RemoveControlsByType( FrameControlType eType, const SwFrame* pFrame )
{
    SwFrameControlPtrMap& rMap = m_aControls[eType];
    rMap.erase(pFrame);
}

void SwFrameControlsManager::HideControls( FrameControlType eType )
{
    for ( const auto& rCtrl : m_aControls[eType] )
        rCtrl.second->ShowAll( false );
}

void SwFrameControlsManager::SetReadonlyControls( bool bReadonly )
{
    for ( auto& rEntry : m_aControls )
        for ( auto& rCtrl : rEntry.second )
            rCtrl.second->SetReadonly( bReadonly );
}

void SwFrameControlsManager::SetHeaderFooterControl( const SwPageFrame* pPageFrame, FrameControlType eType, Point aOffset )
{
    assert( eType == FrameControlType::Header || eType == FrameControlType::Footer );

    // Check if we already have the control
    SwFrameControlPtr pControl;
    const bool bHeader = ( eType == FrameControlType::Header );

    SwFrameControlPtrMap& rControls = m_aControls[eType];

    SwFrameControlPtrMap::iterator lb = rControls.lower_bound(pPageFrame);
    if (lb != rControls.end() && !(rControls.key_comp()(pPageFrame, lb->first)))
        pControl = lb->second;
    else
    {
        SwFrameControlPtr pNewControl =
                std::make_shared<SwFrameControl>( VclPtr<SwHeaderFooterWin>::Create(
                                        m_pEditWin, pPageFrame, bHeader ).get() );
        const SwViewOption* pViewOpt = m_pEditWin->GetView().GetWrtShell().GetViewOptions();
        pNewControl->SetReadonly( pViewOpt->IsReadonly() );
        rControls.insert(lb, make_pair(pPageFrame, pNewControl));
        pControl.swap( pNewControl );
    }

    tools::Rectangle aPageRect = m_pEditWin->LogicToPixel( pPageFrame->getFrameArea().SVRect() );

    SwHeaderFooterWin* pWin = dynamic_cast<SwHeaderFooterWin *>(pControl->GetWindow());
    assert( pWin != nullptr) ;
    assert( pWin->IsHeader() == bHeader );
    pWin->SetOffset( aOffset, aPageRect.Left(), aPageRect.Right() );

    if (!pWin->IsVisible())
        pControl->ShowAll( true );
}

void SwFrameControlsManager::SetPageBreakControl( const SwPageFrame* pPageFrame )
{
    // Check if we already have the control
    SwFrameControlPtr pControl;

    SwFrameControlPtrMap& rControls = m_aControls[FrameControlType::PageBreak];

    SwFrameControlPtrMap::iterator lb = rControls.lower_bound(pPageFrame);
    if (lb != rControls.end() && !(rControls.key_comp()(pPageFrame, lb->first)))
        pControl = lb->second;
    else
    {
        SwFrameControlPtr pNewControl = std::make_shared<SwFrameControl>(
                VclPtr<SwPageBreakWin>::Create( m_pEditWin, pPageFrame ).get() );
        const SwViewOption* pViewOpt = m_pEditWin->GetView().GetWrtShell().GetViewOptions();
        pNewControl->SetReadonly( pViewOpt->IsReadonly() );

        rControls.insert(lb, make_pair(pPageFrame, pNewControl));

        pControl.swap( pNewControl );
    }

    SwPageBreakWin* pWin = dynamic_cast<SwPageBreakWin *>(pControl->GetWindow());
    assert (pWin != nullptr);
    pWin->UpdatePosition();
    if (!pWin->IsVisible())
        pControl->ShowAll( true );
}

void SwFrameControlsManager::SetUnfloatTableButton( const SwFlyFrame* pFlyFrame, bool bShow, Point aTopRightPixel )
{
    if(pFlyFrame == nullptr)
        return;

    // Check if we already have the control
    SwFrameControlPtr pControl;

    SwFrameControlPtrMap& rControls = m_aControls[FrameControlType::FloatingTable];

    SwFrameControlPtrMap::iterator lb = rControls.lower_bound(pFlyFrame);
    if (lb != rControls.end() && !(rControls.key_comp()(pFlyFrame, lb->first)))
        pControl = lb->second;
    else if (!bShow) // Do not create the control when it's not shown
        return;
    else
    {
        SwFrameControlPtr pNewControl = std::make_shared<SwFrameControl>(
                VclPtr<UnfloatTableButton>::Create( m_pEditWin, pFlyFrame ).get() );
        const SwViewOption* pViewOpt = m_pEditWin->GetView().GetWrtShell().GetViewOptions();
        pNewControl->SetReadonly( pViewOpt->IsReadonly() );

        rControls.insert(lb, make_pair(pFlyFrame, pNewControl));

        pControl.swap( pNewControl );
    }

    UnfloatTableButton* pButton = dynamic_cast<UnfloatTableButton*>(pControl->GetWindow());
    assert(pButton != nullptr);
    pButton->SetOffset(aTopRightPixel);
    pControl->ShowAll( bShow );
}

SwFrameMenuButtonBase::SwFrameMenuButtonBase(SwEditWin* pEditWin, const SwFrame* pFrame,
                                             const OUString& rUIXMLDescription, const OString& rID)
    : InterimItemWindow(pEditWin, rUIXMLDescription, rID)
    , m_pEditWin(pEditWin)
    , m_pFrame(pFrame)
{
}

void SwFrameControlsManager::SetOutlineContentVisibilityButtons()
{
    // remove entries with outline node keys that are not in the outline nodes list
    IDocumentOutlineNodes::tSortedOutlineNodeList aOutlineNodes;
    m_pEditWin->GetView().GetWrtShell().getIDocumentOutlineNodesAccess()->getOutlineNodes(aOutlineNodes);
    std::map<const SwTextNode*, const SwContentFrame*>::iterator it = m_aTextNodeContentFrameMap.begin();
    while(it != m_aTextNodeContentFrameMap.end())
    {
        const SwNode* pNd = it->first;
        IDocumentOutlineNodes::tSortedOutlineNodeList::iterator i = std::find(aOutlineNodes.begin(), aOutlineNodes.end(), pNd);
        if (i == aOutlineNodes.end())
        {
            RemoveControlsByType(FrameControlType::Outline, it->second);
            it = m_aTextNodeContentFrameMap.erase(it);
        }
        else
            it++;
    }
    for (SwNode* pNd : m_pEditWin->GetView().GetWrtShell().GetNodes().GetOutLineNds())
    {
        bool bOutlineContentVisibleAttr = true;
        pNd->GetTextNode()->GetAttrOutlineContentVisible(bOutlineContentVisibleAttr);
        if (!bOutlineContentVisibleAttr)
            SetOutlineContentVisibilityButton(pNd->GetTextNode());
    }
}

void SwFrameControlsManager::SetOutlineContentVisibilityButton(const SwTextNode* pTextNd)
{
    const SwContentFrame* pContentFrame = pTextNd->getLayoutFrame(nullptr);

    // has node frame changed or been deleted?
    std::map<const SwTextNode*, const SwContentFrame*>::iterator iter = m_aTextNodeContentFrameMap.find(pTextNd);
    if (iter != m_aTextNodeContentFrameMap.end())
    {
        const SwContentFrame* pFrameWas = iter->second;
        if (pContentFrame != pFrameWas)
        {
            // frame does not match frame in map for node
            RemoveControlsByType(FrameControlType::Outline, pFrameWas);
            m_aTextNodeContentFrameMap.erase(iter);
        }
    }
    if (pContentFrame && !pContentFrame->IsInDtor())
    {
        // frame is not being destroyed and isn't in map
        m_aTextNodeContentFrameMap.insert(make_pair(pTextNd, pContentFrame));
    }
    else
    {
        if (pContentFrame)
        {
            // frame is being destroyed
            RemoveControlsByType(FrameControlType::Outline, pContentFrame);
        }
        return;
    }

    // Check if we already have the control
    SwFrameControlPtr pControl;

    SwFrameControlPtrMap& rControls = m_aControls[FrameControlType::Outline];

    SwFrameControlPtrMap::iterator lb = rControls.lower_bound(pContentFrame);
    if (lb != rControls.end() && !(rControls.key_comp()(pContentFrame, lb->first)))
    {
        pControl = lb->second;
    }
    else
    {
        SwFrameControlPtr pNewControl =
                std::make_shared<SwFrameControl>(VclPtr<SwOutlineContentVisibilityWin>::Create(
                                        m_pEditWin, pContentFrame).get());
        rControls.insert(lb, make_pair(pContentFrame, pNewControl));
        pControl.swap(pNewControl);
    }

    SwOutlineContentVisibilityWin* pWin = dynamic_cast<SwOutlineContentVisibilityWin *>(pControl->GetWindow());
    assert(pWin != nullptr) ;
    pWin->Set();

    if (pWin->GetSymbol() == SymbolType::ARROW_RIGHT)
    {
        // show expand button immediately
        pWin->Show();
        /*
           The outline content might be visible here. This happens on document load,
           undo outline moves, and show of outline content that itself has outline nodes
           having outline content visibility attribute false, for example tables and text
           frames containing outline nodes.
        */
        SwOutlineNodes::size_type nPos;
        SwOutlineNodes rOutlineNds = m_pEditWin->GetView().GetWrtShell().GetNodes().GetOutLineNds();
        if (rOutlineNds.Seek_Entry(const_cast<SwTextNode*>(pTextNd), &nPos))
        {
            SwNodeIndex aIdx(*pTextNd, +1);
            // there shouldn't be a layout frame
            // if there is then force visiblity false
            if (!m_pEditWin->GetView().GetWrtShell().GetViewOptions()->IsTreatSubOutlineLevelsAsContent())
            {
                if (!(aIdx.GetNode().IsEndNode() ||
                      (nPos + 1 < rOutlineNds.size() && &aIdx.GetNode() == rOutlineNds[nPos +1]))
                        && aIdx.GetNode().IsContentNode()
                        // this determines if the content is really visible
                        && aIdx.GetNode().GetContentNode()->getLayoutFrame(nullptr))
                {
                    // force outline content visibility false
                    m_pEditWin->GetView().GetWrtShell().ToggleOutlineContentVisibility(nPos, true);
                }
            }
            else if (!aIdx.GetNode().IsEndNode()
                     && aIdx.GetNode().IsContentNode()
                     // this determines if the content is really visible
                     && aIdx.GetNode().GetContentNode()->getLayoutFrame(nullptr))
            {
                // force outline content visibility false
                m_pEditWin->GetView().GetWrtShell().ToggleOutlineContentVisibility(nPos, true);
            }
        }
    }
    else if (!pWin->IsVisible() && pWin->GetSymbol() == SymbolType::ARROW_DOWN)
        pWin->ShowAll(true);
}

const SwPageFrame* SwFrameMenuButtonBase::GetPageFrame() const
{
    if (m_pFrame->IsPageFrame())
        return static_cast<const SwPageFrame*>( m_pFrame );

    if (m_pFrame->IsFlyFrame())
        return static_cast<const SwFlyFrame*>(m_pFrame)->GetAnchorFrame()->FindPageFrame();

    return m_pFrame->FindPageFrame();
}

void SwFrameMenuButtonBase::dispose()
{
    m_pEditWin.clear();
    m_pFrame = nullptr;
    m_xVirDev.disposeAndClear();
    InterimItemWindow::dispose();
}

void SwFrameMenuButtonBase::SetVirDevFont()
{
    // Get the font and configure it
    vcl::Font aFont = Application::GetSettings().GetStyleSettings().GetToolFont();
    if (vcl::Window* pDefaultDevice = dynamic_cast<vcl::Window*>(Application::GetDefaultDevice()))
        pDefaultDevice->SetPointFont(*m_xVirDev, aFont);
}

SwFrameControl::SwFrameControl( const VclPtr<vcl::Window> &pWindow )
{
    assert(static_cast<bool>(pWindow));
    mxWindow.reset( pWindow );
    mpIFace = dynamic_cast<ISwFrameControl *>( pWindow.get() );
}

SwFrameControl::~SwFrameControl()
{
    mpIFace = nullptr;
    mxWindow.disposeAndClear();
}

ISwFrameControl::~ISwFrameControl()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
