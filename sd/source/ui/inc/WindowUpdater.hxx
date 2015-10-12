/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SD_SOURCE_UI_INC_WINDOWUPDATER_HXX
#define INCLUDED_SD_SOURCE_UI_INC_WINDOWUPDATER_HXX

#include <svl/lstner.hxx>
#include <svl/ctloptions.hxx>
#include <vcl/vclptr.hxx>
#include "sddllapi.h"

#include <vector>

namespace vcl { class Window; }
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
    : public utl::ConfigurationListener
{
public:
    explicit WindowUpdater();
    virtual ~WindowUpdater() throw();

    /** Add the given device to the list of devices which will be updated
        when one of the monitored values changes.
        @param pWindow
            This device is added to the device list if it is not <null/> and
            when it is not already a member of that list.
    */
    void RegisterWindow (vcl::Window* pWindow);

    /** Remove the given device from the list of devices which will be updated
        when one of the monitored values changes.
        @param pWindow
            This device is removed from the device list when it is a member
            of that list.
    */
    void UnregisterWindow (vcl::Window* pWindow);

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
    virtual void ConfigurationChanged ( utl::ConfigurationBroadcaster*, sal_uInt32 nHint) override;

private:
    /// Options to monitor for changes.
    SvtCTLOptions maCTLOptions;

    /// Keep the output devices of this view shell up to date.
    ViewShell* mpViewShell;

    /// The document rendered in the output devices.
    SdDrawDocument* mpDocument;

    WindowUpdater (const WindowUpdater& rUpdater) = delete;

    WindowUpdater operator= (const WindowUpdater& rUpdater) = delete;

    /** Type and data member for a list of devices that have to be kept
        up-to-date.
    */
    typedef ::std::vector< VclPtr<vcl::Window> > tWindowList;
    tWindowList maWindowList;

    /** The central method of this class.  Update the given output device.
        It is the task of the caller to initiate a refrormatting of the
        document that is rendered on this device to reflect the changes.
        @param pWindow
            The output device to update.  When it is <null/> then the call
            is ignored.
    */
    SAL_DLLPRIVATE void UpdateWindow (OutputDevice* pDevice) const;
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
