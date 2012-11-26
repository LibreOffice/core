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

#include <basegfx/matrix/b3dhommatrixtools.hxx>

///////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace tools
    {
        B3DHomMatrix UnoHomogenMatrixToB3DHomMatrix(
            const com::sun::star::drawing::HomogenMatrix rMatrixIn)
        {
            B3DHomMatrix aRetval;

            aRetval.set(0, 0, rMatrixIn.Line1.Column1);
            aRetval.set(0, 1, rMatrixIn.Line1.Column2);
            aRetval.set(0, 2, rMatrixIn.Line1.Column3);
            aRetval.set(0, 3, rMatrixIn.Line1.Column4);
            aRetval.set(1, 0, rMatrixIn.Line2.Column1);
            aRetval.set(1, 1, rMatrixIn.Line2.Column2);
            aRetval.set(1, 2, rMatrixIn.Line2.Column3);
            aRetval.set(1, 3, rMatrixIn.Line2.Column4);
            aRetval.set(2, 0, rMatrixIn.Line3.Column1);
            aRetval.set(2, 1, rMatrixIn.Line3.Column2);
            aRetval.set(2, 2, rMatrixIn.Line3.Column3);
            aRetval.set(2, 3, rMatrixIn.Line3.Column4);
            aRetval.set(3, 0, rMatrixIn.Line4.Column1);
            aRetval.set(3, 1, rMatrixIn.Line4.Column2);
            aRetval.set(3, 2, rMatrixIn.Line4.Column3);
            aRetval.set(3, 3, rMatrixIn.Line4.Column4);

            return aRetval;
        }

        void B3DHomMatrixToUnoHomogenMatrix(
            const B3DHomMatrix& rMatrixIn,
            com::sun::star::drawing::HomogenMatrix& rMatrixOut)
        {
            rMatrixOut.Line1.Column1 = rMatrixIn.get(0, 0);
            rMatrixOut.Line1.Column2 = rMatrixIn.get(0, 1);
            rMatrixOut.Line1.Column3 = rMatrixIn.get(0, 2);
            rMatrixOut.Line1.Column4 = rMatrixIn.get(0, 3);
            rMatrixOut.Line2.Column1 = rMatrixIn.get(1, 0);
            rMatrixOut.Line2.Column2 = rMatrixIn.get(1, 1);
            rMatrixOut.Line2.Column3 = rMatrixIn.get(1, 2);
            rMatrixOut.Line2.Column4 = rMatrixIn.get(1, 3);
            rMatrixOut.Line3.Column1 = rMatrixIn.get(2, 0);
            rMatrixOut.Line3.Column2 = rMatrixIn.get(2, 1);
            rMatrixOut.Line3.Column3 = rMatrixIn.get(2, 2);
            rMatrixOut.Line3.Column4 = rMatrixIn.get(2, 3);
            rMatrixOut.Line4.Column1 = rMatrixIn.get(3, 0);
            rMatrixOut.Line4.Column2 = rMatrixIn.get(3, 1);
            rMatrixOut.Line4.Column3 = rMatrixIn.get(3, 2);
            rMatrixOut.Line4.Column4 = rMatrixIn.get(3, 3);
        }

    } // end of namespace tools
} // end of namespace basegfx

///////////////////////////////////////////////////////////////////////////////
// eof
