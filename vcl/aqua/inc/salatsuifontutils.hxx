/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salatsuifontutils.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2007-10-09 15:08:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SV_SALATSUIFONTUTILS_HXX
#define _SV_SALATSUIFONTUTILS_HXX

class ImplMacFontData;
class ImplDevFontList;

#include <vector>

#include <premac.h>
#include <ApplicationServices/ApplicationServices.h>
#include <postmac.h>

/* This class has the responsibility of assembling a list
   of atsui compatible fonts available on the system and
   enabling access to that list.
 */
class SystemFontList
{
public:

    SystemFontList();
    ~SystemFontList();

    void AnnounceFonts( ImplDevFontList& ) const;

    ATSUFontFallbacks maFontFallbacks;

private:
  typedef std::vector<ImplMacFontData*> ImplFontDataContainer_t;
  ImplFontDataContainer_t mFontContainer;

    void InitGlyphFallbacks();
};

#endif  // _SV_SALATSUIFONTUTILS_HXX

