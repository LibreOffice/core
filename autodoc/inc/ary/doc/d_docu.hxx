/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
