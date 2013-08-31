/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
    bool                    operator== ( const ScPrintOptions& rOpt ) const;
    bool                    operator!= ( const ScPrintOptions& rOpt ) const;
};

// item for the dialog / options page

class SC_DLLPUBLIC ScTpPrintItem : public SfxPoolItem
{
public:
                TYPEINFO();
                ScTpPrintItem( sal_uInt16 nWhich,
                               const ScPrintOptions& rOpt );
                ScTpPrintItem( const ScTpPrintItem& rItem );
                ~ScTpPrintItem();

    virtual OUString        GetValueText() const;
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

    const ScPrintOptions&   GetPrintOptions() const { return theOptions; }

private:
    ScPrintOptions theOptions;
};

// config item

class ScPrintCfg : public ScPrintOptions, public utl::ConfigItem
{
    com::sun::star::uno::Sequence<OUString> GetPropertyNames();

public:
            ScPrintCfg();

    void            SetOptions( const ScPrintOptions& rNew );

    virtual void    Commit();
    virtual void Notify( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
