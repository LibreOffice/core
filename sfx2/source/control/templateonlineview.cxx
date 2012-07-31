/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/templateonlineview.hxx>

#include <comphelper/processfactory.hxx>
#include <officecfg/Office/Common.hxx>
#include <sfx2/templateonlineviewitem.hxx>
#include <sfx2/templateview.hxx>
#include <sfx2/templateviewitem.hxx>
#include <svtools/imagemgr.hxx>
#include <tools/urlobj.hxx>
#include <ucbhelper/content.hxx>
#include <ucbhelper/commandenvironment.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>

#define ITEM_MAX_WIDTH 192
#define ITEM_MAX_HEIGHT 192
#define ITEM_PADDING 5
#define ITEM_SPACE 20
#define THUMBNAIL_MAX_HEIGHT 128 - 2*ITEM_PADDING
#define THUMBNAIL_MAX_WIDTH ITEM_MAX_WIDTH - 2*ITEM_PADDING

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

TemplateOnlineView::TemplateOnlineView (Window *pParent, WinBits nWinStyle, bool bDisableTransientChildren)
    : TemplateAbstractView(pParent,nWinStyle,bDisableTransientChildren)
{
    mpItemView->SetColor(Color(COL_WHITE));

    Reference< XMultiServiceFactory > xFactory = comphelper::getProcessServiceFactory();
    Reference< XInteractionHandler >  xGlobalInteractionHandler = Reference< XInteractionHandler >(
        xFactory->createInstance("com.sun.star.task.InteractionHandler" ), UNO_QUERY );

    m_xCmdEnv = new ucbhelper::CommandEnvironment( xGlobalInteractionHandler, Reference< XProgressHandler >() );
}

TemplateOnlineView::~TemplateOnlineView ()
{
    for (size_t i = 0, n = maRepositories.size(); i < n; ++i)
        delete maRepositories[i];
}

void TemplateOnlineView::Populate()
{
    uno::Reference < uno::XComponentContext > m_context(comphelper::getProcessComponentContext());

    // Load from user settings
    com::sun::star::uno::Sequence< rtl::OUString >  aUrls =
            officecfg::Office::Common::Misc::FilePickerPlacesUrls::get(m_context);

    com::sun::star::uno::Sequence< rtl::OUString > aNames =
            officecfg::Office::Common::Misc::FilePickerPlacesNames::get(m_context);

    for (sal_Int32 i = 0; i < aUrls.getLength() && i < aNames.getLength(); ++i)
    {
        TemplateOnlineViewItem *pItem = new TemplateOnlineViewItem(*this,this);

        pItem->mnId = i+1;
        pItem->maTitle = aNames[i];
        pItem->setURL(aUrls[i]);

        maRepositories.push_back(pItem);
    }
}

void TemplateOnlineView::filterTemplatesByApp(const FILTER_APPLICATION &eApp)
{
    if (mpItemView->IsVisible())
        mpItemView->filterItems(ViewFilter_Application(eApp));
}

void TemplateOnlineView::showOverlay (bool bVisible)
{
    mbActive = !bVisible;
    mpItemView->Show(bVisible);

    // Clear items is the overlay is closed.
    if (!bVisible)
    {
        mpItemView->Clear();

        setSelectionMode(mbSelectionMode);
    }
}

bool TemplateOnlineView::loadRepository (const sal_uInt16 nRepositoryId)
{
    TemplateOnlineViewItem *pItem = NULL;

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

    rtl::OUString aURL = static_cast<TemplateOnlineViewItem*>(pItem)->getURL();

    try
    {

        uno::Sequence< rtl::OUString > aProps(8);

        aProps[0] = "Title";
        aProps[1] = "Size";
        aProps[2] = "DateModified";
        aProps[3] = "DateCreated";
        aProps[4] = "TargetURL";
        aProps[5] = "IsHidden";
        aProps[6] = "IsRemote";
        aProps[7] = "IsRemoveable";

        ucbhelper::Content aContent(aURL,m_xCmdEnv);

        uno::Reference< XResultSet > xResultSet;
        uno::Reference< XDynamicResultSet > xDynResultSet;

        ucbhelper::ResultSetInclude eInclude = ucbhelper::INCLUDE_DOCUMENTS_ONLY;
        xDynResultSet = aContent.createDynamicCursor( aProps, eInclude );

        if ( xDynResultSet.is() )
            xResultSet = xDynResultSet->getStaticResultSet();

        if ( xResultSet.is() )
        {
            mpItemView->Clear();

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

                    rtl::OUString aContentURL = xContentAccess->queryContentIdentifierString();
                    rtl::OUString aTargetURL = xRow->getString( ROW_TARGET_URL );
                    bool bHasTargetURL = !xRow->wasNull() && !aTargetURL.isEmpty();

                    rtl::OUString sRealURL = bHasTargetURL ? aTargetURL : aContentURL;

                    TemplateItemProperties aTemplateItem;
                    aTemplateItem.nId = nIdx+1;
                    aTemplateItem.nRegionId = pItem->mnId-1;
                    aTemplateItem.aPath = sRealURL;
                    aTemplateItem.aThumbnail = TemplateAbstractView::fetchThumbnail(sRealURL,
                                                                                    THUMBNAIL_MAX_WIDTH,
                                                                                    THUMBNAIL_MAX_HEIGHT);
//                    pData->mbIsRemote = xRow->getBoolean( ROW_IS_REMOTE ) && !xRow->wasNull();
//                    pData->mbIsRemoveable = xRow->getBoolean( ROW_IS_REMOVEABLE ) && !xRow->wasNull();
                    aTemplateItem.aName = xRow->getString( ROW_TITLE );
//                    pData->maSize = xRow->getLong( ROW_SIZE );

                    if ( bHasTargetURL &&
                        INetURLObject( aContentURL ).GetProtocol() == INET_PROT_VND_SUN_STAR_HIER )
                    {
                        ucbhelper::Content aCnt( aTargetURL, m_xCmdEnv );

                        try
                        {
//                            aCnt.getPropertyValue("Size") >>= pData->maSize;
                            aCnt.getPropertyValue("DateModified") >>= aDT;
                        }
                        catch (...)
                        {}
                    }

                    aTemplateItem.aType = SvFileInformationManager::GetFileDescription(INetURLObject(sRealURL));

                    aItems.push_back(aTemplateItem);
                    ++nIdx;
                }
            }

            mpItemView->setName(pItem->maTitle);
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

void TemplateOnlineView::setItemDimensions(long ItemWidth, long ThumbnailHeight, long DisplayHeight, int itemPadding)
{
    ThumbnailView::setItemDimensions(ItemWidth,ThumbnailHeight,DisplayHeight,itemPadding);

    mpItemView->setItemDimensions(ItemWidth,ThumbnailHeight,DisplayHeight,itemPadding);
}

void TemplateOnlineView::Resize()
{
    mpItemView->SetSizePixel(GetSizePixel());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
