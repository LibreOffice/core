/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: out_tree.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:58:20 $
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
