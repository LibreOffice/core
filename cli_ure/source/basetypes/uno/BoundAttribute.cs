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

using System;


namespace uno
{
/** is used to mark UNO interface attributes as &quot;bound&quot.

    <p>It corresponds to <const
    scope="com::sun::star::beans">PropertyAttribute::BOUND</const>.
    </p>
    UNO interface attributes are mapped to CLI properties.
 */
[AttributeUsage(AttributeTargets.Property, Inherited=false)]
public sealed class BoundAttribute: System.Attribute
{
}

}

