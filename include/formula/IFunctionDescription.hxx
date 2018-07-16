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

#include <memory>
#include <vector>

#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

namespace com { namespace sun { namespace star {
    namespace sheet { struct FormulaToken; }
    namespace sheet { class XFormulaOpCodeMapper; }
    namespace sheet { class XFormulaParser; }
} } }

namespace formula
{
    class IFunctionCategory;
    class IFunctionDescription;
    class FormEditData;
    class FormulaTokenArray;
    class FormulaCompiler;

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
        virtual bool isHidden() const = 0;

        // parameter
        virtual sal_uInt32 getParameterCount() const = 0;
        virtual sal_uInt32 getVarArgsStart() const = 0;
        virtual OUString getParameterName(sal_uInt32 _nPos) const = 0;
        virtual OUString getParameterDescription(sal_uInt32 _nPos) const = 0;
        virtual bool isParameterOptional(sal_uInt32 _nPos) const = 0;

    protected:
        ~IFunctionDescription() {}
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
        virtual bool calculateValue(const OUString& _sExpression, OUString& _rResult, bool bMatrixFormula) = 0;

        /** Obtain a resident FormulaCompiler instance, created without
            FormulaTokenArray and reused but being application specific derived.
         */
        virtual std::shared_ptr<FormulaCompiler> getCompiler() const = 0;

        /** Create an application specific FormulaCompiler instance with
            FormulaTokenArray. The FormulaTokenArray had to be created using
            convertToTokenArray().
         */
        virtual std::unique_ptr<FormulaCompiler> createCompiler( FormulaTokenArray& rArray ) const = 0;

        virtual void switchBack() = 0;

        virtual void clear() = 0;
        virtual void deleteFormData() = 0;

        virtual IFunctionManager*   getFunctionManager() = 0;
        virtual ::std::unique_ptr<FormulaTokenArray> convertToTokenArray(const css::uno::Sequence< css::sheet::FormulaToken >& _aTokenList) = 0;

        virtual css::uno::Reference< css::sheet::XFormulaParser> getFormulaParser() const = 0;
        virtual css::uno::Reference< css::sheet::XFormulaOpCodeMapper> getFormulaOpCodeMapper() const = 0;
        virtual css::table::CellAddress getReferencePosition() const = 0;

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
