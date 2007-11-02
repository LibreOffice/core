/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: easywri.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:26:22 $
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
    return *aCurDestination.top();
}

