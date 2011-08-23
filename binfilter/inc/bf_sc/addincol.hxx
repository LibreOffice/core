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


#include <com/sun/star/sheet/XVolatileResult.hpp>

#include <com/sun/star/sheet/XAddIn.hpp>

#include <com/sun/star/sheet/XResultListener.hpp>

#include <com/sun/star/sheet/ResultEvent.hpp>

#include <com/sun/star/container/XNameAccess.hpp>


#include <rtl/ustring.h>

#include <hash_map>

class String;
namespace binfilter {


class SfxObjectShell;
class ScUnoAddInFuncData;
class ScMatrix;
class ScFuncDesc;


struct ScAddInStringHashCode
{
    size_t operator()( const String& rStr ) const
    {
        return rtl_ustr_hashCode_WithLength( rStr.GetBuffer(), rStr.Len() );
    }
};
typedef ::std::hash_map< String, const ScUnoAddInFuncData*, ScAddInStringHashCode, ::std::equal_to< String > > ScAddInHashMap;


enum ScAddInArgumentType
{
    SC_ADDINARG_NONE,					// -
    SC_ADDINARG_INTEGER,				// long
    SC_ADDINARG_DOUBLE,					// double
    SC_ADDINARG_STRING,					// string
    SC_ADDINARG_INTEGER_ARRAY,			// sequence<sequence<long>>
    SC_ADDINARG_DOUBLE_ARRAY,			// sequence<sequence<double>>
    SC_ADDINARG_STRING_ARRAY,			// sequence<sequence<string>>
    SC_ADDINARG_MIXED_ARRAY,			// sequence<sequence<any>>
    SC_ADDINARG_VALUE_OR_ARRAY,			// any
    SC_ADDINARG_CELLRANGE,				// XCellRange
    SC_ADDINARG_CALLER,					// XPropertySet
    SC_ADDINARG_VARARGS					// sequence<any>
};


class ScUnoAddInCollection
{
private:
    long					nFuncCount;
    ScUnoAddInFuncData**	ppFuncData;
    ScAddInHashMap*         pExactHashMap;      // exact internal name
    ScAddInHashMap*         pNameHashMap;       // internal name upper
    ScAddInHashMap*         pLocalHashMap;      // localized name upper
    BOOL					bInitialized;

    void		Initialize();
    void		ReadFromAddIn( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::uno::XInterface>& xInterface );

public:
                ScUnoAddInCollection();
                ~ScUnoAddInCollection();

                        /// User enetered name. rUpperName MUST already be upper case!
    String				FindFunction( const String& rUpperName, BOOL bLocalFirst );
    const ScUnoAddInFuncData*   GetFuncData( const String& rName );             // exact name



                                // both leave rRet... unchanged, if no matching name is found
};


class ScUnoAddInCall
{
private:
    const ScUnoAddInFuncData*	pFuncData;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any>			aArgs;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any>			aVarArg;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>	xCaller;
    BOOL						bValidCount;
    // result:
    USHORT						nErrCode;
    BOOL						bHasString;
    double						fValue;
    String						aString;
    ScMatrix*					pMatrix;
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XVolatileResult> xVarRes;

    void			ExecuteCallWithArgs(
                        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any>& rCallArgs);

public:
                    // exact name
    ScUnoAddInCall( ScUnoAddInCollection& rColl, const String& rName,
                                    long nParamCount );
                    ~ScUnoAddInCall();

    BOOL				NeedsCaller() const;
    void				SetCaller( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::uno::XInterface>& rInterface );
    void				SetCallerFromObjectShell( SfxObjectShell* pSh );

    BOOL				ValidParamCount();
    ScAddInArgumentType	GetArgType( long nPos );
    void				SetParam( long nPos, const ::com::sun::star::uno::Any& rValue );

    void				ExecuteCall();

    void				SetResult( const ::com::sun::star::uno::Any& rNewRes );

    USHORT				GetErrCode() const		{ return nErrCode; }
    BOOL				HasString() const		{ return bHasString; }
    BOOL				HasMatrix() const		{ return ( pMatrix != NULL ); }
    BOOL				HasVarRes() const		{ return ( xVarRes.is() ); }
    double				GetValue() const		{ return fValue; }
    const String&		GetString() const		{ return aString; }
    const ScMatrix*		GetMatrix() const		{ return pMatrix; }
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XVolatileResult>
                        GetVarRes() const		{ return xVarRes; }
};


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
