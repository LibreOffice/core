/*************************************************************************
 *
 *  $RCSfile: formcontroller.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: fs $ $Date: 2001-04-03 12:44:06 $
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
#ifndef _COM_SUN_STAR_SDBC_XROWSET_HPP_
#include <com/sun/star/sdbc/XRowSet.hpp>
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
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
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

// for the font handling
#define ITEMID_FONT             SID_ATTR_CHAR_FONT
#define ITEMID_POSTURE          SID_ATTR_CHAR_POSTURE
#define ITEMID_WEIGHT           SID_ATTR_CHAR_WEIGHT
#define ITEMID_SHADOWED         SID_ATTR_CHAR_SHADOWED
#define ITEMID_WORDLINEMODE     SID_ATTR_CHAR_WORDLINEMODE
#define ITEMID_CONTOUR          SID_ATTR_CHAR_CONTOUR
#define ITEMID_CROSSEDOUT       SID_ATTR_CHAR_STRIKEOUT
#define ITEMID_UNDERLINE        SID_ATTR_CHAR_UNDERLINE
#define ITEMID_FONTHEIGHT       SID_ATTR_CHAR_FONTHEIGHT
#define ITEMID_PROPSIZE         SID_ATTR_CHAR_PROPSIZE
#define ITEMID_COLOR            SID_ATTR_CHAR_COLOR
#define ITEMID_KERNING          SID_ATTR_CHAR_KERNING
#define ITEMID_CASEMAP          SID_ATTR_CHAR_CASEMAP
#define ITEMID_LANGUAGE         SID_ATTR_CHAR_LANGUAGE
#define ITEMID_ESCAPEMENT       SID_ATTR_CHAR_ESCAPEMENT
#define ITEMID_FONTLIST         SID_ATTR_CHAR_FONTLIST
#define ITEMID_AUTOKERN         SID_ATTR_CHAR_AUTOKERN
#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE
#define ITEMID_BLINK            SID_ATTR_FLASH
#define ITEMID_BRUSH            SID_ATTR_BRUSH
#ifndef _SVX_FONTITEM_HXX
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX
#include <svx/postitem.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX
#include <svx/wghtitem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX
#include <svx/udlnitem.hxx>
#endif
#ifndef _SVX_CRSDITEM_HXX
#include <svx/crsditem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_FLSTITEM_HXX
#include <svx/flstitem.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX
#include <svx/langitem.hxx>
#endif
#ifndef _SVX_WRLMITEM_HXX
#include <svx/wrlmitem.hxx>
#endif
#ifndef _SV_SYSTEM_HXX
#include <vcl/system.hxx>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTWEIGHT_HPP_
#include <com/sun/star/awt/FontWeight.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTSLANT_HPP_
#include <com/sun/star/awt/FontSlant.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTUNDERLINE_HPP_
#include <com/sun/star/awt/FontUnderline.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTSTRIKEOUT_HPP_
#include <com/sun/star/awt/FontStrikeout.hpp>
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

#ifndef _IODLG_HXX
#include <sfx2/iodlg.hxx>
#endif
#ifndef _SVX_IMPGRF_HXX
#include <svx/impgrf.hxx>
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
    using namespace ::com::sun::star::container;

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
    ::rtl::OUString OPropertyBrowserController::AnyToString( const Any& rValue, const Property& _rProp, sal_Int32 _nPropId)
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

    // TODO TODO TODO
    // this is surely heavyly formdependent. Need another mechanism for converting Any->Display-String
            switch (_nPropId)
            {
                // ListTypen
                case PROPERTY_ID_ALIGN:
                case PROPERTY_ID_DATEFORMAT:
                case PROPERTY_ID_TIMEFORMAT:
                case PROPERTY_ID_BORDER:
                case PROPERTY_ID_DEFAULT_CHECKED:
                case PROPERTY_ID_COMMANDTYPE:
                case PROPERTY_ID_CYCLE:
                case PROPERTY_ID_LISTSOURCETYPE:
                case PROPERTY_ID_NAVIGATION:
                case PROPERTY_ID_BUTTONTYPE:
                case PROPERTY_ID_SUBMIT_METHOD:
                case PROPERTY_ID_SUBMIT_ENCODING:
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
                                DBG_ERROR("OPropertyBrowserController::AnyToString: could not translate an enum value");
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
            }
        }
        catch (Exception&)
        {
            DBG_ERROR("OPropertyBrowserController::AnyToString: caught an exception!")
        }

        return sReturn;
    }

    //------------------------------------------------------------------------
    Any OPropertyBrowserController::StringToAny( const ::rtl::OUString& _rString, const Property& _rProp, sal_Int32 _nPropId)
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
            if ((TypeClass_ANY == ePropertyType) && (PROPERTY_ID_EFFECTIVE_DEFAULT == _nPropId))
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
                case PROPERTY_ID_ALIGN:
                case PROPERTY_ID_DATEFORMAT:
                case PROPERTY_ID_TIMEFORMAT:
                case PROPERTY_ID_BORDER:
                case PROPERTY_ID_DEFAULT_CHECKED:
                case PROPERTY_ID_COMMANDTYPE:
                case PROPERTY_ID_CYCLE:
                case PROPERTY_ID_LISTSOURCETYPE:
                case PROPERTY_ID_NAVIGATION:
                case PROPERTY_ID_BUTTONTYPE:
                case PROPERTY_ID_SUBMIT_METHOD:
                case PROPERTY_ID_SUBMIT_ENCODING:
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
                            DBG_ERROR("OPropertyBrowserController::StringToAny: could not translate the enum string!");
                    }
                break;
            }
        }
        catch(Exception&)
        {
            DBG_ERROR("OPropertyBrowserController::StringToAny: caught an exception !")
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

                // Festellen des Feldes
                Reference< XNameAccess >  xFields;
                Reference< XPropertySet >  xField;
                try
                {
                    Reference< XConnection >   xConnection = ::dbtools::calcConnection(Reference< XRowSet > (xFormSet, UNO_QUERY),m_xORB);
                    if (!xConnection.is())
                        return;

                    switch (nObjectType)
                    {
                        case 0:
                        {
                            Reference< XTablesSupplier >  xSupplyTables(xConnection, UNO_QUERY);
                            Reference< XColumnsSupplier >  xSupplyColumns;
                            xSupplyTables->getTables()->getByName(aObjectName) >>= xSupplyColumns;
                            xFields = xSupplyColumns->getColumns();
                        }
                        break;
                        case 1:
                        {
                            Reference< XQueriesSupplier >  xSupplyQueries(xConnection, UNO_QUERY);
                            Reference< XColumnsSupplier >  xSupplyColumns;
                            xSupplyQueries->getQueries()->getByName(aObjectName) >>= xSupplyColumns;
                            xFields = xSupplyColumns->getColumns();
                        }
                        break;
                        default:
                        {
                            xStatement = xConnection->prepareStatement(aObjectName);
                            // not interested in any results
                            Reference< XPropertySet > (xStatement,UNO_QUERY)->setPropertyValue( ::rtl::OUString::createFromAscii("MaxRows"),makeAny(sal_Int32(0)));
                            Reference< XColumnsSupplier >  xSupplyCols(xStatement->executeQuery(), UNO_QUERY);
                            if (xSupplyCols.is())
                                xFields = xSupplyCols->getColumns();
                        }
                    }
                }
                catch (Exception&)
                {
                    DBG_ERROR("OPropertyBrowserController::SetFields: Exception occured!");
                }


                if (!xFields.is())
                    return;

                Sequence< ::rtl::OUString> aFields(xFields->getElementNames());
                const ::rtl::OUString* pFields = aFields.getConstArray();
                for (sal_Int32 i=0; i<aFields.getLength(); i++,++pFields )
                    rProperty.aListValues.push_back(*pFields);
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

            // get the RowSet interface of the form of the object we're inspecting
            Reference< XRowSet > xRowSet(m_xPropValueAccess, UNO_QUERY);
            if (!xRowSet.is())
            {
                // are we inspecting a combo-/listbox?
                if ((FormComponentType::COMBOBOX == m_nClassId) || (FormComponentType::LISTBOX == m_nClassId))
                    xRowSet = Reference< XRowSet >(m_xObjectParent, UNO_QUERY);

                if (!xRowSet.is())
                {
                    DBG_ERROR("OPropertyBrowserController::SetQueries: could not obtain the rowset for the introspectee!");
                    return;
                }
            }

            Reference< XTablesSupplier >  xTables;
            try
            {
                xTables = Reference< XTablesSupplier > (::dbtools::calcConnection(xRowSet,m_xORB),UNO_QUERY);
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

            // get the RowSet interface of the form of the object we're inspecting
            Reference< XRowSet > xRowSet(m_xPropValueAccess, UNO_QUERY);
            if (!xRowSet.is())
            {
                // are we inspecting a combo-/listbox?
                if ((FormComponentType::COMBOBOX == m_nClassId) || (FormComponentType::LISTBOX == m_nClassId))
                    xRowSet = Reference< XRowSet >(m_xObjectParent, UNO_QUERY);

                if (!xRowSet.is())
                {
                    DBG_ERROR("OPropertyBrowserController::SetQueries: could not obtain the rowset for the introspectee!");
                    return;
                }
            }

            Reference< XQueriesSupplier >  xSupplyQueries;
            try
            {
                xSupplyQueries = Reference< XQueriesSupplier > (::dbtools::calcConnection(xRowSet,m_xORB),UNO_QUERY);
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
    void OPropertyBrowserController::SetCursorSource(sal_Bool bInit)
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
            aProperty.eControlType = BCT_MEDIT;

            String fCommandName = PROPERTY_COMMAND;
            aProperty.sName = fCommandName;
            aProperty.sTitle = m_pPropertyInfo->getPropertyTranslation(PROPERTY_ID_COMMAND);
            aProperty.pControl = NULL;
            aProperty.bHasBrowseButton = sal_False;
            aProperty.bIsHyperlink = sal_False;
            aProperty.bIsLocked = sal_False;

            aProperty.nHelpId = m_pPropertyInfo->getPropertyHelpId(PROPERTY_ID_COMMAND);
            if (bInit)
                aProperty.sValue = sCommand;
            else
                aProperty.sValue = String();

            ////////////////////////////////////////////////////////////
            // Enums setzen
            Sequence< ::rtl::OUString > aCommandTypes = m_pPropertyInfo->getPropertyEnumRepresentations(PROPERTY_ID_COMMANDTYPE);
            sal_Int32 nPos = GetStringPos(sCommandType, aCommandTypes);
            if (0 == nPos)
                SetTables(aProperty);
            else if (1 == nPos)
                SetQueries(aProperty);

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
            String fPropName = PROPERTY_LISTSOURCE;
            aProperty.sName = fPropName;
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
    sal_Bool OPropertyBrowserController::implGetCheckFontProperty(const ::rtl::OUString& _rPropName, Any& _rValue)
    {
        _rValue = m_xPropValueAccess->getPropertyValue(_rPropName);
        if (m_xPropStateAccess.is())
            return PropertyState_DEFAULT_VALUE == m_xPropStateAccess->getPropertyState(_rPropName);

        return sal_False;
    }

    //------------------------------------------------------------------------
    ::rtl::OUString OPropertyBrowserController::implGetStringFontProperty(const ::rtl::OUString& _rPropName, const ::rtl::OUString& _rDefault)
    {
        Any aValue;
        if (implGetCheckFontProperty(_rPropName, aValue))
            return _rDefault;

        return ::comphelper::getString(aValue);
    }

    //------------------------------------------------------------------------
    sal_Int16 OPropertyBrowserController::implGetInt16FontProperty(const ::rtl::OUString& _rPropName, const sal_Int16 _nDefault)
    {
        Any aValue;
        if (implGetCheckFontProperty(_rPropName, aValue))
            return _nDefault;

        return ::comphelper::getINT16(aValue);
    }

    //------------------------------------------------------------------------
    float OPropertyBrowserController::implGetFloatFontProperty(const ::rtl::OUString& _rPropName, const float _nDefault)
    {
        Any aValue;
        if (implGetCheckFontProperty(_rPropName, aValue))
            return _nDefault;

        return ::comphelper::getFloat(aValue);
    }

    //------------------------------------------------------------------------
    static sal_uInt16 pPropFontRanges[] =
    {
        SID_ATTR_CHAR, SID_ATTR_CHAR_FONTLIST,
        0
    };

    //------------------------------------------------------------------------
    void OPropertyBrowserController::ChangeFontProperty( const ::rtl::OUString& rName )
    {
        SfxItemSet aCoreSet( SFX_APP()->GetPool(), pPropFontRanges);

        try
        {
            // some items, which may be in default state, have to be filled with non-void information
            Font aDefaultVCLFont = Application::GetDefaultDevice()->GetSettings().GetStyleSettings().GetAppFont();
            ::com::sun::star::awt::FontDescriptor aDefaultFont = VCLUnoHelper::CreateFontDescriptor(aDefaultVCLFont);

            // get the current properties
            ::rtl::OUString aFontName       = implGetStringFontProperty(PROPERTY_FONT_NAME, aDefaultFont.Name);
            ::rtl::OUString aFontStyleName  = implGetStringFontProperty(PROPERTY_FONT_STYLENAME, aDefaultFont.StyleName);
            sal_Int16   nFontFamily         = implGetInt16FontProperty(PROPERTY_FONT_FAMILY, aDefaultFont.Family);
            sal_Int16   nFontCharset        = implGetInt16FontProperty(PROPERTY_FONT_CHARSET, aDefaultFont.CharSet);
            float   nFontHeight             = implGetFloatFontProperty(PROPERTY_FONT_HEIGHT, (float)aDefaultFont.Height);
            float   nFontWeight             = implGetFloatFontProperty(PROPERTY_FONT_WEIGHT, aDefaultFont.Weight);
            sal_Int16 nFontSlant            = implGetInt16FontProperty(PROPERTY_FONT_SLANT, aDefaultFont.Slant);
            sal_Int16 nFontUnderline        = implGetInt16FontProperty(PROPERTY_FONT_UNDERLINE, aDefaultFont.Underline);
            sal_Int16 nFontStrikeout        = implGetInt16FontProperty(PROPERTY_FONT_STRIKEOUT, aDefaultFont.Strikeout);

            Any aValue;
            sal_Bool bWordLineMode          = implGetCheckFontProperty(PROPERTY_WORDLINEMODE, aValue) ? aDefaultFont.WordLineMode : ::cppu::any2bool(aValue);
            sal_Int32 nColor32 = implGetCheckFontProperty(PROPERTY_TEXTCOLOR, aValue) ? (sal_Int32)0 : ::comphelper::getINT32(aValue);

            // build SfxItems with the values
            SvxFontItem aFontItem((FontFamily)nFontFamily, aFontName, aFontStyleName, PITCH_DONTKNOW, nFontCharset, SID_ATTR_CHAR_FONT);

            nFontHeight = (float)OutputDevice::LogicToLogic(Size(0, (sal_Int32)nFontHeight), MAP_POINT, MAP_TWIP).Height();
            SvxFontHeightItem aSvxFontHeightItem((sal_uInt32)nFontHeight,100,SID_ATTR_CHAR_FONTHEIGHT);

            FontWeight      eWeight=VCLUnoHelper::ConvertFontWeight(nFontWeight);
            FontItalic      eItalic=(FontItalic)nFontSlant;
            FontUnderline   eUnderline=(FontUnderline)nFontUnderline;
            FontStrikeout   eStrikeout=(FontStrikeout)nFontStrikeout;

            SvxPostureItem      aPostureItem(eItalic,SID_ATTR_CHAR_POSTURE);
            SvxWeightItem       aWeightItem(eWeight,SID_ATTR_CHAR_WEIGHT);

            SvxUnderlineItem    aUnderlineItem(eUnderline,SID_ATTR_CHAR_UNDERLINE);
            SvxCrossedOutItem   aCrossedOutItem(eStrikeout,SID_ATTR_CHAR_STRIKEOUT );
            SvxWordLineModeItem aWordLineModeItem(bWordLineMode, SID_ATTR_CHAR_WORDLINEMODE);

            SvxColorItem aSvxColorItem(nColor32,SID_ATTR_CHAR_COLOR);

            SvxLanguageItem aLanguageItem(Application::GetAppInternational().GetLanguage(), SID_ATTR_CHAR_LANGUAGE);

            aCoreSet.Put(aFontItem, SID_ATTR_CHAR_FONT);
            aCoreSet.Put(aSvxFontHeightItem,SID_ATTR_CHAR_FONTHEIGHT);
            aCoreSet.Put(aWeightItem, SID_ATTR_CHAR_WEIGHT );
            aCoreSet.Put(aPostureItem, SID_ATTR_CHAR_POSTURE);
            aCoreSet.Put(aUnderlineItem,SID_ATTR_CHAR_UNDERLINE );
            aCoreSet.Put(aCrossedOutItem,SID_ATTR_CHAR_STRIKEOUT );
            aCoreSet.Put(aSvxColorItem, SID_ATTR_CHAR_COLOR);
            aCoreSet.Put(aLanguageItem, SID_ATTR_CHAR_LANGUAGE);
            aCoreSet.Put(aWordLineModeItem, SID_ATTR_CHAR_WORDLINEMODE);

            if (m_xPropStateAccess.is())
            {
                if (m_xPropStateAccess->getPropertyState(PROPERTY_FONT_NAME)== PropertyState_AMBIGUOUS_VALUE)
                    aCoreSet.InvalidateItem(SID_ATTR_CHAR_FONT);
                if (m_xPropStateAccess->getPropertyState(PROPERTY_FONT_HEIGHT)==PropertyState_AMBIGUOUS_VALUE)
                    aCoreSet.InvalidateItem(SID_ATTR_CHAR_FONTHEIGHT);
                if ((m_xPropStateAccess->getPropertyState(PROPERTY_FONT_WEIGHT)==PropertyState_AMBIGUOUS_VALUE) || (::com::sun::star::awt::FontWeight::DONTKNOW == nFontWeight))
                    aCoreSet.InvalidateItem(SID_ATTR_CHAR_WEIGHT );
                if ((m_xPropStateAccess->getPropertyState(PROPERTY_FONT_SLANT)==PropertyState_AMBIGUOUS_VALUE) || (::com::sun::star::awt::FontSlant_DONTKNOW == nFontSlant))
                    aCoreSet.InvalidateItem(SID_ATTR_CHAR_POSTURE);
                if ((m_xPropStateAccess->getPropertyState(PROPERTY_FONT_UNDERLINE)==PropertyState_AMBIGUOUS_VALUE) || (::com::sun::star::awt::FontUnderline::DONTKNOW == nFontUnderline))
                    aCoreSet.InvalidateItem(SID_ATTR_CHAR_UNDERLINE );
                if ((m_xPropStateAccess->getPropertyState(PROPERTY_FONT_STRIKEOUT)==PropertyState_AMBIGUOUS_VALUE) || (::com::sun::star::awt::FontStrikeout::DONTKNOW == nFontStrikeout))
                    aCoreSet.InvalidateItem(SID_ATTR_CHAR_STRIKEOUT );
                if (m_xPropStateAccess->getPropertyState(PROPERTY_TEXTCOLOR)==PropertyState_AMBIGUOUS_VALUE)
                    aCoreSet.InvalidateItem(SID_ATTR_CHAR_COLOR );
                if (m_xPropStateAccess->getPropertyState(PROPERTY_WORDLINEMODE)==PropertyState_AMBIGUOUS_VALUE)
                    aCoreSet.InvalidateItem(SID_ATTR_CHAR_WORDLINEMODE);
            }
        }
        catch (Exception&)
        {
            DBG_ERROR("OPropertyBrowserController::ChangeFontProperty : caught an exception (1) !")
            return;
        }

        aCoreSet.DisableItem(SID_ATTR_CHAR_CJK_FONT);
        aCoreSet.DisableItem(SID_ATTR_CHAR_CJK_FONTHEIGHT);
        aCoreSet.DisableItem(SID_ATTR_CHAR_CJK_LANGUAGE);
        aCoreSet.DisableItem(SID_ATTR_CHAR_CJK_POSTURE);
        aCoreSet.DisableItem(SID_ATTR_CHAR_CJK_WEIGHT);
        aCoreSet.DisableItem(SID_ATTR_CHAR_CJK_RUBY);

        aCoreSet.DisableItem(SID_ATTR_CHAR_RELIEF);
        aCoreSet.DisableItem(SID_ATTR_CHAR_WORDLINEMODE);
        aCoreSet.DisableItem(SID_ATTR_CHAR_EMPHASISMARK);
        aCoreSet.DisableItem(SID_ATTR_CHAR_CASEMAP);
        aCoreSet.DisableItem(SID_ATTR_CHAR_CONTOUR);
        aCoreSet.DisableItem(SID_ATTR_CHAR_SHADOWED);

        FontList aFontList(Application::GetDefaultDevice());
        SvxFontListItem aFontListItem( &aFontList, SID_ATTR_CHAR_FONTLIST );
        aCoreSet.Put(aFontListItem, SID_ATTR_CHAR_FONTLIST);

        ControlCharacterDialog aDlg(GetpApp()->GetAppWindow(), aCoreSet);

        try
        {
            if ( RET_OK == aDlg.Execute() )
            {
                const SfxItemSet* pOut = aDlg.GetOutputItemSet();

                if (pOut != NULL)
                {
                    SfxItemState eState = pOut->GetItemState( SID_ATTR_CHAR_FONT );

                    if ( eState == SFX_ITEM_SET )
                    {
                        const SvxFontItem&          rFontItem=
                            (const SvxFontItem&)        pOut->Get(SID_ATTR_CHAR_FONT );

                        getPropertyBox()->SetPropertyValue( String::CreateFromAscii("Font"), rFontItem.GetFamilyName());

                        m_xPropValueAccess->setPropertyValue( PROPERTY_FONT_NAME     , makeAny(::rtl::OUString(rFontItem.GetFamilyName())));
                        m_xPropValueAccess->setPropertyValue( PROPERTY_FONT_STYLENAME, makeAny(::rtl::OUString(rFontItem.GetStyleName())));
                        m_xPropValueAccess->setPropertyValue( PROPERTY_FONT_FAMILY   , makeAny((sal_Int16)rFontItem.GetFamily()));
                        m_xPropValueAccess->setPropertyValue( PROPERTY_FONT_CHARSET  , makeAny((sal_Int16)rFontItem.GetCharSet()));
                    }

                    eState = pOut->GetItemState( SID_ATTR_CHAR_FONTHEIGHT );

                    if ( eState == SFX_ITEM_SET )
                    {
                        const SvxFontHeightItem&    rSvxFontHeightItem=
                            (const SvxFontHeightItem&)  pOut->Get(SID_ATTR_CHAR_FONTHEIGHT);

                        float nHeight = (float)OutputDevice::LogicToLogic(Size(0, rSvxFontHeightItem.GetHeight()), MAP_TWIP, MAP_POINT).Height();
                        m_xPropValueAccess->setPropertyValue( PROPERTY_FONT_HEIGHT,makeAny(nHeight));

                    }

                    eState = pOut->GetItemState( SID_ATTR_CHAR_WEIGHT );

                    if ( eState == SFX_ITEM_SET )
                    {
                        const SvxWeightItem&        rWeightItem=
                            (const SvxWeightItem&)      pOut->Get(SID_ATTR_CHAR_WEIGHT );
                        float nWeight = VCLUnoHelper::ConvertFontWeight( rWeightItem.GetWeight());

                        m_xPropValueAccess->setPropertyValue( PROPERTY_FONT_WEIGHT,makeAny(nWeight));
                    }

                    eState = pOut->GetItemState( SID_ATTR_CHAR_POSTURE );

                    if ( eState == SFX_ITEM_SET )
                    {
                        const SvxPostureItem&       rPostureItem=
                            (const SvxPostureItem&)     pOut->Get(SID_ATTR_CHAR_POSTURE);
                        sal_Int16 eSlant = rPostureItem.GetPosture();
                        m_xPropValueAccess->setPropertyValue( PROPERTY_FONT_SLANT, makeAny(eSlant));
                    }

                    eState = pOut->GetItemState( SID_ATTR_CHAR_UNDERLINE );

                    if ( eState == SFX_ITEM_SET )
                    {
                        const SvxUnderlineItem&     rUnderlineItem=
                            (const SvxUnderlineItem&)   pOut->Get(SID_ATTR_CHAR_UNDERLINE );
                        sal_Int16 nUnderline = rUnderlineItem.GetUnderline();
                        m_xPropValueAccess->setPropertyValue( PROPERTY_FONT_UNDERLINE,makeAny(nUnderline));
                    }

                    eState = pOut->GetItemState( SID_ATTR_CHAR_STRIKEOUT );

                    if ( eState == SFX_ITEM_SET )
                    {
                        const SvxCrossedOutItem&    rCrossedOutItem=
                            (const SvxCrossedOutItem&)  pOut->Get(SID_ATTR_CHAR_STRIKEOUT );
                        sal_Int16 nStrikeout = rCrossedOutItem.GetStrikeout();
                        m_xPropValueAccess->setPropertyValue( PROPERTY_FONT_STRIKEOUT,makeAny(nStrikeout));
                    }


                    eState = pOut->GetItemState( SID_ATTR_CHAR_WORDLINEMODE );

                    if ( eState == SFX_ITEM_SET )
                    {
                        const SvxWordLineModeItem&  rWordLineModeItem =
                            (const SvxWordLineModeItem&)    pOut->Get(SID_ATTR_CHAR_WORDLINEMODE);
                        m_xPropValueAccess->setPropertyValue( PROPERTY_WORDLINEMODE, ::cppu::bool2any(rWordLineModeItem.GetValue()));
                    }


                    eState = pOut->GetItemState( SID_ATTR_CHAR_COLOR );

                    if ( eState == SFX_ITEM_SET )
                    {
                        const SvxColorItem&     rColorItem=
                            (const SvxColorItem&)       pOut->Get(SID_ATTR_CHAR_COLOR );
                        sal_Int32 nColor= rColorItem.GetValue().GetColor();
                        m_xPropValueAccess->setPropertyValue( PROPERTY_TEXTCOLOR,makeAny(nColor));
                    }

                }
            }
        }
        catch (Exception&)
        {
            DBG_ERROR("OPropertyBrowserController::ChangeFontProperty : caught an exception (2) !")
            return;
        }
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
                    aListenerClassName= aListener.GetToken(nTokenCount-1, '.' );
                else
                    aListenerClassName= aListener;

                if (aListenerClassName.len()>0)
                {
                    // Methoden der Listener ausgeben
                    aMethSeq = getEventMethods( *pListeners );
                    const ::rtl::OUString * pMethods = aMethSeq.getConstArray();
                    sal_uInt32 nMethCount = aMethSeq.getLength();

                    for (sal_uInt32 j = 0 ; j < nMethCount ; j++,++pMethods )
                    {

                        EventDisplayDescription* pEventDisplayDescription = GetEvtTranslation(*pMethods);

                        // be sure that the event method isn't mentioned twice
                        if (pEventDisplayDescription != NULL)
                        {
                            if (sListenerClassName == aListenerClassName && sMethodName == (*pMethods))
                            {
                                nIndex=aNameArray.size();
                            }


                            for (sal_uInt32 nI=0; nI<nScrEvts;nI++)
                            {
                                const ScriptEventDescriptor& rEvDe = pEvDes[nI];

                                if ((rEvDe.ListenerType == aListenerClassName || rEvDe.ListenerType == aOUListener )
                                    && rEvDe.EventMethod == (*pMethods))
                                {
                                    SvxMacro* pMacro=NULL;

                                    const ScriptEventDescriptor& rTheEvDe = pEvDes[nI];
                                    if (rTheEvDe.ScriptCode.len()>0 && rTheEvDe.ScriptType.len()>0)
                                    {
                                        pMacro = new SvxMacro(rTheEvDe.ScriptCode,rTheEvDe.ScriptType);
                                        aTable.Insert(aNameArray.size(),pMacro);
                                    }
                                    else
                                    {
                                        aTable.Insert(aNameArray.size(),NULL);
                                    }
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
                // formerly (before we outsources this code) here was a SetModified on the SdrModel of
                // the shell we're working with ...
                // TODO: need a replacement for this

                const SvxMacroTableDtor& aTab=pMacroTabPage->GetMacroTbl();

                if (nObjIdx>=0 && m_xEventManager.is())
                        m_xEventManager->revokeScriptEvents(nObjIdx);


                sal_uInt16 nEventCount = (sal_uInt16)aTab.Count();
                sal_uInt16 nEventIndex = 0;

                Sequence< ScriptEventDescriptor > aSeqScriptEvts(nEventCount);

                ScriptEventDescriptor *pSeqScriptEvts=aSeqScriptEvts.getArray();
                nIndex=0;

                String aListenerClassName,aName,aListener;
                //  Sequence< Reference< XIdlMethod > > aMethSeq;

                pListeners = m_aObjectListenerTypes.getConstArray();

                ::rtl::OUString aMacStr;
                for (i = 0 ; i < nLength ; i++,++pListeners )
                {
                    // Methode ansprechen
                    //  const Reference< XIdlClass > & (*pListeners) = pListeners[i];

                    // Namen besorgen
                    aListener = pListeners->getTypeName();
                    sal_Int32 nTokenCount=aListener.GetTokenCount('.');

                    if (nTokenCount>0)
                        aListenerClassName = aListener.GetToken(nTokenCount-1, '.' );
                    else
                        aListenerClassName = aListener;

                    if (aListenerClassName.Len() != 0)
                    {
                        // Methoden der Listener ausgeben
                        aMethSeq = getEventMethods( (*pListeners) );

                        const ::rtl::OUString* pMethods = aMethSeq.getConstArray();
                        sal_uInt32 nMethCount = aMethSeq.getLength();

                        for (sal_uInt32 j = 0 ; j < nMethCount ; j++,++pMethods )
                        {
                            //  Reference< XIdlMethod >  xMethod = pMethods[ j ];

                            //  aMethName=xMethod->getName();

                            EventDisplayDescription *pEventDisplayDescription=GetEvtTranslation(*pMethods);

                            if (pEventDisplayDescription != NULL)
                            {
                                (aName = aListenerClassName) += ';';

                                aName += (*pMethods).getStr();

                                SvxMacro *pMacro=aTab.Get(nIndex++);
                                if (pMacro!=NULL)
                                {
                                    aMacStr = String(pMacro->GetMacName());
                                    if (nEventIndex<nEventCount)
                                    {
                                        if( m_xEventManager.is() )
                                            pSeqScriptEvts[nEventIndex].ListenerType = aListenerClassName;
                                        else    // Dialog editor mode
                                            pSeqScriptEvts[nEventIndex].ListenerType = aListener;
                                        pSeqScriptEvts[nEventIndex].EventMethod = *pMethods;
                                        pSeqScriptEvts[nEventIndex].ScriptType = pMacro->GetLanguage();
                                        pSeqScriptEvts[nEventIndex].ScriptCode = aMacStr;
                                    }
                                    nEventIndex++;
                                }
                                else
                                    aMacStr = ::rtl::OUString();
                                getPropertyBox()->SetPropertyValue( aName, aMacStr);

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
                    for( sal_Int32 i = nLen - 1; i >= 0 ; i-- )
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

            InsertEvents();
            sal_uInt32 nPropCount = m_aObjectProperties.getLength();
            const Property* pProps = m_aObjectProperties.getConstArray();
            OLineDescriptor* pProperty = NULL;
            sal_Bool bRemoveDatPage=sal_True;

            TypeClass eType;
            Any aVal,aSupplier,aKey,aDigits,aSeparator,aDefault;
            ::rtl::OUString aStrVal;
            PropertyState eState;

            // check, if this is a dialog control
            sal_Bool bIsDialogControl = sal_False;
            Reference< XPropertySetInfo >  xPropInfo = m_xIntrospecteeAsProperty->getPropertySetInfo();
            if ( xPropInfo->hasPropertyByName( PROPERTY_WIDTH ) &&
                 xPropInfo->hasPropertyByName( PROPERTY_HEIGHT ) &&
                 xPropInfo->hasPropertyByName( PROPERTY_POSITIONX ) &&
                 xPropInfo->hasPropertyByName( PROPERTY_POSITIONY ) )
                bIsDialogControl = sal_True;

            for (sal_uInt32 i=0; i<nPropCount; ++i, ++pProps)
            {
                sal_Int32 nPropId = m_pPropertyInfo->getPropertyId(pProps->Name);
                String sDisplayName = m_pPropertyInfo->getPropertyTranslation(nPropId);
                if (!sDisplayName.Len())
                    continue;

                pProperty = new OLineDescriptor();


                //////////////////////////////////////////////////////////////////////
                // TypeClass des Property ermitteln
                eType = pProps->Type.getTypeClass();

                //////////////////////////////////////////////////////////////////////
                // Wert holen und in ::rtl::OUString wandeln
                eState=PropertyState_DIRECT_VALUE;
                if (m_xPropStateAccess.is())
                    eState=m_xPropStateAccess->getPropertyState(pProps->Name);

                aVal = m_xPropValueAccess->getPropertyValue( pProps->Name );
                aStrVal = AnyToString(aVal, *pProps, nPropId);

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
                    pProperty->bHasDefaultValue =sal_True;
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
                else if (PROPERTY_ID_CONTROLLABEL == nPropId)
                {
                    bFilter = sal_False;
                    pProperty->bHasBrowseButton = sal_True;
                    pProperty->bIsLocked = sal_True;
                    pProperty->sValue = AnyToString(aVal, *pProps, PROPERTY_ID_CONTROLLABEL);
                    pProperty->nUniqueButtonId = UID_PROP_DLG_CONTROLLABEL;
                }
                else if ((PROPERTY_ID_FORMATKEY == nPropId) || (PROPERTY_ID_EFFECTIVE_MIN == nPropId)
                    || (PROPERTY_ID_EFFECTIVE_MAX == nPropId) || (PROPERTY_ID_EFFECTIVE_DEFAULT == nPropId))
                {
                    // only if the set has a formatssupplier, too
                    if (::comphelper::hasProperty(PROPERTY_FORMATSSUPPLIER, m_xPropValueAccess))
                    {
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
                    if (nPropId==PROPERTY_ID_DATEMIN || nPropId==PROPERTY_ID_DATEMAX || nPropId==PROPERTY_ID_DEFAULT_DATE)
                        pProperty->eControlType = BCT_DATEFIELD;
                    else if (nPropId==PROPERTY_ID_TIMEMIN || nPropId==PROPERTY_ID_TIMEMAX|| nPropId==PROPERTY_ID_DEFAULT_TIME)
                        pProperty->eControlType = BCT_TIMEFIELD;
                    else
                    {
                        if (nPropId== PROPERTY_ID_VALUEMIN || nPropId== PROPERTY_ID_VALUEMAX || nPropId==PROPERTY_ID_DEFAULT_VALUE)
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

                            // and allow empty values only for the default value
                            ((OFormattedNumericControl*)pProperty->pControl)->EnableEmptyField(PROPERTY_ID_DEFAULT_VALUE == nPropId);
                        }
                        else
                        {
                            if ( (nPropId== PROPERTY_ID_HEIGHT || nPropId== PROPERTY_ID_WIDTH || nPropId== PROPERTY_ID_ROWHEIGHT)
                                && !bIsDialogControl )
                                pProperty->nDigits=1;

                            pProperty->eControlType = BCT_NUMFIELD;
                        }
                    }
                }


                if (bIsDialogControl)
                    bFilter = sal_False;    // don't filter dialog controls

                //////////////////////////////////////////////////////////////////////
                // Filter
                if (bFilter)
                {
                    switch( eType )     // TypeClass Inspection
                    {
                    case TypeClass_INTERFACE:
                    case TypeClass_ARRAY:
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
                        delete pProperty;
                        continue;
                    }

                    if (pProps->Name.compareTo(::rtl::OUString::createFromAscii("type unknown")) == COMPARE_EQUAL )
                    {
                        delete pProperty;
                        continue;
                    }

                    if (pProps->Attributes & PropertyAttribute::TRANSIENT )
                    {
                        delete pProperty;
                        continue;
                    }

                    else if (pProps->Attributes & PropertyAttribute::READONLY )
                    {
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
                    case PROPERTY_ID_SUBMIT_METHOD:
                    case PROPERTY_ID_SUBMIT_ENCODING:
                    case PROPERTY_ID_DATEFORMAT:
                    case PROPERTY_ID_TIMEFORMAT:
                    case PROPERTY_ID_BORDER:
                    case PROPERTY_ID_CYCLE:
                    case PROPERTY_ID_NAVIGATION:
                    case PROPERTY_ID_TARGET_FRAME:
                    case PROPERTY_ID_DEFAULT_CHECKED:
                    case PROPERTY_ID_LISTSOURCETYPE:
                    {
                        Sequence< ::rtl::OUString > aEnumValues = m_pPropertyInfo->getPropertyEnumRepresentations(nPropId);
                        const ::rtl::OUString* pStart = aEnumValues.getConstArray();
                        const ::rtl::OUString* pEnd = pStart + aEnumValues.getLength();

                        // for a checkbox: if "ambiguous" is not allowed, remove this from the sequence
                        if (PROPERTY_ID_DEFAULT_CHECKED == nPropId)
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
                }

                //////////////////////////////////////////////////////////////////////
                // DataSource
                if (nPropId == PROPERTY_ID_DATASOURCE )
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

                //////////////////////////////////////////////////////////////////////
                // ControlSource
                else if (nPropId == PROPERTY_ID_CONTROLSOURCE )
                    SetFields( *pProperty );

                //////////////////////////////////////////////////////////////////////
                // CursorSource
                else if (nPropId == PROPERTY_ID_COMMAND)
                    m_bHasCursorSource = sal_True;

                //////////////////////////////////////////////////////////////////////
                // ListSource
                else if (nPropId == PROPERTY_ID_LISTSOURCE )
                    m_bHasListSource = sal_True;

                //////////////////////////////////////////////////////////////////////
                // UI-Eintrag
                switch( nPropId )       // DataPage
                {
                    case PROPERTY_ID_COMMAND:
                    case PROPERTY_ID_CONTROLSOURCE:
                    case PROPERTY_ID_LISTSOURCE:
                    case PROPERTY_ID_LISTSOURCETYPE:
                    case PROPERTY_ID_BOUNDCOLUMN:
                    case PROPERTY_ID_MASTERFIELDS:
                    case PROPERTY_ID_DETAILFIELDS:
                    case PROPERTY_ID_DATASOURCE:
                    case PROPERTY_ID_COMMANDTYPE:
                    case PROPERTY_ID_INSERTONLY:
                    case PROPERTY_ID_NAVIGATION:
                    case PROPERTY_ID_CYCLE:
                    case PROPERTY_ID_ALLOWADDITIONS:
                    case PROPERTY_ID_ALLOWEDITS:
                    case PROPERTY_ID_ALLOWDELETIONS:
                    case PROPERTY_ID_ESCAPE_PROCESSING:
                    case PROPERTY_ID_FILTER_CRITERIA:
                    case PROPERTY_ID_SORT:
                    case PROPERTY_ID_EMPTY_IS_NULL:
                    case PROPERTY_ID_FILTERPROPOSAL:
                        bRemoveDatPage =sal_False;
                        getPropertyBox()->SetPage( m_nDataPageId );
                        break;
                    default:
                        getPropertyBox()->SetPage( m_nGenericPageId );
                }

                pProperty->nHelpId = m_pPropertyInfo->getPropertyHelpId(nPropId);
                pProperty->sTitle = sDisplayName;

                if (PropertyState_AMBIGUOUS_VALUE == eState)
                {
                    pProperty->bUnknownValue = sal_True;
                    pProperty->sValue = String();
                }
                getPropertyBox()->InsertEntry(*pProperty);
                delete pProperty;
            }

            SetCursorSource(sal_True);
            SetListSource(sal_True);

            if (bRemoveDatPage && !m_bHasCursorSource && !m_bHasListSource)
            {
                getPropertyBox()->RemovePage(m_nDataPageId);
                m_nDataPageId=0;
            }

            getPropertyBox()->SetPage( m_nDataPageId );

            getPropertyBox()->EnableUpdate();
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
                SetCursorSource();
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
    void OPropertyBrowserController::Clicked( const String& aName, const String& aVal, void* pData )
    {
        try
        {
            sal_Int32 nPropId = m_pPropertyInfo->getPropertyId(aName);

            //////////////////////////////////////////////////////////////////////
            // DataSource & ImageURL
            if (PROPERTY_ID_TARGET_URL == nPropId)
            {

                ::rtl::OUString aStrTrans = m_pPropertyInfo->getPropertyTranslation( nPropId );

                SfxFileDialog* pDlg = new SfxFileDialog(
                             GetpApp()->GetAppWindow(), WB_3DLOOK );

                pDlg->SetPath( aVal );
                if (pDlg->Execute() == RET_OK )
                {
                    String aDataSource = pDlg->GetPath();
                    Commit( aName, aDataSource, pData );

                }
                delete pDlg;
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

                ::rtl::OUString aStrTrans = m_pPropertyInfo->getPropertyTranslation( nPropId );

                SvxImportGraphicDialog* pDlg = new SvxImportGraphicDialog(
                                               NULL,aStrTrans,
                                               ENABLE_STANDARD );

                if (aVal.Len() != 0)
                    pDlg ->SetPath( aVal,sal_False);


                if (pDlg->Execute() == RET_OK )
                {
                    String aDataSource = pDlg->GetPath();
                    Commit( aName, aDataSource, pData );
                }
                delete pDlg;
            }


            //////////////////////////////////////////////////////////////////////
            // Color
            else if (nPropId == PROPERTY_ID_BACKGROUNDCOLOR )
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
            sal_Int32 nPropId = m_pPropertyInfo->getPropertyId( rName );

            Property aProp = getIntrospecteeProperty( rName );

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
                aValue = StringToAny( aUserVal, aProp, nPropId);
            }

            sal_Bool bDontSetToDefault = sal_False;

            if  (   (   (nPropId == PROPERTY_ID_DEFAULT_VALUE)
                    ||  (nPropId == PROPERTY_ID_DEFAULT_DATE)
                    ||  (nPropId == PROPERTY_ID_DEFAULT_TIME)
                    ||  (nPropId==PROPERTY_ID_BOUNDCOLUMN)
                    )
                &&  (0 == aVal.Len())
                )
            {
                aValue = Any();
                bDontSetToDefault = sal_True;
            }

            //////////////////////////////////////////////////////////////////////
            // Wert setzen
            sal_Bool bDontForwardToPropSet = !(aProp.Attributes & PropertyAttribute::MAYBEVOID) &&
                        aValue.getValueType().equals( ::getVoidCppuType());


            if (PROPERTY_ID_CONTROLLABEL == nPropId)
            {
                bDontForwardToPropSet = sal_True;
                // the string fo the control label is not to be set as PropertyValue, it's only for displaying
                bDontSetToDefault = sal_True;
            }

            if (!bDontForwardToPropSet)
                m_xPropValueAccess->setPropertyValue( rName, aValue );

            if (m_xPropStateAccess.is() && aValue.getValueType().equals( ::getVoidCppuType()) && !bDontSetToDefault)
            {
                m_xPropStateAccess->setPropertyToDefault(rName);
            }
            //////////////////////////////////////////////////////////////////////
            // Wert neu holen und ggf. neu setzen
            Any aNewValue = m_xPropValueAccess->getPropertyValue(rName);
            ::rtl::OUString aNewStrVal = AnyToString(aNewValue, aProp, nPropId);

            getPropertyBox()->SetPropertyValue( rName, aNewStrVal );

            if (nPropId==PROPERTY_ID_TRISTATE)
            {
                OLineDescriptor aProperty;
                String fPropName = PROPERTY_DEFAULTCHECKED;
                aProperty.sName             = fPropName;
                aProperty.sTitle            =   m_pPropertyInfo->getPropertyTranslation(PROPERTY_ID_DEFAULT_CHECKED);
                aProperty.nHelpId           =   m_pPropertyInfo->getPropertyHelpId(PROPERTY_ID_DEFAULT_CHECKED);
                aProperty.eControlType      =   BCT_LISTBOX;
                aProperty.sValue            =   getPropertyBox()->GetPropertyValue(PROPERTY_DEFAULTCHECKED);
                sal_uInt16 nPos             =   getPropertyBox()->GetPropertyPos(PROPERTY_DEFAULTCHECKED);

                Sequence< ::rtl::OUString > aEntries =
                    m_pPropertyInfo->getPropertyEnumRepresentations(PROPERTY_ID_DEFAULT_CHECKED);
                sal_Int32 nEntryCount = aEntries.getLength();

                if (!::comphelper::getBOOL(aNewValue))
                    // tristate not allowed -> remove the "don't know" state
                    --nEntryCount;

                sal_Bool bValidDefaultCheckedValue = sal_False;

                const ::rtl::OUString* pStart = aEntries.getConstArray();
                const ::rtl::OUString* pEnd = pStart + aEntries.getLength();
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
            else if ((PROPERTY_ID_DECIMAL_ACCURACY == nPropId) || (PROPERTY_ID_SHOWTHOUSANDSEP == nPropId))
            {
                sal_Bool bAccuracy = (PROPERTY_ID_DECIMAL_ACCURACY == nPropId);
                sal_uInt16  nNewDigits = bAccuracy ? ::comphelper::getINT16(aNewValue) : 0;
                sal_Bool    bUseSep = bAccuracy ? sal_False : ::comphelper::getBOOL(aNewValue);

                getPropertyBox()->DisableUpdate();

                // propagate the changes to the min/max/default fields
                Any aCurrentProp;
                ::rtl::OUString aAffectedProps[] = { PROPERTY_DEFAULT_VALUE, PROPERTY_VALUEMIN, PROPERTY_VALUEMAX };
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
            else if (PROPERTY_ID_FORMATKEY == nPropId)
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
            }

                //////////////////////////////////////////////////////////////////////
            // Bei Datenquelle auch Cursor-/ListSource fuellen
            if (nPropId == PROPERTY_ID_DATASOURCE )
            {
                Property aProp = getIntrospecteeProperty( rName );

                Any aValue = StringToAny( aUserVal, aProp, nPropId);

                sal_Bool bFlag= !(aProp.Attributes & PropertyAttribute::MAYBEVOID) && !aValue.hasValue();


                if (!bFlag)
                    m_xPropValueAccess->setPropertyValue(rName, aValue );

                if (m_xPropStateAccess.is()&& !aValue.hasValue())
                {
                    m_xPropStateAccess->setPropertyToDefault(rName);
                }

                SetCursorSource();
                SetListSource();
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

//............................................................................
} // namespace pcr
//............................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.13  2001/03/21 15:42:13  fs
 *  #82696# use the new font dialog for changing the control font
 *
 *  Revision 1.12  2001/03/15 09:02:28  fs
 *  cppuhelper/extract -> comphelper/extract
 *
 *  Revision 1.11  2001/03/03 14:23:49  ab
 *  Integrated access to XScriptsEventSupplier for new UNO dialogs
 *
 *  Revision 1.10  2001/02/22 09:31:29  tbe
 *  added properties for dialog controls
 *
 *  Revision 1.9  2001/02/20 08:49:56  fs
 *  #84111# allow number formats to be removed
 *
 *  Revision 1.8  2001/02/13 16:27:08  fs
 *  #83848# no multiline input for 'DefaultText' of FileControl / #83655# -1 as minimum for MaxTextLength
 *
 *  Revision 1.7  2001/02/06 10:21:19  fs
 *  #83527# set the minimum for the BoundField property to 1
 *
 *  Revision 1.6  2001/02/05 14:38:26  fs
 *  #83461# no 'not defined' for radio buttons check state
 *
 *  Revision 1.5  2001/02/05 08:59:02  fs
 *  #83468# SetTables ... correctly retrieve the rowset for combo-/listboxes
 *
 *  Revision 1.4  2001/01/18 14:45:10  rt
 *  #65293# semicolon removed
 *
 *  Revision 1.3  2001/01/12 17:02:53  fs
 *  StringToAny: corrected the evaluation of booleans
 *
 *  Revision 1.2  2001/01/12 14:44:49  fs
 *  don't hold the form info service statically - caused problems 'cause it was the last ModuleResourceClient and destroyed upon unloaded the library
 *
 *  Revision 1.1  2001/01/12 11:28:05  fs
 *  initial checkin - outsourced the form property browser
 *
 *
 *  Revision 1.0 10.01.01 08:51:55  fs
 ************************************************************************/

