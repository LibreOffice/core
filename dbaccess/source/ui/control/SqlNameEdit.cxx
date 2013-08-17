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

#include "SqlNameEdit.hxx"
namespace dbaui
{
    sal_Bool isCharOk(sal_Unicode _cChar,sal_Bool _bFirstChar,sal_Bool _bUpperCase,const OUString& _sAllowedChars)
    {
        return  (
                 (_cChar >= 'A' && _cChar <= 'Z') ||
                 _cChar == '_' ||
                 _sAllowedChars.indexOf(_cChar) != -1 ||
                 (!_bFirstChar && (_cChar >= '0' && _cChar <= '9')) ||
                 (!_bUpperCase && (_cChar >= 'a' && _cChar <= 'z'))
                );
    }
    sal_Bool OSQLNameChecker::checkString(const OUString& _sToCheck,
                                        OUString& _rsCorrected)
    {
        sal_Bool bCorrected = sal_False;
        if ( m_bCheck )
        {
            OUString sText = _sToCheck;
            sal_Int32 nMatch = 0;
            for (sal_Int32 i = nMatch; i < sText.getLength(); ++i)
            {
                if ( !isCharOk( sText[i], i == 0, m_bOnlyUpperCase, m_sAllowedChars ) )
                {
                    _rsCorrected += sText.copy(nMatch, i - nMatch);
                    bCorrected = sal_True;
                    nMatch = i + 1;
                }
            }
            _rsCorrected += sText.copy( nMatch, sText.getLength() - nMatch );
        }
        return bCorrected;
    }
    void OSQLNameEdit::Modify()
    {
        OUString sCorrected;
        if ( checkString( GetText(),sCorrected ) )
        {
            Selection aSel = GetSelection();
            aSel.setMax( aSel.getMin() );
            SetText( sCorrected, aSel );

            SaveValue();
        }
        Edit::Modify();
    }
    void OSQLNameComboBox::Modify()
    {
        OUString sCorrected;
        if ( checkString( GetText(),sCorrected ) )
        {
            Selection aSel = GetSelection();
            aSel.setMax( aSel.getMin() );
            SetText( sCorrected );

            SaveValue();
        }
        ComboBox::Modify();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
