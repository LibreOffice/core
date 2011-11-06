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


#ifndef SW_VBA_INFORMATIONHELPER_HXX
#define SW_VBA_INFORMATIONHELPER_HXX

#include <vbahelper/vbahelperinterface.hxx>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

class SwVbaInformationHelper
{
public:
    static sal_Int32 handleWdActiveEndPageNumber( const css::uno::Reference< css::text::XTextViewCursor >& xTVCursor ) throw( css::uno::RuntimeException );
    static sal_Int32 handleWdNumberOfPagesInDocument( const css::uno::Reference< css::frame::XModel >& xModel ) throw( css::uno::RuntimeException );
    static double handleWdVerticalPositionRelativeToPage( const css::uno::Reference< css::frame::XModel >& xModel, const css::uno::Reference< css::text::XTextViewCursor >& xTVCursor ) throw( css::uno::RuntimeException );
    //static double verticalPositionRelativeToPageBoundary( const css::uno::Reference< css::frame::XModel >& xModel, const css::uno::Reference< css::text::XTextViewCursor >& xTVCursor, const css::uno::Reference< css::beans::XPropertySet >& xStyleProps ) throw( css::uno::RuntimeException );

};
#endif /* SW_VBA_INFORMATIONHELPER_HXX */
