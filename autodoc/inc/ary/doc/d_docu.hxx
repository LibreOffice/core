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

#ifndef ARY_DOC_D_DOCU_HXX
#define ARY_DOC_D_DOCU_HXX

// BASE CLASSES
#include <cosv/tpl/processor.hxx>

// USED SERVICES
#include <ary/doc/d_node.hxx>




namespace ary
{
namespace doc
{


/** Represents a documentation which is assigned to an Autodoc
    repository entity.
*/
class Documentation : public csv::ConstProcessorClient
{
  public:
                        Documentation();
                        ~Documentation();
    // OPERATIONS
    void                Clear();

    // INQUIRY
    const Node *        Data() const;

    // ACCESS
    Node *              Data();
    void                Set_Data(
                            ary::doc::Node &    i_data );

  private:
    // Interface csv::ConstProcessorClient:
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;
    // DATA
    Dyn<Node>           pData;
};




// IMPLEMENTATION
inline void
Documentation::Clear()
{
    pData = 0;
}

inline const Node *
Documentation::Data() const
{
    return pData.Ptr();
}

inline Node *
Documentation::Data()
{
    return pData.Ptr();
}

inline void
Documentation::Set_Data(ary::doc::Node & i_data)
{
    pData = &i_data;
}




}   // namespace doc
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
