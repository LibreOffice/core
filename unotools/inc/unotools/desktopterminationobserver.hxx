/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: desktopterminationobserver.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:31:36 $
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
#ifndef INCLUDED_UNOTOOLSDLLAPI_H
#include "unotools/unotoolsdllapi.h"
#endif

#ifndef UNOTOOLS_INC_UNOTOOLS_DESKTOPTERMINATIONOBSERVER_HXX
#define UNOTOOLS_INC_UNOTOOLS_DESKTOPTERMINATIONOBSERVER_HXX

/** === begin UNO includes === **/
/** === end UNO includes === **/

//........................................................................
namespace utl
{
//........................................................................

    //====================================================================
    //= ITerminationListener
    //====================================================================
    /** non-UNO version of the <type scope="com.sun.star.frame">XTerminateListener</type>
    */
    class ITerminationListener
    {
    public:
        virtual bool    queryTermination() const = 0;
        virtual void    notifyTermination() = 0;
    };

    //====================================================================
    //= DesktopTerminationObserver
    //====================================================================
    /** a class which allows non-UNO components to observe the desktop (aka application)
        for it's shutdown
    */
    class UNOTOOLS_DLLPUBLIC DesktopTerminationObserver
    {
    public:
        /** registers a listener which should be notified when the desktop terminates
            (which means the application is shutting down)
        */
        static void    registerTerminationListener( ITerminationListener* _pListener );

        /** revokes a termination listener
        */
        static void    revokeTerminationListener( ITerminationListener* _pListener );

    private:
        DesktopTerminationObserver();   // never implemented, only static methods
    };

//........................................................................
} // namespace utl
//........................................................................

#endif // UNOTOOLS_INC_UNOTOOLS_DESKTOPTERMINATIONOBSERVER_HXX

