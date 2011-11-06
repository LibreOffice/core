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



#ifndef ADC_DISPLAY_OUT_TREE_HXX
#define ADC_DISPLAY_OUT_TREE_HXX


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include "out_position.hxx"
    // PARAMETERS


namespace output
{

inline const char *
ModuleFileName()
{ return "module-ix.html"; }
inline const char *
IndexFilesDirName()
{ return "index-files"; }
inline const char *
IndexFile_A()
{ return "index-1.html"; }


class Tree
{
  public:
    // LIFECYCLE
                        Tree();
                        ~Tree();

    // OPERATIONS
    void                Set_Overview(
                            const StringVector &
                                                i_path,
                            const String &      i_sFileName );
    Node &              Set_NamesRoot(
                            const StringVector &
                                                i_path );
    Node &              Set_IndexRoot(
                            const StringVector &
                                                i_path );
    Node &              Set_ProjectsRoot(
                            const StringVector &
                                                i_path );
    Node &              Provide_Node(
                            const StringVector &
                                                i_path );

    // ACCESS
    Node &              RootNode()              { return *pRoot; }
    Node &              NamesRootNode()         { return *pNamesRoot; }
    Node &              IndexRootNode()         { return *pIndexRoot; }
    Node &              ProjectsRootNode()      { return *pProjectsRoot; }

    Position            Root()                  { return Position(*pRoot); }
    Position            Overview()              { return aOverview; }
    Position            NamesRoot()             { return Position(*pNamesRoot); }
    Position            IndexRoot()             { return Position(*pIndexRoot); }
    Position            ProjectsRoot()          { return Position(*pProjectsRoot); }

  private:
    // forbidden:
                        Tree(const Tree&);
    Tree &              operator=(const Tree&);

    // DATA
    Dyn<Node>           pRoot;
    Node *              pNamesRoot;
    Node *              pIndexRoot;
    Node *              pProjectsRoot;
    Position            aOverview;
};


// IMPLEMENTATION

inline Node &
Tree::Provide_Node( const StringVector & i_path )
    { return pRoot->Provide_Child(i_path); }


inline void
Tree::Set_Overview( const StringVector & i_path,
                    const String &       i_sFileName )
    { aOverview.Set(Provide_Node(i_path), i_sFileName); }

inline Node &
Tree::Set_NamesRoot( const StringVector & i_path )
    { pNamesRoot = &Provide_Node(i_path);
      return *pNamesRoot; }

inline Node &
Tree::Set_IndexRoot( const StringVector & i_path )
    { pIndexRoot = &Provide_Node(i_path);
      return *pIndexRoot; }

inline Node &
Tree::Set_ProjectsRoot( const StringVector & i_path )
    { pProjectsRoot = &Provide_Node(i_path);
      return *pProjectsRoot; }



}   // namespace output


#endif
