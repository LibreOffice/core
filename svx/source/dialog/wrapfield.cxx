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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include "svx/wrapfield.hxx"

namespace svx {

// ============================================================================

WrapField::WrapField( Window* pParent, WinBits nWinStyle ) :
    NumericField( pParent, nWinStyle )
{
}

WrapField::WrapField( Window* pParent, const ResId& rResId ) :
    NumericField( pParent, rResId )
{
}

void WrapField::Up()
{
    SetValue( ((GetValue() + GetSpinSize() - GetMin()) % (GetMax() + 1)) + GetMin() );
    GetUpHdl().Call( this );
}

void WrapField::Down()
{
    SetValue( ((GetValue() - GetSpinSize() + GetMax() + 1 - GetMin()) % (GetMax() + 1)) + GetMin() );
    GetDownHdl().Call( this );
}

// ============================================================================

} // namespace svx

