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

#ifndef INCLUDED_FRAMEWORK_INC_RECORDING_DISPATCHRECORDER_HXX
#define INCLUDED_FRAMEWORK_INC_RECORDING_DISPATCHRECORDER_HXX

/** Attention: stl headers must(!) be included at first. Otherwise it can make trouble
               with solaris headers ...
*/
#include <vector>

#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <macros/generic.hxx>
#include <general.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/frame/XDispatchRecorder.hpp>
#include <com/sun/star/frame/DispatchStatement.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <cppuhelper/implbase.hxx>

namespace framework{

class DispatchRecorder
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
        virtual ~DispatchRecorder();

        // XInterface, XTypeProvider, XServiceInfo
        DECLARE_XSERVICEINFO

        // XDispatchRecorder
        virtual void SAL_CALL            startRecording         ( const css::uno::Reference< css::frame::XFrame >& xFrame ) throw( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL            recordDispatch         ( const css::util::URL& aURL, const css::uno::Sequence< css::beans::PropertyValue >& lArguments ) throw( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL            recordDispatchAsComment( const css::util::URL& aURL, const css::uno::Sequence< css::beans::PropertyValue >& lArguments ) throw( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL            endRecording           () throw( css::uno::RuntimeException, std::exception ) override;
        virtual OUString SAL_CALL getRecordedMacro       () throw( css::uno::RuntimeException, std::exception ) override;

        virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException, std::exception) override;

        virtual sal_Bool SAL_CALL hasElements()  throw (css::uno::RuntimeException, std::exception) override;

        virtual sal_Int32 SAL_CALL getCount() throw (css::uno::RuntimeException, std::exception) override;

        virtual css::uno::Any SAL_CALL getByIndex(sal_Int32)  throw (css::uno::RuntimeException, css::lang::WrappedTargetException, css::lang::IndexOutOfBoundsException, std::exception) override;

        virtual void SAL_CALL replaceByIndex(sal_Int32, const css::uno::Any&)  throw (css::lang::IllegalArgumentException, css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // private functions
    private:
        void SAL_CALL implts_recordMacro( const OUString& aURL,
                                          const css::uno::Sequence< css::beans::PropertyValue >& lArguments,
                                                bool bAsComment, OUStringBuffer& );
        void SAL_CALL AppendToBuffer( css::uno::Any aValue, OUStringBuffer& aArgumentBuffer );

}; // class DispatcRecorder

} // namespace framework

#endif // define __FRAMEWORK...

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
