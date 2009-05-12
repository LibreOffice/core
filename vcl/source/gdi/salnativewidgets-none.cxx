/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salnativewidgets-none.cxx,v $
 * $Revision: 1.7 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <vcl/salgdi.hxx>

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

