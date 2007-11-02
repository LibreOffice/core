/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: doc_deal.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 17:02:24 $
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

#ifndef ADC_DOC_DEAL_HXX
#define ADC_DOC_DEAL_HXX

// BASE CLASSES
#include <tokens/tokproct.hxx>
// USED SERVICES
#include <ary/cpp/c_types4cpp.hxx>

namespace ary
{
namespace doc
{
    class OldCppDocu;
}
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
                            DYN ary::doc::OldCppDocu &
                                                let_drInfo );
  private:
    virtual void        do_TakeDocu(
                            DYN ary::doc::OldCppDocu &
                                                let_drInfo ) = 0;
};




// IMPLEMENTATION
inline void
DocuDealer::TakeDocu( DYN ary::doc::OldCppDocu & let_drInfo )
    { do_TakeDocu(let_drInfo); }




#endif
