/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: formcontroller.cxx,v $
 *
 *  $Revision: 1.87 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-23 12:51:43 $
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
#ifndef EXTENSIONS_INC_EXTENSIO_HRC
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
#ifndef SVTOOLS_FILENOTATION_HXX_
#include <svtools/filenotation.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _COM_SUN_STAR_SDB_XSINGLESELECTQUERYCOMPOSER_HPP_
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
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
#ifndef _SFX_DOCFILT_HACK_HXX
#include <sfx2/docfilt.hxx>
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
#ifndef EXTENSIONS_PROPCTRLR_CELLBINDINGHELPER_HXX
#include "cellbindinghelper.hxx"
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
#ifndef _COM_SUN_STAR_FORM_BINDING_XLISTENTRYSOURCE_HPP_
#include <com/sun/star/form/binding/XListEntrySource.hpp>
#endif

#ifndef _MACROPG_HXX
#include <sfx2/macropg.hxx>
#endif
#ifndef _SFXMACITEM_HXX
#include <svtools/macitem.hxx>
#endif

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
#ifndef _UNOTOOLS_CONFIGNODE_HXX_
#include <unotools/confignode.hxx>
#endif

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_PROPERTYCOMPOSER_HXX
#include "propertycomposer.hxx"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_CELLBINDINGHANDLER_HXX
#include "cellbindinghandler.hxx"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_BUTTONNAVIGATIONHANDLER_HXX
#include "buttonnavigationhandler.hxx"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_EDITPROPERTYHANDLER_HXX
#include "editpropertyhandler.hxx"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_EFORMSPROPERTYHANDLER_HXX
#include "eformspropertyhandler.hxx"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_XSDVALIDATIONPROPERTYHANDLER_HXX
#include "xsdvalidationpropertyhandler.hxx"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_SUBMISSIONHANDLER_HXX
#include "submissionhandler.hxx"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_STRINGREPRESENTATION_HXX
#include "stringrepresentation.hxx"
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
#include <algorithm>

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
    using namespace ::com::sun::star::util;
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
    ::rtl::OUString OPropertyBrowserController::getStringRepFromPropertyValue( sal_Int32 _nPropId, const Any& rValue )
    {
        ::rtl::OUString sReturn;
        if ( !rValue.hasValue() )
            return sReturn;

        try
        {
            StringRepresentation aConversionHelper( m_xTypeConverter );
            sReturn = aConversionHelper.convertSimpleToString(rValue);

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
            switch ( _nPropId )
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
                case PROPERTY_ID_SHOW_POSITION:
                case PROPERTY_ID_SHOW_NAVIGATION:
                case PROPERTY_ID_SHOW_RECORDACTIONS:
                case PROPERTY_ID_SHOW_FILTERSORT:
                {
                    String aShowHide = String( ModuleRes( RID_STR_SHOW_HIDE ) );
                    sReturn = aShowHide.GetToken( ::comphelper::getBOOL( rValue ) ? 1 : 0 );
                }
                break;
                case PROPERTY_ID_DATASOURCE:
                {
                    rValue >>= sReturn;
                    if ( sReturn.getLength() )
                    {
                        ::svt::OFileNotation aTransformer(sReturn);
                        sReturn = aTransformer.get(::svt::OFileNotation::N_SYSTEM);
                    }
                }
                break;
                default:
                {
                    StringRepresentation aConversionHelper( m_xTypeConverter );
                    sReturn = aConversionHelper.getStringRepFromPropertyValue( rValue, _nPropId, m_pPropertyInfo );
                }
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
            if ( ( _rProp.Attributes & PropertyAttribute::MAYBEVOID ) && !_rString.getLength() )
                return aReturn;

            if (  ( _nPropId == PROPERTY_ID_SHOW_POSITION )
               || (_nPropId == PROPERTY_ID_SHOW_NAVIGATION )
               || (_nPropId == PROPERTY_ID_SHOW_RECORDACTIONS )
               || (_nPropId == PROPERTY_ID_SHOW_FILTERSORT )
               )
            {
                // TODO: do this with property handlers
                String sShowHide = String( ModuleRes( RID_STR_SHOW_HIDE ) );
                aReturn <<= (sal_Bool)( sShowHide.GetToken(1) == String( _rString ) );
            }
            else
            {
                Type  aPropertyType = _rProp.Type;

                // special handling : we have a prop which has a TypeClass "ANY" and needs a double
                // (originally it needed a double _or_ a string, but our UI only supports a double for it)
                if  (  aPropertyType.equals( ::getCppuType( static_cast< const Any* >( NULL ) ) )
                    && (  ( PROPERTY_ID_EFFECTIVE_DEFAULT == _nPropId )
                       || ( PROPERTY_ID_EFFECTIVE_VALUE == _nPropId )
                       )
                    )
                    aPropertyType = ::getCppuType( static_cast< const double* >( NULL ) );

                StringRepresentation aConversionHelper( m_xTypeConverter );
                aReturn = aConversionHelper.getPropertyValueFromStringRep( _rString, aPropertyType, _nPropId, m_pPropertyInfo );
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
                Commit( _rName, ::rtl::OUString::valueOf((sal_Int32)((SfxUInt32Item*)pItem)->GetValue()) );
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
            Reference< XPropertySet > xFormSet;

            if ( Reference< XForm >( m_xObjectParent, UNO_QUERY ).is() )
            {
                xFormSet = xFormSet.query( m_xObjectParent );
            }
            else if ( Reference< XGridColumnFactory >( m_xObjectParent, UNO_QUERY ).is() )
            {   // we're inspecting a grid column -> the form is one step above
                Reference< XChild > xChild( m_xObjectParent, UNO_QUERY );
                if ( xChild.is() )
                    xFormSet = xFormSet.query( xChild->getParent() );
            }

            if ( !xFormSet.is() )
                return;

            ::rtl::OUString aObjectName = ::comphelper::getString(xFormSet->getPropertyValue(PROPERTY_COMMAND));
            // when there is no command we don't need to ask for columns
            if (aObjectName.getLength())
            {
                ::rtl::OUString aDatabaseName = ::comphelper::getString(xFormSet->getPropertyValue(PROPERTY_DATASOURCE));
                sal_Int32 nObjectType = ::comphelper::getINT32(xFormSet->getPropertyValue(PROPERTY_COMMANDTYPE));

                Reference< XConnection > xConnection = ensureAndGetRowsetConnection();
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
                xTables = Reference< XTablesSupplier >( ensureAndGetRowsetConnection( ), UNO_QUERY );
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
                xSupplyQueries = Reference< XQueriesSupplier >( ensureAndGetRowsetConnection(), UNO_QUERY );
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
        m_xRowsetConnection.clear();
        // will automatically dispose it, if and only if it had the ownership
    }

    //------------------------------------------------------------------------
    Reference< XConnection > OPropertyBrowserController::ensureAndGetRowsetConnection()
    {
        Reference< XConnection > xConnection;
        ensureRowsetConnection( xConnection );
        return xConnection;
    }

    //------------------------------------------------------------------------
    bool OPropertyBrowserController::ensureRowsetConnection()
    {
        Reference< XConnection > xNotInterestedIn;
        return ensureRowsetConnection( xNotInterestedIn );
    }

    //------------------------------------------------------------------------
    bool OPropertyBrowserController::ensureRowsetConnection( Reference< XConnection >& _rConnection )
    {
        _rConnection.clear();

        cleanupRowsetConnection();

        Reference< XRowSet > xRowSet( getRowSet() );
        Reference< XPropertySet > xRowSetProps( xRowSet, UNO_QUERY );

        // connect the row set - this is delegated to elsewhere - while observing errors
        SQLExceptionInfo aError;
        try
        {
            if ( xRowSetProps.is() )
            {
                ::std::auto_ptr< WaitObject > pWaitCursor;
                if ( m_pView )
                    pWaitCursor.reset( new WaitObject( m_pView ) );

                m_xRowsetConnection = ::dbtools::ensureRowSetConnection( xRowSet, m_xORB, false );
            }
        }
        catch (SQLContext& e) { aError = e; }
        catch (SQLWarning& e) { aError = e; }
        catch (SQLException& e) { aError = e; }
        catch (WrappedTargetException& e ) { aError = SQLExceptionInfo( e.TargetException ); }
        catch (Exception&) { }

        // report errors, if necessary
        if ( aError.isValid() && haveView() )
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
                DBG_ERROR( "OPropertyBrowserController::ensureRowsetConnection: caught an exception during error handling!" );
            }
            // additional info about what happended
            String sInfo( ModuleRes( RID_STR_UNABLETOCONNECT ) );
            INetURLObject aParser( sDataSourceName );
            if ( aParser.GetProtocol() != INET_PROT_NOT_VALID )
                sDataSourceName = aParser.getBase( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );
            sInfo.SearchAndReplaceAllAscii( "$name$", sDataSourceName );

            SQLContext aContext;
            aContext.Message = sInfo;
            aContext.NextException = aError.get();
            showError( aContext, VCLUnoHelper::GetInterface( m_pView ), m_xORB);
        }

        xRowSetProps->getPropertyValue( PROPERTY_ACTIVE_CONNECTION ) >>= _rConnection;
        return _rConnection.is();
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
            aProperty.nHelpId = m_pPropertyInfo->getPropertyHelpId( PROPERTY_ID_COMMAND );
            aProperty.nButtonHelpId = UID_PROP_DLG_SQLCOMMAND;
            if ( _bInit )
                aProperty.sValue = GetPropertyValueStringRep( PROPERTY_COMMAND );

            ////////////////////////////////////////////////////////////
            if ( _bConnect ? ensureRowsetConnection() : isRowsetConnected() )
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

            getPropertyBox()->ChangeEntry( aProperty );
            Commit( aProperty.sName, aProperty.sValue );
        }
        catch (Exception&)
        {
            DBG_ERROR("OPropertyBrowserController::SetCursorSource : caught an exception !")
        }
    }
    //------------------------------------------------------------------------
    bool OPropertyBrowserController::isRowsetConnected( ) const
    {
        Reference< XConnection > xConnection;
        Reference< XPropertySet > xProps( getRowSet(), UNO_QUERY );
        if ( xProps.is() )
            xProps->getPropertyValue( PROPERTY_ACTIVE_CONNECTION ) >>= xConnection;
        if ( !xConnection.is() )
            isEmbeddedInDatabase( getRowSet(), xConnection );
        return xConnection.is();
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

            ::rtl::OUString aListSource = GetPropertyValueStringRep( PROPERTY_LISTSOURCE );

            ////////////////////////////////////////////////////////////
            // Setzen der UI-Daten
            OLineDescriptor aProperty;
            aProperty.eControlType = BCT_MEDIT;
            aProperty.sName = (const ::rtl::OUString&)PROPERTY_LISTSOURCE;
            aProperty.sTitle = m_pPropertyInfo->getPropertyTranslation(PROPERTY_ID_LISTSOURCE);
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
            getPropertyBox()->ChangeEntry( aProperty );
            Commit( aProperty.sName, aProperty.sValue );
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
                xEventsSupplier = xEventsSupplier.query( m_xIntrospecteeAsProperty );
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
                setDocumentModified( ); // OJ: #96105#

                //const SvxMacroTableDtor& aTab = pMacroTabPage->GetMacroTbl();

                if ( nObjIdx>=0 && m_xEventManager.is() )
                    m_xEventManager->revokeScriptEvents( nObjIdx );


                //sal_uInt16 nEventCount = (sal_uInt16)aTab.Count();
                sal_Int32 nEventCount = xNameReplace->getElementNames().getLength();
                sal_Int32 nEventIndex = 0;

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
                                if( sal_True == ( xNameReplace->getByName( *pMethods ) >>= props ) )
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
                Reference< XScriptEventsSupplier > xEventsSupplier( m_xIntrospecteeAsProperty, UNO_QUERY );
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
            aProperty.nFlags = LINE_FLAG_EVENT;

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
                            aProperty.nButtonHelpId = pEventDescription->nUniqueBrowseId;
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
    bool OPropertyBrowserController::describePropertyLine( sal_Int16 _nControlType, const Property& _rProperty, OLineDescriptor& _rDescriptor ) SAL_THROW((Exception))
    {
        _rDescriptor.clear();

        sal_Int32   nPropId = m_pPropertyInfo->getPropertyId( _rProperty.Name );
        if ( nPropId == -1 )
            return false;

        //////////////////////////////////////////////////////////////////////
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
            DBG_ERROR( "OPropertyBrowserController::describePropertyLine: no display name! corrupt meta data?" );
            return false;
        }

        sal_uInt32  nPropertyUIFlags = m_pPropertyInfo->getPropertyUIFlags( nPropId );
        bool bIsEnumProperty      = ( nPropertyUIFlags & PROP_FLAG_ENUM           ) != 0;
        bool bIsVisibleForForms   = ( nPropertyUIFlags & PROP_FLAG_FORM_VISIBLE   ) != 0;
        bool bIsVisibleForDialogs = ( nPropertyUIFlags & PROP_FLAG_DIALOG_VISIBLE ) != 0;

        // depending on whether we're working for a form or a UNO dialog, some
        // properties are not displayed
        if (  ( _nControlType == CONTROL_TYPE_FORM   && !bIsVisibleForForms )
           || ( _nControlType == CONTROL_TYPE_DIALOG && !bIsVisibleForDialogs )
           )
           return false;

        if ( nPropId == PROPERTY_ID_DATASOURCE )
        {
            Reference< XConnection > xConn;
            if ( m_xPropValueAccess.is() && ::dbtools::isEmbeddedInDatabase( m_xPropValueAccess, xConn ) )
                return false;
        }

        _rDescriptor.sName = _rProperty.Name;

        //////////////////////////////////////////////////////////////////////
        // do we have a dedicated handler for this property, which we can delegate some tasks to?
        PropertyHandlerRepository::const_iterator aHandlerPos = m_aPropertyHandlers.find( nPropId );
        bool bDedicatedHandler = aHandlerPos != m_aPropertyHandlers.end();

        bool bNeedGenericUIDescriptor = true;
        if ( bDedicatedHandler && aHandlerPos->second->supportsUIDescriptor( nPropId ) )
        {
            aHandlerPos->second->describePropertyUI( nPropId, _rDescriptor );
            bNeedGenericUIDescriptor = false;
        }
        else
        {
            _rDescriptor.eControlType = BCT_EDIT;
        }

        //////////////////////////////////////////////////////////////////////

        PropertyState   ePropertyState = PropertyState_DIRECT_VALUE;
        Any             aPropertyValue;
        if ( bDedicatedHandler )
        {
            ePropertyState = aHandlerPos->second->getPropertyState( nPropId );
            aPropertyValue = aHandlerPos->second->getPropertyValue( nPropId );
            _rDescriptor.sValue = aHandlerPos->second->getStringRepFromPropertyValue( nPropId, aPropertyValue );
        }
        else
        {
            if ( m_xPropStateAccess.is() )
                ePropertyState = m_xPropStateAccess->getPropertyState( _rProperty.Name );
            aPropertyValue = GetUnoPropertyValue( _rProperty.Name );
            _rDescriptor.sValue = getStringRepFromPropertyValue( nPropId, aPropertyValue );
        }

        //////////////////////////////////////////////////////////////////////

        if  (  ( _rProperty.Attributes & PropertyAttribute::MAYBEVOID )
            && ( nPropId != PROPERTY_ID_BORDER )
            && ( nPropId != PROPERTY_ID_TABSTOP )
            )
        {
            _rDescriptor.bHasDefaultValue = sal_True;
            if ( !aPropertyValue.hasValue() )
                _rDescriptor.sValue = m_sStandard;
        }

        //////////////////////////////////////////////////////////////////////

        TypeClass eType = _rProperty.Type.getTypeClass();

        sal_Bool bFilter = sal_True;
        switch ( nPropId )
        {
        case PROPERTY_ID_DEFAULT_SELECT_SEQ:
        case PROPERTY_ID_SELECTEDITEMS:
            _rDescriptor.nButtonHelpId = UID_PROP_DLG_SELECTION;
            break;

        case PROPERTY_ID_FILTER:
            _rDescriptor.nButtonHelpId = UID_PROP_DLG_FILTER;
            break;

        case PROPERTY_ID_SORT:
            _rDescriptor.nButtonHelpId = UID_PROP_DLG_ORDER;
            break;

        case PROPERTY_ID_MASTERFIELDS:
        case PROPERTY_ID_DETAILFIELDS:
            if ( !m_bInspectingSubForm )
                // no master and detail fields for forms which are no sub forms
                return false;

            _rDescriptor.eControlType = BCT_LEDIT;
            bFilter = sal_False;
            _rDescriptor.nButtonHelpId = UID_PROP_DLG_FORMLINKFIELDS;
            break;

        case PROPERTY_ID_COMMAND:
            _rDescriptor.nButtonHelpId = UID_PROP_DLG_SQLCOMMAND;
            break;

        case PROPERTY_ID_TABINDEX:
        {
            if ( m_xControlsView.is() )
                _rDescriptor.nButtonHelpId = UID_PROP_DLG_TABINDEX;
            _rDescriptor.eControlType = BCT_NUMFIELD;
        };
        break;

        case PROPERTY_ID_FONT_NAME:
        {
            bFilter = sal_False;

            _rDescriptor.sName = String::CreateFromAscii("Font");
            _rDescriptor.sTitle = _rDescriptor.sName;
            _rDescriptor.bIsLocked = sal_True;
            _rDescriptor.nButtonHelpId = UID_PROP_DLG_FONT_TYPE;
            ::rtl::OUString sValue;
            aPropertyValue >>= sValue;
            _rDescriptor.sValue = sValue;
        }
        break;

        case PROPERTY_ID_TARGET_URL:
        case PROPERTY_ID_IMAGE_URL:
        {
            _rDescriptor.eControlType = BCT_USERDEFINED;
            _rDescriptor.pControl = new OFileUrlControl( getPropertyBox(), WB_TABSTOP | WB_BORDER );

            _rDescriptor.nButtonHelpId = ( PROPERTY_ID_TARGET_URL == nPropId )
                ? UID_PROP_DLG_ATTR_TARGET_URL : UID_PROP_DLG_IMAGE_URL;
        }
        break;

        case PROPERTY_ID_ECHO_CHAR:
        {
            _rDescriptor.eControlType = BCT_CHAREDIT;
        }
        break;

        case PROPERTY_ID_BACKGROUNDCOLOR:
        case PROPERTY_ID_FILLCOLOR:
        case PROPERTY_ID_SYMBOLCOLOR:
        case PROPERTY_ID_BORDERCOLOR:
        {
            bFilter = sal_False;
            _rDescriptor.eControlType = BCT_COLORBOX;

            switch( nPropId )
            {
            case PROPERTY_ID_BACKGROUNDCOLOR:
                _rDescriptor.nButtonHelpId = UID_PROP_DLG_BACKGROUNDCOLOR; break;
            case PROPERTY_ID_FILLCOLOR:
                _rDescriptor.nButtonHelpId = UID_PROP_DLG_FILLCOLOR; break;
            case PROPERTY_ID_SYMBOLCOLOR:
                _rDescriptor.nButtonHelpId = UID_PROP_DLG_SYMBOLCOLOR; break;
            case PROPERTY_ID_BORDERCOLOR:
                _rDescriptor.nButtonHelpId = UID_PROP_DLG_BORDERCOLOR; break;
            }
        }
        break;

        case PROPERTY_ID_LABEL:
        {
            _rDescriptor.eControlType = BCT_MEDIT;
        }
        break;

        case PROPERTY_ID_DEFAULT_TEXT:
        {
            if (FormComponentType::FILECONTROL == m_nClassId)
                _rDescriptor.eControlType = BCT_EDIT;
            else
                _rDescriptor.eControlType = BCT_MEDIT;
        }
        break;

        case PROPERTY_ID_TEXT:
        {
            if ( ControlType::FORMATTEDFIELD == m_nClassId )
                return false;

            if ( haveIntrospecteeProperty( PROPERTY_MULTILINE ) )
                _rDescriptor.eControlType = BCT_MEDIT;
        }
        break;

        case PROPERTY_ID_CONTROLLABEL:
        {
            bFilter = sal_False;
            _rDescriptor.bIsLocked = sal_True;
            _rDescriptor.sValue = getStringRepFromPropertyValue( PROPERTY_ID_CONTROLLABEL, aPropertyValue );
            _rDescriptor.nButtonHelpId = UID_PROP_DLG_CONTROLLABEL;
        }
        break;

        case PROPERTY_ID_FORMATKEY:
        case PROPERTY_ID_EFFECTIVE_MIN:
        case PROPERTY_ID_EFFECTIVE_MAX:
        case PROPERTY_ID_EFFECTIVE_DEFAULT:
        case PROPERTY_ID_EFFECTIVE_VALUE:
        {
            // only if the set has a formatssupplier, too
            if  (   !haveIntrospecteeProperty( PROPERTY_FORMATSSUPPLIER )
                ||  (FormComponentType::DATEFIELD == m_nClassId)
                ||  (FormComponentType::TIMEFIELD == m_nClassId)
                )
                return false;

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

                    _rDescriptor.eControlType = BCT_USERDEFINED;

                    _rDescriptor.bIsLocked = bIsFormatKey;

                    if (bIsFormatKey)
                    {
                        _rDescriptor.pControl = new OFormatDescriptionControl(getPropertyBox(), WB_READONLY | WB_TABSTOP | WB_BORDER);
                            // HACK : the Control need's a non-null parent, but we don't have one ... so use the property box
                        ((OFormatDescriptionControl*)_rDescriptor.pControl)->SetFormatSupplier(pSupplier);

                        _rDescriptor.nButtonHelpId = UID_PROP_DLG_NUMBER_FORMAT;
                    }
                    else
                    {
                        _rDescriptor.pControl = new OFormattedNumericControl(getPropertyBox(), WB_TABSTOP | WB_BORDER);
                            // HACK : same as above

                        FormatDescription aDesc;
                        aDesc.pSupplier = pSupplier;
                        Any aFormatKeyValue = m_xPropValueAccess->getPropertyValue(PROPERTY_FORMATKEY);
                        if ( !( aFormatKeyValue >>= aDesc.nKey ) )
                            aDesc.nKey = 0;
                        static_cast< OFormattedNumericControl* >( _rDescriptor.pControl )->SetFormatDescription( aDesc );
                    }

                    // the initial value
                    if ( aPropertyValue.hasValue() )
                    {
                        if (bIsFormatKey)
                        {
                            _rDescriptor.sValue = String::CreateFromInt32( ::comphelper::getINT32( aPropertyValue ) );
                        }
                        else
                        {
                            if ( aPropertyValue.getValueTypeClass() == TypeClass_DOUBLE )
                            {
                                StringRepresentation aConversionHelper( m_xTypeConverter );
                                _rDescriptor.sValue = aConversionHelper.convertSimpleToString( aPropertyValue );
                            }
                            else
                                DBG_WARNING("OPropertyBrowserController::describePropertyLine : non-double values not supported for Effective*-properties !");
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
            _rDescriptor.eControlType = BCT_DATEFIELD;
            break;

        case PROPERTY_ID_TIMEMIN:
        case PROPERTY_ID_TIMEMAX:
        case PROPERTY_ID_DEFAULT_TIME:
        case PROPERTY_ID_TIME:
            _rDescriptor.eControlType = BCT_TIMEFIELD;
            break;

        case PROPERTY_ID_VALUEMIN:
        case PROPERTY_ID_VALUEMAX:
        case PROPERTY_ID_DEFAULT_VALUE:
        case PROPERTY_ID_VALUE:
            {
                _rDescriptor.eControlType = BCT_USERDEFINED;
                _rDescriptor.pControl = new OFormattedNumericControl( getPropertyBox(), WB_TABSTOP | WB_BORDER | WB_SPIN | WB_REPEAT );
                    // HACK : same as above

                // we don't set a formatter so the control uses a default (which uses the application
                // language and a default numeric format)
                // but we set the decimal digits
                static_cast< OFormattedNumericControl* >( _rDescriptor.pControl )->SetDecimalDigits(
                    ::comphelper::getINT16( m_xPropValueAccess->getPropertyValue( PROPERTY_DECIMAL_ACCURACY ) )
                );

                // and the thousands separator
                static_cast< OFormattedNumericControl* >( _rDescriptor.pControl )->SetThousandsSep(
                    ::comphelper::getBOOL( m_xPropValueAccess->getPropertyValue(PROPERTY_SHOWTHOUSANDSEP) )
                );

                // and the default value for the property
                try
                {
                    if (m_xPropStateAccess.is() && ((PROPERTY_ID_VALUEMIN == nPropId) || (PROPERTY_ID_VALUEMAX == nPropId)))
                    {
                        double nDefault = 0;
                        if ( m_xPropStateAccess->getPropertyDefault( _rProperty.Name ) >>= nDefault )
                            static_cast< OFormattedNumericControl* >( _rDescriptor.pControl )->SetDefaultValue( nDefault );
                    }
                }
                catch (Exception&)
                {
                    // just ignore it
                }

                // and allow empty values only for the default value and the value
                static_cast< OFormattedNumericControl* >( _rDescriptor.pControl )
                    ->EnableEmptyField( ( PROPERTY_ID_DEFAULT_VALUE == nPropId )
                                    ||  ( PROPERTY_ID_VALUE == nPropId ) );
            }
            break;

        default:
            if ( bNeedGenericUIDescriptor && ( TypeClass_BYTE <= eType && eType <= TypeClass_DOUBLE ) )
            {
                if ( _nControlType == CONTROL_TYPE_FORM )
                {
                    if  ( nPropId == PROPERTY_ID_HEIGHT )
                        _rDescriptor.nDigits = 1;

                    if  (  ( nPropId == PROPERTY_ID_WIDTH )
                        || ( nPropId == PROPERTY_ID_ROWHEIGHT )
                        )
                    {
                        _rDescriptor.eValueUnit = FUNIT_CM;
                        _rDescriptor.eDisplayUnit = getDocumentMeasurementUnit();
                        _rDescriptor.nDigits = 2;
                    }
                }

                _rDescriptor.eControlType = BCT_NUMFIELD;
            }
            break;
        }

        //////////////////////////////////////////////////////////////////////
        if ( eType == TypeClass_SEQUENCE )
        {
            _rDescriptor.eControlType = BCT_LEDIT;
            bFilter = sal_False;
        }

        // don't filter dialog controls
        if ( _nControlType == CONTROL_TYPE_DIALOG )
            bFilter = sal_False;

        //////////////////////////////////////////////////////////////////////
        // Filter
        if ( bFilter )
        {
            bool bRemoveProperty = false;

            if ( !bRemoveProperty )
            {
                if  (  (  ( aPropertyValue.getValueTypeClass() == TypeClass_INTERFACE )
                       || ( aPropertyValue.getValueTypeClass() == TypeClass_ARRAY )
                       || ( aPropertyValue.getValueTypeClass() == TypeClass_UNKNOWN )
                       )
                    && !bDedicatedHandler
                    )
                    // we can only care for those "complex" property types, if we have a handler
                    // which we can delegate this too
                    bRemoveProperty = true;
            }

            if ( !bRemoveProperty )
            {
                if  (  ( aPropertyValue.getValueTypeClass() == TypeClass_VOID )
                    && ( _rProperty.Attributes & PropertyAttribute::MAYBEVOID ) == 0
                    )
                {
#if OSL_DEBUG_LEVEL > 0
                    ::rtl::OString sMessage( "OPropertyBrowserController::describePropertyLine:" );
                    sMessage += "\n";
                    sMessage += "\nthe following property has a suspicious value:";
                    sMessage += "\nprogrammatic name: ";
                    sMessage += ::rtl::OString( _rProperty.Name.getStr(), _rProperty.Name.getLength(), RTL_TEXTENCODING_ASCII_US );
                    sMessage += "\nUI name: ";
                    sMessage += ::rtl::OString( sDisplayName.GetBuffer(), sDisplayName.Len(), RTL_TEXTENCODING_ASCII_US );
                    sMessage += "\n";
                    sMessage += "\nThe property currently has no value, but it is not declared as MAYBEVOID.";
                    sMessage += "\n";
                    DBG_ERROR( sMessage.getStr() );
#endif
                    bRemoveProperty = true;
                }
            }

            if ( !bRemoveProperty )
            {
                if ( _rProperty.Attributes & PropertyAttribute::TRANSIENT )
                    bRemoveProperty = true;
            }

            if ( !bRemoveProperty )
            {
                if ( _rProperty.Attributes & PropertyAttribute::READONLY )
                    bRemoveProperty = true;
            }

            if ( bRemoveProperty )
            {
                delete _rDescriptor.pControl;
                return false;
            }
        }

        //////////////////////////////////////////////////////////////////////
        // boolean values
        if ( bNeedGenericUIDescriptor )
        {
            if ( eType == TypeClass_BOOLEAN )
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
                    _rDescriptor.aListValues.push_back( aEntries.GetToken(i) );

                _rDescriptor.eControlType = BCT_LISTBOX;
            }
        }
        // void properties
        else if ( eType == TypeClass_VOID )
        {
            DBG_ERROR( "OPropertyBrowserController::describePropertyLine: Huh? A property of type void?" );
                // me thinks this should not happen - but is was in there before the code was refactored ... fs@openoffice.org
            _rDescriptor.sValue = String();
        }

        //////////////////////////////////////////////////////////////////////
        // enum properties
        if ( bNeedGenericUIDescriptor && ( bIsEnumProperty || ( PROPERTY_ID_TARGET_FRAME == nPropId ) ) )
        {
            ::std::vector< String > aEnumValues = m_pPropertyInfo->getPropertyEnumRepresentations( nPropId );
            ::std::vector< String >::const_iterator pStart = aEnumValues.begin();
            ::std::vector< String >::const_iterator pEnd = aEnumValues.end();

            // for a checkbox: if "ambiguous" is not allowed, remove this from the sequence
            if ( PROPERTY_ID_DEFAULTCHECKED == nPropId || PROPERTY_ID_STATE == nPropId )
                if ( haveIntrospecteeProperty( PROPERTY_TRISTATE ) )
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
                _rDescriptor.aListValues.push_back( *pLoop );

            _rDescriptor.eControlType =
                    PROPERTY_ID_TARGET_FRAME == nPropId
                ?   BCT_COMBOBOX
                :   BCT_LISTBOX;
        }

        //////////////////////////////////////////////////////////////////////
        switch( nPropId )
        {
            case PROPERTY_ID_REPEAT_DELAY:
                _rDescriptor.nMaxValue = 0x7FFFFFFF;
                _rDescriptor.bHaveMinMax = sal_True;
                _rDescriptor.nMaxValue = 0;

                _rDescriptor.eControlType = BCT_USERDEFINED;
                _rDescriptor.pControl = new TimeDurationInput( getPropertyBox(), WB_BORDER | WB_TABSTOP );
                break;

            case PROPERTY_ID_TABINDEX:
            case PROPERTY_ID_BOUNDCOLUMN:
            case PROPERTY_ID_VISIBLESIZE:
            case PROPERTY_ID_MAXTEXTLEN:
            case PROPERTY_ID_LINEINCREMENT:
            case PROPERTY_ID_BLOCKINCREMENT:
            case PROPERTY_ID_SPININCREMENT:
                _rDescriptor.nMaxValue = 0x7FFFFFFF;
                _rDescriptor.bHaveMinMax = sal_True;

                if ( nPropId == PROPERTY_ID_MAXTEXTLEN )
                    _rDescriptor.nMinValue = -1;
                else if ( ( nPropId == PROPERTY_ID_BOUNDCOLUMN ) || ( nPropId == PROPERTY_ID_VISIBLESIZE ) )
                    _rDescriptor.nMinValue = 1;
                else
                    _rDescriptor.nMinValue = 0;
                break;

            case PROPERTY_ID_DECIMAL_ACCURACY:
                _rDescriptor.nMaxValue = 20;
                _rDescriptor.nMinValue = 0;
                _rDescriptor.bHaveMinMax = sal_True;
                break;

            //////////////////////////////////////////////////////////////////////
            // DataSource
            case PROPERTY_ID_DATASOURCE:
            {
                _rDescriptor.eControlType = BCT_COMBOBOX;
                _rDescriptor.nButtonHelpId = UID_PROP_DLG_ATTR_DATASOURCE;

                Reference< XConnection > xConnection;
                if ( m_xPropValueAccess.is() && ::dbtools::isEmbeddedInDatabase( m_xPropValueAccess, xConnection ) )
                {
                    getPropertyBox()->EnablePropertyLine( PROPERTY_DATASOURCE, sal_False );
                }

                Reference< XNameAccess > xDatabaseContext(m_xORB->createInstance(SERVICE_DATABASE_CONTEXT), UNO_QUERY);
                if (xDatabaseContext.is())
                {
                    Sequence< ::rtl::OUString > aDatasources = xDatabaseContext->getElementNames();
                    const ::rtl::OUString* pBegin = aDatasources.getConstArray();
                    const ::rtl::OUString* pEnd = pBegin + aDatasources.getLength();

                    ::std::vector< ::rtl::OUString > aPrevious( _rDescriptor.aListValues.size() );
                    ::std::copy( _rDescriptor.aListValues.begin(), _rDescriptor.aListValues.end(), aPrevious.begin() );


                    _rDescriptor.aListValues.resize( aPrevious.size() + aDatasources.getLength() );
                    ::std::copy(
                        aPrevious.begin(),
                        aPrevious.end(),
                        ::std::copy( pBegin, pEnd, _rDescriptor.aListValues.begin() )
                    );
                }
            }
            break;

            case PROPERTY_ID_CONTROLSOURCE:
                SetFields( _rDescriptor );
                break;

            case PROPERTY_ID_COMMAND:
                m_bHasCursorSource = sal_True;
                break;

            case PROPERTY_ID_LISTSOURCE:
                m_bHasListSource = sal_True;
                break;
        }

        //////////////////////////////////////////////////////////////////////
        _rDescriptor.nHelpId = m_pPropertyInfo->getPropertyHelpId( nPropId );
        _rDescriptor.sTitle = sDisplayName;

        if ( PropertyState_AMBIGUOUS_VALUE == ePropertyState )
        {
            _rDescriptor.bUnknownValue = sal_True;
            _rDescriptor.sValue = String();
        }

        return true;
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
            sal_Bool bRemoveDataPage=sal_True;

            // get control type
            sal_Int16 nControlType = getControlType();

            ::std::vector< sal_Int32 > aActuatingProperties;
            ::std::vector< Any > aActuatingPropertyValues;
            for (sal_uInt32 i=0; i<nPropCount; ++i, ++pProps)
            {
                // for the MultiLine property, we have different UI translations depending on the control
                // type
                ::rtl::OUString sPropertyName( pProps->Name );
                if ( sPropertyName == PROPERTY_MULTILINE )
                {
                    if  (  ( m_nClassId == FormComponentType::FIXEDTEXT )
                        || ( m_nClassId == FormComponentType::COMMANDBUTTON )
                        || ( m_nClassId == FormComponentType::RADIOBUTTON )
                        || ( m_nClassId == FormComponentType::CHECKBOX )
                        )
                        sPropertyName = PROPERTY_WORDBREAK;
                }

                OLineDescriptor aDescriptor;
                if ( !describePropertyLine( nControlType, *pProps, aDescriptor ) )
                    continue;

                sal_Int32   nPropertyId = m_pPropertyInfo->getPropertyId( sPropertyName );
                sal_uInt32  nPropertyUIFlags = m_pPropertyInfo->getPropertyUIFlags( nPropertyId );
                bool bIsDataProperty         = ( nPropertyUIFlags & PROP_FLAG_DATA_PROPERTY  ) != 0;
                bool bIsActuatingProperty    = ( nPropertyUIFlags & PROP_FLAG_ACTUATING      ) != 0;
                bool bIsExperimentalProperty = ( nPropertyUIFlags & PROP_FLAG_EXPERIMENTAL   ) != 0;

                // don't show experimental properties unless allowed to do so
                if ( bIsExperimentalProperty )
                {
                    if ( true ) // TODO
                        continue;
                }

                //////////////////////////////////////////////////////////////////////
                if ( bIsDataProperty )
                {
                    bRemoveDataPage = sal_False;
                    getPropertyBox()->SetPage( m_nDataPageId );
                }
                else
                {
                    getPropertyBox()->SetPage( m_nGenericPageId );
                }

                // finally insert this property control
                getPropertyBox()->InsertEntry( aDescriptor );

                // if it's an actuating property, remember it
                if ( bIsActuatingProperty )
                {
                    aActuatingProperties.push_back( nPropertyId );
                    aActuatingPropertyValues.push_back( GetAnyPropertyValue( sPropertyName ) );
                }
            }

            // update any dependencies for the actuating properties which we encountered
            {
                ::std::vector< sal_Int32 >::const_iterator aProperty = aActuatingProperties.begin();
                ::std::vector< Any >::const_iterator aPropertyValue = aActuatingPropertyValues.begin();
                for ( ; aProperty != aActuatingProperties.end(); ++aProperty, ++aPropertyValue )
                    actuatingPropertyChanged( *aProperty, *aPropertyValue, *aPropertyValue, true );
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
    void OPropertyBrowserController::Modified( const String& aName, const String& aVal )
    {
        try
        {
            sal_Int32 nPropId = m_pPropertyInfo->getPropertyId( aName );

            // Wenn CursorSourceType veraendert wurde, CursorSource anpassen
            if (PROPERTY_ID_COMMANDTYPE == nPropId)
            {
                Commit( aName, aVal );
                SetCursorSource( sal_False, sal_False );
            }

            //////////////////////////////////////////////////////////////////////
            // Wenn ListSourceType veraendert wurde, ListSource anpassen
            if (PROPERTY_ID_LISTSOURCETYPE == nPropId)
            {
                Commit( aName, aVal );
                SetListSource();
            }
        }
        catch (Exception&)
        {
            DBG_ERROR("OPropertyBrowserController::Modified : caught an exception !")
        }
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserController::OnImageURLClicked( const String& _rName, const String& _rVal )
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
                Commit( _rName, aFileDlg.GetPath() );
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserController::Clicked( const String& aName, sal_uInt16 _nFlags, bool _bPrimary )
    {
        try
        {
            // since the browse buttons do not get the focus when clicked with the mouse,
            // we need to commit the changes in the current property field
            getPropertyBox()->CommitModified();

            sal_Int32 nPropId = m_pPropertyInfo->getPropertyId( aName );

            // do we have a dedicated handler do delegate this to?
            PropertyHandlerRepository::const_iterator aHandlerPos = m_aPropertyHandlers.find( nPropId );
            if ( aHandlerPos != m_aPropertyHandlers.end() )
            {
                Any aData;
                if ( aHandlerPos->second->requestUserInputOnButtonClick( nPropId, _bPrimary, aData ) )
                    aHandlerPos->second->executeButtonClick( nPropId, _bPrimary, aData, this );
                return;
            }

            DBG_ASSERT( _bPrimary, "OPropertyBrowserController::Clicked: no properties here which support two buttons!" );

            if ( _nFlags & LINE_FLAG_EVENT )
            {
                ChangeEventProperty( aName );
                return;
            }

            ::rtl::OUString sStringValue = GetPropertyValueStringRep( aName );

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

                INetURLObject aParser( sStringValue );
                if ( INET_PROT_FILE == aParser.GetProtocol() )
                    // set the initial directory only for file-URLs. Everything else
                    // is considered to be potentially expensive
                    // 106126 - 2002/12/10 - fs@openoffice.org
                    aFileDlg.SetDisplayDirectory( sStringValue );

                if (0 == aFileDlg.Execute())
                {
                    String aDataSource = aFileDlg.GetPath();
                    Commit( aName, aDataSource );
                }
            }
            break;

            case PROPERTY_ID_IMAGE_URL:
                OnImageURLClicked( aName, sStringValue );
                break;

            //////////////////////////////////////////////////////////////////////
            // Bei Datenquelle auch Cursor-/ListSource fuellen
            case PROPERTY_ID_DATASOURCE:
            {
                ::sfx2::FileDialogHelper aFileDlg(WB_3DLOOK);

                INetURLObject aParser( sStringValue );
                if ( INET_PROT_FILE == aParser.GetProtocol() )
                    // set the initial directory only for file-URLs. Everything else
                    // is considered to be potentially expensive
                    // 106126 - 2002/12/10 - fs@openoffice.org
                    aFileDlg.SetDisplayDirectory( sStringValue );

                static const String s_sDatabaseType = String::CreateFromAscii("StarOffice XML (Base)");
                const SfxFilter* pFilter = SfxFilter::GetFilterByName( s_sDatabaseType);
                OSL_ENSURE(pFilter,"Filter: StarOffice XML (Base) could not be found!");
                if ( pFilter )
                {
                    aFileDlg.AddFilter(pFilter->GetFilterName(),pFilter->GetDefaultExtension());
                }

                if (0 == aFileDlg.Execute())
                {
                    String aDataSource = aFileDlg.GetPath();
                    Commit( aName, aDataSource );
                }
            }
            break;

            //////////////////////////////////////////////////////////////////////
            // Color
            case PROPERTY_ID_BACKGROUNDCOLOR:
            case PROPERTY_ID_FILLCOLOR:
            case PROPERTY_ID_SYMBOLCOLOR:
            case PROPERTY_ID_BORDERCOLOR:
            {
                sal_uInt32 nColor = sStringValue.toInt32();
                ::Color aColor( nColor );
                SvColorDialog aColorDlg( getDialogParent() );
                aColorDlg.SetColor( aColor );

                if (aColorDlg.Execute() )
                {
                    aColor = aColorDlg.GetColor();
                    nColor = aColor.GetColor();

                    String aColorString = String::CreateFromInt32( (sal_Int32)nColor );
                    Commit( aName, aColorString );
                }
            }
            break;

            case PROPERTY_ID_FORMATKEY:
            {
                ChangeFormatProperty( aName, sStringValue );
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
                ChangeFontProperty();
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
    void OPropertyBrowserController::Commit( const String& rName, const String& _rStringValue )
    {
        m_sCommittingProperty = rName;

        try
        {
            //////////////////////////////////////////////////////////////////////
            // Property-Info holen
            const Property* pProp = findObjectPropertyByName( rName );
            DBG_ASSERT( pProp, "OPropertyBrowserController::Commit: where does this come from?" );
            if ( !pProp )
                return;

            sal_Int32   nPropId             = m_pPropertyInfo->getPropertyId( rName );
            sal_uInt32  nPropertyUIFlags    = m_pPropertyInfo->getPropertyUIFlags( nPropId );
            bool bIsActuatingProperty       = ( nPropertyUIFlags & PROP_FLAG_ACTUATING ) != 0;

            Any aOldValue;
            if ( bIsActuatingProperty )
                aOldValue = GetAnyPropertyValue( rName );

            String sNormalizedStringValue = _rStringValue;

            // do we have a dedicated handler for this property, which we can delegate some tasks to?
            PropertyHandlerRepository::const_iterator aHandlerPos = m_aPropertyHandlers.find( nPropId );
            bool bDedicatedHandler = aHandlerPos != m_aPropertyHandlers.end();

            //////////////////////////////////////////////////////////////////////
            // URL properties must be made absolute
            if  (   ( ( nPropId == PROPERTY_ID_TARGET_URL )
                   || ( nPropId == PROPERTY_ID_IMAGE_URL )
                    )
                && _rStringValue.Len()
                )
            {
                INetURLObject aDocURL( getDocumentURL() );
                sNormalizedStringValue = URIHelper::SmartRel2Abs( aDocURL, _rStringValue, Link(), false, true, INetURLObject::WAS_ENCODED, INetURLObject::DECODE_TO_IURI );
            }

            Any aValue;
            if (  !m_sStandard.equals( _rStringValue )
               || ( 0 == ( pProp->Attributes & PropertyAttribute::MAYBEVOID ) )
               )
            {
                if ( bDedicatedHandler )
                    aValue = aHandlerPos->second->getPropertyValueFromStringRep( nPropId, sNormalizedStringValue );
                else
                    aValue = getPropertyValueFromStringRep( sNormalizedStringValue, *pProp, nPropId);
            }

            if  (   (   ( nPropId == PROPERTY_ID_DEFAULT_VALUE )
                    ||  ( nPropId == PROPERTY_ID_VALUE )
                    ||  ( nPropId == PROPERTY_ID_DEFAULT_DATE )
                    ||  ( nPropId == PROPERTY_ID_DATE )
                    ||  ( nPropId == PROPERTY_ID_DEFAULT_TIME )
                    ||  ( nPropId == PROPERTY_ID_TIME )
                    ||  ( nPropId == PROPERTY_ID_BOUNDCOLUMN )
                    )
                &&  ( 0 == _rStringValue.Len() )
                )
            {
                aValue = Any();
            }

            //////////////////////////////////////////////////////////////////////
            // set the value
            if ( bDedicatedHandler )
            {
                aHandlerPos->second->setPropertyValue( nPropId, aValue );
                setDocumentModified();
            }
            else
            {
                sal_Bool bDontForwardToPropSet =
                        ( ( pProp->Attributes & PropertyAttribute::MAYBEVOID ) == 0 )    // VOID is not allowed
                    &&  !aValue.hasValue();                                             // but it *is* void

                if ( PROPERTY_ID_CONTROLLABEL == nPropId )
                    // the string fo the control label is not to be set as PropertyValue, it's only for displaying
                    bDontForwardToPropSet = sal_True;

                if ( !bDontForwardToPropSet )
                    m_xPropValueAccess->setPropertyValue( rName, aValue );
            }

            // re-retrieve the value
            ::rtl::OUString sNewStrVal;
            if ( bDedicatedHandler )
            {
                aValue = aHandlerPos->second->getPropertyValue( nPropId );
                sNewStrVal = aHandlerPos->second->getStringRepFromPropertyValue( nPropId, aValue );
            }
            else
            {
                aValue = m_xPropValueAccess->getPropertyValue( rName );
                sNewStrVal = getStringRepFromPropertyValue( nPropId, aValue );
            }

            // care for any inter-property dependencies
            if ( bIsActuatingProperty )
                actuatingPropertyChanged( nPropId, aValue, aOldValue, false );

            // and display it again. This ensures proper formatting
            getPropertyBox()->SetPropertyValue( rName, sNewStrVal );

            // TODO: I think all of the stuff below can be moved into actuatingPropertyChanged
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

                getPropertyBox()->ChangeEntry( aProperty );

                Commit( aProperty.sName, aProperty.sValue );
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
                aNewDesc.nKey = _rStringValue.ToInt32();
                    // nKey will be zero if _rStringValue is empty or standard

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
                Any aValue = getPropertyValueFromStringRep( sNormalizedStringValue, *pProp, nPropId);

                sal_Bool bFlag= !(pProp->Attributes & PropertyAttribute::MAYBEVOID) && !aValue.hasValue();

                if (!bFlag)
                {
                    ::rtl::OUString sDataSource;
                    aValue >>= sDataSource;
                    Reference< XNameAccess > xDatabaseContext(m_xORB->createInstance(SERVICE_DATABASE_CONTEXT), UNO_QUERY);
                    if ( sDataSource.getLength() && ( !xDatabaseContext.is() || !xDatabaseContext->hasByName(sDataSource) ) )
                    {
                        ::svt::OFileNotation aTransformer(sDataSource);
                        sDataSource = aTransformer.get(::svt::OFileNotation::N_URL);
                        aValue <<= sDataSource;
                    }
                    m_xPropValueAccess->setPropertyValue(rName, aValue );
                }

                if (m_xPropStateAccess.is()&& !aValue.hasValue())
                {
                    m_xPropStateAccess->setPropertyToDefault(rName);
                }

                SetCursorSource( sal_True, sal_True );
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

        m_sCommittingProperty = ::rtl::OUString();
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserController::getPropertyHandlers( PropertyHandlerArray& _rHandlers )
    {
        DBG_CHKTHIS( OPropertyBrowserController, CheckPropertyBrowserInvariants );

        _rHandlers.resize( 0 );

        // factories for all known handlers
        HandlerFactory aFactories[] = {

            // a handler which introduces virtual properties for binding controls to spreadsheet cells
            &CellBindingPropertyHandler::Create,

            // properties related to binding to an XForms DOM node
            &EFormsPropertyHandler::Create,

            // properties related to the XSD data against which a control content is validated
            &XSDValidationPropertyHandler::Create,

            // a handler which cares for XForms submissions
            &SubmissionPropertyHandler::Create,

            // generic virtual edit properties
            &EditPropertyHandler::Create,

            // a handler which virtualizes the ButtonType property, to provide additional types like
            // "move to next record"
            ButtonNavigationHandler::Create
        };

        for ( sal_Int32 i = 0; i < sizeof( aFactories ) / sizeof( aFactories[ 0 ] ); ++i )
        {
            if ( m_aIntrospectedCollection.getLength() == 1 )
            {   // we're inspecting only one object -> one handler
                _rHandlers.push_back( (*aFactories[i])( m_xORB, m_xIntrospecteeAsProperty, m_xContextDocument, m_xTypeConverter ) );
            }
            else
            {
                // create a single handler for every single object
                ::std::vector< ::rtl::Reference< IPropertyHandler > > aSingleHandlers( m_aIntrospectedCollection.getLength() );
                ::std::vector< ::rtl::Reference< IPropertyHandler > >::iterator pHandler = aSingleHandlers.begin();

                const Reference< XPropertySet >* pObject = m_aIntrospectedCollection.getConstArray();

                for ( ; pHandler != aSingleHandlers.end(); ++pHandler, ++pObject )
                    *pHandler = (*aFactories[i])( m_xORB, *pObject, m_xContextDocument, m_xTypeConverter );

                // then create a handler which composes information out of those single handlers
                _rHandlers.push_back( new PropertyComposer( aSingleHandlers ) );
            }
        }

        // note that the handlers will not be used by our caller, if they indicate that there are no
        // properties they feel responsible for
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

    //------------------------------------------------------------------------
    sal_uInt16 OPropertyBrowserController::getTargetPageId( const ::rtl::OUString& _rPropertyName )
    {
        sal_uInt32 nFlags = m_pPropertyInfo->getPropertyUIFlags( m_pPropertyInfo->getPropertyId( _rPropertyName ) );
        return ( nFlags & PROP_FLAG_DATA_PROPERTY ) ? m_nDataPageId : m_nGenericPageId;
            // events currently not covered by this mechanism
    }

    //------------------------------------------------------------------------
    const Property* OPropertyBrowserController::findObjectPropertyByName( const ::rtl::OUString& _rName )
    {
        return findObjectPropertyByHandle( m_pPropertyInfo->getPropertyId( _rName ) );
    }

    //------------------------------------------------------------------------
    const Property* OPropertyBrowserController::findObjectPropertyByHandle( sal_Int32 _nHandle )
    {
        const Property* pObjectPropsStart = m_aObjectProperties.getConstArray();
        const Property* pObjectPropsEnd   = m_aObjectProperties.getConstArray() + m_aObjectProperties.getLength();
        const Property* pObjectPropsPos   = ::std::find_if( pObjectPropsStart, pObjectPropsEnd, FindPropertyByHandle( _nHandle ) );

        if ( pObjectPropsPos != pObjectPropsEnd )
            return pObjectPropsPos;
        return NULL;
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserController::rebuildPropertyUI( const ::rtl::OUString& _rPropertyName )
    {
        // obtain the Property structure for the property name
        sal_Int32 nPropertyId = m_pPropertyInfo->getPropertyId( _rPropertyName );

        const Property* pProperty = findObjectPropertyByHandle( nPropertyId );
        DBG_ASSERT( pProperty, "OPropertyBrowserController::rebuildPropertyUI: our current introspectee does not support this property!" );
        if ( !pProperty )
            return;

        OLineDescriptor aDescriptor;
        bool bSuccess = false;
        try
        {
            bSuccess = describePropertyLine( getControlType(), *pProperty, aDescriptor );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "OPropertyBrowserController::rebuildPropertyUI: caught an exception!" );
        }
        if ( !bSuccess )
            return;

        getPropertyBox()->ChangeEntry( aDescriptor );

        // since the UI for this entry changed, give the handler (if there is one) a chance
        // to re-initialize
        PropertyHandlerRepository::const_iterator aHandlerPos = m_aPropertyHandlers.find( nPropertyId );
        if ( aHandlerPos != m_aPropertyHandlers.end() )
            aHandlerPos->second->initializePropertyUI( nPropertyId, this );
   }

    //------------------------------------------------------------------------
    void OPropertyBrowserController::enablePropertyUI( const ::rtl::OUString& _rPropertyName, bool _bEnable )
    {
        DBG_ASSERT( getPropertyBox(), "OPropertyBrowserController::enablePropertyUI: how did you reach this?!" );
        if ( !getPropertyBox() )
            return;

        getPropertyBox()->EnablePropertyLine( _rPropertyName, _bEnable );
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserController::enablePropertyButtons( const ::rtl::OUString& _rPropertyName, bool _bEnablePrimary, bool _bEnableSecondary )
    {
        getPropertyBox()->EnablePropertyControls( _rPropertyName, getPropertyBox()->IsPropertyInputEnabled( _rPropertyName ), _bEnablePrimary, _bEnableSecondary );
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserController::showPropertyUI( const ::rtl::OUString& _rPropertyName, bool _bRefreshIfExistent )
    {
        if ( getPropertyBox()->GetPropertyPos( _rPropertyName ) != LISTBOX_ENTRY_NOTFOUND )
        {
            if ( _bRefreshIfExistent )
                rebuildPropertyUI( _rPropertyName );
            return;
        }

        // look up the property in our object properties
        const Property* pProperty = findObjectPropertyByName( _rPropertyName  );
        DBG_ASSERT( pProperty, "OPropertyBrowserController::showPropertyUI: There is no such property!" );
        if ( !pProperty )
            return;

        OLineDescriptor aDescriptor;
        if ( !describePropertyLine( getControlType(), *pProperty, aDescriptor ) )
            return;

        // look for the position to insert the property

        // side note: The methods GetPropertyPos and InsertEntry of the OPropertyEditor work
        // only on the current page. This implies that it's impossible to use this method here
        // to show property lines which are *not* on the current page.
        // This is sufficient for now, but should be changed in the future.

        // by definition, the properties in m_aObjectProperties are in the order in which they appear in the UI
        // So all we need is a predecessor of pProperty in m_aObjectProperties
        size_t nPosition = pProperty - m_aObjectProperties.getConstArray();
        sal_uInt16 nUIPos = LISTBOX_ENTRY_NOTFOUND;
        while ( ( nUIPos == LISTBOX_ENTRY_NOTFOUND ) && nPosition-- )
        {
            nUIPos = getPropertyBox()->GetPropertyPos( m_aObjectProperties[ nPosition ].Name );
        }
        if ( nUIPos == LISTBOX_ENTRY_NOTFOUND )
            // insert at the very top
            nUIPos = 0;
        else
            // insert right after the predecessor we found
            ++nUIPos;

        getPropertyBox()->InsertEntry( aDescriptor, nUIPos, getTargetPageId( _rPropertyName ) );
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserController::hidePropertyUI( const ::rtl::OUString& _rPropertyName )
    {
        getPropertyBox()->RemoveEntry( _rPropertyName );
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserController::showCategory( EPropertyCategory _eCategory, bool _bShow )
    {
        sal_uInt16 nPageId = 0;
        switch( _eCategory )
        {
        case eGeneral:  nPageId = m_nGenericPageId;  break;
        case eData:     nPageId = m_nDataPageId;     break;
        case eEvents:   nPageId = m_nEventPageId;    break;
        default:
            OSL_ENSURE( sal_False, "OPropertyBrowserController::showCategory: invalid category!" );
        }
        if ( nPageId )
            getPropertyBox()->ShowPropertyPage( nPageId, _bShow );
    }

#define STRING_ARRAY_LIMITS( array ) \
    array, \
    array + sizeof( array ) / sizeof( array[0] )

    //------------------------------------------------------------------------
    void OPropertyBrowserController::actuatingPropertyChanged( sal_Int32 _nPropId, const Any& _rNewValue, const Any& _rOldValue, bool _bFirstTimeInit )
    {
        DBG_ASSERT( getPropertyBox(), "OPropertyBrowserController::actuatingPropertyChanged: no view!" );
        if ( !getPropertyBox() )
            return;

        // are there one or more handlers which are interested in the actuation?
        ::std::pair< PropertyHandlerMultiRepository::iterator, PropertyHandlerMultiRepository::iterator > aInterestedHandlers =
            m_aDependencyHandlers.equal_range( _nPropId );
        if ( aInterestedHandlers.first != aInterestedHandlers.second )
        {
            PropertyHandlerMultiRepository::iterator aLoop = aInterestedHandlers.first;
            while ( aLoop != aInterestedHandlers.second )
            {
                aLoop->second->actuatingPropertyChanged( _nPropId, _rNewValue, _rOldValue, this, _bFirstTimeInit );
                ++aLoop;
            }

            // leave - as soon as any handler expresses interest in a certain actuating property,
            // our "generic" code below will not be triggered anymore
            return;
        }

        // some properties depend on more than a single property changed, they will be updated
        // below
        ::std::vector< ::rtl::OUString > aComplexDependentProperties;
        aComplexDependentProperties.reserve( 5 );   // guessed

        switch ( _nPropId )
        {
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
                PropertyHandlerRepository::const_iterator aCellExchangeHandler = m_aPropertyHandlers.find( PROPERTY_ID_CELL_EXCHANGE_TYPE );
                if ( aCellExchangeHandler != m_aPropertyHandlers.end() )
                    getPropertyBox()->SetPropertyValue(
                        PROPERTY_CELL_EXCHANGE_TYPE,
                        aCellExchangeHandler->second->getStringRepFromPropertyValue(
                            PROPERTY_ID_CELL_EXCHANGE_TYPE,
                            makeAny( (sal_Int16) 0 )
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
            if ( !_bFirstTimeInit )
            {
                try
                {
                    if ( !xSource.is() )
                        m_xPropValueAccess->setPropertyValue( PROPERTY_STRINGITEMLIST, makeAny( Sequence< ::rtl::OUString >() ) );
                }
                catch( const Exception& )
                {
                    OSL_ENSURE( sal_False, "OPropertyBrowserController::actuatingPropertyChanged( ListCellRange ): caught an exception while resetting the string items!" );
                }
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
            DBG_ERROR( "OPropertyBrowserController::actuatingPropertyChanged: this is no actuating property!" );
            break;
        }

        for ( ::std::vector< ::rtl::OUString >::const_iterator aLoop = aComplexDependentProperties.begin();
              aLoop != aComplexDependentProperties.end();
              ++aLoop
            )
        {
            if ( haveIntrospecteeProperty( *aLoop ) )
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

        bool bIsSpreadsheetDocument = CellBindingHelper::isSpreadsheetDocument( m_xContextDocument );
        bool bIsEnabled = true;                 // should the whole property line be en/disabled?
        bool bEnableBrowseButtonOnly = false;   // does |bIsEnabled| hold for the browse button only?

        switch ( m_pPropertyInfo->getPropertyId( _rPropertyName ) )
        {
        case PROPERTY_ID_SCALEIMAGE:
            GetUnoPropertyValue( PROPERTY_CONTROLSOURCE, true ) >>= sStringValue;
            bIsEnabled = sStringValue.getLength() != 0;
            GetAnyPropertyValue( PROPERTY_IMAGE_URL ) >>= sStringValue;
            bIsEnabled |= ( sStringValue.getLength() != 0 );
            break;

        case PROPERTY_ID_FILTERPROPOSAL:
        case PROPERTY_ID_EMPTY_IS_NULL:
            GetAnyPropertyValue( PROPERTY_CONTROLSOURCE ) >>= sStringValue;
            if ( bIsSpreadsheetDocument )
                GetAnyPropertyValue( PROPERTY_BOUND_CELL ) >>= xBinding;
            bIsEnabled = !xBinding.is() && sStringValue.getLength();
            break;

        case PROPERTY_ID_BOUNDCOLUMN:
            GetAnyPropertyValue( PROPERTY_CONTROLSOURCE ) >>= sStringValue;
            if ( bIsSpreadsheetDocument )
            {
                GetAnyPropertyValue( PROPERTY_BOUND_CELL ) >>= xBinding;
                GetAnyPropertyValue( PROPERTY_LIST_CELL_RANGE ) >>= xSource;
            }
            GetAnyPropertyValue( PROPERTY_LISTSOURCETYPE ) >>= eLSType;

            bIsEnabled = ( sStringValue.getLength() > 0 )
                      && ( !xBinding.is() )
                      && ( !xSource.is() )
                      && ( eLSType != ListSourceType_TABLEFIELDS )
                      && ( eLSType != ListSourceType_VALUELIST );
            break;

        case PROPERTY_ID_STRINGITEMLIST:
            if ( bIsSpreadsheetDocument )
                GetAnyPropertyValue( PROPERTY_LIST_CELL_RANGE ) >>= xSource;
            GetAnyPropertyValue( PROPERTY_LISTSOURCETYPE ) >>= eLSType;

            {
                Sequence< ::rtl::OUString > aListSource;
                Any aListSourceValue( GetAnyPropertyValue( PROPERTY_LISTSOURCE ) );
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
            GetAnyPropertyValue( PROPERTY_TARGET_URL ) >>= sStringValue;
            FormButtonType eButtonType( FormButtonType_PUSH );
            if ( 0 != m_nClassId )
            {   // if we're inspecting a control which has a TargetFrame, then it also has
                // the FormButtonType property, since it's a push/image button then
                GetAnyPropertyValue( PROPERTY_BUTTONTYPE ) >>= eButtonType;
            }
            bIsEnabled = ( eButtonType == FormButtonType_URL )
                      && ( sStringValue.getLength() > 0 );
        }
        break;

        case PROPERTY_ID_COMMAND:
        {
            sal_Int32 nIntValue( 0 );
            sal_Bool  bBoolValue( sal_False );
            GetAnyPropertyValue( PROPERTY_COMMANDTYPE ) >>= nIntValue;
            GetAnyPropertyValue( PROPERTY_ESCAPE_PROCESSING ) >>= bBoolValue;

            bIsEnabled = ( nIntValue == CommandType::COMMAND )
                      && ( bBoolValue )
                      && (  isRowsetConnected()
                         || isValidDataSourceName( GetPropertyValueStringRep( PROPERTY_DATASOURCE ) )
                         );

            bEnableBrowseButtonOnly = true;
        }
        break;

        case PROPERTY_ID_DETAILFIELDS:
        {
            Reference< XConnection > xDummyConn;
            bool bAllowEmptyDS = ::dbtools::isEmbeddedInDatabase( m_xObjectParent, xDummyConn );

            // both our current form, and it's parent form, need to have a valid
            // data source signature
            bIsEnabled = hasValidDataSourceSignature( Reference< XForm >( m_xIntrospecteeAsProperty, UNO_QUERY ), bAllowEmptyDS )
                      && hasValidDataSourceSignature( Reference< XForm >( m_xObjectParent,           UNO_QUERY ), bAllowEmptyDS );

            // in opposite to the other properties, here in real *two* properties are
            // affected
            getPropertyBox()->EnablePropertyControls( PROPERTY_DETAILFIELDS, true, bIsEnabled, false );
            getPropertyBox()->EnablePropertyControls( PROPERTY_MASTERFIELDS, true, bIsEnabled, false );
            return;
        }
        break;

        case PROPERTY_ID_SORT:
        case PROPERTY_ID_FILTER:
        {
            Reference< XConnection > xDummyConn;
            bool bAllowEmptyDS = ::dbtools::isEmbeddedInDatabase( m_xObjectParent, xDummyConn );

            sal_Bool  bBoolValue( sal_False );
            GetAnyPropertyValue( PROPERTY_ESCAPE_PROCESSING ) >>= bBoolValue;
            bIsEnabled = hasValidDataSourceSignature( Reference< XForm >( m_xIntrospecteeAsProperty, UNO_QUERY ), bAllowEmptyDS )
                      && bBoolValue;

            getPropertyBox()->EnablePropertyLine( _rPropertyName, bBoolValue );
            // if escape processing is on, then we need to disable/enable the browse button only,
            // else the whole line
            getPropertyBox()->EnablePropertyControls( _rPropertyName, true, bIsEnabled, false );
            return;
        }
        break;

        case PROPERTY_ID_SELECTEDITEMS:
        case PROPERTY_ID_DEFAULT_SELECT_SEQ:
        {
            Sequence< ::rtl::OUString > aEntries;
            GetAnyPropertyValue( PROPERTY_STRINGITEMLIST ) >>= aEntries;
            bIsEnabled = aEntries.getLength() != 0;

            if ( ( m_nClassId == FormComponentType::LISTBOX ) && ( getControlType() == CONTROL_TYPE_FORM ) )
            {
                GetAnyPropertyValue( PROPERTY_LISTSOURCETYPE ) >>= eLSType;
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
            getPropertyBox()->EnablePropertyControls( _rPropertyName, true, bIsEnabled, false );
        else
            getPropertyBox()->EnablePropertyLine( _rPropertyName, bIsEnabled );
    }


    //------------------------------------------------------------------------
    bool OPropertyBrowserController::hasValidDataSourceSignature( const Reference< XForm >& _rxForm, bool _bAllowEmptyDataSourceName ) SAL_THROW(())
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
            bHas = ( sPropertyValue.getLength() != 0 ) || _bAllowEmptyDataSourceName;

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

        if ( haveIntrospecteeProperty( PROPERTY_CLASSID ) )
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

        Reference< XServiceInfo > xDocumentSI( m_xContextDocument, UNO_QUERY );
        OSL_ENSURE( xDocumentSI.is(), "OPropertyBrowserController::getDocumentMeasurementUnit: No context document - where do I live?" );
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
            getPropertyBox()->EnablePropertyLine( PROPERTY_DATASOURCE, true );
            getPropertyBox()->EnablePropertyLine( PROPERTY_COMMANDTYPE, true );
            getPropertyBox()->EnablePropertyLine( PROPERTY_COMMAND, true );
            getPropertyBox()->EnablePropertyLine( PROPERTY_ESCAPE_PROCESSING, true );

            getPropertyBox()->EnablePropertyControls( PROPERTY_DATASOURCE, true, true, false );
            getPropertyBox()->EnablePropertyControls( PROPERTY_COMMANDTYPE, true, true, false );
            getPropertyBox()->EnablePropertyControls( PROPERTY_COMMAND, true, true, false );
            getPropertyBox()->EnablePropertyControls( PROPERTY_ESCAPE_PROCESSING, true, true, false );
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
            Reference< XConnection > xConnection = ensureAndGetRowsetConnection();
            if ( !xConnection.is() )
                return;

            // get a composer for the statement which the form is currently based on
            Reference< XSingleSelectQueryComposer > xComposer = getCurrentSettingsComposer( m_xIntrospecteeAsProperty, m_xORB );
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
                    m_xPropValueAccess->setPropertyValue( PROPERTY_FILTER, makeAny( xComposer->getFilter() ) );
                else
                    m_xPropValueAccess->setPropertyValue( PROPERTY_SORT,   makeAny( xComposer->getOrder()  ) );
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

            Reference< XConnection > xConnection = ensureAndGetRowsetConnection();
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

                getPropertyBox()->EnablePropertyControls( PROPERTY_DATASOURCE, false, true, false );
                getPropertyBox()->EnablePropertyControls( PROPERTY_COMMANDTYPE, false, true, false );
                getPropertyBox()->EnablePropertyControls( PROPERTY_COMMAND, false, true, false );
                getPropertyBox()->EnablePropertyControls( PROPERTY_ESCAPE_PROCESSING, false, true, false );
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

