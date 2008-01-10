/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WW8OutputWithDepth.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 11:47:58 $
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

#ifndef INCLUDED_WW8_OUTPUT_WITH_DEPTH
#define INCLUDED_WW8_OUTPUT_WITH_DEPTH

#ifndef INCLUDED_OUTPUT_WITH_DEPTH
#include <resourcemodel/OutputWithDepth.hxx>
#endif

#include <string>

using namespace ::std;

namespace writerfilter {
namespace doctok
{
class WW8OutputWithDepth : public OutputWithDepth<string>
{
protected:
    void output(const string & output) const;
public:
    WW8OutputWithDepth();
    virtual ~WW8OutputWithDepth() { finalize(); }
};

extern WW8OutputWithDepth output;

}}

#endif // INCLUDED_WW8_OUTPUT_WITH_DEPTH
