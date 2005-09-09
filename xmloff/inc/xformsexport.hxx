/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xformsexport.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 13:12:03 $
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
#ifndef _XMLOFF_XFORMSEXPORT_HXX
#define _XMLOFF_XFORMSEXPORT_HXX

class SvXMLExport;
namespace com { namespace sun { namespace star {
    namespace uno { template<typename T> class Reference; }
    namespace frame { class XModel; }
    namespace beans { class XPropertySet; }
} } }
namespace rtl { class OUString; }


/** export an XForms model. */
void SAL_DLLPRIVATE exportXForms( SvXMLExport& );

rtl::OUString SAL_DLLPRIVATE getXFormsBindName( const com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet>& xBinding );

rtl::OUString SAL_DLLPRIVATE getXFormsListBindName( const com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet>& xBinding );

rtl::OUString SAL_DLLPRIVATE getXFormsSubmissionName( const com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet>& xBinding );

#endif
