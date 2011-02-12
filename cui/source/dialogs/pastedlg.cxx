/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
#include "svuidlg.hrc"
#include <sot/formats.hxx>
#include <sot/stg.hxx>
#include <svtools/sores.hxx>
#include <vcl/svapp.hxx>

#include <dialmgr.hxx>

SvPasteObjectDialog::SvPasteObjectDialog( Window* pParent )

    : ModalDialog( pParent, CUI_RES( MD_PASTE_OBJECT ) ),
    aFtSource( this, CUI_RES( FT_SOURCE ) ),
    aFtObjectSource( this, CUI_RES( FT_OBJECT_SOURCE ) ),
    aRbPaste( this, CUI_RES( RB_PASTE ) ),
    aRbPasteLink( this, CUI_RES( RB_PASTE_LINK ) ),
    aLbInsertList( this, CUI_RES( LB_INSERT_LIST ) ),
    aCbDisplayAsIcon( this, CUI_RES( CB_DISPLAY_AS_ICON ) ),
    aPbChangeIcon( this, CUI_RES( PB_CHANGE_ICON ) ),
    aFlChoice( this, CUI_RES( FL_CHOICE ) ),
    aOKButton1( this, CUI_RES( 1 ) ),
    aCancelButton1( this, CUI_RES( 1 ) ),
    aHelpButton1( this, CUI_RES( 1 ) ),
    aSObject( CUI_RES( S_OBJECT ) )
{
    FreeResource();
    SetHelpId( HID_PASTE_DLG );
    SetUniqueId( HID_PASTE_DLG );

    Font aFont = aFtObjectSource.GetFont();
    aFont.SetWeight( WEIGHT_LIGHT );
    aFtObjectSource.SetFont( aFont );
    aOKButton1.Disable();

    ObjectLB().SetSelectHdl( LINK( this, SvPasteObjectDialog, SelectHdl ) );
    ObjectLB().SetDoubleClickHdl( LINK( this, SvPasteObjectDialog, DoubleClickHdl ) );
    SetDefault();
}

void SvPasteObjectDialog::SelectObject()
{
    if ( aLbInsertList.GetEntryCount() &&
         !aRbPaste.IsVisible() && !aRbPasteLink.IsVisible() )
    {
        aLbInsertList.SelectEntryPos(0);
        SelectHdl( &aLbInsertList );
    }
}

IMPL_LINK( SvPasteObjectDialog, SelectHdl, ListBox *, pListBox )
{
    (void)pListBox;

    if ( !aOKButton1.IsEnabled() )
        aOKButton1.Enable();
    return 0;
}

IMPL_LINK_INLINE_START( SvPasteObjectDialog, DoubleClickHdl, ListBox *, pListBox )
{
    (void)pListBox;

    EndDialog( RET_OK );
    return 0;
}
IMPL_LINK_INLINE_END( SvPasteObjectDialog, DoubleClickHdl, ListBox *, pListBox )

void SvPasteObjectDialog::SetDefault()
{
    bLink   = FALSE;
    nAspect = (USHORT)::com::sun::star::embed::Aspects::MSOLE_CONTENT;
}

SvPasteObjectDialog::~SvPasteObjectDialog()
{
    void * pStr = aSupplementTable.First();
    while( pStr )
    {
        delete (String *)pStr;
        pStr = aSupplementTable.Next();
    }
}

/*************************************************************************
|*    SvPasteObjectDialog::Insert()
*************************************************************************/
void SvPasteObjectDialog::Insert( SotFormatStringId nFormat, const String& rFormatName )
{
    String * pStr = new String( rFormatName );
    if( !aSupplementTable.Insert( nFormat, pStr ) )
        delete pStr;
}

ULONG SvPasteObjectDialog::GetFormat( const TransferableDataHelper& rHelper,
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

    //Dialogbox erzeugen und fuellen
    String aSourceName, aTypeName;
    ULONG nSelFormat = 0;
    SvGlobalName aEmptyNm;

    ObjectLB().SetUpdateMode( FALSE );

    DataFlavorExVector::iterator aIter( ((DataFlavorExVector&)*pFormats).begin() ),
                                 aEnd( ((DataFlavorExVector&)*pFormats).end() );
    while( aIter != aEnd )
    {
        ::com::sun::star::datatransfer::DataFlavor aFlavor( *aIter );
        SotFormatStringId nFormat = (*aIter++).mnSotId;

        String* pName = (String*) aSupplementTable.Get( nFormat );
        String aName;

        // if there is an "Embed Source" or and "Embedded Object" on the
        // Clipboard we read the Description and the Source of this object
        // from an accompanied "Object Descriptor" format on the clipboard
        // Remember: these formats mostly appear together on the clipboard
        if ( !pName )
        {
            SvPasteObjectHelper::GetEmbeddedName(rHelper,aName,aSourceName,nFormat);
            if ( aName.Len() )
                pName = &aName;
        }


        if( pName )
        {
            aName = *pName;

            if( SOT_FORMATSTR_ID_EMBED_SOURCE == nFormat )
            {
                if( aDesc.maClassName != aEmptyNm )                                {
                    aSourceName = aDesc.maDisplayName;

                    if( aDesc.maClassName == aObjClassName )
                        aName = aObjName;
                    else
                        aName = aTypeName = aDesc.maTypeName;
                }
            }
            else if( SOT_FORMATSTR_ID_LINK_SOURCE == nFormat )
            {
                PasteLink().Enable();
                continue;
            }
            else if( !aName.Len() )
                aName = SvPasteObjectHelper::GetSotFormatUIName( nFormat );

            if( LISTBOX_ENTRY_NOTFOUND == ObjectLB().GetEntryPos( aName ) )
                ObjectLB().SetEntryData(
                    ObjectLB().InsertEntry( aName ), (void*) nFormat );
        }
    }

    if( !aTypeName.Len() && !aSourceName.Len() )
    {
        if( aDesc.maClassName != aEmptyNm )
        {
            aSourceName = aDesc.maDisplayName;
            aTypeName = aDesc.maTypeName;
        }

        if( !aTypeName.Len() && !aSourceName.Len() )
        {
            com::sun::star::lang::Locale aLocale = Application::GetSettings().GetUILocale();
            ResMgr* pMgr = ResMgr::CreateResMgr( "svt", aLocale );
            // global resource from svtools (former so3 resource)
            if( pMgr )
                aSourceName = String( ResId( STR_UNKNOWN_SOURCE, *pMgr ) );
            delete pMgr;
        }
    }

    ObjectLB().SetUpdateMode( TRUE );
    SelectObject();

    if( aSourceName.Len() )
    {
        if( aTypeName.Len() )
            aTypeName += '\n';

        aTypeName += aSourceName;
        aTypeName.ConvertLineEnd();
    }

    ObjectSource().SetText( aTypeName );

    SetDefault();

    if( Dialog::Execute() == RET_OK )
    {
        bLink = PasteLink().IsChecked();

        if( AsIconBox().IsChecked() )
            nAspect = (USHORT)com::sun::star::embed::Aspects::MSOLE_ICON;

        nSelFormat  = (ULONG)ObjectLB().GetEntryData( ObjectLB().GetSelectEntryPos() );
    }

    return nSelFormat;
}

void SvPasteObjectDialog::SetObjName( const SvGlobalName & rClass, const String & rObjName )
{
    aObjClassName = rClass;
    aObjName = rObjName;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
