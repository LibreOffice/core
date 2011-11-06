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
