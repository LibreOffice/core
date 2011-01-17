/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SC_PRINTOPT_HXX
#define SC_PRINTOPT_HXX

#include <svl/poolitem.hxx>
#include <unotools/configitem.hxx>
#include "scdllapi.h"

class SC_DLLPUBLIC ScPrintOptions
{
private:
    sal_Bool    bSkipEmpty;
    sal_Bool    bAllSheets;

public:
                ScPrintOptions();
                ScPrintOptions( const ScPrintOptions& rCpy );
                ~ScPrintOptions();

    sal_Bool    GetSkipEmpty() const            { return bSkipEmpty; }
    void    SetSkipEmpty( sal_Bool bVal )       { bSkipEmpty = bVal; }
    sal_Bool    GetAllSheets() const            { return bAllSheets; }
    void    SetAllSheets( sal_Bool bVal )       { bAllSheets = bVal; }

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
//UNUSED2008-05  ScTpPrintItem( sal_uInt16 nWhich );
                ScTpPrintItem( sal_uInt16 nWhich,
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

    virtual void    Commit();
    virtual void Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
};

#endif
