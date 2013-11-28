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


#ifndef _DOCTXM_HXX
#define _DOCTXM_HXX


#include <svl/svarray.hxx>
#include <tools/gen.hxx>
#include <tox.hxx>
#include <section.hxx>

class  SwTOXInternational;
class  SvUShorts;
class  SvStringsDtor;
class  SvPtrarr;
class  SwPageDesc;
class  SwTxtNode;
class  SwTxtFmtColl;
struct SwPosition;
struct SwTOXSortTabBase;

typedef SwTOXSortTabBase* SwTOXSortTabBasePtr;

SV_DECL_PTRARR(SwTOXSortTabBases, SwTOXSortTabBasePtr, 0, 5 )

/*--------------------------------------------------------------------
     Beschreibung: Ring der speziellen Verzeichnisse
 --------------------------------------------------------------------*/

class SwTOXBaseSection : public SwTOXBase, public SwSection
{
    SwTOXSortTabBases aSortArr;

    void    UpdateMarks( const SwTOXInternational& rIntl,
                            const SwTxtNode* pOwnChapterNode );
    void    UpdateOutline( const SwTxtNode* pOwnChapterNode );
    void    UpdateTemplate( const SwTxtNode* pOwnChapterNode );
    void    UpdateCntnt( SwTOXElement eType,
                            const SwTxtNode* pOwnChapterNode );
    void    UpdateTable( const SwTxtNode* pOwnChapterNode );
    void    UpdateSequence( const SwTxtNode* pOwnChapterNode );
    void    UpdateAuthorities( const SwTOXInternational& rIntl );
    void    UpdateAll();

    // Sortiert einfuegen ins Array fuer die Generierung
    void    InsertSorted(SwTOXSortTabBase* pBase);

    // Alpha-Trennzeichen bei der Generierung einfuegen
    void    InsertAlphaDelimitter( const SwTOXInternational& rIntl );

    // Textrumpf generieren
    // OD 18.03.2003 #106329# - add parameter <_TOXSectNdIdx> and <_pDefaultPageDesc>
    void GenerateText( sal_uInt16 nArrayIdx,
                       sal_uInt16 nCount,
                       SvStringsDtor&,
                       const sal_uInt32   _nTOXSectNdIdx,
                       const SwPageDesc*  _pDefaultPageDesc );

    // Seitennummerplatzhalter gegen aktuelle Nummern austauschen
    void    _UpdatePageNum( SwTxtNode* pNd,
                            const SvUShorts& rNums,
                            const SvPtrarr &rDescs,
                            const SvUShorts* pMainEntryNums,
                            const SwTOXInternational& rIntl );

    // Bereich fuer Stichwort einfuegen suchen
    Range GetKeyRange( const String& rStr, const String& rStrReading,
                       const SwTOXSortTabBase& rNew, sal_uInt16 nLevel,
                       const Range& rRange );

    // returne die TextCollection ueber den Namen / aus Format-Pool
    SwTxtFmtColl* GetTxtFmtColl( sal_uInt16 nLevel );

public:
    SwTOXBaseSection(SwTOXBase const& rBase, SwSectionFmt & rFmt);
    virtual ~SwTOXBaseSection();

    // OD 19.03.2003 #106329# - add parameter <_bNewTOX> in order to distinguish
    // between the creation of a new table-of-content or an update of
    // a table-of-content. Default value: false
    void Update( const SfxItemSet* pAttr = 0,
                 const bool        _bNewTOX = false ); // Formatieren
    void UpdatePageNum();               // Seitennummern einfuegen
    TYPEINFO();                         // fuers rtti
    SwTOXSortTabBases* GetTOXSortTabBases() { return &aSortArr; }

    sal_Bool SetPosAtStartEnd( SwPosition& rPos, sal_Bool bAtStart = sal_True ) const;
};
/* -----------------02.09.99 07:52-------------------

 --------------------------------------------------*/
struct SwDefTOXBase_Impl
{
    SwTOXBase* pContBase;
    SwTOXBase* pIdxBase;
    SwTOXBase* pUserBase;
    SwTOXBase* pTblBase;
    SwTOXBase* pObjBase;
    SwTOXBase* pIllBase;
    SwTOXBase* pAuthBase;

    SwDefTOXBase_Impl() :
    pContBase(0),
    pIdxBase(0),
    pUserBase(0),
    pTblBase(0),
    pObjBase(0),
    pIllBase(0),
    pAuthBase(0)
    {}
    ~SwDefTOXBase_Impl()
    {
        delete pContBase;
        delete pIdxBase;
        delete pUserBase;
        delete pTblBase;
        delete pObjBase;
        delete pIllBase;
        delete pAuthBase;
    }

};

#endif  // _DOCTXM_HXX
