/*************************************************************************
 *
 *  $RCSfile: sunversion.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 11:52:30 $
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

#if !defined INCLUDED_JVMACCESS_SUNVERSION_HXX
#define INCLUDED_JVMACCESS_SUNVERSION_HXX

#include "rtl/ustring.hxx"

namespace jfw_plugin {
// Define SUNVERSION_SELFTEST to run a test when this lib is loaded
//#define SUNVERSION_SELFTEST

/* SunVersion is used to compare java versions based on a string, as taken
   from the registry. The strings look like "1.3", "1.3.1", "1.3.1_02" etc.
   Versions such as "1.4.1_01a" are allowed although this is not specified.
   1.4.1_01 < 1.4.1_01a < 1.4.1_01b < 1.4.1_02
   Pre - release versions, such as 1.4.1-ea, 1.4.1-beta, 1.4.1-rc are recognized,
   but are treated as minor to release versions:
   1.4.0 > 1.4.2-beta
   Pre releases relate this way
   1.4.1-ea < 1.4.1-beta < 1.4.1-rc1

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
};

}

#endif // INCLUDED_JVMACCESS_SUNVERSION_HXX
