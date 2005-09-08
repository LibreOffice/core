/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: igate.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:35:53 $
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

#ifndef __FRAMEWORK_THREADHELP_IGATE_H_
#define __FRAMEWORK_THREADHELP_IGATE_H_

//_________________________________________________________________________________________________________________
//  includes
//_________________________________________________________________________________________________________________

#ifndef _OSL_TIME_H_
#include <osl/time.h>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @descr          We need this interface to support using of different gate implementations in a generic way.
*//*-*************************************************************************************************************/
class IGate
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------
    public:

        /*-****************************************************************************************************//**
            @descr      These functions must be supported by a derived class!
                            open()      -open access for all waiting threads
                            close()     -close access for all further coming threads
                            openGap()   -open access for current waiting threads only
                            wait()      -must be called to pass the gate
        *//*-*****************************************************************************************************/
        virtual void     open    (                                   ) = 0;
        virtual void     close   (                                   ) = 0;
        virtual void     openGap (                                   ) = 0;
        virtual sal_Bool wait    ( const TimeValue* pTimeOut = NULL  ) = 0;

};      //  class IGate

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_THREADHELP_IGATE_H_
