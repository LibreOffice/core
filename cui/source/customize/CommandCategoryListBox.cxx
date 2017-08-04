/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/frame/XDispatchInformationProvider.hpp>
#include <vcl/builderfactory.hxx>

#include "CommandCategoryListBox.hxx"

CommandCategoryListBox::CommandCategoryListBox(vcl::Window* pParent, WinBits nStyle)
    : ListBox( pParent, nStyle)
{
    SetDropDownLineCount(25);
}

VCL_BUILDER_FACTORY(CommandCategoryListBox);

void CommandCategoryListBox::FillCategoryEntries()
{
    //Temporary dummy data
    SetUpdateMode(false);
    Clear();
    for ( sal_uInt32 i = 0; i < 10; ++i )
    {
        OUString aStr = "Entry " + OUString::number(i);
        sal_Int32 nPos = InsertEntry( aStr );
        SetEntryData( nPos, reinterpret_cast<void*>((sal_uLong)i) );
    }

    SetUpdateMode(true);
    SelectEntryPos(0);

    /*try
    {
        css::uno::Reference< css::frame::XDispatchInformationProvider > xProvider(xFrame, css::uno::UNO_QUERY_THROW);
        css::uno::Sequence< sal_Int16 > lGroups = xProvider->getSupportedCommandGroups();
        sal_Int32                       c1      = lGroups.getLength();
        sal_Int32                       i1      = 0;

        if ( c1 )
        {
            // Add All Commands category
            sal_Int32 entryPos = InsertEntry( CuiResId(RID_SVXSTR_ALLFUNCTIONS) );
            ImplEntryType* pEntry = GetEntryPtr( entryPos );
            aArr.push_back( o3tl::make_unique<SfxGroupInfo_Impl>( SfxCfgKind::GROUP_ALLFUNCTIONS, 0 ) );
            pEntry->SetUserData(aArr.back().get());
            pEntry->SetEntryData( sal_Int32  nPos, void* pNewData )
        }

        for (i1=0; i1<c1; ++i1)
        {
            sal_Int16&      rGroupID   = lGroups[i1];
            OUString sGroupID   = OUString::number(rGroupID);
            OUString sGroupName ;

            try
            {
                m_xModuleCategoryInfo->getByName(sGroupID) >>= sGroupName;
                if (sGroupName.isEmpty())
                    continue;
            }
            catch(const css::container::NoSuchElementException&)
            { continue; }

            SvTreeListEntry*    pEntry = InsertEntry(sGroupName);
            aArr.push_back( o3tl::make_unique<SfxGroupInfo_Impl>( SfxCfgKind::GROUP_FUNCTION, rGroupID ) );
            pEntry->SetUserData(aArr.back().get());
        }
    }
    catch(const css::uno::RuntimeException&)
    { throw; }
    catch(const css::uno::Exception&)
    {}

    SetUpdateMode( true );*/
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
