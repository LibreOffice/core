/*************************************************************************
 *
 *  $RCSfile: outdevprovider.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: thb $ $Date: 2004-03-18 10:38:43 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _VCLCANVAS_OUTDEVPROVIDER_HXX
#define _VCLCANVAS_OUTDEVPROVIDER_HXX

#ifndef _COMPHELPER_IMPLEMENTATIONREFERENCE_HXX
#include <comphelper/implementationreference.hxx>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_XCANVAS_HPP_
#include <drafts/com/sun/star/rendering/XCanvas.hpp>
#endif

class OutputDevice;

namespace vclcanvas
{
    /* Definition of OutDevProvider interface */

    /** Helper interface to connect CanvasBase with its various
        clients.
    */

    // The problem here is the fact that first of all, XCanvas and its
    // specialised interfaces form an inheritance hierarchy. Thus,
    // every client of a base class implementing the commons of
    // canvases, will have to implement all interface methods, and
    // forward them to the base class. Thus, there's no real gain in
    // using implementation inheritance here. If we instead use a
    // helper class to be held as a member by its client, we have the
    // problem that several base methods serve as object factories,
    // generating objects which require links to the canvas
    // implementation object _and_ a UNO reference. The latter is for
    // lifetime issues, and should better be directly from the client
    // object.

    // Therefore, every client of CanvasBase must implement this
    // interface and pass a pointer to it to the CanvasBase
    // instance. The XInterface base class is necessary to have basic
    // UNO reference semantics.

    class OutDevProvider : public ::com::sun::star::uno::XInterface
    {
    public:
        /** Use this type to store a C++ pointer alongside a UNO
            reference to this interface.

            This is advantageous e.g. for CanvasBase, where several
            helper methods (color conversion etc.) need access to the
            XCanvas UNO interface. Performing a QueryInterface
            everytime is a real performance killer there.
         */
        typedef ::comphelper::ImplementationReference<
            OutDevProvider,
            ::drafts::com::sun::star::rendering::XCanvas,
            ::com::sun::star::uno::XInterface > ImplRef;

        virtual ~OutDevProvider() {}

        virtual OutputDevice&       getOutDev() = 0;
        virtual const OutputDevice& getOutDev() const = 0;
    };
}

#endif /* _VCLCANVAS_OUTDEVPROVIDER_HXX */
