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


#ifndef CHART2_RELATIVESIZEHELPER_HXX
#define CHART2_RELATIVESIZEHELPER_HXX

#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "charttoolsdllapi.hxx"

namespace chart
{

class OOO_DLLPUBLIC_CHARTTOOLS RelativeSizeHelper
{
public:
    static double calculate(
        double fValue,
        const ::com::sun::star::awt::Size & rOldReferenceSize,
        const ::com::sun::star::awt::Size & rNewReferenceSize );

    static void adaptFontSizes(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > & xTargetProperties,
        const ::com::sun::star::awt::Size & rOldReferenceSize,
        const ::com::sun::star::awt::Size & rNewReferenceSize );

private:
    // not implemented
    RelativeSizeHelper();
};

} //  namespace chart

// CHART2_RELATIVESIZEHELPER_HXX
#endif
