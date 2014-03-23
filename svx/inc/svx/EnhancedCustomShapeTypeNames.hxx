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



#ifndef _ENHANCED_CUSTOMSHAPE_TYPE_NAMES_HXX
#define _ENHANCED_CUSTOMSHAPE_TYPE_NAMES_HXX
#include <svx/msdffdef.hxx>
#include <rtl/ustring.hxx>

namespace EnhancedCustomShapeTypeNames
{
    SVX_DLLPUBLIC MSO_SPT Get( const rtl::OUString& );
    SVX_DLLPUBLIC rtl::OUString Get( const MSO_SPT );

    rtl::OUString GetAccName( const rtl::OUString& );
}

#endif
