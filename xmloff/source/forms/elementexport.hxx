/*************************************************************************
 *
 *  $RCSfile: elementexport.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2003-10-21 08:38:25 $
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

#ifndef _XMLOFF_ELEMENTEXPORT_HXX_
#define _XMLOFF_ELEMENTEXPORT_HXX_

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_SCRIPTEVENTDESCRIPTOR_HPP_
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#endif
#ifndef _XMLOFF_FORMS_PROPERTYEXPORT_HXX_
#include "propertyexport.hxx"
#endif
#ifndef _XMLOFF_FORMS_CALLBACKS_HXX_
#include "callbacks.hxx"
#endif
#ifndef _XMLOFF_FORMS_CONTROLELEMENT_HXX_
#include "controlelement.hxx"
#endif
#ifndef _XMLOFF_FORMS_VALUEPROPERTIES_HXX_
#include "valueproperties.hxx"
#endif

class SvXMLElementExport;
//.........................................................................
namespace xmloff
{
//.........................................................................

    //=====================================================================
    //= OElementExport
    //=====================================================================
    class OElementExport : public OPropertyExport
    {
    protected:
        ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >
                                m_aEvents;

        SvXMLElementExport*     m_pXMLElement;          // XML element doing the concrete startElement etc.

    public:
        OElementExport(IFormsExportContext& _rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxProps,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >& _rEvents);
        ~OElementExport();

        void doExport();

    protected:
        /// get the name of the XML element
        virtual const sal_Char* getXMLElementName() const = 0;
        /// examine the element we're exporting
        virtual void examine();
        /// export the attributes
        virtual void exportAttributes();
        /// export any sub tags
        virtual void exportSubTags();

        /** exports the events (as script:events tag)
        */
        void exportEvents();

        /** add the service-name attribute to the export context
        */
        virtual void exportServiceNameAttribute();

        /// start the XML element
        virtual void implStartElement(const sal_Char* _pName);

        /// ends the XML element
        virtual void implEndElement();
    };

    //=====================================================================
    //= OControlExport
    //=====================================================================
    /** Helper class for handling xml elements representing a form control
    */
    class OControlExport
                :public OControlElement
                ,public OValuePropertiesMetaData
                ,public OElementExport
    {
    protected:
        DECLARE_STL_STDKEY_SET(sal_Int16, Int16Set);
            // used below

        ::rtl::OUString         m_sControlId;           // the control id to use when exporting
        ::rtl::OUString         m_sReferringControls;   // list of referring controls (i.e. their id's)
        sal_Int16               m_nClassId;             // class id of the control we're representing
        ElementType             m_eType;                // (XML) type of the control we're representing
        sal_Int32               m_nIncludeCommon;       // common control attributes to include
        sal_Int32               m_nIncludeDatabase;     // common database attributes to include
        sal_Int32               m_nIncludeSpecial;      // special attributes to include
        sal_Int32               m_nIncludeEvents;       // events to include
        sal_Int32               m_nIncludeBindings;     // binding attributes to include

        SvXMLElementExport*     m_pOuterElement;        // XML element doing the concrete startElement etc. for the outer element

    public:
        /** constructs an object capable of exporting controls

            <p>You need at least two pre-requisites from outside: The control to be exported needs to have a class id
            assigned, and you need the list control-ids of all the controls referring to this one as LabelControl.<br/>
            This information can't be collected when known only the control itself and not it's complete context.</p>

            @param _rControlId
                the control id to use when exporting the control
            @param _rReferringControls
                the comma-separated list of control-ids of all the controls referring to this one as LabelControl
        */
        OControlExport(IFormsExportContext& _rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControl,
            const ::rtl::OUString& _rControlId,
            const ::rtl::OUString& _rReferringControls,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >& _rxEvents);
        ~OControlExport();

    protected:
        /// start the XML element
        virtual void implStartElement(const sal_Char* _pName);

        /// ends the XML element
        virtual void implEndElement();

        /// get the name of the outer XML element
        virtual const sal_Char* getOuterXMLElementName() const;

        // get the name of the XML element
        virtual const sal_Char* getXMLElementName() const;

        /** examine the control. Some kind of CtorImpl.
        */
        virtual void examine();

        /// exports the attributes for the outer element
        void exportOuterAttributes();

        /// exports the attributes for the inner element
        void exportInnerAttributes();

        /// export the attributes
        virtual void exportAttributes();

        /** writes everything which needs to be represented as sub tag
        */
        void exportSubTags() throw (::com::sun::star::uno::Exception);

        /** adds common control attributes to the XMLExport context given

            <p>The attribute list of the context is not cleared initially, this is the responsibility of the caller.</p>
        */
        void exportCommonControlAttributes();

        /** adds database attributes to the XMLExport context given

            <p>The attribute list of the context is not cleared initially, this is the responsibility of the caller.</p>
        */
        void exportDatabaseAttributes();

        /** adds the XML attributes which are related to binding controls to
            external values and/or list sources
        */
        void exportBindingAtributes();

        /** adds attributes which are special to a control type to the export context's attribute list
        */
        void exportSpecialAttributes();

        /** exports the ListSource property of a control as attribute

            <p>The ListSource property may be exported in different ways: For a ComboBox, it is an attribute
            of the form:combobox element.</p>

            <p>For a ListBox, it's an attribute if the ListSourceType states that the ListBox does <em>not</em>
            display a value list. In case of a value list, the ListSource is not exported, and the pairs of
            StringItem/ValueItem are exported as sub-elements.<br/>
            (For a value list ListBox, As every setting of the ListSource immediately sets the ValueList to the
            same value, so nothing is lost when exporting this).</p>

            <p>It's really strange, isn't it? Don't know why we have this behaviour in our controls ...</p>

            <p>This method does the attribute part: It exports the ListSource property as attribute, not caring
            about whether the object is a ComboBox or a ListBox.</p>
        */
        void exportListSourceAsAttribute();

        /** exports the ListSource property of a control as XML elements

            @see exportListSourceAsAttribute
        */
        void exportListSourceAsElements();

        /** get's a Sequence&lt; sal_Int16 &gt; property value as set of sal_Int16's
            @param _rPropertyName
                the property name to use
            @param _rOut
                out parameter. The set of integers.
        */
        void getSequenceInt16PropertyAsSet(const ::rtl::OUString& _rPropertyName, Int16Set& _rOut);

        /** exports the attribute which descrives a cell value binding of a control
            in a spreadsheet document
        */
        void exportCellBindingAttributes( bool _bIncludeListLinkageType );

        /** exports the attribute which descrives a cell range which acts as list source for
            a list-like control
        */
        void exportCellListSourceRange( );
    };

    //=====================================================================
    //= OColumnExport
    //=====================================================================
    /** Helper class for exporting a grid column
    */
    class OColumnExport : public OControlExport
    {
    public:
        /** ctor
            @see OColumnExport::OColumnExport
        */
        OColumnExport(IFormsExportContext& _rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControl,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >& _rxEvents);

        ~OColumnExport();

    protected:
        // OControlExport overridables
        virtual const sal_Char* getOuterXMLElementName() const;
        virtual void exportServiceNameAttribute();
        virtual void exportAttributes();

        // OElementExport overridables
        virtual void examine();
    };

    //=====================================================================
    //= OFormExport
    //=====================================================================
    /** Helper class for handling xml elements representing a form

        <p>In opposite to the class <type>OControlExport</type>, OFormExport is unable to export a <em>complete</em>
        form. Instead the client has to care for sub elements of the form itself.</p>
    */
    class OFormExport
                :public OControlElement
                ,public OElementExport
    {
    public:
        /** constructs an object capable of exporting controls
        */
        OFormExport(IFormsExportContext& _rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxForm,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >& _rxEvents
            );

    protected:
        virtual const sal_Char* getXMLElementName() const;
        virtual void exportSubTags();
        virtual void exportAttributes();
    };

//.........................................................................
}   // namespace xmloff
//.........................................................................

#endif // _XMLOFF_ELEMENTEXPORT_HXX_

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.7.198.1  2003/10/01 09:55:18  fs
 *  #i18994# merging the changes from the CWS fs002
 *
 *  Revision 1.7.194.1  2003/09/25 14:28:36  fs
 *  #18994# merging the changes from cws_srx645_fs002 branch
 *
 *  Revision 1.7.190.2  2003/09/18 14:00:36  fs
 *  #18995# changes for binding list boxes to cells, while exchanging selection indexes instead of strings
 *
 *  Revision 1.7.190.1  2003/09/17 12:26:46  fs
 *  #18999# #19367# persistence for cell value and cell range bindings
 *
 *  Revision 1.7  2001/01/03 16:25:34  fs
 *  file format change (extra wrapper element for controls, similar to columns)
 *
 *  Revision 1.6  2001/01/02 15:58:21  fs
 *  event ex- & import
 *
 *  Revision 1.5  2000/12/18 15:14:35  fs
 *  some changes ... now exporting/importing styles
 *
 *  Revision 1.4  2000/12/13 10:38:10  fs
 *  moved some code to a more central place to reuse it
 *
 *  Revision 1.3  2000/12/06 17:28:05  fs
 *  changes for the formlayer import - still under construction
 *
 *  Revision 1.2  2000/11/19 15:41:32  fs
 *  extended the export capabilities - generic controls / grid columns / generic properties / some missing form properties
 *
 *  Revision 1.1  2000/11/17 19:01:28  fs
 *  initial checkin - export and/or import the applications form layer
 *
 *
 *  Revision 1.0 13.11.00 18:41:40  fs
 ************************************************************************/

