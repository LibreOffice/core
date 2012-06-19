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
