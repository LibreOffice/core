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


#ifndef _ENVIMG_HXX
#define _ENVIMG_HXX

#include <svl/poolitem.hxx>
#include <unotools/configitem.hxx>
#include "swdllapi.h"

// MakeSender -----------------------------------------------------------------

SW_DLLPUBLIC String MakeSender();

// enum -----------------------------------------------------------------------

enum SwEnvAlign
{
    ENV_HOR_LEFT = 0,
    ENV_HOR_CNTR,
    ENV_HOR_RGHT,
    ENV_VER_LEFT,
    ENV_VER_CNTR,
    ENV_VER_RGHT
};

// class SwEnvItem ------------------------------------------------------------

class SW_DLLPUBLIC SwEnvItem : public SfxPoolItem
{
public:

    rtl::OUString   aAddrText;       // Text fuer Empfaenger
    sal_Bool        bSend;           // Absender?
    rtl::OUString   aSendText;       // Text fuer Absender
    sal_Int32       lAddrFromLeft;   // Linker Abstand fuer Empfaenger (twips)
    sal_Int32       lAddrFromTop;    // Oberer Abstand fuer Empfaenger (twips)
    sal_Int32       lSendFromLeft;   // Linker Abstand fuer Absender   (twips)
    sal_Int32       lSendFromTop;    // Oberer Abstand fuer Absender   (twips)
    sal_Int32       lWidth;          // Breite des Umschlags           (twips)
    sal_Int32       lHeight;         // Hoehe  des Umschlags           (twips)
    SwEnvAlign      eAlign;          // Ausrichtung beim Einzug
    sal_Bool        bPrintFromAbove; // Von oben drucken?
    sal_Int32       lShiftRight;     // Verschiebung nach rechts       (twips)
    sal_Int32       lShiftDown;      // Verschiebung nach unten        (twips)

    POOLITEM_FACTORY()
    SwEnvItem();
    SwEnvItem(const SwEnvItem& rItem);

    SwEnvItem& operator =(const SwEnvItem& rItem);

    virtual int operator ==(const SfxPoolItem& rItem) const;

    virtual SfxPoolItem*     Clone(SfxItemPool* = 0) const;
    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
};

// class SwEnvCfgItem -------------------------------------------------------
class SwEnvCfgItem : public utl::ConfigItem
{
    SwEnvItem aEnvItem;

    com::sun::star::uno::Sequence<rtl::OUString> GetPropertyNames();
public:
    SwEnvCfgItem();
    ~SwEnvCfgItem();

    SwEnvItem& GetItem() {return aEnvItem;}

    virtual void            Commit();
    virtual void Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
};
#endif

