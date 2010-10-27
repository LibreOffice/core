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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
