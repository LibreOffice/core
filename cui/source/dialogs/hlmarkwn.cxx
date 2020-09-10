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

#include <dialmgr.hxx>
#include <unotools/viewoptions.hxx>
#include <vcl/graph.hxx>

// UNO-Stuff
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/document/XLinkTargetSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/io/IOException.hpp>

#include <toolkit/helper/vclunohelper.hxx>

#include <strings.hrc>
#include <hlmarkwn.hxx>
#include <hltpbase.hxx>
#include <hlmarkwn_def.hxx>

using namespace ::com::sun::star;

namespace {

// Userdata-struct for tree-entries
struct TargetData
{
    OUString aUStrLinkname;
    bool     bIsTarget;

    TargetData (const OUString& aUStrLName, bool bTarget)
        : bIsTarget(bTarget)
    {
        if (bIsTarget)
            aUStrLinkname = aUStrLName;
    }
};

}

//*** Window-Class ***
// Constructor / Destructor
SvxHlinkDlgMarkWnd::SvxHlinkDlgMarkWnd(weld::Window* pParentDialog, SvxHyperlinkTabPageBase *pParentPage)
    : GenericDialogController(pParentDialog, "cui/ui/hyperlinkmarkdialog.ui", "HyperlinkMark")
    , mpParent(pParentPage)
    , mnError(LERR_NOERROR)
    , mxBtApply(m_xBuilder->weld_button("ok"))
    , mxBtClose(m_xBuilder->weld_button("close"))
    , mxLbTree(m_xBuilder->weld_tree_view("TreeListBox"))
    , mxError(m_xBuilder->weld_label("error"))
{
    mxLbTree->set_size_request(mxLbTree->get_approximate_digit_width() * 25,
                               mxLbTree->get_height_rows(12));
    mxBtApply->connect_clicked( LINK ( this, SvxHlinkDlgMarkWnd, ClickApplyHdl_Impl ) );
    mxBtClose->connect_clicked( LINK ( this, SvxHlinkDlgMarkWnd, ClickCloseHdl_Impl ) );
    mxLbTree->connect_row_activated( LINK ( this, SvxHlinkDlgMarkWnd, DoubleClickApplyHdl_Impl ) );
}

SvxHlinkDlgMarkWnd::~SvxHlinkDlgMarkWnd()
{
    ClearTree();
}

void SvxHlinkDlgMarkWnd::ErrorChanged()
{
    if (mnError == LERR_NOENTRIES)
    {
        OUString aStrMessage = CuiResId( RID_SVXSTR_HYPDLG_ERR_LERR_NOENTRIES );
        mxError->set_label(aStrMessage);
        mxError->show();
        mxLbTree->hide();
    }
    else if (mnError == LERR_DOCNOTOPEN)
    {
        OUString aStrMessage = CuiResId( RID_SVXSTR_HYPDLG_ERR_LERR_DOCNOTOPEN );
        mxError->set_label(aStrMessage);
        mxError->show();
        mxLbTree->hide();
    }
    else
    {
        mxLbTree->show();
        mxError->hide();
    }
}

// Set an errorstatus
sal_uInt16 SvxHlinkDlgMarkWnd::SetError( sal_uInt16 nError)
{
    sal_uInt16 nOldError = mnError;
    mnError = nError;

    if( mnError != LERR_NOERROR )
        ClearTree();

    ErrorChanged();

    return nOldError;
}

// Move window
void SvxHlinkDlgMarkWnd::MoveTo(const Point& rNewPos)
{
    m_xDialog->window_move(rNewPos.X(), rNewPos.Y());
}

namespace
{
    void SelectPath(weld::TreeIter* pEntry, weld::TreeView& rLbTree,
        std::deque<OUString> &rLastSelectedPath)
    {
        OUString sTitle(rLastSelectedPath.front());
        rLastSelectedPath.pop_front();
        if (sTitle.isEmpty())
            return;
        while (pEntry)
        {
            if (sTitle == rLbTree.get_text(*pEntry))
            {
                rLbTree.select(*pEntry);
                rLbTree.scroll_to_row(*pEntry);
                if (!rLastSelectedPath.empty())
                {
                    rLbTree.expand_row(*pEntry);
                    if (!rLbTree.iter_children(*pEntry))
                        pEntry = nullptr;
                    SelectPath(pEntry, rLbTree, rLastSelectedPath);
                }
                break;
            }
            if (!rLbTree.iter_next_sibling(*pEntry))
                pEntry = nullptr;
        }
    }
}

#define TG_SETTING_MANAGER  "TargetInDocument"
#define TG_SETTING_LASTMARK "LastSelectedMark"
#define TG_SETTING_LASTPATH "LastSelectedPath"

void SvxHlinkDlgMarkWnd::RestoreLastSelection()
{
    bool bSelectedEntry = false;

    OUString sLastSelectedMark;
    std::deque<OUString> aLastSelectedPath;
    SvtViewOptions aViewSettings( EViewType::Dialog, TG_SETTING_MANAGER );
    if (aViewSettings.Exists())
    {
        //Maybe we might want to have some sort of mru list and keep a mapping
        //per document, rather than the current reuse of "the last thing
        //selected, regardless of the document"
        aViewSettings.GetUserItem(TG_SETTING_LASTMARK) >>= sLastSelectedMark;
        uno::Sequence<OUString> aTmp;
        aViewSettings.GetUserItem(TG_SETTING_LASTPATH) >>= aTmp;
        aLastSelectedPath = comphelper::sequenceToContainer< std::deque<OUString> >(aTmp);
    }
    //fallback to previous entry selected the last time we executed this dialog.
    //First see if the exact mark exists and re-use that
    if (!sLastSelectedMark.isEmpty())
        bSelectedEntry = SelectEntry(sLastSelectedMark);
    //Otherwise just select the closest path available
    //now to what was available at dialog close time
    if (!bSelectedEntry && !aLastSelectedPath.empty())
    {
        std::deque<OUString> aTmpSelectedPath(aLastSelectedPath);
        std::unique_ptr<weld::TreeIter> xEntry(mxLbTree->make_iterator());
        if (!mxLbTree->get_iter_first(*xEntry))
            xEntry.reset();
        SelectPath(xEntry.get(), *mxLbTree, aTmpSelectedPath);
    }
}

// Interface to refresh tree
void SvxHlinkDlgMarkWnd::RefreshTree (const OUString& aStrURL)
{
    OUString aUStrURL;

    weld::WaitObject aWait(m_xDialog.get());

    ClearTree();

    sal_Int32 nPos = aStrURL.indexOf('#');

    if (nPos != 0)
        aUStrURL = aStrURL;

    if (!RefreshFromDoc(aUStrURL))
        ErrorChanged();

    bool bSelectedEntry = false;

    if ( nPos != -1 )
    {
        OUString aStrMark = aStrURL.copy(nPos+1);
        bSelectedEntry = SelectEntry(aStrMark);
    }

    if (!bSelectedEntry)
        RestoreLastSelection();
}

// get links from document
bool SvxHlinkDlgMarkWnd::RefreshFromDoc(const OUString& aURL)
{
    mnError = LERR_NOERROR;

    uno::Reference< frame::XDesktop2 > xDesktop = frame::Desktop::create( ::comphelper::getProcessComponentContext() );
    uno::Reference< lang::XComponent > xComp;

    if( !aURL.isEmpty() )
    {
        // load from url
        if( xDesktop.is() )
        {
            try
            {
                uno::Sequence< beans::PropertyValue > aArg(1);
                aArg.getArray()[0].Name = "Hidden";
                aArg.getArray()[0].Value <<= true;
                xComp = xDesktop->loadComponentFromURL( aURL, "_blank", 0, aArg );
            }
            catch( const io::IOException& )
            {

            }
            catch( const lang::IllegalArgumentException& )
            {

            }
        }
    }
    else
    {
        // the component with user focus ( current document )
        xComp = xDesktop->getCurrentComponent();
    }

    if( xComp.is() )
    {
        uno::Reference< document::XLinkTargetSupplier > xLTS( xComp, uno::UNO_QUERY );

        if( xLTS.is() )
        {
            if( FillTree( xLTS->getLinks() ) == 0 )
                mnError = LERR_NOENTRIES;
        }
        else
            mnError = LERR_DOCNOTOPEN;

        if ( !aURL.isEmpty() )
            xComp->dispose();
    }
    else
    {
        if( !aURL.isEmpty() )
            mnError=LERR_DOCNOTOPEN;
    }
    return (mnError==0);
}

// Fill Tree-Control
int SvxHlinkDlgMarkWnd::FillTree( const uno::Reference< container::XNameAccess >& xLinks, const weld::TreeIter* pParentEntry )
{
    int nEntries=0;
    const uno::Sequence< OUString > aNames( xLinks->getElementNames() );
    const sal_Int32 nLinks = aNames.getLength();
    const OUString* pNames = aNames.getConstArray();

    const OUString aProp_LinkDisplayName( "LinkDisplayName" );
    const OUString aProp_LinkTarget( "com.sun.star.document.LinkTarget" );
    const OUString aProp_LinkDisplayBitmap( "LinkDisplayBitmap" );
    for( sal_Int32 i = 0; i < nLinks; i++ )
    {
        uno::Any aAny;
        OUString aLink( *pNames++ );

        bool bError = false;
        try
        {
            aAny = xLinks->getByName( aLink );
        }
        catch(const uno::Exception&)
        {
            // if the name of the target was invalid (like empty headings)
            // no object can be provided
            bError = true;
        }
        if(bError)
            continue;

        uno::Reference< beans::XPropertySet > xTarget;

        if( aAny >>= xTarget )
        {
            try
            {
                // get name to display
                aAny = xTarget->getPropertyValue( aProp_LinkDisplayName );
                OUString aDisplayName;
                aAny >>= aDisplayName;
                OUString aStrDisplayname ( aDisplayName );

                // is it a target ?
                uno::Reference< lang::XServiceInfo > xSI( xTarget, uno::UNO_QUERY );
                bool bIsTarget = xSI->supportsService( aProp_LinkTarget );

                // create userdata
                TargetData *pData = new TargetData ( aLink, bIsTarget );
                OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pData)));

                std::unique_ptr<weld::TreeIter> xEntry(mxLbTree->make_iterator());
                mxLbTree->insert(pParentEntry, -1, &aStrDisplayname, &sId, nullptr, nullptr, false, xEntry.get());

                try
                {
                    // get bitmap for the tree-entry
                    uno::Reference< awt::XBitmap >
                        aXBitmap( xTarget->getPropertyValue( aProp_LinkDisplayBitmap ), uno::UNO_QUERY );
                    if (aXBitmap.is())
                    {
                        Graphic aBmp(Graphic(VCLUnoHelper::GetBitmap(aXBitmap)));
                        // insert Displayname into treelist with bitmaps
                        mxLbTree->set_image(*xEntry, aBmp.GetXGraphic(), -1);
                    }
                }
                catch(const css::uno::Exception&)
                {
                }

                nEntries++;

                uno::Reference< document::XLinkTargetSupplier > xLTS( xTarget, uno::UNO_QUERY );
                if( xLTS.is() )
                    nEntries += FillTree( xLTS->getLinks(), xEntry.get() );
            }
            catch(const css::uno::Exception&)
            {
            }
        }
    }

    return nEntries;
}

// Clear Tree
void SvxHlinkDlgMarkWnd::ClearTree()
{
    std::unique_ptr<weld::TreeIter> xEntry = mxLbTree->make_iterator();
    bool bEntry = mxLbTree->get_iter_first(*xEntry);

    while (bEntry)
    {
        TargetData* pUserData = reinterpret_cast<TargetData*>(mxLbTree->get_id(*xEntry).toInt64());
        delete pUserData;

        bEntry = mxLbTree->iter_next(*xEntry);
    }

    mxLbTree->clear();
}

// Find Entry for String
std::unique_ptr<weld::TreeIter> SvxHlinkDlgMarkWnd::FindEntry (const OUString& aStrName)
{
    bool bFound=false;
    std::unique_ptr<weld::TreeIter> xEntry = mxLbTree->make_iterator();
    bool bEntry = mxLbTree->get_iter_first(*xEntry);

    while (bEntry && !bFound)
    {
        TargetData* pUserData = reinterpret_cast<TargetData*>(mxLbTree->get_id(*xEntry).toInt64());
        if (aStrName == pUserData->aUStrLinkname)
            bFound = true;
        else
            bEntry = mxLbTree->iter_next(*xEntry);
    }

    if (!bFound)
        xEntry.reset();

    return xEntry;
}

// Select Entry
bool SvxHlinkDlgMarkWnd::SelectEntry(const OUString& aStrMark)
{
    std::unique_ptr<weld::TreeIter> xEntry = FindEntry(aStrMark);
    if (!xEntry)
        return false;
    mxLbTree->select(*xEntry);
    mxLbTree->scroll_to_row(*xEntry);
    return true;
}

// Click on Apply-Button / Double-click on item in tree
IMPL_LINK_NOARG(SvxHlinkDlgMarkWnd, DoubleClickApplyHdl_Impl, weld::TreeView&, bool)
{
    ClickApplyHdl_Impl(*mxBtApply);
    return true;
}

IMPL_LINK_NOARG(SvxHlinkDlgMarkWnd, ClickApplyHdl_Impl, weld::Button&, void)
{
    std::unique_ptr<weld::TreeIter> xEntry(mxLbTree->make_iterator());
    bool bEntry = mxLbTree->get_cursor(xEntry.get());
    if (bEntry)
    {
        TargetData* pData = reinterpret_cast<TargetData*>(mxLbTree->get_id(*xEntry).toInt64());
        if (pData->bIsTarget)
        {
            mpParent->SetMarkStr(pData->aUStrLinkname);
        }
    }
}

// Click on Close-Button
IMPL_LINK_NOARG(SvxHlinkDlgMarkWnd, ClickCloseHdl_Impl, weld::Button&, void)
{
    std::unique_ptr<weld::TreeIter> xEntry(mxLbTree->make_iterator());
    bool bEntry = mxLbTree->get_cursor(xEntry.get());
    if (bEntry)
    {
        TargetData* pUserData = reinterpret_cast<TargetData*>(mxLbTree->get_id(*xEntry).toInt64());
        OUString sLastSelectedMark = pUserData->aUStrLinkname;

        std::deque<OUString> aLastSelectedPath;
        //If the bottommost entry is expanded but nothing
        //underneath it is selected leave a dummy entry
        if (mxLbTree->get_row_expanded(*xEntry))
            aLastSelectedPath.push_front(OUString());
        while (bEntry)
        {
            aLastSelectedPath.push_front(mxLbTree->get_text(*xEntry));
            bEntry = mxLbTree->iter_parent(*xEntry);
        }

        uno::Sequence< beans::NamedValue > aSettings
        {
            { TG_SETTING_LASTMARK, css::uno::Any(sLastSelectedMark) },
            { TG_SETTING_LASTPATH, css::uno::Any(comphelper::containerToSequence(aLastSelectedPath)) }
        };

        // write
        SvtViewOptions aViewSettings( EViewType::Dialog, TG_SETTING_MANAGER );
        aViewSettings.SetUserData( aSettings );
    }

    m_xDialog->response(RET_CANCEL);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
