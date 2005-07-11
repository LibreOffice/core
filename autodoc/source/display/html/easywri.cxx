/*************************************************************************
 *
 *  $RCSfile: easywri.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-07-11 15:36:32 $
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
#include "easywri.hxx"


// NOT FULLY DEFINED SERVICES


using namespace csi::html;


EasyWriter::EasyWriter()
{
}

EasyWriter::~EasyWriter()
{
}

void
EasyWriter::Open_OutputNode( csi::xml::Element & io_rDestination )
{
    aCurDestination.push(&io_rDestination);
}

void
EasyWriter::Finish_OutputNode()
{
    csv_assert( NOT aCurDestination.empty() );
    aCurDestination.pop();
}

void
EasyWriter::Add_HorizontalLine()
{
    Out() << new HorizontalLine;
}

void
EasyWriter::Write( const char *        i_sFormatTag,
                   const char *        i_sText )
{
    DYN csi::xml::Element * dpNew = new csi::xml::APureElement( i_sFormatTag );
    *dpNew << i_sText;
    Out() << dpNew;
}


csi::xml::Element &
EasyWriter::Out()
{
    csv_assert( aCurDestination.size() > 0);
//    if ( aCurDestination.size() <= 0 )
//        csv_assert( false );

    return *aCurDestination.top();
}
































#if 0
/*
class Html_Inverse : public Html::Piece
{
  public:
                        Html_Inverse(
                            DYN Html::Piece &   let_drInhalt,
                            const udmstri       i_sBgColor );
                        ~Html_Inverse();
    virtual void        WriteOut(
                            csv::AnyWrite &     io_aFile );
  private:
    DYN Html::Piece *   dpInhalt;
    udmstri             sBgColor;
    udmstri             sColor;
};

Html_Inverse::Html_Inverse( DYN Html::Piece &       let_drInhalt,
                            const udmstri       i_sBgColor )
    :   dpInhalt(&let_drInhalt),
        sBgColor(i_sBgColor)
{
}

Html_Inverse::~Html_Inverse()
{
    Delete_dyn(dpInhalt);
}


void
Html_Inverse::WriteOut( csv::File & io_aFile )
{
    io_aFile.WriteStr( "<TABLE WIDTH=100% BORDER=0 CELLPADDING=4 CELLSPACING=0>"
                       " <TR> <TD BGCOLOR=\"");
    io_aFile.WriteStr( sBgColor );
    io_aFile.WriteStr( "\" VALIGN=\"MIDDLE\">\r\n");
    dpInhalt->WriteOut(io_aFile);
    io_aFile.WriteStr( "\r\n</TD></TR></TABLE>\r\n" );
}




*/
#endif // 0




