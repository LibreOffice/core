/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ci_text.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-18 13:40:53 $
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
#include <ary/info/ci_text.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/info/all_dts.hxx>


namespace ary
{
namespace info
{

DocuText::DocuText()
    :   bUsesHtml(false)
{
}

DocuText::~DocuText()
{
    for ( TokenList::iterator iter = aTokens.begin();
          iter != aTokens.end();
          ++iter )
    {
        delete (*iter);
    }
}

void
DocuText::StoreAt( DocuDisplay & o_rDisplay ) const
{
    ary::info::DocuText::TokenList::const_iterator itEnd = aTokens.end();
    for ( ary::info::DocuText::TokenList::const_iterator it = aTokens.begin();
          it != itEnd;
          ++it )
    {
        (*it)->StoreAt(o_rDisplay);
    }
}

}   // namespace info
}   // namespace ary


