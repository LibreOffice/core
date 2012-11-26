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


#ifndef _SFXMETRICITEM_HXX
#define _SFXMETRICITEM_HXX

#include "svl/svldllapi.h"
#include <svl/intitem.hxx>

DBG_NAMEEX_VISIBILITY(SfxMetricItem, SVL_DLLPUBLIC)

// -----------------------------------------------------------------------

class SVL_DLLPUBLIC SfxMetricItem: public SfxInt32Item
{
public:
                             SfxMetricItem( sal_uInt16 nWhich = 0, sal_uInt32 nValue = 0 );
                             SfxMetricItem( sal_uInt16 nWhich, SvStream & );
                             SfxMetricItem( const SfxMetricItem& );
                             ~SfxMetricItem() {
                                 DBG_DTOR(SfxMetricItem, 0); }

    virtual void             ScaleMetrics( long lMult, long lDiv );
    virtual bool             HasMetrics() const;

};

#endif

