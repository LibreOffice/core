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

#ifndef INCLUDED_SVX_IACCESSIBLEVIEWFORWARDERLISTENER_HXX
#define INCLUDED_SVX_IACCESSIBLEVIEWFORWARDERLISTENER_HXX

namespace accessibility
{
/** <p>The purpose of this interface is to notify a user of an
    IAccessibleViewForwarder when that view forwarder changes its
    properties.  Such a change may be one of the following:
    <ul>
    <li>Change of the coordinate transformation.</li>
    <li>Change of the visible area (which in turn results in a change of the
    coordinate transformation.</li>
    <li>Change of the validity state of the view forwarder.</li>
*/
class IAccessibleViewForwarderListener
{
public:
    /** This method is called to indicate a change of the specified view
        forwarder, specifically, a change in visible area.
    */
    virtual void ViewForwarderChanged() = 0;

protected:
    ~IAccessibleViewForwarderListener() {}
};

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
