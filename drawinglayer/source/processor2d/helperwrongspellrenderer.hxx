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



#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_HELPERWRONGSPELLRENDER_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR2D_HELPERWRONGSPELLRENDER_HXX

#include <sal/types.h>

//////////////////////////////////////////////////////////////////////////////
// predefines

class OutputDevice;

namespace drawinglayer { namespace primitive2d {
    class WrongSpellPrimitive2D;
}}

namespace basegfx {
    class B2DHomMatrix;
    class BColorModifierStack;
}

//////////////////////////////////////////////////////////////////////////////
// support WrongSpell rendreing using VCL from primitives due to VCLs nice
// and fast solution with wavelines

namespace drawinglayer
{
    bool renderWrongSpellPrimitive2D(
        const primitive2d::WrongSpellPrimitive2D& rWrongSpellCandidate,
        OutputDevice& rOutputDevice,
        const basegfx::B2DHomMatrix& rObjectToViewTransformation,
        const basegfx::BColorModifierStack& rBColorModifierStack);

} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_DRAWINGLAYER_PROCESSOR2D_HELPERWRONGSPELLRENDER_HXX

// eof
