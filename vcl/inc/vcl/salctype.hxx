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



#ifndef _SV_SALCTYPE_HXX
#define _SV_SALCTYPE_HXX

#include <com/sun/star/script/XInvocation.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <vcl/graph.hxx>

// -----------
// - Defines -
// -----------

#define CVT_UNKNOWN (0x00000000UL)
#define CVT_BMP     (0x00000001UL)
#define CVT_GIF     (0x00000002UL)
#define CVT_JPG     (0x00000003UL)
#define CVT_MET     (0x00000004UL)
#define CVT_PCT     (0x00000005UL)
#define CVT_PNG     (0x00000006UL)
#define CVT_SVM     (0x00000007UL)
#define CVT_TIF     (0x00000008UL)
#define CVT_WMF     (0x00000009UL)
#define CVT_EMF     (0x0000000aUL)
#define CVT_SVG     (0x0000000bUL)

// ---------------
// - ConvertData -
// ---------------

class SvStream;

struct ConvertData
{
private:

                        ConvertData();

public:

    Graphic             maGraphic;
    SvStream&           mrStm;
    sal_uLong               mnFormat;

                        ConvertData( const Graphic& rGraphic, SvStream& rStm, sal_uLong nFormat ) :
                            maGraphic( rGraphic ), mrStm( rStm ), mnFormat( nFormat ) {}
                        ~ConvertData() {}
};

// ------------
// - Callback -
// ------------

typedef sal_uLong (*SALGRFCVTPROC)( void* pInst,
                                sal_uLong nInFormat, void* pInBuffer, sal_uLong nInBufSize,
                                sal_uLong nOutFormat, void** ppOutBuffer );

// -------------------
// - BitmapConverter -
// -------------------

namespace vcl
{
com::sun::star::uno::Reference< com::sun::star::script::XInvocation > createBmpConverter();
}

#endif // _SV_SALCTYPE_HXX
