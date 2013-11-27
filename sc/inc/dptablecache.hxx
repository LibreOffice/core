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


#ifndef DPTABLECACHE_HXX
#define DPTABLECACHE_HXX
// Add Data Cache Support.
#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif
#include <svl/zforlist.hxx>
#include <vector>
#include "dpglobal.hxx"

#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>

class ScDPTableDataCache;
class TypedStrData;
struct ScQueryParam;

// --------------------------------------------------------------------
//
//  base class ScDPTableData to allow implementation with tabular data
//  by deriving only of this
//

class SC_DLLPUBLIC ScDPTableDataCache
{
    long    mnID;
    ScDocument* mpDoc;

    long                         mnColumnCount;     // Column count

    std::vector<ScDPItemData*>*      mpTableDataValues; //Data Pilot Table's index - value map
    std::vector<SCROW>*          mpSourceData;      //Data Pilot Table's Source data
    std::vector<SCROW>*          mpGlobalOrder;     //Sorted members index
    std::vector<SCROW>*          mpIndexOrder;      //Index the sorted number
    std::vector<ScDPItemData*>   mrLabelNames;      //Source Label data
    std::vector<sal_Bool>            mbEmptyRow;        //If empty row?
    mutable ScDPItemDataPool                 maAdditionalDatas;
public:
    SCROW GetOrder( long nDim, SCROW nIndex ) const;
    SCROW GetIdByItemData( long nDim,  String sItemData  ) const;
    SCROW GetIdByItemData( long nDim, const ScDPItemData& rData ) const;

    SCROW GetAdditionalItemID ( String sItemData );
    SCROW GetAdditionalItemID( const ScDPItemData& rData );

    SCCOL GetDimensionIndex( String sName) const;
    const ScDPItemData* GetSortedItemData( SCCOL nDim, SCROW nOrder ) const;
    sal_uLong GetNumType ( sal_uLong nFormat ) const;
    sal_uLong GetNumberFormat( long nDim ) const;
    sal_Bool  IsDateDimension( long nDim ) const ;
    sal_uLong GetDimNumType( SCCOL nDim) const;
    SCROW GetDimMemberCount( SCCOL nDim ) const;

    SCROW GetSortedItemDataId( SCCOL nDim, SCROW nOrder ) const;
    const std::vector<ScDPItemData*>& GetDimMemberValues( SCCOL nDim )const;
    void    SetId( long nId ){ mnID = nId;}
    void    AddRow( ScDPItemData* pRow, sal_uInt16 nCount );
    bool    InitFromDoc(  ScDocument* pDoc, const ScRange& rRange );
    bool InitFromDataBase (const  ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>& xRowSet, const Date& rNullDate);

    SCROW   GetRowCount() const;
    SCROW   GetItemDataId( sal_uInt16 nDim, SCROW nRow, sal_Bool bRepeatIfEmpty ) const;
    String  GetDimensionName( sal_uInt16 nColumn ) const;
    bool    IsEmptyMember( SCROW nRow, sal_uInt16 nColumn ) const;
    bool    IsRowEmpty( SCROW nRow ) const;
    bool    IsValid() const;
    bool    ValidQuery( SCROW nRow, const ScQueryParam& rQueryParam, sal_Bool* pSpecial );

    ScDocument* GetDoc() const;//ms-cache-core
    long GetColumnCount() const;
    long    GetId() const;

    const ScDPItemData* GetItemDataById( long nDim, SCROW nId ) const;

    sal_Bool operator== ( const ScDPTableDataCache& r ) const;

//construction
    ScDPTableDataCache( ScDocument* pDoc );
//deconstruction
    virtual ~ScDPTableDataCache();

protected:
private:
public:
    void        AddLabel( ScDPItemData* pData);
    sal_Bool    AddData( long nDim, ScDPItemData* itemData, bool bCheckDate = true );
};

#endif //DPTABLECACHE_HXX
