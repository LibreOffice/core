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

        sal_Bool                GetNextFunc( const String&  rFormula,
                                                 sal_Bool           bBack,
                                                 xub_StrLen&    rFStart, // Ein- und Ausgabe
                                                 xub_StrLen*    pFEnd = NULL,
                                                 const IFunctionDescription** ppFDesc = NULL,
                                                 ::std::vector< ::rtl::OUString>*      pArgs = NULL ) const;

        xub_StrLen          GetFunctionStart( const String& rFormula, xub_StrLen nStart,
                                                        sal_Bool bBack, String* pFuncName = NULL ) const;

        xub_StrLen          GetFunctionEnd  ( const String& rFormula, xub_StrLen nStart ) const;

        xub_StrLen          GetArgStart     ( const String& rFormula, xub_StrLen nStart,
                                                      sal_uInt16 nArg ) const;

        void                GetArgStrings   ( ::std::vector< ::rtl::OUString >& _rArgs,
                                                      const String& rFormula,
                                                      xub_StrLen    nFuncPos,
                                                      sal_uInt16        nArgs ) const;

        void                FillArgStrings  ( const String& rFormula,
                                                      xub_StrLen    nFuncPos,
                                                      sal_uInt16        nArgs,
                                                      ::std::vector< ::rtl::OUString >&      _rArgs ) const;
    };
// =============================================================================
} // formula
// =============================================================================

#endif //FORMULA_FORMULA_HELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
