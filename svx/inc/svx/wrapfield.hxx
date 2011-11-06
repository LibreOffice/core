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



#ifndef SVX_WRAPFIELD_HXX
#define SVX_WRAPFIELD_HXX

#include <vcl/field.hxx>
#include "svx/svxdllapi.h"

namespace svx {

// ============================================================================

/** A numeric spin field that wraps around the value on limits.
    @descr  Note: Use type "NumericField" in resources. */
class SVX_DLLPUBLIC WrapField : public NumericField
{
public:
    explicit            WrapField( Window* pParent, WinBits nWinStyle );
    explicit            WrapField( Window* pParent, const ResId& rResId );

protected:
    /** Up event with wrap-around functionality. */
    virtual void        Up();
    /** Down event with wrap-around functionality. */
    virtual void        Down();
};

// ============================================================================

} // namespace svx

#endif

