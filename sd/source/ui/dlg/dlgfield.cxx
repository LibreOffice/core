/*************************************************************************
 *
 *  $RCSfile: dlgfield.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 10:57:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#pragma hdrstop

#include <svx/editeng.hxx>

#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif
#define ITEMID_FIELD    EE_FEATURE_FIELD
#include <svx/flditem.hxx>
#ifndef _ZFORLIST_HXX
#include <svtools/zforlist.hxx>
#endif
#ifndef _SVX_ADRITEM_HXX
#include <svx/adritem.hxx>
#endif
#ifndef _SFX_OBJSH_HXX
#include <sfx2/objsh.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif
#define ITEMID_LANGUAGE EE_CHAR_LANGUAGE
#ifndef _SVX_LANGITEM_HXX
#include <svx/langitem.hxx>
#endif

#include "strings.hrc"
#include "dlgfield.hrc"
#include "sdattr.hxx"
#include "sdresid.hxx"
#include "sdmod.hxx"
#include "dlgfield.hxx"
#include "drawdoc.hxx"
#include "docshell.hxx"

/*************************************************************************
|*
|* Dialog zum Bearbeiten von Feldbefehlen
|*
\************************************************************************/

SdModifyFieldDlg::SdModifyFieldDlg( Window* pWindow, const SvxFieldData* pInField, const SfxItemSet& rSet ) :
    ModalDialog ( pWindow, SdResId( DLG_FIELD_MODIFY ) ),
    aRbtFix     ( this, SdResId( RBT_FIX ) ),
    aRbtVar     ( this, SdResId( RBT_VAR ) ),
    aGrpType    ( this, SdResId( GRP_TYPE ) ),
    maFtLanguage( this, SdResId( FT_LANGUAGE ) ),
    maLbLanguage( this, SdResId( LB_LANGUAGE ) ),
    aFtFormat   ( this, SdResId( FT_FORMAT ) ),
    aLbFormat   ( this, SdResId( LB_FORMAT ) ),
    aBtnOK      ( this, SdResId( BTN_OK ) ),
    aBtnCancel  ( this, SdResId( BTN_CANCEL ) ),
    aBtnHelp    ( this, SdResId( BTN_HELP ) ),
    pField      ( pInField ),
    maInputSet  ( rSet )
{
    FreeResource();

    maLbLanguage.SetLanguageList( LANG_LIST_ALL|LANG_LIST_ONLY_KNOWN, false );
    maLbLanguage.SetSelectHdl( LINK( this, SdModifyFieldDlg, LanguageChangeHdl ) );
    FillControls();
}

/*************************************************************************
|*
|* Gibt das neue Feld zurueck, gehoert dem Caller.
|* Liefert NULL, wenn sich nichts geaendert hat.
|*
\************************************************************************/

SvxFieldData* SdModifyFieldDlg::GetField()
{
    SvxFieldData* pNewField = NULL;

    if( aRbtFix.IsChecked() != aRbtFix.GetSavedValue() ||
        aRbtVar.IsChecked() != aRbtVar.GetSavedValue() ||
        aLbFormat.GetSelectEntryPos() != aLbFormat.GetSavedValue() )
    {
        if( pField->ISA( SvxDateField ) )
        {
            const SvxDateField* pDateField = (const SvxDateField*) pField;
            SvxDateType   eType;
            SvxDateFormat eFormat;

            if( aRbtFix.IsChecked() )
                eType = SVXDATETYPE_FIX;
            else
                eType = SVXDATETYPE_VAR;

            eFormat = (SvxDateFormat) ( aLbFormat.GetSelectEntryPos() + 2 );

            pNewField = new SvxDateField( *pDateField );
            ( (SvxDateField*) pNewField )->SetType( eType );
            ( (SvxDateField*) pNewField )->SetFormat( eFormat );
        }
        else if( pField->ISA( SvxExtTimeField ) )
        {
            const SvxExtTimeField* pTimeField = (const SvxExtTimeField*) pField;
            SvxTimeType   eType;
            SvxTimeFormat eFormat;

            if( aRbtFix.IsChecked() )
                eType = SVXTIMETYPE_FIX;
            else
                eType = SVXTIMETYPE_VAR;

            eFormat = (SvxTimeFormat) ( aLbFormat.GetSelectEntryPos() + 2 );

            pNewField = new SvxExtTimeField( *pTimeField );
            ( (SvxExtTimeField*) pNewField )->SetType( eType );
            ( (SvxExtTimeField*) pNewField )->SetFormat( eFormat );
        }
        else if( pField->ISA( SvxExtFileField ) )
        {
            const SvxExtFileField* pFileField = (const SvxExtFileField*) pField;
            SvxFileType   eType;
            SvxFileFormat eFormat;

            if( aRbtFix.IsChecked() )
                eType = SVXFILETYPE_FIX;
            else
                eType = SVXFILETYPE_VAR;

            eFormat = (SvxFileFormat) ( aLbFormat.GetSelectEntryPos() );

            SdDrawDocShell* pDocSh = PTR_CAST( SdDrawDocShell,
                                               SfxObjectShell::Current() );

            if( pDocSh )
            {
                SvxExtFileField aFileField( *pFileField );

                String aName;
                if( pDocSh->HasName() )
                    aName = pDocSh->GetMedium()->GetName();

                // #91225# Get current filename, not the one stored in the old field
                pNewField = new SvxExtFileField( aName );
                ( (SvxExtFileField*) pNewField )->SetType( eType );
                ( (SvxExtFileField*) pNewField )->SetFormat( eFormat );
            }
        }
        else if( pField->ISA( SvxAuthorField ) )
        {
            const SvxAuthorField* pAuthorField = (const SvxAuthorField*) pField;
            SvxAuthorType   eType;
            SvxAuthorFormat eFormat;

            if( aRbtFix.IsChecked() )
                eType = SVXAUTHORTYPE_FIX;
            else
                eType = SVXAUTHORTYPE_VAR;

            eFormat = (SvxAuthorFormat) ( aLbFormat.GetSelectEntryPos() );

            // #91225# Get current state of address, not the old one
            pNewField = new SvxAuthorField( SvxAddressItem() );
            ( (SvxAuthorField*) pNewField )->SetType( eType );
            ( (SvxAuthorField*) pNewField )->SetFormat( eFormat );
        }
    }

    return( pNewField );
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

void SdModifyFieldDlg::FillFormatList()
{
    LanguageType eLangType = maLbLanguage.GetSelectLanguage();

    aLbFormat.Clear();

    if( pField->ISA( SvxDateField ) )
    {
        const SvxDateField* pDateField = (const SvxDateField*) pField;
        SvxDateField aDateField( *pDateField );

        //SVXDATEFORMAT_APPDEFAULT,     // Wird nicht benutzt
        //SVXDATEFORMAT_SYSTEM,         // Wird nicht benutzt
        aLbFormat.InsertEntry( String( SdResId( STR_STANDARD_SMALL ) ) );
        aLbFormat.InsertEntry( String( SdResId( STR_STANDARD_BIG ) ) );

        SvNumberFormatter* pNumberFormatter = SD_MOD()->GetNumberFormatter();
        aDateField.SetFormat( SVXDATEFORMAT_A );    // 13.02.96
        aLbFormat.InsertEntry( aDateField.GetFormatted( *pNumberFormatter, eLangType ) );
        aDateField.SetFormat( SVXDATEFORMAT_B );    // 13.02.1996
        aLbFormat.InsertEntry( aDateField.GetFormatted( *pNumberFormatter, eLangType ) );
        aDateField.SetFormat( SVXDATEFORMAT_C );    // 13.Feb 1996
        aLbFormat.InsertEntry( aDateField.GetFormatted( *pNumberFormatter, eLangType ) );
        aDateField.SetFormat( SVXDATEFORMAT_D );    // 13.Februar 1996
        aLbFormat.InsertEntry( aDateField.GetFormatted( *pNumberFormatter, eLangType ) );
        aDateField.SetFormat( SVXDATEFORMAT_E );    // Die, 13.Februar 1996
        aLbFormat.InsertEntry( aDateField.GetFormatted( *pNumberFormatter, eLangType ) );
        aDateField.SetFormat( SVXDATEFORMAT_F );    // Dienstag, 13.Februar 1996
        aLbFormat.InsertEntry( aDateField.GetFormatted( *pNumberFormatter, eLangType ) );

        aLbFormat.SelectEntryPos( (USHORT) ( pDateField->GetFormat() - 2 ) );
    }
    else if( pField->ISA( SvxExtTimeField ) )
    {
        const SvxExtTimeField* pTimeField = (const SvxExtTimeField*) pField;
        SvxExtTimeField aTimeField( *pTimeField );

        //SVXTIMEFORMAT_APPDEFAULT,     // Wird nicht benutzt
        //SVXTIMEFORMAT_SYSTEM,         // Wird nicht benutzt
        aLbFormat.InsertEntry( String( SdResId( STR_STANDARD_NORMAL ) ) );

        SvNumberFormatter* pNumberFormatter = SD_MOD()->GetNumberFormatter();
        aTimeField.SetFormat( SVXTIMEFORMAT_24_HM );    // 13:49
        aLbFormat.InsertEntry( aTimeField.GetFormatted( *pNumberFormatter, eLangType ) );
        aTimeField.SetFormat( SVXTIMEFORMAT_24_HMS );   // 13:49:38
        aLbFormat.InsertEntry( aTimeField.GetFormatted( *pNumberFormatter, eLangType ) );
        aTimeField.SetFormat( SVXTIMEFORMAT_24_HMSH );  // 13:49:38.78
        aLbFormat.InsertEntry( aTimeField.GetFormatted( *pNumberFormatter, eLangType ) );
        aTimeField.SetFormat( SVXTIMEFORMAT_12_HM );    // 01:49
        aLbFormat.InsertEntry( aTimeField.GetFormatted( *pNumberFormatter, eLangType ) );
        aTimeField.SetFormat( SVXTIMEFORMAT_12_HMS );   // 01:49:38
        aLbFormat.InsertEntry( aTimeField.GetFormatted( *pNumberFormatter, eLangType ) );
        aTimeField.SetFormat( SVXTIMEFORMAT_12_HMSH );  // 01:49:38.78
        aLbFormat.InsertEntry( aTimeField.GetFormatted( *pNumberFormatter, eLangType ) );
        //SVXTIMEFORMAT_AM_HM,  // 01:49 PM
        //SVXTIMEFORMAT_AM_HMS, // 01:49:38 PM
        //SVXTIMEFORMAT_AM_HMSH // 01:49:38.78 PM

        aLbFormat.SelectEntryPos( (USHORT) ( pTimeField->GetFormat() - 2 ) );
    }
    else if( pField->ISA( SvxExtFileField ) )
    {
        const SvxExtFileField* pFileField = (const SvxExtFileField*) pField;
        SvxExtFileField aFileField( *pFileField );

        aLbFormat.InsertEntry( String( SdResId( STR_FILEFORMAT_NAME_EXT ) ) );
        aLbFormat.InsertEntry( String( SdResId( STR_FILEFORMAT_FULLPATH ) ) );
        aLbFormat.InsertEntry( String( SdResId( STR_FILEFORMAT_PATH ) ) );
        aLbFormat.InsertEntry( String( SdResId( STR_FILEFORMAT_NAME ) ) );

        aLbFormat.SelectEntryPos( (USHORT) ( pFileField->GetFormat() ) );
    }
    else if( pField->ISA( SvxAuthorField ) )
    {
        const SvxAuthorField* pAuthorField = (const SvxAuthorField*) pField;
        SvxAuthorField aAuthorField( *pAuthorField );

        for( USHORT i = 0; i < 4; i++ )
        {
            aAuthorField.SetFormat( (SvxAuthorFormat) i );
            aLbFormat.InsertEntry( aAuthorField.GetFormatted() );
        }

        aLbFormat.SelectEntryPos( (USHORT) ( pAuthorField->GetFormat() ) );

    }


}

void SdModifyFieldDlg::FillControls()
{
    aLbFormat.Clear();

    if( pField->ISA( SvxDateField ) )
    {
        const SvxDateField* pDateField = (const SvxDateField*) pField;
        SvxDateField aDateField( *pDateField );

        if( pDateField->GetType() == SVXDATETYPE_FIX )
            aRbtFix.Check();
        else
            aRbtVar.Check();
    }
    else if( pField->ISA( SvxExtTimeField ) )
    {
        const SvxExtTimeField* pTimeField = (const SvxExtTimeField*) pField;
        SvxExtTimeField aTimeField( *pTimeField );

        if( pTimeField->GetType() == SVXTIMETYPE_FIX )
            aRbtFix.Check();
        else
            aRbtVar.Check();
    }
    else if( pField->ISA( SvxExtFileField ) )
    {
        const SvxExtFileField* pFileField = (const SvxExtFileField*) pField;
        SvxExtFileField aFileField( *pFileField );

        if( pFileField->GetType() == SVXFILETYPE_FIX )
            aRbtFix.Check();
        else
            aRbtVar.Check();
    }
    else if( pField->ISA( SvxAuthorField ) )
    {
        const SvxAuthorField* pAuthorField = (const SvxAuthorField*) pField;
        SvxAuthorField aAuthorField( *pAuthorField );

        if( pAuthorField->GetType() == SVXAUTHORTYPE_FIX )
            aRbtFix.Check();
        else
            aRbtVar.Check();
    }
    aRbtFix.SaveValue();
    aRbtVar.SaveValue();

    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET == maInputSet.GetItemState(EE_CHAR_LANGUAGE, TRUE, &pItem ) )
        maLbLanguage.SelectLanguage( static_cast<const SvxLanguageItem*>(pItem)->GetLanguage() );

    maLbLanguage.SaveValue();

    FillFormatList();
    aLbFormat.SaveValue();
}


IMPL_LINK( SdModifyFieldDlg, LanguageChangeHdl, void *, EMPTYARG )
{
    FillFormatList();

    return 0L;
}

SfxItemSet SdModifyFieldDlg::GetItemSet()
{
    SfxItemSet aOutput( *maInputSet.GetPool(), EE_CHAR_LANGUAGE, EE_CHAR_LANGUAGE_CTL );

    if( maLbLanguage.GetSelectEntryPos() != maLbLanguage.GetSavedValue() )
    {
        LanguageType eLangType = maLbLanguage.GetSelectLanguage();
        SvxLanguageItem aItem( eLangType, EE_CHAR_LANGUAGE );
        aOutput.Put( aItem );

        SvxLanguageItem aItemCJK( eLangType, EE_CHAR_LANGUAGE_CJK );
        aOutput.Put( aItemCJK );

        SvxLanguageItem aItemCTL( eLangType, EE_CHAR_LANGUAGE_CTL );
        aOutput.Put( aItemCTL );
    }

    return aOutput;
}

