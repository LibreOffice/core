/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/processfactory.hxx>
#include <svtools/foldertree.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/urlobj.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include <vcl/dialog.hxx>
#include <vcl/treelistentry.hxx>
#include <vcl/ptrstyle.hxx>
#include <com/sun/star/task/InteractionHandler.hpp>
#include "contentenumeration.hxx"
#include <bitmaps.hlst>

using namespace ::com::sun::star::task;

FolderTree::FolderTree( vcl::Window* pParent, WinBits nBits )
    : SvTreeListBox( pParent, nBits | WB_SORT | WB_TABSTOP )
{
    Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    Reference< XInteractionHandler > xInteractionHandler(
                InteractionHandler::createWithParent(xContext, VCLUnoHelper::GetInterface(GetParentDialog())), UNO_QUERY_THROW );
    m_xEnv = new ::ucbhelper::CommandEnvironment( xInteractionHandler, Reference< XProgressHandler >() );

    Image aFolderImage(StockImage::Yes, RID_BMP_FOLDER);
    Image aFolderExpandedImage(StockImage::Yes, RID_BMP_FOLDER_OPEN);
    SetDefaultCollapsedEntryBmp( aFolderImage );
    SetDefaultExpandedEntryBmp( aFolderExpandedImage );
}

void FolderTree::RequestingChildren( SvTreeListEntry* pEntry )
{
    EnableChildPointerOverwrite( true );
    SetPointer( PointerStyle::Wait );
    Invalidate(InvalidateFlags::Update);

    FillTreeEntry( pEntry );

    SetPointer( PointerStyle::Arrow );
    EnableChildPointerOverwrite( false );
}

void FolderTree::FillTreeEntry( SvTreeListEntry* pEntry )
{
    if( !pEntry )
        return;

    OUString* pURL = static_cast< OUString* >( pEntry->GetUserData() );

    if( pURL && m_sLastUpdatedDir != *pURL )
    {
        while (SvTreeListEntry* pChild = FirstChild(pEntry))
        {
            GetModel()->Remove(pChild);
        }

        ::std::vector< std::unique_ptr<SortingData_Impl> > aContent;

        ::rtl::Reference< ::svt::FileViewContentEnumerator >
            xContentEnumerator(new FileViewContentEnumerator(
            m_xEnv, aContent, m_aMutex));

        FolderDescriptor aFolder( *pURL );

        EnumerationResult eResult =
            xContentEnumerator->enumerateFolderContentSync( aFolder, m_aBlackList );

        if ( EnumerationResult::SUCCESS == eResult )
        {
            for(auto & i : aContent)
            {
                if( i->mbIsFolder )
                {
                    SvTreeListEntry* pNewEntry = InsertEntry( i->GetTitle(), pEntry, true );

                    OUString* sData = new OUString( i->maTargetURL );
                    pNewEntry->SetUserData( static_cast< void* >( sData ) );
                }
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
    SetTreePath( rUrl );

    SvTreeListEntry* pParent = GetCurEntry();

    if( !(pParent && !IsExpanded( pParent )) )
        return;

    while (SvTreeListEntry* pChild = FirstChild(pParent))
    {
        GetModel()->Remove(pChild);
    }

    for (auto const& folder : rFolders)
    {
        SvTreeListEntry* pNewEntry = InsertEntry( folder.first, pParent, true  );
        OUString* sData = new OUString( folder.second );
        pNewEntry->SetUserData( static_cast< void* >( sData ) );
    }

    m_sLastUpdatedDir = rUrl;
    Expand( pParent );
}

void FolderTree::SetTreePath( OUString const & sUrl )
{
    INetURLObject aUrl( sUrl );
    aUrl.setFinalSlash();

    OUString sPath = aUrl.GetURLPath( INetURLObject::DecodeMechanism::WithCharset );

    SvTreeListEntry* pEntry = First();
    bool end = false;

    while( pEntry && !end )
    {
        if( pEntry->GetUserData() )
        {
            OUString sNodeUrl = *static_cast< OUString* >( pEntry->GetUserData() );

            INetURLObject aUrlObj( sNodeUrl );
            aUrlObj.setFinalSlash();

            sNodeUrl = aUrlObj.GetURLPath( INetURLObject::DecodeMechanism::WithCharset );

            if( sPath == sNodeUrl )
            {
                Select( pEntry );
                end = true;
            }
            else if( sPath.startsWith( sNodeUrl ) )
            {
                if( !IsExpanded( pEntry ) )
                    Expand( pEntry );

                pEntry = FirstChild( pEntry );
            }
            else
            {
                pEntry = pEntry->NextSibling();
            }
        }
        else
            break;
    }
}

void FolderTree::SetBlackList( const css::uno::Sequence< OUString >& rBlackList )
{
    m_aBlackList = rBlackList;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
