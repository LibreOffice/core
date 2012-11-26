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



#ifndef _SVX_CFGCHART_HXX
#define _SVX_CFGCHART_HXX

// header for ConfigItem
#include <unotools/configitem.hxx>
// header for SfxPoolItem
#include <svl/poolitem.hxx>
// header for XColorEntry
#include <svx/xtable.hxx>

#include <vector>

class SvxChartColorTable
{
private:
    ::std::vector< XColorEntry >     m_aColorEntries;

public:
    SvxChartColorTable();
    explicit SvxChartColorTable( const SvxChartColorTable & _rSource );

    // accessors
    size_t size() const;
    const XColorEntry & operator[]( size_t _nIndex ) const;
    ColorData getColorData( size_t _nIndex ) const;

    // mutators
    void clear();
    void append( const XColorEntry & _rEntry );
    void replace( size_t _nIndex, const XColorEntry & _rEntry );
    void useDefault();

    // comparison
    bool operator==( const SvxChartColorTable & _rOther ) const;
};

// ====================
// all options
// ====================
class SvxChartOptions : public ::utl::ConfigItem
{
private:
    SvxChartColorTable      maDefColors;
    sal_Bool                    mbIsInitialized;

    ::com::sun::star::uno::Sequence< ::rtl::OUString >
                            maPropertyNames;

    inline ::com::sun::star::uno::Sequence< ::rtl::OUString > GetPropertyNames() const
        { return maPropertyNames; }
    sal_Bool RetrieveOptions();

public:
    SvxChartOptions();
    virtual ~SvxChartOptions();

    const SvxChartColorTable&   GetDefaultColors();
    void                        SetDefaultColors( const SvxChartColorTable& aCol );

    virtual void                Commit();
    virtual void Notify( const com::sun::star::uno::Sequence< rtl::OUString >& _rPropertyNames);
};

// ====================
// items
// ====================
class SvxChartColorTableItem : public SfxPoolItem
{
public:
    SvxChartColorTableItem( sal_uInt16 nWhich, const SvxChartColorTable& );
    SvxChartColorTableItem( const SvxChartColorTableItem& );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;
    void                    SetOptions( SvxChartOptions* pOpts ) const;

    const SvxChartColorTable & GetColorTable() const ;
    SvxChartColorTable &       GetColorTable();
    void                    ReplaceColorByIndex( size_t _nIndex, const XColorEntry & _rEntry );

private:
    SvxChartColorTable      m_aColorTable;
};

#endif  // _SVX_CFGCHART_HXX

