/*************************************************************************
 *
 *  $RCSfile: layerimport.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-20 13:39:58 $
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

#ifndef _XMLOFF_FORMS_LAYERIMPORT_HXX_
#define _XMLOFF_FORMS_LAYERIMPORT_HXX_

#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif
#include <com/sun/star/container/XNameContainer.hpp>
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif
#ifndef _XMLOFF_FORMATTRIBUTES_HXX_
#include "formattributes.hxx"
#endif
#ifndef _XMLOFF_FORMS_CALLBACKS_HXX_
#include "callbacks.hxx"
#endif
#ifndef _XMLOFF_FORMS_EVENTIMPORT_HXX_
#include "eventimport.hxx"
#endif
#ifndef _REF_HXX
#include <tools/ref.hxx>
#endif

class SvXMLImport;
class SvXMLImportContext;
class XMLPropertyHandlerFactory;
class SvXMLImportPropertyMapper;

SV_DECL_REF( SvXMLStylesContext );
    // unfortunately, we can't put this into our namespace, as the macro expands to (amongst others) a forward
    // declaration of the class name, which then would be in the namespace, too

//.........................................................................
namespace xmloff
{
//.........................................................................

    class OAttribute2Property;

    //=====================================================================
    //= ControlReference
    //=====================================================================
    /// a structure containing a property set (the referred control) and a string (the list of referring controls)
    struct ControlReference
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                    xReferredControl;
        ::rtl::OUString
                    sReferringControls;

        ControlReference(
                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxReferredControl,
                const ::rtl::OUString& _rReferringControls)
            :xReferredControl(_rxReferredControl)
            ,sReferringControls(_rReferringControls)
        {
        }
    };

    //=====================================================================
    //= OFormLayerXMLImport_Impl
    //=====================================================================
    class OFormLayerXMLImport_Impl
                :public OAttributeMetaData
                ,public IControlIdMap
                ,public IFormsImportContext
                ,public ODefaultEventAttacherManager
    {
        friend class OFormLayerXMLImport;

    protected:
        SvXMLImport&                        m_rImporter;
        OAttribute2Property                 m_aAttributeMetaData;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >
                                            m_xForms;   // the forms of the currently imported page

    protected:
        // style handling
        ::vos::ORef< XMLPropertyHandlerFactory >    m_xPropertyHandlerFactory;
        ::vos::ORef< SvXMLImportPropertyMapper >    m_xImportMapper;

        DECLARE_STL_USTRINGACCESS_MAP( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >, MapString2PropertySet );
        DECLARE_STL_MAP( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >, MapString2PropertySet, ODrawPageCompare, MapDrawPage2Map);

        MapDrawPage2Map         m_aControlIds;          // ids of the controls on all known page
        MapDrawPage2MapIterator m_aCurrentPageIds;      // ifs of the controls on the current page

        DECLARE_STL_VECTOR( ControlReference, ControlReferenceArray );
        ControlReferenceArray   m_aControlReferences;   // control reference descriptions for current page

    public:
        // IControlIdMap
        virtual void    registerControlId(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControl,
            const ::rtl::OUString& _rId);
        virtual void    registerControlReferences(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControl,
            const ::rtl::OUString& _rReferringControls);

        // IFormsImportContext
        virtual IControlIdMap&              getControlIdMap();
        virtual OAttribute2Property&        getAttributeMap();
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                                            getServiceFactory();
        virtual SvXMLImport&                getGlobalContext();
        virtual void                        enterEventContext();
        virtual void                        leaveEventContext();

    protected:
        OFormLayerXMLImport_Impl(SvXMLImport& _rImporter);
        ~OFormLayerXMLImport_Impl();

        /** retrieves the property mapper form form related auto styles.
        */
        ::vos::ORef< SvXMLImportPropertyMapper > getStylePropertyMapper() const;

        /** start importing the forms of the given page
        */
        void startPage(
            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& _rxDrawPage);

        /** end importing the forms of the current page
        */
        void endPage();

        /** creates an import context for the office:forms element
        */
        SvXMLImportContext* createOfficeFormsContext(
            SvXMLImport& _rImport,
            sal_uInt16 _nPrefix,
            const rtl::OUString& _rLocalName);

        /** create an <type>SvXMLImportContext</type> instance which is able to import the &lt;form:form&gt;
            element.
        */
        SvXMLImportContext* createContext(
            const sal_uInt16 _nPrefix,
            const rtl::OUString& _rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttribs);

        /** get the control with the given id
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                lookupControlId(const ::rtl::OUString& _rControlId);

        /** seek to the given page
        */
        void seekPage(
            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& _rxDrawPage);
    };

//.........................................................................
}   // namespace xmloff
//.........................................................................

#endif // _XMLOFF_FORMS_LAYERIMPORT_HXX_

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.7  2001/02/01 09:46:47  fs
 *  no own style handling anymore - the shape exporter is responsible for our styles now
 *
 *  Revision 1.6  2001/01/24 09:34:40  fs
 *  +enter-/leaveEventContext
 *
 *  Revision 1.5  2001/01/02 15:58:22  fs
 *  event ex- & import
 *
 *  Revision 1.4  2000/12/18 15:14:35  fs
 *  some changes ... now exporting/importing styles
 *
 *  Revision 1.3  2000/12/13 10:40:15  fs
 *  new import related implementations - at this version, we should be able to import everything we export (which is all except events and styles)
 *
 *  Revision 1.2  2000/12/12 12:01:05  fs
 *  new implementations for the import - still under construction
 *
 *  Revision 1.1  2000/12/06 17:31:42  fs
 *  initial checkin - implementations for formlayer import/export - still under construction
 *
 *
 *  Revision 1.0 04.12.00 15:48:40  fs
 ************************************************************************/

