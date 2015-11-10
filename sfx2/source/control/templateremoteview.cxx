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
#include <sfx2/templaterepository.hxx>
#include <sfx2/templateviewitem.hxx>
#include <svtools/imagemgr.hxx>
#include <tools/urlobj.hxx>
#include <ucbhelper/content.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include <vcl/builderfactory.hxx>

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
    ROW_IS_REMOVABLE
};

TemplateRemoteView::TemplateRemoteView (vcl::Window *pParent, WinBits nWinStyle, bool bDisableTransientChildren)
    : TemplateAbstractView(pParent,nWinStyle,bDisableTransientChildren)
{
    Reference< XComponentContext > xContext = comphelper::getProcessComponentContext();
    Reference< XInteractionHandler > xGlobalInteractionHandler(
        InteractionHandler::createWithParent(xContext, nullptr), UNO_QUERY_THROW );

    m_xCmdEnv = new ucbhelper::CommandEnvironment( xGlobalInteractionHandler, Reference< XProgressHandler >() );
}

VCL_BUILDER_DECL_FACTORY(TemplateRemoteView)
{
    (void)rMap;
    rRet = VclPtr<TemplateRemoteView>::Create(pParent, WB_VSCROLL, false);
}

void TemplateRemoteView::showRootRegion()
{
    //TODO:
}

void TemplateRemoteView::showRegion(ThumbnailViewItem * /*pItem*/)
{
    //TODO:
}

bool TemplateRemoteView::loadRepository (TemplateRepository* pItem, bool bRefresh)
{
    if (!pItem)
        return false;

    if (!pItem->getTemplates().empty() && !bRefresh)
    {
        insertItems(pItem->getTemplates());
        return true;
    }

    mnCurRegionId = pItem->mnId;
    maCurRegionName = pItem->maTitle;
    maFTName->SetText(maCurRegionName);

    OUString aURL = pItem->getURL();

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

        ucbhelper::ResultSetInclude eInclude = ucbhelper::INCLUDE_FOLDERS_AND_DOCUMENTS;
        xDynResultSet = aContent.createDynamicCursor( aProps, eInclude );

        if ( xDynResultSet.is() )
            xResultSet = xDynResultSet->getStaticResultSet();

        if ( xResultSet.is() )
        {
            pItem->clearTemplates();

            uno::Reference< XRow > xRow( xResultSet, UNO_QUERY );
            uno::Reference< XContentAccess > xContentAccess( xResultSet, UNO_QUERY );

            std::vector<TemplateItemProperties> aItems;

            sal_uInt16 nIdx = 0;
            while ( xResultSet->next() )
            {
                bool bIsHidden = xRow->getBoolean( ROW_IS_HIDDEN );

                // don't show hidden files or anything besides documents
                if ( !bIsHidden || xRow->wasNull() )
                {
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
                    aTemplateItem.aName = xRow->getString( ROW_TITLE );

                    pItem->insertTemplate(aTemplateItem);
                    aItems.push_back(aTemplateItem);
                    ++nIdx;
                }
            }

            insertItems(aItems);
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

sal_uInt16 TemplateRemoteView::createRegion(const OUString &/*rName*/)
{
    // TODO: Create new folder in current remote repository
    return 0;
}

bool TemplateRemoteView::isNestedRegionAllowed() const
{
    return true;
}

bool TemplateRemoteView::isImportAllowed() const
{
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
