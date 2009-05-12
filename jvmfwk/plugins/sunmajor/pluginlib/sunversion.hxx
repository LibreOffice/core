/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sunversion.hxx,v $
 * $Revision: 1.8 $
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

#if !defined INCLUDED_JVMACCESS_SUNVERSION_HXX
#define INCLUDED_JVMACCESS_SUNVERSION_HXX

#include "rtl/ustring.hxx"

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
class SunVersion
{
protected:

    enum PreRelease
    {
        Rel_NONE,
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
    SunVersion();
    SunVersion(const char * szVer);
    SunVersion(const rtl::OUString& usVer);
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
    operator bool ();

    /** Will always contain a value if the object has been constructed with
        a version string.
     */
    rtl::OUString usVersion;

protected:
    bool init(const char * szVer);

    bool m_bValid;

    /* Determines if a string constitutes a pre release. For example, if
       "ea" is passed then Rel_EA is returned. If the string is no pre release
       then Rel_NONE is returned.
    */
    PreRelease getPreRelease(const char *szRel);
};

}

#endif // INCLUDED_JVMACCESS_SUNVERSION_HXX
