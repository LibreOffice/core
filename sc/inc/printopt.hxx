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

#ifndef INCLUDED_SC_INC_PRINTOPT_HXX
#define INCLUDED_SC_INC_PRINTOPT_HXX

#include <svl/poolitem.hxx>
#include <unotools/configitem.hxx>
#include "scdllapi.h"

class SC_DLLPUBLIC ScPrintOptions
{
private:
    bool    bSkipEmpty;
    bool    bAllSheets;
    bool    bForceBreaks;

public:
                ScPrintOptions();
                ScPrintOptions( const ScPrintOptions& rCpy );
                ~ScPrintOptions();

    bool    GetSkipEmpty() const            { return bSkipEmpty; }
    void    SetSkipEmpty( bool bVal )       { bSkipEmpty = bVal; }
    bool    GetAllSheets() const            { return bAllSheets; }
    void    SetAllSheets( bool bVal )       { bAllSheets = bVal; }
    bool    GetForceBreaks() const              { return bForceBreaks; }
    void    SetForceBreaks( bool bVal )     { bForceBreaks = bVal; }

    void    SetDefaults();

    const ScPrintOptions&   operator=  ( const ScPrintOptions& rCpy );
    bool                    operator== ( const ScPrintOptions& rOpt ) const;
    bool                    operator!= ( const ScPrintOptions& rOpt ) const;
};

// item for the dialog / options page

class SC_DLLPUBLIC ScTpPrintItem : public SfxPoolItem
{
public:
                TYPEINFO_OVERRIDE();
                ScTpPrintItem( sal_uInt16 nWhich,
                               const ScPrintOptions& rOpt );
                ScTpPrintItem( const ScTpPrintItem& rItem );
                virtual ~ScTpPrintItem();

    OUString        GetValueText() const;
    virtual bool            operator==( const SfxPoolItem& ) const SAL_OVERRIDE;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const SAL_OVERRIDE;

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

    virtual void    Commit() SAL_OVERRIDE;
    virtual void Notify( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
