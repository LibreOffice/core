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

#ifndef ADC_DISPLAY_OUT_POSITION_HXX
#define ADC_DISPLAY_OUT_POSITION_HXX


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include <toolkit/out_node.hxx>
    // PARAMETERS



namespace output
{



class Position
{
  public:
    // LIFECYCLE
                        Position();
    explicit            Position(
                            Node &              i_directory,
                            const String &      i_file = String::Null_() );
                        Position(
                            const Position &    i_directory,
                            const String &      i_rDifferentFile );
                        ~Position();

    // OPERATIONS
    Position &          operator=(
                            Node &              i_node );
    Position &          operator+=(
                            const String &      i_nodeName );
    Position &          operator-=(
                            intt                i_levels );

    // INQUIRY
    bool                IsValid() const         { return pDirectory->Depth() >= 0; }
    const String &      Name() const            { return pDirectory->Name(); }
    const String &      File() const            { return sFile; }
    intt                Depth() const           { return pDirectory->Depth(); }

    void                Get_Chain(
                            StringVector &      o_result ) const
                                                { pDirectory->Get_Chain(o_result); }
    String              LinkToRoot() const;

    void                Get_LinkTo(
                            StreamStr &         o_result,
                            const Position &    i_destination,
                            const String &      i_localLabel = String::Null_() ) const;
    void                Get_LinkToRoot(
                            StreamStr &         o_result ) const;

    static char         Delimiter()             { return '/'; }

    // ACCESS
    Node &              RelatedNode() const     { return *pDirectory; }

    void                Set(
                            Node &              i_node,
                            const String &      i_file = String::Null_() );
    void                Set_File(
                            const String &      i_file );

  private:
    // DATA
    String              sFile;
    Node *              pDirectory;
};


/// @return No delimiter at start, with delimiter at end.
const char *        get_UpLink(
                        uintt                   i_depth );


// IMPLEMENTATION

inline void
Position::Set_File( const String & i_file )
    { sFile = i_file; }

}   // namespace output

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
