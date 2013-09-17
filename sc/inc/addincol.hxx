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

#ifndef SC_ADDINCOL_HXX
#define SC_ADDINCOL_HXX

#include "global.hxx"
#include <com/sun/star/sheet/XVolatileResult.hpp>
#include <com/sun/star/sheet/XAddIn.hpp>
#include <com/sun/star/sheet/XResultListener.hpp>
#include <com/sun/star/sheet/ResultEvent.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/reflection/XIdlMethod.hpp>
#include <i18nlangtag/lang.h>
#include <rtl/ustring.h>
#include "scdllapi.h"
#include <rtl/ustring.hxx>

#include "types.hxx"

#include <boost/unordered_map.hpp>

class SfxObjectShell;
class ScUnoAddInFuncData;
class ScFuncDesc;

typedef ::boost::unordered_map< OUString, const ScUnoAddInFuncData*, ScStringHashCode, ::std::equal_to< OUString > > ScAddInHashMap;

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

                    LocalizedName( const OUString& rLocale, const OUString& rName )
                        : maLocale( rLocale), maName( rName) { }
    };
private:
    OUString     aOriginalName;      ///< kept in formula
    OUString     aLocalName;         ///< for display
    OUString     aUpperName;         ///< for entering formulas
    OUString     aUpperLocal;        ///< for entering formulas
    OUString     aDescription;
    com::sun::star::uno::Reference< com::sun::star::reflection::XIdlMethod> xFunction;
    com::sun::star::uno::Any            aObject;
    long                nArgCount;
    ScAddInArgDesc*     pArgDescs;
    long                nCallerPos;
    sal_uInt16          nCategory;
    OString        sHelpId;
    mutable ::std::vector< LocalizedName > maCompNames;
    mutable bool        bCompInitialized;

public:
                ScUnoAddInFuncData( const OUString& rNam, const OUString& rLoc,
                                    const OUString& rDesc,
                                    sal_uInt16 nCat, const OString&,
                                    const com::sun::star::uno::Reference<
                                        com::sun::star::reflection::XIdlMethod>& rFunc,
                                    const com::sun::star::uno::Any& rO,
                                    long nAC, const ScAddInArgDesc* pAD,
                                    long nCP );
                ~ScUnoAddInFuncData();

    const OUString&  GetOriginalName() const     { return aOriginalName; }
    const OUString&  GetLocalName() const        { return aLocalName; }
    const OUString&  GetUpperName() const        { return aUpperName; }
    const OUString&  GetUpperLocal() const       { return aUpperLocal; }
    const com::sun::star::uno::Reference< com::sun::star::reflection::XIdlMethod>&   GetFunction() const
                                                        { return xFunction; }
    const com::sun::star::uno::Any& GetObject() const   { return aObject; }
    long                    GetArgumentCount() const    { return nArgCount; }
    const ScAddInArgDesc*   GetArguments() const        { return pArgDescs; }
    long                    GetCallerPos() const        { return nCallerPos; }
    const OUString&  GetDescription() const      { return aDescription; }
    sal_uInt16              GetCategory() const         { return nCategory; }
    const OString      GetHelpId() const           { return sHelpId; }

    const ::std::vector< LocalizedName >&  GetCompNames() const;
    bool                    GetExcelName( LanguageType eDestLang, OUString& rRetExcelName ) const;

    void    SetFunction( const com::sun::star::uno::Reference< com::sun::star::reflection::XIdlMethod>& rNewFunc,
                         const com::sun::star::uno::Any& rNewObj );
    void    SetArguments( long nNewCount, const ScAddInArgDesc* pNewDescs );
    void    SetCallerPos( long nNewPos );
    void    SetCompNames( const ::std::vector< LocalizedName >& rNew );
};

class SC_DLLPUBLIC ScUnoAddInCollection
{
private:
    long                    nFuncCount;
    ScUnoAddInFuncData**    ppFuncData;
    ScAddInHashMap*         pExactHashMap;      ///< exact internal name
    ScAddInHashMap*         pNameHashMap;       ///< internal name upper
    ScAddInHashMap*         pLocalHashMap;      ///< localized name upper
    bool                    bInitialized;

    void        Initialize();
    void        ReadConfiguration();
    void        ReadFromAddIn( const com::sun::star::uno::Reference<
                                com::sun::star::uno::XInterface>& xInterface );
    void        UpdateFromAddIn( const com::sun::star::uno::Reference<
                                  com::sun::star::uno::XInterface>& xInterface,
                                const OUString& rServiceName );
    void        LoadComponent( const ScUnoAddInFuncData& rFuncData );

public:
                ScUnoAddInCollection();
                ~ScUnoAddInCollection();

                        /// User enetered name. rUpperName MUST already be upper case!
    OUString     FindFunction( const OUString& rUpperName, bool bLocalFirst );

                        /** Only if bComplete is set, the function reference and argument types
                            are initialized (component may have to be loaded).
                            @param rName is the exact Name. */
    const ScUnoAddInFuncData*   GetFuncData( const OUString& rName, bool bComplete = false );

                        /** For enumeration in ScCompiler::OpCodeMap::getAvailableMappings().
                            @param nIndex
                                0 <= nIndex < GetFuncCount()
                         */
    const ScUnoAddInFuncData*   GetFuncData( long nIndex );

    void                Clear();

    void                LocalizeString( OUString& rName );    ///< modify rName - input: exact name

    long                GetFuncCount();
    bool                FillFunctionDesc( long nFunc, ScFuncDesc& rDesc );

    static bool         FillFunctionDescFromData( const ScUnoAddInFuncData& rFuncData, ScFuncDesc& rDesc );
                  /// leave rRetExcelName unchanged, if no matching name is found
    bool                GetExcelName( const OUString& rCalcName, LanguageType eDestLang, OUString& rRetExcelName );
                  /// leave rRetCalcName unchanged, if no matching name is found
    bool                GetCalcName( const OUString& rExcelName, OUString& rRetCalcName );
};

class ScUnoAddInCall
{
private:
    const ScUnoAddInFuncData*   pFuncData;
    com::sun::star::uno::Sequence<com::sun::star::uno::Any>         aArgs;
    com::sun::star::uno::Sequence<com::sun::star::uno::Any>         aVarArg;
    com::sun::star::uno::Reference<com::sun::star::uno::XInterface> xCaller;
    bool                        bValidCount;
    // result:
    sal_uInt16                  nErrCode;
    bool                        bHasString;
    double                      fValue;
    OUString                    aString;
    ScMatrixRef                 xMatrix;
    com::sun::star::uno::Reference<com::sun::star::sheet::XVolatileResult> xVarRes;

    void            ExecuteCallWithArgs(
                        com::sun::star::uno::Sequence<com::sun::star::uno::Any>& rCallArgs);

public:
                    // exact name
                    ScUnoAddInCall( ScUnoAddInCollection& rColl, const OUString& rName,
                                    long nParamCount );
                    ~ScUnoAddInCall();

    bool                NeedsCaller() const;
    void                SetCaller( const com::sun::star::uno::Reference<
                                    com::sun::star::uno::XInterface>& rInterface );
    void                SetCallerFromObjectShell( SfxObjectShell* pSh );

    bool                ValidParamCount();
    ScAddInArgumentType GetArgType( long nPos );
    void                SetParam( long nPos, const com::sun::star::uno::Any& rValue );

    void                ExecuteCall();

    void                SetResult( const com::sun::star::uno::Any& rNewRes );

    sal_uInt16          GetErrCode() const      { return nErrCode; }
    bool                HasString() const       { return bHasString; }
    bool                HasMatrix() const       { return xMatrix.get(); }
    bool                HasVarRes() const       { return ( xVarRes.is() ); }
    double              GetValue() const        { return fValue; }
    const OUString&     GetString() const       { return aString; }
    ScMatrixRef         GetMatrix() const;
    com::sun::star::uno::Reference<com::sun::star::sheet::XVolatileResult>
                        GetVarRes() const       { return xVarRes; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
