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


#ifndef SW_VBA_STYLES_HXX
#define SW_VBA_STYLES_HXX

#include <ooo/vba/word/XStyles.hpp>
#include <vbahelper/vbacollectionimpl.hxx>

typedef CollTestImplHelper< ooo::vba::word::XStyles > SwVbaStyles_BASE;
class SwVbaStyles: public SwVbaStyles_BASE
{
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::lang::XMultiServiceFactory > mxMSF;
public:
    SwVbaStyles( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::frame::XModel >& xModel ) throw ( css::script::BasicErrorException );

    virtual css::uno::Any SAL_CALL Item( const css::uno::Any& Index1, const css::uno::Any& Index2 ) throw (css::uno::RuntimeException);
    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);
    virtual css::uno::Any createCollectionObject(const css::uno::Any&);
    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

#endif
