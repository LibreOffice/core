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


#ifndef SW_VBA_RANGEHELPER_HXX
#define SW_VBA_RANGEHELPER_HXX

#include <vbahelper/vbahelperinterface.hxx>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/text/XText.hpp>

class SwVbaRangeHelper
{
public:
    static css::uno::Reference< css::text::XTextRange > getRangeByPosition( const css::uno::Reference< css::text::XText >& rText, sal_Int32 _position )throw ( css::uno::RuntimeException );
    static void insertString( css::uno::Reference< css::text::XTextRange >& rTextRange, css::uno::Reference< css::text::XText >& rText, const rtl::OUString& rStr, sal_Bool _bAbsorb ) throw ( css::uno::RuntimeException );
    static css::uno::Reference< css::text::XTextCursor > initCursor( const css::uno::Reference< css::text::XTextRange >& rTextRange, const css::uno::Reference< css::text::XText >& rText ) throw ( css::uno::RuntimeException );
    static sal_Int32 getPosition( const css::uno::Reference< css::text::XText >& rText, const css::uno::Reference< css::text::XTextRange >& rTextRange ) throw ( css::uno::RuntimeException );

};
#endif /* SW_VBA_RANGEHELPER_HXX */
