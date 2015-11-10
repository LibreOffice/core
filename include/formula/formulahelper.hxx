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

#ifndef INCLUDED_FORMULA_FORMULAHELPER_HXX
#define INCLUDED_FORMULA_FORMULAHELPER_HXX

#include <formula/IFunctionDescription.hxx>
#include <formula/formuladllapi.h>
#include <rtl/ustring.hxx>
#include <vector>
#include <memory>

class SvtSysLocale;
class CharClass;

namespace formula
{
    class FORMULA_DLLPUBLIC FormulaHelper
    {
        ::std::unique_ptr<SvtSysLocale> m_pSysLocale;
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

        bool                GetNextFunc( const OUString&     rFormula,
                                               bool          bBack,
                                               sal_Int32&    rFStart, // Ein- und Ausgabe
                                               sal_Int32*    pFEnd = nullptr,
                                         const IFunctionDescription** ppFDesc = nullptr,
                                               ::std::vector< OUString>*      pArgs = nullptr ) const;

        sal_Int32           GetFunctionStart( const OUString& rFormula, sal_Int32 nStart,
                                                        bool bBack, OUString* pFuncName = nullptr ) const;

        sal_Int32           GetFunctionEnd  ( const OUString& rFormula, sal_Int32 nStart ) const;

        sal_Int32           GetArgStart     ( const OUString& rFormula, sal_Int32 nStart,
                                                      sal_uInt16 nArg ) const;

        void                GetArgStrings   ( ::std::vector< OUString >& _rArgs,
                                                      const OUString&   rFormula,
                                                      sal_Int32         nFuncPos,
                                                      sal_uInt16        nArgs ) const;

        void                FillArgStrings  ( const OUString& rFormula,
                                                      sal_Int32         nFuncPos,
                                                      sal_uInt16        nArgs,
                                                      ::std::vector< OUString >&      _rArgs ) const;
    };

} // formula


#endif // INCLUDED_FORMULA_FORMULAHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
