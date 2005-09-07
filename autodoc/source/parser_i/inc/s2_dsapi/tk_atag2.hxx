/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tk_atag2.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:57:07 $
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

#ifndef DSAPI_TK_ATAG2_HXX
#define DSAPI_TK_ATAG2_HXX

// USED SERVICES
    // BASE CLASSES
#include <s2_dsapi/dsapitok.hxx>
    // COMPONENTS
    // PARAMETERS
#include <luxenum.hxx>

namespace csi
{
namespace dsapi
{


class Tok_AtTag : public Token
{
  public:
    // TYPE
    enum E_TokenId
    {
        e_none = 0,
        author =  1,
        see = 2,
        param = 3,
        e_return = 4,
        e_throw = 5,
        example = 6,
        deprecated = 7,
        suspicious = 8,
        missing = 9,
        incomplete = 10,
        version = 11,
        guarantees = 12,
        exception = 13,
        since = 14
    };
    typedef lux::Enum<E_TokenId> EV_TokenId;

    // Spring and Fall
                        Tok_AtTag(
                            EV_TokenId          i_eTag )
                                                :   eTag(i_eTag) {}
    // OPERATIONS
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    // INQUIRY
    virtual const char* Text() const;
    E_TokenId           Id() const              { return eTag; }

  private:
    EV_TokenId          eTag;
};

}   // namespace dsapi
}   // namespace csi


#endif

