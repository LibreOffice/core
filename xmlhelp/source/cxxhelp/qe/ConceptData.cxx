/*************************************************************************
 *
 *  $RCSfile: ConceptData.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: abi $ $Date: 2001-06-18 12:10:12 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
      concept_( id ),
      proximity_( nColumns * ProxPerTerm ),
      role_( sal_uInt8( role & 0xF ) ),
      penalty_( score ),
      ctx_( contextTables ),
      next_( 0 )
{
}


void ConceptData::runBy( std::vector< Query* >& queries )
{
    ConceptData* cd = this;
    do
    {
        Query* query = queries[ cd->queryNo_ ];
        query->updateEstimate( cd->role_,cd->penalty_ );
    }
    while( cd = cd->next_ );
}


void ConceptData::addLast( ConceptData* r )
{
    if( next_ )
        next_->addLast( r );
    else
        next_ = r;
}


void ConceptData::generateFillers( std::vector< RoleFiller* >& array, sal_Int32 pos )
{
    if( array[ queryNo_ ] != RoleFiller::STOP() ) // not 'prohibited'
    {
        sal_Int32 wcl = ctx_->wordContextLin( pos );
        RoleFiller* p = new RoleFiller( nColumns_,
                                        this,
                                        role_,
                                        pos,
                                        wcl,
                                        pos + proximity_ );
        p->use( array, queryNo_ );
    }
    // !!! maybe eliminate tail recursion
    if( next_ )
        next_->generateFillers( array,pos );
}
