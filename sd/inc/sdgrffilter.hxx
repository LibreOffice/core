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



#ifndef _SD_SDGRFFILTER_HXX
#define _SD_SDGRFFILTER_HXX

#include <com/sun/star/drawing/XShape.hpp>

#include <tools/errinf.hxx>
#include "sdfilter.hxx"

class Graphic;

// ---------------
// - SdCGMFilter -
// ---------------

class SdGRFFilter : public SdFilter
{
public:
    SdGRFFilter ( SfxMedium& rMedium, ::sd::DrawDocShell& rDocShell );

    virtual ~SdGRFFilter (void);

    sal_Bool        Import();
    sal_Bool        Export();

    static void             SaveGraphic( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape );
    static void             HandleGraphicFilterError( sal_uInt16 nFilterError, sal_uLong nStreamError = ERRCODE_NONE );
};

#endif // _SD_SDGRFFILTER_HXX
