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



#ifndef _HYP_HXX
#define _HYP_HXX

#include <tools/link.hxx>
#include <editeng/splwrap.hxx>
#include <com/sun/star/linguistic2/XHyphenator.hpp>

class SwView;

class SwHyphWrapper : public SvxSpellWrapper {
private:
    SwView* pView;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XHyphenator >  xHyph;
    Link            aOldLnk;
    LanguageType        nLangError;     // nicht vorhandene Sprache
    sal_uInt16          nPageCount;     // Seitenanzahl fuer Progressanzeige
    sal_uInt16          nPageStart;     // 1. gepruefte Seite
    sal_Bool            bInSelection : 1; // Trennen von selektiertem Text
    sal_Bool            bShowError : 1; // nicht vorhandene Sprache melden
    sal_Bool            bAutomatic : 1; // Trennstellen ohne Rueckfrage einfuegen
    sal_Bool            bInfoBox : 1;   // display info-box when ending
    DECL_LINK( SpellError, LanguageType * );

protected:
    virtual void SpellStart( SvxSpellArea eSpell );
    virtual sal_Bool SpellContinue();
    virtual void SpellEnd( );
    virtual sal_Bool SpellMore();
    virtual void InsertHyphen( const sal_uInt16 nPos ); // Hyphen einfuegen

public:
    SwHyphWrapper( SwView* pVw,
                   ::com::sun::star::uno::Reference<
                        ::com::sun::star::linguistic2::XHyphenator >  &rxHyph,
                   sal_Bool bStart, sal_Bool bOther, sal_Bool bSelect );
    ~SwHyphWrapper();
};

#endif
