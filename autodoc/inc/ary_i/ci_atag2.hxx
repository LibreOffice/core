/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ci_atag2.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:20:06 $
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

#ifndef ARY_CI_ATAG2_HXX
#define ARY_CI_ATAG2_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include <ary_i/ci_text2.hxx>
    // PARAMETERS



namespace ary
{
namespace inf
{


class DocumentationDisplay;

class AtTag2
{
  public:
    virtual             ~AtTag2() {}

    virtual void        DisplayAt(
                            DocumentationDisplay &
                                                o_rDisplay ) const = 0;

    const char *        Title() const           { return sTitle; }
    const DocuTex2 &    Text() const            { return aText; }
    DocuTex2 &          Access_Text()           { return aText; }

  protected:
                        AtTag2(
                            const char *        i_sTitle)
                                                :   sTitle(i_sTitle) {}
    String              sTitle;
    DocuTex2            aText;
};



// IMPLEMENTATION

}   // namespace inf
}   // namespace ary


#endif

