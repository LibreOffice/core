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
{
    mpItemView->SetColor(Color(COL_WHITE));

    Reference< XComponentContext > xContext = comphelper::getProcessComponentContext();
    Reference< XInteractionHandler > xGlobalInteractionHandler(
        InteractionHandler::createWithParent(xContext, 0), UNO_QUERY_THROW );

    m_xCmdEnv = new ucbhelper::CommandEnvironment( xGlobalInteractionHandler, Reference< XProgressHandler >() );
}

TemplateRemoteView::~TemplateRemoteView ()
{
}

bool TemplateRemoteView::loadRepository (TemplateRepository* pItem, bool bRefresh)
{
    if (!pItem)
        return false;

    if (!pItem->getTemplates().empty() && !bRefresh)
    {
        mpItemView->InsertItems(pItem->getTemplates());
        return true;
    }

    mpItemView->Clear();
    mpItemView->setId(pItem->mnId);
    mpItemView->setName(pItem->maTitle);

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
                    aTemplateItem.aName = xRow->getString( ROW_TITLE );

                    if ( bHasTargetURL &&
                        INetURLObject( aContentURL ).GetProtocol() == INET_PROT_VND_SUN_STAR_HIER )
                    {
                        ucbhelper::Content aCnt( aTargetURL, m_xCmdEnv, comphelper::getProcessComponentContext() );

                        try
                        {
                            aCnt.getPropertyValue("DateModified") >>= aDT;
                        }
                        catch (...)
                        {}
                    }

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
