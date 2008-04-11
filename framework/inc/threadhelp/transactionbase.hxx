/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: transactionbase.hxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef __FRAMEWORK_THREADHELP_TRANSACTIONBASE_H_
#define __FRAMEWORK_THREADHELP_TRANSACTIONBASE_H_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <threadhelp/transactionmanager.hxx>

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
