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


#ifndef _EXTINPUT_HXX
#define _EXTINPUT_HXX

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_USHORTS
#include <svl/svstdarr.hxx>
#endif
#include <pam.hxx>
#include <i18npool/lang.h>

class CommandExtTextInputData;

class SwExtTextInput : public SwPaM
{
    SvUShorts aAttrs;
    String sOverwriteText;
    sal_Bool bInsText : 1;
    sal_Bool bIsOverwriteCursor : 1;
    LanguageType eInputLanguage;
public:
    SwExtTextInput( const SwPaM& rPam, Ring* pRing = 0 );
    virtual ~SwExtTextInput();

    void SetInputData( const CommandExtTextInputData& rData );
    const SvUShorts& GetAttrs() const   { return aAttrs; }
    void SetInsText( sal_Bool bFlag )       { bInsText = bFlag; }
    sal_Bool IsOverwriteCursor() const      { return bIsOverwriteCursor; }
    void SetOverwriteCursor( sal_Bool bFlag );
    void SetLanguage(LanguageType eSet) { eInputLanguage = eSet;}
};

#endif  //_EXTINPUT_HXX

