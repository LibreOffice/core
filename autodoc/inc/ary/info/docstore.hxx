/*************************************************************************
 *
 *  $RCSfile: docstore.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:15 $
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

#ifndef ARY_INFO_DOCSTORE_HXX
#define ARY_INFO_DOCSTORE_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include <ary/info/inftypes.hxx>

namespace ary
{
    class Documentation;

namespace info
{

class DocuStore
{
  public:
    virtual             ~DocuStore() {}

    void                Store2CurProject(
                            DYN Documentation & let_drDocu );
    void                Store2CurFile(
                            DYN Documentation & let_drDocu );
    void                Store2CurNamespace(
                            DYN Documentation & let_drDocu );

    void                Store2ConnectedDeclaration(
                            DYN Documentation & let_drDocu );

    void                Store2Glossary(
                            DYN Documentation & let_drDocu,
                            const udmstri &     i_sExplainedTerm );
    void                Store2GlobalTexts(
                            DYN Documentation & let_drDocu,
                            ary::info::GlobalTextId
                                                i_nId );
  private:
    virtual void        do_Store2CurProject(
                            DYN Documentation & let_drDocu ) = 0;
    virtual void        do_Store2CurFile(
                            DYN Documentation & let_drDocu ) = 0;
    virtual void        do_Store2CurNamespace(
                            DYN Documentation & let_drDocu ) = 0;

    virtual void        do_Store2ConnectedDeclaration(
                            DYN Documentation & let_drDocu ) = 0;

    virtual void        do_Store2Glossary(
                            DYN Documentation & let_drDocu,
                            const udmstri &     i_sExplainedTerm ) = 0;
    virtual void        do_Store2GlobalTexts(
                            DYN Documentation & let_drDocu,
                            ary::info::GlobalTextId
                                                i_nId ) = 0;
};


// IMPLEMENTATION


inline void
DocuStore::Store2CurProject( DYN Documentation & let_drDocu )
    { do_Store2CurProject(let_drDocu);  }
inline void
DocuStore::Store2CurFile( DYN Documentation & let_drDocu )
    { do_Store2CurFile(let_drDocu);  }
inline void
DocuStore::Store2CurNamespace( DYN Documentation & let_drDocu )
    { do_Store2CurNamespace(let_drDocu);  }
inline void
DocuStore::Store2ConnectedDeclaration( DYN Documentation & let_drDocu )
    { do_Store2ConnectedDeclaration(let_drDocu);  }
inline void
DocuStore::Store2Glossary( DYN Documentation &  let_drDocu,
                           const udmstri &      i_sExplainedTerm )
    { do_Store2Glossary(let_drDocu, i_sExplainedTerm);  }
inline void
DocuStore::Store2GlobalTexts( DYN Documentation &       let_drDocu,
                              ary::info::GlobalTextId   i_nId )
    { do_Store2GlobalTexts(let_drDocu, i_nId);  }


}   // namespace info
}   // namespace ary


#endif

