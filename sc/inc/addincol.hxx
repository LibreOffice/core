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

#ifndef SC_ADDINCOL_HXX
#define SC_ADDINCOL_HXX

#include "global.hxx"
#include <com/sun/star/sheet/XVolatileResult.hpp>
#include <com/sun/star/sheet/XAddIn.hpp>
#include <com/sun/star/sheet/XResultListener.hpp>
#include <com/sun/star/sheet/ResultEvent.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/reflection/XIdlMethod.hpp>
#include <com/sun/star/sheet/LocalizedName.hpp>
#include <i18npool/lang.h>
#include <rtl/ustring.h>
#include "scdllapi.h"
#include <rtl/ustring.hxx>

#include "scmatrix.hxx"

#include <boost/unordered_map.hpp>


class String;
class SfxObjectShell;
class ScUnoAddInFuncData;
class ScFuncDesc;


typedef ::boost::unordered_map< ::rtl::OUString, const ScUnoAddInFuncData*, ScStringHashCode, ::std::equal_to< ::rtl::OUString > > ScAddInHashMap;


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

//------------------------------------------------------------------------

struct ScAddInArgDesc
{
    ::rtl::OUString         aInternalName; ///< used to match configuration and reflection information
    ::rtl::OUString         aName;
    ::rtl::OUString         aDescription;
    ScAddInArgumentType eType;
    sal_Bool                bOptional;
};

class ScUnoAddInFuncData
{
private:
    ::rtl::OUString     aOriginalName;      ///< kept in formula
    ::rtl::OUString     aLocalName;         ///< for display
    ::rtl::OUString     aUpperName;         ///< for entering formulas
    ::rtl::OUString     aUpperLocal;        ///< for entering formulas
    ::rtl::OUString     aDescription;
    com::sun::star::uno::Reference< com::sun::star::reflection::XIdlMethod> xFunction;
    com::sun::star::uno::Any            aObject;
    long                nArgCount;
    ScAddInArgDesc*     pArgDescs;
    long                nCallerPos;
    sal_uInt16          nCategory;
    rtl::OString        sHelpId;
    mutable com::sun::star::uno::Sequence< com::sun::star::sheet::LocalizedName> aCompNames;
    mutable sal_Bool        bCompInitialized;

public:
                ScUnoAddInFuncData( const ::rtl::OUString& rNam, const ::rtl::OUString& rLoc,
                                    const ::rtl::OUString& rDesc,
                                    sal_uInt16 nCat, const rtl::OString&,
                                    const com::sun::star::uno::Reference<
                                        com::sun::star::reflection::XIdlMethod>& rFunc,
                                    const com::sun::star::uno::Any& rO,
                                    long nAC, const ScAddInArgDesc* pAD,
                                    long nCP );
                ~ScUnoAddInFuncData();

    const ::rtl::OUString&  GetOriginalName() const     { return aOriginalName; }
    const ::rtl::OUString&  GetLocalName() const        { return aLocalName; }
    const ::rtl::OUString&  GetUpperName() const        { return aUpperName; }
    const ::rtl::OUString&  GetUpperLocal() const       { return aUpperLocal; }
    const com::sun::star::uno::Reference< com::sun::star::reflection::XIdlMethod>&   GetFunction() const
                                                        { return xFunction; }
    const com::sun::star::uno::Any& GetObject() const   { return aObject; }
    long                    GetArgumentCount() const    { return nArgCount; }
    const ScAddInArgDesc*   GetArguments() const        { return pArgDescs; }
    long                    GetCallerPos() const        { return nCallerPos; }
    const ::rtl::OUString&  GetDescription() const      { return aDescription; }
    sal_uInt16              GetCategory() const         { return nCategory; }
    const rtl::OString      GetHelpId() const           { return sHelpId; }

    const com::sun::star::uno::Sequence< com::sun::star::sheet::LocalizedName>&  GetCompNames() const;
    sal_Bool                    GetExcelName( LanguageType eDestLang, ::rtl::OUString& rRetExcelName ) const;

    void    SetFunction( const com::sun::star::uno::Reference< com::sun::star::reflection::XIdlMethod>& rNewFunc,
                         const com::sun::star::uno::Any& rNewObj );
    void    SetArguments( long nNewCount, const ScAddInArgDesc* pNewDescs );
    void    SetCallerPos( long nNewPos );
    void    SetCompNames( const com::sun::star::uno::Sequence< com::sun::star::sheet::LocalizedName>& rNew );
};

//------------------------------------------------------------------------

class SC_DLLPUBLIC ScUnoAddInCollection
{
private:
    long                    nFuncCount;
    ScUnoAddInFuncData**    ppFuncData;
    ScAddInHashMap*         pExactHashMap;      ///< exact internal name
    ScAddInHashMap*         pNameHashMap;       ///< internal name upper
    ScAddInHashMap*         pLocalHashMap;      ///< localized name upper
    sal_Bool                bInitialized;

    void        Initialize();
    void        ReadConfiguration();
    void        ReadFromAddIn( const com::sun::star::uno::Reference<
                                com::sun::star::uno::XInterface>& xInterface );
    void        UpdateFromAddIn( const com::sun::star::uno::Reference<
                                  com::sun::star::uno::XInterface>& xInterface,
                                const ::rtl::OUString& rServiceName );
    void        LoadComponent( const ScUnoAddInFuncData& rFuncData );

public:
                ScUnoAddInCollection();
                ~ScUnoAddInCollection();

                        /// User enetered name. rUpperName MUST already be upper case!
    ::rtl::OUString     FindFunction( const ::rtl::OUString& rUpperName, sal_Bool bLocalFirst );

                        /** Only if bComplete is set, the function reference and argument types
                            are initialized (component may have to be loaded).
                            @param rName is the exact Name. */
    const ScUnoAddInFuncData*   GetFuncData( const ::rtl::OUString& rName, bool bComplete = false );

                        /** For enumeration in ScCompiler::OpCodeMap::getAvailableMappings().
                            @param nIndex
                                0 <= nIndex < GetFuncCount()
                         */
    const ScUnoAddInFuncData*   GetFuncData( long nIndex );

    void                Clear();

    void                LocalizeString( ::rtl::OUString& rName );    ///< modify rName - input: exact name

    long                GetFuncCount();
    sal_Bool            FillFunctionDesc( long nFunc, ScFuncDesc& rDesc );

    static sal_Bool     FillFunctionDescFromData( const ScUnoAddInFuncData& rFuncData, ScFuncDesc& rDesc );
                  /// leave rRetExcelName unchanged, if no matching name is found
    sal_Bool                GetExcelName( const ::rtl::OUString& rCalcName, LanguageType eDestLang, ::rtl::OUString& rRetExcelName );
                  /// leave rRetCalcName unchanged, if no matching name is found
    sal_Bool                GetCalcName( const ::rtl::OUString& rExcelName, ::rtl::OUString& rRetCalcName );
};


class ScUnoAddInCall
{
private:
    const ScUnoAddInFuncData*   pFuncData;
    com::sun::star::uno::Sequence<com::sun::star::uno::Any>         aArgs;
    com::sun::star::uno::Sequence<com::sun::star::uno::Any>         aVarArg;
    com::sun::star::uno::Reference<com::sun::star::uno::XInterface> xCaller;
    sal_Bool                        bValidCount;
    // result:
    sal_uInt16                      nErrCode;
    sal_Bool                        bHasString;
    double                      fValue;
    ::rtl::OUString             aString;
    ScMatrixRef                 xMatrix;
    com::sun::star::uno::Reference<com::sun::star::sheet::XVolatileResult> xVarRes;

    void            ExecuteCallWithArgs(
                        com::sun::star::uno::Sequence<com::sun::star::uno::Any>& rCallArgs);

public:
                    // exact name
                    ScUnoAddInCall( ScUnoAddInCollection& rColl, const ::rtl::OUString& rName,
                                    long nParamCount );
                    ~ScUnoAddInCall();

    sal_Bool                NeedsCaller() const;
    void                SetCaller( const com::sun::star::uno::Reference<
                                    com::sun::star::uno::XInterface>& rInterface );
    void                SetCallerFromObjectShell( SfxObjectShell* pSh );

    sal_Bool                ValidParamCount();
    ScAddInArgumentType GetArgType( long nPos );
    void                SetParam( long nPos, const com::sun::star::uno::Any& rValue );

    void                ExecuteCall();

    void                SetResult( const com::sun::star::uno::Any& rNewRes );

    sal_uInt16              GetErrCode() const      { return nErrCode; }
    sal_Bool                HasString() const       { return bHasString; }
    bool                HasMatrix() const       { return xMatrix.get(); }
    sal_Bool                HasVarRes() const       { return ( xVarRes.is() ); }
    double              GetValue() const        { return fValue; }
    const ::rtl::OUString&       GetString() const       { return aString; }
    ScMatrixRef         GetMatrix() const       { return xMatrix; }
    com::sun::star::uno::Reference<com::sun::star::sheet::XVolatileResult>
                        GetVarRes() const       { return xVarRes; }
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
