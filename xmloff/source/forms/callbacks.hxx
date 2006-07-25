/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: callbacks.hxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-25 09:21:47 $
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

        /** registers a control model for later binding to an XForms binding */
        virtual void                        registerXFormsValueBinding(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControlModel,
            const ::rtl::OUString& _rBindingID
        ) = 0;

        /** registers a control model for later binding an XForms list binding */
        virtual void                        registerXFormsListBinding(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControlModel,
            const ::rtl::OUString& _rBindingID
        ) = 0;

        /** registers a control model for later binding to an XForms submission */
        virtual void                        registerXFormsSubmission(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControlModel,
            const ::rtl::OUString& _rBindingID
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

