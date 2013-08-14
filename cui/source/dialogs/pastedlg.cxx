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

#include <com/sun/star/embed/Aspects.hpp>

#include <pastedlg.hxx>
#include <svtools/svmedit.hxx>
#include <svtools/insdlg.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/msgbox.hxx>
#include <sot/formats.hxx>
#include <sot/stg.hxx>
#include <svtools/sores.hxx>
#include <vcl/svapp.hxx>

#include <dialmgr.hxx>

SvPasteObjectDialog::SvPasteObjectDialog( Window* pParent )
    : ModalDialog(pParent, "PasteSpecialDialog", "cui/ui/pastespecial.ui")
{
    get(m_pFtObjectSource, "source");
    get(m_pLbInsertList, "list");
    get(m_pOKButton, "ok");

    m_pLbInsertList->SetDropDownLineCount(8);
    m_pLbInsertList->set_width_request(m_pLbInsertList->approximate_char_width() * 32);
    m_pOKButton->Disable();

    ObjectLB().SetSelectHdl( LINK( this, SvPasteObjectDialog, SelectHdl ) );
    ObjectLB().SetDoubleClickHdl( LINK( this, SvPasteObjectDialog, DoubleClickHdl ) );
}

void SvPasteObjectDialog::SelectObject()
{
    if (m_pLbInsertList->GetEntryCount())
    {
        m_pLbInsertList->SelectEntryPos(0);
        SelectHdl(m_pLbInsertList);
    }
}

IMPL_LINK( SvPasteObjectDialog, SelectHdl, ListBox *, pListBox )
{
    (void)pListBox;

    if ( !m_pOKButton->IsEnabled() )
        m_pOKButton->Enable();
    return 0;
}

IMPL_LINK_INLINE_START( SvPasteObjectDialog, DoubleClickHdl, ListBox *, pListBox )
{
    (void)pListBox;

    EndDialog( RET_OK );
    return 0;
}
IMPL_LINK_INLINE_END( SvPasteObjectDialog, DoubleClickHdl, ListBox *, pListBox )

SvPasteObjectDialog::~SvPasteObjectDialog()
{
}

/*************************************************************************
|*    SvPasteObjectDialog::Insert()
*************************************************************************/
void SvPasteObjectDialog::Insert( SotFormatStringId nFormat, const String& rFormatName )
{
    aSupplementMap.insert( ::std::make_pair( nFormat, rFormatName ) );
}

sal_uLong SvPasteObjectDialog::GetFormat( const TransferableDataHelper& rHelper,
                                      const DataFlavorExVector* pFormats,
                                      const TransferableObjectDescriptor* )
{
    //TODO/LATER: why is the Descriptor never used?!
    TransferableObjectDescriptor aDesc;
    if( rHelper.HasFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR ) )
        ((TransferableDataHelper&)rHelper).GetTransferableObjectDescriptor(
                                SOT_FORMATSTR_ID_OBJECTDESCRIPTOR, aDesc );
    if ( !pFormats )
        pFormats = &rHelper.GetDataFlavorExVector();

    // create and fill dialog box
    OUString aSourceName, aTypeName;
    sal_uLong nSelFormat = 0;
    SvGlobalName aEmptyNm;

    ObjectLB().SetUpdateMode( sal_False );

    DataFlavorExVector::iterator aIter( ((DataFlavorExVector&)*pFormats).begin() ),
                                 aEnd( ((DataFlavorExVector&)*pFormats).end() );
    while( aIter != aEnd )
    {
        ::com::sun::star::datatransfer::DataFlavor aFlavor( *aIter );
        SotFormatStringId nFormat = (*aIter++).mnSotId;

        ::std::map< SotFormatStringId, OUString >::iterator itName =
            aSupplementMap.find( nFormat );

        // if there is an "Embed Source" or and "Embedded Object" on the
        // Clipboard we read the Description and the Source of this object
        // from an accompanied "Object Descriptor" format on the clipboard
        // Remember: these formats mostly appear together on the clipboard
        OUString aName;
        const OUString* pName = NULL;
        if ( itName == aSupplementMap.end() )
        {
            SvPasteObjectHelper::GetEmbeddedName(rHelper,aName,aSourceName,nFormat);
            if ( !aName.isEmpty() )
                pName = &aName;
        }
        else
        {
            pName = &(itName->second);
        }

        if( pName )
        {
            aName = *pName;

            if( SOT_FORMATSTR_ID_EMBED_SOURCE == nFormat )
            {
                if( aDesc.maClassName != aEmptyNm )
                {
                    aSourceName = aDesc.maDisplayName;

                    if( aDesc.maClassName == aObjClassName )
                        aName = aObjName;
                    else
                        aName = aTypeName = aDesc.maTypeName;
                }
            }
            else if( SOT_FORMATSTR_ID_LINK_SOURCE == nFormat )
            {
                continue;
            }
            else if( aName.isEmpty() )
                aName = SvPasteObjectHelper::GetSotFormatUIName( nFormat );

            if( LISTBOX_ENTRY_NOTFOUND == ObjectLB().GetEntryPos( aName ) )
                ObjectLB().SetEntryData(
                    ObjectLB().InsertEntry( aName ), (void*) nFormat );
        }
    }

    if( aTypeName.isEmpty() && aSourceName.isEmpty() )
    {
        if( aDesc.maClassName != aEmptyNm )
        {
            aSourceName = aDesc.maDisplayName;
            aTypeName = aDesc.maTypeName;
        }

        if( aTypeName.isEmpty() && aSourceName.isEmpty() )
        {
            ResMgr* pMgr = ResMgr::CreateResMgr( "svt", Application::GetSettings().GetUILanguageTag() );
            // global resource from svtools (former so3 resource)
            if( pMgr )
                aSourceName = String( ResId( STR_UNKNOWN_SOURCE, *pMgr ) );
            delete pMgr;
        }
    }

    ObjectLB().SetUpdateMode( sal_True );
    SelectObject();

    if( !aSourceName.isEmpty() )
    {
        if( !aTypeName.isEmpty() )
            aTypeName += "\n";

        aTypeName += aSourceName;
        aTypeName = convertLineEnd(aTypeName, GetSystemLineEnd());
    }

    ObjectSource().SetText( aTypeName );

    if( Dialog::Execute() == RET_OK )
    {
        nSelFormat  = (sal_uLong)ObjectLB().GetEntryData( ObjectLB().GetSelectEntryPos() );
    }

    return nSelFormat;
}

void SvPasteObjectDialog::SetObjName( const SvGlobalName & rClass, const String & rObjName )
{
    aObjClassName = rClass;
    aObjName = rObjName;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
