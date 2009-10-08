/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SqlNameEdit.cxx,v $
 * $Revision: 1.10 $
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
#include "precompiled_dbaccess.hxx"
#ifndef DBAUI_SQLNAMEEDIT_HXX
#include "SqlNameEdit.hxx"
#endif
namespace dbaui
{
    //------------------------------------------------------------------
    sal_Bool isCharOk(sal_Unicode _cChar,sal_Bool _bFirstChar,sal_Bool _bUpperCase,const ::rtl::OUString& _sAllowedChars)
    {
        return  (
                 (_cChar >= 'A' && _cChar <= 'Z') ||
                 _cChar == '_' ||
                 _sAllowedChars.indexOf(_cChar) != -1 ||
                 (!_bFirstChar && (_cChar >= '0' && _cChar <= '9')) ||
                 (!_bUpperCase && (_cChar >= 'a' && _cChar <= 'z'))
                );
    }
    //------------------------------------------------------------------
    sal_Bool OSQLNameChecker::checkString(  const ::rtl::OUString& _sOldValue,
                                        const ::rtl::OUString& _sToCheck,
                                        ::rtl::OUString& _rsCorrected)
    {
        sal_Bool bCorrected = sal_False;
        if ( m_bCheck )
        {
            XubString sSavedValue   = _sOldValue;
            XubString sText         = _sToCheck;
            xub_StrLen nMatch       = 0;
            for ( xub_StrLen i=nMatch;i < sText.Len(); ++i )
            {
                if ( !isCharOk( sText.GetBuffer()[i], i == 0, m_bOnlyUpperCase, m_sAllowedChars ) )
                {
                    _rsCorrected += sText.Copy( nMatch, i - nMatch );
                    bCorrected = sal_True;
                    nMatch = i + 1;
                }
            }
            _rsCorrected += sText.Copy( nMatch, sText.Len() - nMatch );
        }
        return bCorrected;
    }
    //------------------------------------------------------------------
    void OSQLNameEdit::Modify()
    {
        ::rtl::OUString sCorrected;
        if ( checkString( GetSavedValue(),GetText(),sCorrected ) )
        {
            Selection aSel = GetSelection();
            aSel.setMax( aSel.getMin() );
            SetText( sCorrected, aSel );

            SaveValue();
        }
        Edit::Modify();
    }
    //------------------------------------------------------------------
    void OSQLNameComboBox::Modify()
    {
        ::rtl::OUString sCorrected;
        if ( checkString( GetSavedValue(),GetText(),sCorrected ) )
        {
            Selection aSel = GetSelection();
            aSel.setMax( aSel.getMin() );
            SetText( sCorrected );

            SaveValue();
        }
        ComboBox::Modify();
    }
}
// -----------------------------------------------------------------------------

