/*************************************************************************
 *
 *  $RCSfile: out_position.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2005-03-23 09:04:47 $
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
                            Position &          i_directory,
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
    String              LinkTo(
                            Position &          i_destination,
                            const String &      i_localLabel = String::Null_() ) const;
    String              LinkToRoot(
                            const String &      i_localLabel = String::Null_() ) const;

    void                Get_LinkTo(
                            StreamStr &         o_result,
                            Position &          i_destination,
                            const String &      i_localLabel = String::Null_() ) const;
    void                Get_LinkToRoot(
                            StreamStr &         o_result,
                            const String &      i_localLabel = String::Null_() ) const;

    static char         Delimiter()             { return '/'; }

    // ACCESS
    Node &              RelatedNode()           { return *pDirectory; }

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
