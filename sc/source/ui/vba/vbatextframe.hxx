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


#ifndef SC_VBA_TEXTFRAME_HXX
#define SC_VBA_TEXTFRAME_HXX
#include <ooo/vba/excel/XTextFrame.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <vbahelper/vbatextframe.hxx>

//typedef InheritedHelperInterfaceImpl1< ov::excel::XTextFrame > ScVbaTextFrame_BASE;
typedef cppu::ImplInheritanceHelper1< VbaTextFrame, ov::excel::XTextFrame > ScVbaTextFrame_BASE;

class ScVbaTextFrame : public ScVbaTextFrame_BASE
{
public:
    ScVbaTextFrame( css::uno::Sequence< css::uno::Any > const& aArgs, css::uno::Reference< css::uno::XComponentContext > const& xContext ) throw ( css::lang::IllegalArgumentException );
    virtual ~ScVbaTextFrame() {}
    // Methods
    virtual css::uno::Any SAL_CALL Characters(  ) throw (css::uno::RuntimeException);
    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();

};

#endif//SC_VBA_TEXTFRAME_HXX
