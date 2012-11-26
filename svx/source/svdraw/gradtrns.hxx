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



#ifndef _GRADTRANS_HXX
#define _GRADTRANS_HXX

#include <svx/xgrad.hxx>
#include <tools/gen.hxx>
#include <basegfx/point/b2dpoint.hxx>

class SdrObject;

class GradTransVector
{
public:
    basegfx::B2DPoint           maPositionA;
    basegfx::B2DPoint           maPositionB;
    Color                       aCol1;
    Color                       aCol2;
};

class GradTransGradient
{
public:
    XGradient                   aGradient;
};

class GradTransformer
{
public:
    GradTransformer() {}

    void GradToVec(
        GradTransGradient& rG,
        GradTransVector& rV,
        const SdrObject* pObj);

    void VecToGrad(
        GradTransVector& rV,
        GradTransGradient& rG,
        GradTransGradient& rGOld,
        const SdrObject* pObj,
        bool bMoveSingle,
        bool bMoveFirst);
};

#endif // _GRADTRANS_HXX

