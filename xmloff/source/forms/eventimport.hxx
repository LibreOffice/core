/*************************************************************************
 *
 *  $RCSfile: eventimport.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2001-01-02 15:56:11 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _XMLOFF_FORMS_EVENTIMPORT_HXX_
#define _XMLOFF_FORMS_EVENTIMPORT_HXX_

#ifndef _XMLOFF_XMLEVENTSIMPORTCONTEXT_HXX
#include "XMLEventsImportContext.hxx"
#endif
#ifndef _XMLOFF_FORMS_CALLBACKS_HXX_
#include "callbacks.hxx"
#endif
#ifndef _XMLOFF_FORMS_IFACECOMPARE_HXX_
#include "ifacecompare.hxx"
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif

class SvXMLImport;
//.........................................................................
namespace xmloff
{
//.........................................................................

    //=====================================================================
    //= OFormEventsImportContext
    //=====================================================================
    class OFormEventsImportContext : public XMLEventsImportContext
    {
    protected:
        IEventAttacher& m_rEventAttacher;

    public:
        OFormEventsImportContext(SvXMLImport& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName,
            IEventAttacher& _rEventAttacher);

    protected:
        virtual void EndElement();
    };

    //=====================================================================
    //= ODefaultEventAttacherManager
    //=====================================================================
    class ODefaultEventAttacherManager : public IEventAttacherManager
    {
    protected:
        DECLARE_STL_MAP(
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >,              // map from
            ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >,     // map to
            OPropertySetCompare,                                                                    // compare method
            MapPropertySet2ScriptSequence);                                                         // class name
        // usually an event attacher manager will need to collect all script events registered, 'cause
        // the _real_ XEventAttacherManager handles it's events by index, but out indicies are not fixed
        // until _all_ controls have been inserted.

        MapPropertySet2ScriptSequence   m_aEvents;

    public:
        // IEventAttacherManager
        virtual void registerEvents(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxElement,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >& _rEvents
            );

    protected:
        void setEvents(
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& _rxContainer
            );
    };

//.........................................................................
}   // namespace xmloff
//.........................................................................

#endif // _XMLOFF_FORMS_EVENTIMPORT_HXX_

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *
 *  Revision 1.0 21.12.00 15:18:21  fs
 ************************************************************************/

