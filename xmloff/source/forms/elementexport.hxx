/*************************************************************************
 *
 *  $RCSfile: elementexport.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-11-19 15:41:32 $
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
#ifndef _XMLOFF_FORMS_PROPERTYEXPORT_HXX_
#include "propertyexport.hxx"
#endif
#ifndef _XMLOFF_FORMS_CALLBACKS_HXX_
#include "callbacks.hxx"
#endif

class SvXMLExport;
class SvXMLElementExport;
//.........................................................................
namespace xmloff
{
//.........................................................................

    //=====================================================================
    //= OControlElement
    //=====================================================================
    /** helper for translating between control types and XML tags
    */
    class OControlElement
    {
    public:
        enum ElementType
        {
            TEXT,
            TEXT_AREA,
            PASSWORD,
            FILE,
            FORMATTED_TEXT,
            FIXED_TEXT,
            COMBOBOX,
            LISTBOX,
            BUTTON,
            IMAGE,
            CHECKBOX,
            RADIO,
            FRAME,
            IMAGE_FRAME,
            HIDDEN,
            GRID,
            COLUMN,
            CONTROL
        };

    protected:
        /** ctor.
            <p>This default constructor is protected, 'cause this class is not intended to be instantiated
            directly. Instead, the derived classes should be used.</p>
        */
        OControlElement() { }

    public:
        /** retrieves the tag name to be used to describe a control of the given type

            <p>The retuned string is the pure element name, without any namespace.</p>

            @param  _eType
                the element type
        */
        const sal_Char* getElementName(ElementType _eType);
    };

    //=====================================================================
    //= OControlExport
    //=====================================================================
    /** Helper class for handling xml elements representing a form control
    */
    class OControlExport
                :public OControlElement
                ,public OPropertyExport
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
        IExportImplementation*  m_pCallback;            // the callback to export collections - (GridControls are collections)

        SvXMLElementExport*     m_pXMLElement;          // XML element doing the concrete startElement etc.

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
        OControlExport(SvXMLExport& _rContext, IExportImplementation* _pCallback,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControl,
            const ::rtl::OUString& _rControlId,
            const ::rtl::OUString& _rReferringControls);

        /** starts the export.

            <p>This is an extra method because we need to call virtual methods here, else we would have placed
            it in the ctor ...<p>
        */
        void doExport();

        /** dtor.

            <p>Automatically closes the element tag which was opened for the control in the ctor</p>
        */
        ~OControlExport();

    protected:
        /** examine the control. Some kind of CtorImpl.
        */
        virtual void examine();

        /** starts the XML element which represents the control.

            <p>The default implementation only creates <member>m_pXMLElement</member> with the parameters
            derived from the results of <method>examine</method>.
        */
        virtual void startExportElement();

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

        /** calculate the property names for the <em>current-value</em> and the <em>value</em> attribute.

            <p>If the control does not have any of the properties requested, the respective out parameter will be set
            to NULL.</p>
        */
        void getPropertyNames_ca(sal_Char const * & _rpCurrentValue, sal_Char const * & _rpValue);
    };

    //=====================================================================
    //= OColumnExport
    //=====================================================================
    /** Helper class for exporting a grid column
    */
    class OColumnExport : public OControlExport
    {
    protected:
        SvXMLElementExport*     m_pColumnXMLElement;
            // in addition to the element written by the base class, we need another one indicating that we're a
            // column

    public:
        /** ctor
            @see OColumnExport::OColumnExport
        */
        OColumnExport(SvXMLExport& _rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControl);

        ~OColumnExport();

    protected:
        virtual void examine();
        virtual void startExportElement();
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
                ,public OPropertyExport
    {
    protected:
        IExportImplementation*      m_pCallback;
        SvXMLElementExport*         m_pXMLElement;          // XML element doing the concrete startElement etc.
    public:
        /** constructs an object capable of exporting controls
        */
        OFormExport(SvXMLExport& _rContext,
            IExportImplementation* _pCallback,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxForm);

        /** dtor.
        */
        ~OFormExport();

        /** adds form attributes of the form the object represents to the XMLExport
        */
        void exportAttributes();
    };

//.........................................................................
}   // namespace xmloff
//.........................................................................

#endif // _XMLOFF_ELEMENTEXPORT_HXX_

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.1  2000/11/17 19:01:28  fs
 *  initial checkin - export and/or import the applications form layer
 *
 *
 *  Revision 1.0 13.11.00 18:41:40  fs
 ************************************************************************/

