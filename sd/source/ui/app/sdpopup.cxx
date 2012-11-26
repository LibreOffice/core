/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"


#include <editeng/eeitem.hxx>

#include <editeng/editeng.hxx>
#include <editeng/flditem.hxx>
#include <svl/zforlist.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfile.hxx>
#include <unotools/useroptions.hxx>

#include "strings.hrc"
#include "sdpopup.hxx"
#include "sdresid.hxx"
#include "sdmod.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"

/*************************************************************************
|*
|* PopupMenu zum Bearbeiten von Feldbefehlen
|*
\************************************************************************/

SdFieldPopup::SdFieldPopup( const SvxFieldData* pInField, LanguageType eLanguage ) :
        PopupMenu   (),
        pField      ( pInField )
{
    Fill( eLanguage );
}

/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/

SdFieldPopup::~SdFieldPopup()
{
}

/*************************************************************************
|*
|* Fill
|*
\************************************************************************/

void SdFieldPopup::Fill( LanguageType eLanguage )
{
    sal_uInt16 nID = 1;
    sal_uInt16 nStyle = MIB_RADIOCHECK | MIB_AUTOCHECK;
    InsertItem( nID++, String( SdResId( STR_FIX ) ), nStyle );
    InsertItem( nID++, String( SdResId( STR_VAR ) ), nStyle );
    InsertSeparator();

    const SvxDateField* pDateField = dynamic_cast< const SvxDateField* >(pField);

    if( pDateField )
    {
        SvxDateField aDateField( *pDateField );

        if( pDateField->GetType() == SVXDATETYPE_FIX )
            CheckItem( 1 );
        else
            CheckItem( 2 );

        //SVXDATEFORMAT_APPDEFAULT,     // Wird nicht benutzt
        //SVXDATEFORMAT_SYSTEM,         // Wird nicht benutzt
        InsertItem( nID++, String( SdResId( STR_STANDARD_SMALL ) ), nStyle );
        InsertItem( nID++, String( SdResId( STR_STANDARD_BIG ) ), nStyle );

        SvNumberFormatter* pNumberFormatter = SD_MOD()->GetNumberFormatter();
        aDateField.SetFormat( SVXDATEFORMAT_A );    // 13.02.96
        InsertItem( nID++, aDateField.GetFormatted( *pNumberFormatter, eLanguage ), nStyle );
        aDateField.SetFormat( SVXDATEFORMAT_B );    // 13.02.1996
        InsertItem( nID++, aDateField.GetFormatted( *pNumberFormatter, eLanguage ), nStyle );
        aDateField.SetFormat( SVXDATEFORMAT_C );    // 13.Feb 1996
        InsertItem( nID++, aDateField.GetFormatted( *pNumberFormatter, eLanguage ), nStyle );

        aDateField.SetFormat( SVXDATEFORMAT_D );    // 13.Februar 1996
        InsertItem( nID++, aDateField.GetFormatted( *pNumberFormatter, eLanguage ), nStyle );
        aDateField.SetFormat( SVXDATEFORMAT_E );    // Die, 13.Februar 1996
        InsertItem( nID++, aDateField.GetFormatted( *pNumberFormatter, eLanguage ), nStyle );
        aDateField.SetFormat( SVXDATEFORMAT_F );    // Dienstag, 13.Februar 1996
        InsertItem( nID++, aDateField.GetFormatted( *pNumberFormatter, eLanguage ), nStyle );

        CheckItem( (sal_uInt16) ( pDateField->GetFormat() ) + 1 ); // - 2 + 3 !
    }
    else
    {
        const SvxExtTimeField* pTimeField = dynamic_cast< const SvxExtTimeField* >(pField);

        if( pTimeField )
        {
            SvxExtTimeField aTimeField( *pTimeField );

            if( pTimeField->GetType() == SVXTIMETYPE_FIX )
                CheckItem( 1 );
            else
                CheckItem( 2 );

            //SVXTIMEFORMAT_APPDEFAULT,     // Wird nicht benutzt
            //SVXTIMEFORMAT_SYSTEM,         // Wird nicht benutzt
            InsertItem( nID++, String( SdResId( STR_STANDARD_NORMAL ) ), nStyle );

            SvNumberFormatter* pNumberFormatter = SD_MOD()->GetNumberFormatter();
            aTimeField.SetFormat( SVXTIMEFORMAT_24_HM );    // 13:49
            InsertItem( nID++, aTimeField.GetFormatted( *pNumberFormatter, eLanguage ), nStyle );
            aTimeField.SetFormat( SVXTIMEFORMAT_24_HMS );   // 13:49:38
            InsertItem( nID++, aTimeField.GetFormatted( *pNumberFormatter, eLanguage ), nStyle );
            aTimeField.SetFormat( SVXTIMEFORMAT_24_HMSH );  // 13:49:38.78
            InsertItem( nID++, aTimeField.GetFormatted( *pNumberFormatter, eLanguage ), nStyle );

            aTimeField.SetFormat( SVXTIMEFORMAT_12_HM );    // 01:49
            InsertItem( nID++, aTimeField.GetFormatted( *pNumberFormatter, eLanguage ), nStyle );
            aTimeField.SetFormat( SVXTIMEFORMAT_12_HMS );   // 01:49:38
            InsertItem( nID++, aTimeField.GetFormatted( *pNumberFormatter, eLanguage ), nStyle );
            aTimeField.SetFormat( SVXTIMEFORMAT_12_HMSH );  // 01:49:38.78
            InsertItem( nID++, aTimeField.GetFormatted( *pNumberFormatter, eLanguage ), nStyle );
            //SVXTIMEFORMAT_AM_HM,  // 01:49 PM
            //SVXTIMEFORMAT_AM_HMS, // 01:49:38 PM
            //SVXTIMEFORMAT_AM_HMSH // 01:49:38.78 PM

            CheckItem( (sal_uInt16) ( pTimeField->GetFormat() ) + 1 ); // - 2 + 3 !
        }
        else
        {
            const SvxExtFileField* pFileField = dynamic_cast< const SvxExtFileField* >(pField);

            if( pFileField )
            {
                if( pFileField->GetType() == SVXFILETYPE_FIX )
                    CheckItem( 1 );
                else
                    CheckItem( 2 );

                InsertItem( nID++, String( SdResId( STR_FILEFORMAT_NAME_EXT ) ), nStyle );
                InsertItem( nID++, String( SdResId( STR_FILEFORMAT_FULLPATH ) ), nStyle );
                InsertItem( nID++, String( SdResId( STR_FILEFORMAT_PATH ) ), nStyle );
                InsertItem( nID++, String( SdResId( STR_FILEFORMAT_NAME ) ), nStyle );

                CheckItem( (sal_uInt16) ( pFileField->GetFormat() ) + 3 );
            }
            else
            {
                const SvxAuthorField* pAuthorField = dynamic_cast< const SvxAuthorField* >(pField);

                if( pAuthorField )
                {
                    SvxAuthorField aAuthorField( *pAuthorField );

                    if( pAuthorField->GetType() == SVXAUTHORTYPE_FIX )
                        CheckItem( 1 );
                    else
                        CheckItem( 2 );

                    for( sal_uInt16 i = 0; i < 4; i++ )
                    {
                        aAuthorField.SetFormat( (SvxAuthorFormat) i );
                        InsertItem( nID++, aAuthorField.GetFormatted(), nStyle );
                    }
                    CheckItem( (sal_uInt16) ( pAuthorField->GetFormat() ) + 3 );
                }
            }
        }
    }
}

/*************************************************************************
|*
|* Gibt das neue Feld zurueck, gehoert dem Caller.
|* Liefert NULL, wenn sich nichts geaendert hat.
|*
\************************************************************************/

SvxFieldData* SdFieldPopup::GetField()
{
    SvxFieldData* pNewField = NULL;
    sal_uInt16 nCount = GetItemCount();
    const SvxDateField* pDateField = dynamic_cast< const SvxDateField* >(pField);

    if( pDateField )
    {
        SvxDateType   eType;
        SvxDateFormat eFormat;
        sal_uInt16 i;

        if( IsItemChecked( 1 ) )
            eType = SVXDATETYPE_FIX;
        else
            eType = SVXDATETYPE_VAR;

        for( i = 3; i <= nCount; i++ )
        {
            if( IsItemChecked( i ) )
                break;
        }
        eFormat = (SvxDateFormat) ( i - 1 );

        if( pDateField->GetFormat() != eFormat ||
            pDateField->GetType() != eType )
        {
            pNewField = new SvxDateField( *pDateField );
            ( (SvxDateField*) pNewField )->SetType( eType );
            ( (SvxDateField*) pNewField )->SetFormat( eFormat );

            if( (pDateField->GetType() == SVXDATETYPE_VAR) && (eType == SVXDATETYPE_FIX) )
            {
                Date aDate;
                ( (SvxDateField*) pNewField )->SetFixDate( aDate );
            }
        }
    }
    else
    {
        const SvxExtTimeField* pTimeField = dynamic_cast< const SvxExtTimeField* >(pField);

        if( pTimeField )
        {
            SvxTimeType   eType;
            SvxTimeFormat eFormat;
            sal_uInt16 i;

            if( IsItemChecked( 1 ) )
                eType = SVXTIMETYPE_FIX;
            else
                eType = SVXTIMETYPE_VAR;

            for( i = 3; i <= nCount; i++ )
            {
                if( IsItemChecked( i ) )
                    break;
            }
            eFormat = (SvxTimeFormat) ( i - 1 );

            if( pTimeField->GetFormat() != eFormat ||
                pTimeField->GetType() != eType )
            {
                pNewField = new SvxExtTimeField( *pTimeField );
                ( (SvxExtTimeField*) pNewField )->SetType( eType );
                ( (SvxExtTimeField*) pNewField )->SetFormat( eFormat );

                if( (pTimeField->GetType() == SVXTIMETYPE_VAR) && (eType == SVXTIMETYPE_FIX) )
                {
                    Time aTime;
                    ( (SvxExtTimeField*) pNewField )->SetFixTime( aTime );
                }

            }
        }
        else
        {
            const SvxExtFileField* pFileField = dynamic_cast< const SvxExtFileField* >(pField);

            if( pFileField )
            {
                SvxFileType   eType;
                SvxFileFormat eFormat;
                sal_uInt16 i;

                if( IsItemChecked( 1 ) )
                    eType = SVXFILETYPE_FIX;
                else
                    eType = SVXFILETYPE_VAR;

                for( i = 3; i <= nCount; i++ )
                {
                    if( IsItemChecked( i ) )
                        break;
                }
                eFormat = (SvxFileFormat) ( i - 3 );

                if( pFileField->GetFormat() != eFormat ||
                    pFileField->GetType() != eType )
                {
                    ::sd::DrawDocShell* pDocSh = dynamic_cast< ::sd::DrawDocShell* >(SfxObjectShell::Current() );

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
            }
            else
            {
                const SvxAuthorField* pAuthorField = dynamic_cast< const SvxAuthorField* >(pField);

                if( pAuthorField )
                {
                    SvxAuthorType   eType;
                    SvxAuthorFormat eFormat;
                    sal_uInt16 i;

                    if( IsItemChecked( 1 ) )
                        eType = SVXAUTHORTYPE_FIX;
                    else
                        eType = SVXAUTHORTYPE_VAR;

                    for( i = 3; i <= nCount; i++ )
                    {
                        if( IsItemChecked( i ) )
                            break;
                    }
                    eFormat = (SvxAuthorFormat) ( i - 3 );

                    if( pAuthorField->GetFormat() != eFormat ||
                        pAuthorField->GetType() != eType )
                    {
                        // #91225# Get current state of address, not the old one
                        SvtUserOptions aUserOptions;
                        pNewField = new SvxAuthorField( aUserOptions.GetFirstName(), aUserOptions.GetLastName(), aUserOptions.GetID() );
                        ( (SvxAuthorField*) pNewField )->SetType( eType );
                        ( (SvxAuthorField*) pNewField )->SetFormat( eFormat );
                    }
                }
            }
        }
    }
    return( pNewField );
}


