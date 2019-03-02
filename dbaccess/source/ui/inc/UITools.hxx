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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_UITOOLS_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_UITOOLS_HXX

#include <connectivity/dbexception.hxx>
#include <comphelper/stl_types.hxx>
#include "TypeInfo.hxx"
#include <editeng/svxenum.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/taskpanelist.hxx>
#include <connectivity/dbtools.hxx>

#include <memory>

#define RET_ALL     100

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
    }
    namespace awt
    {
        struct FontDescriptor;
        class XWindow;
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

namespace vcl { class Window; }
namespace weld {class Window; }
class ToolBox;
namespace vcl { class Font; }
class SvNumberFormatter;
class SfxFilter;

namespace dbaui
{

    /** creates a new connection and appends the eventlistener
        @param  _rsDataSourceName       name of the datasource
        @param  _xDatabaseContext       the database context
        @param  _rxContext              the UNO component context
        @param  _rEvtLst                the eventlistener which will be added to the new created connection
        @param  _rOUTConnection         this parameter will be filled with the new created connection
        @return SQLExceptionInfo        contains a SQLException, SQLContext or a SQLWarning when they araised else .isValid() will return false
    */
    ::dbtools::SQLExceptionInfo createConnection(
                                    const OUString& _rsDataSourceName,
                                    const css::uno::Reference< css::container::XNameAccess >& _xDatabaseContext,
                                    const css::uno::Reference< css::uno::XComponentContext >& _rxContext,
                                    css::uno::Reference< css::lang::XEventListener> const & _rEvtLst,
                                    css::uno::Reference< css::sdbc::XConnection>& _rOUTConnection );
    /** creates a new connection and appends the eventlistener
        @param  _xDataSource            the datasource
        @param  _rxContext              the UNO component context
        @param  _rEvtLst                the eventlistener which will be added to the new created connection
        @param  _rOUTConnection         this parameter will be filled with the new created connection
        @return SQLExceptionInfo        contains a SQLException, SQLContext or a SQLWarning when they araised else .isValid() will return false
    */
    ::dbtools::SQLExceptionInfo createConnection(
                                    const css::uno::Reference< css::beans::XPropertySet >& _xDataSource,
                                    const css::uno::Reference< css::uno::XComponentContext >& _rxContext,
                                    css::uno::Reference< css::lang::XEventListener> const & _rEvtLst,
                                    css::uno::Reference< css::sdbc::XConnection>& _rOUTConnection );

    /** fills a map and a vector with localized type names
        @param  _rxConnection   the connection to access the metadata
        @param  _rsTypeNames    a list of localized type names separated with ';'
        @param  _rTypeInfoMap   the filled map with the type names
        @param  _rTypeInfoIters the vector filled with map iterators
    */
    void fillTypeInfo(  const css::uno::Reference< css::sdbc::XConnection>& _rxConnection,
                        const OUString& _rsTypeNames,
                        OTypeInfoMap& _rTypeInfoMap,
                        std::vector<OTypeInfoMap::iterator>& _rTypeInfoIters);

    /** fill a column with data of a field description
        @param  _rxColumn   the column which should be filled
        @param  _pFieldDesc the source of the data
    */
    class OFieldDescription;
    void setColumnProperties(   const css::uno::Reference< css::beans::XPropertySet>& _rxColumn,
                                const OFieldDescription* _pFieldDesc);

    OUString createDefaultName(  const css::uno::Reference< css::sdbc::XDatabaseMetaData>& _xMetaData,
                                 const css::uno::Reference< css::container::XNameAccess>& _xTables,
                                 const OUString& _sName);

    /** checks if the given name exists in the database context
    */
    bool checkDataSourceAvailable(  const OUString& _sDataSourceName,
                                    const css::uno::Reference< css::uno::XComponentContext >& _rxContext);

    /** maps SvxCellHorJustify to css::awt::TextAlign
        @param SvxCellHorJustify& _eAlignment
        @return the corresponding css::awt::TextAlign
    */
    sal_Int32 mapTextAllign(const SvxCellHorJustify& _eAlignment);

    /** retrieves a data source given by name or URL, and displays an error if this fails

        Any <type scope="css::sdbc">SQLException</type>s which occur will be displayed.
        Additionally, and Exceptions which indicate a data source name pointing to a non-existent database
        URL will also be denoted. Yet more additionally, and other exceptions will be forwarded to
        a <type scope="css::sdb">InteractionHandler</type>.

        @param _rDataSourceName
            the URL of the database document, or the name of a registered data source
        @param _pErrorMessageParent
            the window to use as parent for error messages
        @param _rxContext
            a service factory to use for components to be created
        @param _pErrorInfo
            takes the error info in case of failure. If <NULL/>, the error is displayed to the user.
    */
    css::uno::Reference< css::sdbc::XDataSource >
        getDataSourceByName(
                const OUString& _rDataSourceName,
                vcl::Window* _pErrorMessageParent,
                const css::uno::Reference< css::uno::XComponentContext >& _rxContext,
                ::dbtools::SQLExceptionInfo* _pErrorInfo
            );

    /** returns either the model when data source is given as parameter,
        or returns a data source when a model is given.
        @param _xObject Either a data source or a model.
    */
    css::uno::Reference< css::uno::XInterface > getDataSourceOrModel(const css::uno::Reference< css::uno::XInterface >& _xObject);

    /** maps css::awt::TextAlign to SvxCellHorJustify
        @param css::awt::TextAlign& _nAlignment
        @return the corresponding SvxCellHorJustify
    */
    SvxCellHorJustify mapTextJustify(sal_Int32 _nAlignment);

    /** call the format dialog and set the selected format at the column
        @param  _xAffectedCol   Font to be converted
        @param  _xField         Font to be converted
    */
    void callColumnFormatDialog(const css::uno::Reference< css::beans::XPropertySet>& _xAffectedCol,
                                const css::uno::Reference< css::beans::XPropertySet>& _xField,
                                SvNumberFormatter* _pFormatter,
                                const vcl::Window* _pParent);

    /** second variant of the function before
    */
    bool callColumnFormatDialog(const vcl::Window* _pParent,
                                    SvNumberFormatter* _pFormatter,
                                    sal_Int32 _nDataType,
                                    sal_Int32& _nFormatKey,
                                    SvxCellHorJustify& _eJustify,
                                    bool  _bHasFormat);
    /** append a name to tablefilter of a datasource
        @param  xConnection    the connection is need to get the datasource
        @param  rName          the name which should be appended
        @param  rxContext      needed to check if datasource is available
        @param  pParent        needed when an error must be shown
        @return false when datsource is not available otherwise true
    */
    bool appendToFilter(const css::uno::Reference< css::sdbc::XConnection>& xConnection,
                        const OUString& rName,
                        const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                        weld::Window* pParent);

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
    void notifySystemWindow(vcl::Window const * _pWindow,
                            vcl::Window* _pToRegister,
                            const ::comphelper::mem_fun1_t<TaskPaneList,vcl::Window*>& _rMemFunc);

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
    bool isSQL92CheckEnabled(const css::uno::Reference< css::sdbc::XConnection>& _xConnection);

    /** check if the alias name of the table should be added at select statements
        @param  _xConnection
            Used to get the datasource as parent from the connection.
        @return
            <TRUE/> if so otherwise <FALSE/>
    */
    bool isAppendTableAliasEnabled(const css::uno::Reference< css::sdbc::XConnection>& _xConnection);

    /** determines whether when generating SQL statements, AS should be placed before a table alias
    */
    bool generateAsBeforeTableAlias( const css::uno::Reference< css::sdbc::XConnection>& _rxConnection );

    /** fills the bool and string value with information out of the datasource info property
        @param  _xDatasource
            Asked for the properties.
        @param  _rAutoIncrementValueEnabled
            <OUT/> Set to sal_True when the property was set in the datasource.
        @param  _rsAutoIncrementValue
            <OUT/> Set to the value when the property was set in the datasource.
    */
    void fillAutoIncrementValue(const css::uno::Reference< css::beans::XPropertySet>& _xDatasource
                                ,bool& _rAutoIncrementValueEnabled
                                ,OUString& _rsAutoIncrementValue);

    /** fills the bool and string value with information out of the datasource info property
        @param  _xConnection
            Used to get the datasource as parent from the connection.
        @param  _rAutoIncrementValueEnabled
            <OUT/> Set to sal_True when the property was set in the datasource.
        @param  _rsAutoIncrementValue
            <OUT/> Set to the value when the property was set in the datasource.
    */
    void fillAutoIncrementValue(const css::uno::Reference< css::sdbc::XConnection>& _xConnection
                                ,bool& _rAutoIncrementValueEnabled
                                ,OUString& _rsAutoIncrementValue);

    /** set the evaluation flag at the number formatter
        @param  _rxFormatter
    */
    void setEvalDateFormatForFormatter(css::uno::Reference< css::util::XNumberFormatter > const & _rxFormatter);

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
        @param  pParent
            The parent of the dialog
        @param  pTitle
            A string resource id for the text which will be displayed as title.
        @param  pText
            A string resource id for the text which will be displayed above the buttons.
            When the string contains a #1. This will be replaced by the name.
        @param  bAll
            When set to <TRUE/>, the all button will be appended.
        @param  rName
            The name of the object to ask for.
        @return
            RET_YES, RET_NO, RET_ALL
    */
    sal_Int32 askForUserAction(weld::Window* pParent, const char* pTitle, const char* pText, bool bAll, const OUString& rName);

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
    css::uno::Reference< css::beans::XPropertySet> createView( const OUString& _sName
                                                    ,const css::uno::Reference< css::sdbc::XConnection >& _xConnection
                                                    ,const css::uno::Reference< css::beans::XPropertySet>& _xSourceObject);

    /** creates a view with the given command
    */
    css::uno::Reference< css::beans::XPropertySet> createView(
        const OUString& _rName,
        const css::uno::Reference< css::sdbc::XConnection >& _xConnection,
        const OUString& _rCommand
    );

    /** returns the stripped database name.
        @param  _xDataSource
            The data source
        @param  _rsDatabaseName
            Will be filled with the original data source if it is empty.
        @return
            The stripped database name either the registered name or if it is a file url the last segment.
    */
    OUString getStrippedDatabaseName(const css::uno::Reference< css::beans::XPropertySet>& _xDataSource
                                            ,OUString& _rsDatabaseName);

    /** returns the standard database filter
        @return
            the filter
    */
    std::shared_ptr<const SfxFilter> getStandardDatabaseFilter();

    /** opens a save dialog to store a form or report folder in the current hierarchy.
        @param  _pParent
            The parent of the dialog.
        @param _rxContext
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
            <TRUE/> if the insert opertions was successful, otherwise <FALSE/>.
    */
    bool insertHierachyElement(
                weld::Window* pParent,
                const css::uno::Reference< css::uno::XComponentContext >& _rxContext,
                const css::uno::Reference< css::container::XHierarchicalNameContainer>& _xNames,
                const OUString& _sParentFolder,
                bool _bForm,
                bool _bCollection = true,
                const css::uno::Reference< css::ucb::XContent>& _xContent = nullptr,
                bool _bMove = false
            );

    /** creates a number formatter
        @param  _rxConnection
            The connection is needed to create the formatter
        @param  _rxContext
            The multi service factory
    */
    css::uno::Reference< css::util::XNumberFormatter > getNumberFormatter(const css::uno::Reference< css::sdbc::XConnection >& _rxConnection,const css::uno::Reference< css::uno::XComponentContext >& _rxContext );

}
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_UITOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
