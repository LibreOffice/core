/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tkp_uidl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:06:42 $
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

#ifndef ADC_TKP_UIDL_HXX
#define ADC_TKP_UIDL_HXX

// USED SERVICES
    // BASE CLASSES
#include <tokens/tkp2.hxx>
    // COMPONENTS
    // PARAMETRS

class TkpDocuContext;


namespace csi
{
namespace uidl
{



class Token_Receiver;
class Context_UidlCode;


/** This is a TokenParser which is able to parse tokens from
    C++ source code.
*/
class TokenParser_Uidl : public TokenParse2
{
  public:
    // LIFECYCLE
                        TokenParser_Uidl(
                            Token_Receiver &    o_rUidlReceiver,
                            DYN TkpDocuContext &
                                                let_drDocuContext );
    virtual             ~TokenParser_Uidl();

    // OPERATIONS
  private:
    virtual ::TkpContext &
                        CurrentContext();

    virtual void        SetStartContext();
    virtual void        SetCurrentContext(
                            TkpContext &        io_rContext );
    // DATA
    Dyn<Context_UidlCode>
                        pBaseContext;
    ::TkpContext *      pCurContext;
};


}   // namespace uidl
}   // namespace csi

#endif


