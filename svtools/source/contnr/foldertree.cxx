/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svtools/foldertree.hxx>

#include "contentenumeration.hxx"

FolderTree::FolderTree( vcl::Window* pParent, WinBits nBits )
    : SvTreeListBox( pParent, nBits | WB_SORT | WB_TABSTOP )
    , m_aFolderImage( SvtResId( IMG_SVT_FOLDER ) )
    , m_aFolderExpandedImage( SvtResId( IMG_SVT_FOLDER_OPEN ) )
{
    Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    Reference< XInteractionHandler > xInteractionHandler(
                InteractionHandler::createWithParent( xContext, 0 ), UNO_QUERY_THROW );
    m_xEnv = new ::ucbhelper::CommandEnvironment( xInteractionHandler, Reference< XProgressHandler >() );

    SetDefaultCollapsedEntryBmp( m_aFolderImage );
    SetDefaultExpandedEntryBmp( m_aFolderExpandedImage );
}

void FolderTree::RequestingChildren( SvTreeListEntry* pEntry )
{
    FillTreeEntry( pEntry );
}

void FolderTree::FillTreeEntry( SvTreeListEntry* pEntry )
{
    if( pEntry )
    {
        while (SvTreeListEntry* pChild = FirstChild(pEntry))
        {
            GetModel()->Remove(pChild);
        }

        ::std::vector< SortingData_Impl* > aContent;

        ::rtl::Reference< ::svt::FileViewContentEnumerator >
            xContentEnumerator(new FileViewContentEnumerator(
            m_xEnv, aContent, m_aMutex, NULL));

        OUString* pURL = static_cast< OUString* >( pEntry->GetUserData() );

        if( pURL )
        {
            FolderDescriptor aFolder( *pURL );

            EnumerationResult eResult =
                xContentEnumerator->enumerateFolderContentSync( aFolder, m_aBlackList );

            if ( SUCCESS == eResult )
            {
                for( std::vector<SortingData_Impl *>::size_type i = 0; i < aContent.size(); i++ )
                {
                    if( aContent[i]->mbIsFolder )
                    {
                        SvTreeListEntry* pNewEntry = InsertEntry( aContent[i]->GetTitle(), pEntry, true );

                        OUString* sData = new OUString( aContent[i]->maTargetURL );
                        pNewEntry->SetUserData( static_cast< void* >( sData ) );
                    }
                }
            }
        }
    }
}

void FolderTree::SetTreePath( OUString const & sUrl )
{
    INetURLObject aUrl( sUrl );
    aUrl.setFinalSlash();

    OUString sPath = aUrl.GetURLPath( INetURLObject::DECODE_WITH_CHARSET );

    SvTreeListEntry* pEntry = First();
    bool end = false;

    while( pEntry && !end )
    {
        if( pEntry->GetUserData() )
        {
            OUString sNodeUrl = *static_cast< OUString* >( pEntry->GetUserData() );

            INetURLObject aUrlObj( sNodeUrl );
            aUrlObj.setFinalSlash();

            sNodeUrl = aUrlObj.GetURLPath( INetURLObject::DECODE_WITH_CHARSET );

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
                pEntry = NextSibling( pEntry );
            }
        }
        else
            break;
    }
}

void FolderTree::SetBlackList( const ::com::sun::star::uno::Sequence< OUString >& rBlackList )
{
    m_aBlackList = rBlackList;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
