/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLStarBasicExportHandler.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 12:56:30 $
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

#ifndef _XMLOFF_XMLSTARBASICEXPORTHANDLER_HXX
#define _XMLOFF_XMLSTARBASICEXPORTHANDLER_HXX

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring>
#endif

#ifndef _XMLOFF_XMLEVENT_HXX
#include "xmlevent.hxx"
#endif

#include <map>

class SvXMLExport;
namespace com { namespace sun { namespace star {
    namespace beans { struct PropertyValue; }
} } }

class XMLStarBasicExportHandler : public XMLEventExportHandler
{
    const ::rtl::OUString sStarBasic;
    const ::rtl::OUString sLibrary;
    const ::rtl::OUString sMacroName;
    const ::rtl::OUString sStarOffice;
    const ::rtl::OUString sApplication;

public:
    XMLStarBasicExportHandler();
    virtual ~XMLStarBasicExportHandler();

    virtual void Export(
        SvXMLExport& rExport,
        const ::rtl::OUString& rEventName,
        ::com::sun::star::uno::Sequence<
            ::com::sun::star::beans::PropertyValue> & rValues,
        sal_Bool bUseWhitespace);
};

#endif
