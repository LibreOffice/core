 /*************************************************************************
 *
 *  $RCSfile: cfgimport.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:17:04 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
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

#include "cfgimport.hxx"

#ifndef CFG_REGHELPER_HXX
#include "cfg_reghelper.hxx"
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_XNAMINGSERVICE_HPP_
#include <com/sun/star/uno/XNamingService.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERYDEFINITIONSSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueryDefinitionsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDATADESCRIPTORFACTORY_HPP_
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XAPPEND_HPP_
#include <com/sun/star/sdbcx/XAppend.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif
#ifndef _URLOBJ_HXX //autogen wg. INetURLObject
#include <tools/urlobj.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef DBACCESS_SHARED_CFGSTRINGS_HRC
#include "cfgstrings.hrc"
#endif
#ifndef _UNOTOOLS_CONFIGNODE_HXX_
#include <unotools/confignode.hxx>
#endif
#ifndef _UNOTOOLS_UCBHELPER_HXX
#include <unotools/ucbhelper.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX
#include <sfx2/docfilt.hxx>
#endif
#ifndef _COMPHELPER_STREAMSECTION_HXX_
#include <comphelper/streamsection.hxx>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOBJECTINPUTSTREAM_HPP_
#include <com/sun/star/io/XObjectInputStream.hpp>
#endif
#ifndef _COMPHELPER_BASIC_IO_HXX_
#include <comphelper/basicio.hxx>
#endif
#ifndef _COMPHELPER_SEQSTREAM_HXX
#include <comphelper/seqstream.hxx>
#endif
#ifndef _COM_SUN_STAR_SDB_XREPORTDOCUMENTSSUPPLIER_HPP_
#include <com/sun/star/sdb/XReportDocumentsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XFORMDOCUMENTSSUPPLIER_HPP_
#include <com/sun/star/sdb/XFormDocumentsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XLOADABLE_HPP_
#include <com/sun/star/frame/XLoadable.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XTYPEDETECTION_HPP_
#include <com/sun/star/document/XTypeDetection.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTSSUPPLIER_HPP_
#include <com/sun/star/document/XEventsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEREPLACE_HPP_
#include <com/sun/star/container/XNameReplace.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMCOMPONENTTYPE_HPP_
#include <com/sun/star/form/FormComponentType.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESUPPLIER_HPP_
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMSSUPPLIER_HPP_
#include <com/sun/star/form/XFormsSupplier.hpp>
#endif


extern "C" void SAL_CALL createRegistryInfo_OCfgImport( )
{
    static ::dbacfg::OMultiInstanceAutoRegistration< ::dbacfg::OCfgImport > aAutoRegistration;
}
//--------------------------------------------------------------------------
namespace dbacfg
{
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::document;
    using namespace ::com::sun::star::io;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::drawing;
    using namespace ::com::sun::star::container;
    using namespace ::utl;
    using namespace ::comphelper;

    void convertObjects(const OConfigurationNode& _rObjects,const Reference< XNameAccess >& _xParentContainer,sal_Bool _bQuery,const Reference< XMultiServiceFactory >& _xORB);
    void convertLinks(const OConfigurationNode& _rObjects,const Reference< XPropertySet >& _xDataSource,const Reference< XMultiServiceFactory >& _xORB);
    void convertColumns(const OConfigurationNode& _rColumns,const Reference<XPropertySet>& _xObject);
    void convertDataSettings(const OConfigurationNode& _rObject,const Reference<XPropertySet>& _xObject);
    void setProperty(const ::rtl::OUString& _sPropertyName, const ::rtl::OUString& _sConfigName,const Reference<XPropertySet>& _xProp,const OConfigurationNode& _aNode);
    void LoadTableWindows(const Reference< XObjectInputStream>& _rxIn,Sequence<PropertyValue>& _rViewProps);
    void LoadTableWindowData(const Reference<XObjectInputStream>& _rxIn,PropertyValue* _pValue);
    void LoadTableFields(const Reference< XObjectInputStream>& _rxIn,Sequence<PropertyValue>& _rViewProps);
    void LoadTableFieldDesc(const Reference< XObjectInputStream>& _rxIn,PropertyValue& _rProperty);
    sal_Bool isDocumentReport(const Reference< XMultiServiceFactory >& _xORB,const ::rtl::OUString& _sDocumentLocation);
// -------------
// - OCfgImport -
// -------------

OCfgImport::OCfgImport( const Reference< XMultiServiceFactory >& _rxMSF )
    :m_xORB( _rxMSF )
{
}

// -----------------------------------------------------------------------------

OCfgImport::~OCfgImport() throw()
{
}
// -----------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO1_STATIC( OCfgImport, "com.sun.star.comp.sdb.DataSourceMigration", "com.sun.star.sdb.DataSourceMigration")
// -----------------------------------------------------------------------------
// XInitialization
void SAL_CALL OCfgImport::initialize( const Sequence< Any >& _aArguments ) throw(Exception, RuntimeException)
{
    const Any* pBegin = _aArguments.getConstArray();
    const Any* pEnd = pBegin + _aArguments.getLength();
    PropertyValue aValue;;
    for(;pBegin != pEnd;++pBegin)
    {
        *pBegin >>= aValue;
        if ( aValue.Name.equalsAscii("Parent") )
        {
        }
    }
    convert();
}
// -----------------------------------------------------------------------------
void OCfgImport::convert()
{
    // the config node where all pooling relevant info are stored under
    OConfigurationTreeRoot aDSNamesRoot = OConfigurationTreeRoot::createWithServiceFactory(
        m_xORB, CFG_DATASOURCEPATH, -1, OConfigurationTreeRoot::CM_READONLY);

    SvtPathOptions aPathOptions;
    const String& rsWorkPath = aPathOptions.GetWorkPath();

    ::rtl::OUString sExtension;
    static const String s_sDatabaseType = String::CreateFromAscii("StarOffice XML (Base)");
    const SfxFilter* pFilter = SfxFilter::GetFilterByName( s_sDatabaseType);
    OSL_ENSURE(pFilter,"Filter: StarOffice XML (Base) could not be found!");
    if ( pFilter )
    {
        String aRet = pFilter->GetDefaultExtension();
        while( aRet.SearchAndReplaceAscii( "*.", String() ) != STRING_NOTFOUND );
        sExtension = aRet;
    }
    // then look for which of them settings are stored in the configuration
    Sequence< ::rtl::OUString > aDS = aDSNamesRoot.getNodeNames();
    const ::rtl::OUString* pDSIter = aDS.getConstArray();
    const ::rtl::OUString* pDSEnd = pDSIter + aDS.getLength();
    for (;pDSIter != pDSEnd; ++pDSIter)
    {
        ::rtl::OUString sFileName;
        try
        {
            Reference<XPropertySet> xDataSource(m_xORB->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.DatabaseDocument"))),UNO_QUERY);
            Reference< XModel > xModel(xDataSource,UNO_QUERY);
            if ( !xModel.is() )
                break;

            INetURLObject aURL(rsWorkPath,INetURLObject::WAS_ENCODED);
            aURL.insertName(*pDSIter,false,INetURLObject::LAST_SEGMENT,true,INetURLObject::ENCODE_ALL);
            aURL.setExtension(sExtension);

            sFileName = aURL.GetMainURL(INetURLObject::NO_DECODE);

            sal_Int32 i = 0;
            // create unique name
            while ( UCBContentHelper::IsDocument(sFileName) )
            {
                sFileName = *pDSIter + ::rtl::OUString::valueOf(++i);
                aURL.setName(sFileName,INetURLObject::LAST_SEGMENT,true,INetURLObject::ENCODE_ALL);
                aURL.setExtension(sExtension);
                sFileName = aURL.GetMainURL(INetURLObject::NO_DECODE);
            }

            xModel->attachResource(sFileName,Sequence<PropertyValue>());

            OConfigurationNode aDataSource = aDSNamesRoot.openNode(*pDSIter);

            setProperty(PROPERTY_URL,CONFIGKEY_DBLINK_CONNECTURL,xDataSource,aDataSource);
            setProperty(PROPERTY_USER,CONFIGKEY_DBLINK_USER,xDataSource,aDataSource);
            setProperty(PROPERTY_TABLEFILTER,CONFIGKEY_DBLINK_TABLEFILTER,xDataSource,aDataSource);
            setProperty(PROPERTY_TABLETYPEFILTER,CONFIGKEY_DBLINK_TABLETYEFILTER,xDataSource,aDataSource);
            setProperty(PROPERTY_ISPASSWORDREQUIRED,CONFIGKEY_DBLINK_PASSWORDREQUIRED,xDataSource,aDataSource);
            setProperty(PROPERTY_SUPPRESSVERSIONCL,CONFIGKEY_DBLINK_SUPPRESSVERSIONCL,xDataSource,aDataSource);

            // convert layout information
            Any aValue(aDataSource.getNodeValue(CONFIGKEY_LAYOUTINFORMATION));
            if ( aValue.hasValue() )
            {
                Sequence< sal_Int8 > aInputSequence;
                aValue >>= aInputSequence;
                if ( aInputSequence.getLength() )
                {
                    Reference< XInputStream>       xInStreamHelper = new SequenceInputStream(aInputSequence);;  // used for wrapping sequence to xinput
                    Reference< XObjectInputStream> xInStream = Reference< XObjectInputStream >(m_xORB->createInstance(::rtl::OUString::createFromAscii("com.sun.star.io.ObjectInputStream")),UNO_QUERY);
                    Reference< XInputStream> xMarkInStream = Reference< XInputStream >(m_xORB->createInstance(::rtl::OUString::createFromAscii("com.sun.star.io.MarkableInputStream")),UNO_QUERY);
                    Reference< XActiveDataSink >(xMarkInStream,UNO_QUERY)->setInputStream(xInStreamHelper);
                    Reference< XActiveDataSink >   xInDataSource(xInStream, UNO_QUERY);
                    OSL_ENSURE(xInDataSource.is(),"Couldn't create com.sun.star.io.ObjectInputStream!");
                    xInDataSource->setInputStream(xMarkInStream);

                    Sequence< PropertyValue > aLayout;
                    LoadTableWindows(xInStream,aLayout);
                    xDataSource->setPropertyValue(PROPERTY_LAYOUTINFORMATION,makeAny(aLayout));
                }
            }

            // the property sequence in Info
            OConfigurationNode aInfoNode = aDataSource.openNode(CONFIGKEY_DBLINK_INFO);
            if ( aInfoNode.isValid() )
            {
                Sequence< ::rtl::OUString > aNodeNames = aInfoNode.getNodeNames();
                Sequence< PropertyValue > aInfo(aNodeNames.getLength());
                PropertyValue* pInfos = aInfo.getArray();

                for (   const ::rtl::OUString* pNodeNames = aNodeNames.getConstArray() + aNodeNames.getLength() - 1;
                        pNodeNames >= aNodeNames.getConstArray();
                        --pNodeNames, ++pInfos
                    )
                {
                    OConfigurationNode aItemSubNode = aInfoNode.openNode(*pNodeNames);
                    pInfos->Name = *pNodeNames;
                    pInfos->Value = aItemSubNode.getNodeValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Value")));
                }
                xDataSource->setPropertyValue(PROPERTY_INFO,makeAny(aInfo));
            }

            Reference<XQueryDefinitionsSupplier> xQueriesSupplier(xDataSource,UNO_QUERY);
            Reference<XNameAccess> xQueries(xQueriesSupplier->getQueryDefinitions(),UNO_QUERY);
            convertObjects(aDataSource.openNode(CONFIGKEY_DBLINK_QUERYDOCUMENTS),xQueries,sal_True,m_xORB);

            Reference<XTablesSupplier> xTablesSupplier(xDataSource,UNO_QUERY);
            Reference<XNameAccess> xTables(xTablesSupplier->getTables(),UNO_QUERY);
            convertObjects(aDataSource.openNode(CONFIGKEY_DBLINK_TABLES),xTables,sal_False,m_xORB);

            convertLinks(aDataSource.openNode(CONFIGKEY_DBLINK_BOOKMARKS),xDataSource,m_xORB);

            Reference<XStorable> xStr(xModel,UNO_QUERY);
            if ( xStr.is() )
                xStr->store();
            // register the new datbase document

            // create unique name
            Reference< XNameAccess > xDatabaseContext(m_xORB->createInstance(SERVICE_SDB_DATABASECONTEXT), UNO_QUERY);
            if ( xDatabaseContext.is() )
            {
                i = 0;
                ::rtl::OUString sName = *pDSIter;
                while ( xDatabaseContext->hasByName(sName) )
                {
                    sName = *pDSIter + ::rtl::OUString::valueOf(++i);
                }
                Reference< XNamingService>(xDatabaseContext,UNO_QUERY)->registerObject(sName,xModel);
            }
            xDataSource = NULL;
            ::comphelper::disposeComponent(xModel);
        }
        catch(Exception)
        {
            OSL_ENSURE(0,"Exception: convert");
            UCBContentHelper::Kill(sFileName);
        }
    }
}
// -----------------------------------------------------------------------------
void convertObjects(const OConfigurationNode& _rObjects,const Reference< XNameAccess >& _xParentContainer,sal_Bool _bQuery,const Reference< XMultiServiceFactory >& _xORB)
{
    Reference<XNameContainer> xNameContainer(_xParentContainer,UNO_QUERY);
    if ( xNameContainer.is() )
    {
        // then look for which of them settings are stored in the configuration
        Sequence< ::rtl::OUString > aNames = _rObjects.getNodeNames();
        const ::rtl::OUString* pIter = aNames.getConstArray();
        const ::rtl::OUString* pEnd = pIter + aNames.getLength();
        for (;pIter != pEnd; ++pIter)
        {
            try
            {
                Sequence< Any > aArguments(1);
                PropertyValue aValue;
                // set as folder
                aValue.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Name"));
                aValue.Value <<= *pIter;
                aArguments[0] <<= aValue;
                Reference<XPropertySet> xProp(_xORB->createInstanceWithArguments(_bQuery ? SERVICE_SDB_COMMAND_DEFINITION : SERVICE_SDB_TABLEDEFINITION,aArguments ),UNO_QUERY);

                OConfigurationNode aObject = _rObjects.openNode(*pIter);

                if ( _bQuery )
                {
                    setProperty(PROPERTY_COMMAND,CONFIGKEY_QRYDESCR_COMMAND,xProp,aObject);
                    setProperty(PROPERTY_USE_ESCAPE_PROCESSING,CONFIGKEY_QRYDESCR_USE_ESCAPE_PROCESSING,xProp,aObject);
                    setProperty(PROPERTY_UPDATE_TABLENAME,CONFIGKEY_QRYDESCR_UPDATE_TABLENAME,xProp,aObject);
                    setProperty(PROPERTY_UPDATE_SCHEMANAME,CONFIGKEY_QRYDESCR_UPDATE_SCHEMANAME,xProp,aObject);
                    setProperty(PROPERTY_UPDATE_CATALOGNAME,CONFIGKEY_QRYDESCR_UPDATE_CATALOGNAME,xProp,aObject);

                    // convert layout information
                    Any aValue(aObject.getNodeValue(CONFIGKEY_LAYOUTINFORMATION));
                    if ( aValue.hasValue() )
                    {
                        Sequence< sal_Int8 > aInputSequence;
                        aValue >>= aInputSequence;
                        if ( aInputSequence.getLength() )
                        {
                            Reference< XInputStream>       xInStreamHelper = new SequenceInputStream(aInputSequence);;  // used for wrapping sequence to xinput
                            Reference< XObjectInputStream> xInStream = Reference< XObjectInputStream >(_xORB->createInstance(::rtl::OUString::createFromAscii("com.sun.star.io.ObjectInputStream")),UNO_QUERY);
                            Reference< XInputStream> xMarkInStream = Reference< XInputStream >(_xORB->createInstance(::rtl::OUString::createFromAscii("com.sun.star.io.MarkableInputStream")),UNO_QUERY);
                            Reference< XActiveDataSink >(xMarkInStream,UNO_QUERY)->setInputStream(xInStreamHelper);
                            Reference< XActiveDataSink >   xInDataSource(xInStream, UNO_QUERY);
                            OSL_ENSURE(xInDataSource.is(),"Couldn't create com.sun.star.io.ObjectInputStream!");
                            xInDataSource->setInputStream(xMarkInStream);

                            Sequence< PropertyValue > aLayout;
                            LoadTableFields(xInStream,aLayout);

                            xProp->setPropertyValue(PROPERTY_LAYOUTINFORMATION,makeAny(aLayout));
                        }
                    }
                }

                convertDataSettings(aObject,xProp);
                convertColumns(aObject.openNode( CONFIGKEY_QRYDESCR_COLUMNS ), xProp);

                xNameContainer->insertByName(*pIter,makeAny(xProp));
            }
            catch(Exception&)
            {
                OSL_ENSURE(0,"Exception: convertObjects");
            }
        }
    }
}
// -----------------------------------------------------------------------------
void convertColumns(const OConfigurationNode& _rColumns,const Reference<XPropertySet>& _xObject)
{
    Reference<XColumnsSupplier> xSupplier(_xObject,UNO_QUERY);
    if ( xSupplier.is() )
    {
        Reference<XDataDescriptorFactory> xFact(xSupplier->getColumns(),UNO_QUERY);

        Reference<XPropertySet> xProp = xFact.is() ? xFact->createDataDescriptor() : Reference<XPropertySet>();
        if ( !xProp.is() )
            return;
        // then look for which of them settings are stored in the configuration
        Sequence< ::rtl::OUString > aNames = _rColumns.getNodeNames();
        const ::rtl::OUString* pIter = aNames.getConstArray();
        const ::rtl::OUString* pEnd = pIter + aNames.getLength();
        for (;pIter != pEnd; ++pIter)
        {
            try
            {
                // the name of the driver in this round
                OConfigurationNode aColumn = _rColumns.openNode(*pIter);
                xProp->setPropertyValue(PROPERTY_NAME,makeAny(*pIter));

                setProperty(PROPERTY_ALIGN,CONFIGKEY_COLUMN_ALIGNMENT,xProp,aColumn);
                setProperty(PROPERTY_WIDTH,CONFIGKEY_COLUMN_WIDTH,xProp,aColumn);
                setProperty(PROPERTY_RELATIVEPOSITION,CONFIGKEY_COLUMN_RELPOSITION,xProp,aColumn);
                setProperty(PROPERTY_HIDDEN,CONFIGKEY_COLUMN_HIDDEN,xProp,aColumn);
                setProperty(PROPERTY_HELPTEXT,CONFIGKEY_COLUMN_HELPTEXT,xProp,aColumn);
                setProperty(PROPERTY_CONTROLDEFAULT,CONFIGKEY_COLUMN_CONTROLDEFAULT,xProp,aColumn);
                setProperty(PROPERTY_NUMBERFORMAT,CONFIGKEY_COLUMN_NUMBERFORMAT,xProp,aColumn);
            }
            catch(Exception)
            {
                OSL_ENSURE(0,"Exception: convertColumns");
            }
        }
        Reference<XAppend> xAppend(xFact,UNO_QUERY);
        if ( xAppend.is() )
        {
            xAppend->appendByDescriptor(xProp);
        }
    }
}
// -----------------------------------------------------------------------------
void convertDataSettings(const OConfigurationNode& _rObject,const Reference<XPropertySet>& _xObject)
{
    // data settings
    OConfigurationNode aSettingsNode = _rObject.openNode(CONFIGKEY_SETTINGS);
    if ( aSettingsNode.isValid() )
    {
        setProperty(PROPERTY_FILTER,CONFIGKEY_DEFSET_FILTER,_xObject,aSettingsNode);
        setProperty(PROPERTY_ORDER,CONFIGKEY_DEFSET_ORDER,_xObject,aSettingsNode);
        setProperty(PROPERTY_APPLYFILTER,CONFIGKEY_DEFSET_APPLYFILTER,_xObject,aSettingsNode);
        setProperty(PROPERTY_ROW_HEIGHT,CONFIGKEY_DEFSET_ROW_HEIGHT,_xObject,aSettingsNode);
        setProperty(PROPERTY_TEXTCOLOR,CONFIGKEY_DEFSET_TEXTCOLOR,_xObject,aSettingsNode);
        setProperty(PROPERTY_TEXTLINECOLOR,CONFIGKEY_DEFSET_FONT_UNDERLINECOLOR,_xObject,aSettingsNode);
        setProperty(PROPERTY_TEXTEMPHASIS,CONFIGKEY_DEFSET_FONT_CHARACTEREMPHASIS,_xObject,aSettingsNode);
        setProperty(PROPERTY_TEXTRELIEF,CONFIGKEY_DEFSET_FONT_CHARACTERRELIEF,_xObject,aSettingsNode);
            // font
        setProperty(PROPERTY_FONTHEIGHT,CONFIGKEY_DEFSET_FONT_HEIGHT,_xObject,aSettingsNode);
        setProperty(PROPERTY_FONTWIDTH,CONFIGKEY_DEFSET_FONT_WIDTH,_xObject,aSettingsNode);
        setProperty(PROPERTY_FONTSTYLENAME,CONFIGKEY_DEFSET_FONT_STYLENAME,_xObject,aSettingsNode);
        setProperty(PROPERTY_FONTFAMILY,CONFIGKEY_DEFSET_FONT_FAMILY,_xObject,aSettingsNode);
        setProperty(PROPERTY_FONTCHARSET,CONFIGKEY_DEFSET_FONT_CHARSET,_xObject,aSettingsNode);
        setProperty(PROPERTY_FONTPITCH,CONFIGKEY_DEFSET_FONT_PITCH,_xObject,aSettingsNode);
        setProperty(PROPERTY_FONTCHARWIDTH,CONFIGKEY_DEFSET_FONT_CHARACTERWIDTH,_xObject,aSettingsNode);
        setProperty(PROPERTY_FONTWEIGHT,CONFIGKEY_DEFSET_FONT_WEIGHT,_xObject,aSettingsNode);
        setProperty(PROPERTY_FONTUNDERLINE,CONFIGKEY_DEFSET_FONT_UNDERLINE,_xObject,aSettingsNode);
        setProperty(PROPERTY_FONTSTRIKEOUT,CONFIGKEY_DEFSET_FONT_STRIKEOUT,_xObject,aSettingsNode);
        setProperty(PROPERTY_FONTORIENTATION,CONFIGKEY_DEFSET_FONT_ORIENTATION,_xObject,aSettingsNode);
        setProperty(PROPERTY_FONTKERNING,CONFIGKEY_DEFSET_FONT_KERNING,_xObject,aSettingsNode);
        setProperty(PROPERTY_FONTWORDLINEMODE,CONFIGKEY_DEFSET_FONT_WORDLINEMODE,_xObject,aSettingsNode);
        setProperty(PROPERTY_FONTTYPE,CONFIGKEY_DEFSET_FONT_TYPE,_xObject,aSettingsNode);
        setProperty(PROPERTY_FONTSLANT,CONFIGKEY_DEFSET_FONT_SLANT,_xObject,aSettingsNode);
    }
}
// -----------------------------------------------------------------------------
void setProperty(const ::rtl::OUString& _sPropertyName, const ::rtl::OUString& _sConfigName,const Reference<XPropertySet>& _xProp,const OConfigurationNode& _aNode)
{
    try
    {
        Any aValue(_aNode.getNodeValue(_sConfigName));
        if ( aValue.hasValue() )
            _xProp->setPropertyValue(_sPropertyName,aValue);
    }
    catch(Exception&)
    {
        OSL_ENSURE(0,"Property could not be set!");
    }
}
// -----------------------------------------------------------------------------
void LoadTableWindows(const Reference< XObjectInputStream>& _rxIn,Sequence<PropertyValue>& _rViewProps)
{
    try
    {
        OStreamSection aSection(_rxIn.get());

        sal_Int32 nCount = 0;
        _rxIn >> nCount;
        if ( nCount > 0 )
        {
            PropertyValue *pViewIter = _rViewProps.getArray();
            PropertyValue *pEnd = pViewIter + _rViewProps.getLength();
            const static ::rtl::OUString s_sTables(RTL_CONSTASCII_USTRINGPARAM("Tables"));
            for (; pViewIter != pEnd && pViewIter->Name != s_sTables; ++pViewIter)
                ;

            if ( pViewIter == pEnd )
            {
                sal_Int32 nLen = _rViewProps.getLength();
                _rViewProps.realloc( nLen + 1 );
                pViewIter = _rViewProps.getArray() + nLen;
                pViewIter->Name = s_sTables;
            }

            Sequence<PropertyValue> aTables(nCount);
            PropertyValue *pIter = aTables.getArray();


            for(sal_Int32 i=0;i<nCount;++i,++pIter)
            {
                pIter->Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Table")) + ::rtl::OUString::valueOf(i+1);
                LoadTableWindowData(_rxIn,pIter);
            }
        }
    }
    catch(Exception&)
    {
    }
}
// -----------------------------------------------------------------------------
void LoadTableWindowData(const Reference<XObjectInputStream>& _rxIn,PropertyValue* _pValue)
{
    ::rtl::OUString sComposedName,aTableName,aWinName;
    sal_Int32 nX,nY,nWidth,nHeight;
    sal_Bool bShowAll;

    OStreamSection aSection(_rxIn.get());
    _rxIn >> sComposedName;
    _rxIn >> aTableName;
    _rxIn >> aWinName;
    _rxIn >> nX;
    _rxIn >> nY;
    _rxIn >> nWidth;
    _rxIn >> nHeight;
    _rxIn >> bShowAll;

    Sequence<PropertyValue> aWindow(8);
    sal_Int32 nPos = 0;
    aWindow[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ComposedName"));
    aWindow[nPos++].Value <<= sComposedName;
    aWindow[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TableName"));
    aWindow[nPos++].Value <<= aTableName;
    aWindow[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("WindowName"));
    aWindow[nPos++].Value <<= aWinName;
    aWindow[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("WindowTop"));
    aWindow[nPos++].Value <<= nY;
    aWindow[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("WindowLeft"));
    aWindow[nPos++].Value <<= nX;
    aWindow[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("WindowWidth"));
    aWindow[nPos++].Value <<= nWidth;
    aWindow[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("WindowHeight"));
    aWindow[nPos++].Value <<= nHeight;
    aWindow[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowAll"));
    aWindow[nPos++].Value <<= bShowAll;

    _pValue->Value <<= aWindow;
}
// -----------------------------------------------------------------------------
void LoadTableFields(const Reference< XObjectInputStream>& _rxIn,Sequence<PropertyValue>& _rViewProps)
{
    LoadTableWindows(_rxIn,_rViewProps);
    OStreamSection aSection(_rxIn.get());
    PropertyValue *pIter = NULL;
    try
    {
        sal_Int32 nSplitPos,nVisibleRows;
        // some data
        _rxIn >> nSplitPos;
        _rxIn >> nVisibleRows;

        sal_Int32 nCount = 0;
        _rxIn >> nCount;

        sal_Int32 nLen = _rViewProps.getLength();
        _rViewProps.realloc( nLen + 2 + (nCount != 0 ? 1 : 0) );
        pIter = _rViewProps.getArray() + nLen;

        if ( nCount != 0 )
        {
            pIter->Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Fields"));

            Sequence<PropertyValue> aFields(nCount);
            PropertyValue *pFieldsIter = aFields.getArray();
            // the fielddata
            for(sal_Int32 j=0;j<nCount;++j)
            {
                if ( aSection.available() )
                {
                    pFieldsIter->Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Field")) + ::rtl::OUString::valueOf(j+1);
                    LoadTableFieldDesc(_rxIn,*pFieldsIter++);
                }
            }
            pIter->Value <<= aFields;
            ++pIter;
        }

        pIter->Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SplitterPosition"));
        pIter->Value <<= nSplitPos;
        ++pIter;
        pIter->Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VisibleRows"));
        pIter->Value <<= nVisibleRows;
    }
    catch(Exception&)
    {
    }
}
// -----------------------------------------------------------------------------
void LoadTableFieldDesc(const Reference< XObjectInputStream>& _rxIn,PropertyValue& _rProperty)
{
    ::rtl::OUString     aTableName;
    ::rtl::OUString     aAliasName;     // table range
    ::rtl::OUString     aFieldName;     // column
    ::rtl::OUString     aFieldAlias;    // column alias
    ::rtl::OUString     aDatabaseName;  // qualifier or catalog
    ::rtl::OUString     aFunctionName;  // enth"alt den Funktionsnamen, nur wenn eFunctionType != FKT_NONE gesetzt

    sal_Int32           eDataType;
    sal_Int32           eFunctionType;
    sal_Int32           eFieldType;
    sal_Int32           eOrderDir;
    sal_Int32           nColWidth;
    sal_Bool            bGroupBy;
    sal_Bool            bVisible;

    OStreamSection aSection(_rxIn.get());
    _rxIn >> aTableName;
    _rxIn >> aAliasName;
    _rxIn >> aFieldName;
    _rxIn >> aFieldAlias;
    _rxIn >> aDatabaseName;
    _rxIn >> aFunctionName;
    _rxIn >> eDataType;
    _rxIn >> eFunctionType;
    _rxIn >> eFieldType;
    _rxIn >> eOrderDir;
    _rxIn >> nColWidth;
    _rxIn >> bGroupBy;
    _rxIn >> bVisible;

    Sequence<PropertyValue> aFieldDesc(13);
    sal_Int32 nPos = 0;
    aFieldDesc[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AliasName"));
    aFieldDesc[nPos++].Value <<= aAliasName;
    aFieldDesc[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TableName"));
    aFieldDesc[nPos++].Value <<= aTableName;
    aFieldDesc[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FieldName"));
    aFieldDesc[nPos++].Value <<= aFieldName;
    aFieldDesc[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FieldAlias"));
    aFieldDesc[nPos++].Value <<= aFieldAlias;
    aFieldDesc[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DatabaseName"));
    aFieldDesc[nPos++].Value <<= aDatabaseName;
    aFieldDesc[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FunctionName"));
    aFieldDesc[nPos++].Value <<= aFunctionName;
    aFieldDesc[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DataType"));
    aFieldDesc[nPos++].Value <<= eDataType;
    aFieldDesc[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FunctionType"));
    aFieldDesc[nPos++].Value <<= (sal_Int32)eFunctionType;
    aFieldDesc[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FieldType"));
    aFieldDesc[nPos++].Value <<= (sal_Int32)eFieldType;
    aFieldDesc[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OrderDir"));
    aFieldDesc[nPos++].Value <<= (sal_Int32)eOrderDir;
    aFieldDesc[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ColWidth"));
    aFieldDesc[nPos++].Value <<= nColWidth;
    aFieldDesc[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("GroupBy"));
    aFieldDesc[nPos++].Value <<= bGroupBy;
    aFieldDesc[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Visible"));
    aFieldDesc[nPos++].Value <<= bVisible;

    _rProperty.Value <<= aFieldDesc;
}
// -----------------------------------------------------------------------------
void convertLinks(const OConfigurationNode& _rObjects,const Reference< XPropertySet >& _xDataSource,const Reference< XMultiServiceFactory >& _xORB)
{
    // then look for which of them settings are stored in the configuration
    Sequence< ::rtl::OUString > aNames = _rObjects.getNodeNames();
    const ::rtl::OUString* pIter = aNames.getConstArray();
    const ::rtl::OUString* pEnd = pIter + aNames.getLength();
    for (;pIter != pEnd; ++pIter)
    {
        try
        {
            OConfigurationNode aObject = _rObjects.openNode(*pIter);
            ::rtl::OUString sDocumentLocation;
            aObject.getNodeValue(CONFIGKEY_DBLINK_DOCUMENTLOCATION) >>= sDocumentLocation;
            if ( !UCBContentHelper::IsDocument(sDocumentLocation) )
                continue;

            sal_Bool bForm = sal_True;
            bForm = !isDocumentReport(_xORB,sDocumentLocation);
            Reference<XNameAccess> xNames;
            if ( bForm )
            {
                Reference<XFormDocumentsSupplier> xSup(_xDataSource,UNO_QUERY);
                if ( xSup.is() )
                    xNames = xSup->getFormDocuments();
            }
            else
            {
                Reference<XReportDocumentsSupplier> xSup(_xDataSource,UNO_QUERY);
                if ( xSup.is() )
                    xNames = xSup->getReportDocuments();
            }

            if ( xNames.is() && pIter->getLength() )
            {
                ::rtl::OUString sServiceName = SERVICE_SDB_DOCUMENTDEFINITION;
                Sequence< Any > aArguments(3);
                PropertyValue aValue;
                // set as folder
                aValue.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Name"));
                aValue.Value <<= *pIter;
                aArguments[0] <<= aValue;
                //parent
                aValue.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Parent"));
                aValue.Value <<= xNames;
                aArguments[1] <<= aValue;

                aValue.Name = PROPERTY_URL;
                aValue.Value <<= sDocumentLocation;
                aArguments[2] <<= aValue;

                Reference<XMultiServiceFactory> xORB(xNames,UNO_QUERY);
                if ( xORB.is() )
                {
                    Reference<XInterface> xComponent = xORB->createInstanceWithArguments(SERVICE_SDB_DOCUMENTDEFINITION,aArguments);
                    Reference<XNameContainer> xNameContainer(xNames,UNO_QUERY);
                    if ( xNameContainer.is() )
                        xNameContainer->insertByName(*pIter,makeAny(xComponent));
                }
            }
        }
        catch(Exception&)
        {
            OSL_ENSURE(0,"convertLinks: Exception catched!");
        }
    }
}
// -----------------------------------------------------------------------------
void closeDocument(const Reference< XModel >& _xDocument)
{
    Reference< XCloseable > xCloseable( _xDocument, UNO_QUERY );
    if ( xCloseable.is() )
    {
        try
        {
            xCloseable->close( sal_True );
        }
        catch( Exception& )
        {
        }
    }
}
// -----------------------------------------------------------------------------
sal_Bool isDocumentReport(const Reference< XMultiServiceFactory >& _xORB,const ::rtl::OUString& _sDocumentLocation)
{
    sal_Bool bReport = sal_False;
    try
    {
        Reference< XModel > xDocument(_xORB->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextDocument"))),UNO_QUERY);
        if ( xDocument.is() )
        {
            Reference< XLoadable > xLoadable( xDocument, UNO_QUERY );
            if ( xLoadable.is() )
            {
                Sequence< PropertyValue > aMedDescr(4);
                sal_Int32 nPos = 0;
                aMedDescr[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AsTemplate"));
                aMedDescr[nPos++].Value <<= sal_False;
                aMedDescr[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Hidden"));
                aMedDescr[nPos++].Value <<= sal_False;
                aMedDescr[nPos].Name = PROPERTY_URL;
                aMedDescr[nPos++].Value <<= _sDocumentLocation;
                aMedDescr[nPos].Name = ::rtl::OUString::createFromAscii( "ReadOnly" );
                aMedDescr[nPos++].Value <<= sal_True;
                Reference< XTypeDetection > xTypeDetection(_xORB->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.TypeDetection")) ),UNO_QUERY );

                if ( !xTypeDetection.is() )
                    throw RuntimeException(); // TODO

                // get TypeName
                ::rtl::OUString aTypeName = xTypeDetection->queryTypeByDescriptor( aMedDescr, sal_True );
                const PropertyValue* pIter = aMedDescr.getConstArray();
                const PropertyValue* pEnd     = pIter + aMedDescr.getLength();
                for( ; pIter != pEnd && !pIter->Name.equalsAscii( "FilterName" ); ++pIter)
                    ;
                if ( aTypeName.getLength() && pIter == pEnd )
                {
                    Reference<XNameAccess> xNameAccess( xTypeDetection, UNO_QUERY );
                    if ( xNameAccess.is() && xNameAccess->hasByName( aTypeName ) )
                    {
                        Sequence<PropertyValue> aTypes;
                        xNameAccess->getByName(aTypeName) >>= aTypes;
                        const PropertyValue* pIter = aTypes.getConstArray();
                        const PropertyValue* pEnd     = pIter + aTypes.getLength();
                        for( ; pIter != pEnd && !pIter->Name.equalsAscii( "PreferredFilter" ) ; ++pIter)
                            ;
                        if ( pIter != pEnd )
                        {
                            sal_Int32 nLen = aMedDescr.getLength();
                            aMedDescr.realloc(nLen+1);
                            aMedDescr[nLen].Value = pIter->Value;
                            aMedDescr[nLen].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FilterName"));
                        }
                    }
                }
                try
                {
                    xLoadable->load(aMedDescr);

                    Reference< XEventsSupplier> xEventsSup(xDocument,UNO_QUERY);
                    Reference< XNameReplace> xEvents = xEventsSup.is() ? xEventsSup->getEvents() : Reference< XNameReplace>();
                    static const ::rtl::OUString s_sOnNew = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OnNew"));
                    if ( xEvents.is() && xEvents->hasByName(s_sOnNew) )
                    {
                        Sequence<PropertyValue> aEventDesc;
                        if ( (xEvents->getByName(s_sOnNew) >>= aEventDesc ) && aEventDesc.getLength() )
                        {
                            ::rtl::OUString sScript;
                            const PropertyValue* pIter = aEventDesc.getConstArray();
                            const PropertyValue* pEnd  = pIter + aEventDesc.getLength();
                            for( ; pIter != pEnd && !pIter->Name.equalsAscii( "Script" ) ; ++pIter)
                                ;
                            if ( pIter != pEnd && (pIter->Value >>= sScript) )
                                bReport = sScript.equalsAscii("service:com.sun.star.wizards.report.CallReportWizard?fill");
                        }
                    }
                    if ( !bReport )
                    {
                        sal_Bool bForm = sal_False;
                        Reference< XDrawPageSupplier> xDrawPageSup(xDocument,UNO_QUERY);
                        Reference< XFormsSupplier> xFormsSup(xDrawPageSup.is() ? xDrawPageSup->getDrawPage() : Reference< XDrawPage>(),UNO_QUERY);
                        Reference< XNameContainer> xForms(xFormsSup.is() ? xFormsSup->getForms() : Reference< XNameContainer>(),UNO_QUERY);
                        Sequence< ::rtl::OUString> aSeq = xForms.is() ? xForms->getElementNames() : Sequence< ::rtl::OUString>();
                        const ::rtl::OUString* pIter = aSeq.getConstArray();
                        const ::rtl::OUString* pEnd   = pIter + aSeq.getLength();
                        for(;pIter != pEnd && !bForm;++pIter)
                        {
                            Reference< XNameContainer> xControls(xForms->getByName(*pIter),UNO_QUERY);
                            Sequence< ::rtl::OUString> aControlSeq = xControls.is() ? xControls->getElementNames() : Sequence< ::rtl::OUString>();
                            const ::rtl::OUString* pControlIter = aControlSeq.getConstArray();
                            const ::rtl::OUString* pControlEnd  = pControlIter + aControlSeq.getLength();
                            for(;pControlIter != pControlEnd && !bForm;++pControlIter)
                            {
                                Reference<XPropertySet> xProp(xControls->getByName(*pControlIter),UNO_QUERY);
                                sal_Int16 nClassId = 0;
                                const static ::rtl::OUString s_sClassId = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ClassId"));
                                if ( xProp.is()
                                    && xProp->getPropertySetInfo().is()
                                    && xProp->getPropertySetInfo()->hasPropertyByName(s_sClassId)
                                    && (xProp->getPropertyValue(s_sClassId) >>= nClassId) )
                                {
                                    bForm = nClassId != FormComponentType::HIDDENCONTROL;
                                }
                            }
                        }
                        bReport = !bForm;
                    }
                }
                catch( Exception& )
                {
                    closeDocument(xDocument);
                }
            }
            closeDocument(xDocument);
        }
    }
    catch(Exception)
    {
        OSL_ENSURE(0,"isDocumentReport: catched exception!");
    }
    return bReport;
}
// -----------------------------------------------------------------------------
}// dbacfg
// -----------------------------------------------------------------------------
