/*************************************************************************
 *
 *  $RCSfile: WindowUpdater.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-01-27 14:16:42 $
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

#ifndef SD_OUTPUT_DEVICE_UPDATER_HXX
#define SD_OUTPUT_DEVICE_UPDATER_HXX

#ifndef _SFXLSTNER_HXX
#include <svtools/lstner.hxx>
#endif
#ifndef _SVTOOLS_CTLOPTIONS_HXX
#include <svtools/ctloptions.hxx>
#endif

#ifndef INCLUDED_SDDLLAPI_H
#include "sddllapi.h"
#endif

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

class Window;
class OutputDevice;
class SdDrawDocument;


namespace sd {

class ViewShell;

/** The purpose of the <type>WindowUpdater</type> is to update output
    devices to take care of modified global values.  These values are
    monitored for changes.  At the moment this is
    the digit language that defines the glyphs to use to render digits.
    Other values may be added in the future.

    <p>The methods of this class have not been included into the
    <type>ViewShell</type> class in order to not clutter its interface any
    further.  This class accesses some of <type>ViewShell</type> data
    members directly and thus is declared as its friend.</p>

    <p>Windows that are to be kept up-to-date have to be registered via the
    <member>RegisterWindow()</member> method.  When a document is given then
    this document is reformatted when the monitored option changes.</p>
*/
class SD_DLLPUBLIC WindowUpdater
    : public SfxListener
{
public:
    explicit WindowUpdater (void);
    virtual ~WindowUpdater (void) throw();

    /** Add the given device to the list of devices which will be updated
        when one of the monitored values changes.
        @param pWindow
            This device is added to the device list if it is not <null/> and
            when it is not already a member of that list.
    */
    void RegisterWindow (::Window* pWindow);

    /** Remove the given device from the list of devices which will be updated
        when one of the monitored values changes.
        @param pWindow
            This device is removed from the device list when it is a member
            of that list.
    */
    void UnregisterWindow (::Window* pWindow);

    /** Set the view shell whose output devices shall be kept up to date.
        It is used to clear the master page cache so that a redraw affects
        the master page content as well.
    */
    void SetViewShell (ViewShell& rViewShell);

    /** Set the document so that it is reformatted when one of the monitored
        values changes.
        @param pDocument
            When <null/> is given document reformatting will not take
            place in the future.
    */
    void SetDocument (SdDrawDocument* pDocument);

    /** Update the given output device and update all text objects of the
        view shell if not told otherwise.
        @param pWindow
            The device to update.  When the given pointer is NULL then
            nothing is done.
        @param pDocument
            When given a pointer to a document then tell it to reformat all
            text objects.  This refromatting is necessary for the new values
            to take effect.
    */
    void Update (OutputDevice* pDevice, SdDrawDocument* pDocument=0) const;

    /** Callback that waits for notifications of a
        <type>SvtCTLOptions</type> object.
    */
    virtual void Notify (SfxBroadcaster& rBC, const SfxHint& rHint);

private:
    /// Options to monitor for changes.
    SvtCTLOptions maCTLOptions;

    /// Keep the output devices of this view shell up to date.
    ViewShell* mpViewShell;

    /// The document rendered in the output devices.
    SdDrawDocument* mpDocument;

    /// Copy constructor not supported.
    WindowUpdater (const WindowUpdater& rUpdater);

    /// Assignment operator not supported.
    WindowUpdater operator= (const WindowUpdater& rUpdater);

    /** Type and data member for a list of devices that have to be kept
        up-to-date.
    */
    typedef ::std::vector< ::Window*> tWindowList;
    tWindowList maWindowList;

    /** The central method of this class.  Update the given output device.
        It is the task of the caller to initiate a refrormatting of the
        document that is rendered on this device to reflect the changes.
        @param pWindow
            The output device to update.  When it is <null/> then the call
            is ignored.
    */
    SD_DLLPRIVATE void UpdateWindow (OutputDevice* pDevice) const;
};

} // end of namespace sd

#endif
