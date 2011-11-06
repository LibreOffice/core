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


#ifndef SW_WORD_VBA_HEADERFOOTERHELPER_HXX
#define SW_WORD_VBA_HEADERFOOTERHELPER_HXX

#include <vbahelper/vbahelper.hxx>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/style/XStyle.hpp>

class HeaderFooterHelper
{
public:
    static sal_Bool isHeader( const css::uno::Reference< css::frame::XModel >& xModel, const css::uno::Reference< css::text::XText >& xCurrentText ) throw (css::uno::RuntimeException);
    static sal_Bool isFirstPageHeader( const css::uno::Reference< css::frame::XModel >& xModel, const css::uno::Reference< css::text::XText >& xCurrentText ) throw (css::uno::RuntimeException);
    static sal_Bool isEvenPagesHeader( const css::uno::Reference< css::frame::XModel >& xModel, const css::uno::Reference< css::text::XText >& xCurrentText ) throw (css::uno::RuntimeException);
    static sal_Bool isFooter( const css::uno::Reference< css::frame::XModel >& xModel, const css::uno::Reference< css::text::XText >& xCurrentText ) throw (css::uno::RuntimeException);
    static sal_Bool isFirstPageFooter( const css::uno::Reference< css::frame::XModel >& xModel, const css::uno::Reference< css::text::XText >& xCurrentText ) throw (css::uno::RuntimeException);
    static sal_Bool isEvenPagesFooter( const css::uno::Reference< css::frame::XModel >& xModel, const css::uno::Reference< css::text::XText >& xCurrentText ) throw (css::uno::RuntimeException);
#ifdef TOMORROW
    static sal_Bool isPrimaryHeader( const css::uno::Reference< css::frame::XModel >& xModel, const css::uno::Reference< css::text::XText >& xCurrentText ) throw (css::uno::RuntimeException);
    static sal_Bool isPrimaryFooter( const css::uno::Reference< css::frame::XModel >& xModel, const css::uno::Reference< css::text::XText >& xCurrentText ) throw (css::uno::RuntimeException);
#endif
};

#endif
