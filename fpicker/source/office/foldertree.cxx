/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/processfactory.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/urlobj.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include <com/sun/star/task/InteractionHandler.hpp>
#include "contentenumeration.hxx"
#include "foldertree.hxx"
#include <bitmaps.hlst>

using namespace ::com::sun::star::task;

FolderTree::FolderTree(std::unique_ptr<weld::TreeView> xTreeView, weld::Window* pTopLevel)
    : m_xTreeView(std::move(xTreeView))
    , m_xScratchIter(m_xTreeView->make_iterator())
    , m_pTopLevel(pTopLevel)
{
    m_xTreeView->set_size_request(m_xTreeView->get_approximate_digit_width() * 24,
                                  m_xTreeView->get_height_rows(7));

    Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    Reference< XInteractionHandler > xInteractionHandler(
                InteractionHandler::createWithParent(xContext, pTopLevel->GetXWindow()), UNO_QUERY_THROW);
    m_xEnv = new ::ucbhelper::CommandEnvironment( xInteractionHandler, Reference< XProgressHandler >() );

    m_xTreeView->connect_expanding(LINK(this, FolderTree, RequestingChildrenHdl));
}

IMPL_LINK(FolderTree, RequestingChildrenHdl, const weld::TreeIter&, rEntry, bool)
{
    weld::WaitObject aWait(m_pTopLevel);

    FillTreeEntry(rEntry);

    return true;
}

void FolderTree::InsertRootEntry(const OUString& rId, const OUString& rRootLabel)
{
    m_xTreeView->insert(nullptr, -1, &rRootLabel, &rId, nullptr, nullptr,
                        true, m_xScratchIter.get());
    m_xTreeView->set_image(*m_xScratchIter, RID_BMP_FOLDER);
    m_xTreeView->set_cursor(*m_xScratchIter);
}

void FolderTree::FillTreeEntry(const weld::TreeIter& rEntry)
{
    OUString sURL = m_xTreeView->get_id(rEntry);
    OUString sFolderImage(RID_BMP_FOLDER);

    if (m_sLastUpdatedDir != sURL)
    {
        while (m_xTreeView->iter_has_child(rEntry))
        {
            std::unique_ptr<weld::TreeIter> xChild(m_xTreeView->make_iterator(&rEntry));
            (void)m_xTreeView->iter_children(*xChild);
            m_xTreeView->remove(*xChild);
        }

        ::std::vector< std::unique_ptr<SortingData_Impl> > aContent;

        ::rtl::Reference< ::svt::FileViewContentEnumerator >
            xContentEnumerator(new FileViewContentEnumerator(
            m_xEnv, aContent, m_aMutex));

        FolderDescriptor aFolder(sURL);

        EnumerationResult eResult =
            xContentEnumerator->enumerateFolderContentSync( aFolder, m_aDenyList );

        if (EnumerationResult::SUCCESS == eResult)
        {
            for(const auto & i : aContent)
            {
                if (!i->mbIsFolder)
                    continue;
                m_xTreeView->insert(&rEntry, -1, &i->GetTitle(), &i->maTargetURL,
                    nullptr, nullptr, true, m_xScratchIter.get());
                m_xTreeView->set_image(*m_xScratchIter, sFolderImage);
            }
        }
    }
    else
    {
        // this dir was updated recently
        // next time read this remote folder
        m_sLastUpdatedDir.clear();
    }
}

void FolderTree::FillTreeEntry( const OUString & rUrl, const ::std::vector< std::pair< OUString, OUString > >& rFolders )
{
    SetTreePath(rUrl);

    std::unique_ptr<weld::TreeIter> xParent(m_xTreeView->make_iterator());
    bool bParent = m_xTreeView->get_cursor(xParent.get());

    if (!bParent || m_xTreeView->get_row_expanded(*xParent))
        return;

    OUString sFolderImage(RID_BMP_FOLDER);
    while (m_xTreeView->iter_has_child(*xParent))
    {
        std::unique_ptr<weld::TreeIter> xChild(m_xTreeView->make_iterator(xParent.get()));
        (void)m_xTreeView->iter_children(*xChild);
        m_xTreeView->remove(*xChild);
    }

    for (auto const& folder : rFolders)
    {
        m_xTreeView->insert(xParent.get(), -1, &folder.first, &folder.second,
            nullptr, nullptr, true, m_xScratchIter.get());
        m_xTreeView->set_image(*m_xScratchIter, sFolderImage);
    }

    m_sLastUpdatedDir = rUrl;
    m_xTreeView->expand_row(*xParent);
}

void FolderTree::SetTreePath( OUString const & sUrl )
{
    INetURLObject aUrl( sUrl );
    aUrl.setFinalSlash();

    OUString sPath = aUrl.GetURLPath( INetURLObject::DecodeMechanism::WithCharset );

    std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());
    bool bEntry = m_xTreeView->get_iter_first(*xEntry);
    bool bEnd = false;

    while (bEntry && !bEnd)
    {
        if (!m_xTreeView->get_id(*xEntry).isEmpty())
        {
            OUString sNodeUrl = m_xTreeView->get_id(*xEntry);

            INetURLObject aUrlObj( sNodeUrl );
            aUrlObj.setFinalSlash();

            sNodeUrl = aUrlObj.GetURLPath( INetURLObject::DecodeMechanism::WithCharset );

            if( sPath == sNodeUrl )
            {
                m_xTreeView->select(*xEntry);
                bEnd = true;
            }
            else if( sPath.startsWith( sNodeUrl ) )
            {
                if (!m_xTreeView->get_row_expanded(*xEntry))
                    m_xTreeView->expand_row(*xEntry);

                bEntry = m_xTreeView->iter_children(*xEntry);
            }
            else
            {
                bEntry = m_xTreeView->iter_next_sibling(*xEntry);
            }
        }
        else
            break;
    }
}

void FolderTree::SetDenyList( const css::uno::Sequence< OUString >& rDenyList )
{
    m_aDenyList = rDenyList;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
