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



#ifndef SC_EXCIMP8_HXX
#define SC_EXCIMP8_HXX

#include <string.h>
#include "imp_op.hxx"
#include "root.hxx"
#include "excscen.hxx"
#include "excdefs.hxx"
#include "ftools.hxx"
#include "queryparam.hxx"

class SotStorage;

class ScBaseCell;
class ScRangeList;
class ScDBData;

class ScfSimpleProgressBar;

class XclImpStream;



class ImportExcel8 : public ImportExcel
{
public:
                            ImportExcel8( XclImpRootData& rImpData, SvStream& rStrm );
    virtual                 ~ImportExcel8( void );

    virtual FltError        Read( void );

protected:
    ExcScenarioList         aScenList;

    void                    Calccount( void );              // 0x0C
    void                    Precision( void );              // 0x0E
    void                    Delta( void );                  // 0x10
    void                    Iteration( void );              // 0x11
    void                    Boundsheet( void );             // 0x85
    void                    FilterMode( void );             // 0x9B
    void                    AutoFilterInfo( void );         // 0x9D
    void                    AutoFilter( void );             // 0x9E
    void                    Scenman( void );                // 0xAE
    void                    Scenario( void );               // 0xAF
    void                    ReadBasic( void );              // 0xD3
    void                    Labelsst( void );               // 0xFD

    void                    Hlink( void );                  // 0x01B8
    void                    Codename( sal_Bool bWBGlobals );    // 0x01BA
    void                    SheetProtection( void );        // 0x0867

    virtual void            EndSheet( void );
    virtual void            PostDocLoad( void );

private:
    void                    LoadDocumentProperties();
};



//___________________________________________________________________
// classes AutoFilterData, AutoFilterBuffer

class XclImpAutoFilterData : private ExcRoot
{
private:
    ScDBData*                   pCurrDBData;
    ScQueryParam                aParam;
    SCSIZE                      nFirstEmpty;
    sal_Bool                        bActive;
    sal_Bool                        bHasConflict;
    sal_Bool                        bCriteria;
    sal_Bool                        bAutoOrAdvanced;
    ScRange                     aCriteriaRange;
    String                      aFilterName;

    void                        CreateFromDouble( String& rStr, double fVal );
    void                        SetCellAttribs();
    void                        InsertQueryParam();
    void                        AmendAFName(const sal_Bool bUseUnNamed);

protected:
public:
                                XclImpAutoFilterData(
                                    RootData* pRoot,
                                    const ScRange& rRange,
                                    const String& rName );

    inline bool                 IsActive() const    { return bActive; }
    inline bool                 IsFiltered() const  { return bAutoOrAdvanced; }
    inline SCTAB                Tab() const         { return aParam.nTab; }
    inline SCCOL                StartCol() const    { return aParam.nCol1; }
    inline SCROW                StartRow() const    { return aParam.nRow1; }
    inline SCCOL                EndCol() const      { return aParam.nCol2; }
    inline SCROW                EndRow() const      { return aParam.nRow2; }

    void                        ReadAutoFilter( XclImpStream& rStrm );

    inline void                 Activate()          { bActive = sal_True; }
    void                        SetAdvancedRange( const ScRange* pRange );
    void                        SetExtractPos( const ScAddress& rAddr );
    inline void                 SetAutoOrAdvanced()  { bAutoOrAdvanced = sal_True; }
    void                        Apply( const sal_Bool bUseUnNamed = sal_False );
    void                        CreateScDBData( const sal_Bool bUseUnNamed );
    void                        EnableRemoveFilter();
};


class XclImpAutoFilterBuffer : private List
{
private:
    using                       List::Insert;

    sal_uInt16                      nAFActiveCount;

    inline XclImpAutoFilterData* _First()   { return (XclImpAutoFilterData*) List::First(); }
    inline XclImpAutoFilterData* _Next()    { return (XclImpAutoFilterData*) List::Next(); }

    inline void                 Append( XclImpAutoFilterData* pData )
                                    { List::Insert( pData, LIST_APPEND ); }
protected:
public:
                                XclImpAutoFilterBuffer();
    virtual                     ~XclImpAutoFilterBuffer();

    void                        Insert( RootData* pRoot, const ScRange& rRange,
                                                const String& rName );
    void                        AddAdvancedRange( const ScRange& rRange );
    void                        AddExtractPos( const ScRange& rRange );
    void                        Apply();

    XclImpAutoFilterData*       GetByTab( SCTAB nTab );
    inline void                 IncrementActiveAF() { nAFActiveCount++; }
    inline sal_Bool                 UseUnNamed() { return nAFActiveCount == 1; }
};

#endif

