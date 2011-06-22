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

#ifndef ARY_DOC_D_BOOLEAN_HXX
#define ARY_DOC_D_BOOLEAN_HXX

// BASE CLASSES
#include <ary/doc/d_node.hxx>

// USED SERVICES




namespace ary
{
namespace doc
{


/** Repesents a boolean documentation item like "optional" or "not optional".
*/
class Boolean : public Node
{
  public:
    // LIFECYCLE
    explicit            Boolean(
                            nodetype::id        i_type );
    virtual             ~Boolean();

    // OPERATIONS
    void                Set(
                            bool                i_b );
    // INQUIRY
    bool                IsTrue() const;

  private:
    // Interface csv::ConstProcessorClient:
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;
    // DATA
    bool                b;
};




// IMPLEMENTATION
inline
Boolean::Boolean(nodetype::id i_type)
    :   Node(i_type),
        b(false)
{
}

inline bool
Boolean::IsTrue() const
{
    return b;
}

inline void
Boolean::Set( bool i_b )
{
    b = i_b;
}



}   // namespace doc
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
