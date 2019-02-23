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

#ifndef INCLUDED_JVMFWK_PLUGINS_SUNMAJOR_PLUGINLIB_SUNVERSION_HXX
#define INCLUDED_JVMFWK_PLUGINS_SUNMAJOR_PLUGINLIB_SUNVERSION_HXX

#include <rtl/ustring.hxx>

namespace jfw_plugin {
// Define OSL_DEBUG_LEVEL >= 2 to run a test when this lib is loaded

/* SunVersion is used to compare java versions based on a string, as taken
   from the registry. The strings look like "1.3", "1.3.1", "1.3.1_02" etc.
   Versions such as "1.4.1_01a" are allowed although this is not specified.
   1.4.1_01 < 1.4.1_01a < 1.4.1_01b < 1.4.1_02
   Pre - release versions, such as 1.4.1-ea, 1.4.1-beta, 1.4.1-rc are recognized,
   but are treated as minor to release versions:
   1.4.0 > 1.4.2-beta
   Pre releases relate this way
   1.4.1-ea < 1.4.1-beta < 1.4.1-rc1

   This class supports also a FreeBSD Java. This is currently necessary because
   it also has the vendor string "Sun Microsystems Inc.".

   An object acts as holder for the version string. That string may be present
   even if the version could not be parsed. Then the version may not be compatible
   to a SUN Java version.

   An invalid object, that is, operator bool returns false, will always be
   the lower version in a comparison. If two invalid objects are compared
   then they are considered equal.

   To test if the version is ok, that is this object can be compared to others,
   use the bool conversion operator.
 */
class SunVersion final
{
    enum PreRelease
    {
        Rel_NONE,
        Rel_INTERNAL,
        Rel_EA,
        Rel_EA1,
        Rel_EA2,
        Rel_EA3,
        Rel_BETA,
        Rel_BETA1,
        Rel_BETA2,
        Rel_BETA3,
        Rel_RC,
        Rel_RC1,
        Rel_RC2,
        Rel_RC3
#if defined(FREEBSD)
    ,
        Rel_FreeBSD
#endif
    };

    //contains major,minor,micro,update
    int m_arVersionParts[4];
    // The update can be followed by a char, e.g. 1.4.1_01a
    char m_nUpdateSpecial;

    PreRelease m_preRelease;
public:
    explicit SunVersion(const char * szVer);
    explicit SunVersion(const OUString& usVer);
    ~SunVersion();

    /**
       Pre-release versions are taken into account.
       1.5.0-beta > 1.5.0-ea > 1.4.2
     */
    bool operator > (const SunVersion& ver) const;
    bool operator < (const SunVersion& ver) const;
    bool operator == (const SunVersion& ver) const;

    /** Test if the version is compatible tu SUN's versioning scheme
     */
    operator bool () { return m_bValid;}

private:
    bool init(const char * szVer);

    bool m_bValid;

    /* Determines if a string constitutes a pre release. For example, if
       "ea" is passed then Rel_EA is returned. If the string is no pre release
       then Rel_NONE is returned.
    */
    static PreRelease getPreRelease(const char *szRel);
};

}

#endif // INCLUDED_JVMFWK_PLUGINS_SUNMAJOR_PLUGINLIB_SUNVERSION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
