/*************************************************************************
 *
 *  $RCSfile: AccessibleEmptyEditSource.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-01 13:47:03 $
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

#ifndef _SVX_ACCESSIBLEEMPTYEDITSOURCE_HXX
#define _SVX_ACCESSIBLEEMPTYEDITSOURCE_HXX

#ifndef _SFXBRDCST_HXX
#include <svtools/brdcst.hxx>
#endif
#ifndef _SFXLSTNER_HXX
#include <svtools/lstner.hxx>
#endif

#include <memory>
#include "unoedsrc.hxx"

class SdrObject;
class SdrView;
class Window;

namespace accessibility
{
    /** Proxy edit source for shapes without text

        Extracted from old SvxDummyEditSource
     */
    class AccessibleEmptyEditSource : public SvxEditSource, public SfxListener, public SfxBroadcaster
    {
    public:
        /** Create proxy edit source for shapes without text

            Since the views don't broadcast their dying, make sure that
            this object gets destroyed if the view becomes invalid

            The window is necessary, since our views can display on multiple windows

            Make sure you only create such an object if the shape _really_
            does not contain text.
        */
        AccessibleEmptyEditSource(  SdrObject& rObj, SdrView& rView, const Window& rViewWindow );
        ~AccessibleEmptyEditSource();

        // from the SvxEditSource interface
        SvxTextForwarder*       GetTextForwarder();
        SvxViewForwarder*       GetViewForwarder();

        SvxEditSource*          Clone() const;

        // this method internally switches from empty to proxy mode,
        // creating an SvxTextEditSource for the functionality.
        SvxEditViewForwarder*   GetEditViewForwarder( sal_Bool bCreate = sal_False );

        void                    UpdateData();
        SfxBroadcaster&         GetBroadcaster() const;

        // from the SfxListener interface
        void                    Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    private:
        void Switch2ProxyEditSource();

        /** Pointer to edit source implementation. This is switched on
            a GetEditViewForwarder( true ) call, to actually create a
            SvxTextEditSource.

            @dyn
         */
        std::auto_ptr< SvxEditSource >  mpEditSource;

        SdrObject&                      mrObj;
        SdrView&                        mrView;
        const Window&                   mrViewWindow;

        bool                            mbEditSourceEmpty;
    };

} // namespace accessibility

#endif

