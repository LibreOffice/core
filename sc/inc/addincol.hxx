/*************************************************************************
 *
 *  $RCSfile: addincol.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:47 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SC_ADDINCOL_HXX
#define SC_ADDINCOL_HXX


#ifndef _COM_SUN_STAR_SHEET_XVOLATILERESULT_HPP_
#include <com/sun/star/sheet/XVolatileResult.hpp>
#endif

#ifndef _COM_SUN_STAR_SHEET_XADDIN_HPP_
#include <com/sun/star/sheet/XAddIn.hpp>
#endif

#ifndef _COM_SUN_STAR_SHEET_XRESULTLISTENER_HPP_
#include <com/sun/star/sheet/XResultListener.hpp>
#endif

#ifndef _COM_SUN_STAR_SHEET_RESULTEVENT_HPP_
#include <com/sun/star/sheet/ResultEvent.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif


class String;
class SfxObjectShell;
class ScUnoAddInFuncData;
class ScMatrix;
class ScFuncDesc;


enum ScAddInArgumentType
{
    SC_ADDINARG_NONE,                   // -
    SC_ADDINARG_INTEGER,                // long
    SC_ADDINARG_DOUBLE,                 // double
    SC_ADDINARG_STRING,                 // string
    SC_ADDINARG_INTEGER_ARRAY,          // sequence<sequence<long>>
    SC_ADDINARG_DOUBLE_ARRAY,           // sequence<sequence<double>>
    SC_ADDINARG_STRING_ARRAY,           // sequence<sequence<string>>
    SC_ADDINARG_MIXED_ARRAY,            // sequence<sequence<any>>
    SC_ADDINARG_VALUE_OR_ARRAY,         // any
    SC_ADDINARG_CELLRANGE,              // XCellRange
    SC_ADDINARG_CALLER,                 // XPropertySet
    SC_ADDINARG_VARARGS                 // sequence<any>
};


class ScUnoAddInCollection
{
private:
    long                    nFuncCount;
    ScUnoAddInFuncData**    ppFuncData;
    BOOL                    bInitialized;

    void        Initialize();
    void        ReadFromAddIn( const com::sun::star::uno::Reference<
                                com::sun::star::uno::XInterface>& xInterface );

public:
                ScUnoAddInCollection();
                ~ScUnoAddInCollection();

    String              FindFunction( const String& rName, BOOL bLocalFirst );  // user entered name
    ScUnoAddInFuncData* GetFuncData( const String& rName );                     // exact name

    void                LocalizeString( String& rName );    // modify rName - input: exact name

    long                GetFuncCount();
    BOOL                FillFunctionDesc( long nFunc, ScFuncDesc& rDesc );
};


class ScUnoAddInCall
{
private:
    const ScUnoAddInFuncData*   pFuncData;
    com::sun::star::uno::Sequence<com::sun::star::uno::Any>         aArgs;
    com::sun::star::uno::Sequence<com::sun::star::uno::Any>         aVarArg;
    com::sun::star::uno::Reference<com::sun::star::uno::XInterface> xCaller;
    BOOL                        bValidCount;
    // result:
    USHORT                      nErrCode;
    BOOL                        bHasString;
    double                      fValue;
    String                      aString;
    ScMatrix*                   pMatrix;
    com::sun::star::uno::Reference<com::sun::star::sheet::XVolatileResult> xVarRes;

    void            ExecuteCallWithArgs(
                        com::sun::star::uno::Sequence<com::sun::star::uno::Any>& rCallArgs);

public:
                    // exact name
                    ScUnoAddInCall( ScUnoAddInCollection& rColl, const String& rName,
                                    long nParamCount );
                    ~ScUnoAddInCall();

    BOOL                NeedsCaller() const;
    void                SetCaller( const com::sun::star::uno::Reference<
                                    com::sun::star::uno::XInterface>& rInterface );
    void                SetCallerFromObjectShell( SfxObjectShell* pSh );

    BOOL                ValidParamCount();
    ScAddInArgumentType GetArgType( long nPos );
    void                SetParam( long nPos, const com::sun::star::uno::Any& rValue );

    void                ExecuteCall();

    void                SetResult( const com::sun::star::uno::Any& rNewRes );

    USHORT              GetErrCode() const      { return nErrCode; }
    BOOL                HasString() const       { return bHasString; }
    BOOL                HasMatrix() const       { return ( pMatrix != NULL ); }
    BOOL                HasVarRes() const       { return ( xVarRes.is() ); }
    double              GetValue() const        { return fValue; }
    const String&       GetString() const       { return aString; }
    const ScMatrix*     GetMatrix() const       { return pMatrix; }
    com::sun::star::uno::Reference<com::sun::star::sheet::XVolatileResult>
                        GetVarRes() const       { return xVarRes; }
};


#endif

