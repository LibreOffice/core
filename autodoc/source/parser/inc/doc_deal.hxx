/*************************************************************************
 *
 *  $RCSfile: doc_deal.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:32 $
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

#ifndef ADC_DOC_DEAL_HXX
#define ADC_DOC_DEAL_HXX



// USED SERVICES
    // BASE CLASSES
#include <tokens/tokproct.hxx>
    // COMPONENTS
    // PARAMETERS
#include <ary/cpp/c_etypes.hxx>

namespace ary
{
    class Documentation;
}



class DocuDealer
{
  public:
    // INQUIRY
    virtual             ~DocuDealer() {}

    // OPERATIONS
    /** @descr
        This distributes the let_drDocu to the matching ary::RepositoryEntity .

        If the docu is not inline, it will be saved and later given to the next
        ary::CodeEntity. Or it will be discarded, if there does not come a matching
        ary::CodeEntity .

        If the docu is inline after a function header or after an enum value
        or after a function parameter or after a base class, it will be stored
        together with the matching function, enum value, parameter or base class.

        If the documentation is @file or @project or @glos(sary) it will be
        stored at the matching ary::cpp::FileGroup, ary::cpp::ProjectGroup
        or ary::Glossary.
    */
    void                TakeDocu(
                            DYN ary::Documentation &
                                                let_drInfo );
  private:
    virtual void        do_TakeDocu(
                            DYN ary::Documentation &
                                                let_drInfo ) = 0;
};



// IMPLEMENTATION

inline void
DocuDealer::TakeDocu( DYN ary::Documentation & let_drInfo )
    { do_TakeDocu(let_drInfo); }




#endif

