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



#ifndef ADABASUI_SQLNAMEEDIT_HXX
#include "ASQLNameEdit.hxx"
#endif
namespace adabasui
{
    //------------------------------------------------------------------
    sal_Bool isCharOk(sal_Unicode _cChar)
    {
        return (
                (_cChar >= 'A' && _cChar <= 'Z') ||
                _cChar == '_' ||
                ((_cChar >= '0' && _cChar <= '9')) ||
                ((_cChar >= 'a' && _cChar <= 'z'))
               );
    }
    //------------------------------------------------------------------
    sal_Bool OSQLNameChecker::checkString(  const ::rtl::OUString& _sOldValue,
                                        const ::rtl::OUString& _sToCheck,
                                        ::rtl::OUString& _rsCorrected)
    {
        sal_Bool bCorrected = sal_False;
        XubString sSavedValue   = _sOldValue;
        XubString sText         = _sToCheck;
        xub_StrLen nMatch       = 0;
        for ( xub_StrLen i=nMatch;i < sText.Len(); ++i )
        {
            if ( !isCharOk( sText.GetBuffer()[i]) )
            {
                _rsCorrected += sText.Copy( nMatch, i - nMatch );
                bCorrected = sal_True;
                nMatch = i + 1;
            }
        }
        _rsCorrected += sText.Copy( nMatch, sText.Len() - nMatch );
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
}
// -----------------------------------------------------------------------------

