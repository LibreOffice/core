/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: out_node.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:41:07 $
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

#ifndef ADC_DISPLAY_OUT_NODE_HXX
#define ADC_DISPLAY_OUT_NODE_HXX




namespace output
{


/** @resp
    Represents a tree of names where each node can have only one parent,
    but a list of children.

    @see    Position
    @see    Tree
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
                          ~Node();

      // OPERATORS
      bool                operator==(
                              const Node &        i_node ) const
                                                  { return pParent == i_node.pParent AND sName == i_node.sName; }
      bool                operator!=(
                              const Node &        i_node ) const
                                                  { return NOT operator==(i_node); }

      // OPERATIONS
      /// Seek, and if not existent, create.
      Node &              Provide_Child(
                              const String &      i_name );
      /// Seek, and if not existent, create.
      Node &              Provide_Child(
                              const StringVector &
                                                  i_path )
                                                  { return provide_Child(i_path.begin(), i_path.end()); }
      // INQUIRY
      intt                Depth() const           { return nDepth; }

    const String &      Name() const            { return sName; }
    /// @return Id of a namespace or class etc. this directory represents.
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
                          Node(
                              const String &      i_name,
                              Node &              i_parent );

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
