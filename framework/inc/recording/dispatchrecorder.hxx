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

#pragma once

/** Attention: stl headers must(!) be included at first. Otherwise it can make trouble
               with solaris headers ...
*/
#include <vector>

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/frame/XDispatchRecorder.hpp>
#include <com/sun/star/frame/DispatchStatement.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase.hxx>

namespace framework{

class DispatchRecorder final
    : public  ::cppu::WeakImplHelper<
                css::lang::XServiceInfo
              , css::frame::XDispatchRecorder
              , css::container::XIndexReplace >
{
    // private member
    private:
        ::std::vector < css::frame::DispatchStatement >        m_aStatements;
        sal_Int32                                              m_nRecordingID;
        css::uno::Reference< css::script::XTypeConverter >     m_xConverter;

    // public interface
    public:
        DispatchRecorder( const css::uno::Reference< css::uno::XComponentContext >& xSMGR );
        virtual ~DispatchRecorder() override;

        /* interface XServiceInfo */
        virtual OUString SAL_CALL getImplementationName() override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& sServiceName ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

        // XDispatchRecorder
        virtual void SAL_CALL            startRecording         ( const css::uno::Reference< css::frame::XFrame >& xFrame ) override;
        virtual void SAL_CALL            recordDispatch         ( const css::util::URL& aURL, const css::uno::Sequence< css::beans::PropertyValue >& lArguments ) override;
        virtual void SAL_CALL            recordDispatchAsComment( const css::util::URL& aURL, const css::uno::Sequence< css::beans::PropertyValue >& lArguments ) override;
        virtual void SAL_CALL            endRecording           () override;
        virtual OUString SAL_CALL getRecordedMacro       () override;

        virtual css::uno::Type SAL_CALL getElementType() override;

        virtual sal_Bool SAL_CALL hasElements() override;

        virtual sal_Int32 SAL_CALL getCount() override;

        virtual css::uno::Any SAL_CALL getByIndex(sal_Int32) override;

        virtual void SAL_CALL replaceByIndex(sal_Int32, const css::uno::Any&) override;

    // private functions
    private:
        void implts_recordMacro( std::u16string_view aURL,
                                          const css::uno::Sequence< css::beans::PropertyValue >& lArguments,
                                                bool bAsComment, OUStringBuffer& );
        void AppendToBuffer( const css::uno::Any& aValue, OUStringBuffer& aArgumentBuffer );

}; // class DispatcRecorder

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
