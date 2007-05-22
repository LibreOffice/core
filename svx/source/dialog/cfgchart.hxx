/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cfgchart.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 15:16:47 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SVX_CFGCHART_HXX
#define _SVX_CFGCHART_HXX

// header for TYPEINFO
#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif
// header for ConfigItem
#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
// header for SfxPoolItem
#ifndef _SFXPOOLITEM_HXX
#include <svtools/poolitem.hxx>
#endif
// header for XColorEntry
#ifndef _XTABLE_HXX
#include <xtable.hxx>
#endif

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
    BOOL                    mbIsInitialized;

    ::com::sun::star::uno::Sequence< ::rtl::OUString >
                            maPropertyNames;

    inline ::com::sun::star::uno::Sequence< ::rtl::OUString > GetPropertyNames() const
        { return maPropertyNames; }
    BOOL RetrieveOptions();

public:
    SvxChartOptions();
    virtual ~SvxChartOptions();

    const SvxChartColorTable&   GetDefaultColors();
    void                        SetDefaultColors( const SvxChartColorTable& aCol );

    virtual void                Commit();
};

// ====================
// items
// ====================
class SvxChartColorTableItem : public SfxPoolItem
{
public:
    TYPEINFO();
    SvxChartColorTableItem( USHORT nWhich, const SvxChartColorTable& );
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

