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
#include "precompiled_extensions.hxx"
#include "fieldmappingpage.hxx"
#include "fieldmappingimpl.hxx"
#include "addresssettings.hxx"
#include "abspilot.hxx"

//.........................................................................
namespace abp
{
//.........................................................................

    //=====================================================================
    //= FieldMappingPage
    //=====================================================================
    //---------------------------------------------------------------------
    FieldMappingPage::FieldMappingPage( OAddessBookSourcePilot* _pParent )
        :AddressBookSourcePage( _pParent, ModuleRes( RID_PAGE_FIELDMAPPING ) )
        ,m_aExplanation     ( this, ModuleRes( FT_FIELDASSIGMENTEXPL ) )
        ,m_aInvokeDialog    ( this, ModuleRes( PB_INVOKE_FIELDS_DIALOG ) )
        ,m_aHint            ( this, ModuleRes( FT_ASSIGNEDFIELDS ) )
    {
        FreeResource();

        m_aInvokeDialog.SetClickHdl( LINK( this, FieldMappingPage, OnInvokeDialog ) );

        // check the size of the InvokeDialog button - some languages are very ... gossipy here ....
        // 96349 - 09.01.2002 - fs@openoffice.org
        sal_Int32 nTextWidth = m_aInvokeDialog.GetTextWidth( m_aInvokeDialog.GetText() );

        sal_Int32 nBorderSpace = m_aInvokeDialog.LogicToPixel( Point( 4, 0 ), MAP_APPFONT ).X();
        sal_Int32 nSpace = m_aInvokeDialog.GetOutputSizePixel().Width() - 2 * nBorderSpace;

        if ( nSpace < nTextWidth )
        {
            Size aButtonSize = m_aInvokeDialog.GetSizePixel();
            aButtonSize.Width() += nTextWidth - nSpace;
            m_aInvokeDialog.SetSizePixel( aButtonSize );
        }
    }

    //---------------------------------------------------------------------
    void FieldMappingPage::ActivatePage()
    {
        AddressBookSourcePage::ActivatePage();
        m_aInvokeDialog.GrabFocus();
    }

    //---------------------------------------------------------------------
    void FieldMappingPage::DeactivatePage()
    {
        AddressBookSourcePage::DeactivatePage();
    }

    //---------------------------------------------------------------------
    void FieldMappingPage::initializePage()
    {
        AddressBookSourcePage::initializePage();
        implUpdateHint();
    }

    //---------------------------------------------------------------------
    void FieldMappingPage::implUpdateHint()
    {
        const AddressSettings& rSettings = getSettings();
        String sHint;
        if ( 0 == rSettings.aFieldMapping.size() )
            sHint = String( ModuleRes( RID_STR_NOFIELDSASSIGNED ) );
        m_aHint.SetText( sHint );
    }

    //---------------------------------------------------------------------
    IMPL_LINK( FieldMappingPage, OnInvokeDialog, void*, /*NOTINTERESTEDIN*/ )
    {
        AddressSettings& rSettings = getSettings();

        // invoke the dialog doing the mapping
        if ( fieldmapping::invokeDialog( getORB(), this, getDialog()->getDataSource().getDataSource(), rSettings ) )
        {
            if ( rSettings.aFieldMapping.size() )
                getDialog()->travelNext();
            else
                implUpdateHint();
        }

        return 0L;
    }

//.........................................................................
}   // namespace abp
//.........................................................................

