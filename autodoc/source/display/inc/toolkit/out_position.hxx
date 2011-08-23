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
    String              LinkToRoot(
                            const String &      i_localLabel = String::Null_() ) const;

    void                Get_LinkTo(
                            StreamStr &         o_result,
                            const Position &    i_destination,
                            const String &      i_localLabel = String::Null_() ) const;
    void                Get_LinkToRoot(
                            StreamStr &         o_result,
                            const String &      i_localLabel = String::Null_() ) const;

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
