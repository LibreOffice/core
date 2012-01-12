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



#ifndef _SV_SALGEOM_HXX
#define _SV_SALGEOM_HXX

typedef struct _SalFrameGeometry {
    // screen position of upper left corner of drawable area in pixel
    long                nX, nY;
    // dimensions of the drawable area in pixel
    unsigned long       nWidth, nHeight;
    // thickness of the decoration in pixel
    unsigned long       nLeftDecoration,
                        nTopDecoration,
                        nRightDecoration,
                        nBottomDecoration;
    unsigned int        nScreenNumber;

    _SalFrameGeometry() :
    nX( 0 ), nY( 0 ), nWidth( 1 ), nHeight( 1 ),
    nLeftDecoration( 0 ), nTopDecoration( 0 ),
    nRightDecoration( 0 ), nBottomDecoration( 0 ),
    nScreenNumber( 0 )
    {}
} SalFrameGeometry;

#endif // _SV_SALGEOM_HXX
