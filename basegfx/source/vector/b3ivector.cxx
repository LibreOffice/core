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
#include "precompiled_basegfx.hxx"
#include <basegfx/vector/b3ivector.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>

namespace basegfx
{
    B3IVector& B3IVector::operator*=( const B3DHomMatrix& rMat )
    {
        mnX = fround( rMat.get(0,0)*mnX + rMat.get(0,1)*mnY + rMat.get(0,2)*mnZ );
        mnY = fround( rMat.get(1,0)*mnX + rMat.get(1,1)*mnY + rMat.get(1,2)*mnZ );
        mnZ = fround( rMat.get(2,0)*mnX + rMat.get(2,1)*mnY + rMat.get(2,2)*mnZ );

        return *this;
    }

    B3IVector operator*( const B3DHomMatrix& rMat, const B3IVector& rVec )
    {
        B3IVector aRes( rVec );
        return aRes*=rMat;
    }
} // end of namespace basegfx

// eof
