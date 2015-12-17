/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "notebookbarwindow.hxx"

#include "notebookbartab.hxx"
#include <vcl/layout.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/types.hxx>
#include <unotools/confignode.hxx>

namespace
{

OUString getString(utl::OConfigurationNode const & aNode, const char* pNodeName)
{
    return comphelper::getString(aNode.getNodeValue(pNodeName));
}
sal_Int32 getInt32(utl::OConfigurationNode const & aNode, const char* pNodeName)
{
    return comphelper::getINT32(aNode.getNodeValue(pNodeName));
}
bool getBool(utl::OConfigurationNode const & aNode, const char* pNodeName)
{
    return comphelper::getBOOL(aNode.getNodeValue(pNodeName));
}

} //end anonymous namespace

NotebookBarWindow::NotebookBarWindow(Window* pParent)
    : TabControl(pParent)
{
    SetStyle(GetStyle() | WB_DIALOGCONTROL);
    insertTabs();
}

NotebookBarWindow::~NotebookBarWindow()
{
    disposeOnce();
}

void NotebookBarWindow::dispose()
{
    TabControl::dispose();
}

Size NotebookBarWindow::GetOptimalSize() const
{
    if (isLayoutEnabled(this))
        return VclContainer::getLayoutRequisition(*GetWindow(GetWindowType::FirstChild));

    return TabControl::GetOptimalSize();
}

void NotebookBarWindow::setPosSizePixel(long nX, long nY, long nWidth, long nHeight, PosSizeFlags nFlags)
{
    bool bCanHandleSmallerWidth = false;
    bool bCanHandleSmallerHeight = false;

    bool bIsLayoutEnabled = isLayoutEnabled(this);
    Window *pChild = GetWindow(GetWindowType::FirstChild);

    if (bIsLayoutEnabled && pChild->GetType() == WINDOW_SCROLLWINDOW)
    {
        WinBits nStyle = pChild->GetStyle();
        if (nStyle & (WB_AUTOHSCROLL | WB_HSCROLL))
            bCanHandleSmallerWidth = true;
        if (nStyle & (WB_AUTOVSCROLL | WB_VSCROLL))
            bCanHandleSmallerHeight = true;
    }

    Size aSize(GetOptimalSize());
    if (!bCanHandleSmallerWidth)
        nWidth = std::max(nWidth, aSize.Width());
    if (!bCanHandleSmallerHeight)
        nHeight = std::max(nHeight, aSize.Height());

    TabControl::setPosSizePixel(nX, nY, nWidth, nHeight, nFlags);

    if (bIsLayoutEnabled && (nFlags & PosSizeFlags::Size))
        VclContainer::setLayoutAllocation(*pChild, Point(0, 0), Size(nWidth, nHeight));
}

void NotebookBarWindow::insertTabs()
{
    const utl::OConfigurationTreeRoot aTabRootNode(
        comphelper::getProcessComponentContext(),
        OUString("org.openoffice.Office.UI.Notebookbar/Content/TabList"),
        false);

    if (!aTabRootNode.isValid())
        return;

    const css::uno::Sequence<OUString> aTabNodeNames(aTabRootNode.getNodeNames());
    const sal_Int32 nCount(aTabNodeNames.getLength());
    for (sal_Int32 nReadIndex(0); nReadIndex<nCount; ++nReadIndex)
    {
        const utl::OConfigurationNode aTabNode(aTabRootNode.openNode(aTabNodeNames[nReadIndex]));
        if (!aTabNode.isValid())
            continue;

        /*NotebookBarTab& rTab(&this);
        rTab.msTitle = getString(aTabNode, "Title");
        rTab.msId = getString(aTabNode, "Id");
        rTab.msHelpURL = getString(aTabNode, "HelpURL");
        rTab.mnOrderIndex = getInt32(aTabNode, "OrderIndex");
        rTab.mbExperimental = getBool(aTabNode, "IsExperimental");

        this->InsertPage(rTab.mnOrderIndex, rTab.msTitle, rTab.mnOrderIndex);
        this->SetTabPage(rTab.mnOrderIndex, rTab);*/

        // TODO
        //ReadContextList(aTabNode, rTab.maContextList, OUString());
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
