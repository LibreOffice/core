/*************************************************************************
 *
 *  $RCSfile: callbacks.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: obo $ $Date: 2003-10-21 08:38:06 $
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

#ifndef _XMLOFF_FORMS_CALLBACKS_HXX_
#define _XMLOFF_FORMS_CALLBACKS_HXX_

#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_SCRIPTEVENTDESCRIPTOR_HPP_
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif

class SvXMLImport;
class SvXMLExport;
class SvXMLExportPropertyMapper;
class SvXMLStyleContext;

//.........................................................................
namespace xmloff
{
//.........................................................................

    class OAttribute2Property;

    //=====================================================================
    //= IFormsExportContext
    //=====================================================================
    class IFormsExportContext
    {
    public:
        virtual SvXMLExport&                                getGlobalContext() = 0;
        virtual ::vos::ORef< SvXMLExportPropertyMapper >    getStylePropertyMapper() = 0;

        /** steps through a collection and exports all children of this collection
        */
        virtual void    exportCollectionElements(
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& _rxCollection) = 0;

        virtual ::rtl::OUString                             getObjectStyleName(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxObject ) = 0;
    };

    //=====================================================================
    //= IControlIdMap
    //=====================================================================
    class IControlIdMap
    {
    public:
        /** register a control id
        */
        virtual void    registerControlId(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControl,
            const ::rtl::OUString& _rId) = 0;

        /** register references to a control.

            <p>In the XML representation, the control which is refered by others stores the ids of these other
            controls, but in "real life" :) the referring controls store the referred one as property.</p>

            <p>This method allows a referred control to announce to ids of the referring ones.</p>

            @param _rxControl
                the referred control
            @param _rReferringControls
                a (comma separated) list of control ids of referring controls
        */
        virtual void    registerControlReferences(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControl,
            const ::rtl::OUString& _rReferringControls) = 0;
    };

    //=====================================================================
    //= IFormsImportContext
    //=====================================================================
    class IFormsImportContext
    {
    public:
        virtual IControlIdMap&              getControlIdMap() = 0;
        virtual OAttribute2Property&        getAttributeMap() = 0;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                                            getServiceFactory() = 0;
        virtual SvXMLImport&                getGlobalContext() = 0;

        virtual const SvXMLStyleContext*    getStyleElement(const ::rtl::OUString& _rStyleName) const = 0;

        virtual void                        enterEventContext() = 0;
        virtual void                        leaveEventContext() = 0;

        /** applies the given number style to the given object
        */
        virtual void applyControlNumberStyle(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControlModel,
            const ::rtl::OUString& _rControlNumerStyleName
        ) = 0;

        /** registers a control model for later binding to a spreadsheet cell value
        */
        virtual void                        registerCellValueBinding(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControlModel,
            const ::rtl::OUString& _rCellAddress
        ) = 0;

        /** registers a list-like control model for later binding to a spreadsheet cell range as list source
        */
        virtual void                        registerCellRangeListSource(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControlModel,
            const ::rtl::OUString& _rCellRangeAddress
        ) = 0;
    };

    //=====================================================================
    //= IEventAttacherManager
    //=====================================================================
    class IEventAttacherManager
    {
    public:
        virtual void registerEvents(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxElement,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >& _rEvents
            ) = 0;
    };

    //=====================================================================
    //= IEventAttacher
    //=====================================================================
    class IEventAttacher
    {
    public:
        virtual void registerEvents(
            const ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >& _rEvents
            ) = 0;
    };

//.........................................................................
}   // namespace xmloff
//.........................................................................

#endif // _XMLOFF_FORMS_CALLBACKS_HXX_

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.9.160.1  2003/10/01 09:55:17  fs
 *  #i18994# merging the changes from the CWS fs002
 *
 *  Revision 1.9.156.1  2003/09/25 14:28:35  fs
 *  #18994# merging the changes from cws_srx645_fs002 branch
 *
 *  Revision 1.9.152.1  2003/09/17 12:26:44  fs
 *  #18999# #19367# persistence for cell value and cell range bindings
 *
 *  Revision 1.9  2002/10/25 13:11:31  fs
 *  #104402# new methods on the import context callback for importing column styles
 *
 *  Revision 1.8  2002/10/25 07:35:35  fs
 *  #104402# +IFormsExportContext::getObjectStyleName
 *
 *  Revision 1.7  2001/02/01 09:46:47  fs
 *  no own style handling anymore - the shape exporter is responsible for our styles now
 *
 *  Revision 1.6  2001/01/24 09:34:10  fs
 *  +IFormsImportContext::enter-/leaveEventContext
 *
 *  Revision 1.5  2001/01/02 15:58:21  fs
 *  event ex- & import
 *
 *  Revision 1.4  2000/12/18 15:14:35  fs
 *  some changes ... now exporting/importing styles
 *
 *  Revision 1.3  2000/12/13 10:35:50  fs
 *  included XPropertySet.hpp
 *
 *  Revision 1.2  2000/12/12 12:00:37  fs
 *  +IControlIdMap / +IFormsImportContext
 *
 *  Revision 1.1  2000/11/17 19:01:13  fs
 *  initial checkin - export and/or import the applications form layer
 *
 *
 *  Revision 1.0 17.11.00 18:42:07  fs
 ************************************************************************/

