/*************************************************************************
 *
 *  $RCSfile: formcontroller.cxx,v $
 *
 *  $Revision: 1.55 $
 *
 *  last change: $Author: obo $ $Date: 2003-10-21 09:04:59 $
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
#define ITEMID_MACRO SID_ATTR_MACROITEM

#ifndef _EXTENSIONS_PROPCTRLR_PROPCONTROLLER_HXX_
#include "propcontroller.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_USERCONTROL_HXX_
#include "usercontrol.hxx"
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _EXTENSIONS_FORMSCTRLR_FORMBROWSERTOOLS_HXX_
#include "formbrowsertools.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_MODULEPRC_HXX_
#include "modulepcr.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_LINEDESCRIPTOR_HXX_
#include "linedescriptor.hxx"
#endif
#ifndef _EXTENSIONS_FORMSCTRLR_FORMSTRINGS_HXX_
#include "formstrings.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_PROPRESID_HRC_
#include "propresid.hrc"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_FORMMETADATA_HXX_
#include "formmetadata.hxx"
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_EXTENDEDFILEPICKERELEMENTIDS_HPP_
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XFILEPICKERCONTROLACCESS_HPP_
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XFILEPICKER_HPP_
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTDESCRIPTOR_HPP_
#include <com/sun/star/awt/FontDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTYPES_HPP_
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XPREPAREDSTATEMENT_HPP_
#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_LISTSOURCETYPE_HPP_
#include <com/sun/star/form/ListSourceType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERIESSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMCOMPONENTTYPE_HPP_
#include <com/sun/star/form/FormComponentType.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XNAMINGSERVICE_HPP_
#include <com/sun/star/uno/XNamingService.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XGRIDCOLUMNFACTORY_HPP_
#include <com/sun/star/form/XGridColumnFactory.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_FORM_XBINDABLEVALUE_HPP_
#include <drafts/com/sun/star/form/XBindableValue.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLADDRESS_HPP_
#include <com/sun/star/table/CellAddress.hpp>
#endif

#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif
#ifndef _NUMUNO_HXX
#include <svtools/numuno.hxx>
#endif
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif
#define ITEMID_NUMBERINFO   SID_ATTR_NUMBERFORMAT_INFO
#ifndef _SVX_NUMINF_HXX
#include <svx/numinf.hxx>
#endif
#ifndef _BASEDLGS_HXX
#include <sfx2/basedlgs.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SVX_NUMFMT_HXX
#include <svx/numfmt.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _EXTENSIONS_PROPCTRLR_PROPERTYEDITOR_HXX_
#include "propertyeditor.hxx"
#endif
#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif
#ifndef _EXTENSIONS_PROPCTRLR_FONTDIALOG_HXX_
#include "fontdialog.hxx"
#endif
#ifndef _FILEDLGHELPER_HXX
#include <sfx2/filedlghelper.hxx>
#endif

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSERFACTORY_HPP_
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif
#ifndef _CTRLTOOL_HXX
#include <svtools/ctrltool.hxx>
#endif
#ifndef _SVX_CHARDLG_HXX
#include <svx/chardlg.hxx>
#endif
#ifndef _EXTENSIONS_FORMCTRLR_PROPRESID_HRC_
#include "formresid.hrc"
#endif

// event handling
#ifndef _COM_SUN_STAR_SCRIPT_SCRIPTEVENTDESCRIPTOR_HPP_
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XSCRIPTEVENTSSUPPLIER_HPP_
#include <com/sun/star/script/XScriptEventsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

#ifndef _MACROPG_HXX
#include <sfx2/macropg.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _SFXMACITEM_HXX
#include <svtools/macitem.hxx>
#endif
#define LINETYPE_EVENT  reinterpret_cast<void*>(0xFFFFFFFF)

#ifndef _EXTENSIONS_FORMCTRLR_FORMHELPID_HRC_
#include "formhelpid.hrc"
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

#ifndef _SV_COLRDLG_HXX
#include <svtools/colrdlg.hxx>
#endif
#ifndef _EXTENSIONS_PROPCTRLR_SELECTLABELDIALOG_HXX_
#include "selectlabeldialog.hxx"
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif

#ifndef EXTENSIONS_PROPCTRLR_CELLBINDINGHELPER_HXX
#include "cellbindinghelper.hxx"
#endif

//............................................................................
namespace pcr
{
//............................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::script;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::ui::dialogs;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::table;
    using namespace ::drafts::com::sun::star::form;
    using namespace ::dbtools;

    //========================================================================
    //= helper
    //========================================================================
    Sequence< ::rtl::OUString> getEventMethods(const Type& type)
    {
        typelib_InterfaceTypeDescription *pType=0;
        type.getDescription( (typelib_TypeDescription**)&pType);

        if (!pType)
            return Sequence< ::rtl::OUString>();

        Sequence< ::rtl::OUString> aNames(pType->nMembers);
        ::rtl::OUString* pNames = aNames.getArray();
        for (sal_Int32 i=0;i<pType->nMembers;i++,++pNames)
        {
            // the decription reference
            typelib_TypeDescriptionReference* pMemberDescriptionReference = pType->ppMembers[i];
            // the description for the reference
            typelib_TypeDescription* pMemberDescription = NULL;
            typelib_typedescriptionreference_getDescription(&pMemberDescription, pMemberDescriptionReference);
            if (pMemberDescription)
            {
                typelib_InterfaceMemberTypeDescription* pRealMemberDescription =
                    reinterpret_cast<typelib_InterfaceMemberTypeDescription*>(pMemberDescription);
                *pNames = pRealMemberDescription->pMemberName;
            }
        }

        typelib_typedescription_release( (typelib_TypeDescription *)pType );
        return aNames;
    }

    //------------------------------------------------------------------------
    class OLineDescriptorLess
    {
    public:
        bool operator() (const OLineDescriptor& lhs, const OLineDescriptor& rhs) const
        {
            return lhs.nUniqueButtonId < rhs.nUniqueButtonId;
        }
    };

    //========================================================================
    //= OPropertyBrowserController
    //========================================================================
    //------------------------------------------------------------------------
    void OPropertyBrowserController::initFormStuff()
    {
        m_pPropertyInfo = new OFormPropertyInfoService();
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserController::deinitFormStuff()
    {
        delete static_cast<const OFormPropertyInfoService*>(m_pPropertyInfo);
        m_pPropertyInfo = NULL;
    }

    //------------------------------------------------------------------------
    ::rtl::OUString OPropertyBrowserController::getStringRepFromPropertyValue( const Any& rValue, sal_Int32 _nPropId)
    {
        ::rtl::OUString sReturn;
        if (!rValue.hasValue())
            return sReturn;

        try
        {
            sReturn = convertSimpleToString(rValue);

            // translations for some known types
            switch(rValue.getValueTypeClass())
            {
                // booleans
                case TypeClass_BOOLEAN:
                {
                    String aEntries(ModuleRes(RID_STR_BOOL));
                    sReturn = ::comphelper::getBOOL(rValue) ? aEntries.GetToken(1) : aEntries.GetToken(0);
                }
                break;

                // sequences
                case TypeClass_SEQUENCE:
                {
                    // string sequences
                    if (rValue.getValueType() == ::getCppuType((const Sequence< ::rtl::OUString>*)0))
                    {
                        Sequence< ::rtl::OUString> aStringSeq;
                        rValue >>= aStringSeq;

                        String aRet;

                        // loop through the elements and concatenate the elements (separated by a line break)
                        const ::rtl::OUString* pStringArray = aStringSeq.getConstArray();
                        sal_uInt32 nCount = aStringSeq.getLength();
                        for (sal_uInt32 i=0; i<nCount; ++i, ++pStringArray )
                        {
                            aRet += pStringArray->getStr();
                            if (i!=(nCount-1))
                                aRet += '\n';
                        }
                        sReturn = aRet;
                    }
                    // uInt16 sequences
                    else if (rValue.getValueType() == ::getCppuType((Sequence<sal_uInt16>*)0))
                    {
                        String aRet;
                        Sequence<sal_uInt16> aValues;
                        rValue >>= aValues;

                        // loop through the elements and concatenate the string representations of the integers
                        // (separated by a line break)
                        const sal_uInt16* pArray = aValues.getConstArray();
                        sal_uInt32 nCount = aValues.getLength();
                        for (sal_uInt32 i=0; i<nCount; ++i, ++pArray)
                        {
                            aRet += String::CreateFromInt32(*pArray);
                            if (i!=(nCount-1) )
                                aRet += '\n';
                        }
                        sReturn = aRet;
                    }
                    // Int16 sequences
                    else if (rValue.getValueType() == ::getCppuType((const Sequence<sal_Int16>*)0))
                    {
                        String aRet;
                        Sequence<sal_Int16> aValues;
                        rValue >>= aValues;

                        // loop through the elements and concatenate the string representations of the integers
                        // (separated by a line break)
                        const sal_Int16* pArray = aValues.getConstArray();
                        sal_uInt32 nCount = aValues.getLength();
                        for (sal_uInt32 i=0; i<nCount; ++i, ++pArray)
                        {
                            aRet += String::CreateFromInt32(*pArray);
                            if (i!=(nCount-1) )
                                aRet += '\n';
                        }
                        sReturn = aRet;
                    }
                    // uInt32 sequences
                    else if (rValue.getValueType() == ::getCppuType((const Sequence<sal_uInt32>*)0))
                    {
                        String aRet;
                        Sequence<sal_uInt32> aValues;
                        rValue >>= aValues;

                        // loop through the elements and concatenate the string representations of the integers
                        // (separated by a line break)
                        const sal_uInt32* pArray = aValues.getConstArray();
                        sal_uInt32 nCount = aValues.getLength();
                        for (sal_uInt32 i=0; i<nCount; ++i, ++pArray )
                        {
                            aRet += String::CreateFromInt32(*pArray);
                            if (i!=(nCount-1) )
                                aRet += '\n';
                        }
                        sReturn = aRet;
                    }
                    // Int32 sequences
                    else if (rValue.getValueType() == ::getCppuType((const Sequence<sal_Int16>*)0))
                    {
                        String aRet;
                        Sequence<sal_Int32> aValues;
                        rValue >>= aValues;

                        // loop through the elements and concatenate the string representations of the integers
                        // (separated by a line break)
                        const sal_Int32* pArray = aValues.getConstArray();
                        sal_uInt32 nCount = aValues.getLength();
                        for (sal_uInt32 i=0; i<nCount; ++i, ++pArray )
                        {
                            aRet += String::CreateFromInt32(*pArray);
                            if (i!=(nCount-1) )
                                aRet += '\n';
                        }
                        sReturn = aRet;
                    }


                }
                break;

            }

            // TODO
            // this is surely heavyly formdependent. Need another mechanism for converting Any->Display-String
            switch (_nPropId)
            {
                // ListTypen
                case PROPERTY_ID_ALIGN:
                case PROPERTY_ID_DATEFORMAT:
                case PROPERTY_ID_TIMEFORMAT:
                case PROPERTY_ID_BORDER:
                case PROPERTY_ID_DEFAULTCHECKED:
                case PROPERTY_ID_STATE:
                case PROPERTY_ID_COMMANDTYPE:
                case PROPERTY_ID_CYCLE:
                case PROPERTY_ID_LISTSOURCETYPE:
                case PROPERTY_ID_NAVIGATION:
                case PROPERTY_ID_BUTTONTYPE:
                case PROPERTY_ID_PUSHBUTTONTYPE:
                case PROPERTY_ID_SUBMIT_METHOD:
                case PROPERTY_ID_SUBMIT_ENCODING:
                case PROPERTY_ID_ORIENTATION:
                case PROPERTY_ID_IMAGEALIGN:
                case PROPERTY_ID_CELL_EXCHANGE_TYPE:
                {
                    if (m_pPropertyInfo)
                    {
                        sal_Int32 nIntValue = -1;
                        if (::cppu::enum2int(nIntValue, rValue) && m_pPropertyInfo)
                        {
                            Sequence< ::rtl::OUString > aEnumStrings = m_pPropertyInfo->getPropertyEnumRepresentations(_nPropId);
                            if ((nIntValue >= 0) && (nIntValue < aEnumStrings.getLength()))
                            {
                                sReturn = aEnumStrings[nIntValue];
                            }
                            else
                                DBG_ERROR("OPropertyBrowserController::getStringRepFromPropertyValue: could not translate an enum value");
                        }
                    }
                }
                break;

                case PROPERTY_ID_CONTROLLABEL:
                {
                    String aReturn;
                    Reference< XPropertySet >  xSet;
                    rValue >>= xSet;
                    if (xSet.is() && ::comphelper::hasProperty(PROPERTY_LABEL, xSet))
                    {
                        aReturn = '<';
                        aReturn += ::comphelper::getString(xSet->getPropertyValue(PROPERTY_LABEL)).getStr();
                        aReturn += '>';
                    }
                    sReturn = aReturn;
                }
                break;

                case PROPERTY_ID_BOUND_CELL:
                {
                    Reference< XValueBinding > xBinding;
#if OSL_DEBUG_LEVEL > 0
                    sal_Bool bSuccess =
#endif
                    rValue >>= xBinding;
                    DBG_ASSERT( bSuccess, "OPropertyBrowserController::getStringRepFromPropertyValue: invalid value (1)!" );

                    // the only value binding we support so far is linking to spreadsheet cells
                    CellBindingHelper aHelper( m_xORB, m_xPropValueAccess );
                    sReturn = aHelper.getStringAddressFromCellBinding( xBinding );
                }
                break;

                case PROPERTY_ID_LIST_CELL_RANGE:
                {
                    Reference< XListEntrySource > xSource;
#if OSL_DEBUG_LEVEL > 0
                    sal_Bool bSuccess =
#endif
                    rValue >>= xSource;
                    DBG_ASSERT( bSuccess, "OPropertyBrowserController::getStringRepFromPropertyValue: invalid value (2)!" );

                    // the only value binding we support so far is linking to spreadsheet cells
                    CellBindingHelper aHelper( m_xORB, m_xPropValueAccess );
                    sReturn = aHelper.getStringAddressFromCellListSource( xSource );
                }
                break;
            }
        }
        catch (Exception&)
        {
            DBG_ERROR("OPropertyBrowserController::getStringRepFromPropertyValue: caught an exception!")
        }

        return sReturn;
    }

    //------------------------------------------------------------------------
    Any OPropertyBrowserController::getPropertyValueFromStringRep( const ::rtl::OUString& _rString, const Property& _rProp, sal_Int32 _nPropId)
    {
        Any aReturn;
        try
        {
            // void values
            if ((_rProp.Attributes & PropertyAttribute::MAYBEVOID) && !_rString.getLength() )
                return aReturn;

            //////////////////////////////////////////////////////////////////////
            // TypeClass
            Type  aPropertyType = _rProp.Type;
            TypeClass ePropertyType = aPropertyType.getTypeClass();

            // (one more) special handling : we have a prop which has a TypeClass "ANY" and needs a double
            // (originally it needed a double _or_ a string, but our UI only supports a double for it)
            if ((TypeClass_ANY == ePropertyType) && ((PROPERTY_ID_EFFECTIVE_DEFAULT == _nPropId) || (PROPERTY_ID_EFFECTIVE_VALUE == _nPropId)))
                ePropertyType = TypeClass_DOUBLE;

            switch (ePropertyType)
            {
                case TypeClass_STRING:
                case TypeClass_FLOAT:
                case TypeClass_DOUBLE:
                case TypeClass_BYTE:
                case TypeClass_SHORT:
                case TypeClass_LONG:
                case TypeClass_HYPER:
                case TypeClass_UNSIGNED_SHORT:
                case TypeClass_UNSIGNED_LONG:
                case TypeClass_UNSIGNED_HYPER:
                    // TODO: same as above ... the type converter is expensive
                    try
                    {
                        aReturn = m_xTypeConverter->convertToSimpleType(makeAny(_rString), ePropertyType);
                    }
                    catch(CannotConvertException&) { }
                    catch(IllegalArgumentException&) { }
                    break;
            }

            switch( ePropertyType )
            {

                case TypeClass_BOOLEAN:
                {
                    String sBooleanValues(ModuleRes(RID_STR_BOOL));
                    if (sBooleanValues.GetToken(0) == String(_rString))
                        aReturn <<= (sal_Bool)sal_False;
                    else
                        aReturn <<= (sal_Bool)sal_True;
                }
                break;

                case TypeClass_SEQUENCE:
                {
                    Type aElementType = ::comphelper::getSequenceElementType(aPropertyType);

                    String aStr(_rString);
                    switch (aElementType.getTypeClass())
                    {
                        case TypeClass_STRING:
                        {
                            sal_uInt32 nEntryCount = aStr.GetTokenCount('\n');
                            Sequence< ::rtl::OUString> aStringSeq( nEntryCount );
                            ::rtl::OUString* pStringArray = aStringSeq.getArray();

                            for (sal_Int32 i=0; i<aStringSeq.getLength(); ++i, ++pStringArray)
                                *pStringArray = aStr.GetToken((sal_uInt16)i, '\n');
                            aReturn <<= aStringSeq;
                        }
                        break;
                        case TypeClass_UNSIGNED_SHORT:
                        {
                            sal_uInt32 nEntryCount = aStr.GetTokenCount('\n');
                            Sequence<sal_uInt16> aSeq( nEntryCount );

                            sal_uInt16* pArray = aSeq.getArray();

                            for (sal_Int32 i=0; i<aSeq.getLength(); ++i, ++pArray)
                                *pArray = (sal_uInt16)aStr.GetToken((sal_uInt16)i, '\n').ToInt32();

                            aReturn <<= aSeq;

                        }
                        break;
                        case TypeClass_SHORT:
                        {
                            sal_uInt32 nEntryCount = aStr.GetTokenCount('\n');
                            Sequence<sal_Int16> aSeq( nEntryCount );

                            sal_Int16* pArray = aSeq.getArray();

                            for (sal_Int32 i=0; i<aSeq.getLength(); ++i, ++pArray)
                                *pArray = (sal_Int16)aStr.GetToken((sal_uInt16)i, '\n').ToInt32();

                            aReturn <<= aSeq;

                        }
                        break;
                        case TypeClass_LONG:
                        {
                            sal_uInt32 nEntryCount = aStr.GetTokenCount('\n');
                            Sequence<sal_Int32> aSeq( nEntryCount );

                            sal_Int32* pArray = aSeq.getArray();

                            for (sal_Int32 i=0; i<aSeq.getLength(); ++i, ++pArray)
                                *pArray = aStr.GetToken((sal_uInt16)i, '\n').ToInt32();

                            aReturn <<= aSeq;

                        }
                        break;
                        case TypeClass_UNSIGNED_LONG:
                        {
                            sal_uInt32 nEntryCount = aStr.GetTokenCount('\n');
                            Sequence<sal_uInt32> aSeq( nEntryCount );

                            sal_uInt32* pArray = aSeq.getArray();

                            for (sal_Int32 i=0; i<aSeq.getLength(); ++i, ++pArray)
                                *pArray = aStr.GetToken((sal_uInt16)i, '\n').ToInt32();

                            aReturn <<= aSeq;

                        }
                    }
                }
                break;
            }

            switch( _nPropId )
            {
                case PROPERTY_ID_LIST_CELL_RANGE:
                {
                    CellBindingHelper aHelper( m_xORB, m_xPropValueAccess );
                    aReturn = makeAny( aHelper.createCellListSourceFromStringAddress( _rString ) );
                }
                break;

                case PROPERTY_ID_BOUND_CELL:
                {
                    CellBindingHelper aHelper( m_xORB, m_xPropValueAccess );

                    // if we have the possibility of an integer binding, then we must preserve
                    // this property's value (e.g. if the current binding is an integer binding, then
                    // the newly created one must be, too)
                    bool bIntegerBinding = false;
                    if ( aHelper.isCellIntegerBindingAllowed() )
                    {
                        sal_Int16 nCurrentBindingType = 0;
                        getVirtualPropertyValue( PROPERTY_ID_CELL_EXCHANGE_TYPE ) >>= nCurrentBindingType;
                        bIntegerBinding = ( nCurrentBindingType != 0 );
                    }
                    aReturn = makeAny( aHelper.createCellBindingFromStringAddress( _rString, bIntegerBinding ) );
                }
                break;

                case PROPERTY_ID_ALIGN:
                case PROPERTY_ID_DATEFORMAT:
                case PROPERTY_ID_TIMEFORMAT:
                case PROPERTY_ID_BORDER:
                case PROPERTY_ID_DEFAULTCHECKED:
                case PROPERTY_ID_STATE:
                case PROPERTY_ID_COMMANDTYPE:
                case PROPERTY_ID_CYCLE:
                case PROPERTY_ID_LISTSOURCETYPE:
                case PROPERTY_ID_NAVIGATION:
                case PROPERTY_ID_BUTTONTYPE:
                case PROPERTY_ID_PUSHBUTTONTYPE:
                case PROPERTY_ID_SUBMIT_METHOD:
                case PROPERTY_ID_SUBMIT_ENCODING:
                case PROPERTY_ID_ORIENTATION:
                case PROPERTY_ID_IMAGEALIGN:
                case PROPERTY_ID_CELL_EXCHANGE_TYPE:
                    if (m_pPropertyInfo)
                    {
                        Sequence< ::rtl::OUString > aEnumStrings = m_pPropertyInfo->getPropertyEnumRepresentations(_nPropId);
                        sal_Int32 nPos = GetStringPos(_rString, aEnumStrings);
                        if (-1 != nPos)
                        {
                            switch (aPropertyType.getTypeClass())
                            {
                                case TypeClass_ENUM:
                                    aReturn = ::cppu::int2enum(nPos, aPropertyType);
                                    break;
                                case TypeClass_SHORT:
                                    aReturn <<= (sal_Int16)nPos;
                                    break;
                                case TypeClass_UNSIGNED_SHORT:
                                    aReturn <<= (sal_uInt16)nPos;
                                    break;
                                case TypeClass_UNSIGNED_LONG:
                                    aReturn <<= (sal_uInt32)nPos;
                                    break;
                                default:
                                    aReturn <<= (sal_Int32)nPos;
                                    break;
                            }
                        }
                        else
                            DBG_ERROR("OPropertyBrowserController::getPropertyValueFromStringRep: could not translate the enum string!");
                    }
                break;
            }
        }
        catch(Exception&)
        {
            DBG_ERROR("OPropertyBrowserController::getPropertyValueFromStringRep: caught an exception !")
        }

        return aReturn;
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserController::ChangeFormatProperty(const ::rtl::OUString& _rName, const ::rtl::OUString& _rCurVal)
    {
        // create the itemset for the dialog
        SfxItemSet aCoreSet(SFX_APP()->GetPool(),
            SID_ATTR_NUMBERFORMAT_VALUE, SID_ATTR_NUMBERFORMAT_VALUE,
            SID_ATTR_NUMBERFORMAT_INFO, SID_ATTR_NUMBERFORMAT_INFO,
            0);     // ripped this somewhere ... don't understand it :(

        // get the number formats supplier
        Reference< XNumberFormatsSupplier >  xSupplier;
        m_xPropValueAccess->getPropertyValue(PROPERTY_FORMATSSUPPLIER) >>= xSupplier;

        DBG_ASSERT(xSupplier.is(), "OPropertyBrowserController::ChangeFormatProperty : invalid call !");
        Reference< XUnoTunnel > xTunnel(xSupplier,UNO_QUERY);
        DBG_ASSERT(xTunnel.is(), "OPropertyBrowserController::ChangeFormatProperty : xTunnel is invalid!");
        SvNumberFormatsSupplierObj* pSupplier = (SvNumberFormatsSupplierObj*)xTunnel->getSomething(SvNumberFormatsSupplierObj::getUnoTunnelId());
        //  SvNumberFormatsSupplierObj* pSupplier = (SvNumberFormatsSupplierObj*)xSupplier->getImplementation(::getCppuType((const SvNumberFormatsSupplierObj*)0));

        DBG_ASSERT(pSupplier != NULL, "OPropertyBrowserController::ChangeFormatProperty : invalid call !");

        sal_Int32 nFormatKey = String(_rCurVal.getStr()).ToInt32();
        aCoreSet.Put(SfxUInt32Item(SID_ATTR_NUMBERFORMAT_VALUE, nFormatKey));

        SvNumberFormatter* pFormatter = pSupplier->GetNumberFormatter();
        double dPreviewVal = 1234.56789;
        SvxNumberInfoItem aFormatter(pFormatter, dPreviewVal, SID_ATTR_NUMBERFORMAT_INFO);
        aCoreSet.Put(aFormatter);

        // a tab dialog with a single page
        SfxSingleTabDialog* pDlg = new SfxSingleTabDialog(GetpApp()->GetAppWindow(), aCoreSet, 0);
        SvxNumberFormatTabPage* pPage = (SvxNumberFormatTabPage*) SvxNumberFormatTabPage::Create(pDlg, aCoreSet);
        const SfxPoolItem& rInfoItem = pPage->GetItemSet().Get(SID_ATTR_NUMBERFORMAT_INFO);
        pDlg->SetTabPage(pPage);

        if (RET_OK == pDlg->Execute())
        {
            const SfxItemSet* pResult = pDlg->GetOutputItemSet();

            const SfxPoolItem* pItem = pResult->GetItem( SID_ATTR_NUMBERFORMAT_INFO );
            const SvxNumberInfoItem* pInfoItem = static_cast<const SvxNumberInfoItem*>(pItem);
            if (pInfoItem && pInfoItem->GetDelCount())
            {
                const sal_uInt32* pDeletedKeys = pInfoItem->GetDelArray();

                for (sal_uInt16 i=0; i< pInfoItem->GetDelCount(); ++i, ++pDeletedKeys)
                    pFormatter->DeleteEntry(*pDeletedKeys);
            }

            pItem = NULL;
            if (SFX_ITEM_SET == pResult->GetItemState(SID_ATTR_NUMBERFORMAT_VALUE, sal_False, &pItem))
                Commit(_rName, ::rtl::OUString::valueOf((sal_Int32)((SfxUInt32Item*)pItem)->GetValue()), pSupplier);
        }
        delete pDlg;
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserController::SetFields( OLineDescriptor& rProperty )
    {
        try
        {
            WaitObject aWaitCursor(m_pView);

            rProperty.eControlType = BCT_COMBOBOX;
            Reference< XPreparedStatement >  xStatement;

            // get the form of the control we're inspecting
            Reference< XChild > xChild(m_xPropValueAccess, UNO_QUERY);
            Reference< XPropertySet > xFormSet;
            if (xChild.is())
                xFormSet = Reference< XPropertySet >(xChild->getParent(), UNO_QUERY);

            if (Reference< XGridColumnFactory >(xFormSet, UNO_QUERY).is())
            {   // we're inspecting a grid column -> the form is one step above
                xChild = Reference< XChild >(xFormSet, UNO_QUERY);
                if (xChild.is())
                    xFormSet = Reference< XPropertySet >(xChild->getParent(), UNO_QUERY);
                else
                    xFormSet.clear();
            }
            if (!xFormSet.is())
                return;

            ::rtl::OUString aObjectName = ::comphelper::getString(xFormSet->getPropertyValue(PROPERTY_COMMAND));
            // when there is no command we don't need to ask for columns
            if (aObjectName.getLength())
            {
                ::rtl::OUString aDatabaseName = ::comphelper::getString(xFormSet->getPropertyValue(PROPERTY_DATASOURCE));
                sal_Int32 nObjectType = ::comphelper::getINT32(xFormSet->getPropertyValue(PROPERTY_COMMANDTYPE));

                Reference< XConnection > xConnection = ensureRowsetConnection();
                Sequence< ::rtl::OUString > aFields;
                if ( xConnection.is() )
                    aFields = getFieldNamesByCommandDescriptor( xConnection, nObjectType, aObjectName );

                const ::rtl::OUString* pFields = aFields.getConstArray();
                for ( sal_Int32 i = 0; i < aFields.getLength(); ++i, ++pFields )
                    rProperty.aListValues.push_back( *pFields );
            }
        }
        catch (Exception&)
        {
            DBG_ERROR("OPropertyBrowserController::SetFields : caught an exception !")
        }
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserController::SetTables( OLineDescriptor& rProperty )
    {
        try
        {
            WaitObject aWaitCursor(m_pView);

            rProperty.eControlType = BCT_COMBOBOX;

            Reference< XTablesSupplier >  xTables;
            try
            {
                xTables = Reference< XTablesSupplier >( ensureRowsetConnection( ), UNO_QUERY );
            }
            catch (Exception&)
            {
                return;
            }

            Reference< XNameAccess >  xAccess;
            if (xTables.is())
                xAccess = xTables->getTables();
            if (!xAccess.is())
                return;

            Sequence< ::rtl::OUString> aTableNameSeq = xAccess->getElementNames();
            sal_uInt32 nCount = aTableNameSeq.getLength();
            const ::rtl::OUString* pTableNames = aTableNameSeq.getConstArray();

            for (sal_uInt32 i=0; i<nCount; ++i ,++pTableNames)
                rProperty.aListValues.push_back( *pTableNames);
        }

        catch (Exception&)
        {
            DBG_ERROR("OPropertyBrowserController::SetTables : caught an exception !")
        }
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserController::SetQueries( OLineDescriptor& rProperty )
    {
        try
        {
            WaitObject aWaitCursor(m_pView);

            rProperty.eControlType = BCT_COMBOBOX;

            Reference< XQueriesSupplier >  xSupplyQueries;
            try
            {
                xSupplyQueries = Reference< XQueriesSupplier >( ensureRowsetConnection(), UNO_QUERY );
            }
            catch (Exception&)
            {
                return;
            }

            Reference< XNameAccess >  xAccess;
            if (xSupplyQueries.is())
                xAccess = xSupplyQueries->getQueries();


            if (!xAccess.is())
                return;

            Sequence< ::rtl::OUString> aQueryNameSeq = xAccess->getElementNames();
            sal_uInt32 nCount = aQueryNameSeq.getLength();
            const ::rtl::OUString* pQueryNames = aQueryNameSeq.getConstArray();
            for (sal_uInt32 i=0; i<nCount; i++,++pQueryNames )
                rProperty.aListValues.push_back( *pQueryNames );
        }
        catch (Exception&)
        {
            DBG_ERROR("OPropertyBrowserController::SetQueries : caught an exception !")
        }
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserController::cleanupRowsetConnection()
    {
        Reference< XComponent > xConnComp( m_xRowsetConnection, UNO_QUERY );
        if ( xConnComp.is() )
            xConnComp->dispose();
        m_xRowsetConnection.clear();
    }

    //------------------------------------------------------------------------
    Reference< XConnection > OPropertyBrowserController::ensureRowsetConnection()
    {
        Reference< XConnection > xReturn;

        // get the row set we're working for
        Reference< XPropertySet > xProps( getRowSet( ), UNO_QUERY );
        if ( xProps.is() )
        {
            // get it's current active connection
            xProps->getPropertyValue( PROPERTY_ACTIVE_CONNECTION ) >>= xReturn;
            // do we need to connect?
            if ( !xReturn.is() )
            {
                connectRowset( );
                // get the property again
                xProps->getPropertyValue( PROPERTY_ACTIVE_CONNECTION ) >>= xReturn;
            }
        }

        // outta here
        return xReturn;
    }

    //------------------------------------------------------------------------
    Reference< XRowSet > OPropertyBrowserController::getRowSet( ) const
    {
        Reference< XRowSet > xRowSet(m_xPropValueAccess, UNO_QUERY);
        if (!xRowSet.is())
        {
            // are we inspecting a control?
            if ( 0 != m_nClassId )
            {
                xRowSet = Reference< XRowSet >(m_xObjectParent, UNO_QUERY);
                if (!xRowSet.is())
                {
                    // are we inspecting a grid column?
                    if (Reference< XGridColumnFactory >(m_xObjectParent, UNO_QUERY).is())
                    {   // we're inspecting a grid column
                        Reference< XChild > xParentAsChild(m_xObjectParent, UNO_QUERY);
                        if (xParentAsChild.is())
                            xRowSet = Reference< XRowSet >(xParentAsChild->getParent(), UNO_QUERY);
                    }
                }
            }

        }
        DBG_ASSERT( xRowSet.is(), "OPropertyBrowserController::SetQueries: could not obtain the rowset for the introspectee!" );
        return xRowSet;
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserController::connectRowset()
    {
        // if we have a previous connection, dispose it
        if ( haveRowsetConnection() )
            cleanupRowsetConnection();

        SQLExceptionInfo aErrorInfo;
        try
        {
            // the rowset
            Reference< XRowSet > xRowSet( getRowSet() );
            Reference< XPropertySet > xRowSetProps( xRowSet, UNO_QUERY );
            if (xRowSetProps.is())
            {
                // does the rowset already have a connection?
                Reference< XConnection > xConnection;
                xRowSetProps->getPropertyValue( PROPERTY_ACTIVE_CONNECTION ) >>= xConnection;

                if ( !xConnection.is() )
                {   // no -> calculate one
                    if (m_pView)
                    {
                        WaitObject aWaitCursor(m_pView);
                        xConnection = ::dbtools::connectRowset( xRowSet, m_xORB, sal_False );
                    }
                    else
                    {
                        xConnection = ::dbtools::connectRowset( xRowSet, m_xORB, sal_False );
                    }

                    // set on the row set
                    xRowSetProps->setPropertyValue( PROPERTY_ACTIVE_CONNECTION, makeAny( xConnection ) );

                    // remember for later disposal
                    // (we opened the connection, thus we own it)
                    m_xRowsetConnection = xConnection;
                }
            }
        }
        catch (SQLContext& e) { aErrorInfo = e; }
        catch (SQLWarning& e) { aErrorInfo = e; }
        catch (SQLException& e) { aErrorInfo = e; }
        catch (Exception&) { }

        if (aErrorInfo.isValid() && haveView())
        {
            ::rtl::OUString sDataSourceName;
            try
            {
                Reference< XPropertySet > xRSP( getRowSet(), UNO_QUERY );
                if ( xRSP.is() )
                    xRSP->getPropertyValue( PROPERTY_DATASOURCE ) >>= sDataSourceName;
            }
            catch( const Exception& )
            {
                DBG_ERROR( "OPropertyBrowserController::connectRowset: caught an exception during error handling!" );
            }
            // additional info about what happended
            String sInfo( ModuleRes( RID_STR_UNABLETOCONNECT ) );
            sInfo.SearchAndReplaceAllAscii( "$name$", sDataSourceName );

            SQLContext aContext;
            aContext.Message = sInfo;
            aContext.NextException = aErrorInfo.get();
            showError( aContext, VCLUnoHelper::GetInterface( m_pView ), m_xORB);
        }
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserController::SetCursorSource( sal_Bool _bConnect, sal_Bool _bInit )
    {
        try
        {
            if (!m_bHasCursorSource)
                return;

            WaitObject aWaitCursor(m_pView);

            // force the data page to be shown
            if (getPropertyBox()->GetCurPage() != m_nDataPageId)
                getPropertyBox()->SetPage(m_nDataPageId);

            ////////////////////////////////////////////////////////////
            // Auslesen des CursorSourceTypes
            String sCommandType = GetPropertyValue(PROPERTY_COMMANDTYPE);
            String sCommand = GetPropertyValue(PROPERTY_COMMAND);

            ////////////////////////////////////////////////////////////
            // Setzen der UI-Daten
            OLineDescriptor aProperty;
            aProperty.eControlType = BCT_COMBOBOX;

            aProperty.sName = (const ::rtl::OUString&)PROPERTY_COMMAND;
            aProperty.sTitle = m_pPropertyInfo->getPropertyTranslation(PROPERTY_ID_COMMAND);
            aProperty.pControl = NULL;
            aProperty.bHasBrowseButton = sal_False;
            aProperty.bIsHyperlink = sal_False;
            aProperty.bIsLocked = sal_False;
            aProperty.nHelpId = m_pPropertyInfo->getPropertyHelpId(PROPERTY_ID_COMMAND);
            if ( _bInit )
                aProperty.sValue = sCommand;

            if ( _bConnect )
                connectRowset();

            ////////////////////////////////////////////////////////////
            // Enums setzen

            sal_Bool bFailedToConnect = _bConnect && !haveRowsetConnection();
            if ( !bFailedToConnect )
            {
                Sequence< ::rtl::OUString > aCommandTypes = m_pPropertyInfo->getPropertyEnumRepresentations(PROPERTY_ID_COMMANDTYPE);
                sal_Int32 nPos = GetStringPos(sCommandType, aCommandTypes);
                if (0 == nPos)
                    SetTables(aProperty);
                else if (1 == nPos)
                    SetQueries(aProperty);
            }

            getPropertyBox()->ChangeEntry(aProperty, getPropertyBox()->GetPropertyPos(aProperty.sName));
            Commit(aProperty.sName, aProperty.sValue, NULL);
        }
        catch (Exception&)
        {
            DBG_ERROR("OPropertyBrowserController::SetCursorSource : caught an exception !")
        }
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserController::SetListSource(sal_Bool bInit)
    {
        try
        {
            if (!m_bHasListSource)
                return;

            WaitObject aWaitCursor(m_pView);

            // force the data page to be shown
            if (getPropertyBox()->GetCurPage() != m_nDataPageId)
                getPropertyBox()->SetPage(m_nDataPageId);

            ////////////////////////////////////////////////////////////
            // Auslesen des ListSourceTypes
            Any aListSourceTypeAny;

            ::rtl::OUString aStrVal;
            if (m_xPropStateAccess.is())
                aListSourceTypeAny = m_xPropValueAccess->getPropertyValue(PROPERTY_LISTSOURCETYPE );

            sal_Int32 nListSourceType;
            ::cppu::enum2int(nListSourceType,aListSourceTypeAny);

            ::rtl::OUString aListSource = GetPropertyValue( PROPERTY_LISTSOURCE );

            ////////////////////////////////////////////////////////////
            // Setzen der UI-Daten
            OLineDescriptor aProperty;
            aProperty.eControlType = BCT_MEDIT;
            aProperty.sName = (const ::rtl::OUString&)PROPERTY_LISTSOURCE;
            aProperty.sTitle = m_pPropertyInfo->getPropertyTranslation(PROPERTY_ID_LISTSOURCE);
            aProperty.pControl = NULL;
            aProperty.bHasBrowseButton = sal_False;
            aProperty.bIsHyperlink = sal_False;
            aProperty.bIsLocked = sal_False;
            aProperty.nHelpId=m_pPropertyInfo->getPropertyHelpId(PROPERTY_ID_LISTSOURCE);



            if (bInit)
                aProperty.sValue = aListSource;
            else
                aProperty.sValue = String();

            ////////////////////////////////////////////////////////////
            // Enums setzen
            switch( nListSourceType )
            {
                case ListSourceType_VALUELIST:
                    aProperty.eControlType = BCT_LEDIT;
                    break;

                case ListSourceType_TABLEFIELDS:
                case ListSourceType_TABLE:
                    SetTables( aProperty );
                    break;
                case ListSourceType_QUERY:
                    SetQueries( aProperty );
                    break;
            }

            ////////////////////////////////////////////////////////////
            // Eintrag umsetzen
            sal_uInt16 nCurPage = getPropertyBox()->GetCurPage();
            getPropertyBox()->SetPage( m_nDataPageId );
            getPropertyBox()->ChangeEntry( aProperty, getPropertyBox()->GetPropertyPos(aProperty.sName) );
            Commit( aProperty.sName, aProperty.sValue, NULL );
            getPropertyBox()->SetPage( nCurPage );
        }
        catch (Exception&)
        {
            DBG_ERROR("OPropertyBrowserController::SetListSource : caught an exception !")
        }
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserController::ChangeFontProperty( const ::rtl::OUString& rName )
    {
        // create an item set for use with the dialog
        SfxItemSet* pSet = NULL;
        SfxItemPool* pPool = NULL;
        SfxPoolItem** pDefaults = NULL;
        ControlCharacterDialog::createItemSet(pSet, pPool, pDefaults);
        ControlCharacterDialog::translatePropertiesToItems(m_xPropValueAccess, pSet);

        {   // do this in an own block. The dialog needs to be destroyed before we call
            // destroyItemSet
            ControlCharacterDialog aDlg(GetpApp()->GetAppWindow(), *pSet);
            if (RET_OK == aDlg.Execute())
            {
                const SfxItemSet* pOut = aDlg.GetOutputItemSet();
                String sNewFontName = ControlCharacterDialog::translatePropertiesToItems(pOut, m_xPropValueAccess);
                if (0 != sNewFontName.Len())
                    getPropertyBox()->SetPropertyValue( String::CreateFromAscii("Font"), sNewFontName);
            }
        }

        ControlCharacterDialog::destroyItemSet(pSet, pPool, pDefaults);
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserController::ChangeEventProperty( const ::rtl::OUString& _Name )
    {
        SfxMacroAssignDlg* pMacroDlg = NULL;
        String rName(_Name.getStr());

        if (rName.GetTokenCount()==0)
            return;

        ::rtl::OUString sListenerClassName = rName.GetToken( 0);
        ::rtl::OUString sMethodName = rName.GetToken(1);

        ::std::vector< ::rtl::OUString> aNameArray;

        try
        {

            Reference< XIndexAccess >  xIndexAcc(m_xObjectParent, UNO_QUERY);

            sal_Int32 nObjIdx=-1;
            // calc the index of the object with it's parent
            if (xIndexAcc.is())
            {
                sal_Int32 nCount = xIndexAcc->getCount();

                Reference< XPropertySet >  xTestSet;
                for (sal_Int32 i=0;i<nCount; ++i)
                {
                    ::cppu::extractInterface(xTestSet, xIndexAcc->getByIndex(i));
                    if (xTestSet.get() == m_xPropValueAccess.get())
                    {
                        nObjIdx=i;
                        break;
                    }
                }
            }

            // the the script events for this index
            sal_uInt32 nScrEvts=0;

            // For dialog editor mode, no EventManager but xEventsSupplier
            Reference< XScriptEventsSupplier > xEventsSupplier;

            Sequence< ScriptEventDescriptor > aSeqScrEvts;
            if (nObjIdx>=0 && m_xEventManager.is())
            {
                 aSeqScrEvts = m_xEventManager->getScriptEvents(nObjIdx);
            }
            else
            {
                // Dialog editor mode, no EventManager
                ::cppu::extractInterface( xEventsSupplier, m_aIntrospectee );
                if( xEventsSupplier.is() )
                {
                    Reference< XNameContainer > xEventCont = xEventsSupplier->getEvents();
                    Sequence< ::rtl::OUString > aNames = xEventCont->getElementNames();
                    sal_Int32 nLen = aNames.getLength();

                    const ::rtl::OUString* pNames = aNames.getConstArray();
                    aSeqScrEvts.realloc( nLen );
                    ScriptEventDescriptor* pDescs = aSeqScrEvts.getArray();

                    for( sal_Int32 i = 0 ; i < nLen ; i++ )
                    {
                        Any aElem = xEventCont->getByName( pNames[i] );
                        aElem >>= pDescs[i];
                    }
                }
            }
            nScrEvts = aSeqScrEvts.getLength();


            sal_uInt32 nLength = m_aObjectListenerTypes.getLength();
            const Type * pListeners = m_aObjectListenerTypes.getConstArray();
            const ScriptEventDescriptor* pEvDes = aSeqScrEvts.getConstArray();

            SvxMacroTableDtor aTable;

            sal_uInt16 nIndex=0;
            sal_uInt32 i;

            String aListener;
            ::rtl::OUString aOUListener;
            ::rtl::OUString aListenerClassName;
            Sequence< ::rtl::OUString> aMethSeq;

            for (i = 0 ; i < nLength ; i++ ,++pListeners)
            {
                // Namen besorgen
                aOUListener = pListeners->getTypeName();
                aListener = aOUListener;
                sal_Int32 nTokenCount = aListener.GetTokenCount('.');

                if (nTokenCount>0)
                    aListenerClassName= aListener.GetToken( (xub_StrLen)(nTokenCount-1), '.' );
                else
                    aListenerClassName= aListener;

                if (aListenerClassName.getLength()>0)
                {
                    // Methoden der Listener ausgeben
                    aMethSeq = getEventMethods( *pListeners );
                    const ::rtl::OUString * pMethods = aMethSeq.getConstArray();
                    sal_uInt32 nMethCount = aMethSeq.getLength();

                    for (sal_uInt32 j = 0 ; j < nMethCount ; ++j,++pMethods )
                    {

                        EventDisplayDescription* pEventDisplayDescription = GetEvtTranslation(*pMethods);

                        // be sure that the event method isn't mentioned twice
                        if (pEventDisplayDescription != NULL)
                        {
                            if (sListenerClassName == aListenerClassName && sMethodName == (*pMethods))
                            {
                                nIndex=aNameArray.size();
                            }


                            const ScriptEventDescriptor* pEvent = pEvDes;
                            for ( sal_uInt32 nI=0; nI<nScrEvts; ++nI, ++pEvent)
                            {
                                if  (   (   ( pEvent->ListenerType == aListenerClassName )
                                        ||  ( pEvent->ListenerType == aOUListener )
                                        )
                                    &&  ( pEvent->EventMethod == (*pMethods) )
                                    )
                                {
                                    SvxMacro* pMacro = NULL;

                                    if  (   (pEvent->ScriptCode.getLength() > 0)
                                        &&  (pEvent->ScriptType.getLength() > 0)
                                        )
                                    {
                                        ::rtl::OUString sScriptType = pEvent->ScriptType;
                                        ::rtl::OUString sScriptCode = pEvent->ScriptCode;
                                        ::rtl::OUString sLibName;

                                        if ( 0 == sScriptType.compareToAscii( "StarBasic" ) )
                                        {   // it's a StarBasic macro
                                            // strip the prefix from the macro name (if any)

                                            sal_Int32 nPrefixLen = sScriptCode.indexOf( ':' );
                                            if ( nPrefixLen >= 0 )
                                            {   // it contains a prefix
                                                ::rtl::OUString sPrefix = sScriptCode.copy( 0, nPrefixLen );
                                                sScriptCode = sScriptCode.copy( nPrefixLen + 1 );
                                                if ( 0 == sPrefix.compareToAscii( "application" ) )
                                                {
                                                    sLibName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "StarOffice" ) );
                                                }
                                                else if ( 0 == sPrefix.compareToAscii( "document" ) )
                                                {
                                                    // ??? document name is unknown here!
                                                }
                                                else
                                                    DBG_ERROR( "OPropertyBrowserController::ChangeEventProperty: invalid (unknown) prefix" );
                                            }
                                        }

                                        SvxMacro aTypeTranslator( sScriptCode, sScriptType );
                                        pMacro = new SvxMacro( sScriptCode, sLibName, aTypeTranslator.GetScriptType() );
                                    }

                                    aTable.Insert(aNameArray.size(), pMacro);
                                }
                            }

                            aNameArray.push_back(pEventDisplayDescription->sDisplayName);
                        }
                    }
                }
            }

            SvxMacroItem aMacroItem;

            aMacroItem.SetMacroTable(aTable);

            SfxItemSet aSet( SFX_APP()->GetPool(), SID_ATTR_MACROITEM, SID_ATTR_MACROITEM );
            aSet.Put(aMacroItem, SID_ATTR_MACROITEM);
            pMacroDlg = new SfxMacroAssignDlg(
                GetpApp()->GetAppWindow(), aSet );
            SfxMacroTabPage* pMacroTabPage = (SfxMacroTabPage*)pMacroDlg->GetTabPage();

            for (sal_uInt32 j = 0 ; j < aNameArray.size(); j++ )
                pMacroTabPage->AddEvent( aNameArray[j], (sal_uInt16)j);

            if (nIndex<aNameArray.size())
                pMacroTabPage->SelectEvent( aNameArray[nIndex], nIndex);

            if ( pMacroDlg->Execute() == RET_OK )
            {
                setDocumentModified( ); // OJ: #96105#

                const SvxMacroTableDtor& aTab = pMacroTabPage->GetMacroTbl();

                if ( nObjIdx>=0 && m_xEventManager.is() )
                    m_xEventManager->revokeScriptEvents( nObjIdx );


                sal_uInt16 nEventCount = (sal_uInt16)aTab.Count();
                sal_uInt16 nEventIndex = 0;

                Sequence< ScriptEventDescriptor > aSeqScriptEvts(nEventCount);

                ScriptEventDescriptor* pWriteScriptEvents = aSeqScriptEvts.getArray();
                nIndex=0;

                String aListenerClassName,aName,aListener;

                pListeners = m_aObjectListenerTypes.getConstArray();

                ::rtl::OUString sScriptCode;
                for (i = 0 ; i < nLength ; ++i, ++pListeners )
                {
                    // Methode ansprechen

                    // Namen besorgen
                    aListener = pListeners->getTypeName();
                    sal_Int32 nTokenCount=aListener.GetTokenCount('.');

                    if (nTokenCount>0)
                        aListenerClassName = aListener.GetToken((xub_StrLen)(nTokenCount-1), '.' );
                    else
                        aListenerClassName = aListener;

                    if (aListenerClassName.Len() != 0)
                    {
                        // Methoden der Listener ausgeben
                        aMethSeq = getEventMethods( *pListeners );

                        const ::rtl::OUString* pMethods     =               +   aMethSeq.getConstArray();
                        const ::rtl::OUString* pMethodsEnd  =   pMethods    +   aMethSeq.getLength();
                        for ( ; pMethods != pMethodsEnd; ++pMethods )
                        {
                            EventDisplayDescription* pEventDisplayDescription = GetEvtTranslation( *pMethods );

                            if ( pEventDisplayDescription )
                            {
                                SvxMacro* pMacro = aTab.Get( nIndex++ );
                                if ( pMacro )
                                {
                                    sScriptCode = pMacro->GetMacName();
                                    if ( nEventIndex < nEventCount )
                                    {
                                        if ( m_xEventManager.is() )
                                        {
                                            pWriteScriptEvents->ListenerType = aListenerClassName;
                                        }
                                        else
                                        {   // Dialog editor mode
                                            pWriteScriptEvents->ListenerType = aListener;
                                        }

                                        sal_Bool bApplicationMacro = pMacro->GetLibName().EqualsAscii("StarOffice");

                                        sScriptCode = ::rtl::OUString::createFromAscii( bApplicationMacro ? "application:" : "document:" );
                                        sScriptCode += pMacro->GetMacName();

                                        pWriteScriptEvents->ScriptCode = sScriptCode;
                                        pWriteScriptEvents->EventMethod = *pMethods;
                                        pWriteScriptEvents->ScriptType = pMacro->GetLanguage();

                                        ++nEventIndex;
                                        ++pWriteScriptEvents;
                                    }
                                }
                                else
                                    sScriptCode = ::rtl::OUString();

                                // set the new "property value"
                                aName = aListenerClassName;
                                aName += ';';
                                aName += pMethods->getStr();
                                getPropertyBox()->SetPropertyValue( aName, sScriptCode);
                            }
                        }
                    }
                }

                if (nObjIdx>=0 && m_xEventManager.is())
                {
                    m_xEventManager->registerScriptEvents(nObjIdx,aSeqScriptEvts);
                }
                else if( xEventsSupplier.is() )
                {
                    Reference< XNameContainer > xEventCont = xEventsSupplier->getEvents();

                    // Make it simple: Revove all old events...
                    Sequence< ::rtl::OUString > aNames = xEventCont->getElementNames();
                    sal_Int32 nLen = aNames.getLength();
                    const ::rtl::OUString* pNames = aNames.getConstArray();
                    sal_Int32 i = 0;
                    for( i = nLen - 1; i >= 0 ; i-- )
                        xEventCont->removeByName( pNames[i] );

                    // ... and insert the new ones
                    const ScriptEventDescriptor* pDescs = aSeqScriptEvts.getConstArray();
                    sal_Int32 nNewCount = aSeqScriptEvts.getLength();
                    for( i = 0 ; i < nNewCount ; i++ )
                    {
                        const ScriptEventDescriptor& rDesc = pDescs[ i ];
                        ::rtl::OUString aName = rDesc.ListenerType;
                        aName += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "::" ) );
                        aName += rDesc.EventMethod;

                        Any aEventAny;
                        aEventAny <<= rDesc;
                        xEventCont->insertByName( aName, aEventAny );
                    }
                }
            }
        }
        catch (Exception&)
        {
            DBG_ERROR("OPropertyBrowserController::ChangeEventProperty : caught an exception !")
        }

        delete pMacroDlg;
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserController::InsertEvents()
    {
        //////////////////////////////////////////////////////////////////////
        // Seite fuer Events
        m_nEventPageId = getPropertyBox()->AppendPage(String(ModuleRes(RID_STR_EVENTS)), HID_FM_PROPDLG_TAB_EVT);

        sal_Bool  bRemoveFlag = sal_True;

        try
        {
            Reference< XIndexAccess >  xIndexAcc(m_xObjectParent, UNO_QUERY);
            sal_Int32 nObjIdx=-1;

            // get the index of the inspected object within it's parent container
            if (xIndexAcc.is() && m_xPropValueAccess.is())
            {
                sal_Int32 nCount = xIndexAcc->getCount();
                Reference< XPropertySet >  xTestSet;
                for (sal_Int32 i=0; i<nCount; ++i)
                {
                    ::cppu::extractInterface(xTestSet, xIndexAcc->getByIndex(i));
                    if (xTestSet.get() == m_xPropValueAccess.get())
                    {
                        nObjIdx=i;
                        break;
                    }
                }
            }

            // get the current script events for this index
            sal_uInt32 nScrEvts=0;
            sal_Bool bShowEventPage = sal_False;
            Sequence< ScriptEventDescriptor > aSeqScrEvts;
            if (nObjIdx>=0 && m_xEventManager.is())
            {
                aSeqScrEvts = m_xEventManager->getScriptEvents(nObjIdx);
                bShowEventPage = sal_True;
            }
            else
            {
                // Dialog editor mode, no EventManager
                Reference< XScriptEventsSupplier > xEventsSupplier;
                ::cppu::extractInterface( xEventsSupplier, m_aIntrospectee );
                if( xEventsSupplier.is() )
                {
                    Reference< XNameContainer > xEventCont = xEventsSupplier->getEvents();
                    Sequence< ::rtl::OUString > aNames = xEventCont->getElementNames();
                    sal_Int32 nLen = aNames.getLength();

                    const ::rtl::OUString* pNames = aNames.getConstArray();
                    aSeqScrEvts.realloc( nLen );
                    ScriptEventDescriptor* pDescs = aSeqScrEvts.getArray();

                    for( sal_Int32 i = 0 ; i < nLen ; i++ )
                    {
                        Any aElem = xEventCont->getByName( pNames[i] );
                        aElem >>= pDescs[i];
                    }
                    bShowEventPage = sal_True;
                }
            }
            nScrEvts = aSeqScrEvts.getLength();

            if( !bShowEventPage )
            {   // could not obtain the position in the event attacher manager
                // (or don't have this manager)
                // -> no event page
                if (m_nEventPageId)
                    getPropertyBox()->RemovePage(m_nEventPageId);
                m_nEventPageId=0;
                return;
            }

            sal_uInt32 nLength = m_aObjectListenerTypes.getLength();
            const Type * pListeners = m_aObjectListenerTypes.getConstArray();

            OLineDescriptor aProperty;
            aProperty.pDataPtr = LINETYPE_EVENT;
            aProperty.bIsLocked = sal_True;

            DECLARE_STL_SET( OLineDescriptor, OLineDescriptorLess, LineDescriptorSet );
            LineDescriptorSet aEventLines;

            const ScriptEventDescriptor* pEvDes = aSeqScrEvts.getConstArray();
            String aListener;
            String aListenerClassName;
            String aMethName;
            for (sal_uInt32 i = 0 ; i < nLength ; ++i, ++pListeners )
            {
                // Methode ansprechen
                //  const Reference< XIdlClass > & rxClass = pListeners[i];

                // Namen besorgen
                aListener = pListeners->getTypeName();
                sal_uInt32 nTokenCount = aListener.GetTokenCount('.');


                if (nTokenCount>0)
                    aListenerClassName= aListener.GetToken((sal_uInt16)nTokenCount-1, '.');
                else
                    aListenerClassName= aListener;

                if (aListenerClassName.Len() != 0)
                {
                    // Methoden der Listener ausgeben
                    Sequence< ::rtl::OUString > aMethSeq(getEventMethods( *pListeners ));
                    const ::rtl::OUString * pMethods = aMethSeq.getConstArray();
                    sal_uInt32 nMethCount = aMethSeq.getLength();

                    for (sal_uInt32 j = 0 ; j < nMethCount ; j++,++pMethods )
                    {
                        //  Reference< XIdlMethod >  xMethod = pMethods[ j ];

                        //  aMethName=xMethod->getName();
                        aProperty.eControlType = BCT_EDIT;
                        aProperty.sName = aListenerClassName;
                        aProperty.sName += String(';');
                        aProperty.sName += (const sal_Unicode*)*pMethods;
                        aProperty.sTitle = *pMethods;
                        aProperty.nHelpId=0;
                        aProperty.sValue = String();
                        aProperty.bHasBrowseButton = sal_True;

                        for (sal_uInt32 nI=0; nI<nScrEvts;nI++)
                        {
                            const ScriptEventDescriptor& rEvDe = pEvDes[nI];
                            if ( (aListenerClassName.Equals((const sal_Unicode*)rEvDe.ListenerType)
                                 || aListener.Equals((const sal_Unicode*)rEvDe.ListenerType) )
                                && pMethods->equals(rEvDe.EventMethod))
                                aProperty.sValue = rEvDe.ScriptCode;
                        }

                        EventDisplayDescription* pEventDescription = GetEvtTranslation(*pMethods);
                        if (pEventDescription)
                        {
                            aProperty.sTitle = pEventDescription->sDisplayName;
                            aProperty.nHelpId = pEventDescription->nHelpId;
                            aProperty.nUniqueButtonId = pEventDescription->nIndex;
                            aEventLines.insert(aProperty);
                        }
                    }
                }
            }

            for (   ConstLineDescriptorSetIterator iter = aEventLines.begin();
                    iter != aEventLines.end();
                    ++iter
                )
            {
                // Now set the right id
                OLineDescriptor aData(*iter);
                aData.nUniqueButtonId = UID_EVT_MACRODLG;
                getPropertyBox()->InsertEntry( aData );
            }

            bRemoveFlag = aEventLines.empty();
        }
        catch (Exception&)
        {
            DBG_ERROR("OPropertyBrowserController::InsertEvents : caught an exception !")
            bRemoveFlag=sal_True;
        }

        if (bRemoveFlag)
        {
            getPropertyBox()->RemovePage(m_nEventPageId);
            m_nEventPageId=0;
        }
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserController::UpdateUI()
    {
        // Introspection auswerten
        try
        {
            getPropertyBox()->DisableUpdate();

            sal_Bool bHaveFocus = getPropertyBox()->HasChildPathFocus();

            InsertEvents();
            sal_uInt32 nPropCount = m_aObjectProperties.getLength();
            const Property* pProps = m_aObjectProperties.getConstArray();
            OLineDescriptor* pProperty = NULL;
            sal_Bool bRemoveDataPage=sal_True;

            TypeClass eType;
            Any aVal,aSupplier,aKey,aDigits,aSeparator,aDefault;
            ::rtl::OUString aStrVal;
            PropertyState eState;

            // get control type
            sal_Int16 nControlType = getControlType();

            // the properties for which I need to update any dependent properties
            ::std::vector< ::rtl::OUString > aFoundDependencies;

            for (sal_uInt32 i=0; i<nPropCount; ++i, ++pProps)
            {
                sal_Int32   nPropId         = m_pPropertyInfo->getPropertyId( pProps->Name );
                if ( nPropId == -1 )
                    continue;

                String      sDisplayName    = m_pPropertyInfo->getPropertyTranslation( nPropId );
                sal_uInt32  nPropertyUIFlags= m_pPropertyInfo->getPropertyUIFlags( nPropId );

                if ( !sDisplayName.Len() )
                {
                    DBG_ERROR( "OPropertyBrowserController::UpdateUI: no display name! corrupt meta data?" );
                    continue;
                }

                bool bIsVirtualProperty = ( nPropertyUIFlags & PROP_FLAG_VIRTUAL_PROP ) != 0;

                pProperty = new OLineDescriptor();

                //////////////////////////////////////////////////////////////////////
                // TypeClass des Property ermitteln
                eType = pProps->Type.getTypeClass();

                //////////////////////////////////////////////////////////////////////
                // Wert holen und in ::rtl::OUString wandeln
                eState = PropertyState_DIRECT_VALUE;
                if ( m_xPropStateAccess.is() && !bIsVirtualProperty )
                    eState = m_xPropStateAccess->getPropertyState( pProps->Name );

                if ( bIsVirtualProperty )
                    aVal = getVirtualPropertyValue( nPropId );
                else
                    aVal = m_xPropValueAccess->getPropertyValue( pProps->Name );

                aStrVal = getStringRepFromPropertyValue( aVal, nPropId );

                //////////////////////////////////////////////////////////////////////
                // Default Properties
                pProperty->eControlType = BCT_EDIT;
                pProperty->sName = pProps->Name;
                pProperty->sTitle = pProps->Name;
                pProperty->sValue = aStrVal;
                pProperty->pControl = NULL;
                pProperty->bIsLocked = sal_False;
                pProperty->bHasBrowseButton = sal_False;
                pProperty->bIsHyperlink = sal_False;

                if ((pProps->Attributes & PropertyAttribute::MAYBEVOID) && nPropId != PROPERTY_ID_BORDER) //&& eState!=DIRECT_VALUE
                {
                    pProperty->bHasDefaultValue = sal_True;
                    if (!aVal.hasValue())
                        pProperty->sValue = m_sStandard;
                }
                else
                    pProperty->bHasDefaultValue =sal_False;

                //////////////////////////////////////////////////////////////////////
                // Font
                sal_Bool bFilter = sal_True;
                if (nPropId == PROPERTY_ID_FONT_NAME)
                {
                    bFilter = sal_False;

                    pProperty->sName = String::CreateFromAscii("Font");
                    pProperty->sTitle = pProperty->sName;
                    pProperty->bIsLocked = sal_True;
                    pProperty->bHasBrowseButton = sal_True;
                    pProperty->nUniqueButtonId = UID_PROP_DLG_FONT_TYPE;
                    ::rtl::OUString sValue;
                    aVal >>= sValue;
                    pProperty->sValue = sValue;
                }
                else if (nPropId == PROPERTY_ID_TARGET_URL)
                {
                    pProperty->bHasBrowseButton = sal_True;
                    pProperty->nUniqueButtonId = UID_PROP_DLG_ATTR_TARGET_URL;
                }
                else if (nPropId == PROPERTY_ID_IMAGE_URL)
                {
                    pProperty->bHasBrowseButton = sal_True;
                    pProperty->nUniqueButtonId = UID_PROP_DLG_IMAGE_URL;
                }

                else if (nPropId== PROPERTY_ID_ECHO_CHAR)
                {
                    pProperty->eControlType = BCT_CHAREDIT;  //@ new CharEdit
                }
                //////////////////////////////////////////////////////////////////////
                // Color
                else if (nPropId== PROPERTY_ID_BACKGROUNDCOLOR )
                {
                    bFilter = sal_False;
                    pProperty->eControlType = BCT_COLORBOX;  //@ new ColorListbox
                    pProperty->bIsLocked = sal_True;
                    pProperty->bHasBrowseButton = sal_True;
                    pProperty->nUniqueButtonId = UID_PROP_DLG_BACKGROUNDCOLOR;
                }
                else if (nPropId== PROPERTY_ID_FILLCOLOR )
                {
                    bFilter = sal_False;
                    pProperty->eControlType = BCT_COLORBOX;  //@ new ColorListbox
                    pProperty->bIsLocked = sal_True;
                    pProperty->bHasBrowseButton = sal_True;
                    pProperty->nUniqueButtonId = UID_PROP_DLG_FILLCOLOR;
                }
                else if (nPropId == PROPERTY_ID_LABEL)
                {
                    pProperty->eControlType = BCT_MEDIT;
                }
                else if (nPropId == PROPERTY_ID_DEFAULT_TEXT)
                {
                    if (FormComponentType::FILECONTROL == m_nClassId)
                        pProperty->eControlType = BCT_EDIT;
                    else
                        pProperty->eControlType = BCT_MEDIT;
                }
                else if (nPropId == PROPERTY_ID_TEXT)
                {
                    if ( m_xIntrospecteeAsProperty.is() )
                    {
                        Reference< XServiceInfo > xInfo(m_xIntrospecteeAsProperty, UNO_QUERY);
                        if ( xInfo.is() )
                        {
                            if ( xInfo->supportsService( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlFormattedFieldModel" ) ) ) )
                            {
                                delete pProperty;
                                continue;
                            }
                        }

                        Reference< XPropertySetInfo > xPropInfo = m_xIntrospecteeAsProperty->getPropertySetInfo();
                        if ( xPropInfo.is() )
                        {
                            if ( xPropInfo->hasPropertyByName( PROPERTY_MULTILINE ) )
                                pProperty->eControlType = BCT_MEDIT;
                        }
                    }
                }
                else if (PROPERTY_ID_CONTROLLABEL == nPropId)
                {
                    bFilter = sal_False;
                    pProperty->bHasBrowseButton = sal_True;
                    pProperty->bIsLocked = sal_True;
                    pProperty->sValue = getStringRepFromPropertyValue(aVal, PROPERTY_ID_CONTROLLABEL);
                    pProperty->nUniqueButtonId = UID_PROP_DLG_CONTROLLABEL;
                }
                else if ((PROPERTY_ID_FORMATKEY == nPropId) || (PROPERTY_ID_EFFECTIVE_MIN == nPropId)
                    || (PROPERTY_ID_EFFECTIVE_MAX == nPropId) || (PROPERTY_ID_EFFECTIVE_DEFAULT == nPropId)
                    || (PROPERTY_ID_EFFECTIVE_VALUE == nPropId))
                {
                    // only if the set has a formatssupplier, too
                    if  (   !::comphelper::hasProperty(PROPERTY_FORMATSSUPPLIER, m_xPropValueAccess)
                        ||  (FormComponentType::DATEFIELD == m_nClassId)
                        ||  (FormComponentType::TIMEFIELD == m_nClassId)
                        )
                    {
                        delete pProperty;
                        continue;
                    }

                    // and the supplier is really available
                    Reference< XNumberFormatsSupplier >  xSupplier;
                    m_xPropValueAccess->getPropertyValue(PROPERTY_FORMATSSUPPLIER) >>= xSupplier;
                    if (xSupplier.is())
                    {
                        Reference< XUnoTunnel > xTunnel(xSupplier,UNO_QUERY);
                        DBG_ASSERT(xTunnel.is(), "OPropertyBrowserController::ChangeFormatProperty : xTunnel is invalid!");
                        SvNumberFormatsSupplierObj* pSupplier = (SvNumberFormatsSupplierObj*)xTunnel->getSomething(SvNumberFormatsSupplierObj::getUnoTunnelId());

                        if (pSupplier != NULL)
                        {
                            bFilter = sal_False;    // don't do further checks
                            sal_Bool bIsFormatKey = (PROPERTY_ID_FORMATKEY == nPropId);

                            pProperty->eControlType = BCT_USERDEFINED;

                            pProperty->bIsLocked = bIsFormatKey;
                            pProperty->bHasBrowseButton = bIsFormatKey;

                            if (bIsFormatKey)
                            {
                                pProperty->pControl = new OFormatDescriptionControl(getPropertyBox(), WB_READONLY | WB_TABSTOP | WB_BORDER);
                                    // HACK : the Control need's a non-null parent, but we don't have one ... so use the property box
                                ((OFormatDescriptionControl*)pProperty->pControl)->SetFormatSupplier(pSupplier);

                                pProperty->nUniqueButtonId = UID_PROP_DLG_NUMBER_FORMAT;
                            }
                            else
                            {
                                pProperty->pControl = new OFormattedNumericControl(getPropertyBox(), WB_TABSTOP | WB_BORDER);
                                    // HACK : same as above

                                FormatDescription aDesc;
                                aDesc.pSupplier = pSupplier;
                                aKey = m_xPropValueAccess->getPropertyValue(PROPERTY_FORMATKEY);
                                aDesc.nKey = aKey.hasValue() ? ::comphelper::getINT32(aKey) : 0;
                                ((OFormattedNumericControl*)pProperty->pControl)->SetFormatDescription(aDesc);
                            }

                            // the initial value
                            if (aVal.hasValue())
                            {
                                if (bIsFormatKey)
                                {
                                    pProperty->sValue = String::CreateFromInt32(::comphelper::getINT32(aVal));
                                }
                                else
                                {
                                    if (aVal.getValueTypeClass() == TypeClass_DOUBLE)
                                        pProperty->sValue = convertSimpleToString(aVal);
                                    else
                                        DBG_WARNING("OPropertyBrowserController::UpdateUI : non-double values not supported for Effective*-properties !");
                                        // our UI supports only setting double values for the min/max/default, but by definition
                                        // the default may be a string if the field is not in numeric mode ....
                                }
                            }
                        }
                    }
                }
                //////////////////////////////////////////////////////////////////////
                // ::rtl::OUString Sequence
                else if (eType == TypeClass_SEQUENCE )
                {
                    pProperty->eControlType = BCT_LEDIT;
                    bFilter = sal_False;
                }

                else if (TypeClass_BYTE <=eType && eType<=TypeClass_DOUBLE)
                {
                    if (nPropId==PROPERTY_ID_DATEMIN || nPropId==PROPERTY_ID_DATEMAX || nPropId==PROPERTY_ID_DEFAULT_DATE || nPropId==PROPERTY_ID_DATE)
                        pProperty->eControlType = BCT_DATEFIELD;
                    else if (nPropId==PROPERTY_ID_TIMEMIN || nPropId==PROPERTY_ID_TIMEMAX || nPropId==PROPERTY_ID_DEFAULT_TIME || nPropId==PROPERTY_ID_TIME)
                        pProperty->eControlType = BCT_TIMEFIELD;
                    else
                    {
                        if (nPropId== PROPERTY_ID_VALUEMIN || nPropId== PROPERTY_ID_VALUEMAX || nPropId==PROPERTY_ID_DEFAULT_VALUE || nPropId==PROPERTY_ID_VALUE)
                        {
                            pProperty->eControlType = BCT_USERDEFINED;
                            pProperty->pControl = new OFormattedNumericControl(getPropertyBox(), WB_TABSTOP | WB_BORDER | WB_SPIN);
                                // HACK : same as above

                            // we don't set a formatter so the control uses a default (which uses the application
                            // language and a default numeric format)
                            // but we set the decimal digits
                            aDigits = m_xPropValueAccess->getPropertyValue(PROPERTY_DECIMAL_ACCURACY);
                            ((OFormattedNumericControl*)pProperty->pControl)->SetDecimalDigits(::comphelper::getINT16(aDigits));

                            // and the thousands separator
                            aSeparator = m_xPropValueAccess->getPropertyValue(PROPERTY_SHOWTHOUSANDSEP);
                            ((OFormattedNumericControl*)pProperty->pControl)->SetThousandsSep(::comphelper::getBOOL(aSeparator));

                            // and the default value for the property
                            try
                            {
                                if (m_xPropStateAccess.is() && ((PROPERTY_ID_VALUEMIN == nPropId) || (PROPERTY_ID_VALUEMAX == nPropId)))
                                {
                                    aDefault = m_xPropStateAccess->getPropertyDefault(pProps->Name);
                                    if (aDefault.getValueTypeClass() == TypeClass_DOUBLE)

                                        ((OFormattedNumericControl*)pProperty->pControl)->SetDefaultValue(::comphelper::getDouble(aDefault));
                                }
                            }
                            catch (Exception&)
                            {
                                // just ignore it
                            }

                            // and allow empty values only for the default value and the value
                            static_cast< OFormattedNumericControl* >( pProperty->pControl )
                                ->EnableEmptyField( ( PROPERTY_ID_DEFAULT_VALUE == nPropId )
                                                ||  ( PROPERTY_ID_VALUE == nPropId ) );
}
                        else
                        {
                            if ( (nPropId== PROPERTY_ID_HEIGHT || nPropId== PROPERTY_ID_WIDTH || nPropId== PROPERTY_ID_ROWHEIGHT)
                                && nControlType == CONTROL_TYPE_FORM )
                                pProperty->nDigits=1;

                            pProperty->eControlType = BCT_NUMFIELD;
                        }
                    }
                }

                // don't filter dialog controls
                if ( nControlType == CONTROL_TYPE_DIALOG )
                    bFilter = sal_False;
                // and don't filter virtual properties
                if ( bIsVirtualProperty )
                    bFilter = sal_False;

                //////////////////////////////////////////////////////////////////////
                // Filter
                if (bFilter)
                {
                    switch( eType )     // TypeClass Inspection
                    {
                    case TypeClass_INTERFACE:
                    case TypeClass_ARRAY:
                        delete pProperty->pControl;
                        delete pProperty;
                        continue;
                    }

                    switch( aVal.getValueTypeClass() )      // TypeClass Any

                    {
                    case TypeClass_VOID:
                        if (pProps->Attributes & PropertyAttribute::MAYBEVOID)
                            break;

                    case TypeClass_INTERFACE:
                    case TypeClass_ARRAY:
                    case TypeClass_UNKNOWN:
                        delete pProperty->pControl;
                        delete pProperty;
                        continue;
                    }

                    if (pProps->Name.compareTo(::rtl::OUString::createFromAscii("type unknown")) == COMPARE_EQUAL )
                    {
                        delete pProperty->pControl;
                        delete pProperty;
                        continue;
                    }

                    if (pProps->Attributes & PropertyAttribute::TRANSIENT )
                    {
                        delete pProperty->pControl;
                        delete pProperty;
                        continue;
                    }

                    else if (pProps->Attributes & PropertyAttribute::READONLY )
                    {
                        delete pProperty->pControl;
                        delete pProperty;
                        continue;
                    }
                }

                //////////////////////////////////////////////////////////////////////
                // sal_Bool-Werte
                if (eType == TypeClass_BOOLEAN )
                {
                    String aEntries(ModuleRes(RID_STR_BOOL));
                    for ( xub_StrLen i=0; i<2; ++i )
                        pProperty->aListValues.push_back( aEntries.GetToken(i) );

                    pProperty->eControlType = BCT_LISTBOX;
                }

                //////////////////////////////////////////////////////////////////////
                // TYPECLASS_VOID
                else if (eType == TypeClass_VOID )
                    pProperty->sValue = String();

                //////////////////////////////////////////////////////////////////////
                // Listen mit ResStrings fuellen
                switch( nPropId )
                {
                    case PROPERTY_ID_COMMANDTYPE:
                    case PROPERTY_ID_ALIGN:
                    case PROPERTY_ID_BUTTONTYPE:
                    case PROPERTY_ID_PUSHBUTTONTYPE:
                    case PROPERTY_ID_SUBMIT_METHOD:
                    case PROPERTY_ID_SUBMIT_ENCODING:
                    case PROPERTY_ID_DATEFORMAT:
                    case PROPERTY_ID_TIMEFORMAT:
                    case PROPERTY_ID_BORDER:
                    case PROPERTY_ID_CYCLE:
                    case PROPERTY_ID_NAVIGATION:
                    case PROPERTY_ID_TARGET_FRAME:
                    case PROPERTY_ID_DEFAULTCHECKED:
                    case PROPERTY_ID_STATE:
                    case PROPERTY_ID_LISTSOURCETYPE:
                    case PROPERTY_ID_ORIENTATION:
                    case PROPERTY_ID_IMAGEALIGN:
                    case PROPERTY_ID_CELL_EXCHANGE_TYPE:
                    {
                        Sequence< ::rtl::OUString > aEnumValues = m_pPropertyInfo->getPropertyEnumRepresentations(nPropId);
                        const ::rtl::OUString* pStart = aEnumValues.getConstArray();
                        const ::rtl::OUString* pEnd = pStart + aEnumValues.getLength();

                        // for a checkbox: if "ambiguous" is not allowed, remove this from the sequence
                        if (PROPERTY_ID_DEFAULTCHECKED == nPropId || PROPERTY_ID_STATE == nPropId)
                            if (::comphelper::hasProperty(PROPERTY_TRISTATE, m_xPropValueAccess))
                            {
                                if (!::comphelper::getBOOL(m_xPropValueAccess->getPropertyValue(PROPERTY_TRISTATE)))
                                {   // remove the last sequence element
                                    if (pEnd > pStart)
                                        --pEnd;
                                }
                            }
                            else
                                --pEnd;

                        if (PROPERTY_ID_LISTSOURCETYPE == nPropId)
                            if (FormComponentType::COMBOBOX == m_nClassId)
                                // remove the first sequence element
                                ++pStart;

                        // copy the sequence
                        for (const ::rtl::OUString* pLoop = pStart; pLoop != pEnd; ++pLoop)
                            pProperty->aListValues.push_back(*pLoop);

                        pProperty->eControlType =
                                PROPERTY_ID_TARGET_FRAME == nPropId
                            ?   BCT_COMBOBOX
                            :   BCT_LISTBOX;
                    }
                    break;

                    case PROPERTY_ID_MAXTEXTLEN:
                    case PROPERTY_ID_TABINDEX:
                    case PROPERTY_ID_BOUNDCOLUMN:
                        pProperty->nMaxValue = 0x7FFFFFFF;
                        pProperty->bHaveMinMax = sal_True;
                        switch (nPropId)
                        {
                            case PROPERTY_ID_MAXTEXTLEN:    pProperty->nMinValue = -1; break;
                            case PROPERTY_ID_TABINDEX:      pProperty->nMinValue = 0; break;
                            case PROPERTY_ID_BOUNDCOLUMN:   pProperty->nMinValue = 1; break;
                        }
                        break;

                    case PROPERTY_ID_DECIMAL_ACCURACY:
                        pProperty->nMaxValue = 20;
                        pProperty->nMinValue = 0;
                        pProperty->bHaveMinMax = sal_True;
                        break;

                    case PROPERTY_ID_BOUND_CELL:
                    case PROPERTY_ID_LIST_CELL_RANGE:
                        aFoundDependencies.push_back( pProps->Name );
                        break;

                    //////////////////////////////////////////////////////////////////////
                    // DataSource
                    case PROPERTY_ID_DATASOURCE:
                    {
                        pProperty->nUniqueButtonId = UID_PROP_DLG_ATTR_DATASOURCE;
                        // if the form already belong to a Database, don't set this property
                        Reference< XInterface > xInter;
                        m_aIntrospectee >>= xInter;
                        pProperty->bHasBrowseButton = sal_False;
                        pProperty->eControlType = BCT_COMBOBOX;

                        Reference< XNameAccess > xDatabaseContext(m_xORB->createInstance(SERVICE_DATABASE_CONTEXT), UNO_QUERY);
                        if (xDatabaseContext.is())
                        {
                            Sequence< ::rtl::OUString > aDatasources = xDatabaseContext->getElementNames();
                            const ::rtl::OUString* pBegin = aDatasources.getConstArray();
                            const ::rtl::OUString* pEnd = pBegin + aDatasources.getLength();
                            for (; pBegin != pEnd;++pBegin)
                                pProperty->aListValues.push_back(*pBegin);
                        }
                    }
                    break;

                    //////////////////////////////////////////////////////////////////////
                    // ControlSource
                    case PROPERTY_ID_CONTROLSOURCE:
                        SetFields( *pProperty );
                        break;

                    //////////////////////////////////////////////////////////////////////
                    // CursorSource
                    case PROPERTY_ID_COMMAND:
                        m_bHasCursorSource = sal_True;
                        break;

                    //////////////////////////////////////////////////////////////////////
                    // ListSource
                    case PROPERTY_ID_LISTSOURCE:
                        m_bHasListSource = sal_True;
                        break;
                }

                //////////////////////////////////////////////////////////////////////
                // UI-Eintrag
                bool bIsDataProperty = ( nPropertyUIFlags & PROP_FLAG_DATA_PROPERTY ) != 0;
                if ( bIsDataProperty )
                {
                    bRemoveDataPage = sal_False;
                    getPropertyBox()->SetPage( m_nDataPageId );
                }
                else
                {
                    getPropertyBox()->SetPage( m_nGenericPageId );
                }

                pProperty->nHelpId = m_pPropertyInfo->getPropertyHelpId(nPropId);
                pProperty->sTitle = sDisplayName;

                if (PropertyState_AMBIGUOUS_VALUE == eState)
                {
                    pProperty->bUnknownValue = sal_True;
                    pProperty->sValue = String();
                }

                if ( ( nControlType == CONTROL_TYPE_FORM   && ((nPropertyUIFlags & PROP_FLAG_FORM_VISIBLE) == PROP_FLAG_FORM_VISIBLE) ) ||
                     ( nControlType == CONTROL_TYPE_DIALOG && ((nPropertyUIFlags & PROP_FLAG_DIALOG_VISIBLE) == PROP_FLAG_DIALOG_VISIBLE) ) )
                {
                    getPropertyBox()->InsertEntry(*pProperty);
                }
                else
                    delete pProperty->pControl;

                delete pProperty;
            }

            SetCursorSource( sal_True, sal_True );
            SetListSource( sal_True );

            // initially update some inter-property dependencies
            for (   ::std::vector< ::rtl::OUString >::const_iterator aDependLoop = aFoundDependencies.begin();
                    aDependLoop != aFoundDependencies.end();
                    ++aDependLoop
                )
            {
                updateDependentProperties(
                    m_pPropertyInfo->getPropertyId( *aDependLoop ),
                    GetPropertyUnoValue( *aDependLoop )
                );
            }

            if (bRemoveDataPage && !m_bHasCursorSource && !m_bHasListSource)
            {
                getPropertyBox()->RemovePage(m_nDataPageId);
                m_nDataPageId=0;
            }

            getPropertyBox()->SetPage( m_nDataPageId );

            getPropertyBox()->EnableUpdate();

            if ( bHaveFocus )
                getPropertyBox()->GrabFocus();
        }
        catch (Exception&)
        {
            DBG_ERROR("OPropertyBrowserController::UpdateUI : caught an exception !")
        }
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserController::Modified( const String& aName, const String& aVal, void* pData )
    {
        try
        {
            sal_Int32 nPropId = m_pPropertyInfo->getPropertyId( aName );

            // Wenn CursorSourceType veraendert wurde, CursorSource anpassen
            if (PROPERTY_ID_COMMANDTYPE == nPropId)
            {
                Commit( aName, aVal, pData );
                SetCursorSource( sal_False, sal_False );
            }

            //////////////////////////////////////////////////////////////////////
            // Wenn ListSourceType veraendert wurde, ListSource anpassen
            if (PROPERTY_ID_LISTSOURCETYPE == nPropId)
            {
                Commit( aName, aVal, pData );
                SetListSource();
            }
        }
        catch (Exception&)
        {
            DBG_ERROR("OPropertyBrowserController::Modified : caught an exception !")
        }
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserController::OnImageURLClicked( const String& _rName, const String& _rVal, void* _pData )
    {
            ::rtl::OUString aStrTrans = m_pPropertyInfo->getPropertyTranslation( PROPERTY_ID_IMAGE_URL );

            ::sfx2::FileDialogHelper aFileDlg(SFXWB_GRAPHIC);

            aFileDlg.SetTitle(aStrTrans);

            Reference< XFilePickerControlAccess > xController(aFileDlg.GetFilePicker(), UNO_QUERY);
            DBG_ASSERT(xController.is(), "OPropertyBrowserController::Clicked: missing the controller interface on the file picker!");
            if (xController.is())
            {
                // do a preview by default
                xController->setValue(ExtendedFilePickerElementIds::CHECKBOX_PREVIEW, 0, ::cppu::bool2any(sal_True));

                // "as link" is checked, but disabled
                xController->setValue(ExtendedFilePickerElementIds::CHECKBOX_LINK, 0, ::cppu::bool2any(sal_True));
                xController->enableControl(ExtendedFilePickerElementIds::CHECKBOX_LINK, sal_False);
            }

            if (_rVal.Len() != 0)
            {
                aFileDlg.SetDisplayDirectory(_rVal);
                // TODO: need to set the display directory _and_ the default name
            }

            if (!aFileDlg.Execute())
                Commit( _rName, aFileDlg.GetPath(), _pData );
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserController::Clicked( const String& aName, const String& aVal, void* pData )
    {
        try
        {
            sal_Int32 nPropId = m_pPropertyInfo->getPropertyId(aName);

            //////////////////////////////////////////////////////////////////////
            // DataSource & ImageURL
            if (PROPERTY_ID_TARGET_URL == nPropId)
            {
                ::sfx2::FileDialogHelper aFileDlg(WB_3DLOOK);

                INetURLObject aParser( aVal );
                if ( INET_PROT_FILE == aParser.GetProtocol() )
                    // set the initial directory only for file-URLs. Everything else
                    // is considered to be potentially expensive
                    // 106126 - 2002/12/10 - fs@openoffice.org
                    aFileDlg.SetDisplayDirectory( aVal );

                if (0 == aFileDlg.Execute())
                {
                    String aDataSource = aFileDlg.GetPath();
                    Commit( aName, aDataSource, pData );
                }
            }


            //////////////////////////////////////////////////////////////////////
            // Bei Datenquelle auch Cursor-/ListSource fuellen
            else if (PROPERTY_ID_DATASOURCE == nPropId)
            {
                String aUserVal=aVal;

                Reference< XNamingService >  xDatabaseAccesses(m_xORB->createInstance(SERVICE_DATABASE_CONTEXT), UNO_QUERY);
                if (xDatabaseAccesses.is())
                {
                    Reference< XDataSource >  xDataSource;
                    try
                    {
                        xDataSource = Reference< XDataSource >(xDatabaseAccesses->getRegisteredObject(aVal), UNO_QUERY);
                    }
                    catch(NoSuchElementException&)
                    {
                        DBG_ERROR("Use of unknown datasource name");
                    }
                }
            }

            //////////////////////////////////////////////////////////////////////
            // URL
            else if (nPropId == PROPERTY_ID_IMAGE_URL)
            {
                OnImageURLClicked(aName, aVal, pData);
            }


            //////////////////////////////////////////////////////////////////////
            // Color
            else if (nPropId == PROPERTY_ID_BACKGROUNDCOLOR || nPropId == PROPERTY_ID_FILLCOLOR )
            {
                sal_uInt32 nColor = aVal.ToInt32();
                Color aColor( nColor );
                SvColorDialog aColorDlg( GetpApp()->GetAppWindow() );
                aColorDlg.SetColor( aColor );

                if (aColorDlg.Execute() )
                {
                    aColor = aColorDlg.GetColor();
                    nColor = aColor.GetColor();

                    String aColorString = String::CreateFromInt32( (sal_Int32)nColor );
                    Commit( aName, aColorString, pData );
                }
            }

            else if (PROPERTY_ID_FORMATKEY == nPropId)
            {
                ChangeFormatProperty(aName, aVal);
            }

            else if (PROPERTY_ID_CONTROLLABEL == nPropId)
            {
                OSelectLabelDialog dlgSelectLabel(GetpApp()->GetAppWindow(), m_xPropValueAccess);
                if (RET_OK == dlgSelectLabel.Execute())
                {
                    // if the properties transport would be via UsrAnys (instead of strings) we would have a chance
                    // to use the regular commit mechanism here ....
                    Reference< XPropertySet >  xSelected(dlgSelectLabel.GetSelected());
                    if (xSelected.is())
                        m_xPropValueAccess->setPropertyValue(PROPERTY_CONTROLLABEL, makeAny(xSelected));
                    else
                        m_xPropValueAccess->setPropertyValue(PROPERTY_CONTROLLABEL, Any());
                }
            }

            //////////////////////////////////////////////////////////////////////
            // Font
            else if (aName.EqualsAscii("Font"))
            {
                ChangeFontProperty(aName);
            }
            else if (pData == LINETYPE_EVENT)
            {
                ChangeEventProperty(aName);
            }
        }
        catch (Exception&)
        {
            DBG_ERROR("OPropertyBrowserController::Clicked : caught an exception !")
        }
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserController::Commit( const String& rName, const String& aVal, void* pData )
    {
        if (m_pChangeMultiplexer)
            m_pChangeMultiplexer->lock();

        try
        {
            //////////////////////////////////////////////////////////////////////
            // Property-Info holen
            Property    aProp               = getIntrospecteeProperty( rName );
            sal_Int32   nPropId             = m_pPropertyInfo->getPropertyId( rName );
            sal_uInt32  nPropertyUIFlags    = m_pPropertyInfo->getPropertyUIFlags( nPropId );
            bool bIsVirtualProperty         = ( nPropertyUIFlags & PROP_FLAG_VIRTUAL_PROP ) != 0;

            String aUserVal=aVal;

            //////////////////////////////////////////////////////////////////////
            // URL- Adresse koennte relativ sein
            if ((nPropId == PROPERTY_ID_TARGET_URL || nPropId == PROPERTY_ID_IMAGE_URL) && aVal.Len())
            {
                aUserVal = URIHelper::SmartRelToAbs(aVal);
            }

            Any aValue;
            if (!(m_sStandard.equals(aVal) &&(aProp.Attributes & PropertyAttribute::MAYBEVOID)))
            {
                aValue = getPropertyValueFromStringRep( aUserVal, aProp, nPropId);
            }

            if  (   (   (nPropId == PROPERTY_ID_DEFAULT_VALUE) || (nPropId == PROPERTY_ID_VALUE)
                    ||  (nPropId == PROPERTY_ID_DEFAULT_DATE)  || (nPropId == PROPERTY_ID_DATE)
                    ||  (nPropId == PROPERTY_ID_DEFAULT_TIME)  || (nPropId == PROPERTY_ID_TIME)
                    ||  (nPropId==PROPERTY_ID_BOUNDCOLUMN)
                    )
                &&  (0 == aVal.Len())
                )
            {
                aValue = Any();
            }

            //////////////////////////////////////////////////////////////////////
            // set the value
            if ( bIsVirtualProperty )
            {
                setVirtualPropertyValue( nPropId, aValue );
                setDocumentModified();
            }
            else
            {
                sal_Bool bDontForwardToPropSet =
                        ( ( aProp.Attributes & PropertyAttribute::MAYBEVOID ) == 0 )    // VOID is not allowed
                    &&  !aValue.hasValue();                                             // but it *is* void

                if ( PROPERTY_ID_CONTROLLABEL == nPropId )
                    // the string fo the control label is not to be set as PropertyValue, it's only for displaying
                    bDontForwardToPropSet = sal_True;

                if ( !bDontForwardToPropSet )
                    m_xPropValueAccess->setPropertyValue( rName, aValue );
            }

            // care for any inter-property dependencies
            updateDependentProperties( nPropId, aValue );

            // re-retrieve the value
            Any aNewValue;
            if ( bIsVirtualProperty )
            {
                aNewValue = getVirtualPropertyValue( nPropId );
            }
            else
            {
                aNewValue = m_xPropValueAccess->getPropertyValue( rName );
            }
            ::rtl::OUString sNewStrVal = getStringRepFromPropertyValue( aNewValue, nPropId );

            // and display it again. This ensures proper formatting
            getPropertyBox()->SetPropertyValue( rName, sNewStrVal );

            // TODO: I think all of the stuff below can be moved into updateDependentProperties
            // but I'm uncertain, and it's too risky for 1.1.1
            switch ( nPropId )
            {
            case PROPERTY_ID_TRISTATE:
            {
                ::rtl::OUString aStateName;
                sal_Int32 nStateId;
                sal_Int16 nControlType = getControlType();

                if ( nControlType == CONTROL_TYPE_FORM )
                {
                    aStateName = PROPERTY_DEFAULTCHECKED;
                    nStateId = PROPERTY_ID_DEFAULTCHECKED;
                }
                else if ( nControlType == CONTROL_TYPE_DIALOG )
                {
                    aStateName = PROPERTY_STATE;
                    nStateId = PROPERTY_ID_STATE;
                }

                OLineDescriptor aProperty;
                aProperty.sName             =   aStateName;
                aProperty.sTitle            =   m_pPropertyInfo->getPropertyTranslation(nStateId);
                aProperty.nHelpId           =   m_pPropertyInfo->getPropertyHelpId(nStateId);
                aProperty.eControlType      =   BCT_LISTBOX;
                aProperty.sValue            =   getPropertyBox()->GetPropertyValue(aStateName);
                sal_uInt16 nPos             =   getPropertyBox()->GetPropertyPos(aStateName);

                Sequence< ::rtl::OUString > aEntries =
                    m_pPropertyInfo->getPropertyEnumRepresentations(nStateId);
                sal_Int32 nEntryCount = aEntries.getLength();

                if (!::comphelper::getBOOL(aNewValue))
                    // tristate not allowed -> remove the "don't know" state
                    --nEntryCount;

                sal_Bool bValidDefaultCheckedValue = sal_False;

                const ::rtl::OUString* pStart = aEntries.getConstArray();
                const ::rtl::OUString* pEnd = pStart + nEntryCount;
                for (const ::rtl::OUString* pLoop = pStart; pLoop != pEnd; ++pLoop)
                {
                    aProperty.aListValues.push_back(*pLoop);
                    if (pLoop->equals(aProperty.sValue))
                        bValidDefaultCheckedValue = sal_True;
                }

                if (!bValidDefaultCheckedValue)
                    aProperty.sValue = *pStart;

                if (nPos != EDITOR_LIST_APPEND)
                    getPropertyBox()->ChangeEntry(aProperty,nPos);

                Commit(aProperty.sName, aProperty.sValue, NULL);
            }
            break;

            case PROPERTY_ID_DECIMAL_ACCURACY:
            case PROPERTY_ID_SHOWTHOUSANDSEP:
            {
                sal_Bool bAccuracy = (PROPERTY_ID_DECIMAL_ACCURACY == nPropId);
                sal_uInt16  nNewDigits = bAccuracy ? ::comphelper::getINT16(aNewValue) : 0;
                sal_Bool    bUseSep = bAccuracy ? sal_False : ::comphelper::getBOOL(aNewValue);

                getPropertyBox()->DisableUpdate();

                // propagate the changes to the min/max/default fields
                Any aCurrentProp;
                ::rtl::OUString aAffectedProps[] = { PROPERTY_VALUE, PROPERTY_DEFAULT_VALUE, PROPERTY_VALUEMIN, PROPERTY_VALUEMAX };
                for (sal_uInt16 i=0; i<sizeof(aAffectedProps)/sizeof(aAffectedProps[0]); ++i)
                {
                    OFormattedNumericControl* pField = (OFormattedNumericControl*)getPropertyBox()->GetPropertyControl(aAffectedProps[i]);
                    if (pField)
                        if (bAccuracy)
                            pField->SetDecimalDigits(nNewDigits);
                        else
                            pField->SetThousandsSep(bUseSep);
                }

                getPropertyBox()->EnableUpdate();
            }
            break;

            case PROPERTY_ID_FORMATKEY:
            {
                FormatDescription aNewDesc;

                Any aSupplier = m_xPropValueAccess->getPropertyValue(PROPERTY_FORMATSSUPPLIER);
                DBG_ASSERT(aSupplier.getValueType().equals(::getCppuType(
                    (const Reference< XNumberFormatsSupplier>*)0)),

                    "OPropertyBrowserController::Commit : invalid property change !");
                    // we only allowed the FormatKey property to be displayed if the set had a valid FormatsSupplier
                Reference< XNumberFormatsSupplier >  xSupplier;
                aSupplier >>= xSupplier;
                DBG_ASSERT(xSupplier.is(), "OPropertyBrowserController::Commit : invalid property change !");
                    // same argument
                Reference< XUnoTunnel > xTunnel(xSupplier,UNO_QUERY);
                DBG_ASSERT(xTunnel.is(), "OPropertyBrowserController::ChangeFormatProperty : xTunnel is invalid!");
                SvNumberFormatsSupplierObj* pSupplier = (SvNumberFormatsSupplierObj*)xTunnel->getSomething(SvNumberFormatsSupplierObj::getUnoTunnelId());
                    // the same again

                aNewDesc.pSupplier = pSupplier;
                aNewDesc.nKey = aVal.ToInt32();
                    // nKey will be zero if aVal is empty or standard

                // give each control which has to know this an own copy of the description
                IBrowserControl* pControl = getPropertyBox()->GetPropertyControl(PROPERTY_EFFECTIVE_MIN);
                if (pControl)
                    ((OFormattedNumericControl*)pControl)->SetFormatDescription(aNewDesc);

                pControl = getPropertyBox()->GetPropertyControl(PROPERTY_EFFECTIVE_MAX);
                if (pControl)
                    ((OFormattedNumericControl*)pControl)->SetFormatDescription(aNewDesc);

                pControl = getPropertyBox()->GetPropertyControl(PROPERTY_EFFECTIVE_DEFAULT);
                if (pControl)
                    ((OFormattedNumericControl*)pControl)->SetFormatDescription(aNewDesc);

                pControl = getPropertyBox()->GetPropertyControl(PROPERTY_EFFECTIVE_VALUE);
                if (pControl)
                    ((OFormattedNumericControl*)pControl)->SetFormatDescription(aNewDesc);
            }

            case PROPERTY_ID_DATASOURCE:
            {
                //////////////////////////////////////////////////////////////////////
                // Bei Datenquelle auch Cursor-/ListSource fuellen
                Property aProp = getIntrospecteeProperty( rName );
                Any aValue = getPropertyValueFromStringRep( aUserVal, aProp, nPropId);

                sal_Bool bFlag= !(aProp.Attributes & PropertyAttribute::MAYBEVOID) && !aValue.hasValue();

                if (!bFlag)
                    m_xPropValueAccess->setPropertyValue(rName, aValue );

                if (m_xPropStateAccess.is()&& !aValue.hasValue())
                {
                    m_xPropStateAccess->setPropertyToDefault(rName);
                }

                // try to open a connection for the new data source. Needed for filling the table list etc., but the methods doing this
                // don't display errors, and we want to have an error message.
                connectRowset();

                SetCursorSource( sal_False, sal_True );
                SetListSource();
            }
            break;
            }
        }
        catch(PropertyVetoException& eVetoException)
        {
            InfoBox(m_pView, eVetoException.Message).Execute();
        }
        catch(Exception&)
        {
            DBG_ERROR("OPropertyBrowserController::Commit : caught an exception !")
        }

        if (m_pChangeMultiplexer)
            m_pChangeMultiplexer->unlock();
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserController::Select( const String& aName, void* pData )
    {
    }

    //------------------------------------------------------------------------
    Any OPropertyBrowserController::getVirtualPropertyValue( sal_Int32 _nPropId )
    {
        Any aReturn;

        CellBindingHelper aHelper( m_xORB, m_xPropValueAccess );

        switch ( _nPropId )
        {
        case PROPERTY_ID_BOUND_CELL:
        {
            Reference< XValueBinding > xBinding( aHelper.getCurrentBinding() );
            if ( !aHelper.isCellBinding( xBinding ) )
                xBinding.clear();

            aReturn <<= xBinding;
        }
        break;

        case PROPERTY_ID_LIST_CELL_RANGE:
        {
            Reference< XListEntrySource > xSource( aHelper.getCurrentListSource() );
            if ( !aHelper.isCellRangeListSource( xSource ) )
                xSource.clear();

            aReturn <<= xSource;
        }
        break;

        case PROPERTY_ID_CELL_EXCHANGE_TYPE:
        {
            Reference< XValueBinding > xBinding( aHelper.getCurrentBinding() );
            aReturn <<= (sal_Int16)( aHelper.isCellIntegerBinding( xBinding ) ? 1 : 0 );
        }
        break;

        default:
            DBG_ERROR( "OPropertyBrowserController::getVirtualPropertyValue: given id does not refer to a virtual property!" );
        }

        return aReturn;
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserController::setVirtualPropertyValue( sal_Int32 _nPropId, const Any& _rValue )
    {
        CellBindingHelper aHelper( m_xORB, m_xPropValueAccess );

        switch ( _nPropId )
        {
        case PROPERTY_ID_BOUND_CELL:
        {
            Reference< XValueBinding > xBinding;
            _rValue >>= xBinding;
            aHelper.setBinding( xBinding );
        }
        break;

        case PROPERTY_ID_LIST_CELL_RANGE:
        {
            Reference< XListEntrySource > xSource;
            _rValue >>= xSource;
            aHelper.setListSource( xSource );
        }
        break;

        case PROPERTY_ID_CELL_EXCHANGE_TYPE:
        {
            Reference< XValueBinding > xBinding = aHelper.getCurrentBinding( );
            OSL_ENSURE( xBinding.is(), "OPropertyBrowserController::setVirtualPropertyValue: how this?" );
                // this property here should be disabled (see updateDependentProperties) if there's no binding
                // at our current document
            if ( !xBinding.is() )
                break;

            sal_Int16 nExchangeType = 0;
            _rValue >>= nExchangeType;

            sal_Bool bNeedIntegerBinding = ( nExchangeType == 1 );
            if ( (bool)bNeedIntegerBinding != aHelper.isCellIntegerBinding( xBinding ) )
            {
                CellAddress aAddress;
                if ( aHelper.getAddressFromCellBinding( xBinding, aAddress ) )
                {
                    xBinding = aHelper.createCellBindingFromAddress( aAddress, bNeedIntegerBinding );
                    aHelper.setBinding( xBinding );
                }
            }
        }
        break;

        default:
            OSL_ENSURE( sal_False, "OPropertyBrowserController::setVirtualPropertyValue: given id does not refer to a virtual property!" );
        }
    }

    //------------------------------------------------------------------------
    bool OPropertyBrowserController::describeVirtualProperties( Sequence< Property >& /* [out] */ _rProps )
    {
        _rProps.realloc( 0 );

        if ( CellBindingHelper::livesInSpreadsheetDocument( m_xPropValueAccess ) )
        {
            // check for properties which are related to binding controls to Calc cells
            CellBindingHelper aHelper( m_xORB, m_xPropValueAccess );
            bool bAllowCellLinking      = aHelper.isCellBindingAllowed();
            bool bAllowCellIntLinking   = aHelper.isCellIntegerBindingAllowed();
            bool bAllowListCellRange    = aHelper.isListCellRangeAllowed();
            if ( !bAllowCellLinking && !bAllowListCellRange && !bAllowCellIntLinking )
                // no more virtual properties at the moment
                return false;

            sal_Int32 nPos =  ( bAllowCellLinking    ? 1 : 0 )
                            + ( bAllowListCellRange  ? 1 : 0 )
                            + ( bAllowCellIntLinking ? 1 : 0 );
            _rProps.realloc( nPos );

            if ( bAllowCellLinking )
            {
                _rProps[ --nPos ] = Property( PROPERTY_BOUND_CELL, PROPERTY_ID_BOUND_CELL,
                    ::getCppuType( static_cast< ::rtl::OUString* >( NULL ) ), 0 );
            }
            if ( bAllowCellIntLinking )
            {
                _rProps[ --nPos ] = Property( PROPERTY_CELL_EXCHANGE_TYPE, PROPERTY_ID_CELL_EXCHANGE_TYPE,
                    ::getCppuType( static_cast< sal_Int16* >( NULL ) ), 0 );
            }
            if ( bAllowListCellRange )
            {
                _rProps[ --nPos ] = Property( PROPERTY_LIST_CELL_RANGE, PROPERTY_ID_LIST_CELL_RANGE,
                    ::getCppuType( static_cast< ::rtl::OUString* >( NULL ) ), 0 );
            }
        }

        return _rProps.getLength() != 0;
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserController::enablePropertyLines( const ::rtl::OUString* _pPropertyStart, const ::rtl::OUString* _pPropertyEnd,
        sal_Bool _bEnable )
    {
        if ( getPropertyBox() )
        {
            for (   const ::rtl::OUString* pLoop = _pPropertyStart;
                    pLoop != _pPropertyEnd;
                    ++pLoop
                )
            {
                getPropertyBox()->EnablePropertyLine( *pLoop, _bEnable );
            }
        }
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserController::updateDependentProperties( sal_Int32 _nPropId, const Any& _rNewValue )
    {
        switch ( _nPropId )
        {
        case PROPERTY_ID_BOUND_CELL:
        {
            // the SQL-data-binding related properties need to be enabled if and only if
            // there is *no* valid cell binding
            Reference< XValueBinding > xBinding;
            _rNewValue >>= xBinding;

            ::rtl::OUString aSqlBindingProperties[] =
            {
                PROPERTY_CONTROLSOURCE, PROPERTY_DATASOURCE, PROPERTY_COMMANDTYPE,
                PROPERTY_COMMAND, PROPERTY_ESCAPE_PROCESSING, PROPERTY_FILTER_CRITERIA,
                PROPERTY_SORT, PROPERTY_EMPTY_IS_NULL, PROPERTY_FILTERPROPOSAL,
            };
            enablePropertyLines(
                aSqlBindingProperties,
                aSqlBindingProperties + sizeof( aSqlBindingProperties ) / sizeof( aSqlBindingProperties[0] ),
                !xBinding.is()
            );

            // and additionally, the "transfer selection indexes" property is available
            // if and only if there's an active binding
            if ( getPropertyBox() )
                getPropertyBox()->EnablePropertyLine( PROPERTY_CELL_EXCHANGE_TYPE, xBinding.is() );

            if ( !xBinding.is() )
            {
                // ensure that the "transfer selection as" property is reset. Since we can't remember
                // it at the object itself, but derive it from the binding only, we have to normalize
                // it now that there *is* no binding anymore.
                if ( getPropertyBox() )
                {
                    getPropertyBox()->SetPropertyValue(
                        PROPERTY_CELL_EXCHANGE_TYPE,
                        getStringRepFromPropertyValue(
                            makeAny( (sal_Int16) 0 ),
                            PROPERTY_ID_CELL_EXCHANGE_TYPE
                        )
                    );
                }
            }
        }
        break;

        case PROPERTY_ID_LIST_CELL_RANGE:
        {
            // the list source related properties need to be enabled if and only if
            // there is *no* valid external list source for the control
            Reference< XListEntrySource > xSource;
            _rNewValue >>= xSource;

            ::rtl::OUString aListSourceProperties[] =
            {
                PROPERTY_STRINGITEMLIST,
                PROPERTY_BOUNDCOLUMN, PROPERTY_LISTSOURCETYPE, PROPERTY_LISTSOURCE
            };
            enablePropertyLines(
                aListSourceProperties,
                aListSourceProperties + sizeof( aListSourceProperties ) / sizeof( aListSourceProperties[0] ),
                !xSource.is()
            );
        }
        break;
        }
    }

//............................................................................
} // namespace pcr
//............................................................................

