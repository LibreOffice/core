/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLSectionFootnoteConfigExport.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:16:44 $
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


#ifndef _XMLOFF_XMLSECTIONFOOTNOTECONFIGEXPORT_HXX
#define _XMLOFF_XMLSECTIONFOOTNOTECONFIGEXPORT_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#include <vector>

class SvXMLExport;
class XMLPropertySetMapper;
struct XMLPropertyState;
template<class X> class UniReference;

/**
 * Export the footnote-/endnote-configuration element in section styles.
 *
 * Because this class contains only one method, and all information is
 * available during that method call, we simply make it static.
 */
class XMLSectionFootnoteConfigExport
{

public:
    static void exportXML(
        SvXMLExport& rExport,
        sal_Bool bEndnote,
        const ::std::vector<XMLPropertyState> * pProperties,
        sal_uInt32 nIdx,
        const UniReference<XMLPropertySetMapper> & rMapper);    /// used only for debugging
};

#endif
