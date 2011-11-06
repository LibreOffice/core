/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
