/*************************************************************************
 *
 *  $RCSfile: definesunocontrols.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:11:17 $
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

#ifndef _UNOCONTROLS_DEFINES_CTRL_HXX
#define _UNOCONTROLS_DEFINES_CTRL_HXX

//-------------------------------------------------------------------------------------------------------------------
// includes
#include <limits.h> // defines for min/max of INT

namespace unocontrols{

//-------------------------------------------------------------------------------------------------------------------
// global defines for all UnoControls
//-------------------------------------------------------------------------------------------------------------------
#define UNOCONTROLS_TRGB_COLORDATA( t,r,g,b )               ((INT32)(((UINT32)((UINT8)(b))))|(((UINT32)((UINT8)(g)))<<8)|(((UINT32)((UINT8)(r)))<<16)|(((UINT32)((UINT8)(t)))<<24))
//-------------------------------------------------------------------------------------------------------------------

// defines for BaseControl
#ifdef _UNOCONTROLS_BASECONTROL_CTRL_HXX
    #define BASECONTROL_IDLCLASSNAME                        "BaseControl"
    #define BASECONTROL_DEFAULT_PMULTIPLEXER                NULL
    #define BASECONTROL_DEFAULT_X                           0
    #define BASECONTROL_DEFAULT_Y                           0
    #define BASECONTROL_DEFAULT_WIDTH                       100
    #define BASECONTROL_DEFAULT_HEIGHT                      100
    #define BASECONTROL_DEFAULT_VISIBLE                     FALSE
    #define BASECONTROL_DEFAULT_INDESIGNMODE                FALSE
    #define BASECONTROL_DEFAULT_ENABLE                      TRUE
    #define BASECONTROL_SERVICE_VCLTOOLKIT                  "com.sun.star.awt.VclToolkit"
#endif
//-------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------
// defines for BaseContainerControl
#ifdef _UNOCONTROLS_BASECONTAINERCONTROL_CTRL_HXX
    #define BASECONTAINERCONTROL_IDLCLASSNAME               "BaseContainerControl"
#endif
//-------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------
// defines for ProgressBar
#ifdef _UNOCONTROLS_PROGRESSBAR_CTRL_HXX
    #define PROGRESSBAR_IDLCLASSNAME                        "XProgressbar"
    #define PROGRESSBAR_SERVICENAME                         "com.sun.star.awt.XProgressBar"
    #define PROGRESSBAR_IMPLEMENTATIONNAME                  "stardiv.UnoControls.ProgressBar"
    #define PROGRESSBAR_FREEBORDER                          2
    #define PROGRESSBAR_DEFAULT_HORIZONTAL                  TRUE
    #define PROGRESSBAR_DEFAULT_BLOCKDIMENSION              Size(1,1)
    #define PROGRESSBAR_DEFAULT_BACKGROUNDCOLOR             UNOCONTROLS_TRGB_COLORDATA( 0x00, 0xC0, 0xC0, 0xC0 )    // lightgray
    #define PROGRESSBAR_DEFAULT_FOREGROUNDCOLOR             UNOCONTROLS_TRGB_COLORDATA( 0x00, 0x00, 0x00, 0x80 )    // blue
    #define PROGRESSBAR_DEFAULT_MINRANGE                    INT_MIN
    #define PROGRESSBAR_DEFAULT_MAXRANGE                    INT_MAX
    #define PROGRESSBAR_DEFAULT_BLOCKVALUE                  1
    #define PROGRESSBAR_DEFAULT_VALUE                       PROGRESSBAR_DEFAULT_MINRANGE
    #define PROGRESSBAR_LINECOLOR_BRIGHT                    UNOCONTROLS_TRGB_COLORDATA( 0x00, 0xFF, 0xFF, 0xFF )    // white
    #define PROGRESSBAR_LINECOLOR_SHADOW                    UNOCONTROLS_TRGB_COLORDATA( 0x00, 0x00, 0x00, 0x00 )    // black
#endif
//-------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------
// defines for FrameControl
#ifdef _UNOCONTROLS_FRAMECONTROL_CTRL_HXX
    #define FRAMECONTROL_IDLCLASSNAME                       "FrameControl"
    #define FRAMECONTROL_SERVICENAME                        "com.sun.star.frame.FrameControl"
    #define FRAMECONTROL_IMPLEMENTATIONNAME                 "stardiv.UnoControls.FrameControl"
    #define FRAMECONTROL_DEFAULT_COMPONENTURL               "private:factory/swriter"
    #define FRAMECONTROL_PROPERTYNAME_LOADERARGUMENTS       "LoaderArguments"
    #define FRAMECONTROL_PROPERTYNAME_COMPONENTURL          "ComponentURL"
    #define FRAMECONTROL_PROPERTYNAME_FRAME                 "Frame"
    #define FRAMECONTROL_PROPERTYNAME_IDLCLASSES            "IDLClasses"
    #define FRAMECONTROL_ERRORTEXT_VOSENSHURE               "This is an invalid property handle."
    #define FRAMECONTROL_PROPERTY_COUNT                     4                                                       // you must count the propertys
    #define FRAMECONTROL_PROPERTY_COMPONENTURL              0                                                       // Id must be the index into the array
    #define FRAMECONTROL_PROPERTY_FRAME                     1
    #define FRAMECONTROL_PROPERTY_IDLCLASSES                2
    #define FRAMECONTROL_PROPERTY_LOADERARGUMENTS           3
#endif
//-------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------
// defines for ProgressMonitor
#ifdef _UNOCONTROLS_PROGRESSMONITOR_CTRL_HXX
    #define PROGRESSMONITOR_IDLCLASSNAME                    "XProgressMonitor"
    #define PROGRESSMONITOR_SERVICENAME                     "com.sun.star.awt.XProgressMonitor"
    #define PROGRESSMONITOR_IMPLEMENTATIONNAME              "stardiv.UnoControls.ProgressMonitor"
    #define PROGRESSMONITOR_FREEBORDER                      10                                                      // border around and between the controls
    #define PROGRESSMONITOR_WIDTH_RELATION                  4                                                       // reserve 1/4 for button width and rest for progressbar width
    #define PROGRESSMONITOR_HEIGHT_RELATION                 5                                                       // reserve 1/5 for button and progressbar heigth and rest for text height
    #define PROGRESSMONITOR_FIXEDTEXT_SERVICENAME           "com.sun.star.awt.UnoControlFixedText"
    #define PROGRESSMONITOR_BUTTON_SERVICENAME              "com.sun.star.awt.UnoControlButton"
    #define PROGRESSMONITOR_FIXEDTEXT_MODELNAME             "com.sun.star.awt.UnoControlFixedTextModel"
    #define PROGRESSMONITOR_BUTTON_MODELNAME                "com.sun.star.awt.UnoControlButtonModel"
    #define PROGRESSMONITOR_CONTROLNAME_TEXT                "Text"                                                  // identifier the control in container
    #define PROGRESSMONITOR_CONTROLNAME_BUTTON              "Button"                                                //              -||-
    #define PROGRESSMONITOR_CONTROLNAME_PROGRESSBAR         "ProgressBar"                                           //              -||-
    #define PROGRESSMONITOR_DEFAULT_BUTTONLABEL             "Abbrechen"
    #define PROGRESSMONITOR_DEFAULT_TOPIC                   "\0"
    #define PROGRESSMONITOR_DEFAULT_TEXT                    "\0"
    #define PROGRESSMONITOR_BACKGROUNDCOLOR                 UNOCONTROLS_TRGB_COLORDATA( 0x00, 0xC0, 0xC0, 0xC0 )    // lighgray
    #define PROGRESSMONITOR_LINECOLOR_BRIGHT                UNOCONTROLS_TRGB_COLORDATA( 0x00, 0xFF, 0xFF, 0xFF )    // white
    #define PROGRESSMONITOR_LINECOLOR_SHADOW                UNOCONTROLS_TRGB_COLORDATA( 0x00, 0x00, 0x00, 0x00 )    // black
    #define PROGRESSMONITOR_HEIGHT_PROGRESSBAR              15
    #define PROGRESSMONITOR_DEFAULT_WIDTH                   350
    #define PROGRESSMONITOR_DEFAULT_HEIGHT                  100
#endif
//-------------------------------------------------------------------------------------------------------------------

}   // namespace unocontrols

#endif  // #ifndef _UNOCONTROLS_DEFINES_CTRL_HXX
