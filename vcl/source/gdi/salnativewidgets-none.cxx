/*************************************************************************
 *
 *  $RCSfile: salnativewidgets-none.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 15:49:20 $
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
 *  Contributor(s): Juergen Keil
 *
 *
 ************************************************************************/

#include <salgdi.hxx>

using namespace rtl;

/****************************************************************
 *  Placeholder for no native widgets
 ***************************************************************/


/*
 * IsNativeControlSupported()
 *
 *  Returns TRUE if the platform supports native
 *  drawing of the control defined by nPart
 */
BOOL SalGraphics::IsNativeControlSupported( ControlType nType, ControlPart nPart )
{
    return( FALSE );
}


/*
 * HitTestNativeControl()
 *
 *  If the return value is TRUE, bIsInside contains information whether
 *  aPos was or was not inside the native widget specified by the
 *  nType/nPart combination.
 */
BOOL SalGraphics::hitTestNativeControl( ControlType nType,
                              ControlPart nPart,
                              const Region& rControlRegion,
                              const Point& aPos,
                              SalControlHandle& rControlHandle,
                              BOOL& rIsInside )
{
    return( FALSE );
}


/*
 * DrawNativeControl()
 *
 *  Draws the requested control described by nPart/nState.
 *
 *  rControlRegion: The bounding region of the complete control in VCL frame coordinates.
 *  aValue:         An optional value (tristate/numerical/string)
 *  rControlHandle: Carries platform dependent data and is maintained by the SalFrame implementation.
 *  aCaption:   A caption or title string (like button text etc)
 */
BOOL SalGraphics::drawNativeControl(    ControlType nType,
                            ControlPart nPart,
                            const Region& rControlRegion,
                            ControlState nState,
                            const ImplControlValue& aValue,
                            SalControlHandle& rControlHandle,
                            OUString aCaption )
{
    return( FALSE );
}


/*
 * DrawNativeControlText()
 *
 *  OPTIONAL.  Draws the requested text for the control described by nPart/nState.
 *     Used if text not drawn by DrawNativeControl().
 *
 *  rControlRegion: The bounding region of the complete control in VCL frame coordinates.
 *  aValue:         An optional value (tristate/numerical/string)
 *  rControlHandle: Carries platform dependent data and is maintained by the SalFrame implementation.
 *  aCaption:   A caption or title string (like button text etc)
 */
BOOL SalGraphics::drawNativeControlText(    ControlType nType,
                                ControlPart nPart,
                                const Region& rControlRegion,
                                ControlState nState,
                                const ImplControlValue& aValue,
                                SalControlHandle& rControlHandle,
                                OUString aCaption )
{
    return( FALSE );
}


/*
 * GetNativeControlRegion()
 *
 *  If the return value is TRUE, rNativeBoundingRegion
 *  contains the TRUE bounding region covered by the control
 *  including any adornment, while rNativeContentRegion contains the area
 *  within the control that can be safely drawn into without drawing over
 *  the borders of the control.
 *
 *  rControlRegion: The bounding region of the control in VCL frame coordinates.
 *  aValue:     An optional value (tristate/numerical/string)
 *  rControlHandle: Carries platform dependent data and is maintained by the SalFrame implementation.
 *  aCaption:       A caption or title string (like button text etc)
 */
BOOL SalGraphics::getNativeControlRegion(  ControlType nType,
                                ControlPart nPart,
                                const Region& rControlRegion,
                                ControlState nState,
                                const ImplControlValue& aValue,
                                SalControlHandle& rControlHandle,
                                OUString aCaption,
                                Region &rNativeBoundingRegion,
                                Region &rNativeContentRegion )
{
    return( FALSE );
}

