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

#ifndef _BGFX_MATRIX_B3DHOMMATRIXTOOLS_HXX
#define _BGFX_MATRIX_B3DHOMMATRIXTOOLS_HXX

#include <sal/types.h>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <com/sun/star/drawing/HomogenMatrix.hpp>

///////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace tools
    {
        /* tooling methods for converting API matrices (drawing::HomogenMatrix) to
           B3DHomMatrix. drawing::HomogenMatrix4 is not used by OOo
         */
        B3DHomMatrix UnoHomogenMatrixToB3DHomMatrix(
            const com::sun::star::drawing::HomogenMatrix rMatrixIn);
        void B3DHomMatrixToUnoHomogenMatrix(
            const B3DHomMatrix& rMatrixIn,
            com::sun::star::drawing::HomogenMatrix& rMatrixOut);

    } // end of namespace tools
} // end of namespace basegfx

///////////////////////////////////////////////////////////////////////////////

#endif /* _BGFX_MATRIX_B3DHOMMATRIXTOOLS_HXX */
