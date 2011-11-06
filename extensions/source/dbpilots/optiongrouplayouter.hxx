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



#ifndef _EXTENSIONS_DBP_OPTIONGROUPLAYOUTER_HXX_
#define _EXTENSIONS_DBP_OPTIONGROUPLAYOUTER_HXX_

#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include "dbptypes.hxx"

//.........................................................................
namespace dbp
{
//.........................................................................

    struct OControlWizardContext;
    struct OOptionGroupSettings;
    //=====================================================================
    //= OOptionGroupLayouter
    //=====================================================================
    class OOptionGroupLayouter
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                    m_xORB;

    public:
        OOptionGroupLayouter(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
            );

    public:
        void    doLayout(
            const OControlWizardContext& _rContext,
            const OOptionGroupSettings& _rSettings
        );

    protected:
        void    implAnchorShape(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxShapeProps
            );
    };

//.........................................................................
}   // namespace dbp
//.........................................................................

#endif // _EXTENSIONS_DBP_OPTIONGROUPLAYOUTER_HXX_

