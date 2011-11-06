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


#ifndef SW_VBA_SYSTEM_HXX
#define SW_VBA_SYSTEM_HXX

#include <ooo/vba/word/XSystem.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <cppuhelper/implbase1.hxx>
#include <vbahelper/vbapropvalue.hxx>

typedef InheritedHelperInterfaceImpl1< ooo::vba::word::XSystem > SwVbaSystem_BASE;

class PrivateProfileStringListener : public PropListener
{
private:
    rtl::OUString maFileName;
    ByteString maGroupName;
    ByteString  maKey;
public:
    PrivateProfileStringListener(){};
    virtual ~PrivateProfileStringListener();
    void Initialize( const rtl::OUString& rFileName, const ByteString& rGroupName, const ByteString& rKey );

    //PropListener
    virtual void setValueEvent( const css::uno::Any& value );
    virtual css::uno::Any getValueEvent();
};

class SwVbaSystem : public SwVbaSystem_BASE
{
private:
    PrivateProfileStringListener maPrivateProfileStringListener;

public:
    SwVbaSystem( css::uno::Reference< css::uno::XComponentContext >& m_xContext );
    virtual ~SwVbaSystem();

    // XSystem
    virtual sal_Int32 SAL_CALL getCursor() throw ( css::uno::RuntimeException );
    virtual void SAL_CALL setCursor( sal_Int32 _cursor ) throw ( css::uno::RuntimeException );
    virtual css::uno::Any SAL_CALL PrivateProfileString( const rtl::OUString& rFilename, const rtl::OUString& rSection, const rtl::OUString& rKey ) throw ( css::uno::RuntimeException );

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif /* SW_VBA_SYSTEM_HXX */
