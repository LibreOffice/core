/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: printopt.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:49:04 $
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

#ifndef SC_PRINTOPT_HXX
#define SC_PRINTOPT_HXX

#ifndef _SFXPOOLITEM_HXX
#include <svtools/poolitem.hxx>
#endif

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif

#ifndef INCLUDED_SCDLLAPI_H
#include "scdllapi.h"
#endif

class SC_DLLPUBLIC ScPrintOptions
{
private:
    BOOL    bSkipEmpty;
    BOOL    bAllSheets;

public:
                ScPrintOptions();
                ScPrintOptions( const ScPrintOptions& rCpy );
                ~ScPrintOptions();

    BOOL    GetSkipEmpty() const            { return bSkipEmpty; }
    void    SetSkipEmpty( BOOL bVal )       { bSkipEmpty = bVal; }
    BOOL    GetAllSheets() const            { return bAllSheets; }
    void    SetAllSheets( BOOL bVal )       { bAllSheets = bVal; }

    void    SetDefaults();

    const ScPrintOptions&   operator=  ( const ScPrintOptions& rCpy );
    int                     operator== ( const ScPrintOptions& rOpt ) const;
    int                     operator!= ( const ScPrintOptions& rOpt ) const;
};

//==================================================================
// item for the dialog / options page
//==================================================================

class SC_DLLPUBLIC ScTpPrintItem : public SfxPoolItem
{
public:
                TYPEINFO();
                ScTpPrintItem( USHORT nWhich );
                ScTpPrintItem( USHORT nWhich,
                               const ScPrintOptions& rOpt );
                ScTpPrintItem( const ScTpPrintItem& rItem );
                ~ScTpPrintItem();

    virtual String          GetValueText() const;
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

    const ScPrintOptions&   GetPrintOptions() const { return theOptions; }

private:
    ScPrintOptions theOptions;
};

//==================================================================
// config item
//==================================================================

class ScPrintCfg : public ScPrintOptions, public utl::ConfigItem
{
    com::sun::star::uno::Sequence<rtl::OUString> GetPropertyNames();

public:
            ScPrintCfg();

    void            SetOptions( const ScPrintOptions& rNew );
    void            OptionsChanged();   // after direct access to ScPrintOptions base class

    virtual void    Commit();
};

#endif
