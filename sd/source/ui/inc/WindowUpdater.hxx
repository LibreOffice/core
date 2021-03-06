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

#pragma once

#include <svl/ctloptions.hxx>
#include <vcl/vclptr.hxx>

#include <vector>

namespace vcl
{
class Window;
}
class OutputDevice;
class SdDrawDocument;

namespace sd
{
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
class WindowUpdater : public utl::ConfigurationListener
{
public:
    explicit WindowUpdater();
    virtual ~WindowUpdater() throw() override;

    /** Add the given device to the list of devices which will be updated
        when one of the monitored values changes.
        @param pWindow
            This device is added to the device list if it is not <null/> and
            when it is not already a member of that list.
    */
    void RegisterWindow(vcl::Window* pWindow);

    /** Remove the given device from the list of devices which will be updated
        when one of the monitored values changes.
        @param pWindow
            This device is removed from the device list when it is a member
            of that list.
    */
    void UnregisterWindow(vcl::Window* pWindow);

    /** Set the document so that it is reformatted when one of the monitored
        values changes.
        @param pDocument
            When <null/> is given document reformatting will not take
            place in the future.
    */
    void SetDocument(SdDrawDocument* pDocument);

    /** Update the given output device and update all text objects of the
        view shell if not told otherwise.
        @param pWindow
            The device to update.  When the given pointer is NULL then
            nothing is done.
    */
    void Update(OutputDevice* pDevice) const;

    /** Callback that waits for notifications of a
        <type>SvtCTLOptions</type> object.
    */
    virtual void ConfigurationChanged(utl::ConfigurationBroadcaster*,
                                      ConfigurationHints nHint) override;

private:
    /// Options to monitor for changes.
    SvtCTLOptions maCTLOptions;

    /// The document rendered in the output devices.
    SdDrawDocument* mpDocument;

    WindowUpdater(const WindowUpdater& rUpdater) = delete;

    WindowUpdater operator=(const WindowUpdater& rUpdater) = delete;

    /** Type and data member for a list of devices that have to be kept
        up-to-date.
    */
    typedef ::std::vector<VclPtr<vcl::Window>> tWindowList;
    tWindowList maWindowList;

    /** The central method of this class.  Update the given output device.
        It is the task of the caller to initiate a reformatting of the
        document that is rendered on this device to reflect the changes.
        @param pWindow
            The output device to update.  When it is <null/> then the call
            is ignored.
    */
    void UpdateWindow(OutputDevice* pDevice) const;
};

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
