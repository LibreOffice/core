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

#ifndef INCLUDE_FUNCTION_DESCRIPTION
#define INCLUDE_FUNCTION_DESCRIPTION

#include <vector>
#include <memory>
#include "formula/formuladllapi.h"
#include <rtl/ustring.hxx>
#include <tools/string.hxx>
#include <com/sun/star/sheet/XFormulaParser.hpp>
#include <com/sun/star/sheet/XFormulaOpCodeMapper.hpp>

class SvLBoxEntry;

namespace formula
{
    class IFunctionCategory;
    class IFunctionDescription;
    class FormEditData;
    class FormulaTokenArray;

    // ScFunctionMgr* pFuncMgr = ScGlobal::GetStarCalcFunctionMgr();
    class SAL_NO_VTABLE IFunctionManager
    {
    public:
        IFunctionManager(){}
        enum EToken
        {
            eOk,
            eClose,
            eSep,
            eArrayOpen,
            eArrayClose
        };
        virtual sal_uInt32 getCount() const = 0;
        virtual const IFunctionCategory* getCategory(sal_uInt32 nPos) const = 0;
        virtual void fillLastRecentlyUsedFunctions(::std::vector< const IFunctionDescription*>& _rLastRUFunctions) const = 0;
        virtual const IFunctionDescription* getFunctionByName(const ::rtl::OUString& _sFunctionName) const = 0;

        virtual sal_Unicode getSingleToken(const EToken _eToken) const = 0;
    };

    class SAL_NO_VTABLE IFunctionCategory
    {
    public:
        IFunctionCategory(){}
        virtual const IFunctionManager*     getFunctionManager() const = 0;
        virtual sal_uInt32                  getCount() const = 0;
        virtual const IFunctionDescription* getFunction(sal_uInt32 _nPos) const = 0;
        virtual sal_uInt32                  getNumber() const = 0;
        virtual ::rtl::OUString             getName() const = 0;
    };

    class SAL_NO_VTABLE IFunctionDescription
    {
    public:
        IFunctionDescription(){}
        virtual ::rtl::OUString getFunctionName() const = 0;
        virtual const IFunctionCategory* getCategory() const = 0;
        virtual ::rtl::OUString getDescription() const = 0;
        // GetSuppressedArgCount
        virtual xub_StrLen getSuppressedArgumentCount() const = 0;
        // GetFormulaString
        virtual ::rtl::OUString getFormula(const ::std::vector< ::rtl::OUString >& _aArguments) const = 0;
        // GetVisibleArgMapping
        virtual void fillVisibleArgumentMapping(::std::vector<sal_uInt16>& _rArguments) const = 0;
        virtual void initArgumentInfo() const = 0;
        virtual ::rtl::OUString getSignature() const = 0;
        virtual rtl::OString getHelpId() const = 0;

        // parameter
        virtual sal_uInt32 getParameterCount() const = 0;
        virtual ::rtl::OUString getParameterName(sal_uInt32 _nPos) const = 0;
        virtual ::rtl::OUString getParameterDescription(sal_uInt32 _nPos) const = 0;
        virtual bool isParameterOptional(sal_uInt32 _nPos) const = 0;
    };

    class SAL_NO_VTABLE IFormulaToken
    {
    public:
        virtual bool isFunction() const = 0;
        /*
        OpCode eOp = pToken->GetOpCode();
        if(!(pToken->IsFunction()|| ocArcTan2<=eOp))
        */
        virtual sal_uInt32 getArgumentCount() const = 0;
    };

    class SAL_NO_VTABLE IStructHelper
    {
    public:
        IStructHelper(){}
        virtual SvLBoxEntry*    InsertEntry(const XubString& rText, SvLBoxEntry* pParent,
                                sal_uInt16 nFlag,sal_uLong nPos=0,IFormulaToken* pScToken=NULL) = 0;

        virtual String          GetEntryText(SvLBoxEntry* pEntry) const = 0;
        virtual SvLBoxEntry*    GetParent(SvLBoxEntry* pEntry) const = 0;
    };

    class SAL_NO_VTABLE IFormulaEditorHelper
    {
    public:
        IFormulaEditorHelper(){}
        virtual void notifyChange() = 0;
        virtual void fill() = 0;

        virtual String  getCurrentFormula() const = 0;
        virtual void    setCurrentFormula(const String& _sReplacement) = 0;

        virtual void getSelection(xub_StrLen& _nStart,xub_StrLen& _nEnd) const = 0;
        virtual void setSelection(xub_StrLen _nStart,xub_StrLen _nEnd) = 0;

        virtual FormEditData* getFormEditData() const = 0;
        virtual bool calculateValue(const String& _sExpression,String& _rResult) = 0;

        virtual void switchBack() = 0;

        virtual void clear() = 0;
        virtual void deleteFormData() = 0;
        virtual void setReferenceInput(const FormEditData* _pData) = 0;

        virtual IFunctionManager*   getFunctionManager() = 0;
        virtual ::std::auto_ptr<FormulaTokenArray> convertToTokenArray(const ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FormulaToken >& _aTokenList) = 0;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XFormulaParser> getFormulaParser() const = 0;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XFormulaOpCodeMapper> getFormulaOpCodeMapper() const = 0;
        virtual ::com::sun::star::table::CellAddress getReferencePosition() const = 0;

        virtual void setDispatcherLock( sal_Bool bLock ) = 0;
        virtual void dispatch(sal_Bool _bOK,sal_Bool _bMartixChecked) = 0;
        virtual void doClose(sal_Bool _bOk) = 0;
        virtual void insertEntryToLRUList(const IFunctionDescription*   pDesc) = 0;
        virtual void showReference(const String& _sFormula) = 0;
    };

}
#endif //INCLUDE_FUNCTION_DESCRIPTION

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
