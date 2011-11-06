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


#ifndef EXTENSIONS_SOURCE_PROPCTRLR_CONTROLTYPE_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_CONTROLTYPE_HXX

#include <sal/types.h>

//........................................................................
namespace pcr
{
//........................................................................

    //====================================================================
    //= control types, analogous to FormComponentType
    //====================================================================
    namespace ControlType
    {
        static const sal_Int16 FIXEDLINE      = (sal_Int16)100;
        static const sal_Int16 FORMATTEDFIELD = (sal_Int16)101;
        static const sal_Int16 PROGRESSBAR    = (sal_Int16)102;

        // need only those which are not already covered as FormComponentType
    }

//........................................................................
} // namespacepcr
//........................................................................

#endif // EXTENSIONS_SOURCE_PROPCTRLR_CONTROLTYPE_HXX

