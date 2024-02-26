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

#pragma once

#include <memory>
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#include <formula/errorcodes.hxx>
#include <i18nlangtag/lang.h>
#include "scdllapi.h"
#include <rtl/ustring.hxx>
#include <tools/long.hxx>

#include "types.hxx"

#include <string_view>
#include <utility>
#include <vector>
#include <unordered_map>

namespace com::sun::star::reflection { class XIdlMethod; }
namespace com::sun::star::sheet { class XVolatileResult; }
namespace com::sun::star::uno { class XInterface; }

class SfxObjectShell;
class ScUnoAddInFuncData;
class ScFuncDesc;
class LanguageTag;
class ScDocument;

typedef std::unordered_map< OUString, const ScUnoAddInFuncData* > ScAddInHashMap;

enum ScAddInArgumentType
{
    SC_ADDINARG_NONE,                   ///< -
    SC_ADDINARG_INTEGER,                ///< long
    SC_ADDINARG_DOUBLE,                 ///< double
    SC_ADDINARG_STRING,                 ///< string
    SC_ADDINARG_INTEGER_ARRAY,          ///< sequence<sequence<long>>
    SC_ADDINARG_DOUBLE_ARRAY,           ///< sequence<sequence<double>>
    SC_ADDINARG_STRING_ARRAY,           ///< sequence<sequence<string>>
    SC_ADDINARG_MIXED_ARRAY,            ///< sequence<sequence<any>>
    SC_ADDINARG_VALUE_OR_ARRAY,         ///< any
    SC_ADDINARG_CELLRANGE,              ///< XCellRange
    SC_ADDINARG_CALLER,                 ///< XPropertySet
    SC_ADDINARG_VARARGS                 ///< sequence<any>
};

struct ScAddInArgDesc
{
    OUString         aInternalName; ///< used to match configuration and reflection information
    OUString         aName;
    OUString         aDescription;
    ScAddInArgumentType eType;
    bool                    bOptional;
};

class ScUnoAddInFuncData
{
public:
    struct LocalizedName
    {
        OUString    maLocale;
        OUString    maName;

                    LocalizedName( OUString aLocale, OUString aName )
                        : maLocale(std::move(aLocale)), maName(std::move(aName)) { }
    };
private:
    OUString            aOriginalName;      ///< kept in formula
    OUString            aLocalName;         ///< for display
    OUString            aUpperName;         ///< for entering formulas
    OUString            aUpperLocal;        ///< for entering formulas
    OUString            aUpperEnglish;      ///< for Function Wizard and tooltips
    OUString            aDescription;
    css::uno::Reference< css::reflection::XIdlMethod> xFunction;
    css::uno::Any       aObject;
    tools::Long                nArgCount;
    std::unique_ptr<ScAddInArgDesc[]>
                        pArgDescs;
    tools::Long                nCallerPos;
    sal_uInt16          nCategory;
    OUString            sHelpId;
    mutable ::std::vector< LocalizedName > maCompNames;
    mutable bool        bCompInitialized;

public:
                ScUnoAddInFuncData( const OUString& rNam, const OUString& rLoc,
                                    OUString aDesc,
                                    sal_uInt16 nCat, OUString ,
                                    css::uno::Reference< css::reflection::XIdlMethod> aFunc,
                                    css::uno::Any aO,
                                    tools::Long nAC, const ScAddInArgDesc* pAD,
                                    tools::Long nCP );
                ~ScUnoAddInFuncData();

    const OUString&  GetOriginalName() const     { return aOriginalName; }
    const OUString&  GetLocalName() const        { return aLocalName; }
    const OUString&  GetUpperName() const        { return aUpperName; }
    const OUString&  GetUpperLocal() const       { return aUpperLocal; }
    const OUString&  GetUpperEnglish() const     { return aUpperEnglish; }
    const css::uno::Reference< css::reflection::XIdlMethod>&   GetFunction() const
                                                        { return xFunction; }
    const css::uno::Any& GetObject() const   { return aObject; }
    tools::Long                    GetArgumentCount() const    { return nArgCount; }
    const ScAddInArgDesc*   GetArguments() const        { return pArgDescs.get(); }
    tools::Long                    GetCallerPos() const        { return nCallerPos; }
    const OUString&         GetDescription() const      { return aDescription; }
    sal_uInt16              GetCategory() const         { return nCategory; }
    const OUString&         GetHelpId() const           { return sHelpId; }

    const ::std::vector< LocalizedName >&  GetCompNames() const;
    bool                    GetExcelName( const LanguageTag& rDestLang, OUString& rRetExcelName,
                                          bool bFallbackToAny = true ) const;

    void    SetFunction( const css::uno::Reference< css::reflection::XIdlMethod>& rNewFunc,
                         const css::uno::Any& rNewObj );
    void    SetArguments( tools::Long nNewCount, const ScAddInArgDesc* pNewDescs );
    void    SetCallerPos( tools::Long nNewPos );
    void    SetCompNames( ::std::vector< LocalizedName >&& rNew );

    /// Takes care of handling an empty name *after* upper local name was set.
    void    SetEnglishName( const OUString& rEnglishName );
};

class ScUnoAddInCollection
{
private:
    tools::Long                    nFuncCount;
    std::unique_ptr<std::unique_ptr<ScUnoAddInFuncData>[]> ppFuncData;
    std::unique_ptr<ScAddInHashMap>       pExactHashMap;      ///< exact internal name
    std::unique_ptr<ScAddInHashMap>       pNameHashMap;       ///< internal name upper
    std::unique_ptr<ScAddInHashMap>       pLocalHashMap;      ///< localized name upper
    std::unique_ptr<ScAddInHashMap>       pEnglishHashMap;    ///< English name upper
    bool                    bInitialized;

    void        Initialize();
    void        ReadConfiguration();
    void        ReadFromAddIn( const css::uno::Reference< css::uno::XInterface>& xInterface );
    void        UpdateFromAddIn( const css::uno::Reference< css::uno::XInterface>& xInterface,
                                std::u16string_view rServiceName );
    void        LoadComponent( const ScUnoAddInFuncData& rFuncData );

public:
                ScUnoAddInCollection();
                ~ScUnoAddInCollection();

                        /// User entered name. rUpperName MUST already be upper case!
    OUString    FindFunction( const OUString& rUpperName, bool bLocalFirst );

                        /** Only if bComplete is set, the function reference and argument types
                            are initialized (component may have to be loaded).
                            @param rName is the exact Name. */
    const ScUnoAddInFuncData*   GetFuncData( const OUString& rName, bool bComplete = false );

                        /** For enumeration in ScCompiler::OpCodeMap::getAvailableMappings().
                            @param nIndex
                                0 <= nIndex < GetFuncCount()
                         */
    const ScUnoAddInFuncData*   GetFuncData( tools::Long nIndex );

    void                Clear();

    void                LocalizeString( OUString& rName );    ///< modify rName - input: exact name

    tools::Long                GetFuncCount();
    bool                FillFunctionDesc( tools::Long nFunc, ScFuncDesc& rDesc, bool bEnglishFunctionNames );

    static bool         FillFunctionDescFromData( const ScUnoAddInFuncData& rFuncData, ScFuncDesc& rDesc,
                                                  bool bEnglishFunctionNames );
                  /// leave rRetExcelName unchanged, if no matching name is found
    SC_DLLPUBLIC bool   GetExcelName( const OUString& rCalcName, LanguageType eDestLang, OUString& rRetExcelName );
                  /// leave rRetCalcName unchanged, if no matching name is found
    SC_DLLPUBLIC bool   GetCalcName( const OUString& rExcelName, OUString& rRetCalcName );
};

class ScUnoAddInCall
{
private:
    const ScUnoAddInFuncData*   pFuncData;
    css::uno::Sequence<css::uno::Any>         aArgs;
    css::uno::Sequence<css::uno::Any>         aVarArg;
    css::uno::Reference<css::uno::XInterface> xCaller;
    ScDocument&                 mrDoc;
    bool                        bValidCount;
    // result:
    FormulaError                nErrCode;
    bool                        bHasString;
    double                      fValue;
    OUString                    aString;
    ScMatrixRef                 xMatrix;
    css::uno::Reference<css::sheet::XVolatileResult> xVarRes;

    void            ExecuteCallWithArgs(css::uno::Sequence<css::uno::Any>& rCallArgs);

public:
                    // exact name
                    ScUnoAddInCall( ScDocument& rDoc, ScUnoAddInCollection& rColl, const OUString& rName,
                                    tools::Long nParamCount );
                    ~ScUnoAddInCall();

    bool                NeedsCaller() const;
    void                SetCaller( const css::uno::Reference<css::uno::XInterface>& rInterface );
    void                SetCallerFromObjectShell( const SfxObjectShell* pSh );

    bool                ValidParamCount() { return bValidCount;}
    ScAddInArgumentType GetArgType( tools::Long nPos );
    void                SetParam( tools::Long nPos, const css::uno::Any& rValue );

    void                ExecuteCall();

    void                SetResult( const css::uno::Any& rNewRes );

    FormulaError        GetErrCode() const      { return nErrCode; }
    bool                HasString() const       { return bHasString; }
    bool                HasMatrix() const       { return bool(xMatrix); }
    bool                HasVarRes() const       { return xVarRes.is(); }
    double              GetValue() const        { return fValue; }
    const OUString&     GetString() const       { return aString; }
    const ScMatrixRef&  GetMatrix() const       { return xMatrix;}
    const css::uno::Reference<css::sheet::XVolatileResult>&
                        GetVarRes() const       { return xVarRes; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
