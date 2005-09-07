/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tokproct.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:07:16 $
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

#ifndef ADC_TOKPROCT_HXX
#define ADC_TOKPROCT_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS


namespace csi
{
namespace uidl
{


class UnoIDL_PE;


/** is a parent class for classes, which take part in parsing tokens semantically.
    It provides some types for them.
*/
class TokenProcessing_Types
{
  public:
    enum E_TokenDone
    {
        not_done = 0,
        done = 1
    };

    enum E_EnvStackAction
    {
        stay,           // same parse environment
        push_sure,      // push sub environment, which must be the correct one
        push_try,       // push sub environment, which is tried, if it may be the right one
        pop_success,    // return to parent environment, parsing was successful
        pop_failure     // return to parent environment, but an error occured.
    };

    struct TokenProcessing_Result
    {
        E_TokenDone         eDone;
        E_EnvStackAction    eStackAction;
        UnoIDL_PE *         pEnv2Push;

                            TokenProcessing_Result()
                                                : eDone(not_done), eStackAction(stay), pEnv2Push(0) {}
        void                reset()             { eDone = not_done; eStackAction = stay; pEnv2Push = 0; }
    };

    enum E_ParseResult
    {
        res_error,
        res_complete,
        res_predeclaration
    };
};


}   // namespace uidl
}   // namespace csi

#endif

