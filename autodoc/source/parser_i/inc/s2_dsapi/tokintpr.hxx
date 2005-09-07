/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tokintpr.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:58:11 $
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

#ifndef ADC_DSAPI_TOKINTPR_HXX
#define ADC_DSAPI_TOKINTPR_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS

namespace csi
{
namespace dsapi
{


class Tok_AtTag;
class Tok_XmlConst;
class Tok_XmlLink_BeginTag;
class Tok_XmlLink_EndTag;
class Tok_XmlFormat_BeginTag;
class Tok_XmlFormat_EndTag;
class Tok_Word;
class Tok_HtmlTag;

class TokenInterpreter
{
  public:
    virtual             ~TokenInterpreter() {}

    virtual void        Process_AtTag(
                            const Tok_AtTag &   i_rToken ) = 0;
    virtual void        Process_HtmlTag(
                            const Tok_HtmlTag & i_rToken ) = 0;
    virtual void        Process_XmlConst(
                            const Tok_XmlConst &
                                                i_rToken ) = 0;
    virtual void        Process_XmlLink_BeginTag(
                            const Tok_XmlLink_BeginTag &
                                                i_rToken ) = 0;
    virtual void        Process_XmlLink_EndTag(
                            const Tok_XmlLink_EndTag &
                                                i_rToken ) = 0;
    virtual void        Process_XmlFormat_BeginTag(
                            const Tok_XmlFormat_BeginTag &
                                                i_rToken ) = 0;
    virtual void        Process_XmlFormat_EndTag(
                            const Tok_XmlFormat_EndTag &
                                                i_rToken ) = 0;
    virtual void        Process_Word(
                            const Tok_Word &    i_rToken ) = 0;
    virtual void        Process_Comma() = 0;
    virtual void        Process_DocuEnd() = 0;
    virtual void        Process_EOL() = 0;
};



// IMPLEMENTATION


}   // namespace dsapi
}   // namespace csi

#endif

