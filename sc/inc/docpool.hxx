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



#ifndef SC_SCDOCPOL_HXX
#define SC_SCDOCPOL_HXX

#include <svl/itempool.hxx>
#include "scdllapi.h"

class ScStyleSheet;

//------------------------------------------------------------------------

class SC_DLLPUBLIC ScDocumentPool: public SfxItemPool
{
    SfxPoolItem**   ppPoolDefaults;
    SfxItemPool*    pSecondary;
    static sal_uInt16*  pVersionMap1;
    static sal_uInt16*  pVersionMap2;
    static sal_uInt16*  pVersionMap3;
    static sal_uInt16*  pVersionMap4;
    static sal_uInt16*  pVersionMap5;
    static sal_uInt16*  pVersionMap6;
    static sal_uInt16*  pVersionMap7;
    static sal_uInt16*  pVersionMap8;
    static sal_uInt16*  pVersionMap9;
    static sal_uInt16*  pVersionMap10;
    static sal_uInt16*  pVersionMap11;

public:
            ScDocumentPool( SfxItemPool* pSecPool = NULL, sal_Bool bLoadRefCounts = sal_False );
protected:
            virtual ~ScDocumentPool();
public:

    virtual SfxItemPool*        Clone() const;
    virtual SfxMapUnit          GetMetric( sal_uInt16 nWhich ) const;

    virtual const SfxPoolItem&  Put( const SfxPoolItem&, sal_uInt16 nWhich = 0 );
    virtual void                Remove( const SfxPoolItem& );
    static void                 CheckRef( const SfxPoolItem& );

    void StyleDeleted( ScStyleSheet* pStyle );      // Loeschen von Vorlagen im Organizer
    void CellStyleCreated( const String& rName );
    virtual SfxItemPresentation     GetPresentation(
                                        const SfxPoolItem&  rItem,
                                        SfxItemPresentation ePresentation,
                                        SfxMapUnit          ePresentationMetric,
                                        String&             rText,
                                        const IntlWrapper* pIntl = 0 ) const;

    static void InitVersionMaps();
    static void DeleteVersionMaps();
};




#endif
