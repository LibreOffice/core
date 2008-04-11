/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ConceptData.cxx,v $
 * $Revision: 1.8 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmlhelp.hxx"
#include <qe/ConceptData.hxx>
#include <qe/Query.hxx>
#include <qe/DocGenerator.hxx>


using namespace xmlsearch::qe;


const sal_Int32 ConceptData::ProxPerTerm = 10;


ConceptData::ConceptData( sal_Int32 id,
                          sal_Int32 role,
                          double score,
                          sal_Int32 queryNo,
                          sal_Int32 nColumns,
                          ContextTables* contextTables )
    : queryNo_( sal_uInt8( queryNo & 0xF ) ),
      nColumns_( sal_uInt8( nColumns & 0xF ) ),
      role_( sal_uInt8( role & 0xF ) ),
      concept_( id ),
      proximity_( nColumns * ProxPerTerm ),
      penalty_( score ),
      m_nRefcount( 0 ),
      ctx_( contextTables ),
      next_( 0 )
{
}


ConceptData::~ConceptData()
{
}


void ConceptData::runBy( std::vector< Query* >& queries )
{
    rtl::Reference< ConceptData > cd( this );
    do
    {
        Query* query = queries[ cd->queryNo_ ];
        query->updateEstimate( cd->role_,cd->penalty_ );
    }
    while( (cd = cd->next_).is() );
}


void ConceptData::addLast( ConceptData* r )
{
    if( next_.is() )
        next_->addLast( r );
    else
        next_ = r;
}


void ConceptData::generateFillers( std::vector< RoleFiller* >& array, sal_Int32 pos )
{
    if( array[ queryNo_ ] != RoleFiller::STOP() ) // not 'prohibited'
    {
        sal_Int32 wcl = ctx_->wordContextLin( pos );
        roleFillers_.push_back( new RoleFiller( nColumns_,
                                                this,
                                                role_,
                                                pos,
                                                wcl,
                                                pos + proximity_ ) );
        roleFillers_.back()->use( array, queryNo_ );
    }
    // !!! maybe eliminate tail recursion
    if( next_.is() )
        next_->generateFillers( array,pos );
}
