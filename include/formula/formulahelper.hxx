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

#ifndef FORMULA_FORMULA_HELPER_HXX
#define FORMULA_FORMULA_HELPER_HXX

#include "formula/IFunctionDescription.hxx"
#include <vector>
#include <tools/string.hxx>
#include <memory>
#include "formula/formuladllapi.h"

class SvtSysLocale;
class CharClass;

namespace formula
{
    class FORMULA_DLLPUBLIC FormulaHelper
    {
        ::std::auto_ptr<SvtSysLocale>   m_pSysLocale;
        const CharClass*                m_pCharClass;
        const IFunctionManager*         m_pFunctionManager;
        const sal_Unicode open;
        const sal_Unicode close;
        const sal_Unicode sep;
        const sal_Unicode arrayOpen;
        const sal_Unicode arrayClose;
    public:
        FormulaHelper(const IFunctionManager* _pFunctionManager);

        inline const CharClass* GetCharClass() const { return m_pCharClass; }

        sal_Bool                GetNextFunc( const OUString&  rFormula,
                                                 sal_Bool           bBack,
                                                 xub_StrLen&    rFStart, // Ein- und Ausgabe
                                                 xub_StrLen*    pFEnd = NULL,
                                                 const IFunctionDescription** ppFDesc = NULL,
                                                 ::std::vector< OUString>*      pArgs = NULL ) const;

        xub_StrLen          GetFunctionStart( const OUString& rFormula, xub_StrLen nStart,
                                                        sal_Bool bBack, OUString* pFuncName = NULL ) const;

        xub_StrLen          GetFunctionEnd  ( const OUString& rFormula, xub_StrLen nStart ) const;

        xub_StrLen          GetArgStart     ( const OUString& rFormula, xub_StrLen nStart,
                                                      sal_uInt16 nArg ) const;

        void                GetArgStrings   ( ::std::vector< OUString >& _rArgs,
                                                      const OUString& rFormula,
                                                      xub_StrLen    nFuncPos,
                                                      sal_uInt16        nArgs ) const;

        void                FillArgStrings  ( const OUString& rFormula,
                                                      xub_StrLen    nFuncPos,
                                                      sal_uInt16        nArgs,
                                                      ::std::vector< OUString >&      _rArgs ) const;
    };
// =============================================================================
} // formula
// =============================================================================

#endif //FORMULA_FORMULA_HELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
