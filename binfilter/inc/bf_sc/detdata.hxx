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

#ifndef SC_DETDATA_HXX
#define SC_DETDATA_HXX

#include <bf_svtools/svarray.hxx>

#include "global.hxx"
namespace binfilter {


//------------------------------------------------------------------------

#define SC_DETOP_GROW	4

//------------------------------------------------------------------------
enum ScDetOpType
{
    SCDETOP_ADDSUCC,
    SCDETOP_DELSUCC,
    SCDETOP_ADDPRED,
    SCDETOP_DELPRED,
    SCDETOP_ADDERROR
};

//------------------------------------------------------------------------

class ScDetOpData
{
    ScAddress		aPos;
    ScDetOpType		eOperation;

public:
                        ScDetOpData( const ScAddress& rP, ScDetOpType eOp ) :
                            aPos(rP), eOperation(eOp) {}

                        ScDetOpData( const ScDetOpData& rData ) :
                            aPos(rData.aPos), eOperation(rData.eOperation) {}

    const ScAddress&	GetPos() const			{ return aPos; }
    ScDetOpType			GetOperation() const	{ return eOperation; }

    // fuer UpdateRef:
    void				SetPos(const ScAddress& rNew)	{ aPos=rNew; }

    int operator==		( const ScDetOpData& r ) const
                            { return eOperation == r.eOperation && aPos == r.aPos; }
};

//------------------------------------------------------------------------

//
//	Liste der Operationen
//

typedef ScDetOpData* ScDetOpDataPtr;

SV_DECL_PTRARR_DEL(ScDetOpArr_Impl, ScDetOpDataPtr, SC_DETOP_GROW, SC_DETOP_GROW)//STRIP008 ;

class ScDetOpList : public ScDetOpArr_Impl
{
    BOOL	bHasAddError;		// updated in Append

public:
        ScDetOpList() : bHasAddError(FALSE) {}
        ScDetOpList(const ScDetOpList& rList);
        ~ScDetOpList() {}

/*N*/ 	void	UpdateReference( ScDocument* pDoc, UpdateRefMode eUpdateRefMode,
/*N*/ 								const ScRange& rRange, short nDx, short nDy, short nDz );


    void	Append( ScDetOpData* pData );

    void	Load( SvStream& rStream );
    void	Store( SvStream& rStream ) const;

    BOOL	HasAddError() const		{ return bHasAddError; }
};



} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
