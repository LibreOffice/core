/*************************************************************************
 *
 *  $RCSfile: formcontroller.cxx,v $
 *
 *  $Revision: 1.72 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-30 16:47:33 $
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
#ifndef __EXTENSIONS_INC_EXTENSIO_HRC__
#include "extensio.hrc"
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
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
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
#ifndef _COM_SUN_STAR_UI_DIALOGS_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XFILEPICKER_HPP_
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTOPWINDOW_HPP_
#include <com/sun/star/awt/XTopWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTDESCRIPTOR_HPP_
#include <com/sun/star/awt/FontDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROLMODEL_HPP_
#include <com/sun/star/awt/XControlModel.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
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
#ifndef _COM_SUN_STAR_FORM_FORMSUBMITENCODING_HPP_
#include <com/sun/star/form/FormSubmitEncoding.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERIESSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
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
#ifndef _COM_SUN_STAR_FORM_BINDING_XBINDABLEVALUE_HPP_
#include <com/sun/star/form/binding/XBindableValue.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLADDRESS_HPP_
#include <com/sun/star/table/CellAddress.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMBUTTONTYPE_HPP_
#include <com/sun/star/form/FormButtonType.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTABCONTROLLER_HPP_
#include <com/sun/star/awt/XTabControllerModel.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_VISUALEFFECT_HPP_
#include <com/sun/star/awt/VisualEffect.hpp>
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
#ifndef _SVTOOLS_LOCALRESACCESS_HXX_
#include <svtools/localresaccess.hxx>
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
#ifndef _VCL_STDTEXT_HXX
#include <vcl/stdtext.hxx>
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
#ifndef _EXTENSIONS_FORMCTRLR_PROPRESID_HRC_
#include "formresid.hrc"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_TABORDER_HXX
#include "taborder.hxx"
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
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif

#ifndef _MACROPG_HXX
#include <sfx2/macropg.hxx>
#endif
#ifndef _SFXMACITEM_HXX
#include <svtools/macitem.hxx>
#endif
#define LINETYPE_EVENT  reinterpret_cast<void*>(0xFFFFFFFF)

#ifndef _EXTENSIONS_FORMCTRLR_FORMHELPID_HRC_
#include "formhelpid.hrc"
#endif
#ifndef __EXTENSIONS_INC_EXTENSIO_HRC__
#include "extensio.hrc"
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
#ifndef _UNOTOOLS_CONFIGNODE_HXX_
#include <unotools/confignode.hxx>
#endif

#ifndef EXTENSIONS_PROPCTRLR_CELLBINDINGHELPER_HXX
#include "cellbindinghelper.hxx"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_CONTROLTYPE_HXX
#include "controltype.hxx"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_UNOURL_HXX
#include "unourl.hxx"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_FORMLINKDIALOG_HXX
#include "formlinkdialog.hxx"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_LISTSELECTIONDLG_HXX
#include "listselectiondlg.hxx"
#endif

#ifndef EXTENSIONS_PROPCTRLR_PUSHBUTTONNAVIGATION_HXX
#include "pushbuttonnavigation.hxx"
#endif

#define TEXTTYPE_SINGLELINE     0
#define TEXTTYPE_MULTILINE      1
#define TEXTTYPE_RICHTEXT       2

#include <svx/svxdlg.hxx> //CHINA001
#include <svx/dialogs.hrc> //CHINA001

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEREPLACE_HPP_
#include <com/sun/star/container/XNameReplace.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#include <svx/svxdlg.hxx>
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/implbase1.hxx>
#include <list>

typedef ::std::list< ::std::pair< ::rtl::OUString, ::rtl::OUString > > EventList;

//............................................................................
namespace pcr
{
//............................................................................

    namespace css = ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::script;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::ui::dialogs;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::table;
    using namespace ::com::sun::star::form::binding;
    using namespace ::dbtools;

//
// XNameReplace impl for transition to new SvxMacroAssignDlg
class EventsNameReplace_Impl:
        public ::cppu::WeakImplHelper1 < ::com::sun::star::container::XNameReplace >
{
    public:
        EventsNameReplace_Impl( );
        ~EventsNameReplace_Impl( );

        void AddEvent( ::rtl::OUString sEventName, ::rtl::OUString sMacroURL );

        //XNameReplace
        void SAL_CALL replaceByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw (::com::sun::star::uno::RuntimeException);
        ::sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw (::com::sun::star::uno::RuntimeException);
        ::sal_Bool SAL_CALL hasElements(  ) throw (::com::sun::star::uno::RuntimeException);


    private:
        EventList m_eventList;
};

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
            return lhs.nMinValue < rhs.nMinValue;
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
    void OPropertyBrowserController::fakePropertyValue( Any& _rValue, sal_Int32 _nPropId )
    {
        switch ( _nPropId )
        {
        case PROPERTY_ID_TABSTOP:
            if ( !_rValue.hasValue() )
            {
                switch ( m_nClassId )
                {
                case FormComponentType::COMMANDBUTTON:
                case FormComponentType::RADIOBUTTON:
                case FormComponentType::CHECKBOX:
                case FormComponentType::TEXTFIELD:
                case FormComponentType::LISTBOX:
                case FormComponentType::COMBOBOX:
                case FormComponentType::FILECONTROL:
                case FormComponentType::DATEFIELD:
                case FormComponentType::TIMEFIELD:
                case FormComponentType::NUMERICFIELD:
                case ControlType::FORMATTEDFIELD:
                case FormComponentType::CURRENCYFIELD:
                case FormComponentType::PATTERNFIELD:
                    _rValue = makeAny( (sal_Bool)sal_True );
                    break;
                default:
                    _rValue = makeAny( (sal_Bool)sal_False );
                    break;
                }
            }
            break;
        }
    }

    //------------------------------------------------------------------------
    ::rtl::OUString OPropertyBrowserController::getStringRepFromPropertyValue( const Any& rValue, sal_Int32 _nPropId)
    {
        ::rtl::OUString sReturn;
        if ( !rValue.hasValue() )
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
                    String aEntries;
                    if (  _nPropId == PROPERTY_ID_SHOW_POSITION
                       || _nPropId == PROPERTY_ID_SHOW_NAVIGATION
                       || _nPropId == PROPERTY_ID_SHOW_RECORDACTIONS
                       || _nPropId == PROPERTY_ID_SHOW_FILTERSORT
                       )
                       aEntries = String( ModuleRes( RID_STR_SHOW_HIDE ) );
                    else
                       aEntries = String( ModuleRes( RID_STR_BOOL ) );

                    sReturn = ::comphelper::getBOOL( rValue ) ? aEntries.GetToken( 1 ) : aEntries.GetToken( 0 );
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

            // enum properties
            if ( m_pPropertyInfo )
            {
                sal_uInt32  nPropertyUIFlags = m_pPropertyInfo->getPropertyUIFlags( _nPropId );
                if ( ( nPropertyUIFlags & PROP_FLAG_ENUM ) != 0 )
                {
                    sal_Int32 nIntValue = -1;
                    if ( ::cppu::enum2int( nIntValue, rValue ) )
                    {
                        if ( ( nPropertyUIFlags & PROP_FLAG_ENUM_ONE ) == PROP_FLAG_ENUM_ONE )
                            // we do not have a UI corresponding to the value "0"
                            --nIntValue;

                        ::std::vector< String > aEnumStrings = m_pPropertyInfo->getPropertyEnumRepresentations( _nPropId );
                        if ( ( nIntValue >= 0 ) && ( nIntValue < (sal_Int32)aEnumStrings.size() ) )
                        {
                            sReturn = aEnumStrings[ nIntValue ];
                        }
                        else
                            DBG_ERROR("OPropertyBrowserController::getStringRepFromPropertyValue: could not translate an enum value");
                    }
                }
            }

            switch (_nPropId)
            {
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
                    CellBindingHelper aHelper( m_xPropValueAccess );
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
                    CellBindingHelper aHelper( m_xPropValueAccess );
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
                    String sBooleanValues;
                    if (  _nPropId == PROPERTY_ID_SHOW_POSITION
                       || _nPropId == PROPERTY_ID_SHOW_NAVIGATION
                       || _nPropId == PROPERTY_ID_SHOW_RECORDACTIONS
                       || _nPropId == PROPERTY_ID_SHOW_FILTERSORT
                       )
                       sBooleanValues = String( ModuleRes( RID_STR_SHOW_HIDE ) );
                    else
                       sBooleanValues = String( ModuleRes( RID_STR_BOOL ) );

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

            // enum properties
            if ( m_pPropertyInfo )
            {
                sal_uInt32  nPropertyUIFlags = m_pPropertyInfo->getPropertyUIFlags( _nPropId );
                if ( ( nPropertyUIFlags & PROP_FLAG_ENUM ) != 0 )
                {
                    ::std::vector< String > aEnumStrings = m_pPropertyInfo->getPropertyEnumRepresentations( _nPropId );
                    sal_Int32 nPos = GetStringPos( _rString, aEnumStrings );
                    if ( -1 != nPos )
                    {
                        if ( ( nPropertyUIFlags & PROP_FLAG_ENUM_ONE ) == PROP_FLAG_ENUM_ONE )
                            // for the LineEndFormat, we do not have a UI corresponding to the value "0"
                            ++nPos;

                        switch ( aPropertyType.getTypeClass() )
                        {
                            case TypeClass_ENUM:
                                aReturn = ::cppu::int2enum( nPos, aPropertyType );
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
            }

            switch( _nPropId )
            {
                case PROPERTY_ID_LIST_CELL_RANGE:
                {
                    CellBindingHelper aHelper( m_xPropValueAccess );
                    aReturn = makeAny( aHelper.createCellListSourceFromStringAddress( _rString ) );
                }
                break;

                case PROPERTY_ID_BOUND_CELL:
                {
                    CellBindingHelper aHelper( m_xPropValueAccess );

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
        Reference< ::com::sun::star::util::XNumberFormatsSupplier >  xSupplier;
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
        SfxSingleTabDialog* pDlg = new SfxSingleTabDialog( getDialogParent(), aCoreSet, 0 );
        //CHINA001 SvxNumberFormatTabPage* pPage = (SvxNumberFormatTabPage*) SvxNumberFormatTabPage::Create(pDlg, aCoreSet);
        //add by CHINA001 begin
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create(); //CHINA001
        DBG_ASSERT(pFact, "CreateFactory fail!");
        ::CreateTabPage fnCreatePage  = pFact->GetTabPageCreatorFunc(RID_SVXPAGE_NUMBERFORMAT);
        DBG_ASSERT(fnCreatePage, "Dialogdiet error!");//CHINA001
        SfxTabPage* pPage = (*fnCreatePage)( pDlg, aCoreSet );
        // end by CHINA001
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
        DBG_ASSERT( xRowSet.is(), "OPropertyBrowserController::getRowSet: could not obtain the rowset for the introspectee!" );
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
            // Setzen der UI-Daten
            OLineDescriptor aProperty;
            aProperty.eControlType = BCT_COMBOBOX;

            aProperty.sName = (const ::rtl::OUString&)PROPERTY_COMMAND;
            aProperty.sTitle = m_pPropertyInfo->getPropertyTranslation(PROPERTY_ID_COMMAND);
            aProperty.pControl = NULL;
            aProperty.bIsLocked = sal_False;
            aProperty.nHelpId = m_pPropertyInfo->getPropertyHelpId( PROPERTY_ID_COMMAND );
            aProperty.nUniqueButtonId = UID_PROP_DLG_SQLCOMMAND;
            if ( _bInit )
                aProperty.sValue = GetPropertyValue(PROPERTY_COMMAND);

            if ( _bConnect )
                connectRowset();

            ////////////////////////////////////////////////////////////
            // Enums setzen

            sal_Bool bFailedToConnect = _bConnect && !haveRowsetConnection();
            if ( !bFailedToConnect )
            {
                sal_Int32 nCommandType = CommandType::COMMAND;
                GetUnoPropertyValue( PROPERTY_COMMANDTYPE ) >>= nCommandType;

                switch ( nCommandType )
                {
                case CommandType::TABLE:
                    SetTables( aProperty );
                    break;

                case CommandType::QUERY:
                    SetQueries( aProperty );
                    break;

                default:
                    aProperty.eControlType = BCT_MEDIT;
                    break;
                }
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
    void OPropertyBrowserController::ChangeFontProperty( )
    {
        // create an item set for use with the dialog
        SfxItemSet* pSet = NULL;
        SfxItemPool* pPool = NULL;
        SfxPoolItem** pDefaults = NULL;
        ControlCharacterDialog::createItemSet(pSet, pPool, pDefaults);
        ControlCharacterDialog::translatePropertiesToItems(m_xPropValueAccess, pSet);

        {   // do this in an own block. The dialog needs to be destroyed before we call
            // destroyItemSet
            ControlCharacterDialog aDlg( getDialogParent(), *pSet );
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

        OSL_TRACE("ChangeEventProp with arg = %s",::rtl::OUStringToOString(_Name, RTL_TEXTENCODING_ASCII_US ).pData->buffer);
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
                        OSL_TRACE("whats this? = %s",::rtl::OUStringToOString(pNames[i], RTL_TEXTENCODING_ASCII_US ).pData->buffer);
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

            Reference< css::container::XNameReplace > xNameReplace( new EventsNameReplace_Impl );

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
                        ::rtl::OUString sEventName = *pMethods;
                        ::rtl::OUString sMacroURL;

                        EventDisplayDescription* pEventDisplayDescription = GetEvtTranslation(*pMethods);
                        // pMethod now contains the event name!!
                        //nope contains keyReleased != on-keyup
                        OSL_TRACE("method is = %s",::rtl::OUStringToOString(*pMethods, RTL_TEXTENCODING_ASCII_US ).pData->buffer);

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
                                        sMacroURL = sScriptCode;
                                    }

                                    aTable.Insert(aNameArray.size(), pMacro);
                                }
                            }

                            aNameArray.push_back(pEventDisplayDescription->sDisplayName);
                            EventsNameReplace_Impl* pENR = static_cast< EventsNameReplace_Impl* >(xNameReplace.get());
                            pENR->AddEvent(sEventName,sMacroURL);
                        }
                    }
                }
            }

            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            if( !pFact )
            {
                return;
            }
            SfxItemSet rSet( SFX_APP()->GetPool(), SID_ATTR_MACROITEM, SID_ATTR_MACROITEM );
            VclAbstractDialog* pDlg = pFact->CreateSvxMacroAssignDlg( getDialogParent(), rSet, xNameReplace, nIndex );
                //VclAbstractDialog* pDlg = pFact->CreateSvxMacroAssignDlg( (Window*)0, rSet, xNameReplace );
            if( !pDlg )
            {
                return;
            }

            /*SvxMacroItem aMacroItem;

            aMacroItem.SetMacroTable(aTable);

            SfxItemSet aSet( SFX_APP()->GetPool(), SID_ATTR_MACROITEM, SID_ATTR_MACROITEM );
            aSet.Put(aMacroItem, SID_ATTR_MACROITEM);
            pMacroDlg = new SfxMacroAssignDlg( getDialogParent(), aSet );
            SfxMacroTabPage* pMacroTabPage = (SfxMacroTabPage*)pMacroDlg->GetTabPage();

            for (sal_uInt32 j = 0 ; j < aNameArray.size(); j++ )
            {
                OSL_TRACE("adding event = %s",::rtl::OUStringToOString(aNameArray[j], RTL_TEXTENCODING_ASCII_US ).pData->buffer);
                pMacroTabPage->AddEvent( aNameArray[j], (sal_uInt16)j);
            }

            if (nIndex<aNameArray.size())
                pMacroTabPage->SelectEvent( aNameArray[nIndex], nIndex);

            if ( pMacroDlg->Execute() == RET_OK )
            */

            //DF definite problem here
            // OK & Cancel seem to be both returning 0
            if ( pDlg->Execute() == 0 )
            {
                OSL_TRACE("here?");
                setDocumentModified( ); // OJ: #96105#

                //const SvxMacroTableDtor& aTab = pMacroTabPage->GetMacroTbl();

                if ( nObjIdx>=0 && m_xEventManager.is() )
                    m_xEventManager->revokeScriptEvents( nObjIdx );


                //sal_uInt16 nEventCount = (sal_uInt16)aTab.Count();
                sal_uInt16 nEventCount = xNameReplace->getElementNames().getLength();
                sal_uInt16 nEventIndex = 0;

                Sequence< ScriptEventDescriptor > aSeqScriptEvts(nEventCount);

                ScriptEventDescriptor* pWriteScriptEvents = aSeqScriptEvts.getArray();
                nIndex=0;

                String aListenerClassName,aName,aListener;

                pListeners = m_aObjectListenerTypes.getConstArray();

                //this will contain the macro url
                ::rtl::OUString sScriptCode;
                OSL_TRACE("nlen = %d",nLength);
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
                                OSL_TRACE("method is = %s",::rtl::OUStringToOString(*pMethods, RTL_TEXTENCODING_ASCII_US ).pData->buffer);
                                //SvxMacro* pMacro = aTab.Get( nIndex++ );
                                Sequence< css::beans::PropertyValue > props;
                                ::rtl::OUString macroURL;
                                if( xNameReplace->getByName( *pMethods ) >>= props )
                                {
                                    sal_Int32 nCount = props.getLength();
                                    for( sal_Int32 index = 0; index < nCount ; ++index )
                                    {
                                        if ( props[ index ].Name.compareToAscii( "Script" ) == 0 )
                                            props[ index ].Value >>= macroURL;
                                    }
                                }
                                OSL_TRACE("macroURL = %s",::rtl::OUStringToOString(macroURL, RTL_TEXTENCODING_ASCII_US ).pData->buffer);
                                //if ( pMacro )
                                if( macroURL.getLength() > 0 )
                                {
                                    //sScriptCode = pMacro->GetMacName();
                                    sScriptCode = macroURL;
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
                                        /*
                                        if ( pMacro->GetLanguage().EqualsAscii("StarBasic") )
                                        {
                                            sal_Bool bApplicationMacro = pMacro->GetLibName().EqualsAscii("StarOffice");

                                            sScriptCode = ::rtl::OUString::createFromAscii( bApplicationMacro ? "application:" : "document:" );
                                            sScriptCode += pMacro->GetMacName();
                                        }
                                        */

                                        // the url as returned from the dialog
                                        pWriteScriptEvents->ScriptCode = sScriptCode;
                                        //leave this as is
                                        pWriteScriptEvents->EventMethod = *pMethods;
                                        //needs to be set to "Script"
                                        // pWriteScriptEvents->ScriptType = pMacro->GetLanguage();
                                        pWriteScriptEvents->ScriptType = ::rtl::OUString::createFromAscii("Script");
                OSL_TRACE("code = %s",::rtl::OUStringToOString(pWriteScriptEvents->ScriptCode, RTL_TEXTENCODING_ASCII_US ).pData->buffer);
                OSL_TRACE("method = %s",::rtl::OUStringToOString(pWriteScriptEvents->EventMethod, RTL_TEXTENCODING_ASCII_US ).pData->buffer);
                OSL_TRACE("type = %s",::rtl::OUStringToOString(pWriteScriptEvents->ScriptType, RTL_TEXTENCODING_ASCII_US ).pData->buffer);

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
                        if ( rDesc.ScriptCode.getLength() > 0 )
                        {
                            aName += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "::" ) );
                            aName += rDesc.EventMethod;
                            Any aEventAny;
                            aEventAny <<= rDesc;
                            xEventCont->insertByName( aName, aEventAny );
                        }
                    }
                }
            }
            else
            {
                OSL_TRACE("did not click ok?");
            }
        }
        catch (Exception&)
        {
            DBG_ERROR("OPropertyBrowserController::ChangeEventProperty : caught an exception !")
        }

        //delete pMacroDlg;
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
                            aProperty.nUniqueButtonId = pEventDescription->nUniqueBrowseId;
                            aProperty.nMinValue = pEventDescription->nIndex;    // misuse for sorting
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
                getPropertyBox()->InsertEntry( *iter );
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

            ::std::vector< sal_Int32 > aActuatingProperties;
            ::std::vector< Any > aActuatingPropertyValues;
            for (sal_uInt32 i=0; i<nPropCount; ++i, ++pProps)
            {
                sal_Int32   nPropId         = m_pPropertyInfo->getPropertyId( pProps->Name );
                if ( nPropId == -1 )
                    continue;

                // for the MultiLine property, we have different UI translations depending on the control
                // type
                if ( nPropId == PROPERTY_ID_MULTILINE )
                {
                    if (  ( m_nClassId == FormComponentType::FIXEDTEXT )
                       || ( m_nClassId == FormComponentType::COMMANDBUTTON )
                       || ( m_nClassId == FormComponentType::RADIOBUTTON )
                       || ( m_nClassId == FormComponentType::CHECKBOX )
                       )
                        nPropId = PROPERTY_ID_WORDBREAK;
                }

                String sDisplayName = m_pPropertyInfo->getPropertyTranslation( nPropId );
                if ( !sDisplayName.Len() )
                {
                    DBG_ERROR( "OPropertyBrowserController::UpdateUI: no display name! corrupt meta data?" );
                    continue;
                }

                sal_uInt32  nPropertyUIFlags = m_pPropertyInfo->getPropertyUIFlags( nPropId );
                bool bIsVirtualProperty   = ( nPropertyUIFlags & PROP_FLAG_VIRTUAL_PROP   ) != 0;
                bool bIsDataProperty      = ( nPropertyUIFlags & PROP_FLAG_DATA_PROPERTY  ) != 0;
                bool bIsActuatingProperty = ( nPropertyUIFlags & PROP_FLAG_ACTUATING      ) != 0;
                bool bIsEnumProperty      = ( nPropertyUIFlags & PROP_FLAG_ENUM           ) != 0;
                bool bIsVisibleForForms   = ( nPropertyUIFlags & PROP_FLAG_FORM_VISIBLE   ) != 0;
                bool bIsVisibleForDialogs = ( nPropertyUIFlags & PROP_FLAG_DIALOG_VISIBLE ) != 0;

                // depending on whether we're working for a form or a UNO dialog, some
                // properties are not displayed
                if (  ( nControlType == CONTROL_TYPE_FORM   && !bIsVisibleForForms )
                   || ( nControlType == CONTROL_TYPE_DIALOG && !bIsVisibleForDialogs )
                   )
                   continue;

                pProperty = new OLineDescriptor();

                eType = pProps->Type.getTypeClass();

                //////////////////////////////////////////////////////////////////////
                // retrieve the value, and convert it to it's string representation
                eState = PropertyState_DIRECT_VALUE;
                if ( bIsVirtualProperty )
                {
                    eState = getVirtualPropertyState( nPropId );
                    aVal = getVirtualPropertyValue( nPropId );
                }
                else
                {
                    if ( m_xPropStateAccess.is() )
                        eState = m_xPropStateAccess->getPropertyState( pProps->Name );
                    aVal = m_xPropValueAccess->getPropertyValue( pProps->Name );
                    fakePropertyValue( aVal, nPropId );
                }

                aStrVal = getStringRepFromPropertyValue( aVal, nPropId );

                //////////////////////////////////////////////////////////////////////
                // default characteristics for the property control
                pProperty->eControlType = BCT_EDIT;
                pProperty->sName = pProps->Name;
                pProperty->sTitle = pProps->Name;
                pProperty->sValue = aStrVal;

                if (  ( pProps->Attributes & PropertyAttribute::MAYBEVOID )
                   && ( nPropId != PROPERTY_ID_BORDER )
                   && ( nPropId != PROPERTY_ID_TABSTOP )
                   )
                {
                    pProperty->bHasDefaultValue = sal_True;
                    if (!aVal.hasValue())
                        pProperty->sValue = m_sStandard;
                }


                //////////////////////////////////////////////////////////////////////

                sal_Bool bFilter = sal_True;
                switch ( nPropId )
                {
                case PROPERTY_ID_DEFAULT_SELECT_SEQ:
                case PROPERTY_ID_SELECTEDITEMS:
                    pProperty->nUniqueButtonId = UID_PROP_DLG_SELECTION;
                    break;

                case PROPERTY_ID_FILTER:
                    pProperty->nUniqueButtonId = UID_PROP_DLG_FILTER;
                    break;

                case PROPERTY_ID_SORT:
                    pProperty->nUniqueButtonId = UID_PROP_DLG_ORDER;
                    break;

                case PROPERTY_ID_MASTERFIELDS:
                case PROPERTY_ID_DETAILFIELDS:
                    if ( !m_bInspectingSubForm )
                    {   // no master and detail fields for forms which are no sub forms
                        delete pProperty;
                        continue;
                    }

                    pProperty->eControlType = BCT_LEDIT;
                    bFilter = sal_False;
                    pProperty->nUniqueButtonId = UID_PROP_DLG_FORMLINKFIELDS;
                    break;

                case PROPERTY_ID_COMMAND:
                    pProperty->nUniqueButtonId = UID_PROP_DLG_SQLCOMMAND;
                    break;

                case PROPERTY_ID_TABINDEX:
                {
                    if ( m_xControlsView.is() )
                    {
                        pProperty->nUniqueButtonId = UID_PROP_DLG_TABINDEX;
                    }
                    pProperty->eControlType = BCT_NUMFIELD;
                };
                break;

                case PROPERTY_ID_HSCROLL:
                case PROPERTY_ID_VSCROLL:
                {
                    Reference< XPropertySetInfo > xPropInfo = m_xIntrospecteeAsProperty->getPropertySetInfo();
                    if ( xPropInfo.is() )
                    {
                        // if the other property is also present, then don't show the prop
                        // (since it's then simulated with the virtual property ShowScrollbars)
                        ::rtl::OUString sOtherProp( PROPERTY_ID_HSCROLL == nPropId ? PROPERTY_VSCROLL : PROPERTY_HSCROLL );
                        if ( xPropInfo->hasPropertyByName( sOtherProp ) )
                        {
                            delete pProperty;
                            continue;
                        }
                    }
                }
                break;

                case PROPERTY_ID_FONT_NAME:
                {
                    bFilter = sal_False;

                    pProperty->sName = String::CreateFromAscii("Font");
                    pProperty->sTitle = pProperty->sName;
                    pProperty->bIsLocked = sal_True;
                    pProperty->nUniqueButtonId = UID_PROP_DLG_FONT_TYPE;
                    ::rtl::OUString sValue;
                    aVal >>= sValue;
                    pProperty->sValue = sValue;
                }
                break;

                case PROPERTY_ID_TARGET_URL:
                case PROPERTY_ID_IMAGE_URL:
                {
                    pProperty->eControlType = BCT_USERDEFINED;
                    pProperty->pControl = new OFileUrlControl( getPropertyBox(), WB_TABSTOP | WB_BORDER );

/*
                    INetURLObject aParser( getDocumentURL() );
                    aParser.removeSegment();
                    String sBaseURL = aParser.GetMainURL( INetURLObject::NO_DECODE );
                    if ( !sBaseURL.Len() )
                        sBaseURL = SvtPathOptions().GetWorkPath();
                    static_cast< OFileUrlControl* >( pProperty->pControl )->SetBaseURL( sBaseURL );
*/

                    pProperty->nUniqueButtonId = ( PROPERTY_ID_TARGET_URL == nPropId )
                        ? UID_PROP_DLG_ATTR_TARGET_URL : UID_PROP_DLG_IMAGE_URL;
                }
                break;

                case PROPERTY_ID_ECHO_CHAR:
                {
                    pProperty->eControlType = BCT_CHAREDIT;
                }
                break;

                case PROPERTY_ID_BACKGROUNDCOLOR:
                case PROPERTY_ID_FILLCOLOR:
                case PROPERTY_ID_SYMBOLCOLOR:
                case PROPERTY_ID_BORDERCOLOR:
                {
                    bFilter = sal_False;
                    pProperty->eControlType = BCT_COLORBOX;
                    pProperty->bIsLocked = sal_True;

                    switch( nPropId )
                    {
                    case PROPERTY_ID_BACKGROUNDCOLOR:
                        pProperty->nUniqueButtonId = UID_PROP_DLG_BACKGROUNDCOLOR; break;
                    case PROPERTY_ID_FILLCOLOR:
                        pProperty->nUniqueButtonId = UID_PROP_DLG_FILLCOLOR; break;
                    case PROPERTY_ID_SYMBOLCOLOR:
                        pProperty->nUniqueButtonId = UID_PROP_DLG_SYMBOLCOLOR; break;
                    case PROPERTY_ID_BORDERCOLOR:
                        pProperty->nUniqueButtonId = UID_PROP_DLG_BORDERCOLOR; break;
                    }
                }
                break;

                case PROPERTY_ID_LABEL:
                {
                    pProperty->eControlType = BCT_MEDIT;
                }
                break;

                case PROPERTY_ID_DEFAULT_TEXT:
                {
                    if (FormComponentType::FILECONTROL == m_nClassId)
                        pProperty->eControlType = BCT_EDIT;
                    else
                        pProperty->eControlType = BCT_MEDIT;
                }
                break;

                case PROPERTY_ID_TEXT:
                {
                    if ( m_xIntrospecteeAsProperty.is() )
                    {
                        if ( ControlType::FORMATTEDFIELD == m_nClassId )
                        {
                            delete pProperty;
                            continue;
                        }

                        Reference< XPropertySetInfo > xPropInfo = m_xIntrospecteeAsProperty->getPropertySetInfo();
                        if ( xPropInfo.is() )
                        {
                            if ( xPropInfo->hasPropertyByName( PROPERTY_MULTILINE ) )
                                pProperty->eControlType = BCT_MEDIT;
                        }
                    }
                }
                case PROPERTY_ID_MULTILINE:
                {
                    Reference< XPropertySetInfo > xPropInfo = m_xIntrospecteeAsProperty->getPropertySetInfo();
                    if ( xPropInfo.is() )
                    {
                        // if the "RichText" property is also present, then don't show the prop
                        // (since the combination of the two is then simulated with the virtual property TextType)
                        if ( xPropInfo->hasPropertyByName( PROPERTY_RICHTEXT ) )
                        {
                            delete pProperty;
                            continue;
                        }
                    }
                }
                break;
                case PROPERTY_ID_CONTROLLABEL:
                {
                    bFilter = sal_False;
                    pProperty->bIsLocked = sal_True;
                    pProperty->sValue = getStringRepFromPropertyValue(aVal, PROPERTY_ID_CONTROLLABEL);
                    pProperty->nUniqueButtonId = UID_PROP_DLG_CONTROLLABEL;
                }
                break;

                case PROPERTY_ID_FORMATKEY:
                case PROPERTY_ID_EFFECTIVE_MIN:
                case PROPERTY_ID_EFFECTIVE_MAX:
                case PROPERTY_ID_EFFECTIVE_DEFAULT:
                case PROPERTY_ID_EFFECTIVE_VALUE:
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
                    Reference< ::com::sun::star::util::XNumberFormatsSupplier >  xSupplier;
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
                break;

                case PROPERTY_ID_DATEMIN:
                case PROPERTY_ID_DATEMAX:
                case PROPERTY_ID_DEFAULT_DATE:
                case PROPERTY_ID_DATE:
                    pProperty->eControlType = BCT_DATEFIELD;
                    break;

                case PROPERTY_ID_TIMEMIN:
                case PROPERTY_ID_TIMEMAX:
                case PROPERTY_ID_DEFAULT_TIME:
                case PROPERTY_ID_TIME:
                    pProperty->eControlType = BCT_TIMEFIELD;
                    break;

                case PROPERTY_ID_VALUEMIN:
                case PROPERTY_ID_VALUEMAX:
                case PROPERTY_ID_DEFAULT_VALUE:
                case PROPERTY_ID_VALUE:
                    {
                        pProperty->eControlType = BCT_USERDEFINED;
                        pProperty->pControl = new OFormattedNumericControl( getPropertyBox(), WB_TABSTOP | WB_BORDER | WB_SPIN | WB_REPEAT );
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
                    break;

                default:
                    if ( TypeClass_BYTE <= eType && eType <= TypeClass_DOUBLE )
                    {
                        if ( nControlType == CONTROL_TYPE_FORM )
                        {
                            if  ( nPropId == PROPERTY_ID_HEIGHT )
                                pProperty->nDigits = 1;

                            if (  ( nPropId == PROPERTY_ID_WIDTH )
                               || ( nPropId == PROPERTY_ID_ROWHEIGHT )
                               )
                            {
                                pProperty->eValueUnit = FUNIT_CM;
                                pProperty->eDisplayUnit = getDocumentMeasurementUnit();
                                pProperty->nDigits = 2;
                            }
                        }

                        pProperty->eControlType = BCT_NUMFIELD;
                    }
                    break;
                }

                if ( eType == TypeClass_SEQUENCE )
                {
                    pProperty->eControlType = BCT_LEDIT;
                    bFilter = sal_False;
                }

                // don't filter dialog controls
                if ( nControlType == CONTROL_TYPE_DIALOG )
                    bFilter = sal_False;
                // and don't filter virtual properties
                if ( bIsVirtualProperty )
                    bFilter = sal_False;

                //////////////////////////////////////////////////////////////////////
                // Filter
                if ( bFilter )
                {
                    bool bRemoveProperty = false;
                    if ( ( TypeClass_INTERFACE == eType ) || ( TypeClass_ARRAY == eType ) )
                        bRemoveProperty = true;

                    if ( !bRemoveProperty )
                    {
                        if (  ( aVal.getValueTypeClass() == TypeClass_INTERFACE )
                           || ( aVal.getValueTypeClass() == TypeClass_ARRAY )
                           || ( aVal.getValueTypeClass() == TypeClass_UNKNOWN )
                           )
                            bRemoveProperty = true;
                    }

                    if ( !bRemoveProperty )
                    {
                        if (  ( aVal.getValueTypeClass() == TypeClass_VOID )
                           && ( pProps->Attributes & PropertyAttribute::MAYBEVOID ) == 0
                           )
                            bRemoveProperty = true;
                    }

                    if ( !bRemoveProperty )
                    {
                        if ( pProps->Name.equalsAscii( "type unknown" ) )
                            bRemoveProperty = true;
                    }

                    if ( !bRemoveProperty )
                    {
                        if ( pProps->Attributes & PropertyAttribute::TRANSIENT )
                            bRemoveProperty = true;
                    }

                    if ( !bRemoveProperty )
                    {
                        if ( pProps->Attributes & PropertyAttribute::READONLY )
                            bRemoveProperty = true;
                    }

                    if ( bRemoveProperty )
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
                    String aEntries;
                    if (  nPropId == PROPERTY_ID_SHOW_POSITION
                       || nPropId == PROPERTY_ID_SHOW_NAVIGATION
                       || nPropId == PROPERTY_ID_SHOW_RECORDACTIONS
                       || nPropId == PROPERTY_ID_SHOW_FILTERSORT
                       )
                       aEntries = String( ModuleRes( RID_STR_SHOW_HIDE ) );
                    else
                       aEntries = String( ModuleRes( RID_STR_BOOL ) );

                    for ( xub_StrLen i=0; i<2; ++i )
                        pProperty->aListValues.push_back( aEntries.GetToken(i) );

                    pProperty->eControlType = BCT_LISTBOX;
                }

                //////////////////////////////////////////////////////////////////////
                // TYPECLASS_VOID
                else if (eType == TypeClass_VOID )
                    pProperty->sValue = String();

                //////////////////////////////////////////////////////////////////////
                // enum properties
                if ( bIsEnumProperty || ( PROPERTY_ID_TARGET_FRAME == nPropId ) )
                {
                    ::std::vector< String > aEnumValues = m_pPropertyInfo->getPropertyEnumRepresentations( nPropId );
                    ::std::vector< String >::const_iterator pStart = aEnumValues.begin();
                    ::std::vector< String >::const_iterator pEnd = aEnumValues.end();

                    // for a checkbox: if "ambiguous" is not allowed, remove this from the sequence
                    if ( PROPERTY_ID_DEFAULTCHECKED == nPropId || PROPERTY_ID_STATE == nPropId )
                        if ( ::comphelper::hasProperty( PROPERTY_TRISTATE, m_xPropValueAccess ) )
                        {
                            if ( !::comphelper::getBOOL( m_xPropValueAccess->getPropertyValue( PROPERTY_TRISTATE ) ) )
                            {   // remove the last sequence element
                                if ( pEnd > pStart )
                                    --pEnd;
                            }
                        }
                        else
                            --pEnd;

                    if ( PROPERTY_ID_LISTSOURCETYPE == nPropId )
                        if ( FormComponentType::COMBOBOX == m_nClassId )
                            // remove the first sequence element -> value list not possible for combo boxes
                            ++pStart;

                    // copy the sequence
                    for ( ::std::vector< String >::const_iterator pLoop = pStart; pLoop != pEnd; ++pLoop )
                        pProperty->aListValues.push_back( *pLoop );

                    pProperty->eControlType =
                            PROPERTY_ID_TARGET_FRAME == nPropId
                        ?   BCT_COMBOBOX
                        :   BCT_LISTBOX;
                }

                //////////////////////////////////////////////////////////////////////
                switch( nPropId )
                {
                    case PROPERTY_ID_REPEAT_DELAY:
                        pProperty->nMaxValue = 0x7FFFFFFF;
                        pProperty->bHaveMinMax = sal_True;
                        pProperty->nMaxValue = 0;

                        pProperty->eControlType = BCT_USERDEFINED;
                        pProperty->pControl = new TimeDurationInput( getPropertyBox(), WB_BORDER | WB_TABSTOP );
                        break;

                    case PROPERTY_ID_TABINDEX:
                    case PROPERTY_ID_BOUNDCOLUMN:
                    case PROPERTY_ID_VISIBLESIZE:
                    case PROPERTY_ID_MAXTEXTLEN:
                    case PROPERTY_ID_LINEINCREMENT:
                    case PROPERTY_ID_BLOCKINCREMENT:
                    case PROPERTY_ID_SPININCREMENT:
                        pProperty->nMaxValue = 0x7FFFFFFF;
                        pProperty->bHaveMinMax = sal_True;

                        if ( nPropId == PROPERTY_ID_MAXTEXTLEN )
                            pProperty->nMinValue = -1;
                        else if ( ( nPropId == PROPERTY_ID_BOUNDCOLUMN ) || ( nPropId == PROPERTY_ID_VISIBLESIZE ) )
                            pProperty->nMinValue = 1;
                        else
                            pProperty->nMinValue = 0;
                        break;

                    case PROPERTY_ID_DECIMAL_ACCURACY:
                        pProperty->nMaxValue = 20;
                        pProperty->nMinValue = 0;
                        pProperty->bHaveMinMax = sal_True;
                        break;

                    //////////////////////////////////////////////////////////////////////
                    // DataSource
                    case PROPERTY_ID_DATASOURCE:
                    {
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

                    case PROPERTY_ID_CONTROLSOURCE:
                        SetFields( *pProperty );
                        break;
                    case PROPERTY_ID_COMMAND:
                        m_bHasCursorSource = sal_True;
                        break;
                    case PROPERTY_ID_LISTSOURCE:
                        m_bHasListSource = sal_True;
                        break;
                }

                //////////////////////////////////////////////////////////////////////
                // UI-Eintrag
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

                if ( PropertyState_AMBIGUOUS_VALUE == eState )
                {
                    pProperty->bUnknownValue = sal_True;
                    pProperty->sValue = String();
                }

                // finally insert this property control
                getPropertyBox()->InsertEntry(*pProperty);
                delete pProperty;

                // if it's an actuating property, remember it
                if ( bIsActuatingProperty )
                {
                    aActuatingProperties.push_back( nPropId );
                    aActuatingPropertyValues.push_back( GetUnoPropertyValue( pProps->Name ) );
                }
            }

            // update any dependencies for the actuating properties which we encountered
            {
                ::std::vector< sal_Int32 >::const_iterator aProperty = aActuatingProperties.begin();
                ::std::vector< Any >::const_iterator aPropertyValue = aActuatingPropertyValues.begin();
                for ( ; aProperty != aActuatingProperties.end(); ++aProperty, ++aPropertyValue )
                    updateDependentProperties( *aProperty, *aPropertyValue );
            }

            SetCursorSource( sal_True, sal_True );
            SetListSource( sal_True );

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
            DBG_ASSERT(xController.is(), "OPropertyBrowserController::OnImageURLClicked: missing the controller interface on the file picker!");
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
            // since the browse buttons do not get the focus when clicked with the mouse,
            // we need to commit the changes in the current property field
            getPropertyBox()->CommitModified();

            sal_Int32 nPropId = m_pPropertyInfo->getPropertyId(aName);

            switch ( nPropId )
            {
            case PROPERTY_ID_DEFAULT_SELECT_SEQ:
                chooseListSelection( PROPERTY_DEFAULT_SELECT_SEQ );
                break;

            case PROPERTY_ID_SELECTEDITEMS:
                chooseListSelection( PROPERTY_SELECTEDITEMS );
                break;

            case PROPERTY_ID_FILTER:
                executeFilterOrSortDialog( true );
                break;

            case PROPERTY_ID_SORT:
                executeFilterOrSortDialog( false );
                break;

            case PROPERTY_ID_MASTERFIELDS:
            case PROPERTY_ID_DETAILFIELDS:
                doEnterLinkedFormFields();
                break;

            case PROPERTY_ID_COMMAND:
                doDesignSQLCommand();
                break;

            case PROPERTY_ID_TABINDEX:
            {
                Reference< XTabControllerModel > xTabControllerModel( getRowSet(), UNO_QUERY );
                TabOrderDialog aDialog( getDialogParent(), xTabControllerModel, m_xControlsView, m_xORB );
                aDialog.Execute();
            }
            break;

            case PROPERTY_ID_TARGET_URL:
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
            break;

            case PROPERTY_ID_IMAGE_URL:
                OnImageURLClicked( aName, aVal, pData );
                break;

            case PROPERTY_ID_BACKGROUNDCOLOR:
            case PROPERTY_ID_FILLCOLOR:
            case PROPERTY_ID_SYMBOLCOLOR:
            case PROPERTY_ID_BORDERCOLOR:
            {
                sal_uInt32 nColor = aVal.ToInt32();
                Color aColor( nColor );
                SvColorDialog aColorDlg( getDialogParent() );
                aColorDlg.SetColor( aColor );

                if (aColorDlg.Execute() )
                {
                    aColor = aColorDlg.GetColor();
                    nColor = aColor.GetColor();

                    String aColorString = String::CreateFromInt32( (sal_Int32)nColor );
                    Commit( aName, aColorString, pData );
                }
            }
            break;

            case PROPERTY_ID_FORMATKEY:
            {
                ChangeFormatProperty(aName, aVal);
            }
            break;

            case PROPERTY_ID_CONTROLLABEL:
            {
                OSelectLabelDialog dlgSelectLabel( getDialogParent(), m_xPropValueAccess );
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
            break;

            default:
            if ( aName.EqualsAscii( "Font" ) )
            {
                ChangeFontProperty( );
            }
            else if ( pData == LINETYPE_EVENT )
            {
                ChangeEventProperty(aName);
            }
            break;
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
            bool bIsActuatingProperty       = ( nPropertyUIFlags & PROP_FLAG_ACTUATING ) != 0;

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

            // re-retrieve the value
            if ( bIsVirtualProperty )
            {
                aValue = getVirtualPropertyValue( nPropId );
            }
            else
            {
                aValue = m_xPropValueAccess->getPropertyValue( rName );
            }

            // care for any inter-property dependencies
            if ( bIsActuatingProperty )
                updateDependentProperties( nPropId, aValue );

            ::rtl::OUString sNewStrVal = getStringRepFromPropertyValue( aValue, nPropId );

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

                ::std::vector< String > aEntries =
                    m_pPropertyInfo->getPropertyEnumRepresentations(nStateId);
                sal_Int32 nEntryCount = aEntries.size();

                if (!::comphelper::getBOOL(aValue))
                    // tristate not allowed -> remove the "don't know" state
                    --nEntryCount;

                sal_Bool bValidDefaultCheckedValue = sal_False;

                ::std::vector< String >::const_iterator pStart = aEntries.begin();
                ::std::vector< String >::const_iterator pEnd = aEntries.end();
                for ( ::std::vector< String >::const_iterator pLoop = pStart; pLoop != pEnd; ++pLoop )
                {
                    aProperty.aListValues.push_back(*pLoop);
                    if ( *pLoop == aProperty.sValue )
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
                sal_uInt16  nNewDigits = bAccuracy ? ::comphelper::getINT16(aValue) : 0;
                sal_Bool    bUseSep = bAccuracy ? sal_False : ::comphelper::getBOOL(aValue);

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
                    (const Reference< ::com::sun::star::util::XNumberFormatsSupplier>*)0)),

                    "OPropertyBrowserController::Commit : invalid property change !");
                    // we only allowed the FormatKey property to be displayed if the set had a valid FormatsSupplier
                Reference< ::com::sun::star::util::XNumberFormatsSupplier >  xSupplier;
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
    PropertyState OPropertyBrowserController::getVirtualPropertyState( sal_Int32 _nPropId )
    {
        PropertyState eState = PropertyState_DIRECT_VALUE;

        switch ( _nPropId )
        {
        case PROPERTY_ID_BUTTONTYPE:
        {
            PushButtonNavigation aHelper( m_xPropValueAccess );
            eState = aHelper.getCurrentButtonTypeState();
        }
        break;
        case PROPERTY_ID_TARGET_URL:
        {
            PushButtonNavigation aHelper( m_xPropValueAccess );
            eState = aHelper.getCurrentTargetURLState();
        }
        break;
        }

        return eState;
    }

    //------------------------------------------------------------------------
    Any OPropertyBrowserController::getVirtualPropertyValue( sal_Int32 _nPropId )
    {
        Any aReturn;

        switch ( _nPropId )
        {
        case PROPERTY_ID_BUTTONTYPE:
        {
            PushButtonNavigation aHelper( m_xPropValueAccess );
            aReturn = aHelper.getCurrentButtonType();
        }
        break;

        case PROPERTY_ID_TARGET_URL:
        {
            PushButtonNavigation aHelper( m_xPropValueAccess );
            aReturn = aHelper.getCurrentTargetURL();
        }
        break;

        case PROPERTY_ID_BOUND_CELL:
        {
            CellBindingHelper aHelper( m_xPropValueAccess );
            Reference< XValueBinding > xBinding( aHelper.getCurrentBinding() );
            if ( !aHelper.isCellBinding( xBinding ) )
                xBinding.clear();

            aReturn <<= xBinding;
        }
        break;

        case PROPERTY_ID_LIST_CELL_RANGE:
        {
            CellBindingHelper aHelper( m_xPropValueAccess );
            Reference< XListEntrySource > xSource( aHelper.getCurrentListSource() );
            if ( !aHelper.isCellRangeListSource( xSource ) )
                xSource.clear();

            aReturn <<= xSource;
        }
        break;

        case PROPERTY_ID_CELL_EXCHANGE_TYPE:
        {
            CellBindingHelper aHelper( m_xPropValueAccess );
            Reference< XValueBinding > xBinding( aHelper.getCurrentBinding() );
            aReturn <<= (sal_Int16)( aHelper.isCellIntegerBinding( xBinding ) ? 1 : 0 );
        }
        break;

        case PROPERTY_ID_SHOW_SCROLLBARS:
        {
            sal_Bool bHasVScroll = sal_False;
            GetUnoPropertyValue( PROPERTY_VSCROLL ) >>= bHasVScroll;
            sal_Bool bHasHScroll = sal_False;
            GetUnoPropertyValue( PROPERTY_HSCROLL ) >>= bHasHScroll;

            aReturn <<= (sal_Int32)( ( bHasVScroll ? 2 : 0 ) + ( bHasHScroll ? 1 : 0 ) );
        }
        break;

        case PROPERTY_ID_TEXTTYPE:
        {
            sal_Int32 nTextType = TEXTTYPE_SINGLELINE;
            try
            {
                sal_Bool bRichText = sal_False;
                OSL_VERIFY( m_xPropValueAccess->getPropertyValue( PROPERTY_RICHTEXT ) >>= bRichText );
                if ( bRichText )
                    nTextType = TEXTTYPE_RICHTEXT;
                else
                {
                    sal_Bool bMultiLine = sal_False;
                    OSL_VERIFY( m_xPropValueAccess->getPropertyValue( PROPERTY_MULTILINE ) >>= bMultiLine );
                    if ( bMultiLine )
                        nTextType = TEXTTYPE_MULTILINE;
                    else
                        nTextType = TEXTTYPE_SINGLELINE;
                }
                aReturn <<= nTextType;
            }
            catch( const Exception& )
            {
                OSL_ENSURE( sal_False, "OPropertyBrowserController::getVirtualPropertyValue: caught an exception!" );
            }
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
        switch ( _nPropId )
        {
        case PROPERTY_ID_BUTTONTYPE:
        {
            PushButtonNavigation aHelper( m_xPropValueAccess );
            aHelper.setCurrentButtonType( _rValue );
        }
        break;

        case PROPERTY_ID_TARGET_URL:
        {
            PushButtonNavigation aHelper( m_xPropValueAccess );
            aHelper.setCurrentTargetURL( _rValue );
        }
        break;

        case PROPERTY_ID_BOUND_CELL:
        {
            CellBindingHelper aHelper( m_xPropValueAccess );
            Reference< XValueBinding > xBinding;
            _rValue >>= xBinding;
            aHelper.setBinding( xBinding );
        }
        break;

        case PROPERTY_ID_LIST_CELL_RANGE:
        {
            CellBindingHelper aHelper( m_xPropValueAccess );
            Reference< XListEntrySource > xSource;
            _rValue >>= xSource;
            aHelper.setListSource( xSource );
        }
        break;

        case PROPERTY_ID_CELL_EXCHANGE_TYPE:
        {
            CellBindingHelper aHelper( m_xPropValueAccess );
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

        case PROPERTY_ID_SHOW_SCROLLBARS:
        {
            sal_Int32 nScrollbars = 0;
            _rValue >>= nScrollbars;

            sal_Bool bHasVScroll = 0 != ( nScrollbars & 2 );
            sal_Bool bHasHScroll = 0 != ( nScrollbars & 1 );

            m_xPropValueAccess->setPropertyValue( PROPERTY_VSCROLL, makeAny( (sal_Bool)bHasVScroll ) );
            m_xPropValueAccess->setPropertyValue( PROPERTY_HSCROLL, makeAny( (sal_Bool)bHasHScroll ) );
        }
        break;

        case PROPERTY_ID_TEXTTYPE:
        {
            sal_Bool bMultiLine = sal_False;
            sal_Bool bRichText = sal_False;
            sal_Int32 nTextType = TEXTTYPE_SINGLELINE;
            OSL_VERIFY( _rValue >>= nTextType );
            switch ( nTextType )
            {
            case TEXTTYPE_SINGLELINE: bMultiLine = bRichText = sal_False; break;
            case TEXTTYPE_MULTILINE:  bMultiLine = sal_True; bRichText = sal_False; break;
            case TEXTTYPE_RICHTEXT:   bMultiLine = sal_True; bRichText = sal_True; break;
            default:
                OSL_ENSURE( sal_False, "OPropertyBrowserController::setVirtualPropertyValue: invalid text type!" );
            }

            try
            {
                m_xPropValueAccess->setPropertyValue( PROPERTY_MULTILINE, makeAny( bMultiLine ) );
                m_xPropValueAccess->setPropertyValue( PROPERTY_RICHTEXT, makeAny( bRichText ) );
            }
            catch( const Exception& )
            {
                OSL_ENSURE( sal_False, "OPropertyBrowserController::setVirtualPropertyValue: caught an exception!" );
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
            CellBindingHelper aHelper( m_xPropValueAccess );
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

        // have a "Scrollbars" property if the object supports both "HScroll" and "VScroll"
        Reference< XPropertySetInfo > xPSI;
        if ( m_xPropValueAccess.is() )
            xPSI = m_xPropValueAccess->getPropertySetInfo();
        if ( xPSI.is() )
        {
            if ( xPSI->hasPropertyByName( PROPERTY_HSCROLL ) && xPSI->hasPropertyByName( PROPERTY_VSCROLL ) )
            {
                sal_Int32 nLength = _rProps.getLength();
                _rProps.realloc( nLength + 1 );
                _rProps[ nLength ] = Property( PROPERTY_SHOW_SCROLLBARS, PROPERTY_ID_SHOW_SCROLLBARS,
                        ::getCppuType( static_cast< sal_Int32* >( NULL ) ), 0 );
            }

            // if the control supports both the "RichText" and "MultiLine" properties, then we
            // have a virtual property "TextType", which combines these two
            if ( xPSI->hasPropertyByName( PROPERTY_MULTILINE ) && xPSI->hasPropertyByName( PROPERTY_RICHTEXT ) )
            {
                sal_Int32 nLength = _rProps.getLength();
                _rProps.realloc( nLength + 1 );
                _rProps[ nLength ] = Property( PROPERTY_TEXTTYPE, PROPERTY_ID_TEXTTYPE,
                        ::getCppuType( static_cast< sal_Int32* >( NULL ) ), 0 );
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
    void OPropertyBrowserController::enablePropertyLinesIfNonEmptyString( const ::rtl::OUString* _pPropertyStart,
        const ::rtl::OUString* _pPropertyEnd, const Any& _rStringPropertyValue )
    {
        ::rtl::OUString sValue;
        _rStringPropertyValue >>= sValue;
        enablePropertyLines( _pPropertyStart, _pPropertyEnd, sValue.getLength() != 0 );
    }

#define STRING_ARRAY_LIMITS( array ) \
    array, \
    array + sizeof( array ) / sizeof( array[0] )

    //------------------------------------------------------------------------
    void OPropertyBrowserController::updateDependentProperties( sal_Int32 _nPropId, const Any& _rNewValue )
    {
        DBG_ASSERT( getPropertyBox(), "OPropertyBrowserController::updateDependentProperties: no view!" );
        if ( !getPropertyBox() )
            return;

        // some properties depend on more than a single property changed, they will be updated
        // below
        ::std::vector< ::rtl::OUString > aComplexDependentProperties;
        aComplexDependentProperties.reserve( 5 );

        switch ( _nPropId )
        {
        case PROPERTY_ID_TEXTTYPE:
        {
            sal_Int32 nTextType = TEXTTYPE_SINGLELINE;
            getVirtualPropertyValue( PROPERTY_ID_TEXTTYPE ) >>= nTextType;

            getPropertyBox()->EnablePropertyLine( PROPERTY_WORDBREAK,       nTextType == TEXTTYPE_RICHTEXT );
            getPropertyBox()->EnablePropertyLine( PROPERTY_MAXTEXTLEN,      nTextType != TEXTTYPE_RICHTEXT );
            getPropertyBox()->EnablePropertyLine( PROPERTY_ECHO_CHAR,       nTextType == TEXTTYPE_SINGLELINE );
            getPropertyBox()->EnablePropertyLine( PROPERTY_FONT_NAME,       nTextType != TEXTTYPE_RICHTEXT );
            getPropertyBox()->EnablePropertyLine( PROPERTY_ALIGN,           nTextType != TEXTTYPE_RICHTEXT );
            getPropertyBox()->EnablePropertyLine( PROPERTY_DEFAULT_TEXT,    nTextType != TEXTTYPE_RICHTEXT );
            getPropertyBox()->EnablePropertyLine( ::rtl::OUString::createFromAscii( "Font" ), nTextType != TEXTTYPE_RICHTEXT );
            getPropertyBox()->EnablePropertyLine( PROPERTY_SHOW_SCROLLBARS, nTextType != TEXTTYPE_SINGLELINE );
            getPropertyBox()->EnablePropertyLine( PROPERTY_LINEEND_FORMAT,  nTextType != TEXTTYPE_SINGLELINE );

            getPropertyBox()->ShowPropertyPage( m_nDataPageId, nTextType != TEXTTYPE_RICHTEXT );
        }
        break;

        case PROPERTY_ID_BOUND_CELL:
        {
            // the SQL-data-binding related properties need to be enabled if and only if
            // there is *no* valid cell binding
            Reference< XValueBinding > xBinding;
            _rNewValue >>= xBinding;

            getPropertyBox()->EnablePropertyLine( PROPERTY_CELL_EXCHANGE_TYPE, xBinding.is() );
            getPropertyBox()->EnablePropertyLine( PROPERTY_CONTROLSOURCE, !xBinding.is() );

            aComplexDependentProperties.push_back( PROPERTY_FILTERPROPOSAL );
            aComplexDependentProperties.push_back( PROPERTY_EMPTY_IS_NULL );
            aComplexDependentProperties.push_back( PROPERTY_BOUNDCOLUMN );

            if ( !xBinding.is() )
            {
                // ensure that the "transfer selection as" property is reset. Since we can't remember
                // it at the object itself, but derive it from the binding only, we have to normalize
                // it now that there *is* no binding anymore.
                getPropertyBox()->SetPropertyValue(
                    PROPERTY_CELL_EXCHANGE_TYPE,
                    getStringRepFromPropertyValue(
                        makeAny( (sal_Int16) 0 ),
                        PROPERTY_ID_CELL_EXCHANGE_TYPE
                    )
                );
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
                PROPERTY_LISTSOURCE, PROPERTY_LISTSOURCETYPE
            };
            enablePropertyLines( STRING_ARRAY_LIMITS( aListSourceProperties ), !xSource.is() );

            aComplexDependentProperties.push_back( PROPERTY_BOUNDCOLUMN );
            aComplexDependentProperties.push_back( PROPERTY_STRINGITEMLIST );

            // also reset the list entries if the cell range is reset
            // #i28319# - 2004-04-27 - fs@openoffice.org
            try
            {
                if ( !xSource.is() )
                    m_xPropValueAccess->setPropertyValue( PROPERTY_STRINGITEMLIST, makeAny( Sequence< ::rtl::OUString >() ) );
            }
            catch( const Exception& )
            {
                OSL_ENSURE( sal_False, "OPropertyBrowserController::updateDependentProperties( ListCellRange ): caught an exception while resetting the string items!" );
            }
        }
        break;

        case PROPERTY_ID_CONTROLSOURCE:
        {
            ::rtl::OUString sControlSource;
            _rNewValue >>= sControlSource;
            getPropertyBox()->EnablePropertyLine( PROPERTY_BOUND_CELL, sControlSource.getLength() == 0 );

            aComplexDependentProperties.push_back( PROPERTY_FILTERPROPOSAL );
            aComplexDependentProperties.push_back( PROPERTY_EMPTY_IS_NULL );
            aComplexDependentProperties.push_back( PROPERTY_BOUNDCOLUMN );
            aComplexDependentProperties.push_back( PROPERTY_SCALEIMAGE );
        }
        break;

        case PROPERTY_ID_LISTSOURCETYPE:
            aComplexDependentProperties.push_back( PROPERTY_BOUNDCOLUMN );
            aComplexDependentProperties.push_back( PROPERTY_STRINGITEMLIST );
            // NO break!

        case PROPERTY_ID_STRINGITEMLIST:
            aComplexDependentProperties.push_back( PROPERTY_SELECTEDITEMS );
            aComplexDependentProperties.push_back( PROPERTY_DEFAULT_SELECT_SEQ );
            break;

        case PROPERTY_ID_BUTTONTYPE:
        {
            sal_Int32 nButtonType( FormButtonType_PUSH );
            _rNewValue >>= nButtonType;

            // TargetURL depends on the button type *only*
            getPropertyBox()->EnablePropertyLine( PROPERTY_TARGET_URL, FormButtonType_URL == nButtonType );

            // TargetFrame depends on the button type *plus* other properties
            aComplexDependentProperties.push_back( PROPERTY_TARGET_FRAME );
        }
        break;

        case PROPERTY_ID_IMAGE_URL:
        {
            ::rtl::OUString aImageRelatedProperties[] =
            {
                PROPERTY_IMAGEPOSITION
            };
            enablePropertyLinesIfNonEmptyString( STRING_ARRAY_LIMITS( aImageRelatedProperties ), _rNewValue );

            aComplexDependentProperties.push_back( PROPERTY_SCALEIMAGE );
        }
        break;

        case PROPERTY_ID_DROPDOWN:
        {
            sal_Bool bDropDown = sal_True;
            _rNewValue >>= bDropDown;
            getPropertyBox()->EnablePropertyLine( PROPERTY_LINECOUNT, bDropDown );
        }
        break;

        case PROPERTY_ID_TARGET_URL:
        {
            ::rtl::OUString aTargetUrlRelatedProperties[] =
            {
                PROPERTY_TARGET_FRAME
            };
            enablePropertyLinesIfNonEmptyString( STRING_ARRAY_LIMITS( aTargetUrlRelatedProperties ), _rNewValue );
        }
        break;

        case PROPERTY_ID_SUBMIT_ENCODING:
        {
            FormSubmitEncoding eEncoding = FormSubmitEncoding_URL;
            _rNewValue >>= eEncoding;
            getPropertyBox()->EnablePropertyLine( PROPERTY_SUBMIT_METHOD, eEncoding == FormSubmitEncoding_URL );
        }
        break;

        case PROPERTY_ID_LISTSOURCE:
            aComplexDependentProperties.push_back( PROPERTY_STRINGITEMLIST );
            break;

        case PROPERTY_ID_MULTILINE:
        {
            sal_Bool bIsMultiline = sal_False;
            _rNewValue >>= bIsMultiline;

            getPropertyBox()->EnablePropertyLine( PROPERTY_SHOW_SCROLLBARS, bIsMultiline );
            getPropertyBox()->EnablePropertyLine( PROPERTY_ECHO_CHAR, !bIsMultiline );
            getPropertyBox()->EnablePropertyLine( PROPERTY_LINEEND_FORMAT, bIsMultiline );
        }
        break;

        case PROPERTY_ID_COMMAND:
        case PROPERTY_ID_DATASOURCE:
            aComplexDependentProperties.push_back( PROPERTY_FILTER );
            aComplexDependentProperties.push_back( PROPERTY_SORT );
            if ( m_bInspectingSubForm )
                aComplexDependentProperties.push_back( PROPERTY_DETAILFIELDS );
            if ( _nPropId == PROPERTY_ID_DATASOURCE )
                aComplexDependentProperties.push_back( PROPERTY_COMMAND );
            break;

        case PROPERTY_ID_ESCAPE_PROCESSING:
            aComplexDependentProperties.push_back( PROPERTY_FILTER );
            aComplexDependentProperties.push_back( PROPERTY_SORT );
            // NO break!

        case PROPERTY_ID_COMMANDTYPE:
            aComplexDependentProperties.push_back( PROPERTY_COMMAND );
            break;

        case PROPERTY_ID_REPEAT:
        {
            sal_Bool bIsRepeating = sal_False;
            _rNewValue >>= bIsRepeating;

            getPropertyBox()->EnablePropertyLine( PROPERTY_REPEAT_DELAY, bIsRepeating );
        }
        break;

        case PROPERTY_ID_TABSTOP:
        {
            sal_Bool bHasTabStop = sal_False;
            _rNewValue >>= bHasTabStop;

            getPropertyBox()->EnablePropertyLine( PROPERTY_TABINDEX, bHasTabStop );
        }
        break;

        case PROPERTY_ID_BORDER:
        {
            sal_Int16 nBordeType = VisualEffect::NONE;
            OSL_VERIFY( _rNewValue >>= nBordeType );
            getPropertyBox()->EnablePropertyLine( PROPERTY_BORDERCOLOR, nBordeType == VisualEffect::FLAT );
        }
        break;

        default:
            DBG_ERROR( "OPropertyBrowserController::updateDependentProperties: this is no actuating property!" );
            break;
        }

        Reference< XPropertySetInfo > xPropInfo;
        if ( m_xIntrospecteeAsProperty.is() )
            xPropInfo = m_xIntrospecteeAsProperty->getPropertySetInfo();
        DBG_ASSERT( xPropInfo.is(), "OPropertyBrowserController::updateDependentProperties: no property set info!" );

        for ( ::std::vector< ::rtl::OUString >::const_iterator aLoop = aComplexDependentProperties.begin();
              aLoop != aComplexDependentProperties.end();
              ++aLoop
            )
        {
            if ( xPropInfo.is() && xPropInfo->hasPropertyByName( *aLoop ) )
                updateComplexPropertyDependency( *aLoop );
        }
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserController::updateComplexPropertyDependency( const ::rtl::OUString& _rPropertyName )
    {
        DBG_ASSERT( getPropertyBox(), "OPropertyBrowserController::updateComplexPropertyDependency: no view!" );
        if ( !getPropertyBox() )
            return;

        Reference< XValueBinding > xBinding;
        Reference< XListEntrySource > xSource;
        ::rtl::OUString sStringValue;
        ListSourceType eLSType = ListSourceType_VALUELIST;

        bool bIsSpreadsheetDocument = CellBindingHelper::livesInSpreadsheetDocument( m_xPropValueAccess );
        bool bIsEnabled = true;                 // should the whole property line be en/disabled?
        bool bEnableBrowseButtonOnly = false;   // does |bIsEnabled| hold for the browse button only?

        switch ( m_pPropertyInfo->getPropertyId( _rPropertyName ) )
        {
        case PROPERTY_ID_SCALEIMAGE:
            GetUnoPropertyValue( PROPERTY_CONTROLSOURCE, true ) >>= sStringValue;
            bIsEnabled = sStringValue.getLength() != 0;
            GetUnoPropertyValue( PROPERTY_IMAGE_URL ) >>= sStringValue;
            bIsEnabled |= ( sStringValue.getLength() != 0 );
            break;

        case PROPERTY_ID_FILTERPROPOSAL:
        case PROPERTY_ID_EMPTY_IS_NULL:
            GetUnoPropertyValue( PROPERTY_CONTROLSOURCE ) >>= sStringValue;
            if ( bIsSpreadsheetDocument )
                GetUnoPropertyValue( PROPERTY_BOUND_CELL ) >>= xBinding;
            bIsEnabled = !xBinding.is() && sStringValue.getLength();
            break;

        case PROPERTY_ID_BOUNDCOLUMN:
            GetUnoPropertyValue( PROPERTY_CONTROLSOURCE ) >>= sStringValue;
            if ( bIsSpreadsheetDocument )
            {
                GetUnoPropertyValue( PROPERTY_BOUND_CELL ) >>= xBinding;
                GetUnoPropertyValue( PROPERTY_LIST_CELL_RANGE ) >>= xSource;
            }
            GetUnoPropertyValue( PROPERTY_LISTSOURCETYPE ) >>= eLSType;

            bIsEnabled = ( sStringValue.getLength() > 0 )
                      && ( !xBinding.is() )
                      && ( !xSource.is() )
                      && ( eLSType != ListSourceType_TABLEFIELDS )
                      && ( eLSType != ListSourceType_VALUELIST );
            break;

        case PROPERTY_ID_STRINGITEMLIST:
            if ( bIsSpreadsheetDocument )
                GetUnoPropertyValue( PROPERTY_LIST_CELL_RANGE ) >>= xSource;
            GetUnoPropertyValue( PROPERTY_LISTSOURCETYPE ) >>= eLSType;

            {
                Sequence< ::rtl::OUString > aListSource;
                Any aListSourceValue( GetUnoPropertyValue( PROPERTY_LISTSOURCE ) );
                if ( aListSourceValue >>= aListSource )
                {
                    if ( aListSource.getLength() )
                        sStringValue = aListSource[0];
                }
                else
                    OSL_VERIFY( aListSourceValue >>= sStringValue );
            }

            bIsEnabled = ( !xSource.is() )
                      && (  ( eLSType == ListSourceType_VALUELIST )
                         || ( sStringValue.getLength() == 0 )
                         );
            break;

        case PROPERTY_ID_TARGET_FRAME:
        {
            GetUnoPropertyValue( PROPERTY_TARGET_URL ) >>= sStringValue;
            FormButtonType eButtonType( FormButtonType_PUSH );
            if ( 0 != m_nClassId )
            {   // if we're inspecting a control which has a TargetFrame, then it also has
                // the FormButtonType property, since it's a push/image button then
                GetUnoPropertyValue( PROPERTY_BUTTONTYPE ) >>= eButtonType;
            }
            bIsEnabled = ( eButtonType == FormButtonType_URL )
                      && ( sStringValue.getLength() > 0 );
        }
        break;

        case PROPERTY_ID_COMMAND:
        {
            sal_Int32 nIntValue( 0 );
            sal_Bool  bBoolValue( sal_False );
            GetUnoPropertyValue( PROPERTY_COMMANDTYPE ) >>= nIntValue;
            GetUnoPropertyValue( PROPERTY_ESCAPE_PROCESSING ) >>= bBoolValue;

            bIsEnabled = ( nIntValue == CommandType::COMMAND )
                      && ( bBoolValue )
                      && (  haveRowsetConnection()
                         || isValidDataSourceName( GetPropertyValue( PROPERTY_DATASOURCE ) )
                         );

            bEnableBrowseButtonOnly = true;
        }
        break;

        case PROPERTY_ID_DETAILFIELDS:
        {
            // both our current form, and it's parent form, need to have a valid
            // data source signature
            bIsEnabled = hasValidDataSourceSignature( Reference< XForm >( m_xIntrospecteeAsProperty, UNO_QUERY ) )
                      && hasValidDataSourceSignature( Reference< XForm >( m_xObjectParent,           UNO_QUERY ) );

            // in opposite to the other properties, here in real *two* properties are
            // affected
            getPropertyBox()->EnablePropertyInput( PROPERTY_DETAILFIELDS, true, bIsEnabled );
            getPropertyBox()->EnablePropertyInput( PROPERTY_MASTERFIELDS, true, bIsEnabled );
            return;
        }
        break;

        case PROPERTY_ID_SORT:
        case PROPERTY_ID_FILTER:
        {
            sal_Bool  bBoolValue( sal_False );
            GetUnoPropertyValue( PROPERTY_ESCAPE_PROCESSING ) >>= bBoolValue;
            bIsEnabled = hasValidDataSourceSignature( Reference< XForm >( m_xIntrospecteeAsProperty, UNO_QUERY ) )
                      && bBoolValue;

            // if escape processing is on, then we need to disable/enable the browse button only,
            // else the whole line
            if ( bBoolValue )
                getPropertyBox()->EnablePropertyInput( _rPropertyName, true, bIsEnabled );
            else
                getPropertyBox()->EnablePropertyInput( _rPropertyName, false, false );
            return;
        }
        break;

        case PROPERTY_ID_SELECTEDITEMS:
        case PROPERTY_ID_DEFAULT_SELECT_SEQ:
        {
            Sequence< ::rtl::OUString > aEntries;
            GetUnoPropertyValue( PROPERTY_STRINGITEMLIST ) >>= aEntries;
            bIsEnabled = aEntries.getLength() != 0;

            if ( ( m_nClassId == FormComponentType::LISTBOX ) && ( getControlType() == CONTROL_TYPE_FORM ) )
            {
                GetUnoPropertyValue( PROPERTY_LISTSOURCETYPE ) >>= eLSType;
                bIsEnabled &= ( eLSType == ListSourceType_VALUELIST );
            }
            bEnableBrowseButtonOnly = true;
        }
        break;

        default:
            DBG_ERROR( "OPropertyBrowserController::updateComplexPropertyDependency: don't know what to do for this property!" );
            break;
        }

        if ( bEnableBrowseButtonOnly )
            getPropertyBox()->EnablePropertyInput( _rPropertyName, true, bIsEnabled );
        else
            getPropertyBox()->EnablePropertyLine( _rPropertyName, bIsEnabled );
    }


    //------------------------------------------------------------------------
    bool OPropertyBrowserController::hasValidDataSourceSignature( const Reference< XForm >& _rxForm ) SAL_THROW(())
    {
        Reference< XPropertySet > xProps( _rxForm, UNO_QUERY );
        DBG_ASSERT( xProps.is(), "OPropertyBrowserController::hasValidDataSourceSignature: invalid form!" );
        if ( !xProps.is() )
            return false;

        bool bHas = false;
        try
        {
            ::rtl::OUString sPropertyValue;
            // first, we need the name of an existent data source
            xProps->getPropertyValue( PROPERTY_DATASOURCE ) >>= sPropertyValue;
            bHas = ( sPropertyValue.getLength() != 0 );
            // then, the command should not be empty
            if ( bHas )
            {
                xProps->getPropertyValue( PROPERTY_COMMAND ) >>= sPropertyValue;
                bHas = ( sPropertyValue.getLength() != 0 );
            }
        }
        catch( const Exception& )
        {
            DBG_ERROR( "OPropertyBrowserController::hasValidDataSourceSignature: caught an exception!" );
        }
        return bHas;
    }

    //------------------------------------------------------------------------
    bool OPropertyBrowserController::isValidDataSourceName( const ::rtl::OUString& _rDSName )
    {
        bool bHas = false;
        Reference< XNameAccess >  xDatabaseAccesses( m_xORB->createInstance( SERVICE_DATABASE_CONTEXT ), UNO_QUERY );
        if ( xDatabaseAccesses.is() )
        {
            Reference< XDataSource >  xDataSource;
            try
            {
                bHas = xDatabaseAccesses->hasByName( _rDSName );
            }
            catch ( const Exception& )
            {
                OSL_ENSURE( sal_False, "OPropertyBrowserController::isValidDataSourceName: caught an exception!" );
            }
        }
        return bHas;
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserController::classifyControlModel( )
    {
        m_nClassId = 0;

        if ( ::comphelper::hasProperty( PROPERTY_CLASSID, m_xPropValueAccess ) )
        {
            m_nClassId = ::comphelper::getINT16(m_xPropValueAccess->getPropertyValue(PROPERTY_CLASSID));
        }
        else if ( CONTROL_TYPE_DIALOG == getControlType() )
        {
            Reference< XControlModel > xControlModel( m_xIntrospecteeAsProperty, UNO_QUERY );
            Reference< XServiceInfo > xServiceInfo( xControlModel, UNO_QUERY );
            if ( xServiceInfo.is() )
            {
                // it's a control model, and can tell about it's supported services
                m_nClassId = FormComponentType::CONTROL;

                const sal_Char* aControlModelServiceNames[] =
                {
                    "UnoControlButtonModel",
                    "UnoControlCheckBoxModel",
                    "UnoControlComboBoxModel",
                    "UnoControlCurrencyFieldModel",
                    "UnoControlDateFieldModel",
                    "UnoControlEditModel",
                    "UnoControlFileControlModel",
                    "UnoControlFixedTextModel",
                    "UnoControlGroupBoxModel",
                    "UnoControlImageControlModel",
                    "UnoControlListBoxModel",
                    "UnoControlNumericFieldModel",
                    "UnoControlPatternFieldModel",
                    "UnoControlRadioButtonModel",
                    "UnoControlScrollBarModel",
                    "UnoControlSpinButtonModel",
                    "UnoControlTimeFieldModel",

                    "UnoControlFixedLineModel",
                    "UnoControlFormattedFieldModel",
                    "UnoControlProgressBarModel"
                };
                const sal_Int16 nClassIDs[] =
                {
                    FormComponentType::COMMANDBUTTON,
                    FormComponentType::CHECKBOX,
                    FormComponentType::COMBOBOX,
                    FormComponentType::CURRENCYFIELD,
                    FormComponentType::DATEFIELD,
                    FormComponentType::TEXTFIELD,
                    FormComponentType::FILECONTROL,
                    FormComponentType::FIXEDTEXT,
                    FormComponentType::GROUPBOX,
                    FormComponentType::IMAGECONTROL,
                    FormComponentType::LISTBOX,
                    FormComponentType::NUMERICFIELD,
                    FormComponentType::PATTERNFIELD,
                    FormComponentType::RADIOBUTTON,
                    FormComponentType::SCROLLBAR,
                    FormComponentType::SPINBUTTON,
                    FormComponentType::TIMEFIELD,

                    ControlType::FIXEDLINE,
                    ControlType::FORMATTEDFIELD,
                    ControlType::PROGRESSBAR
                };

                sal_Int32 nKnownControlTypes = sizeof( aControlModelServiceNames ) / sizeof( aControlModelServiceNames[ 0 ] );
                OSL_ENSURE( nKnownControlTypes == sizeof( nClassIDs ) / sizeof( nClassIDs[ 0 ] ),
                    "OPropertyBrowserController::classifyControlModel: inconsistence" );

                for ( sal_Int32 i = 0; i < nKnownControlTypes; ++i )
                {
                    ::rtl::OUString sServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt." ) );
                    sServiceName += ::rtl::OUString::createFromAscii( aControlModelServiceNames[ i ] );

                    if ( xServiceInfo->supportsService( sServiceName ) )
                    {
                        m_nClassId = nClassIDs[ i ];
                        break;
                    }
                }
            }
        }
    }

    //------------------------------------------------------------------------
    FieldUnit OPropertyBrowserController::getDocumentMeasurementUnit() const
    {
        FieldUnit eUnit = FUNIT_NONE;

        Reference< XServiceInfo > xDocumentSI( getDocumentModel(), UNO_QUERY );
        if ( xDocumentSI.is() )
        {
            // determine the application type we live in
            ::rtl::OUString sConfigurationLocation;
            ::rtl::OUString sConfigurationProperty;
            if ( xDocumentSI->supportsService( SERVICE_WEB_DOCUMENT ) )
            {   // writer
                sConfigurationLocation = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.WriterWeb/Layout/Other" ) );
                sConfigurationProperty = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MeasureUnit" ) );
            }
            else if ( xDocumentSI->supportsService( SERVICE_TEXT_DOCUMENT ) )
            {   // writer
                sConfigurationLocation = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.Writer/Layout/Other" ) );
                sConfigurationProperty = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MeasureUnit" ) );
            }
            else if ( xDocumentSI->supportsService( SERVICE_SPREADSHEET_DOCUMENT ) )
            {   // calc
                sConfigurationLocation = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.Calc/Layout/Other/MeasureUnit" ) );
                sConfigurationProperty = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Metric" ) );
            }
            else if ( xDocumentSI->supportsService( SERVICE_DRAWING_DOCUMENT ) )
            {
                sConfigurationLocation = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.Draw/Layout/Other/MeasureUnit" ) );
                sConfigurationProperty = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Metric" ) );
            }
            else if ( xDocumentSI->supportsService( SERVICE_PRESENTATION_DOCUMENT ) )
            {
                sConfigurationLocation = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.Impress/Layout/Other/MeasureUnit" ) );
                sConfigurationProperty = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Metric" ) );
            }

            // read the measurement unit from the configuration
            if ( sConfigurationLocation.getLength() && sConfigurationProperty.getLength() )
            {
                ::utl::OConfigurationTreeRoot aConfigTree( ::utl::OConfigurationTreeRoot::createWithServiceFactory(
                    m_xORB, sConfigurationLocation, -1, ::utl::OConfigurationTreeRoot::CM_READONLY ) );
                sal_Int32 nUnitAsInt = (sal_Int32)FUNIT_NONE;
                aConfigTree.getNodeValue( sConfigurationProperty ) >>= nUnitAsInt;

                // if this denotes a valid (and accepted) unit, then use it
                if  ( ( nUnitAsInt > FUNIT_NONE ) && ( nUnitAsInt <= FUNIT_100TH_MM ) )
                    eUnit = static_cast< FieldUnit >( nUnitAsInt );
            }
        }

        if ( FUNIT_NONE == eUnit )
        {
            MeasurementSystem eSystem = SvtSysLocale().GetLocaleData().getMeasurementSystemEnum();
            eUnit = MEASURE_METRIC == eSystem ? FUNIT_CM : FUNIT_INCH;
        }
        return eUnit;
    }

    //------------------------------------------------------------------------
    sal_Bool OPropertyBrowserController::suspendDependentComponent()
    {
        if ( m_xDependentComponent.is() )
        {
            sal_Bool bAllow = sal_True;
            m_bSuspendingDependentComp = sal_True;
            try
            {
                bAllow = m_xDependentComponent->suspend( sal_True );
            }
            catch( const Exception& )
            {
                OSL_ENSURE( sal_False, "OPropertyBrowserController::suspendDependentComponent: caught an exception!" );
            }
            m_bSuspendingDependentComp = sal_False;
            return bAllow;
        }
        return sal_True;
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserController::closeDependentComponent()
    {
        OSL_ENSURE( m_xDependentComponent.is(), "OPropertyBrowserController::closeDependentComponent: not to be called without dependent component!" );
        if ( !m_xDependentComponent.is() )
            return;

        // do not listen anymore ....
        Reference< XPropertySet > xProps( m_xDependentComponent, UNO_QUERY );
        OSL_ENSURE( xProps.is(), "OPropertyBrowserController::closeDependentComponent: invalid dependent component!" );
        if ( xProps.is() )
            xProps->removePropertyChangeListener( PROPERTY_ACTIVECOMMAND, this );

        // close it
        try
        {
            // we need to close the frame via the "user interface", by dispatching a close command,
            // instead of calling XCloseable::close directly. The latter method would also close
            // the frame, but not care for things like shutting down the office when the last
            // frame is gone ...
            UnoURL aCloseURL( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:CloseDoc" ) ), m_xORB );

            Reference< XDispatchProvider > xProvider( m_xDependentComponent->getFrame(), UNO_QUERY );
            Reference< XDispatch > xDispatch;
            if ( xProvider.is() )
                xDispatch = xProvider->queryDispatch( aCloseURL, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "_top" ) ), FrameSearchFlag::SELF );
            OSL_ENSURE( xDispatch.is(), "OPropertyBrowserController::dispose: no dispatcher for the CloseDoc command!" );
            if ( xDispatch.is() )
            {
                xDispatch->dispatch( aCloseURL, Sequence< PropertyValue >( ) );
            }
            else
            {
                // fallback: use the XCloseable::close (with all possible disadvantages)
                Reference< css::util::XCloseable > xClose( m_xDependentComponent->getFrame(), UNO_QUERY );
                if ( xClose.is() )
                    xClose->close( sal_True );
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "OPropertyBrowserController::dispose: caught an exception!" );
        }

        m_xDependentComponent = NULL;
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserController::dependentComponentClosed()
    {
        OSL_ENSURE( getPropertyBox(), "OPropertyBrowserController::dependentComponentClosed: no view!" );
        if ( getPropertyBox() )
        {
            getPropertyBox()->EnablePropertyInput( PROPERTY_DATASOURCE, true, true );
            getPropertyBox()->EnablePropertyInput( PROPERTY_COMMANDTYPE, true, true );
            getPropertyBox()->EnablePropertyInput( PROPERTY_COMMAND, true, true );
            getPropertyBox()->EnablePropertyInput( PROPERTY_ESCAPE_PROCESSING, true, true );
        }
    }

    //------------------------------------------------------------------------
    Reference< XFrame > OPropertyBrowserController::createEmptyParentlessTask( ) const
    {
        Reference< XFrame > xFrame;
        try
        {
            Reference< XInterface      > xDesktop          ( m_xORB->createInstance( SERVICE_DESKTOP ) );
            Reference< XFrame          > xDesktopFrame     ( xDesktop,      UNO_QUERY );
            Reference< XFramesSupplier > xSuppDesktopFrames( xDesktopFrame, UNO_QUERY );

            Reference< XFrames > xDesktopFramesCollection;
            if ( xSuppDesktopFrames.is() )
                xDesktopFramesCollection = xSuppDesktopFrames->getFrames();
            OSL_ENSURE( xDesktopFramesCollection.is(), "OPropertyBrowserController::createEmptyParentlessTask: invalid desktop!" );

            if ( xDesktopFramesCollection.is() )
            {
                xFrame = xDesktopFrame->findFrame( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "_blank" ) ), FrameSearchFlag::CREATE );
                OSL_ENSURE( xFrame.is(), "OPropertyBrowserController::createEmptyParentlessTask: could not create an empty frame!" );
                xDesktopFramesCollection->remove( xFrame );
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "OPropertyBrowserController::createEmptyParentlessTask: caught an exception!" );
        }
        return xFrame;
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserController::executeFilterOrSortDialog( bool _bFilter )
    {
        OSL_ENSURE( Reference< XRowSet >( m_xIntrospecteeAsProperty, UNO_QUERY ).is(),
            "OPropertyBrowserController::executeFilterOrSortDialog: to be called for forms only!" );

        SQLExceptionInfo aErrorInfo;
        try
        {
            Reference< XConnection > xConnection = ensureRowsetConnection();
            if ( !xConnection.is() )
                return;

            // get a composer for the statement which the form is currently based on
            Reference< XSQLQueryComposer > xComposer = getCurrentSettingsComposer( m_xIntrospecteeAsProperty, m_xORB );
            OSL_ENSURE( xComposer.is(), "OPropertyBrowserController::executeFilterOrSortDialog: could not obtain a composer!" );
            if ( !xComposer.is() )
                return;

            ::rtl::OUString sPropertyUIName;
            if ( m_pPropertyInfo )
                sPropertyUIName = m_pPropertyInfo->getPropertyTranslation( _bFilter ? PROPERTY_ID_FILTER : PROPERTY_ID_SORT );

            ::rtl::OUString sDialogServiceName;
            if ( _bFilter )
                sDialogServiceName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sdb.FilterDialog" ) );
            else
                sDialogServiceName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sdb.OrderDialog" ) );
            // create the dialog
            Reference< XExecutableDialog > xDialog( m_xORB->createInstance( sDialogServiceName ), UNO_QUERY );
            if ( !xDialog.is() )
            {
                ShowServiceNotAvailableError( getDialogParent(), sDialogServiceName, sal_True );
                return;
            }

            // initialize the dialog
            Reference< XPropertySet > xDialogProps( xDialog, UNO_QUERY );
            OSL_ENSURE( xDialogProps.is(), "OPropertyBrowserController::executeFilterOrSortDialog: no properties for the dialog!" );
            if ( !xDialogProps.is() )
                return;
            xDialogProps->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "QueryComposer" ) ), makeAny( xComposer ) );
            xDialogProps->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "RowSet" ) ),        makeAny( getRowSet() ) );
            xDialogProps->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ParentWindow" ) ),  makeAny( VCLUnoHelper::GetInterface( getDialogParent() ) ) );
            xDialogProps->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) ),         makeAny( sPropertyUIName ) );

            if ( RET_OK == xDialog->execute() )
                if ( _bFilter )
                    m_xIntrospecteeAsProperty->setPropertyValue( PROPERTY_FILTER, makeAny( xComposer->getFilter() ) );
                else
                    m_xIntrospecteeAsProperty->setPropertyValue( PROPERTY_SORT,   makeAny( xComposer->getOrder()  ) );
        }
        catch (SQLContext& e) { aErrorInfo = e; }
        catch (SQLWarning& e) { aErrorInfo = e; }
        catch (SQLException& e) { aErrorInfo = e; }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "OPropertyBrowserController::executeFilterOrSortDialog: caught an exception!" );
        }

        if ( aErrorInfo.isValid() && getDialogParent() )
            showError( aErrorInfo, VCLUnoHelper::GetInterface( getDialogParent() ), m_xORB );
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserController::chooseListSelection( const ::rtl::OUString& _rProperty )
    {
        OSL_PRECOND( m_pPropertyInfo, "OPropertyBrowserController::chooseListSelection: no property meta data!" );

        String sPropertyUIName( m_pPropertyInfo->getPropertyTranslation( m_pPropertyInfo->getPropertyId( _rProperty ) ) );
        ListSelectionDialog aDialog( getDialogParent(), m_xIntrospecteeAsProperty, _rProperty, sPropertyUIName );
        aDialog.Execute();
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserController::doEnterLinkedFormFields()
    {
        Reference< XForm > xDetailForm( m_xIntrospecteeAsProperty, UNO_QUERY );
        Reference< XForm > xMasterForm( m_xObjectParent,           UNO_QUERY );
        OSL_ENSURE( xDetailForm.is() && xMasterForm.is(), "OPropertyBrowserController::doEnterLinkedFormFields: no forms!" );
        if ( !xDetailForm.is() || !xMasterForm.is() )
            return;

        FormLinkDialog aDialog( getDialogParent(), xDetailForm, xMasterForm, m_xORB );
        aDialog.Execute();
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserController::doDesignSQLCommand( )
    {
        try
        {
            if ( m_xDependentComponent.is() )
            {
                // activate the frame for this component
                Reference< XFrame > xFrame( m_xDependentComponent->getFrame() );
                OSL_ENSURE( xFrame.is(), "OPropertyBrowserController::doDesignSQLCommand: a controller without frame?" );
                if ( xFrame.is() )
                {
                    Reference< XWindow >    xWindow   ( xFrame->getContainerWindow() );
                    Reference< XTopWindow > xTopWindow( xWindow, UNO_QUERY );
                    OSL_ENSURE( xTopWindow.is(), "OPropertyBrowserController::doDesignSQLCommand: can't activate the frame!" );
                    if ( xTopWindow.is() )
                        xTopWindow->toFront();
                    if ( xWindow.is() )
                        xWindow->setFocus();
                }
                return;
            }

            Reference< XConnection > xConnection = ensureRowsetConnection();
            if ( !xConnection.is() )
                return;

            // for various reasons, we don't want the new frame to appear in the desktop's frame list
            // thus, we create a blank frame at the desktop, remove it from the desktop's frame list
            // immediately, and then load the component into this blank (and now parent-less) frame
            Reference< XComponentLoader > xLoader( createEmptyParentlessTask(), UNO_QUERY );
            OSL_ENSURE( xLoader.is(), "OPropertyBrowserController::doDesignSQLCommand: uhoh - no loader at the frame!" );

            if ( xLoader.is() )
            {
                Sequence< PropertyValue > aArgs( 3 );
                aArgs[0].Name  = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IndependentSQLCommand" ) );
                aArgs[0].Value = GetUnoPropertyValue( PROPERTY_COMMAND );
                aArgs[1].Name  = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ActiveConnection" ) );
                aArgs[1].Value <<= xConnection;
                aArgs[2].Name  = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "QueryDesignView" ) );
                aArgs[2].Value <<= (sal_Bool)sal_True;

                Reference< XComponent > xQueryDesign = xLoader->loadComponentFromURL(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".component:DB/QueryDesign" ) ),
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "_self" ) ),
                    FrameSearchFlag::TASKS | FrameSearchFlag::CREATE,
                    aArgs
                );

                // remember this newly loaded component - we need to care for it e.g. when we're suspended
                m_xDependentComponent = m_xDependentComponent.query( xQueryDesign );
                OSL_ENSURE( m_xDependentComponent.is() || !xQueryDesign.is(), "OPropertyBrowserController::doDesignSQLCommand: the component is expected to be a controller!" );
                if ( m_xDependentComponent.is() )
                {
                    Reference< XPropertySet > xQueryDesignProps( m_xDependentComponent, UNO_QUERY );
                    OSL_ENSURE( xQueryDesignProps.is(), "OPropertyBrowserController::doDesignSQLCommand: the controller should have properties!" );
                    if ( xQueryDesignProps.is() )
                        xQueryDesignProps->addPropertyChangeListener( PROPERTY_ACTIVECOMMAND, this );
                }

                // get the frame which we just opened and set it's title
                Reference< XPropertySet > xFrameProps;
                Reference< XController > xController( xQueryDesign, UNO_QUERY );
                if ( xController.is() )
                    xFrameProps = xFrameProps.query( xController->getFrame() );

                if ( xFrameProps.is() && xFrameProps->getPropertySetInfo().is() && xFrameProps->getPropertySetInfo()->hasPropertyByName( PROPERTY_TITLE ) )
                {
                    ::svt::OLocalResourceAccess aEnumStrings( ModuleRes( RID_RSC_ENUM_COMMAND_TYPE ), RSC_RESOURCE );
                    ::rtl::OUString sTitle = String( ResId( 3 ) );
                    xFrameProps->setPropertyValue( PROPERTY_TITLE, makeAny( sTitle ) );
                }

                getPropertyBox()->EnablePropertyInput( PROPERTY_DATASOURCE, false, true );
                getPropertyBox()->EnablePropertyInput( PROPERTY_COMMANDTYPE, false, true );
                getPropertyBox()->EnablePropertyInput( PROPERTY_COMMAND, false, true );
                getPropertyBox()->EnablePropertyInput( PROPERTY_ESCAPE_PROCESSING, false, true );
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "OPropertyBrowserController::doDesignSQLCommand: caught an exception!" );
        }
    }

//*****************************************************************************
//  constructor
//*****************************************************************************
EventsNameReplace_Impl::EventsNameReplace_Impl()
{
}

//*****************************************************************************
//  destructor
//*****************************************************************************
EventsNameReplace_Impl::~EventsNameReplace_Impl()
{
    m_eventList.clear();
}

//*****************************************************************************
//  public method - AddEvent
//*****************************************************************************
void EventsNameReplace_Impl::AddEvent( ::rtl::OUString sEventName, ::rtl::OUString sMacroURL )
{
    OSL_TRACE("event = %s, macroURL = %s",
        rtl::OUStringToOString(sEventName, RTL_TEXTENCODING_ASCII_US ).pData->buffer,
        rtl::OUStringToOString(sMacroURL, RTL_TEXTENCODING_ASCII_US ).pData->buffer);
    m_eventList.push_back( ::std::make_pair( sEventName, sMacroURL) );
}

//*****************************************************************************
//  XNameReplace
//*****************************************************************************
void SAL_CALL EventsNameReplace_Impl::replaceByName( const ::rtl::OUString& aName, const Any& aElement ) throw (css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::lang::WrappedTargetException, RuntimeException)
{
    Sequence< css::beans::PropertyValue > props;
    if( sal_False == ( aElement >>= props ) )
    {
        throw css::lang::IllegalArgumentException( ::rtl::OUString::createFromAscii(""),
                Reference< XInterface > (), 2);
    }
    ::rtl::OUString macroURL;
    sal_Int32 nPropCount = props.getLength();
    for( sal_Int32 index = 0 ; index < nPropCount ; ++index )
    {
        if ( props[ index ].Name.compareToAscii( "Script" ) == 0 )
            props[ index ].Value >>= macroURL;
    }

    EventList::iterator it = m_eventList.begin();
    EventList::iterator it_end = m_eventList.end();
    for(;it!=it_end;++it)
    {
        if( aName.equals( it->first ) )
            break;

    }
    if(it==it_end)
        throw css::container::NoSuchElementException( ::rtl::OUString::createFromAscii("No such element in event configuration"),
                Reference< XInterface > () );

    it->second = macroURL;
}

Any SAL_CALL EventsNameReplace_Impl::getByName( const ::rtl::OUString& aName ) throw (css::container::NoSuchElementException, css::lang::WrappedTargetException, RuntimeException)
{
    Any aRet;
    Sequence< css::beans::PropertyValue > props(2);
    props[0].Name = ::rtl::OUString::createFromAscii("EventType");
    props[0].Value <<= ::rtl::OUString::createFromAscii("Script");
    props[1].Name = ::rtl::OUString::createFromAscii("Script");
    EventList::const_iterator it = m_eventList.begin();
    EventList::const_iterator it_end = m_eventList.end();
    for(;it!=it_end;++it)
    {
        if( aName.equals( it->first ) )
            break;

    }
    if(it==it_end)
        throw css::container::NoSuchElementException( ::rtl::OUString::createFromAscii("No such element in event configuration"),
            Reference< XInterface > () );

    props[1].Value <<= it->second;
    aRet <<= props;
    return aRet;
}

Sequence< ::rtl::OUString > SAL_CALL EventsNameReplace_Impl::getElementNames(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > ret(m_eventList.size());
    EventList::const_iterator it = m_eventList.begin();
    EventList::const_iterator it_end = m_eventList.end();
    for(int i=0;it!=it_end;++it,++i)
    {
        ret[ i ] = it->first;
    }
    return ret;
}

sal_Bool SAL_CALL EventsNameReplace_Impl::hasByName( const ::rtl::OUString& aName ) throw (RuntimeException)
{
    EventList::const_iterator it = m_eventList.begin();
    EventList::const_iterator it_end = m_eventList.end();
    for(;it!=it_end;++it)
    {
        if( aName.equals( it->first ) )
            return sal_True;
    }
    return sal_False;
}

Type SAL_CALL EventsNameReplace_Impl::getElementType(  ) throw (RuntimeException)
{
    //DF definitly not sure about this??
    return ::getCppuType((const Sequence< css::beans::PropertyValue >*)0);
}

sal_Bool SAL_CALL EventsNameReplace_Impl::hasElements(  ) throw (RuntimeException)
{
    return ( m_eventList.empty() );
}


//............................................................................
} // namespace pcr
//............................................................................

