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

#include "newdatatype.hxx"
#include "newdatatype.hrc"

#include "modulepcr.hxx"
#include "formresid.hrc"

//........................................................................
namespace pcr
{
//........................................................................

    //====================================================================
    //= NewDataTypeDialog
    //====================================================================
    //--------------------------------------------------------------------
    NewDataTypeDialog::NewDataTypeDialog( Window* _pParent, const OUString& _rNameBase, const ::std::vector< OUString >& _rProhibitedNames )
        :ModalDialog( _pParent, PcrRes( RID_DLG_NEW_DATA_TYPE ) )
        ,m_aLabel   ( this, PcrRes( FT_LABEL  ) )
        ,m_aName    ( this, PcrRes( ED_NAME   ) )
        ,m_aOK      ( this, PcrRes( PB_OK     ) )
        ,m_aCancel  ( this, PcrRes( PB_CANCEL ) )
        ,m_aProhibitedNames( _rProhibitedNames.begin(), _rProhibitedNames.end() )
    {
        FreeResource();

        m_aName.SetModifyHdl( LINK( this, NewDataTypeDialog, OnNameModified ) );

        // find an initial name
        // for this, first remove trailing digits
        sal_Int32 nStripUntil = _rNameBase.getLength();
        while ( nStripUntil > 0 )
        {
            sal_Unicode nChar = _rNameBase[ --nStripUntil ];
            if ( ( nChar < '0' ) || ( nChar > '9' ) )
            {
                if ( nChar == ' ' )
                    --nStripUntil;  // strip the space, too
                break;
            }
        }

        String sNameBase( _rNameBase.copy( 0, nStripUntil ? nStripUntil + 1 : 0 ) );
        sNameBase.Append( ' ' );
        String sInitialName;
        sal_Int32 nPostfixNumber = 1;
        do
        {
            ( sInitialName = sNameBase ) += OUString::number(nPostfixNumber++);
        }
        while ( m_aProhibitedNames.find( sInitialName ) != m_aProhibitedNames.end() );

        m_aName.SetText( sInitialName );
        OnNameModified( NULL );
    }

    //--------------------------------------------------------------------
    IMPL_LINK( NewDataTypeDialog, OnNameModified, void*, /*_pNotInterestedIn*/ )
    {
        String sCurrentName = GetName();
        bool bNameIsOK = ( sCurrentName.Len() > 0 )
                      && ( m_aProhibitedNames.find( sCurrentName ) == m_aProhibitedNames.end() );

        m_aOK.Enable( bNameIsOK );

        return 0L;
    }

//........................................................................
} // namespace pcr
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
