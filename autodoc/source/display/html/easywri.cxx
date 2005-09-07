/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: easywri.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:27:33 $
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




