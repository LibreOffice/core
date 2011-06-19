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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
