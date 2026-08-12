/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <sal/config.h>

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
        virtual OUString getImplementationName() override;
        virtual bool supportsService( const OUString& sServiceName ) override;
        virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

        // XDispatchRecorder
        virtual void            startRecording         ( const css::uno::Reference< css::frame::XFrame >& xFrame ) override;
        virtual void            recordDispatch         ( const css::util::URL& aURL, const cpo::uno::Sequence< css::beans::PropertyValue >& lArguments ) override;
        virtual void            recordDispatchAsComment( const css::util::URL& aURL, const cpo::uno::Sequence< css::beans::PropertyValue >& lArguments ) override;
        virtual void            endRecording           () override;
        virtual OUString getRecordedMacro       () override;

        virtual cpo::uno::Type getElementType() override;

        virtual bool hasElements() override;

        virtual sal_Int32 getCount() override;

        virtual cpo::uno::Any getByIndex(sal_Int32) override;

        virtual void replaceByIndex(sal_Int32, const cpo::uno::Any&) override;

    // private functions
    private:
        void implts_recordMacro( std::u16string_view aURL,
                                          const cpo::uno::Sequence< css::beans::PropertyValue >& lArguments,
                                                bool bAsComment, OUStringBuffer& );
        void AppendToBuffer( const cpo::uno::Any& aValue, OUStringBuffer& aArgumentBuffer );

}; // class DispatcRecorder

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
