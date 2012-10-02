/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/templateremoteview.hxx>

#include <comphelper/processfactory.hxx>
#include <officecfg/Office/Common.hxx>
#include <sfx2/templateremoteviewitem.hxx>
#include <sfx2/templateview.hxx>
#include <sfx2/templateviewitem.hxx>
#include <svtools/imagemgr.hxx>
#include <tools/urlobj.hxx>
#include <ucbhelper/content.hxx>
#include <ucbhelper/commandenvironment.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>

using namespace com::sun::star;
using namespace com::sun::star::lang;
using namespace com::sun::star::task;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;

enum
{
    ROW_TITLE = 1,
    ROW_SIZE,
    ROW_DATE_MOD,
    ROW_DATE_CREATE,
    ROW_TARGET_URL,
    ROW_IS_HIDDEN,
    ROW_IS_REMOTE,
    ROW_IS_REMOVEABLE
};

TemplateRemoteView::TemplateRemoteView (Window *pParent, WinBits nWinStyle, bool bDisableTransientChildren)
    : TemplateAbstractView(pParent,nWinStyle,bDisableTransientChildren)
    , mbIsSynced(true)
{
    mpItemView->SetColor(Color(COL_WHITE));
    mpItemView->setChangeNameHdl(LINK(this,TemplateRemoteView,ChangeNameHdl));

    Reference< XComponentContext > xContext = comphelper::getProcessComponentContext();
    Reference< XInteractionHandler >  xGlobalInteractionHandler = Reference< XInteractionHandler >(
        InteractionHandler::createDefault(xContext), UNO_QUERY_THROW );

    m_xCmdEnv = new ucbhelper::CommandEnvironment( xGlobalInteractionHandler, Reference< XProgressHandler >() );
}

TemplateRemoteView::~TemplateRemoteView ()
{
    for (size_t i = 0, n = maRepositories.size(); i < n; ++i)
        delete maRepositories[i];
}

void TemplateRemoteView::Populate()
{
    uno::Reference < uno::XComponentContext > m_context(comphelper::getProcessComponentContext());

    // Load from user settings
    com::sun::star::uno::Sequence<OUString>  aUrls =
            officecfg::Office::Common::Misc::TemplateRepositoryUrls::get(m_context);

    com::sun::star::uno::Sequence<OUString> aNames =
            officecfg::Office::Common::Misc::TemplateRepositoryNames::get(m_context);

    for (sal_Int32 i = 0; i < aUrls.getLength() && i < aNames.getLength(); ++i)
    {
        TemplateRemoteViewItem *pItem = new TemplateRemoteViewItem(*this,this);

        pItem->mnId = i+1;
        pItem->maTitle = aNames[i];
        pItem->setURL(aUrls[i]);

        maRepositories.push_back(pItem);
    }
}

void TemplateRemoteView::reload ()
{
    loadRepository(mpItemView->getId(),true);
}

void TemplateRemoteView::filterTemplatesByApp(const FILTER_APPLICATION &eApp)
{
    if (mpItemView->IsVisible())
        mpItemView->filterItems(ViewFilter_Application(eApp));
}

void TemplateRemoteView::showOverlay (bool bVisible)
{
    mpItemView->Show(bVisible);

    // Clear items is the overlay is closed.
    if (!bVisible)
    {
        mpItemView->Clear();

        setSelectionMode(mbSelectionMode);
    }
}

void TemplateRemoteView::setOverlayChangeNameHdl(const Link &rLink)
{
    maChangeNameHdl = rLink;
}

bool TemplateRemoteView::loadRepository (const sal_uInt16 nRepositoryId, bool bRefresh)
{
    TemplateRemoteViewItem *pItem = NULL;

    for (size_t i = 0, n = maRepositories.size(); i < n; ++i)
    {
        if (maRepositories[i]->mnId == nRepositoryId)
        {
            pItem = maRepositories[i];
            break;
        }
    }

    if (!pItem)
        return false;

    if (!pItem->getTemplates().empty() && !bRefresh)
    {
        mpItemView->InsertItems(pItem->getTemplates());
        return true;
    }

    mpItemView->Clear();
    mpItemView->setId(nRepositoryId);
    mpItemView->setName(pItem->maTitle);

    OUString aURL = static_cast<TemplateRemoteViewItem*>(pItem)->getURL();

    try
    {

        uno::Sequence<OUString> aProps(8);

        aProps[0] = "Title";
        aProps[1] = "Size";
        aProps[2] = "DateModified";
        aProps[3] = "DateCreated";
        aProps[4] = "TargetURL";
        aProps[5] = "IsHidden";
        aProps[6] = "IsRemote";
        aProps[7] = "IsRemoveable";

        ucbhelper::Content aContent(aURL, m_xCmdEnv, comphelper::getProcessComponentContext());

        uno::Reference< XResultSet > xResultSet;
        uno::Reference< XDynamicResultSet > xDynResultSet;

        ucbhelper::ResultSetInclude eInclude = ucbhelper::INCLUDE_DOCUMENTS_ONLY;
        xDynResultSet = aContent.createDynamicCursor( aProps, eInclude );

        if ( xDynResultSet.is() )
            xResultSet = xDynResultSet->getStaticResultSet();

        if ( xResultSet.is() )
        {
            pItem->clearTemplates();

            uno::Reference< XRow > xRow( xResultSet, UNO_QUERY );
            uno::Reference< XContentAccess > xContentAccess( xResultSet, UNO_QUERY );

            util::DateTime aDT;
            std::vector<TemplateItemProperties> aItems;

            sal_uInt16 nIdx = 0;
            while ( xResultSet->next() )
            {
                bool bIsHidden = xRow->getBoolean( ROW_IS_HIDDEN );

                // don't show hidden files or anything besides documents
                if ( !bIsHidden || xRow->wasNull() )
                {
                    aDT = xRow->getTimestamp( ROW_DATE_MOD );
                    bool bContainsDate = !xRow->wasNull();

                    if ( !bContainsDate )
                    {
                        aDT = xRow->getTimestamp( ROW_DATE_CREATE );
                        bContainsDate = !xRow->wasNull();
                    }

                    OUString aContentURL = xContentAccess->queryContentIdentifierString();
                    OUString aTargetURL = xRow->getString( ROW_TARGET_URL );
                    bool bHasTargetURL = !xRow->wasNull() && !aTargetURL.isEmpty();

                    OUString sRealURL = bHasTargetURL ? aTargetURL : aContentURL;

                    TemplateItemProperties aTemplateItem;
                    aTemplateItem.nId = nIdx+1;
                    aTemplateItem.nRegionId = pItem->mnId-1;
                    aTemplateItem.aPath = sRealURL;
                    aTemplateItem.aThumbnail = TemplateAbstractView::fetchThumbnail(sRealURL,
                                                                                    TEMPLATE_THUMBNAIL_MAX_WIDTH,
                                                                                    TEMPLATE_THUMBNAIL_MAX_HEIGHT);
//                    pData->mbIsRemote = xRow->getBoolean( ROW_IS_REMOTE ) && !xRow->wasNull();
//                    pData->mbIsRemoveable = xRow->getBoolean( ROW_IS_REMOVEABLE ) && !xRow->wasNull();
                    aTemplateItem.aName = xRow->getString( ROW_TITLE );
//                    pData->maSize = xRow->getLong( ROW_SIZE );

                    if ( bHasTargetURL &&
                        INetURLObject( aContentURL ).GetProtocol() == INET_PROT_VND_SUN_STAR_HIER )
                    {
                        ucbhelper::Content aCnt( aTargetURL, m_xCmdEnv, comphelper::getProcessComponentContext() );

                        try
                        {
//                            aCnt.getPropertyValue("Size") >>= pData->maSize;
                            aCnt.getPropertyValue("DateModified") >>= aDT;
                        }
                        catch (...)
                        {}
                    }

                    aTemplateItem.aType = SvFileInformationManager::GetFileDescription(INetURLObject(sRealURL));

                    pItem->insertTemplate(aTemplateItem);
                    aItems.push_back(aTemplateItem);
                    ++nIdx;
                }
            }

            mpItemView->InsertItems(aItems);
        }
    }
    catch( ucb::CommandAbortedException& )
    {
    }
    catch( uno::RuntimeException& )
    {
    }
    catch( uno::Exception& )
    {
    }

    return true;
}

bool TemplateRemoteView::insertRepository(const OUString &rName, const OUString &rURL)
{
    for (size_t i = 0, n = maRepositories.size(); i < n; ++i)
    {
        if (maRepositories[i]->maTitle == rName)
            return false;
    }

    TemplateRemoteViewItem *pItem = new TemplateRemoteViewItem(*this,this);

    pItem->mnId = maRepositories.size()+1;
    pItem->maTitle = rName;
    pItem->setURL(rURL);

    maRepositories.push_back(pItem);

    mbIsSynced = false;
    return true;
}

bool TemplateRemoteView::deleteRepository(const sal_uInt16 nRepositoryId)
{
    bool bRet = false;

    for (size_t i = 0, n = maRepositories.size(); i < n; ++i)
    {
        if (maRepositories[i]->mnId == nRepositoryId)
        {
            delete maRepositories[i];

            maRepositories.erase(maRepositories.begin() + i);
            mbIsSynced = false;
            bRet = true;
            break;
        }
    }

    return bRet;
}

void TemplateRemoteView::syncRepositories() const
{
    if (!mbIsSynced)
    {
        uno::Reference < uno::XComponentContext > pContext(comphelper::getProcessComponentContext());
        boost::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create(pContext));

        size_t nSize = maRepositories.size();
        uno::Sequence<OUString> aUrls(nSize);
        uno::Sequence<OUString> aNames(nSize);

        for(size_t i = 0; i < nSize; ++i)
        {
            aUrls[i] = maRepositories[i]->getURL();
            aNames[i] = maRepositories[i]->maTitle;
        }

        officecfg::Office::Common::Misc::TemplateRepositoryUrls::set(aUrls, batch, pContext);
        officecfg::Office::Common::Misc::TemplateRepositoryNames::set(aNames, batch, pContext);
        batch->commit();
    }
}

IMPL_LINK (TemplateRemoteView, ChangeNameHdl, TemplateView*, pView)
{
    bool bRet = false;

    // check if there isnt another repository with the same name.
    for (size_t i = 0, n = maRepositories.size(); i < n; ++i)
    {
        if (maRepositories[i]->mnId == pView->getId())
        {
            maRepositories[i]->maTitle = pView->getName();

            bRet = true;
            mbIsSynced = false;
            maChangeNameHdl.Call(this);
            break;
        }
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
