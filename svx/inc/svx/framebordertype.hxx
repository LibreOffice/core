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



#ifndef SVX_FRAMEBORDERTYPE_HXX
#define SVX_FRAMEBORDERTYPE_HXX

#include "svx/svxdllapi.h"

namespace svx {

// ============================================================================

/** Enumerates all borders a frame selection control can contain. */
enum FrameBorderType
{
    FRAMEBORDER_NONE,       /// No border (special state).
    FRAMEBORDER_LEFT,       /// Left frame border.
    FRAMEBORDER_RIGHT,      /// Right frame border.
    FRAMEBORDER_TOP,        /// Top frame border.
    FRAMEBORDER_BOTTOM,     /// Bottom frame border.
    FRAMEBORDER_HOR,        /// Inner horizontal frame border.
    FRAMEBORDER_VER,        /// Inner vertical frame border.
    FRAMEBORDER_TLBR,       /// Top-left to bottom-right frame border.
    FRAMEBORDER_BLTR        /// Bottom-left to top-right frame border.
};

/** The number of valid frame border types (excluding FRAMEBORDER_NONE). */
const int FRAMEBORDERTYPE_COUNT = 8;

/** Returns the frame border type from a 0-based integer index. */
SVX_DLLPUBLIC FrameBorderType GetFrameBorderTypeFromIndex( size_t nIndex );

/** Returns the zero-based index of a valid frame border type. */
size_t GetIndexFromFrameBorderType( FrameBorderType eBorder );

// ============================================================================

} // namespace svx

#endif

