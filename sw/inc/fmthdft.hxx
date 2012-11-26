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


#ifndef _FMTHDFT_HXX
#define _FMTHDFT_HXX

#include <hintids.hxx>
#include <format.hxx>
#include <svl/poolitem.hxx>
#include <calbck.hxx>

class SwFrmFmt;
class IntlWrapper;
class SwFmt;

//Kopfzeile, fuer Seitenformate
//Client von FrmFmt das den Header beschreibt.

class SW_DLLPUBLIC SwFmtHeader: public SfxPoolItem, public SwClient
{
    sal_Bool bActive;       //Nur zur Steuerung (Erzeugung des Inhaltes)

public:
    SwFmtHeader( sal_Bool bOn = sal_False );
    SwFmtHeader( SwFrmFmt *pHeaderFmt );
    SwFmtHeader( const SwFmtHeader &rCpy );
    ~SwFmtHeader();
    SwFmtHeader& operator=( const SwFmtHeader &rCpy );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;

    const SwFrmFmt *GetHeaderFmt() const { return (SwFrmFmt*)GetRegisteredIn(); }
          SwFrmFmt *GetHeaderFmt()       { return (SwFrmFmt*)GetRegisteredIn(); }

    void RegisterToFormat( SwFmt& rFmt );
    sal_Bool IsActive() const { return bActive; }
    void SetActive( sal_Bool bNew = sal_True ) { bActive = bNew; }
};


//Fusszeile, fuer Seitenformate
//Client von FrmFmt das den Footer beschreibt.

class SW_DLLPUBLIC SwFmtFooter: public SfxPoolItem, public SwClient
{
    sal_Bool bActive;       //Nur zur Steuerung (Erzeugung des Inhaltes)

public:
    SwFmtFooter( sal_Bool bOn = sal_False );
    SwFmtFooter( SwFrmFmt *pFooterFmt );
    SwFmtFooter( const SwFmtFooter &rCpy );
    ~SwFmtFooter();
    SwFmtFooter& operator=( const SwFmtFooter &rCpy );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;

    const SwFrmFmt *GetFooterFmt() const { return (SwFrmFmt*)GetRegisteredIn(); }
          SwFrmFmt *GetFooterFmt()       { return (SwFrmFmt*)GetRegisteredIn(); }

    void RegisterToFormat( SwFmt& rFmt );
    sal_Bool IsActive() const { return bActive; }
    void SetActive( sal_Bool bNew = sal_True ) { bActive = bNew; }
};

inline const SwFmtHeader &SwAttrSet::GetHeader(sal_Bool bInP) const
    { return (const SwFmtHeader&)Get( RES_HEADER,bInP); }
inline const SwFmtFooter &SwAttrSet::GetFooter(sal_Bool bInP) const
    { return (const SwFmtFooter&)Get( RES_FOOTER,bInP); }

inline const SwFmtHeader &SwFmt::GetHeader(sal_Bool bInP) const
    { return aSet.GetHeader(bInP); }
inline const SwFmtFooter &SwFmt::GetFooter(sal_Bool bInP) const
    { return aSet.GetFooter(bInP); }

#endif

