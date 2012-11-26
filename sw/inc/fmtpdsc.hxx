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


#ifndef _FMTPDSC_HXX
#define _FMTPDSC_HXX


#include <svl/poolitem.hxx>
#include "swdllapi.h"
#include <hintids.hxx>
#include <format.hxx>
#include <calbck.hxx>

class SwPageDesc;
class SwHistory;
class SwPaM;
class IntlWrapper;
class SwEndNoteInfo;

//Pagedescriptor
//Client vom SwPageDesc der durch das Attribut "beschrieben" wird.

#define IVER_FMTPAGEDESC_NOAUTO ((sal_uInt16)0x0001)
#define IVER_FMTPAGEDESC_LONGPAGE   ((sal_uInt16)0x0002)

class SW_DLLPUBLIC SwFmtPageDesc : public SfxPoolItem, public SwClient
{
    // diese "Doc"-Funktion ist friend, um nach dem kopieren das
    // Auto-Flag setzen zu koennen !!
    friend sal_Bool InsAttr( SwDoc*, const SwPaM &, const SfxItemSet&, sal_uInt16,
                        SwHistory* );
    sal_uInt16 nNumOffset;          // Seitennummer Offset
    sal_uInt16 nDescNameIdx;        // SW3-Reader: Stringpool-Index des Vorlagennamens
    SwModify* pDefinedIn;       // Verweis auf das Objekt, in dem das
                                // Attribut gesetzt wurde (CntntNode/Format)
protected:
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew );
    virtual void SwClientNotify( const SwModify&, const SfxHint& rHint );

public:
    SwFmtPageDesc( const SwPageDesc *pDesc = 0 );
    SwFmtPageDesc( const SwFmtPageDesc &rCpy );
    SwFmtPageDesc &operator=( const SwFmtPageDesc &rCpy );
    ~SwFmtPageDesc();

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;
    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

          SwPageDesc *GetPageDesc() { return (SwPageDesc*)GetRegisteredIn(); }
    const SwPageDesc *GetPageDesc() const { return (SwPageDesc*)GetRegisteredIn(); }

    sal_uInt16  GetNumOffset() const        { return nNumOffset; }
    void    SetNumOffset( sal_uInt16 nNum ) { nNumOffset = nNum; }

    // erfrage/setze, wo drin das Attribut verankert ist
    inline const SwModify* GetDefinedIn() const { return pDefinedIn; }
    void ChgDefinedIn( const SwModify* pNew ) { pDefinedIn = (SwModify*)pNew; }
    void RegisterToEndNotInfo( SwEndNoteInfo& );
    void RegisterToPageDesc( SwPageDesc& );
    bool KnowsPageDesc() const;
};


inline const SwFmtPageDesc &SwAttrSet::GetPageDesc(sal_Bool bInP) const
    { return (const SwFmtPageDesc&)Get( RES_PAGEDESC,bInP); }

inline const SwFmtPageDesc &SwFmt::GetPageDesc(sal_Bool bInP) const
    { return aSet.GetPageDesc(bInP); }

#endif

