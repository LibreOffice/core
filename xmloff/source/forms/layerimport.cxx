/*************************************************************************
 *
 *  $RCSfile: layerimport.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-20 08:05:15 $
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
#include "layerimport.hxx"
#endif
#ifndef _XMLOFF_FORMENUMS_HXX_
#include "formenums.hxx"
#endif
#ifndef _XMLOFF_FORMS_ELEMENTIMPORT_HXX_
#include "elementimport.hxx"
#endif
#ifndef _XMLOFF_FORMS_STRINGS_HXX_
#include "strings.hxx"
#endif
#ifndef _XMLOFF_XMLICTXT_HXX
#include "xmlictxt.hxx"
#endif
#ifndef _XMLOFF_XMLSTYLE_HXX
#include "xmlstyle.hxx"
#endif
#ifndef _XMLOFF_FAMILIES_HXX_
#include "families.hxx"
#endif
#ifndef _XMLOFF_PROPERTYSETMAPPER_HXX
#include "xmlprmap.hxx"
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif
#ifndef _XMLOFF_EVENTIMPORTHELPER_HXX
#include "XMLEventImportHelper.hxx"
#endif
#ifndef _XMLOFF_XMLIMPPR_HXX
#include "xmlimppr.hxx"
#endif
#ifndef _COM_SUN_STAR_FORM_FORMSUBMITENCODING_HPP_
#include <com/sun/star/form/FormSubmitEncoding.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMSUBMITMETHOD_HPP_
#include <com/sun/star/form/FormSubmitMethod.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_NAVIGATIONBARMODE_HPP_
#include <com/sun/star/form/NavigationBarMode.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_TABULATORCYCLE_HPP_
#include <com/sun/star/form/TabulatorCycle.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMBUTTONTYPE_HPP_
#include <com/sun/star/form/FormButtonType.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_LISTSOURCETYPE_HPP_
#include <com/sun/star/form/ListSourceType.hpp>
#endif
#ifndef _SV_WINTYPES_HXX
#include <vcl/wintypes.hxx>     // for check states
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMSSUPPLIER_HPP_
#include <com/sun/star/form/XFormsSupplier.hpp>
#endif
#ifndef _XMLOFF_FORMS_CONTROLPROPERTYHDL_HXX_
#include "controlpropertyhdl.hxx"
#endif
#ifndef _XMLOFF_FORMS_CONTROLPROPERTYMAP_HXX_
#include "controlpropertymap.hxx"
#endif
#ifndef _XMLOFF_FORMS_FORMEVENTS_HXX_
#include "formevents.hxx"
#endif

SV_IMPL_REF( SvXMLStylesContext );

//.........................................................................
namespace xmloff
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::drawing;
    using namespace ::com::sun::star::xml;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::sdb;

    //=====================================================================
    //= OFormLayerXMLImport_Impl
    //=====================================================================
    //---------------------------------------------------------------------
    OFormLayerXMLImport_Impl::OFormLayerXMLImport_Impl(SvXMLImport& _rImporter)
        :m_rImporter(_rImporter)
    {
        // build the attribute2property map
        // string properties which are exported as attributes
        m_aAttributeMetaData.addStringProperty(
            OAttributeMetaData::getCommonControlAttributeName(CCA_NAME), PROPERTY_NAME);
        m_aAttributeMetaData.addStringProperty(
            OAttributeMetaData::getCommonControlAttributeName(CCA_IMAGE_DATA), PROPERTY_IMAGEURL);
        m_aAttributeMetaData.addStringProperty(
            OAttributeMetaData::getCommonControlAttributeName(CCA_LABEL), PROPERTY_LABEL);
        m_aAttributeMetaData.addStringProperty(
            OAttributeMetaData::getCommonControlAttributeName(CCA_TARGET_LOCATION), PROPERTY_TARGETURL);
        m_aAttributeMetaData.addStringProperty(
            OAttributeMetaData::getCommonControlAttributeName(CCA_TITLE), PROPERTY_TITLE);
        m_aAttributeMetaData.addStringProperty(
            OAttributeMetaData::getCommonControlAttributeName(CCA_TARGET_FRAME), PROPERTY_TARGETFRAME, "_blank");
        m_aAttributeMetaData.addStringProperty(
            OAttributeMetaData::getDatabaseAttributeName(DA_DATA_FIELD), PROPERTY_DATAFIELD);
        m_aAttributeMetaData.addStringProperty(
            OAttributeMetaData::getFormAttributeName(faCommand), PROPERTY_COMMAND);
        m_aAttributeMetaData.addStringProperty(
            OAttributeMetaData::getFormAttributeName(faDatasource), PROPERTY_DATASOURCENAME);
        m_aAttributeMetaData.addStringProperty(
            OAttributeMetaData::getFormAttributeName(faFilter), PROPERTY_FILTER);
        m_aAttributeMetaData.addStringProperty(
            OAttributeMetaData::getFormAttributeName(faOrder), PROPERTY_ORDER);

        // properties not added because they're already present in another form
        OSL_ENSURE(
            0 == ::rtl::OUString::createFromAscii(OAttributeMetaData::getCommonControlAttributeName(CCA_TARGET_LOCATION)).compareToAscii(
                OAttributeMetaData::getFormAttributeName(faAction)),
            "OFormLayerXMLImport_Impl::OFormLayerXMLImport_Impl: invalid attribute names (1)!");
            // if this fails, we would have to add a translation from faAction->PROPERTY_TARGETURL
            // We did not because we already have one CCA_TARGET_LOCATION->PROPERTY_TARGETURL,
            // and CCA_TARGET_LOCATION and faAction should be represented by the same attribute

        OSL_ENSURE(
            0 == ::rtl::OUString::createFromAscii(OAttributeMetaData::getCommonControlAttributeName(CCA_NAME)).compareToAscii(
                OAttributeMetaData::getFormAttributeName(faName)),
            "OFormLayerXMLImport_Impl::OFormLayerXMLImport_Impl: invalid attribute names (2)!");
            // the same for faName, CCA_NAME and PROPERTY_NAME

        // boolean properties which are exported as attributes
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getCommonControlAttributeName(CCA_CURRENT_SELECTED), PROPERTY_STATE, sal_False);
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getCommonControlAttributeName(CCA_DISABLED), PROPERTY_ENABLED, sal_False, sal_True);
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getCommonControlAttributeName(CCA_DROPDOWN), PROPERTY_DROPDOWN, sal_False);
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getCommonControlAttributeName(CCA_PRINTABLE), PROPERTY_PRINTABLE, sal_True);
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getCommonControlAttributeName(CCA_READONLY), PROPERTY_READONLY, sal_False);
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getCommonControlAttributeName(CCA_SELECTED), PROPERTY_DEFAULT_STATE, sal_False);
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getCommonControlAttributeName(CCA_TAB_STOP), PROPERTY_TABSTOP, sal_True);
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getDatabaseAttributeName(DA_CONVERT_EMPTY), PROPERTY_EMPTY_IS_NULL, sal_False);
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getSpecialAttributeName(SCA_VALIDATION), PROPERTY_STRICTFORMAT, sal_False);
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getSpecialAttributeName(SCA_MULTI_LINE), PROPERTY_MULTILINE, sal_False);
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getSpecialAttributeName(SCA_AUTOMATIC_COMPLETION), PROPERTY_AUTOCOMPLETE, sal_False);
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getSpecialAttributeName(SCA_MULTIPLE), PROPERTY_MULTISELECTION, sal_False);
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getSpecialAttributeName(SCA_DEFAULT_BUTTON), PROPERTY_DEFAULTBUTTON, sal_False);
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getSpecialAttributeName(SCA_IS_TRISTATE), PROPERTY_TRISTATE, sal_False);
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getFormAttributeName(faAllowDeletes), PROPERTY_ALLOWDELETES, sal_True);
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getFormAttributeName(faAllowInserts), PROPERTY_ALLOWINSERTS, sal_True);
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getFormAttributeName(faAllowUpdates), PROPERTY_ALLOWUPDATES, sal_True);
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getFormAttributeName(faApplyFilter), PROPERTY_APPLYFILTER, sal_False);
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getFormAttributeName(faEscapeProcessing), PROPERTY_ESCAPEPROCESSING, sal_True);
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getFormAttributeName(faIgnoreResult), PROPERTY_IGNORERESULT, sal_False);

        // the int16 attributes
        m_aAttributeMetaData.addInt16Property(
            OAttributeMetaData::getCommonControlAttributeName(CCA_MAX_LENGTH), PROPERTY_MAXTEXTLENGTH, 0);
        m_aAttributeMetaData.addInt16Property(
            OAttributeMetaData::getCommonControlAttributeName(CCA_SIZE), PROPERTY_LINECOUNT, 5);
        m_aAttributeMetaData.addInt16Property(
            OAttributeMetaData::getCommonControlAttributeName(CCA_TAB_INDEX), PROPERTY_TABINDEX, 0);
        m_aAttributeMetaData.addInt16Property(
            OAttributeMetaData::getDatabaseAttributeName(DA_BOUND_COLUMN), PROPERTY_BOUNDCOLUMN, 0);

        // the enum attributes
        m_aAttributeMetaData.addEnumProperty(
            OAttributeMetaData::getCommonControlAttributeName(CCA_BUTTON_TYPE), PROPERTY_BUTTONTYPE,
            FormButtonType_PUSH, OEnumMapper::getEnumMap(OEnumMapper::epButtonType),
            &::getCppuType( static_cast<FormButtonType*>(NULL) ));
        m_aAttributeMetaData.addEnumProperty(
            OAttributeMetaData::getDatabaseAttributeName(DA_LIST_SOURCE_TYPE), PROPERTY_LISTSOURCETYPE,
            ListSourceType_VALUELIST, OEnumMapper::getEnumMap(OEnumMapper::epListSourceType),
            &::getCppuType( static_cast<ListSourceType*>(NULL) ));
        m_aAttributeMetaData.addEnumProperty(
            OAttributeMetaData::getSpecialAttributeName(SCA_STATE), PROPERTY_DEFAULT_STATE, STATE_NOCHECK,
            OEnumMapper::getEnumMap(OEnumMapper::epCheckState),
            &::getCppuType( static_cast< sal_Int16* >(NULL)));
        m_aAttributeMetaData.addEnumProperty(
            OAttributeMetaData::getSpecialAttributeName(SCA_CURRENT_STATE), PROPERTY_STATE, STATE_NOCHECK,
            OEnumMapper::getEnumMap(OEnumMapper::epCheckState),
            &::getCppuType( static_cast< sal_Int16* >(NULL)));
        m_aAttributeMetaData.addEnumProperty(
            OAttributeMetaData::getFormAttributeName(faEnctype), PROPERTY_SUBMIT_ENCODING,
            FormSubmitEncoding_URL, OEnumMapper::getEnumMap(OEnumMapper::epSubmitEncoding),
            &::getCppuType( static_cast<FormSubmitEncoding*>(NULL) ));
#if SUPD<628
        // for compatibility:
        m_aAttributeMetaData.addEnumProperty(
            "enc-type", PROPERTY_SUBMIT_ENCODING,
            FormSubmitEncoding_URL, OEnumMapper::getEnumMap(OEnumMapper::epSubmitEncoding),
            &::getCppuType( static_cast<FormSubmitEncoding*>(NULL) ));
#endif
        m_aAttributeMetaData.addEnumProperty(
            OAttributeMetaData::getFormAttributeName(faMethod), PROPERTY_SUBMIT_METHOD,
            FormSubmitMethod_GET, OEnumMapper::getEnumMap(OEnumMapper::epSubmitMethod),
            &::getCppuType( static_cast<FormSubmitMethod*>(NULL) ));
        m_aAttributeMetaData.addEnumProperty(
            OAttributeMetaData::getFormAttributeName(faCommandType), PROPERTY_COMMAND_TYPE,
            CommandType::COMMAND, OEnumMapper::getEnumMap(OEnumMapper::epCommandType));
        m_aAttributeMetaData.addEnumProperty(
            OAttributeMetaData::getFormAttributeName(faNavigationMode), PROPERTY_NAVIGATION,
            NavigationBarMode_NONE, OEnumMapper::getEnumMap(OEnumMapper::epNavigationType),
            &::getCppuType( static_cast<NavigationBarMode*>(NULL) ));
        m_aAttributeMetaData.addEnumProperty(
            OAttributeMetaData::getFormAttributeName(faTabbingCycle), PROPERTY_CYCLE,
            TabulatorCycle_RECORDS, OEnumMapper::getEnumMap(OEnumMapper::epTabCyle),
            &::getCppuType( static_cast<TabulatorCycle*>(NULL) ));

        // initialize our style map
        m_xPropertyHandlerFactory = new OControlPropertyHandlerFactory;
        ::vos::ORef< XMLPropertySetMapper > xStylePropertiesMapper = new XMLPropertySetMapper(aControlStyleProperties, m_xPropertyHandlerFactory.getBodyPtr());
        m_xImportMapper = new SvXMLImportPropertyMapper(xStylePropertiesMapper.getBodyPtr());

        // 'initialize'
        m_aCurrentPageIds = m_aControlIds.end();
    }

    //---------------------------------------------------------------------
    OFormLayerXMLImport_Impl::~OFormLayerXMLImport_Impl()
    {
        // outlined to allow forward declaration of OAttribute2Property in the header
    }

    //---------------------------------------------------------------------
    IControlIdMap& OFormLayerXMLImport_Impl::getControlIdMap()
    {
        return *this;
    }

    //---------------------------------------------------------------------
    OAttribute2Property& OFormLayerXMLImport_Impl::getAttributeMap()
    {
        return m_aAttributeMetaData;
    }

    //---------------------------------------------------------------------
    Reference< XMultiServiceFactory > OFormLayerXMLImport_Impl::getServiceFactory()
    {
        return ::comphelper::getProcessServiceFactory();
    }

    //---------------------------------------------------------------------
    SvXMLImport& OFormLayerXMLImport_Impl::getGlobalContext()
    {
        return m_rImporter;
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLImport_Impl::enterEventContext()
    {
        // install our own translation table. We need to disable the other tables because of name conflicts.
        m_rImporter.GetEventImport().PushTranslationTable();
        m_rImporter.GetEventImport().AddTranslationTable(g_pFormsEventTranslation);
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLImport_Impl::leaveEventContext()
    {
        // install the original event tables.
        m_rImporter.GetEventImport().PopTranslationTable();
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLImport_Impl::registerControlId(const Reference< XPropertySet >& _rxControl, const ::rtl::OUString& _rId)
    {
        OSL_ENSURE(m_aCurrentPageIds != m_aControlIds.end(), "OFormLayerXMLImport_Impl::registerControlId: no current page!");
        OSL_ENSURE(_rId.getLength(), "OFormLayerXMLImport_Impl::registerControlId: invalid (empty) control id!");

        OSL_ENSURE(m_aCurrentPageIds->second.end() == m_aCurrentPageIds->second.find(_rId), "OFormLayerXMLImport_Impl::registerControlId: control id already used!");
        m_aCurrentPageIds->second[_rId] = _rxControl;
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLImport_Impl::registerControlReferences(const Reference< XPropertySet >& _rxControl, const ::rtl::OUString& _rReferringControls)
    {
        OSL_ENSURE(_rReferringControls.getLength(), "OFormLayerXMLImport_Impl::registerControlReferences: invalid (empty) control id list!");
        OSL_ENSURE(_rxControl.is(), "OFormLayerXMLImport_Impl::registerControlReferences: invalid (NULL) control!");
        m_aControlReferences.push_back(ControlReference(_rxControl, _rReferringControls));
    }

    //---------------------------------------------------------------------
    ::vos::ORef< SvXMLImportPropertyMapper > OFormLayerXMLImport_Impl::getStylePropertyMapper() const
    {
        return m_xImportMapper;
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLImport_Impl::startPage(const Reference< XDrawPage >& _rxDrawPage)
    {
        m_xForms.clear();

        OSL_ENSURE(_rxDrawPage.is(), "OFormLayerXMLImport_Impl::startPage: NULL page!");
        Reference< XFormsSupplier > xFormsSupp(_rxDrawPage, UNO_QUERY);
        OSL_ENSURE(xFormsSupp.is(), "OFormLayerXMLImport_Impl::startPage: invalid draw page (no XFormsSupplier)!");
        if (!xFormsSupp.is())
            return;

        m_xForms = Reference< XNameContainer >(xFormsSupp->getForms(), UNO_QUERY);
        OSL_ENSURE(m_xForms.is(), "OFormLayerXMLImport_Impl::startPage: invalid forms collection!");

        // add a new entry to our page map
        ::std::pair< MapDrawPage2MapIterator, bool > aPagePosition =
            m_aControlIds.insert(MapDrawPage2Map::value_type(_rxDrawPage, MapString2PropertySet()));
        OSL_ENSURE(aPagePosition.second, "OFormLayerXMLImport_Impl::startPage: already imported this page!");
        m_aCurrentPageIds = aPagePosition.first;
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLImport_Impl::endPage()
    {
        OSL_ENSURE(m_xForms.is(), "OFormLayerXMLImport_Impl::endPage: sure you called startPage before?");

        // do some knittings for the controls which are referring to each other
        try
        {
            static const sal_Unicode s_nSeparator = ',';
            ::rtl::OUString sReferring;
            ::rtl::OUString sCurrentReferring;
            ::rtl::OUString sSeparator(&s_nSeparator, 1);
            Reference< XPropertySet > xCurrentReferring;
            sal_Int32 nSeparator, nPrevSep;
            for (   ConstControlReferenceArrayIterator aReferences = m_aControlReferences.begin();
                    aReferences != m_aControlReferences.end();
                    ++aReferences
                )
            {
                // the list of control ids is comma separated

                // in a list of n ids there are only n-1 separators ... have to catch this last id
                // -> normalize the list
                sReferring = aReferences->sReferringControls;
                sReferring += sSeparator;

                nPrevSep = -1;
                while (-1 != (nSeparator = sReferring.indexOf(s_nSeparator, nPrevSep + 1)))
                {
                    sCurrentReferring = sReferring.copy(nPrevSep + 1, nSeparator - nPrevSep - 1);
                    xCurrentReferring = lookupControlId(sCurrentReferring);
                    if (xCurrentReferring.is())
                        // if this condition fails, this is an error, but lookupControlId should have asserted this ...
                        xCurrentReferring->setPropertyValue(PROPERTY_CONTROLLABEL, makeAny(aReferences->xReferredControl));

                    nPrevSep = nSeparator;
                }
            }
        }
        catch(Exception&)
        {
            OSL_ENSURE(sal_False, "OFormLayerXMLImport_Impl::endPage: unable to knit the control references (caught an exception)!");
        }

        // now that we have all children of the forms collection, attach the events
        Reference< XIndexAccess > xIndexContainer(m_xForms, UNO_QUERY);
        if (xIndexContainer.is())
            ODefaultEventAttacherManager::setEvents(xIndexContainer);

        // clear the structures for the control references.
        m_aControlReferences.clear();

        // and no we have no current page anymore
        m_aCurrentPageIds = m_aControlIds.end();
    }

    //---------------------------------------------------------------------
    Reference< XPropertySet > OFormLayerXMLImport_Impl::lookupControlId(const ::rtl::OUString& _rControlId)
    {
        OSL_ENSURE(m_aCurrentPageIds != m_aControlIds.end(), "OFormLayerXMLImport_Impl::lookupControlId: no current page!");
        Reference< XPropertySet > xReturn;
        if (m_aCurrentPageIds != m_aControlIds.end())
        {
            ConstMapString2PropertySetIterator aPos = m_aCurrentPageIds->second.find(_rControlId);
            if (m_aCurrentPageIds->second.end() != aPos)
                xReturn = aPos->second;
            else
                OSL_ENSURE(sal_False, "OFormLayerXMLImport_Impl::lookupControlId: invalid control id (did not find it)!");
        }
        return xReturn;
    }

    //---------------------------------------------------------------------
    SvXMLImportContext* OFormLayerXMLImport_Impl::createContext(const sal_uInt16 _nPrefix, const rtl::OUString& _rLocalName,
        const Reference< sax::XAttributeList >& _rxAttribs)
    {
        OSL_ENSURE(m_xForms.is(), "OFormLayerXMLImport_Impl::createContext: have no forms collection (did you use startPage?)!");
        OSL_ENSURE(0 == _rLocalName.compareToAscii("form"), "OFormLayerXMLImport_Impl::createContext: don't know the element name (must be \"form\")!");

        if (!m_xForms.is() || (0 != _rLocalName.compareToAscii("form")))
        {
            return new SvXMLImportContext(m_rImporter, _nPrefix, _rLocalName);
        }

        return new OFormImport(*this, *this, _nPrefix, _rLocalName, m_xForms );
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLImport_Impl::seekPage(const Reference< XDrawPage >& _rxDrawPage)
    {
        OSL_ENSURE(m_aCurrentPageIds == m_aControlIds.end(), "OFormLayerXMLImport_Impl::seekPage: importing another page currently! This will smash your import!");
        m_aCurrentPageIds = m_aControlIds.find(_rxDrawPage);
        OSL_ENSURE(m_aCurrentPageIds != m_aControlIds.end(), "OFormLayerXMLImport_Impl::seekPage: did not find the given page (perhaps it has not been imported, yet?)!");
    }

//.........................................................................
}   // namespace xmloff
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.9  2001/02/28 16:42:16  fs
 *  State/DefaultState are int16-properties
 *
 *  Revision 1.8  2001/02/01 09:46:47  fs
 *  no own style handling anymore - the shape exporter is responsible for our styles now
 *
 *  Revision 1.7  2001/01/24 09:34:40  fs
 *  +enter-/leaveEventContext
 *
 *  Revision 1.6  2001/01/02 15:58:22  fs
 *  event ex- & import
 *
 *  Revision 1.5  2000/12/19 12:13:57  fs
 *  some changes ... now the exported styles are XSL conform
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
 *  Revision 1.1  2000/12/06 17:31:31  fs
 *  initial checkin - implementations for formlayer import/export - still under construction
 *
 *
 *  Revision 1.0 04.12.00 15:48:46  fs
 ************************************************************************/

