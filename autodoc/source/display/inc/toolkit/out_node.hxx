/*************************************************************************
 *
 *  $RCSfile: out_node.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-11-01 17:15:19 $
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

#ifndef ADC_DISPLAY_OUT_NODE_HXX
#define ADC_DISPLAY_OUT_NODE_HXX


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS



namespace output
{


/** @resp

    @descr
*/
class Node
{
  public:
    typedef std::vector< Node* >    List;
    typedef UINT32                  relative_id;

      // LIFECYCLE
      enum E_NullObject { null_object };

                          Node();
      explicit            Node(
                              E_NullObject        );
                          Node(
                              const String &      i_name,
                              Node &              i_parent );
                          ~Node();

      // OPERATORS
      bool                operator==(
                              const Node &        i_node ) const
                                                  { return pParent == i_node.pParent AND sName == i_node.sName; }
      bool                operator!=(
                              const Node &        i_node ) const
                                                  { return NOT operator==(i_node); }

      // OPERATIONS
      Node &              Provide_Child(
                              const String &      i_name );
      Node &              Provide_Child(
                              const StringVector &
                                                  i_path )
                                                  { return provide_Child(i_path.begin(), i_path.end()); }
      // INQUIRY
      intt                Depth() const           { return nDepth; }

    const String &      Name() const            { return sName; }
    relative_id         RelatedNameRoom() const { return nNameRoomId; }
    /// @return No delimiter at start, with delimiter at end.
    void                Get_Path(
                            StreamStr &         o_result,
                            intt                i_maxDepth = -1 ) const;
    void                Get_Chain(
                            StringVector &      o_result,
                            intt                i_maxDepth = -1 ) const;

    // ACCESS
    void                Set_RelatedNameRoom(
                            relative_id         i_nNameRoomId )
                                                { nNameRoomId = i_nNameRoomId; }
      Node *              Parent()                { return pParent; }
      Node *              Child(
                              const String &      i_name )
                                                  { return find_Child(i_name); }
    List &              Children()              { return aChildren; }

    /// @return a reference to a Node with Depth() == -1.
    static Node &       Null_();

  private:
      // Local
      Node *              find_Child(
                              const String &      i_name );
      Node &              add_Child(
                              const String &      i_name );
      Node &              provide_Child(
                              StringVector::const_iterator
                                                  i_next,
                              StringVector::const_iterator
                                                  i_end );
      // Data
      String              sName;
      Node *              pParent;
      List                aChildren;
      intt                nDepth;
    relative_id         nNameRoomId;
};



}   // namespace output

#endif
