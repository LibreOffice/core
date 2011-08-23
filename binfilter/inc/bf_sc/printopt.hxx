/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <bf_svtools/poolitem.hxx>

#include <unotools/configitem.hxx>
namespace binfilter {


class ScPrintOptions
{
private:
    BOOL	bSkipEmpty;
    BOOL	bAllSheets;

public:
                ScPrintOptions();
                ScPrintOptions( const ScPrintOptions& rCpy );
                ~ScPrintOptions();

    BOOL	GetSkipEmpty() const			{ return bSkipEmpty; }
    void	SetSkipEmpty( BOOL bVal )		{ bSkipEmpty = bVal; }
    BOOL	GetAllSheets() const			{ return bAllSheets; }
    void	SetAllSheets( BOOL bVal )		{ bAllSheets = bVal; }

    void	SetDefaults();

};

//==================================================================
// item for the dialog / options page
//==================================================================


//==================================================================
// config item
//==================================================================

class ScPrintCfg : public ScPrintOptions, public ::utl::ConfigItem
{

public:
    ScPrintCfg();

    void			SetOptions( const ScPrintOptions& rNew ){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 	void			SetOptions( const ScPrintOptions& rNew );
    virtual void                Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
    virtual void                Commit();
};

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
