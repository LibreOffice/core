/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: d_docu.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 14:58:22 $
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
    explicit            Documentation(
                            DYN Node &          pass_data);
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
