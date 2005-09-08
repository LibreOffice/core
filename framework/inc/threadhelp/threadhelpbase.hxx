/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: threadhelpbase.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:37:57 $
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

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#define __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_THREADHELP_LOCKHELPER_HXX_
#include <threadhelp/lockhelper.hxx>
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
    @short          "baseclass" to make own classes threadsafe
    @descr          Sometimes you must share your lock- or mutex member with any other baseclasses.
                    And baseclasses are initialized erlier then members! That's why you should use
                    this struct as first of your baseclasses!!!
                    Then you will get a public member "m_aLock" which can be used by special guard implementations
                    to make your code threadsafe.

    @seealso        class LockHelper

    @implements     -
    @base           -

    @devstatus      ready to use
*//*-*************************************************************************************************************/
struct ThreadHelpBase
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------
    public:
        ThreadHelpBase( ::vos::IMutex* pSolarMutex = NULL )
                :   m_aLock( pSolarMutex )
        {
        }

    //-------------------------------------------------------------------------------------------------------------
    //  public member
    //  Make it mutable for using in const functions!
    //-------------------------------------------------------------------------------------------------------------
    public:

        mutable LockHelper m_aLock;
};

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
