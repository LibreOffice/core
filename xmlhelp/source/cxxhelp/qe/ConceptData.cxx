/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ConceptData.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 00:40:46 $
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
#ifndef _XMLSEARCH_QE_CONCEPTDATA_HXX_
#include <qe/ConceptData.hxx>
#endif
#ifndef _XMLSEARCH_QE_QUERY_HXX_
#include <qe/Query.hxx>
#endif
#ifndef _XMLSEARCH_QE_DOCGENERATOR_HXX_
#include <qe/DocGenerator.hxx>
#endif


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
