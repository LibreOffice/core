/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salnativewidgets-none.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 12:11:27 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

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
BOOL SalGraphics::IsNativeControlSupported( ControlType, ControlPart )
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
BOOL SalGraphics::hitTestNativeControl( ControlType,
                              ControlPart,
                              const Region&,
                              const Point&,
                              SalControlHandle&,
                              BOOL& )
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
BOOL SalGraphics::drawNativeControl(    ControlType,
                            ControlPart,
                            const Region&,
                            ControlState,
                            const ImplControlValue&,
                            SalControlHandle&,
                            const OUString& )
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
BOOL SalGraphics::drawNativeControlText(    ControlType,
                                ControlPart,
                                const Region&,
                                ControlState,
                                const ImplControlValue&,
                                SalControlHandle&,
                                const OUString& )
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
BOOL SalGraphics::getNativeControlRegion(  ControlType,
                                ControlPart,
                                const Region&,
                                ControlState,
                                const ImplControlValue&,
                                SalControlHandle&,
                                const OUString&,
                                Region &,
                                Region & )
{
    return( FALSE );
}

