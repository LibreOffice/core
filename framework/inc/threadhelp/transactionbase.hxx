/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: transactionbase.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:38:10 $
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

#ifndef __FRAMEWORK_THREADHELP_TRANSACTIONBASE_H_
#define __FRAMEWORK_THREADHELP_TRANSACTIONBASE_H_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_THREADHELP_TRANSACTIONMANAGER_HXX_
#include <threadhelp/transactionmanager.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          make it possible to instanciate a transacion manager as first member!
    @descr          If you use a transaction manager as a member of your class and whish to use it earlier then other ones
                    you should have a look on this implementation. You must use it as the first base class
                    of your implementation - because base classes are initialized by his order and before your
                    member! Thats why ist a good place to declare this member.

    @implements     -
    @base           -

    @devstatus      ready to use
*//*-*************************************************************************************************************/
struct TransactionBase
{
    //-------------------------------------------------------------------------------------------------------------
    //  public member
    //-------------------------------------------------------------------------------------------------------------
    public:

        mutable TransactionManager    m_aTransactionManager    ;   /// "your" public manager-member!
                                                                   /// Make it mutable for using in const functions!

};      //  struct TransactionBase

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_THREADHELP_TRANSACTIONBASE_H_
