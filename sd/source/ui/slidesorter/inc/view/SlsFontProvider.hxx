/*************************************************************************
 *
 *  $RCSfile: SlsFontProvider.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-03-18 16:52:29 $
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
 *  The Initial Developer of the Original Ckode is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SD_SLIDESORTER_VIEW_FONT_PROVIDER_HXX
#define SD_SLIDESORTER_VIEW_FONT_PROVIDER_HXX

#include "tools/SdGlobalResourceContainer.hxx"

#include <boost/shared_ptr.hpp>
#include <vcl/mapmod.hxx>

class Font;
class OutputDevice;
class VclWindowEvent;

namespace sd { namespace slidesorter { namespace view {

/** This simple singleton class provides fonts that are scaled so that they
    have a fixed height on the given device regardless of its map mode.
*/
class FontProvider
    : public SdGlobalResource
{
public:
    typedef ::boost::shared_ptr<Font> SharedFontPointer;

    /** Return the single instance of this class.  Throws a RuntimeException
        when no instance can be created.
    */
    static FontProvider& Instance (void);

    /** Return a font that is scaled according to the current map mode of
        the given device.  Repeated calls with a device, not necessarily the
        same device, with the same map mode will return the same font.  The
        first call with a different map mode will release the old font and
        create a new one that is correctly scaled.
    */
    SharedFontPointer GetFont (const OutputDevice& rDevice);

    /** Call this method to tell an object to release its currently used
        font.  The next call to GetFont() will then create a new one.
        Typically called after a DataChange event when for instance a system
        font has been modified.
    */
    void Invalidate (void);

private:
    static FontProvider* mpInstance;

    /** The font that was created by a previous call to GetFont().  It is
        replaced by another one only when GetFont() is called with a device
        with a different map mode or by a call to Invalidate().
    */
    SharedFontPointer maFont;
    /** The mape mode for which maFont was created.
    */
    MapMode maMapMode;

    FontProvider (void);
    virtual ~FontProvider (void);

    // Copy constructor is not implemented.
    FontProvider (const FontProvider&);
    // Assignment operator is not implemented.
    FontProvider& operator= (const FontProvider&);
};

} } } // end of namespace ::sd::slidesorter::view

#endif
