/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef DBAUI_TOOLS_HXX
#define DBAUI_TOOLS_HXX

#include <connectivity/dbexception.hxx>
#include <comphelper/stl_types.hxx>
#include "TypeInfo.hxx"
#include <editeng/svxenum.hxx>
#include <vcl/taskpanelist.hxx>
#include <connectivity/dbtools.hxx>

#define RET_ALL     10

// we only need forward decl here
namespace com { namespace sun { namespace star {

    namespace beans     { class XPropertySet;}
    namespace container
    {
        class XNameAccess;
        class XHierarchicalNameContainer;
    }
    namespace lang
    {
        class XEventListener;
        class XMultiServiceFactory;
    }
    namespace awt
    {
        struct FontDescriptor;
    }
    namespace sdbc
    {
        class XDatabaseMetaData;
        class XConnection;
    }
    namespace util
    {
        struct URL;
        class XNumberFormatter;
    }
    namespace ucb { class XContent; }
    namespace uno { class XComponentContext; }

}}}

namespace svt
{
    class EditBrowseBox;
}

class Window;
class ToolBox;
class Font;
class SvNumberFormatter;
class SfxFilter;

// .........................................................................
namespace dbaui
{
// .........................................................................

    /** creates a new connection and appends the eventlistener
        @param  _rsDataSourceName       name of the datasource
        @param  _xDatabaseContext       the database context
        @param  _rxContext              the UNO component context
        @param  _rEvtLst                the eventlistener which will be added to the new created connection
        @param  _rOUTConnection         this parameter will be filled with the new created connection
        @return SQLExceptionInfo        contains a SQLException, SQLContext or a SQLWarning when they araised else .isValid() will return false
    */
    ::dbtools::SQLExceptionInfo createConnection(
                                    const ::rtl::OUString& _rsDataSourceName,
                                     const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _xDatabaseContext,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext,
                                    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener>& _rEvtLst,
                                    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _rOUTConnection );
    /** creates a new connection and appends the eventlistener
        @param  _xDataSource            the datasource
        @param  _rxContext              the UNO component context
        @param  _rEvtLst                the eventlistener which will be added to the new created connection
        @param  _rOUTConnection         this parameter will be filled with the new created connection
        @return SQLExceptionInfo        contains a SQLException, SQLContext or a SQLWarning when they araised else .isValid() will return false
    */
    ::dbtools::SQLExceptionInfo createConnection(
                                     const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _xDataSource,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext,
                                    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener>& _rEvtLst,
                                    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _rOUTConnection );

    /**  creates a error dialog which displays the SQLExceptionInfo. Also it supports a "more" button where detailed information are available
        @param  _rInfo                  the error which should be shown, if the info is not valid no error dialog will appear
        @param  _pParent                the parent of the error dialog
        @param  _xFactory               need to create the dialog
    */
    void showError( const ::dbtools::SQLExceptionInfo& _rInfo,
                    Window* _pParent,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xFactory);

    /** fills a map and a vector with localized type names
        @param  _rxConnection   the connection to acces the metadata
        @param  _rsTypeNames    a list of localized type names seperated with ';'
        @param  _rTypeInfoMap   the filled map with the type names
        @param  _rTypeInfoIters the vector filled with map iterators
    */
    void fillTypeInfo(  const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _rxConnection,
                        const String& _rsTypeNames,
                        OTypeInfoMap& _rTypeInfoMap,
                        ::std::vector<OTypeInfoMap::iterator>& _rTypeInfoIters);

    /** fill a column with data of a field description
        @param  _rxColumn   the column which should be filled
        @param  _pFieldDesc the source of the data
    */
    class OFieldDescription;
    void setColumnProperties(   const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxColumn,
                                const OFieldDescription* _pFieldDesc);

    ::rtl::OUString createDefaultName(  const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData>& _xMetaData,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _xTables,
                                        const ::rtl::OUString& _sName);

    /** checks if the given name exists in the database context
    */
    sal_Bool checkDataSourceAvailable(  const ::rtl::OUString& _sDataSourceName,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext);

    /** maps SvxCellHorJustify to com::sun::star::awt::TextAlign
        @param SvxCellHorJustify& _eAlignment
        @return the corresponding com::sun::star::awt::TextAlign
    */
    sal_Int32 mapTextAllign(const SvxCellHorJustify& _eAlignment);

    /** retrieves a data source given by name or URL, and displays an error if this fails

        Any <type scope="com::sun::star::sdbc">SQLException</type>s which occur will be displayed.
        Additionally, and Exceptions which indicate a data source name pointing to a non-existent database
        URL will also be denoted. Yet more additionally, and other exceptions will be forwarded to
        a <type scope="com::sun::star::sdb">InteractionHandler</type>.

        @param _rDataSourceName
            the URL of the database document, or the name of a registered data source
        @param _pErrorMessageParent
            the window to use as parent for error messages
        @param _rxORB
            a service factory to use for components to be created
        @param _pErrorInfo
            takes the error info in case of failure. If <NULL/>, the error is displayed to the user.
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource >
        getDataSourceByName(
                const ::rtl::OUString& _rDataSourceName,
                Window* _pErrorMessageParent,
                ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > _rxORB,
                ::dbtools::SQLExceptionInfo* _pErrorInfo
            );

    /** returns either the model when data source is given as parameter,
        or returns a data source when a model is given.
        @param _xObject Either a data source or a model.
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getDataSourceOrModel(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xObject);

    /** maps com::sun::star::awt::TextAlign to SvxCellHorJustify
        @param com::sun::star::awt::TextAlign& _nAlignment
        @return the corresponding SvxCellHorJustify
    */
    SvxCellHorJustify mapTextJustify(const sal_Int32& _nAlignment);

    /** convert Font to ::com::sun::star::awt::FontDescriptor
        @param  _rFont  Font to be converted
        @return the new FontDescriptor
    */
    ::com::sun::star::awt::FontDescriptor CreateFontDescriptor( const Font& _rFont );

    /** call teh format dialog and set the selected format at the column
        @param  _xAffectedCol   Font to be converted
        @param  _xField         Font to be converted
    */
    void callColumnFormatDialog(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xAffectedCol,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xField,
                                SvNumberFormatter* _pFormatter,
                                Window* _pParent);

    /** second variant of the function before
    */
    sal_Bool callColumnFormatDialog(Window* _pParent,
                                    SvNumberFormatter* _pFormatter,
                                    sal_Int32 _nDataType,
                                    sal_Int32& _nFormatKey,
                                    SvxCellHorJustify& _eJustify,
                                    sal_uInt16& _nFlags,
                                    sal_Bool  _bHasFormat);
    /** append a name to tablefilter of a datasource
        @param  _xConnection    the connection is need to get the datasource
        @param  _sName          the name which should be appended
        @param  _xFactory       needed to check if datasource is available
        @param  _pParent        needed when an error must be shown
        @return false when datsource is not available otherwise true
    */
    sal_Bool appendToFilter(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection,
                            const ::rtl::OUString& _sName,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext,
                            Window* _pParent);

    /** notifySystemWindow adds or remove the given window _pToRegister at the Systemwindow found when search _pWindow.
        @param  _pWindow
            The window which is used to search for the SystemWindow.
        @param  _pToRegister
            The window which should be added or removed on the TaskPaneList.
        @param  _rMemFunc
            The member function which should be called at the SystemWindow when found.
            Possible values are:
            ::comphelper::mem_fun(&TaskPaneList::AddWindow)
            ::comphelper::mem_fun(&TaskPaneList::RemoveWindow)
    */
    void notifySystemWindow(Window* _pWindow,
                            Window* _pToRegister,
                            ::comphelper::mem_fun1_t<TaskPaneList,Window*> _rMemFunc);

    /** adjustToolBoxSize checks if the size of the ToolBox is still valid. If not it will be resized.
        @param  _pToolBox
            The Toolbox which should be resized.
    */
    void adjustToolBoxSize(ToolBox* _pToolBox);

    void adjustBrowseBoxColumnWidth( ::svt::EditBrowseBox* _pBox, sal_uInt16 _nColId );

    /** check if SQL92 name checking is enabled
        @param  _xConnection
            Used to get the datasource as parent from the connection.
        @return
            <TRUE/> if so otherwise <FALSE/>
    */
    sal_Bool isSQL92CheckEnabled(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection);

    /** check if the alias name of the table should be added at select statements
        @param  _xConnection
            Used to get the datasource as parent from the connection.
        @return
            <TRUE/> if so otherwise <FALSE/>
    */
    sal_Bool isAppendTableAliasEnabled(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection);

    /** determines whether when generating SQL statements, AS should be placed before a table alias
    */
    sal_Bool generateAsBeforeTableAlias( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _rxConnection );

    /** fills the bool and string value with information out of the datasource info property
        @param  _xDatasource
            Asked for the properties.
        @param  _rAutoIncrementValueEnabled
            <OUT/> Set to sal_True when the property was set in the datasource.
        @param  _rsAutoIncrementValue
            <OUT/> Set to the value when the property was set in the datasource.
    */
    void fillAutoIncrementValue(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xDatasource
                                ,sal_Bool& _rAutoIncrementValueEnabled
                                ,::rtl::OUString& _rsAutoIncrementValue);

    /** fills the bool and string value with information out of the datasource info property
        @param  _xConnection
            Used to get the datasource as parent from the connection.
        @param  _rAutoIncrementValueEnabled
            <OUT/> Set to sal_True when the property was set in the datasource.
        @param  _rsAutoIncrementValue
            <OUT/> Set to the value when the property was set in the datasource.
    */
    void fillAutoIncrementValue(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection
                                ,sal_Bool& _rAutoIncrementValueEnabled
                                ,::rtl::OUString& _rsAutoIncrementValue);

    /** creates the URL or the help agent
        @param  _sModuleName
        @param  _nHelpId
        @return
            The URL for the help agent to dispatch.
    */
    ::com::sun::star::util::URL createHelpAgentURL(const ::rtl::OUString& _sModuleName,const rtl::OString& _rHelpId);

    /** set the evaluation flag at the number formatter
        @param  _rxFormatter
    */
    void setEvalDateFormatForFormatter(::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxFormatter);

    /** query for a type info which can be used to create a primary key column
        @param  _rTypeInfo
            The map which contains all available types.
        @return
            The type info which can be used to create a primary key column.
    */
    TOTypeInfoSP queryPrimaryKeyType(const OTypeInfoMap& _rTypeInfo);

    /** query for a specific type.
        @param  _nDataType
            The type we are searching.
        @param  _rTypeInfo
            The map which contains all available types.
        @return
            The type or <NULL/> if we can't find it.
    */
    TOTypeInfoSP queryTypeInfoByType(sal_Int32 _nDataType,const OTypeInfoMap& _rTypeInfo);

    /** returns the configuration node name of user defined drivers.
        @return
            the configuration node name of user defined drivers.
    */

    /** returns the result of the user action when view the query dialog.
        @param  _pParent
            The parent of the dialog
        @param  _nTitle
            A string resource id for the text which will be displayed as title.
        @param  _nText
            A string resource id for the text which will be displayed above the buttons.
            When the string contains a #1. This will be replaced by the name.
        @param  _bAll
            When set to <TRUE/>, the all button will be appended.
        @param  _sName
            The name of the object to ask for.
        @return
            RET_YES, RET_NO, RET_ALL
    */
    sal_Int32 askForUserAction(Window* _pParent,sal_uInt16 _nTitle,sal_uInt16 _nText,sal_Bool _bAll,const ::rtl::OUString& _sName);

    /** creates a new view from a query or table
        @param  _sName
            The name of the view to be created.
        @param  _xConnection
            The source connection.
        @param  _xSourceObject
            The object for which a view should be created.
        @return
            The created view.
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> createView( const ::rtl::OUString& _sName
                                                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection
                                                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xSourceObject);

    /** creates a view with the given command
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> createView(
        const ::rtl::OUString& _rName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection,
        const ::rtl::OUString& _rCommand
    );

    /** returns the stripped database name.
        @param  _xDataSource
            The data source
        @param  _rsDatabaseName
            Will be filled with the original data source if it is empty.
        @return
            The stripped database name either the registered naem or if it is a file url the last segment.
    */
    ::rtl::OUString getStrippedDatabaseName(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xDataSource
                                            ,::rtl::OUString& _rsDatabaseName);

    /** returns the standard database filter
        @retrun
            the filter
    */
    const SfxFilter* getStandardDatabaseFilter();

    /** opens a save dialog to store a form or report folder in the current hierachy.
        @param  _pParent
            The parent of the dialog.
        @param _rxORB
            a multi service factory which can be used to instantiate usual global services
        @param  _xNames
            Where to insert the new object.
        @param  _sParentFolder
            The name of the parent folder.
        @param  _bForm
            <TRUE/> if a form should be inserted
        @param  _bCollection
            A folder should be inserted
        @param  _xContent
            The content which should be copied.
        @param  _bMove
                if <TRUE/> the name of the content must be inserted without any change, otherwise not.
        @return
            <TRUE/> if the insert opertions was successfull, otherwise <FALSE/>.
    */
    sal_Bool insertHierachyElement(
                Window* _pParent,
                const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
                const ::com::sun::star::uno::Reference< ::com::sun::star::container::XHierarchicalNameContainer>& _xNames,
                const String& _sParentFolder,
                sal_Bool _bForm,
                sal_Bool _bCollection = sal_True,
                const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent>& _xContent = NULL,
                sal_Bool _bMove = sal_False
            );

    /** creates a number formatter
        @param  _rxConnection
            The connection is needed to create the formatter
        @param  _rMF
            The multi service factory
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter > getNumberFormatter(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rMF );

    // this completes a help url with the system parameters "Language" and "System"
    // detect installed locale
    void AppendConfigToken( ::rtl::OUString& _rURL, sal_Bool _bQuestionMark );

// .........................................................................
}
// .........................................................................
#endif // DBAUI_TOOLS_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
