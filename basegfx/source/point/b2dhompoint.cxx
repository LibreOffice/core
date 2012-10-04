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
#include <basegfx/point/b2dhompoint.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>

namespace basegfx
{
    void B2DHomPoint::implHomogenize()
    {
        const double fFactor(1.0 / mfW);
        maTuple.setX(maTuple.getX() * fFactor);
        maTuple.setY(maTuple.getY() * fFactor);
        mfW = 1.0;
    }

    B2DHomPoint& B2DHomPoint::operator*=( const B2DHomMatrix& rMat )
    {
        const double fTempX( rMat.get(0,0)*maTuple.getX() +
                            rMat.get(0,1)*maTuple.getY() +
                            rMat.get(0,2)*mfW );

        const double fTempY( rMat.get(1,0)*maTuple.getX() +
                            rMat.get(1,1)*maTuple.getY() +
                            rMat.get(1,2)*mfW );

        const double fTempZ( rMat.get(2,0)*maTuple.getX() +
                            rMat.get(2,1)*maTuple.getY() +
                            rMat.get(2,2)*mfW );
        maTuple.setX( fTempX );
        maTuple.setY( fTempY );
        mfW = fTempZ;

        return *this;
    }
} // end of namespace basegfx

// eof
