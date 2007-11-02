/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: docstore.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:15:45 $
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

#ifndef ARY_INFO_DOCSTORE_HXX
#define ARY_INFO_DOCSTORE_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include <ary/info/inftypes.hxx>

namespace ary
{
namespace doc
{
    class Node;
}



namespace info
{

class DocuStore
{
  public:
    virtual             ~DocuStore() {}

    void                Store2CurFile(
                            DYN doc::Node     & let_drDocu );
    void                Store2CurNamespace(
                            DYN doc::Node     & let_drDocu );

    void                Store2ConnectedDeclaration(
                            DYN doc::Node     & let_drDocu );

    void                Store2Glossary(
                            DYN doc::Node     & let_drDocu,
                            const String  &     i_sExplainedTerm );
    void                Store2GlobalTexts(
                            DYN doc::Node     & let_drDocu,
                            ary::info::GlobalTextId
                                                i_nId );
  private:
    virtual void        do_Store2CurFile(
                            DYN doc::Node     & let_drDocu ) = 0;
    virtual void        do_Store2CurNamespace(
                            DYN doc::Node     & let_drDocu ) = 0;

    virtual void        do_Store2ConnectedDeclaration(
                            DYN doc::Node     & let_drDocu ) = 0;

    virtual void        do_Store2Glossary(
                            DYN doc::Node     & let_drDocu,
                            const String  &     i_sExplainedTerm ) = 0;
    virtual void        do_Store2GlobalTexts(
                            DYN doc::Node     & let_drDocu,
                            ary::info::GlobalTextId
                                                i_nId ) = 0;
};




// IMPLEMENTATION
inline void
DocuStore::Store2CurFile( DYN doc::Node     & let_drDocu )
    { do_Store2CurFile(let_drDocu);  }
inline void
DocuStore::Store2CurNamespace( DYN doc::Node     & let_drDocu )
    { do_Store2CurNamespace(let_drDocu);  }
inline void
DocuStore::Store2ConnectedDeclaration( DYN doc::Node     & let_drDocu )
    { do_Store2ConnectedDeclaration(let_drDocu);  }
inline void
DocuStore::Store2Glossary( DYN doc::Node     &  let_drDocu,
                           const String  &      i_sExplainedTerm )
    { do_Store2Glossary(let_drDocu, i_sExplainedTerm);  }
inline void
DocuStore::Store2GlobalTexts( DYN doc::Node     &       let_drDocu,
                              ary::info::GlobalTextId   i_nId )
    { do_Store2GlobalTexts(let_drDocu, i_nId);  }




}   // namespace info
}   // namespace ary
#endif
