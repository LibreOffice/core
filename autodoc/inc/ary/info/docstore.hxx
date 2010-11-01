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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
