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

#include "formula/formulahelper.hxx"
#include <unotools/charclass.hxx>
#include <unotools/syslocale.hxx>

namespace formula
{

    namespace
    {
        //============================================================================
        class OEmptyFunctionDescription : public IFunctionDescription
        {
        public:
            OEmptyFunctionDescription(){}
            virtual ~OEmptyFunctionDescription(){}

            virtual OUString getFunctionName() const { return OUString(); }
            virtual const IFunctionCategory* getCategory() const { return NULL; }
            virtual OUString getDescription() const { return OUString(); }
            virtual xub_StrLen getSuppressedArgumentCount() const { return 0; }
            virtual OUString getFormula(const ::std::vector< OUString >& ) const { return OUString(); }
            virtual void fillVisibleArgumentMapping(::std::vector<sal_uInt16>& ) const {}
            virtual void initArgumentInfo()  const {}
            virtual OUString getSignature() const { return OUString(); }
            virtual OString getHelpId() const { return ""; }
            virtual sal_uInt32 getParameterCount() const { return 0; }
            virtual OUString getParameterName(sal_uInt32 ) const { return OUString(); }
            virtual OUString getParameterDescription(sal_uInt32 ) const { return OUString(); }
            virtual bool isParameterOptional(sal_uInt32 ) const { return sal_False; }
        };
    }
//===================================================================
//  class FormulaHelper - static Method
//===================================================================

#define FUNC_NOTFOUND 0xffff

FormulaHelper::FormulaHelper(const IFunctionManager* _pFunctionManager)
    :m_pSysLocale(new SvtSysLocale)
    ,m_pFunctionManager(_pFunctionManager)
    ,open(_pFunctionManager->getSingleToken(IFunctionManager::eOk))
    ,close(_pFunctionManager->getSingleToken(IFunctionManager::eClose))
    ,sep(_pFunctionManager->getSingleToken(IFunctionManager::eSep))
    ,arrayOpen(_pFunctionManager->getSingleToken(IFunctionManager::eArrayOpen))
    ,arrayClose(_pFunctionManager->getSingleToken(IFunctionManager::eArrayClose))
{
    m_pCharClass = m_pSysLocale->GetCharClassPtr();
}

sal_Bool FormulaHelper::GetNextFunc( const OUString&  rFormula,
                                 sal_Bool           bBack,
                                 xub_StrLen&    rFStart,   // Input and output
                                 xub_StrLen*    pFEnd,     // = NULL
                                 const IFunctionDescription**   ppFDesc,   // = NULL
                                 ::std::vector< OUString>*   pArgs )  const // = NULL
{
    xub_StrLen  nOldStart = rFStart;
    OUString      aFname;

    rFStart = GetFunctionStart( rFormula, rFStart, bBack, ppFDesc ? &aFname : NULL );
    sal_Bool bFound  = ( rFStart != FUNC_NOTFOUND );

    if ( bFound )
    {
        if ( pFEnd )
            *pFEnd = GetFunctionEnd( rFormula, rFStart );

        if ( ppFDesc )
        {
            *ppFDesc = NULL;
            const OUString sTemp( aFname );
            const sal_uInt32 nCategoryCount = m_pFunctionManager->getCount();
            for(sal_uInt32 j= 0; j < nCategoryCount && !*ppFDesc; ++j)
            {
                const IFunctionCategory* pCategory = m_pFunctionManager->getCategory(j);
                const sal_uInt32 nCount = pCategory->getCount();
                for(sal_uInt32 i = 0 ; i < nCount; ++i)
                {
                    const IFunctionDescription* pCurrent = pCategory->getFunction(i);
                    if ( pCurrent->getFunctionName().equalsIgnoreAsciiCase(sTemp) )
                    {
                        *ppFDesc = pCurrent;
                        break;
                    }
                }// for(sal_uInt32 i = 0 ; i < nCount; ++i)
            }
            if ( *ppFDesc && pArgs )
            {
                GetArgStrings( *pArgs,rFormula, rFStart, static_cast<sal_uInt16>((*ppFDesc)->getParameterCount() ));
            }
            else
            {
                static OEmptyFunctionDescription s_aFunctionDescription;
                *ppFDesc = &s_aFunctionDescription;
            }
        }
    }
    else
        rFStart = nOldStart;

    return bFound;
}

//------------------------------------------------------------------------

void FormulaHelper::FillArgStrings( const OUString&   rFormula,
                                    xub_StrLen      nFuncPos,
                                    sal_uInt16          nArgs,
                                    ::std::vector< OUString >& _rArgs ) const
{
    xub_StrLen  nStart  = 0;
    xub_StrLen  nEnd    = 0;
    sal_uInt16      i;
    sal_Bool        bLast   = sal_False;

    for ( i=0; i<nArgs && !bLast; i++ )
    {
        nStart = GetArgStart( rFormula, nFuncPos, i );

        if ( i+1<nArgs ) // last argument?
        {
            nEnd = GetArgStart( rFormula, nFuncPos, i+1 );

            if ( nEnd != nStart )
                _rArgs.push_back(rFormula.copy( nStart, nEnd-1-nStart ));
            else
                _rArgs.push_back(OUString()), bLast = sal_True;
        }
        else
        {
            nEnd = GetFunctionEnd( rFormula, nFuncPos )-1;
            if ( nStart < nEnd )
                _rArgs.push_back( rFormula.copy( nStart, nEnd-nStart ) );
            else
                _rArgs.push_back(OUString());
        }
    }

    if ( bLast )
        for ( ; i<nArgs; i++ )
            _rArgs.push_back(OUString());
}

//------------------------------------------------------------------------

void FormulaHelper::GetArgStrings( ::std::vector< OUString >& _rArgs,
                                   const OUString& rFormula,
                                   xub_StrLen nFuncPos,
                                   sal_uInt16 nArgs ) const
{
    if (nArgs)
    {
        FillArgStrings( rFormula, nFuncPos, nArgs, _rArgs );
    }
}

//------------------------------------------------------------------------

inline sal_Bool IsFormulaText( const CharClass* _pCharClass,const OUString& rStr, xub_StrLen nPos )
{
    if( _pCharClass->isLetterNumeric( rStr, nPos ) )
        return sal_True;
    else
    {   // In internationalized versions function names may contain a dot
        //  and in every version also an underscore... ;-)
        sal_Unicode c = rStr[nPos];
        return c == '.' || c == '_';
    }

}

xub_StrLen FormulaHelper::GetFunctionStart( const OUString&   rFormula,
                                            xub_StrLen        nStart,
                                            sal_Bool          bBack,
                                            OUString*         pFuncName ) const
{
    xub_StrLen nStrLen = rFormula.getLength();

    if ( nStrLen < nStart )
        return nStart;

    xub_StrLen  nFStart = FUNC_NOTFOUND;
    xub_StrLen  nParPos = nStart;

    sal_Bool bRepeat, bFound;
    do
    {
        bFound  = sal_False;
        bRepeat = sal_False;

        if ( bBack )
        {
            while ( !bFound && (nParPos > 0) )
            {
                if ( rFormula[nParPos] == '"' )
                {
                    nParPos--;
                    while ( (nParPos > 0) && rFormula[nParPos] != '"' )
                        nParPos--;
                    if (nParPos > 0)
                        nParPos--;
                }
                else if ( (bFound = ( rFormula[nParPos] == '(' ) ) == sal_False )
                    nParPos--;
            }
        }
        else
        {
            while ( !bFound && (nParPos < nStrLen) )
            {
                if ( rFormula[nParPos] == '"' )
                {
                    nParPos++;
                    while ( (nParPos < nStrLen) && rFormula[nParPos] != '"' )
                        nParPos++;
                    nParPos++;
                }
                else if ( (bFound = ( rFormula[nParPos] == '(' ) ) == sal_False )
                    nParPos++;
            }
        }

        if ( bFound && (nParPos > 0) )
        {
            nFStart = nParPos-1;

            while ( (nFStart > 0) && IsFormulaText(m_pCharClass, rFormula, nFStart ))
                nFStart--;
        }

        nFStart++;

        if ( bFound )
        {
            if ( IsFormulaText( m_pCharClass,rFormula, nFStart ) )
            {
                                    //  Function found
                if ( pFuncName )
                    *pFuncName = rFormula.copy( nFStart, nParPos-nFStart );
            }
            else                    // Brackets without function -> keep searching
            {
                bRepeat = sal_True;
                if ( !bBack )
                    nParPos++;
                else if (nParPos > 0)
                    nParPos--;
                else
                    bRepeat = sal_False;
            }
        }
        else                        // No brackets found
        {
            nFStart = FUNC_NOTFOUND;
            if ( pFuncName )
                (*pFuncName) = "";
        }
    }
    while(bRepeat);

    return nFStart;
}

//------------------------------------------------------------------------

xub_StrLen  FormulaHelper::GetFunctionEnd( const OUString& rStr, xub_StrLen nStart ) const
{
    xub_StrLen nStrLen = rStr.getLength();

    if ( nStrLen < nStart )
        return nStart;

    short   nParCount = 0;
    bool    bInArray = false;
    sal_Bool    bFound = sal_False;

    while ( !bFound && (nStart < nStrLen) )
    {
        sal_Unicode c = rStr[nStart];

        if ( c == '"' )
        {
            nStart++;
            while ( (nStart < nStrLen) && rStr[nStart] != '"' )
                nStart++;
        }
        else if ( c == open )
            nParCount++;
        else if ( c == close )
        {
            nParCount--;
            if ( nParCount == 0 )
                bFound = sal_True;
            else if ( nParCount < 0 )
            {
                bFound = sal_True;
                nStart--;   // read one too far
            }
        }
        else if ( c == arrayOpen )
        {
            bInArray = true;
        }
        else if ( c == arrayClose )
        {
            bInArray = false;
        }
        else if ( c == sep )
        {
            if ( !bInArray && nParCount == 0 )
            {
                bFound = sal_True;
                nStart--;   // read one too far
            }
        }
        nStart++; // Set behind found position
    }

    return nStart;
}

//------------------------------------------------------------------

xub_StrLen FormulaHelper::GetArgStart( const OUString& rStr, xub_StrLen nStart, sal_uInt16 nArg ) const
{
    xub_StrLen nStrLen = rStr.getLength();

    if ( nStrLen < nStart )
        return nStart;

    short   nParCount   = 0;
    bool    bInArray    = false;
    sal_Bool    bFound      = sal_False;

    while ( !bFound && (nStart < nStrLen) )
    {
        sal_Unicode c = rStr[nStart];

        if ( c == '"' )
        {
            nStart++;
            while ( (nStart < nStrLen) && rStr[nStart] != '"' )
                nStart++;
        }
        else if ( c == open )
        {
            bFound = ( nArg == 0 );
            nParCount++;
        }
        else if ( c == close )
        {
            nParCount--;
            bFound = ( nParCount == 0 );
        }
        else if ( c == arrayOpen )
        {
            bInArray = true;
        }
        else if ( c == arrayClose )
        {
            bInArray = false;
        }
        else if ( c == sep )
        {
            if ( !bInArray && nParCount == 1 )
            {
                nArg--;
                bFound = ( nArg == 0  );
            }
        }
        nStart++;
    }

    return nStart;
}
// =============================================================================
} // formula
// =============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
