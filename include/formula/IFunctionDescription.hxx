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

#ifndef INCLUDED_FORMULA_IFUNCTIONDESCRIPTION_HXX
#define INCLUDED_FORMULA_IFUNCTIONDESCRIPTION_HXX

#include <vector>
#include <memory>
#include <formula/formuladllapi.h>
#include <rtl/ustring.hxx>
#include <tools/solar.h>
#include <com/sun/star/sheet/XFormulaParser.hpp>
#include <com/sun/star/sheet/XFormulaOpCodeMapper.hpp>

class SvTreeListEntry;

namespace formula
{
    class IFunctionCategory;
    class IFunctionDescription;
    class FormEditData;
    class FormulaTokenArray;

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

        virtual sal_Unicode getSingleToken(const EToken _eToken) const = 0;

    protected:
        ~IFunctionManager() {}
    };

    class SAL_NO_VTABLE IFunctionCategory
    {
    public:
        IFunctionCategory(){}
        virtual sal_uInt32                  getCount() const = 0;
        virtual const IFunctionDescription* getFunction(sal_uInt32 _nPos) const = 0;
        virtual sal_uInt32                  getNumber() const = 0;
        virtual OUString             getName() const = 0;

    protected:
        ~IFunctionCategory() {}
    };

    class SAL_NO_VTABLE IFunctionDescription
    {
    public:
        IFunctionDescription(){}
        virtual OUString getFunctionName() const = 0;
        virtual const IFunctionCategory* getCategory() const = 0;
        virtual OUString getDescription() const = 0;
        // GetSuppressedArgCount
        virtual sal_Int32 getSuppressedArgumentCount() const = 0;
        // GetFormulaString
        virtual OUString getFormula(const ::std::vector< OUString >& _aArguments) const = 0;
        // GetVisibleArgMapping
        virtual void fillVisibleArgumentMapping(::std::vector<sal_uInt16>& _rArguments) const = 0;
        virtual void initArgumentInfo() const = 0;
        virtual OUString getSignature() const = 0;
        virtual OString getHelpId() const = 0;

        // parameter
        virtual sal_uInt32 getParameterCount() const = 0;
        virtual OUString getParameterName(sal_uInt32 _nPos) const = 0;
        virtual OUString getParameterDescription(sal_uInt32 _nPos) const = 0;
        virtual bool isParameterOptional(sal_uInt32 _nPos) const = 0;

    protected:
        ~IFunctionDescription() {}
    };

    class SAL_NO_VTABLE IFormulaToken
    {
    public:
        virtual bool isFunction() const = 0;
        virtual sal_uInt32 getArgumentCount() const = 0;

    protected:
        ~IFormulaToken() {}
    };

    class SAL_NO_VTABLE IFormulaEditorHelper
    {
    public:
        IFormulaEditorHelper(){}
        virtual void notifyChange() = 0;
        virtual void fill() = 0;

        virtual OUString getCurrentFormula() const = 0;
        virtual void     setCurrentFormula(const OUString& _sReplacement) = 0;

        virtual void getSelection(sal_Int32& _nStart, sal_Int32& _nEnd) const = 0;
        virtual void setSelection(sal_Int32 _nStart, sal_Int32 _nEnd) = 0;

        virtual FormEditData* getFormEditData() const = 0;
        virtual bool calculateValue(const OUString& _sExpression, OUString& _rResult) = 0;

        virtual void switchBack() = 0;

        virtual void clear() = 0;
        virtual void deleteFormData() = 0;
        virtual void setReferenceInput(const FormEditData* _pData) = 0;

        virtual IFunctionManager*   getFunctionManager() = 0;
        virtual ::std::unique_ptr<FormulaTokenArray> convertToTokenArray(const ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FormulaToken >& _aTokenList) = 0;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XFormulaParser> getFormulaParser() const = 0;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XFormulaOpCodeMapper> getFormulaOpCodeMapper() const = 0;
        virtual ::com::sun::star::table::CellAddress getReferencePosition() const = 0;

        virtual void setDispatcherLock( bool bLock ) = 0;
        virtual void dispatch(bool _bOK, bool _bMatrixChecked) = 0;
        virtual void doClose(bool _bOk) = 0;
        virtual void insertEntryToLRUList(const IFunctionDescription*   pDesc) = 0;
        virtual void showReference(const OUString& _sFormula) = 0;

    protected:
        ~IFormulaEditorHelper() {}
    };

}
#endif // INCLUDED_FORMULA_IFUNCTIONDESCRIPTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
