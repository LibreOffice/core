/*************************************************************************
 *
 *  $RCSfile: AccessibleOutlineEditSource.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: thb $ $Date: 2002-06-13 18:50:10 $
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

#ifndef _SD_ACCESSIBILITY_ACCESSIBLE_OUTLINE_EDITSOURCE_HXX
#define _SD_ACCESSIBILITY_ACCESSIBLE_OUTLINE_EDITSOURCE_HXX

#include <memory>

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _SFXBRDCST_HXX
#include <svtools/brdcst.hxx>
#endif

#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif

#ifndef _SVX_UNOEDSRC_HXX
#include <svx/unoedsrc.hxx>
#endif

#ifndef _MyEDITDATA_HXX
#include <svx/editdata.hxx>
#endif

#ifndef _SVX_UNOFOROU_HXX
#include <svx/unoforou.hxx>
#endif

#ifndef _SVX_UNOVIWOU_HXX
#include <svx/unoviwou.hxx>
#endif

class SdrOutliner;
class SdrView;
class OutlinerView;
class Window;

namespace accessibility
{
    class AccessibleOutlineEditSource_Impl;

    /** Implementation of the SvxEditSource interface in the SdOutlineView

        This class connects the SdOutlineView and its EditEngine
        outliner with the AccessibleTextHelper, which provides all
        necessary functionality to make the outliner text accessible

        @see SvxEditSource
        @see SvxViewForwarder
    */
    class AccessibleOutlineEditSource : public SvxEditSource, public SvxViewForwarder, public SfxBroadcaster, public SfxListener
    {
    public:
        /// Create an SvxEditSource interface for the given Outliner
        AccessibleOutlineEditSource( SdrOutliner& rOutliner, SdrView& rView, OutlinerView& rOutlView, const Window& rViewWindow );
        virtual ~AccessibleOutlineEditSource();

        /// This method is disabled and always returns NULL
        virtual SvxEditSource*          Clone() const;
        virtual SvxTextForwarder*       GetTextForwarder();
        virtual SvxViewForwarder*       GetViewForwarder();
        virtual SvxEditViewForwarder*   GetEditViewForwarder( sal_Bool bCreate = sal_False );
        virtual void                    UpdateData();
        virtual SfxBroadcaster&         GetBroadcaster() const;

        // the view forwarder
        virtual BOOL        IsValid() const;
        virtual Rectangle   GetVisArea() const;
        virtual Point       LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const;
        virtual Point       PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const;

        // SfxListener
        virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    private:

        // declared, but not defined
        AccessibleOutlineEditSource( const AccessibleOutlineEditSource& );
        AccessibleOutlineEditSource& operator=( const AccessibleOutlineEditSource& );

        DECL_LINK( NotifyHdl, EENotify* );

        SdrView&                        mrView;
        const Window&                   mrWindow;
        SdrOutliner*                    mpOutliner;
        OutlinerView*                   mpOutlinerView;

        SvxOutlinerForwarder            mTextForwarder;
        SvxDrawOutlinerViewForwarder    mViewForwarder;

    };

} // end of namespace accessibility

#endif
