/*************************************************************************
 *
 *  $RCSfile: out_position.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-11-01 17:15:27 $
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


#include <precomp.h>
#include <toolkit/out_position.hxx>


// NOT FULLY DEFINED SERVICES



namespace output
{



namespace
{

const int       C_nAssumedMaxLinkLength = 500;

const int       C_nMaxDepth = 30;
char            C_sUpLinkArray[3*C_nMaxDepth+1] =
                        "../../../../../../../../../../"
                        "../../../../../../../../../../"
                        "../../../../../../../../../../";

const char *    C_sUpLink = &C_sUpLinkArray[0];


void                move_ToParent(
                        Node * &            io_node,
                        intt                i_levels = 1 );

void
move_ToParent( Node * &   io_node,
               intt       i_levels )
{
    for ( intt n = 0; n < i_levels; ++n )
    {
        csv_assert(io_node != 0);
        io_node = io_node->Parent();
    }
}



}   // namepace anonymous



Position::Position()
    :   sFile(),
        pDirectory(&Node::Null_())
{
}


Position::Position( Node &              i_directory,
                    const String &      i_file )
    :   sFile(i_file),
        pDirectory(&i_directory)
{
}

Position::Position( Position &      i_directory,
                    const String &  i_sDifferentFile )
    :   sFile(i_sDifferentFile),
        pDirectory(i_directory.pDirectory)
{
}


Position::~Position()
{
}


Position &
Position::operator=( Node & i_node )
{
    pDirectory = &i_node;
    sFile.clear();
    return *this;
}

Position &
Position::operator+=( const String & i_nodeName )
{
    csv_assert(pDirectory != 0);

    pDirectory = &pDirectory->Provide_Child(i_nodeName);
    sFile.clear();

    return *this;
}

Position &
Position::operator-=( intt i_levels )
{
    csv_assert(pDirectory != 0);

    pDirectory = pDirectory->Parent();
    if (pDirectory == 0)
        pDirectory = &Node::Null_();
    sFile.clear();

    return *this;
}

String
Position::LinkTo( Position &        i_destination,
                  const String &    i_localLabel ) const
{
    StreamLock  aLink(C_nAssumedMaxLinkLength);
    StreamStr & rLink = aLink();
    Get_LinkTo(rLink, i_destination, i_localLabel);
    return rLink.c_str();
}

String
Position::LinkToRoot( const String & i_localLabel ) const
{
    return StreamLock(C_nAssumedMaxLinkLength)() << get_UpLink(Depth()) << c_str;
}

void
Position::Get_LinkTo( StreamStr &         o_result,
                      Position &          i_destination,
                      const String &      i_localLabel ) const
{
    Node * p1 = pDirectory;
    Node * p2 = i_destination.pDirectory;
    intt diff = Depth() - i_destination.Depth();
    intt pathLength1 = 0;
    intt pathLength2 = 0;

    if ( diff > 0 )
    {
        pathLength1 = diff;
        move_ToParent(p1,pathLength1);
    }
    else if ( diff < 0 )
    {
        pathLength2 = -diff;
        move_ToParent(p2,pathLength2);
    }

    while ( p1 != p2 )
    {
        move_ToParent(p1);
        move_ToParent(p2);
        ++pathLength1;
        ++pathLength2;
    }

    o_result << get_UpLink(pathLength1);
    i_destination.pDirectory->Get_Path(o_result, pathLength2);
    o_result << i_destination.sFile;
    if (i_localLabel.length())
        o_result << "#" << i_localLabel;
}

void
Position::Get_LinkToRoot( StreamStr &         o_result,
                          const String &      i_localLabel ) const
{
    o_result << get_UpLink(Depth());
}

void
Position::Set( Node &           i_node,
               const String &   i_file )
{
    sFile = i_file;
    pDirectory = &i_node;
}




const char *
get_UpLink(intt i_depth)
{
    if ( i_depth <= C_nMaxDepth )
        return C_sUpLink + 3*(C_nMaxDepth - i_depth);


    StreamLock  aRet(i_depth*3 + 1);
    StreamStr & rRet = aRet();
    for ( intt nDepth = i_depth; nDepth > C_nMaxDepth; nDepth -= C_nMaxDepth )
    {
        rRet << C_sUpLink;
    }
    rRet << C_sUpLink + 3*(C_nMaxDepth - nDepth);
    return rRet.c_str();
}



}   // namespace output
