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


#ifndef SC_VBA_DIALOG_HXX
#define SC_VBA_DIALOG_HXX

#include <cppuhelper/implbase1.hxx>
#include <ooo/vba/excel/XDialog.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <vbahelper/vbadialogbase.hxx>

typedef cppu::ImplInheritanceHelper1< VbaDialogBase, ov::excel::XDialog > ScVbaDialog_BASE;

class ScVbaDialog : public ScVbaDialog_BASE
{
public:
    ScVbaDialog( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > xContext, const css::uno::Reference< css::frame::XModel >& xModel, sal_Int32 nIndex ):ScVbaDialog_BASE( xParent, xContext, xModel, nIndex ) {}
    virtual ~ScVbaDialog() {}

    // Methods
    virtual rtl::OUString mapIndexToName( sal_Int32 nIndex );
    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

#endif /* SC_VBA_DIALOG_HXX */
