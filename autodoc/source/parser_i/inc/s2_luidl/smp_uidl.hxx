/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: smp_uidl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:05:09 $
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

#ifndef ADC_SMP_HXX
#define ADC_SMP_HXX



// USED SERVICES
    // BASE CLASSES
#include <s2_luidl/tok_recv.hxx>
#include <s2_dsapi/tok_recv.hxx>
    // COMPONENTS
    // PARAMETERS

namespace csi
{
namespace uidl
{



/** is an implementation class for ParseEnvironment
*/
class SemanticParser : public csi::uidl::Token_Receiver,
                       public csi::dsapi::Token_Receiver
{
  public:
  typedef std::deque< DYN TextToken * > TokenQueue;

                        ~SemanticParser();


    void                Receive(
                            DYN csi::uidl::Token &
                                                let_drToken );
    void                Receive(
                            DYN csi::dsapi::Token &
                                                let_drToken );

  private:
    // DATA
    TokenQueue          aTokenQueue;


};



// IMPLEMENTATION


}   // namespace uidl
}   // namespace csi

#endif


